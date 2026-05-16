void block_port_win(int port) {
    string cmd = "powershell New-NetFirewallRule -DisplayName \"BlockPort\" "
                 "-Direction Inbound -LocalPort " + to_string(port) +
                 " -Protocol TCP -Action Block";

    system(cmd.c_str());
}

