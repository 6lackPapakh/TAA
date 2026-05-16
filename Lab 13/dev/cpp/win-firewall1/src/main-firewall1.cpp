// firewall_windows.cpp
// in case of gnu project,compile line is
// g++ firewall.cpp -o firewall.exe -lws2_32 -liphlpapi

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <fstream>
#include <ctime>


#include <iomanip>
#include <sstream>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

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
// 1. Բաց պորտերի ցուցադրում
// =============================
void list_open_ports() {
    PMIB_TCPTABLE_OWNER_PID tcpTable;
    DWORD size = 0;

    GetExtendedTcpTable(NULL, &size, TRUE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0);
    tcpTable = (PMIB_TCPTABLE_OWNER_PID)malloc(size);

    if (GetExtendedTcpTable(tcpTable, &size, TRUE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0) == NO_ERROR) {
        for (DWORD i = 0; i < tcpTable->dwNumEntries; i++) {
            int port = ntohs((u_short)tcpTable->table[i].dwLocalPort);

            string proto = PORT_PROTOCOLS.count(port) ? PORT_PROTOCOLS[port] : "TCP";

            cout << "Port: " << port << " Protocol: " << proto << endl;
        }
    }

    free(tcpTable);
}


// =============================
// 2. Block port
// =============================
void block_port(int port, string dir) {
    if (dir == "in") blocked_in.insert(port);
    else blocked_out.insert(port);
}


// =============================
// 3. Open port
// =============================
void open_port(int port) {
    blocked_in.erase(port);
    blocked_out.erase(port);
}


// =============================
// 4. Fake traffic monitoring
// =============================
void monitor_traffic() {
    PMIB_TCPTABLE_OWNER_PID tcpTable;
    DWORD size = 0;

    GetExtendedTcpTable(NULL, &size, TRUE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0);
    tcpTable = (PMIB_TCPTABLE_OWNER_PID)malloc(size);

    if (GetExtendedTcpTable(tcpTable, &size, TRUE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0) == NO_ERROR) {
        for (DWORD i = 0; i < tcpTable->dwNumEntries; i++) {
            int port = ntohs((u_short)tcpTable->table[i].dwLocalPort);
            traffic_stats[port]++;
        }
    }

    free(tcpTable);
}


// =============================
// 5. Heavy ports
// =============================
void show_heavy_ports() {
    cout << "\nHeavy ports:\n";
    for (auto& p : traffic_stats) {
        if (p.second > 5)
            cout << "Port " << p.first << " -> " << p.second << endl;
    }
}


// =============================
// 6. Save log
// =============================
/**
void save_log() {
    time_t now = time(0);
    char* dt = ctime(&now);

    string filename = "firewall_log_" + string(dt) + ".txt";

    ofstream file(filename);

    for (auto& p : traffic_stats) {
        file << "Port: " << p.first << " Count: " << p.second << endl;
    }

    file.close();
}
**/


string get_timestamp() {
    time_t now = time(0);
    tm* ltm = localtime(&now);

    stringstream ss;
    ss << (1900 + ltm->tm_year) << "-"
        << setw(2) << setfill('0') << (1 + ltm->tm_mon) << "-"
        << setw(2) << setfill('0') << ltm->tm_mday << "_"
        << setw(2) << setfill('0') << ltm->tm_hour << "-"
        << setw(2) << setfill('0') << ltm->tm_min;

    return ss.str();
}



void save_log() {
    string filename = "firewall_log_" + get_timestamp() + ".txt";

    ofstream file(filename);

    if (!file.is_open()) {
        cerr << "ERROR: Cannot open file -    " << "!\n";
        return;
    }

    for (auto& p : traffic_stats) {
        file << "Port: " << p.first
            << " Count: " << p.second << endl;
    }

    file.close();

    cout << "Saved to " << filename << endl;
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
