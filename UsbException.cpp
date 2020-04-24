#include "UsbException.h"

UsbException::UsbException(const char* msg) : std::runtime_error(msg) {}
