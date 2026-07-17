path = r"C:\Users\邓涛\Documents\New project 3\file-crypto\src\crypto.cpp"
with open(path, "r", encoding="utf-8") as f:
    content = f.read()

# Add progress printing in GenerateKeyEncrypt's encryption loop
# Find the loop and add progress
old_loop = '''    for (size_t i = 0; i < paddedLen; i += AES256::BLOCK_SIZE) {
        uint8_t block[AES256::BLOCK_SIZE];
        for (int j = 0; j < AES256::BLOCK_SIZE; ++j) block[j] = padded[i + j] ^ prev[j];
        aes.EncryptBlock(block, ciphertext.data() + i);
        std::memcpy(prev, ciphertext.data() + i, AES256::BLOCK_SIZE);
    }

    // Auto-append .enc'''

new_loop = '''    std::cout << "Encrypting... 0%" << std::flush;
    size_t lastPct = 0;
    for (size_t i = 0; i < paddedLen; i += AES256::BLOCK_SIZE) {
        uint8_t block[AES256::BLOCK_SIZE];
        for (int j = 0; j < AES256::BLOCK_SIZE; ++j) block[j] = padded[i + j] ^ prev[j];
        aes.EncryptBlock(block, ciphertext.data() + i);
        std::memcpy(prev, ciphertext.data() + i, AES256::BLOCK_SIZE);
        size_t pct = i * 100 / paddedLen;
        if (pct / 10 != lastPct / 10) { lastPct = pct; std::cout << "\rEncrypting... " << pct << "%" << std::flush; }
    }
    std::cout << "\rEncrypting... 100%\n";

    // Auto-append .enc'''

# Only replace the first occurrence (in GenerateKeyEncrypt)
idx = content.find(old_loop)
if idx != -1:
    content = content[:idx] + new_loop + content[idx + len(old_loop):]
    print("Added progress to GenerateKeyEncrypt")
else:
    print("WARNING: loop not found for GenerateKeyEncrypt")

with open(path, "w", encoding="utf-8") as f:
    f.write(content)
