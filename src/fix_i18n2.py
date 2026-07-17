path = r"C:\Users\邓涛\Documents\New project 3\file-crypto\src\i18n.cpp"
with open(path, "r", encoding="utf-8") as f:
    content = f.read()

# Add core methods after currentStrings_
old = "std::unordered_map<StrKey, std::string> I18n::currentStrings_;\n\n"
new = """std::unordered_map<StrKey, std::string> I18n::currentStrings_;

const std::string& I18n::Get(StrKey key) {
    auto it = currentStrings_.find(key);
    if (it != currentStrings_.end()) return it->second;
    static const std::string empty;
    return empty;
}

void I18n::SetLanguage(const std::string& code) {
    currentLang_ = code;
    currentStrings_ = GetStrings(code);
}

const std::string& I18n::CurrentCode() {
    return currentLang_;
}

"""
content = content.replace(old, new)
with open(path, "w", encoding="utf-8") as f:
    f.write(content)
print("Added core I18n methods")
