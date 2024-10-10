

#ifndef __mainwork_h__

#define __mainwork_h__

#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <tlhelp32.h>
#include "../log/log.h"

void MonitorLogin();
bool IsWinlogonRunning();
std::vector<std::wstring> ReadStartupPaths(const std::wstring& filePath);
void StartPrograms(const std::vector<std::wstring>& paths);
void LogMessage(const std::wstring& message);
BOOL workmain();

#endif