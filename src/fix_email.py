import json
path = r"C:\Users\邓涛\Documents\New project 3\file-crypto\src\lang_data.json"
with open(path, "r", encoding="utf-8") as f:
    langs = json.load(f)

# zh-CN: keep 微信 + 邮箱
langs["zh-CN"]["CONTACT_INFO"] = "微信: forever870422  邮箱: 810372789@qq.com"

# cn-tw and all others: WeChat + Email
for code in langs:
    if code != "zh-CN":
        langs[code]["CONTACT_INFO"] = "WeChat: forever870422  Email: 810372789@qq.com"

with open(path, "w", encoding="utf-8") as f:
    json.dump(langs, f, ensure_ascii=False, indent=2)
print("Done: zh-CN=微信+邮箱, others=WeChat+Email")
