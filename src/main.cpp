#include "i18n.hpp"
#include "config.hpp"
#include "crypto.hpp"
#include <iostream>
#include <string>
#include <limits>
#include <windows.h>

static void ClearScreen() {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(h, &csbi)) return;
    SMALL_RECT r = {0, 0, csbi.dwSize.X, csbi.dwSize.Y};
    COORD t = {0, static_cast<SHORT>(-csbi.dwSize.Y)};
    CHAR_INFO fill; fill.Char.UnicodeChar = ' '; fill.Attributes = csbi.wAttributes;
    ScrollConsoleScreenBuffer(h, &r, NULL, t, &fill);
    csbi.dwCursorPosition.X = 0; csbi.dwCursorPosition.Y = 0;
    SetConsoleCursorPosition(h, csbi.dwCursorPosition);
}

static void Pause() {
    std::cout << "\n" << I18n::Get(StrKey::PRESS_ENTER);
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

static void ShowLanguageMenu() {
    ClearScreen();
    std::cout << I18n::Get(StrKey::TITLE) << "\n\n";
    std::cout << I18n::Get(StrKey::SELECT_LANG) << "\n\n";
    const auto& langs = I18n::GetLanguages();
    for (size_t i = 0; i < langs.size(); ++i)
        std::cout << "  " << (i + 1) << ". " << langs[i].name << "\n";
    std::cout << "\n" << I18n::Get(StrKey::LANG_PROMPT);
    int choice; std::cin >> choice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    if (choice >= 1 && choice <= static_cast<int>(langs.size())) {
        I18n::SetLanguage(langs[choice - 1].code);
        Config::SaveLanguage(langs[choice - 1].code);
        std::cout << "\n" << I18n::Get(StrKey::CONFIG_SAVED) << "\n";
        Pause();
    }
}

// 1. Password encrypt
static void DoEncrypt() {
    ClearScreen();
    std::cout << "=== " << I18n::Get(StrKey::ENC_PROMPT) << " ===\n\n";
    std::string in, out, pass, confirm;
    std::cout << I18n::Get(StrKey::INPUT_FILE); std::getline(std::cin, in);
    std::cout << I18n::Get(StrKey::OUTPUT_FILE); std::getline(std::cin, out);
    std::cout << I18n::Get(StrKey::ENTER_PASSWORD); std::getline(std::cin, pass);
    std::cout << I18n::Get(StrKey::CONFIRM_PASSWORD); std::getline(std::cin, confirm);
    if (pass != confirm) {
        std::cout << "\n" << I18n::Get(StrKey::PASSWORD_MISMATCH) << "\n"; Pause(); return;
    }
    std::string err;
    if (FileCrypto::EncryptFile(in, out, pass, err))
        std::cout << "\n" << I18n::Get(StrKey::ENC_SUCCESS) << "\n";
    else
        std::cout << "\n" << I18n::Get(StrKey::ENC_FAILED) << err << "\n";
    Pause();
}

// 2. API Key encrypt (no password)
static void DoApiEncrypt() {
    ClearScreen();
    std::cout << "=== " << I18n::Get(StrKey::ENC_KEYFILE) << " ===\n\n";
    std::string in, out;
    std::cout << I18n::Get(StrKey::INPUT_FILE); std::getline(std::cin, in);
    std::cout << I18n::Get(StrKey::OUTPUT_FILE); std::getline(std::cin, out);

    std::string apiKey, err;
    if (FileCrypto::GenerateKeyEncrypt(in, out, apiKey, err)) {
        std::cout << "\n" << I18n::Get(StrKey::ENC_SUCCESS) << "\n";
        std::cout << "\n========== " << I18n::Get(StrKey::KEYFILE_SAVED) << " ==========\n";
        std::cout << apiKey << "\n";
        std::cout << "========================================\n";
        std::cout << I18n::Get(StrKey::WELCOME_CONTRIB) << "\n";
    } else {
        std::cout << "\n" << I18n::Get(StrKey::ENC_FAILED) << err << "\n";
    }
    Pause();
}

// 3. Password decrypt
static void DoDecrypt() {
    ClearScreen();
    std::cout << "=== " << I18n::Get(StrKey::DEC_PROMPT) << " ===\n\n";
    std::string in, out, pass;
    std::cout << I18n::Get(StrKey::INPUT_FILE); std::getline(std::cin, in);
    std::cout << I18n::Get(StrKey::OUTPUT_FILE); std::getline(std::cin, out);
    std::cout << I18n::Get(StrKey::ENTER_PASSWORD); std::getline(std::cin, pass);
    std::string err;
    if (FileCrypto::DecryptFile(in, out, pass, err))
        std::cout << "\n" << I18n::Get(StrKey::DEC_SUCCESS) << "\n";
    else
        std::cout << "\n" << I18n::Get(StrKey::DEC_FAILED) << err << "\n";
    Pause();
}

// 4. API Key decrypt
static void DoApiDecrypt() {
    ClearScreen();
    std::cout << "=== " << I18n::Get(StrKey::DEC_KEYFILE) << " ===\n\n";
    std::string in, out, apiKey;
    std::cout << I18n::Get(StrKey::INPUT_FILE); std::getline(std::cin, in);
    std::cout << I18n::Get(StrKey::OUTPUT_FILE); std::getline(std::cin, out);
    std::cout << I18n::Get(StrKey::KEY_FILE_PATH); std::getline(std::cin, apiKey);

    std::string err;
    if (FileCrypto::KeyDecrypt(in, out, apiKey, err))
        std::cout << "\n" << I18n::Get(StrKey::DEC_SUCCESS) << "\n";
    else
        std::cout << "\n" << I18n::Get(StrKey::DEC_FAILED) << err << "\n";
    Pause();
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    if (Config::Exists()) {
        I18n::SetLanguage(Config::LoadLanguage());
    } else {
        I18n::SetLanguage("en");
        ShowLanguageMenu();
    }

    while (true) {
        ClearScreen();
        std::cout << I18n::Get(StrKey::TITLE) << "\n\n";
        std::cout << I18n::Get(StrKey::MAIN_MENU) << "\n\n";
        std::cout << "  " << I18n::Get(StrKey::ENCRYPT) << "\n";
        std::cout << "  " << I18n::Get(StrKey::ENC_KEYFILE) << "\n";
        std::cout << "  " << I18n::Get(StrKey::DECRYPT) << "\n";
        std::cout << "  " << I18n::Get(StrKey::DEC_KEYFILE) << "\n";
        std::cout << "  " << I18n::Get(StrKey::CHANGE_LANG) << "\n";
        std::cout << "  " << I18n::Get(StrKey::EXIT) << "\n";
        std::cout << "\n" << I18n::Get(StrKey::ENTER_CHOICE);

        int choice; std::cin >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (choice) {
            case 1: DoEncrypt(); break;
            case 2: DoApiEncrypt(); break;
            case 3: DoDecrypt(); break;
            case 4: DoApiDecrypt(); break;
            case 5: ShowLanguageMenu(); break;
            case 6:
                std::cout << "\n" << I18n::Get(StrKey::GOODBYE) << "\n";
                return 0;
            default:
                std::cout << "\n" << I18n::Get(StrKey::INVALID_CHOICE) << "\n";
                Pause(); break;
        }
    }
}
