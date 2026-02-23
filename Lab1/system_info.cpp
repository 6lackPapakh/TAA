#include <iostream>
#include <cstdlib>

int main() {
    std::cout << "CPU Info:\n";
    system("lscpu | grep 'Model name'");

    std::cout << "\nMemory:\n";
    system("free -h");

    std::cout << "\nDisk:\n";
    system("lsblk");

    std::cout << "\nMAC:\n";
    system("ip link | grep link/ether");

    return 0;
}
