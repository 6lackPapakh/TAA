#include <iostream>
#include <vector>

int step_add_one(int x) {
    return x + 1;
}

int step_double(int x) {
    return x * 2;
}

int step_subtract_three(int x) {
    return x - 3;
}

int run_scattered_flow(int value) {
    std::vector<int (*)(int)> scattered_parts = {
        step_add_one,
        step_double,
        step_subtract_three,
    };

    for (auto part : scattered_parts) {
        value = part(value);
    }

    return value;
}

int main() {
    int input = 5;
    std::cout << "Input: " << input << '\n';
    std::cout << "Scattered flow result: " << run_scattered_flow(input) << '\n';
    return 0;
}
