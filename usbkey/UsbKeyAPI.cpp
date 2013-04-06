// UsbKeyAPI.cpp : 定义 DLL 应用程序的入口点。
//

#include "stdafx.h"
#include <tchar.h>
#include "./inc/AK_Log.h"
#include "./inc/AK_Mutex.h"


#ifdef _MANAGED
#pragma managed(push, off)
#endif

#pragma   data_seg("UsbKeyAPIDataName")//開設共享數據段  
HHOOK g_hHook = NULL;  
int g_DevNum = 0;

#pragma   data_seg()     
#pragma comment(linker,"/section:UsbKeyAPIDataName,rws")

HANDLE g_hDevEvent = NULL;
HINSTANCE g_hModule = NULL; 
void* gpMutex = NULL;


BOOL APIENTRY DllMain( HMODULE hModule,
        DWORD  ul_reason_for_call,
        LPVOID lpReserved
        )
{
    switch(ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            g_hModule = (HINSTANCE)hModule;
            //创建设备插拔事件
            g_hDevEvent = CreateEvent(
                    NULL,				// 安全属性
                    TRUE,				// 复位方式: TRUE：手动；FALSE：自动
                    FALSE,				// 初始状态: TRUE：有信号；FALSE：无信号
                    _T("AIUSBKEY_EVENT"));	// 对象名称
            if(!g_hDevEvent)
            {
                AK_Log(AK_LOG_ERROR, "create event failed.");
                return FALSE;
            }

            if( 0 != AK_CreateMutex((char *)"gUKeyAPIMutex",&gpMutex) )
            {
                AK_Log(AK_LOG_ERROR, "DllMain ==> AK_CreateMutex Error");
                return FALSE;
            }
            break;

        case DLL_PROCESS_DETACH:
            g_hModule=NULL;
            CloseHandle(g_hDevEvent);
            //销毁锁
            if( gpMutex == NULL )
            {
                AK_Log(AK_LOG_ERROR, "DllMain ==> gpMutex not create");
                return FALSE;
            }
            AK_DestroyMutex(gpMutex);
            gpMutex = NULL;
            break;

    }
    return TRUE;

}

#ifdef _MANAGED
#pragma managed(pop)
#endif

