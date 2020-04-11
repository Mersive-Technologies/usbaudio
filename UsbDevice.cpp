#include <iostream>
#include <libusb.h>
#include <sstream>

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
    int isoPktCnt = 6;
    size_t pktSz = 128;
    auto *buffer = ( unsigned char * ) calloc( pktSz, isoPktCnt );
    int length = pktSz * isoPktCnt;
    int timeout = 1000;
    auto xfer = libusb_alloc_transfer( isoPktCnt );
    libusb_fill_iso_transfer( xfer, dev_handle, SPEAKER_EP, buffer, length, isoPktCnt, xferComplete, this, timeout );
    libusb_set_iso_packet_lengths( xfer, pktSz );
    return xfer;
}

void UsbDevice::xferComplete( struct libusb_transfer *transfer ) {
    auto that = ( UsbDevice * ) transfer->user_data;
    auto xfer = that->genXfer( );
    if ( libusb_submit_transfer( xfer ) != LIBUSB_SUCCESS ) throw new UsbException( "Failed to complete transfer!" );
    std::cout << "Iso transfer complete!";
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