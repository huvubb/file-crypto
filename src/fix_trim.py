path = r"C:\Users\邓涛\Documents\New project 3\file-crypto\src\main.cpp"
with open(path, "r", encoding="utf-8") as f:
    content = f.read()

# Fix the broken TrimPath
old_trim = '''static std::string TrimPath(const std::string& raw) {
    std::string s = raw;
    // Trim leading/trailing whitespace
    while (!s.empty() && (s.front() == ' ' || s.front() == '	')) s.erase(0, 1);
    while (!s.empty() && (s.back() == ' ' || s.back() == '	' || s.back() == '
')) s.pop_back();
    // Strip surrounding quotes
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"') s = s.substr(1, s.size() - 2);
    return s;
}'''

new_trim = '''static std::string TrimPath(const std::string& raw) {
    std::string s = raw;
    while (!s.empty() && (s.front() == ' ' || s.front() == 9)) s.erase(0, 1);
    while (!s.empty()) {
        char c = s.back();
        if (c == ' ' || c == 9 || c == 10 || c == 13) s.pop_back();
        else break;
    }
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"')
        s = s.substr(1, s.size() - 2);
    return s;
}'''

content = content.replace(old_trim, new_trim)
with open(path, "w", encoding="utf-8") as f:
    f.write(content)
print("Fixed TrimPath - using ASCII codes")
