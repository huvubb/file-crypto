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

static std::mt19937 gRng(static_cast<unsigned>(
    std::chrono::steady_clock::now().time_since_epoch().count()));

static std::string RandomSuffix() {
    static const char chars[] = "abcdefghijklmnopqrstuvwxyz0123456789";
    std::string s(6, ' ');
    std::uniform_int_distribution<int> dist(0, 35);
    for (int i = 0; i < 6; ++i) s[i] = chars[dist(gRng)];
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
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

static void ShowLanguageMenu() {
    ClearScreen();
    std::cout << I18n::Get(StrKey::SELECT_LANG) << "\n\n";
    const auto& langs = I18n::GetLanguages();
    for (size_t i = 0; i < langs.size(); ++i)
        std::cout << "  " << (i + 1) << ". " << langs[i].name << "\n";
    std::cout << "\n" << I18n::Get(StrKey::LANG_PROMPT);
    int c; std::cin >> c;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
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
    std::cout << I18n::Get(StrKey::INPUT_FILE); std::getline(std::cin, in);
    std::string out = MakeEncPath(in);
    std::cout << I18n::Get(StrKey::ENTER_PASSWORD); std::getline(std::cin, pass);
    std::cout << I18n::Get(StrKey::CONFIRM_PASSWORD); std::getline(std::cin, confirm);
    if (pass != confirm) { std::cout << "\n" << I18n::Get(StrKey::PASSWORD_MISMATCH) << "\n"; Pause(); return; }
    std::string err;
    if (FileCrypto::EncryptFile(in, out, pass, err))
        std::cout << "\n" << I18n::Get(StrKey::ENC_SUCCESS) << " -> " << out << "\n";
    else
        std::cout << "\n" << I18n::Get(StrKey::ENC_FAILED) << err << "\n";
    Pause();
}

static void DoApiEncrypt() {
    ClearScreen();
    std::cout << "=== " << I18n::Get(StrKey::ENC_KEYFILE) << " ===\n\n";
    std::string in;
    std::cout << I18n::Get(StrKey::INPUT_FILE); std::getline(std::cin, in);
    std::string out = MakeEncPath(in);
    std::string apiKey, err;
    if (FileCrypto::GenerateKeyEncrypt(in, out, apiKey, err)) {
        std::cout << "\n" << I18n::Get(StrKey::ENC_SUCCESS) << " -> " << out << "\n";
        std::cout << "\n========== " << I18n::Get(StrKey::KEYFILE_SAVED) << " ==========\n";
        std::cout << apiKey << "\n";
        std::cout << "========================================\n";
        std::cout << I18n::Get(StrKey::WELCOME_CONTRIB) << "\n";
    } else {
        std::cout << "\n" << I18n::Get(StrKey::ENC_FAILED) << err << "\n";
    }
    Pause();
}

static void DoDecrypt() {
    ClearScreen();
    std::cout << "=== " << I18n::Get(StrKey::DEC_PROMPT) << " ===\n\n";
    std::string in, pass;
    std::cout << I18n::Get(StrKey::INPUT_FILE); std::getline(std::cin, in);
    std::string out = MakeDecPath(in);
    std::cout << I18n::Get(StrKey::ENTER_PASSWORD); std::getline(std::cin, pass);
    std::string err;
    if (FileCrypto::DecryptFile(in, out, pass, err))
        std::cout << "\n" << I18n::Get(StrKey::DEC_SUCCESS) << " -> " << out << "\n";
    else
        std::cout << "\n" << I18n::Get(StrKey::DEC_FAILED) << err << "\n";
    Pause();
}

static void DoApiDecrypt() {
    ClearScreen();
    std::cout << "=== " << I18n::Get(StrKey::DEC_KEYFILE) << " ===\n\n";
    std::string in, apiKey;
    std::cout << I18n::Get(StrKey::INPUT_FILE); std::getline(std::cin, in);
    std::string out = MakeDecPath(in);
    std::cout << I18n::Get(StrKey::KEY_FILE_PATH); std::getline(std::cin, apiKey);
    std::string err;
    if (FileCrypto::KeyDecrypt(in, out, apiKey, err))
        std::cout << "\n" << I18n::Get(StrKey::DEC_SUCCESS) << " -> " << out << "\n";
    else
        std::cout << "\n" << I18n::Get(StrKey::DEC_FAILED) << err << "\n";
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

        int c; std::cin >> c;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
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
