#include <iostream>
#include "xor_obfuscation.h"
#include "control_flow.h"

using namespace std;

int main() {
    string text = "Secret";
    int key = 12;

    vector<int> enc = encode(text, key);

    cout << "Encoded: ";
    for (int val : enc) {
        cout << val << " ";
    }
    cout << endl;

    cout << "Decoded: " << decode(enc, key) << endl;

    cout << "Multiply: " << hidden_multiply(3, 4) << endl;

    return 0;
}
