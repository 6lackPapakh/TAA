#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {
constexpr unsigned char kKey = 0xAA;

std::vector<unsigned char> read_file(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    return std::vector<unsigned char>(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>());
}

bool write_file(const std::string& path, const std::vector<unsigned char>& data) {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    file.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
    return file.good();
}

void xor_buffer(std::vector<unsigned char>& data) {
    for (auto& byte : data) {
        byte ^= kKey;
    }
}
}  // namespace

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: ./simple_packer <input_binary> <packed_output>\n";
        return 1;
    }

    std::vector<unsigned char> data = read_file(argv[1]);
    if (data.empty()) {
        std::cerr << "Failed to read input file or file is empty.\n";
        return 1;
    }

    xor_buffer(data);

    if (!write_file(argv[2], data)) {
        std::cerr << "Failed to write packed file.\n";
        return 1;
    }

    std::cout << "Packed successfully: " << argv[2] << '\n';
    return 0;
}
