path = r"C:\Users\邓涛\Documents\New project 3\file-crypto\src\crypto.cpp"
with open(path, "r", encoding="utf-8") as f:
    content = f.read()

# Add Base64 encode/decode before the ParitySplit
b64_code = '''
// --- Base64 encode/decode ---
static const char B64_CHARS[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static std::string Base64Encode(const uint8_t* data, size_t len) {
    std::string result;
    result.reserve(((len + 2) / 3) * 4);
    for (size_t i = 0; i < len; i += 3) {
        uint32_t n = static_cast<uint32_t>(data[i]) << 16;
        if (i + 1 < len) n |= static_cast<uint32_t>(data[i + 1]) << 8;
        if (i + 2 < len) n |= static_cast<uint32_t>(data[i + 2]);
        result += B64_CHARS[(n >> 18) & 0x3F];
        result += B64_CHARS[(n >> 12) & 0x3F];
        result += (i + 1 < len) ? B64_CHARS[(n >> 6) & 0x3F] : '=';
        result += (i + 2 < len) ? B64_CHARS[n & 0x3F] : '=';
    }
    return result;
}

static std::vector<uint8_t> Base64Decode(const std::string& s) {
    auto idx = [](char c) -> int {
        if (c >= 'A' && c <= 'Z') return c - 'A';
        if (c >= 'a' && c <= 'z') return c - 'a' + 26;
        if (c >= '0' && c <= '9') return c - '0' + 52;
        if (c == '+') return 62;
        if (c == '/') return 63;
        return -1;
    };
    std::vector<uint8_t> result;
    result.reserve((s.size() / 4) * 3);
    int vals[4];
    for (size_t i = 0; i + 3 < s.size(); i += 4) {
        vals[0] = idx(s[i]); vals[1] = idx(s[i + 1]);
        vals[2] = idx(s[i + 2]); vals[3] = idx(s[i + 3]);
        int pads = (s[i + 2] == '=') + (s[i + 3] == '=');
        uint32_t n = (static_cast<uint32_t>(vals[0]) << 18) |
                     (static_cast<uint32_t>(vals[1]) << 12);
        if (s[i + 2] != '=') n |= static_cast<uint32_t>(vals[2]) << 6;
        if (s[i + 3] != '=') n |= static_cast<uint32_t>(vals[3]);
        result.push_back(static_cast<uint8_t>(n >> 16));
        if (pads < 2) result.push_back(static_cast<uint8_t>(n >> 8));
        if (pads < 1) result.push_back(static_cast<uint8_t>(n));
    }
    return result;
}

'''

# Insert after the DeriveMasterKey function, before ParitySplit
marker = "// --- Parity Split / Rebuild (64-byte master key) ---"
content = content.replace(marker, b64_code + marker)

with open(path, "w", encoding="utf-8") as f:
    f.write(content)
print("Base64 functions added")
