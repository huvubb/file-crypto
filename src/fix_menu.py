import json
path = r"C:\Users\邓涛\Documents\New project 3\file-crypto\src\lang_data.json"
with open(path, "r", encoding="utf-8") as f:
    langs = json.load(f)

# Fix menu numbering - 6 items
fixes = {
    "en":    {"ENCRYPT":"1. Encrypt (Password)","ENC_KEYFILE":"2. Encrypt (API Key)","DECRYPT":"3. Decrypt (Password)","DEC_KEYFILE":"4. Decrypt (API Key)","CHANGE_LANG":"5. Change Language","EXIT":"6. Exit"},
    "zh-CN": {"ENCRYPT":"1. 加密(密码)","ENC_KEYFILE":"2. 加密(API密钥)","DECRYPT":"3. 解密(密码)","DEC_KEYFILE":"4. 解密(API密钥)","CHANGE_LANG":"5. 切换语言","EXIT":"6. 退出"},
    "cn-tw": {"ENCRYPT":"1. 加密(密碼)","ENC_KEYFILE":"2. 加密(API金鑰)","DECRYPT":"3. 解密(密碼)","DEC_KEYFILE":"4. 解密(API金鑰)","CHANGE_LANG":"5. 切換語言","EXIT":"6. 退出"},
}
for code, kv in fixes.items():
    if code in langs:
        for k, v in kv.items(): langs[code][k] = v

# For all other languages, use English numbering
for code in langs:
    if code not in fixes:
        langs[code]["ENCRYPT"] = "1. Encrypt (Password)"
        langs[code]["ENC_KEYFILE"] = "2. Encrypt (API Key)"
        langs[code]["DECRYPT"] = "3. Decrypt (Password)"
        langs[code]["DEC_KEYFILE"] = "4. Decrypt (API Key)"
        langs[code]["CHANGE_LANG"] = "5. Change Language"
        langs[code]["EXIT"] = "6. Exit"

with open(path, "w", encoding="utf-8") as f:
    json.dump(langs, f, ensure_ascii=False, indent=2)
print("Fixed menu numbering for all 50 languages")
