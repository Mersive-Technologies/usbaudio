cmake_minimum_required(VERSION 3.15)
project(uacplay)

set(CMAKE_CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED 20)

if (WIN32)
include_directories(./libusb/include/libusb-1.0)
endif (WIN32)
if (UNIX)
include_directories(/usr/include/libusb-1.0)
endif (UNIX)

add_executable(uacplay
        UsbDevice.cpp
        main.cpp
        UsbException.cpp UsbException.h)

if (UNIX)
target_link_libraries(uacplay usb-1.0)
endif (UNIX)
if (WIN32)
target_link_libraries(uacplay ${PROJECT_SOURCE_DIR}/libusb/MS64/dll/libusb-1.0.lib)
add_custom_command(TARGET uacplay POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/libusb/MS64/dll/libusb-1.0.dll"
        $<TARGET_FILE_DIR:uacplay>)
endif (WIN32)

