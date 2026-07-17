path = r"C:\Users\邓涛\Documents\New project 3\file-crypto\src\crypto.cpp"
with open(path, "r", encoding="utf-8") as f:
    content = f.read()

# Find GenerateKeyEncrypt and rewrite the encryption part to use single buffer
# The old pattern for GenerateKeyEncrypt reads file into plaintext, then pads, then encrypts
# New approach: read into one buffer, pad it, encrypt in-place

old_gen = '''    std::ifstream in(ToWidePath(inputPath), std::ios::binary | std::ios::ate);
    if (!in) { errorMsg = "Cannot open input file"; return false; }
    std::streamsize fileSize = in.tellg();
    in.seekg(0, std::ios::beg);
    std::vector<uint8_t> plaintext(fileSize);
    if (!in.read(reinterpret_cast<char*>(plaintext.data()), fileSize)) {
        errorMsg = "Cannot read input file"; return false;
    }
    in.close();

    uint8_t masterKey[MASTER_KEY_SIZE];
    uint8_t iv[AES256::BLOCK_SIZE];
    {
        std::mt19937 rng(static_cast<unsigned>(
            std::chrono::steady_clock::now().time_since_epoch().count()));
        std::uniform_int_distribution<int> dist(0, 255);
        for (int i = 0; i < MASTER_KEY_SIZE; ++i)
            masterKey[i] = static_cast<uint8_t>(dist(rng));
        for (int i = 0; i < AES256::BLOCK_SIZE; ++i)
            iv[i] = static_cast<uint8_t>(dist(rng));
    }

    AES256 aes(masterKey);

    size_t paddedLen = ((fileSize / AES256::BLOCK_SIZE) + 1) * AES256::BLOCK_SIZE;
    std::vector<uint8_t> padded(paddedLen);
    std::memcpy(padded.data(), plaintext.data(), fileSize);
    uint8_t padVal = static_cast<uint8_t>(AES256::BLOCK_SIZE - (fileSize % AES256::BLOCK_SIZE));
    for (size_t i = fileSize; i < paddedLen; ++i) padded[i] = padVal;

    std::vector<uint8_t> ciphertext(paddedLen);
    uint8_t prev[AES256::BLOCK_SIZE];
    std::memcpy(prev, iv, AES256::BLOCK_SIZE);

    auto pipBar = [](size_t pct) {
        const int w = 30;
        int filled = static_cast<int>(pct * w / 100);
        std::cout << "\\rEncrypting [";
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

new_gen = '''    try {
    std::ifstream in(ToWidePath(inputPath), std::ios::binary | std::ios::ate);
    if (!in) { errorMsg = "Cannot open input file"; return false; }
    std::streamsize fsize = in.tellg();
    in.seekg(0, std::ios::beg);

    // Single buffer: read file, pad in-place, encrypt in-place
    size_t paddedLen = ((static_cast<size_t>(fsize) / AES256::BLOCK_SIZE) + 1) * AES256::BLOCK_SIZE;
    std::vector<uint8_t> buf(paddedLen);
    if (!in.read(reinterpret_cast<char*>(buf.data()), fsize)) {
        errorMsg = "Cannot read input file"; return false;
    }
    in.close();

    // PKCS7 padding in-place
    uint8_t padVal = static_cast<uint8_t>(AES256::BLOCK_SIZE - (static_cast<size_t>(fsize) % AES256::BLOCK_SIZE));
    for (size_t i = static_cast<size_t>(fsize); i < paddedLen; ++i) buf[i] = padVal;

    // Generate key + IV
    uint8_t masterKey[MASTER_KEY_SIZE], iv[AES256::BLOCK_SIZE];
    {
        std::mt19937 rng(static_cast<unsigned>(
            std::chrono::steady_clock::now().time_since_epoch().count()));
        std::uniform_int_distribution<int> dist(0, 255);
        for (int i = 0; i < MASTER_KEY_SIZE; ++i) masterKey[i] = static_cast<uint8_t>(dist(rng));
        for (int i = 0; i < AES256::BLOCK_SIZE; ++i) iv[i] = static_cast<uint8_t>(dist(rng));
    }
    AES256 aes(masterKey);

    // CBC encrypt in-place (buffer = ciphertext output)
    uint8_t prev[AES256::BLOCK_SIZE];
    std::memcpy(prev, iv, AES256::BLOCK_SIZE);

    auto pipBar = [](size_t pct) {
        const int w = 30;
        int filled = static_cast<int>(pct * w / 100);
        std::cout << "\\rEncrypting [";
        for (int k = 0; k < w; ++k) std::cout << (k < filled ? '=' : (k == filled ? '>' : ' '));
        std::cout << "] " << pct << "%" << std::flush;
    };
    pipBar(0);
    size_t lastPct = 0;
    for (size_t i = 0; i < paddedLen; i += AES256::BLOCK_SIZE) {
        uint8_t block[AES256::BLOCK_SIZE];
        for (int j = 0; j < AES256::BLOCK_SIZE; ++j) block[j] = buf[i + j] ^ prev[j];
        aes.EncryptBlock(block, buf.data() + i);
        std::memcpy(prev, buf.data() + i, AES256::BLOCK_SIZE);
        size_t pct = i * 100 / paddedLen;
        if (pct / 5 != lastPct / 5) { lastPct = pct; pipBar(pct); }
    }
    pipBar(100);
    std::cout << std::endl;'''

content = content.replace(old_gen, new_gen)

# Also update the file write part to use buf instead of ciphertext
content = content.replace('''    std::string actualOutput = outputPath;
    bool hasExt = false;
    for (const char* ext : {".enc", ".bin", ".crypt"}) {
        size_t elen = strlen(ext);
        if (actualOutput.size() > elen &&
            actualOutput.compare(actualOutput.size() - elen, elen, ext) == 0) {
            hasExt = true; break;
        }
    }
    if (!hasExt) actualOutput += ".enc";

    std::ofstream out(ToWidePath(actualOutput), std::ios::binary);
    if (!out) { errorMsg = "Cannot create output file"; return false; }

    uint8_t header[48] = {};
    std::memcpy(header, iv, AES256::BLOCK_SIZE);
    std::memcpy(header + 32, "CRYPT02", 7);
    out.write(reinterpret_cast<char*>(header), 48);
    out.write(reinterpret_cast<char*>(ciphertext.data()), ciphertext.size());
    out.close();

    apiKeyOut = KEY_PREFIX + Base64Encode(masterKey, MASTER_KEY_SIZE);
    return true;
}''', '''    std::string actualOutput = outputPath;
    bool hasExt = false;
    for (const char* ext : {".enc", ".bin", ".crypt"}) {
        size_t elen = strlen(ext);
        if (actualOutput.size() > elen &&
            actualOutput.compare(actualOutput.size() - elen, elen, ext) == 0) {
            hasExt = true; break;
        }
    }
    if (!hasExt) actualOutput += ".enc";

    std::ofstream out(ToWidePath(actualOutput), std::ios::binary);
    if (!out) { errorMsg = "Cannot create output file"; return false; }

    uint8_t header[48] = {};
    std::memcpy(header, iv, AES256::BLOCK_SIZE);
    std::memcpy(header + 32, "CRYPT02", 7);
    out.write(reinterpret_cast<char*>(header), 48);
    out.write(reinterpret_cast<char*>(buf.data()), paddedLen);
    out.close();

    apiKeyOut = KEY_PREFIX + Base64Encode(masterKey, MASTER_KEY_SIZE);
    } catch (const std::exception& e) { errorMsg = std::string("Exception: ") + e.what(); return false; }
    catch (...) { errorMsg = "Unknown exception during encryption"; return false; }
    return true;
}''')

with open(path, "w", encoding="utf-8") as f:
    f.write(content)
print("Single buffer + exception handling")
