#ifndef UACPLAY_USBDEVICE_H
#define UACPLAY_USBDEVICE_H

#include <mutex>
#include <libusb.h>
#include "libusbwrapper.h"

#define ID_VENDOR 0x046d
#define ID_PRODUCT 0x0867
#define SAMPLE_RATE 32000
#define ISO_PKT_PER_FRAME 6
#define BYTES_PER_ISO_PKT 128
#define TIMEOUT 1000
#define A4 440.0f
#define CHANNEL_CNT 2
#define TAU ( M_PI * 2.0 )

typedef int16_t SAMPLE_SZ;

class UsbDevice
{

    uint32_t         samplesPlayed = 0;
    std::mutex       mutex;
    libusb::context  _ctx;
    libusb::device   _dev;
    libusb_transfer* xfers[3];
    int              _controlInterface = 0;
    int              _speakerInterface = 3;
    int              _alternateSetting = 1;
    uint8_t          _speakerEndpoint  = 1;

public:
    UsbDevice();

    ~UsbDevice();

    void play();

    static void xferComplete( libusb_transfer* transfer );

    libusb_transfer* genXfer();

    void submitXfer( libusb_transfer* xfer );

    void open();
};

#endif
