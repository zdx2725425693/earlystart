#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <tlhelp32.h>

#define STARTUP_PATH L".earlystart"
#define LOG_FILE L"earlystart.log"

void MonitorLogin();
bool IsWinlogonRunning();
std::vector<std::wstring> ReadStartupPaths(const std::wstring& filePath);
void StartPrograms(const std::vector<std::wstring>& paths);
void LogMessage(const std::wstring& message);

std::wofstream logFile;

int main() {
    logFile.open(LOG_FILE, std::ios::app);
    if (!logFile.is_open()) {
        std::wcerr << L"Failed to open log file: " << LOG_FILE << std::endl;
        return 1;
    }

    MonitorLogin();
    system("pause");
    logFile.close();
    return 0;
}

void MonitorLogin() {
    while (true) {
        if (IsWinlogonRunning()) {
            LogMessage(L"Login detected successfully.");
            std::vector<std::wstring> startupPaths = ReadStartupPaths(STARTUP_PATH);
            StartPrograms(startupPaths);
            break;
        }
        Sleep(800);
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
                return true;
            }
        } while (Process32NextW(hSnapshot, &processEntry));
    }
    CloseHandle(hSnapshot);
    return false;
}

std::vector<std::wstring> ReadStartupPaths(const std::wstring& filePath) {
    std::vector<std::wstring> paths;
    std::wifstream file(filePath.c_str());
    if (!file.is_open()) {
        std::wcerr << L"Failed to open file: " << filePath << std::endl;
        return paths;
    }

    std::wstring line;
    while (std::getline(file, line)) {
        if (!line.empty() && line[0] != L'#') {
            paths.push_back(line);
        }
    }

    return paths;
}

void StartPrograms(const std::vector<std::wstring>& paths) {
    for (const auto& programPath : paths) {
        STARTUPINFOW si; // 使用宽字符版本的 STARTUPINFO
        PROCESS_INFORMATION pi;

        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

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
            std::wcerr << L"Failed to start program: " << programPath << L" Error: " << GetLastError() << std::endl;
            LogMessage(L"Failed to start program: " + programPath);
        } else {
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            LogMessage(L"Started program: " + programPath);
        }
    }
}

void LogMessage(const std::wstring& message) {
    SYSTEMTIME st;
    GetLocalTime(&st);
    wchar_t timeBuffer[100];
    swprintf(timeBuffer, 100, L"[%04d-%02d-%02d %02d:%02d:%02d] %s\n",
             st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, message.c_str());

    logFile << timeBuffer; // 写入日志文件
}
