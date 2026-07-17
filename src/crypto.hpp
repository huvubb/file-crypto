#ifndef CRYPTO_HPP
#define CRYPTO_HPP

#include <string>
#include <vector>
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
};

#endif
