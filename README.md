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


## Disclaimer / 免责声明

**English:** The author is not responsible for any data loss or damage caused by using this software. Use at your own risk.

**中文:** 作者不对使用本软件造成的任何数据丢失或损坏承担责任。使用风险自负。

**日本語:** 作者はこのソフトウェアの使用によるいかなるデータ損失や損害についても責任を負いません。

**한국어:** 작성자는 이 소프트웨어 사용으로 인한 데이터 손실이나 손해에 대해 책임을 지지 않습니다.

**Deutsch:** Der Autor übernimmt keine Verantwortung für Datenverluste oder Schäden durch diese Software.

**Français:** L'auteur n'est pas responsable des pertes de données ou dommages causés par ce logiciel.

**Español:** El autor no se responsabiliza por pérdida de datos o daños causados por este software.

**Русский:** Автор не несет ответственности за потерю данных или ущерб от использования ПО.

**العربية:** المؤلف غير مسؤول عن أي فقدان للبيانات أو ضرر ناتج عن هذا البرنامج.

**हिन्दी:** लेखक इस सॉफ़्टवेयर के उपयोग से होने वाली किसी भी डेटा हानि या क्षति के लिए जिम्मेदार नहीं है।