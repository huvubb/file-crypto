path = r"C:\Users\邓涛\Documents\New project 3\file-crypto\src\aes256.cpp"
with open(path, "r", encoding="utf-8") as f:
    content = f.read()

# Add multiply tables
tables = '''

// Pre-computed GF(2^8) multiply tables for MixColumns
namespace {
    static uint8_t gfMul(uint8_t a, uint8_t b) {
        uint8_t r = 0;
        for (int i = 0; i < 8; ++i) {
            if (b & 1) r ^= a;
            uint8_t hi = static_cast<uint8_t>(a & 0x80);
            a <<= 1;
            if (hi) a ^= 0x1b;
            b >>= 1;
        }
        return r;
    }

    static uint8_t gMul2[256], gMul3[256], gMul9[256];
    static uint8_t gMul11[256], gMul13[256], gMul14[256];
    static bool gInited = false;

    struct MulInit {
        MulInit() {
            for (int i = 0; i < 256; ++i) {
                gMul2[i]  = gfMul(static_cast<uint8_t>(i), 2);
                gMul3[i]  = gfMul(static_cast<uint8_t>(i), 3);
                gMul9[i]  = gfMul(static_cast<uint8_t>(i), 9);
                gMul11[i] = gfMul(static_cast<uint8_t>(i), 11);
                gMul13[i] = gfMul(static_cast<uint8_t>(i), 13);
                gMul14[i] = gfMul(static_cast<uint8_t>(i), 14);
            }
            gInited = true;
        }
    };
    static MulInit __mulInit;
}

const uint8_t AES256::mul2_[256]  = {0};
const uint8_t AES256::mul3_[256]  = {0};
const uint8_t AES256::mul9_[256]  = {0};
const uint8_t AES256::mul11_[256] = {0};
const uint8_t AES256::mul13_[256] = {0};
const uint8_t AES256::mul14_[256] = {0};

// Lazy init: copy from static tables on first use
static void EnsureMulTables() {
    if (!gInited) return;
    static bool copied = false;
    if (!copied) {
        memcpy(const_cast<uint8_t*>(AES256::mul2_),  gMul2,  256);
        memcpy(const_cast<uint8_t*>(AES256::mul3_),  gMul3,  256);
        memcpy(const_cast<uint8_t*>(AES256::mul9_),  gMul9,  256);
        memcpy(const_cast<uint8_t*>(AES256::mul11_), gMul11, 256);
        memcpy(const_cast<uint8_t*>(AES256::mul13_), gMul13, 256);
        memcpy(const_cast<uint8_t*>(AES256::mul14_), gMul14, 256);
        copied = true;
    }
}
'''

# Insert after the rcon table, before the end
content = content.replace('''    0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d
};''', '''    0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d
};
''' + tables)

with open(path, "w", encoding="utf-8") as f:
    f.write(content)
print("Pre-computed GF multiply tables added")
