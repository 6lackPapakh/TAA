cmake_minimum_required(VERSION 3.10)
project(ProtectUSB)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(protectusb
    src/main.cpp
    src/cipher.cpp
    src/file_operations.cpp
    src/usb_detector.cpp
    src/utils.cpp
)
