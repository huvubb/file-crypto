import json, os

with open(r"C:\Users\邓涛\Documents\New project 3\file-crypto\src\lang_data.json", "r", encoding="utf-8") as f:
    langs = json.load(f)

keys = ["TITLE","SELECT_LANG","MAIN_MENU",
        "ENCRYPT","ENC_KEYFILE","DECRYPT","DEC_KEYFILE",
        "CHANGE_LANG","EXIT",
        "ENTER_CHOICE","INVALID_CHOICE","INPUT_FILE","OUTPUT_FILE","KEY_FILE_PATH",
        "ENTER_PASSWORD","CONFIRM_PASSWORD","PASSWORD_MISMATCH",
        "ENC_SUCCESS","DEC_SUCCESS","ENC_FAILED","DEC_FAILED","KEYFILE_SAVED",
        "FILE_NOT_FOUND","WRONG_PASS","CONFIG_SAVED","CONFIG_LOADED",
        "GOODBYE","PRESS_ENTER","ERROR_PREFIX","LANG_PROMPT","ENC_PROMPT","DEC_PROMPT",
        "CONTACT_INFO","WELCOME_CONTRIB"]

def esc(s):
    return s.replace(chr(92), chr(92)+chr(92)).replace('"', chr(92)+'"').replace("\n", chr(92)+"n").replace("\r", chr(92)+"r")

lines = []
lines.append('#include "i18n.hpp"')
lines.append("")
lines.append('std::string I18n::currentLang_ = "en";')
lines.append("std::unordered_map<StrKey, std::string> I18n::currentStrings_;")
lines.append("")

for code in sorted(langs.keys()):
    cname = "Make_lang_" + code.replace("-", "_")
    lines.append(f"static std::unordered_map<StrKey, std::string> {cname}() {{")
    lines.append("    return {")
    for key in keys:
        val = langs[code].get(key, langs["en"][key])
        lines.append(f'        {{StrKey::{key}, "{esc(val)}"}},')
    lines.append("    };")
    lines.append("}")
    lines.append("")

lines.append("std::unordered_map<StrKey, std::string> I18n::GetStrings(const std::string& code) {")
for code in sorted(langs.keys()):
    cname = "Make_lang_" + code.replace("-", "_")
    lines.append(f'    if (code == "{code}") return {cname}();')
lines.append('    return Make_lang_en();')
lines.append("}")

out = r"C:\Users\邓涛\Documents\New project 3\file-crypto\src\i18n.cpp"
with open(out, "w", encoding="utf-8") as f:
    f.write("\n".join(lines))

size = os.path.getsize(out)
print(f"Generated i18n.cpp: {size:,} bytes, {len(langs)} languages, {len(keys)} keys each")
