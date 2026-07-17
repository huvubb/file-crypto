import json, os, sys
sys.path.insert(0, r"C:\Users\邓涛\Documents\New project 3\file-crypto\src")
from gen_i18n_part1 import langs, add

# German
add("de", TITLE="=== Datei-Verschluesselung ===", SELECT_LANG="--- Sprache waehlen ---", MAIN_MENU="--- Hauptmenue ---",
    ENCRYPT="1. Datei verschluesseln", DECRYPT="2. Datei entschluesseln", CHANGE_LANG="3. Sprache aendern", EXIT="4. Beenden",
    ENTER_CHOICE="Auswahl eingeben: ", INVALID_CHOICE="Ungueltige Auswahl!", INPUT_FILE="Eingabedatei: ", OUTPUT_FILE="Ausgabedatei: ",
    ENTER_PASSWORD="Passwort: ", CONFIRM_PASSWORD="Passwort bestaetigen: ", PASSWORD_MISMATCH="Passwoerter stimmen nicht ueberein!",
    ENC_SUCCESS="Verschluesselung erfolgreich!", DEC_SUCCESS="Entschluesselung erfolgreich!",
    ENC_FAILED="Verschluesselung fehlgeschlagen: ", DEC_FAILED="Entschluesselung fehlgeschlagen: ",
    FILE_NOT_FOUND="Datei nicht gefunden!", WRONG_PASS="Falsches Passwort oder beschaedigte Datei!",
    CONFIG_SAVED="Konfiguration gespeichert.", CONFIG_LOADED="Konfiguration geladen.",
    GOODBYE="Auf Wiedersehen!", PRESS_ENTER="Enter druecken...", ERROR_PREFIX="Fehler",
    LANG_PROMPT="Sprachnummer (1-50): ", ENC_PROMPT="Verschluesseln", DEC_PROMPT="Entschluesseln",
    CONTACT_INFO="Kontakt: WeChat forever870422 / Email 810372789@qq.com",
    WELCOME_CONTRIB="Hilf beim Uebersetzen! Kontakt forever870422")

# Japanese
add("ja", TITLE="=== ファイル暗号化・復号ツール ===", SELECT_LANG="--- 言語を選択 ---", MAIN_MENU="--- メインメニュー ---",
    ENCRYPT="1. ファイルを暗号化", DECRYPT="2. ファイルを復号", CHANGE_LANG="3. 言語を変更", EXIT="4. 終了",
    ENTER_CHOICE="選択肢を入力：", INVALID_CHOICE="無効な選択です！", INPUT_FILE="入力ファイル：", OUTPUT_FILE="出力ファイル：",
    ENTER_PASSWORD="パスワード：", CONFIRM_PASSWORD="パスワード確認：", PASSWORD_MISMATCH="パスワードが一致しません！",
    ENC_SUCCESS="暗号化が成功しました！", DEC_SUCCESS="復号が成功しました！",
    ENC_FAILED="暗号化に失敗：", DEC_FAILED="復号に失敗：",
    FILE_NOT_FOUND="ファイルが見つかりません！", WRONG_PASS="パスワードが違うかファイルが壊れています！",
    CONFIG_SAVED="設定を保存しました。", CONFIG_LOADED="設定を読み込みました。",
    GOODBYE="さようなら！", PRESS_ENTER="Enterキーを押してください...", ERROR_PREFIX="エラー",
    LANG_PROMPT="言語番号 (1-50)：", ENC_PROMPT="暗号化", DEC_PROMPT="復号",
    CONTACT_INFO="連絡先: WeChat forever870422 / Email 810372789@qq.com",
    WELCOME_CONTRIB="翻訳協力募集中！連絡先 forever870422")

# Korean
add("ko", TITLE="=== 파일 암호화/복호화 도구 ===", SELECT_LANG="--- 언어 선택 ---", MAIN_MENU="--- 메인 메뉴 ---",
    ENCRYPT="1. 파일 암호화", DECRYPT="2. 파일 복호화", CHANGE_LANG="3. 언어 변경", EXIT="4. 종료",
    ENTER_CHOICE="선택 입력: ", INVALID_CHOICE="잘못된 선택!", INPUT_FILE="입력 파일: ", OUTPUT_FILE="출력 파일: ",
    ENTER_PASSWORD="비밀번호: ", CONFIRM_PASSWORD="비밀번호 확인: ", PASSWORD_MISMATCH="비밀번호가 일치하지 않습니다!",
    ENC_SUCCESS="암호화 성공!", DEC_SUCCESS="복호화 성공!",
    ENC_FAILED="암호화 실패: ", DEC_FAILED="복호화 실패: ",
    FILE_NOT_FOUND="파일을 찾을 수 없습니다!", WRONG_PASS="잘못된 비밀번호 또는 손상된 파일!",
    CONFIG_SAVED="설정이 저장되었습니다.", CONFIG_LOADED="설정이 로드되었습니다.",
    GOODBYE="안녕히 가세요!", PRESS_ENTER="Enter 키를 누르세요...", ERROR_PREFIX="오류",
    LANG_PROMPT="언어 번호 (1-50): ", ENC_PROMPT="암호화", DEC_PROMPT="복호화",
    CONTACT_INFO="연락처: WeChat forever870422 / Email 810372789@qq.com",
    WELCOME_CONTRIB="번역 기여 환영! 연락처 forever870422")

# Russian
add("ru", TITLE="=== Инструмент шифрования ===", SELECT_LANG="--- Выбор языка ---", MAIN_MENU="--- Главное меню ---",
    ENCRYPT="1. Зашифровать файл", DECRYPT="2. Расшифровать файл", CHANGE_LANG="3. Сменить язык", EXIT="4. Выход",
    ENTER_CHOICE="Введите выбор: ", INVALID_CHOICE="Неверный выбор!", INPUT_FILE="Исходный файл: ", OUTPUT_FILE="Выходной файл: ",
    ENTER_PASSWORD="Пароль: ", CONFIRM_PASSWORD="Подтвердите пароль: ", PASSWORD_MISMATCH="Пароли не совпадают!",
    ENC_SUCCESS="Шифрование успешно!", DEC_SUCCESS="Расшифровка успешна!",
    ENC_FAILED="Ошибка шифрования: ", DEC_FAILED="Ошибка расшифровки: ",
    FILE_NOT_FOUND="Файл не найден!", WRONG_PASS="Неверный пароль или файл поврежден!",
    CONFIG_SAVED="Конфигурация сохранена.", CONFIG_LOADED="Конфигурация загружена.",
    GOODBYE="До свидания!", PRESS_ENTER="Нажмите Enter...", ERROR_PREFIX="Ошибка",
    LANG_PROMPT="Номер языка (1-50): ", ENC_PROMPT="Шифровать", DEC_PROMPT="Расшифровать",
    CONTACT_INFO="Контакт: WeChat forever870422 / Email 810372789@qq.com",
    WELCOME_CONTRIB="Помогите с переводом! forever870422")

# Arabic
add("ar", TITLE="=== أداة تشفير/فك تشفير الملفات ===", SELECT_LANG="--- اختر اللغة ---", MAIN_MENU="--- القائمة الرئيسية ---",
    ENCRYPT="1. تشفير ملف", DECRYPT="2. فك تشفير ملف", CHANGE_LANG="3. تغيير اللغة", EXIT="4. خروج",
    ENTER_CHOICE="أدخل اختيارك: ", INVALID_CHOICE="اختيار غير صالح!", INPUT_FILE="مسار الملف المدخل: ", OUTPUT_FILE="مسار الملف المخرج: ",
    ENTER_PASSWORD="كلمة المرور: ", CONFIRM_PASSWORD="تأكيد كلمة المرور: ", PASSWORD_MISMATCH="كلمات المرور غير متطابقة!",
    ENC_SUCCESS="تم التشفير بنجاح!", DEC_SUCCESS="تم فك التشفير بنجاح!",
    ENC_FAILED="فشل التشفير: ", DEC_FAILED="فشل فك التشفير: ",
    FILE_NOT_FOUND="الملف غير موجود!", WRONG_PASS="كلمة مرور خاطئة أو ملف تالف!",
    CONFIG_SAVED="تم حفظ الإعدادات.", CONFIG_LOADED="تم تحميل الإعدادات.",
    GOODBYE="وداعا!", PRESS_ENTER="اضغط Enter...", ERROR_PREFIX="خطأ",
    LANG_PROMPT="رقم اللغة (1-50): ", ENC_PROMPT="تشفير", DEC_PROMPT="فك تشفير",
    CONTACT_INFO="للتواصل: WeChat forever870422 / Email 810372789@qq.com",
    WELCOME_CONTRIB="مساهمات الترجمة مرحب بها! forever870422")

# Portuguese
add("pt", TITLE="=== Ferramenta de Criptografia ===", SELECT_LANG="--- Selecionar Idioma ---", MAIN_MENU="--- Menu Principal ---",
    ENCRYPT="1. Criptografar arquivo", DECRYPT="2. Descriptografar arquivo", CHANGE_LANG="3. Mudar idioma", EXIT="4. Sair",
    ENTER_CHOICE="Digite sua opcao: ", INVALID_CHOICE="Opcao invalida!", INPUT_FILE="Arquivo de entrada: ", OUTPUT_FILE="Arquivo de saida: ",
    ENTER_PASSWORD="Senha: ", CONFIRM_PASSWORD="Confirmar senha: ", PASSWORD_MISMATCH="Senhas nao coincidem!",
    ENC_SUCCESS="Criptografia bem-sucedida!", DEC_SUCCESS="Descriptografia bem-sucedida!",
    ENC_FAILED="Falha na criptografia: ", DEC_FAILED="Falha na descriptografia: ",
    FILE_NOT_FOUND="Arquivo nao encontrado!", WRONG_PASS="Senha incorreta ou arquivo corrompido!",
    CONFIG_SAVED="Configuracao salva.", CONFIG_LOADED="Configuracao carregada.",
    GOODBYE="Adeus!", PRESS_ENTER="Pressione Enter...", ERROR_PREFIX="Erro",
    LANG_PROMPT="Numero do idioma (1-50): ", ENC_PROMPT="Criptografar", DEC_PROMPT="Descriptografar",
    CONTACT_INFO="Contato: WeChat forever870422 / Email 810372789@qq.com",
    WELCOME_CONTRIB="Ajude a traduzir! forever870422")

# Italian
add("it", TITLE="=== Strumento di Cifratura ===", SELECT_LANG="--- Seleziona Lingua ---", MAIN_MENU="--- Menu Principale ---",
    ENCRYPT="1. Cifra file", DECRYPT="2. Decifra file", CHANGE_LANG="3. Cambia lingua", EXIT="4. Esci",
    ENTER_CHOICE="Inserisci scelta: ", INVALID_CHOICE="Scelta non valida!", INPUT_FILE="Percorso file input: ", OUTPUT_FILE="Percorso file output: ",
    ENTER_PASSWORD="Password: ", CONFIRM_PASSWORD="Conferma password: ", PASSWORD_MISMATCH="Le password non corrispondono!",
    ENC_SUCCESS="Cifratura riuscita!", DEC_SUCCESS="Decifratura riuscita!",
    ENC_FAILED="Cifratura fallita: ", DEC_FAILED="Decifratura fallita: ",
    FILE_NOT_FOUND="File non trovato!", WRONG_PASS="Password errata o file corrotto!",
    CONFIG_SAVED="Configurazione salvata.", CONFIG_LOADED="Configurazione caricata.",
    GOODBYE="Arrivederci!", PRESS_ENTER="Premi Enter...", ERROR_PREFIX="Errore",
    LANG_PROMPT="Numero lingua (1-50): ", ENC_PROMPT="Cifra", DEC_PROMPT="Decifra",
    CONTACT_INFO="Contatto: WeChat forever870422 / Email 810372789@qq.com",
    WELCOME_CONTRIB="Aiuta a tradurre! forever870422")

# Dutch
add("nl", TITLE="=== Bestand Versleuteling ===", SELECT_LANG="--- Kies Taal ---", MAIN_MENU="--- Hoofdmenu ---",
    ENCRYPT="1. Bestand versleutelen", DECRYPT="2. Bestand ontsleutelen", CHANGE_LANG="3. Taal wijzigen", EXIT="4. Afsluiten",
    ENTER_CHOICE="Voer keuze in: ", INVALID_CHOICE="Ongeldige keuze!", INPUT_FILE="Invoerbestand: ", OUTPUT_FILE="Uitvoerbestand: ",
    ENTER_PASSWORD="Wachtwoord: ", CONFIRM_PASSWORD="Bevestig wachtwoord: ", PASSWORD_MISMATCH="Wachtwoorden komen niet overeen!",
    ENC_SUCCESS="Versleuteling gelukt!", DEC_SUCCESS="Ontsleuteling gelukt!",
    ENC_FAILED="Versleuteling mislukt: ", DEC_FAILED="Ontsleuteling mislukt: ",
    FILE_NOT_FOUND="Bestand niet gevonden!", WRONG_PASS="Verkeerd wachtwoord of beschadigd bestand!",
    CONFIG_SAVED="Configuratie opgeslagen.", CONFIG_LOADED="Configuratie geladen.",
    GOODBYE="Tot ziens!", PRESS_ENTER="Druk op Enter...", ERROR_PREFIX="Fout",
    LANG_PROMPT="Taalnummer (1-50): ", ENC_PROMPT="Versleutelen", DEC_PROMPT="Ontsleutelen",
    CONTACT_INFO="Contact: WeChat forever870422 / Email 810372789@qq.com",
    WELCOME_CONTRIB="Help met vertalen! forever870422")

# Hindi
add("hi", TITLE="=== फ़ाइल एन्क्रिप्शन उपकरण ===", SELECT_LANG="--- भाषा चुनें ---", MAIN_MENU="--- मुख्य मेनू ---",
    ENCRYPT="1. फ़ाइल एन्क्रिप्ट करें", DECRYPT="2. फ़ाइल डिक्रिप्ट करें", CHANGE_LANG="3. भाषा बदलें", EXIT="4. बाहर निकलें",
    ENTER_CHOICE="अपनी पसंद दर्ज करें: ", INVALID_CHOICE="अमान्य विकल्प!", INPUT_FILE="इनपुट फ़ाइल: ", OUTPUT_FILE="आउटपुट फ़ाइल: ",
    ENTER_PASSWORD="पासवर्ड: ", CONFIRM_PASSWORD="पासवर्ड पुष्टि करें: ", PASSWORD_MISMATCH="पासवर्ड मेल नहीं खाते!",
    ENC_SUCCESS="एन्क्रिप्शन सफल!", DEC_SUCCESS="डिक्रिप्शन सफल!",
    ENC_FAILED="एन्क्रिप्शन विफल: ", DEC_FAILED="डिक्रिप्शन विफल: ",
    FILE_NOT_FOUND="फ़ाइल नहीं मिली!", WRONG_PASS="गलत पासवर्ड या दूषित फ़ाइल!",
    CONFIG_SAVED="कॉन्फ़िगरेशन सहेजा गया।", CONFIG_LOADED="कॉन्फ़िगरेशन लोड किया गया।",
    GOODBYE="अलविदा!", PRESS_ENTER="जारी रखने के लिए Enter दबाएं...", ERROR_PREFIX="त्रुटि",
    LANG_PROMPT="भाषा संख्या (1-50): ", ENC_PROMPT="एन्क्रिप्ट", DEC_PROMPT="डिक्रिप्ट",
    CONTACT_INFO="संपर्क: WeChat forever870422 / Email 810372789@qq.com",
    WELCOME_CONTRIB="अनुवाद में मदद करें! forever870422")

# Turkish
add("tr", TITLE="=== Dosya Sifreleme Araci ===", SELECT_LANG="--- Dil Secin ---", MAIN_MENU="--- Ana Menu ---",
    ENCRYPT="1. Dosyayi sifrele", DECRYPT="2. Dosyayi coz", CHANGE_LANG="3. Dil degistir", EXIT="4. Cikis",
    ENTER_CHOICE="Seciminizi girin: ", INVALID_CHOICE="Gecersiz secim!", INPUT_FILE="Girdi dosyasi: ", OUTPUT_FILE="Cikti dosyasi: ",
    ENTER_PASSWORD="Sifre: ", CONFIRM_PASSWORD="Sifreyi onaylayin: ", PASSWORD_MISMATCH="Sifreler eslesmiyor!",
    ENC_SUCCESS="Sifreleme basarili!", DEC_SUCCESS="Cozme basarili!",
    ENC_FAILED="Sifreleme basarisiz: ", DEC_FAILED="Cozme basarisiz: ",
    FILE_NOT_FOUND="Dosya bulunamadi!", WRONG_PASS="Yanlis sifre veya bozuk dosya!",
    CONFIG_SAVED="Yapilandirma kaydedildi.", CONFIG_LOADED="Yapilandirma yuklendi.",
    GOODBYE="Hoscakal!", PRESS_ENTER="Devam etmek icin Enter...", ERROR_PREFIX="Hata",
    LANG_PROMPT="Dil numarasi (1-50): ", ENC_PROMPT="Sifrele", DEC_PROMPT="Coz",
    CONTACT_INFO="Iletisim: WeChat forever870422 / Email 810372789@qq.com",
    WELCOME_CONTRIB="Ceviriye yardim edin! forever870422")

# Vietnamese
add("vi", TITLE="=== Cong cu Ma hoa/Giai ma ===", SELECT_LANG="--- Chon ngon ngu ---", MAIN_MENU="--- Menu Chinh ---",
    ENCRYPT="1. Ma hoa tap tin", DECRYPT="2. Giai ma tap tin", CHANGE_LANG="3. Doi ngon ngu", EXIT="4. Thoat",
    ENTER_CHOICE="Nhap lua chon: ", INVALID_CHOICE="Lua chon khong hop le!", INPUT_FILE="Tap tin dau vao: ", OUTPUT_FILE="Tap tin dau ra: ",
    ENTER_PASSWORD="Mat khau: ", CONFIRM_PASSWORD="Xac nhan mat khau: ", PASSWORD_MISMATCH="Mat khau khong khop!",
    ENC_SUCCESS="Ma hoa thanh cong!", DEC_SUCCESS="Giai ma thanh cong!",
    ENC_FAILED="Ma hoa that bai: ", DEC_FAILED="Giai ma that bai: ",
    FILE_NOT_FOUND="Khong tim thay tap tin!", WRONG_PASS="Sai mat khau hoac tap tin bi hong!",
    CONFIG_SAVED="Cau hinh da luu.", CONFIG_LOADED="Cau hinh da tai.",
    GOODBYE="Tam biet!", PRESS_ENTER="Nhan Enter de tiep tuc...", ERROR_PREFIX="Loi",
    LANG_PROMPT="So ngon ngu (1-50): ", ENC_PROMPT="Ma hoa", DEC_PROMPT="Giai ma",
    CONTACT_INFO="Lien he: WeChat forever870422 / Email 810372789@qq.com",
    WELCOME_CONTRIB="Giup dich thuat! forever870422")

# Thai
add("th", TITLE="=== เครื่องมือเข้ารหัส/ถอดรหัส ===", SELECT_LANG="--- เลือกภาษา ---", MAIN_MENU="--- เมนูหลัก ---",
    ENCRYPT="1. เข้ารหัสไฟล์", DECRYPT="2. ถอดรหัสไฟล์", CHANGE_LANG="3. เปลี่ยนภาษา", EXIT="4. ออก",
    ENTER_CHOICE="ป้อนตัวเลือก: ", INVALID_CHOICE="ตัวเลือกไม่ถูกต้อง!", INPUT_FILE="ไฟล์นำเข้า: ", OUTPUT_FILE="ไฟล์ส่งออก: ",
    ENTER_PASSWORD="รหัสผ่าน: ", CONFIRM_PASSWORD="ยืนยันรหัสผ่าน: ", PASSWORD_MISMATCH="รหัสผ่านไม่ตรงกัน!",
    ENC_SUCCESS="เข้ารหัสสำเร็จ!", DEC_SUCCESS="ถอดรหัสสำเร็จ!",
    ENC_FAILED="เข้ารหัสล้มเหลว: ", DEC_FAILED="ถอดรหัสล้มเหลว: ",
    FILE_NOT_FOUND="ไม่พบไฟล์!", WRONG_PASS="รหัสผ่านผิดหรือไฟล์เสียหาย!",
    CONFIG_SAVED="บันทึกการตั้งค่าแล้ว", CONFIG_LOADED="โหลดการตั้งค่าแล้ว",
    GOODBYE="ลาก่อน!", PRESS_ENTER="กด Enter เพื่อดำเนินการต่อ...", ERROR_PREFIX="ข้อผิดพลาด",
    LANG_PROMPT="หมายเลขภาษา (1-50): ", ENC_PROMPT="เข้ารหัส", DEC_PROMPT="ถอดรหัส",
    CONTACT_INFO="ติดต่อ: WeChat forever870422 / Email 810372789@qq.com",
    WELCOME_CONTRIB="ช่วยแปลภาษา! forever870422")

print(f"Total languages so far: {len(langs)}")
