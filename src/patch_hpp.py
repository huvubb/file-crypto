path = r"C:\Users\邓涛\Documents\New project 3\file-crypto\src\crypto.hpp"
with open(path, "r", encoding="utf-8") as f:
    content = f.read()

# Add progress callback type and update signatures
old = '''#ifndef CRYPTO_HPP
#define CRYPTO_HPP

#include <string>
#include <cstdint>

class FileCrypto {'''
new = '''#ifndef CRYPTO_HPP
#define CRYPTO_HPP

#include <string>
#include <cstdint>
#include <functional>

using ProgressCb = std::function<void(int pct)>;

class FileCrypto {'''
content = content.replace(old, new)

# Update GenerateKeyEncrypt to include progress callback
old_sig = '''    static bool GenerateKeyEncrypt(const std::string& inputPath,
                                   const std::string& outputPath,
                                   std::string& apiKeyOut,
                                   std::string& errorMsg);'''
new_sig = '''    static bool GenerateKeyEncrypt(const std::string& inputPath,
                                   const std::string& outputPath,
                                   std::string& apiKeyOut,
                                   std::string& errorMsg,
                                   ProgressCb onProgress = nullptr);'''
content = content.replace(old_sig, new_sig)

# Update EncryptFile similarly
old_enc = '''    static bool EncryptFile(const std::string& inputPath,
                            const std::string& outputPath,
                            const std::string& password,
                            std::string& errorMsg);'''
new_enc = '''    static bool EncryptFile(const std::string& inputPath,
                            const std::string& outputPath,
                            const std::string& password,
                            std::string& errorMsg,
                            ProgressCb onProgress = nullptr);'''
content = content.replace(old_enc, new_enc)

with open(path, "w", encoding="utf-8") as f:
    f.write(content)
print("Added progress callback to crypto.hpp")
