#ifndef I18N_HPP
#define I18N_HPP

#include <string>
#include <unordered_map>
#include <vector>

enum class StrKey {
    TITLE, SELECT_LANG, MAIN_MENU,
    ENCRYPT, DECRYPT, CHANGE_LANG, EXIT,
    ENC_KEYFILE, DEC_KEYFILE,
    KEY_FILE_PATH,
    ENTER_CHOICE, INVALID_CHOICE,
    INPUT_FILE, OUTPUT_FILE,
    ENTER_PASSWORD, CONFIRM_PASSWORD,
    PASSWORD_MISMATCH,
    ENC_SUCCESS, DEC_SUCCESS, ENC_FAILED, DEC_FAILED,
    KEYFILE_SAVED,
    FILE_NOT_FOUND, WRONG_PASS, CONFIG_SAVED, CONFIG_LOADED,
    GOODBYE, PRESS_ENTER, ERROR_PREFIX,
    LANG_PROMPT, ENC_PROMPT, DEC_PROMPT,
    CONTACT_INFO, WELCOME_CONTRIB,
    DISCLAIMER,
    PART_ENC, PART_DEC, PART_WARN, PART_LOGICAL_MODE, PART_PHYSICAL_MODE, PART_SYS_BLOCKED, PART_CANT_SYS, PART_CONFIRM, PART_CANCELLED, PART_ENCRYPTING, PART_DECRYPTING, PART_RECOVERY, PART_AVAIL_VOL, PART_AVAIL_DISK, PART_SELECT, PART_GB, PART_DONE, PART_CHOICE, DISK_SECTION, DISK_MODE_PW, DISK_MODE_API, DISK_MODE_FAST
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
