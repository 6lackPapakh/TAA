# ProtectUSB — USB Drive Content Protection

A command-line tool that encrypts and decrypts files on USB drives using XOR cipher.

## Build

```bash
make
```

Or with CMake:
```bash
mkdir build && cd build
cmake ..
make
```

## Usage

```
./protectusb [command] [arguments]
```

| Command | Description |
|---|---|
| `scan` | Detect connected USB drives |
| `list <path>` | List files at path |
| `protect <path>` | Encrypt all files on the drive |
| `unprotect <path>` | Decrypt all files on the drive |
| `encrypt <file>` | Encrypt a single file |
| `decrypt <file>` | Decrypt a single file |
| `info <path>` | Show drive info |

## Examples

```bash
./protectusb scan
./protectusb list /media/usb
./protectusb protect /media/usb
./protectusb unprotect /media/usb
./protectusb encrypt secret.txt
./protectusb decrypt secret.txt.encrypted
```
