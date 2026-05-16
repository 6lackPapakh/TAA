#ifndef XOR_OBFUSCATION_H
#define XOR_OBFUSCATION_H

#include <vector>
#include <string>

std::vector<int> encode(const std::string& text, int key);
std::string decode(const std::vector<int>& data, int key);

#endif
