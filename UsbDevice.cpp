#include <libusb.h>
#include <iostream>

#include "UsbDevice.h"
#include "UsbException.h"

UsbDevice::UsbDevice( ) {
    libusb_device **devs;
    if ( libusb_init( &ctx ) < 0 ) throw UsbException( "Couldn't init usb" );

    libusb_set_debug( ctx, 3 );
    if ( libusb_get_device_list( ctx, &devs ) < 0 ) throw UsbException( "Couldn't get device list" );

    dev_handle = libusb_open_device_with_vid_pid( ctx, ID_VENDOR, ID_PRODUCT );
    if ( dev_handle == nullptr ) throw UsbException( "Couldn't open device" );
    libusb_free_device_list( devs, 1 );

    detachKernel( CTRL_EP );
    detachKernel( SPEAKER_EP );

    claimInterface( CTRL_EP );
    claimInterface( SPEAKER_EP );

    if ( libusb_set_interface_alt_setting( dev_handle, SPEAKER_EP, 1 ) < 0 ) {
        throw UsbException( "Can't enable interface!" );
    }
}

void UsbDevice::play() {
    int isoPktCnt = 6;
    size_t pktSz = 128;
    auto *buffer = (unsigned char *)calloc(pktSz, isoPktCnt);
    int length = pktSz * isoPktCnt;
    int timeout = 1000;

    for(auto & xfer : xfers) {
        xfer = libusb_alloc_transfer(isoPktCnt);
        libusb_fill_iso_transfer(xfer, dev_handle, SPEAKER_EP, buffer, length, isoPktCnt, xferComplete, this, timeout);
        if(libusb_submit_transfer(xfer) < 0) throw UsbException( "Can't submit ISO transfer!" );
    }

    while(libusb_handle_events( nullptr ) == 0);
}

void UsbDevice::xferComplete(struct libusb_transfer *transfer) {
    auto that = (UsbDevice*)transfer->user_data;
    std::cout << "Iso transfer complete!";
    libusb_free_transfer(transfer);
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