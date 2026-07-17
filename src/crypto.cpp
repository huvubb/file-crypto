#include "crypto.hpp"
#include "aes256.hpp"
#include <fstream>
#include <random>
#include <chrono>
#include <cstring>
#include <iostream>

static constexpr int MASTER_KEY_SIZE = 64;
static constexpr const char* KEY_PREFIX = "fck-v2-";

static const char B64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static std::string Base64Encode(const uint8_t* data, size_t len) {
    std::string r;
    r.reserve(((len + 2) / 3) * 4);
    for (size_t i = 0; i < len; i += 3) {
        uint32_t n = static_cast<uint32_t>(data[i]) << 16;
        if (i + 1 < len) n |= static_cast<uint32_t>(data[i + 1]) << 8;
        if (i + 2 < len) n |= static_cast<uint32_t>(data[i + 2]);
        r += B64[(n >> 18) & 0x3F];
        r += B64[(n >> 12) & 0x3F];
        r += (i + 1 < len) ? B64[(n >> 6) & 0x3F] : '=';
        r += (i + 2 < len) ? B64[n & 0x3F] : '=';
    }
    return r;
}

static std::vector<uint8_t> Base64Decode(const std::string& s) {
    auto idx = [](char c) -> int {
        if (c >= 'A' && c <= 'Z') return c - 'A';
        if (c >= 'a' && c <= 'z') return c - 'a' + 26;
        if (c >= '0' && c <= '9') return c - '0' + 52;
        if (c == '+') return 62; if (c == '/') return 63;
        return -1;
    };
    std::vector<uint8_t> r;
    r.reserve((s.size() / 4) * 3);
    for (size_t i = 0; i + 3 < s.size(); i += 4) {
        int v0 = idx(s[i]), v1 = idx(s[i+1]), v2 = idx(s[i+2]), v3 = idx(s[i+3]);
        int pads = (s[i+2] == '=') + (s[i+3] == '=');
        uint32_t n = (static_cast<uint32_t>(v0) << 18) | (static_cast<uint32_t>(v1) << 12);
        if (s[i+2] != '=') n |= static_cast<uint32_t>(v2) << 6;
        if (s[i+3] != '=') n |= static_cast<uint32_t>(v3);
        r.push_back(static_cast<uint8_t>(n >> 16));
        if (pads < 2) r.push_back(static_cast<uint8_t>(n >> 8));
        if (pads < 1) r.push_back(static_cast<uint8_t>(n));
    }
    return r;
}

bool FileCrypto::EncryptFile(const std::string& inputPath,
                              const std::string& outputPath,
                              const std::string& password,
                              std::string& errorMsg) {
    std::ifstream in(inputPath, std::ios::binary | std::ios::ate);
    if (!in) { errorMsg = "Cannot open input file"; return false; }
    std::streamsize fileSize = in.tellg();
    in.seekg(0, std::ios::beg);
    std::vector<uint8_t> plaintext(fileSize);
    if (!in.read(reinterpret_cast<char*>(plaintext.data()), fileSize)) {
        errorMsg = "Cannot read input file"; return false;
    }
    in.close();

    uint8_t salt[16], iv[AES256::BLOCK_SIZE];
    {
        std::mt19937 rng(static_cast<unsigned>(
            std::chrono::steady_clock::now().time_since_epoch().count()));
        std::uniform_int_distribution<int> dist(0, 255);
        for (int i = 0; i < 16; ++i) salt[i] = static_cast<uint8_t>(dist(rng));
        for (int i = 0; i < AES256::BLOCK_SIZE; ++i)
            iv[i] = static_cast<uint8_t>(dist(rng));
    }

    auto aesKey = AES256::DeriveKey256(password, salt, 16);
    AES256 aes(aesKey.data());

    size_t paddedLen = ((fileSize / AES256::BLOCK_SIZE) + 1) * AES256::BLOCK_SIZE;
    std::vector<uint8_t> padded(paddedLen);
    std::memcpy(padded.data(), plaintext.data(), fileSize);
    uint8_t padVal = static_cast<uint8_t>(AES256::BLOCK_SIZE - (fileSize % AES256::BLOCK_SIZE));
    for (size_t i = fileSize; i < paddedLen; ++i) padded[i] = padVal;

    std::vector<uint8_t> ciphertext(paddedLen);
    uint8_t prev[AES256::BLOCK_SIZE];
    std::memcpy(prev, iv, AES256::BLOCK_SIZE);
    for (size_t i = 0; i < paddedLen; i += AES256::BLOCK_SIZE) {
        uint8_t block[AES256::BLOCK_SIZE];
        for (int j = 0; j < AES256::BLOCK_SIZE; ++j) block[j] = padded[i + j] ^ prev[j];
        aes.EncryptBlock(block, ciphertext.data() + i);
        std::memcpy(prev, ciphertext.data() + i, AES256::BLOCK_SIZE);
    }

    std::ofstream out(outputPath, std::ios::binary);
    if (!out) { errorMsg = "Cannot create output file"; return false; }
    uint8_t header[48];
    std::memcpy(header, salt, 16);
    std::memcpy(header + 16, iv, AES256::BLOCK_SIZE);
    std::memcpy(header + 32, "CRYPT01", 7);
    out.write(reinterpret_cast<char*>(header), 48);
    out.write(reinterpret_cast<char*>(ciphertext.data()), ciphertext.size());
    out.close();
    return true;
}

bool FileCrypto::DecryptFile(const std::string& inputPath,
                              const std::string& outputPath,
                              const std::string& password,
                              std::string& errorMsg) {
    std::ifstream in(inputPath, std::ios::binary | std::ios::ate);
    if (!in) { errorMsg = "Cannot open input file"; return false; }
    std::streamsize fileSize = in.tellg();
    if (fileSize < 48) { errorMsg = "Invalid encrypted file"; return false; }
    in.seekg(0, std::ios::beg);
    uint8_t header[48];
    if (!in.read(reinterpret_cast<char*>(header), 48)) {
        errorMsg = "Cannot read file header"; return false;
    }
    if (std::memcmp(header + 32, "CRYPT01", 7) != 0) {
        errorMsg = "Not a valid encrypted file"; return false;
    }
    uint8_t salt[16], iv[AES256::BLOCK_SIZE];
    std::memcpy(salt, header, 16);
    std::memcpy(iv, header + 16, AES256::BLOCK_SIZE);

    size_t cipherLen = static_cast<size_t>(fileSize - 48);
    std::vector<uint8_t> ciphertext(cipherLen);
    if (!in.read(reinterpret_cast<char*>(ciphertext.data()), cipherLen)) {
        errorMsg = "Cannot read ciphertext"; return false;
    }
    in.close();

    auto aesKey = AES256::DeriveKey256(password, salt, 16);
    AES256 aes(aesKey.data());
    std::vector<uint8_t> plaintext(cipherLen);
    uint8_t prev[AES256::BLOCK_SIZE];
    std::memcpy(prev, iv, AES256::BLOCK_SIZE);
    for (size_t i = 0; i < cipherLen; i += AES256::BLOCK_SIZE) {
        uint8_t decrypted[AES256::BLOCK_SIZE];
        aes.DecryptBlock(ciphertext.data() + i, decrypted);
        for (int j = 0; j < AES256::BLOCK_SIZE; ++j)
            plaintext[i + j] = decrypted[j] ^ prev[j];
        std::memcpy(prev, ciphertext.data() + i, AES256::BLOCK_SIZE);
    }
    uint8_t padVal = plaintext.back();
    if (padVal == 0 || padVal > AES256::BLOCK_SIZE) {
        errorMsg = "Wrong password or corrupted file"; return false;
    }
    for (int i = 0; i < padVal; ++i)
        if (plaintext[plaintext.size() - 1 - i] != padVal) {
            errorMsg = "Wrong password or corrupted file"; return false;
        }
    size_t origLen = plaintext.size() - padVal;
    std::ofstream out(outputPath, std::ios::binary);
    if (!out) { errorMsg = "Cannot create output file"; return false; }
    out.write(reinterpret_cast<char*>(plaintext.data()), origLen);
    out.close();
    return true;
}

bool FileCrypto::GenerateKeyEncrypt(const std::string& inputPath,
                                     const std::string& outputPath,
                                     std::string& apiKeyOut,
                                     std::string& errorMsg) {
    std::ifstream in(inputPath, std::ios::binary | std::ios::ate);
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

    std::cout << "Encrypting... 0%" << std::flush;
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
    std::cout << "\rEncrypting... 100%" << std::endl;

    std::string actualOutput = outputPath;
    bool hasExt = false;
    for (const char* ext : {".enc", ".bin", ".crypt"}) {
        size_t elen = strlen(ext);
        if (actualOutput.size() > elen &&
            actualOutput.compare(actualOutput.size() - elen, elen, ext) == 0) {
            hasExt = true; break;
        }
    }
    if (!hasExt) actualOutput += ".enc";

    std::ofstream out(actualOutput, std::ios::binary);
    if (!out) { errorMsg = "Cannot create output file"; return false; }

    uint8_t header[48] = {};
    std::memcpy(header, iv, AES256::BLOCK_SIZE);
    std::memcpy(header + 32, "CRYPT02", 7);
    out.write(reinterpret_cast<char*>(header), 48);
    out.write(reinterpret_cast<char*>(ciphertext.data()), ciphertext.size());
    out.close();

    apiKeyOut = KEY_PREFIX + Base64Encode(masterKey, MASTER_KEY_SIZE);
    return true;
}

bool FileCrypto::KeyDecrypt(const std::string& inputPath,
                             const std::string& outputPath,
                             const std::string& apiKey,
                             std::string& errorMsg) {
    if (apiKey.compare(0, strlen(KEY_PREFIX), KEY_PREFIX) != 0) {
        errorMsg = "Invalid API key format (expected fck-v2-...)";
        return false;
    }
    std::string b64part = apiKey.substr(strlen(KEY_PREFIX));
    auto keyBytes = Base64Decode(b64part);
    if (keyBytes.size() != MASTER_KEY_SIZE) {
        errorMsg = "Invalid API key (wrong key length)";
        return false;
    }

    std::ifstream in(inputPath, std::ios::binary | std::ios::ate);
    if (!in) { errorMsg = "Cannot open encrypted file"; return false; }
    std::streamsize fileSize = in.tellg();
    if (fileSize < 48) { errorMsg = "Invalid encrypted file"; return false; }
    in.seekg(0, std::ios::beg);

    uint8_t header[48];
    if (!in.read(reinterpret_cast<char*>(header), 48)) {
        errorMsg = "Cannot read file header"; return false;
    }
    if (std::memcmp(header + 32, "CRYPT02", 7) != 0) {
        errorMsg = "Not a valid API-key encrypted file (use password decrypt instead)";
        return false;
    }
    uint8_t iv[AES256::BLOCK_SIZE];
    std::memcpy(iv, header, AES256::BLOCK_SIZE);

    size_t cipherLen = static_cast<size_t>(fileSize - 48);
    std::vector<uint8_t> ciphertext(cipherLen);
    if (!in.read(reinterpret_cast<char*>(ciphertext.data()), cipherLen)) {
        errorMsg = "Cannot read ciphertext"; return false;
    }
    in.close();

    AES256 aes(keyBytes.data());
    std::vector<uint8_t> plaintext(cipherLen);
    uint8_t prev[AES256::BLOCK_SIZE];
    std::memcpy(prev, iv, AES256::BLOCK_SIZE);
    for (size_t i = 0; i < cipherLen; i += AES256::BLOCK_SIZE) {
        uint8_t decrypted[AES256::BLOCK_SIZE];
        aes.DecryptBlock(ciphertext.data() + i, decrypted);
        for (int j = 0; j < AES256::BLOCK_SIZE; ++j)
            plaintext[i + j] = decrypted[j] ^ prev[j];
        std::memcpy(prev, ciphertext.data() + i, AES256::BLOCK_SIZE);
    }

    uint8_t padVal2 = plaintext.back();
    if (padVal2 == 0 || padVal2 > AES256::BLOCK_SIZE) {
        errorMsg = "Wrong API key or corrupted file"; return false;
    }
    for (int i = 0; i < padVal2; ++i)
        if (plaintext[plaintext.size() - 1 - i] != padVal2) {
            errorMsg = "Wrong API key or corrupted file"; return false;
        }
    size_t origLen = plaintext.size() - padVal2;

    std::ofstream out(outputPath, std::ios::binary);
    if (!out) { errorMsg = "Cannot create output file"; return false; }
    out.write(reinterpret_cast<char*>(plaintext.data()), origLen);
    out.close();
    return true;
}
