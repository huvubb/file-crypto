path = r"C:\Users\邓涛\Documents\New project 3\file-crypto\src\main.cpp"
with open(path, "r", encoding="utf-8") as f:
    c = f.read()

old = '''    std::cout << "\n" << I18n::Get(StrKey::PRESS_ENTER);
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();'''
new = '''    std::cout << "\n" << I18n::Get(StrKey::PRESS_ENTER);
    ReadLineUtf8();'''
c = c.replace(old, new)
with open(path, "w", encoding="utf-8") as f:
    f.write(c)
print("Fixed Pause()")
