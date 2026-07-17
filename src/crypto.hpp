#ifndef CRYPTO_HPP
#define CRYPTO_HPP

#include <string>
#include <cstdint>

class FileCrypto {
public:
    // Password-based encrypt/decrypt (legacy)
    static bool EncryptFile(const std::string& inputPath,
                            const std::string& outputPath,
                            const std::string& password,
                            std::string& errorMsg);
    static bool DecryptFile(const std::string& inputPath,
                            const std::string& outputPath,
                            const std::string& password,
                            std::string& errorMsg);

    // API-key mode: generate random 64-byte key, encrypt, return API-key string
    static bool GenerateKeyEncrypt(const std::string& inputPath,
                                   const std::string& outputPath,
                                   std::string& apiKeyOut,
                                   std::string& errorMsg);

    // API-key mode: decrypt using API-key string (no password needed)
    static bool KeyDecrypt(const std::string& inputPath,
                           const std::string& outputPath,
                           const std::string& apiKey,
                           std::string& errorMsg);
};

#endif
