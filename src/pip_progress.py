path = r"C:\Users\邓涛\Documents\New project 3\file-crypto\src\crypto.cpp"
with open(path, "r", encoding="utf-8") as f:
    content = f.read()

# Replace the progress printing with pip-style bar
old_progress = '''    std::cout << "Encrypting... 0%" << std::flush;
    size_t lastPct = 0;
    for (size_t i = 0; i < paddedLen; i += AES256::BLOCK_SIZE) {
        uint8_t block[AES256::BLOCK_SIZE];
        for (int j = 0; j < AES256::BLOCK_SIZE; ++j) block[j] = padded[i + j] ^ prev[j];
        aes.EncryptBlock(block, ciphertext.data() + i);
        std::memcpy(prev, ciphertext.data() + i, AES256::BLOCK_SIZE);
        size_t pct = i * 100 / paddedLen;
        if (pct / 10 != lastPct / 10) {
            lastPct = pct;
            std::cout << "\rEncrypting... " << pct << "%" << std::flush;
        }
    }
    std::cout << "\rEncrypting... 100%" << std::endl;'''

new_progress = '''    auto pipBar = [](size_t pct) {
        const int w = 30;
        int filled = static_cast<int>(pct * w / 100);
        std::cout << "\rEncrypting [";
        for (int k = 0; k < w; ++k) std::cout << (k < filled ? '=' : (k == filled ? '>' : ' '));
        std::cout << "] " << pct << "%" << std::flush;
    };
    pipBar(0);
    size_t lastPct = 0;
    for (size_t i = 0; i < paddedLen; i += AES256::BLOCK_SIZE) {
        uint8_t block[AES256::BLOCK_SIZE];
        for (int j = 0; j < AES256::BLOCK_SIZE; ++j) block[j] = padded[i + j] ^ prev[j];
        aes.EncryptBlock(block, ciphertext.data() + i);
        std::memcpy(prev, ciphertext.data() + i, AES256::BLOCK_SIZE);
        size_t pct = i * 100 / paddedLen;
        if (pct / 5 != lastPct / 5) { lastPct = pct; pipBar(pct); }
    }
    pipBar(100);
    std::cout << std::endl;'''

content = content.replace(old_progress, new_progress)
with open(path, "w", encoding="utf-8") as f:
    f.write(content)
print("Pip-style progress bar added")
