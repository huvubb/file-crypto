path = r"C:\Users\邓涛\Documents\New project 3\file-crypto\src\crypto.cpp"
with open(path, "r", encoding="utf-8") as f:
    content = f.read()

# Add helper and filesystem include at top
includes_end = '#include <iostream>'
new_includes = '''#include <iostream>
#include <windows.h>
#include <filesystem>

static std::filesystem::path ToWidePath(const std::string& utf8) {
    if (utf8.empty()) return std::filesystem::path();
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);
    if (len <= 1) return std::filesystem::path(utf8);
    std::wstring ws(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &ws[0], len);
    ws.resize(len - 1);
    return std::filesystem::path(ws);
}'''
content = content.replace(includes_end, new_includes)

# Replace all std::ifstream and std::ofstream constructors that take string paths
# Pattern: std::ifstream VAR(PATH_STRING, ...)
# Replace with: std::ifstream VAR(ToWidePath(PATH_STRING), ...)
# Same for ofstream

import re
# Find all ifstream/ofstream constructors with string path
for stream_type, mode_pattern in [('std::ifstream', 'std::ifstream in'), ('std::ofstream', 'std::ofstream out')]:
    # Find patterns like: std::ifstream in(PATH, std::ios::binary | std::ios::ate);
    # or: std::ofstream out(PATH, std::ios::binary);
    pattern = stream_type + r' (\w+)\((.*?), (std::ios::[^)]+)\)'
    def replace_fn(m):
        var_name = m.group(1)
        path_expr = m.group(2)
        mode = m.group(3)
        return f'{stream_type} {var_name}(ToWidePath({path_expr}), {mode})'
    content = re.sub(pattern, replace_fn, content)

# Also handle the backup/other constructors
# std::ofstream out(outputPath, std::ios::binary)
# etc.
pattern2 = r'std::ofstream (\w+)\((\w+Path), (std::ios::[^)]+)\)'
content = re.sub(pattern2, r'std::ofstream \1(ToWidePath(\2), \3)', content)

pattern3 = r'std::ofstream (\w+)\((\w+)\)'  # constructor without mode
content = re.sub(pattern3, r'std::ofstream \1(ToWidePath(\2))', content)

with open(path, "w", encoding="utf-8") as f:
    f.write(content)
print("Added ToWidePath + filesystem for all file opens")
