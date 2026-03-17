#pragma once
#ifndef HARDWAREID_H
#define HARDWAREID_H

#include <string>

class HardwareID {
private:
    std::string cpuId;
    std::string motherboardId;
    std::string macAddress;

    std::string combineAndHash(const std::string& data);
    std::string getCpuIdLinux();
    std::string getMacAddressLinux();
    std::string getMotherboardIdLinux();

public:
    HardwareID();

    std::string getMachineID();

    std::string getCpuId()         { return cpuId; }
    std::string getMotherboardId() { return motherboardId; }
    std::string getMacAddress()    { return macAddress; }
};

#endif
