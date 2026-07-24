#include <windows.h>
#include <cstdio>

// 无控制台窗口
#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#pragma comment(linker, "/ENTRY:mainCRTStartup")

static bool IsElevated() {
    BOOL elevated = FALSE;
    HANDLE hToken = NULL;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        TOKEN_ELEVATION te;
        DWORD size = sizeof(te);
        if (GetTokenInformation(hToken, TokenElevation, &te, size, &size))
            elevated = te.TokenIsElevated;
        CloseHandle(hToken);
    }
    return elevated == TRUE;
}

int main() {
    // 如果没有管理员权限，提升自己
    if (!IsElevated()) {
        wchar_t path[MAX_PATH];
        GetModuleFileNameW(NULL, path, MAX_PATH);
        ShellExecuteW(NULL, L"runas", path, NULL, NULL, SW_SHOW);
        return 0;
    }

    // 已拥有管理员权限 → 启动目标程序
    SHELLEXECUTEINFOW sei = {0};
    sei.cbSize = sizeof(sei);
    sei.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_NO_CONSOLE;
    sei.lpVerb = L"open";
    sei.lpFile = L"D:\\Desktop\\被报毒用管理员运行我.com";
    sei.nShow = SW_SHOWNORMAL;

    if (!ShellExecuteExW(&sei)) {
        DWORD err = GetLastError();
        wchar_t msg[256];
        swprintf(msg, 256, L"启动失败 (Err=%lu)\n请确认文件路径正确", err);
        MessageBoxW(NULL, msg, L"错误", MB_ICONERROR);
        return 1;
    }
    return 0;
}
