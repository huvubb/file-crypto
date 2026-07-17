path = r"C:\Users\邓涛\Documents\New project 3\file-crypto\src\main.cpp"
with open(path, "r", encoding="utf-8") as f:
    content = f.read()

# Update error messages to show the actual path
for func_name, old_err, new_err in [
    ("DoEncrypt",    '<< I18n::Get(StrKey::ENC_FAILED) << err << "\\n"', '<< I18n::Get(StrKey::ENC_FAILED) << err << " [" << in << "]" << "\\n"'),
    ("DoApiEncrypt", '<< I18n::Get(StrKey::ENC_FAILED) << err << "\\n"', '<< I18n::Get(StrKey::ENC_FAILED) << err << " [" << in << "]" << "\\n"'),
    ("DoDecrypt",    '<< I18n::Get(StrKey::DEC_FAILED) << err << "\\n"', '<< I18n::Get(StrKey::DEC_FAILED) << err << " [" << in << "]" << "\\n"'),
    ("DoApiDecrypt", '<< I18n::Get(StrKey::DEC_FAILED) << err << "\\n"', '<< I18n::Get(StrKey::DEC_FAILED) << err << " [" << in << "]" << "\\n"'),
]:
    content = content.replace(old_err, new_err)

with open(path, "w", encoding="utf-8") as f:
    f.write(content)
print("Added path info to error messages")
