#include <iostream>
#include <libusb.h>
#include <sstream>
#include <math.h>

#include "UsbDevice.h"
#include "UsbException.h"

UsbDevice::UsbDevice( ) {
    if ( libusb_init( &ctx ) < 0 ) throw UsbException( "Couldn't init usb" );
    libusb_set_debug( ctx, 3 );

    dev_handle = libusb_open_device_with_vid_pid( ctx, ID_VENDOR, ID_PRODUCT );
    if ( dev_handle == nullptr ) throw UsbException( "Couldn't open device" );

    detachKernel( CTRL_EP );
    detachKernel( SPEAKER_IFACE );

    claimInterface( CTRL_EP );
    claimInterface( SPEAKER_IFACE );

    if ( libusb_set_interface_alt_setting( dev_handle, SPEAKER_IFACE, 1 ) < 0 ) {
        throw UsbException( "Can't enable speaker!" );
    }
}

void UsbDevice::play( ) {
    for ( auto &xfer : xfers ) {
        xfer = genXfer( );
        submitXfer( xfer );
    }

    while ( libusb_handle_events( nullptr ) == LIBUSB_SUCCESS );
}

void UsbDevice::submitXfer( libusb_transfer *xfer ) {
    int ret = libusb_submit_transfer( xfer );
    if ( ret < 0 ) {
        std::ostringstream stringStream;
        stringStream << "Can't submit ISO transfer: " << ret;
        throw UsbException( stringStream.str( ));
    }
}

libusb_transfer *UsbDevice::genXfer( ) {
    std::lock_guard<std::mutex> guard( mutex );
    int byteCnt = BYTES_PER_ISO_PKT * ISO_PKT_PER_FRAME;
    int sampleCnt = byteCnt / CHANNEL_CNT / sizeof( SAMPLE_SZ );
    auto *buffer = ( SAMPLE_SZ * ) calloc( sizeof( SAMPLE_SZ ), sampleCnt * CHANNEL_CNT );
    int maxSampVol = pow( 2, sizeof( SAMPLE_SZ ) * 8 ) / 2 - 1;
    for ( int i = 0; i < sampleCnt; i++ ) {
        float seconds = ( float ) ( samplesPlayed + i ) / ( float ) SAMPLE_RATE;
        float cycles = A4 * seconds;
        SAMPLE_SZ samp = sin( cycles * TAU) * maxSampVol;
        buffer[i * 2 + 0] = samp;
        buffer[i * 2 + 1] = samp;
    }
    samplesPlayed += sampleCnt;
    auto xfer = libusb_alloc_transfer( ISO_PKT_PER_FRAME );
    libusb_fill_iso_transfer(
            xfer,
            dev_handle,
            SPEAKER_EP,
            ( unsigned char * ) buffer,
            byteCnt,
            ISO_PKT_PER_FRAME,
            xferComplete,
            this,
            TIMEOUT
    );
    libusb_set_iso_packet_lengths( xfer, BYTES_PER_ISO_PKT );
    return xfer;
}

void UsbDevice::xferComplete( struct libusb_transfer *transfer ) {
    auto that = ( UsbDevice * ) transfer->user_data;
    auto xfer = that->genXfer( );
    if ( libusb_submit_transfer( xfer ) != LIBUSB_SUCCESS ) throw UsbException( "Failed to complete transfer!" );
    std::cout << "Iso transfer complete!\n";
    free( transfer->buffer );
    libusb_free_transfer( transfer );
}

void UsbDevice::detachKernel( int interfaceId ) {
    if ( libusb_kernel_driver_active( dev_handle, interfaceId ) != 1 ) return;
    if ( libusb_detach_kernel_driver( dev_handle, interfaceId ) != 0 ) throw UsbException( "failed to detatch" );
}

void UsbDevice::claimInterface( int interfaceId ) {
    if ( libusb_claim_interface( dev_handle, interfaceId ) < 0 ) throw UsbException( "Couldn't claim interface" );
}

UsbDevice::~UsbDevice( ) {
    if ( libusb_release_interface( dev_handle, CTRL_EP ) != 0 ) throw UsbException( "Couldn't release interface 0" );
    if ( libusb_release_interface( dev_handle, SPEAKER_EP ) != 0 ) throw UsbException( "Couldn't release interface 1" );
    libusb_close( dev_handle );
    libusb_exit( ctx );
}