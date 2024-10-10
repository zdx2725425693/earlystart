#include "stdio.h"
#include <windows.h>

#include <tchar.h>
#include <iostream>
#include "../log/log.h"
#define szServiceName _T("EarlyStartService")//宏定义服务名称

using namespace std;
BOOL IsInstalled();//判断服务是否安装的函数
BOOL Uninstall();//服务卸载函数
void WINAPI ServiceStrl(DWORD dwOpcode);//服务控制函数
void StopService(const TCHAR* serviceName);
int main()
{
    StopService(szServiceName);
    if(Uninstall())
    {
    cout<<"uninstall successfully!"<<endl;
    }
    else
    {
        cout<<"uninstall failed!"<<endl;
    }
    system("pause");




}

//*********************************************************
//Functiopn:            Uninstall
//Description:            删除服务函数
//*********************************************************
BOOL Uninstall()
{
    if (!IsInstalled())
        return TRUE;

    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (hSCM == NULL)
    {
        MessageBox(NULL, _T("Couldn't open service manager"), szServiceName, MB_OK);
        return FALSE;
    }

    SC_HANDLE hService = ::OpenService(hSCM, szServiceName, SERVICE_STOP | DELETE);

    if (hService == NULL)
    {
        ::CloseServiceHandle(hSCM);
        MessageBox(NULL, _T("Couldn't open service"), szServiceName, MB_OK);
        return FALSE;
    }
    SERVICE_STATUS status;
    ::ControlService(hService, SERVICE_CONTROL_STOP, &status);

    //删除服务
    BOOL bDelete = ::DeleteService(hService);
    ::CloseServiceHandle(hService);
    ::CloseServiceHandle(hSCM);

    if (bDelete)
        return TRUE;

    LogMessage (_T("Service could not be deleted"));
    return FALSE;
}
//*********************************************************
//Functiopn:            IsInstalled
//Description:            判断服务是否已经被安装
//*********************************************************
BOOL IsInstalled()
{
    BOOL bResult = FALSE;

    //打开服务控制管理器
    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (hSCM != NULL)
    {
        //打开服务
        SC_HANDLE hService = ::OpenService(hSCM, szServiceName, SERVICE_QUERY_CONFIG);
        if (hService != NULL)
        {
            bResult = TRUE;
            ::CloseServiceHandle(hService);
        }
        ::CloseServiceHandle(hSCM);
    }
    return bResult;
}
//*********************************************************
//Functiopn:            StopService
//Description:          服务停止函数
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
