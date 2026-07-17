path = r"C:\Users\邓涛\Documents\New project 3\file-crypto\src\crypto.cpp"
with open(path, "r", encoding="utf-8") as f:
    content = f.read()

# 1. Auto-append .enc extension
old1 = '    std::ofstream out(outputPath, std::ios::binary);\n    if (!out) { errorMsg = "Cannot create output file"; return false; }\n    uint8_t header[48];\n    std::memcpy(header, salt, 16);\n    std::memcpy(header + 16, iv, AES256::BLOCK_SIZE);\n    std::memcpy(header + 32, "CRYPT01", 7);\n    out.write(reinterpret_cast<char*>(header), 48);\n    out.write(reinterpret_cast<char*>(ciphertext.data()), ciphertext.size());\n    out.close();\n\n    // --- Generate Key File ---'

new1 = '''    // Auto-append .enc if no recognized extension
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

    // --- Generate Key File ---'''

if old1 in content:
    content = content.replace(old1, new1)
    print("Patch 1 applied: .enc auto-append")
else:
    print("WARNING: Patch 1 not found!")

# 2. Add backup key file
old2 = '    kf.write(reinterpret_cast<char*>(oddHalf), HALF_KEY_SIZE);\n    kf.close();\n\n    return true;\n}'

new2 = '''    kf.write(reinterpret_cast<char*>(oddHalf), HALF_KEY_SIZE);
    kf.close();

    // --- Backup Key File (.bak) ---
    std::string bakPath = keyFilePath + ".bak";
    std::ofstream bkf(bakPath, std::ios::binary);
    if (bkf) {
        bkf.write("KEYFILE1", 8);
        bkf.write(reinterpret_cast<char*>(&version), 1);
        bkf.write(reinterpret_cast<char*>(salt), 16);
        bkf.write(reinterpret_cast<char*>(iv), AES256::BLOCK_SIZE);
        bkf.write(reinterpret_cast<char*>(keyHash.data()), SHA256::HASH_SIZE);
        bkf.write(reinterpret_cast<char*>(evenHalf), HALF_KEY_SIZE);
        bkf.write(reinterpret_cast<char*>(oddHalf), HALF_KEY_SIZE);
        bkf.close();
    }

    return true;
}'''

if old2 in content:
    content = content.replace(old2, new2)
    print("Patch 2 applied: .bak backup key")
else:
    print("WARNING: Patch 2 not found!")

with open(path, "w", encoding="utf-8") as f:
    f.write(content)
print("Done")
