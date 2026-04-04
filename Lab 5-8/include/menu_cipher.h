#pragma once

#ifndef MENU_CIPHER_H
#define MENU_CIPHER_H

#include <string>
#include <vector>

enum class EncryptionAlgorithm {
    XOR = 1,
    Caesar = 2,
    Vigenere = 3,
    XorShiftStream = 4,
    RotateXOR = 5,
    AES256 = 6,
    ChaCha20 = 7,
    Unknown = 255
};

enum class CompressionMode {
    None = 0,
    RLE = 1,
    Unknown = 255
};

class MenuCipher {
public:
    static bool encryptFile(const std::string& inputPath,
                            const std::string& outputPath,
                            const std::string& key,
                            EncryptionAlgorithm algorithm,
                            CompressionMode compression);

    static bool decryptFile(const std::string& inputPath,
                            const std::string& outputPath,
                            const std::string& key,
                            EncryptionAlgorithm fallbackAlgorithm,
                            CompressionMode fallbackCompression = CompressionMode::None,
                            EncryptionAlgorithm* detectedAlgorithm = 0,
                            CompressionMode* detectedCompression = 0,
                            bool* usedLegacyFormat = 0);

    static std::string getAlgorithmName(EncryptionAlgorithm algorithm);
    static std::string getCompressionName(CompressionMode compression);

private:
    static std::vector<char> applyAlgorithm(const std::vector<char>& data,
                                            const std::string& key,
                                            EncryptionAlgorithm algorithm,
                                            bool decrypt);
    static std::vector<char> applyXOR(const std::vector<char>& data, const std::string& key);
    static std::vector<char> applyCaesar(const std::vector<char>& data, const std::string& key, bool decrypt);
    static std::vector<char> applyVigenere(const std::vector<char>& data, const std::string& key, bool decrypt);
    static std::vector<char> applyXorShiftStream(const std::vector<char>& data, const std::string& key);
    static std::vector<char> applyRotateXOR(const std::vector<char>& data, const std::string& key, bool decrypt);
    static std::vector<char> compressData(const std::vector<char>& data, CompressionMode compression);
    static std::vector<char> decompressData(const std::vector<char>& data, CompressionMode compression, bool* ok);
    static std::vector<char> compressRLE(const std::vector<char>& data);
    static std::vector<char> decompressRLE(const std::vector<char>& data, bool* ok);
    static unsigned char deriveShift(const std::string& key);
    static unsigned int deriveSeed(const std::string& key);
};

#endif
