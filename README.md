# File Crypto - Multi-Language File Encryption Tool

A C++ file encryption/decryption tool with **50 language** support, featuring AES-256-CBC encryption, SHA-256 verification, and parity-based key splitting.

## Features

- **AES-256-CBC** encryption with 200,000-round key derivation
- **SHA-256** hash verification of encryption keys
- **64-byte (512-bit) master key** with parity split/reconstruction
- **Key file generation** (.key) with automatic backup (.key.bak)
- **Parity split**: key split by even/odd byte positions, stored separately
- **Parity rebuild**: reconstruct master key from even/odd halves, verified by SHA-256
- Encrypted output auto-appends `.enc` binary extension
- **50 languages** supported (config saved to `D:\file_crypto_config.ini`)
- MIT licensed, open source

## Menu Options

1. **Encrypt (Password)** - simple password-based encryption
2. **Encrypt + Key File** - encrypts file, generates .key + .key.bak with parity-split 512-bit key
3. **Decrypt (Password)** - password-based decryption
4. **Decrypt with Key File** - decrypt using .key file (parity-rebuild + SHA-256 verify)
5. **Change Language** - switch UI language
6. **Exit**

## Key File Format (v2)

| Offset | Size | Field |
|--------|------|-------|
| 0 | 8 | Magic "KEYFILE1" |
| 8 | 1 | Version (2) |
| 9 | 16 | Salt |
| 25 | 16 | IV |
| 41 | 32 | SHA-256 of 64-byte master key |
| 73 | 32 | Even-parity half (bytes 0,2,4,...,62) |
| 105 | 32 | Odd-parity half (bytes 1,3,5,...,63) |

Total: 137 bytes

## Build

### Prerequisites
- CMake 3.14+
- C++17 compiler (MSVC, GCC, or Clang)

### Windows (MSVC)
```bash
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

## Contact

- WeChat: **forever870422**
- Email: **810372789@qq.com**

## Contributing

**Welcome translation contributions!** Contact forever870422 to help translate.

## License

MIT License - see [LICENSE](LICENSE)
