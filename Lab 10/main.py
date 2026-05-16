from xor_obfuscation import encode, decode
from control_flow import hidden_multiply

text = "Secret"
key = 12

enc = encode(text, key)
print("Encoded:", enc)

dec = decode(enc, key)
print("Decoded:", dec)

print("Multiply:", hidden_multiply(3, 4))
