#include "config.hpp"
#include <fstream>
#include <windows.h>
#include <filesystem>

static std::filesystem::path ToWideCfg(const std::string& s) {
    if (s.empty()) return std::filesystem::path();
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, NULL, 0);
    if (len <= 1) return std::filesystem::path(s);
    std::wstring ws(len, L'\x00');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &ws[0], len);
    ws.resize(len - 1);
    return std::filesystem::path(ws);
}

bool Config::Exists() {
    DWORD attr = GetFileAttributesA(CONFIG_PATH);
    return (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
}

std::string Config::LoadLanguage() {
    std::ifstream file(ToWideCfg(CONFIG_PATH));
    if (!file) return "en";

    std::string line;
    while (std::getline(file, line)) {
        if (line.rfind("language=", 0) == 0) {
            return line.substr(9);
        }
    }
    return "en";
}

bool Config::SaveLanguage(const std::string& langCode) {
    std::ofstream file(ToWideCfg(CONFIG_PATH), std::ios::trunc);
    if (!file) return false;
    file << "language=" << langCode << "\n";
    return true;
}
