path = r"C:\Users\邓涛\Documents\New project 3\file-crypto\src\aes256.hpp"
with open(path, "r", encoding="utf-8") as f:
    content = f.read()

# Update DeriveKey256 to use SHA-256 with salt for proper KDF
old_kdf = '''    static std::vector<uint8_t> DeriveKey256(const std::string& password, const uint8_t* salt, size_t saltLen) {
        std::vector<uint8_t> key(KEY_SIZE);
        std::vector<uint8_t> data(saltLen + password.size());
        std::memcpy(data.data(), salt, saltLen);
        std::memcpy(data.data() + saltLen, password.data(), password.size());

        std::vector<uint8_t> hash(KEY_SIZE, 0);
        for (int round = 0; round < 100000; ++round) {
            SimpleHash(data, hash, round);
        }

        std::memcpy(key.data(), hash.data(), KEY_SIZE);
        return key;
    }'''

new_kdf = '''    // PBKDF2-style key derivation: SHA-256(salt + password) iterated 200k rounds
    static std::vector<uint8_t> DeriveKey256(const std::string& password, const uint8_t* salt, size_t saltLen) {
        std::vector<uint8_t> key(KEY_SIZE);
        std::vector<uint8_t> data(saltLen + password.size());
        std::memcpy(data.data(), salt, saltLen);
        std::memcpy(data.data() + saltLen, password.data(), password.size());

        // Initial SHA-256
        SHA256 sha;
        sha.Update(data.data(), data.size());
        std::vector<uint8_t> hash(SHA256::HASH_SIZE);
        sha.Finalize(hash.data());

        // Iterate 200k rounds (slow down brute force)
        for (int round = 0; round < 200000; ++round) {
            sha.Reset();
            sha.Update(hash.data(), SHA256::HASH_SIZE);
            sha.Update(reinterpret_cast<const uint8_t*>(&round), sizeof(round));
            sha.Finalize(hash.data());
        }

        std::memcpy(key.data(), hash.data(), KEY_SIZE);
        return key;
    }'''

content = content.replace(old_kdf, new_kdf)

# Add SHA256 include
if '#include "sha256.hpp"' not in content:
    content = content.replace('#include <vector>', '#include <vector>\n#include "sha256.hpp"')
    content = content.replace('#include <stdexcept>', '')

with open(path, "w", encoding="utf-8") as f:
    f.write(content)
print("SHA-256 salted KDF added (200k rounds)")
