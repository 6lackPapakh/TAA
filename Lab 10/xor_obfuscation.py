def encode(text, key):
    return [ord(c) ^ key for c in text]

def decode(data, key):
    return ''.join(chr(c ^ key) for c in data)
