#include <libusb.h>
#include "UsbDevice.h"

using namespace std;

int main( ) {

    UsbDevice usbDev;

    usbDev.open();
    usbDev.play();

    return 0;
}

