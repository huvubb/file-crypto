#include "crypto.hpp"
#include "aes256.hpp"
#include "sha256.hpp"
#include <fstream>
#include <random>
#include <chrono>
#include <cstring>

static constexpr int MASTER_KEY_SIZE = 64;   // 512-bit master key
static constexpr int HALF_KEY_SIZE   = 32;   // each parity half = 32 bytes

// --- Parity Split / Rebuild (64-byte master key) ---

void FileCrypto::ParitySplit(const uint8_t key[MASTER_KEY_SIZE],
                              uint8_t evenHalf[HALF_KEY_SIZE],
                              uint8_t oddHalf[HALF_KEY_SIZE]) {
    for (int i = 0; i < HALF_KEY_SIZE; ++i) {
        evenHalf[i] = key[2 * i];       // positions 0,2,4,...,62
        oddHalf[i]  = key[2 * i + 1];   // positions 1,3,5,...,63
    }
}

void FileCrypto::ParityRebuild(const uint8_t evenHalf[HALF_KEY_SIZE],
                                const uint8_t oddHalf[HALF_KEY_SIZE],
                                uint8_t key[MASTER_KEY_SIZE]) {
    for (int i = 0; i < HALF_KEY_SIZE; ++i) {
        key[2 * i]     = evenHalf[i];
        key[2 * i + 1] = oddHalf[i];
    }
}

// Helper: derive 64-byte master key from password + salt
static std::vector<uint8_t> DeriveMasterKey(const std::string& password,
                                             const uint8_t* salt, size_t saltLen) {
    std::vector<uint8_t> key(MASTER_KEY_SIZE);
    std::vector<uint8_t> data(saltLen + password.size());
    std::memcpy(data.data(), salt, saltLen);
    std::memcpy(data.data() + saltLen, password.data(), password.size());

    std::vector<uint8_t> hash(MASTER_KEY_SIZE, 0);
    for (int round = 0; round < 200000; ++round) {
        for (size_t i = 0; i < data.size(); ++i) {
            hash[i % MASTER_KEY_SIZE] ^= data[i];
            hash[i % MASTER_KEY_SIZE] = (hash[i % MASTER_KEY_SIZE] << 3) |
                                        (hash[i % MASTER_KEY_SIZE] >> 5);
            hash[i % MASTER_KEY_SIZE] ^= static_cast<uint8_t>(round & 0xFF);
            hash[(i + 13) % MASTER_KEY_SIZE] += hash[i % MASTER_KEY_SIZE];
            hash[(i + 7)  % MASTER_KEY_SIZE] ^= hash[(i + 13) % MASTER_KEY_SIZE];
        }
    }
    std::memcpy(key.data(), hash.data(), MASTER_KEY_SIZE);
    return key;
}

// --- Original Password-based ---

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
        for (int j = 0; j < AES256::BLOCK_SIZE; ++j)
            block[j] = padded[i + j] ^ prev[j];
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

// --- Encrypt with Key File (64-byte master key + parity split) ---

bool FileCrypto::EncryptWithKeyFile(const std::string& inputPath,
                                     const std::string& outputPath,
                                     const std::string& keyFilePath,
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

    // Derive 64-byte master key, use first 32 bytes as AES-256 key
    auto masterKey = DeriveMasterKey(password, salt, 16);
    AES256 aes(masterKey.data());  // uses first 32 bytes

    // Encrypt the file (same as before)
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
        for (int j = 0; j < AES256::BLOCK_SIZE; ++j)
            block[j] = padded[i + j] ^ prev[j];
        aes.EncryptBlock(block, ciphertext.data() + i);
        std::memcpy(prev, ciphertext.data() + i, AES256::BLOCK_SIZE);
    }

    // Auto-append .enc if no recognized extension
    std::string actualOutput = outputPath;
    bool hasExt = false;
    for (const char* ext : {".enc", ".bin", ".crypt", ".encrypted"}) {
        if (actualOutput.size() > strlen(ext) &&
            actualOutput.compare(actualOutput.size() - strlen(ext), strlen(ext), ext) == 0) {
            hasExt = true; break;
        }
    }
    if (!hasExt) actualOutput += ".enc";

    std::ofstream out(actualOutput, std::ios::binary);
    if (!out) { errorMsg = "Cannot create output file"; return false; }
    uint8_t header[48];
    std::memcpy(header, salt, 16);
    std::memcpy(header + 16, iv, AES256::BLOCK_SIZE);
    std::memcpy(header + 32, "CRYPT01", 7);
    out.write(reinterpret_cast<char*>(header), 48);
    out.write(reinterpret_cast<char*>(ciphertext.data()), ciphertext.size());
    out.close();

    // --- Generate Key File ---
    // SHA-256 of the 64-byte master key
    auto keyHash = SHA256::Hash(masterKey.data(), MASTER_KEY_SIZE);

    // Parity split the 64-byte master key into two 32-byte halves
    uint8_t evenHalf[HALF_KEY_SIZE], oddHalf[HALF_KEY_SIZE];
    ParitySplit(masterKey.data(), evenHalf, oddHalf);

    // Key file format:
    // Magic(8) + version(1) + salt(16) + IV(16) + SHA256(32) + even(32) + odd(32) = 137 bytes
    std::ofstream kf(keyFilePath, std::ios::binary);
    if (!kf) { errorMsg = "Cannot create key file"; return false; }
    kf.write("KEYFILE1", 8);
    uint8_t version = 2;  // v2 = 64-byte master key
    kf.write(reinterpret_cast<char*>(&version), 1);
    kf.write(reinterpret_cast<char*>(salt), 16);
    kf.write(reinterpret_cast<char*>(iv), AES256::BLOCK_SIZE);
    kf.write(reinterpret_cast<char*>(keyHash.data()), SHA256::HASH_SIZE);
    kf.write(reinterpret_cast<char*>(evenHalf), HALF_KEY_SIZE);
    kf.write(reinterpret_cast<char*>(oddHalf), HALF_KEY_SIZE);
    kf.close();

    // --- Backup Key File (.bak) - Human-readable hex text ---
    std::string bakPath = keyFilePath + ".bak";
    std::ofstream bkf(bakPath);
    if (bkf) {
        auto toHex = [](const uint8_t* data, int len) -> std::string {
            static const char hex[] = "0123456789abcdef";
            std::string s;
            s.reserve(len * 2);
            for (int i = 0; i < len; ++i) {
                s += hex[data[i] >> 4];
                s += hex[data[i] & 0x0f];
            }
            return s;
        };
        bkf << "# File Crypto Key Backup
";
        bkf << "# Generated by FileCrypto v2 (64-byte master key)
";
        bkf << "# Keep this file safe - it can reconstruct your encryption key

";
        bkf << "master_key=" << toHex(masterKey.data(), MASTER_KEY_SIZE) << "
";
        bkf << "salt=" << toHex(salt, 16) << "
";
        bkf << "iv=" << toHex(iv, AES256::BLOCK_SIZE) << "
";
        bkf << "sha256=" << toHex(keyHash.data(), SHA256::HASH_SIZE) << "
";
        bkf.close();
    }

    return true;
}

// --- Decrypt with Key File (parity rebuild 64-byte key) ---

bool FileCrypto::DecryptWithKeyFile(const std::string& inputPath,
                                     const std::string& outputPath,
                                     const std::string& keyFilePath,
                                     std::string& errorMsg) {
    // Read key file
    std::ifstream kf(keyFilePath, std::ios::binary | std::ios::ate);
    if (!kf) { errorMsg = "Cannot open key file"; return false; }
    std::streamsize kfSize = kf.tellg();
    // v1 = 105 bytes, v2 = 137 bytes
    if (kfSize < 105) { errorMsg = "Invalid key file (too small)"; return false; }

    kf.seekg(0, std::ios::beg);
    char magic[9] = {};
    kf.read(magic, 8);
    if (std::memcmp(magic, "KEYFILE1", 8) != 0) {
        errorMsg = "Not a valid key file"; return false;
    }

    uint8_t version;
    kf.read(reinterpret_cast<char*>(&version), 1);
    if (version != 1 && version != 2) {
        errorMsg = "Unsupported key file version"; return false;
    }

    uint8_t salt[16], iv[AES256::BLOCK_SIZE];
    kf.read(reinterpret_cast<char*>(salt), 16);
    kf.read(reinterpret_cast<char*>(iv), AES256::BLOCK_SIZE);

    std::vector<uint8_t> storedHash(SHA256::HASH_SIZE);
    kf.read(reinterpret_cast<char*>(storedHash.data()), SHA256::HASH_SIZE);

    int halfSize = (version == 2) ? HALF_KEY_SIZE : 16;
    int masterSize = (version == 2) ? MASTER_KEY_SIZE : 32;

    std::vector<uint8_t> evenHalf(halfSize), oddHalf(halfSize);
    kf.read(reinterpret_cast<char*>(evenHalf.data()), halfSize);
    kf.read(reinterpret_cast<char*>(oddHalf.data()), halfSize);
    kf.close();

    // Parity rebuild
    std::vector<uint8_t> rebuiltKey(masterSize);
    if (version == 2) {
        // 64-byte rebuild
        for (int i = 0; i < halfSize; ++i) {
            rebuiltKey[2 * i]     = evenHalf[i];
            rebuiltKey[2 * i + 1] = oddHalf[i];
        }
    } else {
        // legacy 32-byte rebuild
        for (int i = 0; i < halfSize; ++i) {
            rebuiltKey[2 * i]     = evenHalf[i];
            rebuiltKey[2 * i + 1] = oddHalf[i];
        }
    }

    // Verify SHA-256
    auto computedHash = SHA256::Hash(rebuiltKey.data(), masterSize);
    if (std::memcmp(computedHash.data(), storedHash.data(), SHA256::HASH_SIZE) != 0) {
        errorMsg = "Key file verification failed (SHA-256 mismatch)";
        return false;
    }

    // Read encrypted file
    std::ifstream in(inputPath, std::ios::binary | std::ios::ate);
    if (!in) { errorMsg = "Cannot open encrypted file"; return false; }
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

    size_t cipherLen = static_cast<size_t>(fileSize - 48);
    std::vector<uint8_t> ciphertext(cipherLen);
    if (!in.read(reinterpret_cast<char*>(ciphertext.data()), cipherLen)) {
        errorMsg = "Cannot read ciphertext"; return false;
    }
    in.close();

    // Use first 32 bytes of rebuilt key as AES-256 key
    AES256 aes(rebuiltKey.data());

    std::vector<uint8_t> plaintext(cipherLen);
    uint8_t prevBlock[AES256::BLOCK_SIZE];
    std::memcpy(prevBlock, iv, AES256::BLOCK_SIZE);
    for (size_t i = 0; i < cipherLen; i += AES256::BLOCK_SIZE) {
        uint8_t decrypted[AES256::BLOCK_SIZE];
        aes.DecryptBlock(ciphertext.data() + i, decrypted);
        for (int j = 0; j < AES256::BLOCK_SIZE; ++j)
            plaintext[i + j] = decrypted[j] ^ prevBlock[j];
        std::memcpy(prevBlock, ciphertext.data() + i, AES256::BLOCK_SIZE);
    }

    uint8_t padVal2 = plaintext.back();
    if (padVal2 == 0 || padVal2 > AES256::BLOCK_SIZE) {
        errorMsg = "Wrong key or corrupted file"; return false;
    }
    for (int i = 0; i < padVal2; ++i)
        if (plaintext[plaintext.size() - 1 - i] != padVal2) {
            errorMsg = "Wrong key or corrupted file"; return false;
        }
    size_t origLen = plaintext.size() - padVal2;

    std::ofstream out(outputPath, std::ios::binary);
    if (!out) { errorMsg = "Cannot create output file"; return false; }
    out.write(reinterpret_cast<char*>(plaintext.data()), origLen);
    out.close();

    return true;
}
