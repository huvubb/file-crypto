path = r"C:\Users\邓涛\Documents\New project 3\file-crypto\src\main.cpp"
with open(path, "r", encoding="utf-8") as f:
    content = f.read()

# Add ReadLineUtf8 helper after TrimPath
marker = "static std::string TrimPath"
helper = """static std::string ReadLineUtf8() {
    std::wstring ws;
    HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
    DWORD read;
    WCHAR buf[4096];
    while (ReadConsoleW(h, buf, 4096, &read, NULL) && read > 0) {
        ws.append(buf, read);
        if (buf[read - 1] == L'\\n') break;
    }
    while (!ws.empty() && (ws.back() == L'\\r' || ws.back() == L'\\n'))
        ws.pop_back();
    int len = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, NULL, 0, NULL, NULL);
    if (len <= 1) return "";
    std::string result(len - 1, '\\0');
    WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, &result[0], len, NULL, NULL);
    return result;
}

"""
content = content.replace(marker, helper + marker)

# Replace all std::getline(std::cin, xxx) with xxx = ReadLineUtf8()
import re
# Pattern: std::string VAR; ... std::getline(std::cin, VAR);
# More simply: replace 'std::getline(std::cin, in)' etc

for var in ['in', 'pass', 'confirm', 'apiKey', 'out']:
    old = f'std::getline(std::cin, {var})'
    new = f'{var} = ReadLineUtf8()'
    content = content.replace(old, new)

with open(path, "w", encoding="utf-8") as f:
    f.write(content)
print("Replaced std::getline with ReadLineUtf8 for Unicode support")
