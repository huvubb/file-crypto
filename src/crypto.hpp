#ifndef CRYPTO_HPP
#define CRYPTO_HPP

#include <string>
#include <cstdint>

class FileCrypto {
public:
    static bool EncryptFile(const std::string& inputPath,
                            const std::string& outputPath,
                            const std::string& password,
                            std::string& errorMsg);
    static bool DecryptFile(const std::string& inputPath,
                            const std::string& outputPath,
                            const std::string& password,
                            std::string& errorMsg);
    static bool GenerateKeyEncrypt(const std::string& inputPath,
                                   const std::string& outputPath,
                                   std::string& apiKeyOut,
                                   std::string& errorMsg);
    static bool KeyDecrypt(const std::string& inputPath,
                           const std::string& outputPath,
                           const std::string& apiKey,
                           std::string& errorMsg);
};

#endif
