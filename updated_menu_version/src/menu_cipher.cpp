#include "menu_cipher.h"

#include <fstream>
#include <iostream>

namespace {
const char HEADER_MAGIC[] = {'P', 'U', 'S', 'B'};
const unsigned char HEADER_VERSION = 1;
const size_t HEADER_SIZE = 6;

std::vector<char> readFile(const std::string& inputPath) {
    std::ifstream inputFile(inputPath.c_str(), std::ios::binary);
    if (!inputFile) {
        return std::vector<char>();
    }

    inputFile.seekg(0, std::ios::end);
    size_t fileSize = static_cast<size_t>(inputFile.tellg());
    inputFile.seekg(0, std::ios::beg);

    std::vector<char> buffer(fileSize);
    if (fileSize > 0) {
        inputFile.read(buffer.data(), static_cast<std::streamsize>(fileSize));
    }
    return buffer;
}

bool writeFile(const std::string& outputPath, const std::vector<char>& data) {
    std::ofstream outputFile(outputPath.c_str(), std::ios::binary);
    if (!outputFile) {
        return false;
    }

    if (!data.empty()) {
        outputFile.write(data.data(), static_cast<std::streamsize>(data.size()));
    }
    return outputFile.good();
}

}

bool MenuCipher::encryptFile(const std::string& inputPath,
                             const std::string& outputPath,
                             const std::string& key,
                             EncryptionAlgorithm algorithm) {
    try {
        std::vector<char> buffer = readFile(inputPath);
        std::ifstream check(inputPath.c_str(), std::ios::binary);
        if (!check) {
            std::cerr << "Error: Cannot open file: " << inputPath << std::endl;
            return false;
        }

        std::vector<char> encrypted = applyAlgorithm(buffer, key, algorithm, false);
        if (algorithm == EncryptionAlgorithm::Unknown) {
            std::cerr << "Error: Unsupported encryption algorithm." << std::endl;
            return false;
        }

        std::vector<char> output;
        output.reserve(HEADER_SIZE + encrypted.size());
        output.insert(output.end(), HEADER_MAGIC, HEADER_MAGIC + 4);
        output.push_back(static_cast<char>(HEADER_VERSION));
        output.push_back(static_cast<char>(algorithm));
        output.insert(output.end(), encrypted.begin(), encrypted.end());

        if (!writeFile(outputPath, output)) {
            std::cerr << "Error: Cannot create output file: " << outputPath << std::endl;
            return false;
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error during encryption: " << e.what() << std::endl;
        return false;
    }
}

bool MenuCipher::decryptFile(const std::string& inputPath,
                             const std::string& outputPath,
                             const std::string& key,
                             EncryptionAlgorithm fallbackAlgorithm,
                             EncryptionAlgorithm* detectedAlgorithm,
                             bool* usedLegacyFormat) {
    try {
        std::vector<char> buffer = readFile(inputPath);
        std::ifstream check(inputPath.c_str(), std::ios::binary);
        if (!check) {
            std::cerr << "Error: Cannot open file: " << inputPath << std::endl;
            return false;
        }

        EncryptionAlgorithm algorithm = fallbackAlgorithm;
        bool legacyFormat = true;
        std::vector<char> payload = buffer;

        if (buffer.size() >= HEADER_SIZE &&
            buffer[0] == HEADER_MAGIC[0] &&
            buffer[1] == HEADER_MAGIC[1] &&
            buffer[2] == HEADER_MAGIC[2] &&
            buffer[3] == HEADER_MAGIC[3] &&
            static_cast<unsigned char>(buffer[4]) == HEADER_VERSION) {
            algorithm = static_cast<EncryptionAlgorithm>(static_cast<unsigned char>(buffer[5]));
            payload.assign(buffer.begin() + HEADER_SIZE, buffer.end());
            legacyFormat = false;
        }

        std::vector<char> decrypted = applyAlgorithm(payload, key, algorithm, true);
        if (algorithm == EncryptionAlgorithm::Unknown) {
            std::cerr << "Error: Unsupported decryption algorithm." << std::endl;
            return false;
        }

        if (!writeFile(outputPath, decrypted)) {
            std::cerr << "Error: Cannot create output file: " << outputPath << std::endl;
            return false;
        }

        if (detectedAlgorithm) {
            *detectedAlgorithm = algorithm;
        }

        if (usedLegacyFormat) {
            *usedLegacyFormat = legacyFormat;
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error during decryption: " << e.what() << std::endl;
        return false;
    }
}

std::string MenuCipher::getAlgorithmName(EncryptionAlgorithm algorithm) {
    if (algorithm == EncryptionAlgorithm::XOR) {
        return "XOR";
    }

    if (algorithm == EncryptionAlgorithm::Caesar) {
        return "Caesar";
    }

    return "Unknown";
}

std::vector<char> MenuCipher::applyAlgorithm(const std::vector<char>& data,
                                             const std::string& key,
                                             EncryptionAlgorithm algorithm,
                                             bool decrypt) {
    if (algorithm == EncryptionAlgorithm::XOR) {
        return applyXOR(data, key);
    }

    if (algorithm == EncryptionAlgorithm::Caesar) {
        return applyCaesar(data, key, decrypt);
    }

    return std::vector<char>();
}

std::vector<char> MenuCipher::applyXOR(const std::vector<char>& data, const std::string& key) {
    std::vector<char> result = data;
    size_t keyLen = key.length();
    if (keyLen == 0) {
        return result;
    }

    for (size_t i = 0; i < result.size(); i++) {
        result[i] ^= key[i % keyLen];
    }

    return result;
}

std::vector<char> MenuCipher::applyCaesar(const std::vector<char>& data,
                                          const std::string& key,
                                          bool decrypt) {
    std::vector<char> result = data;
    unsigned char shift = deriveShift(key);

    for (size_t i = 0; i < result.size(); i++) {
        unsigned char value = static_cast<unsigned char>(result[i]);
        if (decrypt) {
            value = static_cast<unsigned char>(value - shift);
        } else {
            value = static_cast<unsigned char>(value + shift);
        }
        result[i] = static_cast<char>(value);
    }

    return result;
}

unsigned char MenuCipher::deriveShift(const std::string& key) {
    unsigned int sum = 0;
    for (size_t i = 0; i < key.size(); i++) {
        sum += static_cast<unsigned char>(key[i]);
    }
    return static_cast<unsigned char>(sum % 256);
}
