#include <iostream>
#include <vector>

using namespace std;

struct RLE_Pair {
    int count;
    int value;
};

// Encode
vector<RLE_Pair> rle_encode(const vector<int>& data) {
    vector<RLE_Pair> encoded;
    if (data.empty()) return encoded;
    int count = 1;
    int current = data[0];
    for (size_t i = 1; i < data.size(); ++i) {
        if (data[i] == current && count < 255) {
            count++;
        } else {
            // If value > 255 → split it
            if (current > 255) {
                int temp = current;
                while (temp > 255) {
                    encoded.push_back({1, 255});
                    temp -= 255;
                }
                encoded.push_back({1, temp});
            } 
            else {
                encoded.push_back({count, current});
            }
            current = data[i];
            count = 1;
        }
    }
    // Last element
    if (current > 255) {
        int temp = current;
        while (temp > 255) {
            encoded.push_back({1, 255});
            temp -= 255;
        }
        encoded.push_back({1, temp});
    } 
    else {
        encoded.push_back({count, current});
    }
    return encoded;
}

// Decode
vector<int> rle_decode(const vector<RLE_Pair>& encoded) {
    vector<int> decoded;
    int accumulator = 0;
    for (const auto& p : encoded) {
        if (p.count == 1 && p.value == 255) {
            accumulator += 255;
        }
        else if (p.count == 1 && accumulator > 0) {
            accumulator += p.value;
            decoded.push_back(accumulator);
            accumulator = 0;
        }
        else {
            for (int i = 0; i < p.count; ++i) {
                decoded.push_back(p.value);
            }
        }
    }
    return decoded;
}

// MAIN
int main() {
    int n;
    cout << "How many elements do you want to enter in the array? ";
    cin >> n;
    if (n <= 0) {
        cout << "The array is empty.\n";
        return 0;
    }
    vector<int> original(n);
    cout << "Enter " << n << " elements:\n>> ";
    for (int i = 0; i < n; ++i) {
        cin >> original[i];
    }
    // Encode
    vector<RLE_Pair> encoded = rle_encode(original);
    cout << "\n[ ENCODE ]\n";
    for (const auto& p : encoded) {
        cout << "[" << p.count << "," << p.value << "] ";
    }
    cout << "\n";
    // Decode
    vector<int> decoded = rle_decode(encoded);
    cout << "\n[ DECODE ]\n";
    for (int x : decoded) {
        cout << x << " ";
    }
    // Verification
    cout << "\n\n[ VERIFICATION ] ";
    if (original == decoded)
        cout << "YES ";
    else
        cout << "NO";
    return 0;
}
