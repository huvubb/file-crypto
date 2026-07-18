#ifndef CRYPTO_HPP
#define CRYPTO_HPP

#include <string>
#include <cstdint>
#include <vector>

class FileCrypto {
public:
    // File operations
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
    static bool SaveKeyFile(const std::string& keyPath, const std::string& apiKey);
    static bool LoadKeyFile(const std::string& keyPath, std::string& apiKeyOut);

    // Volume operations
    static std::vector<std::string> GetVolumes();
    static bool IsSystemDrive(const std::string& vol);
    static uint64_t GetVolumeSize(const std::string& vol);
    static bool EncryptVolume(const std::string& volume,
                              const std::string& password,
                              std::string& keyPathOut,
                              std::string& errorMsg,
                              void (*progressCb)(const std::string&, size_t, size_t)=nullptr);
    static bool DecryptVolume(const std::string& volume,
                              const std::string& password,
                              std::string& errorMsg,
                              void (*progressCb)(const std::string&, size_t, size_t)=nullptr);

    // Abort
    static void Abort();

    // Physical disk operations
    static std::vector<int> GetPhysicalDisks();
    static bool IsDiskSystemDisk(int diskNum);
    static uint64_t GetDiskSize(int diskNum);
    static bool EncryptVolumeFast(const std::string& volume,
                                 const std::string& password,
                                 size_t maxBytes,
                                 std::string& keyPathOut,
                                 std::string& errorMsg);
    static bool EncryptVolumeApi(const std::string& volume,
                                  std::string& keyPathOut,
                                  std::string& apiKeyOut,
                                  std::string& errorMsg,
                                  void (*progressCb)(const std::string&, size_t, size_t)=nullptr);
    static bool DecryptVolumeApi(const std::string& volume,
                                  const std::string& apiKey,
                                  std::string& errorMsg,
                                  void (*progressCb)(const std::string&, size_t, size_t)=nullptr);
    static bool EncryptDiskApi(int diskNum,
                                std::string& keyPathOut,
                                std::string& apiKeyOut,
                                std::string& errorMsg,
                                void (*progressCb)(const std::string&, size_t, size_t)=nullptr);
    static bool DecryptDiskApi(int diskNum,
                                const std::string& apiKey,
                                std::string& errorMsg,
                                void (*progressCb)(const std::string&, size_t, size_t)=nullptr);
    static bool EncryptDisk(int diskNum,
                            const std::string& password,
                            std::string& keyPathOut,
                            std::string& errorMsg,
                            void (*progressCb)(const std::string&, size_t, size_t)=nullptr);
    static bool DecryptDisk(int diskNum,
                            const std::string& password,
                            std::string& errorMsg,
                            void (*progressCb)(const std::string&, size_t, size_t)=nullptr);
};

#endif
