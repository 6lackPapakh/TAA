#include <iostream>
#include <string>

bool check_password(const std::string& input) {
    const int key[] = {25, 20, 23, 100, 103};

    if (input.size() != 5) {
        return false;
    }

    for (size_t i = 0; i < input.size(); ++i) {
        if ((input[i] ^ 0x55) != key[i]) {
            return false;
        }
    }

    return true;
}

int main() {
    std::string password;
    std::cout << "Enter password: ";
    std::cin >> password;

    if (check_password(password)) {
        std::cout << "Access Granted!\n";
    } else {
        std::cout << "Access Denied!\n";
    }

    return 0;
}
