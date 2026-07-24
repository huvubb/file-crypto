// MakeSFX - 把任意文件嵌入到 sfx_stub 生成自解压 exe
// 用法: MakeSFX.exe "目标文件.com" [输出路径.exe]

#include <windows.h>
#include <cstdio>
#include <vector>
#include <string>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("用法: MakeSFX.exe \"目标文件.com\" [输出文件名.exe]\n");
        printf("例: 把文件拖到 MakeSFX.exe 上即可\n");
        printf("按回车退出..."); getchar(); return 1;
    }

    // 路径转宽字符
    int len = MultiByteToWideChar(CP_UTF8, 0, argv[1], -1, NULL, 0);
    std::wstring src(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, argv[1], -1, &src[0], len);
    src.pop_back(); // 去掉末尾 null

    // 自动确定输出路径
    std::wstring out;
    if (argc >= 3) {
        len = MultiByteToWideChar(CP_UTF8, 0, argv[2], -1, NULL, 0);
        out.resize(len); MultiByteToWideChar(CP_UTF8, 0, argv[2], -1, &out[0], len); out.pop_back();
    } else {
        out = src + L"_installer.exe";
    }

    // 读目标文件
    HANDLE hSrc = CreateFileW(src.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hSrc == INVALID_HANDLE_VALUE) { printf("无法打开目标文件\n"); getchar(); return 1; }
    DWORD sz = GetFileSize(hSrc, NULL);
    std::vector<BYTE> data(sz);
    DWORD rd; ReadFile(hSrc, data.data(), sz, &rd, NULL); CloseHandle(hSrc);

    // 读 sfx_stub.exe
    wchar_t self[MAX_PATH]; GetModuleFileNameW(NULL, self, MAX_PATH);
    std::wstring stubPath = self;
    auto pos = stubPath.rfind(L'\\');
    if (pos != std::wstring::npos) stubPath = stubPath.substr(0, pos + 1) + L"sfx_stub.exe";
    else stubPath = L"sfx_stub.exe";

    HANDLE hStub = CreateFileW(stubPath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hStub == INVALID_HANDLE_VALUE) { printf("找不到 sfx_stub.exe（请放在同一目录）\n"); getchar(); return 1; }
    DWORD stubSz = GetFileSize(hStub, NULL);
    std::vector<BYTE> stub(stubSz);
    ReadFile(hStub, stub.data(), stubSz, &rd, NULL); CloseHandle(hStub);

    // 构造输出: [stub][data][4字节大小][SFX_MARK]
    std::vector<BYTE> trailer(12);
    *(DWORD*)trailer.data() = sz;
    memcpy(trailer.data() + 4, "SFX_MARK", 8);

    HANDLE hOut = CreateFileW(out.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
    if (hOut == INVALID_HANDLE_VALUE) { printf("无法创建输出文件\n"); getchar(); return 1; }
    DWORD wt;
    WriteFile(hOut, stub.data(), stubSz, &wt, NULL);
    WriteFile(hOut, data.data(), sz, &wt, NULL);
    WriteFile(hOut, trailer.data(), 12, &wt, NULL);
    CloseHandle(hOut);

    printf("自解压文件已生成: %S\n", out.c_str());
    printf("大小: %lu KB\n", (stubSz + sz + 12) / 1024);
    printf("按回车退出..."); getchar();
    return 0;
}
