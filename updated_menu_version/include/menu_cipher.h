#pragma once

#ifndef MENU_CIPHER_H
#define MENU_CIPHER_H

#include <string>
#include <vector>

enum class EncryptionAlgorithm {
    XOR = 1,
    Caesar = 2,
    Unknown = 255
};

class MenuCipher {
public:
    static bool encryptFile(const std::string& inputPath,
                            const std::string& outputPath,
                            const std::string& key,
                            EncryptionAlgorithm algorithm);

    static bool decryptFile(const std::string& inputPath,
                            const std::string& outputPath,
                            const std::string& key,
                            EncryptionAlgorithm fallbackAlgorithm,
                            EncryptionAlgorithm* detectedAlgorithm = 0,
                            bool* usedLegacyFormat = 0);

    static std::string getAlgorithmName(EncryptionAlgorithm algorithm);

private:
    static std::vector<char> applyAlgorithm(const std::vector<char>& data,
                                            const std::string& key,
                                            EncryptionAlgorithm algorithm,
                                            bool decrypt);
    static std::vector<char> applyXOR(const std::vector<char>& data, const std::string& key);
    static std::vector<char> applyCaesar(const std::vector<char>& data, const std::string& key, bool decrypt);
    static unsigned char deriveShift(const std::string& key);
};

#endif
