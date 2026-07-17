path = r"C:\Users\邓涛\Documents\New project 3\file-crypto\src\aes256.hpp"
with open(path, "r", encoding="utf-8") as f:
    content = f.read()

# Remove SimpleHash (replaced by SHA-256 KDF)
old_simple = '''    static void SimpleHash(const std::vector<uint8_t>& data, std::vector<uint8_t>& hash, int round) {
        for (size_t i = 0; i < data.size(); ++i) {
            hash[i % 32] ^= data[i];
            hash[i % 32] = (hash[i % 32] << 3) | (hash[i % 32] >> 5);
            hash[i % 32] ^= static_cast<uint8_t>(round & 0xFF);
            hash[(i + 7) % 32] += hash[i % 32];
        }
    }

'''
content = content.replace(old_simple, '')
with open(path, "w", encoding="utf-8") as f:
    f.write(content)
print("Removed old SimpleHash")
