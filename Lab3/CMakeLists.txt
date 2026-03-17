cmake_minimum_required(VERSION 3.10)
project(LicensingSystem)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(LicensingSystem
    src/main.cpp
    src/HardwareID.cpp
    src/LicenseKey.cpp
    src/LicenseManager.cpp
)
