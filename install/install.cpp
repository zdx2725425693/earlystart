#include "stdio.h"
#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <fstream>
#include "log.h"



TCHAR szServiceName[] = _T("EarlyStartService");
using namespace std;
#define STARTUP_DIR "C:\\EarlyStart"              // 配置文件存放位置
#define STARTUP_PATH "C:\\EarlyStart\\.earlystart" // 配置文件路径
BOOL IsInstalled(); // 判断服务是否安装的函数
BOOL Install();     // 服务安装函数
BOOL Uninstall();   // 服务卸载函数
void StopService(const TCHAR* serviceName);
bool Creat_configfile();

int main() {


    if (IsInstalled()) {
        StopService(szServiceName);
        if (Uninstall()) {
            LogMessage(L"Service be deleted");
            wcout << L"Service be deleted" << std::endl;
        } else {
            LogMessage(L"Service could not be deleted");
            wcout << L"Service could not be deleted" << std::endl;
            return 1;
        }
    }

    if (Install()) {


        LogMessage(L"Service be installed");
        std::wcout << L"Service be installed" << std::endl;
        if(Creat_configfile())
        {
            LogMessage(L"Creat config file successfully ");

        }
    } else {
        LogMessage(L"Service could not be install");
        std::wcout << L"Service could not be install" << std::endl;
        return 1;
    }

    system("pause");
    return 0;
}

//*********************************************************
// Function: Install
// Description: 安装服务函数
//*********************************************************
BOOL Install() {
    if (IsInstalled())
        return TRUE;

    // 打开服务控制管理器
    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hSCM == NULL) {
        MessageBox(NULL, _T("Couldn't open service manager"), szServiceName, MB_OK);
        return FALSE;
    }

    // 获取当前执行程序的路径
    TCHAR szCurrentPath[MAX_PATH];
    ::GetModuleFileName(NULL, szCurrentPath, MAX_PATH);

    // 获取文件名的目录部分
    TCHAR* lastSlash = _tcsrchr(szCurrentPath, _T('\\'));
    if (lastSlash != NULL) {
        // 将文件名部分替换为 earlystart.exe
        _tcscpy(lastSlash + 1, _T("earlystart.exe"));
    }

    // 创建服务，设置为自启动类型
    SC_HANDLE hService = ::CreateService(
            hSCM,
            szServiceName,
            szServiceName,
            SERVICE_ALL_ACCESS,
            SERVICE_WIN32_OWN_PROCESS,
            SERVICE_AUTO_START, // 自启动类型
            SERVICE_ERROR_NORMAL,
            szCurrentPath,
            NULL,
            NULL,
            _T(""),
            NULL, // 使用 LocalSystem 账户
            NULL  // 这里传 NULL 将使用 LocalSystem 帐户
    );

    if (hService == NULL) {
        ::CloseServiceHandle(hSCM);
        MessageBox(NULL, _T("Couldn't create service"), szServiceName, MB_OK);
        return FALSE;
    }

    ::CloseServiceHandle(hService);
    ::CloseServiceHandle(hSCM);
    return TRUE;
}

//*********************************************************
// Function: Uninstall
// Description: 删除服务函数
//*********************************************************
BOOL Uninstall() {
    if (!IsInstalled())
        return TRUE;

    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hSCM == NULL) {
        MessageBox(NULL, _T("Couldn't open service manager"), szServiceName, MB_OK);
        return FALSE;
    }

    SC_HANDLE hService = ::OpenService(hSCM, szServiceName, SERVICE_STOP | DELETE);
    if (hService == NULL) {
        ::CloseServiceHandle(hSCM);
        MessageBox(NULL, _T("Couldn't open service"), szServiceName, MB_OK);
        return FALSE;
    }

    SERVICE_STATUS status;
    ::ControlService(hService, SERVICE_CONTROL_STOP, &status);

    // 删除服务
    BOOL bDelete = ::DeleteService(hService);
    ::CloseServiceHandle(hService);
    ::CloseServiceHandle(hSCM);

    if (bDelete)
        return TRUE;

    return FALSE;
}

//*********************************************************
// Function: StopService
// Description: 服务停止函数
//*********************************************************
void StopService(const TCHAR* serviceName) {
    // 打开服务控制管理器
    SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hSCM == NULL) {
        std::cerr << "Failed to open service manager." << std::endl;
        return;
    }

    // 打开指定的服务
    SC_HANDLE hService = OpenService(hSCM, serviceName, SERVICE_STOP | SERVICE_QUERY_STATUS);
    if (hService == NULL) {
        std::cerr << "Failed to open service." << std::endl;
        CloseServiceHandle(hSCM);
        return;
    }

    SERVICE_STATUS status;
    // 发送停止命令
    if (ControlService(hService, SERVICE_CONTROL_STOP, &status)) {
        std::cout << "Service is stopping..." << std::endl;

        // 等待服务停止
        while (QueryServiceStatus(hService, &status)) {
            if (status.dwCurrentState == SERVICE_STOPPED) {
                std::cout << "Service stopped successfully." << std::endl;
                break;
            }
            Sleep(1000); // 等待一秒钟再查询状态
        }
    } else {
        std::cerr << "Failed to stop service." << std::endl;
    }

    // 关闭句柄
    CloseServiceHandle(hService);
    CloseServiceHandle(hSCM);
}

//*********************************************************
// Function: IsInstalled
// Description: 判断服务是否已经被安装
//*********************************************************
BOOL IsInstalled() {
    BOOL bResult = FALSE;

    // 打开服务控制管理器
    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (hSCM != NULL) {
        // 打开服务
        SC_HANDLE hService = ::OpenService(hSCM, szServiceName, SERVICE_QUERY_CONFIG);
        if (hService != NULL) {
            bResult = TRUE;
            ::CloseServiceHandle(hService);
        }
        ::CloseServiceHandle(hSCM);
    }
    return bResult;
}
//*********************************************************
// Function: Creat_configfile
// Description: 创建配置文件
//*********************************************************

bool Creat_configfile()
{


    string mkdirCommand = "mkdir \"" + string(STARTUP_DIR) + "\"";
    int result = system(mkdirCommand.c_str()); // 调用 mkdir 命令

    // 检查是否成功执行命令
    if (result != 0) {
        cerr << "Directory could not be created via command line." << endl;
        return false;
    }


    // 创建配置文件
    ofstream outfile(STARTUP_PATH);

    // 检查文件是否成功创建
    if (!outfile) {
        cerr << "File could not be created." << endl;
        return false;
    }

    // 写入内容到文件
    outfile << "D:\\SteamLibrary\\steamapps\\common\\MyDockFinder\\Dock_64.exe\n";

    // 关闭文件
    outfile.close();

    cout << "File and directory created successfully." << endl;
    return true;


}











