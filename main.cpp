#include <iostream>
#include <libusb.h>
#include "UsbDevice.h"

using namespace std;

int main( ) {

    UsbDevice usbDev;

    usbDev.play();

//    auto *data = new unsigned char[4]; //data to write
//    data[0] = 'a';
//    data[1] = 'b';
//    data[2] = 'c';
//    data[3] = 'd'; //some dummy values

//    cout << "Data->" << data << "<-" << endl; //just to see the data we want to write : abcd
//    cout << "Writing Data..." << endl;
//    r = libusb_bulk_transfer( dev_handle, ( 2 | LIBUSB_ENDPOINT_OUT ), data, 4, &actual,
//                              0 ); //my device's out endpoint was 2, found with trial- the device had 2 endpoints: 2 and 129
//    if ( r == 0 && actual == 4 ) //we wrote the 4 bytes successfully
//        cout << "Writing Successful!" << endl;
//    else
//        cout << "Write Error" << endl;

//    delete[] data; //delete the allocated memory for data
    return 0;
}

