#include <iostream>
#include <libusb.h>

#include "UsbDevice.h"
#include "UsbException.h"

UsbDevice::UsbDevice() {
    libusb_device **devs;
    int ret = libusb_init( &ctx );
    if ( ret < 0 ) throw UsbException("Couldn't init usb");

    libusb_set_debug( ctx, 3 );
    ssize_t cnt = libusb_get_device_list( ctx, &devs );
    if ( cnt < 0 ) throw UsbException("Couldn't get device list");

    uint16_t idVendor = 0x046d;
    uint16_t idProduct = 0x0867;
    dev_handle = libusb_open_device_with_vid_pid( ctx, idVendor, idProduct );
    if ( dev_handle == nullptr ) throw UsbException("Couldn't open device");
    libusb_free_device_list( devs, 1 );

    DetatchKernel(0);
    DetatchKernel(1);

    ClaimInterface(0);
    ClaimInterface(1);
}

void UsbDevice::DetatchKernel(int interfaceId ) {
    if ( libusb_kernel_driver_active( dev_handle, interfaceId ) != 1 ) return;
    if ( libusb_detach_kernel_driver( dev_handle, interfaceId ) != 0 )  throw UsbException("failed to detatch");
}

void UsbDevice::ClaimInterface(int interfaceId) {
    int ret = libusb_claim_interface( dev_handle, interfaceId );
    if ( ret < 0 ) throw UsbException("Couldn't claim interface");
}

UsbDevice::~UsbDevice( ) {
    int ret = libusb_release_interface( dev_handle, 0 );
    if ( ret != 0 ) throw UsbException("Couldn't release interface 0");
    ret = libusb_release_interface( dev_handle, 1 );
    if ( ret != 0 ) throw UsbException("Couldn't release interface 1");
    libusb_close( dev_handle );
    libusb_exit( ctx );
}