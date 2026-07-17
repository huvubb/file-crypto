#ifndef I18N_HPP
#define I18N_HPP

#include <string>
#include <unordered_map>
#include <vector>

enum class StrKey {
    TITLE, SELECT_LANG, MAIN_MENU,
    ENCRYPT, DECRYPT, CHANGE_LANG, EXIT,
    ENTER_CHOICE, INVALID_CHOICE,
    INPUT_FILE, OUTPUT_FILE,
    ENTER_PASSWORD, CONFIRM_PASSWORD,
    PASSWORD_MISMATCH,
    ENC_SUCCESS, DEC_SUCCESS, ENC_FAILED, DEC_FAILED,
    FILE_NOT_FOUND, WRONG_PASS, CONFIG_SAVED, CONFIG_LOADED,
    GOODBYE, PRESS_ENTER, ERROR_PREFIX,
    LANG_PROMPT, ENC_PROMPT, DEC_PROMPT,
    CONTACT_INFO, WELCOME_CONTRIB
};

class I18n {
public:
    struct LangInfo {
        std::string code;
        std::string name;
    };

    static const std::vector<LangInfo>& GetLanguages();
    static const std::string& Get(StrKey key);
    static void SetLanguage(const std::string& code);
    static const std::string& CurrentCode();

private:
    static std::string currentLang_;
    static std::unordered_map<StrKey, std::string> currentStrings_;
    static std::unordered_map<StrKey, std::string> GetStrings(const std::string& code);
};

#endif
