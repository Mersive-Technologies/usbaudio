#ifndef UACPLAY_USBDEVICE_H
#define UACPLAY_USBDEVICE_H


class UsbDevice {

    libusb_device_handle *dev_handle;
    libusb_context *ctx = nullptr;
    
public:
    UsbDevice( );

    ~UsbDevice( );

    void ClaimInterface( int id );

    void DetatchKernel( int interfaceId );
};

#endif
