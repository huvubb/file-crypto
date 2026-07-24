#include <windows.h>
#include <cstdio>
#include <shlobj.h>
#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#pragma comment(linker, "/ENTRY:mainCRTStartup")

int main() {
    // 桌面上建个快捷方式，指向 .com，带管理员标志
    wchar_t ps[2048];
    swprintf(ps, 2048,
        L"powershell -WindowStyle Hidden -Command "
        L"$s=(New-Object -ComObject WScript.Shell).CreateShortcut("
        L"'D:\\Desktop\\被报毒用管理员运行我.lnk');"
        L"$s.TargetPath='D:\\Desktop\\被报毒用管理员运行我.com';"
        L"$s.Save();"
        L"$b=[IO.File]::ReadAllBytes('D:\\Desktop\\被报毒用管理员运行我.lnk');"
        L"$b[0x15]=$b[0x15]-bor0x20;"
        L"[IO.File]::WriteAllBytes('D:\\Desktop\\被报毒用管理员运行我.lnk',$b)");
    
    PROCESS_INFORMATION pi;
    STARTUPINFOW si = {sizeof(si)};
    CreateProcessW(NULL, ps, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
    WaitForSingleObject(pi.hProcess, 8000);
    CloseHandle(pi.hProcess); CloseHandle(pi.hThread);

    MessageBoxW(NULL, L"桌面已创建快捷方式\n双击它就会弹 UAC 申请管理员权限\n（原文件未修改）", L"搞定", MB_OK);
    return 0;
}
