#include "usb_detector.h"
#include "file_operations.h"
#include "menu_cipher.h"
#include "utils.h"

#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

namespace {
void showMenuHelp() {
    std::cout << "\n";
    std::cout << "ProtectUSB Menu Edition\n";
    std::cout << "======================\n\n";
    std::cout << "This copy keeps the original protectusb workflow intact and adds\n";
    std::cout << "a menu so you can choose compression and encryption per operation.\n\n";
    std::cout << "Example:\n";
    std::cout << "  1. Choose 'Encrypt a single file'\n";
    std::cout << "  2. Enter secret.txt\n";
    std::cout << "  3. Choose AES-256, ChaCha20, or another algorithm\n";
    std::cout << "  4. Choose RLE or no compression\n\n";
}

void removeDirectoryIfEmpty(const std::string& path) {
    DIR* dir = opendir(path.c_str());
    if (!dir) {
        return;
    }

    bool empty = true;
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            empty = false;
            break;
        }
    }

    closedir(dir);
    if (empty) {
        rmdir(path.c_str());
    }
}

EncryptionAlgorithm chooseAlgorithm(const std::string& prompt, bool allowAuto) {
    while (true) {
        std::cout << "\n" << prompt << "\n";
        if (allowAuto) {
            std::cout << "  0. Auto-detect from file header\n";
        }
        std::cout << "  1. XOR\n";
        std::cout << "  2. Caesar\n";
        std::cout << "  3. Vigenere\n";
        std::cout << "  4. XorShiftStream\n";
        std::cout << "  5. RotateXOR\n";
        std::cout << "  6. AES-256\n";
        std::cout << "  7. ChaCha20\n";

        std::string input = Utils::getUserInput("Select option: ");
        if (allowAuto && input == "0") {
            return EncryptionAlgorithm::Unknown;
        }
        if (input == "1") {
            return EncryptionAlgorithm::XOR;
        }
        if (input == "2") {
            return EncryptionAlgorithm::Caesar;
        }
        if (input == "3") {
            return EncryptionAlgorithm::Vigenere;
        }
        if (input == "4") {
            return EncryptionAlgorithm::XorShiftStream;
        }
        if (input == "5") {
            return EncryptionAlgorithm::RotateXOR;
        }
        if (input == "6") {
            return EncryptionAlgorithm::AES256;
        }
        if (input == "7") {
            return EncryptionAlgorithm::ChaCha20;
        }

        std::cout << "Invalid selection.\n";
    }
}

CompressionMode chooseCompression() {
    while (true) {
        std::cout << "\nChoose a compression mode:\n";
        std::cout << "  1. None\n";
        std::cout << "  2. RLE\n";

        std::string input = Utils::getUserInput("Select option: ");
        if (input == "1") {
            return CompressionMode::None;
        }
        if (input == "2") {
            return CompressionMode::RLE;
        }

        std::cout << "Invalid selection.\n";
    }
}

std::string buildDecryptedOutputPath(const std::string& inputFile) {
    size_t pos = inputFile.rfind(".encrypted");
    if (pos != std::string::npos) {
        return inputFile.substr(0, pos);
    }

    return inputFile + ".decrypted";
}

bool protectUSBDrive(const std::string& drivePath,
                     const std::string& key,
                     EncryptionAlgorithm algorithm,
                     CompressionMode compression) {
    std::cout << "\nProtecting: " << drivePath << std::endl;
    std::cout << "Algorithm: " << MenuCipher::getAlgorithmName(algorithm) << "\n";
    std::cout << "Compression: " << MenuCipher::getCompressionName(compression) << "\n";
    std::cout << "--------------------------------\n";

    std::string protectedDir = drivePath + "/Protected";
    if (!FileOperations::fileExists(protectedDir)) {
        FileOperations::createDirectory(protectedDir);
    }

    std::vector<std::string> files = FileOperations::collectFiles(drivePath);
    std::vector<std::string> filesToProtect;

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

    int successCount = 0;
    for (size_t i = 0; i < filesToProtect.size(); i++) {
        std::string encryptedFile = filesToProtect[i] + ".encrypted";
        std::cout << "Encrypting: " << filesToProtect[i] << " ... ";

        if (MenuCipher::encryptFile(filesToProtect[i], encryptedFile, key, algorithm, compression)) {
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

bool unprotectUSBDrive(const std::string& drivePath,
                       const std::string& key,
                       EncryptionAlgorithm fallbackAlgorithm) {
    std::cout << "\nUnprotecting: " << drivePath << std::endl;
    std::cout << "--------------------------------\n";

    std::vector<std::string> encryptedFiles =
        FileOperations::findFilesByExtension(drivePath, ".encrypted");

    if (encryptedFiles.empty()) {
        std::cout << "No encrypted files found.\n";
        return true;
    }

    std::cout << "Found " << encryptedFiles.size() << " encrypted file(s).\n\n";

    int successCount = 0;
    for (size_t i = 0; i < encryptedFiles.size(); i++) {
        std::string enc = encryptedFiles[i];
        std::string originalFile = buildDecryptedOutputPath(enc);
        EncryptionAlgorithm detectedAlgorithm = EncryptionAlgorithm::Unknown;
        CompressionMode detectedCompression = CompressionMode::Unknown;
        bool legacyFormat = false;

        std::cout << "Decrypting: " << enc << " ... ";

        if (MenuCipher::decryptFile(enc,
                                    originalFile,
                                    key,
                                    fallbackAlgorithm,
                                    CompressionMode::None,
                                    &detectedAlgorithm,
                                    &detectedCompression,
                                    &legacyFormat)) {
            if (FileOperations::deleteFile(enc)) {
                std::cout << "OK";
                if (legacyFormat) {
                    std::cout << " (legacy "
                              << MenuCipher::getAlgorithmName(detectedAlgorithm)
                              << ", no compression)";
                } else {
                    std::cout << " (" << MenuCipher::getAlgorithmName(detectedAlgorithm)
                              << ", " << MenuCipher::getCompressionName(detectedCompression) << ")";
                }
                std::cout << "\n";
                successCount++;
            } else {
                std::cout << "Decrypted but failed to delete encrypted file\n";
            }
        } else {
            std::cout << "FAILED\n";
        }
    }

    removeDirectoryIfEmpty(drivePath + "/Protected");

    std::cout << "\nDone. Decrypted " << successCount
              << " of " << encryptedFiles.size() << " file(s).\n";
    return true;
}

void showDrives() {
    std::cout << "Detected USB drives:\n";
    std::cout << "--------------------\n";
    std::vector<std::string> drives = USBDetector::getUSBDrives();
    if (drives.empty()) {
        std::cout << "No USB drives detected.\n";
        return;
    }

    for (size_t i = 0; i < drives.size(); i++) {
        std::cout << i + 1 << ". " << drives[i] << "\n";
    }
}

void showMainMenu() {
    std::cout << "\nMain Menu\n";
    std::cout << "---------\n";
    std::cout << "  1. Scan USB drives\n";
    std::cout << "  2. List files in a path\n";
    std::cout << "  3. Show drive info\n";
    std::cout << "  4. Protect a USB drive\n";
    std::cout << "  5. Unprotect a USB drive\n";
    std::cout << "  6. Encrypt a single file\n";
    std::cout << "  7. Decrypt a single file\n";
    std::cout << "  8. Show help\n";
    std::cout << "  0. Exit\n\n";
}
}

int main() {
    showMenuHelp();

    while (true) {
        showMainMenu();
        std::string choice = Utils::getUserInput("Select an option: ");

        if (choice == "0") {
            std::cout << "Exiting.\n";
            return 0;
        }

        if (choice == "8") {
            showMenuHelp();
            continue;
        }

        if (choice == "1") {
            showDrives();
            continue;
        }

        if (choice == "2") {
            std::string path = Utils::getUserInput("Enter path: ");
            if (path.empty()) {
                std::cout << "Path cannot be empty.\n";
                continue;
            }
            FileOperations::listFiles(path, false);
            continue;
        }

        if (choice == "3") {
            std::string path = Utils::getUserInput("Enter path: ");
            if (path.empty()) {
                std::cout << "Path cannot be empty.\n";
                continue;
            }
            std::cout << USBDetector::getDriveInfo(path) << std::endl;
            continue;
        }

        if (choice == "4") {
            std::string path = Utils::getUserInput("Enter USB path: ");
            std::string key = Utils::getPassword("Enter encryption key: ");
            if (path.empty() || key.empty()) {
                std::cout << "Path and key are required.\n";
                continue;
            }

            EncryptionAlgorithm algorithm = chooseAlgorithm("Choose an encryption algorithm:", false);
            CompressionMode compression = chooseCompression();
            protectUSBDrive(path, key, algorithm, compression);
            continue;
        }

        if (choice == "5") {
            std::string path = Utils::getUserInput("Enter USB path: ");
            std::string key = Utils::getPassword("Enter decryption key: ");
            if (path.empty() || key.empty()) {
                std::cout << "Path and key are required.\n";
                continue;
            }

            EncryptionAlgorithm fallbackAlgorithm =
                chooseAlgorithm("Choose the fallback algorithm for older files:", true);
            if (fallbackAlgorithm == EncryptionAlgorithm::Unknown) {
                fallbackAlgorithm = EncryptionAlgorithm::XOR;
            }

            unprotectUSBDrive(path, key, fallbackAlgorithm);
            continue;
        }

        if (choice == "6") {
            std::string inputFile = Utils::getUserInput("Enter file path: ");
            std::string key = Utils::getPassword("Enter encryption key: ");
            if (inputFile.empty() || key.empty()) {
                std::cout << "File path and key are required.\n";
                continue;
            }

            EncryptionAlgorithm algorithm = chooseAlgorithm("Choose an encryption algorithm:", false);
            CompressionMode compression = chooseCompression();
            std::string outputFile = inputFile + ".encrypted";
            if (MenuCipher::encryptFile(inputFile, outputFile, key, algorithm, compression)) {
                std::cout << "[OK] Encrypted to: " << outputFile
                          << " (" << MenuCipher::getAlgorithmName(algorithm)
                          << ", " << MenuCipher::getCompressionName(compression) << ")\n";
            } else {
                std::cout << "[FAIL] Encryption failed.\n";
            }
            continue;
        }

        if (choice == "7") {
            std::string inputFile = Utils::getUserInput("Enter file path: ");
            std::string key = Utils::getPassword("Enter decryption key: ");
            if (inputFile.empty() || key.empty()) {
                std::cout << "File path and key are required.\n";
                continue;
            }

            EncryptionAlgorithm fallbackAlgorithm =
                chooseAlgorithm("Choose the fallback algorithm for older files:", true);
            if (fallbackAlgorithm == EncryptionAlgorithm::Unknown) {
                fallbackAlgorithm = EncryptionAlgorithm::XOR;
            }

            std::string outputFile = buildDecryptedOutputPath(inputFile);
            EncryptionAlgorithm detectedAlgorithm = EncryptionAlgorithm::Unknown;
            CompressionMode detectedCompression = CompressionMode::Unknown;
            bool legacyFormat = false;

            if (MenuCipher::decryptFile(inputFile,
                                        outputFile,
                                        key,
                                        fallbackAlgorithm,
                                        CompressionMode::None,
                                        &detectedAlgorithm,
                                        &detectedCompression,
                                        &legacyFormat)) {
                std::cout << "[OK] Decrypted to: " << outputFile
                          << " (" << MenuCipher::getAlgorithmName(detectedAlgorithm);
                if (legacyFormat) {
                    std::cout << ", legacy format, no compression";
                } else {
                    std::cout << ", " << MenuCipher::getCompressionName(detectedCompression);
                }
                std::cout << ")\n";
            } else {
                std::cout << "[FAIL] Decryption failed.\n";
            }
            continue;
        }

        std::cout << "Invalid option.\n";
    }
}
