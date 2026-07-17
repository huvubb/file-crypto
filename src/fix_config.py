path = r"C:\Users\邓涛\Documents\New project 3\file-crypto\src\config.cpp"
with open(path, "r", encoding="utf-8") as f:
    content = f.read()

content = content.replace('#include <windows.h>', '''#include <windows.h>
#include <filesystem>

static std::filesystem::path ToWideCfg(const std::string& s) {
    if (s.empty()) return std::filesystem::path();
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, NULL, 0);
    if (len <= 1) return std::filesystem::path(s);
    std::wstring ws(len, L'\\x00');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &ws[0], len);
    ws.resize(len - 1);
    return std::filesystem::path(ws);
}''')

content = content.replace('std::ifstream file(CONFIG_PATH)', 'std::ifstream file(ToWideCfg(CONFIG_PATH))')
content = content.replace('std::ofstream file(CONFIG_PATH, std::ios::trunc)', 'std::ofstream file(ToWideCfg(CONFIG_PATH), std::ios::trunc)')

with open(path, "w", encoding="utf-8") as f:
    f.write(content)
print("config.cpp fixed")
