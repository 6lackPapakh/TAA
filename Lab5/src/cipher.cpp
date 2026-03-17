#include "cipher.h"
#include <fstream>
#include <iostream>
#include <vector>

SimpleXORCipher::SimpleXORCipher(const std::string& encryptionKey) : key(encryptionKey) {}

std::vector<char> SimpleXORCipher::encrypt(const std::vector<char>& data) {
    std::vector<char> result = data;
    size_t keyLen = key.length();
    if (keyLen == 0) return result;

    for (size_t i = 0; i < result.size(); i++) {
        result[i] ^= key[i % keyLen];
    }
    return result;
}

// XOR is symmetric — decrypt is identical to encrypt
std::vector<char> SimpleXORCipher::decrypt(const std::vector<char>& data) {
    return encrypt(data);
}

bool SimpleXORCipher::encryptFile(const std::string& inputPath, const std::string& outputPath) {
    try {
        std::ifstream inputFile(inputPath.c_str(), std::ios::binary);
        if (!inputFile) {
            std::cerr << "Error: Cannot open file: " << inputPath << std::endl;
            return false;
        }

        // Read entire file into buffer
        inputFile.seekg(0, std::ios::end);
        size_t fileSize = static_cast<size_t>(inputFile.tellg());
        inputFile.seekg(0, std::ios::beg);

        std::vector<char> buffer(fileSize);
        inputFile.read(buffer.data(), static_cast<std::streamsize>(fileSize));
        inputFile.close();

        // Encrypt
        std::vector<char> encrypted = encrypt(buffer);

        // Write output
        std::ofstream outputFile(outputPath.c_str(), std::ios::binary);
        if (!outputFile) {
            std::cerr << "Error: Cannot create output file: " << outputPath << std::endl;
            return false;
        }
        outputFile.write(encrypted.data(), static_cast<std::streamsize>(encrypted.size()));
        outputFile.close();

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error during encryption: " << e.what() << std::endl;
        return false;
    }
}

bool SimpleXORCipher::decryptFile(const std::string& inputPath, const std::string& outputPath) {
    try {
        std::ifstream inputFile(inputPath.c_str(), std::ios::binary);
        if (!inputFile) {
            std::cerr << "Error: Cannot open file: " << inputPath << std::endl;
            return false;
        }

        inputFile.seekg(0, std::ios::end);
        size_t fileSize = static_cast<size_t>(inputFile.tellg());
        inputFile.seekg(0, std::ios::beg);

        std::vector<char> buffer(fileSize);
        inputFile.read(buffer.data(), static_cast<std::streamsize>(fileSize));
        inputFile.close();

        // Decrypt (same as encrypt for XOR)
        std::vector<char> decrypted = decrypt(buffer);

        std::ofstream outputFile(outputPath.c_str(), std::ios::binary);
        if (!outputFile) {
            std::cerr << "Error: Cannot create output file: " << outputPath << std::endl;
            return false;
        }
        outputFile.write(decrypted.data(), static_cast<std::streamsize>(decrypted.size()));
        outputFile.close();

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error during decryption: " << e.what() << std::endl;
        return false;
    }
}
