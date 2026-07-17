path = r"C:\Users\邓涛\Documents\New project 3\file-crypto\src\crypto.cpp"
with open(path, "r", encoding="utf-8") as f:
    content = f.read()

# Fix 1: the null char in wstring init
old_ws = "std::wstring ws(len, L' ');"
new_ws = "std::wstring ws(len, L'\\x00');"
content = content.replace(old_ws, new_ws)

# Fix 2: Add undef after windows.h include for EncryptFile/DecryptFile macros
old_win = "#include <windows.h>"
new_win = """#include <windows.h>
#ifdef EncryptFile
#undef EncryptFile
#endif
#ifdef DecryptFile
#undef DecryptFile
#endif"""
content = content.replace(old_win, new_win)

with open(path, "w", encoding="utf-8") as f:
    f.write(content)
print("Fixed null char and Windows macro conflicts")
