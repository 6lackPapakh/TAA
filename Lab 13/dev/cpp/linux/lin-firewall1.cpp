// firewall_linux.cpp
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <vector>
#include <ctime>

using namespace std;

map<int, string> PORT_PROTOCOLS = {
    {80, "HTTP"},
    {443, "HTTPS"},
    {21, "FTP"},
    {22, "SSH"},
    {23, "Telnet"}
};

set<int> blocked_in;
set<int> blocked_out;
map<int, int> traffic_stats;


// =============================
// HEX → PORT
// =============================
int hex_to_port(string hex) {
    return stoi(hex, nullptr, 16);
}


// =============================
// 1. Բաց պորտեր
// =============================
void list_open_ports() {
    ifstream file("/proc/net/tcp");
    string line;

    getline(file, line); // skip header

    while (getline(file, line)) {
        stringstream ss(line);
        string local_address;

        ss >> local_address >> local_address;

        string ip_port = local_address.substr(local_address.find(':') + 1);
        int port = hex_to_port(ip_port);

        string proto = PORT_PROTOCOLS.count(port) ? PORT_PROTOCOLS[port] : "TCP";

        cout << "Port: " << port << " Protocol: " << proto << endl;
    }
}


// =============================
void block_port(int port, string dir) {
    if (dir == "in") blocked_in.insert(port);
    else blocked_out.insert(port);
}


void open_port(int port) {
    blocked_in.erase(port);
    blocked_out.erase(port);
}


// =============================
void monitor_traffic() {
    ifstream file("/proc/net/tcp");
    string line;

    getline(file, line);

    while (getline(file, line)) {
        stringstream ss(line);
        string local_address;

        ss >> local_address >> local_address;

        string ip_port = local_address.substr(local_address.find(':') + 1);
        int port = hex_to_port(ip_port);

        traffic_stats[port]++;
    }
}


// =============================
void show_heavy_ports() {
    cout << "\nHeavy ports:\n";
    for (auto &p : traffic_stats) {
        if (p.second > 5)
            cout << "Port " << p.first << " -> " << p.second << endl;
    }
}


// =============================
void save_log() {
    time_t now = time(0);
    string filename = "firewall_log_" + to_string(now) + ".txt";

    ofstream file(filename);

    for (auto &p : traffic_stats) {
        file << "Port: " << p.first << " Count: " << p.second << endl;
    }

    file.close();
}


// =============================
int main() {
    list_open_ports();

    block_port(80, "in");
    block_port(22, "out");

    monitor_traffic();
    show_heavy_ports();

    save_log();

    return 0;
}

// compile line
//
//g++ firewall_linux.cpp -o firewall
