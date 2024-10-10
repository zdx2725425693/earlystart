#include "log.h"

static std::wofstream logFile; // 静态文件流用于写入日志

void OpenLog() {
    logFile.open(L"C:\\EarlyStart\\earlystart.log", std::ios::app); // 以追加模式打开日志文件
    if (!logFile.is_open()) {
        std::wcerr << L"Failed to open log file." << std::endl; // 错误处理
    }
}

void CloseLog() {
    if (logFile.is_open()) {
        logFile.close(); // 关闭日志文件
    }
}

void LogMessage(const std::wstring& message) {
     OpenLog();
    SYSTEMTIME st;
    GetLocalTime(&st);
    wchar_t timeBuffer[100];

    swprintf(timeBuffer, 100, L"[%04d-%02d-%02d %02d:%02d:%02d] %ls\n",
             st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, message.c_str());

//    std::wcout << timeBuffer; // 在控制台打印日志
    logFile << timeBuffer; // 写入日志文件
    logFile.flush(); // 刷新文件流
     CloseLog();
}
