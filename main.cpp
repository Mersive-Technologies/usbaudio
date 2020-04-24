#include <iostream>
#include <stdexcept>
#include "UsbDevice.h"

using namespace std;

int main()
{
    try
    {
        UsbDevice usbDev;

        usbDev.open();
        usbDev.play();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}

