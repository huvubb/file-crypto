# File Crypto - Multi-Language File Encryption Tool

A C++ file encryption/decryption tool with **50 language** support, using AES-256-CBC encryption.

## Features

- **AES-256-CBC** encryption with PBKDF2-style key derivation
- **50 languages** supported (config saved to `D:\file_crypto_config.ini`)
- MIT licensed, open source
- Console-based interactive interface

## Supported Languages (50)

English, 简体中文, 繁體中文, Español, Français, Deutsch, 日本語, 한국어, Русский, العربية, Português, Italiano, Nederlands, हिन्दी, Türkçe, Tiếng Việt, ไทย, Polski, Українська, Svenska, Dansk, Suomi, Norsk, Čeština, Magyar, Română, Ελληνικά, עברית, Bahasa Indonesia, Bahasa Melayu, বাংলা, اردو, فارسی, தமிழ், తెలుగు, मराठी, ಕನ್ನಡ, ગુજરાતી, ਪੰਜਾਬੀ, မြန်မာ, ភាសាខ្មែរ, ລາວ, नेपाली, සිංහල, አማርኛ, Kiswahili, Filipino, Српски, Hrvatski, Български

## Build

### Prerequisites
- CMake 3.14+
- C++17 compiler (MSVC, GCC, or Clang)

### Windows (MSVC)
```bash
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

### Linux/macOS
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

## Usage

Run `file-crypto.exe`. On first launch, select your language (saved to `D:\file_crypto_config.ini`).

1. **Encrypt File** - encrypt any file with a password
2. **Decrypt File** - decrypt a previously encrypted file
3. **Change Language** - switch UI language
4. **Exit**

## Contact

- WeChat: **forever870422**
- Email: **810372789@qq.com**

## Contributing

**Welcome translation contributions!** Contact forever870422 to help translate into more languages or improve existing translations.

## License

MIT License - see [LICENSE](LICENSE)
