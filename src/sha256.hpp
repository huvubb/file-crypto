#ifndef SHA256_HPP
#define SHA256_HPP

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

class SHA256 {
public:
    static constexpr int HASH_SIZE = 32;
    static constexpr int BLOCK_SIZE = 64;

    SHA256() { Reset(); }

    void Reset() {
        h_[0] = 0x6a09e667; h_[1] = 0xbb67ae85;
        h_[2] = 0x3c6ef372; h_[3] = 0xa54ff53a;
        h_[4] = 0x510e527f; h_[5] = 0x9b05688c;
        h_[6] = 0x1f83d9ab; h_[7] = 0x5be0cd19;
        bitLen_ = 0;
        bufLen_ = 0;
    }

    void Update(const uint8_t* data, size_t len) {
        for (size_t i = 0; i < len; ++i) {
            buf_[bufLen_++] = data[i];
            if (bufLen_ == BLOCK_SIZE) {
                Transform(buf_);
                bitLen_ += 512;
                bufLen_ = 0;
            }
        }
    }

    void Update(const std::string& s) {
        Update(reinterpret_cast<const uint8_t*>(s.data()), s.size());
    }

    void Finalize(uint8_t hash[HASH_SIZE]) {
        uint64_t totalBits = bitLen_ + bufLen_ * 8;

        buf_[bufLen_++] = 0x80;
        if (bufLen_ > 56) {
            while (bufLen_ < BLOCK_SIZE) buf_[bufLen_++] = 0;
            Transform(buf_);
            bufLen_ = 0;
        }
        while (bufLen_ < 56) buf_[bufLen_++] = 0;

        for (int i = 7; i >= 0; --i)
            buf_[56 + i] = static_cast<uint8_t>(totalBits >> (56 - i * 8));

        Transform(buf_);

        for (int i = 0; i < 8; ++i) {
            hash[i * 4]     = static_cast<uint8_t>(h_[i] >> 24);
            hash[i * 4 + 1] = static_cast<uint8_t>(h_[i] >> 16);
            hash[i * 4 + 2] = static_cast<uint8_t>(h_[i] >> 8);
            hash[i * 4 + 3] = static_cast<uint8_t>(h_[i]);
        }
    }

    static std::vector<uint8_t> Hash(const uint8_t* data, size_t len) {
        SHA256 sha;
        sha.Update(data, len);
        std::vector<uint8_t> result(HASH_SIZE);
        sha.Finalize(result.data());
        return result;
    }

    static std::string HexDigest(const uint8_t* data, size_t len) {
        auto hash = Hash(data, len);
        std::string hex;
        hex.reserve(HASH_SIZE * 2);
        for (uint8_t b : hash) {
            hex += "0123456789abcdef"[b >> 4];
            hex += "0123456789abcdef"[b & 0x0f];
        }
        return hex;
    }

private:
    uint32_t h_[8];
    uint8_t buf_[BLOCK_SIZE];
    uint64_t bitLen_;
    size_t bufLen_;

    static const uint32_t k_[64];

    static inline uint32_t ROTR(uint32_t x, uint32_t n) {
        return (x >> n) | (x << (32 - n));
    }

    void Transform(const uint8_t block[BLOCK_SIZE]) {
        uint32_t w[64];
        for (int i = 0; i < 16; ++i)
            w[i] = (static_cast<uint32_t>(block[i * 4]) << 24) |
                   (static_cast<uint32_t>(block[i * 4 + 1]) << 16) |
                   (static_cast<uint32_t>(block[i * 4 + 2]) << 8) |
                   static_cast<uint32_t>(block[i * 4 + 3]);

        for (int i = 16; i < 64; ++i) {
            uint32_t s0 = ROTR(w[i - 15], 7) ^ ROTR(w[i - 15], 18) ^ (w[i - 15] >> 3);
            uint32_t s1 = ROTR(w[i - 2], 17) ^ ROTR(w[i - 2], 19) ^ (w[i - 2] >> 10);
            w[i] = w[i - 16] + s0 + w[i - 7] + s1;
        }

        uint32_t a = h_[0], b = h_[1], c = h_[2], d = h_[3];
        uint32_t e = h_[4], f = h_[5], g = h_[6], h = h_[7];

        for (int i = 0; i < 64; ++i) {
            uint32_t S1 = ROTR(e, 6) ^ ROTR(e, 11) ^ ROTR(e, 25);
            uint32_t ch = (e & f) ^ ((~e) & g);
            uint32_t temp1 = h + S1 + ch + k_[i] + w[i];
            uint32_t S0 = ROTR(a, 2) ^ ROTR(a, 13) ^ ROTR(a, 22);
            uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
            uint32_t temp2 = S0 + maj;

            h = g; g = f; f = e; e = d + temp1;
            d = c; c = b; b = a; a = temp1 + temp2;
        }

        h_[0] += a; h_[1] += b; h_[2] += c; h_[3] += d;
        h_[4] += e; h_[5] += f; h_[6] += g; h_[7] += h;
    }
};

#endif
