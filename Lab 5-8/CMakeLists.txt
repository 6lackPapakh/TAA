cmake_minimum_required(VERSION 3.10)
project(ProtectUSBMenu)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

find_package(OpenSSL REQUIRED)

add_executable(protectusb_menu
    src/menu_main.cpp
    src/menu_cipher.cpp
    src/file_operations.cpp
    src/usb_detector.cpp
    src/utils.cpp
)

target_link_libraries(protectusb_menu OpenSSL::SSL OpenSSL::Crypto)
