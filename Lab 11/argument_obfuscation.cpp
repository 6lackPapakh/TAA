#include <cstdarg>
#include <iostream>

int add_dummy(int a, int b, int fake1, int fake2) {
    (void)fake1;
    (void)fake2;
    return a + b;
}

int add_packed(int packed) {
    int a = (packed >> 16) & 0xFFFF;
    int b = packed & 0xFFFF;
    return a + b;
}

int add_variadic(int count, ...) {
    va_list args;
    va_start(args, count);

    int total = 0;
    for (int i = 0; i < count; ++i) {
        total += va_arg(args, int);
    }

    va_end(args);
    return total;
}

int main() {
    int packed = (5 << 16) | 3;

    std::cout << "Dummy parameters: " << add_dummy(5, 3, 999, 12345) << '\n';
    std::cout << "Packed parameter: " << add_packed(packed) << '\n';
    std::cout << "Variadic function: " << add_variadic(2, 5, 3) << '\n';

    return 0;
}
