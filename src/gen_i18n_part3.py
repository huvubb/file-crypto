import sys
sys.path.insert(0, r"C:\Users\邓涛\Documents\New project 3\file-crypto\src")
from gen_i18n_part1 import langs, add

# Polish
add("pl", TITLE="=== Narzedzie Szyfrowania ===", SELECT_LANG="--- Wybierz jezyk ---", MAIN_MENU="--- Menu glowne ---",
    ENCRYPT="1. Zaszyfruj plik", DECRYPT="2. Odszyfruj plik", CHANGE_LANG="3. Zmien jezyk", EXIT="4. Wyjscie",
    ENTER_CHOICE="Wybierz opcje: ", INVALID_CHOICE="Nieprawidlowy wybor!", INPUT_FILE="Plik wejsciowy: ", OUTPUT_FILE="Plik wyjsciowy: ",
    ENTER_PASSWORD="Haslo: ", CONFIRM_PASSWORD="Potwierdz haslo: ", PASSWORD_MISMATCH="Hasla nie sa zgodne!",
    ENC_SUCCESS="Szyfrowanie udane!", DEC_SUCCESS="Odszyfrowanie udane!",
    ENC_FAILED="Szyfrowanie nieudane: ", DEC_FAILED="Odszyfrowanie nieudane: ",
    FILE_NOT_FOUND="Nie znaleziono pliku!", WRONG_PASS="Bledne haslo lub uszkodzony plik!",
    CONFIG_SAVED="Konfiguracja zapisana.", CONFIG_LOADED="Konfiguracja wczytana.",
    GOODBYE="Do widzenia!", PRESS_ENTER="Nacisnij Enter...", ERROR_PREFIX="Blad",
    LANG_PROMPT="Numer jezyka (1-50): ", ENC_PROMPT="Szyfruj", DEC_PROMPT="Odszyfruj",
    CONTACT_INFO="Kontakt: WeChat forever870422 / Email 810372789@qq.com",
    WELCOME_CONTRIB="Pomoz w tlumaczeniu! forever870422")

# Swedish
add("sv", TITLE="=== Filkrypteringsverktyg ===", SELECT_LANG="--- Valj sprak ---", MAIN_MENU="--- Huvudmeny ---",
    ENCRYPT="1. Kryptera fil", DECRYPT="2. Dekryptera fil", CHANGE_LANG="3. Byt sprak", EXIT="4. Avsluta",
    ENTER_CHOICE="Ange val: ", INVALID_CHOICE="Ogiltigt val!", INPUT_FILE="Infil: ", OUTPUT_FILE="Utfil: ",
    ENTER_PASSWORD="Losenord: ", CONFIRM_PASSWORD="Bekrafta losenord: ", PASSWORD_MISMATCH="Losenorden matchar inte!",
    ENC_SUCCESS="Kryptering lyckades!", DEC_SUCCESS="Dekryptering lyckades!",
    ENC_FAILED="Kryptering misslyckades: ", DEC_FAILED="Dekryptering misslyckades: ",
    FILE_NOT_FOUND="Filen hittades inte!", WRONG_PASS="Fel losenord eller skadad fil!",
    CONFIG_SAVED="Konfiguration sparad.", CONFIG_LOADED="Konfiguration laddad.",
    GOODBYE="Hej da!", PRESS_ENTER="Tryck Enter...", ERROR_PREFIX="Fel",
    LANG_PROMPT="Spraknummer (1-50): ", ENC_PROMPT="Kryptera", DEC_PROMPT="Dekryptera",
    CONTACT_INFO="Kontakt: WeChat forever870422 / Email 810372789@qq.com",
    WELCOME_CONTRIB="Hjalp till att oversatta! forever870422")

# Danish
add("da", TITLE="=== Filkrypteringsvaerktoj ===", SELECT_LANG="--- Vaelg sprog ---", MAIN_MENU="--- Hovedmenu ---",
    ENCRYPT="1. Krypter fil", DECRYPT="2. Dekrypter fil", CHANGE_LANG="3. Skift sprog", EXIT="4. Afslut",
    ENTER_CHOICE="Indtast valg: ", INVALID_CHOICE="Ugyldigt valg!", INPUT_FILE="Input fil: ", OUTPUT_FILE="Output fil: ",
    ENTER_PASSWORD="Adgangskode: ", CONFIRM_PASSWORD="Bekraeft adgangskode: ", PASSWORD_MISMATCH="Adgangskoder matcher ikke!",
    ENC_SUCCESS="Kryptering lykkedes!", DEC_SUCCESS="Dekryptering lykkedes!",
    ENC_FAILED="Kryptering mislykkedes: ", DEC_FAILED="Dekryptering mislykkedes: ",
    FILE_NOT_FOUND="Fil ikke fundet!", WRONG_PASS="Forkert adgangskode eller beskadiget fil!",
    CONFIG_SAVED="Konfiguration gemt.", CONFIG_LOADED="Konfiguration indlaest.",
    GOODBYE="Farvel!", PRESS_ENTER="Tryk Enter...", ERROR_PREFIX="Fejl",
    LANG_PROMPT="Sprognummer (1-50): ", ENC_PROMPT="Krypter", DEC_PROMPT="Dekrypter",
    CONTACT_INFO="Kontakt: WeChat forever870422 / Email 810372789@qq.com",
    WELCOME_CONTRIB="Hjaelp med oversaettelse! forever870422")

# Finnish
add("fi", TITLE="=== Tiedoston Salaus ===", SELECT_LANG="--- Valitse kieli ---", MAIN_MENU="--- Paavalikko ---",
    ENCRYPT="1. Salaa tiedosto", DECRYPT="2. Pura salaus", CHANGE_LANG="3. Vaihda kielta", EXIT="4. Lopeta",
    ENTER_CHOICE="Anna valinta: ", INVALID_CHOICE="Virheellinen valinta!", INPUT_FILE="Syotetiedosto: ", OUTPUT_FILE="Tulostiedosto: ",
    ENTER_PASSWORD="Salasana: ", CONFIRM_PASSWORD="Vahvista salasana: ", PASSWORD_MISMATCH="Salasanat eivat taasmaa!",
    ENC_SUCCESS="Salaus onnistui!", DEC_SUCCESS="Salauksen purku onnistui!",
    ENC_FAILED="Salaus epaonnistui: ", DEC_FAILED="Salauksen purku epaonnistui: ",
    FILE_NOT_FOUND="Tiedostoa ei loydy!", WRONG_PASS="Vaara salasana tai vioittunut tiedosto!",
    CONFIG_SAVED="Asetukset tallennettu.", CONFIG_LOADED="Asetukset ladattu.",
    GOODBYE="Nakemiin!", PRESS_ENTER="Paina Enter...", ERROR_PREFIX="Virhe",
    LANG_PROMPT="Kielen numero (1-50): ", ENC_PROMPT="Salaa", DEC_PROMPT="Pura",
    CONTACT_INFO="Yhteystiedot: WeChat forever870422 / Email 810372789@qq.com",
    WELCOME_CONTRIB="Auta kaantamisessa! forever870422")

# Norwegian
add("no", TITLE="=== Filkrypteringsverktoy ===", SELECT_LANG="--- Velg sprak ---", MAIN_MENU="--- Hovedmeny ---",
    ENCRYPT="1. Krypter fil", DECRYPT="2. Dekrypter fil", CHANGE_LANG="3. Bytt sprak", EXIT="4. Avslutt",
    ENTER_CHOICE="Skriv inn valg: ", INVALID_CHOICE="Ugyldig valg!", INPUT_FILE="Innfil: ", OUTPUT_FILE="Utfil: ",
    ENTER_PASSWORD="Passord: ", CONFIRM_PASSWORD="Bekreft passord: ", PASSWORD_MISMATCH="Passordene stemmer ikke!",
    ENC_SUCCESS="Kryptering vellykket!", DEC_SUCCESS="Dekryptering vellykket!",
    ENC_FAILED="Kryptering mislyktes: ", DEC_FAILED="Dekryptering mislyktes: ",
    FILE_NOT_FOUND="Fil ikke funnet!", WRONG_PASS="Feil passord eller odelagt fil!",
    CONFIG_SAVED="Konfigurasjon lagret.", CONFIG_LOADED="Konfigurasjon lastet.",
    GOODBYE="Ha det!", PRESS_ENTER="Trykk Enter...", ERROR_PREFIX="Feil",
    LANG_PROMPT="Spraknummer (1-50): ", ENC_PROMPT="Krypter", DEC_PROMPT="Dekrypter",
    CONTACT_INFO="Kontakt: WeChat forever870422 / Email 810372789@qq.com",
    WELCOME_CONTRIB="Hjelp med oversettelse! forever870422")

# Czech
add("cs", TITLE="=== Sifrovani souboru ===", SELECT_LANG="--- Vyberte jazyk ---", MAIN_MENU="--- Hlavni menu ---",
    ENCRYPT="1. Zasifrovat soubor", DECRYPT="2. Desifrovat soubor", CHANGE_LANG="3. Zmenit jazyk", EXIT="4. Konec",
    ENTER_CHOICE="Zadejte volbu: ", INVALID_CHOICE="Neplatna volba!", INPUT_FILE="Vstupni soubor: ", OUTPUT_FILE="Vystupni soubor: ",
    ENTER_PASSWORD="Heslo: ", CONFIRM_PASSWORD="Potvrdit heslo: ", PASSWORD_MISMATCH="Hesla se neshoduji!",
    ENC_SUCCESS="Sifrovani uspesne!", DEC_SUCCESS="Desifrovani uspesne!",
    ENC_FAILED="Sifrovani selhalo: ", DEC_FAILED="Desifrovani selhalo: ",
    FILE_NOT_FOUND="Soubor nenalezen!", WRONG_PASS="Spatne heslo nebo poskozeny soubor!",
    CONFIG_SAVED="Konfigurace ulozena.", CONFIG_LOADED="Konfigurace nactena.",
    GOODBYE="Nashledanou!", PRESS_ENTER="Stisknete Enter...", ERROR_PREFIX="Chyba",
    LANG_PROMPT="Cislo jazyka (1-50): ", ENC_PROMPT="Sifrovat", DEC_PROMPT="Desifrovat",
    CONTACT_INFO="Kontakt: WeChat forever870422 / Email 810372789@qq.com",
    WELCOME_CONTRIB="Pomozte s prekladem! forever870422")

# Hungarian
add("hu", TITLE="=== Fajltitkosito Eszkoz ===", SELECT_LANG="--- Valasszon nyelvet ---", MAIN_MENU="--- Fomenu ---",
    ENCRYPT="1. Fajl titkositasa", DECRYPT="2. Fajl visszafejtese", CHANGE_LANG="3. Nyelv valtasa", EXIT="4. Kilepes",
    ENTER_CHOICE="Valasztas: ", INVALID_CHOICE="Ervenytelen valasztas!", INPUT_FILE="Bemeneti fajl: ", OUTPUT_FILE="Kimeneti fajl: ",
    ENTER_PASSWORD="Jelszo: ", CONFIRM_PASSWORD="Jelszo megerosit: ", PASSWORD_MISMATCH="A jelszavak nem egyeznek!",
    ENC_SUCCESS="Titkositas sikeres!", DEC_SUCCESS="Visszafejtes sikeres!",
    ENC_FAILED="Titkositas sikertelen: ", DEC_FAILED="Visszafejtes sikertelen: ",
    FILE_NOT_FOUND="A fajl nem talalhato!", WRONG_PASS="Hibas jelszo vagy serult fajl!",
    CONFIG_SAVED="Konfiguracio mentve.", CONFIG_LOADED="Konfiguracio betoltve.",
    GOODBYE="Viszontlatasra!", PRESS_ENTER="Nyomjon Entert...", ERROR_PREFIX="Hiba",
    LANG_PROMPT="Nyelv szama (1-50): ", ENC_PROMPT="Titkosit", DEC_PROMPT="Visszafejt",
    CONTACT_INFO="Kapcsolat: WeChat forever870422 / Email 810372789@qq.com",
    WELCOME_CONTRIB="Segitsen forditani! forever870422")

# Romanian
add("ro", TITLE="=== Instrument Criptare ===", SELECT_LANG="--- Selectati limba ---", MAIN_MENU="--- Meniu Principal ---",
    ENCRYPT="1. Criptare fisier", DECRYPT="2. Decriptare fisier", CHANGE_LANG="3. Schimba limba", EXIT="4. Iesire",
    ENTER_CHOICE="Introduceti optiunea: ", INVALID_CHOICE="Optiune invalida!", INPUT_FILE="Fisier intrare: ", OUTPUT_FILE="Fisier iesire: ",
    ENTER_PASSWORD="Parola: ", CONFIRM_PASSWORD="Confirmati parola: ", PASSWORD_MISMATCH="Parolele nu coincid!",
    ENC_SUCCESS="Criptare reusita!", DEC_SUCCESS="Decriptare reusita!",
    ENC_FAILED="Criptare esuata: ", DEC_FAILED="Decriptare esuata: ",
    FILE_NOT_FOUND="Fisierul nu a fost gasit!", WRONG_PASS="Parola gresita sau fisier corupt!",
    CONFIG_SAVED="Configuratie salvata.", CONFIG_LOADED="Configuratie incarcata.",
    GOODBYE="La revedere!", PRESS_ENTER="Apasati Enter...", ERROR_PREFIX="Eroare",
    LANG_PROMPT="Numar limba (1-50): ", ENC_PROMPT="Criptare", DEC_PROMPT="Decriptare",
    CONTACT_INFO="Contact: WeChat forever870422 / Email 810372789@qq.com",
    WELCOME_CONTRIB="Ajutati la traducere! forever870422")

# Greek
add("el", TITLE="=== Er paleio Krypt o raf isis ===", SELECT_LANG="--- Epil exte l ssa ---", MAIN_MENU="--- B sik Meno ---",
    ENCRYPT="1. Krypt o raf isi ar e o", DECRYPT="2. Apokrypt o raf isi", CHANGE_LANG="3. Alla l ssa", EXIT="4. E odos",
    ENTER_CHOICE="Eis te epil i: ", INVALID_CHOICE="Mi k yr epil i!", INPUT_FILE="Ar e o eis do : ", OUTPUT_FILE="Ar e o e odou: ",
    ENTER_PASSWORD="K dik s: ", CONFIRM_PASSWORD="Epibe ai si k diko : ", PASSWORD_MISMATCH="Oi k diko den tiri o n!",
    ENC_SUCCESS="Krypt o raf isi epity is!", DEC_SUCCESS="Apokrypt o raf isi epity is!",
    ENC_FAILED="Krypt o raf isi apety e: ", DEC_FAILED="Apokrypt o raf isi apety e: ",
    FILE_NOT_FOUND="To ar e o den r eth ke!", WRONG_PASS="L th s k dik s i k t str no ar e o!",
    CONFIG_SAVED="I r th isi apoth ke t ke.", CONFIG_LOADED="I r th isi fort th ke.",
    GOODBYE="Ant o!", PRESS_ENTER="Pat ste Enter...", ERROR_PREFIX="Sf l a",
    LANG_PROMPT="Ari th s l ssas (1-50): ", ENC_PROMPT="Krypt o raf isi", DEC_PROMPT="Apokrypt o raf isi",
    CONTACT_INFO="Epikoinonia: WeChat forever870422 / Email 810372789@qq.com",
    WELCOME_CONTRIB="Voith ste sti et frasi! forever870422")

# Hebrew
add("he", TITLE="=== ??? ?????? ????? ===", SELECT_LANG="--- ??? ??? ---", MAIN_MENU="--- ?????? ????? ---",
    ENCRYPT="1. ???? ???", DECRYPT="2. ???? ???", CHANGE_LANG="3. ???? ???", EXIT="4. ????",
    ENTER_CHOICE="???? ?????: ", INVALID_CHOICE="????? ??? ?????!", INPUT_FILE="????? ????: ", OUTPUT_FILE="????? ????: ",
    ENTER_PASSWORD="?????: ", CONFIRM_PASSWORD="???? ?????: ", PASSWORD_MISMATCH="??????? ?? ???????!",
    ENC_SUCCESS="????? ?????!", DEC_SUCCESS="????? ?????!",
    ENC_FAILED="????? ?????: ", DEC_FAILED="????? ?????: ",
    FILE_NOT_FOUND="???? ?? ?????!", WRONG_PASS="????? ?????? ?? ??? ????",
    CONFIG_SAVED="??????? ??????.", CONFIG_LOADED="??????? ?????.",
    GOODBYE="????!", PRESS_ENTER="??? Enter...", ERROR_PREFIX="????",
    LANG_PROMPT="???? ??? (1-50): ", ENC_PROMPT="????", DEC_PROMPT="????",
    CONTACT_INFO="????: WeChat forever870422 / Email 810372789@qq.com",
    WELCOME_CONTRIB="???? ??????! forever870422")

# Indonesian
add("id", TITLE="=== Alat Enkripsi/Dekripsi ===", SELECT_LANG="--- Pilih Bahasa ---", MAIN_MENU="--- Menu Utama ---",
    ENCRYPT="1. Enkripsi file", DECRYPT="2. Dekripsi file", CHANGE_LANG="3. Ganti bahasa", EXIT="4. Keluar",
    ENTER_CHOICE="Masukkan pilihan: ", INVALID_CHOICE="Pilihan tidak valid!", INPUT_FILE="File masukan: ", OUTPUT_FILE="File keluaran: ",
    ENTER_PASSWORD="Kata sandi: ", CONFIRM_PASSWORD="Konfirmasi kata sandi: ", PASSWORD_MISMATCH="Kata sandi tidak cocok!",
    ENC_SUCCESS="Enkripsi berhasil!", DEC_SUCCESS="Dekripsi berhasil!",
    ENC_FAILED="Enkripsi gagal: ", DEC_FAILED="Dekripsi gagal: ",
    FILE_NOT_FOUND="File tidak ditemukan!", WRONG_PASS="Kata sandi salah atau file rusak!",
    CONFIG_SAVED="Konfigurasi disimpan.", CONFIG_LOADED="Konfigurasi dimuat.",
    GOODBYE="Selamat tinggal!", PRESS_ENTER="Tekan Enter...", ERROR_PREFIX="Kesalahan",
    LANG_PROMPT="Nomor bahasa (1-50): ", ENC_PROMPT="Enkripsi", DEC_PROMPT="Dekripsi",
    CONTACT_INFO="Kontak: WeChat forever870422 / Email 810372789@qq.com",
    WELCOME_CONTRIB="Bantu terjemahan! forever870422")

# Malay
add("ms", TITLE="=== Alat Penyulitan ===", SELECT_LANG="--- Pilih Bahasa ---", MAIN_MENU="--- Menu Utama ---",
    ENCRYPT="1. Sulitkan fail", DECRYPT="2. Nyahsulit fail", CHANGE_LANG="3. Tukar bahasa", EXIT="4. Keluar",
    ENTER_CHOICE="Masukkan pilihan: ", INVALID_CHOICE="Pilihan tidak sah!", INPUT_FILE="Fail input: ", OUTPUT_FILE="Fail output: ",
    ENTER_PASSWORD="Kata laluan: ", CONFIRM_PASSWORD="Sahkan kata laluan: ", PASSWORD_MISMATCH="Kata laluan tidak sepadan!",
    ENC_SUCCESS="Penyulitan berjaya!", DEC_SUCCESS="Nyahsulit berjaya!",
    ENC_FAILED="Penyulitan gagal: ", DEC_FAILED="Nyahsulit gagal: ",
    FILE_NOT_FOUND="Fail tidak dijumpai!", WRONG_PASS="Kata laluan salah atau fail rosak!",
    CONFIG_SAVED="Konfigurasi disimpan.", CONFIG_LOADED="Konfigurasi dimuatkan.",
    GOODBYE="Selamat tinggal!", PRESS_ENTER="Tekan Enter...", ERROR_PREFIX="Ralat",
    LANG_PROMPT="Nombor bahasa (1-50): ", ENC_PROMPT="Sulitkan", DEC_PROMPT="Nyahsulit",
    CONTACT_INFO="Hubungi: WeChat forever870422 / Email 810372789@qq.com",
    WELCOME_CONTRIB="Bantu terjemahan! forever870422")

print(f"Part 3 done. Total: {len(langs)} languages")
