# ProtectUSB Menu Version

This folder contains the updated interactive version with a menu-based algorithm selector.

It now supports compressing data before encryption.

## Available Algorithms

- XOR
- Caesar
- Vigenere
- XorShiftStream
- RotateXOR
- AES-256
- ChaCha20

## Compression Modes

- None
- RLE

## Build

```bash
make
```

## Run

```bash
./protectusb_menu
```

The menu version now applies compression first, then encrypts the compressed data.

`AES-256` and `ChaCha20` use OpenSSL with a password-derived key, random salt, and random IV/nonce stored in the encrypted file header.
