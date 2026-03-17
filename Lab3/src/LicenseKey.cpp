#include "LicenseKey.h"
#include <iostream>
#include <sstream>
#include <iomanip>

LicenseKey::LicenseKey(const std::string& machine, const std::string& user)
    : machineID(machine), licensee(user), isTemporary(false)
{
    issueDate  = time(nullptr);
    expiryDate = 0;
    generateKey();
}

LicenseKey::LicenseKey(const std::string& machine, const std::string& user, int daysValid)
    : machineID(machine), licensee(user), isTemporary(true)
{
    issueDate  = time(nullptr);
    expiryDate = issueDate + static_cast<time_t>(daysValid) * 24 * 60 * 60;
    generateKey();
}

void LicenseKey::generateKey() {
    unsigned int hash = 2166136261u;
    std::string base = machineID + "|" + licensee + "|" + std::to_string(issueDate);
    for (unsigned char c : base) {
        hash ^= c;
        hash *= 16777619u;
    }
    if (isTemporary) {
        for (int shift = 0; shift < 64; shift += 8) {
            unsigned char byte = static_cast<unsigned char>((expiryDate >> shift) & 0xFF);
            hash ^= byte;
            hash *= 16777619u;
        }
    }
    std::stringstream ss;
    ss << std::hex << std::uppercase;
    for (int i = 0; i < 15; ++i) {
        if (i > 0 && i % 5 == 0) ss << "-";
        ss << ((hash >> (i * 2)) & 0xF);
    }
    key = ss.str();
}

bool LicenseKey::isValid(const std::string& currentMachineID) {
    if (currentMachineID != machineID) {
        std::cout << "[License] Machine ID mismatch." << std::endl;
        std::cout << "  Expected : " << machineID << std::endl;
        std::cout << "  Current  : " << currentMachineID << std::endl;
        return false;
    }
    if (isTemporary) {
        time_t now = time(nullptr);
        if (now > expiryDate) {
            std::cout << "[License] License has EXPIRED." << std::endl;
            std::cout << "  Expired on: " << ctime(&expiryDate);
            return false;
        }
        double daysLeft = difftime(expiryDate, now) / (60.0 * 60.0 * 24.0);
        if (daysLeft < 7.0)
            std::cout << "[License] WARNING: License expires in "
                      << static_cast<int>(daysLeft) + 1 << " day(s)." << std::endl;
    }
    return true;
}

void LicenseKey::printInfo() const {
    std::cout << "======================================" << std::endl;
    std::cout << "License Key : " << key      << std::endl;
    std::cout << "Licensee    : " << licensee  << std::endl;
    std::cout << "Machine ID  : " << machineID << std::endl;
    std::cout << "Issue Date  : " << ctime(&issueDate);
    if (isTemporary) {
        std::cout << "Expires     : " << ctime(&expiryDate);
        time_t now = time(nullptr);
        if (now < expiryDate) {
            int daysLeft = static_cast<int>(difftime(expiryDate, now) / (60 * 60 * 24));
            std::cout << "Days Left   : " << daysLeft << std::endl;
        } else {
            std::cout << "Status      : EXPIRED" << std::endl;
        }
        std::cout << "Type        : TEMPORARY" << std::endl;
    } else {
        std::cout << "Type        : PERPETUAL (no expiry)" << std::endl;
    }
    std::cout << "======================================" << std::endl;
}
