#include "UsbException.h"


UsbException::UsbException(std::string msg) {
    this->msg = msg;
}

const char *UsbException::what( ) const throw() {
    return msg.c_str();
}
