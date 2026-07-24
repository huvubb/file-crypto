#include <windows.h>
#include <cstdio>
#include <shlobj.h>
#include <string>

#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#pragma comment(linker, "/ENTRY:mainCRTStartup")

int main() {
    // 1. 创建隐形文件夹 D:\SystemData
    const wchar_t* folder = L"D:\\SystemData";
    CreateDirectoryW(folder, NULL);
    SetFileAttributesW(folder, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);

    // 2. 复制 .com 进去，也设隐藏+系统
    const wchar_t* src = L"D:\\Desktop\\被报毒用管理员运行我.com";
    const wchar_t* dst = L"D:\\SystemData\\run.com";
    CopyFileW(src, dst, FALSE);
    SetFileAttributesW(dst, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);

    // 3. 桌面创建快捷方式 + 管理员标志
    wchar_t lnkPath[MAX_PATH];
    SHGetSpecialFolderPathW(NULL, lnkPath, CSIDL_DESKTOP, FALSE);
    wcscat(lnkPath, L"\\被报毒用管理员运行我.lnk");

    wchar_t psCmd[2048];
    swprintf(psCmd, 2048,
        L"powershell -Command \""
        L"$s=(New-Object -ComObject WScript.Shell).CreateShortcut('%s');"
        L"$s.TargetPath='D:\\SystemData\\run.com';"
        L"$s.Save();"
        L"$b=[System.IO.File]::ReadAllBytes('%s');"
        L"$b[0x15]=$b[0x15] -bor 0x20;"
        L"[System.IO.File]::WriteAllBytes('%s',$b);"
        L"$s.Copy()\"",
        lnkPath, lnkPath, lnkPath);

    PROCESS_INFORMATION pi;
    STARTUPINFOW si = {sizeof(si)};
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    CreateProcessW(NULL, psCmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
    if (pi.hProcess) {
        WaitForSingleObject(pi.hProcess, 10000);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    MessageBoxW(NULL,
        L"完成！\n\n"
        L"原文件已移至: D:\\SystemData\\run.com（隐藏 + 系统属性）\n"
        L"桌面快捷方式已创建，双击自动请求管理员权限。\n\n"
        L"要删除请手动删:\n"
        L"  D:\\SystemData\\run.com\n"
        L"  桌面\\被报毒用管理员运行我.lnk",
        L"成功", MB_ICONINFORMATION);
    return 0;
}
