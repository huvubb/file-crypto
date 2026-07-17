#include "crypto.hpp"
#include "aes256.hpp"
#include <fstream>
#include <random>
#include <chrono>

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

    uint8_t salt[16];
    uint8_t iv[AES256::BLOCK_SIZE];
    {
        std::mt19937 rng(static_cast<unsigned>(
            std::chrono::steady_clock::now().time_since_epoch().count()));
        std::uniform_int_distribution<int> dist(0, 255);
        for (int i = 0; i < 16; ++i) salt[i] = static_cast<uint8_t>(dist(rng));
        for (int i = 0; i < AES256::BLOCK_SIZE; ++i)
            iv[i] = static_cast<uint8_t>(dist(rng));
    }

    auto key = AES256::DeriveKey256(password, salt, 16);
    AES256 aes(key.data());

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
        errorMsg = "Not a valid encrypted file";
        return false;
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

    auto key = AES256::DeriveKey256(password, salt, 16);
    AES256 aes(key.data());

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
        errorMsg = "Wrong password or corrupted file";
        return false;
    }
    for (int i = 0; i < padVal; ++i) {
        if (plaintext[plaintext.size() - 1 - i] != padVal) {
            errorMsg = "Wrong password or corrupted file";
            return false;
        }
    }
    size_t origLen = plaintext.size() - padVal;

    std::ofstream out(outputPath, std::ios::binary);
    if (!out) { errorMsg = "Cannot create output file"; return false; }
    out.write(reinterpret_cast<char*>(plaintext.data()), origLen);
    out.close();

    return true;
}
