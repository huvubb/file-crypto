path = r"C:\Users\邓涛\Documents\New project 3\file-crypto\src\i18n.cpp"
with open(path, "r", encoding="utf-8") as f:
    content = f.read()

getlangs = '''
const std::vector<I18n::LangInfo>& I18n::GetLanguages() {
    static const std::vector<LangInfo> langs = {
        {"en",    "English"},
        {"zh-CN", "\u7b80\u4f53\u4e2d\u6587 (Simplified Chinese)"},
        {"cn-tw", "\u7e41\u9ad4\u4e2d\u6587 (Traditional Chinese)"},
        {"es",    "Espa\u00f1ol (Spanish)"},
        {"fr",    "Fran\u00e7ais (French)"},
        {"de",    "Deutsch (German)"},
        {"ja",    "\u65e5\u672c\u8a9e (Japanese)"},
        {"ko",    "\ud55c\uad6d\uc5b4 (Korean)"},
        {"ru",    "\u0420\u0443\u0441\u0441\u043a\u0438\u0439 (Russian)"},
        {"ar",    "\u0627\u0644\u0639\u0631\u0628\u064a\u0629 (Arabic)"},
        {"pt",    "Portugu\u00eas (Portuguese)"},
        {"it",    "Italiano (Italian)"},
        {"nl",    "Nederlands (Dutch)"},
        {"hi",    "\u0939\u093f\u0928\u094d\u0926\u0940 (Hindi)"},
        {"tr",    "T\u00fcrk\u00e7e (Turkish)"},
        {"vi",    "Ti\u1ebfng Vi\u1ec7t (Vietnamese)"},
        {"th",    "\u0e44\u0e17\u0e22 (Thai)"},
        {"pl",    "Polski (Polish)"},
        {"uk",    "\u0423\u043a\u0440\u0430\u0457\u043d\u0441\u044c\u043a\u0430 (Ukrainian)"},
        {"sv",    "Svenska (Swedish)"},
        {"da",    "Dansk (Danish)"},
        {"fi",    "Suomi (Finnish)"},
        {"no",    "Norsk (Norwegian)"},
        {"cs",    "\u010ce\u0161tina (Czech)"},
        {"hu",    "Magyar (Hungarian)"},
        {"ro",    "Rom\u00e2n\u0103 (Romanian)"},
        {"el",    "\u0395\u03bb\u03bb\u03b7\u03bd\u03b9\u03ba\u03ac (Greek)"},
        {"he",    "\u05e2\u05d1\u05e8\u05d9\u05ea (Hebrew)"},
        {"id",    "Bahasa Indonesia"},
        {"ms",    "Bahasa Melayu (Malay)"},
        {"bn",    "\u09ac\u09be\u0982\u09b2\u09be (Bengali)"},
        {"ur",    "\u0627\u0631\u062f\u0648 (Urdu)"},
        {"fa",    "\u0641\u0627\u0631\u0633\u06cc (Persian)"},
        {"ta",    "\u0ba4\u0bae\u0bbf\u0bb4\u0bcd (Tamil)"},
        {"te",    "\u0c24\u0c46\u0c32\u0c41\u0c17\u0c41 (Telugu)"},
        {"mr",    "\u092e\u0930\u093e\u0920\u0940 (Marathi)"},
        {"kn",    "\u0c95\u0ca8\u0ccd\u0ca8\u0ca1 (Kannada)"},
        {"gu",    "\u0a97\u0ac1\u0a9c\u0ab0\u0abe\u0aa4\u0ac0 (Gujarati)"},
        {"pa",    "\u0a2a\u0a70\u0a1c\u0a3e\u0a2c\u0a40 (Punjabi)"},
        {"my",    "\u1019\u103c\u1014\u103a\u1019\u102c (Burmese)"},
        {"km",    "\u1797\u17b6\u179f\u17b6\u1781\u17d2\u1798\u17c2\u179a (Khmer)"},
        {"lo",    "\u0ea5\u0eb2\u0ea7 (Lao)"},
        {"ne",    "\u0928\u0947\u092a\u093e\u0932\u0940 (Nepali)"},
        {"si",    "\u0dc3\u0dd2\u0d82\u0dc4\u0dbd (Sinhala)"},
        {"am",    "\u12a0\u121b\u122d\u129b (Amharic)"},
        {"sw",    "Kiswahili (Swahili)"},
        {"fil",   "Filipino (Tagalog)"},
        {"sr",    "\u0421\u0440\u043f\u0441\u043a\u0438 (Serbian)"},
        {"hr",    "Hrvatski (Croatian)"},
        {"bg",    "\u0411\u044a\u043b\u0433\u0430\u0440\u0441\u043a\u0438 (Bulgarian)"}
    };
    return langs;
}
'''
content += getlangs
with open(path, "w", encoding="utf-8") as f:
    f.write(content)
print("Added GetLanguages() to i18n.cpp")
import os
print(f"Final size: {os.path.getsize(path):,} bytes")
