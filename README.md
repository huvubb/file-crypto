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

## Translation Disclaimer

> **注意 / Note:** 除简体中文和英文外，其他语言的翻译为机器辅助生成，可能存在不够自然或不够准确的表达。我们欢迎母语使用者提交翻译改进！
>
> Translations for languages other than Simplified Chinese and English are machine-assisted and may contain unnatural or imprecise expressions. **Native speakers are warmly welcome to contribute improvements!**
>
> 簡體中文和英文除外的翻譯為機器輔助生成，歡迎母語使用者貢獻改進！

## Contact

- WeChat: **forever870422**
- Email: **810372789@qq.com**

## Contributing

**Welcome translation contributions!** Contact forever870422.

## License

MIT License — see [LICENSE](LICENSE)
