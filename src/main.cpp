#include "i18n.hpp"
#include "config.hpp"
#include "crypto.hpp"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <limits>
#include <random>
#include <chrono>
#include <windows.h>
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")

#ifdef EncryptFile
#undef EncryptFile
#endif
#ifdef DecryptFile
#undef DecryptFile
#endif


static std::string FormatSize(size_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int u = 0;
    double s = (double)bytes;
    while (s >= 1024.0 && u < 4) { s /= 1024.0; u++; }
    char buf[64];
    if (u == 0) snprintf(buf, sizeof(buf), "%.0f %s", s, units[u]);
    else snprintf(buf, sizeof(buf), "%.1f %s", s, units[u]);
    return std::string(buf);
}

static void OpLog(const std::string& stage, const std::string& target) {
    bool cn = (I18n::CurrentCode() == "zh-CN");
    if (stage == "open") {
        if (cn) std::cout << "  [LOG] 正在打开 " << target << " ..." << std::endl;
        else    std::cout << "  [LOG] Opening " << target << " ..." << std::endl;
    } else if (stage == "hdr") {
        if (cn) std::cout << "  [LOG] 正在读取加密头..." << std::endl;
        else    std::cout << "  [LOG] Reading encryption header..." << std::endl;
    } else if (stage == "enc") {
        if (cn) std::cout << "  [LOG] 正在加密 " << target << " ..." << std::endl;
        else    std::cout << "  [LOG] Encrypting " << target << " ..." << std::endl;
    } else if (stage == "dec") {
        if (cn) std::cout << "  [LOG] 正在解密 " << target << " ..." << std::endl;
        else    std::cout << "  [LOG] Decrypting " << target << " ..." << std::endl;
    } else if (stage == "done") {
        if (cn) std::cout << "  [LOG] 操作完成" << std::endl;
        else    std::cout << "  [LOG] Operation complete" << std::endl;
    } else if (stage == "err") {
        if (cn) std::cout << "  [LOG] 错误: " << target << std::endl;
        else    std::cout << "  [LOG] Error: " << target << std::endl;
    }
}

static void ResetProgress() {
    // Clears the static state so progress bar works across operations
    // Called before each crypto operation
}

static void ShowProgress(const std::string& label, size_t done, size_t total) {
    if (total == 0) return;
    int pct = static_cast<int>(done * 100 / total);
    static int lastPct = -1;
    static std::string lastLabel;
    if (lastLabel != label) { lastPct = -1; lastLabel = label; }
    if (pct == lastPct) return;
    lastPct = pct;
    const int barW = 30;
    int filled = barW * pct / 100;
    std::string bar = "[" + std::string(filled, char(219)) + std::string(barW - filled, char(176)) + "]";
    std::cout << "\r  " << label << " " << bar << " " << pct << "% ("
              << FormatSize(done) << "/" << FormatSize(total) << ")    " << std::flush;
    if (done >= total) { std::cout << "\n"; lastPct = -1; }
}

static std::string ReadLineUtf8() {
    std::wstring ws;
    HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
    DWORD read;
    WCHAR buf[4096];
    while (ReadConsoleW(h, buf, 4096, &read, NULL) && read > 0) {
        ws.append(buf, read);
        if (buf[read - 1] == L'\n') break;
    }
    while (!ws.empty() && (ws.back() == L'\r' || ws.back() == L'\n'))
        ws.pop_back();
    int len = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, NULL, 0, NULL, NULL);
    if (len <= 1) return "";
    std::string result(len - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, &result[0], len, NULL, NULL);
    return result;
}

static int ReadInt() {
    std::string s = ReadLineUtf8();
    try { return std::stoi(s); }
    catch (...) { return -1; }
}


static std::string TrimPath(const std::string& raw) {
    std::string s = raw;
    while (!s.empty() && (s.front() == ' ' || s.front() == 9)) s.erase(0, 1);
    while (!s.empty()) {
        char c = s.back();
        if (c == ' ' || c == 9 || c == 10 || c == 13) s.pop_back();
        else break;
    }
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"')
        s = s.substr(1, s.size() - 2);
    return s;
}

static std::string RandomSuffix() {
    static std::mt19937 rng(static_cast<unsigned>(
        std::chrono::steady_clock::now().time_since_epoch().count()));
    static const char chars[] = "abcdefghijklmnopqrstuvwxyz0123456789";
    std::string s(6, ' ');
    std::uniform_int_distribution<int> dist(0, 35);
    for (int i = 0; i < 6; ++i) s[i] = chars[dist(rng)];
    return s;
}

// Encrypt: input.txt -> input_a3x9k1.txt.enc
static std::string MakeEncPath(const std::string& input) {
    auto pos = input.find_last_of(".");
    if (pos != std::string::npos && pos > input.find_last_of("\\/")) {
        return input.substr(0, pos) + "_" + RandomSuffix() + input.substr(pos) + ".enc";
    }
    return input + "_" + RandomSuffix() + ".enc";
}

// Decrypt: input.txt.enc -> input.txt
static std::string MakeDecPath(const std::string& input) {
    if (input.size() > 4) {
        std::string lower = input;
        for (auto& c : lower) c = static_cast<char>(::tolower(static_cast<unsigned char>(c)));
        if (lower.compare(lower.size() - 4, 4, ".enc") == 0) {
            std::string result = input.substr(0, input.size() - 4);
            // Also strip the random suffix if present: file_a3x9k1.ext -> file.ext
            auto pos = result.find_last_of("_");
            auto dot = result.find_last_of(".");
            if (pos != std::string::npos && dot != std::string::npos && pos < dot) {
                // Check if the part between _ and . looks like random chars (6 alnum)
                std::string middle = result.substr(pos + 1, dot - pos - 1);
                if (middle.size() == 6) {
                    bool allAlnum = true;
                    for (char c : middle) {
                        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))) {
                            allAlnum = false; break;
                        }
                    }
                    if (allAlnum) return result.substr(0, pos) + result.substr(dot);
                }
            }
            return result;
        }
    }
    return input + ".decrypted";
}

static void ClearScreen() {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(h, &csbi)) return;
    SMALL_RECT r = {0, 0, csbi.dwSize.X, csbi.dwSize.Y};
    COORD t = {0, static_cast<SHORT>(-csbi.dwSize.Y)};
    CHAR_INFO fill; fill.Char.UnicodeChar = ' '; fill.Attributes = csbi.wAttributes;
    ScrollConsoleScreenBuffer(h, &r, NULL, t, &fill);
    SetConsoleCursorPosition(h, {0, 0});
}

static void Pause() {
    std::cout << "\n" << I18n::Get(StrKey::PRESS_ENTER);
    ReadLineUtf8();
}

static void ShowLanguageMenu() {
    ClearScreen();
    std::cout << I18n::Get(StrKey::SELECT_LANG) << "\n\n";
    const auto& langs = I18n::GetLanguages();
    for (size_t i = 0; i < langs.size(); ++i)
        std::cout << "  " << (i + 1) << ". " << langs[i].name << "\n";
    std::cout << "\n" << I18n::Get(StrKey::LANG_PROMPT);
    int c = ReadInt();
    if (c >= 1 && c <= (int)langs.size()) {
        I18n::SetLanguage(langs[c-1].code);
        Config::SaveLanguage(langs[c-1].code);
        std::cout << "\n" << I18n::Get(StrKey::CONFIG_SAVED) << "\n"; Pause();
    }
}

static void DoEncrypt() {
    ClearScreen();
    std::cout << "=== " << I18n::Get(StrKey::ENC_PROMPT) << " ===\n\n";
    std::string in, pass, confirm;
    std::cout << I18n::Get(StrKey::INPUT_FILE); in = ReadLineUtf8();
    in = TrimPath(in);
    in = TrimPath(in);
    in = TrimPath(in);
    in = TrimPath(in);
    std::string out = MakeEncPath(in);
    std::cout << I18n::Get(StrKey::ENTER_PASSWORD); pass = ReadLineUtf8();
    std::cout << I18n::Get(StrKey::CONFIRM_PASSWORD); confirm = ReadLineUtf8();
    if (pass != confirm) { std::cout << "\n" << I18n::Get(StrKey::PASSWORD_MISMATCH) << "\n"; Pause(); return; }
    std::string err;
    if (FileCrypto::EncryptFile(in, out, pass, err))
        std::cout << "\n" << I18n::Get(StrKey::ENC_SUCCESS) << " -> " << out << "\n";
    else
        std::cout << "\n" << I18n::Get(StrKey::ENC_FAILED) << err << " [" << in << "]" << "\n";
    Pause();
}

static void DoApiEncrypt() {
    ClearScreen();
    std::cout << "=== " << I18n::Get(StrKey::ENC_KEYFILE) << " ===\n\n";
    std::string in;
    std::cout << I18n::Get(StrKey::INPUT_FILE); in = ReadLineUtf8();
    in = TrimPath(in);
    in = TrimPath(in);
    in = TrimPath(in);
    in = TrimPath(in);
    std::string out = MakeEncPath(in);
    std::string apiKey, err;
    if (FileCrypto::GenerateKeyEncrypt(in, out, apiKey, err)) {
        std::string keyPath = out + ".key";
        FileCrypto::SaveKeyFile(keyPath, apiKey);
        std::cout << "\n" << I18n::Get(StrKey::ENC_SUCCESS) << " -> " << out << "\n";
        std::cout << "Key saved -> " << keyPath << "\n";
    } else {
        std::cout << "\n" << I18n::Get(StrKey::ENC_FAILED) << err << " [" << in << "]" << "\n";
    }
    Pause();
}

static void DoDecrypt() {
    ClearScreen();
    std::cout << "=== " << I18n::Get(StrKey::DEC_PROMPT) << " ===\n\n";
    std::string in, pass;
    std::cout << I18n::Get(StrKey::INPUT_FILE); in = ReadLineUtf8();
    in = TrimPath(in);
    in = TrimPath(in);
    in = TrimPath(in);
    in = TrimPath(in);
    std::string out = MakeDecPath(in);
    std::cout << I18n::Get(StrKey::ENTER_PASSWORD); pass = ReadLineUtf8();
    std::string err;
    if (FileCrypto::DecryptFile(in, out, pass, err))
        std::cout << "\n" << I18n::Get(StrKey::DEC_SUCCESS) << " -> " << out << "\n";
    else
        std::cout << "\n" << I18n::Get(StrKey::DEC_FAILED) << err << " [" << in << "]" << "\n";
    Pause();
}

static void DoApiDecrypt() {
    ClearScreen();
    std::cout << "=== " << I18n::Get(StrKey::DEC_KEYFILE) << " ===\n\n";
    std::string in, apiKey;
    std::cout << I18n::Get(StrKey::INPUT_FILE); in = ReadLineUtf8();
    in = TrimPath(in);
    in = TrimPath(in);
    in = TrimPath(in);
    in = TrimPath(in);
    std::string out = MakeDecPath(in);
    std::string keyPath = in + ".key";
    if (!FileCrypto::LoadKeyFile(keyPath, apiKey)) {
        std::cout << I18n::Get(StrKey::KEY_FILE_PATH); apiKey = ReadLineUtf8();
    }
    std::string err;
    if (FileCrypto::KeyDecrypt(in, out, apiKey, err))
        std::cout << "\n" << I18n::Get(StrKey::DEC_SUCCESS) << " -> " << out << "\n";
    else
        std::cout << "\n" << I18n::Get(StrKey::DEC_FAILED) << err << " [" << in << "]" << "\n";
    Pause();
}


static void DoVolumeEncrypt() {
    ClearScreen();
    std::cout << "=== " << I18n::Get(StrKey::PART_ENC) << " ===\n\n";
    std::cout << I18n::Get(StrKey::PART_WARN) << "\n\n";
    std::cout << "  " << I18n::Get(StrKey::DISK_MODE_PW) << "\n  " << I18n::Get(StrKey::DISK_MODE_API) << "\n  " << I18n::Get(StrKey::DISK_MODE_FAST) << "\n\n" << I18n::Get(StrKey::PART_CHOICE);
    int crMode = ReadInt();
    if (crMode < 1 || crMode > 3) { Pause(); return; }
    std::cout << "\n  " << I18n::Get(StrKey::PART_LOGICAL_MODE) << "\n  " << I18n::Get(StrKey::PART_PHYSICAL_MODE) << "\n\n" << I18n::Get(StrKey::PART_CHOICE);
    int mode = ReadInt();
    std::string pass, apiKey, keyPath, err, target;
    if (mode == 1) {
        auto vols = FileCrypto::GetVolumes();
        std::cout << "\n" << I18n::Get(StrKey::PART_AVAIL_VOL) << "\n";
        for (size_t i = 0; i < vols.size(); ++i) {
            bool sys = FileCrypto::IsSystemDrive(vols[i]);
            uint64_t sz = FileCrypto::GetVolumeSize(vols[i]);
            std::cout << "  " << (i+1) << ". " << vols[i] << " (" << (sz/1073741824.0) << I18n::Get(StrKey::PART_GB) << ")";
            if (sys) std::cout << I18n::Get(StrKey::PART_SYS_BLOCKED);
            std::cout << "\n";
        }
        if (vols.empty()) { std::cout << "  --\n"; Pause(); return; }
        std::cout << "\n" << I18n::Get(StrKey::PART_SELECT) << " (1-" << vols.size() << "): ";
        int c = ReadInt(); if (c < 1 || c > (int)vols.size()) { Pause(); return; }
        target = vols[c-1];
        if (FileCrypto::IsSystemDrive(target)) { std::cout << "\n" << I18n::Get(StrKey::PART_CANT_SYS) << "\n"; Pause(); return; }
        std::cout << "\n" << I18n::Get(StrKey::PART_CONFIRM) << " " << target << ": ";
        std::string confirm = ReadLineUtf8();
        if (confirm != "YES" && confirm != "yes") { std::cout << I18n::Get(StrKey::PART_CANCELLED) << "\n"; Pause(); return; }
        if (crMode == 1 || crMode == 3) { std::cout << I18n::Get(StrKey::ENTER_PASSWORD); pass = ReadLineUtf8(); }
        OpLog("enc", target);
        std::cout << "[ESC to abort]\n";
        bool ok; size_t maxB = (crMode == 3) ? 268435456 : 0;
        if (crMode == 2) ok = FileCrypto::EncryptVolumeApi(target, keyPath, apiKey, err, ShowProgress);
        else if (crMode == 3) ok = FileCrypto::EncryptVolumeFast(target, pass, maxB, keyPath, err);
        else ok = FileCrypto::EncryptVolume(target, pass, keyPath, err, ShowProgress);
        if (ok) { std::cout << "\n" << I18n::Get(StrKey::PART_DONE) << " " << I18n::Get(StrKey::PART_RECOVERY) << ": " << keyPath << "\n"; }
        else std::cout << "\n" << I18n::Get(StrKey::ERROR_PREFIX) << err << "\n";
    } else if (mode == 2) {
        auto disks = FileCrypto::GetPhysicalDisks();
        std::cout << "\n" << I18n::Get(StrKey::PART_AVAIL_DISK) << "\n";
        for (size_t i = 0; i < disks.size(); ++i) {
            bool sys = FileCrypto::IsDiskSystemDisk(disks[i]);
            uint64_t sz = FileCrypto::GetDiskSize(disks[i]);
            std::cout << "  " << (i+1) << ". Disk " << disks[i] << " (" << (sz/1073741824.0) << I18n::Get(StrKey::PART_GB) << ")";
            if (sys) std::cout << I18n::Get(StrKey::PART_SYS_BLOCKED);
            std::cout << "\n";
        }
        if (disks.empty()) { std::cout << "  --\n"; Pause(); return; }
        std::cout << "\n" << I18n::Get(StrKey::PART_SELECT) << " (1-" << disks.size() << "): ";
        int c = ReadInt(); if (c < 1 || c > (int)disks.size()) { Pause(); return; }
        int dn = disks[c-1];
        if (FileCrypto::IsDiskSystemDisk(dn)) { std::cout << "\n" << I18n::Get(StrKey::PART_CANT_SYS) << "\n"; Pause(); return; }
        std::cout << "\n" << I18n::Get(StrKey::PART_CONFIRM) << " Disk " << dn << ": ";
        std::string confirm = ReadLineUtf8();
        if (confirm != "YES" && confirm != "yes") { std::cout << I18n::Get(StrKey::PART_CANCELLED) << "\n"; Pause(); return; }
        if (crMode == 1 || crMode == 3) { std::cout << I18n::Get(StrKey::ENTER_PASSWORD); pass = ReadLineUtf8(); }
        std::cout << "\n[LOG] Disk " << dn << " - " << I18n::Get(StrKey::PART_ENCRYPTING) << "\n[ESC to abort]\n";
        bool ok;
        if (crMode == 2) ok = FileCrypto::EncryptDiskApi(dn, keyPath, apiKey, err, ShowProgress);
        else if (crMode == 3) { std::string dev = "\\\\.\\PhysicalDrive" + std::to_string(dn); ok = FileCrypto::EncryptVolumeFast(dev, pass, 268435456, keyPath, err); }
        else ok = FileCrypto::EncryptDisk(dn, pass, keyPath, err, ShowProgress);
        if (ok) { std::cout << "\n" << I18n::Get(StrKey::PART_DONE) << " " << I18n::Get(StrKey::PART_RECOVERY) << ": " << keyPath << "\n"; }
        else std::cout << "\n" << I18n::Get(StrKey::ERROR_PREFIX) << err << "\n";
    }
    Pause();
}

static void DoVolumeDecrypt() {
    ClearScreen();
    std::cout << "=== " << I18n::Get(StrKey::PART_DEC) << " ===\n\n";
    std::cout << "  " << I18n::Get(StrKey::DISK_MODE_PW) << "\n  " << I18n::Get(StrKey::DISK_MODE_API) << "\n\n" << I18n::Get(StrKey::PART_CHOICE);
    int crMode = ReadInt();
    if (crMode < 1 || crMode > 2) { Pause(); return; }
    std::cout << "\n  " << I18n::Get(StrKey::PART_LOGICAL_MODE) << "\n  " << I18n::Get(StrKey::PART_PHYSICAL_MODE) << "\n\n" << I18n::Get(StrKey::PART_CHOICE);
    int mode = ReadInt();
    std::string pass, apiKey, err, target;
    if (mode == 1) {
        auto vols = FileCrypto::GetVolumes();
        std::cout << "\n" << I18n::Get(StrKey::PART_AVAIL_VOL) << "\n";
        for (size_t i = 0; i < vols.size(); ++i) std::cout << "  " << (i+1) << ". " << vols[i] << "\n";
        if (vols.empty()) { Pause(); return; }
        std::cout << "\n" << I18n::Get(StrKey::PART_SELECT) << " (1-" << vols.size() << "): ";
        int c = ReadInt(); if (c < 1 || c > (int)vols.size()) { Pause(); return; }
        target = vols[c-1];
        std::cout << "\n" << I18n::Get(StrKey::PART_CONFIRM) << " " << target << ": ";
        std::string confirm = ReadLineUtf8();
        if (confirm != "YES" && confirm != "yes") { std::cout << I18n::Get(StrKey::PART_CANCELLED) << "\n"; Pause(); return; }
        if (crMode == 1) { std::cout << I18n::Get(StrKey::ENTER_PASSWORD); pass = ReadLineUtf8(); }
        else {
            std::string kp = "D:\\" + std::string(1, (char)tolower((unsigned char)target[0])) + "_api_recovery.key";
            if (!FileCrypto::LoadKeyFile(kp, apiKey)) { std::cout << I18n::Get(StrKey::KEY_FILE_PATH); apiKey = ReadLineUtf8(); }
        }
        OpLog("dec", target);
        std::cout << "[ESC to abort]\n";
        bool ok = (crMode == 1) ? FileCrypto::DecryptVolume(target, pass, err, ShowProgress) : FileCrypto::DecryptVolumeApi(target, apiKey, err, ShowProgress);
        if (ok) std::cout << "\n" << I18n::Get(StrKey::PART_DONE) << "\n";
        else std::cout << "\n" << I18n::Get(StrKey::ERROR_PREFIX) << err << "\n";
    } else if (mode == 2) {
        auto disks = FileCrypto::GetPhysicalDisks();
        std::cout << "\n" << I18n::Get(StrKey::PART_AVAIL_DISK) << "\n";
        for (size_t i = 0; i < disks.size(); ++i) std::cout << "  " << (i+1) << ". Disk " << disks[i] << "\n";
        if (disks.empty()) { Pause(); return; }
        std::cout << "\n" << I18n::Get(StrKey::PART_SELECT) << " (1-" << disks.size() << "): ";
        int c = ReadInt(); if (c < 1 || c > (int)disks.size()) { Pause(); return; }
        int dn = disks[c-1];
        std::cout << "\n" << I18n::Get(StrKey::PART_CONFIRM) << " Disk " << dn << ": ";
        std::string confirm = ReadLineUtf8();
        if (confirm != "YES" && confirm != "yes") { std::cout << I18n::Get(StrKey::PART_CANCELLED) << "\n"; Pause(); return; }
        if (crMode == 1) { std::cout << I18n::Get(StrKey::ENTER_PASSWORD); pass = ReadLineUtf8(); }
        else {
            std::string kp = "D:\\disk" + std::to_string(dn) + "_api_recovery.key";
            if (!FileCrypto::LoadKeyFile(kp, apiKey)) { std::cout << I18n::Get(StrKey::KEY_FILE_PATH); apiKey = ReadLineUtf8(); }
        }
        std::string diskLabel2 = "Disk " + std::to_string(dn);
        OpLog("dec", diskLabel2);
        std::cout << "[ESC to abort]\n";
        bool ok = (crMode == 1) ? FileCrypto::DecryptDisk(dn, pass, err, ShowProgress) : FileCrypto::DecryptDiskApi(dn, apiKey, err, ShowProgress);
        if (ok) std::cout << "\n" << I18n::Get(StrKey::PART_DONE) << "\n";
        else std::cout << "\n" << I18n::Get(StrKey::ERROR_PREFIX) << err << "\n";
    }
    Pause();
}



// === 崩溃处理 / Crash Handler ===
static const char* GetExceptionName(DWORD code) {
    switch (code) {
        case 0xC0000005: return "访问违规 (Access Violation)";
        case 0xC00000FD: return "栈溢出 (Stack Overflow)";
        case 0xC000001D: return "非法指令 (Illegal Instruction)";
        case 0xC0000008: return "无效句柄 (Invalid Handle)";
        case 0xC0000017: return "内存不足 (Out of Memory)";
        case 0xC0000094: return "除零错误 (Divide by Zero)";
        case 0xC0000096: return "特权指令 (Privileged Instruction)";
        default: return "未知异常 (Unknown Exception)";
    }
}

static LONG WINAPI MyCrashHandler(EXCEPTION_POINTERS* ep) {
    DWORD code = ep->ExceptionRecord->ExceptionCode;
    void* addr = ep->ExceptionRecord->ExceptionAddress;
    bool cn = (I18n::CurrentCode() == "zh-CN" || I18n::CurrentCode() == "zh-TW");
    
    if (cn) {
        std::cerr << "\n========================================" << std::endl;
        std::cerr << "  *** 程序崩溃 ***" << std::endl;
        std::cerr << "========================================" << std::endl;
        std::cerr << "  异常类型: " << GetExceptionName(code) << std::endl;
    } else {
        std::cerr << "\n========================================" << std::endl;
        std::cerr << "  *** CRASH DETECTED ***" << std::endl;
        std::cerr << "========================================" << std::endl;
        std::cerr << "  Exception: " << GetExceptionName(code) << std::endl;
    }
    
    char buf[128];
    snprintf(buf, sizeof(buf), "  错误代码: 0x%08X  地址: 0x%p", code, addr);
    std::cerr << buf << std::endl;
    
    HMODULE hMod;
    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQuery(addr, &mbi, sizeof(mbi))) {
        hMod = (HMODULE)mbi.AllocationBase;
        WCHAR modName[MAX_PATH];
        if (GetModuleFileNameW(hMod, modName, MAX_PATH)) {
            std::wcerr << L"  模块: " << modName << L"\n";
        }
    }
    
    if (cn) {
        std::cerr << "\n  可能原因:" << std::endl;
        if (code == 0xC0000005) {
            std::cerr << "  - 密钥不匹配或文件已损坏" << std::endl;
            std::cerr << "  - 磁盘/分区未由此程序加密" << std::endl;
            std::cerr << "  - 请确认选择了正确的加密模式(密码/API Key)" << std::endl;
        }
        std::cerr << "\n  按 Enter 退出..." << std::endl;
    } else {
        std::cerr << "\n  Possible causes:" << std::endl;
        if (code == 0xC0000005) {
            std::cerr << "  - Wrong password or corrupted data" << std::endl;
            std::cerr << "  - Disk not encrypted by this tool" << std::endl;
            std::cerr << "  - Check encryption mode (Password vs API Key)" << std::endl;
        }
        std::cerr << "\n  Press Enter to exit..." << std::endl;
    }
    
    std::cin.get();
    return EXCEPTION_EXECUTE_HANDLER;
}

static void InstallCrashHandler() {
    SetUnhandledExceptionFilter(MyCrashHandler);
    _set_purecall_handler([]() {
        std::cerr << "\n*** PURE VIRTUAL CALL ***\nPress Enter...";
        std::cin.get();
    });
    _set_invalid_parameter_handler([](const wchar_t*, const wchar_t*, const wchar_t*, unsigned int, uintptr_t) {
        std::cerr << "\n*** INVALID PARAMETER ***\nPress Enter...";
        std::cin.get();
    });
}
int main() {
    InstallCrashHandler();
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    if (Config::Exists()) I18n::SetLanguage(Config::LoadLanguage());
    else { I18n::SetLanguage("en"); ShowLanguageMenu(); }

    while (true) {
        ClearScreen();
        std::cout << I18n::Get(StrKey::TITLE) << " v2.0\n";
        std::cout << I18n::Get(StrKey::DISCLAIMER) << "\n\n";
        std::cout << "  " << I18n::Get(StrKey::ENCRYPT) << "\n";
        std::cout << "  " << I18n::Get(StrKey::ENC_KEYFILE) << "\n";
        std::cout << "  " << I18n::Get(StrKey::DECRYPT) << "\n";
        std::cout << "  " << I18n::Get(StrKey::DEC_KEYFILE) << "\n";
        std::cout << "  " << I18n::Get(StrKey::CHANGE_LANG) << "\n";
        std::cout << "  " << I18n::Get(StrKey::PART_ENC) << "\n";
        std::cout << "  " << I18n::Get(StrKey::PART_DEC) << "\n";
        std::cout << "  " << I18n::Get(StrKey::EXIT) << "\n\n";
        std::cout << I18n::Get(StrKey::ENTER_CHOICE);

        int c = ReadInt();
        switch (c) {
            case 1: DoEncrypt(); break;
            case 2: DoApiEncrypt(); break;
            case 3: DoDecrypt(); break;
            case 4: DoApiDecrypt(); break;
            case 5: ShowLanguageMenu(); break;
            case 7: DoVolumeEncrypt(); break;
            case 8: DoVolumeDecrypt(); break;
            
            
            case 12: std::cout << "\n" << I18n::Get(StrKey::GOODBYE) << "\n"; return 0;
            default: std::cout << "\n" << I18n::Get(StrKey::INVALID_CHOICE) << "\n"; Pause();
        }
    }
}
