path = r"C:\Users\邓涛\Documents\New project 3\file-crypto\src\main.cpp"
with open(path, "r", encoding="utf-8") as f:
    content = f.read()

# Add ReadInt after ReadLineUtf8
marker = '''    return result;
}'''
helper = '''

static int ReadInt() {
    std::string s = ReadLineUtf8();
    try { return std::stoi(s); }
    catch (...) { return -1; }
}
'''
content = content.replace(marker, marker + helper, 1)  # only first occurrence

# Replace all std::cin >> c; std::cin.ignore(...); 
# Pattern: int c; std::cin >> c; \n std::cin.ignore(...)
import re
old_pattern = r'int (\w+);\s*std::cin >> \1;\s*std::cin\.ignore\(std::numeric_limits<std::streamsize>::max\(\), .\\n.\);'
def replace_menu_read(m):
    var = m.group(1)
    return f'int {var} = ReadInt();'
content = re.sub(old_pattern, replace_menu_read, content)

# Also handle ShowLanguageMenu specifically
old_lang = '''    int choice; std::cin >> choice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\\n');'''
content = content.replace(old_lang, '    int choice = ReadInt();')

with open(path, "w", encoding="utf-8") as f:
    f.write(content)
print("Replaced all std::cin >> with ReadInt()")
