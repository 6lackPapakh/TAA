#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#ifndef _WIN32
#include <sys/stat.h>
#endif

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

std::string build_run_command(const std::string& output_path) {
#ifdef _WIN32
    return output_path;
#else
    if (output_path.find('/') == std::string::npos) {
        return "./" + output_path;
    }
    return output_path;
#endif
}
}  // namespace

int main(int argc, char* argv[]) {
    if (argc < 3 || argc > 4) {
        std::cerr << "Usage: ./simple_stub <packed_input> <unpacked_output> [--run]\n";
        return 1;
    }

    const std::string packed_path = argv[1];
    const std::string unpacked_path = argv[2];
    const bool should_run = argc == 4 && std::string(argv[3]) == "--run";

    std::vector<unsigned char> data = read_file(packed_path);
    if (data.empty()) {
        std::cerr << "Failed to read packed input or file is empty.\n";
        return 1;
    }

    xor_buffer(data);

    if (!write_file(unpacked_path, data)) {
        std::cerr << "Failed to write unpacked file.\n";
        return 1;
    }

#ifndef _WIN32
    chmod(unpacked_path.c_str(), 0755);
#endif

    std::cout << "Unpacked successfully: " << unpacked_path << '\n';

    if (should_run) {
        const std::string command = build_run_command(unpacked_path);
        std::cout << "Running: " << command << '\n';
        return system(command.c_str());
    }

    return 0;
}
