#include "HardwareID.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>

std::string HardwareID::combineAndHash(const std::string& data) {
    unsigned long long hash = 5381;
    for (unsigned char c : data)
        hash = ((hash << 5) + hash) + c;
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0') << std::setw(16) << hash;
    return ss.str();
}

std::string HardwareID::getCpuIdLinux() {
    std::ifstream f("/proc/cpuinfo");
    std::string line;
    while (std::getline(f, line)) {
        if (line.find("model name") != std::string::npos) {
            size_t pos = line.find(':');
            if (pos != std::string::npos)
                return line.substr(pos + 2);
        }
    }
    return "UNKNOWN-CPU";
}

std::string HardwareID::getMacAddressLinux() {
    struct ifconf ifc;
    struct ifreq ifr[16];
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) return "00:00:00:00:00:00";

    ifc.ifc_len = sizeof(ifr);
    ifc.ifc_buf = (char*)ifr;
    if (ioctl(sock, SIOCGIFCONF, &ifc) < 0) {
        close(sock);
        return "00:00:00:00:00:00";
    }

    int count = ifc.ifc_len / sizeof(struct ifreq);
    for (int i = 0; i < count; ++i) {
        if (std::string(ifr[i].ifr_name) == "lo") continue;
        if (ioctl(sock, SIOCGIFHWADDR, &ifr[i]) == 0) {
            unsigned char* mac = (unsigned char*)ifr[i].ifr_hwaddr.sa_data;
            bool valid = false;
            for (int j = 0; j < 6; ++j) if (mac[j]) { valid = true; break; }
            if (valid) {
                std::stringstream ss;
                for (int j = 0; j < 6; ++j) {
                    if (j) ss << ":";
                    ss << std::hex << std::setfill('0') << std::setw(2) << (int)mac[j];
                }
                close(sock);
                return ss.str();
            }
        }
    }
    close(sock);
    return "00:00:00:00:00:00";
}

std::string HardwareID::getMotherboardIdLinux() {
    std::ifstream f("/sys/class/dmi/id/board_serial");
    if (f.is_open()) {
        std::string s;
        std::getline(f, s);
        if (!s.empty() && s != "None" && s != "To Be Filled By O.E.M.")
            return s;
    }
    return "UNKNOWN-MB";
}

HardwareID::HardwareID() {
    cpuId         = getCpuIdLinux();
    motherboardId = getMotherboardIdLinux();
    macAddress    = getMacAddressLinux();
}

std::string HardwareID::getMachineID() {
    std::string combined = cpuId + "|" + motherboardId + "|" + macAddress;
    return combineAndHash(combined);
}
