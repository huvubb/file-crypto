# File Crypto - Multi-Language File Encryption Tool

A C++ file encryption/decryption tool with **50 language** support and API-key-style key management.

## Features

- **AES-256-CBC** encryption with random key generation
- **API Key mode**: generates a portable key string like `fck-v2-<base64>` for passwordless decrypt
- **Password mode**: traditional password-based encryption
- Auto-append `.enc` to encrypted outputs
- **50 languages** with config saved to `D:\file_crypto_config.ini`
- MIT licensed

## Menu

1. **Encrypt (Password)** — encrypt with a password
2. **API Key Encrypt** — generates a random 64-byte key, encrypts, displays API key string
3. **Decrypt (Password)** — decrypt with the original password
4. **API Key Decrypt** — decrypt using the API key string
5. **Change Language**
6. **Exit**

## API Key Format

```
fck-v2-eGhpcyBpc2E2NGJ5dGVrZXk...
```
Base64-encoded 64-byte master key with `fck-v2-` prefix. Save this string securely — it is the only way to decrypt!

## Build

```bash
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

## Disclaimer

**The author is not responsible for any data loss or damage caused by using this software. Use at your own risk.**

## Translation Disclaimer

Translations for languages other than Simplified Chinese and English are machine-assisted and may contain unnatural or imprecise expressions. Native speakers are warmly welcome to contribute improvements!

机器辅助翻译（非中英文）可能存在不自然的表达，欢迎母语使用者贡献改进！

## Contact

- WeChat: **forever870422**
- Email: **810372789@qq.com**

## Contributing

Welcome translation contributions! Contact forever870422.

## License

MIT License — see [LICENSE](LICENSE)