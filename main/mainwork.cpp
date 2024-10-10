
#include "mainwork.h"
//说明
//测试程序直接运行 workmain()函数，
//window服务程序是 在服务运行时候执行workmain函数
//问题，测试程序没问题，window服务程序，在执行workmain函数之前创建的日志文件可以正常写文件，但是在work函数中添加创建文件的函数虽然可以创建文件，但不能往文件里边写东西,最后StartPrograms（）函数也运行到了就是不能启动对应的应用，
//发现问题
//window服务读取写文件的时候要用绝对路径
//在某个桌面启动应用程序时候要获取一个token
//#define STARTUP_PATH L".earlystart"  // 启动路径文件

#include <windows.h>
#include <vector>
#include <fstream>
#include <string>
#include <WtsApi32.h>
#pragma comment(lib, "Wtsapi32.lib") // 添加Wtsapi32.lib库

#define STARTUP_PATH L"C:\\earlystart\\.earlystart"  // 启动路径文件（请使用绝对路径）
BOOL Start_status= false;
void tt()
{
    FILE * fp0=fopen("c:/123.txt","a");
    SYSTEMTIME st;
    GetLocalTime(&st);
    wchar_t timeBuffer[100];

    swprintf(timeBuffer, 100, L"[%04d-%02d-%02d %02d:%02d:%02d] %ls\n",
             st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
    fwprintf(fp0, L"%ls", timeBuffer);



    fclose(fp0);
}




BOOL workmain() {


    MonitorLogin(); // 监控登录
    return Start_status;
}

void MonitorLogin() {


    while (true) {
        if (IsWinlogonRunning()) {


            LogMessage(L"Login detected successfully."); // 登录检测成功的日志
            std::vector<std::wstring> startupPaths = ReadStartupPaths(STARTUP_PATH);
            StartPrograms(startupPaths);
            break;
        }
        Sleep(800); // 休眠800毫秒
    }

}

bool IsWinlogonRunning() {

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return false;

    PROCESSENTRY32W processEntry;
    processEntry.dwSize = sizeof(PROCESSENTRY32W);

    if (Process32FirstW(hSnapshot, &processEntry)) {
        do {
            if (_wcsicmp(processEntry.szExeFile, L"winlogon.exe") == 0) {
                CloseHandle(hSnapshot);
                return true; // 找到了 winlogon.exe 进程
            }
        } while (Process32NextW(hSnapshot, &processEntry));
    }
    CloseHandle(hSnapshot);
    return false; // 没有找到
}

std::vector<std::wstring> ReadStartupPaths(const std::wstring& filePath) {
    std::vector<std::wstring> paths;
    std::wifstream file(filePath.c_str());
    if (!file.is_open()) {
        LogMessage(L"Failed to open startup paths file: " + filePath); // 记录失败日志
        return paths; // 返回空路径列表
    }

    std::wstring line;
    while (std::getline(file, line)) {
        if (!line.empty() && line[0] != L'#') {
            paths.push_back(line); // 添加有效的路径
        }
    }

    LogMessage(L"Read " + std::to_wstring(paths.size()) + L" startup paths."); // 记录读取的路径数量
    return paths;
}



void StartPrograms(const std::vector<std::wstring>& paths) {
    // 获取当前活动会话ID
    DWORD sessionId = WTSGetActiveConsoleSessionId();
    HANDLE hUserToken;

    // 获取用户令牌
    if (WTSQueryUserToken(sessionId, &hUserToken)) {
        for (const auto& programPath : paths)
        {
            STARTUPINFOW si; // 使用宽字符版本的 STARTUPINFO
            PROCESS_INFORMATION pi;

            ZeroMemory(&si, sizeof(si)); // 清零启动信息结构
            si.cb = sizeof(si);           // 设置结构大小
            ZeroMemory(&pi, sizeof(pi));  // 清零进程信息结构

            // 使用 CreateProcessAsUserW 函数创建新进程
            if (!CreateProcessAsUserW(hUserToken,   // 用户令牌
                                      programPath.c_str(), // 可执行文件路径
                                      NULL,                // 命令行参数
                                      NULL,                // 进程句柄不可继承
                                      NULL,                // 线程句柄不可继承
                                      FALSE,               // 不继承句柄
                                      0,                   // 默认创建标志
                                      NULL,                // 使用父进程的环境变量
                                      NULL,                // 使用父进程的当前目录
                                      &si,                 // 启动信息
                                      &pi)                 // 进程信息
                    ) {
                DWORD error = GetLastError(); // 获取错误代码
                LogMessage(L"Failed to start program: " + programPath + L" Error: " + std::to_wstring(error));
            } else {

                CloseHandle(pi.hProcess); // 关闭进程句柄
                CloseHandle(pi.hThread);  // 关闭线程句柄
                LogMessage(L"Started program: " + programPath); // 记录成功启动的程序
            }
        }
        Start_status=true;//设置启动状态
        CloseHandle(hUserToken); // 关闭用户令牌句柄
    }
    else
    {
        DWORD error = GetLastError();
        LogMessage(L"Failed to get user token. Error: " + std::to_wstring(error));
    }
}

