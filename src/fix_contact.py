import json
path = r"C:\Users\邓涛\Documents\New project 3\file-crypto\src\lang_data.json"
with open(path, "r", encoding="utf-8") as f:
    langs = json.load(f)

new_contact = "vx: forever870422  邮箱: 810372789@qq.com"
new_contrib = "欢迎贡献翻译！vx: forever870422"

for code in langs:
    langs[code]["CONTACT_INFO"] = new_contact
    langs[code]["WELCOME_CONTRIB"] = new_contrib

with open(path, "w", encoding="utf-8") as f:
    json.dump(langs, f, ensure_ascii=False, indent=2)
print(f"Updated contact for all {len(langs)} languages")
