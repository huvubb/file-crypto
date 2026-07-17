#ifndef AES256_HPP
#define AES256_HPP

#include <cstdint>
#include <cstring>
#include <vector>
#include "sha256.hpp"


void EnsureMulTables();

class AES256 {
public:
    static constexpr int BLOCK_SIZE = 16;
    static constexpr int KEY_SIZE = 32;

    AES256(const uint8_t* key) {
        EnsureMulTables();
        KeyExpansion(key, roundKeys_);
    }

    void EncryptBlock(const uint8_t in[BLOCK_SIZE], uint8_t out[BLOCK_SIZE]) const {
        uint8_t state[BLOCK_SIZE];
        std::memcpy(state, in, BLOCK_SIZE);
        AddRoundKey(state, roundKeys_[0]);

        for (int round = 1; round < 14; ++round) {
            SubBytes(state);
            ShiftRows(state);
            MixColumns(state);
            AddRoundKey(state, roundKeys_[round]);
        }

        SubBytes(state);
        ShiftRows(state);
        AddRoundKey(state, roundKeys_[14]);
        std::memcpy(out, state, BLOCK_SIZE);
    }

    void DecryptBlock(const uint8_t in[BLOCK_SIZE], uint8_t out[BLOCK_SIZE]) const {
        uint8_t state[BLOCK_SIZE];
        std::memcpy(state, in, BLOCK_SIZE);
        AddRoundKey(state, roundKeys_[14]);

        for (int round = 13; round >= 1; --round) {
            InvShiftRows(state);
            InvSubBytes(state);
            AddRoundKey(state, roundKeys_[round]);
            InvMixColumns(state);
        }

        InvShiftRows(state);
        InvSubBytes(state);
        AddRoundKey(state, roundKeys_[0]);
        std::memcpy(out, state, BLOCK_SIZE);
    }

    // PBKDF2-style key derivation: SHA-256(salt + password) iterated 200k rounds
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
    }

    static const uint8_t mul2_[256];
    static const uint8_t mul3_[256];
    static const uint8_t mul9_[256];
    static const uint8_t mul11_[256];
    static const uint8_t mul13_[256];
    static const uint8_t mul14_[256];

private:
    uint8_t roundKeys_[15][BLOCK_SIZE];

    static const uint8_t sbox_[256];
    static const uint8_t inv_sbox_[256];
    static const uint8_t rcon_[15];

    void SubBytes(uint8_t state[BLOCK_SIZE]) const {
        for (int i = 0; i < BLOCK_SIZE; ++i) state[i] = sbox_[state[i]];
    }

    void InvSubBytes(uint8_t state[BLOCK_SIZE]) const {
        for (int i = 0; i < BLOCK_SIZE; ++i) state[i] = inv_sbox_[state[i]];
    }

    void ShiftRows(uint8_t state[BLOCK_SIZE]) const {
        uint8_t temp[BLOCK_SIZE];
        temp[0] = state[0];  temp[1] = state[5];  temp[2] = state[10]; temp[3] = state[15];
        temp[4] = state[4];  temp[5] = state[9];  temp[6] = state[14]; temp[7] = state[3];
        temp[8] = state[8];  temp[9] = state[13]; temp[10] = state[2]; temp[11] = state[7];
        temp[12] = state[12]; temp[13] = state[1]; temp[14] = state[6]; temp[15] = state[11];
        std::memcpy(state, temp, BLOCK_SIZE);
    }

    void InvShiftRows(uint8_t state[BLOCK_SIZE]) const {
        uint8_t temp[BLOCK_SIZE];
        temp[0] = state[0];  temp[1] = state[13]; temp[2] = state[10]; temp[3] = state[7];
        temp[4] = state[4];  temp[5] = state[1];  temp[6] = state[14]; temp[7] = state[11];
        temp[8] = state[8];  temp[9] = state[5];  temp[10] = state[2]; temp[11] = state[15];
        temp[12] = state[12]; temp[13] = state[9]; temp[14] = state[6]; temp[15] = state[3];
        std::memcpy(state, temp, BLOCK_SIZE);
    }

    void MixColumns(uint8_t state[BLOCK_SIZE]) const {
        for (int i = 0; i < 4; ++i) {
            int c = i * 4;
            uint8_t a0 = state[c], a1 = state[c+1], a2 = state[c+2], a3 = state[c+3];
            state[c]   = mul2_[a0] ^ mul3_[a1] ^ a2 ^ a3;
            state[c+1] = a0 ^ mul2_[a1] ^ mul3_[a2] ^ a3;
            state[c+2] = a0 ^ a1 ^ mul2_[a2] ^ mul3_[a3];
            state[c+3] = mul3_[a0] ^ a1 ^ a2 ^ mul2_[a3];
        }
    }

    void InvMixColumns(uint8_t state[BLOCK_SIZE]) const {
        for (int i = 0; i < 4; ++i) {
            int c = i * 4;
            uint8_t a0 = state[c], a1 = state[c+1], a2 = state[c+2], a3 = state[c+3];
            state[c]   = mul14_[a0] ^ mul11_[a1] ^ mul13_[a2] ^ mul9_[a3];
            state[c+1] = mul9_[a0]  ^ mul14_[a1] ^ mul11_[a2] ^ mul13_[a3];
            state[c+2] = mul13_[a0] ^ mul9_[a1]  ^ mul14_[a2] ^ mul11_[a3];
            state[c+3] = mul11_[a0] ^ mul13_[a1] ^ mul9_[a2]  ^ mul14_[a3];
        }
    }

    void AddRoundKey(uint8_t state[BLOCK_SIZE], const uint8_t roundKey[BLOCK_SIZE]) const {
        for (int i = 0; i < BLOCK_SIZE; ++i) state[i] ^= roundKey[i];
    }

    static void KeyExpansion(const uint8_t* key, uint8_t roundKeys[15][BLOCK_SIZE]) {
        uint8_t temp[4];
        int nk = 8;
        int nr = 14;
        int totalWords = BLOCK_SIZE * (nr + 1) / 4;
        uint8_t* w = new uint8_t[totalWords * 4];

        for (int i = 0; i < nk * 4; ++i) w[i] = key[i];

        for (int i = nk; i < totalWords; ++i) {
            temp[0] = w[(i - 1) * 4];
            temp[1] = w[(i - 1) * 4 + 1];
            temp[2] = w[(i - 1) * 4 + 2];
            temp[3] = w[(i - 1) * 4 + 3];

            if (i % nk == 0) {
                uint8_t t = temp[0];
                temp[0] = sbox_[temp[1]] ^ rcon_[i / nk];
                temp[1] = sbox_[temp[2]];
                temp[2] = sbox_[temp[3]];
                temp[3] = sbox_[t];
            } else if (nk > 6 && i % nk == 4) {
                temp[0] = sbox_[temp[0]];
                temp[1] = sbox_[temp[1]];
                temp[2] = sbox_[temp[2]];
                temp[3] = sbox_[temp[3]];
            }

            w[i * 4]     = w[(i - nk) * 4]     ^ temp[0];
            w[i * 4 + 1] = w[(i - nk) * 4 + 1] ^ temp[1];
            w[i * 4 + 2] = w[(i - nk) * 4 + 2] ^ temp[2];
            w[i * 4 + 3] = w[(i - nk) * 4 + 3] ^ temp[3];
        }

        for (int round = 0; round <= nr; ++round)
            std::memcpy(roundKeys[round], &w[round * BLOCK_SIZE], BLOCK_SIZE);

        delete[] w;
    }
};

#endif
