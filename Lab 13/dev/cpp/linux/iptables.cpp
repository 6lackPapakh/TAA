// firewall_linux_real.cpp

// sudo iptables-save > /etc/iptables/rules.v4

#include <iostream>
#include <cstdlib>
#include <string>

using namespace std;

// =============================
// Block IN
// =============================
void block_port_in(int port) {
    string cmd = "sudo iptables -A INPUT -p tcp --dport " + to_string(port) + " -j DROP";
    system(cmd.c_str());
}

// =============================
// Block OUT
// =============================
void block_port_out(int port) {
    string cmd = "sudo iptables -A OUTPUT -p tcp --dport " + to_string(port) + " -j DROP";
    system(cmd.c_str());
}

// =============================
// UNBLOCK
// =============================
void unblock_port(int port) {
    string cmd1 = "sudo iptables -D INPUT -p tcp --dport " + to_string(port) + " -j DROP";
    string cmd2 = "sudo iptables -D OUTPUT -p tcp --dport " + to_string(port) + " -j DROP";

    system(cmd1.c_str());
    system(cmd2.c_str());
}

// =============================
int main() {
    cout << "Blocking port 80 IN...\n";
    block_port_in(80);

    cout << "Blocking port 22 OUT...\n";
    block_port_out(22);

    cout << "Unblocking port 80...\n";
    unblock_port(80);

    return 0;
}
