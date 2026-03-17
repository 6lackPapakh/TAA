#include "usb_detector.h"
#include "file_operations.h"
#include "cipher.h"
#include "utils.h"
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

// ── showHelp ──────────────────────────────────────────────────────────────────
void showHelp() {
    std::cout << "\n";
    std::cout << "ProtectUSB - USB Drive Content Protection\n";
    std::cout << "==========================================\n\n";
    std::cout << "Usage:\n";
    std::cout << "  protectusb [command] [arguments]\n\n";
    std::cout << "Commands:\n";
    std::cout << "  scan                - Detect connected USB drives\n";
    std::cout << "  list   <path>       - List files in specified path\n";
    std::cout << "  protect   <path>    - Protect USB drive (encrypt all files)\n";
    std::cout << "  unprotect <path>    - Restore USB drive (decrypt all files)\n";
    std::cout << "  encrypt   <file>    - Encrypt a single file\n";
    std::cout << "  decrypt   <file>    - Decrypt a single file\n";
    std::cout << "  info   <path>       - Show drive information\n";
    std::cout << "  help                - Show this help\n\n";
    std::cout << "Examples:\n";
    std::cout << "  protectusb scan\n";
    std::cout << "  protectusb list /media/usb\n";
    std::cout << "  protectusb protect /media/usb\n";
    std::cout << "  protectusb encrypt secret.txt\n";
}

// ── removeDirectoryIfEmpty ────────────────────────────────────────────────────
static void removeDirectoryIfEmpty(const std::string& path) {
    DIR* dir = opendir(path.c_str());
    if (!dir) return;

    bool empty = true;
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            empty = false;
            break;
        }
    }
    closedir(dir);

    if (empty) rmdir(path.c_str());
}

// ── protectUSBDrive ───────────────────────────────────────────────────────────
bool protectUSBDrive(const std::string& drivePath, const std::string& key) {
    std::cout << "\nProtecting: " << drivePath << std::endl;
    std::cout << "--------------------------------\n";

    // Create Protected sub-folder if it doesn't exist
    std::string protectedDir = drivePath + "/Protected";
    if (!FileOperations::fileExists(protectedDir)) {
        FileOperations::createDirectory(protectedDir);
    }

    // Collect all files recursively
    std::vector<std::string> files = FileOperations::collectFiles(drivePath);
    std::vector<std::string> filesToProtect;

    // Filter: skip already-encrypted files, config files, and Protected folder
    for (size_t i = 0; i < files.size(); i++) {
        if (files[i].find(".encrypted") == std::string::npos &&
            files[i].find("config.dat") == std::string::npos &&
            files[i].find("/Protected") == std::string::npos) {
            filesToProtect.push_back(files[i]);
        }
    }

    if (filesToProtect.empty()) {
        std::cout << "No files to protect found.\n";
        return true;
    }

    std::cout << "Found " << filesToProtect.size() << " file(s) to encrypt.\n\n";

    SimpleXORCipher cipher(key);
    int successCount = 0;

    for (size_t i = 0; i < filesToProtect.size(); i++) {
        std::string encryptedFile = filesToProtect[i] + ".encrypted";
        std::cout << "Encrypting: " << filesToProtect[i] << " ... ";

        if (cipher.encryptFile(filesToProtect[i], encryptedFile)) {
            if (FileOperations::deleteFile(filesToProtect[i])) {
                std::cout << "OK\n";
                successCount++;
            } else {
                std::cout << "Encrypted but failed to delete original\n";
            }
        } else {
            std::cout << "FAILED\n";
        }
    }

    std::cout << "\nDone. Encrypted " << successCount
              << " of " << filesToProtect.size() << " file(s).\n";
    return true;
}

// ── unprotectUSBDrive ─────────────────────────────────────────────────────────
bool unprotectUSBDrive(const std::string& drivePath, const std::string& key) {
    std::cout << "\nUnprotecting: " << drivePath << std::endl;
    std::cout << "--------------------------------\n";

    std::vector<std::string> encryptedFiles =
        FileOperations::findFilesByExtension(drivePath, ".encrypted");

    if (encryptedFiles.empty()) {
        std::cout << "No encrypted files found.\n";
        return true;
    }

    std::cout << "Found " << encryptedFiles.size() << " encrypted file(s).\n\n";

    SimpleXORCipher cipher(key);
    int successCount = 0;

    for (size_t i = 0; i < encryptedFiles.size(); i++) {
        // Strip ".encrypted" from the end to recover the original filename
        std::string enc = encryptedFiles[i];
        std::string originalFile;
        size_t pos = enc.rfind(".encrypted");
        if (pos != std::string::npos) {
            originalFile = enc.substr(0, pos);
        } else {
            originalFile = enc + ".decrypted";
        }

        std::cout << "Decrypting: " << enc << " ... ";

        if (cipher.decryptFile(enc, originalFile)) {
            if (FileOperations::deleteFile(enc)) {
                std::cout << "OK\n";
                successCount++;
            } else {
                std::cout << "Decrypted but failed to delete encrypted file\n";
            }
        } else {
            std::cout << "FAILED\n";
        }
    }

    // Remove Protected folder if it is now empty
    std::string protectedDir = drivePath + "/Protected";
    removeDirectoryIfEmpty(protectedDir);

    std::cout << "\nDone. Decrypted " << successCount
              << " of " << encryptedFiles.size() << " file(s).\n";
    return true;
}

// ── main ──────────────────────────────────────────────────────────────────────
int main(int argc, char* argv[]) {
    if (argc < 2) {
        showHelp();
        return 1;
    }

    std::string command = argv[1];

    if (command == "help") {
        showHelp();

    } else if (command == "scan") {
        std::cout << "Detected USB drives:\n";
        std::cout << "--------------------\n";
        std::vector<std::string> drives = USBDetector::getUSBDrives();
        if (drives.empty()) {
            std::cout << "No USB drives detected.\n";
        } else {
            for (size_t i = 0; i < drives.size(); i++)
                std::cout << i + 1 << ". " << drives[i] << "\n";
        }

    } else if (command == "list") {
        if (argc < 3) { std::cerr << "Error: Please specify path\n"; return 1; }
        FileOperations::listFiles(argv[2], false);

    } else if (command == "info") {
        if (argc < 3) { std::cerr << "Error: Please specify path\n"; return 1; }
        std::cout << USBDetector::getDriveInfo(argv[2]) << std::endl;

    } else if (command == "protect") {
        if (argc < 3) { std::cerr << "Error: Please specify path\n"; return 1; }
        std::string key = Utils::getPassword("Enter encryption key: ");
        if (key.empty()) { std::cerr << "Key cannot be empty.\n"; return 1; }
        protectUSBDrive(argv[2], key);

    } else if (command == "unprotect") {
        if (argc < 3) { std::cerr << "Error: Please specify path\n"; return 1; }
        std::string key = Utils::getPassword("Enter decryption key: ");
        if (key.empty()) { std::cerr << "Key cannot be empty.\n"; return 1; }
        unprotectUSBDrive(argv[2], key);

    } else if (command == "encrypt") {
        if (argc < 3) { std::cerr << "Error: Please specify file path\n"; return 1; }
        std::string key = Utils::getPassword("Enter encryption key: ");
        if (key.empty()) { std::cerr << "Key cannot be empty.\n"; return 1; }
        std::string outputFile = std::string(argv[2]) + ".encrypted";
        SimpleXORCipher cipher(key);
        if (cipher.encryptFile(argv[2], outputFile))
            std::cout << "[OK] Encrypted to: " << outputFile << "\n";
        else
            std::cout << "[FAIL] Encryption failed.\n";

    } else if (command == "decrypt") {
        if (argc < 3) { std::cerr << "Error: Please specify file path\n"; return 1; }
        std::string key = Utils::getPassword("Enter decryption key: ");
        if (key.empty()) { std::cerr << "Key cannot be empty.\n"; return 1; }

        std::string inputFile  = argv[2];
        std::string outputFile;
        size_t pos = inputFile.rfind(".encrypted");
        if (pos != std::string::npos)
            outputFile = inputFile.substr(0, pos);
        else
            outputFile = inputFile + ".decrypted";

        SimpleXORCipher cipher(key);
        if (cipher.decryptFile(inputFile, outputFile))
            std::cout << "[OK] Decrypted to: " << outputFile << "\n";
        else
            std::cout << "[FAIL] Decryption failed.\n";

    } else {
        std::cerr << "Unknown command: " << command << "\n";
        showHelp();
        return 1;
    }

    return 0;
}
