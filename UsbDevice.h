#ifndef UACPLAY_USBDEVICE_H
#define UACPLAY_USBDEVICE_H

#define CTRL_EP 0
#define SPEAKER_EP 1
#define SPEAKER_IFACE 2
#define ID_VENDOR 0x046d
#define ID_PRODUCT 0x0867

class UsbDevice {

    libusb_device_handle *dev_handle;
    libusb_context *ctx = nullptr;
    libusb_transfer* xfers[3];
    
public:
    UsbDevice( );

    ~UsbDevice( );

    void claimInterface( int id );

    void detachKernel( int interfaceId );

    void play( );

    static void xferComplete( libusb_transfer *transfer );
};

#endif
