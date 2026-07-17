import json
path = r"C:\Users\邓涛\Documents\New project 3\file-crypto\src\lang_data.json"
with open(path, "r", encoding="utf-8") as f:
    langs = json.load(f)

updates = {
    "en":    {"ENC_KEYFILE":"2. API Key Encrypt","DEC_KEYFILE":"4. API Key Decrypt","KEY_FILE_PATH":"Enter API Key: ","KEYFILE_SAVED":"Your API Key (save this!):"},
    "zh-CN": {"ENC_KEYFILE":"2. API密钥加密","DEC_KEYFILE":"4. API密钥解密","KEY_FILE_PATH":"请输入API密钥：","KEYFILE_SAVED":"你的API密钥（请保存！）："},
    "cn-tw": {"ENC_KEYFILE":"2. API金鑰加密","DEC_KEYFILE":"4. API金鑰解密","KEY_FILE_PATH":"請輸入API金鑰：","KEYFILE_SAVED":"你的API金鑰（請保存！）："},
    "ja":    {"ENC_KEYFILE":"2. APIキー暗号化","DEC_KEYFILE":"4. APIキー復号","KEY_FILE_PATH":"APIキー: ","KEYFILE_SAVED":"APIキー（保存してください！）："},
    "ko":    {"ENC_KEYFILE":"2. API키 암호화","DEC_KEYFILE":"4. API키 복호화","KEY_FILE_PATH":"API키 입력: ","KEYFILE_SAVED":"API키 (저장하세요!):"},
}
for code, kv in updates.items():
    if code in langs:
        for k, v in kv.items(): langs[code][k] = v

for code in langs:
    for k in ["ENC_KEYFILE","DEC_KEYFILE","KEY_FILE_PATH","KEYFILE_SAVED"]:
        if k not in langs[code]: langs[code][k] = langs["en"][k]

with open(path, "w", encoding="utf-8") as f:
    json.dump(langs, f, ensure_ascii=False, indent=2)
print("Updated i18n keys")
