#ifndef UACPLAY_USBEXCEPTION_H
#define UACPLAY_USBEXCEPTION_H

#include <exception>
#include <string>

class UsbException : public std::exception {

    virtual const char *what( ) const throw( );

    std::string msg;
public:
    UsbException( std::string msg );
};

#endif //UACPLAY_USBEXCEPTION_H
