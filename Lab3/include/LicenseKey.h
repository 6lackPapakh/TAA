#pragma once
#ifndef LICENSEKEY_H
#define LICENSEKEY_H

#include <string>
#include <ctime>

class LicenseKey {
private:
    std::string key;
    std::string machineID;
    time_t issueDate;
    time_t expiryDate;
    std::string licensee;
    bool isTemporary;

    void generateKey();

public:
    LicenseKey(const std::string& machine, const std::string& user);
    LicenseKey(const std::string& machine, const std::string& user, int daysValid);

    bool isValid(const std::string& currentMachineID);
    void printInfo() const;

    std::string getKey()       const { return key; }
    std::string getMachineID() const { return machineID; }
    bool isTemporaryLicense()  const { return isTemporary; }
    std::string getLicensee()  const { return licensee; }
    time_t getIssueDate()      const { return issueDate; }
    time_t getExpiryDate()     const { return expiryDate; }
};

#endif
