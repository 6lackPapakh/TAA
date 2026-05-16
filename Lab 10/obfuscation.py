def xor_encode(text, key):
    return [ord(c) ^ key for c in text]

def xor_decode(data, key):
    return ''.join(chr(c ^ key) for c in data)



def obfuscated_sum(a, b):
    result = 0

    # Dead code
    for i in range(5):
        temp = i * 999  # useless

    # Control flow obfuscation
    x = 0
    while True:
        if x == 0:
            result = a + b
            x = 1
        elif x == 1:
            break

    return result



original = "Hello World"
key = 23

encoded = xor_encode(original, key)
print("Encoded:", encoded)

decoded = xor_decode(encoded, key)
print("Decoded:", decoded)


print(obfuscated_sum(5, 7))
