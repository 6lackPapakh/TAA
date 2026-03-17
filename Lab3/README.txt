the script is designed for linux environment

this compiles everything into an executable: g++ -std=c++11 -Iinclude src/HardwareID.cpp src/LicenseKey.cpp src/LicenseManager.cpp src/main.cpp -o LicensingSystem

then run: ./LicensingSystem

do this if permission is denied: chmod +x LicensingSystem 
