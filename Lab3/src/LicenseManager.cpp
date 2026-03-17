#include "LicenseManager.h"
#include "HardwareID.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>

LicenseManager::LicenseManager() {
    HardwareID hwid;
    currentMachineID = hwid.getMachineID();
    std::cout << "------------------------------------------------" << std::endl;
    std::cout << "  Hardware Fingerprint" << std::endl;
    std::cout << "------------------------------------------------" << std::endl;
    std::cout << "  Machine ID  : " << currentMachineID       << std::endl;
    std::cout << "  CPU ID      : " << hwid.getCpuId()         << std::endl;
    std::cout << "  Motherboard : " << hwid.getMotherboardId() << std::endl;
    std::cout << "  MAC Address : " << hwid.getMacAddress()    << std::endl;
    std::cout << "------------------------------------------------" << std::endl;
    std::cout << std::endl;
}

LicenseKey LicenseManager::createPerpetualLicense(const std::string& userName) {
    LicenseKey license(currentMachineID, userName);
    licenses.push_back(license);
    std::cout << "[LicenseManager] Perpetual license created for: " << userName << std::endl;
    return license;
}

LicenseKey LicenseManager::createTemporaryLicense(const std::string& userName, int days) {
    LicenseKey license(currentMachineID, userName, days);
    licenses.push_back(license);
    std::cout << "[LicenseManager] Temporary license (" << days << " days) created for: " << userName << std::endl;
    return license;
}

bool LicenseManager::verifyLicense(const std::string& licenseKey) {
    for (auto& lic : licenses) {
        if (lic.getKey() == licenseKey) {
            bool valid = lic.isValid(currentMachineID);
            if (valid)
                std::cout << "[LicenseManager] In-memory license is VALID." << std::endl;
            return valid;
        }
    }
    std::cout << "[LicenseManager] Key not in memory. Checking file: " << licenseKey << std::endl;
    return verifyFromFile(licenseKey);
}

bool LicenseManager::saveToFile(const LicenseKey& license, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "[LicenseManager] ERROR: Cannot open file: " << filename << std::endl;
        return false;
    }
    file << "KEY="      << license.getKey()       << "\n";
    file << "MACHINE="  << license.getMachineID()  << "\n";
    file << "TYPE="     << (license.isTemporaryLicense() ? "TEMPORARY" : "PERPETUAL") << "\n";
    file << "ISSUED="   << license.getIssueDate()  << "\n";
    if (license.isTemporaryLicense())
        file << "EXPIRY=" << license.getExpiryDate() << "\n";
    file << "LICENSEE=" << license.getLicensee() << "\n";
    file.close();
    std::cout << "[LicenseManager] License saved to: " << filename << std::endl;
    return true;
}

bool LicenseManager::verifyFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "[LicenseManager] File not found: " << filename << std::endl;
        return false;
    }
    std::string savedKey, savedMachine, savedType, savedLicensee;
    time_t savedExpiry = 0;
    std::string line;
    while (std::getline(file, line)) {
        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;
        std::string name  = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        if      (name == "KEY")      savedKey      = value;
        else if (name == "MACHINE")  savedMachine  = value;
        else if (name == "TYPE")     savedType     = value;
        else if (name == "EXPIRY")   savedExpiry   = static_cast<time_t>(std::stoll(value));
        else if (name == "LICENSEE") savedLicensee = value;
    }
    file.close();
    if (savedMachine != currentMachineID) {
        std::cout << "[LicenseManager] ERROR: License is for a different machine." << std::endl;
        std::cout << "  Stored  : " << savedMachine     << std::endl;
        std::cout << "  Current : " << currentMachineID << std::endl;
        return false;
    }
    if (savedType == "TEMPORARY") {
        if (savedExpiry == 0) {
            std::cout << "[LicenseManager] ERROR: Temporary license has no expiry date." << std::endl;
            return false;
        }
        time_t now = time(nullptr);
        if (now > savedExpiry) {
            std::cout << "[LicenseManager] License EXPIRED on: " << ctime(&savedExpiry);
            return false;
        }
        int daysLeft = static_cast<int>(difftime(savedExpiry, now) / (60 * 60 * 24));
        std::cout << "[LicenseManager] Temporary license valid. Days remaining: " << daysLeft << std::endl;
    }
    std::cout << "[LicenseManager] File-based license is VALID." << std::endl;
    std::cout << "  Licensee : " << savedLicensee << std::endl;
    std::cout << "  Type     : " << savedType     << std::endl;
    return true;
}

void LicenseManager::listAllLicenses() const {
    if (licenses.empty()) {
        std::cout << "[LicenseManager] No licenses in memory." << std::endl;
        return;
    }
    std::cout << "\n======== ALL LICENSES (" << licenses.size() << ") ========" << std::endl;
    for (const auto& lic : licenses)
        lic.printInfo();
}

const LicenseKey* LicenseManager::findByKey(const std::string& key) const {
    for (const auto& lic : licenses)
        if (lic.getKey() == key) return &lic;
    return nullptr;
}
