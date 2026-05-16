#include <iostream>

int global_initialized = 42;
int global_uninitialized;
const char* greeting = "Hello, World!";

int main() {
    static int local_static = 7;

    std::cout << greeting << '\n';
    std::cout << "Initialized: " << global_initialized << '\n';
    std::cout << "Uninitialized: " << global_uninitialized << '\n';
    std::cout << "Static: " << local_static << '\n';

    return 0;
}
