#pragma once
#ifndef USB_DETECTOR_H
#define USB_DETECTOR_H

#include <string>
#include <vector>

class USBDetector {
public:
    static std::vector<std::string> getUSBDrives();
    static bool isUSBDrive(const std::string& path);
    static std::string getDriveInfo(const std::string& path);
};

#endif
