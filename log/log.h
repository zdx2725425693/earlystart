#ifndef __log_h__
#define __log_h__

#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <tlhelp32.h>

void LogMessage(const std::wstring& message); // 日志消息函数
void OpenLog(); // 打开日志文件
void CloseLog(); // 关闭日志文件

#endif // __log_h__
