#include "xor_obfuscation.h"

std::vector<int> encode(const std::string& text, int key) {
    std::vector<int> result;
    for (char c : text) {
        result.push_back(c ^ key);
    }
    return result;
}

std::string decode(const std::vector<int>& data, int key) {
    std::string result;
    for (int val : data) {
        result.push_back(char(val ^ key));
    }
    return result;
}
