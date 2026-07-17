#include "i18n.hpp"
#include "config.hpp"
#include "crypto.hpp"
#include <iostream>
#include <string>
#include <limits>
#include <random>
#include <chrono>
#include <windows.h>

#ifdef EncryptFile
#undef EncryptFile
#endif
#ifdef DecryptFile
#undef DecryptFile
#endif


static void ShowProgress(const std::string& label, size_t done, size_t total) {
    if (total == 0) return;
    int pct = static_cast<int>(done * 100 / total);
    static int lastPct = -1;
    if (pct != lastPct && pct % 10 == 0) {
        lastPct = pct;
        std::cout << "\r" << label << " " << pct << "%" << std::flush;
    }
    if (done >= total) { std::cout << "\r" << label << " 100%\n"; lastPct = -1; }
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
        std::cout << "\n" << I18n::Get(StrKey::ENC_SUCCESS) << " -> " << out << "\n";
        std::cout << "\n========== " << I18n::Get(StrKey::KEYFILE_SAVED) << " ==========\n";
        std::cout << apiKey << "\n";
        std::cout << "========================================\n";
        std::cout << I18n::Get(StrKey::WELCOME_CONTRIB) << "\n";
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
    std::cout << I18n::Get(StrKey::KEY_FILE_PATH); apiKey = ReadLineUtf8();
    std::string err;
    if (FileCrypto::KeyDecrypt(in, out, apiKey, err))
        std::cout << "\n" << I18n::Get(StrKey::DEC_SUCCESS) << " -> " << out << "\n";
    else
        std::cout << "\n" << I18n::Get(StrKey::DEC_FAILED) << err << " [" << in << "]" << "\n";
    Pause();
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    if (Config::Exists()) I18n::SetLanguage(Config::LoadLanguage());
    else { I18n::SetLanguage("en"); ShowLanguageMenu(); }

    while (true) {
        ClearScreen();
        std::cout << I18n::Get(StrKey::TITLE) << "\n\n";
        std::cout << "  " << I18n::Get(StrKey::ENCRYPT) << "\n";
        std::cout << "  " << I18n::Get(StrKey::ENC_KEYFILE) << "\n";
        std::cout << "  " << I18n::Get(StrKey::DECRYPT) << "\n";
        std::cout << "  " << I18n::Get(StrKey::DEC_KEYFILE) << "\n";
        std::cout << "  " << I18n::Get(StrKey::CHANGE_LANG) << "\n";
        std::cout << "  " << I18n::Get(StrKey::EXIT) << "\n\n";
        std::cout << I18n::Get(StrKey::ENTER_CHOICE);

        int c = ReadInt();
        switch (c) {
            case 1: DoEncrypt(); break;
            case 2: DoApiEncrypt(); break;
            case 3: DoDecrypt(); break;
            case 4: DoApiDecrypt(); break;
            case 5: ShowLanguageMenu(); break;
            case 6: std::cout << "\n" << I18n::Get(StrKey::GOODBYE) << "\n"; return 0;
            default: std::cout << "\n" << I18n::Get(StrKey::INVALID_CHOICE) << "\n"; Pause();
        }
    }
}
