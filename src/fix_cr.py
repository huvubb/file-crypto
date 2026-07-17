path = r"C:\Users\邓涛\Documents\New project 3\file-crypto\src\crypto.cpp"
with open(path, "r", encoding="utf-8") as f:
    content = f.read()

# The broken progress block
broken = """    std::cout << "Encrypting... 0%" << std::flush;
    size_t lastPct = 0;
    for (size_t i = 0; i < paddedLen; i += AES256::BLOCK_SIZE) {
        uint8_t block[AES256::BLOCK_SIZE];
        for (int j = 0; j < AES256::BLOCK_SIZE; ++j) block[j] = padded[i + j] ^ prev[j];
        aes.EncryptBlock(block, ciphertext.data() + i);
        std::memcpy(prev, ciphertext.data() + i, AES256::BLOCK_SIZE);
        size_t pct = i * 100 / paddedLen;
        if (pct / 10 != lastPct / 10) { lastPct = pct; std::cout << \"\rEncrypting... \" << pct << \"%\" << std::flush; }
    }
    std::cout << \"\rEncrypting... 100%\n\";

    // Auto-append .enc
    std::string actualOutput = outputPath;"""

fixed = """    std::cout << "Encrypting... 0%" << std::flush;
    size_t lastPct = 0;
    for (size_t i = 0; i < paddedLen; i += AES256::BLOCK_SIZE) {
        uint8_t block[AES256::BLOCK_SIZE];
        for (int j = 0; j < AES256::BLOCK_SIZE; ++j) block[j] = padded[i + j] ^ prev[j];
        aes.EncryptBlock(block, ciphertext.data() + i);
        std::memcpy(prev, ciphertext.data() + i, AES256::BLOCK_SIZE);
        size_t pct = i * 100 / paddedLen;
        if (pct / 10 != lastPct / 10) { lastPct = pct; std::cout << char(13) << "Encrypting... " << pct << "%" << std::flush; }
    }
    std::cout << char(13) << "Encrypting... 100%\n";

    // Auto-append .enc
    std::string actualOutput = outputPath;"""

content = content.replace(broken, fixed)
with open(path, "w", encoding="utf-8") as f:
    f.write(content)
print("Fixed CR in progress strings")
