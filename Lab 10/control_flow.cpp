#include "control_flow.h"

int hidden_multiply(int a, int b) {
    int state = 0;
    int result = 0;

    while (true) {
        if (state == 0) {
            result = a * b;
            state = 1;
        }
        else if (state == 1) {
            break;
        }
    }

    return result;
}
