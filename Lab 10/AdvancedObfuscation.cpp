
// Macro-based Obfuscation

// a+b-ը փոխարինվում է բիթային մաթեմատիկայով
#define ADD(a,b) ((a ^ b) + 2*(a & b))



// Template Obfuscation
//Հաշվարկը կատարվում է compile-time-ում
// Reverse engineering-ը բարդանում է

template<int A, int B>
struct Add {
    static const int value = A + B;
};



// Inline ASM (Low-level Obfuscation)

int add(int a, int b) {
    int result;
    __asm__ (
        "addl %%ebx, %%eax;"
        : "=a"(result)
        : "a"(a), "b"(b)
    );
    return result;
}




int main() {
    int x = 5, y = 3;
    int result1 = ADD(x,y);

	int result2 = Add<5, 3>::value;

	int result3 = add(5, 3);


}


