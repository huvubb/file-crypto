#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>

class Config {
public:
    static constexpr const char* CONFIG_PATH = "D:\\file_crypto_config.ini";

    static bool Exists();
    static std::string LoadLanguage();
    static bool SaveLanguage(const std::string& langCode);
};

#endif
