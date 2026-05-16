#include <iostream>
#include <sys/ptrace.h>

int main() {
    if (ptrace(PTRACE_TRACEME, 0, nullptr, nullptr) == -1) {
        std::cout << "Debugger detected.\n";
        return 1;
    }

    std::cout << "No debugger detected.\n";
    return 0;
}
