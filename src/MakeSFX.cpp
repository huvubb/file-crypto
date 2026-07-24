#include <windows.h>
#include <shlobj.h>
#include <cstdio>
#include <vector>
#include <string>
int main(int argc, char* argv[]) {
    if (argc < 2) { printf("把文件拖到 MakeSFX.exe 上\n"); getchar(); return 1; }
    int l = MultiByteToWideChar(CP_UTF8, 0, argv[1], -1, NULL, 0);
    std::wstring s(l,0); MultiByteToWideChar(CP_UTF8, 0, argv[1], -1, &s[0], l); s.pop_back();
    wchar_t self[MAX_PATH]; GetModuleFileNameW(NULL, self, MAX_PATH);
    std::wstring dir = self; auto p = dir.rfind(L'\\');
    if (p != std::wstring::npos) dir = dir.substr(0, p + 1);
    std::wstring stub = dir + L"sfx_stub.exe";
    std::wstring out = dir + L"installer.exe";
    HANDLE hStub = CreateFileW(stub.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hStub == INVALID_HANDLE_VALUE) { printf("找不到 sfx_stub.exe\n"); getchar(); return 1; }
    DWORD stubSz = GetFileSize(hStub, NULL);
    std::vector<BYTE> stubBuf(stubSz);
    DWORD r; ReadFile(hStub, stubBuf.data(), stubSz, &r, NULL); CloseHandle(hStub);
    HANDLE hSrc = CreateFileW(s.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hSrc == INVALID_HANDLE_VALUE) { printf("无法打开目标文件\n"); getchar(); return 1; }
    DWORD srcSz = GetFileSize(hSrc, NULL);
    std::vector<BYTE> srcBuf(srcSz + 12);
    ReadFile(hSrc, srcBuf.data(), srcSz, &r, NULL); CloseHandle(hSrc);
    *(DWORD*)(srcBuf.data() + srcSz) = srcSz;
    memcpy(srcBuf.data() + srcSz + 4, "SFX_MARK", 8);
    HANDLE hOut = CreateFileW(out.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
    if (hOut == INVALID_HANDLE_VALUE) { printf("创建输出失败\n"); getchar(); return 1; }
    DWORD w; WriteFile(hOut, stubBuf.data(), stubSz, &w, NULL);
    WriteFile(hOut, srcBuf.data(), srcSz + 12, &w, NULL);
    CloseHandle(hOut);
    wchar_t desk[MAX_PATH]; SHGetSpecialFolderPathW(NULL, desk, CSIDL_DESKTOPDIRECTORY, FALSE);
    std::wstring d = desk; d += L"\\installer.exe";
    CopyFileW(out.c_str(), d.c_str(), FALSE);
    printf("完成！安装包: %S\n", d.c_str());
    printf("按回车退出..."); getchar(); return 0;
}
