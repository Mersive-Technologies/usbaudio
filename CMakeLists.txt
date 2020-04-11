cmake_minimum_required(VERSION 3.15)
project(uacplay)

set(CMAKE_CXX_STANDARD 20)

include_directories(/usr/include/libusb-1.0)

add_executable(uacplay main.cpp)

target_link_libraries(uacplay usb-1.0)
