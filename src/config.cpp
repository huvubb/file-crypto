#include "config.hpp"
#include <fstream>
#include <windows.h>

bool Config::Exists() {
    DWORD attr = GetFileAttributesA(CONFIG_PATH);
    return (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
}

std::string Config::LoadLanguage() {
    std::ifstream file(CONFIG_PATH);
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
    std::ofstream file(CONFIG_PATH, std::ios::trunc);
    if (!file) return false;
    file << "language=" << langCode << "\n";
    return true;
}
