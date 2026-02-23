#!/bin/bash

cpu=$(lscpu | grep "Model name" | cut -d: -f2 | xargs)
mac=$(ip link | grep link/ether | awk '{print $2}' | head -n 1)
disk=$(lsblk -d -o SIZE | sed -n '2p')

fingerprint="$cpu-$mac-$disk"

echo "Fingerprint:"
echo "$fingerprint"

echo
echo "SHA256:"
echo -n "$fingerprint" | sha256sum
