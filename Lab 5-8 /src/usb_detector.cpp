#include "usb_detector.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <sys/stat.h>
#include <sys/statvfs.h>

// ── Linux USB detection via /proc/mounts ─────────────────────────────────────
// On Linux, USB drives are mounted under /media/<user>/ or /run/media/<user>/
// /proc/mounts lists every currently mounted filesystem with its device path.
// We look for entries whose device path starts with /dev/sd (SCSI/USB block
// devices) and whose mount point is under /media or /run/media.

std::vector<std::string> USBDetector::getUSBDrives() {
    std::vector<std::string> usbDrives;

    std::ifstream mounts("/proc/mounts");
    if (!mounts.is_open()) {
        // Fallback: check common mount points directly
        struct stat st;
        std::vector<std::string> candidates = {
            "/media", "/mnt", "/run/media"
        };
        for (const auto& c : candidates) {
            if (stat(c.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
                usbDrives.push_back(c);
            }
        }
        return usbDrives;
    }

    std::string line;
    while (std::getline(mounts, line)) {
        std::istringstream iss(line);
        std::string device, mountPoint, fsType, options;
        iss >> device >> mountPoint >> fsType >> options;

        // USB block devices start with /dev/sd, /dev/nvme excluded
        bool isBlockDevice = (device.substr(0, 8) == "/dev/sd");

        // Mount point under /media or /run/media indicates removable
        bool isRemovable = (mountPoint.substr(0, 6)  == "/media" ||
                            mountPoint.substr(0, 10) == "/run/media" ||
                            mountPoint.substr(0, 4)  == "/mnt");

        if (isBlockDevice && isRemovable) {
            usbDrives.push_back(mountPoint);
        }
    }

    return usbDrives;
}

bool USBDetector::isUSBDrive(const std::string& path) {
    std::vector<std::string> drives = getUSBDrives();
    for (size_t i = 0; i < drives.size(); i++) {
        if (drives[i] == path) return true;
    }
    return false;
}

std::string USBDetector::getDriveInfo(const std::string& path) {
    std::string info = "Path: " + path + "\n";

    // Get filesystem stats (total/free space)
    struct statvfs vfs;
    if (statvfs(path.c_str(), &vfs) == 0) {
        unsigned long long total = (unsigned long long)vfs.f_blocks * vfs.f_frsize;
        unsigned long long free  = (unsigned long long)vfs.f_bfree  * vfs.f_frsize;
        unsigned long long used  = total - free;

        // Convert to MB for readability
        info += "Total : " + std::to_string(total / (1024 * 1024)) + " MB\n";
        info += "Used  : " + std::to_string(used  / (1024 * 1024)) + " MB\n";
        info += "Free  : " + std::to_string(free  / (1024 * 1024)) + " MB\n";
    }

    // Try to find the device from /proc/mounts
    std::ifstream mounts("/proc/mounts");
    if (mounts.is_open()) {
        std::string line;
        while (std::getline(mounts, line)) {
            std::istringstream iss(line);
            std::string device, mountPoint, fsType;
            iss >> device >> mountPoint >> fsType;
            if (mountPoint == path) {
                info += "Device: " + device + "\n";
                info += "FS Type: " + fsType + "\n";
                break;
            }
        }
    }

    return info;
}
