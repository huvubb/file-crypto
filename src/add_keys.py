import json
path = r"C:\Users\邓涛\Documents\New project 3\file-crypto\src\lang_data.json"
with open(path, "r", encoding="utf-8") as f:
    langs = json.load(f)

additions = {
    "en":    {"ENC_KEYFILE":"2. Encrypt + Key File","DEC_KEYFILE":"3. Decrypt with Key File","KEY_FILE_PATH":"Key file path: ","KEYFILE_SAVED":"Key file saved!"},
    "zh-CN": {"ENC_KEYFILE":"2. 加密并生成密钥文件","DEC_KEYFILE":"3. 密钥文件解密","KEY_FILE_PATH":"密钥文件路径：","KEYFILE_SAVED":"密钥文件已保存！"},
    "cn-tw": {"ENC_KEYFILE":"2. 加密並生成密鑰檔案","DEC_KEYFILE":"3. 密鑰檔案解密","KEY_FILE_PATH":"密鑰檔案路徑：","KEYFILE_SAVED":"密鑰檔案已儲存！"},
    "ja":    {"ENC_KEYFILE":"2. 暗号化+鍵ファイル","DEC_KEYFILE":"3. 鍵ファイルで復号","KEY_FILE_PATH":"鍵ファイル: ","KEYFILE_SAVED":"鍵ファイルを保存！"},
    "ko":    {"ENC_KEYFILE":"2. 암호화+키파일","DEC_KEYFILE":"3. 키파일로 복호화","KEY_FILE_PATH":"키파일 경로: ","KEYFILE_SAVED":"키파일 저장됨!"},
}
for code, kv in additions.items():
    if code in langs:
        for k, v in kv.items():
            langs[code][k] = v

for code in langs:
    for k in ["ENC_KEYFILE","DEC_KEYFILE","KEY_FILE_PATH","KEYFILE_SAVED"]:
        if k not in langs[code]:
            langs[code][k] = langs["en"][k]

with open(path, "w", encoding="utf-8") as f:
    json.dump(langs, f, ensure_ascii=False, indent=2)
print(f"Added new keys. {len(langs)} languages, {len(langs['en'])} keys each")
