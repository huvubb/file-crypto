import json
path = r"C:\Users\邓涛\Documents\New project 3\file-crypto\src\lang_data.json"
with open(path, "r", encoding="utf-8") as f:
    langs = json.load(f)

# zh-CN: use 微信
langs["zh-CN"]["CONTACT_INFO"] = "微信: forever870422  邮箱: 810372789@qq.com"
langs["zh-CN"]["WELCOME_CONTRIB"] = "欢迎贡献翻译！微信: forever870422"

# cn-tw: use WeChat (Traditional Chinese might still prefer 微信, but user said others = wechat)
langs["cn-tw"]["CONTACT_INFO"] = "WeChat: forever870422  邮箱: 810372789@qq.com"
langs["cn-tw"]["WELCOME_CONTRIB"] = "歡迎貢獻翻譯！WeChat: forever870422"

# All other languages: WeChat
for code in langs:
    if code not in ("zh-CN", "cn-tw"):
        langs[code]["CONTACT_INFO"] = "WeChat: forever870422  邮箱: 810372789@qq.com"
        langs[code]["WELCOME_CONTRIB"] = "Welcome! WeChat: forever870422"

# Fix cn-tw welcome message
langs["cn-tw"]["WELCOME_CONTRIB"] = "歡迎貢獻翻譯！WeChat: forever870422"

with open(path, "w", encoding="utf-8") as f:
    json.dump(langs, f, ensure_ascii=False, indent=2)
print("Updated: zh-CN=微信, others=WeChat")
