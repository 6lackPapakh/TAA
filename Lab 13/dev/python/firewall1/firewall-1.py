import socket
import psutil
import datetime
import json
from collections import defaultdict

# Պրոտոկոլների mapping
PORT_PROTOCOLS = {
    80: "HTTP",
    443: "HTTPS",
    21: "FTP",
    22: "SSH",
    23: "Telnet"
}

# Firewall կանոնների պահպանում
firewall_rules = {
    "blocked_in": set(),
    "blocked_out": set()
}

# Տրաֆիկի վիճակագրություն
traffic_stats = defaultdict(int)


# ===============================
# 1. Բաց պորտերի ցուցադրում
# ===============================
def list_open_ports():
    connections = psutil.net_connections()
    result = []

    for conn in connections:
        if conn.laddr:
            port = conn.laddr.port
            proto = PORT_PROTOCOLS.get(port, "TCP/UDP")

            result.append({
                "port": port,
                "protocol": proto,
                "status": conn.status
            })

    return result


# ===============================
# 2. Պորտի արգելափակում
# ===============================
def block_port(port, direction="in"):
    if direction == "in":
        firewall_rules["blocked_in"].add(port)
    else:
        firewall_rules["blocked_out"].add(port)


# ===============================
# 3. Պորտի բացում
# ===============================
def open_port(port):
    firewall_rules["blocked_in"].discard(port)
    firewall_rules["blocked_out"].discard(port)


# ===============================
# 4. Տրաֆիկի մոնիտորինգ
# ===============================
def monitor_traffic(duration=10):
    print(f"Monitoring traffic for {duration} seconds...")

    for _ in range(duration):
        connections = psutil.net_connections()

        for conn in connections:
            if conn.laddr:
                port = conn.laddr.port
                traffic_stats[port] += 1


# ===============================
# 5. Ծանրաբեռնված պորտեր
# ===============================
def get_heavy_ports(top_n=5):
    sorted_ports = sorted(
        traffic_stats.items(),
        key=lambda x: x[1],
        reverse=True
    )

    return sorted_ports[:top_n]


# ===============================
# 6. Տվյալների պահպանում
# ===============================
def save_to_file(data):
#'''
    timestamp = datetime.datetime.now().strftime("%Y-%m-%d_%H-%M")
    filename = f"firewall_log_{timestamp}.json"

    with open(filename, "w") as f:
        json.dump(data, f, indent=4)

    print(f"Saved to {filename}")
#'''



# ===============================
# MAIN DEMO
# ===============================
if __name__ == "__main__":
    ports = list_open_ports()
    print("Open ports:")
    for p in ports[:10]:
        print(p)

    block_port(80, "in")
    block_port(22, "out")

    monitor_traffic(5)

    heavy = get_heavy_ports()
    print("Heavy ports:", heavy)

'''
    save_to_file({
        "open_ports": ports,
        "heavy_ports": heavy,
        "rules": list(firewall_rules.items())
    })

'''

save_to_file({
    "open_ports": ports,
    "heavy_ports": heavy,
    "rules": {
        "blocked_in": list(firewall_rules["blocked_in"]),
        "blocked_out": list(firewall_rules["blocked_out"])
    }
})
