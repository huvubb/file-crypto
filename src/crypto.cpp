#include "crypto.hpp"
#define _WIN32_WINNT 0x0601

#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")

#ifdef EncryptFile
#undef EncryptFile
#endif
#ifdef DecryptFile
#undef DecryptFile
#endif

#include <iostream>
#include <vector>
#include <cstring>
#include <cstdarg>
#include <ctime>
#include <fstream>
#include <filesystem>

// --- Constants ---
static constexpr const char* KEY_PREFIX = "fck-v2-";
static constexpr int MASTER_KEY_SIZE = 64;
static constexpr int AES_BLOCK = 16;
static constexpr int SHA256_LEN = 32;
static constexpr size_t HDR_PW = 80;   // salt+iv+sha256+reserved+magic
static constexpr size_t HDR_KEY = 64;  // iv+sha256+reserved+magic
static constexpr size_t HDR_OLD = 48;

// --- RAII secure wipe ---
template<typename T>
static void SecureWipe(T& container) {
    if (!container.empty()) {
        SecureZeroMemory(container.data(), container.size());
    }
}

static void SecureWipeStr(std::string& s) {
    if (!s.empty()) {
        SecureZeroMemory(&s[0], s.size());
    }
}

// --- Base64 (unchanged) ---
static const char B64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static std::string Base64Encode(const uint8_t* d, size_t n) {
    std::string r; r.reserve(((n+2)/3)*4);
    for(size_t i=0;i<n;i+=3){
        uint32_t v=static_cast<uint32_t>(d[i])<<16;
        if(i+1<n)v|=static_cast<uint32_t>(d[i+1])<<8;
        if(i+2<n)v|=static_cast<uint32_t>(d[i+2]);
        r+=B64[(v>>18)&0x3F];r+=B64[(v>>12)&0x3F];
        r+=(i+1<n)?B64[(v>>6)&0x3F]:'=';r+=(i+2<n)?B64[v&0x3F]:'=';
    }
    return r;
}
static std::vector<uint8_t> Base64Decode(const std::string& s){
    auto idx=[](char c)->int{if(c>='A'&&c<='Z')return c-'A';if(c>='a'&&c<='z')return c-'a'+26;if(c>='0'&&c<='9')return c-'0'+52;if(c=='+')return 62;if(c=='/')return 63;return-1;};
    std::vector<uint8_t> r;r.reserve((s.size()/4)*3);
    for(size_t i=0;i+3<s.size();i+=4){
        int v0=idx(s[i]),v1=idx(s[i+1]),v2=idx(s[i+2]),v3=idx(s[i+3]);
        int p=(s[i+2]=='=')+(s[i+3]=='=');
        uint32_t n=(static_cast<uint32_t>(v0)<<18)|(static_cast<uint32_t>(v1)<<12);
        if(s[i+2]!='=')n|=static_cast<uint32_t>(v2)<<6;
        if(s[i+3]!='=')n|=static_cast<uint32_t>(v3);
        r.push_back(static_cast<uint8_t>(n>>16));if(p<2)r.push_back(static_cast<uint8_t>(n>>8));if(p<1)r.push_back(static_cast<uint8_t>(n));
    }
    return r;
}

// --- Debug log ---
static FILE* g_dbg = NULL;
static void DbgLog(const char* fmt, ...) {
    if (!g_dbg) {
        g_dbg = fopen("D:\\crypto_debug.log", "a");
        if (g_dbg) {
            time_t t = time(NULL);
            fprintf(g_dbg, "\n=== RUN %lld ===\n", (long long)t);
            fflush(g_dbg);
        }
    }
    if (!g_dbg) return;
    va_list args;
    va_start(args, fmt);
    vfprintf(g_dbg, fmt, args);
    va_end(args);
    fflush(g_dbg);
}

// --- BCrypt helpers ---
static void RandBytes(uint8_t* buf, size_t len) { BCryptGenRandom(NULL, buf, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG); }

static BCRYPT_ALG_HANDLE OpenAes() {
    BCRYPT_ALG_HANDLE h = NULL;
    NTSTATUS s = BCryptOpenAlgorithmProvider(&h, BCRYPT_AES_ALGORITHM, NULL, 0);
    DbgLog("[AES] BCryptOpenAlgorithmProvider: s=0x%08X h=%p\n", s, h);
    if (!BCRYPT_SUCCESS(s) || !h) return NULL;
    s = BCryptSetProperty(h, BCRYPT_CHAINING_MODE, (PUCHAR)BCRYPT_CHAIN_MODE_CBC, sizeof(BCRYPT_CHAIN_MODE_CBC), 0);
    DbgLog("[AES] BCryptSetProperty(CBC): s=0x%08X\n", s);
    if (!BCRYPT_SUCCESS(s)) { BCryptCloseAlgorithmProvider(h, 0); return NULL; }
    return h;
}

static BCRYPT_KEY_HANDLE CreateAesKey(BCRYPT_ALG_HANDLE alg, const uint8_t* key, size_t len) {
    if (!alg || !key || len == 0) { DbgLog("[KEY] FAIL: invalid params\n"); return NULL; }
    BCRYPT_KEY_HANDLE hk = NULL;
    NTSTATUS s = BCryptGenerateSymmetricKey(alg, &hk, NULL, 0, (PUCHAR)key, (ULONG)len, 0);
    DbgLog("[KEY] BCryptGenerateSymmetricKey(len=%zu): s=0x%08X hk=%p\n", len, s, hk);
    if (!BCRYPT_SUCCESS(s)) { DbgLog("[KEY] FAIL: BCryptGenerateSymmetricKey failed\n"); return NULL; }
    if (!hk) { DbgLog("[KEY] FAIL: hk is NULL despite success\n"); return NULL; }
    return hk;
}

// --- SHA-256 ---
static void ComputeSha256(const uint8_t* data, size_t len, uint8_t* hash) {
    BCRYPT_ALG_HANDLE hAlg;
    BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, NULL, 0);
    BCRYPT_HASH_HANDLE hHash;
    BCryptCreateHash(hAlg, &hHash, NULL, 0, NULL, 0, 0);
    BCryptHashData(hHash, (PUCHAR)data, (ULONG)len, 0);
    BCryptFinishHash(hHash, hash, SHA256_LEN, 0);
    BCryptDestroyHash(hHash);
    BCryptCloseAlgorithmProvider(hAlg, 0);
}

// --- AES-256-CBC encrypt (IV-safe, error-checked, malloc) ---
static bool AesCbcEncrypt(const uint8_t* key, const uint8_t* iv, const uint8_t* in, size_t inLen, std::vector<uint8_t>& out) {
    if (!key || !iv || !in || inLen == 0 || (inLen % AES_BLOCK) != 0) return false;
    BCRYPT_ALG_HANDLE alg = OpenAes();
    if (!alg) return false;
    BCRYPT_KEY_HANDLE hk = CreateAesKey(alg, key, 32);
    if (!hk) { BCryptCloseAlgorithmProvider(alg, 0); return false; }
    uint8_t ivCopy[AES_BLOCK]; memcpy(ivCopy, iv, AES_BLOCK);
    ULONG outLen;
    NTSTATUS s = BCryptEncrypt(hk, (PUCHAR)in, (ULONG)inLen, NULL, (PUCHAR)ivCopy, AES_BLOCK, NULL, 0, &outLen, 0);
    if (!BCRYPT_SUCCESS(s)) { BCryptDestroyKey(hk); BCryptCloseAlgorithmProvider(alg, 0); return false; }
    uint8_t* outBuf = (uint8_t*)malloc(outLen);
    if (!outBuf) { BCryptDestroyKey(hk); BCryptCloseAlgorithmProvider(alg, 0); return false; }
    memcpy(ivCopy, iv, AES_BLOCK);
    s = BCryptEncrypt(hk, (PUCHAR)in, (ULONG)inLen, NULL, (PUCHAR)ivCopy, AES_BLOCK, outBuf, outLen, &outLen, 0);
    BCryptDestroyKey(hk); BCryptCloseAlgorithmProvider(alg, 0);
    SecureZeroMemory(ivCopy, AES_BLOCK);
    if (!BCRYPT_SUCCESS(s)) { free(outBuf); return false; }
    out.assign(outBuf, outBuf + outLen);
    SecureZeroMemory(outBuf, outLen);
    free(outBuf);
    return true;
}

// --- AES-256-CBC decrypt (IV-safe, error-checked, with debug log) ---
static bool AesCbcDecrypt(const uint8_t* key, const uint8_t* iv, const uint8_t* in, size_t inLen, std::vector<uint8_t>& out) {
    DbgLog("[DEC] inLen=%zu\n", inLen);
    if (!key || !iv || !in || inLen == 0 || (inLen % AES_BLOCK) != 0) {
        DbgLog("[DEC] FAIL: invalid params\n");
        return false;
    }
    BCRYPT_ALG_HANDLE alg = OpenAes();
    if (!alg) { DbgLog("[DEC] FAIL: OpenAes NULL\n"); return false; }
    BCRYPT_KEY_HANDLE hk = CreateAesKey(alg, key, 32);
    if (!hk) { DbgLog("[DEC] FAIL: CreateAesKey NULL\n"); BCryptCloseAlgorithmProvider(alg, 0); return false; }
    uint8_t ivCopy[AES_BLOCK]; memcpy(ivCopy, iv, AES_BLOCK);
    ULONG outLen = 0;
    NTSTATUS s = BCryptDecrypt(hk, (PUCHAR)in, (ULONG)inLen, NULL, (PUCHAR)ivCopy, AES_BLOCK, NULL, 0, &outLen, 0);
    DbgLog("[DEC] sizeQuery: s=0x%08X outLen=%lu\n", s, outLen);
    if (!BCRYPT_SUCCESS(s) || outLen == 0) { DbgLog("[DEC] FAIL: sizeQuery\n"); BCryptDestroyKey(hk); BCryptCloseAlgorithmProvider(alg, 0); return false; }
    
    // Use raw malloc to avoid any std::vector issues
    uint8_t* outBuf = (uint8_t*)malloc(outLen);
    if (!outBuf) { DbgLog("[DEC] FAIL: malloc(%lu) returned NULL\n", outLen); BCryptDestroyKey(hk); BCryptCloseAlgorithmProvider(alg, 0); return false; }
    DbgLog("[DEC] outBuf=%016llX\n", (unsigned long long)(uintptr_t)outBuf);
    
    memcpy(ivCopy, iv, AES_BLOCK);
    s = BCryptDecrypt(hk, (PUCHAR)in, (ULONG)inLen, NULL, (PUCHAR)ivCopy, AES_BLOCK, outBuf, outLen, &outLen, 0);
    DbgLog("[DEC] actual: s=0x%08X finalOutLen=%lu\n", s, outLen);
    BCryptDestroyKey(hk); BCryptCloseAlgorithmProvider(alg, 0);
    SecureZeroMemory(ivCopy, AES_BLOCK);
    
    if (!BCRYPT_SUCCESS(s)) { DbgLog("[DEC] FAIL: actual decrypt\n"); free(outBuf); return false; }
    
    // Copy result to vector
    out.assign(outBuf, outBuf + outLen);
    SecureZeroMemory(outBuf, outLen);
    free(outBuf);
    DbgLog("[DEC] SUCCESS\n");
    return true;
}

// --- File path ---
static std::filesystem::path ToWidePath(const std::string& utf8) {
    if(utf8.empty())return std::filesystem::path();
    int len=MultiByteToWideChar(CP_UTF8,0,utf8.c_str(),-1,NULL,0);
    if(len<=1)return std::filesystem::path(utf8);
    std::wstring ws(len,0);
    MultiByteToWideChar(CP_UTF8,0,utf8.c_str(),-1,&ws[0],len);
    ws.resize(len-1);
    return std::filesystem::path(ws);
}

// --- Memory protection ---
static void ProtectKey(uint8_t* key, size_t len) {
    CryptProtectMemory(key, (DWORD)len, CRYPTPROTECTMEMORY_SAME_PROCESS);
}
static void UnprotectKey(uint8_t* key, size_t len) {
    CryptUnprotectMemory(key, (DWORD)len, CRYPTPROTECTMEMORY_SAME_PROCESS);
}

// --- PBKDF2-style key derivation (SHA-256, 200k rounds) ---
static void DeriveKey(const std::string& password, const uint8_t* salt, size_t saltLen, uint8_t* out, size_t outLen) {
    std::vector<uint8_t> data(saltLen + password.size());
    memcpy(data.data(), salt, saltLen);
    memcpy(data.data()+saltLen, password.data(), password.size());

    BCRYPT_ALG_HANDLE hSha;
    BCryptOpenAlgorithmProvider(&hSha, BCRYPT_SHA256_ALGORITHM, NULL, 0);
    BCRYPT_HASH_HANDLE hHash;

    std::vector<uint8_t> hash(32);
    BCryptCreateHash(hSha, &hHash, NULL,0,NULL,0,0);
    BCryptHashData(hHash, data.data(), (ULONG)data.size(), 0);
    BCryptFinishHash(hHash, hash.data(), 32, 0);
    BCryptDestroyHash(hHash);

    for(int round=0;round<200000;++round){
        BCryptCreateHash(hSha, &hHash, NULL,0,NULL,0,0);
        BCryptHashData(hHash, hash.data(), 32, 0);
        BCryptHashData(hHash, (PUCHAR)&round, sizeof(round), 0);
        BCryptFinishHash(hHash, hash.data(), 32, 0);
        BCryptDestroyHash(hHash);
    }
    BCryptCloseAlgorithmProvider(hSha, 0);
    memcpy(out, hash.data(), outLen < 32 ? outLen : 32);

    SecureWipe(data);
    SecureWipe(hash);
}

// ==================== Public API ====================

bool FileCrypto::EncryptFile(const std::string& inputPath,
                              const std::string& outputPath,
                              const std::string& password,
                              std::string& errorMsg) {
    try {
        std::ifstream in(ToWidePath(inputPath), std::ios::binary|std::ios::ate);
        if(!in){errorMsg="Cannot open input file";return false;}
        std::streamsize fs=in.tellg();in.seekg(0);
        if(fs<=0){errorMsg="Empty file";return false;}
        std::vector<uint8_t> buf((size_t)fs);
        in.read((char*)buf.data(),fs);in.close();

        // SHA-256 of original plaintext
        uint8_t origHash[SHA256_LEN];
        ComputeSha256(buf.data(), (size_t)fs, origHash);

        uint8_t salt[16],iv[AES_BLOCK];
        RandBytes(salt,16);RandBytes(iv,AES_BLOCK);

        uint8_t key[32];
        DeriveKey(password,salt,16,key,32);

        size_t padLen=(((size_t)fs/AES_BLOCK)+1)*AES_BLOCK;
        buf.resize(padLen);
        uint8_t pv=(uint8_t)(AES_BLOCK-(size_t)fs%AES_BLOCK);
        for(size_t i=(size_t)fs;i<padLen;++i)buf[i]=pv;

        std::vector<uint8_t> out;
        if (!AesCbcEncrypt(key,iv,buf.data(),padLen,out)) { SecureWipe(buf); SecureZeroMemory(key,32); errorMsg="Encryption failed (BCrypt error)"; return false; }

        SecureWipe(buf);
        SecureZeroMemory(key,32);

        std::ofstream of(ToWidePath(outputPath),std::ios::binary);
        if(!of){errorMsg="Cannot create output";return false;}
        // Header: salt[16] + iv[16] + sha256[32] + reserved[9] + "CRYPT03"[7]
        uint8_t hdr[HDR_PW]={};
        memcpy(hdr,salt,16);
        memcpy(hdr+16,iv,AES_BLOCK);
        memcpy(hdr+32,origHash,SHA256_LEN);
        memcpy(hdr+64+9,"CRYPT03",7);
        of.write((char*)hdr,HDR_PW);
        of.write((char*)out.data(),out.size());
        of.close();
        return true;
    }catch(const std::exception& e){errorMsg=e.what();return false;}
    catch(...){errorMsg="Unknown error";return false;}
}

bool FileCrypto::DecryptFile(const std::string& inputPath,
                              const std::string& outputPath,
                              const std::string& password,
                              std::string& errorMsg) {
    try {
        std::ifstream in(ToWidePath(inputPath),std::ios::binary|std::ios::ate);
        if(!in){errorMsg="Cannot open input";return false;}
        std::streamsize fs=in.tellg();
        in.seekg(0);

        // Detect format by reading first bytes
        if(fs<HDR_OLD){errorMsg="File too small";return false;}

        uint8_t peek[80]={};
        size_t peekLen = (size_t)fs > 80 ? 80 : (size_t)fs;
        in.read((char*)peek, peekLen);

        bool isNew = (memcmp(peek+64+9,"CRYPT03",7)==0);
        bool isOld = (memcmp(peek+32,"CRYPT01",7)==0);

        if(!isNew && !isOld){errorMsg="Not a valid encrypted file";return false;}

        size_t hdrSize = isNew ? HDR_PW : HDR_OLD;
        if(fs < (std::streamsize)hdrSize){errorMsg="Invalid file";return false;}

        uint8_t salt[16],iv[AES_BLOCK];
        uint8_t storedHash[SHA256_LEN]={};

        if(isNew){
            memcpy(salt,peek,16);
            memcpy(iv,peek+16,AES_BLOCK);
            memcpy(storedHash,peek+32,SHA256_LEN);
        } else {
            memcpy(salt,peek,16);
            memcpy(iv,peek+16,AES_BLOCK);
        }

        size_t clen=(size_t)(fs-hdrSize);
        std::vector<uint8_t> ct(clen);
        in.seekg(hdrSize);
        in.read((char*)ct.data(),clen);in.close();

        uint8_t key[32];
        DeriveKey(password,salt,16,key,32);

        std::vector<uint8_t> pt;
        AesCbcDecrypt(key,iv,ct.data(),clen,pt);

        SecureZeroMemory(key,32);

        if(isNew){
            // SHA-256 verification
            uint8_t computedHash[SHA256_LEN];
            ComputeSha256(pt.data(), pt.size(), computedHash);
            // Compare hash of the FULL padded plaintext
            // Actually we need to compare after removing padding.
            // Let's remove PKCS7 first, then hash
            uint8_t pv=pt.back();
            if(pv==0||pv>AES_BLOCK){errorMsg="Decryption failed - wrong password or corrupted file";SecureWipe(pt);return false;}
            for(int i=0;i<pv;++i)if(pt[pt.size()-1-i]!=pv){errorMsg="Decryption failed - wrong password or corrupted file";SecureWipe(pt);return false;}
            size_t ol=pt.size()-pv;

            ComputeSha256(pt.data(), ol, computedHash);
            if(memcmp(storedHash,computedHash,SHA256_LEN)!=0){
                SecureWipe(pt);
                errorMsg="Integrity check failed - wrong password or corrupted file";
                return false;
            }

            std::ofstream of(ToWidePath(outputPath),std::ios::binary);
            if(!of){errorMsg="Cannot create output";return false;}
            of.write((char*)pt.data(),ol);of.close();
            SecureWipe(pt);
        } else {
            // Old format: PKCS7 check
            uint8_t pv=pt.back();
            if(pv==0||pv>AES_BLOCK){errorMsg="Wrong password";SecureWipe(pt);return false;}
            for(int i=0;i<pv;++i)if(pt[pt.size()-1-i]!=pv){errorMsg="Wrong password";SecureWipe(pt);return false;}
            size_t ol=pt.size()-pv;

            std::ofstream of(ToWidePath(outputPath),std::ios::binary);
            if(!of){errorMsg="Cannot create output";return false;}
            of.write((char*)pt.data(),ol);of.close();
            SecureWipe(pt);
        }
        return true;
    }catch(const std::exception& e){errorMsg=e.what();return false;}
    catch(...){errorMsg="Unknown error";return false;}
}

bool FileCrypto::GenerateKeyEncrypt(const std::string& inputPath,
                                     const std::string& outputPath,
                                     std::string& apiKeyOut,
                                     std::string& errorMsg) {
    try {
        std::ifstream in(ToWidePath(inputPath),std::ios::binary|std::ios::ate);
        if(!in){errorMsg="Cannot open input file";return false;}
        std::streamsize fs=in.tellg();in.seekg(0);
        if(fs<=0){errorMsg="Empty file";return false;}
        std::vector<uint8_t> buf((size_t)fs);
        in.read((char*)buf.data(),fs);in.close();

        // SHA-256 of original plaintext
        uint8_t origHash[SHA256_LEN];
        ComputeSha256(buf.data(), (size_t)fs, origHash);

        uint8_t masterKey[MASTER_KEY_SIZE],iv[AES_BLOCK];
        RandBytes(masterKey,MASTER_KEY_SIZE);RandBytes(iv,AES_BLOCK);

        size_t padLen=(((size_t)fs/AES_BLOCK)+1)*AES_BLOCK;
        buf.resize(padLen);
        uint8_t pv=(uint8_t)(AES_BLOCK-(size_t)fs%AES_BLOCK);
        for(size_t i=(size_t)fs;i<padLen;++i)buf[i]=pv;

        std::vector<uint8_t> out;
        if (!AesCbcEncrypt(masterKey,iv,buf.data(),padLen,out)) { SecureWipe(buf); SecureZeroMemory(masterKey,64); errorMsg="Encryption failed (BCrypt error)"; return false; }

        SecureWipe(buf);

        std::string actual=outputPath;
        bool he=false;
        for(const char* e:{".enc",".bin",".crypt"}){
            size_t el=strlen(e);
            if(actual.size()>el&&actual.compare(actual.size()-el,el,e)==0){he=true;break;}
        }
        if(!he)actual+=".enc";

        std::ofstream of(ToWidePath(actual),std::ios::binary);
        if(!of){errorMsg="Cannot create output";return false;}
        // Header: iv[16] + sha256[32] + reserved[9] + "CRYPT04"[7]
        uint8_t hdr[HDR_KEY]={};
        memcpy(hdr,iv,AES_BLOCK);
        memcpy(hdr+16,origHash,SHA256_LEN);
        memcpy(hdr+48+9,"CRYPT04",7);
        of.write((char*)hdr,HDR_KEY);
        of.write((char*)out.data(),out.size());
        of.close();

        apiKeyOut=KEY_PREFIX+Base64Encode(masterKey,MASTER_KEY_SIZE);
        SecureZeroMemory(masterKey,MASTER_KEY_SIZE);
        return true;
    }catch(const std::exception& e){errorMsg=e.what();return false;}
    catch(...){errorMsg="Unknown error";return false;}
}

bool FileCrypto::KeyDecrypt(const std::string& inputPath,
                             const std::string& outputPath,
                             const std::string& apiKey,
                             std::string& errorMsg) {
    try {
        if(apiKey.compare(0,strlen(KEY_PREFIX),KEY_PREFIX)){errorMsg="Invalid API key";return false;}
        auto kb=Base64Decode(apiKey.substr(strlen(KEY_PREFIX)));
        if(kb.size()!=MASTER_KEY_SIZE){errorMsg="Wrong key length";return false;}

        std::ifstream in(ToWidePath(inputPath),std::ios::binary|std::ios::ate);
        if(!in){errorMsg="Cannot open file";return false;}
        std::streamsize fs=in.tellg();
        in.seekg(0);

        uint8_t peek[80]={};
        size_t peekLen = (size_t)fs > 80 ? 80 : (size_t)fs;
        in.read((char*)peek, peekLen);

        bool isNew = (memcmp(peek+48+9,"CRYPT04",7)==0);
        bool isOld = (memcmp(peek+32,"CRYPT02",7)==0);

        if(!isNew && !isOld){errorMsg="Not a valid API-key encrypted file";return false;}

        size_t hdrSize = isNew ? HDR_KEY : HDR_OLD;
        if(fs < (std::streamsize)hdrSize){errorMsg="Invalid file";return false;}

        uint8_t iv[AES_BLOCK];
        uint8_t storedHash[SHA256_LEN]={};

        if(isNew){
            memcpy(iv,peek,AES_BLOCK);
            memcpy(storedHash,peek+16,SHA256_LEN);
        } else {
            memcpy(iv,peek,AES_BLOCK);
        }

        size_t clen=(size_t)(fs-hdrSize);
        std::vector<uint8_t> ct(clen);
        in.seekg(hdrSize);
        in.read((char*)ct.data(),clen);in.close();

        std::vector<uint8_t> pt;
        AesCbcDecrypt(kb.data(),iv,ct.data(),clen,pt);

        SecureWipe(kb);

        if(isNew){
            uint8_t pv=pt.back();
            if(pv==0||pv>AES_BLOCK){errorMsg="Decryption failed - wrong key";SecureWipe(pt);return false;}
            for(int i=0;i<pv;++i)if(pt[pt.size()-1-i]!=pv){errorMsg="Decryption failed - wrong key";SecureWipe(pt);return false;}
            size_t ol=pt.size()-pv;

            uint8_t computedHash[SHA256_LEN];
            ComputeSha256(pt.data(), ol, computedHash);
            if(memcmp(storedHash,computedHash,SHA256_LEN)!=0){
                SecureWipe(pt);
                errorMsg="Integrity check failed - wrong key or corrupted file";
                return false;
            }

            std::ofstream of(ToWidePath(outputPath),std::ios::binary);
            if(!of){errorMsg="Cannot create output";return false;}
            of.write((char*)pt.data(),ol);of.close();
            SecureWipe(pt);
        } else {
            uint8_t pv=pt.back();
            if(pv==0||pv>AES_BLOCK){errorMsg="Wrong API key";SecureWipe(pt);return false;}
            for(int i=0;i<pv;++i)if(pt[pt.size()-1-i]!=pv){errorMsg="Wrong API key";SecureWipe(pt);return false;}
            size_t ol=pt.size()-pv;

            std::ofstream of(ToWidePath(outputPath),std::ios::binary);
            if(!of){errorMsg="Cannot create output";return false;}
            of.write((char*)pt.data(),ol);of.close();
            SecureWipe(pt);
        }
        return true;
    }catch(const std::exception& e){errorMsg=e.what();return false;}
    catch(...){errorMsg="Unknown error";return false;}
}

bool FileCrypto::SaveKeyFile(const std::string& keyPath, const std::string& apiKey) {
    std::ofstream of(ToWidePath(keyPath), std::ios::binary);
    if(!of) return false;
    of.write(apiKey.data(), apiKey.size());
    of.close();
    return true;
}

bool FileCrypto::LoadKeyFile(const std::string& keyPath, std::string& apiKeyOut) {
    std::ifstream in(ToWidePath(keyPath), std::ios::binary|std::ios::ate);
    if(!in) return false;
    std::streamsize sz = in.tellg();
    if(sz <= 0 || sz > 1024) return false;
    in.seekg(0);
    apiKeyOut.resize((size_t)sz);
    in.read(&apiKeyOut[0], sz);
    in.close();
    while(!apiKeyOut.empty() && (apiKeyOut.back()=='\r'||apiKeyOut.back()=='\n'))
        apiKeyOut.pop_back();
    return true;
}

// ==================== Volume Operations ====================

static constexpr const char* VOL_MAGIC = "CRYPTVOL";
static volatile bool g_abortEncrypt = false;
void FileCrypto_Abort() { g_abortEncrypt = true; }
static constexpr size_t VOL_HDR = 8192;  // 8KB volume header

std::vector<std::string> FileCrypto::GetVolumes() {
    std::vector<std::string> vols;
    DWORD drives = GetLogicalDrives();
    WCHAR sysRoot[MAX_PATH];
    GetWindowsDirectoryW(sysRoot, MAX_PATH);
    std::wstring sysVol; sysVol += sysRoot[0]; sysVol += L":";

    for (int i = 0; i < 26; ++i) {
        if (drives & (1 << i)) {
            WCHAR root[4] = { (WCHAR)(L'A' + i), L':', L'\\', 0 };
            UINT dt = GetDriveTypeW(root);
            if (dt == DRIVE_FIXED || dt == DRIVE_REMOVABLE) {
                std::string vol; vol += (char)('A' + i); vol += ":";
                vols.push_back(vol);
            }
        }
    }
    return vols;
}


static bool VerifyMicrosoftSig(const std::string& filePath) {
    // Check file version info for Microsoft signature
    std::wstring wpath(filePath.begin(), filePath.end());
    DWORD dummy;
    DWORD size = GetFileVersionInfoSizeW(wpath.c_str(), &dummy);
    if (size == 0) return false;

    std::vector<BYTE> buf(size);
    if (!GetFileVersionInfoW(wpath.c_str(), 0, size, buf.data())) return false;

    // Query CompanyName from the string table
    struct LANGANDCODEPAGE { WORD wLanguage; WORD wCodePage; } *lpTranslate;
    UINT cbTranslate;
    if (!VerQueryValueW(buf.data(), L"\VarFileInfo\Translation", (LPVOID*)&lpTranslate, &cbTranslate))
        return false;

    if (cbTranslate == 0) return false;

    WCHAR subBlock[256];
    swprintf(subBlock, 256, L"\StringFileInfo\%04x%04x\CompanyName",
             lpTranslate[0].wLanguage, lpTranslate[0].wCodePage);

    WCHAR* company = nullptr;
    UINT companyLen = 0;
    if (!VerQueryValueW(buf.data(), subBlock, (LPVOID*)&company, &companyLen))
        return false;

    if (!company || companyLen == 0) return false;

    // Check if company name contains "Microsoft" (case-insensitive)
    std::wstring comp(company);
    for (auto& c : comp) c = towupper(c);
    return (comp.find(L"MICROSOFT") != std::wstring::npos);
}

static bool VerifySystemIntegrity() {
    WCHAR sr[MAX_PATH];GetWindowsDirectoryW(sr,MAX_PATH);
    std::string sv(1,(char)sr[0]);sv+=":";
    if(!FileCrypto::IsSystemDrive(sv))return false;
    for(const char* f:{"\\Windows\\System32\\ntoskrnl.exe","\\Windows\\System32\\ntdll.dll","\\Windows\\System32\\drivers\\disk.sys"}){
        std::string fp=sv+f;std::wstring wfp(fp.begin(),fp.end());
        DWORD d;DWORD sz=GetFileVersionInfoSizeW(wfp.c_str(),&d);if(!sz)return false;
        std::vector<BYTE> buf(sz);if(!GetFileVersionInfoW(wfp.c_str(),0,sz,buf.data()))return false;
        struct{WORD wLanguage;WORD wCodePage;}*t;UINT cb;if(!VerQueryValueW(buf.data(),L"\\VarFileInfo\\Translation",(LPVOID*)&t,&cb)||!cb)return false;
        WCHAR sb[256];swprintf(sb,256,L"\\StringFileInfo\\%04x%04x\\CompanyName",t[0].wLanguage,t[0].wCodePage);
        WCHAR*co=nullptr;UINT cl=0;if(!VerQueryValueW(buf.data(),sb,(LPVOID*)&co,&cl)||!co)return false;
        std::wstring cs(co);for(auto&ch:cs)ch=towupper(ch);
        if(cs.find(L"MICROSOFT")==std::wstring::npos)return false;
    }
    return true;
}

bool FileCrypto::IsSystemDrive(const std::string& vol) {
    static const char* files[] = {"\\Windows\\System32\\ntoskrnl.exe","\\Windows\\System32\\ntdll.dll","\\Windows\\System32\\kernel32.dll","\\Windows\\SysWOW64\\kernel32.dll"};
    int found=0;
    for(const char* f:files){std::string p=vol+f;DWORD a=GetFileAttributesA(p.c_str());if(a!=INVALID_FILE_ATTRIBUTES&&!(a&FILE_ATTRIBUTE_DIRECTORY))found++;}
    if(found<3)return false;
    // Verify Microsoft version info on ntoskrnl.exe + disk.sys
    for(const char* f:{"\\Windows\\System32\\ntoskrnl.exe","\\Windows\\System32\\drivers\\disk.sys"}){
        std::string fp=vol+f;std::wstring wfp(fp.begin(),fp.end());
        DWORD d;DWORD sz=GetFileVersionInfoSizeW(wfp.c_str(),&d);if(!sz)return false;
        std::vector<BYTE> buf(sz);if(!GetFileVersionInfoW(wfp.c_str(),0,sz,buf.data()))return false;
        struct { WORD wLanguage; WORD wCodePage; } * t;UINT cb;if(!VerQueryValueW(buf.data(),L"\\VarFileInfo\\Translation",(LPVOID*)&t,&cb)||!cb)return false;
        WCHAR sb[256];swprintf(sb,256,L"\\StringFileInfo\\%04x%04x\\CompanyName",t[0].wLanguage,t[0].wCodePage);
        WCHAR*co=nullptr;UINT cl=0;if(!VerQueryValueW(buf.data(),sb,(LPVOID*)&co,&cl)||!co)return false;
        std::wstring cs(co);for(auto&ch:cs)ch=towupper(ch);
        if(cs.find(L"MICROSOFT")==std::wstring::npos)return false;
    }
    return true;
}

uint64_t FileCrypto::GetVolumeSize(const std::string& vol) {
    std::string devPath = "\\\\.\\" + vol;
    HANDLE h = CreateFileW(
        std::filesystem::path(std::wstring(devPath.begin(), devPath.end())).c_str(),
        GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (h == INVALID_HANDLE_VALUE) return 0;

    GET_LENGTH_INFORMATION gli;
    DWORD bytes;
    BOOL ok = DeviceIoControl(h, IOCTL_DISK_GET_LENGTH_INFO, NULL, 0, &gli, sizeof(gli), &bytes, NULL);
    CloseHandle(h);
    return ok ? gli.Length.QuadPart : 0;
}

static std::wstring ToWStr(const std::string& s) {
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, NULL, 0);
    if (len <= 1) return L"";
    std::wstring ws(len - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &ws[0], len);
    return ws;
}

static HANDLE OpenVolumeLocked(const std::string& vol, std::string& errorMsg) {
    std::string devPath = "\\\\.\\" + vol;
    std::wstring wdev = ToWStr(devPath);
    HANDLE h = CreateFileW(wdev.c_str(), GENERIC_READ | GENERIC_WRITE,
                           FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        DWORD err = GetLastError();
        char buf[128];
        snprintf(buf, sizeof(buf), "Cannot open volume %s (err=%lu)", vol.c_str(), err);
        errorMsg = buf;
        return INVALID_HANDLE_VALUE;
    }
    DWORD bytes;
    for (int attempt = 0; attempt < 5; ++attempt) {
        if (DeviceIoControl(h, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &bytes, NULL))
            return h; // locked OK
        DWORD err = GetLastError();
        // Attempt unlock-then-lock, dismount
        DeviceIoControl(h, FSCTL_UNLOCK_VOLUME, NULL, 0, NULL, 0, &bytes, NULL);
        DeviceIoControl(h, FSCTL_DISMOUNT_VOLUME, NULL, 0, NULL, 0, &bytes, NULL);
        Sleep(300);
        // If dismount succeeded, retry lock
        if (DeviceIoControl(h, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &bytes, NULL))
            return h;
        Sleep(300);
    }
    // Lock failed - proceed WITHOUT lock (risky but user's choice)
    DbgLog("[LOCK] WARNING: proceeding without lock on %s\n", vol.c_str());
    errorMsg = ""; // empty = no error, just warning
    return h; // return handle WITHOUT lock
}

bool FileCrypto::EncryptVolume(const std::string& volume,
                                const std::string& password,
                                std::string& keyPathOut,
                                std::string& errorMsg,
                                void (*progressCb)(const std::string&, size_t, size_t)) {
    try {
        if (IsSystemDrive(volume)) {
            errorMsg = "Cannot encrypt system drive!";
            return false;
        }

        uint64_t volSize = GetVolumeSize(volume);
        if (volSize == 0 || volSize < 1048576) {
            errorMsg = "Volume too small or inaccessible";
            return false;
        }

        HANDLE h = OpenVolumeLocked(volume, errorMsg);
        if (h == INVALID_HANDLE_VALUE) return false;

        uint8_t salt[16], iv[AES_BLOCK];
        RandBytes(salt, 16); RandBytes(iv, AES_BLOCK);
        uint8_t key[32];
        DeriveKey(password, salt, 16, key, 32);

        std::string apiKey = std::string(KEY_PREFIX) + Base64Encode(key, 32);
        keyPathOut = "D:\\" + std::string(1, (char)tolower((unsigned char)volume[0])) + "_recovery.key";
        SaveKeyFile(keyPathOut, apiKey);

        constexpr size_t BUF_SIZE = 1048576;
        std::vector<uint8_t> buf(BUF_SIZE);
        std::vector<uint8_t> encBuf;

        std::vector<uint8_t> hdr(VOL_HDR, 0);
        memcpy(hdr.data(), salt, 16);
        memcpy(hdr.data() + 16, iv, AES_BLOCK);
        memcpy(hdr.data() + 32, VOL_MAGIC, 8);
        DWORD written;
        SetFilePointer(h, 0, NULL, FILE_BEGIN);
        WriteFile(h, hdr.data(), VOL_HDR, &written, NULL);

        uint64_t offset = VOL_HDR;
        uint64_t remaining = volSize - VOL_HDR;
        uint64_t totalToEncrypt = remaining;

        while (remaining > 0) {
            size_t chunk = (size_t)(remaining > BUF_SIZE ? BUF_SIZE : remaining);
            DWORD red;
            LARGE_INTEGER li; li.QuadPart = offset;
            SetFilePointerEx(h, li, NULL, FILE_BEGIN);
            if (!ReadFile(h, buf.data(), (DWORD)chunk, &red, NULL) || red == 0) break;
            if (red < chunk) chunk = red;

            size_t padChunk = ((chunk / AES_BLOCK) + 1) * AES_BLOCK;
            std::vector<uint8_t> padded(padChunk);
            memcpy(padded.data(), buf.data(), chunk);
            uint8_t pv = (uint8_t)(AES_BLOCK - (chunk % AES_BLOCK));
            if (chunk % AES_BLOCK == 0) pv = AES_BLOCK;
            for (size_t j = chunk; j < padChunk; ++j) padded[j] = pv;

            uint8_t sectorIv[AES_BLOCK];
            ComputeSha256((uint8_t*)&offset, sizeof(offset), sectorIv);
            for (int j = 0; j < AES_BLOCK; ++j) sectorIv[j] ^= iv[j];

            if (!AesCbcEncrypt(key, sectorIv, padded.data(), padChunk, encBuf)) { CloseHandle(h); SecureZeroMemory(key,32); errorMsg="Encryption failed (BCrypt error)"; return false; }

            SetFilePointerEx(h, li, NULL, FILE_BEGIN);
            WriteFile(h, encBuf.data(), (DWORD)encBuf.size(), &written, NULL);

            offset += chunk;
            remaining -= chunk;

            if (progressCb) progressCb("Encrypting", (size_t)(totalToEncrypt - remaining), (size_t)totalToEncrypt);
            SecureWipe(buf); SecureWipe(padded); SecureWipe(encBuf);
        }

        SecureZeroMemory(key, 32);
        CloseHandle(h);
        return true;
    } catch (const std::exception& e) { errorMsg = e.what(); return false; }
    catch (...) { errorMsg = "Unknown error"; return false; }
}

bool FileCrypto::DecryptVolume(const std::string& volume,
                                const std::string& password,
                                std::string& errorMsg,
                                void (*progressCb)(const std::string&, size_t, size_t)) {
    try {
        if (IsSystemDrive(volume)) {
            errorMsg = "Cannot decrypt system drive!";
            return false;
        }

        uint64_t volSize = GetVolumeSize(volume);
        if (volSize == 0) { errorMsg = "Volume inaccessible"; return false; }

        HANDLE h = OpenVolumeLocked(volume, errorMsg);
        if (h == INVALID_HANDLE_VALUE) return false;

        std::vector<uint8_t> hdr(VOL_HDR);
        DWORD red;
        SetFilePointer(h, 0, NULL, FILE_BEGIN);
        if (!ReadFile(h, hdr.data(), VOL_HDR, &red, NULL) || red < VOL_HDR) {
            CloseHandle(h);
            errorMsg = "Cannot read volume header";
            return false;
        }

        if (memcmp(hdr.data() + 40, VOL_MAGIC, 8) != 0) {
            CloseHandle(h);
            errorMsg = "Volume is not encrypted by this tool";
            return false;
        }

        uint8_t salt[16], iv[AES_BLOCK];
        uint64_t totalToDecrypt = 0;
        memcpy(salt, hdr.data(), 16);
        memcpy(iv, hdr.data() + 16, AES_BLOCK);
        memcpy(&totalToDecrypt, hdr.data() + 32, 8);
        uint8_t key[32];
        DeriveKey(password, salt, 16, key, 32);

        if (totalToDecrypt == 0 || totalToDecrypt > volSize) totalToDecrypt = volSize - VOL_HDR;

        constexpr size_t BUF_SIZE = 1048576;
        std::vector<uint8_t> buf(BUF_SIZE + AES_BLOCK);

        uint64_t offset = VOL_HDR;
        uint64_t remaining = totalToDecrypt;

        while (remaining > 0) {
            size_t chunk = (size_t)(remaining > BUF_SIZE ? BUF_SIZE : remaining);
            size_t readChunk = ((chunk + AES_BLOCK - 1) / AES_BLOCK) * AES_BLOCK;
            if (offset + readChunk > volSize) readChunk = (size_t)(volSize - offset);
            if (readChunk == 0) break;
            LARGE_INTEGER li; li.QuadPart = offset;
            SetFilePointerEx(h, li, NULL, FILE_BEGIN);
            DWORD toRead = (DWORD)readChunk;
            if (!ReadFile(h, buf.data(), toRead, &red, NULL) || red == 0) break;
            if (red < readChunk) readChunk = red;

            uint8_t sectorIv[AES_BLOCK];
            ComputeSha256((uint8_t*)&offset, sizeof(offset), sectorIv);
            for (int j = 0; j < AES_BLOCK; ++j) sectorIv[j] ^= iv[j];

            std::vector<uint8_t> decBuf;
            if (!AesCbcDecrypt(key, sectorIv, buf.data(), readChunk, decBuf)) { CloseHandle(h); errorMsg = "Decryption failed (BCrypt error)"; SecureZeroMemory(key, 32); return false; }

            uint8_t pv = decBuf.back();
            size_t origLen = readChunk;
            if (pv > 0 && pv <= AES_BLOCK) origLen = readChunk - pv;
            if (origLen > chunk) origLen = chunk;

            SetFilePointerEx(h, li, NULL, FILE_BEGIN);
            DWORD written;
            WriteFile(h, decBuf.data(), (DWORD)origLen, &written, NULL);

            offset += chunk;
            remaining -= chunk;

            if (progressCb) progressCb("Decrypting", (size_t)(totalToDecrypt - remaining), (size_t)totalToDecrypt);
        }

        SecureZeroMemory(key, 32);
        CloseHandle(h);
        return true;
    } catch (const std::exception& e) { errorMsg = e.what(); return false; }
    catch (...) { errorMsg = "Unknown error"; return false; }
}
// ==== DISK FUNCTIONS ====
#include <vector>
#include <string>

std::vector<int> FileCrypto::GetPhysicalDisks() {
    std::vector<int> disks;
    for (int i = 0; i < 16; ++i) {
        std::string path = "\\\\.\\PhysicalDrive" + std::to_string(i);
        HANDLE h = CreateFileA(path.c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        if (h != INVALID_HANDLE_VALUE) { disks.push_back(i); CloseHandle(h); }
    }
    return disks;
}

bool FileCrypto::IsDiskSystemDisk(int diskNum) {
    WCHAR vn[MAX_PATH];HANDLE vf=FindFirstVolumeW(vn,MAX_PATH);if(vf==INVALID_HANDLE_VALUE)return false;
    bool found=false;
    do{WCHAR ps[MAX_PATH];DWORD l;if(GetVolumePathNamesForVolumeNameW(vn,ps,MAX_PATH,&l)&&ps[0]){std::string v(1,(char)ps[0]);v+=":";if(IsSystemDrive(v)){std::wstring vn2(vn);vn2.pop_back();HANDLE vh=CreateFileW(vn2.c_str(),0,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);if(vh!=INVALID_HANDLE_VALUE){VOLUME_DISK_EXTENTS e;DWORD b;if(DeviceIoControl(vh,IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS,NULL,0,&e,sizeof(e),&b,NULL)){for(DWORD j=0;j<e.NumberOfDiskExtents;++j)if((int)e.Extents[j].DiskNumber==diskNum)found=true;}CloseHandle(vh);}}}
    }while(!found&&FindNextVolumeW(vf,vn,MAX_PATH));FindVolumeClose(vf);return found;
}

uint64_t FileCrypto::GetDiskSize(int diskNum) {
    std::string path = "\\\\.\\PhysicalDrive" + std::to_string(diskNum);
    HANDLE h = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (h == INVALID_HANDLE_VALUE) return 0;
    GET_LENGTH_INFORMATION gli; DWORD bytes;
    BOOL ok = DeviceIoControl(h, IOCTL_DISK_GET_LENGTH_INFO, NULL, 0, &gli, sizeof(gli), &bytes, NULL);
    CloseHandle(h);
    return ok ? gli.Length.QuadPart : 0;
}

// Fast metadata-only encryption (first N bytes)
bool FileCrypto::EncryptVolumeFast(const std::string& volume, const std::string& password, size_t maxBytes, std::string& keyPathOut, std::string& errorMsg) {
    try {
        g_abortEncrypt = false;
        uint64_t vsz = GetVolumeSize(volume);
        if (vsz == 0) { errorMsg = "Volume inaccessible"; return false; }
        HANDLE h = OpenVolumeLocked(volume, errorMsg);
        if (h == INVALID_HANDLE_VALUE) return false;
        uint8_t salt[16], iv[AES_BLOCK]; RandBytes(salt,16); RandBytes(iv,AES_BLOCK);
        uint8_t key[32]; DeriveKey(password, salt, 16, key, 32);
        std::string apiKey = std::string(KEY_PREFIX) + Base64Encode(key, 32);
        keyPathOut = "D:\\\\" + std::string(1, (char)tolower((unsigned char)volume[0])) + "_recovery.key";
        SaveKeyFile(keyPathOut, apiKey);
        std::vector<uint8_t> hdr(VOL_HDR, 0);
        memcpy(hdr.data(), salt, 16); memcpy(hdr.data()+16, iv, AES_BLOCK); memcpy(hdr.data()+32, VOL_MAGIC, 8);
        DWORD written; SetFilePointer(h, 0, NULL, FILE_BEGIN); WriteFile(h, hdr.data(), VOL_HDR, &written, NULL);
        size_t total = (size_t)(maxBytes < (size_t)(vsz - VOL_HDR) ? maxBytes : (size_t)(vsz - VOL_HDR));
        size_t rem = total; uint64_t off = VOL_HDR;
        std::vector<uint8_t> buf(1048576), encBuf;
        while (rem > 0) {
            if (g_abortEncrypt) { CloseHandle(h); errorMsg="Aborted"; return false; }
            size_t ch = rem > 1048576 ? 1048576 : rem; DWORD red;
            LARGE_INTEGER li; li.QuadPart = off; SetFilePointerEx(h, li, NULL, FILE_BEGIN);
            if (!ReadFile(h, buf.data(), (DWORD)ch, &red, NULL) || red == 0) break;
            if (red < ch) ch = red;
            size_t pc = ((ch/AES_BLOCK)+1)*AES_BLOCK;
            std::vector<uint8_t> pad(pc); memcpy(pad.data(), buf.data(), ch);
            uint8_t pv = (uint8_t)(AES_BLOCK-(ch%AES_BLOCK)); if (ch%AES_BLOCK==0) pv=AES_BLOCK;
            for (size_t j=ch;j<pc;++j) pad[j]=pv;
            uint8_t siv[AES_BLOCK]; ComputeSha256((uint8_t*)&off, sizeof(off), siv);
            for (int j=0;j<AES_BLOCK;++j) siv[j]^=iv[j];
            if (!AesCbcEncrypt(key, siv, pad.data(), pc, encBuf)) { CloseHandle(h); SecureZeroMemory(key,32); errorMsg="Encryption failed (BCrypt error)"; return false; }
            SetFilePointerEx(h, li, NULL, FILE_BEGIN);
            WriteFile(h, encBuf.data(), (DWORD)encBuf.size(), &written, NULL);
            off += ch; rem -= ch;
            SecureWipe(buf); SecureWipe(pad); SecureWipe(encBuf);
        }
        SecureZeroMemory(key, 32); CloseHandle(h);
        return true;
    } catch (const std::exception& e) { errorMsg = e.what(); return false; }
    catch (...) { errorMsg = "Unknown error"; return false; }
}

bool FileCrypto::EncryptDisk(int diskNum, const std::string& password, std::string& keyPathOut, std::string& errorMsg, void (*progressCb)(const std::string&, size_t, size_t)) {
    g_abortEncrypt = false;
    if (IsDiskSystemDisk(diskNum)) { errorMsg = "Cannot encrypt system disk!"; return false; }
    std::string devPath = "\\\\.\\PhysicalDrive" + std::to_string(diskNum);
    uint64_t sz = GetDiskSize(diskNum);
    if (sz == 0) { errorMsg = "Disk inaccessible"; return false; }
    HANDLE h = CreateFileA(devPath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (h == INVALID_HANDLE_VALUE) { errorMsg = "Cannot open disk (run as Administrator)"; return false; }
    uint8_t salt[16], iv[AES_BLOCK]; RandBytes(salt,16); RandBytes(iv,AES_BLOCK);
    uint8_t key[32]; DeriveKey(password, salt, 16, key, 32);
    std::string apiKey = std::string(KEY_PREFIX) + Base64Encode(key, 32);
    keyPathOut = "D:\\disk" + std::to_string(diskNum) + "_recovery.key";
    SaveKeyFile(keyPathOut, apiKey);
    uint64_t total = sz - VOL_HDR;
    std::vector<uint8_t> hdr(VOL_HDR, 0);
    memcpy(hdr.data(), salt, 16); memcpy(hdr.data()+16, iv, AES_BLOCK);
    memcpy(hdr.data()+32, &total, 8); memcpy(hdr.data()+40, VOL_MAGIC, 8);
    DWORD written; SetFilePointer(h, 0, NULL, FILE_BEGIN); WriteFile(h, hdr.data(), VOL_HDR, &written, NULL);
    constexpr size_t BS = 1048576; std::vector<uint8_t> buf(BS), encBuf;
    uint64_t off = VOL_HDR, rem = total;
    while (rem > 0) {
        size_t ch = (size_t)(rem > BS ? BS : rem); DWORD red;
        LARGE_INTEGER li; li.QuadPart = off; SetFilePointerEx(h, li, NULL, FILE_BEGIN);
        if (!ReadFile(h, buf.data(), (DWORD)ch, &red, NULL) || red == 0) break;
        if (red < ch) ch = red;
        size_t padCh = ((ch / AES_BLOCK) + 1) * AES_BLOCK;
        std::vector<uint8_t> pad(padCh); memcpy(pad.data(), buf.data(), ch);
        uint8_t pv = (uint8_t)(AES_BLOCK - (ch % AES_BLOCK)); if (ch % AES_BLOCK == 0) pv = AES_BLOCK;
        for (size_t j = ch; j < padCh; ++j) pad[j] = pv;
        uint8_t siv[AES_BLOCK]; ComputeSha256((uint8_t*)&off, sizeof(off), siv);
        for (int j = 0; j < AES_BLOCK; ++j) siv[j] ^= iv[j];
        if (!AesCbcEncrypt(key, siv, pad.data(), padCh, encBuf)) { CloseHandle(h); SecureZeroMemory(key,32); errorMsg="Encryption failed (BCrypt error)"; return false; }
        SetFilePointerEx(h, li, NULL, FILE_BEGIN);
        WriteFile(h, encBuf.data(), (DWORD)encBuf.size(), &written, NULL);
        off += ch; rem -= ch;
        if (progressCb) progressCb("Encrypting", (size_t)(total - rem), (size_t)total);
        SecureWipe(buf); SecureWipe(pad); SecureWipe(encBuf);
    }
    SecureZeroMemory(key, 32); CloseHandle(h);
    return true;
}

bool FileCrypto::DecryptDisk(int diskNum, const std::string& password, std::string& errorMsg, void (*progressCb)(const std::string&, size_t, size_t)) {
    g_abortEncrypt = false;
    if (IsDiskSystemDisk(diskNum)) { errorMsg = "Cannot decrypt system disk!"; return false; }
    std::string devPath = "\\\\.\\PhysicalDrive" + std::to_string(diskNum);
    uint64_t sz = GetDiskSize(diskNum);
    if (sz == 0) { errorMsg = "Disk inaccessible"; return false; }
    HANDLE h = CreateFileA(devPath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (h == INVALID_HANDLE_VALUE) { errorMsg = "Cannot open disk (run as Administrator)"; return false; }
    std::vector<uint8_t> hdr(VOL_HDR); DWORD red;
    SetFilePointer(h, 0, NULL, FILE_BEGIN);
    if (!ReadFile(h, hdr.data(), VOL_HDR, &red, NULL) || red < VOL_HDR) { CloseHandle(h); errorMsg = "Cannot read header"; return false; }
    if (memcmp(hdr.data()+40, VOL_MAGIC, 8)) { CloseHandle(h); errorMsg = "Disk not encrypted by this tool"; return false; }
    uint8_t salt[16], iv[AES_BLOCK]; uint64_t total = 0;
    memcpy(salt, hdr.data(), 16); memcpy(iv, hdr.data()+16, AES_BLOCK);
    memcpy(&total, hdr.data()+32, 8);
    uint8_t key[32]; DeriveKey(password, salt, 16, key, 32);
    if (total == 0 || total > sz) total = sz - VOL_HDR;
    constexpr size_t BS = 1048576; std::vector<uint8_t> buf(BS + AES_BLOCK);
    uint64_t off = VOL_HDR, rem = total;
    while (rem > 0) {
        if (g_abortEncrypt) { CloseHandle(h); errorMsg = "Aborted"; return false; }
        size_t ch = (size_t)(rem > BS ? BS : rem);
        size_t readCh = ((ch + AES_BLOCK - 1) / AES_BLOCK) * AES_BLOCK;
        if (off + readCh > sz) readCh = (size_t)(sz - off);
        if (readCh == 0) break;
        LARGE_INTEGER li; li.QuadPart = off; SetFilePointerEx(h, li, NULL, FILE_BEGIN);
        DWORD toRead = (DWORD)readCh;
        if (!ReadFile(h, buf.data(), toRead, &red, NULL) || red == 0) break;
        if (red < readCh) readCh = red;
        uint8_t siv[AES_BLOCK]; ComputeSha256((uint8_t*)&off, sizeof(off), siv);
        for (int j = 0; j < AES_BLOCK; ++j) siv[j] ^= iv[j];
        std::vector<uint8_t> decBuf;
        if (!AesCbcDecrypt(key, siv, buf.data(), readCh, decBuf)) { CloseHandle(h); errorMsg = "Decryption failed (BCrypt error)"; SecureZeroMemory(key, 32); return false; }
        DbgLog("[DISK] dec ok, decBuf.size=%zu\n", decBuf.size());
        uint8_t pv = decBuf.back(); size_t ol = readCh;
        if (pv > 0 && pv <= AES_BLOCK) ol = readCh - pv;
        if (ol > ch) ol = ch;
        DbgLog("[DISK] pv=%u ol=%zu ch=%zu\n", pv, ol, ch);
        SetFilePointerEx(h, li, NULL, FILE_BEGIN);
        DWORD written; WriteFile(h, decBuf.data(), (DWORD)ol, &written, NULL);
        DbgLog("[DISK] wrote %lu bytes at off=%llu\n", written, off);
        off += ch; rem -= ch;
        if (progressCb) progressCb("Decrypting", (size_t)(total - rem), (size_t)total);
    }
    SecureZeroMemory(key, 32); CloseHandle(h);
    return true;
}

// ==== API KEY DISK OPERATIONS ====

bool FileCrypto::EncryptVolumeApi(const std::string& volume, std::string& keyPathOut, std::string& apiKeyOut, std::string& errorMsg, void (*progressCb)(const std::string&, size_t, size_t)) {
    try {
        g_abortEncrypt = false;
        if (!VerifySystemIntegrity()) { errorMsg = "System integrity check failed"; return false; }
        if (IsSystemDrive(volume)) { errorMsg = "Cannot encrypt system drive!"; return false; }
        uint64_t vsz = GetVolumeSize(volume);
        if (vsz == 0) { errorMsg = "Volume inaccessible"; return false; }
        HANDLE h = OpenVolumeLocked(volume, errorMsg);
        if (h == INVALID_HANDLE_VALUE) return false;
        uint8_t mk[64], iv[AES_BLOCK]; RandBytes(mk,64); RandBytes(iv,AES_BLOCK);
        apiKeyOut = std::string(KEY_PREFIX) + Base64Encode(mk, 64);
        keyPathOut = "D:\\" + std::string(1, (char)tolower((unsigned char)volume[0])) + "_api_recovery.key";
        SaveKeyFile(keyPathOut, apiKeyOut);
        uint64_t total = vsz - VOL_HDR;
        std::vector<uint8_t> hdr(VOL_HDR, 0);
        memcpy(hdr.data(), iv, AES_BLOCK);
        memcpy(hdr.data()+16, &total, 8); memcpy(hdr.data()+24, "CRYPTAPI", 8);
        DWORD w; SetFilePointer(h, 0, NULL, FILE_BEGIN); WriteFile(h, hdr.data(), VOL_HDR, &w, NULL);
        uint64_t off = VOL_HDR, rem = total;
        constexpr size_t BS = 1048576; std::vector<uint8_t> buf(BS), encBuf;
        while (rem > 0) {
            if (g_abortEncrypt) { CloseHandle(h); errorMsg="Aborted"; return false; }
            size_t ch = (size_t)(rem > BS ? BS : rem); DWORD red;
            LARGE_INTEGER li; li.QuadPart = off; SetFilePointerEx(h, li, NULL, FILE_BEGIN);
            if (!ReadFile(h, buf.data(), (DWORD)ch, &red, NULL) || red == 0) break;
            if (red < ch) ch = red;
            size_t pc = ((ch/AES_BLOCK)+1)*AES_BLOCK;
            std::vector<uint8_t> pad(pc); memcpy(pad.data(), buf.data(), ch);
            uint8_t pv = (uint8_t)(AES_BLOCK-(ch%AES_BLOCK)); if (ch%AES_BLOCK==0) pv=AES_BLOCK;
            for (size_t j=ch;j<pc;++j) pad[j]=pv;
            uint8_t siv[AES_BLOCK]; ComputeSha256((uint8_t*)&off, sizeof(off), siv);
            for (int j=0;j<AES_BLOCK;++j) siv[j]^=iv[j];
            if (!AesCbcEncrypt(mk, siv, pad.data(), pc, encBuf)) { CloseHandle(h); SecureZeroMemory(mk,64); errorMsg="Encryption failed (BCrypt error)"; return false; }
            SetFilePointerEx(h, li, NULL, FILE_BEGIN);
            WriteFile(h, encBuf.data(), (DWORD)encBuf.size(), &w, NULL);
            off += ch; rem -= ch;
            if (progressCb) progressCb("Encrypting", (size_t)(total - rem), (size_t)total);
            if (g_abortEncrypt) { CloseHandle(h); errorMsg="Aborted"; return false; }
            SecureWipe(buf); SecureWipe(pad); SecureWipe(encBuf);
        }
        SecureZeroMemory(mk, 64); CloseHandle(h);
        return true;
    } catch(const std::exception& e){errorMsg=e.what();return false;}
    catch(...){errorMsg="Unknown error";return false;}
}

bool FileCrypto::DecryptVolumeApi(const std::string& volume, const std::string& apiKey, std::string& errorMsg, void (*progressCb)(const std::string&, size_t, size_t)) {
    try {
        g_abortEncrypt = false;
        if (IsSystemDrive(volume)) { errorMsg = "Cannot decrypt system drive!"; return false; }
        if (apiKey.compare(0, strlen(KEY_PREFIX), KEY_PREFIX)) { errorMsg="Invalid API key"; return false; }
        auto kb = Base64Decode(apiKey.substr(strlen(KEY_PREFIX)));
        if (kb.size() != 64) { errorMsg="Wrong key length"; return false; }
        uint64_t vsz = GetVolumeSize(volume);
        if (vsz == 0) { errorMsg="Volume inaccessible"; return false; }
        HANDLE h = OpenVolumeLocked(volume, errorMsg);
        if (h == INVALID_HANDLE_VALUE) return false;
        std::vector<uint8_t> hdr(VOL_HDR); DWORD red;
        SetFilePointer(h, 0, NULL, FILE_BEGIN);
        if (!ReadFile(h, hdr.data(), VOL_HDR, &red, NULL) || red < VOL_HDR) { CloseHandle(h); errorMsg="Bad header"; return false; }
        if (memcmp(hdr.data()+24, "CRYPTAPI", 8)) { CloseHandle(h); errorMsg="Not API-key encrypted"; return false; }
        uint8_t iv[AES_BLOCK]; uint64_t total = 0;
        memcpy(iv, hdr.data(), AES_BLOCK);
        memcpy(&total, hdr.data()+16, 8);
        if (total == 0 || total > vsz) total = vsz - VOL_HDR;
        uint64_t off = VOL_HDR, rem = total;
        constexpr size_t BS = 1048576; std::vector<uint8_t> buf(BS + AES_BLOCK);
        while (rem > 0) {
            if (g_abortEncrypt) { CloseHandle(h); errorMsg="Aborted"; return false; }
            size_t ch = (size_t)(rem > BS ? BS : rem);
            size_t readCh = ((ch + AES_BLOCK - 1) / AES_BLOCK) * AES_BLOCK;
            if (off + readCh > vsz) readCh = (size_t)(vsz - off);
            if (readCh == 0) break;
            LARGE_INTEGER li; li.QuadPart = off; SetFilePointerEx(h, li, NULL, FILE_BEGIN);
            DWORD toRead = (DWORD)readCh;
            if (!ReadFile(h, buf.data(), toRead, &red, NULL) || red == 0) break;
            if (red < readCh) readCh = red;
            uint8_t siv[AES_BLOCK]; ComputeSha256((uint8_t*)&off, sizeof(off), siv);
            for (int j=0;j<AES_BLOCK;++j) siv[j]^=iv[j];
            std::vector<uint8_t> decBuf;
            if (!AesCbcDecrypt(kb.data(), siv, buf.data(), readCh, decBuf)) { CloseHandle(h); errorMsg = "Decryption failed (BCrypt error)"; SecureWipe(kb); return false; }
            DbgLog("[VAPI] dec ok, size=%zu\n", decBuf.size());
            uint8_t pv = decBuf.back(); size_t ol = readCh;
            if (pv > 0 && pv <= AES_BLOCK) ol = readCh - pv;
            if (ol > ch) ol = ch;
            DWORD w; SetFilePointerEx(h, li, NULL, FILE_BEGIN);
            WriteFile(h, decBuf.data(), (DWORD)ol, &w, NULL);
            off += ch; rem -= ch;
            if (progressCb) progressCb("Decrypting", (size_t)(total - rem), (size_t)total);
            if (g_abortEncrypt) { CloseHandle(h); errorMsg="Aborted"; return false; }
        }
        SecureWipe(kb); CloseHandle(h);
        return true;
    } catch(const std::exception& e){errorMsg=e.what();return false;}
    catch(...){errorMsg="Unknown error";return false;}
}

bool FileCrypto::EncryptDiskApi(int diskNum, std::string& keyPathOut, std::string& apiKeyOut, std::string& errorMsg, void (*progressCb)(const std::string&, size_t, size_t)) {
    try {
        g_abortEncrypt = false;
        if (IsDiskSystemDisk(diskNum)) { errorMsg = "Cannot encrypt system disk!"; return false; }
        std::string devPath = "\\\\.\\PhysicalDrive" + std::to_string(diskNum);
        uint64_t sz = GetDiskSize(diskNum);
        if (sz == 0) { errorMsg = "Disk inaccessible"; return false; }
        HANDLE h = CreateFileA(devPath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        if (h == INVALID_HANDLE_VALUE) { errorMsg = "Cannot open disk (run as Administrator)"; return false; }
        uint8_t mk[64], iv[AES_BLOCK]; RandBytes(mk, 64); RandBytes(iv, AES_BLOCK);
        apiKeyOut = std::string(KEY_PREFIX) + Base64Encode(mk, 64);
        keyPathOut = "D:\\disk" + std::to_string(diskNum) + "_api_recovery.key";
        SaveKeyFile(keyPathOut, apiKeyOut);
        uint64_t total = sz - VOL_HDR;
        std::vector<uint8_t> hdr(VOL_HDR, 0);
        memcpy(hdr.data(), iv, AES_BLOCK);
        memcpy(hdr.data()+16, &total, 8); memcpy(hdr.data()+24, "CRYPTAPI", 8);
        DWORD w; SetFilePointer(h, 0, NULL, FILE_BEGIN); WriteFile(h, hdr.data(), VOL_HDR, &w, NULL);
        uint64_t off = VOL_HDR, rem = total;
        constexpr size_t BS = 1048576; std::vector<uint8_t> buf(BS), encBuf;
        while (rem > 0) {
            if (g_abortEncrypt) { CloseHandle(h); errorMsg = "Aborted"; return false; }
            size_t ch = (size_t)(rem > BS ? BS : rem); DWORD red;
            LARGE_INTEGER li; li.QuadPart = off; SetFilePointerEx(h, li, NULL, FILE_BEGIN);
            if (!ReadFile(h, buf.data(), (DWORD)ch, &red, NULL) || red == 0) break;
            if (red < ch) ch = red;
            size_t pc = ((ch / AES_BLOCK) + 1) * AES_BLOCK;
            std::vector<uint8_t> pad(pc); memcpy(pad.data(), buf.data(), ch);
            uint8_t pv = (uint8_t)(AES_BLOCK - (ch % AES_BLOCK)); if (ch % AES_BLOCK == 0) pv = AES_BLOCK;
            for (size_t j = ch; j < pc; ++j) pad[j] = pv;
            uint8_t siv[AES_BLOCK]; ComputeSha256((uint8_t*)&off, sizeof(off), siv);
            for (int j = 0; j < AES_BLOCK; ++j) siv[j] ^= iv[j];
            if (!AesCbcEncrypt(mk, siv, pad.data(), pc, encBuf)) { CloseHandle(h); SecureZeroMemory(mk,64); errorMsg="Encryption failed (BCrypt error)"; return false; }
            SetFilePointerEx(h, li, NULL, FILE_BEGIN);
            WriteFile(h, encBuf.data(), (DWORD)encBuf.size(), &w, NULL);
            off += ch; rem -= ch;
            if (progressCb) progressCb("Encrypting", (size_t)(total - rem), (size_t)total);
            SecureWipe(buf); SecureWipe(pad); SecureWipe(encBuf);
        }
        SecureZeroMemory(mk, 64); CloseHandle(h);
        return true;
    } catch (const std::exception& e) { errorMsg = e.what(); return false; }
    catch (...) { errorMsg = "Unknown error"; return false; }
}

bool FileCrypto::DecryptDiskApi(int diskNum, const std::string& apiKey, std::string& errorMsg, void (*progressCb)(const std::string&, size_t, size_t)) {
    try {
        g_abortEncrypt = false;
        if (IsDiskSystemDisk(diskNum)) { errorMsg = "Cannot decrypt system disk!"; return false; }
        if (apiKey.compare(0, strlen(KEY_PREFIX), KEY_PREFIX)) { errorMsg = "Invalid API key"; return false; }
        auto kb = Base64Decode(apiKey.substr(strlen(KEY_PREFIX)));
        if (kb.size() != 64) { errorMsg = "Wrong key length"; return false; }
        std::string devPath = "\\\\.\\PhysicalDrive" + std::to_string(diskNum);
        uint64_t sz = GetDiskSize(diskNum);
        if (sz == 0) { errorMsg = "Disk inaccessible"; return false; }
        HANDLE h = CreateFileA(devPath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        if (h == INVALID_HANDLE_VALUE) { errorMsg = "Cannot open disk (run as Administrator)"; return false; }
        std::vector<uint8_t> hdr(VOL_HDR); DWORD red;
        SetFilePointer(h, 0, NULL, FILE_BEGIN);
        if (!ReadFile(h, hdr.data(), VOL_HDR, &red, NULL) || red < VOL_HDR) { CloseHandle(h); errorMsg = "Bad header"; return false; }
        if (memcmp(hdr.data() + 24, "CRYPTAPI", 8)) { CloseHandle(h); errorMsg = "Not API-key encrypted"; return false; }
        uint8_t iv[AES_BLOCK]; uint64_t total = 0;
        memcpy(iv, hdr.data(), AES_BLOCK);
        memcpy(&total, hdr.data()+16, 8);
        if (total == 0 || total > sz) total = sz - VOL_HDR;
        uint64_t off = VOL_HDR, rem = total;
        constexpr size_t BS = 1048576; std::vector<uint8_t> buf(BS + AES_BLOCK), decBuf;
        while (rem > 0) {
            if (g_abortEncrypt) { CloseHandle(h); errorMsg = "Aborted"; return false; }
            size_t ch = (size_t)(rem > BS ? BS : rem);
            size_t readCh = ((ch + AES_BLOCK - 1) / AES_BLOCK) * AES_BLOCK;
            if (off + readCh > sz) readCh = (size_t)(sz - off);
            if (readCh == 0) break;
            LARGE_INTEGER li; li.QuadPart = off; SetFilePointerEx(h, li, NULL, FILE_BEGIN);
            DWORD toRead = (DWORD)readCh;
            if (!ReadFile(h, buf.data(), toRead, &red, NULL) || red == 0) break;
            if (red < readCh) readCh = red;
            uint8_t siv[AES_BLOCK]; ComputeSha256((uint8_t*)&off, sizeof(off), siv);
            for (int j = 0; j < AES_BLOCK; ++j) siv[j] ^= iv[j];
            if (!AesCbcDecrypt(kb.data(), siv, buf.data(), readCh, decBuf)) { CloseHandle(h); errorMsg = "Decryption failed (BCrypt error)"; SecureWipe(kb); return false; }
            uint8_t pv = decBuf.back(); size_t ol = readCh;
            if (pv > 0 && pv <= AES_BLOCK) ol = readCh - pv;
            if (ol > ch) ol = ch;
            DWORD w; SetFilePointerEx(h, li, NULL, FILE_BEGIN);
            WriteFile(h, decBuf.data(), (DWORD)ol, &w, NULL);
            off += ch; rem -= ch;
            if (progressCb) progressCb("Decrypting", (size_t)(total - rem), (size_t)total);
            SecureWipe(buf); SecureWipe(decBuf);
        }
        SecureWipe(kb); CloseHandle(h);
        return true;
    } catch (const std::exception& e) { errorMsg = e.what(); return false; }
    catch (...) { errorMsg = "Unknown error"; return false; }
}