#pragma once
#ifndef CIPHER_H
#define CIPHER_H

#include <vector>
#include <string>

class SimpleXORCipher {
private:
    std::string key;

public:
    SimpleXORCipher(const std::string& encryptionKey);

    std::vector<char> encrypt(const std::vector<char>& data);
    std::vector<char> decrypt(const std::vector<char>& data);

    bool encryptFile(const std::string& inputPath, const std::string& outputPath);
    bool decryptFile(const std::string& inputPath, const std::string& outputPath);
};

#endif
