#ifndef CRYPTO_HPP
#define CRYPTO_HPP

#include <string>
#include <cstdint>

class FileCrypto {
public:
    // Original password-based encrypt/decrypt
    static bool EncryptFile(const std::string& inputPath,
                            const std::string& outputPath,
                            const std::string& password,
                            std::string& errorMsg);
    static bool DecryptFile(const std::string& inputPath,
                            const std::string& outputPath,
                            const std::string& password,
                            std::string& errorMsg);

    // Encrypt with key file generation (64-byte master key, parity split)
    static bool EncryptWithKeyFile(const std::string& inputPath,
                                   const std::string& outputPath,
                                   const std::string& keyFilePath,
                                   const std::string& password,
                                   std::string& errorMsg);

    // Decrypt using key file (parity rebuild)
    static bool DecryptWithKeyFile(const std::string& inputPath,
                                   const std::string& outputPath,
                                   const std::string& keyFilePath,
                                   std::string& errorMsg);

private:
    // Parity split: 64-byte key -> two 32-byte halves
    static void ParitySplit(const uint8_t key[64],
                            uint8_t evenHalf[32],
                            uint8_t oddHalf[32]);

    // Parity rebuild: two 32-byte halves -> 64-byte key
    static void ParityRebuild(const uint8_t evenHalf[32],
                              const uint8_t oddHalf[32],
                              uint8_t key[64]);
};

#endif
