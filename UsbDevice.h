#ifndef UACPLAY_USBDEVICE_H
#define UACPLAY_USBDEVICE_H

#include <mutex>

#define CTRL_EP 0
#define SPEAKER_EP 1
#define SPEAKER_IFACE 2
#define ID_VENDOR 0x046d
#define ID_PRODUCT 0x0867
#define SAMPLE_RATE 32000
#define ISO_PKT_PER_FRAME 6
#define BYTES_PER_ISO_PKT 128
#define TIMEOUT 1000
#define A4 440.0f
#define CHANNEL_CNT 2
#define TAU (M_PI * 2.0)

typedef int16_t SAMPLE_SZ;

class UsbDevice {

    u_int32_t samplesPlayed = 0;
    std::mutex mutex;
    libusb_device_handle *dev_handle;
    libusb_context *ctx = nullptr;
    libusb_transfer *xfers[3];

public:
    UsbDevice( );

    ~UsbDevice( );

    void claimInterface( int id );

    void detachKernel( int interfaceId );

    void play( );

    static void xferComplete( libusb_transfer *transfer );

    libusb_transfer *genXfer( );

    void submitXfer( libusb_transfer *xfer );
};

#endif
