# File Crypto x64 for Windows — Multi-Language File & Disk Encryption

A C++ file/disk encryption tool with **50 languages**, AES-256-CBC + SHA-256, supporting file, partition, and physical disk encryption. MIT licensed.

## Features

- **AES-256-CBC** + **SHA-256** sector-IV derivation
- **File Encryption**: encrypt any file with password or API Key
- **Partition Encryption** (logical drives like D:, E:): full or fast mode
- **Physical Disk Encryption** (PhysicalDrive N): raw sector-level encryption
- **API Key Mode**: generates a portable key string like `fck-v2-<base64>`
- **Password Mode**: PBKDF2-derived 256-bit key with random salt
- System/boot drive protection — cannot encrypt or decrypt system disks
- **50 languages**, config saved to `D:\file_crypto_config.ini`
- Crash handler with Chinese diagnostic messages

## Menu

1. **Encrypt File (Password)** — AES-256-CBC with password
2. **Encrypt File (API Key)** — generates random 64-byte key, outputs API key string
3. **Decrypt File (Password)**
4. **Decrypt File (API Key)**
5. **Change Language**
6. **Exit**
7. **Encrypt Partition** — choose password or API Key mode, encrypt logical volume
8. **Decrypt Partition** — choose password or API Key mode, decrypt logical volume

### Disk Encryption (sub-menu)

- **Encrypt Physical Disk (Password)** — raw sector encryption
- **Encrypt Physical Disk (API Key)** — 64-byte random key
- **Decrypt Physical Disk (Password)**
- **Decrypt Physical Disk (API Key)**

## API Key Format

```
fck-v2-eGhpcyBpc2E2NGJ5dGVrZXk...
```
Base64-encoded master key with `fck-v2-` prefix. **Save securely — the only way to decrypt!**

## Recovery Keys

Saved to `D:\` on encrypt:
- Password mode: `x_recovery.key`
- API Key mode: `x_api_recovery.key`

## Build

```bash
g++ -std=c++17 -O2 -march=native -static -pipe -o fc.exe src/*.cpp -I src -lbcrypt -lversion -ldbghelp
```

## Disclaimer

**This software may cause permanent data loss. The author is not responsible for any data loss or damage caused by using this software. Use at your own risk.**

## Translation Disclaimer

Translations for languages other than Simplified Chinese (zh-CN) and English are machine-assisted and may contain unnatural or imprecise expressions. Native speakers are warmly welcome to contribute improvements!

机器辅助翻译可能存在不自然的表达，欢迎母语使用者贡献改进！

## Contact

- WeChat: **forever870422**
- Email: **810372789@qq.com**

## License

MIT License — see [LICENSE](LICENSE)
