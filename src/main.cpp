#include "i18n.hpp"
#include "config.hpp"
#include "crypto.hpp"
#include <iostream>
#include <string>
#include <limits>
#include <windows.h>

static void ClearScreen() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    SMALL_RECT scrollRect;
    COORD scrollTarget;
    CHAR_INFO fill;
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) return;
    scrollRect.Left = 0;
    scrollRect.Top = 0;
    scrollRect.Right = csbi.dwSize.X;
    scrollRect.Bottom = csbi.dwSize.Y;
    scrollTarget.X = 0;
    scrollTarget.Y = static_cast<SHORT>(-csbi.dwSize.Y);
    fill.Char.UnicodeChar = ' ';
    fill.Attributes = csbi.wAttributes;
    ScrollConsoleScreenBuffer(hConsole, &scrollRect, NULL, scrollTarget, &fill);
    csbi.dwCursorPosition.X = 0;
    csbi.dwCursorPosition.Y = 0;
    SetConsoleCursorPosition(hConsole, csbi.dwCursorPosition);
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
    for (size_t i = 0; i < langs.size(); ++i) {
        std::cout << "  " << (i + 1) << ". " << langs[i].name << "\n";
    }
    std::cout << "\n" << I18n::Get(StrKey::LANG_PROMPT);

    int choice;
    std::cin >> choice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (choice >= 1 && choice <= static_cast<int>(langs.size())) {
        std::string code = langs[choice - 1].code;
        I18n::SetLanguage(code);
        Config::SaveLanguage(code);
        std::cout << "\n" << I18n::Get(StrKey::CONFIG_SAVED) << "\n";
        Pause();
    }
}

static void DoEncrypt() {
    ClearScreen();
    std::cout << "=== " << I18n::Get(StrKey::ENC_PROMPT) << " ===\n\n";

    std::string inputPath, outputPath, password, confirm;

    std::cout << I18n::Get(StrKey::INPUT_FILE);
    std::getline(std::cin, inputPath);

    std::cout << I18n::Get(StrKey::OUTPUT_FILE);
    std::getline(std::cin, outputPath);

    std::cout << I18n::Get(StrKey::ENTER_PASSWORD);
    std::getline(std::cin, password);

    std::cout << I18n::Get(StrKey::CONFIRM_PASSWORD);
    std::getline(std::cin, confirm);

    if (password != confirm) {
        std::cout << "\n" << I18n::Get(StrKey::PASSWORD_MISMATCH) << "\n";
        Pause();
        return;
    }

    std::string error;
    if (FileCrypto::EncryptFile(inputPath, outputPath, password, error)) {
        std::cout << "\n" << I18n::Get(StrKey::ENC_SUCCESS) << "\n";
    } else {
        std::cout << "\n" << I18n::Get(StrKey::ENC_FAILED) << error << "\n";
    }
    Pause();
}

static void DoDecrypt() {
    ClearScreen();
    std::cout << "=== " << I18n::Get(StrKey::DEC_PROMPT) << " ===\n\n";

    std::string inputPath, outputPath, password;

    std::cout << I18n::Get(StrKey::INPUT_FILE);
    std::getline(std::cin, inputPath);

    std::cout << I18n::Get(StrKey::OUTPUT_FILE);
    std::getline(std::cin, outputPath);

    std::cout << I18n::Get(StrKey::ENTER_PASSWORD);
    std::getline(std::cin, password);

    std::string error;
    if (FileCrypto::DecryptFile(inputPath, outputPath, password, error)) {
        std::cout << "\n" << I18n::Get(StrKey::DEC_SUCCESS) << "\n";
    } else {
        std::cout << "\n" << I18n::Get(StrKey::DEC_FAILED) << error << "\n";
    }
    Pause();
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    if (Config::Exists()) {
        std::string savedLang = Config::LoadLanguage();
        I18n::SetLanguage(savedLang);
    } else {
        I18n::SetLanguage("en");
        ShowLanguageMenu();
    }

    while (true) {
        ClearScreen();
        std::cout << I18n::Get(StrKey::TITLE) << "\n\n";
        std::cout << I18n::Get(StrKey::MAIN_MENU) << "\n\n";
        std::cout << "  " << I18n::Get(StrKey::ENCRYPT) << "\n";
        std::cout << "  " << I18n::Get(StrKey::DECRYPT) << "\n";
        std::cout << "  " << I18n::Get(StrKey::CHANGE_LANG) << "\n";
        std::cout << "  " << I18n::Get(StrKey::EXIT) << "\n";
        std::cout << "\n" << I18n::Get(StrKey::ENTER_CHOICE);

        int choice;
        std::cin >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (choice) {
            case 1: DoEncrypt(); break;
            case 2: DoDecrypt(); break;
            case 3: ShowLanguageMenu(); break;
            case 4:
                std::cout << "\n" << I18n::Get(StrKey::GOODBYE) << "\n";
                return 0;
            default:
                std::cout << "\n" << I18n::Get(StrKey::INVALID_CHOICE) << "\n";
                Pause();
                break;
        }
    }
    return 0;
}
