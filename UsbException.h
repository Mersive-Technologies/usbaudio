#ifndef UACPLAY_USBEXCEPTION_H
#define UACPLAY_USBEXCEPTION_H

#include <stdexcept>
#include <string>

class UsbException : public std::runtime_error {
public:
    UsbException( const char* msg );
};

#endif //UACPLAY_USBEXCEPTION_H
