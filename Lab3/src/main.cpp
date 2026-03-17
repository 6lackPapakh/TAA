#include <iostream>
#include <string>
#include <limits>
#include "LicenseManager.h"

static void clearInput() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

static int readPositiveInt(const std::string& prompt) {
    int value = 0;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value && value > 0) {
            clearInput();
            return value;
        }
        clearInput();
        std::cout << "  Please enter a positive whole number." << std::endl;
    }
}

static bool readYesNo(const std::string& prompt) {
    char answer;
    while (true) {
        std::cout << prompt << " (y/n): ";
        std::cin >> answer;
        clearInput();
        if (answer == 'y' || answer == 'Y') return true;
        if (answer == 'n' || answer == 'N') return false;
        std::cout << "  Please enter y or n." << std::endl;
    }
}

int main() {
    std::cout << "==================================================" << std::endl;
    std::cout << "    LICENSING SYSTEM - LABORATORY WORK" << std::endl;
    std::cout << "==================================================" << std::endl;
    std::cout << std::endl;

    LicenseManager manager;

    int choice = -1;
    do {
        std::cout << "\n--- SELECT ACTION ---" << std::endl;
        std::cout << "1. Create license" << std::endl;
        std::cout << "2. Verify license key (in memory)" << std::endl;
        std::cout << "3. Show all licenses" << std::endl;
        std::cout << "4. Save license to file" << std::endl;
        std::cout << "5. Verify license from file" << std::endl;
        std::cout << "0. Exit" << std::endl;
        std::cout << "Your choice: ";

        if (!(std::cin >> choice)) {
            clearInput();
            std::cout << "Invalid input. Please enter a number 0-5." << std::endl;
            choice = -1;
            continue;
        }
        clearInput();

        switch (choice) {

        case 1: {
            std::string userName;
            std::cout << "Enter licensee name: ";
            std::getline(std::cin, userName);
            if (userName.empty()) {
                std::cout << "Name cannot be empty." << std::endl;
                break;
            }
            bool permanent = readYesNo("Is this a permanent (perpetual) license?");
            if (permanent) {
                LicenseKey lic = manager.createPerpetualLicense(userName);
                std::cout << "\n[OK] PERPETUAL LICENSE CREATED:" << std::endl;
                lic.printInfo();
            } else {
                int days = readPositiveInt("Enter number of days the license is valid: ");
                LicenseKey lic = manager.createTemporaryLicense(userName, days);
                std::cout << "\n[OK] TEMPORARY LICENSE CREATED (" << days << " day"
                          << (days == 1 ? "" : "s") << "):" << std::endl;
                lic.printInfo();
            }
            break;
        }

        case 2: {
            std::string key;
            std::cout << "Enter license key to verify: ";
            std::getline(std::cin, key);
            if (key.empty()) { std::cout << "Key cannot be empty." << std::endl; break; }
            if (manager.verifyLicense(key))
                std::cout << "[OK] LICENSE IS VALID" << std::endl;
            else
                std::cout << "[FAIL] LICENSE IS INVALID OR NOT FOUND" << std::endl;
            break;
        }

        case 3: {
            manager.listAllLicenses();
            break;
        }

        case 4: {
            std::string key, filename;
            std::cout << "Enter license key to save: ";
            std::getline(std::cin, key);
            std::cout << "Enter output filename (e.g. license.txt): ";
            std::getline(std::cin, filename);
            if (key.empty() || filename.empty()) {
                std::cout << "Key and filename cannot be empty." << std::endl;
                break;
            }
            const LicenseKey* found = manager.findByKey(key);
            if (!found) {
                std::cout << "[FAIL] No license with that key found in memory." << std::endl;
                break;
            }
            if (manager.saveToFile(*found, filename))
                std::cout << "[OK] License saved to: " << filename << std::endl;
            else
                std::cout << "[FAIL] Could not write to file: " << filename << std::endl;
            break;
        }

        case 5: {
            std::string filename;
            std::cout << "Enter license filename to verify: ";
            std::getline(std::cin, filename);
            if (filename.empty()) { std::cout << "Filename cannot be empty." << std::endl; break; }
            if (manager.verifyLicense(filename))
                std::cout << "[OK] FILE LICENSE IS VALID" << std::endl;
            else
                std::cout << "[FAIL] FILE LICENSE IS INVALID OR EXPIRED" << std::endl;
            break;
        }

        case 0:
            std::cout << "Exiting. Goodbye!" << std::endl;
            break;

        default:
            std::cout << "Invalid choice. Please select 0-5." << std::endl;
            break;
        }

    } while (choice != 0);

    return 0;
}
