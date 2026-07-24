#include <windows.h>
#include <wincrypt.h>
#include <cstdio>
#include <vector>
#pragma comment(lib, "crypt32.lib")

#define MY (X509_ASN_ENCODING | PKCS_7_ASN_ENCODING)

static void Err(const char* p) {
    DWORD e = GetLastError(); LPWSTR m = NULL;
    FormatMessageW(0x1300, NULL, e, 0, (LPWSTR)&m, 0, NULL);
    printf("  [!] %%s (Err=%%lu): %%S\n", p, e, m ? m : L"");
    if (m) LocalFree(m);
}

static PCCERT_CONTEXT MakeCert(const wchar_t* subj, int y) {
    DWORD n = 0; CertStrToNameW(MY, subj, CERT_X500_NAME_STR, NULL, NULL, &n, NULL);
    std::vector<BYTE> nd(n); CERT_NAME_BLOB nb = {n, nd.data()};
    CertStrToNameW(MY, subj, CERT_X500_NAME_STR, NULL, nd.data(), &n, NULL);
    CRYPT_KEY_PROV_INFO ki = {0};
    wchar_t cn[] = L"FileCryptoKey"; ki.pwszContainerName = cn;
    ki.pwszProvName = MS_ENH_RSA_AES_PROV_W; ki.dwProvType = PROV_RSA_AES;
    ki.dwFlags = CRYPT_MACHINE_KEYSET; ki.dwKeySpec = AT_SIGNATURE;
    CRYPT_ALGORITHM_IDENTIFIER al = {0}; al.pszObjId = szOID_RSA_SHA256RSA;
    CERT_ENHKEY_USAGE eu = {0}; const char* oids[] = {szOID_PKIX_KP_CODE_SIGNING};
    eu.cUsageIdentifier = 1; eu.rgpszUsageIdentifier = (LPSTR*)oids;
    BYTE eud[128] = {0}; DWORD el = sizeof(eud);
    CERT_EXTENSION ee = {szOID_ENHANCED_KEY_USAGE, 0, {el, eud}};
    CryptEncodeObjectEx(MY, szOID_ENHANCED_KEY_USAGE, &eu, 0, NULL, eud, &el);
    CERT_BASIC_CONSTRAINTS2_INFO bc = {0};
    BYTE bcd[64] = {0}; DWORD bl = sizeof(bcd);
    CERT_EXTENSION be = {szOID_BASIC_CONSTRAINTS2, 0, {bl, bcd}};
    CryptEncodeObjectEx(MY, szOID_BASIC_CONSTRAINTS2, &bc, 0, NULL, bcd, &bl);
    CERT_EXTENSION exts[] = {ee, be}; CERT_EXTENSIONS ce = {2, exts};
    SYSTEMTIME et; GetSystemTime(&et); et.wYear += y;
    PCCERT_CONTEXT c = CertCreateSelfSignCertificate(
        (HCRYPTPROV_OR_NCRYPT_KEY_HANDLE)0, &nb, 0, &ki, &al, NULL, &et, &ce);
    if (!c) Err("CreateCert"); return c;
}

static bool Install(PCCERT_CONTEXT c, const wchar_t* sn) {
    HCERTSTORE s = CertOpenStore(CERT_STORE_PROV_SYSTEM_W, 0,
        (HCRYPTPROV_LEGACY)0, CERT_SYSTEM_STORE_LOCAL_MACHINE, sn);
    if (!s) { Err("OpenStore"); return false; }
    bool ok = CertAddCertificateContextToStore(s, c,
        CERT_STORE_ADD_REPLACE_EXISTING, NULL) ? true : false;
    if (!ok) Err("AddCert");
    printf("  %%s %%S\n", ok ? "[+]" : "[!]", sn);
    CertCloseStore(s, 0); return ok;
}

int main(int argc, char* argv[]) {
    SetConsoleOutputCP(CP_UTF8);
    printf("========================================\n");
    printf("  FileCrypto 证书安装工具 v1.0\n");
    printf("========================================\n\n");
    BOOL adm = FALSE; PSID ag = NULL;
    SID_IDENTIFIER_AUTHORITY na = SECURITY_NT_AUTHORITY;
    AllocateAndInitializeSid(&na, 2, SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,0,0,0,0,0,0,&ag);
    CheckTokenMembership(NULL, ag, &adm); FreeSid(ag);
    if (!adm) {
        printf("  [!] 需要管理员权限！请右键 → 以管理员身份运行\n\n");
        printf("按回车退出..."); getchar(); return 1;
    }
    printf("[1/3] 正在创建代码签名证书...\n");
    PCCERT_CONTEXT c = MakeCert(L"CN=FileCrypto, O=FileCrypto, C=CN", 20);
    if (!c) { printf("Failed\n按回车退出..."); getchar(); return 1; }
    printf("  [+] 证书创建成功（有效期20年）\n\n");
    printf("[2/3] 正在安装到受信任存储区...\n");
    Install(c, L"ROOT");
    Install(c, L"TRUSTEDPUBLISHER");
    HCERTSTORE ms = CertOpenStore(CERT_STORE_PROV_SYSTEM_W, 0,
        (HCRYPTPROV_LEGACY)0, CERT_SYSTEM_STORE_LOCAL_MACHINE, L"MY");
    if (ms) {
        CertAddCertificateContextToStore(ms, c,
            CERT_STORE_ADD_REPLACE_EXISTING, NULL);
        CertCloseStore(ms, 0); printf("  [+] MY（个人证书存储区）\n");
    }
    printf("\n[3/3] ");
    if (argc > 1) {
        int len = MultiByteToWideChar(CP_UTF8, 0, argv[1], -1, NULL, 0);
        std::vector<wchar_t> wp(len);
        MultiByteToWideChar(CP_UTF8, 0, argv[1], -1, wp.data(), len);
        printf("Signing: %%S\n", wp.data());
        wchar_t cmd[1024];
        swprintf(cmd, 1024, L"signtool sign /a /fd SHA256 /q \"%%s\"", wp.data());
        int r = _wsystem(cmd);
        if (r != 0) printf("  [!] 未找到 signtool，请手动签名\n");
        else printf("  [+] 签名成功\n");
    } else {
        printf("拖放 FileCrypto.exe 到此程序即可签名\n");
    }
    CertFreeCertificateContext(c);
    printf("\n========================================\n");
    printf("  完成！证书已安装到:\n");
    printf("  - ROOT（受信任根证书颁发机构）\n");
    printf("  - TrustedPublisher（受信任发布者）\n");
    printf("========================================\n\n");
    printf("按回车退出..."); getchar(); return 0;
}
