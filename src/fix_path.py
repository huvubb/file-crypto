path = r"C:\Users\邓涛\Documents\New project 3\file-crypto\src\main.cpp"
with open(path, "r", encoding="utf-8") as f:
    content = f.read()

# Add TrimPath helper after the includes
marker = 'static std::string RandomSuffix()'
helper = '''static std::string TrimPath(const std::string& raw) {
    std::string s = raw;
    // Trim leading/trailing whitespace
    while (!s.empty() && (s.front() == ' ' || s.front() == '\t')) s.erase(0, 1);
    while (!s.empty() && (s.back() == ' ' || s.back() == '\t' || s.back() == '\r')) s.pop_back();
    // Strip surrounding quotes
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"') s = s.substr(1, s.size() - 2);
    return s;
}

'''
content = content.replace(marker, helper + marker)

# Update all four functions to use TrimPath
for func in ['DoEncrypt', 'DoApiEncrypt', 'DoDecrypt', 'DoApiDecrypt']:
    old = f'std::cout << I18n::Get(StrKey::INPUT_FILE); std::getline(std::cin, in);'
    new = f'std::cout << I18n::Get(StrKey::INPUT_FILE); std::getline(std::cin, in);\n    in = TrimPath(in);'
    content = content.replace(old, new)

with open(path, "w", encoding="utf-8") as f:
    f.write(content)
print("Added TrimPath + applied to all 4 functions")
