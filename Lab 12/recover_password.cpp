#include <iostream>

int main() {
    const int key[] = {25, 20, 23, 100, 103};

    std::cout << "Recovered password: ";
    for (int value : key) {
        std::cout << static_cast<char>(value ^ 0x55);
    }
    std::cout << '\n';

    return 0;
}
