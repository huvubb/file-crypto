#include <windows.h>
#include <cstdio>
#include <shlobj.h>
#include <shobjidl.h>

#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#pragma comment(linker, "/ENTRY:mainCRTStartup")

#ifndef SLDF_RUNAS_USER
#define SLDF_RUNAS_USER 0x00000020
#endif

int main() {
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    IShellLinkW* psl = NULL;
    HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                                  IID_IShellLinkW, (void**)&psl);
    if (FAILED(hr) || !psl) {
        MessageBoxW(NULL, L"创建失败", L"错误", MB_ICONERROR);
        CoUninitialize(); return 1;
    }

    psl->SetPath(L"D:\\Desktop\\被报毒用管理员运行我.com");

    // 设置管理员运行标志
    IShellLinkDataList* pdl = NULL;
    hr = psl->QueryInterface(IID_IShellLinkDataList, (void**)&pdl);
    if (SUCCEEDED(hr) && pdl) {
        DWORD flags = 0;
        pdl->GetFlags(&flags);
        flags |= SLDF_RUNAS_USER;
        pdl->SetFlags(flags);
        pdl->Release();
    }

    IPersistFile* ppf = NULL;
    hr = psl->QueryInterface(IID_IPersistFile, (void**)&ppf);
    if (SUCCEEDED(hr) && ppf) {
        ppf->Save(L"D:\\Desktop\\被报毒用管理员运行我.lnk", TRUE);
        ppf->Release();
    }

    psl->Release();
    CoUninitialize();
    MessageBoxW(NULL, L"桌面快捷方式已创建\n双击即弹UAC", L"成功", MB_OK);
    return 0;
}
