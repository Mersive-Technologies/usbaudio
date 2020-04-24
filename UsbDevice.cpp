#include <iostream>
#include <stdint.h>
#include <memory>
#include <sstream>
#include <type_traits>
#include <utility>
#define _USE_MATH_DEFINES
#include <math.h>

#include "UsbDevice.h"
#include "UsbException.h"

UsbDevice::UsbDevice()
    : _ctx()
    , _dev( std::move( _ctx.open_device_with_vid_pid( ID_VENDOR, ID_PRODUCT ) ) )
{
}

void UsbDevice::open()
{
    // Get device information
    auto deviceDesc = _dev.get_device_descriptor();

    auto config = _dev.get_active_config_descriptor();
    for ( auto i : config.interfaces() )
    {
        for ( uint8_t j = 0; j < i.num_altsetting; ++j )
        {
            auto& altsetting = i.altsetting[j];
            switch ( altsetting.bInterfaceClass )
            {
            case libusb_class_code::LIBUSB_CLASS_AUDIO:
                switch ( altsetting.bInterfaceSubClass )
                {
                case 1:
                    _controlInterface = altsetting.bInterfaceNumber;
                    break;
                case 2:
                    if ( altsetting.bNumEndpoints )
                    {
                        auto endpoint = altsetting.endpoint;
                        std::cout << "Endpoint address " << static_cast< int >( endpoint->bEndpointAddress )
                                  << std::endl;
                        if ( ( endpoint->bEndpointAddress & LIBUSB_ENDPOINT_IN ) != LIBUSB_ENDPOINT_IN )
                        {
                            _speakerInterface = altsetting.bInterfaceNumber;
                            _alternateSetting = altsetting.bAlternateSetting;
                            _speakerEndpoint  = ( endpoint->bEndpointAddress & 0x0F );
                        }
                    }
                    break;
                }
                break;
            }
        }
    }

    _dev.detach_kernel_driver( _controlInterface );
    _dev.detach_kernel_driver( _speakerInterface );

    _dev.claim_interface( _controlInterface );
    _dev.claim_interface( _speakerInterface );

    _dev.set_interface_alt_setting( _speakerInterface, _alternateSetting );
}

void UsbDevice::play()
{
    for ( auto& xfer : xfers )
    {
        xfer = genXfer();
        submitXfer( xfer );
    }

    while ( libusb_handle_events( _ctx ) == LIBUSB_SUCCESS )
        ;
}

void UsbDevice::submitXfer( libusb_transfer* xfer )
{
    int ret = libusb_submit_transfer( xfer );
    if ( ret < 0 )
    {
        std::ostringstream stringStream;
        stringStream << "Can't submit ISO transfer: " << ret;
        throw UsbException( stringStream.str().c_str() );
    }
}

libusb_transfer* UsbDevice::genXfer()
{
    std::lock_guard< std::mutex >  guard( mutex );
    int                            byteCnt    = BYTES_PER_ISO_PKT * ISO_PKT_PER_FRAME;
    int                            sampleCnt  = byteCnt / CHANNEL_CNT / sizeof( SAMPLE_SZ );
    std::unique_ptr< SAMPLE_SZ[] > buffer     = std::make_unique< SAMPLE_SZ[] >( sampleCnt * CHANNEL_CNT );
    int                            maxSampVol = pow( 2, sizeof( SAMPLE_SZ ) * 8 ) / 2 - 1;
    for ( int i = 0; i < sampleCnt; i++ )
    {
        float     seconds = (float)( samplesPlayed + i ) / (float)SAMPLE_RATE;
        float     cycles  = A4 * seconds;
        SAMPLE_SZ samp    = sin( cycles * TAU ) * maxSampVol;
        buffer[i * 2 + 0] = samp;
        buffer[i * 2 + 1] = samp;
    }
    samplesPlayed += sampleCnt;
    auto xfer = libusb_alloc_transfer( ISO_PKT_PER_FRAME );
    libusb_fill_iso_transfer(
            xfer,
            _dev,
            _speakerEndpoint,
            reinterpret_cast< uint8_t* >( buffer.release() ),
            byteCnt,
            ISO_PKT_PER_FRAME,
            xferComplete,
            this,
            TIMEOUT );
    libusb_set_iso_packet_lengths( xfer, BYTES_PER_ISO_PKT );
    return xfer;
}

void UsbDevice::xferComplete( struct libusb_transfer* transfer )
{
    std::unique_ptr< SAMPLE_SZ[] > buffer( reinterpret_cast< SAMPLE_SZ* >( transfer->buffer ) );

    auto that = (UsbDevice*)transfer->user_data;
    auto xfer = that->genXfer();
    if ( libusb_submit_transfer( xfer ) != LIBUSB_SUCCESS )
        throw UsbException( "Failed to complete transfer!" );
    // std::cout << "Iso transfer complete!\n";
    libusb_free_transfer( transfer );
}

UsbDevice::~UsbDevice()
{
}
