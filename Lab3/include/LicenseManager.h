#pragma once
#ifndef LICENSEMANAGER_H
#define LICENSEMANAGER_H

#include <vector>
#include <string>
#include "LicenseKey.h"

class LicenseManager {
private:
    std::vector<LicenseKey> licenses;
    std::string currentMachineID;

    bool verifyFromFile(const std::string& filename);

public:
    LicenseManager();

    LicenseKey createPerpetualLicense(const std::string& userName);
    LicenseKey createTemporaryLicense(const std::string& userName, int days);

    bool verifyLicense(const std::string& licenseKey);
    bool saveToFile(const LicenseKey& license, const std::string& filename);
    void listAllLicenses() const;
    const LicenseKey* findByKey(const std::string& key) const;

    std::string getCurrentMachineID() const { return currentMachineID; }
};

#endif
