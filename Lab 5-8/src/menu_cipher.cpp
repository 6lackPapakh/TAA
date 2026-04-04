#include "menu_cipher.h"

#include <cstddef>
#include <fstream>
#include <iostream>
#include <vector>

#include <openssl/evp.h>
#include <openssl/rand.h>

namespace {
const char HEADER_MAGIC[] = {'P', 'U', 'S', 'B'};
const unsigned char HEADER_VERSION_V1 = 1;
const unsigned char HEADER_VERSION_V2 = 2;
const unsigned char HEADER_VERSION_V3 = 3;
const size_t HEADER_SIZE_V1 = 6;
const size_t HEADER_SIZE_V2 = 7;
const size_t HEADER_SIZE_V3 = 9;
const size_t SECURE_SALT_SIZE = 16;
const int PBKDF2_ITERATIONS = 100000;

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

bool isSecureAlgorithm(EncryptionAlgorithm algorithm) {
    return algorithm == EncryptionAlgorithm::AES256 ||
           algorithm == EncryptionAlgorithm::ChaCha20;
}

const EVP_CIPHER* getCipher(EncryptionAlgorithm algorithm) {
    if (algorithm == EncryptionAlgorithm::AES256) {
        return EVP_aes_256_ctr();
    }

    if (algorithm == EncryptionAlgorithm::ChaCha20) {
        return EVP_chacha20();
    }

    return 0;
}

bool deriveKey(const std::string& password,
               const std::vector<unsigned char>& salt,
               std::vector<unsigned char>* key) {
    key->assign(32, 0);
    return PKCS5_PBKDF2_HMAC(password.c_str(),
                             static_cast<int>(password.size()),
                             salt.empty() ? 0 : salt.data(),
                             static_cast<int>(salt.size()),
                             PBKDF2_ITERATIONS,
                             EVP_sha256(),
                             static_cast<int>(key->size()),
                             key->data()) == 1;
}

bool runSecureCipher(const std::vector<char>& input,
                     const std::string& password,
                     EncryptionAlgorithm algorithm,
                     bool decrypt,
                     const std::vector<unsigned char>& salt,
                     const std::vector<unsigned char>& iv,
                     std::vector<char>* output) {
    const EVP_CIPHER* cipher = getCipher(algorithm);
    if (!cipher) {
        return false;
    }

    if (iv.size() != static_cast<size_t>(EVP_CIPHER_iv_length(cipher))) {
        return false;
    }

    std::vector<unsigned char> key;
    if (!deriveKey(password, salt, &key)) {
        return false;
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        return false;
    }

    int ok = decrypt
        ? EVP_DecryptInit_ex(ctx, cipher, 0, key.data(), iv.data())
        : EVP_EncryptInit_ex(ctx, cipher, 0, key.data(), iv.data());
    if (ok != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    std::vector<char> buffer(input.size() + static_cast<size_t>(EVP_CIPHER_block_size(cipher)) + 16);
    int outLen1 = 0;
    int outLen2 = 0;

    if (!input.empty()) {
        ok = decrypt
            ? EVP_DecryptUpdate(ctx,
                                reinterpret_cast<unsigned char*>(buffer.data()),
                                &outLen1,
                                reinterpret_cast<const unsigned char*>(input.data()),
                                static_cast<int>(input.size()))
            : EVP_EncryptUpdate(ctx,
                                reinterpret_cast<unsigned char*>(buffer.data()),
                                &outLen1,
                                reinterpret_cast<const unsigned char*>(input.data()),
                                static_cast<int>(input.size()));
        if (ok != 1) {
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
    }

    ok = decrypt
        ? EVP_DecryptFinal_ex(ctx,
                              reinterpret_cast<unsigned char*>(buffer.data()) + outLen1,
                              &outLen2)
        : EVP_EncryptFinal_ex(ctx,
                              reinterpret_cast<unsigned char*>(buffer.data()) + outLen1,
                              &outLen2);
    EVP_CIPHER_CTX_free(ctx);
    if (ok != 1) {
        return false;
    }

    buffer.resize(static_cast<size_t>(outLen1 + outLen2));
    *output = buffer;
    return true;
}

bool encryptSecureData(const std::vector<char>& input,
                       const std::string& password,
                       EncryptionAlgorithm algorithm,
                       std::vector<unsigned char>* salt,
                       std::vector<unsigned char>* iv,
                       std::vector<char>* output) {
    const EVP_CIPHER* cipher = getCipher(algorithm);
    if (!cipher) {
        return false;
    }

    salt->assign(SECURE_SALT_SIZE, 0);
    iv->assign(static_cast<size_t>(EVP_CIPHER_iv_length(cipher)), 0);
    if (RAND_bytes(salt->data(), static_cast<int>(salt->size())) != 1 ||
        RAND_bytes(iv->data(), static_cast<int>(iv->size())) != 1) {
        return false;
    }

    return runSecureCipher(input, password, algorithm, false, *salt, *iv, output);
}

bool decryptSecureData(const std::vector<char>& input,
                       const std::string& password,
                       EncryptionAlgorithm algorithm,
                       const std::vector<unsigned char>& salt,
                       const std::vector<unsigned char>& iv,
                       std::vector<char>* output) {
    return runSecureCipher(input, password, algorithm, true, salt, iv, output);
}
}

bool MenuCipher::encryptFile(const std::string& inputPath,
                             const std::string& outputPath,
                             const std::string& key,
                             EncryptionAlgorithm algorithm,
                             CompressionMode compression) {
    try {
        std::vector<char> buffer = readFile(inputPath);
        std::ifstream check(inputPath.c_str(), std::ios::binary);
        if (!check) {
            std::cerr << "Error: Cannot open file: " << inputPath << std::endl;
            return false;
        }

        if (algorithm == EncryptionAlgorithm::Unknown) {
            std::cerr << "Error: Unsupported encryption algorithm." << std::endl;
            return false;
        }

        if (compression == CompressionMode::Unknown) {
            std::cerr << "Error: Unsupported compression mode." << std::endl;
            return false;
        }

        std::vector<char> compressed = compressData(buffer, compression);
        std::vector<char> encrypted;
        std::vector<unsigned char> salt;
        std::vector<unsigned char> iv;

        if (isSecureAlgorithm(algorithm)) {
            if (!encryptSecureData(compressed, key, algorithm, &salt, &iv, &encrypted)) {
                std::cerr << "Error: Secure encryption failed." << std::endl;
                return false;
            }
        } else {
            encrypted = applyAlgorithm(compressed, key, algorithm, false);
        }

        std::vector<char> output;
        output.reserve(HEADER_SIZE_V3 + salt.size() + iv.size() + encrypted.size());
        output.insert(output.end(), HEADER_MAGIC, HEADER_MAGIC + 4);
        output.push_back(static_cast<char>(HEADER_VERSION_V3));
        output.push_back(static_cast<char>(algorithm));
        output.push_back(static_cast<char>(compression));
        output.push_back(static_cast<char>(salt.size()));
        output.push_back(static_cast<char>(iv.size()));
        output.insert(output.end(), salt.begin(), salt.end());
        output.insert(output.end(), iv.begin(), iv.end());
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
                             CompressionMode fallbackCompression,
                             EncryptionAlgorithm* detectedAlgorithm,
                             CompressionMode* detectedCompression,
                             bool* usedLegacyFormat) {
    try {
        std::vector<char> buffer = readFile(inputPath);
        std::ifstream check(inputPath.c_str(), std::ios::binary);
        if (!check) {
            std::cerr << "Error: Cannot open file: " << inputPath << std::endl;
            return false;
        }

        EncryptionAlgorithm algorithm = fallbackAlgorithm;
        CompressionMode compression = fallbackCompression;
        bool legacyFormat = true;
        std::vector<char> payload = buffer;
        std::vector<unsigned char> salt;
        std::vector<unsigned char> iv;

        if (buffer.size() >= HEADER_SIZE_V3 &&
            buffer[0] == HEADER_MAGIC[0] &&
            buffer[1] == HEADER_MAGIC[1] &&
            buffer[2] == HEADER_MAGIC[2] &&
            buffer[3] == HEADER_MAGIC[3] &&
            static_cast<unsigned char>(buffer[4]) == HEADER_VERSION_V3) {
            algorithm = static_cast<EncryptionAlgorithm>(static_cast<unsigned char>(buffer[5]));
            compression = static_cast<CompressionMode>(static_cast<unsigned char>(buffer[6]));

            size_t saltLen = static_cast<unsigned char>(buffer[7]);
            size_t ivLen = static_cast<unsigned char>(buffer[8]);
            size_t payloadOffset = HEADER_SIZE_V3 + saltLen + ivLen;
            if (buffer.size() < payloadOffset) {
                std::cerr << "Error: Invalid encrypted file header." << std::endl;
                return false;
            }

            salt.assign(buffer.begin() + HEADER_SIZE_V3,
                        buffer.begin() + HEADER_SIZE_V3 + saltLen);
            iv.assign(buffer.begin() + HEADER_SIZE_V3 + saltLen,
                      buffer.begin() + payloadOffset);
            payload.assign(buffer.begin() + payloadOffset, buffer.end());
            legacyFormat = false;
        } else if (buffer.size() >= HEADER_SIZE_V2 &&
                   buffer[0] == HEADER_MAGIC[0] &&
                   buffer[1] == HEADER_MAGIC[1] &&
                   buffer[2] == HEADER_MAGIC[2] &&
                   buffer[3] == HEADER_MAGIC[3] &&
                   static_cast<unsigned char>(buffer[4]) == HEADER_VERSION_V2) {
            algorithm = static_cast<EncryptionAlgorithm>(static_cast<unsigned char>(buffer[5]));
            compression = static_cast<CompressionMode>(static_cast<unsigned char>(buffer[6]));
            payload.assign(buffer.begin() + HEADER_SIZE_V2, buffer.end());
            legacyFormat = false;
        } else if (buffer.size() >= HEADER_SIZE_V1 &&
                   buffer[0] == HEADER_MAGIC[0] &&
                   buffer[1] == HEADER_MAGIC[1] &&
                   buffer[2] == HEADER_MAGIC[2] &&
                   buffer[3] == HEADER_MAGIC[3] &&
                   static_cast<unsigned char>(buffer[4]) == HEADER_VERSION_V1) {
            algorithm = static_cast<EncryptionAlgorithm>(static_cast<unsigned char>(buffer[5]));
            compression = CompressionMode::None;
            payload.assign(buffer.begin() + HEADER_SIZE_V1, buffer.end());
            legacyFormat = false;
        }

        if (algorithm == EncryptionAlgorithm::Unknown) {
            std::cerr << "Error: Unsupported decryption algorithm." << std::endl;
            return false;
        }

        std::vector<char> decrypted;
        if (isSecureAlgorithm(algorithm)) {
            if (salt.empty() || iv.empty() ||
                !decryptSecureData(payload, key, algorithm, salt, iv, &decrypted)) {
                std::cerr << "Error: Secure decryption failed." << std::endl;
                return false;
            }
        } else {
            decrypted = applyAlgorithm(payload, key, algorithm, true);
        }

        bool decompressionOk = true;
        std::vector<char> restored = decompressData(decrypted, compression, &decompressionOk);
        if (compression == CompressionMode::Unknown || !decompressionOk) {
            std::cerr << "Error: Unsupported or invalid compressed data." << std::endl;
            return false;
        }

        if (!writeFile(outputPath, restored)) {
            std::cerr << "Error: Cannot create output file: " << outputPath << std::endl;
            return false;
        }

        if (detectedAlgorithm) {
            *detectedAlgorithm = algorithm;
        }

        if (detectedCompression) {
            *detectedCompression = compression;
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

    if (algorithm == EncryptionAlgorithm::Vigenere) {
        return "Vigenere";
    }

    if (algorithm == EncryptionAlgorithm::XorShiftStream) {
        return "XorShiftStream";
    }

    if (algorithm == EncryptionAlgorithm::RotateXOR) {
        return "RotateXOR";
    }

    if (algorithm == EncryptionAlgorithm::AES256) {
        return "AES-256";
    }

    if (algorithm == EncryptionAlgorithm::ChaCha20) {
        return "ChaCha20";
    }

    return "Unknown";
}

std::string MenuCipher::getCompressionName(CompressionMode compression) {
    if (compression == CompressionMode::None) {
        return "None";
    }

    if (compression == CompressionMode::RLE) {
        return "RLE";
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

    if (algorithm == EncryptionAlgorithm::Vigenere) {
        return applyVigenere(data, key, decrypt);
    }

    if (algorithm == EncryptionAlgorithm::XorShiftStream) {
        return applyXorShiftStream(data, key);
    }

    if (algorithm == EncryptionAlgorithm::RotateXOR) {
        return applyRotateXOR(data, key, decrypt);
    }

    return std::vector<char>();
}

std::vector<char> MenuCipher::compressData(const std::vector<char>& data, CompressionMode compression) {
    if (compression == CompressionMode::None) {
        return data;
    }

    if (compression == CompressionMode::RLE) {
        return compressRLE(data);
    }

    return std::vector<char>();
}

std::vector<char> MenuCipher::decompressData(const std::vector<char>& data,
                                             CompressionMode compression,
                                             bool* ok) {
    if (ok) {
        *ok = true;
    }

    if (compression == CompressionMode::None) {
        return data;
    }

    if (compression == CompressionMode::RLE) {
        return decompressRLE(data, ok);
    }

    if (ok) {
        *ok = false;
    }
    return std::vector<char>();
}

std::vector<char> MenuCipher::compressRLE(const std::vector<char>& data) {
    std::vector<char> output;
    if (data.empty()) {
        return output;
    }

    size_t i = 0;
    while (i < data.size()) {
        unsigned char count = 1;
        while (i + count < data.size() && count < 255 && data[i + count] == data[i]) {
            count++;
        }
        output.push_back(static_cast<char>(count));
        output.push_back(data[i]);
        i += count;
    }

    return output;
}

std::vector<char> MenuCipher::decompressRLE(const std::vector<char>& data, bool* ok) {
    std::vector<char> output;
    if (data.size() % 2 != 0) {
        if (ok) {
            *ok = false;
        }
        return output;
    }

    for (size_t i = 0; i < data.size(); i += 2) {
        unsigned char count = static_cast<unsigned char>(data[i]);
        char value = data[i + 1];
        if (count == 0) {
            if (ok) {
                *ok = false;
            }
            return std::vector<char>();
        }

        for (unsigned int j = 0; j < count; j++) {
            output.push_back(value);
        }
    }

    if (ok) {
        *ok = true;
    }
    return output;
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

std::vector<char> MenuCipher::applyVigenere(const std::vector<char>& data,
                                            const std::string& key,
                                            bool decrypt) {
    std::vector<char> result = data;
    if (key.empty()) {
        return result;
    }

    for (size_t i = 0; i < result.size(); i++) {
        unsigned char value = static_cast<unsigned char>(result[i]);
        unsigned char keyByte = static_cast<unsigned char>(key[i % key.size()]);
        if (decrypt) {
            value = static_cast<unsigned char>(value - keyByte);
        } else {
            value = static_cast<unsigned char>(value + keyByte);
        }
        result[i] = static_cast<char>(value);
    }

    return result;
}

std::vector<char> MenuCipher::applyXorShiftStream(const std::vector<char>& data,
                                                  const std::string& key) {
    std::vector<char> result = data;
    unsigned int state = deriveSeed(key);

    for (size_t i = 0; i < result.size(); i++) {
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;
        result[i] ^= static_cast<char>(state & 0xFFu);
    }

    return result;
}

std::vector<char> MenuCipher::applyRotateXOR(const std::vector<char>& data,
                                             const std::string& key,
                                             bool decrypt) {
    std::vector<char> result = data;
    unsigned char shift = static_cast<unsigned char>(deriveShift(key) % 8u);

    for (size_t i = 0; i < result.size(); i++) {
        unsigned char value = static_cast<unsigned char>(result[i]);
        unsigned char keyByte = key.empty()
            ? 0
            : static_cast<unsigned char>(key[i % key.size()]);

        if (decrypt) {
            value ^= keyByte;
            if (shift != 0) {
                value = static_cast<unsigned char>((value >> shift) | (value << (8u - shift)));
            }
        } else {
            if (shift != 0) {
                value = static_cast<unsigned char>((value << shift) | (value >> (8u - shift)));
            }
            value ^= keyByte;
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

unsigned int MenuCipher::deriveSeed(const std::string& key) {
    unsigned int seed = 2166136261u;
    for (size_t i = 0; i < key.size(); i++) {
        seed ^= static_cast<unsigned char>(key[i]);
        seed *= 16777619u;
    }

    if (seed == 0u) {
        seed = 2463534242u;
    }

    return seed;
}
