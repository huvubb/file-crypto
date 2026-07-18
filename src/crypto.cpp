#include "crypto.hpp"
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

// --- BCrypt helpers ---
static void RandBytes(uint8_t* buf, size_t len) { BCryptGenRandom(NULL, buf, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG); }

static BCRYPT_ALG_HANDLE OpenAes() {
    BCRYPT_ALG_HANDLE h;
    BCryptOpenAlgorithmProvider(&h, BCRYPT_AES_ALGORITHM, NULL, 0);
    BCryptSetProperty(h, BCRYPT_CHAINING_MODE, (PUCHAR)BCRYPT_CHAIN_MODE_CBC, sizeof(BCRYPT_CHAIN_MODE_CBC), 0);
    return h;
}

static BCRYPT_KEY_HANDLE CreateAesKey(BCRYPT_ALG_HANDLE alg, const uint8_t* key, size_t len) {
    BCRYPT_KEY_HANDLE hk;
    BCryptGenerateSymmetricKey(alg, &hk, NULL, 0, (PUCHAR)key, (ULONG)len, 0);
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

// --- AES-256-CBC encrypt (IV-safe) ---
static bool AesCbcEncrypt(const uint8_t* key, const uint8_t* iv, const uint8_t* in, size_t inLen, std::vector<uint8_t>& out) {
    uint8_t ivCopy[AES_BLOCK];
    memcpy(ivCopy, iv, AES_BLOCK);
    BCRYPT_ALG_HANDLE alg = OpenAes();
    BCRYPT_KEY_HANDLE hk = CreateAesKey(alg, key, 32);
    ULONG outLen;
    BCryptEncrypt(hk, (PUCHAR)in, (ULONG)inLen, NULL, (PUCHAR)ivCopy, AES_BLOCK, NULL, 0, &outLen, 0);
    out.resize(outLen);
    memcpy(ivCopy, iv, AES_BLOCK);
    BCryptEncrypt(hk, (PUCHAR)in, (ULONG)inLen, NULL, (PUCHAR)ivCopy, AES_BLOCK, out.data(), outLen, &outLen, 0);
    BCryptDestroyKey(hk);
    BCryptCloseAlgorithmProvider(alg, 0);
    SecureZeroMemory(ivCopy, AES_BLOCK);
    return true;
}

// --- AES-256-CBC decrypt (IV-safe) ---
static bool AesCbcDecrypt(const uint8_t* key, const uint8_t* iv, const uint8_t* in, size_t inLen, std::vector<uint8_t>& out) {
    uint8_t ivCopy[AES_BLOCK];
    memcpy(ivCopy, iv, AES_BLOCK);
    BCRYPT_ALG_HANDLE alg = OpenAes();
    BCRYPT_KEY_HANDLE hk = CreateAesKey(alg, key, 32);
    ULONG outLen;
    BCryptDecrypt(hk, (PUCHAR)in, (ULONG)inLen, NULL, (PUCHAR)ivCopy, AES_BLOCK, NULL, 0, &outLen, 0);
    out.resize(outLen);
    memcpy(ivCopy, iv, AES_BLOCK);
    BCryptDecrypt(hk, (PUCHAR)in, (ULONG)inLen, NULL, (PUCHAR)ivCopy, AES_BLOCK, out.data(), outLen, &outLen, 0);
    BCryptDestroyKey(hk);
    BCryptCloseAlgorithmProvider(alg, 0);
    SecureZeroMemory(ivCopy, AES_BLOCK);
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
        AesCbcEncrypt(key,iv,buf.data(),padLen,out);

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
        AesCbcEncrypt(masterKey,iv,buf.data(),padLen,out);

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

bool FileCrypto::IsSystemDrive(const std::string& vol) {
    WCHAR sysRoot[MAX_PATH];
    GetWindowsDirectoryW(sysRoot, MAX_PATH);
    char sysLetter = (char)sysRoot[0];
    return (vol.size() >= 1 && (vol[0] == sysLetter || vol[0] == (sysLetter + 32)));
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
        errorMsg = "Cannot open volume " + vol + " (run as Administrator)";
        return INVALID_HANDLE_VALUE;
    }
    DWORD bytes;
    if (!DeviceIoControl(h, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &bytes, NULL)) {
        DeviceIoControl(h, FSCTL_DISMOUNT_VOLUME, NULL, 0, NULL, 0, &bytes, NULL);
        Sleep(500);
        if (!DeviceIoControl(h, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &bytes, NULL)) {
            CloseHandle(h);
            errorMsg = "Cannot lock volume - close all programs using it";
            return INVALID_HANDLE_VALUE;
        }
    }
    return h;
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

            AesCbcEncrypt(key, sectorIv, padded.data(), padChunk, encBuf);

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

        if (memcmp(hdr.data() + 32, VOL_MAGIC, 8) != 0) {
            CloseHandle(h);
            errorMsg = "Volume is not encrypted by this tool";
            return false;
        }

        uint8_t salt[16], iv[AES_BLOCK];
        memcpy(salt, hdr.data(), 16);
        memcpy(iv, hdr.data() + 16, AES_BLOCK);
        uint8_t key[32];
        DeriveKey(password, salt, 16, key, 32);

        constexpr size_t BUF_SIZE = 1048576;
        std::vector<uint8_t> buf(BUF_SIZE);
        std::vector<uint8_t> decBuf;

        uint64_t offset = VOL_HDR;
        uint64_t remaining = volSize - VOL_HDR;
        uint64_t totalToDecrypt = remaining;

        while (remaining > 0) {
            size_t chunk = (size_t)(remaining > BUF_SIZE ? BUF_SIZE : remaining);
            LARGE_INTEGER li; li.QuadPart = offset;
            SetFilePointerEx(h, li, NULL, FILE_BEGIN);
            if (!ReadFile(h, buf.data(), (DWORD)chunk, &red, NULL) || red == 0) break;
            if (red < chunk) chunk = red;

            uint8_t sectorIv[AES_BLOCK];
            ComputeSha256((uint8_t*)&offset, sizeof(offset), sectorIv);
            for (int j = 0; j < AES_BLOCK; ++j) sectorIv[j] ^= iv[j];

            AesCbcDecrypt(key, sectorIv, buf.data(), chunk, decBuf);

            uint8_t pv = decBuf.back();
            size_t origLen = chunk;
            if (pv > 0 && pv <= AES_BLOCK) origLen = decBuf.size() - pv;

            SetFilePointerEx(h, li, NULL, FILE_BEGIN);
            DWORD written;
            WriteFile(h, decBuf.data(), (DWORD)origLen, &written, NULL);

            offset += chunk;
            remaining -= chunk;

            if (progressCb) progressCb("Decrypting", (size_t)(totalToDecrypt - remaining), (size_t)totalToDecrypt);
            SecureWipe(buf); SecureWipe(decBuf);
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
    // Check all volumes for system DLLs and verify disk membership
    WCHAR volName[MAX_PATH];
    HANDLE volFind = FindFirstVolumeW(volName, MAX_PATH);
    if (volFind == INVALID_HANDLE_VALUE) return false;
    bool found = false;
    do {
        WCHAR paths[MAX_PATH]; DWORD len;
        if (GetVolumePathNamesForVolumeNameW(volName, paths, MAX_PATH, &len) && paths[0]) {
            // Verify system DLLs
            char drive = (char)paths[0];
            std::string ntos = std::string(1, drive) + ":\\Windows\\System32\\ntoskrnl.exe";
            std::string ntdll = std::string(1, drive) + ":\\Windows\\System32\\ntdll.dll";
            DWORD a1 = GetFileAttributesA(ntos.c_str());
            DWORD a2 = GetFileAttributesA(ntdll.c_str());
            if (a1 != INVALID_FILE_ATTRIBUTES && a2 != INVALID_FILE_ATTRIBUTES) {
                // System volume confirmed - check disk
                std::wstring vn(volName); vn.pop_back();
                HANDLE vh = CreateFileW(vn.c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
                if (vh != INVALID_HANDLE_VALUE) {
                    VOLUME_DISK_EXTENTS ext; DWORD rb;
                    if (DeviceIoControl(vh, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, NULL, 0, &ext, sizeof(ext), &rb, NULL)) {
                        for (DWORD j = 0; j < ext.NumberOfDiskExtents; ++j) {
                            if ((int)ext.Extents[j].DiskNumber == diskNum) { found = true; }
                        }
                    }
                    CloseHandle(vh);
                }
            }
        }
    } while (!found && FindNextVolumeW(volFind, volName, MAX_PATH));
    FindVolumeClose(volFind);
    return found;
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

bool FileCrypto::EncryptDisk(int diskNum, const std::string& password, std::string& keyPathOut, std::string& errorMsg, void (*progressCb)(const std::string&, size_t, size_t)) {
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
    std::vector<uint8_t> hdr(VOL_HDR, 0);
    memcpy(hdr.data(), salt, 16); memcpy(hdr.data()+16, iv, AES_BLOCK); memcpy(hdr.data()+32, VOL_MAGIC, 8);
    DWORD written; SetFilePointer(h, 0, NULL, FILE_BEGIN); WriteFile(h, hdr.data(), VOL_HDR, &written, NULL);
    constexpr size_t BS = 1048576; std::vector<uint8_t> buf(BS), encBuf;
    uint64_t off = VOL_HDR, rem = sz - VOL_HDR, total = rem;
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
        AesCbcEncrypt(key, siv, pad.data(), padCh, encBuf);
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
    std::string devPath = "\\\\.\\PhysicalDrive" + std::to_string(diskNum);
    uint64_t sz = GetDiskSize(diskNum);
    if (sz == 0) { errorMsg = "Disk inaccessible"; return false; }
    HANDLE h = CreateFileA(devPath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (h == INVALID_HANDLE_VALUE) { errorMsg = "Cannot open disk (run as Administrator)"; return false; }
    std::vector<uint8_t> hdr(VOL_HDR); DWORD red;
    SetFilePointer(h, 0, NULL, FILE_BEGIN);
    if (!ReadFile(h, hdr.data(), VOL_HDR, &red, NULL) || red < VOL_HDR) { CloseHandle(h); errorMsg = "Cannot read header"; return false; }
    if (memcmp(hdr.data()+32, VOL_MAGIC, 8)) { CloseHandle(h); errorMsg = "Disk not encrypted by this tool"; return false; }
    uint8_t salt[16], iv[AES_BLOCK]; memcpy(salt, hdr.data(), 16); memcpy(iv, hdr.data()+16, AES_BLOCK);
    uint8_t key[32]; DeriveKey(password, salt, 16, key, 32);
    constexpr size_t BS = 1048576; std::vector<uint8_t> buf(BS), decBuf;
    uint64_t off = VOL_HDR, rem = sz - VOL_HDR, total = rem;
    while (rem > 0) {
        size_t ch = (size_t)(rem > BS ? BS : rem);
        LARGE_INTEGER li; li.QuadPart = off; SetFilePointerEx(h, li, NULL, FILE_BEGIN);
        if (!ReadFile(h, buf.data(), (DWORD)ch, &red, NULL) || red == 0) break;
        if (red < ch) ch = red;
        uint8_t siv[AES_BLOCK]; ComputeSha256((uint8_t*)&off, sizeof(off), siv);
        for (int j = 0; j < AES_BLOCK; ++j) siv[j] ^= iv[j];
        AesCbcDecrypt(key, siv, buf.data(), ch, decBuf);
        uint8_t pv = decBuf.back(); size_t ol = ch;
        if (pv > 0 && pv <= AES_BLOCK) ol = decBuf.size() - pv;
        SetFilePointerEx(h, li, NULL, FILE_BEGIN);
        DWORD written; WriteFile(h, decBuf.data(), (DWORD)ol, &written, NULL);
        off += ch; rem -= ch;
        if (progressCb) progressCb("Decrypting", (size_t)(total - rem), (size_t)total);
        SecureWipe(buf); SecureWipe(decBuf);
    }
    SecureZeroMemory(key, 32); CloseHandle(h);
    return true;
}
