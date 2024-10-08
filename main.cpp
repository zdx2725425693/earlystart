#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <tlhelp32.h>

#define STARTUP_PATH L".earlystart"  // 启动路径文件
#define LOG_FILE L"earlystart.log"    // 日志文件名

void MonitorLogin();
bool IsWinlogonRunning();
std::vector<std::wstring> ReadStartupPaths(const std::wstring& filePath);
void StartPrograms(const std::vector<std::wstring>& paths);
void LogMessage(const std::wstring& message);

std::wofstream logFile;

int main() {
    // 打开日志文件，清空之前的内容
    logFile.open(LOG_FILE, std::ios::out | std::ios::trunc);
    if (!logFile.is_open()) {
        std::wcerr << L"Failed to open log file: " << LOG_FILE << std::endl;
        return 1;
    }

    LogMessage(L"Program started."); // 添加启动日志
    MonitorLogin(); // 监控登录
    logFile.close(); // 关闭日志文件
    system("pause");
    return 0;
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
        std::wcerr << L"Failed to open file: " << filePath << std::endl;
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
    for (const auto& programPath : paths) {
        STARTUPINFOW si; // 使用宽字符版本的 STARTUPINFO
        PROCESS_INFORMATION pi;

        ZeroMemory(&si, sizeof(si)); // 清零启动信息结构
        si.cb = sizeof(si);           // 设置结构大小
        ZeroMemory(&pi, sizeof(pi));  // 清零进程信息结构

        // 使用 CreateProcessW 函数创建新进程
        if (!CreateProcessW(programPath.c_str(),  // 可执行文件路径
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
            std::wcerr << L"Failed to start program: " << programPath << L" Error: " << error << std::endl;
            LogMessage(L"Failed to start program: " + programPath + L" Error: " + std::to_wstring(error));
        } else {
            CloseHandle(pi.hProcess); // 关闭进程句柄
            CloseHandle(pi.hThread);  // 关闭线程句柄
            LogMessage(L"Started program: " + programPath); // 记录成功启动的程序
        }
    }
}

void LogMessage(const std::wstring& message) {
    SYSTEMTIME st;
    GetLocalTime(&st); // 获取当前本地时间
    wchar_t timeBuffer[100];

    // 使用 swprintf 格式化时间和消息
    swprintf(timeBuffer, 100, L"[%04d-%02d-%02d %02d:%02d:%02d] %ls\n",
             st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, message.c_str());

    // 使用 wprintf 打印宽字符到控制台
    wprintf(L"%ls", timeBuffer);

    // 写入日志文件
    logFile << timeBuffer;

    // 刷新文件流
    logFile.flush();
}