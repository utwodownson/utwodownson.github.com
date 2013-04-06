/*	UsbKeyAPI.c
Author  :   caizhun@aisino.com
CO.     :   AISINO
date    :   2008年5月26日，
描述    :   USBKEY底层API接口	*/

#include "stdafx.h"
#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include <Dbt.h>
#include "./inc/global.h"

#include "./inc/UsbKeyAPI.h"
#include "./inc/UsbKeyComDLL.h"

#include "./inc/DataPackage.h"
//#include "./inc/Debug.h"

#include "./inc/util.h"
#include "./inc/pkcs5_padding.h"
#include "./inc/AK_Mutex.h"
#include "./inc/AK_Log.h"

extern void* gpMutex;
unsigned char gDeviceType = 0;
//int gRefCount = 0;		//引用计数，记录设备open 次数
//void* gpRefCountMutex = NULL;


///////////////////////////////////////////////////////////////////////////////
//	底层设备相关API
///////////////////////////////////////////////////////////////////////////////
//	设备插拔事件函数	USBKEY_WaitForDevEvent
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_WaitForDevEvent(
        ULONG ulTimeOut,		//[IN]  超时时间，单位微秒,0：不阻塞，USBKEY_INFINITE：阻塞
        LPSTR szDevName, 		//[OUT] 发生事件的设备名称
        ULONG *pulDevNameLen, 	//[IN,OUT] IN：缓冲区长度，OUT：设备名称的有效长度，长度包含字符串结束符
        ULONG *pulEvent)		//[OUT] 事件类型。1表示插入，2表示拔出
{
    char ppchDevNameList[DEVICE_MAX_QUANTITY][MAX_PATH];
    char chDevNameChanged[1024];
    ULONG ulDevNameChangedLen = 0;

    memset(chDevNameChanged, 0, sizeof(chDevNameChanged));
    if( NULL == szDevName || NULL == pulDevNameLen || NULL == pulEvent )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_WaitForDevEvent ==> param Error,rtn = %#08x", USBKEY_PARAM_ERROR);
        return  USBKEY_PARAM_ERROR;
    }

    //枚举设备，以获得目前系统存在的设备名称
    ULONG rtn = USBKEY_DeviceEnum(ppchDevNameList, &g_DevNum);
    if(USBKEY_OK != rtn)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_WaitForDevEvent ==> USBKEY_DeviceEnum error ,rtn = %#08x", rtn);
        return rtn;
    }

    //创建钩子，以捕获系统设备热插拔消息
    g_hHook = SetWindowsHookEx(WH_CALLWNDPROC, (HOOKPROC)CallUsbProc,  g_hModule, 0);// WH_KEYBOARD
    if(!g_hHook)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_WaitForDevEvent ==> Create hook failed ,rtn = %#08x", USBKEY_INVALID_HANDLE_ERROR);
        return USBKEY_INVALID_HANDLE_ERROR;
    }

    //等待设备插拔事件
    DWORD rv;
    rv = WaitForSingleObject(g_hDevEvent, ulTimeOut);//WAIT_TIME);//INFINITE
    if(WAIT_TIMEOUT == rv)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_WaitForDevEvent ==> timeout.....,rtn = %#08x", USBKEY_TIME_OUT_ERROR);
        return USBKEY_TIME_OUT_ERROR;
    }
    else if(WAIT_OBJECT_0 == rv)
    {
        AK_Log(AK_LOG_DEBUG, "The state of the specified object is signaled");
    }
    else if(WAIT_ABANDONED == rv)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_WaitForDevEvent ==> WAIT_ABANDONED,rtn = %#08x", USBKEY_GENERAL_ERROR);
        return USBKEY_GENERAL_ERROR;
    }

    if(g_hHook)
    {
        UnhookWindowsHookEx(g_hHook);	//释放钩子
        g_hHook = NULL;
    }

    //处理设备插拔事件
    DealDeviceIn(ppchDevNameList, chDevNameChanged, &ulDevNameChangedLen, pulEvent);
    DealDeviceOut(ppchDevNameList, chDevNameChanged, &ulDevNameChangedLen, pulEvent);

    g_DevNum = 0;

    if(!ResetEvent(g_hDevEvent))
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_WaitForDevEvent ==> ResetEvent failed,rtn = %#08x", USBKEY_GENERAL_ERROR);
        return USBKEY_GENERAL_ERROR;
    }

    //设置输出
    if(*pulDevNameLen < ulDevNameChangedLen)
    { 
        *pulDevNameLen = ulDevNameChangedLen;
        AK_Log(AK_LOG_ERROR, "USBKEY_WaitForDevEvent ==> szDevName buffer is too short!,rtn = %#08x", USBKEY_BUFFER_TOO_SHORT);
        return USBKEY_BUFFER_TOO_SHORT;
    }
    memcpy(szDevName, chDevNameChanged, ulDevNameChangedLen);
    *pulDevNameLen = ulDevNameChangedLen;

    return USBKEY_OK;
}

//	取消等待设备插拔事件	USBKEY_CancelWaitForDevEvent	
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_CancelWaitForDevEvent()
{
    if(g_hDevEvent)
    {
        ResetEvent(g_hDevEvent);
    }
    if(g_hHook)
    {
        if(!UnhookWindowsHookEx(g_hHook))//释放钩子
        {
            AK_Log(AK_LOG_ERROR, "USBKEY_CancelWaitForDevEvent ==> Release hook failed,rtn = %#08x", USBKEY_GENERAL_ERROR);
            return USBKEY_GENERAL_ERROR;
        }
        g_hHook = NULL;
    }

    return USBKEY_OK;
}

/**
 *	@brief
 *		枚举计算机上所连接的设备的ID
 *		如果ppcDeviceIdS为NULL，则从pdwDeviceQuantity返回需要分配char *[DEVICE_ID_MAX_LEN]数量
 *		如果ppcDeviceIdS非空，则从ppcDeviceIdS返回连接设备的ID，
 *			并从pdwDeviceQuantity返回实际设备个数
 *	@返回值
 *		0：	OK
 *		>0:	FAIL，返回的是错误码
 *	@param
 *		ppcDeviceIdS		[out]设备ID的列表
 *		piDeviceQuantity	[out]设备ID的个数
 */
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DeviceEnum(
        USBKEY_CHAR	ppcDeviceIdS[DEVICE_MAX_QUANTITY][MAX_PATH],			
        USBKEY_INT32_PTR piDeviceQuantity)	
{
    USBKEY_ULONG rtn = 0;

    AK_LockMutex(gpMutex);
    AK_Log(AK_LOG_DEBUG, "USBKEY_DeviceEnum");

    rtn = DeviceEnum(ppcDeviceIdS, piDeviceQuantity);
    if (0 != rtn)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_DeviceEnum ==> DeviceEnum Error, rtn = %#08x", rtn);
        AK_UnlockMutex(gpMutex);
        return -1;
    }
    else if (*piDeviceQuantity < 0)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_DeviceEnum ==> DeviceEnum Error, \
						DeviceQuantity = %#08x", *piDeviceQuantity);
        AK_UnlockMutex(gpMutex);
        return -1;
    }

    AK_Log(AK_LOG_DEBUG, "USBKEY_DeviceEnum Success!");
    AK_UnlockMutex(gpMutex);
    return rtn;
}


/**
 *	@brief
 *		打开某个设备
 *		如果ucShareMode=1 表示共享模式 其它应用程序可以访问设备
 *	@返回值
 *		0：	OK
 *		>0:	FAIL，返回的是错误码
 *	@param
 *		pcDeviceId		[in]设备的ID
 *		iShareMode		[in]共享模式
 *		*phDeviceHandle	[out]设备句柄
 */
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DeviceOpen(
        USBKEY_CHAR_PTR pcDeviceId,		
        USBKEY_INT32 iShareMode,		
        USBKEY_HANDLE *phDeviceHandle)	
{
    USBKEY_ULONG rtn = 0;

    AK_LockMutex(gpMutex);
    AK_Log(AK_LOG_DEBUG, "USBKEY_DeviceOpen");

    rtn = DeviceOpen(
			(char *)pcDeviceId, 
			iShareMode, 
			phDeviceHandle, 
			&gDeviceType); // gDeviceType is the type of device

    if (0 != rtn)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_DeviceOpen ==> DeviceOpen Error, rtn = %#08x", rtn);
        AK_UnlockMutex(gpMutex);
        return rtn;
    }

    AK_Log(AK_LOG_DEBUG, "USBKEY_DeviceOpen Success!");
    AK_UnlockMutex(gpMutex);
    return rtn;
}

//	关闭某个设备
//	返回值：
//		0：	OK
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DeviceClose(
        USBKEY_HANDLE	hDeviceHandle )	//[in]设备句柄
{
    USBKEY_ULONG rtn = 0;

    AK_Log(AK_LOG_DEBUG, "USBKEY_DeviceClose");

    AK_LockMutex(gpMutex);
    rtn = DeviceClose(hDeviceHandle, gDeviceType);
    if( 0 != rtn )
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_DeviceClose ==> DeviceClose Error, rtn = %#08x", rtn);
        return -1;
    }

    AK_Log(AK_LOG_DEBUG, "USBKEY_DeviceClose Success!");
    AK_UnlockMutex(gpMutex);
    return rtn;
}

/**	
 *	@brief 
 *		重置当前连接的某个设备，使设备进入初始状态（设备句柄第一次刚被打开）
 *	@返回值
 *		0：	OK
 *		>0:	FAIL，返回的是错误码
 *	@param
 *		hDeviceHandle 	[in]设备句柄
 */
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DeviceReset(USBKEY_HANDLE hDeviceHandle)	
{
    USBKEY_UCHAR response[300];
    USBKEY_ULONG rtn, respLen=0;
    USBKEY_UCHAR data[24];
    USBKEY_UCHAR p1, p2, lc, le;

    AK_LockMutex(gpMutex);
    AK_Log(AK_LOG_DEBUG, "USBKEY_DeviceReset");

    p1 = 0x00;
    p2 = 0x00;
    lc = 0x00;
    le = 0x00;

    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_ResetDevice,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_DeviceReset ==> PackageData() Error, rtn = %#08x", rtn);
        return rtn;
    }

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_DeviceReset ==> PackageData() Error, code = %#08x", rtn);
            return rtn;
            break;
    }

    AK_Log(AK_LOG_DEBUG, "USBKEY_DeviceReset Success!");
    AK_UnlockMutex(gpMutex);
    return USBKEY_OK;
}


/**
 * @brief 初始化当前连接的某个设备.
 * @return 
 *		0：	正常.
 *		>0:	FAIL，返回的是错误码
 * @param
 *		hDeviceHandle	[in]设备句柄
 *		ulChipType		[in]芯片类型：CHIP_TYPE_20或CHIP_TYPE_45
 *		appVer			应用版本
 *		pcManufacturer	厂商名称：最长64个字符，
 *						不足64个字符以空白字符(ASCII码为0x20)填充，不能以null（0x00）结束。
 *		pcLabel			设备标签：最长64个字符，
 *						不足64个字符以空白字符(ASCII码为0x20)填充，不能以null（0x00）结束。
 */
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DeviceInit(
        USBKEY_HANDLE	hDeviceHandle,
        USBKEY_ULONG	ulChipType,	
        USBKEY_VERSION  appVer,	
        USBKEY_UCHAR_PTR	pcManufacturer,
        USBKEY_UCHAR_PTR	pcLabel)		
{
#ifdef _USBKEY_INIT
    USBKEY_UCHAR pcDirName[3];
    USBKEY_UCHAR pcFileName[2];
    USBKEY_UCHAR response[300];
    USBKEY_UCHAR p1,p2,lc,le;
    USBKEY_UCHAR_PTR data;
    USBKEY_UCHAR ulAccessCondition = USBKEY_ALL_PRIV;
    USBKEY_ULONG rtn,respLen = 300,len;
    USBKEY_ULONG ulDevPinLen;
    USBKEY_INT32 pucDeviceQuantity = 0;

    AK_LockMutex(gpMutex);
    AK_Log(AK_LOG_DEBUG, "USBKEY_DeviceInit");

    switch(ulChipType)
    {
        case CHIP_TYPE_20:
            break;
        case CHIP_TYPE_45:
            rtn = Erase45Key(hDeviceHandle);
            break;
        case CHIP_TYPE_AC3:
            rtn = EraseAC3Key(hDeviceHandle);
            break;
        case CHIP_TYPE_AC4:
            rtn = EraseAC4Key(hDeviceHandle);
            break;
        default:
            return USBKEY_PARAM_ERROR;
            break;
    }
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_DeviceInit ==> EraseUsbKey() Error, rtn = %#08x", rtn);
        AK_UnlockMutex(gpMutex);
        return rtn;
    }

    AK_Log(AK_LOG_DEBUG, "USBKEY_DeviceInit erase Success!");

    /*************************设置设备信息*******************************
	 *	the version			1.0
	 *	the manufacturer	aero-info
	 *	the label			aisino usbkey
	 ******************************************************************/
    p1 = 0x00;
    p2 = 0x00;
    lc = 0x82;
    le = 0;

    data = (USBKEY_UCHAR_PTR)malloc(lc);
    data[0] = appVer.major;
    data[1] = appVer.minor;
    memcpy(data+2, pcManufacturer, 64);
    memcpy(data+66, pcLabel, 64);

    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_SetDevInfo,response,&respLen);
    if (rtn != 0)
    {
        free(data);
        AK_Log(AK_LOG_ERROR, "USBKEY_DeviceInit ==> PackageData() Set devinfo Error, rtn = %#08x", rtn);
        AK_UnlockMutex(gpMutex);
        return rtn;
    }

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            free(data);
            AK_Log(AK_LOG_ERROR, "USBKEY_DeviceInit ==> PackageData() Set devinfo Error, code = %#08x", rtn);
            AK_UnlockMutex(gpMutex);
            return rtn;
            break;
    }

    free(data);

    AK_Log(AK_LOG_DEBUG, "USBKEY_DeviceInit Set devinfo Success!");

    /*******************************创建MF*******************************
	 *	文件标识				3f 00 
	 *	目录基本文件短标识		01
	 *	建立文件权限			f0
	 *	删除文件权限			99
	 *	文件名				3f 00 01 01 01
	 *******************************************************************/
    memset(pcDirName,0,3);
    pcDirName[0] = 0x3f;
    pcDirName[1] = 0x00;

    p1 = 0x00;
    p2 = 0x00;
    len = 3+2+5;
    memcpy(&lc,&len,1);
    le = 0x00;
    data = (USBKEY_UCHAR_PTR)malloc(lc);
    ulAccessCondition = USBKEY_DEVICE_PRIV;	// ADD by zhoushenshen 1117

    //生成文件标识
    memcpy(data,pcDirName,2);
    memset(data+2,0x01,1);
    memset(data+3,USBKEY_ALL_PRIV,1);		//update in 20120108 因权限即时生效暂时改为建立文件无权限要求

    memcpy(data+4,&ulAccessCondition,1);
    memcpy(data+5,pcDirName,2);
    memset(data+7,0x01,3);

    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_CreateFile,response,&respLen);
    if (rtn != 0)
    {
        free(data);
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_DeviceInit ==> PackageData() Create MF Error, rtn = %#08x", rtn);
        return rtn;
    }

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            free(data);
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_DeviceInit ==> PackageData() Create MF Error, code = %#08x", rtn);
            return rtn;
            break;
    }

    free(data);
    AK_Log(AK_LOG_DEBUG, "USBKEY_DeviceInit Create MF Success!");

    /******************创建系统密钥文件，存放外部认证密钥*****************
	 *	文件标识		3f 01
	 *	文件写入模式	00
	 *	读权限		00
	 *	写权限		f0
	 *	记录长度		00
	 *	记录数		03
	 ****************************************************************/
    p1 = 0x00;
    p2 = 0x06;
    lc = 7;
    le = 0x00;
    data = (USBKEY_UCHAR_PTR)malloc(lc);

    pcFileName[0] = 0x3f;
    pcFileName[1] = 0x01;
    //ulAccessCondition = USBKEY_NOT_USE;	//ADD by zhoushenshen 20091116  系统密钥文件不可写
    ulAccessCondition = USBKEY_ALL_PRIV;	//update in 20120108 因权限即时生效暂时改为建立文件无权限要求

    memset(data,0,7);
    memcpy(data,pcFileName,2);
    le = 0x00;
    memcpy(data+3,&le,1);
    memcpy(data+4,&ulAccessCondition,1);
    memcpy(data+5,&le,1);
    le = 3;
    memcpy(data+6,&le,1);

    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_CreateFile,response,&respLen);
    if (rtn != 0)
    {
        free(data);
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_DeviceInit ==> PackageData() Create System Key File Error, rtn = %#08x", rtn);
        return rtn;
    }

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            free(data);
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_DeviceInit ==> PackageData() Create System Key File Error, code = %#08x", rtn);
            return rtn;
            break;
    }

    free(data);
    AK_Log(AK_LOG_DEBUG, "USBKEY_DeviceInit Create System Key File Success!");


    /***************************写外部认证密钥初始值*************************
	 *	cla:80
	 *	密钥标识	01
	 *	密钥类型	00
	 *	算法标识	ALG_TYPE_SCB2
	 *	版本		01
	 *	获得状态	99
	 *	最大允许尝试次数	5
	 *	使用权限	f0
	 *	更改权限	99
	 *	密钥值	01 * 16 
	 *********************************************************************/
    p1 = 0x00;
    p2 = 0x00;
    len = 8+16;
    memcpy(&lc,&len,1);	
    data = (USBKEY_UCHAR_PTR)malloc(lc);
    memset(data,0,lc);

    le = 0x01;
    memcpy(data,&le,1);
    le = 0x00;						//命令修改 00代表外部认证密钥
    memcpy(data+1,&le,1);
    le = ALG_TYPE_SCB2;
    memcpy(data+2,&le,1);
    le = 0x01;
    memcpy(data+3,&le,1);
    ulAccessCondition = USBKEY_DEVICE_PRIV;
    memcpy(data+4,&ulAccessCondition,1);
    le = KEY_RETRY_MAX_TIMES;
    memcpy(data+5,&le,1);
    le = USBKEY_ALL_PRIV;			//使用权限无限制
    memcpy(data+6,&le,1);
    memcpy(data+7,&ulAccessCondition,1);
    le = 0x00;

    //设置初始密钥值
    memset(data+8,0x01,16);

    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_WriteKey,response,&respLen);
    if (rtn != 0)
    {
        free(data);
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_DeviceInit ==> PackageData() Write Init ExAuthenticate Key Error, rtn = %#08x", rtn);
        return rtn;
    }

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_DeviceInit ==> DeviceExecCmd() Write Init ExAuthenticate Key Error, code = %#08x", rtn);
            return rtn;
            break;
    }

    AK_Log(AK_LOG_DEBUG, "USBKEY_DeviceInit Create Init ExAuthenticate Key Success!");

    /***************************写设备管理员PIN*************************
	 *	cla:80
	 *	密钥标识	01
	 *	密钥类型	02
	 *	获得状态	99
	 *	最大允许尝试次数	3
	 *	使用权限	f0
	 *	更改权限	99
	 *	密钥值	123456
	 *****************************************************************/
    p1 = 0x00;
    p2 = 0x00;

    //生成数据
    ulDevPinLen = strlen(USBKEY_DEVICE_ADMIN_PIN);
    lc = 6+ulDevPinLen;
    data = (USBKEY_UCHAR_PTR)malloc(lc);

    le = USBKEY_DEVICE_PIN_FLAG;
    memcpy(data,&le,1);
    le = 0x02;	
    memcpy(data+1,&le,1);
    le = USBKEY_DEVICE_PRIV;
    memcpy(data+2,&le,1);
    le = DEVICE_ADMIN_PIN_RETRY_TIMES;
    memcpy(data+3,&le,1);
    le = USBKEY_ALL_PRIV;
    memcpy(data+4,&le,1);
    le = USBKEY_DEVICE_PRIV;
    memcpy(data+5,&le,1);
    le = 0x00;

    memcpy(data+6,USBKEY_DEVICE_ADMIN_PIN,ulDevPinLen);

    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_WriteKey,response,&respLen);
    if (rtn != 0)
    {
        free(data);
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_DeviceInit ==> PackageData() Create APP Admin PIN Error, rtn = %#08x", rtn);
        return rtn;
    }

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_DeviceInit ==> DeviceExecCmd() Create APP Admin PIN Error, code = %#08x", rtn);
            return rtn;
            break;
    }

    AK_Log(AK_LOG_DEBUG, "USBKEY_DeviceInit Create APP Admin PIN Success!");

    /***********************创建记录文件，记录应用(DF)名称*****************
	 *	创建个定长记录文件包含3个记录
	 *	文件标识		80 00
	 *	文件写入模式	00
	 *	读权限		f0
	 *	写权限		f0
	 *	记录长度		10
	 *	记录数		03
	 *	dirname		"/"	
	 *
	 *	update record
	 *		cla:00
	 *		p1:1/2/3
	 *		p2:04
	 *		ulRecordLen:10
	 *		ff * 16
	 ******************************************************************/
    rtn = CreateDirRecord(hDeviceHandle, (USBKEY_UCHAR_PTR)"/");
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_DeviceInit ==> CreateDirRecord() Error, rtn = %#08x", rtn);
        return rtn;
    }
    AK_Log(AK_LOG_DEBUG, "USBKEY_DeviceInit Create & Update Dir Record Success!");

    /******************************激活安全机制****************************
	 *	ACTIVATE 命令用于激活安全机制。
	 *	成功执行此命令后，所有文件和命令的安全访问机制开始生效。
	 *	在卡片未激活前，安全访问机制不起作用。
	 *	此命令只能执行一次。这种状态的转变是不可逆的。
	 *	只有当前环境为MF时，才可以执行此命令。
	 ********************************************************************/
    p1 = 0x00;
    p2 = 0x00;
    lc = 0x00;
    le = 0x00;

    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_ActivateSM,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_DeviceInit ==> PackageData() Activate Security Machanism Error, rtn = %#08x", rtn);
        return rtn;
    }

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_DeviceInit ==> DeviceExecCmd() Activate Security Machanism Error, code = %#08x", rtn);
            return rtn;
            break;
    }

    AK_Log(AK_LOG_DEBUG, "USBKEY_DeviceInit Activate Security Machanism Success!");

    //////////////////////////////////////////////////////////////////////////

    AK_Log(AK_LOG_DEBUG, "USBKEY_DeviceInit Success!");
    AK_UnlockMutex(gpMutex);
    return USBKEY_OK;
#else

    AK_Log(AK_LOG_ERROR, "USBKEY_DeviceInit ==> the fun not support,rtn = %#08x", USBKEY_FUNCTION_NOT_SUPPORT);
    return USBKEY_FUNCTION_NOT_SUPPORT;
#endif
}


//	设置设备标签，标签大小小于32字节
//	返回值：
//		0：	OK
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DeviceSetLabel(
        USBKEY_HANDLE	hDeviceHandle,		//[in]设备句柄
        USBKEY_UCHAR_PTR pLabel)			//[in]设备标签字符串，此字符串应小于32字节
{
    USBKEY_ULONG rtn = 0;
    //USBKEY_UCHAR request[256];
    USBKEY_UCHAR response[16];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG respLen=0;
    USBKEY_UCHAR data[256];		//Add by zhoushenshen in 20121108
    USBKEY_UCHAR p1,p2,lc,le;	//Add by zhoushenshen in 20121108
    USBKEY_UCHAR msg[256];

    AK_Log(AK_LOG_DEBUG, "USBKEY_DeviceSetLabel");
    AK_LockMutex(gpMutex);

    memset(msg,0x20,sizeof(msg));
    rtn = GetDeviceInfo(hDeviceHandle,msg,DEVICEINFO_CUR_PRIV);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_DeviceSetLabel ==> GetDeviceInfo() Error, rtn = %#08x", rtn);
        return rtn;
    }

    //Delete by zhoushenshen in 20121108
    /*//设置命令头: CLA INS P1 P2 LC
      request[0] = 0xb8;	//CLA
      request[1] = 0x04;	//INS
      request[2] = 0x00;	//P1
      request[3] = 0x00;	//P2
      request[4] = 0x82;	//LC

    //设置应用版本号
    request[5] = APP_VER_MAJOR;
    request[6] = APP_VER_MINOR;

    //设置设备厂商信息
    memcpy(request+7,msg,64);

    //设置设备标签
    strcpy((char *)request+71,(char *)pLabel);

    //发送命令
    reqLen = 64+64+7;
    respLen = 16;
    rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
    if (rtn != 0)
    {
    AK_UnlockMutex(gpMutex);
    AK_Log(AK_LOG_ERROR, "USBKEY_DeviceSetLabel ==> DeviceExecCmd() Error, rtn = %#08x", rtn);
    return rtn;
    }*/
    //Del end

    //Add by zhoushenshen in 20121108
    p1 = 0x00;
    p2 = 0x00;
    lc = 0x82;
    le = 0x00;

    //设置应用版本号
    data[0] = APP_VER_MAJOR;
    data[1] = APP_VER_MINOR;

    //设置设备厂商信息
    memcpy(data+2,msg,64);

    //设置设备标签
    strcpy((char *)data+66,(char *)pLabel);

    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_SetDevInfo,response,&respLen);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>USBKEY_DeviceSetLabel->PackageData() Error, rtn = %#08x", rtn);
        return rtn;
    }
    //Add end

    //判断命令是否执行成功
    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_DeviceSetLabel ==> DeviceExecCmd() Error, code = %#08x", rtn);
            return rtn;
            break;
    }

    AK_Log(AK_LOG_DEBUG, "USBKEY_DeviceSetLabel Success!");
    AK_UnlockMutex(gpMutex);
    return USBKEY_OK;
}

//	获取当前连接的某个设备的状态.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DeviceGetStat(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_DEVINFO* pcDeviceInfo)   //[Out]返回的信息
{
    USBKEY_ULONG rtn = 0;
    USBKEY_UCHAR response[256];
    USBKEY_ULONG respLen=0;
    USBKEY_UCHAR data[24];
    USBKEY_UCHAR p1,p2,lc,le;
    USBKEY_ULONG len;

    AK_LockMutex(gpMutex);
    AK_Log(AK_LOG_DEBUG, "USBKEY_DeviceGetStat");

    p1 = 0x00;
    p2 = 0x00;
    lc = 0x00;
    le = 0xb2;

    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_GetDeviceInfo,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_DeviceGetStat ==> PackageData() Error, rtn = %#08x", rtn);
        return rtn;
    }

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_DeviceGetStat ==> DeviceExecCmd() Error, code = %#08x", rtn);
            return rtn;
            break;
    }

    memset(pcDeviceInfo,' ',sizeof(USBKEY_DEVINFO));

    pcDeviceInfo->MinPINLen = 2;
    pcDeviceInfo->MaxPINLen = 8;
    pcDeviceInfo->AlgSymCap = USBKEY_TYPE_SCB2|USBKEY_TYPE_SSF33;
    pcDeviceInfo->AlgAsymCap = USBKEY_TYPE_RSA;
    pcDeviceInfo->DevAuthAlgId = USBKEY_TYPE_SCB2;

    memcpy(pcDeviceInfo->Manufacturer, response, 64);
    memcpy(pcDeviceInfo->Issuer, response, 64);
    memcpy(pcDeviceInfo->Label, response+64, 64);
    memcpy(pcDeviceInfo->SerialNumber, response+128, 32);
    pcDeviceInfo->HWVersion.major = response[160];
    pcDeviceInfo->HWVersion.minor = response[161];
    pcDeviceInfo->FirmwareVersion.major = response[162];
    pcDeviceInfo->FirmwareVersion.minor = response[163];
    pcDeviceInfo->AppVersion.major = response[174];
    pcDeviceInfo->AppVersion.minor = response[175];

    len = response[164];
    len = len*256 + response[165];
    len = len*256 + response[166];
    len = len*256 + response[167];
    pcDeviceInfo->TotalMemory = len;

    len = response[168];
    len = len*256 + response[169];
    len = len*256 + response[170];
    len = len*256 + response[171];
    pcDeviceInfo->FreeMemory = len;

    pcDeviceInfo->Reserved = 0;
    if( (response[172]&0x08) != 0 )	
        pcDeviceInfo->Reserved = 0x01;
    if( (response[172]&0x40) != 0 )	
        pcDeviceInfo->Reserved = 0x10;

    AK_Log(AK_LOG_DEBUG, "USBKEY_DeviceGetStat Success!");
    AK_UnlockMutex(gpMutex);
    return USBKEY_OK;
}


//	传输设备命令
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_TransmitDeviceCmd(
        USBKEY_HANDLE		hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR	pbCommand,		//[in]设备命令
        USBKEY_ULONG		ulCommandLen,	//[in]命令长度	
        USBKEY_UCHAR_PTR	pbResponse,		//[out]响应数据
        USBKEY_ULONG		*pulResponseLen)	//[in out]in: 响应buffer的长度，out: 响应长度
{
    AK_Log(AK_LOG_DEBUG, "USBKEY_TransmitDeviceCmd");

    return DeviceExecCmd(hDeviceHandle,pbCommand,ulCommandLen,pbResponse,(int *)pulResponseLen);
}

/**	
 *	@验证设备PIN.
 *	@返回值：
 *		0：	正常.
 *		>0:	FAIL，返回的是错误码
 *	@param
 *	    USBKEY_HANDLE hDeviceHandle		[in]设备句柄
 *      USBKEY_UCHAR_PTR pcPIN			[in]送入的PIN值
 *      USBKEY_ULONG ulLen				[in]送入的PIN长度
 *      USBKEY_ULONG_PTR pulRetryNum	[out]出错后重试次数
 */
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_CheckDeviceAdminPin(
        USBKEY_HANDLE hDeviceHandle,	
        USBKEY_UCHAR_PTR pcPIN,	
        USBKEY_ULONG ulLen,	
        USBKEY_ULONG_PTR pulRetryNum)	
{
    USBKEY_UCHAR response[300];
    USBKEY_ULONG rtn,respLen=0;
    USBKEY_UCHAR data[32];
    USBKEY_UCHAR p1,p2,lc,le;

    AK_Log(AK_LOG_DEBUG, "USBKEY_CheckDeviceAdminPin");

    if( (ulLen > USBKEY_PIN_MAX_LEN) || (ulLen < USBKEY_PIN_MIN_LEN) )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_CheckDeviceAdminPin ==> PIN Length Error, ulLen = %#08x", ulLen);
        return USBKEY_PARAM_ERROR;
    }

    AK_LockMutex(gpMutex);

    rtn = SelectFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"/",NULL);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_CheckDeviceAdminPin ==> SelectFile() Error, rtn = %#08x", rtn);
        return rtn;
    }

    p1 = 0x00;
    p2 = USBKEY_DEVICE_PIN_FLAG;
    lc = (USBKEY_UCHAR)ulLen;
    le = 0x00;

    memset(data,0,sizeof(data));
    memcpy(data,pcPIN,ulLen);

    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_VerifyPIN,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_CheckDeviceAdminPin ==> PackageData() Error, rtn = %#08x", rtn);
        return rtn;
    }

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            if ( (rtn & 0xfff0) == DEVICE_REMAIN_NT )
            {
                *pulRetryNum = rtn & 0x000f;
            }
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_CheckDeviceAdminPin ==> DeviceExecCmd() Error, code = %#08x", rtn);
            return rtn;
            break;
    }

    AK_UnlockMutex(gpMutex);
    AK_Log(AK_LOG_DEBUG, "USBKEY_CheckDeviceAdminPin Success!");
    return USBKEY_OK;
}

/**
 *	@修改设备PIN.
 *	@返回值：
 *		0：	正常.
 *		>0:	FAIL，返回的是错误码
 *	@param:
 *      USBKEY_HANDLE hDeviceHandle		[in]设备句柄
 *      USBKEY_UCHAR_PTR pcOldPIN		[in]送入的原PIN
 *      USBKEY_ULONG ulOldLen			[in]送入的原PIN长度
 *      USBKEY_UCHAR_PTR pcNewPIN		[in]送入的新PIN
 *      USBKEY_ULONG ulNewLen			[in]送入的新PIN长度
 *      USBKEY_ULONG_PTR pulRetryNum	[out]出错后重试次数
 */
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ChangeDeviceAdminPin(
        USBKEY_HANDLE hDeviceHandle,
        USBKEY_UCHAR_PTR pcOldPIN,
        USBKEY_ULONG ulOldLen,	
        USBKEY_UCHAR_PTR pcNewPIN,		
        USBKEY_ULONG ulNewLen,		
        USBKEY_ULONG_PTR pulRetryNum)	
{
    USBKEY_UCHAR response[300];
    USBKEY_ULONG rtn,respLen=0;
    USBKEY_UCHAR data[32];
    USBKEY_UCHAR p1,p2,lc,le;

    AK_Log(AK_LOG_DEBUG, "USBKEY_ChangeDeviceAdminPin");

    if( (ulOldLen > USBKEY_PIN_MAX_LEN) || (ulOldLen < USBKEY_PIN_MIN_LEN) )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_CheckDeviceAdminPin ==> Old PIN Length Error, ulOldLen = %#08x", ulOldLen);
        return USBKEY_PARAM_ERROR;
    }
    if( (ulNewLen > USBKEY_PIN_MAX_LEN) || (ulNewLen < USBKEY_PIN_MIN_LEN) )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_CheckDeviceAdminPin ==> New PIN Length Error, ulNewLen = %#08x", ulNewLen);
        return USBKEY_PARAM_ERROR;
    }

    AK_LockMutex(gpMutex);

    rtn = SelectFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"/",NULL);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ChangeDeviceAdminPin ==> SelectFile() Error, rtn = %#08x", rtn);
        return rtn;
    }

    p1 = 0x01;
    p2 = USBKEY_DEVICE_PIN_FLAG;
    lc = (USBKEY_UCHAR)(ulOldLen + ulNewLen + 1);
    le = 0x00;

    memcpy(data,pcOldPIN,ulOldLen);
    rtn = 0xFF;
    memcpy(data+ulOldLen,&rtn,1);
    memcpy(data+ulOldLen+1,pcNewPIN,ulNewLen);

    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_ChangePIN,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ChangeDeviceAdminPin ==> PackageData() Error, rtn = %#08x", rtn);
        return rtn;
    }

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            if ( (rtn & 0xfff0) == DEVICE_REMAIN_NT )
            {
                *pulRetryNum = rtn & 0x000f;
            }
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_ChangeDeviceAdminPin ==> DeviceExecCmd() Error, code = %#08x", rtn);
            return rtn;
            break;
    }

    AK_UnlockMutex(gpMutex);
    AK_Log(AK_LOG_DEBUG, "USBKEY_ChangeDeviceAdminPin Success!");
    return USBKEY_OK;
}


///////////////////////////////////////////////////////////////////////////////
//	访问控制相关API
///////////////////////////////////////////////////////////////////////////////
//	向USBKEY写入内部认证或者外部认证密钥.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_WriteKey(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcKeyHead,		//[in]密钥头部信息
        USBKEY_UCHAR_PTR pcKeyBody)		//[in]密钥值
{
    //USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    USBKEY_UCHAR data[300];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0,len;
    USBKEY_UCHAR p1,p2,lc,le;
    USBKEY_KEYHEAD *keyhead;
    USBKEY_UCHAR ulAccessCondition;

    AK_Log(AK_LOG_DEBUG, "USBKEY_WriteKey");

    keyhead = (USBKEY_KEYHEAD*)pcKeyHead;
    if ( (keyhead->ApplicationType != KEY_INTERNAL_AUTHENTICATE_KEY) && (keyhead->ApplicationType != KEY_EXTERNAL_AUTHENTICATE_KEY) )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_WriteKey ==> ApplicationType Error,ApplicationType = %#08x", keyhead->ApplicationType);
        return USBKEY_PARAM_ERROR;
    }

    AK_LockMutex(gpMutex);

    p1 = 0x00;
    p2 = 0x00;

    //ADD by zhoushenshen 20091117
    switch(keyhead->ulPrivil)
    {
        case USBKEY_DEVICE_PRIVILEGE:
            ulAccessCondition = USBKEY_DEVICE_PRIV;
            break;
        case USBKEY_USERTYPE_USER:
            ulAccessCondition = USBKEY_USER_PRIV;
            break;		
        case USBKEY_USERTYPE_ADMIN:
            ulAccessCondition = USBKEY_ADMIN_PRIV;
            break;
        case USBKEY_ALL_PRIVILEGE:
            ulAccessCondition = USBKEY_ALL_PRIV;
            break;
        case USBKEY_NEVER_USE:
            ulAccessCondition = USBKEY_NOT_USE;
            break;

        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_WriteKey ==> pcKeyHead right type ulPrivil = %#08x", keyhead->ulPrivil);
            return USBKEY_PARAM_ERROR;
    }
    ////////
    //生成数据
    ////////
    memset(data,0,300);
    if (keyhead->ApplicationType == KEY_INTERNAL_AUTHENTICATE_KEY)
    {
        len = 6 + keyhead->KeyLen;
        memcpy(&lc,&len,1);

        //add by zhoushenshen 20091116
        le = 0x01;
        memcpy(data,&le,1);		
        //le = 0x02;
        le = 0x01;	//命令修改 01代表内部认证密钥
        memcpy(data+1,&le,1);
        memcpy(data+2,&keyhead->ulAlg,1);	
        le = 0x01;
        memcpy(data+3,&le,1);
        le = USBKEY_ALL_PRIV;
        memcpy(data+4,&le,1);	
        memcpy(data+5,&ulAccessCondition,1);
        memcpy(data+6,pcKeyBody,keyhead->KeyLen );
    }
    else if (keyhead->ApplicationType == KEY_EXTERNAL_AUTHENTICATE_KEY)
    {
        len = 8 + keyhead->KeyLen;
        memcpy(&lc,&len,1);

        le = 0x01;
        memcpy(data,&le,1);
        //le = 0x01;
        le = 0x00;	//命令修改 00代表外部认证密钥
        memcpy(data+1,&le,1);	
        memcpy(data+2,&keyhead->ulAlg,1);
        le = 0x01;
        memcpy(data+3,&le,1);
        memcpy(data+4,&ulAccessCondition,1);
        memcpy(data+5,&keyhead->ErrorCounter,1);
        le = USBKEY_ALL_PRIV;
        memcpy(data+6,&le,1);
        memcpy(data+7,&ulAccessCondition,1);
        memcpy(data+8,pcKeyBody,keyhead->KeyLen);
    }
    le = 0x00;

    //Delete by zhoushenshen in 20121108
    /*rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_WriteKey,request,&reqLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_WriteKey ==> PackageData() Error,rtn = %#08x", rtn);
      return rtn;
      }

      respLen = 300;
      rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_WriteKey ==> DeviceExecCmd() Error,rtn = %#08x", rtn);
      return rtn;
      }*/
    //Del end

    //Add by zhoushenshen in 20121108
    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_WriteKey,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_WriteKey ==> PackageData() Error, rtn = %#08x", rtn);
        return rtn;
    }
    //Add end
    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_WriteKey ==> DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
            break;
    }

    AK_Log(AK_LOG_DEBUG, "USBKEY_WriteKey Success!");
    AK_UnlockMutex(gpMutex);

    return USBKEY_OK;
}

//	内部认证，终端对设备的认证.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_IntAuthenticate(
        USBKEY_HANDLE hDeviceHandle,		//[in]设备句柄
        USBKEY_UCHAR_PTR pcRandomNumber,	//[in]送入的随机数
        USBKEY_ULONG ulRandomNumberLen,		//[in]送入的随机数长度
        USBKEY_UCHAR_PTR pcEncryptNumber,	//[out]加密后的随机数
        USBKEY_ULONG_PTR pulEncryptNumberLen)//[out]返回的随机数长度
{
    //USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0;
    USBKEY_UCHAR data[24];
    USBKEY_UCHAR p1,p2,lc,le;

    AK_Log(AK_LOG_DEBUG, "USBKEY_IntAuthenticate");

    p1 = 0x00;
    p2 = 0x01;
    if( (ulRandomNumberLen == 16) || (ulRandomNumberLen == 8) )
    {
        lc = (USBKEY_UCHAR)ulRandomNumberLen;
    }
    else
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_IntAuthenticate ==> RandomNumberLen Error,len = %#08x", ulRandomNumberLen);
        return USBKEY_PARAM_ERROR;
    }
    le = 0x00;

    AK_LockMutex(gpMutex);
    memset(data,0,24);
    memcpy(data,pcRandomNumber,ulRandomNumberLen);

    //Delete by zhoushenshen in 20121108
    /*rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_InternalAuth,request,&reqLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_IntAuthenticate ==> PackageData() Error,rtn = %#08x", rtn);
      return rtn;
      }

      respLen = 300;
      rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_IntAuthenticate ==> DeviceExecCmd() Error,rtn = %#08x", rtn);
      return rtn;
      }*/
    //Del end

    //Add by zhoushenshen in 20121108
    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_InternalAuth,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_IntAuthenticate ==> PackageData() Error, rtn = %#08x", rtn);
        return rtn;
    }
    //Add end

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_IntAuthenticate ==> DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
            break;
    }

    //ADD by zhoushenshen 20091117
    if(*pulEncryptNumberLen < respLen - 2)
    {
        *pulEncryptNumberLen = respLen - 2;
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_IntAuthenticate ==> pulEncryptNumberLen not enough,rtn = %#08x", USBKEY_BUFFER_TOO_SHORT);
        return USBKEY_BUFFER_TOO_SHORT;
    }

    memcpy(pcEncryptNumber,response,respLen-2);
    *pulEncryptNumberLen = respLen - 2;

    AK_Log(AK_LOG_DEBUG, "USBKEY_IntAuthenticate Success!");
    AK_UnlockMutex(gpMutex);
    return USBKEY_OK;
}

//	外部认证，设备对终端的认证.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ExtAuthenticate(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcRandomNumber,//[in]送入的外部认证密钥加密后的随机数
        USBKEY_ULONG ulRandomNumberLen)	//[in]送入的随机数长度
{
    //USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0;
    USBKEY_UCHAR data[24];
    USBKEY_UCHAR p1,p2,lc,le;

    AK_Log(AK_LOG_DEBUG, "USBKEY_ExtAuthenticate");

    p1 = 0x00;
    p2 = 0x01;
    if( (ulRandomNumberLen == 16) || (ulRandomNumberLen == 8) )
    {
        lc = (USBKEY_UCHAR)ulRandomNumberLen;
    }
    else
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ExtAuthenticate ==> RandomNumberLen Error,len = %#08x", ulRandomNumberLen);
        return USBKEY_PARAM_ERROR;
    }
    le = 0x00;

    AK_LockMutex(gpMutex);
    memset(data,0,24);
    memcpy(data,pcRandomNumber,ulRandomNumberLen);

    //Delete by zhoushenshen in 20121108
    /*rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_ExternalAuth,request,&reqLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_ExtAuthenticate ==> PackageData() Error,rtn = %#08x", rtn);
      return rtn;
      }

      respLen = 300;
      rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_ExtAuthenticate ==> DeviceExecCmd() Error,rtn = %#08x", rtn);
      return rtn;
      }*/
    //Del end

    //Add by zhoushenshen in 20121108
    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_ExternalAuth,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ExtAuthenticate ==> PackageData() Error, rtn = %#08x", rtn);
        return rtn;
    }
    //Add end

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_ExtAuthenticate ==> DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
            break;
    }
    AK_Log(AK_LOG_DEBUG, "USBKEY_ExtAuthenticate Success!");

    AK_UnlockMutex(gpMutex);
    return USBKEY_OK;
}

//	初始化用户PIN.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_InitPin(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcPIN,			//[in]送入的PIN
        USBKEY_ULONG ulLen)				//[in]PIN长度
{
    USBKEY_UCHAR response[300];
    USBKEY_ULONG rtn,respLen=0;
    USBKEY_UCHAR_PTR data;
    USBKEY_UCHAR p1,p2,lc,le;

    AK_Log(AK_LOG_DEBUG, "USBKEY_InitPin");

    if( (ulLen > USBKEY_PIN_MAX_LEN) || (ulLen < USBKEY_PIN_MIN_LEN) )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_InitPin ==> PIN Length Error,ulLen = %#08x", ulLen);
        return USBKEY_PARAM_ERROR;
    }

    AK_LockMutex(gpMutex);

    p1 = 0x00;
    p2 = 0x00;

    ////////
    //生成数据
    ////////

    lc = (USBKEY_UCHAR)(6+ulLen);
    data = (USBKEY_UCHAR_PTR)malloc(lc);

    le = USBKEY_USER_PIN_FLAG;
    memcpy(data,&le,1);
    //le = 0x05;
    le = 0x02;	//命令修改 02代表PIN
    memcpy(data+1,&le,1);
    le = USBKEY_USER_PRIV;	//ADD by zhoushenshen 20091118
    memcpy(data+2,&le,1);
    le = KEY_RETRY_MAX_TIMES;
    memcpy(data+3,&le,1);
    le = USBKEY_ALL_PRIV;
    memcpy(data+4,&le,1);
    le = 0x63;
    memcpy(data+5,&le,1);

    le = 0x00;
    memcpy(data+6,pcPIN,ulLen);

    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_WriteKey,response,&respLen);
    if (rtn != 0)
    {
        free(data);
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_InitPin ==> PackageData() Error, rtn = %#08x", rtn);
        return rtn;
    }

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_InitPin ==> DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
            break;
    }

    AK_Log(AK_LOG_DEBUG, "USBKEY_InitPin Success!");
    AK_UnlockMutex(gpMutex);
    return USBKEY_OK;
}

//	修改PIN.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ChangePin(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_ULONG ulPINType,			//[in]PIN类型
        USBKEY_UCHAR_PTR pcOldPIN,		//[in]送入的原PIN
        USBKEY_ULONG ulOldLen,			//[in]送入的原PIN长度
        USBKEY_UCHAR_PTR pcNewPIN,		//[in]送入的新PIN
        USBKEY_ULONG ulNewLen,			//[in]送入的新PIN长度
        USBKEY_ULONG_PTR pulRetryNum)	//[out]出错后重试次数
{
    //USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0;
    USBKEY_UCHAR data[32];
    USBKEY_UCHAR p1,p2,lc,le;

    AK_Log(AK_LOG_DEBUG, "USBKEY_ChangePin");

    /*if (strlen(pcAppName) > 16)
      {
      LogMessage("====>>USBKEY_ChangePin->pcAppName Length Error!");
      return USBKEY_PARAM_ERROR;
      }*/
    if( (ulOldLen > USBKEY_PIN_MAX_LEN) || (ulOldLen < USBKEY_PIN_MIN_LEN) )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ChangePin ==> Old PIN Length Error,ulOldLen = %#08x", ulOldLen);
        return USBKEY_PARAM_ERROR;
    }
    if( (ulNewLen > USBKEY_PIN_MAX_LEN) || (ulNewLen < USBKEY_PIN_MIN_LEN) )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ChangePin ==> New PIN Length Error,ulNewLen = %#08x", ulNewLen);
        return USBKEY_PARAM_ERROR;
    }

    AK_LockMutex(gpMutex);
    /*rtn = SelectFile(hDeviceHandle,pcAppName,NULL);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage16Val("====>>USBKEY_ChangePin->SelectFile() Error,rtn = 0x",rtn);
      return rtn;
      }*/

    p1 = 0x01;
    lc = (USBKEY_UCHAR)(ulOldLen + ulNewLen + 1);
    le = 0x00;
    if (ulPINType == USBKEY_USERTYPE_ADMIN)
    {
        p2 = USBKEY_ADMIN_PIN_FLAG;
    }
    else if (ulPINType == USBKEY_USERTYPE_USER)
    {
        p2 = USBKEY_USER_PIN_FLAG;
    }
#ifdef _SEC_GTAX_APP
    //Add by zhoushenshen in 20101213 for Tax
    else if(ulPINType == USBKEY_USERTYPE_HIDISK_ADMIN)
    {
        p2 = USBKEY_HIDISK_ADMIN_PIN_FLAG;
    }
    else if(ulPINType == USBKEY_USERTYPE_HIDISK_USER)
    {
        p2 = USBKEY_HIDISK_USER_PIN_FLAG;
    }
    else if(ulPINType == USBKEY_USERTYPE_HIDISK_TAX)
    {
        p2 = USBKEY_HIDISK_TAX_PIN_FLAG;
    }
    //End Add ---20101213
#endif
    else
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ChangePin ==> ulPINType Error,ulPINType = %#08x", ulPINType);
        return USBKEY_PARAM_ERROR;
    }

    memcpy(data,pcOldPIN,ulOldLen);
    rtn = 0xFF;
    memcpy(data+ulOldLen,&rtn,1);
    memcpy(data+ulOldLen+1,pcNewPIN,ulNewLen);

    //Delete by zhoushenshen in 20121108
    /*rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_ChangePIN,request,&reqLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_ChangePin ==> PackageData() Error,rtn = %#08x", rtn);
      return rtn;
      }

      respLen = 300;
      rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_ChangePin ==> DeviceExecCmd() Error,rtn = %#08x", rtn);
      return rtn;
      }*/
    //Del end

    //Add by zhoushenshen in 20121108
    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_ChangePIN,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ChangePin ==> PackageData() Error, rtn = %#08x", rtn);
        return rtn;
    }
    //Add end

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            if ( (rtn & 0xfff0) == DEVICE_REMAIN_NT )
            {
                *pulRetryNum = rtn & 0x000f;
            }
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_ChangePin ==> DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
            break;
    }

    AK_Log(AK_LOG_DEBUG, "USBKEY_ChangePin Success!");
    AK_UnlockMutex(gpMutex);
    return USBKEY_OK;
}


//	验证PIN.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_CheckPin(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_ULONG ulPINType,			//[in]PIN类型
        USBKEY_UCHAR_PTR pcPIN,			//[in]送入的PIN值
        USBKEY_ULONG ulLen,				//[in]送入的PIN长度
        USBKEY_ULONG_PTR pulRetryNum)	//[out]出错后重试次数
{
    USBKEY_UCHAR response[300];
    USBKEY_ULONG rtn,respLen=0;
    USBKEY_UCHAR data[32];
    USBKEY_UCHAR p1,p2,lc,le;

    AK_Log(AK_LOG_DEBUG, "USBKEY_CheckPin");

    if( (ulLen > USBKEY_PIN_MAX_LEN) || (ulLen < USBKEY_PIN_MIN_LEN) )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_CheckPin ==> PIN Length Error,ulLen = %#08x", ulLen);
        return USBKEY_PARAM_ERROR;
    }

    AK_LockMutex(gpMutex);

    if (ulPINType == USBKEY_USERTYPE_ADMIN)
    {
        p2 = USBKEY_ADMIN_PIN_FLAG;
    }
    else if (ulPINType == USBKEY_USERTYPE_USER)
    {
        p2 = USBKEY_USER_PIN_FLAG;
    }

#ifdef _SEC_GTAX_APP
    else if(ulPINType == USBKEY_USERTYPE_HIDISK_ADMIN)
    {
        p2 = USBKEY_HIDISK_ADMIN_PIN_FLAG;
    }
    else if(ulPINType == USBKEY_USERTYPE_HIDISK_USER)
    {
        p2 = USBKEY_HIDISK_USER_PIN_FLAG;
    }
    else if(ulPINType == USBKEY_USERTYPE_HIDISK_TAX)
    {
        p2 = USBKEY_HIDISK_TAX_PIN_FLAG;
    }
#endif

    else
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_CheckPin ==> ulPINType Error,ulPINType = %#08x", ulPINType);
        return USBKEY_PARAM_ERROR;
    }

    p1 = 0x00;
    lc = (USBKEY_UCHAR)ulLen;
    le = 0x00;

    memset(data,0,sizeof(data));
    memcpy(data,pcPIN,ulLen);

   
    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_VerifyPIN,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_CheckPin ==> PackageData() Error, rtn = %#08x", rtn);
        return rtn;
    }

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            if ( (rtn & 0xfff0) == DEVICE_REMAIN_NT )
            {
                *pulRetryNum = rtn & 0x000f;
            }
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_CheckPin ==> DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
            break;
    }

    AK_Log(AK_LOG_DEBUG, "USBKEY_CheckPin Success!");
    AK_UnlockMutex(gpMutex);
    return USBKEY_OK;
}


//	解锁用户PIN.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_UnlockPin(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcAdminPIN,	//[in]送入的管理员PIN
        USBKEY_ULONG ulAdminLen,		//[in]送入的PIN长度
        USBKEY_UCHAR_PTR pcUserNewPIN,	//[in]送入的用户新PIN
        USBKEY_ULONG ulUserNewLen,		//[in]送入的用户新PIN长度
        USBKEY_ULONG_PTR pulRetryNum)	//[out]出错后重试次数
{
    USBKEY_ULONG rtn;

    AK_Log(AK_LOG_DEBUG, "USBKEY_UnlockPin");

    /*if (strlen(pcAppName) > 16)
      {
      LogMessage("====>>USBKEY_UnlockPin->pcAppName Length Error!");
      return USBKEY_PARAM_ERROR;
      }*/
    if( (ulAdminLen > USBKEY_PIN_MAX_LEN) || (ulAdminLen < USBKEY_PIN_MIN_LEN) )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_UnlockPin ==> Admin PIN Length Error,ulAdminLen = %#08x", ulAdminLen);
        return USBKEY_PARAM_ERROR;
    }
    if( (ulUserNewLen > USBKEY_PIN_MAX_LEN) || (ulUserNewLen < USBKEY_PIN_MIN_LEN) )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_UnlockPin ==> New User PIN Length Error,ulUserNewLen = %#08x", ulUserNewLen);
        return USBKEY_PARAM_ERROR;
    }

    /*	rtn = SelectFile(hDeviceHandle,pcAppName,NULL);
        if (rtn != 0)
        {
        LogMessage16Val("====>>USBKEY_UnlockPin->SelectFile() Error,rtn = 0x",rtn);
        return rtn;
        }
     */
    //验证PIN
    rtn = USBKEY_CheckPin(hDeviceHandle,USBKEY_USERTYPE_ADMIN,pcAdminPIN,ulAdminLen,pulRetryNum);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_UnlockPin ==> USBKEY_CheckPin() Error,rtn = %#08x", rtn);
        return rtn;
    }

    //初始化PIN
    rtn = USBKEY_InitPin(hDeviceHandle,pcUserNewPIN,ulUserNewLen);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_UnlockPin ==> USBKEY_InitPin() Error,rtn = %#08x", rtn);
        return rtn;
    }

    AK_Log(AK_LOG_DEBUG, "USBKEY_UnlockPin Success!");

    return USBKEY_OK;
}


//获取PIN信息
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_GetPinInfo(
        USBKEY_HANDLE hDeviceHandle,		//[in]设备句柄
        USBKEY_ULONG ulPINType,				//[in]PIN类型
        USBKEY_ULONG_PTR puMaxRetryNum,		//[out]最大重试次数
        USBKEY_ULONG_PTR pulRemainRetryNum)	//[out]剩余重试次数
{
    USBKEY_ULONG rtn = 0;
    //USBKEY_UCHAR request[16];
    USBKEY_UCHAR response[16];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG respLen=0;
    USBKEY_UCHAR data[16];
    USBKEY_UCHAR p1,p2,lc,le;

    AK_Log(AK_LOG_DEBUG, "USBKEY_GetPinInfo");
    AK_LockMutex(gpMutex);

    /*rtn = SelectFile(hDeviceHandle,pcAppName,NULL);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage16Val("====>>USBKEY_GetPinInfo->SelectFile() Error,rtn = 0x",rtn);
      return rtn;
      }*/

    /*//设置命令头: CLA INS P1 P2 LC
      request[0] = 0xb8;	//CLA
      request[1] = 0x03;	//INS
      request[2] = 0x01;	//P1
      if (ulPINType == USBKEY_USERTYPE_ADMIN)
      {
      request[3] = USBKEY_ADMIN_PIN_FLAG;	//P2
      }
      else if (ulPINType == USBKEY_USERTYPE_USER)
      {
      request[3] = USBKEY_USER_PIN_FLAG;	//P2
      }
#ifdef _SEC_GTAX_APP
    //Add by zhoushenshen in 20101213 for Tax
    else if(ulPINType == USBKEY_USERTYPE_HIDISK_ADMIN)
    {
    request[3] = USBKEY_HIDISK_ADMIN_PIN_FLAG;
    }
    else if(ulPINType == USBKEY_USERTYPE_HIDISK_USER)
    {
    request[3] = USBKEY_HIDISK_USER_PIN_FLAG;
    }
    else if(ulPINType == USBKEY_USERTYPE_HIDISK_TAX)
    {
    request[3] = USBKEY_HIDISK_TAX_PIN_FLAG;
    }
    //End Add ---20101213
#endif
request[4] = 0x02;	//LE	

reqLen = 5;
respLen = 16;
rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
if (rtn != 0)
{
AK_UnlockMutex(gpMutex);
AK_Log(AK_LOG_ERROR, "USBKEY_GetPinInfo ==> DeviceExecCmd() Error,rtn = %#08x", rtn);
return rtn;
}
     */

    //设置命令头
    p1 = 0x01;	//P1
if (ulPINType == USBKEY_USERTYPE_ADMIN)
{
    p2 = USBKEY_ADMIN_PIN_FLAG;	//P2
}
else if (ulPINType == USBKEY_USERTYPE_USER)
{
    p2 = USBKEY_USER_PIN_FLAG;	//P2
}
#ifdef _SEC_GTAX_APP
//Add by zhoushenshen in 20101213 for Tax
else if(ulPINType == USBKEY_USERTYPE_HIDISK_ADMIN)
{
    p2 = USBKEY_HIDISK_ADMIN_PIN_FLAG;
}
else if(ulPINType == USBKEY_USERTYPE_HIDISK_USER)
{
    p2 = USBKEY_HIDISK_USER_PIN_FLAG;
}
else if(ulPINType == USBKEY_USERTYPE_HIDISK_TAX)
{
    p2 = USBKEY_HIDISK_TAX_PIN_FLAG;
}
//End Add ---20101213
#endif
lc = 0x00;
le = 0x02;	//LE

//Add by zhoushenshen in 20121108
respLen = 16;
rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_GetDeviceInfo,response,&respLen);
if (rtn != 0)
{
    AK_UnlockMutex(gpMutex);
    AK_Log(AK_LOG_ERROR, "USBKEY_GetPinInfo ==> PackageData() Error, rtn = %#08x", rtn);
    return rtn;
}
//Add end

rtn = *(response+respLen-2);
rtn = rtn*256 + *(response+respLen-1);
switch(rtn)
{
    case DEVICE_SUCCESS:
        break;
    default:
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_GetPinInfo ==> DeviceExecCmd() Error,code = %#08x", rtn);
        return rtn;
        break;
}

*puMaxRetryNum = response[0];
*pulRemainRetryNum = response[1];

AK_Log(AK_LOG_DEBUG, "USBKEY_GetPinInfo Success!");
AK_UnlockMutex(gpMutex);
return USBKEY_OK;
}

///////////////////////////////////////////////////////////////////////////////
//	文件管理相关API
///////////////////////////////////////////////////////////////////////////////

/**
 *	@创建应用.
 *	@返回值：
 *		0：	正常.
 *		>0:	FAIL，返回的是错误码		////Modify by zhoushenshen 20091118
 *	@param:
 *		hDeviceHandle		[in]设备句柄
 *		pcAppName			[in]送入的应用名称
 *      pcAdminPIN			[in]送入的管理员PIN
 *      ulAdminPINLen		[in]送入的管理员PIN长度
 *      ulAdminPinRetryCount[in]管理员PIN最大重试次数
 *      pcUserPIN			[in]送入的用户PIN
 *      ulUserPINLen		[in]送入的用户PIN长度
 *      ulUserPinRetryCount	[in]用户PIN最大重试次数
 *      ulCreateFileRight	[in]在该应用下创建文件和容器的权限
 */
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_CreateApplication(
        USBKEY_HANDLE hDeviceHandle,	
        USBKEY_UCHAR_PTR pcAppName,		
        USBKEY_UCHAR_PTR pcAdminPIN,		
        USBKEY_ULONG ulAdminPINLen,			
        USBKEY_UCHAR ulAdminPinRetryCount,	
        USBKEY_UCHAR_PTR pcUserPIN,			
        USBKEY_ULONG ulUserPINLen,			
        USBKEY_UCHAR ulUserPinRetryCount,	
        USBKEY_ULONG ulCreateFileRight)		
{
    USBKEY_UCHAR response[300];
    USBKEY_ULONG rtn,respLen=300,recNO,ulFileSize;
    USBKEY_UCHAR p1,p2,lc,le;
    USBKEY_UCHAR_PTR data;
    USBKEY_UCHAR fileName[2];
    USBKEY_UCHAR ulAccessCondition;
    USBKEY_ULONG reTryNum;

    AK_Log(AK_LOG_DEBUG, "USBKEY_CreateApplication");

    if (strlen((char *)pcAppName) > 16)
    {
        AK_Log(AK_LOG_ERROR, 
				"USBKEY_CreateApplication ==> pcAppName Length Error,len = %#08x", 
				strlen((char *)pcAppName));
        return USBKEY_PARAM_ERROR;
    }
    if( (ulAdminPINLen < USBKEY_PIN_MIN_LEN) || (ulAdminPINLen >USBKEY_PIN_MAX_LEN) )
    {
        AK_Log(AK_LOG_ERROR, 
				"USBKEY_CreateApplication ==> ulAdminPINLen Error,ulAdminPINLen = %#08x", 
				ulAdminPINLen);
        return USBKEY_PARAM_ERROR;
    }
    if( (ulUserPINLen < USBKEY_PIN_MIN_LEN) || (ulUserPINLen >USBKEY_PIN_MAX_LEN) )
    {
        AK_Log(AK_LOG_ERROR, 
				"USBKEY_CreateApplication ==> ulUserPINLen Error,ulUserPINLen = %#08x", 
				ulUserPINLen);
        return USBKEY_PARAM_ERROR;
    }

    /***************************创建应用——目录DF*************************
	 *	文件标识			 recNO 10 -> eg:0110,0210,0310
	 *	目录基本文件短标识	 00
	 *	建立文件权限		
	 *	删除文件权限		
	 *	文件名			 pcAppName
	 *****************************************************************/
    p1 = 0x00;
    p2 = 0x01;

    //update in 20130106 DF增加一个目录基本文件短标识
    if (strlen((char *)pcAppName) < 5)
        lc = 2+3+5;
    else
        lc = (USBKEY_UCHAR)(2+3+strlen((char *)pcAppName));

    le = 0x00;
    data = (USBKEY_UCHAR_PTR)malloc(lc);

    AK_LockMutex(gpMutex);

    fileName[0] = 0x80;
    fileName[1] = 0x00;
    rtn = SelectFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"/",fileName);
    if (rtn != 0)
    {
        free(data);
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_CreateApplication ==> SelectFile() Error,rtn = %#08x", rtn);
        return rtn;
    }

    //查找应用记录文件，得到一个空记录的记录号
    rtn = GetRecNo(hDeviceHandle,fileName,&recNO);
    if( (rtn != 0) || (recNO == 0) )
    {
        free(data);
        AK_UnlockMutex(gpMutex);
        if(recNO == 0)
        {
            AK_Log(AK_LOG_ERROR, "USBKEY_CreateApplication ==> Reach the max number of applications.");
            return USBKEY_RECORD_NOT_FOUND;
        }
        AK_Log(AK_LOG_ERROR, "USBKEY_CreateApplication ==> GetRecNo() Error,rtn = %#08x", rtn);
        return rtn;
    }

    // 生成文件标识
    memset(data,0,sizeof(data));
    memcpy(data,&recNO,1);
    memset(data+1,0x10,1);

    switch(ulCreateFileRight)
    {
        case USBKEY_USERTYPE_USER:
            ulAccessCondition  = USBKEY_USER_USE_PRIV;
            break;		
        case USBKEY_USERTYPE_ADMIN:
            ulAccessCondition  = USBKEY_ADMIN_USE_PRIV;
            break;
        case USBKEY_ALL_PRIVILEGE:
            ulAccessCondition  = USBKEY_ALL_PRIV;
            break;
        case USBKEY_NEVER_USE:
            ulAccessCondition  = USBKEY_NOT_USE;
            break;

        default:
            free(data);
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_CreateApplication ==> \
						ulCreateFileRight type Error,ulCreateFileRight = %#08x", ulCreateFileRight);
            return USBKEY_PARAM_ERROR;
    }

    memcpy(data+3,&ulAccessCondition,1);	//建立文件权限
    memcpy(data+4,&ulAccessCondition,1);	//删除文件权限
    memcpy(data+5,pcAppName,strlen((char *)pcAppName));
    if (strlen((char *)pcAppName) < 5)
    {
        memset(data+lc-1,0x01,5-strlen((char *)pcAppName));
    }

    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_CreateFile,response,&respLen);
    if (rtn != 0)
    {
        free(data);
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_CreateApplication ==> PackageData() Error, rtn = %#08x", rtn);
        return rtn;
    }

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        case DEVICE_CMD_DATA_ERROR:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_CreateApplication ==> DeviceExecCmd() Error,code = %#08x", rtn);
            return USBKEY_FILE_EXIST;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_CreateApplication ==> DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
    }


    /***********************创建应用的系统密钥文件*************************
	 *	文件标识		3f 01
	 *	文件写入模式	00
	 *	读权限		00
	 *	写权限		99
	 *	记录长度		32
	 *	记录数		10
	 ******************************************************************/
    p1 = 0x00;
    p2 = 0x06;
    lc = 7;
    le = 0x00;
    data = (USBKEY_UCHAR_PTR)malloc(lc);

    fileName[0] = 0x3f;
    fileName[1] = 0x01;

    memset(data,0,7);
    memcpy(data,fileName,2);
    le = 0x00;
    memcpy(data+3,&le,1);
    le = USBKEY_DEVICE_PRIV;
    memcpy(data+4,&le,1);
    le = USBKEY_KEY_REC_LEN;
    memcpy(data+5,&le,1);
    le = MAX_SYS_KEY_NUM;
    memcpy(data+6,&le,1);

    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_CreateFile,response,&respLen);
    if (rtn != 0)
    {
        free(data);
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_CreateApplication ==> PackageData() Create APP SysKey File Error, rtn = %#08x", rtn);
        return rtn;
    }

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_CreateApplication ==> DeviceExecCmd() Create APP SysKey File Error,code = %#08x", rtn);
            return rtn;
            break;
    }	
    AK_Log(AK_LOG_DEBUG, "USBKEY_CreateApplication Create APP System Key File Success!");


    /*******************************创建管理员PIN***************************/
    rtn = CreateAppPin(
            hDeviceHandle,			//[in]设备句柄
            pcAdminPIN,				//[in]送入的PIN
            ulAdminPINLen,			//[in]送入的PIN长度
            USBKEY_ADMIN_PIN_FLAG,	//[in]PIN码标识
            USBKEY_ADMIN_PRIV,		//[in]PIN验证后安全状态
            ulAdminPinRetryCount,	//[in]重试次数
            USBKEY_ALL_PRIV,		//[in]使用权限
            USBKEY_ADMIN_PRIV);		//[in]更改权限
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_CreateApplication ==> CreateAppPin() APP Admin Key Error,rtn = %#08x", rtn);
        return rtn;
    }
    AK_Log(AK_LOG_DEBUG, "USBKEY_CreateApplication Create APP Admin PIN Success!");


    /********************************创建用户PIN****************************/
    rtn = CreateAppPin(
            hDeviceHandle,			//[in]设备句柄
            pcUserPIN,				//[in]送入的PIN
            ulUserPINLen,			//[in]送入的PIN长度
            USBKEY_USER_PIN_FLAG,	//[in]PIN码标识
            USBKEY_USER_PRIV,		//[in]PIN验证后安全状态
            ulUserPinRetryCount,	//[in]重试次数
            USBKEY_ALL_PRIV,		//[in]使用权限
            0x63);					//[in]更改权限
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_CreateApplication ==> CreateAppPin() APP User Key Error,rtn = %#08x", rtn);
        return rtn;
    }
    AK_Log(AK_LOG_DEBUG, "USBKEY_CreateApplication Create APP User PIN Success!");


#ifdef _SEC_GTAX_APP
    //Add by zhoushenshen in 20101213 for Tax
    //**************************创建隐藏区高级管理用户PIN（税控金税卡应用）*************************/
    rtn = CreateAppPin(
            hDeviceHandle,					//[in]设备句柄
            (USBKEY_UCHAR_PTR)USBKEY_HIDISK_ADMIN_PIN,//[in]送入的PIN
            HIDISK_ADMIN_PIN_LEN,			//[in]送入的PIN长度
            USBKEY_HIDISK_ADMIN_PIN_FLAG,	//[in]PIN码标识
            USBKEY_HIDISK_ADMIN_PRIV,		//[in]PIN验证后安全状态
            ulUserPinRetryCount,			//[in]重试次数
            USBKEY_ALL_PRIV,				//[in]使用权限
            0x65);							//[in]更改权限
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_CreateApplication ==> CreateAppPin() APP HiDisk Admin Key Error,rtn = %#08x", rtn);
        return rtn;
    }
    AK_Log(AK_LOG_DEBUG, "USBKEY_CreateApplication Create APP HidDisk Admin PIN Success!");

    //**************************创建隐藏区普通用户PIN（税控金税卡应用）*************************/
    rtn = CreateAppPin(
            hDeviceHandle,					//[in]设备句柄
            (USBKEY_UCHAR_PTR)USBKEY_HIDISK_USER_PIN,//[in]送入的PIN
            HIDISK_USER_PIN_LEN,			//[in]送入的PIN长度
            USBKEY_HIDISK_USER_PIN_FLAG,	//[in]PIN码标识
            USBKEY_HIDISK_USER_PRIV,		//[in]PIN验证后安全状态
            USBKEY_TAX_PIN_RETRY_TIMES,		//[in]重试次数
            USBKEY_ALL_PRIV,				//[in]使用权限
            0x54);							//[in]更改权限
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_CreateApplication ==> CreateAppPin() APP HiDisk User Key Error,rtn = %#08x", rtn);
        return rtn;
    }
    AK_Log(AK_LOG_DEBUG, "USBKEY_CreateApplication Create APP HidDisk User PIN Success!");

    //**************************创建隐藏区税局用户PIN（税控金税卡应用）*************************/
    rtn = CreateAppPin(
            hDeviceHandle,					//[in]设备句柄
            (USBKEY_UCHAR_PTR)USBKEY_HIDISK_TAX_PIN,//[in]送入的PIN
            HIDISK_TAX_PIN_LEN,				//[in]送入的PIN长度
            USBKEY_HIDISK_TAX_PIN_FLAG,		//[in]PIN码标识
            USBKEY_HIDISK_TAX_PRIV,			//[in]PIN验证后安全状态
            USBKEY_TAX_PIN_RETRY_TIMES,		//[in]重试次数
            USBKEY_ALL_PRIV,				//[in]使用权限
            USBKEY_HIDISK_TAX_PRIV);		//[in]更改权限
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_CreateApplication ==> CreateAppPin() APP HiDisk Tax Key Error,rtn = %#08x", rtn);
        return rtn;
    }
    AK_Log(AK_LOG_DEBUG, "USBKEY_CreateApplication Create APP HidDisk Tax PIN Success!");
    //End Add ---20101213
#endif

    /**********************************验证PIN权限*******************************/
    if(USBKEY_USERTYPE_USER == ulCreateFileRight)
    {
        //验证PIN
        rtn = USBKEY_CheckPin(hDeviceHandle,ulCreateFileRight,pcUserPIN,ulUserPINLen,&reTryNum);
        if (rtn != 0)
        {
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_CreateApplication ==> USBKEY_CheckPin() User Error,rtn = %#08x", rtn);
            return rtn;
        }
    }
    else if(USBKEY_USERTYPE_ADMIN == ulCreateFileRight)
    {
        //验证PIN
        rtn = USBKEY_CheckPin(hDeviceHandle,ulCreateFileRight,pcAdminPIN,ulAdminPINLen,&reTryNum);
        if (rtn != 0)
        {
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_CreateApplication ==> USBKEY_CheckPin() Admin Error,rtn = %#08x", rtn);
            return rtn;
        }
    }

    /*******************创建应用的公共密钥文件，存放临时RSA公钥*****************
	 *	文件标识		ff 01
	 *	文件写入模式	00
	 *	读权限		f0
	 *	写权限		f0
	 *	文件长度		516
	 **********************************************************************/

    // 生成数据
    p1 = 0x00;
    p2 = 0x10;
    lc = 0x07;						//lc updtae in 20120106
    le = 0x00;
    data = (USBKEY_UCHAR_PTR)malloc(lc);

    fileName[0] = 0xff;
    fileName[1] = 0x01;

    memset(data,0,9);
    memcpy(data,fileName,2);
    le = 0xf0;
    memcpy(data+3,&le,1);
    memcpy(data+4,&le,1);
    ulFileSize = 516;
    le = (USBKEY_UCHAR)(ulFileSize/256);
    memcpy(data+5,&le,1);
    le = (USBKEY_UCHAR)ulFileSize;
    memcpy(data+6,&le,1);	

    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_CreateFile,response,&respLen);
    if (rtn != 0)
    {
        free(data);
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_CreateApplication ==> PackageData() Create APP Temp RSA Public Key File Error, rtn = %#08x", rtn);
        return rtn;
    }

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_CreateApplication ==> DeviceExecCmd() Create APP Temp RSA Public Key File Error,code = %#08x", rtn);
            return rtn;
    }
    AK_Log(AK_LOG_DEBUG, "USBKEY_CreateApplication Create APP Temp RSA Public Key File Success!");

    /*******************创建应用的私有密钥文件，存放临时RSA私钥*****************
	 *	文件标识		ff 02
	 *	文件写入模式	00
	 *	读权限		f0
	 *	写权限		f0
	 *	文件长度		1412
	 **********************************************************************/

    // 生成数据
    p1 = 0x00;
    p2 = 0x11;
    lc = 0x07;						//lc updtae in 20120106
    le = 0x00;
    data = (USBKEY_UCHAR_PTR)malloc(lc);

    fileName[0] = 0xff;
    fileName[1] = 0x02;

    memset(data,0,9);
    memcpy(data,fileName,2);
    le = 0xf0;
    memcpy(data+3,&le,1);
    memcpy(data+4,&le,1);
    ulFileSize = 1412;
    le = (USBKEY_UCHAR)(ulFileSize/256);
    memcpy(data+5,&le,1);
    le = (USBKEY_UCHAR)ulFileSize;
    memcpy(data+6,&le,1);	

    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_CreateFile,response,&respLen);
    if (rtn != 0)
    {
        free(data);
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_CreateApplication ==> PackageData() Create APP Temp RSA Private Key File Error, rtn = %#08x", rtn);
        return rtn;
    }

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_CreateApplication ==> DeviceExecCmd() Create APP Temp RSA Private Key File Error,code = %#08x", rtn);
            return rtn;
    }
    AK_Log(AK_LOG_DEBUG, "USBKEY_CreateApplication Create APP Temp RSA Private Key File Success!");


    /*******************创建应用的用户密钥文件，存放TOKEN对称密钥*************	
	 *	文件标识		3f 02
	 *	文件写入模式	00
	 *	读权限		00
	 *	写权限		ulAccessCondition
	 *	记录长度		32
	 *	记录数		64
	 ******************************************************************/

    p1 = 0x00;
    p2 = 0x07;
    lc = 7;
    le = 0x00;
    data = (USBKEY_UCHAR_PTR)malloc(lc);

    fileName[0] = 0x3f;
    fileName[1] = 0x02;

    memset(data,0,7);
    memcpy(data,fileName,2);
    le = 0x00;
    memcpy(data+3,&le,1);
    memcpy(data+4,&ulAccessCondition ,1);
    le = USBKEY_KEY_REC_LEN;
    memcpy(data+5,&le,1);
    le = MAX_USER_KEY_NUM;
    memcpy(data+6,&le,1);

    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_CreateFile,response,&respLen);
    if (rtn != 0)
    {
        free(data);
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_CreateApplication ==> PackageData() Create APP User Key File Error, rtn = %#08x", rtn);
        return rtn;
    }

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_CreateApplication ==> DeviceExecCmd() Create APP User Key File Error,code = %#08x", rtn);
            return rtn;
            break;
    }

    AK_Log(AK_LOG_DEBUG, "USBKEY_CreateApplication Create APP User Key File Success!");

    /*********************创建记录文件，记录应用下的文件ID*******************
	 *	定长记录文件
	 *	名称			80 01
	 *	记录长度		0x40
	 *	记录数		0x20
	 *******************************************************************/
    rtn = CreateFileRecord(hDeviceHandle, ulAccessCondition);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_CreateApplication ==> CreateFileRecord() Error,rtn = %#08x", rtn);
        return rtn;
    }

    /***************************记录应用下的容器列表********************
	 *	定长记录文件
	 *	名称			80 02
	 *	记录长度		fe
	 *	记录数		0x06
	 ****************************************************************/
    rtn = CreateContainerRecord(hDeviceHandle, ulAccessCondition);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_CreateApplication ==> CreateContainerRecord() Error,rtn = %#08x", rtn);
        return rtn;
    }

    /*********************创建二进制文件，记录应用下的容器对称密钥ID*******************
	 *	文件标识		80 03
	 *	文件写入模式	0
	 *	读权限		33
	 *	写权限		33
	 *	文件长度		254
	 ****************************************************************************/
    fileName[0] = (USBKEY_SYM_KEY_FILE_ID&0xff00)>>8;
    fileName[1] = USBKEY_SYM_KEY_FILE_ID&0xff;
    rtn = CreateBinaryFile(hDeviceHandle, fileName, 0, USBKEY_USER_USE_PRIV, 
			USBKEY_USER_USE_PRIV, USBKEY_SYM_KEY_ID_FILE_LEN);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "====>>USBKEY_CreateApplication->CreateBinaryFile() Error,rtn = 0x%04x",rtn);
        return rtn;
    }


#ifdef _SEC_GTAX_APP
    /**********************创建二进制文件，记录税控应用信息（税控金税卡应用）**************************/
    fileName[0] = (USBKEY_TAX_INFO_FILE_ID&0xff00)>>8;
    fileName[1] = USBKEY_TAX_INFO_FILE_ID&0xff;
    //rtn = CreateBinaryFile(hDeviceHandle, fileName, 0, 0x41, USBKEY_HIDISK_TAX_PRIV, USBKEY_TAX_INFO_ID_FILE_LEN);
    rtn = CreateBinaryFile(hDeviceHandle, fileName, 0, 0xf0, USBKEY_HIDISK_TAX_PRIV, USBKEY_TAX_INFO_ID_FILE_LEN);	//Update by zhoushenshen in 20110214
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "====>>USBKEY_CreateApplication ==> CreateBinaryFile() Tax Info Error,rtn = 0x%04x",rtn);
        return rtn;
    }
#endif

    // 添加应用记录
    rtn = SelectFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"/",NULL);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "====>>USBKEY_CreateApplication->SelectFile() / Error,rtn = 0x%04x",rtn);
        return rtn;
    }
	/**
	 *	addfile 80 00
	 *	
	 */
    rtn = AddDirRecord(hDeviceHandle,pcAppName);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_CreateApplication ==> AddDirRecord() Error,rtn = %#08x", rtn);
        return rtn;
    }

    rtn = SelectFile(hDeviceHandle,pcAppName,NULL);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "====>>USBKEY_CreateApplication->SelectFile() pcAppName Error,rtn = 0x%04x",rtn);
        return rtn;
    }

    AK_Log(AK_LOG_DEBUG, "USBKEY_CreateApplication Success!");
    AK_UnlockMutex(gpMutex);
    return USBKEY_OK;
}


//	删除应用.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DeleteApplication(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcAppName)		//[in]送入的应用名称

{
    USBKEY_UCHAR response[300];
    USBKEY_ULONG rtn,respLen=0;
    USBKEY_UCHAR p1,p2,lc,le;
    USBKEY_UCHAR_PTR data;

    AK_Log(AK_LOG_DEBUG, "USBKEY_DeleteApplication");

    if (strlen((char *)pcAppName) > 16)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_DeleteApplication ==> pcAppName Length Error,rtn = %#08x", USBKEY_PARAM_ERROR);
        return USBKEY_PARAM_ERROR;
    }
   
    AK_LockMutex(gpMutex);

    rtn = SelectFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"/",NULL);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_DeleteApplication ==> SelectFile() Error,rtn = %#08x", rtn);
        return rtn;
    }

    //*
    /*****************************************删除应用*******************************************/
    p1 = 0x04;
    p2 = 0x00;
    if (strlen((char *)pcAppName) < 5)
        lc = 5;
    else
        lc = (USBKEY_UCHAR)strlen((char *)pcAppName);
    le = 0x00;
    data = (USBKEY_UCHAR_PTR)malloc(lc);

    ////生成文件标识
    //memcpy(data+2,ulAccessCondition,1);
    memcpy(data,pcAppName,strlen((char *)pcAppName));
    if (strlen((char *)pcAppName) < 5)
    {
        memset(data+strlen((char *)pcAppName),0x01,5-strlen((char *)pcAppName));
    }

    //Delete by zhoushenshen in 20121109
    /*rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_DeleteFile,request,&reqLen);
      if (rtn != 0)
      {
      free(data);
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_DeleteApplication ==> PackageData() Error,rtn = %#08x", rtn);
      return rtn;
      }
      free(data);

      respLen = 300;
      rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_DeleteApplication ==> DeviceExecCmd() Error,rtn = %#08x", rtn);
      return rtn;
      }*/
    //Del end

    //Add by zhoushenshen in 20121109
    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_DeleteFile,response,&respLen);
    if (rtn != 0)
    {
        free(data);
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_DeleteApplication ==> PackageData() Error, rtn = %#08x", rtn);
        return rtn;
    }
    //Add end

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_DeleteApplication ==> DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
            break;
    }

    //*/
    //删除应用记录
    rtn = DeleDirRecord(hDeviceHandle,pcAppName);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_DeleteApplication ==> DeleDirRecord() Error,rtn = %#08x", rtn);
        return rtn;
    }

    AK_Log(AK_LOG_DEBUG, "USBKEY_DeleteApplication Success!");

    AK_UnlockMutex(gpMutex);
    return USBKEY_OK;
}


//	枚举应用.
//	如果pcApplicationNameList为NULL，则从pulSize返回需要分配空间数量，pulAppCount返回应用的个数
//	如果pcApplicationNameList非空，则从pcApplicationNameList返回应用列表，应用间以'\0'分隔，
//		并从pulSize返回实际使用空间大小，pulAppCount返回应用的个数
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_EnumApplication(
        USBKEY_HANDLE hDeviceHandle,	    //[in]设备句柄
        USBKEY_UCHAR_PTR pcApplicationNameList,		//[in,out]应用信息列表
        USBKEY_ULONG_PTR pulSize,		    //[in,out]输入为缓冲区大小，输出为实际空间大小
        USBKEY_ULONG_PTR pulAppCount)		//[out]输出应用个数
{
    USBKEY_ULONG recordLen,recNO;
    USBKEY_ULONG rtn = 0;
    USBKEY_ULONG len;
    USBKEY_UCHAR record[NAME_ID_MAX_LEN],deleRecord[NAME_ID_MAX_LEN],fileName[2];

    AK_Log(AK_LOG_DEBUG, "USBKEY_EnumApplication");

    *pulAppCount = 0;
    recNO = 0;

    memset(deleRecord,0xff,sizeof(deleRecord));
    memset(record,0,sizeof(record));

    fileName[0] = 0x80;
    fileName[1] = 0x00;

    AK_LockMutex(gpMutex);
    rtn = SelectFile(hDeviceHandle, (USBKEY_UCHAR_PTR)"/", fileName);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_EnumApplication ==> SelectFile() Error,rtn = %#08x", rtn);
        return rtn;
    }

    if (pcApplicationNameList == NULL)
    {
        *pulSize = 0;
        for (recNO = 1; recNO <= APPLICATION_MAX_QUANTITY ; recNO ++)
        {
            recordLen = 16;
            rtn = ReadRecord(hDeviceHandle,fileName,recNO,record,&recordLen);	//Update by zhoushenshen 20100309

            if (rtn != 0)
            {
                AK_UnlockMutex(gpMutex);
                AK_Log(AK_LOG_ERROR, "USBKEY_EnumApplication ==> ReadRecord() Error,rtn = %#08x", rtn);
                return rtn;
            }
            if (memcmp(deleRecord,record,recordLen) != 0)
            {
                *pulSize = *pulSize + strlen((char *)record) + 1;
                (*pulAppCount)++;
            }
        }
        if ((*pulSize) == 0)
        {
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_DEBUG, "USBKEY_EnumApplication ==> No Record Found!");
            return USBKEY_OK;
        }
        *pulSize = *pulSize +1;
    }
    else
    {
        len = *pulSize;
        memset(pcApplicationNameList,0,len);
        for (recNO = 1; recNO <= APPLICATION_MAX_QUANTITY ; recNO ++)
        {
            recordLen = 16;
            rtn = ReadRecord(hDeviceHandle,fileName,recNO,record,&recordLen);	//Update by zhoushenshen 20100309

            if (rtn != 0)
            {
                AK_UnlockMutex(gpMutex);
                AK_Log(AK_LOG_ERROR, "USBKEY_EnumApplication ==> ReadRecord() Error,rtn = %#08x", rtn);
                return rtn;
            }
            if (memcmp(deleRecord,record,recordLen) != 0)
            {
                if (len < (strlen((char *)record) + 2) )
                {
                    AK_UnlockMutex(gpMutex);
                    AK_Log(AK_LOG_ERROR, "USBKEY_EnumApplication ==> pcApplicationNameList buffer is too short!,rtn = %#08x", USBKEY_BUFFER_TOO_SHORT);
                    return USBKEY_BUFFER_TOO_SHORT;
                }
                memcpy(pcApplicationNameList + *pulSize - len,record,strlen((char *)record));
                len = len - strlen((char *)record) - 1;
                (*pulAppCount)++;
            }
        }
        if ((*pulSize) == len)
        {
            AK_UnlockMutex(gpMutex);
            *pulSize = 0;
            AK_Log(AK_LOG_DEBUG, "USBKEY_EnumApplication ==> No Record Found!");
            return USBKEY_OK;
        }
        if (len > 1)
            *pulSize = *pulSize - len + 1;
    }
    AK_Log(AK_LOG_DEBUG, "USBKEY_EnumApplication Success!");

    AK_UnlockMutex(gpMutex);
    return USBKEY_OK;
}

//	打开应用.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_OpenApplication(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcAppName)	//[in]送入的应用名称
{
    USBKEY_ULONG rtn;
    AK_Log(AK_LOG_DEBUG, "USBKEY_OpenApplication");

    if (strlen((char *)pcAppName) > 16)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_OpenApplication ==> pcAppName Length Error,rtn = %#08x", USBKEY_PARAM_ERROR);
        return USBKEY_PARAM_ERROR;
    }
    AK_LockMutex(gpMutex);
    rtn = SelectFile(hDeviceHandle,pcAppName,NULL);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_OpenApplication ==> SelectFile() Error,rtn = %#08x", rtn);
        return rtn;
    }

    AK_Log(AK_LOG_DEBUG, "USBKEY_OpenApplication Success!");

    AK_UnlockMutex(gpMutex);
    return USBKEY_OK;
}

//	创建文件.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_CreateFile(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcFileName,	//[in]文件名称
        USBKEY_ULONG ulFileSize,		//[in]文件大小
        USBKEY_ULONG ulReadRights,		//[in]文件读权限	//ADD by zhoushenshen 20091116
        USBKEY_ULONG ulWriteRights)		//[in]文件写权限
{
    USBKEY_UCHAR response[300];
    USBKEY_ULONG rtn,respLen=0,recNO;
    USBKEY_UCHAR p1,p2,lc,le,wpriv,rpriv;
    USBKEY_UCHAR data[9],pcFileID[2],temp[USBKEY_FILE_RECORD_LEN];

    AK_Log(AK_LOG_DEBUG, "USBKEY_CreateFile");

    if (ulFileSize >= USBKEY_MAX_FILE_LEN)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_CreateFile ==> ulFileSize Length Error,ulFileSize = %#08x", ulFileSize);
        return USBKEY_PARAM_ERROR;
    }

    AK_LockMutex(gpMutex);

    pcFileID[0] = 0x80;
    pcFileID[1] = 0x01;
    rtn = SelectFile(hDeviceHandle,NULL,pcFileID);	//ADD by zhoushenshen 20100309
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_CreateFile ==> SelectFile() Error,rtn = %#08x", rtn);
        return rtn;
    }

    //检查文件是否重名
    rtn = SearchFileRecNo(hDeviceHandle,pcFileName,&recNO);
    if( rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_CreateFile ==> SearchFileRecNo() Error,rtn = %#08x", rtn);
        return USBKEY_FILE_NOT_FOUND;
    }
    if ( recNO != 0 )
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_DEBUG, "USBKEY_CreateFile ==> pcFileName Exist!");
        return USBKEY_FILE_EXIST;
    }

    /////////检查权限///////////////////////////////////////////////////
    switch(ulReadRights)
    {
        case USBKEY_USERTYPE_USER:
            rpriv  = USBKEY_USER_USE_PRIV;
            break;		
        case USBKEY_USERTYPE_ADMIN:
            rpriv = USBKEY_ADMIN_USE_PRIV;
            break;
        case USBKEY_ALL_PRIVILEGE:
            rpriv  = USBKEY_ALL_PRIV;
            break;
        case USBKEY_NEVER_USE:
            rpriv  = USBKEY_NOT_USE;
            break;
#ifdef _SEC_GTAX_APP
            //Add by zhoushenshen in 20101215 for Tax
        case USBKEY_USERTYPE_HIDISK_USER:
            rpriv = USBKEY_HIDISK_USER_PRIV;
            break;
        case USBKEY_USERTYPE_HIDISK_ADMIN:
            rpriv = USBKEY_HIDISK_ADMIN_PRIV;
            break;
        case USBKEY_USERTYPE_HIDISK_TAX:
            rpriv = USBKEY_HIDISK_TAX_PRIV;
            break;
            //End Add ---20101215
#endif
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_CreateFile ==> ulReadRights type Error,ulReadRights = %#08x", ulReadRights);
            return USBKEY_PARAM_ERROR;
    }

    switch(ulWriteRights)
    {
        case USBKEY_USERTYPE_USER:
            wpriv  = USBKEY_USER_USE_PRIV;
            break;		
        case USBKEY_USERTYPE_ADMIN:
            wpriv = USBKEY_ADMIN_USE_PRIV;
            break;
        case USBKEY_ALL_PRIVILEGE:
            wpriv  = USBKEY_ALL_PRIV;
            break;
        case USBKEY_NEVER_USE:
            wpriv  = USBKEY_NOT_USE;
            break;
#ifdef _SEC_GTAX_APP
            //Add by zhoushenshen in 20101215 for Tax
        case USBKEY_USERTYPE_HIDISK_USER:
            wpriv = USBKEY_HIDISK_USER_PRIV;
            break;
        case USBKEY_USERTYPE_HIDISK_ADMIN:
            wpriv = USBKEY_HIDISK_ADMIN_PRIV;
            break;
        case USBKEY_USERTYPE_HIDISK_TAX:
            wpriv = USBKEY_HIDISK_TAX_PRIV;
            break;
            //End Add ---20101215
#endif
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_CreateFile ==> ulWriteRights type Error,ulWriteRights = %#08x", ulWriteRights);
            return USBKEY_PARAM_ERROR;
    }
   
    //获得文件记录中的记录号，作为文件ID一部分
    rtn = GetFileRecNo(hDeviceHandle,pcFileID,&recNO);	//Update by zhoushenshen 20100309
    if( (rtn != 0) || (recNO == 0) )
    {
        AK_UnlockMutex(gpMutex);
        if(recNO == 0)
        {
            AK_Log(AK_LOG_ERROR, "USBKEY_CreateFile ==> Reach the max number of files.");
            return USBKEY_RECORD_NOT_FOUND;
        }
        AK_Log(AK_LOG_ERROR, "USBKEY_CreateFile ==> GetFileRecNo() Error,rtn = %#08x", rtn);
        return rtn;
    }

    //创建二进制文件
    p1 = 0x00;
    p2 = 0x02;
    //lc = 9;
    lc = 0x07;						//lc updtae in 20120106
    le = 0x00;

    memset(data,0,9);
    memset(temp,0,USBKEY_FILE_RECORD_LEN);
    //获得文件ID: fe+记录号
    //写入记录文件信息 格式：文件ID+读权限+写权限+大小+文件名
    le = 0xfe;
    memcpy(data,&le,1);
    memcpy(data+1,&recNO,1);	
    memcpy(data+3,&rpriv,1);
    memcpy(data+4,&wpriv,1);
    memcpy(temp,&le,1);
    memcpy(temp+1,&recNO,1);	
    memcpy(temp+2,&rpriv,1);
    memcpy(temp+3,&wpriv,1);

    /*memcpy(temp,pcFileName,2);
      memcpy(temp+2,&le,1);
      memcpy(temp+3,&le,1);	
      memcpy(data+3,&le,1);
      memcpy(data+4,&le,1);*/

    //updtae in 20120106 文件长度由4字节改为2字节,文件记录中文件长度维持4字节不变
    le = (USBKEY_UCHAR)ulFileSize;
    //memcpy(data+8,&le,1);
    memcpy(data+6,&le,1);
    memcpy(temp+7,&le,1);
    ulFileSize = ulFileSize/256;
    le = (USBKEY_UCHAR)ulFileSize;
    //memcpy(data+7,&le,1);
    memcpy(data+5,&le,1);
    memcpy(temp+6,&le,1);
    ulFileSize = ulFileSize/256;
    le = (USBKEY_UCHAR)ulFileSize;
    //memcpy(data+6,&le,1);
    memcpy(temp+5,&le,1);
    ulFileSize = ulFileSize/256;
    le = (USBKEY_UCHAR)ulFileSize;
    //memcpy(data+5,&le,1);
    memcpy(temp+4,&le,1);

    strcpy((char *)temp+8,(char *)pcFileName);

    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_CreateFile,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_CreateFile ==> PackageData() Error, rtn = %#08x", rtn);
        return rtn;
    }

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_CreateFile ==> DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
            break;
    }

    //将新建文件的信息加入文件记录文件ID+读权限+写权限+大小+文件名
    rtn = AddFileRecord(hDeviceHandle,temp);	//Update by zhoushenshen 20100309
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_CreateFile ==> AddFileRecord() Error,rtn = %#08x", rtn);
        return rtn;
    }

    AK_Log(AK_LOG_DEBUG, "USBKEY_CreateFile Success!");
    AK_UnlockMutex(gpMutex);

    return USBKEY_OK;
}

//	删除文件.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DeleteFile(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcFileName)	//[in]文件名称
{
    //USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0;
    USBKEY_UCHAR p1,p2,lc,le;
    USBKEY_UCHAR fileInfo[USBKEY_FILE_RECORD_LEN];

    AK_Log(AK_LOG_DEBUG, "USBKEY_DeleteFile");

    /*if (strlen(pcAppName) > 16)
      {
      LogMessage("====>>USBKEY_DeleteFile->pcAppName Length Error!");
      return USBKEY_PARAM_ERROR;
      }*/

    /*rtn = CheckValid(pcFileName);
      if (rtn != 0)
      {
      LogMessage("====>>USBKEY_DeleteFile->File Name Error!");
      return rtn;
      }*/
    AK_LockMutex(gpMutex);
    /*rtn = SelectFile(hDeviceHandle,pcAppName,NULL);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage16Val("====>>USBKEY_DeleteFile->SelectFile() Error,rtn = 0x",rtn);
      return rtn;
      }*/

    /////////检查权限///////////////////////////////////////////////////
    /*rtn = CheckPrivillege(hDeviceHandle);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage16Val("====>>USBKEY_DeleteFile->CheckPrivillege() Error,rtn = 0x",rtn);
      return rtn;
      }*/
    //////////////////////////////////////////////////////////////////////////

    //获得文件信息
    rtn = GetFileInfo(hDeviceHandle,pcFileName,fileInfo);	//Update by zhoushenshen 20100309
    AK_UnlockMutex(gpMutex);
    if (rtn != 0)
    {	
        AK_Log(AK_LOG_ERROR, "USBKEY_DeleteFile ==> GetFileInfo() Error,rtn = %#08x", rtn);
        return rtn;
    }


    //删除文件
    p1 = 0x00;
    p2 = 0x00;
    lc = 2;
    le = 0x00;

    /*	////生成文件标识
    //memcpy(data+2,ulAccessCondition,1);
    le = 0xfe;
    memcpy(data,&le,1);
    memcpy(data+1,&recNO,1);
    //memcpy(data,pcFileName,2);*/

    //Delete by zhoushenshen in 20121109
    /*rtn (p1,p2,lc,fileInfo,le,USBKEY_OP_DeleteFile,request,&reqLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_DeleteFile ==> PackageData() Error,rtn = %#08x", rtn);
      return rtn;
      }

      respLen = 300;
      rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_DeleteFile ==> DeviceExecCmd() Error,rtn = %#08x", rtn);
      return rtn;
      }*/
    //Del end

    //Add by zhoushenshen in 20121109
    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,fileInfo,le,USBKEY_OP_DeleteFile,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_DeleteFile ==> PackageData() Error, rtn = %#08x", rtn);
        return rtn;
    }
    //Add end

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_DeleteFile ==> DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
            break;
    }

    //删除文件记录
    rtn = DeleFileRecord(hDeviceHandle,pcFileName);		//Update by zhoushenshen 20100309
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_DeleteFile ==> DeleFileRecord() Error,rtn = %#08x", rtn);
        return rtn;
    }

    AK_Log(AK_LOG_DEBUG, "USBKEY_DeleteFile Success!");
    AK_UnlockMutex(gpMutex);

    return USBKEY_OK;
}

//	枚举文件.
//	如果pcFileNameList为NULL，则从pulSize返回需要分配char *[NAME_ID_MAX_LEN]数量
//	如果pcFileNameList非空，则从pcFileNameList返回文件列表，文件名间以'\0'分隔，
//		并从pulSize返回实际文件个数
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_EnumFile(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcFileNameList,		//[out]文件信息列表
        USBKEY_ULONG_PTR pulSize)	//[in,out]输入为缓冲区大小，输出为实际个数
{
    USBKEY_ULONG recordLen,recNO;
    USBKEY_ULONG rtn = 0;
    USBKEY_ULONG len;
    USBKEY_UCHAR record[USBKEY_FILE_RECORD_LEN],deleRecord[USBKEY_FILE_RECORD_LEN],fileName[2];

    AK_Log(AK_LOG_DEBUG, "USBKEY_EnumFile");

    /*if (strlen(pcAppName) > 16)
      {
      LogMessage("====>>USBKEY_EnumFile->pcAppName Length Error!");
      return USBKEY_PARAM_ERROR;
      }*/

    //recNO = 0;
    memset(deleRecord,0xff,sizeof(deleRecord));
    memset(record,0,sizeof(record));
    fileName[0] = 0x80;
    fileName[1] = 0x01;

    AK_LockMutex(gpMutex);
    rtn = SelectFile(hDeviceHandle, NULL, fileName);	//Update by zhoushenshen 20100309
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_EnumFile ==> SelectFile() Error,rtn = %#08x", rtn);
        return rtn;
    }

    /////////检查权限///////////////////////////////////////////////////
    /*rtn = CheckPrivillege(hDeviceHandle);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage16Val("====>>USBKEY_EnumFile->CheckPrivillege() Error,rtn = 0x",rtn);
      return rtn;
      }*/
    //////////////////////////////////////////////////////////////////////////

    if (pcFileNameList == NULL)
    {
        *pulSize = 0;
        for (recNO = 1; recNO <= FILE_MAX_QUANTITY ; recNO ++)
        {
            recordLen = USBKEY_FILE_RECORD_LEN;
            rtn = ReadRecord(hDeviceHandle,fileName,recNO,record,&recordLen);	//Update by zhoushenshen 20100309

            if (rtn != 0)
            {
                AK_UnlockMutex(gpMutex);
                AK_Log(AK_LOG_ERROR, "USBKEY_EnumFile ==> ReadRecord() Error,rtn = %#08x", rtn);
                return rtn;
            }
            if (memcmp(deleRecord,record,recordLen) != 0)
            {
                *pulSize = *pulSize + strlen((char *)record+8) + 1;
            }
        }
        if ((*pulSize) == 0)
        {
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_DEBUG, "USBKEY_EnumFile ==> No Record Found");
            return USBKEY_OK;
        }
        *pulSize = *pulSize +1;
    }
    else
    {
        len = *pulSize;
        memset(pcFileNameList,0,len);
        for (recNO = 1; recNO <= FILE_MAX_QUANTITY ; recNO ++)
        {
            recordLen = USBKEY_FILE_RECORD_LEN;
            rtn = ReadRecord(hDeviceHandle,fileName,recNO,record,&recordLen);	//Update by zhoushenshen 20100309
            if (rtn != 0)
            {
                AK_UnlockMutex(gpMutex);
                AK_Log(AK_LOG_ERROR, "USBKEY_EnumFile ==> ReadRecord() Error,rtn = %#08x", rtn);
                return rtn;
            }
            if (memcmp(deleRecord,record,recordLen) != 0)
            {
                if (len < (strlen((char *)record+8) + 2) )
                {
                    AK_UnlockMutex(gpMutex);
                    AK_Log(AK_LOG_ERROR, "USBKEY_EnumFile ==> pcFileNameList buffer is too short!,rtn = %#08x", USBKEY_BUFFER_TOO_SHORT);
                    return USBKEY_BUFFER_TOO_SHORT;
                }
                memcpy(pcFileNameList + *pulSize - len,record+8,strlen((char *)record+8));
                len = len - strlen((char *)record+8) - 1;
            }
        }
        if ((*pulSize) == len)
        {
            AK_UnlockMutex(gpMutex);
            *pulSize = 0;
            AK_Log(AK_LOG_DEBUG, "USBKEY_EnumFile ==> No Record Found");
            return USBKEY_OK;
        }
        if (len > 1)
            *pulSize = *pulSize - len + 1;
    }

    AK_Log(AK_LOG_DEBUG, "USBKEY_EnumFile Success!");
    AK_UnlockMutex(gpMutex);
    return USBKEY_OK;
}

//	获取文件信息.		ADD by zhoushenshen 20091120
//	文件信息 格式：文件ID(2字节)+读权限(1字节)+写权限(1字节)+大小(4字节)+文件名
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_GetFileAttribute(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcFileName,	//[in]文件名称
        USBKEY_UCHAR_PTR pcFileInfo)	//[out]文件信息
{
    USBKEY_ULONG rtn,ulfileLen;
    USBKEY_UCHAR fileInfo[USBKEY_FILE_RECORD_LEN];

    AK_Log(AK_LOG_DEBUG, "USBKEY_GetFileAttribute");

    /*if (strlen(pcAppName) > 16)
      {
      LogMessage("====>>USBKEY_GetFileAttribute->pcAppName Length Error!");
      return USBKEY_PARAM_ERROR;
      }*/

    /*rtn = CheckValid(pcFileName);
      if (rtn != 0)
      {
      LogMessage("====>>USBKEY_GetFileAttribute->File Name Error!");
      return rtn;
      }*/

    AK_LockMutex(gpMutex);
    /*rtn = SelectFile(hDeviceHandle, pcAppName, NULL);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage16Val("====>>USBKEY_GetFileAttribute->SelectFile() Error,rtn = 0x",rtn);
      return rtn;
      }*/

    /////////检查权限///////////////////////////////////////////////////
    /*rtn = CheckPrivillege(hDeviceHandle);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage16Val("====>>USBKEY_GetFileAttribute->CheckPrivillege() Error,rtn = 0x",rtn);
      return rtn;
      }*/
    //////////////////////////////////////////////////////////////////////////

    rtn = GetFileInfo(hDeviceHandle,pcFileName,fileInfo);
    AK_UnlockMutex(gpMutex);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_GetFileAttribute ==> GetFileInfo() Error,rtn = %#08x", rtn);
        return rtn;
    }

    memset(pcFileInfo, 0, sizeof(PUSBKEY_FILEATTRIBUTE));
    //获得文件权限
    switch(fileInfo[2])
    {
        case USBKEY_USER_USE_PRIV:
            ((PUSBKEY_FILEATTRIBUTE)pcFileInfo)->ReadRights  = USBKEY_USERTYPE_USER;
            break;		
        case USBKEY_ADMIN_USE_PRIV:
            ((PUSBKEY_FILEATTRIBUTE)pcFileInfo)->ReadRights = USBKEY_USERTYPE_ADMIN;
            break;
        case USBKEY_ALL_PRIV:
            ((PUSBKEY_FILEATTRIBUTE)pcFileInfo)->ReadRights  = USBKEY_ALL_PRIVILEGE;
            break;
        case USBKEY_NOT_USE:
            ((PUSBKEY_FILEATTRIBUTE)pcFileInfo)->ReadRights  = USBKEY_NEVER_USE;
            break;

        default:
            AK_Log(AK_LOG_ERROR, "USBKEY_GetFileAttribute ==> FileInfo ReadRights type Error,ReadRights = %#08x", fileInfo[2]);
            return USBKEY_GENERAL_ERROR;
    }
    switch(fileInfo[3])
    {
        case USBKEY_USER_USE_PRIV:
            ((PUSBKEY_FILEATTRIBUTE)pcFileInfo)->WriteRights  = USBKEY_USERTYPE_USER;
            break;		
        case USBKEY_ADMIN_USE_PRIV:
            ((PUSBKEY_FILEATTRIBUTE)pcFileInfo)->WriteRights = USBKEY_USERTYPE_ADMIN;
            break;
        case USBKEY_ALL_PRIV:
            ((PUSBKEY_FILEATTRIBUTE)pcFileInfo)->WriteRights  = USBKEY_ALL_PRIVILEGE;
            break;
        case USBKEY_NOT_USE:
            ((PUSBKEY_FILEATTRIBUTE)pcFileInfo)->WriteRights  = USBKEY_NEVER_USE;
            break;

        default:
            AK_Log(AK_LOG_ERROR, "USBKEY_GetFileAttribute ==> FileInfo WriteRights type Error,WriteRights = %#08x", fileInfo[3]);
            return USBKEY_GENERAL_ERROR;
    }		
    //获得文件大小
    ulfileLen = fileInfo[4];
    ulfileLen = ulfileLen*256 + fileInfo[5];
    ulfileLen = ulfileLen*256 + fileInfo[6];
    ulfileLen = ulfileLen*256 + fileInfo[7];
    ((PUSBKEY_FILEATTRIBUTE)pcFileInfo)->FileSize = ulfileLen;

    //获得文件名
    strcpy( ((PUSBKEY_FILEATTRIBUTE)pcFileInfo)->FileName,(char *)fileInfo+8);

    AK_Log(AK_LOG_DEBUG, "USBKEY_GetFileAttribute Success!");
    return USBKEY_OK;
}

//	读取文件.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ReadFile(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcFileName,	//[in]文件名称
        USBKEY_ULONG ulOffset,			//[in]读取偏移量
        USBKEY_UCHAR_PTR pcOutData,		//[out]返回数据
        USBKEY_ULONG_PTR pulSize)		//[in,out]输入读取数据缓冲区大小，输出实际读取长度
{
    //USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[USBKEY_FILE_MAX_BLOCK_LEN+8];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0,loop = 0,offset,len,i;
    //USBKEY_UCHAR p1,p2,lc,le;
    USBKEY_UCHAR p1,p2,lc;
    USBKEY_INT32 le;
    USBKEY_UCHAR data[300],fileInfo[USBKEY_FILE_RECORD_LEN];

    AK_Log(AK_LOG_DEBUG, "USBKEY_ReadFile Success!");

    /*if (strlen(pcAppName) > 16)
      {
      LogMessage("====>>USBKEY_ReadFile->pcAppName Length Error!");
      return USBKEY_PARAM_ERROR;
      }*/
    //COS只接受2字节偏移量
    if(ulOffset >= 128*256)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ReadFile ==> ulOffset too long,ulOffset = %#08x", ulOffset);
        return USBKEY_PARAM_ERROR;
    }	
    /*rtn = CheckValid(pcFileName);
      if (rtn != 0)
      {
      LogMessage("====>>USBKEY_ReadFile->File Name Error!");
      return rtn;
      }*/

    AK_LockMutex(gpMutex);
    /*rtn = SelectFile(hDeviceHandle, pcAppName, NULL);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage16Val("====>>USBKEY_ReadFile->SelectFile() Error,rtn = 0x",rtn);
      return rtn;
      }*/

    /////////检查权限///////////////////////////////////////////////////
    // 	rtn = CheckPrivillege(hDeviceHandle);
    // 	if (rtn != 0)
    // 	{
    // 		LogMessage16Val("====>>USBKEY_ReadFile->CheckPrivillege() Error,rtn = 0x",rtn);
    // 		return rtn;
    // 	}
    //////////////////////////////////////////////////////////////////////////

    rtn = GetFileInfo(hDeviceHandle,pcFileName,fileInfo);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ReadFile ==> GetFileInfo() Error,rtn = %#08x", rtn);
        return rtn;
    }
    //ADD by zhoushenshen 20091120
    len = fileInfo[4];
    len = len*256 + fileInfo[5];
    len = len*256 + fileInfo[6];
    len = len*256 + fileInfo[7];
    if (ulOffset + (*pulSize) > len)
        *pulSize = len -ulOffset;
    /*if (ulOffset + (*pulSize) > len)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage16Val("====>>USBKEY_ReadFile->Write size is overflow! filelength = 0x",len);
      return USBKEY_GENERAL_ERROR;
      }*/
    /*FileID[0] = fileInfo[0];
      FileID[1] = fileInfo[1];*/
    rtn = SelectFile(hDeviceHandle, NULL, fileInfo);		//ADD by zhoushenshen 20091120, Update by zhoushenshen 20100309
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ReadFile ==> SelectFile() fileInfo FileID Error,rtn = %#08x", rtn);
        return rtn;
    }

    if ( (*pulSize) > USBKEY_FILE_MAX_BLOCK_LEN)
    {
        loop = (*pulSize)/USBKEY_FILE_MAX_BLOCK_LEN;
    }
    offset = ulOffset;
    for (i=0; i<loop; i++)
    {
        len = offset/256;
        memcpy(&p2,&offset,1);
        memcpy(&p1,&len,1);
        lc = 0;

        //Delete by zhoushenshen in 20121109
        /*le = 0xff;
          data[0] = USBKEY_FILE_MAX_BLOCK_LEN/256;
          data[1] = (USBKEY_UCHAR)USBKEY_FILE_MAX_BLOCK_LEN;

          rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_ReadBinary,request,&reqLen);
          if (rtn != 0)
          {
          AK_UnlockMutex(gpMutex);
          AK_Log(AK_LOG_ERROR, "USBKEY_ReadFile ==> PackageData() loop Error,rtn = %#08x", rtn);
          return rtn;
          }

          respLen = USBKEY_FILE_MAX_BLOCK_LEN+2;
          rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
          if (rtn != 0)
          {
          AK_UnlockMutex(gpMutex);
          AK_Log(AK_LOG_ERROR, "USBKEY_ReadFile ==> DeviceExecCmd() loop Error,rtn = %#08x", rtn);
          return rtn;
          }*/
        //Del end

        //Add by zhoushenshen in 20121109
        le = USBKEY_FILE_MAX_BLOCK_LEN;

        respLen = USBKEY_FILE_MAX_BLOCK_LEN+2;
        rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_ReadBinary,response,&respLen);
        if (rtn != 0)
        {
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_ReadFile ==> PackageData() loop Error, rtn = %#08x", rtn);
            return rtn;
        }
        //Add end

        rtn = *(response+respLen-2);
        rtn = rtn*256 + *(response+respLen-1);
        switch(rtn)
        {
            case DEVICE_SUCCESS:
                break;
            default:
                AK_UnlockMutex(gpMutex);
                AK_Log(AK_LOG_ERROR, "USBKEY_ReadFile ==> DeviceExecCmd() loop Error,code = %#08x", rtn);
                return rtn;
                break;
        }

        memcpy(pcOutData+i*USBKEY_FILE_MAX_BLOCK_LEN,response,respLen-2);
        offset+=USBKEY_FILE_MAX_BLOCK_LEN;
    }

    len = offset/256;
    memcpy(&p2,&offset,1);
    memcpy(&p1,&len,1);
    lc = 0;
    len = (*pulSize) - loop*USBKEY_FILE_MAX_BLOCK_LEN;
    if(0 == len)
    {
        AK_UnlockMutex(gpMutex);
        *pulSize = offset - ulOffset;
        AK_Log(AK_LOG_DEBUG, "USBKEY_ReadFile Success!");
        return USBKEY_OK;
    }
    //Delete by zhoushenshen in 20121109
    /*else if(len > 254)
      {
      le = 0xff;
      data[0] = (USBKEY_UCHAR)(len/256);
      data[1] = (USBKEY_UCHAR)len;
      }
      else
      le = (USBKEY_UCHAR)len;

      rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_ReadBinary,request,&reqLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_ReadFile ==> PackageData() Error,rtn = %#08x", rtn);
      return rtn;
      }

      respLen = len+2;
      rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_ReadFile ==> DeviceExecCmd() Error,rtn = %#08x", rtn);
      return rtn;
      }*/
    //Del end

    //Add by zhoushenshen in 20121109
    le = len;

    respLen = len+2;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_ReadBinary,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ReadFile ==> PackageData() Error, rtn = %#08x", rtn);
        return rtn;
    }
    //Add end

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_ReadFile ==> DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
            break;
    }

    memcpy(pcOutData+loop*USBKEY_FILE_MAX_BLOCK_LEN,response,respLen-2);
    offset+=len;

    *pulSize = offset - ulOffset;

    AK_Log(AK_LOG_DEBUG, "USBKEY_ReadFile Success!");
    AK_UnlockMutex(gpMutex);
    return USBKEY_OK;
}

//	写文件.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_WriteFile(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcFileName,	//[in]文件名称
        USBKEY_ULONG ulOffset,			//[in]写入偏移量
        USBKEY_UCHAR_PTR pcData,		//[in]写入数据
        USBKEY_ULONG_PTR pulSize)		//[in,out]输入写入数据缓冲区大小，输出实际写入大小
{
    //USBKEY_UCHAR request[USBKEY_FILE_MAX_BLOCK_LEN+8];
    USBKEY_UCHAR response[300];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0,loop = 0,offset,len,i;
    //USBKEY_UCHAR p1,p2,lc,le;
    USBKEY_UCHAR p1,p2,le;
    USBKEY_INT32 lc;
    USBKEY_UCHAR data[USBKEY_FILE_MAX_BLOCK_LEN+8],fileInfo[USBKEY_FILE_RECORD_LEN];

    AK_Log(AK_LOG_DEBUG, "USBKEY_WriteFile");

    /*if (strlen(pcAppName) > 16)
      {
      LogMessage("====>>USBKEY_WriteFile->pcAppName Length Error!");
      return USBKEY_PARAM_ERROR;
      }*/

    /*rtn = CheckValid(pcFileName);
      if (rtn != 0)
      {
      LogMessage("====>>USBKEY_WriteFile->File Name Error!");
      return rtn;
      }*/

    //COS只接受2字节偏移量
    if(ulOffset + *pulSize >= 128*256)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_WriteFile ==> ulOffset too long,ulOffset = %#08x", ulOffset);
        return USBKEY_PARAM_ERROR;
    }	

    /*if (*pulSize > 2048)
      {
      LogMessageVal("====>>USBKEY_WriteFile->pulSize Error! pulSize = ",*pulSize);
      return USBKEY_PARAM_ERROR;
      }*/

    AK_LockMutex(gpMutex);

    /*rtn = SelectFile(hDeviceHandle, pcAppName, NULL);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage16Val("====>>USBKEY_WriteFile->SelectFile() Error,rtn = 0x",rtn);
      return rtn;
      }*/

    /////////检查权限///////////////////////////////////////////////////
    /*rtn = CheckPrivillege(hDeviceHandle);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage16Val("====>>USBKEY_WriteFile->CheckPrivillege() Error,rtn = 0x",rtn);
      return rtn;
      }*/
    //////////////////////////////////////////////////////////////////////////
    rtn = GetFileInfo(hDeviceHandle,pcFileName,fileInfo);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_WriteFile ==> GetFileInfo() Error,rtn = %#08x", rtn);
        return rtn;
    }
    //ADD by zhoushenshen 20091120
    len = fileInfo[4];
    len = len*256 + fileInfo[5];
    len = len*256 + fileInfo[6];
    len = len*256 + fileInfo[7];
    if (ulOffset + (*pulSize) > len)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_WriteFile ==> Write size is overflow! filelength = %#08x", len);
        return USBKEY_BUFFER_TOO_SHORT;
    }

    rtn = SelectFile(hDeviceHandle, NULL, fileInfo);	//Update by zhoushenshen 20100309
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_WriteFile ==> SelectFile() fileInfo FileID Error,rtn = %#08x", rtn);
        return rtn;
    }


    //写入数据------ edit by zhoushenshen 20091120
    if ( (*pulSize) > USBKEY_FILE_MAX_BLOCK_LEN)
    {
        loop = (*pulSize)/USBKEY_FILE_MAX_BLOCK_LEN;
    }
    offset = ulOffset;
    for (i=0; i<loop; i++)
    {
        len = offset/256;
        memcpy(&p2,&offset,1);
        memcpy(&p1,&len,1);
        //Delete by zhoushenshen in 20121109
        /*lc = 0xff;
          data[0] = USBKEY_FILE_MAX_BLOCK_LEN/256;
          data[1] = (USBKEY_UCHAR)USBKEY_FILE_MAX_BLOCK_LEN;
          le = 0x00;
          memcpy(data+2,pcData+i*USBKEY_FILE_MAX_BLOCK_LEN,USBKEY_FILE_MAX_BLOCK_LEN);

          rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_UpdateBinary,request,&reqLen);
          if (rtn != 0)
          {
          AK_Log(AK_LOG_ERROR, "USBKEY_WriteFile ==> PackageData() loop Error,rtn = %#08x", rtn);
          return rtn;
          }

          respLen = 300;
          rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
          if (rtn != 0)
          {
          AK_Log(AK_LOG_ERROR, "USBKEY_WriteFile ==> DeviceExecCmd() loop Error,rtn = %#08x", rtn);
          return rtn;
          }*/
        //Del end

        //Add by zhoushenshen in 20121109
        lc = USBKEY_FILE_MAX_BLOCK_LEN;
        le = 0x00;
        memcpy(data,pcData+i*USBKEY_FILE_MAX_BLOCK_LEN,USBKEY_FILE_MAX_BLOCK_LEN);

        respLen = 300;
        rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_UpdateBinary,response,&respLen);
        if (rtn != 0)
        {
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_WriteFile ==> PackageData() loop Error, rtn = %#08x", rtn);
            return rtn;
        }
        //Add end

        rtn = *(response+respLen-2);
        rtn = rtn*256 + *(response+respLen-1);
        switch(rtn)
        {
            case DEVICE_SUCCESS:
                break;
            default:
                AK_Log(AK_LOG_ERROR, "USBKEY_WriteFile ==> DeviceExecCmd() loop Error,code = %#08x", rtn);
                return rtn;
                break;
        }
        offset+=USBKEY_FILE_MAX_BLOCK_LEN;
    }

    len = offset/256;
    memcpy(&p2,&offset,1);
    memcpy(&p1,&len,1);
    len = (*pulSize) - loop*USBKEY_FILE_MAX_BLOCK_LEN;
    if(0 == len)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_DEBUG, "USBKEY_WriteFile Success!");
        return USBKEY_OK;
    }
    //Delete by zhoushenshen in 20121109
    /*else if(len > 254)
      {
      lc = 0xff;
      data[0] = (USBKEY_UCHAR)(len/256);
      data[1] = (USBKEY_UCHAR)len;
      memcpy(data+2,pcData+loop*USBKEY_FILE_MAX_BLOCK_LEN,len);
      }
      else
      {
      lc = (USBKEY_UCHAR)len;
      memcpy(data,pcData+loop*USBKEY_FILE_MAX_BLOCK_LEN,len);
      }
      le = 0x00;

      rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_UpdateBinary,request,&reqLen);
      if (rtn != 0)
      {
      AK_Log(AK_LOG_ERROR, "USBKEY_WriteFile ==> PackageData() Error,rtn = %#08x", rtn);
      return rtn;
      }*/

    /*//一次写入所有数据------edit by heli 20090619
      len = ulOffset/256;
      memcpy(&p2,&ulOffset,1);
      memcpy(&p1,&len,1);
      le = 0x00;
      if ( (*pulSize) > 254)
      {
      lc = 0xff;
      data[0] = *pulSize/256;
      data[1] = *pulSize%256;
      memcpy(data+2,pcData,*pulSize);
      }
      else
      {
      lc = *pulSize;
      memcpy(data,pcData,*pulSize);
      }

    //组织数据包
    rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_UpdateBinary,request,&reqLen);
    if (rtn != 0)
    {
    AK_UnlockMutex(gpMutex);
    LogMessage16Val("====>>USBKEY_WriteFile->PackageData() Error,rtn = 0x",rtn);
    return rtn;
    }
    //end------edit by heli 20090619*/

    //Delete by zhoushenshen in 20121109
    /*respLen = 300;
      rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_WriteFile ==> DeviceExecCmd() Error,rtn = %#08x", rtn);
      return rtn;
      }*/
    //Del end

    //Add by zhoushenshen in 20121109
    lc = len;
    le = 0x00;
    memcpy(data,pcData+loop*USBKEY_FILE_MAX_BLOCK_LEN,len);

    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_UpdateBinary,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_WriteFile ==> PackageData() Error, rtn = %#08x", rtn);
        return rtn;
    }
    //Add end

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_WriteFile ==> DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
            break;
    }

    //offset+=len;
    //*pulSize = offset - ulOffset;

    AK_Log(AK_LOG_DEBUG, "USBKEY_WriteFile Success!");
    AK_UnlockMutex(gpMutex);
    return USBKEY_OK;
}

///////////////////////////////////////////////////////////////////////////////
//	算法服务相关API
///////////////////////////////////////////////////////////////////////////////
//	权限控制：证书、公钥读权限为USBKEY_ALL_PRIVILEGE；写权限为：USBKEY_USERTYPE_USER(生成公钥时写权限由ulKeyType指定)
//	私钥、对称密钥的读写权限均由ulKeyType指定。导入私钥时其读写权限均为：USBKEY_USERTYPE_USER
//	容器的读写权限均为：USBKEY_ALL_PRIVILEGE
//////////////////////////////////////////////////////////////////////////////
//	创建密钥容器.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_CreateContainer(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcContainerName)		//[in]送入的密钥容器名称
{
    USBKEY_UCHAR response[300];
    USBKEY_ULONG rtn,respLen=0;
    USBKEY_ULONG ulAccessCondition = 0,recNO;
    USBKEY_UCHAR p1,p2,lc,le;
    USBKEY_UCHAR data[300],pcFileID[2];

    AK_Log(AK_LOG_DEBUG, "USBKEY_CreateContainer");

    if (strlen((char *)pcContainerName) > 254)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_CreateContainer ==> pcContainerName Length Error,Max Name Len is 255!");
        return USBKEY_PARAM_ERROR;
    }

    AK_LockMutex(gpMutex);

    rtn = SearchContainerRecNo(hDeviceHandle,pcContainerName,&recNO);
    if(rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_CreateContainer ==> SearchContainerRecNo() Error,rtn = %#08x", rtn);
        return rtn;
    }
    if ( recNO != 0 )
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_CreateContainer ==> SearchContainerRecNo() pcContainerName Exist!");
        return USBKEY_CONTAINER_NAME_EXIST;
    }

    pcFileID[0] = 0x80;
    pcFileID[1] = 0x02;
    rtn = SelectFile(hDeviceHandle,NULL,pcFileID);	//Update by zhoushenshen 20100309
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_CreateContainer ==> SelectFile() pcFileID Error,rtn = %#08x", rtn);
        return rtn;
    }

    rtn = GetConRecNo(hDeviceHandle,pcFileID,&recNO);	//Update by zhoushenshen 20100309
    if( (rtn != 0) || (recNO == 0) )
    {
        AK_UnlockMutex(gpMutex);
        if(recNO == 0)
        {
            AK_Log(AK_LOG_ERROR, "USBKEY_CreateContainer ==> Reach the max number of containers.");
            return USBKEY_MAX_NUM_OF_CONTAINER_ERROR;
        }
        AK_Log(AK_LOG_ERROR, "USBKEY_CreateContainer ==> GetConRecNo() Error,rtn = %#08x", rtn);
        return rtn;
    }

    p1 = 0x00;
    p2 = 0x03;
    lc = 0x07;
    le = 0x00;

    ////////
    //创建文件
    ////////
    pcFileID[0] = 0xef;
    pcFileID[1] = (USBKEY_UCHAR)recNO;
    memset(data,0,300);
    memcpy(data,pcFileID,2);
    le = 0x00;
    memcpy(data+2,&le,1);
    le = USBKEY_ALL_PRIV;
    memcpy(data+3,&le,1);
    le = USBKEY_ALL_PRIV;
    memcpy(data+4,&le,1);
    le = 0x10;
    memcpy(data+5,&le,1);
    le = 0x06;
    memcpy(data+6,&le,1);

    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_CreateFile,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_CreateContainer ==> PackageData() Error, rtn = %#08x", rtn);
        return rtn;
    }
    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_CreateContainer ==> DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
            break;
    }

    rtn = SelectFile(hDeviceHandle,NULL,pcFileID);	//Update by zhoushenshen 20100309
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_CreateContainer ==> SelectFile() pcFileID Error,rtn = %#08x", rtn);
        return rtn;
    }
    memset(data,0xff,sizeof(data));
    for( recNO = 1; recNO <= 6; recNO++ )
    {
        rtn = UpdateRecord(hDeviceHandle,pcFileID,recNO, data,0x10);	//Update by zhoushenshen 20100309
        if (rtn != 0)
        {
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_CreateContainer ==> UpdateRecord() Error,rtn = %#08x", rtn);
            return rtn;
        }
    }
 
    //添加默认记录至8002
    rtn = AddContainerRecord(hDeviceHandle,pcContainerName);	//Update by zhoushenshen 20100309
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_CreateContainer ==> AppendRecord() Error,rtn = %#08x", rtn);
        return rtn;
    }

    AK_Log(AK_LOG_DEBUG, "USBKEY_CreateContainer Success!");
    AK_UnlockMutex(gpMutex);
    return USBKEY_OK;
}

//	删除密钥容器.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DeleteContainer(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcContainerName)	//[in]密钥容器名称
{
    //USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0,recNO;
    USBKEY_UCHAR p1,p2,lc,le;
    USBKEY_UCHAR data[16],fileID[2];

    AK_Log(AK_LOG_DEBUG, "USBKEY_DeleteContainer");

    if (strlen((char *)pcContainerName) > 254)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_DeleteContainer ==> pcContainerName Length Error,Max Name Len is 254!");
        return USBKEY_PARAM_ERROR;
    }

    AK_LockMutex(gpMutex);

    //////////////////////////////////////////////////////////////////////////

    rtn = SearchContainerRecNo(hDeviceHandle,pcContainerName,&recNO);
    if( (rtn != 0) || (recNO == 0) )
    {
        AK_UnlockMutex(gpMutex);
        if(recNO == 0)
        {
            AK_Log(AK_LOG_ERROR, "USBKEY_DeleteContainer ==> not find container record.");
            return USBKEY_RECORD_NOT_FOUND;
        }
        AK_Log(AK_LOG_ERROR, "USBKEY_DeleteContainer ==> SearchContainerRecNo() Error,rtn = %#08x", rtn);
        return rtn;
    }

    //删除密钥容器内的文件
    fileID[0] = 0xef;
    fileID[1] = (USBKEY_UCHAR)recNO;
    rtn = ClearContainer(hDeviceHandle,fileID);		//Update by zhoushenshen 20100309
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_DeleteContainer ==> ClearContainer() Error,rtn = %#08x", rtn);
        return rtn;
    }

    //删除容器对称密钥
    rtn = DeleteContainerSymmetricKey(hDeviceHandle,recNO);	//Update by zhoushenshen 20100309；update by heli 20100607
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_DeleteContainer ==> DeleteContainerSymmetricKey() Error,rtn = %#08x", rtn);
        return rtn;
    }

    //删除密钥容器
    p1 = 0x00;
    p2 = 0x00;
    lc = 2;
    le = 0x00;

    ////生成文件标识
    //memcpy(data+2,ulAccessCondition,1);
    memcpy(data,fileID,2);

    //Delete by zhoushenshen in 20121109
    /*rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_DeleteFile,request,&reqLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_DeleteContainer ==> PackageData() Error,rtn = %#08x", rtn);
      return rtn;
      }

      respLen = 300;
      rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_DeleteContainer ==> DeviceExecCmd() Error,rtn = %#08x", rtn);
      return rtn;
      }*/
    //Del end

    //Add by zhoushenshen in 20121109
    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_DeleteFile,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_DeleteContainer ==> PackageData() Error, rtn = %#08x", rtn);
        return rtn;
    }
    //Add end

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_DeleteContainer ==> DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
            break;
    }
    /*
       p1 = 0x04;
       p2 = 0x00;
       lc = strlen(pcContainerName);
       le = 0x00;
       data = malloc(lc);

    ////生成文件标识
    //memcpy(data+2,ulAccessCondition,1);
    memcpy(data,pcContainerName,lc);
    if (lc < 5)
    {
    memset(data+lc,0x01,5-lc);
    lc = 0x05;
    }

    rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_DeleteFile,request,&reqLen);
    if (rtn != 0)
    {
    LogMessageVal("====>>USBKEY_DeleteContainer->PackageData() Error,rtn = ",rtn);
    free(data);
    return rtn;
    }

    respLen = 300;
    rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,&respLen);
    if (rtn != 0)
    {
    LogMessageVal("====>>USBKEY_DeleteContainer->DeviceExecCmd() Error,rtn = ",rtn);
    free(data);
    return rtn;
    }

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
    case DEVICE_SUCCESS:
    break;
    default:
    LogMessage16Val("====>>USBKEY_DeleteContainer->DeviceExecCmd() Error,code = 0x",rtn);
    return rtn;
    break;
    }

    free(data);
    //*/
    rtn = DeleContainerRecord(hDeviceHandle,pcContainerName);	//Update by zhoushenshen 20100309
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_DeleteContainer ==> DeleContainerRecord() Error,rtn = %#08x", rtn);
        return rtn;
    }

    AK_Log(AK_LOG_DEBUG, "USBKEY_DeleteContainer Success!");
    AK_UnlockMutex(gpMutex);
    return USBKEY_OK;
}


//	判断是否为空容器.
//	返回值：
//		0:	为空
//		-1:不为空
//		>0:错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_JudgeEmptyContainer(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcContainerName)	//[in]密钥容器名称
{
    USBKEY_ULONG rtn,recNO;
    USBKEY_UCHAR data[16],fileID[2];
    USBKEY_ULONG reclen;
    USBKEY_UCHAR record[16];

    AK_Log(AK_LOG_DEBUG, "USBKEY_JudgeEmptyContainer");

    if (strlen((char *)pcContainerName) > 254)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_JudgeEmptyContainer ==> pcContainerName Length Error,Max Name Len is 254!");
        return USBKEY_PARAM_ERROR;
    }

    AK_LockMutex(gpMutex);

    rtn = SearchContainerRecNo(hDeviceHandle,pcContainerName,&recNO);
    if( (rtn != 0) || (recNO == 0) )
    {
        AK_UnlockMutex(gpMutex);
        if(recNO == 0)
        {
            AK_Log(AK_LOG_ERROR, "USBKEY_JudgeEmptyContainer ==> not find container record.");
            return USBKEY_RECORD_NOT_FOUND;
        }
        AK_Log(AK_LOG_ERROR, "USBKEY_JudgeEmptyContainer ==> SearchContainerRecNo() Error,rtn = %#08x", rtn);
        return rtn;
    }

    //删除密钥容器内的文件
    fileID[0] = 0xef;
    fileID[1] = (USBKEY_UCHAR)recNO;

    memset(record,0xff,sizeof(record));
    for( recNO = 1; recNO <= 6; recNO++ )
    {
        rtn = SelectFile(hDeviceHandle,NULL,fileID);		
        if (rtn != 0)
        {
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_JudgeEmptyContainer ==> SelectFile() Error,rtn = %#08x", rtn);
            return rtn;
        }

        reclen = 16;
        rtn = ReadRecord(hDeviceHandle,fileID,recNO,data,&reclen);
        if (rtn != 0)
        {
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_JudgeEmptyContainer ==> ReadRecord() Error,rtn = %#08x", rtn);
            return rtn;
        }

        if ( (reclen >= 2) && (memcmp(data,record,2) != 0) )
            return -1;		//容器不为空
    }

    AK_UnlockMutex(gpMutex);
    return USBKEY_OK;	
}


//	枚举密钥容器.
//	如果pcContianerNameList为NULL，则从pulSize返回需要分配char *[NAME_ID_MAX_LEN]数量
//	如果pcContianerNameList非空，则从pcContianerNameList返回密钥容器列表，ID间以'\0'分隔，
//		并从pulSize返回实际密钥容器个数
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_EnumContainer(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcContianerNameList,		//[out]密钥容器名称列表
        USBKEY_ULONG_PTR pulSize)	//[in,out]输入为缓冲区大小，输出为实际个数
{
    USBKEY_ULONG recordLen,recNO;
    USBKEY_ULONG rtn = 0;
    USBKEY_ULONG len;
    USBKEY_UCHAR record[300],deleRecord[300],fileName[2];;

    AK_Log(AK_LOG_DEBUG, "USBKEY_EnumContainer");

    /*if (strlen(pcAppName) > 16)
      {
      LogMessage("====>>USBKEY_EnumContainer->pcAppName Length Error!");
      return USBKEY_PARAM_ERROR;
      }*/
    recNO = 0;
    memset(deleRecord,0xff,sizeof(deleRecord));
    memset(record,0,sizeof(record));
    fileName[0] = 0x80;
    fileName[1] = 0x02;

    AK_LockMutex(gpMutex);

    rtn = SelectFile(hDeviceHandle, NULL, fileName);	//Update by zhoushenshen 20100309
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_EnumContainer ==> SelectFile() Error,rtn = %#08x", rtn);
        return rtn;
    }

    /////////检查权限///////////////////////////////////////////////////
    // 	rtn = CheckPrivillege(hDeviceHandle);
    // 	if (rtn != 0)
    // 	{
    // 		LogMessage16Val("====>>USBKEY_EnumContainer->CheckPrivillege() Error,rtn = 0x",rtn);
    // 		return rtn;
    // 	}
    //////////////////////////////////////////////////////////////////////////

    if (pcContianerNameList == NULL)
    {
        *pulSize = 0;
        for (recNO = 1; recNO <= CONTAINER_MAX_QUANTITY; recNO ++)
        {
            recordLen = 254;
            rtn = ReadRecord(hDeviceHandle,fileName,recNO,record,&recordLen);	//Update by zhoushenshen 20100309
            if (rtn != 0)
            {
                AK_UnlockMutex(gpMutex);
                AK_Log(AK_LOG_ERROR, "USBKEY_EnumContainer ==> ReadRecord() long Error,rtn = %#08x", rtn);
                return rtn;
            }
            if (memcmp(deleRecord,record,recordLen) != 0)
            {
                *pulSize = *pulSize + strlen((char *)record) + 1;
            }
        }
        if ((*pulSize) == 0)
        {
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_DEBUG, "USBKEY_EnumContainer ==> No Record Found");
            return USBKEY_OK;
        }
        *pulSize = *pulSize +1;
    }
    else
    {
        len = *pulSize;
        memset(pcContianerNameList,0,len);
        for (recNO = 1; recNO <= CONTAINER_MAX_QUANTITY; recNO ++)
        {
            recordLen = 254;
            rtn = ReadRecord(hDeviceHandle,fileName,recNO,record,&recordLen);	//Update by zhoushenshen 20100309
            if (rtn != 0)
            {
                AK_UnlockMutex(gpMutex);
                AK_Log(AK_LOG_ERROR, "USBKEY_EnumContainer ==> ReadRecord() Error,rtn = %#08x", rtn);
                return rtn;
            }
            if (memcmp(deleRecord,record,recordLen) != 0)
            {
                if (len < (strlen((char *)record) + 2) )
                {
                    AK_UnlockMutex(gpMutex);
                    AK_Log(AK_LOG_ERROR, "USBKEY_EnumContainer ==> pcContianerNameList buffer is too short!,rtn = %#08x", USBKEY_BUFFER_TOO_SHORT);
                    return USBKEY_BUFFER_TOO_SHORT;
                }
                memcpy(pcContianerNameList + *pulSize - len,record,strlen((char *)record));
                len = len - strlen((char *)record) - 1;
            }
        }
        if ((*pulSize) == len)
        {
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_DEBUG, "USBKEY_EnumContainer ==> No Record Found");
            return USBKEY_OK;
        }
        if (len > 1)
            *pulSize = *pulSize - len + 1;
    }


    AK_Log(AK_LOG_DEBUG, "USBKEY_EnumContainer Success!");
    AK_UnlockMutex(gpMutex);

    return USBKEY_OK;
}

//	产生随机数.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_GenRandom(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcRandom,		//[out]返回的随机数
        USBKEY_ULONG ulRandomLen)		//[in]需要的随机数长度
{
    //USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0;
    //USBKEY_UCHAR data[24];
    USBKEY_UCHAR data[USBKEY_RANDOM_MAX_BLCOK_LEN];
    USBKEY_UCHAR p1,p2,lc,le;
    USBKEY_INT32 i,loop = 0;

    AK_LockMutex(gpMutex);
    AK_Log(AK_LOG_DEBUG, "USBKEY_GenRandom");

    //loop = ulRandomLen/24;
    loop = ulRandomLen/USBKEY_RANDOM_MAX_BLCOK_LEN;

    p1 = 0x00;
    p2 = 0x00;
    lc = 0x00;
    //le = 0x18;
    le = USBKEY_RANDOM_MAX_BLCOK_LEN;

    /*rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_GetChallenge,request,&reqLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_GenRandom ==> PackageData() Error,rtn = %#08x", rtn);
      return rtn;
      }*/

    for (i = 0 ; i < loop ; i ++)
    {
        respLen = 300;

        //Delete by zhoushenshen in 20121109
        /*rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
          if (rtn != 0)
          {
          AK_UnlockMutex(gpMutex);
          AK_Log(AK_LOG_ERROR, "USBKEY_GenRandom ==> DeviceExecCmd() loop Error,rtn = %#08x", rtn);
          return rtn;
          }*/
        //Del end

        //Add by zhoushenshen in 20121109
        rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_GetChallenge,response,&respLen);
        if (rtn != 0)
        {
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_GenRandom ==> PackageData() loop Error, rtn = %#08x", rtn);
            return rtn;
        }
        //Add end

        rtn = *(response+respLen-2);
        rtn = rtn*256 + *(response+respLen-1);
        switch(rtn)
        {
            case DEVICE_SUCCESS:
                break;
            default:
                AK_UnlockMutex(gpMutex);
                AK_Log(AK_LOG_ERROR, "USBKEY_GenRandom ==> DeviceExecCmd() loop Error,code = %#08x", rtn);
                return rtn;
                break;
        }
        memcpy(pcRandom+i*USBKEY_RANDOM_MAX_BLCOK_LEN,response,USBKEY_RANDOM_MAX_BLCOK_LEN);
    }
    respLen = 300;

    //Delete by zhoushenshen in 20121109
    /*rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_GenRandom ==> DeviceExecCmd() Error,rtn = %#08x", rtn);
      return rtn;
      }*/
    //Del end

    //Add by zhoushenshen in 20121109
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_GetChallenge,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_GenRandom ==> PackageData() Error, rtn = %#08x", rtn);
        return rtn;
    }
    //Add end

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_GenRandom ==> DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
            break;
    }
    memcpy(pcRandom+i*USBKEY_RANDOM_MAX_BLCOK_LEN,response,ulRandomLen-i*USBKEY_RANDOM_MAX_BLCOK_LEN);

    AK_Log(AK_LOG_DEBUG, "USBKEY_GenRandom Success!");
    AK_UnlockMutex(gpMutex);
    return USBKEY_OK;
}

//	产生对称密钥.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_GenSymmetricKey(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_ULONG ulKeyLen,			//[in]密钥长度
        USBKEY_ULONG ulAlgID,			//[in]密钥算法标示
        USBKEY_ULONG ulGenKeyFlags,		//[in]生成密钥的控制标志
        USBKEY_UCHAR_PTR phKey)			//[out]返回的密钥句柄
{
    //USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0;
    USBKEY_UCHAR data[24];
    USBKEY_UCHAR p1,p2,lc,le;

    AK_Log(AK_LOG_DEBUG, "USBKEY_GenSymmetricKey");

    /*if (strlen(pcAppName) > 16)
      {
      LogMessage("====>>USBKEY_GenSymmetricKey->pcAppName Length Error!");
      return USBKEY_PARAM_ERROR;
      }*/

    AK_LockMutex(gpMutex);

    /*pcFileID[0] = 0x80;
      pcFileID[1] = 0x03;
      rtn = SelectFile(hDeviceHandle,pcAppName,pcFileID);
      if (rtn != 0)
      {
      LogMessageVal("====>>USBKEY_GenSymmetricKey->SelectFile() Error,rtn = ",rtn);
      return rtn;
      }

      rtn = GetKeyRecNo(hDeviceHandle,pcAppName,pcFileID,&recNO);
      if( (rtn != 0) || (recNO == 0) )
      {
      LogMessageVal("====>>USBKEY_GenSymmetricKey->GetRecNo() Error,rtn = ",rtn);
      LogMessage("====>>USBKEY_GenSymmetricKey->Reach the max number of keys.");
      return USBKEY_GENERAL_ERROR;
      }*/


    /*rtn = SelectFile(hDeviceHandle,pcAppName,NULL);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage16Val("====>>USBKEY_GenSymmetricKey->SelectFile() Error,rtn = 0x",rtn);
      return rtn;
      }*/

    /////////检查权限///////////////////////////////////////////////////
    /*rtn = CheckPrivillege(hDeviceHandle);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage16Val("====>>USBKEY_GenSymmetricKey->CheckPrivillege() Error,rtn = 0x",rtn);
      return rtn;
      }*/
    //////////////////////////////////////////////////////////////////////////

    p1 = 0x00;
    p2 = 0x00;
    lc = 0x06;
    le = 0x00;

    memset(data,0,sizeof(data));
    if( (ulGenKeyFlags & USBKEY_CRYPT_ONDEVICE) != 0)
    {
        le = 0;
        //返回Key ID
        phKey[0] = 0x10;
        phKey[1] = 0;
        //memcpy(phKey,&le,1);
        memcpy(data,&le,1);
        le = 0x07;
    }
    else
    {
        le = 0;
        //返回Key ID
        phKey[0] = 0x11;
        phKey[1] = 0;
        //memcpy(phKey,&le,1);
        memcpy(data,&le,1);
        le = 0x08;
    }
    memcpy(data+1,&le,1);
    memcpy(data+2,&ulAlgID,1);
    if( (ulGenKeyFlags & USBKEY_CRYPT_EXPORTABLE) != 0)
        le = 0x00;
    else
        le = 0x01;
    memcpy(data+3,&le,1);
    if( (ulGenKeyFlags & USBKEY_CRYPT_USER_PROTECTED) != 0)
        le = USBKEY_USER_USE_PRIV;
    else
        le = USBKEY_ALL_PRIV;
    memcpy(data+4,&le,1);
    memcpy(data+5,&le,1);
    le = 0x00;

    //Delete by zhoushenshen in 20121114
    /*rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_GenSymmKey,request,&reqLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_GenSymmetricKey ==> PackageData() Error,rtn = %#08x", rtn);
      return rtn;
      }

      respLen = 300;
      rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_GenSymmetricKey ==> DeviceExecCmd() Error,rtn = %#08x", rtn);
      return rtn;
      }*/
    //Del end

    //Add by zhoushenshen in 20121114
    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_GenSymmKey,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_GenSymmetricKey ==> PackageData() Error, rtn = %#08x", rtn);
        return rtn;
    }
    //Add end

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_GenSymmetricKey ==> DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
            break;
    }

    /*rtn = AddKeyRecord(hDeviceHandle,pcAppName,phKey);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      LogMessageVal("====>>USBKEY_GenSymmetricKey->AddKeyRecord() Error,rtn = ",rtn);
      return rtn;
      }*/


    AK_Log(AK_LOG_DEBUG, "USBKEY_GenSymmetricKey Success!");
    AK_UnlockMutex(gpMutex);

    return USBKEY_OK;
}

//	产生容器对称密钥.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_GenContainerSymmetricKey(
        USBKEY_HANDLE hDeviceHandle,		//[in]设备句柄
        USBKEY_UCHAR_PTR pcContainerName,	//[in]密钥容器名称
        USBKEY_ULONG ulKeyLen,				//[in]密钥长度
        USBKEY_ULONG ulAlgID,				//[in]密钥算法标示
        USBKEY_ULONG ulGenKeyFlags,			//[in]生成密钥的控制标志
        USBKEY_UCHAR_PTR phKey)				//[out]返回的密钥ID
{

    //USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0,recNO;
    USBKEY_UCHAR data[24];
    USBKEY_UCHAR p1,p2,lc,le;

    USBKEY_UCHAR ucSymKeyID;
    USBKEY_UCHAR pcConSymKeyIDRec[USBKEY_MAX_SYM_KEY_ID];

    AK_Log(AK_LOG_DEBUG, "USBKEY_GenContainerSymmetricKey");

    AK_LockMutex(gpMutex);	

    rtn = SearchContainerRecNo(hDeviceHandle,pcContainerName,&recNO);
    if( (rtn != 0) || (recNO == 0) )
    {
        AK_UnlockMutex(gpMutex);
        if(recNO == 0)
        {
            AK_Log(AK_LOG_ERROR, "USBKEY_GenContainerSymmetricKey ==> not find container record.");
            return USBKEY_RECORD_NOT_FOUND;
        }
        AK_Log(AK_LOG_ERROR, "USBKEY_GenContainerSymmetricKey ==> SearchContainerRecNo() Error,rtn = %#08x", rtn);
        return rtn;
    }

    //获取空闲的对称密钥ID
    rtn = GetFreeConSymKeyID(hDeviceHandle, &ucSymKeyID, pcConSymKeyIDRec);
    if( rtn != 0 )
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_GenContainerSymmetricKey ==> GetFreeConSymKeyID() Error,rtn = %#08x", rtn);
        return rtn;
    }

    p1 = 0x00;
    p2 = 0x00;
    lc = 0x06;
    le = 0x00;

    memset(data,0,sizeof(data));
    le = ucSymKeyID;
    memcpy(data,&le,1);
    le = 0x07;
    memcpy(data+1,&le,1);
    memcpy(data+2,&ulAlgID,1);
    if( (ulGenKeyFlags & USBKEY_CRYPT_EXPORTABLE) != 0)
        le = 0x00;
    else
        le = 0x01;
    memcpy(data+3,&le,1);
    if( (ulGenKeyFlags & USBKEY_CRYPT_USER_PROTECTED) != 0)
        le = USBKEY_USER_USE_PRIV;
    else
        le = USBKEY_ALL_PRIV;
    memcpy(data+4,&le,1);
    memcpy(data+5,&le,1);
    le = 0x00;

    /*rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_GenSymmKey,request,&reqLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_GenContainerSymmetricKey ==> PackageData() Error,rtn = %#08x", rtn);
      return rtn;
      }

      respLen = 300;
      rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_GenContainerSymmetricKey ==> DeviceExecCmd() Error,rtn = %#08x", rtn);
      return rtn;
      }*/

    //Add by zhoushenshen in 20121114
    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_GenSymmKey,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_GenContainerSymmetricKey ==> PackageData() Error, rtn = %#08x", rtn);
        return rtn;
    }
    //Add end
    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_GenContainerSymmetricKey ==> DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
    }

    //把对称密钥增加到容器
    rtn = AddContainerSymKey(hDeviceHandle, ucSymKeyID, (USBKEY_UCHAR)recNO, pcConSymKeyIDRec);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_GenContainerSymmetricKey ==> AddContainerSymKey() Error,rtn = %#08x", rtn);
        return rtn;
    }	

    //返回Key ID
    phKey[0] = 0x10;
    phKey[1] = ucSymKeyID;

    AK_Log(AK_LOG_DEBUG, "USBKEY_GenContainerSymmetricKey Success!");
    AK_UnlockMutex(gpMutex);
    return USBKEY_OK;
}


//	删除容器对称密钥.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DeleteContainerSymmetricKey(
        USBKEY_HANDLE hDeviceHandle,		//[in]设备句柄
        USBKEY_UCHAR_PTR pcContainerName,	//[in]密钥容器名称
        USBKEY_UCHAR_PTR phKey)				//[in]密钥ID
{

    USBKEY_ULONG rtn,recNO;
    USBKEY_UCHAR pcFileID[2];

    USBKEY_UCHAR ucSymKeyID;
    USBKEY_UCHAR pcConSymKeyIDRec[USBKEY_MAX_SYM_KEY_ID];

    AK_Log(AK_LOG_DEBUG, "USBKEY_DeleteContainerSymmetricKey");

    AK_LockMutex(gpMutex);	

    rtn = SearchContainerRecNo(hDeviceHandle,pcContainerName,&recNO);
    if( (rtn != 0) || (recNO == 0) )
    {
        AK_UnlockMutex(gpMutex);
        if(recNO == 0)
        {
            AK_Log(AK_LOG_ERROR, "USBKEY_DeleteContainerSymmetricKey ==> not find container record.");
            return USBKEY_RECORD_NOT_FOUND;
        }
        AK_Log(AK_LOG_ERROR, "USBKEY_DeleteContainerSymmetricKey ==> SearchContainerRecNo() Error,rtn = %#08x", rtn);
        return rtn;
    }

    //选择对称密钥ID文件
    pcFileID[0] = (USBKEY_SYM_KEY_FILE_ID&0xff00)>>8;
    pcFileID[1] = USBKEY_SYM_KEY_FILE_ID&0x00ff;
    rtn = SelectFile(hDeviceHandle,NULL, pcFileID);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "====>>USBKEY_DeleteContainerSymmetricKey->SelectFile() Error,rtn = %#08x",rtn);
        return rtn;
    }

    //读对称密钥ID文件
    rtn = ReadCurBinaryFile(hDeviceHandle, 0, USBKEY_SYM_KEY_ID_FILE_LEN, pcConSymKeyIDRec);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "====>>USBKEY_DeleteContainerSymmetricKey->ReadCurBinaryFile() Error,rtn = %#08x",rtn);
        return rtn;
    }

    ucSymKeyID = phKey[1];
    if(pcConSymKeyIDRec[ucSymKeyID - 1] == recNO)
    {
        rtn = DeleteSymKey(hDeviceHandle, ucSymKeyID);
        if (rtn != 0)
        {
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "====>>USBKEY_DeleteContainerSymmetricKey->DeleteSymKey() Error,rtn = %#08x",rtn);
            return rtn;
        }
        pcConSymKeyIDRec[ucSymKeyID - 1] = 0xff;
    }
    else
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "====>>USBKEY_DeleteContainerSymmetricKey-> phKey not the ContainerName key,rtn = %#08x",USBKEY_KEY_ID_ERROR);
        return USBKEY_KEY_ID_ERROR;
    }

    //写对称密钥ID文件
    rtn = WriteCurBinaryFile(hDeviceHandle, 0, USBKEY_SYM_KEY_ID_FILE_LEN, (USBKEY_UCHAR_PTR)pcConSymKeyIDRec);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "====>>USBKEY_DeleteContainerSymmetricKey->WriteCurBinaryFile() Error,rtn = %#08x",rtn);
        return rtn;
    }

    AK_Log(AK_LOG_DEBUG, "USBKEY_DeleteContainerSymmetricKey Success!");
    AK_UnlockMutex(gpMutex);
    return USBKEY_OK;
}


//	产生RSA非对称密钥.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_GenerateRSAKeyPair(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcContainerName,//[in]密钥容器名称
        USBKEY_ULONG ulKeyUsage,		//[in]密钥用途
        USBKEY_ULONG ulBitLen,			//[in]密钥模长
        USBKEY_ULONG ulGenKeyFlags,		//[in]生成密钥的控制标志
        USBKEY_UCHAR_PTR pcPubKeyID,	//[out]返回的公钥ID
        USBKEY_UCHAR_PTR pcPriKeyID)	//[out]返回的私钥ID
{
    //USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0,recNO,ulAccessCondition,ulFileSize;
    USBKEY_UCHAR fileName[4],data[16];
    USBKEY_UCHAR p1,p2,lc,le;

    AK_Log(AK_LOG_DEBUG, "USBKEY_GenerateRSAKeyPair");

    /*if (strlen(pcAppName) > 16)
      {
      LogMessage("====>>USBKEY_GenerateRSAKeyPair->pcAppName Length Error!");
      return USBKEY_PARAM_ERROR;
      }*/
    if (strlen((char *)pcContainerName) > 254)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_GenerateRSAKeyPair ==> pcContainerName Length Error,Max Name Len is 255!");
        return USBKEY_PARAM_ERROR;
    }

    if ((ulKeyUsage != KEY_EXCHANGE) && (ulKeyUsage != KEY_SIGNATURE) )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_GenerateRSAKeyPair ==>ulKeyUsage Error,ulKeyUsage = %#08x", ulKeyUsage);
        return USBKEY_PARAM_ERROR;
    }
    if( (ulBitLen != 1024) && (ulBitLen != 2048) )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_GenerateRSAKeyPair ==>ulBitLen Error,ulBitLen = %#08x", ulBitLen);
        return USBKEY_PARAM_ERROR;
    }

    AK_LockMutex(gpMutex);

    /*rtn = SelectFile(hDeviceHandle,pcAppName,NULL);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage16Val("====>>USBKEY_GenerateRSAKeyPair->SelectFile() Error, rtn = 0x",rtn);
      return USBKEY_GENERAL_ERROR;
      }*/

    /////////检查权限///////////////////////////////////////////////////
    /*rtn = CheckPrivillege(hDeviceHandle);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage16Val("====>>USBKEY_GenerateRSAKeyPair->CheckPrivillege() Error,rtn = 0x",rtn);
      return rtn;
      }*/
    //////////////////////////////////////////////////////////////////////////

    rtn = SearchContainerRecNo(hDeviceHandle,pcContainerName,&recNO);
    if( (rtn != 0) || (recNO == 0) )
    {
        AK_UnlockMutex(gpMutex);
        if(recNO == 0)
        {
            AK_Log(AK_LOG_ERROR, "USBKEY_GenerateRSAKeyPair ==> not find container record.");
            return USBKEY_RECORD_NOT_FOUND;
        }
        AK_Log(AK_LOG_ERROR, "USBKEY_GenerateRSAKeyPair ==> SearchContainerRecNo() Error,rtn = %#08x", rtn);
        return rtn;
    }

    ////////
    //生成文件名
    ////////
    memcpy(fileName,&recNO,1);
    memcpy(fileName+2,&recNO,1);
    if (ulKeyUsage == KEY_EXCHANGE)
    {
        memset(fileName+1,0x01,1);
        memset(fileName+3,0x02,1);
    }
    else if (ulKeyUsage == KEY_SIGNATURE)
    {
        memset(fileName+1,0x04,1);
        memset(fileName+3,0x05,1);
    }
    if( (ulGenKeyFlags & USBKEY_CRYPT_USER_PROTECTED) != 0)
        ulAccessCondition = USBKEY_USER_USE_PRIV;
    else
        ulAccessCondition = USBKEY_ALL_PRIV;

    //创建公私钥文件
    p1 = 0x00;
    p2 = 0x10;
    //lc = 9;
    lc = 0x07;						//lc updtae in 20120106
    le = 0x00;

    memset(data,0,9);
    memcpy(data,fileName,2);
    data[3] = USBKEY_ALL_PRIV;
    memcpy(data+4,&ulAccessCondition,1);
    //updtae in 20120106 文件长度由4字节改为2字节
    ulFileSize = 516;
    le = (USBKEY_UCHAR)(ulFileSize/256);
    //memcpy(data+7,&le,1);
    memcpy(data+5,&le,1);
    le = (USBKEY_UCHAR)ulFileSize;
    //memcpy(data+8,&le,1);
    memcpy(data+6,&le,1);

    //Delete by zhoushenshen in 20121107
    /*rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_CreateFile,request,&reqLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_GenerateRSAKeyPair ==>PackageData() Create PubKey File Error,rtn = %#08x", rtn);
      return rtn;
      }

      respLen = 300;
      rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_GenerateRSAKeyPair ==>DeviceExecCmd() Create PubKey File Error,rtn = %#08x", rtn);
      return rtn;
      }*/
    //Del end

    //Add by zhoushenshen in 20121107
    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_CreateFile,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_GenerateRSAKeyPair ==> PackageData() Create PubKey File Error, rtn = %#08x", rtn);
        return rtn;
    }
    //Add end

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        case DEVICE_CMD_DATA_ERROR:
            AK_Log(AK_LOG_DEBUG, "USBKEY_GenerateRSAKeyPair Public Key File Exist.");
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_GenerateRSAKeyPair ==>DeviceExecCmd() Create PubKey File Error,code = %#08x", rtn);
            return rtn;
            break;
    }

    AK_Log(AK_LOG_DEBUG, "USBKEY_GenerateRSAKeyPair Create Public Key File Success.");

    ulAccessCondition = USBKEY_USER_USE_PRIV;//私钥权限
    p1 = 0x00;
    p2 = 0x11;
    //lc = 9;
    lc = 0x07;						//lc updtae in 20120106
    le = 0x00;

    memset(data,0,9);
    memcpy(data,fileName+2,2);
    data[2] = 0x02;
    le = (USBKEY_UCHAR)ulAccessCondition;	//设置读权限
    memcpy(data+3,&le,1);
    memcpy(data+4,&ulAccessCondition,1);
    //updtae in 20120106 文件长度由4字节改为2字节
    ulFileSize = 1412;
    le = (USBKEY_UCHAR)(ulFileSize/256);
    //memcpy(data+7,&le,1);
    memcpy(data+5,&le,1);
    le = (USBKEY_UCHAR)ulFileSize;
    //memcpy(data+8,&le,1);
    memcpy(data+6,&le,1);

    //Delete by zhoushenshen in 20121107
    /*rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_CreateFile,request,&reqLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_GenerateRSAKeyPair ==>PackageData() Create PriKey File Error,rtn = %#08x", rtn);
      return rtn;
      }

      respLen = 300;
      rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_GenerateRSAKeyPair ==>DeviceExecCmd() Create PriKey File Error,rtn = %#08x", rtn);
      return rtn;
      }*/
    //Del end

    //Add by zhoushenshen in 20121107
    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_CreateFile,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_GenerateRSAKeyPair ==> PackageData() Create PriKey File Error, rtn = %#08x", rtn);
        return rtn;
    }
    //Add end

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        case DEVICE_CMD_DATA_ERROR:
            AK_Log(AK_LOG_DEBUG, "USBKEY_GenerateRSAKeyPair Private Key File Exist.");
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_GenerateRSAKeyPair ==>DeviceExecCmd() Create PriKey File Error,code = %#08x", rtn);
            return rtn;
            break;
    }

    AK_Log(AK_LOG_DEBUG, "USBKEY_GenerateRSAKeyPair Create Private Key File Success.");

    //生成密钥
    if (ulBitLen == 1024)
    {
        p1 = 0x01;
    }
    else
    {
        p1 = 0x02;
    }
    p2 = 0x00;
    lc = 0x04;
    le = 0x00;

    //Delete by zhoushenshen in 20121116
    /*rtn = PackageData(p1,p2,lc,fileName,le,USBKEY_OP_GenRSAKeyPair,request,&reqLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_GenerateRSAKeyPair ==>PackageData() GenKeyPair Error,rtn = %#08x", rtn);
      return rtn;
      }

      respLen = 300;
      rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_GenerateRSAKeyPair ==>DeviceExecCmd() GenKeyPair Error,rtn = %#08x", rtn);
      return rtn;
      }*/
    //Del end

    //Add by zhoushenshen in 20121116
    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,fileName,le,USBKEY_OP_GenRSAKeyPair,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_GenerateRSAKeyPair ==> PackageData() Create PriKey File Error, rtn = %#08x", rtn);
        return rtn;
    }
    //Add end

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_GenerateRSAKeyPair ==>DeviceExecCmd() GenKeyPair Error,code = %#08x", rtn);
            return rtn;
            break;
    }

    AK_Log(AK_LOG_DEBUG, "USBKEY_GenerateRSAKeyPair GenKeyPair Success.");

    rtn = AddContainerKey(hDeviceHandle,recNO,ulKeyUsage,ulBitLen);	//Update by zhoushenshen 20100309
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_GenerateRSAKeyPair ==>AddContainerKey() Error,rtn = %#08x", rtn);
        return rtn;
    }

    ///////
    //赋值返回ID
    memcpy(pcPubKeyID,fileName,2);
    memcpy(pcPriKeyID,fileName+2,2);

    ///////
    AK_Log(AK_LOG_DEBUG, "USBKEY_GenerateRSAKeyPair Success!");
    AK_UnlockMutex(gpMutex);

    return USBKEY_OK;
}

//	获取非对称密钥ID.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_GetAsymmetricKeyID(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcContainerName,//[in]密钥容器名称
        USBKEY_ULONG ulKeyUsage,		//[in]密钥用途
        USBKEY_UCHAR_PTR pcKeyPair)		//[out]返回的RSA密钥句柄，按照USBKEY_RSA_KEY返回
{
    USBKEY_ULONG recNO;
    USBKEY_ULONG rtn;
    USBKEY_UCHAR head[4];
    USBKEY_UCHAR head1024[4] = {0x00, 0x00, 0x04, 0x00};
    USBKEY_UCHAR head2048[4] = {0x00, 0x00, 0x08, 0x00};
    USBKEY_UCHAR head256[4] = {0x00, 0x00, 0x01, 0x00};
    USBKEY_UCHAR head384[4] = {0x00, 0x00, 0x01, 0x80};

    AK_Log(AK_LOG_DEBUG, "USBKEY_GetAsymmetricKeyID");

    if( NULL == hDeviceHandle || NULL == pcContainerName || NULL == pcKeyPair)
    {
        AK_Log(AK_LOG_ERROR, "====>>USBKEY_GetAsymmetricKeyID->PARAM is NULL");
        return USBKEY_PARAM_ERROR;
    }

    if (strlen((char *)pcContainerName) > 254)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_GetAsymmetricKeyID ==> pcContainerName Length Error,Max Name Len is 255!");
        return USBKEY_PARAM_ERROR;
    }

    if ((ulKeyUsage != KEY_EXCHANGE) && (ulKeyUsage != KEY_SIGNATURE) )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_GetAsymmetricKeyID ==>ulKeyUsage Error,ulKeyUsage = %#08x", ulKeyUsage);
        return USBKEY_PARAM_ERROR;
    }



    AK_LockMutex(gpMutex);

    //////////////////////////////////////////////////////////////////////////

    rtn = SearchContainerRecNo(hDeviceHandle,pcContainerName,&recNO);
    if( (rtn != 0) || (recNO == 0) )
    {
        AK_UnlockMutex(gpMutex);
        if(recNO == 0)
        {
            AK_Log(AK_LOG_ERROR, "USBKEY_GetAsymmetricKeyID ==> not find container record.");
            return USBKEY_RECORD_NOT_FOUND;
        }
        AK_Log(AK_LOG_ERROR, "USBKEY_GetAsymmetricKeyID ==> SearchContainerRecNo() Error,rtn = %#08x", rtn);
        return rtn;
    }

    //赋值
    memcpy((*(USBKEY_KEY_PAIR_ID*)pcKeyPair).publicKeyID,&recNO,1);
    memcpy((*(USBKEY_KEY_PAIR_ID*)pcKeyPair).privateKeyID,&recNO,1);
    if (ulKeyUsage == KEY_EXCHANGE)
    {
        memset((*(USBKEY_KEY_PAIR_ID*)pcKeyPair).publicKeyID+1,0x01,1);
        memset((*(USBKEY_KEY_PAIR_ID*)pcKeyPair).privateKeyID+1,0x02,1);
    }
    else if (ulKeyUsage == KEY_SIGNATURE)
    {
        memset((*(USBKEY_KEY_PAIR_ID*)pcKeyPair).publicKeyID+1,0x04,1);
        memset((*(USBKEY_KEY_PAIR_ID*)pcKeyPair).privateKeyID+1,0x05,1);
    }

    //检查公钥
    //////////////////////////////////////////////////////////////////////////
    rtn = SelectFile(hDeviceHandle,NULL,(*(USBKEY_RSA_KEY*)pcKeyPair).publicKeyID);	//Update by zhoushenshen 20100309
    if (rtn != 0)
    {
        memset((*(USBKEY_RSA_KEY*)pcKeyPair).publicKeyID,0,2);
        (*(USBKEY_RSA_KEY*)pcKeyPair).bits = 0;
    }
    else
    {
        //读公钥文件的模长
        rtn = ReadCurBinaryFile(hDeviceHandle, 0, 4, head);
        if (rtn != 0)
        {
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_GetAsymmetricKeyID ==> ReadCurBinaryFile() Error,rtn = %#08x", rtn);
            return rtn;
        }

        if (memcmp(head,head2048,4) == 0)
        {
            (*(USBKEY_KEY_PAIR_ID*)pcKeyPair).bits = 2048;
        }
        else if (memcmp(head,head1024,4) == 0)
        {
            (*(USBKEY_KEY_PAIR_ID*)pcKeyPair).bits = 1024;
        }
        else if(memcmp(head,head256,4) == 0)
        {
            (*(USBKEY_KEY_PAIR_ID*)pcKeyPair).bits = 256;
        }
        else if(memcmp(head,head384,4) == 0)
        {
            (*(USBKEY_KEY_PAIR_ID*)pcKeyPair).bits = 384;
        }
        else
        {
            memset((*(USBKEY_KEY_PAIR_ID*)pcKeyPair).publicKeyID,0,2);
            (*(USBKEY_KEY_PAIR_ID*)pcKeyPair).bits = 0;
        }
    }

    //判断私钥文件是否存在
    //////////////////////////////////////////////////////////////////////////
    rtn = SelectFile(hDeviceHandle,	NULL,(*(USBKEY_KEY_PAIR_ID*)pcKeyPair).privateKeyID);	//Update by zhoushenshen 20100309
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        memset((*(USBKEY_KEY_PAIR_ID*)pcKeyPair).privateKeyID,0,2);
        (*(USBKEY_KEY_PAIR_ID*)pcKeyPair).bits = 0;
    }

    ///////
    AK_Log(AK_LOG_DEBUG, "USBKEY_GetAsymmetricKeyID Success!");
    AK_UnlockMutex(gpMutex);

    return USBKEY_OK;
}


//	获取非对称私钥模长.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_GetPriKeyBitLen(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcContainerName,//[in]密钥容器名称
        USBKEY_ULONG ulKeyUsage,		//[in]密钥用途
        USBKEY_ULONG_PTR pulBitLen)		//[out]返回的私钥的模长
{
    USBKEY_ULONG recNO;
    USBKEY_ULONG rtn;
    USBKEY_UCHAR privateKeyID[2],fileID[16];
    USBKEY_ULONG ulIDLen = 16;

    AK_Log(AK_LOG_DEBUG, "USBKEY_GetPriKeyBitLen");

    if( NULL == hDeviceHandle || NULL == pcContainerName)
    {
        AK_Log(AK_LOG_ERROR, "====>>USBKEY_GetPriKeyBitLen->PARAM is NULL");
        return USBKEY_PARAM_ERROR;
    }

    if (strlen((char *)pcContainerName) > 254)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_GetPriKeyBitLen ==> pcContainerName Length Error,Max Name Len is 255!");
        return USBKEY_PARAM_ERROR;
    }

    if ((ulKeyUsage != KEY_EXCHANGE) && (ulKeyUsage != KEY_SIGNATURE) )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_GetPriKeyBitLen ==>ulKeyUsage Error,ulKeyUsage = %#08x", ulKeyUsage);
        return USBKEY_PARAM_ERROR;
    }



    AK_LockMutex(gpMutex);

    //////////////////////////////////////////////////////////////////////////

    rtn = SearchContainerRecNo(hDeviceHandle,pcContainerName,&recNO);
    if( (rtn != 0) || (recNO == 0) )
    {
        AK_UnlockMutex(gpMutex);
        if(recNO == 0)
        {
            AK_Log(AK_LOG_ERROR, "USBKEY_GetPriKeyBitLen ==> not find container record.");
            return USBKEY_RECORD_NOT_FOUND;
        }
        AK_Log(AK_LOG_ERROR, "USBKEY_GetPriKeyBitLen ==> SearchContainerRecNo() Error,rtn = %#08x", rtn);
        return rtn;
    }

    //判断私钥文件是否存在
    //////////////////////////////////////////////////////////////////////////
    privateKeyID[0] = (USBKEY_UCHAR)recNO;
    if (ulKeyUsage == KEY_EXCHANGE)
        privateKeyID[1] = 0x02;
    else
        privateKeyID[1] = 0x05;

    rtn = SelectFile(hDeviceHandle,	NULL,privateKeyID);	//Update by zhoushenshen 20100309
    if (rtn != 0)
    {
        pulBitLen = 0;
    }
    else
    {
        //获得私钥模长
        memset(fileID,0,sizeof(fileID));
        rtn = ReadContainerKeyRec(hDeviceHandle,recNO,ulKeyUsage,USBKEY_RSA_PRIVATEKEY,fileID,&ulIDLen);
        if (rtn != 0)
        {
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_GetPriKeyBitLen ==> ReadContainerKeyRec() Error,rtn = %#08x", rtn);
            return rtn;
        }
        memcpy(pulBitLen,fileID+2,4);
    }

    AK_Log(AK_LOG_DEBUG, "USBKEY_GetPriKeyBitLen Success!");
    AK_UnlockMutex(gpMutex);
    return USBKEY_OK;
}


/*
//	获取对称密钥.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ExportSymmetricKey(
USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
USBKEY_UCHAR_PTR pcAppName,		//[in]应用名称
USBKEY_UCHAR_PTR pcSymKeyID,	//[in]被导出对称密钥
USBKEY_UCHAR_PTR pcWrapKeyID,	//[in]用来保护导出密钥的密钥ID
USBKEY_UCHAR_PTR pcSymKey,		//[out]返回的密钥
USBKEY_ULONG_PTR pulSymKeyLen)	//[out]返回的密钥长度
{
USBKEY_UCHAR request[300];
USBKEY_UCHAR response[300];
USBKEY_ULONG rtn,reqLen,respLen=0;
USBKEY_UCHAR data[24];
USBKEY_UCHAR p1,p2,lc,le;

LogMessage("->USBKEY_ExportSymmetricKey");
if (strlen(pcAppName) > 16)
{
LogMessage("====>>USBKEY_ExportSymmetricKey->pcAppName Length Error!");
return USBKEY_PARAM_ERROR;
}
rtn = SelectFile(hDeviceHandle,pcAppName,NULL);
if (rtn != 0)
{
LogMessage16Val("====>>USBKEY_ExportSymmetricKey->SelectFile() Error,rtn = 0x",rtn);
return rtn;
}

/////////检查权限///////////////////////////////////////////////////
rtn = CheckPrivillege(hDeviceHandle);
if (rtn != 0)
{
LogMessage16Val("====>>USBKEY_ExportSymmetricKey->CheckPrivillege() Error,rtn = 0x",rtn);
return rtn;
}
//////////////////////////////////////////////////////////////////////////

if (*pcSymKeyID == 0x10)
{
p1 = 0x07;
}
else if (*pcSymKeyID == 0x11)
{
p1 = 0x08;
}
else
{
LogMessage("====>>USBKEY_ExportSymmetricKey->pcSymKeyID Error!");
return USBKEY_PARAM_ERROR;
}
p2 = 0;
lc = 0x02;
le = 0x00;

memset(data,0,sizeof(data));
memcpy(data,pcWrapKeyID,2);

rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_ExportSymmKey,request,&reqLen);
if (rtn != 0)
{
LogMessage16Val("====>>USBKEY_ExportSymmetricKey->PackageData() Error,rtn = 0x",rtn);
return rtn;
}

respLen = 300;
rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,&respLen);
if (rtn != 0)
{
    LogMessage16Val("====>>USBKEY_ExportSymmetricKey->DeviceExecCmd() Error,rtn = 0x",rtn);
    return rtn;
}
rtn = *(response+respLen-2);
rtn = rtn*256 + *(response+respLen-1);
switch(rtn)
{
    case DEVICE_SUCCESS:
        break;
    default:
        LogMessage16Val("====>>USBKEY_ExportSymmetricKey->DeviceExecCmd() Error,code = 0x",rtn);
        return rtn;
        break;
}

memcpy(pcSymKey,response,respLen-2);
*pulSymKeyLen = respLen - 2;

LogMessage("->USBKEY_ExportSymmetricKey Success");

return USBKEY_OK;
}

//	导入对称密钥.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ImportSymmetricKey(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcAppName,		//[in]应用名称
        USBKEY_UCHAR_PTR pcWrapKeyID,	//[in]用来保护导入密钥的密钥ID
        USBKEY_ULONG ulKeyType,			//[in]密钥类型，表示密钥是卡内固定密钥还是临时密钥，可指定为USBKEY_CRYPT_ONDEVICE或者为0
        USBKEY_UCHAR_PTR pcSymKey,		//[in]导入的密钥
        USBKEY_ULONG ulSymKeyLen,		//[in]导入的密钥长度
        USBKEY_UCHAR_PTR pcSymKeyID)	//[out]返回导入的对称密钥ID
{
    USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    USBKEY_ULONG rtn,reqLen,respLen=0;
    USBKEY_UCHAR data[300];
    USBKEY_UCHAR p1,p2,lc,le;

    LogMessage("->USBKEY_ImportSymmetricKey");
    if (strlen(pcAppName) > 16)
    {
        LogMessage("====>>USBKEY_ImportSymmetricKey->pcAppName Length Error!");
        return USBKEY_PARAM_ERROR;
    }
    rtn = SelectFile(hDeviceHandle,pcAppName,NULL);
    if (rtn != 0)
    {
        LogMessage16Val("====>>USBKEY_ImportSymmetricKey->SelectFile() Error,rtn = 0x",rtn);
        return rtn;
    }

    /////////检查权限///////////////////////////////////////////////////
    rtn = CheckPrivillege(hDeviceHandle);
    if (rtn != 0)
    {
        LogMessage16Val("====>>USBKEY_ImportSymmetricKey->CheckPrivillege() Error,rtn = 0x",rtn);
        return rtn;
    }
    //////////////////////////////////////////////////////////////////////////

    if (pcWrapKeyID == NULL)
    {
        p1 = 0x00;
        p2 = 0x00;
        if ((ulSymKeyLen != 14) && (ulSymKeyLen != 22) && (ulSymKeyLen != 30))
        {
            LogMessage("====>>USBKEY_ImportSymmetricKey->ulSymKeyLen Error!");
            return USBKEY_PARAM_ERROR;
        }
        else
            lc = ulSymKeyLen;
        memcpy(data,pcSymKey,ulSymKeyLen);
    }
    else
    {
        memcpy(&p1,pcWrapKeyID,1);
        memcpy(&p2,pcWrapKeyID+1,1);
        if (ulSymKeyLen == 128)
        {
            lc = ulSymKeyLen;
            memcpy(data,pcSymKey,ulSymKeyLen);
        }
        else if (ulSymKeyLen == 256)
        {
            lc = 0xff;
            memset(data,0x01,1);
            memset(data+1,0x00,1);
            memcpy(data+2,pcSymKey,ulSymKeyLen);
        }
        else
        {
            LogMessage("====>>USBKEY_ImportSymmetricKey->ulSymKeyLen Error!");
            return USBKEY_PARAM_ERROR;
        }
    }
    le = 0x00;

    rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_ImportSymmKey,request,&reqLen);
    if (rtn != 0)
    {
        LogMessage16Val("====>>USBKEY_ImportSymmetricKey->PackageData() Error,rtn = 0x",rtn);
        return rtn;
    }

    respLen = 300;
    rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,&respLen);
    if (rtn != 0)
    {
        LogMessage16Val("====>>USBKEY_ImportSymmetricKey->DeviceExecCmd() Error,rtn = 0x",rtn);
        return rtn;
    }
    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            LogMessage16Val("====>>USBKEY_ImportSymmetricKey->DeviceExecCmd() Error,code = 0x",rtn);
            return rtn;
            break;
    }

    if ( (ulKeyType & USBKEY_CRYPT_ONDEVICE) != 0)
        *pcSymKeyID = 0x10;
    else
        *pcSymKeyID = 0x11;

    LogMessage("->USBKEY_ImportSymmetricKey Success");

    return USBKEY_OK;
}



//	获取分体对称密钥.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ExportDivSymmetricKey(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcAppName,		//[in]应用名称
        USBKEY_UCHAR_PTR pcSymKeyID,	//[in]被导出对称密钥
        USBKEY_UCHAR_PTR pcWrapKeyID,	//[in]用来保护导出密钥的密钥ID
        USBKEY_UCHAR_PTR pcSymKey,		//[out]返回的密钥
        USBKEY_ULONG_PTR pulSymKeyLen)	//[out]返回的密钥长度
{
    USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    USBKEY_ULONG rtn,reqLen,respLen=0;
    USBKEY_UCHAR data[24];
    USBKEY_UCHAR p1,p2,lc,le;

    LogMessage("->USBKEY_ExportDivSymmetricKey");
    if (strlen(pcAppName) > 16)
    {
        LogMessage("====>>USBKEY_ExportDivSymmetricKey->pcAppName Length Error!");
        return USBKEY_PARAM_ERROR;
    }
    rtn = SelectFile(hDeviceHandle,pcAppName,NULL);
    if (rtn != 0)
    {
        LogMessage16Val("====>>USBKEY_ExportDivSymmetricKey->SelectFile() Error,rtn = 0x",rtn);
        return rtn;
    }

    /////////检查权限///////////////////////////////////////////////////
    rtn = CheckPrivillege(hDeviceHandle);
    if (rtn != 0)
    {
        LogMessage16Val("====>>USBKEY_ExportDivSymmetricKey->CheckPrivillege() Error,rtn = 0x",rtn);
        return rtn;
    }
    //////////////////////////////////////////////////////////////////////////

    if (*pcSymKeyID == 0x10)
    {
        p1 = 0x07;
    }
    else if (*pcSymKeyID == 0x11)
    {
        p1 = 0x08;
    }
    else
    {
        LogMessage("====>>USBKEY_ExportDivSymmetricKey->pcSymKeyID Error!");
        return USBKEY_PARAM_ERROR;
    }
    p2 = 0;
    lc = 0x02;
    le = 0x00;

    memset(data,0,sizeof(data));
    memcpy(data,pcWrapKeyID,2);

    rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_ExportDivSymmKey,request,&reqLen);
    if (rtn != 0)
    {
        LogMessage16Val("====>>USBKEY_ExportDivSymmetricKey->PackageData() Error,rtn = 0x",rtn);
        return rtn;
    }

    respLen = 300;
    rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,&respLen);
    if (rtn != 0)
    {
        LogMessage16Val("====>>USBKEY_ExportDivSymmetricKey->DeviceExecCmd() Error,rtn = 0x",rtn);
        return rtn;
    }
    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            LogMessage16Val("====>>USBKEY_ExportDivSymmetricKey->DeviceExecCmd() Error,code = 0x",rtn);
            return rtn;
            break;
    }

    memcpy(pcSymKey,response+6,respLen-8);	//不拷贝密钥属性
    *pulSymKeyLen = respLen - 8;

    LogMessage("->USBKEY_ExportDivSymmetricKey Success");

    return USBKEY_OK;
}

//	导入分体对称密钥.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ImportDivSymmetricKey(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcAppName,		//[in]应用名称
        USBKEY_UCHAR_PTR pcWrapKeyID,	//[in]用来保护导入密钥的密钥ID
        USBKEY_ULONG ulKeyType,			//[in]密钥类型，表示密钥是卡内固定密钥还是临时密钥，可指定为CRYPT_ONDEVICE或者为0
        USBKEY_ULONG ulAlgID,			//[in]密钥算法标识
        USBKEY_UCHAR_PTR pcSymKey,		//[in]导入的密钥
        USBKEY_ULONG ulSymKeyLen,		//[in]导入的密钥长度
        USBKEY_UCHAR_PTR pcSymKeyID)	//[out]返回导入的对称密钥ID
{
    USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    USBKEY_ULONG rtn,reqLen,respLen=0;
    USBKEY_UCHAR data[300];
    USBKEY_UCHAR p1,p2,lc,le;
    USBKEY_UCHAR keyAttr[6];	//密钥属性 add by heli-20090520

    //明文导入：
    //密钥标识（1字节）、密钥类型（1字节）、算法标识（1字节）、控制标志（1字节）、使用权限（1字节）、更改权限（1字节）、密钥值（8字节/16字节/24字节）

    //密文导入：
    //密钥标识（1字节）、密钥类型（1字节）、算法标识（1字节）、控制标志（1字节）、使用权限（1字节）、更改权限（1字节）、用指定的公钥对密钥值加密所得的密文。

    LogMessage("->USBKEY_ImportDivSymmetricKey");
    if (strlen(pcAppName) > 16)
    {
        LogMessage("====>>USBKEY_ImportDivSymmetricKey->pcAppName Length Error!");
        return USBKEY_PARAM_ERROR;
    }
    rtn = SelectFile(hDeviceHandle,pcAppName,NULL);
    if (rtn != 0)
    {
        LogMessage16Val("====>>USBKEY_ImportDivSymmetricKey->SelectFile() Error,rtn = 0x",rtn);
        return rtn;
    }

    /////////检查权限///////////////////////////////////////////////////
    rtn = CheckPrivillege(hDeviceHandle);
    if (rtn != 0)
    {
        LogMessage16Val("====>>USBKEY_ImportDivSymmetricKey->CheckPrivillege() Error,rtn = 0x",rtn);
        return rtn;
    }
    //////////////////////////////////////////////////////////////////////////

    //设置密钥属性：密钥标识（1字节）、密钥类型（1字节）、算法标识（1字节）、控制标志（1字节）、使用权限（1字节）、更改权限（1字节）
    keyAttr[0] = 0;			//密钥标识
    if ( USBKEY_CRYPT_ONDEVICE == ulKeyType )
        keyAttr[1] = 7;		//密钥类型
    else
        keyAttr[1] = 8;		//密钥类型
    keyAttr[2] = ulAlgID;	//算法标识
    keyAttr[3] = 0;			//控制标志: 1，密钥不可以导出；0，密钥可以导出
    keyAttr[4] = 0xf0;		//使用权限
    keyAttr[5] = 0xf3;		//更改权限,用户权限可以修改

    if (pcWrapKeyID == NULL)
    {
        p1 = 0x00;
        p2 = 0x00;
        if ((ulSymKeyLen != 8) && (ulSymKeyLen != 16) && (ulSymKeyLen != 24))
        {
            LogMessage("====>>USBKEY_ImportDivSymmetricKey->ulSymKeyLen Error!");
            return USBKEY_PARAM_ERROR;
        }
        else
            lc = ulSymKeyLen+6;
        memcpy(data,keyAttr,6);
        memcpy(data+6,pcSymKey,ulSymKeyLen);
    }
    else
    {
        memcpy(&p1,pcWrapKeyID,1);
        memcpy(&p2,pcWrapKeyID+1,1);
        if (ulSymKeyLen == 134)
        {
            lc = 0x86;
            memcpy(data,keyAttr,6);
            memcpy(data+6,pcSymKey,ulSymKeyLen);
        }
        else if (ulSymKeyLen == 262)
        {
            lc = 0xff;
            memset(data,0x01,1);
            memset(data+1,0x06,1);
            memcpy(data+2,keyAttr,6);
            memcpy(data+8,pcSymKey,ulSymKeyLen);
        }
        else
        {
            LogMessage("====>>USBKEY_ImportDivSymmetricKey->ulSymKeyLen Error!");
            return USBKEY_PARAM_ERROR;
        }
    }
    le = 0x00;

    rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_ImportDivSymmKey,request,&reqLen);
    if (rtn != 0)
    {
        LogMessage16Val("====>>USBKEY_ImportDivSymmetricKey->PackageData() Error,rtn = 0x",rtn);
        return rtn;
    }

    respLen = 300;
    rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,&respLen);
    if (rtn != 0)
    {
        LogMessage16Val("====>>USBKEY_ImportDivSymmetricKey->DeviceExecCmd() Error,rtn = 0x",rtn);
        return rtn;
    }
    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            LogMessage16Val("====>>USBKEY_ImportDivSymmetricKey->DeviceExecCmd() Error,code = 0x",rtn);
            return rtn;
            break;
    }

    if ( (ulKeyType & USBKEY_CRYPT_ONDEVICE) != 0)
        *pcSymKeyID = 0x10;
    else
        *pcSymKeyID = 0x11;

    LogMessage("->USBKEY_ImportDivSymmetricKey Success");

    return USBKEY_OK;
}
*/

//	导出对称密钥.(???)
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ExportSymmetricKey(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcSymKeyID,	//[in]被导出对称密钥
        USBKEY_UCHAR_PTR pcWrapKeyID,	//[in]用来保护导出密钥的密钥ID
        USBKEY_ULONG ulAsymAlgID,		//[in]用来保护导出密钥的密钥算法：USBKEY_TYPE_RSA或者USBKEY_TYPE_ECC
        USBKEY_UCHAR_PTR pcSymKey,		//[out]返回的密钥
        USBKEY_ULONG_PTR pulSymKeyLen)	//[out]返回的密钥长度
{
    USBKEY_ULONG rtn;


    AK_Log(AK_LOG_DEBUG, "USBKEY_ExportSymmetricKey");

    if( NULL == pcSymKeyID || NULL == hDeviceHandle || NULL == pcWrapKeyID || NULL == pcSymKey || NULL == pulSymKeyLen )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ExportSymmetricKey ==> PARAM is NULL,rtn = %#08x", USBKEY_PARAM_ERROR);
        return USBKEY_PARAM_ERROR;
    }
    if( USBKEY_TYPE_RSA != ulAsymAlgID && USBKEY_TYPE_ECC != ulAsymAlgID )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ExportSymmetricKey ==> ulAsymAlgID Error,ulAsymAlgID = %#08x", ulAsymAlgID);
        return USBKEY_PARAM_ERROR;
    }
    AK_LockMutex(gpMutex);

    //////////////////////////////////////////////////////////////////////////
    if( USBKEY_TYPE_RSA == ulAsymAlgID )
    {
        // RSA加密导出对称密钥
        rtn = RSAExportSessionKey(hDeviceHandle, pcSymKeyID, pcWrapKeyID, pcSymKey, pulSymKeyLen); 
        if (rtn != 0)
        {
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_ExportSymmetricKey ==>RSAExportSessionKey() Error,rtn = %#08x", rtn);
            return rtn;
        }
    }
    else
    {
        // ECC加密导出对称密钥
        rtn = ECCExportSessionKey(hDeviceHandle, pcSymKeyID, pcWrapKeyID, pcSymKey, pulSymKeyLen);
        if (rtn != 0)
        {
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_ExportSymmetricKey ==>ECCExportSessionKey() Error,rtn = %#08x", rtn);
            return rtn;
        }
    }



    AK_Log(AK_LOG_DEBUG, "USBKEY_ExportSymmetricKey Success!");
    AK_UnlockMutex(gpMutex);

    return USBKEY_OK;
}

//	导入对称密钥.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ImportSymmetricKey(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcWrapKeyID,	//[in]用来保护导入密钥的密钥ID，如果明文导入设置为NULL
        USBKEY_ULONG ulAsymAlgID,		//[in]用来保护导入密钥的密钥算法：USBKEY_TYPE_RSA或者USBKEY_TYPE_ECC；如果明文导入设置为0
        USBKEY_ULONG ulKeyType,			//[in]密钥类型，表示密钥是卡内固定密钥还是临时密钥，可指定为USBKEY_CRYPT_ONDEVICE或者为0
        USBKEY_ULONG ulSymAlgID,		//[in]密钥算法标识
        USBKEY_UCHAR_PTR pcSymKey,		//[in]导入的密钥
        USBKEY_ULONG ulSymKeyLen,		//[in]导入的密钥长度
        USBKEY_UCHAR_PTR pcSymKeyID)	//[out]返回导入的对称密钥ID
{
    USBKEY_ULONG rtn;
    USBKEY_UCHAR keyAttr[6];	//密钥属性 add by heli-20090520

    //明文导入：
    //密钥标识（1字节）、密钥类型（1字节）、算法标识（1字节）、控制标志（1字节）、使用权限（1字节）、更改权限（1字节）、密钥值（8字节/16字节/24字节）

    //密文导入：
    //密钥标识（1字节）、密钥类型（1字节）、算法标识（1字节）、控制标志（1字节）、使用权限（1字节）、更改权限（1字节）、用指定的公钥对密钥值加密所得的密文。

    AK_Log(AK_LOG_DEBUG, "USBKEY_ImportSymmetricKey");

    if( USBKEY_TYPE_RSA != ulAsymAlgID && USBKEY_TYPE_ECC != ulAsymAlgID && 0 != ulAsymAlgID )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportSymmetricKey ==> ulAsymAlgID Error,ulAsymAlgID = %#08x", ulAsymAlgID);
        return USBKEY_PARAM_ERROR;
    }

    if( NULL == pcSymKey || NULL == pcSymKeyID  || NULL == hDeviceHandle )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportSymmetricKey ==> pcSymKey or pcSymKeyID is NULL,rtn = %#08x", USBKEY_PARAM_ERROR);
        return USBKEY_PARAM_ERROR;
    }

    AK_LockMutex(gpMutex);

    //////////////////////////////////////////////////////////////////////////

    //设置密钥属性：密钥标识（1字节）、密钥类型（1字节）、算法标识（1字节）、控制标志（1字节）、使用权限（1字节）、更改权限（1字节）
    keyAttr[0] = 0;			//密钥标识
    if ( (ulKeyType&USBKEY_CRYPT_ONDEVICE) != 0 )
        keyAttr[1] = 7;		//密钥类型：卡内固定密钥
    else
        keyAttr[1] = 8;		//密钥类型：卡内临时密钥
    keyAttr[2] = (USBKEY_UCHAR)ulSymAlgID;	//算法标识
    if( (ulKeyType&USBKEY_CRYPT_EXPORTABLE) != 0 )
        keyAttr[3] = 0;			//控制标志: 1，密钥不可以导出；0，密钥可以导出
    else
        keyAttr[3] = 1;

    if( (ulKeyType&USBKEY_CRYPT_USER_PROTECTED) != 0 )
    {
        keyAttr[4] = USBKEY_USER_USE_PRIV;		//使用权限
        keyAttr[5] = USBKEY_USER_USE_PRIV;		//更改权限,用户权限可以修改
    }
    else
    {
        keyAttr[4] = USBKEY_ALL_PRIV;		//使用权限
        keyAttr[5] = USBKEY_ALL_PRIV;
    }

    if( 0 == ulAsymAlgID || USBKEY_TYPE_RSA == ulAsymAlgID )
    {
        //rsa加密导入或者明文导入对称密钥
        rtn = RSAImportSessionKey(hDeviceHandle, pcWrapKeyID, keyAttr, pcSymKey, ulSymKeyLen);
        if (rtn != 0)
        {
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_ImportSymmetricKey ==>RSAImportSessionKey() Error,rtn = %#08x", rtn);
            return rtn;
        }
    }
    else
    {
        //ECC加密导入对称密钥
        rtn = ECCImportSessionKey(hDeviceHandle, pcWrapKeyID, keyAttr, pcSymKey, ulSymKeyLen);
        if (rtn != 0)
        {
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_ImportSymmetricKey ==>ECCImportSessionKey() Error,rtn = %#08x", rtn);
            return rtn;
        }
    }

    //设置密钥ID
    if ( (ulKeyType & USBKEY_CRYPT_ONDEVICE) != 0)
        *pcSymKeyID = 0x10;
    else
        *pcSymKeyID = 0x11;
    *(pcSymKeyID+1) = 0;

    AK_Log(AK_LOG_DEBUG, "USBKEY_ImportSymmetricKey Success!");
    AK_UnlockMutex(gpMutex);

    return USBKEY_OK;
}

//	导入容器对称密钥.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ImportContainerSymmetricKey(
        USBKEY_HANDLE hDeviceHandle,		//[in]设备句柄
        USBKEY_UCHAR_PTR pcContainerName,	//[in]密钥容器名称
        USBKEY_UCHAR_PTR pcWrapKeyID,		//[in]用来保护导入密钥的密钥ID
        USBKEY_ULONG ulAsymAlgID,			//[in]用来保护导入密钥的密钥算法：USBKEY_TYPE_RSA或者USBKEY_TYPE_ECC
        USBKEY_ULONG ulKeyType,				//[in]密钥类型，卡内固定密钥
        USBKEY_ULONG ulAlgID,				//[in]密钥算法标识
        USBKEY_UCHAR_PTR pcSymKey,			//[in]导入的密钥
        USBKEY_ULONG ulSymKeyLen,			//[in]导入的密钥长度
        USBKEY_UCHAR_PTR pcSymKeyID)		//[out]返回导入的对称密钥ID
{
    USBKEY_ULONG rtn,recNO;
    USBKEY_UCHAR keyAttr[6];	//密钥属性 add by heli-20090520
    USBKEY_UCHAR ucSymKeyID;
    USBKEY_UCHAR pcConSymKeyIDRec[USBKEY_MAX_SYM_KEY_ID];

    //明文导入：
    //密钥标识（1字节）、密钥类型（1字节）、算法标识（1字节）、控制标志（1字节）、使用权限（1字节）、更改权限（1字节）、密钥值（8字节/16字节/24字节）

    //密文导入：
    //密钥标识（1字节）、密钥类型（1字节）、算法标识（1字节）、控制标志（1字节）、使用权限（1字节）、更改权限（1字节）、用指定的公钥对密钥值加密所得的密文。

    AK_Log(AK_LOG_DEBUG, "USBKEY_ImportContainerSymmetricKey");

    if( USBKEY_TYPE_RSA != ulAsymAlgID && USBKEY_TYPE_ECC != ulAsymAlgID && 0 != ulAsymAlgID )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportContainerSymmetricKey ==> ulAsymAlgID Error,ulAsymAlgID = %#08x", ulAsymAlgID);
        return USBKEY_PARAM_ERROR;
    }

    if( NULL == pcSymKey || NULL == pcSymKeyID || NULL == pcContainerName || NULL == hDeviceHandle )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportContainerSymmetricKey ==> PARAM is NULL,rtn = %#08x", USBKEY_PARAM_ERROR);
        return USBKEY_PARAM_ERROR;
    }

    AK_LockMutex(gpMutex);

    rtn = SearchContainerRecNo(hDeviceHandle,pcContainerName,&recNO);
    if( (rtn != 0) || (recNO == 0) )
    {
        AK_UnlockMutex(gpMutex);
        if(recNO == 0)
        {
            AK_Log(AK_LOG_ERROR, "USBKEY_ImportContainerSymmetricKey ==> not find container record.");
            return USBKEY_RECORD_NOT_FOUND;
        }
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportContainerSymmetricKey ==> SearchContainerRecNo() Error,rtn = %#08x", rtn);
        return rtn;
    }

    //获取空闲的对称密钥ID
    rtn = GetFreeConSymKeyID(hDeviceHandle, &ucSymKeyID, pcConSymKeyIDRec);
    if( rtn != 0 )
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportContainerSymmetricKey ==> GetFreeConSymKeyID() Error,rtn = %#08x", rtn);
        return rtn;
    }

    //////////////////////////////////////////////////////////////////////////

    //设置密钥属性：密钥标识（1字节）、密钥类型（1字节）、算法标识（1字节）、控制标志（1字节）、使用权限（1字节）、更改权限（1字节）
    keyAttr[0] = ucSymKeyID;			//密钥标识
    keyAttr[1] = 7;		//密钥类型：卡内固定密钥
    keyAttr[2] = (USBKEY_UCHAR)ulAlgID;	//算法标识
    if( (ulKeyType&USBKEY_CRYPT_EXPORTABLE) != 0 )
        keyAttr[3] = 0;			//控制标志: 1，密钥不可以导出；0，密钥可以导出
    else
        keyAttr[3] = 1;

    if( (ulKeyType&USBKEY_CRYPT_USER_PROTECTED) != 0 )
    {
        keyAttr[4] = USBKEY_USER_USE_PRIV;		//使用权限
        keyAttr[5] = USBKEY_USER_USE_PRIV;		//更改权限,用户权限可以修改
    }
    else
    {	
        keyAttr[4] = USBKEY_ALL_PRIV;		//使用权限
        keyAttr[5] = USBKEY_ALL_PRIV;
    }

    if( 0 == ulAsymAlgID || USBKEY_TYPE_RSA == ulAsymAlgID )
    {
        //rsa加密导入或者明文导入对称密钥
        rtn = RSAImportSessionKey(hDeviceHandle, pcWrapKeyID, keyAttr, pcSymKey, ulSymKeyLen);
        if (rtn != 0)
        {
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_ImportContainerSymmetricKey ==> RSAImportSessionKey() Error,rtn = %#08x", rtn);
            return rtn;
        }
    }
    else
    {
        //ECC加密导入对称密钥
        rtn = ECCImportSessionKey(hDeviceHandle, pcWrapKeyID, keyAttr, pcSymKey, ulSymKeyLen);
        if (rtn != 0)
        {
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_ImportContainerSymmetricKey ==> ECCImportSessionKey() Error,rtn = %#08x", rtn);
            return rtn;
        }
    }

    // 把一个对称密钥增加到容器
    rtn = AddContainerSymKey(hDeviceHandle, ucSymKeyID, (USBKEY_UCHAR)recNO, pcConSymKeyIDRec);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportContainerSymmetricKey ==> AddContainerSymKey() Error,rtn = %#08x", rtn);
        return rtn;
    }

    //设置密钥ID
    *pcSymKeyID = 0x10;
    *(pcSymKeyID+1) = ucSymKeyID;

    AK_Log(AK_LOG_DEBUG, "USBKEY_ImportContainerSymmetricKey Success!");
    AK_UnlockMutex(gpMutex);
    return USBKEY_OK;
}

//	获取RSA公钥.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ExportRSAPublicKey(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcPubKeyID,	//[in]被导出公钥
        USBKEY_UCHAR_PTR pcPubKey,		//[out]返回的密钥
        USBKEY_ULONG_PTR pulPubKeyLen)	//[out]返回的密钥长度
{
    //USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[550];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0;
    USBKEY_UCHAR data[300],head1024[4]= {0x00, 0x00, 0x04, 0x00},head2048[4] = {0x00, 0x00, 0x08, 0x00};
    //USBKEY_UCHAR p1,p2,lc,le;
    //USBKEY_UCHAR p1,p2,lc;
    //USBKEY_INT32 le;

    AK_Log(AK_LOG_DEBUG, "USBKEY_ExportRSAPublicKey");

    /*if (strlen(pcAppName) > 16)
      {
      LogMessage("====>>USBKEY_ExportRSAPublicKey->pcAppName Length Error!");
      return USBKEY_PARAM_ERROR;
      }*/

    AK_LockMutex(gpMutex);

    rtn = SelectFile(hDeviceHandle,NULL,pcPubKeyID);	//Update by zhoushenshen 20100310
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ExportRSAPublicKey ==> SelectFile() Error,rtn = %#08x", rtn);
        return rtn;
    }

    /////////检查权限///////////////////////////////////////////////////
    /*rtn = CheckPrivillege(hDeviceHandle);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage16Val("====>>USBKEY_ExportRSAPublicKey->CheckPrivillege() Error,rtn = 0x",rtn);
      return rtn;
      }*/
    //////////////////////////////////////////////////////////////////////////


    //Delete by zhoushenshen in 20121109
    /*p1 = 0x00;
      p2 = 0x00;
      lc = 0;
      le = 0xFF;
      memset(data,0,sizeof(data));
      data[0] = 0x01;
      data[1] = 0x04;

      rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_ReadBinary,request,&reqLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_ExportRSAPublicKey ==> PackageData() Error,rtn = %#08x", rtn);
      return rtn;
      }

      respLen = 550;
      rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_ExportRSAPublicKey ==> DeviceExecCmd() Error,rtn = %#08x", rtn);
      return rtn;
      }
      rtn = *(response+respLen-2);
      rtn = rtn*256 + *(response+respLen-1);
      switch(rtn)
      {
      case DEVICE_SUCCESS:
      break;
      default:
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_ExportRSAPublicKey ==> DeviceExecCmd() Error,code = %#08x", rtn);
      return rtn;
      break;
      }*/
    //Del end

    //Add by zhoushenshen in 20121109
    /*le = 260;
      memset(data,0,sizeof(data));

      respLen = 550;
      rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_ReadBinary,response,&respLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_ExportRSAPublicKey ==> PackageData() Error, rtn = %#08x", rtn);
      return rtn;
      }*/
    //Add end

    respLen = 260;
    rtn = ReadCurBinaryFile(hDeviceHandle,0,respLen,response);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ExportRSAPublicKey ==> ReadCurBinaryFile() Error,rtn = %#08x",rtn);
        return rtn;
    }

    if (memcmp(response,head2048,4) == 0)
    {
        memset(data,0,sizeof(data));
        //Delete by zhoushenshen in 20121109
        /*data[0] = 0x02;
          data[1] = 0x04;

          rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_ReadBinary,request,&reqLen);
          if (rtn != 0)
          {
          AK_UnlockMutex(gpMutex);
          AK_Log(AK_LOG_ERROR, "USBKEY_ExportRSAPublicKey ==> PackageData() 2048 Error,rtn = %#08x", rtn);
          return rtn;
          }

          respLen = 550;
          rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
          if (rtn != 0)
          {
          AK_UnlockMutex(gpMutex);
          AK_Log(AK_LOG_ERROR, "USBKEY_ExportRSAPublicKey ==> DeviceExecCmd() 2048 Error,rtn = %#08x", rtn);
          return rtn;
          }
          rtn = *(response+respLen-2);
          rtn = rtn*256 + *(response+respLen-1);
          switch(rtn)
          {
          case DEVICE_SUCCESS:
          break;
          default:
          AK_UnlockMutex(gpMutex);
          AK_Log(AK_LOG_ERROR, "USBKEY_ExportRSAPublicKey ==> DeviceExecCmd() 2048 Error,code = %#08x", rtn);
          return rtn;
          break;
          }*/
        //Del end

        //Add by zhoushenshen in 20121109
        /*le = 516;

          respLen = 550;
          rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_ReadBinary,response,&respLen);
          if (rtn != 0)
          {
          AK_UnlockMutex(gpMutex);
          AK_Log(AK_LOG_ERROR, "USBKEY_ExportRSAPublicKey ==> PackageData() 2048 Error, rtn = %#08x", rtn);
          return rtn;
          }*/
        //Add end

        respLen = 516;
        rtn = ReadCurBinaryFile(hDeviceHandle,0,respLen,response);
        if (rtn != 0)
        {
            AK_Log(AK_LOG_ERROR, "USBKEY_ExportRSAPublicKey ==> ReadCurBinaryFile() 2048 Error,rtn = %#08x",rtn);
            return rtn;
        }
        //memcpy(pcPubKey,response,respLen-2);
        //*pulPubKeyLen = respLen - 2;
        memcpy(pcPubKey,response,respLen);
        *pulPubKeyLen = respLen;
    }
    else if (memcmp(response,head1024,4) == 0)
    {
        //memcpy(pcPubKey,response,respLen-2);
        //*pulPubKeyLen = respLen - 2;
        memcpy(pcPubKey,response,respLen);
        *pulPubKeyLen = respLen;
    }
    else
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ExportRSAPublicKey ==>The public key is not valid! not 1024 or 2048 bits!");
        return USBKEY_GENERAL_ERROR;
    }


    AK_Log(AK_LOG_DEBUG, "USBKEY_ExportRSAPublicKey Success!");
    AK_UnlockMutex(gpMutex);

    return USBKEY_OK;
}

//	导入RSA公钥.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ImportRSAPublicKey(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcContainerName,	//[in]密钥容器
        USBKEY_ULONG ulKeyUsage,		//[in]密钥用途，KEY_EXCHANGE密钥或者KEY_SIGNATURE密钥
        USBKEY_UCHAR_PTR pcRSAPublicKey,//[in]导入的密钥
        USBKEY_ULONG ulRSAPublicKeyLen,	//[in]导入的密钥长度
        USBKEY_UCHAR_PTR pcPubKeyID)	//[out]返回的公钥ID
{
    //USBKEY_UCHAR request[550];
    USBKEY_UCHAR response[550];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0,recNO,ulAccessCondition = USBKEY_USER_USE_PRIV,ulFileSize;
    USBKEY_UCHAR data[550],fileName[4];
    USBKEY_UCHAR p1,p2,lc,le;
    USBKEY_ULONG ulBitLen;

    AK_Log(AK_LOG_DEBUG, "USBKEY_ImportRSAPublicKey");

    /*if (strlen(pcAppName) > 16)
      {
      LogMessage("====>>USBKEY_ImportRSAPublicKey->pcAppName Length Error!");
      return USBKEY_PARAM_ERROR;
      }*/
    if (strlen((char *)pcContainerName) > 254)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAPublicKey ==> pcContainerName Length Error,Max Name Len is 255!");
        return USBKEY_PARAM_ERROR;
    }

    if ((ulKeyUsage != KEY_EXCHANGE) && (ulKeyUsage != KEY_SIGNATURE) )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAPublicKey ==>ulKeyUsage Error,ulKeyUsage = %#08x", ulKeyUsage);
        return USBKEY_PARAM_ERROR;
    }

    if ( ((ulRSAPublicKeyLen == 260) && (pcRSAPublicKey[2] != 0x04) ) ||
            ((ulRSAPublicKeyLen == 516) && (pcRSAPublicKey[2] != 0x08) ) )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAPublicKey ==>pcRSAPublicKey bits Error,rtn = %#08x", USBKEY_PARAM_ERROR);
        return USBKEY_PARAM_ERROR;
    }

    AK_LockMutex(gpMutex);

    if(ulRSAPublicKeyLen == 260)
        ulBitLen = 1024;
    else if(ulRSAPublicKeyLen == 516)	//Add by zhoushenshen in 20111103
        ulBitLen = 2048;
    else
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAPublicKey ==>ulRSAPublicKeyLen Error,ulRSAPublicKeyLen = %#08x", ulRSAPublicKeyLen);
        return USBKEY_PARAM_ERROR;
    }
    //Add end

    /*rtn = SelectFile(hDeviceHandle,pcAppName,NULL);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage16Val("====>>USBKEY_ImportRSAPublicKey->SelectFile() Error, rtn = 0x",rtn);
      return rtn;
      }*/

    /////////检查权限///////////////////////////////////////////////////
    /*rtn = CheckPrivillege(hDeviceHandle);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage16Val("====>>USBKEY_ImportRSAPublicKey->CheckPrivillege() Error,rtn = 0x",rtn);
      return rtn;
      }*/
    //////////////////////////////////////////////////////////////////////////
    rtn = SearchContainerRecNo(hDeviceHandle,pcContainerName,&recNO);
    if( (rtn != 0) || (recNO == 0) )
    {
        AK_UnlockMutex(gpMutex);
        if(recNO == 0)
        {
            AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAPublicKey ==> not find container record.");
            return USBKEY_RECORD_NOT_FOUND;
        }
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAPublicKey ==> SearchContainerRecNo() Error,rtn = %#08x", rtn);
        return rtn;
    }

    ////////
    //生成文件名
    ////////
    memcpy(fileName,&recNO,1);
    memcpy(fileName+2,&recNO,1);
    if (ulKeyUsage == KEY_EXCHANGE)
    {
        memset(fileName+1,0x01,1);
        memset(fileName+3,0x02,1);
    }
    else if (ulKeyUsage == KEY_SIGNATURE)
    {
        memset(fileName+1,0x04,1);
        memset(fileName+3,0x05,1);
    }

    //创建公私钥文件
    p1 = 0x00;
    p2 = 0x10;
    //lc = 9;
    lc = 0x07;						//lc updtae in 20120106
    le = 0x00;

    memset(data,0,9);
    memcpy(data,fileName,2);
    le = (USBKEY_UCHAR)ulAccessCondition;
    data[3] = USBKEY_ALL_PRIV;
    memcpy(data+4,&le,1);
    //updtae in 20120106 文件长度由4字节改为2字节
    ulFileSize = 516;
    le = (USBKEY_UCHAR)(ulFileSize/256);
    //memcpy(data+7,&le,1);
    memcpy(data+5,&le,1);
    le = (USBKEY_UCHAR)ulFileSize;
    //memcpy(data+8,&le,1);
    memcpy(data+6,&le,1);

    //Delete by zhoushenshen in 20121107
    /*rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_CreateFile,request,&reqLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAPublicKey ==> PackageData() Error,rtn = %#08x", rtn);
      return rtn;
      }

      respLen = 550;
      rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAPublicKey ==> DeviceExecCmd() Error,rtn = %#08x", rtn);
      return rtn;
      }*/
    //Del end

    //Add by zhoushenshen in 20121107
    respLen = 550;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_CreateFile,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAPublicKey ==> PackageData() Error, rtn = %#08x", rtn);
        return rtn;
    }
    //Add end

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        case DEVICE_CMD_DATA_ERROR:
            AK_Log(AK_LOG_DEBUG, "USBKEY_ImportRSAPublicKey Public Key File Exist.");
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAPublicKey ==> DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
            break;
    }

    AK_Log(AK_LOG_DEBUG, "USBKEY_ImportRSAPublicKey Create Public Key File Success.");
    /*
       p1 = 0x00;
       p2 = 0x11;
       lc = 9;
       le = 0x00;

       memset(data,0,9);
       memcpy(data,fileName+2,2);
       data[2] = 0x00;
       le = ulAccessCondition;
       memcpy(data+3,&le,1);
       memcpy(data+4,&le,1);
       ulFileSize = 1412;
       le = ulFileSize/256;
       memcpy(data+7,&le,1);
       le = ulFileSize;
       memcpy(data+8,&le,1);

       rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_CreateFile,request,&reqLen);
       if (rtn != 0)
       {
       LogMessage16Val("====>>USBKEY_ImportRSAPublicKey->PackageData() Error,rtn = 0x",rtn);
       return rtn;
       }

       respLen = 550;
       rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,&respLen);
       if (rtn != 0)
       {
       LogMessage16Val("====>>USBKEY_ImportRSAPublicKey->DeviceExecCmd() Error,rtn = 0x",rtn);
       return rtn;
       }

       rtn = *(response+respLen-2);
       rtn = rtn*256 + *(response+respLen-1);
       switch(rtn)
       {
       case DEVICE_SUCCESS:
       break;
       case DEVICE_CMD_DATA_ERROR:
       LogMessage16Val("====>>USBKEY_ImportRSAPublicKey->Public Key File is Exist,code = 0x",rtn);
       break;
       default:
       LogMessage16Val("====>>USBKEY_ImportRSAPublicKey->DeviceExecCmd() Error,code = 0x",rtn);
       return rtn;
       break;
       }

       LogMessage("USBKEY_ImportRSAPublicKey Create Private Key File Success.");
     */

    //写入公钥
    /*rtn = SelectFile(hDeviceHandle, pcAppName, fileName);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage16Val("====>>USBKEY_ImportRSAPublicKey->SelectFile() Error,rtn = 0x",rtn);
      return rtn;
      }*/

    /*
       p1 = 0x00;
       p2 = 0x00;
       lc = 0xFF;
       le = 0x00;
       if (ulRSAPublicKeyLen = 260)
       data[0] = 0x01;
       else
       data[0] = 0x02;
       data[1] = 0x04;

       memcpy(data+2,pcRSAPublicKey,ulRSAPublicKeyLen);

       rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_UpdatePublicKeyBinary,request,&reqLen);
       if (rtn != 0)
       {
       LogMessageVal("====>>USBKEY_ImportRSAPublicKey->PackageData() Error,rtn = ",rtn);
       return rtn;
       }

       respLen = 550;
       rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,&respLen);
       if (rtn != 0)
       {
       LogMessageVal("====>>USBKEY_ImportRSAPublicKey->DeviceExecCmd() Error,rtn = ",rtn);
       return rtn;
       }

       rtn = *(response+respLen-2);
       rtn = rtn*256 + *(response+respLen-1);
       switch(rtn)
       {
       case DEVICE_SUCCESS:
       break;
       default:
       LogMessage16Val("====>>USBKEY_ImportRSAPublicKey->DeviceExecCmd() Error,code = 0x",rtn);
       return rtn;
       break;
       }
    //*/
    rtn = WritePublicKey(hDeviceHandle,fileName,0,pcRSAPublicKey,&ulRSAPublicKeyLen);		//Update by zhoushenshen 20100310
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAPublicKey ==> WritePublicKey() Error,rtn = %#08x", rtn);
        return rtn;
    }

    ///////
    //赋值返回ID
    memcpy(pcPubKeyID,fileName,2);

    rtn = AddContainerKeySep(hDeviceHandle,recNO,ulKeyUsage, USBKEY_RSA_PUBLICKEY,ulBitLen);	//Update by zhoushenshen 20100310
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAPublicKey ==> AddContainerKeySep() Error,rtn = %#08x", rtn);
        return rtn;
    }

    AK_Log(AK_LOG_DEBUG, "USBKEY_ImportRSAPublicKey Success!");
    AK_UnlockMutex(gpMutex);

    return USBKEY_OK;
}

/*//	导入RSA私钥.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ImportRSAPrivateKey(
USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
USBKEY_UCHAR_PTR pcAppName,		//[in]应用名称
USBKEY_UCHAR_PTR pcWrapKeyID,	//[in]用来保护导入密钥的密钥ID
USBKEY_UCHAR_PTR pcContainerName,	//[in]密钥容器
USBKEY_ULONG ulKeyUsage,		//[in]密钥用途，KEY_EXCHANGE密钥或者KEY_SIGNATURE密钥
USBKEY_UCHAR_PTR pcRSAKeyPair,	//[in]导入的密钥
USBKEY_ULONG ulRSAKeyPairLen,	//[in]导入的密钥长度
USBKEY_UCHAR_PTR pcPriKeyID)	//[out]返回的私钥ID
{
USBKEY_UCHAR request[1500];
USBKEY_UCHAR response[1500];
USBKEY_ULONG rtn,reqLen,respLen=0,recNO,ulAccessCondition = USBKEY_USER_PRIV,ulFileSize;
USBKEY_UCHAR data[1500],fileName[4];
USBKEY_UCHAR p1,p2,lc,le;

LogMessage("->USBKEY_ImportRSAPrivateKey");

if (strlen(pcAppName) > 16)
{
LogMessage("====>>USBKEY_ImportRSAPrivateKey->pcAppName Length Error!");
return USBKEY_PARAM_ERROR;
}
if (strlen(pcContainerName) > 254)
{
LogMessage("====>>USBKEY_ImportRSAPrivateKey->pcContainerName Length Error!");
return USBKEY_PARAM_ERROR;
}

if ((ulKeyUsage != KEY_EXCHANGE) && (ulKeyUsage != KEY_SIGNATURE) )
{
LogMessageVal("====>>USBKEY_ImportRSAPrivateKey->ulKeyUsage Error,ulKeyUsage = ", ulKeyUsage);
return USBKEY_PARAM_ERROR;
}

AK_LockMutex(gpMutex);

rtn = SelectFile(hDeviceHandle,pcAppName,NULL);
if (rtn != 0)
{
AK_UnlockMutex(gpMutex);
LogMessage16Val("====>>USBKEY_ImportRSAPrivateKey->SelectFile() Error, rtn = 0x",rtn);
return USBKEY_GENERAL_ERROR;
}

/////////检查权限///////////////////////////////////////////////////
//	rtn = CheckPrivillege(hDeviceHandle);
//	if (rtn != 0)
//	{
//		AK_UnlockMutex(gpMutex);
//		LogMessage16Val("====>>USBKEY_ImportRSAPrivateKey->CheckPrivillege() Error,rtn = 0x",rtn);
//		return rtn;
//	}
//////////////////////////////////////////////////////////////////////////

rtn = SearchContainerRecNo(hDeviceHandle,pcContainerName,&recNO);
if( (rtn != 0) || (recNO == 0) )
{
AK_UnlockMutex(gpMutex);
LogMessage16Val("====>>USBKEY_ImportRSAPrivateKey->SearchContainerRecNo() Error, rtn = 0x",rtn);
return USBKEY_GENERAL_ERROR;
}

////////
//生成文件名
////////
memcpy(fileName,&recNO,1);
memcpy(fileName+2,&recNO,1);
if (ulKeyUsage == KEY_EXCHANGE)
{
    memset(fileName+1,0x01,1);
    memset(fileName+3,0x02,1);
}
else if (ulKeyUsage == KEY_SIGNATURE)
{
    memset(fileName+1,0x04,1);
    memset(fileName+3,0x05,1);
}

//创建公私钥文件

//	p1 = 0x00;
//	p2 = 0x10;
//	lc = 9;
//	le = 0x00;
//
//	memset(data,0,9);
//	memcpy(data,fileName,2);
//	le = ulAccessCondition;
//	memcpy(data+3,&le,1);
//	memcpy(data+4,&le,1);
//	ulFileSize = 516;
//	le = ulFileSize/256;
//	memcpy(data+7,&le,1);
//	le = ulFileSize;
//	memcpy(data+8,&le,1);
//
//	rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_CreateFile,request,&reqLen);
//	if (rtn != 0)
//	{
//		LogMessage16Val("====>>USBKEY_ImportRSAPrivateKey->PackageData() Error,rtn = 0x",rtn);
//		return rtn;
//	}
//
//	respLen = 1500;
//	rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,&respLen);
//	if (rtn != 0)
//	{
//		LogMessage16Val("====>>USBKEY_ImportRSAPrivateKey->DeviceExecCmd() Error,rtn = 0x",rtn);
//		return rtn;
//	}
//
//	rtn = *(response+respLen-2);
//	rtn = rtn*256 + *(response+respLen-1);
//	switch(rtn)
//	{
//	case DEVICE_SUCCESS:
//		break;
//	case DEVICE_CMD_DATA_ERROR:
//		LogMessage16Val("====>>USBKEY_ImportRSAPrivateKey->Public Key File is Exist,code = 0x",rtn);
//		break;
//	default:
//		LogMessage16Val("====>>USBKEY_ImportRSAPrivateKey->DeviceExecCmd() Error,code = 0x",rtn);
//		return rtn;
//		break;
//	}
//
//	LogMessage("USBKEY_ImportRSAPrivateKey Create Public Key File Success.");

p1 = 0x00;
p2 = 0x11;
lc = 9;
le = 0x00;

memset(data,0,9);
memcpy(data,fileName+2,2);
data[2] = 0x00;
le = ulAccessCondition;
memcpy(data+3,&le,1);
memcpy(data+4,&le,1);
ulFileSize = 1412;
le = ulFileSize/256;
memcpy(data+7,&le,1);
le = ulFileSize;
memcpy(data+8,&le,1);

rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_CreateFile,request,&reqLen);
if (rtn != 0)
{
    AK_UnlockMutex(gpMutex);
    LogMessage16Val("====>>USBKEY_ImportRSAPrivateKey->PackageData() Error,rtn = 0x",rtn);
    return rtn;
}

respLen = 1500;
rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,&respLen);
if (rtn != 0)
{
    AK_UnlockMutex(gpMutex);
    LogMessage16Val("====>>USBKEY_ImportRSAPrivateKey->DeviceExecCmd() Error,rtn = 0x",rtn);
    return rtn;
}

rtn = *(response+respLen-2);
rtn = rtn*256 + *(response+respLen-1);
switch(rtn)
{
    case DEVICE_SUCCESS:
        break;
    case DEVICE_CMD_DATA_ERROR:
        LogMessage16Val("====>>USBKEY_ImportRSAPrivateKey->Public Key File is Exist,code = 0x",rtn);
        break;
    default:
        AK_UnlockMutex(gpMutex);
        LogMessage16Val("====>>USBKEY_ImportRSAPrivateKey->DeviceExecCmd() Error,code = 0x",rtn);
        return rtn;
        break;
}

LogMessage("USBKEY_ImportRSAPrivateKey Create Private Key File Success.");

//		//写入公钥
//		rtn = SelectFile(hDeviceHandle, pcAppName, fileName);
//		if (rtn != 0)
//		{
//			LogMessageVal("====>>USBKEY_ImportRSAPrivateKey->SelectFile() Error,rtn = ",rtn);
//			return rtn;
//		}
//	
//		p1 = 0x00;
//		p2 = 0x00;
//		lc = 0xFF;
//		le = 0x00;
//		if (ulRSAKeyPairLen = 708)
//		{
//			data[0] = 0x01;
//			data[1] = 0x04;
//			memcpy(data+2,pcRSAKeyPair,260);
//		}
//		else
//		{
//			data[0] = 0x02;
//			data[1] = 0x04;
//			memcpy(data+2,pcRSAKeyPair,516);
//		}
//
//		rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_UpdatePublicKeyBinary,request,&reqLen);
//		if (rtn != 0)
//		{
//			LogMessageVal("====>>USBKEY_ImportRSAPrivateKey->PackageData() Error,rtn = ",rtn);
//			return rtn;
//		}
//	
//		respLen = 1500;
//		rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,&respLen);
//		if (rtn != 0)
//		{
//			LogMessageVal("====>>USBKEY_ImportRSAPrivateKey->DeviceExecCmd() Error,rtn = ",rtn);
//			return rtn;
//		}
//	
//		rtn = *(response+respLen-2);
//		rtn = rtn*256 + *(response+respLen-1);
//		switch(rtn)
//		{
//		case DEVICE_SUCCESS:
//			break;
//		default:
//			LogMessage16Val("====>>USBKEY_ImportRSAPrivateKey->DeviceExecCmd() Error,code = 0x",rtn);
//			return rtn;
//			break;
//		}
//		LogMessage("USBKEY_ImportRSAPrivateKey Write Public Key Success.");

//写入私钥
rtn = SelectFile(hDeviceHandle, pcAppName, fileName+2);
if (rtn != 0)
{
    AK_UnlockMutex(gpMutex);
    LogMessage16Val("====>>USBKEY_ImportRSAPrivateKey->SelectFile() Error,rtn = 0x",rtn);
    return rtn;
}

p1 = 0x00;
p2 = 0x00;
lc = 0xFF;
le = 0x00;
le = ulRSAKeyPairLen/256;
memcpy(data,&le,1);
le = ulRSAKeyPairLen;
memcpy(data+1,&le,1);

memcpy(data+2,pcRSAKeyPair,ulRSAKeyPairLen);

if (pcWrapKeyID == NULL)
    rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_UpdatePrivateKeyBinaryPlain,request,&reqLen);
else if (pcWrapKeyID[0] == 0x10)
    rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_UpdatePrivateKeyBinary,request,&reqLen);
else if (pcWrapKeyID[0] == 0x11)
    rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_UpdatePrivateKeyBinaryTemp,request,&reqLen);
    else
{
    AK_UnlockMutex(gpMutex);
    LogMessage("====>>USBKEY_ImportRSAPrivateKey->pcWrapKeyID Error!");
    return USBKEY_PARAM_ERROR;
}
if (rtn != 0)
{
    AK_UnlockMutex(gpMutex);
    LogMessage16Val("====>>USBKEY_ImportRSAPrivateKey->PackageData() Error,rtn = 0x",rtn);
    return rtn;
}

respLen = 1500;
rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,&respLen);
if (rtn != 0)
{
    AK_UnlockMutex(gpMutex);
    LogMessage16Val("====>>USBKEY_ImportRSAPrivateKey->DeviceExecCmd() Error,rtn = 0x",rtn);
    return rtn;
}

rtn = *(response+respLen-2);
rtn = rtn*256 + *(response+respLen-1);
switch(rtn)
{
    case DEVICE_SUCCESS:
        break;
    default:
        AK_UnlockMutex(gpMutex);
        LogMessage16Val("====>>USBKEY_ImportRSAPrivateKey->DeviceExecCmd() Error,code = 0x",rtn);
        return rtn;
        break;
}

//
//	rtn = WriteProtectedPrivateKey(hDeviceHandle,pcAppName,fileName,0,pcRSAKeyPair,&ulRSAKeyPairLen);
//	if (rtn != 0)
//	{
//		LogMessageVal("====>>USBKEY_ImportRSAPrivateKey->USBKEY_WriteFile() Error,rtn = ",rtn);
//		return rtn;
//	}
//
//	LogMessage("USBKEY_ImportRSAPrivateKey Write Private Key Success.");

///////
//赋值返回ID
memcpy(pcPriKeyID,fileName+2,2);

rtn = AddContainerKeySep(hDeviceHandle,pcAppName,recNO,ulKeyUsage, USBKEY_RSA_PRIVATEKEY);
AK_UnlockMutex(gpMutex);
if (rtn != 0)
{
    LogMessage16Val("====>>USBKEY_ImportRSAPrivateKey->AddContainerKeySep() Error,rtn = 0x",rtn);
    return rtn;
}

LogMessage("->USBKEY_ImportRSAPrivateKey Success.");

return USBKEY_OK;
}*/

//	导入RSA私钥.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ImportRSAPrivateKey(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_ULONG ulKeyType,			//[in]私钥类型：USBKEY_PRIKEY_TYPE_CSP,USBKEY_PRIKEY_TYPE_AI
        USBKEY_UCHAR_PTR pcWrapKeyID,	//[in]用来保护导入密钥的密钥ID
        USBKEY_ULONG ulMode,			//[in]解密模式：USBKEY_ECB|USBKEY_PKCS5_PADDING, USBKEY_CBC|USBKEY_PKCS5_PADDING,c若无保护密钥ID，设为ECB模式
        USBKEY_UCHAR_PTR pcIV,			//[in]初始化向量，如果是ecb模式，设置为NULL
        USBKEY_ULONG ulIVLen,			//[in]初始化向量长度，如果是ecb模式，设置为0
        USBKEY_UCHAR_PTR pcContainerName,//[in]密钥容器
        USBKEY_ULONG ulKeyUsage,		//[in]密钥用途，KEY_EXCHANGE密钥或者KEY_SIGNATURE密钥
        USBKEY_UCHAR_PTR pcRSAKeyPair,	//[in]导入的密钥
        USBKEY_ULONG ulRSAKeyPairLen,	//[in]导入的密钥长度
        USBKEY_UCHAR_PTR pcPriKeyID)	//[out]返回的私钥ID
{
    //USBKEY_UCHAR request[1500];
    USBKEY_UCHAR response[300];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0,recNO,ulAccessCondition = USBKEY_USER_USE_PRIV,ulFileSize;
    USBKEY_UCHAR data[1500],fileName[4];
    //USBKEY_UCHAR p1,p2,lc,le;
    USBKEY_UCHAR p1,p2,le;
    USBKEY_INT32 lc;
    USBKEY_ULONG ulEncMode;
    USBKEY_ULONG ulPadMode;
    USBKEY_ULONG ulBitLen;
    USBKEY_ULONG loop = 0,offset,ulPriLen,i;

    AK_Log(AK_LOG_DEBUG, "USBKEY_ImportRSAPrivateKey");

    /*if (strlen(pcAppName) > 16)
      {
      LogMessage("====>>USBKEY_ImportRSAPrivateKey->pcAppName Length Error!");
      return USBKEY_PARAM_ERROR;
      }*/
    if (strlen((char *)pcContainerName) > 254)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAPrivateKey ==> pcContainerName Length Error,Max Name Len is 255!");
        return USBKEY_PARAM_ERROR;
    }

    if ((ulKeyUsage != KEY_EXCHANGE) && (ulKeyUsage != KEY_SIGNATURE) )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAPrivateKey ==> ulKeyUsage Error,ulKeyUsage = %#08x", ulKeyUsage);
        return USBKEY_PARAM_ERROR;
    }
    if( USBKEY_PRIKEY_TYPE_AI != ulKeyType && USBKEY_PRIKEY_TYPE_CSP != ulKeyType )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAPrivateKey ==> ulKeyType Error,ulKeyType = %#08x", ulKeyType);
        return USBKEY_PARAM_ERROR;
    }

    //检查加密模式,填充模式的合法性
    ulEncMode = ulMode & 0xf0;
    ulPadMode = ulMode & 0x0f;

    if(pcWrapKeyID && USBKEY_PKCS5_PADDING != ulPadMode)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAPrivateKey ==> ulMode Error,ulPadMode = %#08x", ulPadMode);
        return USBKEY_PARAM_ERROR;
    }
    if(pcWrapKeyID && ulEncMode != USBKEY_ECB && ulEncMode != USBKEY_CBC)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAPrivateKey ==> ulMode Error,ulEncMode = %#08x", ulEncMode);
        return USBKEY_PARAM_ERROR;
    }

    if(pcWrapKeyID && ulEncMode == USBKEY_CBC && ulIVLen != 8 && ulIVLen != 16)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAPrivateKey ==> ulIVLen Error,ulIVLen = %#08x", ulIVLen);
        return USBKEY_PARAM_ERROR;
    }

    AK_LockMutex(gpMutex);

    /*rtn = SelectFile(hDeviceHandle,pcAppName,NULL);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage16Val("====>>USBKEY_ImportRSAPrivateKey->SelectFile() Error, rtn = 0x",rtn);
      return rtn;
      }*/

    rtn = SearchContainerRecNo(hDeviceHandle,pcContainerName,&recNO);
    if( (rtn != 0) || (recNO == 0) )
    {
        AK_UnlockMutex(gpMutex);
        if(recNO == 0)
        {
            AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAPrivateKey ==> not find container record.");
            return USBKEY_RECORD_NOT_FOUND;
        }
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAPrivateKey ==> SearchContainerRecNo() Error,rtn = %#08x", rtn);
        return rtn;
    }

    ////////
    //生成文件名
    ////////
    memcpy(fileName,&recNO,1);
    memcpy(fileName+2,&recNO,1);
    if (ulKeyUsage == KEY_EXCHANGE)
    {
        memset(fileName+1,0x01,1);
        memset(fileName+3,0x02,1);
    }
    else if (ulKeyUsage == KEY_SIGNATURE)
    {
        memset(fileName+1,0x04,1);
        memset(fileName+3,0x05,1);
    }

    //创建私钥文件
    p1 = 0x00;
    p2 = 0x11;
    //lc = 9;
    lc = 0x07;						//lc updtae in 20120106
    le = 0x00;

    memset(data,0,9);
    memcpy(data,fileName+2,2);
    data[2] = 0x00;
    le = (USBKEY_UCHAR)ulAccessCondition;
    memcpy(data+3,&le,1);
    memcpy(data+4,&le,1);
    //updtae in 20120106 文件长度由4字节改为2字节
    ulFileSize = 1412;
    le = (USBKEY_UCHAR)(ulFileSize/256);
    //memcpy(data+7,&le,1);
    memcpy(data+5,&le,1);
    le = (USBKEY_UCHAR)ulFileSize;
    //memcpy(data+8,&le,1);
    memcpy(data+6,&le,1);

    //Delete by zhoushenshen in 20121107
    /*rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_CreateFile,request,&reqLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAPrivateKey ==> PackageData() Error,rtn = %#08x", rtn);
      return rtn;
      }

      respLen = 300;
      rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAPrivateKey ==> DeviceExecCmd() Error,rtn = %#08x", rtn);
      return rtn;
      }*/
    //Del end

    //Add by zhoushenshen in 20121107
    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_CreateFile,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAPrivateKey ==> PackageData() Error, rtn = %#08x", rtn);
        return rtn;
    }
    //Add end

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        case DEVICE_CMD_DATA_ERROR:
            AK_Log(AK_LOG_DEBUG, "USBKEY_ImportRSAPrivateKey Private Key File Exist.");
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAPrivateKey ==> DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
            break;
    }
    AK_Log(AK_LOG_DEBUG, "USBKEY_ImportRSAPrivateKey Create Private Key File Success.");

    //写入私钥
    if(pcWrapKeyID == NULL)
    {
        //p1 = 0x00;
        //P1的b7位表示是否有后续命令数据：0：无后续命令数据，1：有后续命令数据
        p1 = 0x00| 0x80;
    }
    else if (pcWrapKeyID[0] == 0x10)
    {
        //p1 = 0x07+(USBKEY_UCHAR)ulEncMode;
        //P1的b3~b0位表示密钥类型,P1的b4位表示加密模式：1：CBC, 0：ECB
        p1 = (0x07| 0x80) +(USBKEY_UCHAR)ulEncMode;
    }
    else if (pcWrapKeyID[0] == 0x11)
    {
        //p1 = 0x08+(USBKEY_UCHAR)ulEncMode;
        //P1的b3~b0位表示密钥类型,P1的b4位表示加密模式：1：CBC, 0：ECB
        p1 = (0x08| 0x80) +(USBKEY_UCHAR)ulEncMode;
    }
    else
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAPrivateKey ==> pcWrapKeyID Error,pcWrapKeyID = %#08x", pcWrapKeyID[0]);
        return USBKEY_PARAM_ERROR;
    }

    if(pcWrapKeyID == NULL)
        p2 = 0;
    else
        p2 = pcWrapKeyID[1];

    //Delete by zhoushenshen in 20121204
    /*lc = 0xFF;
      le = 0x00;
      if(ulEncMode == USBKEY_CBC)
      {
      le = (USBKEY_UCHAR)((ulRSAKeyPairLen+4+ulIVLen)/256);
      memcpy(data,&le,1);
      le = (USBKEY_UCHAR)(ulRSAKeyPairLen+4+ulIVLen);
      memcpy(data+1,&le,1);
      }
      else
      {
      le = (USBKEY_UCHAR)((ulRSAKeyPairLen+4)/256);
      memcpy(data,&le,1);
      le = (USBKEY_UCHAR)(ulRSAKeyPairLen+4);
      memcpy(data+1,&le,1);
      }

    //写入密钥属性
    if( USBKEY_PRIKEY_TYPE_AI == ulKeyType )
    {
    data[2] = 0x00;		//自定义
    if(ulRSAKeyPairLen >= 324 && ulRSAKeyPairLen < 360)
    ulBitLen = 1024;
    else if(ulRSAKeyPairLen >= 644 && ulRSAKeyPairLen < 664)
    ulBitLen = 2048;
    else
    ulBitLen = 0;
    }
    else
    {
    data[2] = 0x01;		//CSP私钥密文格式
    if(ulRSAKeyPairLen >= 588 && ulRSAKeyPairLen < 800)
    ulBitLen = 1024;
    else if(ulRSAKeyPairLen >= 1164 && ulRSAKeyPairLen < 1200)
    ulBitLen = 2048;
    else
    ulBitLen = 0;
    }
    data[3] = 0x00;		//RSA密钥
    memcpy(data+4,fileName+2,2);	//私钥ID

    //检查IV指针合法性
    if( ulEncMode == USBKEY_CBC )
    {
    if(NULL == pcIV)
    {
    AK_UnlockMutex(gpMutex);
    AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAPrivateKey ==> pcIV is NULL,rtn = %#08x", USBKEY_PARAM_ERROR);
    return USBKEY_PARAM_ERROR;
    }
    else
    {
    memcpy(data+6, pcIV, ulIVLen);
    memcpy(data+6+ulIVLen,pcRSAKeyPair,ulRSAKeyPairLen);
    }
    }
    else
    memcpy(data+6,pcRSAKeyPair,ulRSAKeyPairLen);

    rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_ImportPrivateKey,request,&reqLen);
    if (rtn != 0)
    {
    AK_UnlockMutex(gpMutex);
    AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAPrivateKey ==> PackageData() Import PriKey Error,rtn = %#08x", rtn);
    return rtn;
    }
    respLen = 300;
    rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
    if (rtn != 0)
    {
    AK_UnlockMutex(gpMutex);
    AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAPrivateKey ==> DeviceExecCmd() Import PriKey Error,rtn = %#08x", rtn);
    return rtn;
}*/
//Del end

//Add by zhoushenshen in 20121204
if(ulEncMode == USBKEY_CBC)
    ulPriLen = ulRSAKeyPairLen+4+ulIVLen;
    else
    ulPriLen = ulRSAKeyPairLen+4;

    //写入密钥属性
if( USBKEY_PRIKEY_TYPE_AI == ulKeyType )
{
    data[0] = 0x00;		//自定义
    if(ulRSAKeyPairLen >= 324 && ulRSAKeyPairLen < 360)
        ulBitLen = 1024;
    else if(ulRSAKeyPairLen >= 644 && ulRSAKeyPairLen < 664)
        ulBitLen = 2048;
    else
        ulBitLen = 0;
}
else
{
    data[0] = 0x01;		//CSP私钥密文格式
    if(ulRSAKeyPairLen >= 588 && ulRSAKeyPairLen < 800)
        ulBitLen = 1024;
    else if(ulRSAKeyPairLen >= 1164 && ulRSAKeyPairLen < 1200)
        ulBitLen = 2048;
    else
        ulBitLen = 0;
}
data[1] = 0x00;		//RSA密钥
memcpy(data+2,fileName+2,2);	//私钥ID

//检查IV指针合法性
if( ulEncMode == USBKEY_CBC )
{
    if(NULL == pcIV)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAPrivateKey ==> pcIV is NULL,rtn = %#08x", USBKEY_PARAM_ERROR);
        return USBKEY_PARAM_ERROR;
    }
    else
    {
        memcpy(data+4, pcIV, ulIVLen);
        memcpy(data+4+ulIVLen,pcRSAKeyPair,ulRSAKeyPairLen);
    }
}
else
memcpy(data+4,pcRSAKeyPair,ulRSAKeyPairLen);

//循环导入，若正好为写入块倍数，则循环数减1，最后处理块的P1与前面快不同
if(ulPriLen % USBKEY_FILE_MAX_BLOCK_LEN)
    loop = ulPriLen/USBKEY_FILE_MAX_BLOCK_LEN;
    else
    loop = ulPriLen/USBKEY_FILE_MAX_BLOCK_LEN - 1;
    offset = 0;
    for (i=0; i<loop; i++)
{
    le = 0x00;
    lc = USBKEY_FILE_MAX_BLOCK_LEN;

    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data+i*USBKEY_FILE_MAX_BLOCK_LEN,le,USBKEY_OP_ImportPrivateKey,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAPrivateKey ==> PackageData() Import prikey loop Error, rtn = %#08x", rtn);
        return rtn;
    }

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAPrivateKey ==> DeviceExecCmd() Import PriKey loop Error,code = %#08x", rtn);
            return rtn;
    }
    //memcpy(data,pcData+i*USBKEY_FILE_MAX_BLOCK_LEN,USBKEY_FILE_MAX_BLOCK_LEN);
}

//最后一次导入
lc = ulPriLen - loop*USBKEY_FILE_MAX_BLOCK_LEN;
le = 0x00;
p1 = p1 & 0x7f;

respLen = 300;
rtn = PackageData(hDeviceHandle,p1,p2,lc,data+loop*USBKEY_FILE_MAX_BLOCK_LEN,le,USBKEY_OP_ImportPrivateKey,response,&respLen);
if (rtn != 0)
{
    AK_UnlockMutex(gpMutex);
    AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAPrivateKey ==> PackageData() Import prikey Error, rtn = %#08x", rtn);
    return rtn;
}
//Add end

rtn = *(response+respLen-2);
rtn = rtn*256 + *(response+respLen-1);
switch(rtn)
{
    case DEVICE_SUCCESS:
        break;
    default:
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAPrivateKey ==> DeviceExecCmd() Import PriKey Error,code = %#08x", rtn);
        return rtn;
        break;
}
AK_Log(AK_LOG_DEBUG, "USBKEY_ImportRSAPrivateKey Import Private Key Success.");

///////
//赋值返回ID
memcpy(pcPriKeyID,fileName+2,2);

rtn = AddContainerKeySep(hDeviceHandle,recNO,ulKeyUsage, USBKEY_RSA_PRIVATEKEY,ulBitLen);	//Update by zhoushenshen 20100310
if (rtn != 0)
{
    AK_UnlockMutex(gpMutex);
    AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAPrivateKey ==>AddContainerKeySep() Error,rtn = %#08x", rtn);
    return rtn;
}

AK_Log(AK_LOG_DEBUG, "USBKEY_ImportRSAPrivateKey Success!");
AK_UnlockMutex(gpMutex);
return USBKEY_OK;
}


//	导入RSA公私钥对.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ImportRSAKeyPair(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcWrapKeyID,	//[in]用来保护导入密钥的密钥ID
        USBKEY_ULONG ulMode,			//[in]解密模式：USBKEY_ECB, USBKEY_CBC(必须为带填充),c若无保护密钥ID，设为ECB模式
        USBKEY_UCHAR_PTR pcIV,			//[in]初始化向量，如果是ecb模式，设置为NULL
        USBKEY_ULONG ulIVLen,			//[in]初始化向量长度，如果是ecb模式，设置为0
        USBKEY_UCHAR_PTR pcContainerName,//[in]密钥容器
        USBKEY_ULONG ulKeyUsage,		//[in]密钥用途，KEY_EXCHANGE密钥或者KEY_SIGNATURE密钥
        USBKEY_UCHAR_PTR pcRSAKeyPair,	//[in]导入的密钥
        USBKEY_ULONG ulRSAKeyPairLen,	//[in]导入的密钥长度
        USBKEY_UCHAR_PTR pcPriKeyID,	//[out]返回的私钥ID
        USBKEY_UCHAR_PTR pcPubKeyID)	//[out]返回的公钥ID
{
    //USBKEY_UCHAR request[1500];
    USBKEY_UCHAR response[300];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0,recNO,ulAccessCondition = USBKEY_USER_USE_PRIV,ulFileSize;
    USBKEY_UCHAR data[1500],fileName[4];
    //USBKEY_UCHAR p1,p2,lc,le;
    USBKEY_UCHAR p1,p2,le;
    USBKEY_INT32 lc;
    USBKEY_ULONG ulEncMode;
    USBKEY_ULONG ulPadMode;
    USBKEY_ULONG ulBitLen;
    USBKEY_ULONG loop = 0,offset,ulPriLen,i;

    AK_Log(AK_LOG_DEBUG, "USBKEY_ImportRSAKeyPair");

    /*if (strlen(pcAppName) > 16)
      {
      LogMessage("====>>USBKEY_ImportRSAKeyPair->pcAppName Length Error!");
      return USBKEY_PARAM_ERROR;
      }*/
    if (strlen((char *)pcContainerName) > 254)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAKeyPair ==> pcContainerName Length Error,Max Name Len is 255!");
        return USBKEY_PARAM_ERROR;
    }

    if ((ulKeyUsage != KEY_EXCHANGE) && (ulKeyUsage != KEY_SIGNATURE) )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAKeyPair ==> ulKeyUsage Error,ulKeyUsage = %#08x", ulKeyUsage);
        return USBKEY_PARAM_ERROR;
    }

    //检查加密模式,填充模式的合法性		//Update by zhoushenshen in 20100423
    ulEncMode = ulMode & 0xf0;
    ulPadMode = ulMode & 0x0f;

    if(pcWrapKeyID && USBKEY_PKCS5_PADDING != ulPadMode)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAKeyPair ==> ulMode Error,ulPadMode = %#08x", ulPadMode);
        return USBKEY_PARAM_ERROR;
    }
    if(pcWrapKeyID && ulEncMode != USBKEY_ECB && ulEncMode != USBKEY_CBC)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAKeyPair ==> ulMode Error,ulEncMode = %#08x", ulEncMode);
        return USBKEY_PARAM_ERROR;
    }

    if(pcWrapKeyID && ulEncMode == USBKEY_CBC && ulIVLen != 8 && ulIVLen != 16)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAKeyPair ==> ulIVLen Error,ulIVLen = %#08x", ulIVLen);
        return USBKEY_PARAM_ERROR;
    }


    //if(ulIVLen != 8 && ulIVLen != 16)
    /*if(ulMode == USBKEY_CBC && ulIVLen != 8 && ulIVLen != 16)	//changed by heli--2010-2-2
      {
      LogMessage("====>>USBKEY_ImportRSAKeyPair->ulIVLen Length Error!");
      return USBKEY_PARAM_ERROR;
      }*/

    AK_LockMutex(gpMutex);

    /*rtn = SelectFile(hDeviceHandle,pcAppName,NULL);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage16Val("====>>USBKEY_ImportRSAKeyPair->SelectFile() Error, rtn = 0x",rtn);
      return rtn;
      }*/

    rtn = SearchContainerRecNo(hDeviceHandle,pcContainerName,&recNO);
    if( (rtn != 0) || (recNO == 0) )
    {
        AK_UnlockMutex(gpMutex);
        if(recNO == 0)
        {
            AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAKeyPair ==> not find container record.");
            return USBKEY_RECORD_NOT_FOUND;
        }
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAKeyPair ==> SearchContainerRecNo() Error,rtn = %#08x", rtn);
        return rtn;
    }

    ////////
    //生成文件名
    ////////
    memcpy(fileName,&recNO,1);
    memcpy(fileName+2,&recNO,1);
    if (ulKeyUsage == KEY_EXCHANGE)
    {
        memset(fileName+1,0x01,1);
        memset(fileName+3,0x02,1);
    }
    else if (ulKeyUsage == KEY_SIGNATURE)
    {
        memset(fileName+1,0x04,1);
        memset(fileName+3,0x05,1);
    }

    //创建公私钥文件
    p1 = 0x00;
    p2 = 0x10;
    //lc = 9;
    lc = 0x07;						//lc updtae in 20120106
    le = 0x00;

    memset(data,0,9);
    memcpy(data,fileName,2);
    le = (USBKEY_UCHAR)ulAccessCondition;
    memcpy(data+3,&le,1);
    memcpy(data+4,&le,1);
    //updtae in 20120106 文件长度由4字节改为2字节
    ulFileSize = 516;
    le = (USBKEY_UCHAR)(ulFileSize/256);
    //memcpy(data+7,&le,1);
    memcpy(data+5,&le,1);
    le = (USBKEY_UCHAR)ulFileSize;
    //memcpy(data+8,&le,1);
    memcpy(data+6,&le,1);

    //Delete by zhoushenshen in 20121107
    /*rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_CreateFile,request,&reqLen);
      if (rtn != 0)
      {
      AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAKeyPair ==> PackageData() Create PubKey File Error,rtn = %#08x", rtn);
      return rtn;
      }

      respLen = 300;
      rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
      if (rtn != 0)
      {
      AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAKeyPair ==> DeviceExecCmd() Create PubKey File Error,rtn = %#08x", rtn);
      return rtn;
      }*/
    //Del end

    //Add by zhoushenshen in 20121107
    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_CreateFile,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAKeyPair ==> PackageData() Create PubKey File Error, rtn = %#08x", rtn);
        return rtn;
    }
    //Add end

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        case DEVICE_CMD_DATA_ERROR:
            AK_Log(AK_LOG_DEBUG, "USBKEY_ImportRSAKeyPair Public Key File Exist.");
            break;
        default:
            AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAKeyPair ==> DeviceExecCmd() Create PubKey File Error,code = %#08x", rtn);
            return rtn;
            break;
    }

    AK_Log(AK_LOG_DEBUG, "USBKEY_ImportRSAKeyPair Create Public Key File Success.");

    p1 = 0x00;
    p2 = 0x11;
    //lc = 9;
    lc = 0x07;						//lc updtae in 20120106
    le = 0x00;

    memset(data,0,9);
    memcpy(data,fileName+2,2);
    data[2] = 0x00;
    le = (USBKEY_UCHAR)ulAccessCondition;
    memcpy(data+3,&le,1);
    memcpy(data+4,&le,1);
    //updtae in 20120106 文件长度由4字节改为2字节
    ulFileSize = 1412;
    le = (USBKEY_UCHAR)(ulFileSize/256);
    //memcpy(data+7,&le,1);
    memcpy(data+5,&le,1);
    le = (USBKEY_UCHAR)ulFileSize;
    //memcpy(data+8,&le,1);
    memcpy(data+6,&le,1);

    //Delete by zhoushenshen in 20121107
    /*rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_CreateFile,request,&reqLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAKeyPair ==> PackageData() Create PriKey File Error,rtn = %#08x", rtn);
      return rtn;
      }

      respLen = 300;
      rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAKeyPair ==> DeviceExecCmd() Create PriKey File Error,rtn = %#08x", rtn);
      return rtn;
      }*/
    //Del end

    //Add by zhoushenshen in 20121107
    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_CreateFile,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAKeyPair ==> PackageData() Create PriKey File Error, rtn = %#08x", rtn);
        return rtn;
    }
    //Add end

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        case DEVICE_CMD_DATA_ERROR:
            AK_Log(AK_LOG_DEBUG, "USBKEY_ImportRSAKeyPair Private Key File Exist.");
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAKeyPair ==> DeviceExecCmd() Create PriKey File Error,code = %#08x", rtn);
            return rtn;
            break;
    }
    AK_Log(AK_LOG_DEBUG, "USBKEY_ImportRSAKeyPair Create Private Key File Success.");


    //写入公私钥
    if(pcWrapKeyID == NULL)
    {
        //p1 = 0x00;
        //P1的b7位表示是否有后续命令数据：0：无后续命令数据，1：有后续命令数据
        p1 = 0x00| 0x80;
    }
    else if (pcWrapKeyID[0] == 0x10)
    {
        //p1 = 0x07+(USBKEY_UCHAR)ulEncMode;
        //P1的b3~b0位表示密钥类型,P1的b4位表示加密模式：1：CBC, 0：ECB
        p1 = (0x07| 0x80) +(USBKEY_UCHAR)ulEncMode;
    }
    else if (pcWrapKeyID[0] == 0x11)
    {
        //p1 = 0x08+(USBKEY_UCHAR)ulEncMode;
        //P1的b3~b0位表示密钥类型,P1的b4位表示加密模式：1：CBC, 0：ECB
        p1 = (0x08| 0x80) +(USBKEY_UCHAR)ulEncMode;;
    }
    else
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAKeyPair ==> pcWrapKeyID Error,pcWrapKeyID = %#08x", pcWrapKeyID[0]);
        return USBKEY_PARAM_ERROR;
    }

    if(pcWrapKeyID == NULL)
        p2 = 0x00;
    else
        p2 = pcWrapKeyID[1];

    //Delete by zhoushenshen in 20121204
    /*lc = 0xFF;
      le = 0x00;
      if(ulEncMode == USBKEY_CBC)
      {
    //le = (ulRSAKeyPairLen+6+ulIVLen)/256;
    le = (USBKEY_UCHAR)((ulRSAKeyPairLen+8+ulIVLen)/256);	//changed by heli--2010-2-2
    memcpy(data,&le,1);
    //le = ulRSAKeyPairLen+6+ulIVLen;		
    le = (USBKEY_UCHAR)(ulRSAKeyPairLen+8+ulIVLen);			//changed by heli--2010-2-2
    memcpy(data+1,&le,1);
    }
    else
    {
    //le = (ulRSAKeyPairLen+6)/256;
    le = (USBKEY_UCHAR)((ulRSAKeyPairLen+8)/256);	//changed by heli--2010-2-2
    memcpy(data,&le,1);
    //le = ulRSAKeyPairLen+6;
    le = (USBKEY_UCHAR)(ulRSAKeyPairLen+8);		//changed by heli--2010-2-2
    memcpy(data+1,&le,1);
    }
    //写入密钥属性
    data[2] = 0x03;		//国密型
    data[3] = 0x00;		//RSA密钥
    memcpy(data+4,fileName+2,2);	//私钥ID
    memcpy(data+6,fileName,2);		//公钥ID

    data[8] = 0x00;	//add by heli--2010-2-2
    data[9] = 0x00;	//add by heli--2010-2-2

    //检查IV指针合法性
    if( ulMode == USBKEY_CBC )
    {
    if(NULL == pcIV)
    {
    AK_UnlockMutex(gpMutex);
    AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAKeyPair ==> pcIV is NULL,rtn = %#08x", USBKEY_PARAM_ERROR);
    return USBKEY_PARAM_ERROR;
    }
    else
    {
    //changed by heli--2010-2-2
    //memcpy(data+8, pcIV, ulIVLen);
    //memcpy(data+8+ulIVLen,pcRSAKeyPair,ulRSAKeyPairLen);
    memcpy(data+10, pcIV, ulIVLen);
    memcpy(data+10+ulIVLen,pcRSAKeyPair,ulRSAKeyPairLen);
    }
    }
    else
    {
    //changed by heli--2010-2-2
    //memcpy(data+8,pcRSAKeyPair,ulRSAKeyPairLen);
    memcpy(data+10,pcRSAKeyPair,ulRSAKeyPairLen);
    }

    rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_ImportPrivateKey,request,&reqLen);
    if (rtn != 0)
    {
    AK_UnlockMutex(gpMutex);
    AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAKeyPair ==> PackageData() Import KeyPair Error,rtn = %#08x", rtn);
    return rtn;
    }

    respLen = 300;
    rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
    if (rtn != 0)
    {
    AK_UnlockMutex(gpMutex);
    AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAKeyPair ==> DeviceExecCmd() Import KeyPair Error,rtn = %#08x", rtn);
    return rtn;
    }*/
    //Del end

    //Add by zhoushenshen in 20121204
    le = 0x00;
    if(ulEncMode == USBKEY_CBC)
        ulPriLen = ulRSAKeyPairLen+8+ulIVLen;
    else
        ulPriLen = ulRSAKeyPairLen+8;

    //写入密钥属性
    data[0] = 0x03;		//国密型
    data[1] = 0x00;		//RSA密钥
    memcpy(data+2,fileName+2,2);	//私钥ID
    memcpy(data+4,fileName,2);		//公钥ID

    data[6] = 0x00;
    data[7] = 0x00;

    //检查IV指针合法性
    if( ulMode == USBKEY_CBC )
    {
        if(NULL == pcIV)
        {
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAKeyPair ==> pcIV is NULL,rtn = %#08x", USBKEY_PARAM_ERROR);
            return USBKEY_PARAM_ERROR;
        }
        else
        {
            memcpy(data+8, pcIV, ulIVLen);
            memcpy(data+8+ulIVLen,pcRSAKeyPair,ulRSAKeyPairLen);
        }
    }
    else
    {
        memcpy(data+8,pcRSAKeyPair,ulRSAKeyPairLen);
    }

    //循环导入，若正好为写入块倍数，则循环数减1，最后处理块的P1与前面快不同
    if(ulPriLen % USBKEY_FILE_MAX_BLOCK_LEN)
        loop = ulPriLen/USBKEY_FILE_MAX_BLOCK_LEN;
    else
        loop = ulPriLen/USBKEY_FILE_MAX_BLOCK_LEN - 1;
    offset = 0;

    for (i=0; i<loop; i++)
    {
        le = 0x00;
        lc = USBKEY_FILE_MAX_BLOCK_LEN;

        respLen = 300;
        rtn = PackageData(hDeviceHandle,p1,p2,lc,data+i*USBKEY_FILE_MAX_BLOCK_LEN,le,USBKEY_OP_ImportPrivateKey,response,&respLen);
        if (rtn != 0)
        {
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAKeyPair ==> PackageData() Import key pair loop Error, rtn = %#08x", rtn);
            return rtn;
        }

        rtn = *(response+respLen-2);
        rtn = rtn*256 + *(response+respLen-1);
        switch(rtn)
        {
            case DEVICE_SUCCESS:
                break;
            default:
                AK_UnlockMutex(gpMutex);
                AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAKeyPair ==> DeviceExecCmd() Import key pair loop Error,code = %#08x", rtn);
                return rtn;
        }
        //memcpy(data,pcData+i*USBKEY_FILE_MAX_BLOCK_LEN,USBKEY_FILE_MAX_BLOCK_LEN);
    }

    //最后一次导入
    lc = ulPriLen - loop*USBKEY_FILE_MAX_BLOCK_LEN;
    le = 0x00;
    p1 = p1 & 0x7f;

    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data+loop*USBKEY_FILE_MAX_BLOCK_LEN,le,USBKEY_OP_ImportPrivateKey,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAKeyPair ==> PackageData() Import key pair Error, rtn = %#08x", rtn);
        return rtn;
    }
    //Add end

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAKeyPair ==> DeviceExecCmd() Import KeyPair Error,code = %#08x", rtn);
            return rtn;
            break;
    }
    AK_Log(AK_LOG_DEBUG, "USBKEY_ImportRSAKeyPair Write Public & Private Key Success.");

    ///////
    //赋值返回ID
    memcpy(pcPubKeyID,fileName,2);
    memcpy(pcPriKeyID,fileName+2,2);

    if(ulRSAKeyPairLen >= 600 && ulRSAKeyPairLen < 800)
        ulBitLen = 1024;
    else if(ulRSAKeyPairLen >= 1120 && ulRSAKeyPairLen < 1600)
        ulBitLen = 2048;
    else
        ulBitLen = 0;

    rtn = AddContainerKeySep(hDeviceHandle,recNO,ulKeyUsage, USBKEY_RSA_PRIVATEKEY,ulBitLen);	//Update by zhoushenshen 20100310
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAKeyPair ==> AddContainerKeySep() PriKey Error,rtn = %#08x", rtn);
        return rtn;
    }
    rtn = AddContainerKeySep(hDeviceHandle,recNO,ulKeyUsage, USBKEY_RSA_PUBLICKEY,ulBitLen);	//Update by zhoushenshen 20100310
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportRSAKeyPair ==> AddContainerKeySep() PubKey Error,rtn = %#08x", rtn);
        return rtn;
    }

    AK_Log(AK_LOG_DEBUG, "USBKEY_ImportRSAKeyPair Success!");
    AK_UnlockMutex(gpMutex);
    return USBKEY_OK;
}

//	RSA私钥运算.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_RSAPrivateKeyOperation(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcKeyID,		//[in]用来运算的密钥ID
        USBKEY_UCHAR_PTR pcInData,		//[in]输入数据
        USBKEY_ULONG ulInDataLen,		//[in]输入数据长度
        USBKEY_UCHAR_PTR pcOutData,		//[out]返回的输出
        USBKEY_ULONG_PTR pulOutDataLen)//[out]返回的输出长度
{
    //USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    USBKEY_UCHAR data[300];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0;
    //USBKEY_UCHAR p1,p2,lc,le;
    USBKEY_UCHAR p1,p2;
    USBKEY_INT32 lc,le;

    AK_Log(AK_LOG_DEBUG, "USBKEY_RSAPrivateKeyOperation");
    /*if (strlen(pcAppName) > 16)
      {
      LogMessage("====>>USBKEY_RSAPrivateKeyOperation->pcAppName Length Error!");
      return USBKEY_PARAM_ERROR;
      }*/
    if ((ulInDataLen != 128) && (ulInDataLen != 256) )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_RSAPrivateKeyOperation ==>ulInDataLen Error,ulInDataLen = %#08x", ulInDataLen);
        return USBKEY_PARAM_ERROR;
    }

    if (*pulOutDataLen < ulInDataLen)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_RSAPrivateKeyOperation ==>pulOutDataLen is too small,ulOutDataLen = %#08x", *pulOutDataLen);
        return USBKEY_BUFFER_TOO_SHORT;
    }
    AK_LockMutex(gpMutex);

    /*rtn = SelectFile(hDeviceHandle,pcAppName,NULL);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage16Val("====>>USBKEY_RSAPrivateKeyOperation->SelectFile() Error, rtn = 0x",rtn);
      return USBKEY_GENERAL_ERROR;
      }*/

    /////////检查权限///////////////////////////////////////////////////
    /*rtn = CheckPrivillege(hDeviceHandle);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage16Val("====>>USBKEY_RSAPrivateKeyOperation->CheckPrivillege() Error,rtn = 0x",rtn);
      return rtn;
      }*/
    //////////////////////////////////////////////////////////////////////////

    ////////
    //生成数据
    ////////
    memset(data,0,sizeof(data));
    memcpy(&p1,pcKeyID,1);
    memcpy(&p2,pcKeyID+1,1);
    //Delete by zhoushenshen in 20121204
    /*if (ulInDataLen == 128)
      {
      lc = (USBKEY_UCHAR)ulInDataLen;
      memcpy(data,pcInData,ulInDataLen);
      }
      else
      {
      lc = 0xff;
      memset(data,0x01,1);
      memset(data+1,0x00,1);
      memcpy(data+2,pcInData,ulInDataLen);
      }
      le = 0x00;

      rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_PrivateKeyOP,request,&reqLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_RSAPrivateKeyOperation ==>PackageData() Error,rtn = %#08x", rtn);
      return rtn;
      }

      respLen = 300;
      rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_RSAPrivateKeyOperation ==>DeviceExecCmd() Error,rtn = %#08x", rtn);
      return rtn;
      }*/
    //Del end

    //Add by zhoushenshen in 20121204
    lc = ulInDataLen;
    le = lc;
    memcpy(data,pcInData,ulInDataLen);

    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_PrivateKeyOP,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_RSAPrivateKeyOperation ==> PackageData() Error, rtn = %#08x", rtn);
        return rtn;
    }
    //Add end

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_RSAPrivateKeyOperation ==>DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
            break;
    }


    ///////
    //获得结果
    ///////
    memcpy(pcOutData,response,respLen-2);
    *pulOutDataLen = respLen - 2;

    AK_Log(AK_LOG_DEBUG, "USBKEY_RSAPrivateKeyOperation Success!");
    AK_UnlockMutex(gpMutex);

    return USBKEY_OK;
}

//	RSA公钥运算.
//	返回值：
//		0：	运算成功.
//		>0:	运算失败，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_RSAPublicKeyOperation(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcKeyID,		//[in]用来运算的密钥ID
        USBKEY_UCHAR_PTR pcInData,		//[in]输入数据
        USBKEY_ULONG ulInDataLen,		//[in]输入数据长度
        USBKEY_UCHAR_PTR pcOutData,		//[out]输出数据
        USBKEY_ULONG_PTR pulOutDataLen) //[out]输出数据长度
{
    //USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    USBKEY_UCHAR data[300];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0;
    //USBKEY_UCHAR p1,p2,lc,le;
    USBKEY_UCHAR p1,p2;
    USBKEY_INT32 lc,le;

    AK_Log(AK_LOG_DEBUG, "USBKEY_RSAPublicKeyOperation");
    /*if (strlen(pcAppName) > 16)
      {
      LogMessage("====>>USBKEY_RSAPublicKeyOperation->pcAppName Length Error!");
      return USBKEY_PARAM_ERROR;
      }*/
    if ((ulInDataLen != 128) && (ulInDataLen != 256) )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_RSAPublicKeyOperation ==>ulInDataLen Error,ulInDataLen = %#08x", ulInDataLen);
        return USBKEY_PARAM_ERROR;
    }
    if (*pulOutDataLen < ulInDataLen)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_RSAPublicKeyOperation ==>pulOutDataLen is too small,ulOutDataLen = %#08x", *pulOutDataLen);
        return USBKEY_BUFFER_TOO_SHORT;
    }

    AK_LockMutex(gpMutex);

    /*rtn = SelectFile(hDeviceHandle,pcAppName,NULL);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage16Val("====>>USBKEY_RSAPublicKeyOperation->SelectFile() Error, rtn = 0x",rtn);
      return USBKEY_GENERAL_ERROR;
      }*/

    /////////检查权限///////////////////////////////////////////////////
    /*rtn = CheckPrivillege(hDeviceHandle);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage16Val("====>>USBKEY_RSAPublicKeyOperation->CheckPrivillege() Error,rtn = 0x",rtn);
      return rtn;
      }*/
    //////////////////////////////////////////////////////////////////////////

    ////////
    //生成数据
    ////////
    memset(data,0,sizeof(data));
    memcpy(&p1,pcKeyID,1);
    memcpy(&p2,pcKeyID+1,1);

    //Delete by zhoushenshen in 20121204
    /*if (ulInDataLen == 128)
      {
      lc = (USBKEY_UCHAR)ulInDataLen;
      memcpy(data,pcInData,ulInDataLen);
      }
      else
      {
      lc = 0xff;
      memset(data,0x01,1);
      memset(data+1,0x00,1);
      memcpy(data+2,pcInData,ulInDataLen);
      }
      le = 0x00;

      rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_PublicKeyOP,request,&reqLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_RSAPublicKeyOperation ==>PackageData() Error,rtn = %#08x", rtn);
      return rtn;
      }

      respLen = 300;
      rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_RSAPublicKeyOperation ==>DeviceExecCmd() Error,rtn = %#08x", rtn);
      return rtn;
      }*/

    //Add by zhoushenshen in 20121204
    lc = ulInDataLen;
    le = lc;
    memcpy(data,pcInData,ulInDataLen);

    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_PublicKeyOP,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_RSAPublicKeyOperation ==> PackageData() Error, rtn = %#08x", rtn);
        return rtn;
    }
    //Add end

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_RSAPublicKeyOperation ==>DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
            break;
    }

    ///////
    //获得结果
    ///////
    memcpy(pcOutData,response,respLen-2);
    *pulOutDataLen = respLen - 2;

    AK_Log(AK_LOG_DEBUG, "USBKEY_RSAPublicKeyOperation Success!");
    AK_UnlockMutex(gpMutex);

    return USBKEY_OK;
}

//	对称加密.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DataEncrypt(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcKeyID,		//[in]用来加密的密钥ID
        USBKEY_UCHAR_PTR pcData,		//[in]待加密数据
        USBKEY_ULONG ulDataLen,			//[in]待加密数据长度
        USBKEY_UCHAR_PTR pcEncryptedData,//[out]返回的加密后数据
        USBKEY_ULONG_PTR pulEncryptedLen)//[out]返回的加密后数据长度
{
    //*
    //USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    USBKEY_UCHAR data[300];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0;
    USBKEY_UCHAR p1,p2,lc,le;

    AK_Log(AK_LOG_DEBUG, "USBKEY_DataEncrypt");
    /*if (strlen(pcAppName) > 16)
      {
      LogMessage("====>>USBKEY_DataEncrypt->pcAppName Length Error!");
      return USBKEY_PARAM_ERROR;
      }*/

    AK_LockMutex(gpMutex);

    /*rtn = SelectFile(hDeviceHandle,pcAppName,NULL);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage16Val("====>>USBKEY_DataEncrypt->SelectFile() Error, rtn = 0x",rtn);
      return USBKEY_GENERAL_ERROR;
      }*/

    /////////检查权限///////////////////////////////////////////////////
    /*rtn = CheckPrivillege(hDeviceHandle);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage16Val("====>>USBKEY_DataEncrypt->CheckPrivillege() Error,rtn = 0x",rtn);
      return rtn;
      }*/
    //////////////////////////////////////////////////////////////////////////

    if (*pcKeyID == 0x10)
        p1 = 0x07;
    else if (*pcKeyID == 0x11)
        p1 = 0x08;
    else
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_DataEncrypt ==>pcKeyID is not 0x10 or 0x11,rtn = %#08x", USBKEY_PARAM_ERROR);
        return USBKEY_PARAM_ERROR;
    }
    p2 = 0;
    lc = (USBKEY_UCHAR)ulDataLen;
    //le = 0x00;	//Delete by zhoushenshen in 20121204
    le = lc;

    ////////
    //生成数据
    ////////
    memset(data,0,300);
    memcpy(data,pcData,ulDataLen);

    //Delete by zhoushenshen in 20121204
    /*rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_DataEncrypt,request,&reqLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_DataEncrypt ==> PackageData() Error,rtn = %#08x", rtn);
      return rtn;
      }

      respLen = 300;
      rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_DataEncrypt ==> DeviceExecCmd() Error,rtn = %#08x", rtn);
      return rtn;
      }*/
    //Del end

    //Add by zhoushenshen in 20121204
    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_DataEncrypt,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_DataEncrypt ==> PackageData() Error,rtn = %#08x", rtn);
        return rtn;
    }
    //Add end

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_DataEncrypt ==> DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
            break;
    }


    ///////
    //获得结果
    ///////
    memcpy(pcEncryptedData,response,respLen-2);
    *pulEncryptedLen = respLen - 2;
    //*/
    AK_Log(AK_LOG_DEBUG, "USBKEY_DataEncrypt Success!");
    AK_UnlockMutex(gpMutex);
    return USBKEY_OK;
}

//	对称解密.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DataDecrypt(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcKeyID,		//[in]用来解密的密钥ID
        USBKEY_UCHAR_PTR pcEncryptedData,//[in]密文数据
        USBKEY_ULONG ulEncryptedLen,	//[in]密文数据长度
        USBKEY_UCHAR_PTR pcData,		//[out]返回的明文
        USBKEY_ULONG_PTR pulDataLen)	//[out]返回的明文长度
{
    //USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    USBKEY_UCHAR data[300];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0;
    USBKEY_UCHAR p1,p2,lc,le;

    AK_Log(AK_LOG_DEBUG, "USBKEY_DataDecrypt");
    /*if (strlen(pcAppName) > 16)
      {
      LogMessage("====>>USBKEY_DataEncrypt->pcAppName Length Error!");
      return USBKEY_PARAM_ERROR;
      }*/

    AK_LockMutex(gpMutex);

    /*rtn = SelectFile(hDeviceHandle,pcAppName,NULL);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage16Val("====>>USBKEY_DataEncrypt->SelectFile() Error, rtn = 0x",rtn);
      return USBKEY_GENERAL_ERROR;
      }*/

    /////////检查权限///////////////////////////////////////////////////
    /*rtn = CheckPrivillege(hDeviceHandle);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage16Val("====>>USBKEY_DataDecrypt->CheckPrivillege() Error,rtn = 0x",rtn);
      return rtn;
      }*/
    //////////////////////////////////////////////////////////////////////////

    if (*pcKeyID == 0x10)
        p1 = 0x07;
    else if (*pcKeyID == 0x11)
        p1 = 0x08;
    else
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_DataDecrypt ==> pcKeyID is not 0x10 or 0x11,rtn = %#08x", USBKEY_PARAM_ERROR);
        return USBKEY_PARAM_ERROR;
    }
    p2 = 0;
    lc = (USBKEY_UCHAR)ulEncryptedLen;
    //le = 0x00;	//Delete by zhoushenshen in 20121204
    le = lc;

    ////////
    //生成数据
    ////////
    memset(data,0,300);
    memcpy(data,pcEncryptedData,ulEncryptedLen);

    //Delete by zhoushenshen in 20121204
    /*rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_DataDecrypt,request,&reqLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_DataDecrypt ==> PackageData() Error,rtn = %#08x", rtn);
      return rtn;
      }

      respLen = 300;
      rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_DataDecrypt ==> DeviceExecCmd() Error,rtn = %#08x", rtn);
      return rtn;
      }*/
    //Del end

    //Add by zhoushenshen in 20121204
    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_DataDecrypt,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_DataDecrypt ==> PackageData() Error,rtn = %#08x", rtn);
        return rtn;
    }
    //Add end

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_DataDecrypt ==> DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
            break;
    }


    ///////
    //获得结果
    ///////
    memcpy(pcData,response,respLen-2);
    *pulDataLen = respLen - 2;
    //*/
    AK_Log(AK_LOG_DEBUG, "USBKEY_DataDecrypt Success!");
    AK_UnlockMutex(gpMutex);
    return USBKEY_OK;
}


//	批量对称加密.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_MassiveDataEncrypt(
        USBKEY_HANDLE		hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR	pcKeyID,		//[in]用来加密的密钥ID
        USBKEY_ULONG		ulAlg,			//[in]加密算法
        USBKEY_ULONG		ulMode,			//[in]加密模式和填充模式：USBKEY_ECB|USBKEY_NO_PADDING, USBKEY_ECB|USBKEY_PKCS5_PADDING, USBKEY_CBC|USBKEY_NO_PADDING, USBKEY_CBC|USBKEY_PKCS5_PADDING
        USBKEY_UCHAR_PTR	pcIV,			//[in]初始化向量，如果是ecb模式，设置为NULL
        USBKEY_UCHAR_PTR	pcData,			//[in]待加密数据
        USBKEY_ULONG		ulDataLen,		//[in]待加密数据长度
        USBKEY_UCHAR_PTR	pcEncryptedData,//[out]返回的加密后数据
        USBKEY_ULONG_PTR	pulEncryptedLen)//[in/out]in: pcEncryptedData缓冲区大小，out: 加密后数据长度或需要的pcEncryptedData缓冲区大小
{
    //*
    //USBKEY_UCHAR request[USBKEY_MAX_COM_LEN];
    USBKEY_UCHAR response[USBKEY_MAX_COM_LEN];
    USBKEY_UCHAR data[USBKEY_MAX_COM_LEN];
    //USBKEY_UCHAR data[300];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn, respLen=0;
    //USBKEY_UCHAR p1,p2,lc,le;
    USBKEY_UCHAR p1,p2;
    USBKEY_INT32 lc,le;
    USBKEY_ULONG ulBlockLen;
    USBKEY_UCHAR *pcBuffer;
    USBKEY_ULONG ulLen;
    USBKEY_ULONG ulCmdDataLen;
    USBKEY_ULONG ulMaxCmdDataLen;
    USBKEY_ULONG ulEncMode;
    USBKEY_ULONG ulPadMode;
    USBKEY_UCHAR pcTmpIV[16];

    AK_Log(AK_LOG_DEBUG, "USBKEY_MassiveDataEncrypt");

    AK_LockMutex(gpMutex);
    /*if(pcAppName)
      {
      if (strlen(pcAppName) > 16)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage("====>>USBKEY_MassiveDataEncrypt->pcAppName Length Error!");
      return USBKEY_PARAM_ERROR;
      }
      rtn = SelectFile(hDeviceHandle,pcAppName,NULL);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage16Val("====>>USBKEY_MassiveDataEncrypt->SelectFile() Error,rtn = 0x",rtn);
      return rtn;
      }
      }*/

    /////////检查权限///////////////////////////////////////////////////
    /*rtn = CheckPrivillege(hDeviceHandle);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage16Val("====>>USBKEY_MassiveDataEncrypt->CheckPrivillege() Error,rtn = 0x",rtn);
      return rtn;
      }*/
    //////////////////////////////////////////////////////////////////////////

    //检查缓冲区指针合法性
    if( NULL == pcData || NULL == pcEncryptedData || NULL == pcKeyID || NULL == pulEncryptedLen)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_MassiveDataEncrypt == PARAM is NULL! rtn = %#08x", USBKEY_PARAM_ERROR);
        return USBKEY_PARAM_ERROR;
    }

    //检查算法合法性，并根据算法设置分组长度
    switch(ulAlg)
    {
        case ALG_TYPE_DES:
        case ALG_TYPE_3DES_2KEY:
        case ALG_TYPE_3DES_3KEY:
            ulBlockLen = 8;
            break;
        case ALG_TYPE_SSF33:
        case ALG_TYPE_SCB2:
            ulBlockLen = 16;
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_MassiveDataEncrypt ==>ulAlg Error,ulAlg = %#08x", ulAlg);
            return USBKEY_PARAM_ERROR;
    }

    //检查加密模式的合法性
    ulEncMode = ulMode & 0xf0;
    ulPadMode = ulMode & 0x0f;
    switch(ulEncMode)
    {
        case USBKEY_ECB:
        case USBKEY_CBC:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_MassiveDataEncrypt ==>ulMode Error,ulEncMode = %#08x", ulEncMode);
            return USBKEY_PARAM_ERROR;
    }

    //检查IV指针合法性
    if( ulEncMode == USBKEY_CBC )
    {
        if(NULL == pcIV)
        {
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_MassiveDataEncrypt ==>pcIV is NULL,rtn = %#08x", USBKEY_PARAM_ERROR);
            return USBKEY_PARAM_ERROR;
        }
        else
        {
            memcpy(pcTmpIV, pcIV, ulBlockLen);
        }
    }


    //检查填充模式的合法性，加密数据长度的合法性，输出缓冲区是否够大，根据模式进行pkcs5填充
    if( USBKEY_NO_PADDING == ulPadMode )
    {
        if(ulDataLen%8 != 0)
        {
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_MassiveDataEncrypt ==>ulDataLen Error,ulDataLen = %#08x", ulDataLen);
            return USBKEY_PARAM_ERROR;
        }
        if( (ALG_TYPE_SSF33 == ulAlg || ALG_TYPE_SCB2 == ulAlg) && (ulDataLen%16 != 0) )
        {
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_MassiveDataEncrypt ==>ulDataLen Error,ulDataLen = %#08x", ulDataLen);
            return USBKEY_PARAM_ERROR;
        }
        if(*pulEncryptedLen < ulDataLen)
        {
            AK_UnlockMutex(gpMutex);
            *pulEncryptedLen = ulDataLen;
            AK_Log(AK_LOG_ERROR, "USBKEY_MassiveDataEncrypt ==>pulEncryptedLen is too small,ulEncryptedLen = %#08x", *pulEncryptedLen);
            return USBKEY_BUFFER_TOO_SHORT;
        }
        memcpy(pcEncryptedData, pcData, ulDataLen);
        ulLen = ulDataLen;
    }
    else if( USBKEY_PKCS5_PADDING == ulPadMode )
    {
        ulLen = ulBlockLen-ulDataLen%ulBlockLen + ulDataLen;
        if(*pulEncryptedLen < ulLen)
        {
            AK_UnlockMutex(gpMutex);
            *pulEncryptedLen = ulLen;
            AK_Log(AK_LOG_ERROR, "USBKEY_MassiveDataEncrypt ==>pulEncryptedLen is too small,ulEncryptedLen = %#08x", *pulEncryptedLen);
            return USBKEY_BUFFER_TOO_SHORT;
        }

        //PKCS5填充
        memcpy(pcEncryptedData, pcData, ulDataLen);
        ulLen = ulDataLen;
        pkcs5_padding(pcEncryptedData, &ulLen, ulBlockLen);
    }
    else
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_MassiveDataEncrypt ==>ulMode Error,ulPadMode = %#08x", ulPadMode);
        return USBKEY_PARAM_ERROR;
    }

    //Delete by zhoushenshen in 20121204
    /*//设置命令头: CLA INS P1 P2 LC
      request[0] = 0xb0;	//CLA
      request[1] = 0x42;	//INS
      if (*pcKeyID == 0x10)
      request[2] = 0x07 + (USBKEY_UCHAR)ulEncMode;	//P1
      else if (*pcKeyID == 0x11)
      request[2] = 0x08 + (USBKEY_UCHAR)ulEncMode;	//P1
      else
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_MassiveDataEncrypt ==>pcKeyID Error,pcKeyID = %#08x", pcKeyID[0]);
      return USBKEY_PARAM_ERROR;
      }
      request[3] = *(pcKeyID+1);		//P2
      request[4] = 0xff;	//LC

      pcBuffer = pcEncryptedData;
     *pulEncryptedLen = 0;
     if( ulEncMode == USBKEY_ECB )//ecb
     {
     ulMaxCmdDataLen = USBKEY_MAX_COM_DATA_LEN;
     }
     else//cbc
     {
     ulMaxCmdDataLen = USBKEY_MAX_COM_DATA_LEN - ulBlockLen;
     }

     while(ulLen)
     {
    //判断发送的命令数据的长度
    if(ulLen >= ulMaxCmdDataLen)
    {
    ulCmdDataLen = ulMaxCmdDataLen;
    }
    else
    {
    ulCmdDataLen = ulLen;
    }

    //设置命令数据
    if( ulEncMode == USBKEY_ECB )//ecb
    {
    request[5] = (USBKEY_UCHAR)(ulCmdDataLen/256);
    request[6] = (USBKEY_UCHAR)(ulCmdDataLen%256);
    memcpy(request+7, pcBuffer, ulCmdDataLen);
    reqLen = 7 + ulCmdDataLen;
    }
    else	//cbc
    {
    request[5] = (USBKEY_UCHAR)((ulCmdDataLen+ulBlockLen)/256);
    request[6] = (USBKEY_UCHAR)((ulCmdDataLen+ulBlockLen)%256);
    memcpy(request+7, pcTmpIV, ulBlockLen);
    memcpy(request+7+ulBlockLen, pcBuffer, ulCmdDataLen);
    reqLen = 7 + ulBlockLen + ulCmdDataLen;
    }

    //发送命令
    respLen = USBKEY_MAX_COM_LEN;
    rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
    if (rtn != 0)
    {
    AK_UnlockMutex(gpMutex);
    AK_Log(AK_LOG_ERROR, "USBKEY_MassiveDataEncrypt ==>DeviceExecCmd() Error,rtn = %#08x", rtn);
    return rtn;
    }

    //判断命令是否执行成功
    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
    case DEVICE_SUCCESS:
    break;
    default:
    AK_UnlockMutex(gpMutex);
    AK_Log(AK_LOG_ERROR, "USBKEY_MassiveDataEncrypt ==>DeviceExecCmd() Error,code = %#08x", rtn);
    return rtn;
    break;
}


///////
//获得结果
///////
if( ulEncMode == USBKEY_CBC )//cbc
{
    memcpy(pcTmpIV, response+(respLen-ulBlockLen-2), ulBlockLen);//设置新的IV
    *pulEncryptedLen += respLen-2-ulBlockLen;					//设置密文长度
    memcpy(pcBuffer,response+ulBlockLen,respLen-2-ulBlockLen);	//设置密文	
}
else
{
    *pulEncryptedLen += respLen-2;					//设置密文长度
    memcpy(pcBuffer,response,respLen-2);			//设置密文
}

pcBuffer += ulCmdDataLen;
ulLen -= ulCmdDataLen;

}*/
//Del end

//Add by zhoushenshen in 20121204
memset(data, 0, sizeof(data));
//设置命令包
if (*pcKeyID == 0x10)
    p1 = 0x07 + (USBKEY_UCHAR)ulEncMode;	//P1
else if (*pcKeyID == 0x11)
    p1 = 0x08 + (USBKEY_UCHAR)ulEncMode;	//P1
    else
{
    AK_UnlockMutex(gpMutex);
    AK_Log(AK_LOG_ERROR, "USBKEY_MassiveDataEncrypt ==>pcKeyID Error,pcKeyID = %#08x", pcKeyID[0]);
    return USBKEY_PARAM_ERROR;
}
p2 = *(pcKeyID+1);		//P2

pcBuffer = pcEncryptedData;
*pulEncryptedLen = 0;
if( ulEncMode == USBKEY_ECB )//ecb
{
    ulMaxCmdDataLen = USBKEY_MAX_COM_DATA_LEN;
}
else//cbc
{
    ulMaxCmdDataLen = USBKEY_MAX_COM_DATA_LEN - ulBlockLen;
}

while(ulLen)
{
    //判断发送的命令数据的长度
    if(ulLen >= ulMaxCmdDataLen)
    {
        ulCmdDataLen = ulMaxCmdDataLen;
    }
    else
    {
        ulCmdDataLen = ulLen;
    }

    //设置命令数据
    if( ulEncMode == USBKEY_ECB )//ecb
    {
        lc = ulCmdDataLen;
        memcpy(data, pcBuffer, ulCmdDataLen);
        le = lc;
    }
    else	//cbc
    {
        lc = ulCmdDataLen+ulBlockLen;
        memcpy(data, pcTmpIV, ulBlockLen);
        memcpy(data+ulBlockLen, pcBuffer, ulCmdDataLen);
        le = lc;
    }

    //发送命令
    respLen = USBKEY_MAX_COM_LEN;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_MassiveDataEncrypt,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_MassiveDataEncrypt ==>PackageData() Error,rtn = %#08x", rtn);
        return rtn;
    }

    //判断命令是否执行成功
    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_MassiveDataEncrypt ==>DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
            break;
    }


    ///////
    //获得结果
    ///////
    if( ulEncMode == USBKEY_CBC )//cbc
    {
        memcpy(pcTmpIV, response+(respLen-ulBlockLen-2), ulBlockLen);//设置新的IV
        *pulEncryptedLen += respLen-2;					//设置密文长度
        memcpy(pcBuffer,response,respLen-2);	//设置密文	
    }
    else
    {
        *pulEncryptedLen += respLen-2;					//设置密文长度
        memcpy(pcBuffer,response,respLen-2);			//设置密文
    }

    pcBuffer += ulCmdDataLen;
    ulLen -= ulCmdDataLen;

}
//Add end
//*/
AK_Log(AK_LOG_DEBUG, "USBKEY_MassiveDataEncrypt Success!");
AK_UnlockMutex(gpMutex);
return USBKEY_OK;
}

//	批量对称解密.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_MassiveDataDecrypt(
        USBKEY_HANDLE		hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR	pcKeyID,		//[in]用来解密的密钥ID
        USBKEY_ULONG		ulAlg,			//[in]解密算法
        USBKEY_ULONG		ulMode,			//[in]解密模式和填充模式：USBKEY_ECB|USBKEY_NO_PADDING, USBKEY_ECB|USBKEY_PKCS5_PADDING, USBKEY_CBC|USBKEY_NO_PADDING, USBKEY_CBC|USBKEY_PKCS5_PADDING
        USBKEY_UCHAR_PTR	pcIV,			//[in]初始化向量，如果是ecb模式，设置为NULL
        USBKEY_UCHAR_PTR	pcEncryptedData,//[in]密文数据
        USBKEY_ULONG		ulEncryptedLen,	//[in]密文数据长度
        USBKEY_UCHAR_PTR	pcData,			//[out]返回的明文
        USBKEY_ULONG_PTR	pulDataLen)		//[in/out]in: pcData缓冲区长度 out:返回的明文长度
{
    //USBKEY_UCHAR request[USBKEY_MAX_COM_LEN];
    USBKEY_UCHAR response[USBKEY_MAX_COM_LEN];
    USBKEY_UCHAR data[USBKEY_MAX_COM_LEN];
    //USBKEY_UCHAR data[300];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0;
    //USBKEY_UCHAR p1,p2,lc,le;
    USBKEY_UCHAR p1,p2;
    USBKEY_INT32 lc,le;
    USBKEY_ULONG ulBlockLen;
    USBKEY_ULONG ulEncMode;
    USBKEY_ULONG ulPadMode;
    USBKEY_UCHAR pcTmpIV[16];
    USBKEY_UCHAR *pcEncBuffer;
    USBKEY_UCHAR *pcDataBuf;
    USBKEY_ULONG ulTmpLen;
    USBKEY_UCHAR flag = 0;
    USBKEY_ULONG ulCmdDataLen;
    USBKEY_ULONG ulMaxCmdDataLen;


    AK_Log(AK_LOG_DEBUG, "USBKEY_MassiveDataDecrypt");

    AK_LockMutex(gpMutex);
    /*if(pcAppName)
      {
      if (strlen(pcAppName) > 16)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage("====>>USBKEY_MassiveDataDecrypt->pcAppName Length Error!");
      return USBKEY_PARAM_ERROR;
      }
      rtn = SelectFile(hDeviceHandle,pcAppName,NULL);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage16Val("====>>USBKEY_MassiveDataDecrypt->SelectFile() Error,rtn = 0x",rtn);
      return rtn;
      }
      }*/

    /////////检查权限///////////////////////////////////////////////////
    /*rtn = CheckPrivillege(hDeviceHandle);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage16Val("====>>USBKEY_MassiveDataDecrypt->CheckPrivillege() Error,rtn = 0x",rtn);
      return rtn;
      }*/
    //////////////////////////////////////////////////////////////////////////

    //检查缓冲区指针合法性
    if( NULL == pcData || NULL == pcEncryptedData || NULL == pcKeyID || NULL == pulDataLen)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_MassiveDataDecrypt == PARAM is NULL! rtn = %#08x", USBKEY_PARAM_ERROR);
        return USBKEY_PARAM_ERROR;
    }
    if( *pulDataLen < ulEncryptedLen )
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_MassiveDataDecrypt ==> pulDataLen is too small,ulDataLen = %#08x", *pulDataLen);
        return USBKEY_BUFFER_TOO_SHORT;
    }

    //检查算法合法性，并根据算法设置分组长度
    switch(ulAlg)
    {
        case ALG_TYPE_DES:
        case ALG_TYPE_3DES_2KEY:
        case ALG_TYPE_3DES_3KEY:
            ulBlockLen = 8;
            if(ulEncryptedLen%8 != 0)
            {
                AK_UnlockMutex(gpMutex);
                AK_Log(AK_LOG_ERROR, "USBKEY_MassiveDataDecrypt ==> ulEncryptedLen Error,ulEncryptedLen = %#08x", ulEncryptedLen);
                return USBKEY_PARAM_ERROR;
            }
            break;
        case ALG_TYPE_SSF33:
        case ALG_TYPE_SCB2:
            ulBlockLen = 16;
            if(ulEncryptedLen%16 != 0)
            {
                AK_UnlockMutex(gpMutex);
                AK_Log(AK_LOG_ERROR, "USBKEY_MassiveDataDecrypt ==> ulEncryptedLen Error,ulEncryptedLen = %#08x", ulEncryptedLen);
                return USBKEY_PARAM_ERROR;
            }
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_MassiveDataDecrypt ==> ulAlg Error,ulAlg = %#08x", ulAlg);
            return USBKEY_PARAM_ERROR;
    }

    //检查加密模式,填充模式的合法性
    ulEncMode = ulMode & 0xf0;
    ulPadMode = ulMode & 0x0f;
    switch(ulEncMode)
    {
        case USBKEY_ECB:
        case USBKEY_CBC:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_MassiveDataDecrypt ==> ulMode Error,ulEncMode = %#08x", ulEncMode);
            return USBKEY_PARAM_ERROR;
    }
    switch(ulPadMode)
    {
        case USBKEY_NO_PADDING:
        case USBKEY_PKCS5_PADDING:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_MassiveDataDecrypt ==> ulMode Error,ulPadMode = %#08x", ulPadMode);
            return USBKEY_PARAM_ERROR;
    }

    //检查IV指针合法性
    if( ulEncMode == USBKEY_CBC )
    {
        if(NULL == pcIV)
        {
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_MassiveDataDecrypt ==> pcIV is NULL,rtn = %#08x", USBKEY_PARAM_ERROR);
            return USBKEY_PARAM_ERROR;
        }
        else
        {
            memcpy(pcTmpIV, pcIV, ulBlockLen);
        }
    }

    //Delete by zhoushenshen in 20121210
    /*//设置命令头: CLA INS P1 P2 LC
      request[0] = 0xb0;	//CLA
      request[1] = 0x43;	//INS
      if (*pcKeyID == 0x10)
      request[2] = 0x07 + (USBKEY_UCHAR)ulEncMode;	//P1
      else if (*pcKeyID == 0x11)
      request[2] = 0x08 + (USBKEY_UCHAR)ulEncMode;	//P1
      else
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_MassiveDataDecrypt ==>pcKeyID Error,pcKeyID = %#08x", pcKeyID[0]);
      return USBKEY_PARAM_ERROR;
      }
      request[3] = *(pcKeyID+1);		//P2
      request[4] = 0xff;	//LC

      pcEncBuffer = pcEncryptedData;
      pcDataBuf = pcData;
      if( ulEncMode == USBKEY_ECB )//ecb
      {
      ulMaxCmdDataLen = USBKEY_MAX_COM_DATA_LEN;
      }
      else//cbc
      {
      ulMaxCmdDataLen = USBKEY_MAX_COM_DATA_LEN - ulBlockLen;
      }
     *pulDataLen = 0;
     while(ulEncryptedLen)
     {
    //判断发送的命令数据的长度
    if(ulEncryptedLen >= ulMaxCmdDataLen)
    {
    ulCmdDataLen = ulMaxCmdDataLen;
    }
    else
    {
    ulCmdDataLen = ulEncryptedLen;
    flag = 1;
    }

    //设置命令数据
    if( ulEncMode == USBKEY_ECB )//ecb
    {
    request[5] = (USBKEY_UCHAR)(ulCmdDataLen/256);
    request[6] = (USBKEY_UCHAR)(ulCmdDataLen%256);
    memcpy(request+7, pcEncBuffer, ulCmdDataLen);
    reqLen = 7 + ulCmdDataLen;
    }
    else	//cbc
    {
    request[5] = (USBKEY_UCHAR)((ulCmdDataLen+ulBlockLen)/256);
    request[6] = (USBKEY_UCHAR)((ulCmdDataLen+ulBlockLen)%256);
    memcpy(request+7, pcTmpIV, ulBlockLen);
    memcpy(request+7+ulBlockLen, pcEncBuffer, ulCmdDataLen);
    reqLen = 7 + ulBlockLen + ulCmdDataLen;
    }
    pcEncBuffer += ulCmdDataLen;
    ulEncryptedLen -= ulCmdDataLen;

    //发送命令
    respLen = USBKEY_MAX_COM_LEN;
    rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
    if (rtn != 0)
    {
    AK_UnlockMutex(gpMutex);
    AK_Log(AK_LOG_ERROR, "USBKEY_MassiveDataDecrypt ==>DeviceExecCmd() Error,rtn = %#08x", rtn);
    return rtn;
    }

    //判断命令是否执行成功
    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_MassiveDataDecrypt ==>DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
            break;
    }

    ///////
    //获得结果
    ///////
    if( ulEncMode == USBKEY_CBC )//cbc
    {
        memcpy(pcTmpIV, pcEncBuffer-ulBlockLen, ulBlockLen);//设置新的IV
        ulTmpLen = respLen - 2 - ulBlockLen;
    }
    else
    {
        ulTmpLen = respLen - 2;
    }


    *pulDataLen += ulTmpLen;
    memcpy(pcDataBuf, response, ulTmpLen);
    if( 1 == flag && USBKEY_PKCS5_PADDING == ulPadMode  )//最后一个数据包，有填充
    {
        //去掉填充
        if( 0 != pkcs5_no_padding(pcData, pulDataLen))//原来数据长度
        {
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_MassiveDataDecrypt ==>pkcs5_no_padding Error");
            return USBKEY_DISCARD_PADDING_ERROR;
        }
    }
    pcDataBuf += ulTmpLen;

}
*/
//Del end

//Add by zhoushenshen in 20121210
memset(data, 0, sizeof(data));
//设置命令包
if (*pcKeyID == 0x10)
    p1 = 0x07 + (USBKEY_UCHAR)ulEncMode;	//P1
else if (*pcKeyID == 0x11)
    p1 = 0x08 + (USBKEY_UCHAR)ulEncMode;	//P1
    else
{
    AK_UnlockMutex(gpMutex);
    AK_Log(AK_LOG_ERROR, "USBKEY_MassiveDataDecrypt ==>pcKeyID Error,pcKeyID = %#08x", pcKeyID[0]);
    return USBKEY_PARAM_ERROR;
}
p2 = *(pcKeyID+1);		//P2

pcEncBuffer = pcEncryptedData;
pcDataBuf = pcData;
if( ulEncMode == USBKEY_ECB )//ecb
{
    ulMaxCmdDataLen = USBKEY_MAX_COM_DATA_LEN;
}
else//cbc
{
    ulMaxCmdDataLen = USBKEY_MAX_COM_DATA_LEN - ulBlockLen;
}
*pulDataLen = 0;

while(ulEncryptedLen)
{
    //判断发送的命令数据的长度
    if(ulEncryptedLen >= ulMaxCmdDataLen)
    {
        ulCmdDataLen = ulMaxCmdDataLen;
    }
    else
    {
        ulCmdDataLen = ulEncryptedLen;
        flag = 1;
    }

    //设置命令数据
    if( ulEncMode == USBKEY_ECB )//ecb
    {
        lc = ulCmdDataLen;
        memcpy(data, pcEncBuffer, ulCmdDataLen);
        le = lc;
    }
    else	//cbc
    {
        lc = ulCmdDataLen+ulBlockLen;
        memcpy(data, pcTmpIV, ulBlockLen);
        memcpy(data+ulBlockLen, pcEncBuffer, ulCmdDataLen);
        le = lc;
    }
    pcEncBuffer += ulCmdDataLen;
    ulEncryptedLen -= ulCmdDataLen;

    //发送命令
    respLen = USBKEY_MAX_COM_LEN;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_MassiveDataDecrypt,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_MassiveDataDecrypt ==>PackageData() Error,rtn = %#08x", rtn);
        return rtn;
    }

    //判断命令是否执行成功
    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_MassiveDataDecrypt ==>DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
            break;
    }

    ///////
    //获得结果
    ///////
    if( ulEncMode == USBKEY_CBC )//cbc
    {
        memcpy(pcTmpIV, pcEncBuffer-ulBlockLen, ulBlockLen);//设置新的IV
        ulTmpLen = respLen - 2;
    }
    else
    {
        ulTmpLen = respLen - 2;
    }


    *pulDataLen += ulTmpLen;
    memcpy(pcDataBuf, response, ulTmpLen);
    if( 1 == flag && USBKEY_PKCS5_PADDING == ulPadMode  )//最后一个数据包，有填充
    {
        //去掉填充
        if( 0 != pkcs5_no_padding(pcData, pulDataLen))//原来数据长度
        {
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_MassiveDataDecrypt ==>pkcs5_no_padding Error");
            return USBKEY_DISCARD_PADDING_ERROR;
        }
    }
    pcDataBuf += ulTmpLen;

}
//Add end


//*/
AK_Log(AK_LOG_DEBUG, "USBKEY_MassiveDataDecrypt Success!");
AK_UnlockMutex(gpMutex);
return USBKEY_OK;
}

//	临时对称密钥加密	ADD by zhoushenshen 20091201
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_TempDataEncrypt(
        USBKEY_HANDLE		hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR	pcKey,		//[in]用来加密的密钥
        USBKEY_ULONG		ulAlg,			//[in]加密算法
        USBKEY_ULONG		ulMode,			//[in]加密模式和填充模式：USBKEY_ECB|USBKEY_NO_PADDING, USBKEY_ECB|USBKEY_PKCS5_PADDING, USBKEY_CBC|USBKEY_NO_PADDING, USBKEY_CBC|USBKEY_PKCS5_PADDING
        USBKEY_UCHAR_PTR	pcIV,			//[in]初始化向量，如果是ecb模式，设置为NULL
        USBKEY_UCHAR_PTR	pcData,			//[in]待加密数据
        USBKEY_ULONG		ulDataLen,		//[in]待加密数据长度
        USBKEY_UCHAR_PTR	pcEncryptedData,//[out]返回的加密后数据
        USBKEY_ULONG_PTR	pulEncryptedLen)//[in/out]in: pcEncryptedData缓冲区大小，out: 加密后数据长度或需要的pcEncryptedData缓冲区大小
{
    //*
    //USBKEY_UCHAR request[USBKEY_MAX_COM_LEN];
    USBKEY_UCHAR response[USBKEY_MAX_COM_LEN];
    USBKEY_UCHAR data[USBKEY_MAX_COM_LEN];
    //USBKEY_UCHAR data[300];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn, respLen=0;
    //	USBKEY_UCHAR p1,p2,lc,le;
    USBKEY_UCHAR p1,p2;
    USBKEY_INT32 lc,le;
    USBKEY_ULONG ulBlockLen,ulKeyLen;
    USBKEY_UCHAR *pcBuffer;
    USBKEY_ULONG ulLen;
    USBKEY_ULONG ulCmdDataLen;
    USBKEY_ULONG ulMaxCmdDataLen;
    USBKEY_ULONG ulEncMode;
    USBKEY_ULONG ulPadMode;
    USBKEY_UCHAR pcTmpIV[16];

    AK_Log(AK_LOG_DEBUG, "USBKEY_TempDataEncrypt");

    AK_LockMutex(gpMutex);

    //检查缓冲区指针合法性
    if( NULL == pcData || NULL == pcEncryptedData || NULL == pcKey || NULL == pulEncryptedLen)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_TempDataEncrypt == PARAM is NULL! rtn = %#08x", USBKEY_PARAM_ERROR);
        return USBKEY_PARAM_ERROR;
    }

    //检查算法合法性，并根据算法设置分组长度
    switch(ulAlg)
    {
        case ALG_TYPE_DES:
            ulKeyLen = 8;
            ulBlockLen = 8;
            break;
        case ALG_TYPE_3DES_2KEY:
            ulKeyLen = 16;
            ulBlockLen = 8;
            break;
        case ALG_TYPE_3DES_3KEY:
            ulKeyLen = 24;
            ulBlockLen = 8;
            break;
        case ALG_TYPE_SSF33:
        case ALG_TYPE_SCB2:
            ulKeyLen = 16;
            ulBlockLen = 16;
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_TempDataEncrypt ==ulAlg Error! ulAlg = %#08x", ulAlg);
            return USBKEY_PARAM_ERROR;
    }

    //检查加密模式的合法性
    ulEncMode = ulMode & 0xf0;
    ulPadMode = ulMode & 0x0f;

    //检查IV指针合法性
    if( ulEncMode == USBKEY_CBC )
    {
        if(NULL == pcIV)
        {
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_TempDataEncrypt ==pcIV is NULL! rtn = %#08x", USBKEY_PARAM_ERROR);
            return USBKEY_PARAM_ERROR;
        }
        else
        {
            memcpy(pcTmpIV, pcIV, ulBlockLen);
        }
    }


    //检查填充模式的合法性，加密数据长度的合法性，输出缓冲区是否够大，根据模式进行pkcs5填充
    if( USBKEY_NO_PADDING == ulPadMode )
    {
        if(ulDataLen%8 != 0)
        {
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_TempDataEncrypt ==ulDataLen Error! ulDataLen = %#08x", ulDataLen);
            return USBKEY_PARAM_ERROR;
        }
        if( (ALG_TYPE_SSF33 == ulAlg || ALG_TYPE_SCB2 == ulAlg) && (ulDataLen%16 != 0) )
        {
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_TempDataEncrypt ==ulDataLen Error! ulDataLen = %#08x", ulDataLen);
            return USBKEY_PARAM_ERROR;
        }
        if(*pulEncryptedLen < ulDataLen)
        {
            AK_UnlockMutex(gpMutex);
            *pulEncryptedLen = ulDataLen;
            AK_Log(AK_LOG_ERROR, "USBKEY_TempDataEncrypt ==> pulEncryptedLen is too small,ulEncryptedLen = %#08x", *pulEncryptedLen);
            return USBKEY_BUFFER_TOO_SHORT;
        }
        memcpy(pcEncryptedData, pcData, ulDataLen);
        ulLen = ulDataLen;
    }
    else if( USBKEY_PKCS5_PADDING == ulPadMode )
    {
        ulLen = ulBlockLen-ulDataLen%ulBlockLen + ulDataLen;
        if(*pulEncryptedLen < ulLen)
        {
            AK_UnlockMutex(gpMutex);
            *pulEncryptedLen = ulLen;
            AK_Log(AK_LOG_ERROR, "USBKEY_TempDataEncrypt ==> pulEncryptedLen is too small,ulEncryptedLen = %#08x", *pulEncryptedLen);
            return USBKEY_BUFFER_TOO_SHORT;
        }

        //PKCS5填充
        memcpy(pcEncryptedData, pcData, ulDataLen);
        ulLen = ulDataLen;
        pkcs5_padding(pcEncryptedData, &ulLen, ulBlockLen);
    }
    else
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_TempDataEncrypt ==ulMode Error! ulPadMode = %#08x", ulPadMode);
        return USBKEY_PARAM_ERROR;
    }

    //Delete by zhoushenshen in 20121210
    /*
    //设置命令头: CLA INS P1 P2 LC
    request[0] = 0xb0;	//CLA
    request[1] = 0x4B;	//INS	
    request[2] = 0x01 + (USBKEY_UCHAR)ulEncMode;	//P1
    request[3] = (USBKEY_UCHAR)ulAlg;		//P2
    request[4] = 0xff;	//LC

    pcBuffer = pcEncryptedData;
     *pulEncryptedLen = 0;
     if( ulEncMode == USBKEY_ECB )//ecb
     {
     ulMaxCmdDataLen = USBKEY_MAX_COM_DATA_LEN - ulBlockLen;
     }
     else//cbc
     {
     ulMaxCmdDataLen = USBKEY_MAX_COM_DATA_LEN - ulBlockLen - ulBlockLen;
     }

     while(ulLen)
     {
    //判断发送的命令数据的长度
    if(ulLen >= ulMaxCmdDataLen)
    {
    ulCmdDataLen = ulMaxCmdDataLen;
    }
    else
    {
    ulCmdDataLen = ulLen;
    }

    //设置命令数据
    if( ulEncMode == USBKEY_ECB )//ecb
    {
    request[5] = (USBKEY_UCHAR)((ulCmdDataLen + ulKeyLen)/256);
    request[6] = (USBKEY_UCHAR)((ulCmdDataLen+ ulKeyLen)%256);
    memcpy(request+7, pcKey, ulKeyLen);
    memcpy(request+7+ulKeyLen, pcBuffer, ulCmdDataLen);
    reqLen = 7 + ulKeyLen + ulCmdDataLen;
    }
    else	//cbc
    {
    request[5] = (USBKEY_UCHAR)((ulCmdDataLen + ulKeyLen + ulBlockLen)/256);
    request[6] = (USBKEY_UCHAR)((ulCmdDataLen + ulKeyLen + ulBlockLen)%256);
    memcpy(request+7, pcKey, ulKeyLen);
    memcpy(request+7+ulKeyLen, pcTmpIV, ulBlockLen);
    memcpy(request+7 + ulKeyLen+ulBlockLen, pcBuffer, ulCmdDataLen);
    reqLen = 7 + ulKeyLen+ ulBlockLen + ulCmdDataLen;
    }

    //发送命令
    respLen = USBKEY_MAX_COM_LEN;
    rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
    if (rtn != 0)
    {
    AK_UnlockMutex(gpMutex);
    AK_Log(AK_LOG_ERROR, "USBKEY_TempDataEncrypt ==>DeviceExecCmd() Error,rtn = %#08x", rtn);
    return rtn;
    }

    //判断命令是否执行成功
    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
    case DEVICE_SUCCESS:
    break;
    default:
    AK_UnlockMutex(gpMutex);
    AK_Log(AK_LOG_ERROR, "USBKEY_TempDataEncrypt ==>DeviceExecCmd() Error,code = %#08x", rtn);
    return rtn;
    break;
}


///////
//获得结果
///////
if( ulEncMode == USBKEY_CBC )//cbc
{
    memcpy(pcTmpIV, response+(respLen-ulBlockLen-2), ulBlockLen);//设置新的IV
    *pulEncryptedLen += respLen-2-ulKeyLen-ulBlockLen;					//设置密文长度
    memcpy(pcBuffer,response+ulKeyLen+ulBlockLen,respLen-2-ulKeyLen-ulBlockLen);	//设置密文(去除无效数据)	
}
else
{
    *pulEncryptedLen += respLen - 2 - ulKeyLen;					//设置密文长度
    memcpy(pcBuffer,response+ulKeyLen,respLen-2-ulKeyLen);			//设置密文
}

pcBuffer += ulCmdDataLen;
ulLen -= ulCmdDataLen;

}*/
//Del end

//Add by zhoushenshen in 20121210
//设置命令头: CLA INS P1 P2 LC
p1 = 0x01 + (USBKEY_UCHAR)ulEncMode;	//P1
p2 = (USBKEY_UCHAR)ulAlg;		//P2

pcBuffer = pcEncryptedData;
*pulEncryptedLen = 0;
if( ulEncMode == USBKEY_ECB )//ecb
{
    ulMaxCmdDataLen = USBKEY_MAX_COM_DATA_LEN - ulBlockLen;
}
else//cbc
{
    ulMaxCmdDataLen = USBKEY_MAX_COM_DATA_LEN - ulBlockLen - ulBlockLen;
}

while(ulLen)
{
    //判断发送的命令数据的长度
    if(ulLen >= ulMaxCmdDataLen)
    {
        ulCmdDataLen = ulMaxCmdDataLen;
    }
    else
    {
        ulCmdDataLen = ulLen;
    }

    //设置命令数据
    if( ulEncMode == USBKEY_ECB )//ecb
    {
        lc = ulCmdDataLen + ulKeyLen;
        memcpy(data, pcKey, ulKeyLen);
        memcpy(data + ulKeyLen, pcBuffer, ulCmdDataLen);
        le = lc;
    }
    else	//cbc
    {
        lc = ulCmdDataLen + ulKeyLen + ulBlockLen;
        memcpy(data, pcKey, ulKeyLen);
        memcpy(data + ulKeyLen, pcTmpIV, ulBlockLen);
        memcpy(data + ulKeyLen + ulBlockLen, pcBuffer, ulCmdDataLen);
        le = lc;
    }

    //发送命令
    respLen = USBKEY_MAX_COM_LEN;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_TempSymmKeyOP,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_TempDataEncrypt ==>PackageData() Error,rtn = %#08x", rtn);
        return rtn;
    }

    //判断命令是否执行成功
    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_TempDataEncrypt ==>DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
            break;
    }


    ///////
    //获得结果
    ///////
    if( ulEncMode == USBKEY_CBC )//cbc
    {
        memcpy(pcTmpIV, response+(respLen-ulBlockLen-2), ulBlockLen);//设置新的IV
        *pulEncryptedLen += respLen-2;					//设置密文长度
        memcpy(pcBuffer,response,respLen-2);	//设置密文(去除无效数据)	
    }
    else
    {
        *pulEncryptedLen += respLen - 2;					//设置密文长度
        memcpy(pcBuffer,response,respLen-2);			//设置密文
    }

    pcBuffer += ulCmdDataLen;
    ulLen -= ulCmdDataLen;

}
//Add end

//*/
AK_Log(AK_LOG_DEBUG, "USBKEY_TempDataEncrypt Success!");
AK_UnlockMutex(gpMutex);
return USBKEY_OK;
}

//	临时对称密钥解密	ADD by zhoushenshen 20091201
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_TempDataDecrypt(
        USBKEY_HANDLE		hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR	pcKey,			//[in]用来解密的密钥
        USBKEY_ULONG		ulAlg,			//[in]解密算法
        USBKEY_ULONG		ulMode,			//[in]解密模式和填充模式：USBKEY_ECB|USBKEY_NO_PADDING, USBKEY_ECB|USBKEY_PKCS5_PADDING, USBKEY_CBC|USBKEY_NO_PADDING, USBKEY_CBC|USBKEY_PKCS5_PADDING
        USBKEY_UCHAR_PTR	pcIV,			//[in]初始化向量，如果是ecb模式，设置为NULL
        USBKEY_UCHAR_PTR	pcEncryptedData,	//[in]密文数据
        USBKEY_ULONG		ulEncryptedLen,	//[in]密文数据长度
        USBKEY_UCHAR_PTR	pcData,			//[out]返回的明文
        USBKEY_ULONG_PTR	pulDataLen)		//[in/out]in: pcData缓冲区长度 out:返回的明文长度
{
    //USBKEY_UCHAR request[USBKEY_MAX_COM_LEN];
    USBKEY_UCHAR response[USBKEY_MAX_COM_LEN];
    USBKEY_UCHAR data[USBKEY_MAX_COM_LEN];
    //USBKEY_UCHAR data[300];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0;
    //USBKEY_UCHAR p1,p2,lc,le;
    USBKEY_UCHAR p1,p2;
    USBKEY_INT32 lc,le;
    USBKEY_ULONG ulBlockLen,ulKeyLen;
    USBKEY_ULONG ulEncMode;
    USBKEY_ULONG ulPadMode;
    USBKEY_UCHAR pcTmpIV[16];
    USBKEY_UCHAR *pcEncBuffer;
    USBKEY_UCHAR *pcDataBuf;
    USBKEY_ULONG ulTmpLen;
    USBKEY_UCHAR flag = 0;
    USBKEY_ULONG ulCmdDataLen;
    USBKEY_ULONG ulMaxCmdDataLen;


    AK_Log(AK_LOG_DEBUG, "USBKEY_TempDataDecrypt");

    AK_LockMutex(gpMutex);

    //检查缓冲区指针合法性
    if( NULL == pcData || NULL == pcEncryptedData || NULL == pcKey || NULL == pulDataLen )
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_TempDataDecrypt ==PARAM is NULL! rtn = %#08x", USBKEY_PARAM_ERROR);
        return USBKEY_PARAM_ERROR;
    }
    if( *pulDataLen < ulEncryptedLen )
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_TempDataDecrypt ==> pulDataLen is too small,ulDataLen = %#08x", *pulDataLen);
        return USBKEY_BUFFER_TOO_SHORT;
    }

    //检查算法合法性，并根据算法设置分组长度
    switch(ulAlg)
    {
        case ALG_TYPE_DES:
            ulKeyLen = 8;
            ulBlockLen = 8;
            break;
        case ALG_TYPE_3DES_2KEY:
            ulKeyLen = 16;
            ulBlockLen = 8;
            break;
        case ALG_TYPE_3DES_3KEY:
            ulKeyLen = 24;
            ulBlockLen = 8;
            break;
        case ALG_TYPE_SSF33:
        case ALG_TYPE_SCB2:
            ulKeyLen = 16;
            ulBlockLen = 16;
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_TempDataDecrypt ==ulMode Error! ulMode = %#08x", ulMode);
            return USBKEY_PARAM_ERROR;
    }

    if(ulEncryptedLen%ulBlockLen != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_TempDataDecrypt ==ulEncryptedLen Error! ulEncryptedLen = %#08x", ulEncryptedLen);
        return USBKEY_PARAM_ERROR;
    }

    //检查加密模式,填充模式的合法性
    ulEncMode = ulMode & 0xf0;
    ulPadMode = ulMode & 0x0f;
    switch(ulPadMode)
    {
        case USBKEY_NO_PADDING:
        case USBKEY_PKCS5_PADDING:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_TempDataDecrypt ==ulMode Error! ulPadMode = %#08x", ulPadMode);
            return USBKEY_PARAM_ERROR;
    }

    //检查IV指针合法性
    if( ulEncMode == USBKEY_CBC )
    {
        if(NULL == pcIV)
        {
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_TempDataDecrypt ==pcIV is NULL! rtn = %#08x", USBKEY_PARAM_ERROR);
            return USBKEY_PARAM_ERROR;
        }
        else
        {
            memcpy(pcTmpIV, pcIV, ulBlockLen);
        }
    }

    //Delete by zhoushenshen in 20121210
    /*//设置命令头: CLA INS P1 P2 LC
      request[0] = 0xb0;	//CLA
      request[1] = 0x4B;	//INS	
      request[2] = 0x00 + (USBKEY_UCHAR)ulEncMode;	//P1
      request[3] = (USBKEY_UCHAR)ulAlg;		//P2
      request[4] = 0xff;	//LC

      pcEncBuffer = pcEncryptedData;
      pcDataBuf = pcData;
      if( ulEncMode == USBKEY_ECB )//ecb
      {
      ulMaxCmdDataLen = USBKEY_MAX_COM_DATA_LEN  - ulBlockLen;
      }
      else//cbc
      {
      ulMaxCmdDataLen = USBKEY_MAX_COM_DATA_LEN  - ulBlockLen - ulBlockLen;
      }
     *pulDataLen = 0;
     while(ulEncryptedLen)
     {
    //判断发送的命令数据的长度
    if(ulEncryptedLen >= ulMaxCmdDataLen)
    {
    ulCmdDataLen = ulMaxCmdDataLen;
    }
    else
    {
    ulCmdDataLen = ulEncryptedLen;
    flag = 1;
    }

    //设置命令数据
    if( ulEncMode == USBKEY_ECB )//ecb
    {
    request[5] = (USBKEY_UCHAR)((ulCmdDataLen + ulKeyLen)/256);
    request[6] = (USBKEY_UCHAR)((ulCmdDataLen+ ulKeyLen)%256);
    memcpy(request+7, pcKey, ulKeyLen);
    memcpy(request+7+ulKeyLen, pcEncBuffer, ulCmdDataLen);
    reqLen = 7 + ulKeyLen + ulCmdDataLen;
    }
    else	//cbc
    {
    request[5] = (USBKEY_UCHAR)((ulCmdDataLen + ulKeyLen + ulBlockLen)/256);
    request[6] = (USBKEY_UCHAR)((ulCmdDataLen + ulKeyLen + ulBlockLen)%256);
    memcpy(request+7, pcKey, ulKeyLen);
    memcpy(request+7+ulKeyLen, pcTmpIV, ulBlockLen);
    memcpy(request+7+ulKeyLen+ulBlockLen, pcEncBuffer, ulCmdDataLen);
    reqLen = 7 + ulKeyLen + ulBlockLen + ulCmdDataLen;
    }
    pcEncBuffer += ulCmdDataLen;
    ulEncryptedLen -= ulCmdDataLen;

    //发送命令
    respLen = USBKEY_MAX_COM_LEN;
    rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
    if (rtn != 0)
    {
    AK_UnlockMutex(gpMutex);
    AK_Log(AK_LOG_ERROR, "USBKEY_TempDataDecrypt ==>DeviceExecCmd() Error,rtn = %#08x", rtn);
    return rtn;
    }

    //判断命令是否执行成功
    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
    case DEVICE_SUCCESS:
    break;
    default:
    AK_UnlockMutex(gpMutex);
    AK_Log(AK_LOG_ERROR, "USBKEY_TempDataDecrypt ==>DeviceExecCmd() Error,code = %#08x", rtn);
    return rtn;
    break;
}


///////
//获得结果
///////
if( ulEncMode == USBKEY_CBC )//cbc
{
    memcpy(pcTmpIV, pcEncBuffer-ulBlockLen, ulBlockLen);//设置新的IV
    ulTmpLen = respLen - 2 - ulKeyLen - ulBlockLen;
}
else
{
    ulTmpLen = respLen - 2 - ulKeyLen;
}


*pulDataLen += ulTmpLen;
memcpy(pcDataBuf, response+ulKeyLen, ulTmpLen);
if( 1 == flag && USBKEY_PKCS5_PADDING == ulPadMode  )//最后一个数据包，有填充
{
    //去掉填充
    if( 0 != pkcs5_no_padding(pcData, pulDataLen))//原来数据长度
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_TempDataDecrypt ==>pkcs5_no_padding Error");
        return USBKEY_DISCARD_PADDING_ERROR;
    }
}
pcDataBuf += ulTmpLen;

}*/
//Del end

//Add by zhoushenshen in 20121210
//设置命令头: CLA INS P1 P2 LC
p1 = 0x00 + (USBKEY_UCHAR)ulEncMode;	//P1
p2 = (USBKEY_UCHAR)ulAlg;		//P2

pcEncBuffer = pcEncryptedData;
pcDataBuf = pcData;
if( ulEncMode == USBKEY_ECB )//ecb
{
    ulMaxCmdDataLen = USBKEY_MAX_COM_DATA_LEN  - ulBlockLen;
}
else//cbc
{
    ulMaxCmdDataLen = USBKEY_MAX_COM_DATA_LEN  - ulBlockLen - ulBlockLen;
}
*pulDataLen = 0;
while(ulEncryptedLen)
{
    //判断发送的命令数据的长度
    if(ulEncryptedLen >= ulMaxCmdDataLen)
    {
        ulCmdDataLen = ulMaxCmdDataLen;
    }
    else
    {
        ulCmdDataLen = ulEncryptedLen;
        flag = 1;
    }

    //设置命令数据
    if( ulEncMode == USBKEY_ECB )//ecb
    {
        lc = ulCmdDataLen + ulKeyLen;
        memcpy(data, pcKey, ulKeyLen);
        memcpy(data+ulKeyLen, pcEncBuffer, ulCmdDataLen);
        le = lc;
    }
    else	//cbc
    {
        lc = ulCmdDataLen + ulKeyLen + ulBlockLen;
        memcpy(data, pcKey, ulKeyLen);
        memcpy(data+ulKeyLen, pcTmpIV, ulBlockLen);
        memcpy(data+ulKeyLen+ulBlockLen, pcEncBuffer, ulCmdDataLen);
        le = lc;
    }
    pcEncBuffer += ulCmdDataLen;
    ulEncryptedLen -= ulCmdDataLen;

    //发送命令
    respLen = USBKEY_MAX_COM_LEN;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_TempSymmKeyOP,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_TempDataDecrypt ==>PackageData() Error,rtn = %#08x", rtn);
        return rtn;
    }

    //判断命令是否执行成功
    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_TempDataDecrypt ==>DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
            break;
    }


    ///////
    //获得结果
    ///////
    if( ulEncMode == USBKEY_CBC )//cbc
    {
        memcpy(pcTmpIV, pcEncBuffer-ulBlockLen, ulBlockLen);//设置新的IV
        ulTmpLen = respLen - 2;
    }
    else
    {
        ulTmpLen = respLen - 2;
    }


    *pulDataLen += ulTmpLen;
    memcpy(pcDataBuf, response, ulTmpLen);
    if( 1 == flag && USBKEY_PKCS5_PADDING == ulPadMode  )//最后一个数据包，有填充
    {
        //去掉填充
        if( 0 != pkcs5_no_padding(pcData, pulDataLen))//原来数据长度
        {
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_TempDataDecrypt ==>pkcs5_no_padding Error");
            return USBKEY_DISCARD_PADDING_ERROR;
        }
    }
    pcDataBuf += ulTmpLen;

}

//*/
AK_Log(AK_LOG_DEBUG, "USBKEY_TempDataDecrypt Success!");
AK_UnlockMutex(gpMutex);
return USBKEY_OK;
}


//	导入临时RSA公钥.	ADD by zhoushenshen 20091125
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ImportTempRSAPublicKey(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcRSAPublicKey,//[in]导入的密钥
        USBKEY_ULONG ulRSAPublicKeyLen,	//[in]导入的密钥长度
        USBKEY_UCHAR_PTR pcPubKeyID)	//[out]返回的公钥ID
{
    //USBKEY_UCHAR request[550];
    //USBKEY_UCHAR response[550];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0;
    USBKEY_UCHAR data[550],fileName[4];
    //USBKEY_UCHAR p1,p2,lc,le;
    //USBKEY_UCHAR p1,p2,le;
    //USBKEY_INT32 lc;

    AK_Log(AK_LOG_DEBUG, "USBKEY_ImportTempRSAPublicKey");

    if ( ((ulRSAPublicKeyLen == 260) && (pcRSAPublicKey[2] != 0x04) ) ||
            ((ulRSAPublicKeyLen == 516) && (pcRSAPublicKey[2] != 0x08) ) )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportTempRSAPublicKey ==>pcRSAPublicKey Error,ulRSAPublicKeyLen = %#08x", ulRSAPublicKeyLen);
        return USBKEY_PARAM_ERROR;
    }

    AK_LockMutex(gpMutex);

    ////////
    //生成文件名
    ////////
    fileName[0] = 0xff;
    fileName[1] = 0x01;
    fileName[2] = 0xff;
    fileName[3] = 0x02;

    rtn = SelectFile(hDeviceHandle, NULL, fileName);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportTempRSAPublicKey ==>SelectFile() Error,rtn = %#08x", rtn);
        return rtn;
    }

    //写入公钥
    //Add by zhoushenshen in 20130109
    rtn = WriteCurBinaryFile(hDeviceHandle,0,ulRSAPublicKeyLen,pcRSAPublicKey);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportTempRSAPublicKey ==>WriteCurBinaryFile() Error,rtn = %#08x",rtn);
        return rtn;
    }

    //Delete by zhoushenshen in 20121204
    /*p1 = 0;
      p2 = 0;
      le = 0x00;

      if ( ulRSAPublicKeyLen > 254)
      {
      lc = 0xff;
      data[0] = (USBKEY_UCHAR)(ulRSAPublicKeyLen/256);
      data[1] = (USBKEY_UCHAR)(ulRSAPublicKeyLen%256);
      memcpy(data+2,pcRSAPublicKey,ulRSAPublicKeyLen);
      }
      else
      {
      lc = (USBKEY_UCHAR)ulRSAPublicKeyLen;
      memcpy(data,pcRSAPublicKey,ulRSAPublicKeyLen);
      }

    //组织数据包
    rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_UpdatePublicKeyBinary,request,&reqLen);
    if (rtn != 0)
    {
    AK_UnlockMutex(gpMutex);
    AK_Log(AK_LOG_ERROR, "USBKEY_ImportTempRSAPublicKey ==>PackageData() Error,rtn = %#08x", rtn);
    return rtn;
    }
    respLen = 300;
    rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
    if (rtn != 0)
    {
    AK_UnlockMutex(gpMutex);
    AK_Log(AK_LOG_ERROR, "USBKEY_ImportTempRSAPublicKey ==>DeviceExecCmd() Error,rtn = %#08x", rtn);
    return rtn;
    }

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
    case DEVICE_SUCCESS:
    break;
    default:
    AK_UnlockMutex(gpMutex);
    AK_Log(AK_LOG_ERROR, "USBKEY_ImportTempRSAPublicKey ==>DeviceExecCmd() Error,code = %#08x", rtn);
    return rtn;
    break;
    }*/
    //Del end

    ///////
    //赋值返回ID
    memcpy(pcPubKeyID,fileName,2);

    AK_Log(AK_LOG_DEBUG, "USBKEY_ImportTempRSAPublicKey Success!");
    AK_UnlockMutex(gpMutex);
    return USBKEY_OK;
}

//	明文导入临时RSA私钥.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ImportTempRSAPrivateKey(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcRSAPrivateKey,	//[in]导入的密钥
        USBKEY_ULONG ulRSAPrivateKeyrLen,	//[in]导入的密钥长度
        USBKEY_UCHAR_PTR pcPriKeyID)	//[out]返回的私钥ID
{
    //USBKEY_UCHAR request[1500];
    //USBKEY_UCHAR response[1500];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0;
    USBKEY_UCHAR data[1500],fileName[4];
    //USBKEY_UCHAR p1,p2,le;
    //USBKEY_INT32 lc;

    AK_Log(AK_LOG_DEBUG, "USBKEY_ImportTempRSAPrivateKey");

    if ( ((ulRSAPrivateKeyrLen == 580) && (pcRSAPrivateKey[2] != 0x04) ) ||
            ((ulRSAPrivateKeyrLen == 1156) && (pcRSAPrivateKey[2] != 0x08) ) )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportTempRSAPublicKey ==>pcRSAPrivateKey Error,ulRSAPrivateKeyrLen = %#08x", ulRSAPrivateKeyrLen);
        return USBKEY_PARAM_ERROR;
    }

    AK_LockMutex(gpMutex);

    ////////
    //生成文件名
    ////////
    fileName[0] = 0xff;
    fileName[1] = 0x01;
    fileName[2] = 0xff;
    fileName[3] = 0x02;

    rtn = SelectFile(hDeviceHandle, NULL, fileName+2);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportTempRSAPrivateKey ==>SelectFile() Error,rtn = %#08x", rtn);
        return rtn;
    }

    //写入私钥
    //Add by zhoushenshen in 20130109
    rtn = WriteCurBinaryFile(hDeviceHandle,0,ulRSAPrivateKeyrLen,pcRSAPrivateKey);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportTempRSAPublicKey ==>WriteCurBinaryFile() Error,rtn = %#08x",rtn);
        return rtn;
    }

    //Delete by zhoushenshen in 20121210
    /*p1 = 0;
      p2 = 0;
      le = 0x00;
      lc = 0xff;
      data[0] = (USBKEY_UCHAR)(ulRSAPrivateKeyrLen/256);
      data[1] = (USBKEY_UCHAR)(ulRSAPrivateKeyrLen%256);
      memcpy(data+2,pcRSAPrivateKey,ulRSAPrivateKeyrLen);


    //组织数据包
    rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_UpdatePrivateKeyBinaryPlain,request,&reqLen);
    if (rtn != 0)
    {
    AK_UnlockMutex(gpMutex);
    AK_Log(AK_LOG_ERROR, "USBKEY_ImportTempRSAPrivateKey ==>PackageData() Error,rtn = %#08x", rtn);
    return rtn;
    }

    respLen = 300;
    rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
    if (rtn != 0)
    {
    AK_UnlockMutex(gpMutex);
    AK_Log(AK_LOG_ERROR, "USBKEY_ImportTempRSAPrivateKey ==>DeviceExecCmd() Error,rtn = %#08x", rtn);
    return rtn;
    }

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
    case DEVICE_SUCCESS:
    break;
    default:
    AK_UnlockMutex(gpMutex);
    AK_Log(AK_LOG_ERROR, "USBKEY_ImportTempRSAPrivateKey ==>DeviceExecCmd() Error,code = %#08x", rtn);
    return rtn;
    break;
    }*/
    //Del end


    ///////
    //赋值返回ID
    memcpy(pcPriKeyID,fileName+2,2);

    AK_Log(AK_LOG_DEBUG, "USBKEY_ImportTempRSAPrivateKey Success!");
    AK_UnlockMutex(gpMutex);
    return USBKEY_OK;
}


//	产生临时RSA非对称密钥.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_GenerateExtRSAKeyPair(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_ULONG ulBitLen,			//[in]密钥模长
        USBKEY_UCHAR_PTR pPriKeyStrc)	//[out]返回的私钥结构
{
    USBKEY_ULONG rtn = 0;
    //USBKEY_UCHAR request[256];
    USBKEY_UCHAR response[1500];
    USBKEY_UCHAR data[7];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG respLen=0;
    USBKEY_ULONG ulLen;
    USBKEY_UCHAR p1,p2,lc;
    USBKEY_INT32 le;

    AK_Log(AK_LOG_DEBUG, "USBKEY_GenerateExtRSAKey");
    AK_LockMutex(gpMutex);

    //Delete by zhoushenshen in 20121210
    //设置命令头: CLA INS P1 P2 LC
    /*request[0] = 0xb0;	//CLA
      request[1] = 0x4e;	//INS

      if(1024 == ulBitLen)
      {
      request[2] = 0x01;	//P1
      ulLen = 5*128+64;
      }
      else if(2048 == ulBitLen)
      {
      request[2] = 0x02;	//P1
      ulLen = 5*256+128;
      }
      else
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_GenerateExtRSAKey ==>ulBitLen Error,ulBitLen = %#08x", ulBitLen);
      return USBKEY_PARAM_ERROR;
      }

      request[3] = 0x00;	//P2
      request[4] = 0xff;	//LE
      request[5] = (USBKEY_UCHAR)(ulLen/256);
      request[6] = (USBKEY_UCHAR)(ulLen%256);

    //发送命令
    reqLen = 7;
    respLen = 1500;
    rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
    if (rtn != 0)
    {
    AK_UnlockMutex(gpMutex);
    AK_Log(AK_LOG_ERROR, "USBKEY_GenerateExtRSAKey ==>DeviceExecCmd() Error,rtn = %#08x", rtn);
    return rtn;
    }*/
    //Del end

    //Add by zhoushenshen in 20121210
    if(1024 == ulBitLen)
    {
        p1 = 0x01;	//P1
        ulLen = 5*128+64;
    }
    else if(2048 == ulBitLen)
    {
        p1 = 0x02;	//P1
        ulLen = 5*256+128;
    }
    else
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_GenerateExtRSAKey ==>ulBitLen Error,ulBitLen = %#08x", ulBitLen);
        return USBKEY_PARAM_ERROR;
    }

    p2 = 0x00;	//P2
    lc = 0x00;
    le = ulLen;

    //发送命令
    respLen = 1500;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_GenExtRSAKeyPair,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_GenerateExtRSAKey ==> PackageData() Error, rtn = %#08x", rtn);
        return rtn;
    }
    //Add end 

    //判断命令是否执行成功
    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_GenerateExtRSAKey ==>DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
            break;
    }
    Reverser(&ulBitLen,sizeof(ulBitLen));
    memcpy(pPriKeyStrc,&ulBitLen,4);
    Reverser(&ulBitLen,sizeof(ulBitLen));
    if(1024 == ulBitLen)
    {
        memcpy(pPriKeyStrc+4,response,256);
        memcpy(pPriKeyStrc+260,response+256+128,respLen-2-384);
    }
    else if(2048 == ulBitLen)
    {
        memcpy(pPriKeyStrc+4,response,512);
        memcpy(pPriKeyStrc+516,response+512+256,respLen-2-768);
    }

    AK_Log(AK_LOG_DEBUG, "USBKEY_GenerateExtRSAKey Success!");
    AK_UnlockMutex(gpMutex);
    return USBKEY_OK;
}

/*//	产生1024位临时RSA非对称密钥.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_GenerateTempRSAKeyPair(
USBKEY_HANDLE hDeviceHandle)	//[in]设备句柄
{
USBKEY_UCHAR request[300];
USBKEY_UCHAR response[300];
USBKEY_ULONG rtn,reqLen,respLen=0; //ulAccessCondition,ulFileSize;
USBKEY_UCHAR fileName[4];//,data[16];
USBKEY_UCHAR p1,p2,lc,le;

LogMessage("->USBKEY_GenerateTempRSAKeyPair");

AK_LockMutex(gpMutex);


////////
//获得文件名
////////
fileName[0] = 0xff;
fileName[1] = 0x01;
fileName[2] = 0xff;
fileName[3] = 0x02;*/

/*//创建公私钥文件
  p1 = 0x00;
  p2 = 0x10;
  lc = 9;
  le = 0x00;

  memset(data,0,9);
  memcpy(data,fileName,2);
  le = ulAccessCondition;
  memcpy(data+3,&le,1);
  memcpy(data+4,&le,1);
  ulFileSize = 516;
  le = ulFileSize/256;
  memcpy(data+7,&le,1);
  le = ulFileSize;
  memcpy(data+8,&le,1);

  rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_CreateFile,request,&reqLen);
  if (rtn != 0)
  {
  AK_UnlockMutex(gpMutex);
  LogMessage16Val("====>>USBKEY_GenerateTempRSAKeyPair->PackageData() Error,rtn = 0x",rtn);
  return rtn;
  }

  respLen = 300;
  rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,&respLen);
  if (rtn != 0)
  {
  AK_UnlockMutex(gpMutex);
  LogMessage16Val("====>>USBKEY_GenerateTempRSAKeyPair->DeviceExecCmd() Error,rtn = 0x",rtn);
  return rtn;
  }

  rtn = *(response+respLen-2);
  rtn = rtn*256 + *(response+respLen-1);
  switch(rtn)
  {
  case DEVICE_SUCCESS:
  break;
  case DEVICE_CMD_DATA_ERROR:
  LogMessage16Val("====>>USBKEY_GenerateTempRSAKeyPair->Public Key File is Exist,code = 0x",rtn);
  break;
  default:
  AK_UnlockMutex(gpMutex);
  LogMessage16Val("====>>USBKEY_GenerateTempRSAKeyPair->DeviceExecCmd() Error,code = 0x",rtn);
  return rtn;
  break;
  }

  LogMessage("USBKEY_GenerateTempRSAKeyPair Create Public Key File Success.");

  p1 = 0x00;
  p2 = 0x11;
  lc = 9;
  le = 0x00;

  memset(data,0,9);
  memcpy(data,fileName+2,2);
  le = ulAccessCondition;
  memcpy(data+3,&le,1);
  memcpy(data+4,&le,1);
  ulFileSize = 1412;
  le = ulFileSize/256;
  memcpy(data+7,&le,1);
  le = ulFileSize;
  memcpy(data+8,&le,1);

  rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_CreateFile,request,&reqLen);
  if (rtn != 0)
  {
AK_UnlockMutex(gpMutex);
LogMessage16Val("====>>USBKEY_GenerateTempRSAKeyPair->PackageData() Error,rtn = 0x",rtn);
return rtn;
}

respLen = 300;
rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,&respLen);
if (rtn != 0)
{
    AK_UnlockMutex(gpMutex);
    LogMessage16Val("====>>USBKEY_GenerateTempRSAKeyPair->DeviceExecCmd() Error,rtn = 0x",rtn);
    return rtn;
}

rtn = *(response+respLen-2);
rtn = rtn*256 + *(response+respLen-1);
switch(rtn)
{
    case DEVICE_SUCCESS:
        break;
    case DEVICE_CMD_DATA_ERROR:
        LogMessage16Val("====>>USBKEY_GenerateTempRSAKeyPair->Private Key File is Exist,code = 0x",rtn);
        break;
    default:
        AK_UnlockMutex(gpMutex);
        LogMessage16Val("====>>USBKEY_GenerateTempRSAKeyPair->DeviceExecCmd() Error,code = 0x",rtn);
        return rtn;
        break;
}

LogMessage("USBKEY_GenerateTempRSAKeyPair Create Private Key File Success.");*/

/*p1 = 0x01;
  p2 = 0x00;
  lc = 0x04;
  le = 0x00;

  rtn = PackageData(p1,p2,lc,fileName,le,USBKEY_OP_GenRSAKeyPair,request,&reqLen);
  if (rtn != 0)
  {
  AK_UnlockMutex(gpMutex);
  LogMessage16Val("====>>USBKEY_GenerateTempRSAKeyPair->PackageData() Error,rtn = 0x",rtn);
  return rtn;
  }

  respLen = 300;
  rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,&respLen);
  if (rtn != 0)
  {
  AK_UnlockMutex(gpMutex);
  LogMessage16Val("====>>USBKEY_GenerateTempRSAKeyPair->DeviceExecCmd() Error,rtn = 0x",rtn);
  return rtn;
  }
  rtn = *(response+respLen-2);
  rtn = rtn*256 + *(response+respLen-1);
  switch(rtn)
  {
  case DEVICE_SUCCESS:
  break;
  default:
  AK_UnlockMutex(gpMutex);
  LogMessage16Val("====>>USBKEY_GenerateTempRSAKeyPair->DeviceExecCmd() Error,code = 0x",rtn);
  return rtn;
  break;
  }

  AK_UnlockMutex(gpMutex);

///////
LogMessage("->USBKEY_GenerateTempRSAKeyPair Success");

return USBKEY_OK;
}*/

//	临时RSA私钥运算
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_TempRSAPrivateKeyOperation(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcPriKey,		//[in]私钥结构	USBKEY_RSA_PRIVATE_KEY_IMPORT_2048/1024
        USBKEY_ULONG ulPriKeyLen,		//[in]私钥的长度
        USBKEY_UCHAR_PTR pcInData,		//[in]输入数据
        USBKEY_ULONG ulInDataLen,		//[in]输入数据长度
        USBKEY_UCHAR_PTR pcOutData,		//[out]返回的输出
        USBKEY_ULONG_PTR pulOutDataLen)	//[out]返回的输出长度
{
    USBKEY_ULONG rtn = 0;
    //USBKEY_UCHAR request[1000];
    USBKEY_UCHAR response[1500];
    //USBKEY_ULONG reqLen;
    USBKEY_UCHAR data[1000];
    USBKEY_ULONG respLen=0;
    USBKEY_ULONG ulLen;
    USBKEY_UCHAR p1,p2;
    USBKEY_INT32 lc,le;

    AK_Log(AK_LOG_DEBUG, "USBKEY_TempRSAPrivateKeyOperation");

    if ((ulInDataLen != 128) && (ulInDataLen != 256) )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_TempRSAPrivateKeyOperation ==> ulInDataLen Error,ulInDataLen = %#08x", ulInDataLen);
        return USBKEY_PARAM_ERROR;
    }
    if (*pulOutDataLen < ulInDataLen)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_TempRSAPrivateKeyOperation ==> pulOutDataLen is too small,ulOutDataLen = %#08x", *pulOutDataLen);
        return USBKEY_PARAM_ERROR;
    }

    if ( ((ulPriKeyLen == 324) && (pcPriKey[2] != 0x04) ) ||
            ((ulPriKeyLen == 644) && (pcPriKey[2] != 0x08) ) )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_TempRSAPrivateKeyOperation ==> pcPriKey Error,ulPriKeyLen = %#08x", ulPriKeyLen);
        return USBKEY_PARAM_ERROR;
    }

    AK_LockMutex(gpMutex);

    //Delete by zhoushenshen in 20121212
    /*//设置命令头: CLA INS P1 P2 LC
      request[0] = 0xb0;	//CLA
      request[1] = 0x4f;	//INS

      if(pcPriKey[2] == 0x04)
      request[2] = 0x01;	//P1

      else if(pcPriKey[2] == 0x08)
      request[2] = 0x02;	//P1
      else
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_TempRSAPrivateKeyOperation ==> ulBitLen Error,pcPriKey = %#08x", pcPriKey[2]);
      return USBKEY_PARAM_ERROR;
      }

      ulLen = ulPriKeyLen - 4 + ulInDataLen;
      request[3] = 0x02;	//P2
      request[4] = 0xff;	//LC
      request[5] = (USBKEY_UCHAR)(ulLen/256);
      request[6] = (USBKEY_UCHAR)(ulLen%256);

    ////////
    //生成数据
    ////////
    reqLen = 7;
    memcpy(request+reqLen,pcPriKey+4,ulPriKeyLen-4);
    reqLen += ulPriKeyLen-4;
    memcpy(request+reqLen,pcInData,ulInDataLen);
    reqLen += ulInDataLen;

    //发送命令
    respLen = 1500;
    rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
    if (rtn != 0)
    {
    AK_UnlockMutex(gpMutex);
    AK_Log(AK_LOG_ERROR, "USBKEY_TempRSAPrivateKeyOperation ==> DeviceExecCmd() Error,rtn = %#08x", rtn);
    return rtn;
    }*/
    //Del end

    //Add by zhoushenshen in 20121212
    //组包
    if(pcPriKey[2] == 0x04)
        p1 = 0x01;	//P1

    else if(pcPriKey[2] == 0x08)
        p1 = 0x02;	//P1
    else
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_TempRSAPrivateKeyOperation ==> ulBitLen Error,pcPriKey = %#08x", pcPriKey[2]);
        return USBKEY_PARAM_ERROR;
    }

    ulLen = ulPriKeyLen - 4 + ulInDataLen;
    p2 = 0x02;	//P2
    lc = ulLen;
    le = lc;

    ////////
    //生成数据
    ////////
    memcpy(data,pcPriKey+4,ulPriKeyLen-4);
    memcpy(data+ulPriKeyLen-4,pcInData,ulInDataLen);

    //发送命令
    respLen = 1500;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_TempRSAOP,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_TempRSAPrivateKeyOperation ==>PackageData() Error,rtn = %#08x", rtn);
        return rtn;
    }
    //Add end

    //判断命令是否执行成功
    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_TempRSAPrivateKeyOperation ==> DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
            break;
    }


    ///////
    //获得结果
    ///////
    //Delete by zhoushenshen in 20130109
    /**pulOutDataLen = respLen-2-ulPriKeyLen+4;
      memcpy(pcOutData,response+ulPriKeyLen-4,*pulOutDataLen);*/
    *pulOutDataLen = respLen-2;
    memcpy(pcOutData,response,respLen-2);

    AK_Log(AK_LOG_DEBUG, "USBKEY_TempRSAPrivateKeyOperation Success!");
    AK_UnlockMutex(gpMutex);

    return USBKEY_OK;
}

//	临时RSA公钥运算
//	返回值：
//		0：	成功.
//		>0:	失败，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_TempRSAPublicKeyOperation(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcPubKey,		//[in]公钥结构	USBKEY_RSA_PUBLIC_KEY_2048/1024
        USBKEY_ULONG ulPubKeyLen,		//[in]公钥的长度
        USBKEY_UCHAR_PTR pcInData,		//[in]输入数据
        USBKEY_ULONG ulInDataLen,		//[in]输入数据长度
        USBKEY_UCHAR_PTR pcOutData,		//[out]输出数据
        USBKEY_ULONG_PTR pulOutDataLen)	//[out]输出数据长度
{
    USBKEY_ULONG rtn = 0;
    //USBKEY_UCHAR request[1000];
    USBKEY_UCHAR response[1500];
    USBKEY_UCHAR data[1000];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG respLen=0;
    USBKEY_ULONG ulLen;
    USBKEY_UCHAR p1,p2;
    USBKEY_INT32 lc,le;

    AK_Log(AK_LOG_DEBUG, "USBKEY_TempRSAPublicKeyOperationt");

    if ((ulInDataLen != 128) && (ulInDataLen != 256) )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_TempRSAPublicKeyOperationt ==>ulInDataLen Error,ulInDataLen = %#08x", ulInDataLen);
        return USBKEY_PARAM_ERROR;
    }
    if (*pulOutDataLen < ulInDataLen)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_TempRSAPublicKeyOperationt ==> pulOutDataLen is too small,ulOutDataLen = %#08x", *pulOutDataLen);
        return USBKEY_PARAM_ERROR;
    }

    if ( ((ulPubKeyLen == 260) && (pcPubKey[2] != 0x04) ) ||
            ((ulPubKeyLen == 516) && (pcPubKey[2] != 0x08) ) )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_TempRSAPublicKeyOperationt ==>pcPubKey Error,ulPubKeyLen = %#08x", ulPubKeyLen);
        return USBKEY_PARAM_ERROR;
    }

    AK_LockMutex(gpMutex);

    //Delete by zhoushenshen in 20121212
    /*//设置命令头: CLA INS P1 P2 LC
      request[0] = 0xb0;	//CLA
      request[1] = 0x4f;	//INS

      if(pcPubKey[2] == 0x04)
      request[2] = 0x01;	//P1
      else if(pcPubKey[2] == 0x08)
      request[2] = 0x02;	//P1
      else
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_TempRSAPublicKeyOperation ==> ulBitLen Error,pcPubKey = %#08x", pcPubKey[2]);
      return USBKEY_PARAM_ERROR;
      }

      ulLen = ulPubKeyLen - 4 + ulInDataLen;
      request[3] = 0x01;	//P2
      request[4] = 0xff;	//LC
      request[5] = (USBKEY_UCHAR)(ulLen/256);
      request[6] = (USBKEY_UCHAR)(ulLen%256);

    ////////
    //生成数据
    ////////
    reqLen = 7;
    memcpy(request+reqLen,pcPubKey+4,ulPubKeyLen-4);
    reqLen += ulPubKeyLen-4;
    memcpy(request+reqLen,pcInData,ulInDataLen);
    reqLen += ulInDataLen;

    //发送命令
    respLen = 1500;
    rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
    if (rtn != 0)
    {
    AK_UnlockMutex(gpMutex);
    AK_Log(AK_LOG_ERROR, "USBKEY_TempRSAPublicKeyOperationt ==>DeviceExecCmd() Error,rtn = %#08x", rtn);
    return rtn;
    }*/
    //Del end

    //Add by zhoushenshen in 20121212
    //组包
    if(pcPubKey[2] == 0x04)
        p1 = 0x01;	//P1
    else if(pcPubKey[2] == 0x08)
        p1 = 0x02;	//P1
    else
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_TempRSAPublicKeyOperation ==> ulBitLen Error,pcPubKey = %#08x", pcPubKey[2]);
        return USBKEY_PARAM_ERROR;
    }

    ulLen = ulPubKeyLen - 4 + ulInDataLen;
    p2 = 0x01;	//P2
    lc = ulLen;
    le = lc;


    ////////
    //生成数据
    ////////
    memcpy(data,pcPubKey+4,ulPubKeyLen-4);
    memcpy(data+ulPubKeyLen-4,pcInData,ulInDataLen);

    //发送命令
    respLen = 1500;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_TempRSAOP,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_TempRSAPublicKeyOperation ==>PackageData() Error,rtn = %#08x", rtn);
        return rtn;
    }

    //判断命令是否执行成功
    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_TempRSAPublicKeyOperationt ==>DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
            break;
    }


    ///////
    //获得结果
    ///////
    //Delete by zhoushenshen in 20130109
    /**pulOutDataLen = respLen-2-ulPubKeyLen+4;
      memcpy(pcOutData,response+ulPubKeyLen-4,*pulOutDataLen);*/
    *pulOutDataLen = respLen-2;
    memcpy(pcOutData,response,respLen-2);

    AK_Log(AK_LOG_DEBUG, "USBKEY_TempRSAPublicKeyOperationt Success!");
    AK_UnlockMutex(gpMutex);

    return USBKEY_OK;
}

//	写入密钥容器证书.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_WriteCertificate(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcContainerName,//[in]密钥容器名称
        USBKEY_ULONG ulKeyUsage,		//[in]密钥用途，KEY_EXCHANGE密钥或者KEY_SIGNATURE密钥
        USBKEY_UCHAR_PTR pcData,		//[in]证书数据
        USBKEY_ULONG_PTR pulSize)		//[in,out]输入写入数据缓冲区大小，输出实际写入大小
{
    //USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0,recNO,ulAccessCondition,ulFileSize;
    USBKEY_UCHAR fileName[2],data[16],temp[USBKEY_FILE_RECORD_LEN];
    USBKEY_UCHAR p1,p2,lc,le;

    AK_Log(AK_LOG_DEBUG, "USBKEY_WriteCertificate");

    /*if (strlen(pcAppName) > 16)
      {
      LogMessage("====>>USBKEY_WriteCertificate->pcAppName Length Error!");
      return USBKEY_PARAM_ERROR;
      }*/
    if (strlen((char *)pcContainerName) > 254)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_WriteCertificate ==> pcContainerName Length Error,Max Name Len is 255!");
        return USBKEY_PARAM_ERROR;
    }

    if ((ulKeyUsage != KEY_EXCHANGE) && (ulKeyUsage != KEY_SIGNATURE) )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_WriteCertificate ==>ulKeyUsage Error,ulKeyUsage = %#08x", ulKeyUsage);
        return USBKEY_PARAM_ERROR;
    }

    AK_LockMutex(gpMutex);

    /*rtn = SelectFile(hDeviceHandle,pcAppName,NULL);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage16Val("====>>USBKEY_WriteCertificate->SelectFile() Error, rtn = 0x",rtn);
      return rtn;
      }*/

    /////////检查权限///////////////////////////////////////////////////
    /*rtn = CheckPrivillege(hDeviceHandle);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage16Val("====>>USBKEY_WriteCertificate->CheckPrivillege() Error,rtn = 0x",rtn);
      return rtn;
      }*/
    //////////////////////////////////////////////////////////////////////////

    rtn = SearchContainerRecNo(hDeviceHandle,pcContainerName,&recNO);
    if( (rtn != 0) || (recNO == 0) )
    {
        AK_UnlockMutex(gpMutex);
        if(recNO == 0)
        {
            AK_Log(AK_LOG_ERROR, "USBKEY_WriteCertificate ==> not find container record.");
            return USBKEY_RECORD_NOT_FOUND;
        }
        AK_Log(AK_LOG_ERROR, "USBKEY_WriteCertificate ==> SearchContainerRecNo() Error,rtn = %#08x", rtn);
        return rtn;
    }

    ////////
    //生成文件名
    ////////
    memcpy(fileName,&recNO,1);
    if (ulKeyUsage == KEY_EXCHANGE)
    {
        memset(fileName+1,0x03,1);
    }
    else if (ulKeyUsage == KEY_SIGNATURE)
    {
        memset(fileName+1,0x06,1);
    }

    //创建文件
    //写入记录文件信息 格式：文件ID(2字节)+读权限(1字节)+写权限(1字节)+大小(4字节)+文件名
    p1 = 0x00;
    p2 = 0x02;
    //lc = 9;
    lc = 0x07;						//lc updtae in 20120106
    le = 0x00;
    memset(data,0,9);
    memset(temp,0,USBKEY_FILE_RECORD_LEN);

    memcpy(data,fileName,2);	
    memcpy(temp,fileName,2);
    ulAccessCondition = USBKEY_ALL_PRIV;
    memcpy(data+3,&le,1);		//读权限
    memcpy(temp+2,&ulAccessCondition,1);
    ulAccessCondition = USBKEY_USER_USE_PRIV;
    memcpy(temp+3,&ulAccessCondition,1);		
    memcpy(data+4,&le,1);		//写权限

    //updtae in 20120106 文件长度由4字节改为2字节,文件记录中文件长度维持4字节不变
    ulFileSize = *pulSize;
    le = (USBKEY_UCHAR)ulFileSize;
    //memcpy(data+8,&le,1);
    memcpy(data+6,&le,1);
    memcpy(temp+7,&le,1);
    ulFileSize = ulFileSize/256;
    le = (USBKEY_UCHAR)ulFileSize;
    //memcpy(data+7,&le,1);
    memcpy(data+5,&le,1);
    memcpy(temp+6,&le,1);
    ulFileSize = ulFileSize/256;
    le = (USBKEY_UCHAR)ulFileSize;
    //memcpy(data+6,&le,1);
    memcpy(temp+5,&le,1);
    ulFileSize = ulFileSize/256;
    le = (USBKEY_UCHAR)ulFileSize;
    //memcpy(data+5,&le,1);
    memcpy(temp+4,&le,1);

    memcpy(temp+8,fileName,2);

    //Delete by zhoushenshen in 20121107
    /*rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_CreateFile,request,&reqLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_WriteCertificate ==>PackageData() Error,rtn = %#08x", rtn);
      return rtn;
      }

      respLen = 300;
      rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_WriteCertificate ==>DeviceExecCmd() Error,rtn = %#08x", rtn);
      return rtn;
      }*/
    //Del end

    //Add by zhoushenshen in 20121107
    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_CreateFile,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_WriteCertificate ==> PackageData() Error, rtn = %#08x", rtn);
        return rtn;
    }
    //Add end

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        case DEVICE_CMD_DATA_ERROR:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_WriteCertificate Certificate File Exist,code = %#08x", rtn);
            return DEVICE_CMD_DATA_ERROR;
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_WriteCertificate ==>DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
            break;
    }

    AK_Log(AK_LOG_DEBUG, "USBKEY_WriteCertificate Create Certificate File Success.");

    rtn = AddFileRecord(hDeviceHandle,temp);	//Update by zhoushenshen 20100309
    AK_UnlockMutex(gpMutex);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_WriteCertificate ==>AddFileRecord() Error,rtn = %#08x", rtn);
        return rtn;
    }

    //写入证书
    rtn = USBKEY_WriteFile(hDeviceHandle,temp+8, 0 , pcData, pulSize);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_WriteCertificate ==>USBKEY_WriteFile() Error,rtn = %#08x", rtn);
        return rtn;
    }

    AK_LockMutex(gpMutex);
    rtn = AddContainerCert(hDeviceHandle,recNO,ulKeyUsage);
    AK_UnlockMutex(gpMutex);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_WriteCertificate ==>AddContainerCert() Error,rtn = %#08x", rtn);
        return rtn;
    }

    ///////
    //赋值返回


    ///////
    AK_Log(AK_LOG_DEBUG, "USBKEY_WriteCertificate Success!");

    return USBKEY_OK;
}

//	读取密钥容器.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ReadCertificate(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcContainerName,//[in]密钥容器名称
        USBKEY_ULONG ulKeyUsage,		//[in]密钥用途，KEY_EXCHANGE密钥或者KEY_SIGNATURE密钥
        USBKEY_UCHAR_PTR pcData,		//[out]证书数据,若为NULL ，返回证书长度
        USBKEY_ULONG_PTR pulSize)		//[in,out]输入读取数据缓冲区大小，输出实际读取长度
{
    USBKEY_ULONG rtn,recNO,len;
    USBKEY_UCHAR fileName[3];
    USBKEY_UCHAR fileInfo[USBKEY_FILE_RECORD_LEN];

    AK_Log(AK_LOG_DEBUG, "USBKEY_ReadCertificate");

    /*if (strlen(pcAppName) > 16)
      {
      LogMessage("====>>USBKEY_ReadCertificate->pcAppName Length Error!");
      return USBKEY_PARAM_ERROR;
      }*/
    if (strlen((char *)pcContainerName) > 254)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ReadCertificate ==> pcContainerName Length Error,Max Name Len is 255!");
        return USBKEY_PARAM_ERROR;
    }

    if ((ulKeyUsage != KEY_EXCHANGE) && (ulKeyUsage != KEY_SIGNATURE) )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ReadCertificate ==>ulKeyUsage Error,ulKeyUsage = %#08x", ulKeyUsage);
        return USBKEY_PARAM_ERROR;
    }

    AK_LockMutex(gpMutex);

    /*rtn = SelectFile(hDeviceHandle,pcAppName,NULL);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage16Val("====>>USBKEY_ReadCertificate->SelectFile() Error, rtn = 0x",rtn);
      return rtn;
      }*/

    /////////检查权限///////////////////////////////////////////////////
    // 	rtn = CheckPrivillege(hDeviceHandle);
    // 	if (rtn != 0)
    // 	{
    // 		LogMessage16Val("====>>USBKEY_ReadCertificate->CheckPrivillege() Error,rtn = 0x",rtn);
    // 		return rtn;
    // 	}
    //////////////////////////////////////////////////////////////////////////

    rtn = SearchContainerRecNo(hDeviceHandle,pcContainerName,&recNO);
    if( (rtn != 0) || (recNO == 0) )
    {
        AK_UnlockMutex(gpMutex);
        if(recNO == 0)
        {
            AK_Log(AK_LOG_ERROR, "USBKEY_ReadCertificate ==> not find container record.");
            return USBKEY_RECORD_NOT_FOUND;
        }
        AK_Log(AK_LOG_ERROR, "USBKEY_ReadCertificate ==> SearchContainerRecNo() Error,rtn = %#08x", rtn);
        return rtn;
    }

    ////////
    //生成文件名
    ////////
    memset(fileName,0,3);
    memcpy(fileName,&recNO,1);
    if (ulKeyUsage == KEY_EXCHANGE)
    {
        memset(fileName+1,0x03,1);
    }
    else if (ulKeyUsage == KEY_SIGNATURE)
    {
        memset(fileName+1,0x06,1);
    }

    if (pcData == NULL)
    {
        rtn = GetFileInfo(hDeviceHandle,fileName,fileInfo);
        if (rtn != 0)
        {
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_ReadCertificate ==>GetFileInfo() Error,rtn = %#08x", rtn);
            return rtn;
        }
        AK_UnlockMutex(gpMutex);

        //ADD by zhoushenshen 20091125
        len = fileInfo[4];
        len = len*256 + fileInfo[5];
        len = len*256 + fileInfo[6];
        len = len*256 + fileInfo[7];
        *pulSize = len;
        return USBKEY_OK;
    }

    AK_UnlockMutex(gpMutex);

    //读取证书
    rtn = USBKEY_ReadFile(hDeviceHandle,fileName, 0 , pcData, pulSize);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ReadCertificate ==>USBKEY_ReadFile() Error,rtn = %#08x", rtn);
        return rtn;
    }

    ///////
    //赋值返回


    ///////
    AK_Log(AK_LOG_DEBUG, "USBKEY_ReadCertificate Success!");

    return USBKEY_OK;
}


//	判断证书是否存在
//	返回值：
//		0: 存在
//		-1:不存在
//		>0:错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_IsCertificateExist(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcContainerName,//[in]密钥容器名称
        USBKEY_ULONG ulKeyUsage)		//[in]密钥用途，KEY_EXCHANGE密钥或者KEY_SIGNATURE密钥
{
    USBKEY_ULONG rtn,recNO;
    USBKEY_UCHAR data[16],fileID[2];
    USBKEY_ULONG reclen;
    USBKEY_UCHAR record[16];

    AK_Log(AK_LOG_DEBUG, "USBKEY_IsCertificateExist");

    if (strlen((char *)pcContainerName) > 254)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_IsCertificateExist ==> pcContainerName Length Error,Max Name Len is 255!");
        return USBKEY_PARAM_ERROR;
    }

    AK_LockMutex(gpMutex);

    rtn = SearchContainerRecNo(hDeviceHandle,pcContainerName,&recNO);
    if( (rtn != 0) || (recNO == 0) )
    {
        AK_UnlockMutex(gpMutex);
        if(recNO == 0)
        {
            AK_Log(AK_LOG_ERROR, "USBKEY_IsCertificateExist ==> not find container record.");
            return USBKEY_RECORD_NOT_FOUND;
        }
        AK_Log(AK_LOG_ERROR, "USBKEY_IsCertificateExist ==> SearchContainerRecNo() Error,rtn = %#08x", rtn);
        return rtn;
    }

    //查找密钥容器内的文件
    fileID[0] = 0xef;
    fileID[1] = (USBKEY_UCHAR)recNO;

    rtn = SelectFile(hDeviceHandle,NULL,fileID);		
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_IsCertificateExist ==> SelectFile() Error,rtn = %#08x", rtn);
        return rtn;
    }

    memset(record,0xff,sizeof(record));
    reclen = 16;
    switch(ulKeyUsage)
    {
        case KEY_EXCHANGE:
            rtn = ReadRecord(hDeviceHandle,fileID,3,data,&reclen);
            if (rtn != 0)
            {
                AK_UnlockMutex(gpMutex);
                AK_Log(AK_LOG_ERROR, "USBKEY_IsCertificateExist ==> ReadRecord() EXCHANGE Error,rtn = %#08x", rtn);
                return rtn;
            }
            break;
        case KEY_SIGNATURE:
            rtn = ReadRecord(hDeviceHandle,fileID,6,data,&reclen);
            if (rtn != 0)
            {
                AK_UnlockMutex(gpMutex);
                AK_Log(AK_LOG_ERROR, "USBKEY_IsCertificateExist ==> ReadRecord() SIGNATURE Error,rtn = %#08x", rtn);
                return rtn;
            }
            break;
    }
    AK_Log(AK_LOG_DEBUG, "USBKEY_IsCertificateExist Success!");
    AK_UnlockMutex(gpMutex);

    if ( (reclen >= 2) && (memcmp(data,record,2) != 0) )
        return USBKEY_OK;		//证书存在
    else
        return -1;
}


//	删除密钥容器证书.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DeleteCertificate(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcContainerName,//[in]密钥容器名称
        USBKEY_ULONG ulKeyUsage)		//[in]密钥用途，KEY_EXCHANGE密钥或者KEY_SIGNATURE密钥
{
    USBKEY_ULONG rtn,recNO;
    USBKEY_UCHAR fileName[3];

    AK_Log(AK_LOG_DEBUG, "USBKEY_DeleteCertificate");

    /*if (strlen(pcAppName) > 16)
      {
      LogMessage("====>>USBKEY_DeleteCertificate->pcAppName Length Error!");
      return USBKEY_PARAM_ERROR;
      }*/
    if (strlen((char *)pcContainerName) > 254)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_DeleteCertificate ==> pcContainerName Length Error,Max Name Len is 255!");
        return USBKEY_PARAM_ERROR;
    }

    if ((ulKeyUsage != KEY_EXCHANGE) && (ulKeyUsage != KEY_SIGNATURE) )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_DeleteCertificate ==>ulKeyUsage Error,ulKeyUsage = %#08x", ulKeyUsage);
        return USBKEY_PARAM_ERROR;
    }

    AK_LockMutex(gpMutex);

    /*rtn = SelectFile(hDeviceHandle,pcAppName,NULL);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage16Val("====>>USBKEY_DeleteCertificate->SelectFile() Error, rtn = 0x",rtn);
      return rtn;
      }*/

    /////////检查权限///////////////////////////////////////////////////
    /*rtn = CheckPrivillege(hDeviceHandle);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage16Val("====>>USBKEY_DeleteCertificate->CheckPrivillege() Error,rtn = 0x",rtn);
      return rtn;
      }*/
    //////////////////////////////////////////////////////////////////////////

    rtn = SearchContainerRecNo(hDeviceHandle,pcContainerName,&recNO);
    if( (rtn != 0) || (recNO == 0) )
    {
        AK_UnlockMutex(gpMutex);
        if(recNO == 0)
        {
            AK_Log(AK_LOG_ERROR, "USBKEY_DeleteCertificate ==> not find container record.");
            return USBKEY_RECORD_NOT_FOUND;
        }
        AK_Log(AK_LOG_ERROR, "USBKEY_DeleteCertificate ==> SearchContainerRecNo() Error,rtn = %#08x", rtn);
        return rtn;
    }

    ////////
    //生成文件名
    ////////
    memset(fileName,0,3);
    memcpy(fileName,&recNO,1);
    if (ulKeyUsage == KEY_EXCHANGE)
    {
        memset(fileName+1,0x03,1);
    }
    else if (ulKeyUsage == KEY_SIGNATURE)
    {
        memset(fileName+1,0x06,1);
    }

    AK_UnlockMutex(gpMutex);

    rtn = USBKEY_DeleteFile(hDeviceHandle,fileName);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_DeleteCertificate ==>USBKEY_DeleteFile() Error,rtn = %#08x", rtn);
        return rtn;
    }

    //ADD by zhoushenshen 20091125
    rtn = DeleteContainerCert(hDeviceHandle,recNO,ulKeyUsage);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_DeleteCertificate ==>DeleteContainerCert() Error,rtn = %#08x", rtn);
        return rtn;
    }

    ///////
    //赋值返回


    ///////
    AK_Log(AK_LOG_DEBUG, "USBKEY_DeleteCertificate Success!");

    return USBKEY_OK;
}

//	删除非对称密钥.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DeleteAsymmetricKey(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcContainerName,//[in]密钥容器名称
        USBKEY_ULONG ulKeyUsage,		//[in]密钥用途
        USBKEY_ULONG ulKeyType)		//[in]密钥类型，USBKEY_RSA_PUBLICKEY或USBKEY_RSA_PRIVATEKEY
{
    //USBKEY_UCHAR request[1500];
    USBKEY_UCHAR response[1500];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0,recNO;
    USBKEY_UCHAR data[1500],fileName[4];
    USBKEY_UCHAR p1,p2,lc,le;

    AK_Log(AK_LOG_DEBUG, "USBKEY_DeleteAsymmetricKey");

    /*if (strlen(pcAppName) > 16)
      {
      LogMessage("====>>USBKEY_DeleteAsymmetricKey->pcAppName Length Error!");
      return USBKEY_PARAM_ERROR;
      }*/
    if (strlen((char *)pcContainerName) > 254)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_DeleteAsymmetricKey ==> pcContainerName Length Error,Max Name Len is 255!");
        return USBKEY_PARAM_ERROR;
    }

    if ((ulKeyUsage != KEY_EXCHANGE) && (ulKeyUsage != KEY_SIGNATURE) )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_DeleteAsymmetricKey ==>ulKeyUsage Error,ulKeyUsage = %#08x", ulKeyUsage);
        return USBKEY_PARAM_ERROR;
    }

    if ((ulKeyType != USBKEY_RSA_PUBLICKEY) && (ulKeyType != USBKEY_RSA_PRIVATEKEY) )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_DeleteAsymmetricKey ==>ulKeyType Error,ulKeyType = %#08x", ulKeyType);
        return USBKEY_PARAM_ERROR;
    }

    AK_LockMutex(gpMutex);

    /*rtn = SelectFile(hDeviceHandle,pcAppName,NULL);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage16Val("====>>USBKEY_DeleteAsymmetricKey->SelectFile() Error, rtn = 0x",rtn);
      return rtn;
      }*/

    /////////检查权限///////////////////////////////////////////////////
    /*rtn = CheckPrivillege(hDeviceHandle);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage16Val("====>>USBKEY_DeleteAsymmetricKey->CheckPrivillege() Error,rtn = 0x",rtn);
      return rtn;
      }*/
    //////////////////////////////////////////////////////////////////////////

    rtn = SearchContainerRecNo(hDeviceHandle,pcContainerName,&recNO);
    if( (rtn != 0) || (recNO == 0) )
    {
        AK_UnlockMutex(gpMutex);
        if(recNO == 0)
        {
            AK_Log(AK_LOG_ERROR, "USBKEY_DeleteAsymmetricKey ==> not find container record.");
            return USBKEY_RECORD_NOT_FOUND;
        }
        AK_Log(AK_LOG_ERROR, "USBKEY_DeleteAsymmetricKey ==> SearchContainerRecNo() Error,rtn = %#08x", rtn);
        return rtn;
    }

    ////////
    //生成文件名
    ////////
    memcpy(fileName,&recNO,1);
    memcpy(fileName+2,&recNO,1);
    if (ulKeyUsage == KEY_EXCHANGE)
    {
        memset(fileName+1,0x01,1);
        memset(fileName+3,0x02,1);
    }
    else if (ulKeyUsage == KEY_SIGNATURE)
    {
        memset(fileName+1,0x04,1);
        memset(fileName+3,0x05,1);
    }

    p1 = 0x00;
    p2 = 0x00;
    lc = 2;
    le = 0x00;

    //// .
    if (ulKeyType == USBKEY_RSA_PUBLICKEY)
    {
        memcpy(data,fileName,2);
    }
    else if (ulKeyType == USBKEY_RSA_PRIVATEKEY)
    {
        memcpy(data,fileName+2,2);
    }

    //Delete by zhoushenshen in 20121109
    /*rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_DeleteFile,request,&reqLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_DeleteAsymmetricKey ==>PackageData() Error,rtn = %#08x", rtn);
      return rtn;
      }

      respLen = 300;
      rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_DeleteAsymmetricKey ==>DeviceExecCmd() Error,rtn = %#08x", rtn);
      return rtn;
      }*/
    //Del end

    //Add by zhoushenshen in 20121109
    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_DeleteFile,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_DeleteAsymmetricKey ==> PackageData() Error, rtn = %#08x", rtn);
        return rtn;
    }
    //Add end

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_DeleteAsymmetricKey ==>DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
            break;
    }

    rtn = DeleteContainerKey(hDeviceHandle,recNO,ulKeyUsage, ulKeyType);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_DeleteAsymmetricKey ==>DeleteContainerKey() Error,rtn = %#08x", rtn);
        return rtn;
    }

    AK_Log(AK_LOG_DEBUG, "USBKEY_DeleteAsymmetricKey Success!");

    AK_UnlockMutex(gpMutex);
    return USBKEY_OK;
}



//	产生ECC非对称密钥.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_GenerateECCKeyPair(
        USBKEY_HANDLE			hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR		pcContainerName,//[in]密钥容器名称
        USBKEY_ULONG			ulKeyUsage,		//[in]密钥用途，KEY_EXCHANGE密钥或者KEY_SIGNATURE密钥
        USBKEY_ULONG			ulBitLen,		//[in]密钥模长：256或者384
        USBKEY_ULONG			ulGenKeyFlags,	//[in]生成密钥的控制标志
        USBKEY_UCHAR_PTR		pcPubKeyID,		//[out]返回的公钥ID，如果为NULL则不返回公钥ID
        USBKEY_UCHAR_PTR		pcPriKeyID,		//[out]返回的私钥ID，如果为NULL则不返回私钥ID
        USBKEY_ECC_PUBLIC_KEY*	pPublicKey)		//[out]返回的公钥数据，如果为NULL则不返回公钥数据
{
    USBKEY_ULONG rtn,recNO,ulAccessCondition;
    USBKEY_UCHAR fileName[4];
    USBKEY_ECC_PUBLIC_KEY pubKey;

    AK_Log(AK_LOG_DEBUG, "USBKEY_GenerateECCKeyPair");

    if( NULL == hDeviceHandle || NULL == pcContainerName )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_GenerateECCKeyPair ==>hDeviceHandle or pcContainerName is NULL,rtn = %#08x", USBKEY_PARAM_ERROR);
        return USBKEY_PARAM_ERROR;
    }

    if (strlen((char *)pcContainerName) > 254)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_GenerateECCKeyPair ==> pcContainerName Length Error,Max Name Len is 255!");
        return USBKEY_PARAM_ERROR;
    }

    if ((ulKeyUsage != KEY_EXCHANGE) && (ulKeyUsage != KEY_SIGNATURE) )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_GenerateECCKeyPair ==>ulKeyUsage Error,ulKeyUsage = %#08x", ulKeyUsage);
        return USBKEY_PARAM_ERROR;
    }
    if( (ulBitLen != 256) && (ulBitLen != 384) )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_GenerateECCKeyPair ==>ulBitLen Error,ulBitLen = %#08x", ulBitLen);
        return USBKEY_PARAM_ERROR;
    }

    AK_LockMutex(gpMutex);

    rtn = SearchContainerRecNo(hDeviceHandle,pcContainerName,&recNO);
    if( (rtn != 0) || (recNO == 0) )
    {
        AK_UnlockMutex(gpMutex);
        if(recNO == 0)
        {
            AK_Log(AK_LOG_ERROR, "USBKEY_GenerateECCKeyPair ==> not find container record.");
            return USBKEY_RECORD_NOT_FOUND;
        }
        AK_Log(AK_LOG_ERROR, "USBKEY_GenerateECCKeyPair ==> SearchContainerRecNo() Error,rtn = %#08x", rtn);
        return rtn;
    }

    ////////
    //生成文件名
    ////////
    memcpy(fileName,&recNO,1);
    memcpy(fileName+2,&recNO,1);
    if (ulKeyUsage == KEY_EXCHANGE)
    {
        memset(fileName+1,0x01,1);
        memset(fileName+3,0x02,1);
    }
    else if (ulKeyUsage == KEY_SIGNATURE)
    {
        memset(fileName+1,0x04,1);
        memset(fileName+3,0x05,1);
    }

    //设置权限
    if( (ulGenKeyFlags & USBKEY_CRYPT_USER_PROTECTED) != 0)
        ulAccessCondition = USBKEY_USER_USE_PRIV;
    else
        ulAccessCondition = USBKEY_ALL_PRIV;

    //创建公钥文件和私钥文件
    rtn = CreateKeyPairFile(hDeviceHandle, fileName, fileName+2, (USBKEY_UCHAR)ulAccessCondition, USBKEY_USER_USE_PRIV);
    if( 0 != rtn )
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_GenerateECCKeyPair ==> CreateKeyPairFile() Error,rtn = %#08x", rtn);
        return rtn;
    }

    //产生ECC密钥对
    rtn = GenEccKeyPair(hDeviceHandle, ulBitLen, fileName, fileName+2, &pubKey);
    if( 0 != rtn )
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_GenerateECCKeyPair ==> GenEccKeyPair() Error,rtn = %#08x", rtn);
        return rtn;
    }

    //设置容器对应的密钥记录
    rtn = AddContainerKey(hDeviceHandle,recNO,ulKeyUsage,ulBitLen);	//Update by zhoushenshen 20100309
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_GenerateECCKeyPair ==> AddContainerKey() Error,rtn = %#08x", rtn);
        return rtn;
    }

    ///////
    //赋值返回ID
    if(NULL != pcPubKeyID)
        memcpy(pcPubKeyID,fileName,2);
    if(NULL != pcPriKeyID)
        memcpy(pcPriKeyID,fileName+2,2);
    if(NULL != pPublicKey)
    {
        pPublicKey->bits = ulBitLen;
        memcpy(pPublicKey->XCoordinate, pubKey.XCoordinate, ulBitLen/8);
        memcpy(pPublicKey->YCoordinate, pubKey.YCoordinate, ulBitLen/8);
    }

    AK_Log(AK_LOG_DEBUG, "USBKEY_GenerateECCKeyPair Success!");
    AK_UnlockMutex(gpMutex);

    return USBKEY_OK;
}

//	导入ECC公私钥对.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ImportECCKeyPair(
        USBKEY_HANDLE		hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR	pcWrapKeyID,	//[in]用来保护导入密钥的密钥ID
        USBKEY_ULONG		ulMode,			//[in]解密模式(必须为带填充)：USBKEY_ECB 或 USBKEY_CBC,若无保护密钥ID，设为ECB模式
        USBKEY_UCHAR_PTR	pcIV,			//[in]初始化向量，如果是ecb模式，设置为NULL
        USBKEY_ULONG		ulIVLen,		//[in]初始化向量长度，如果是ecb模式，设置为0
        USBKEY_UCHAR_PTR	pcContainerName,//[in]密钥容器
        USBKEY_ULONG		ulKeyUsage,		//[in]密钥用途，KEY_EXCHANGE密钥或者KEY_SIGNATURE密钥
        USBKEY_UCHAR_PTR	pcECCKeyPair,	//[in]导入的密钥对
        USBKEY_ULONG		ulECCKeyPairLen,//[in]导入的密钥长度
        USBKEY_UCHAR_PTR	pcPubKeyID,		//[out]返回的私钥ID，如果为NULL则不返回公钥ID
        USBKEY_UCHAR_PTR	pcPriKeyID)	//[out]返回的公钥ID，如果为NULL则不返回私钥ID
{
    AK_Log(AK_LOG_DEBUG, "USBKEY_ImportECCKeyPair");



    AK_Log(AK_LOG_DEBUG, "USBKEY_ImportECCKeyPair Success!");

    return USBKEY_OK;
}

//Add by zhoushenshen 20111102
//	导入ECC私钥.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ImportECCPrivateKey(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcWrapKeyID,	//[in]用来保护导入密钥的密钥ID
        USBKEY_ULONG ulMode,			//[in]解密模式：USBKEY_ECB, USBKEY_CBC(必须为带填充),若无保护密钥ID，设为ECB模式
        USBKEY_UCHAR_PTR pcIV,			//[in]初始化向量，如果是ecb模式，设置为NULL
        USBKEY_ULONG ulIVLen,			//[in]初始化向量长度，如果是ecb模式，设置为0
        USBKEY_UCHAR_PTR pcContainerName,//[in]密钥容器
        USBKEY_ULONG ulKeyUsage,		//[in]密钥用途，KEY_EXCHANGE密钥或者KEY_SIGNATURE密钥
        USBKEY_UCHAR_PTR pcECCPriKey,	//[in]导入的密钥
        USBKEY_ULONG ulECCPriKeyLen,	//[in]导入的密钥长度
        USBKEY_UCHAR_PTR pcPriKeyID)	//[out]返回的私钥ID
{
    //USBKEY_UCHAR request[1500];
    USBKEY_UCHAR response[300];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0,recNO,ulAccessCondition = USBKEY_USER_USE_PRIV,ulFileSize;
    USBKEY_UCHAR data[1500],fileName[4];
    //USBKEY_UCHAR p1,p2,lc,le;
    USBKEY_UCHAR p1,p2,le;
    USBKEY_INT32 lc;
    USBKEY_ULONG ulEncMode;
    USBKEY_ULONG ulPadMode;
    USBKEY_ULONG ulBitLen;

    AK_Log(AK_LOG_DEBUG, "USBKEY_ImportECCPrivateKey");

    if (strlen((char *)pcContainerName) > 254)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportECCPrivateKey ==> pcContainerName Length Error,Max Name Len is 255!");
        return USBKEY_PARAM_ERROR;
    }

    if ((ulKeyUsage != KEY_EXCHANGE) && (ulKeyUsage != KEY_SIGNATURE) )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportECCPrivateKey ==> ulKeyUsage Error,ulKeyUsage = %#08x", ulKeyUsage);
        return USBKEY_PARAM_ERROR;
    }

    //检查加密模式,填充模式的合法性
    ulEncMode = ulMode & 0xf0;
    ulPadMode = ulMode & 0x0f;

    if(pcWrapKeyID && USBKEY_PKCS5_PADDING != ulPadMode)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportECCPrivateKey ==> ulMode Error,ulPadMode = %#08x", ulPadMode);
        return USBKEY_PARAM_ERROR;
    }
    if(pcWrapKeyID && ulEncMode != USBKEY_ECB && ulEncMode != USBKEY_CBC)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportECCPrivateKey ==> ulMode Error,ulEncMode = %#08x", ulEncMode);
        return USBKEY_PARAM_ERROR;
    }

    if(pcWrapKeyID && ulEncMode == USBKEY_CBC && ulIVLen != 8 && ulIVLen != 16)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportECCPrivateKey ==> ulIVLen Error,ulIVLen = %#08x", ulIVLen);
        return USBKEY_PARAM_ERROR;
    }

    AK_LockMutex(gpMutex);

    /*rtn = SelectFile(hDeviceHandle,pcAppName,NULL);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage16Val("====>>USBKEY_ImportRSAPrivateKey->SelectFile() Error, rtn = 0x",rtn);
      return rtn;
      }*/

    rtn = SearchContainerRecNo(hDeviceHandle,pcContainerName,&recNO);
    if( (rtn != 0) || (recNO == 0) )
    {
        AK_UnlockMutex(gpMutex);
        if(recNO == 0)
        {
            AK_Log(AK_LOG_ERROR, "USBKEY_ImportECCPrivateKey ==> not find container record.");
            return USBKEY_RECORD_NOT_FOUND;
        }
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportECCPrivateKey ==> SearchContainerRecNo() Error,rtn = %#08x", rtn);
        return rtn;
    }

    ////////
    //生成文件名
    ////////
    memcpy(fileName,&recNO,1);
    memcpy(fileName+2,&recNO,1);
    if (ulKeyUsage == KEY_EXCHANGE)
    {
        memset(fileName+1,0x01,1);
        memset(fileName+3,0x02,1);
    }
    else if (ulKeyUsage == KEY_SIGNATURE)
    {
        memset(fileName+1,0x04,1);
        memset(fileName+3,0x05,1);
    }

    //创建私钥文件
    p1 = 0x00;
    p2 = 0x11;
    //lc = 9;
    lc = 0x07;						//lc updtae in 20120106
    le = 0x00;

    memset(data,0,9);
    memcpy(data,fileName+2,2);
    data[2] = 0x00;
    le = (USBKEY_UCHAR)ulAccessCondition;
    memcpy(data+3,&le,1);
    memcpy(data+4,&le,1);
    //updtae in 20120106 文件长度由4字节改为2字节
    ulFileSize = USBKEY_PRIVATE_KEY_FILE_LEN;
    le = (USBKEY_UCHAR)(ulFileSize/256);
    //memcpy(data+7,&le,1);
    memcpy(data+5,&le,1);
    le = (USBKEY_UCHAR)ulFileSize;
    //memcpy(data+8,&le,1);
    memcpy(data+6,&le,1);

    //Delete by zhoushenshen in 20121107
    /*rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_CreateFile,request,&reqLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_ImportECCPrivateKey ==> PackageData() Error,rtn = %#08x", rtn);
      return rtn;
      }

      respLen = 300;
      rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_ImportECCPrivateKey ==> DeviceExecCmd() Error,rtn = %#08x", rtn);
      return rtn;
      }*/
    //Del end

    //Add by zhoushenshen in 20121107
    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_CreateFile,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportECCPrivateKey ==> PackageData() Error, rtn = %#08x", rtn);
        return rtn;
    }
    //Add end

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        case DEVICE_CMD_DATA_ERROR:
            AK_Log(AK_LOG_DEBUG, "USBKEY_ImportECCPrivateKey Private Key File Exist.");
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_ImportECCPrivateKey ==> DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
            break;
    }
    AK_Log(AK_LOG_DEBUG, "USBKEY_ImportECCPrivateKey Create Private Key File Success.");

    //写入私钥
    if(pcWrapKeyID == NULL)
        p1 = 0x00;
    else if (pcWrapKeyID[0] == 0x10)
        p1 = 0x07+(USBKEY_UCHAR)ulEncMode;
    else if (pcWrapKeyID[0] == 0x11)
        p1 = 0x08+(USBKEY_UCHAR)ulEncMode;
    else
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportECCPrivateKey ==> pcWrapKeyID Error,pcWrapKeyID = %#08x", pcWrapKeyID[0]);
        return USBKEY_PARAM_ERROR;
    }

    if(pcWrapKeyID == NULL)
        p2 = 0;
    else
        p2 = pcWrapKeyID[1];

    //Delete by zhoushenshen in 20121204
    /*lc = 0xFF;
      le = 0x00;
      if(ulEncMode == USBKEY_CBC)
      {
      le = (USBKEY_UCHAR)((ulECCPriKeyLen+4+ulIVLen)/256);
      memcpy(data,&le,1);
      le = (USBKEY_UCHAR)(ulECCPriKeyLen+4+ulIVLen);
      memcpy(data+1,&le,1);
      }
      else
      {
      le = (USBKEY_UCHAR)((ulECCPriKeyLen+4)/256);
      memcpy(data,&le,1);
      le = (USBKEY_UCHAR)(ulECCPriKeyLen+4);
      memcpy(data+1,&le,1);
      }
    //写入密钥属性
    data[2] = 0x00;		//自定义
    if(ulECCPriKeyLen >= 36 && ulECCPriKeyLen < 49)
    ulBitLen = 256;
    else if(ulECCPriKeyLen >= 52 && ulECCPriKeyLen < 65)
    ulBitLen = 384;
    else
    ulBitLen = 0;

    data[3] = 0x01;		//ECC密钥
    memcpy(data+4,fileName+2,2);	//私钥ID

    //检查IV指针合法性
    if( ulEncMode == USBKEY_CBC )
    {
    if(NULL == pcIV)
    {
    AK_UnlockMutex(gpMutex);
    AK_Log(AK_LOG_ERROR, "USBKEY_ImportECCPrivateKey ==> pcIV is NULL,rtn = %#08x", USBKEY_PARAM_ERROR);
    return USBKEY_PARAM_ERROR;
    }
    else
    {
    memcpy(data+6, pcIV, ulIVLen);
    memcpy(data+6+ulIVLen,pcECCPriKey,ulECCPriKeyLen);
    }
    }
    else
    memcpy(data+6,pcECCPriKey,ulECCPriKeyLen);

    rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_ImportPrivateKey,request,&reqLen);
    if (rtn != 0)
    {
    AK_UnlockMutex(gpMutex);
    AK_Log(AK_LOG_ERROR, "USBKEY_ImportECCPrivateKey ==> PackageData() Import PriKey Error,rtn = %#08x", rtn);
    return rtn;
    }
    respLen = 300;
    rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
    if (rtn != 0)
    {
    AK_UnlockMutex(gpMutex);
    AK_Log(AK_LOG_ERROR, "USBKEY_ImportECCPrivateKey ==> DeviceExecCmd() Import PriKey Error,rtn = %#08x", rtn);
    return rtn;
    }*/
    //Del end

    //Add by zhoushenshen in 20121204
    le = 0x00;
    if(ulEncMode == USBKEY_CBC)
        lc = ulECCPriKeyLen+4+ulIVLen;
    else
        lc = ulECCPriKeyLen+4;

    //写入密钥属性
    data[0] = 0x00;		//自定义
    if(ulECCPriKeyLen >= 36 && ulECCPriKeyLen < 49)
        ulBitLen = 256;
    else if(ulECCPriKeyLen >= 52 && ulECCPriKeyLen < 65)
        ulBitLen = 384;
    else
        ulBitLen = 0;

    data[1] = 0x01;		//ECC密钥
    memcpy(data+2,fileName+2,2);	//私钥ID

    //检查IV指针合法性
    if( ulEncMode == USBKEY_CBC )
    {
        if(NULL == pcIV)
        {
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_ImportECCPrivateKey ==> pcIV is NULL,rtn = %#08x", USBKEY_PARAM_ERROR);
            return USBKEY_PARAM_ERROR;
        }
        else
        {
            memcpy(data+4, pcIV, ulIVLen);
            memcpy(data+4+ulIVLen,pcECCPriKey,ulECCPriKeyLen);
        }
    }
    else
        memcpy(data+4,pcECCPriKey,ulECCPriKeyLen);

    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_ImportPrivateKey,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportECCPrivateKey ==> PackageData() Import prikey Error, rtn = %#08x", rtn);
        return rtn;
    }
    //Add end

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_ImportECCPrivateKey ==> DeviceExecCmd() Import PriKey Error,code = %#08x", rtn);
            return rtn;
            break;
    }
    AK_Log(AK_LOG_DEBUG, "USBKEY_ImportECCPrivateKey Import Private Key Success.");

    rtn = AddContainerKeySep(hDeviceHandle,recNO,ulKeyUsage, USBKEY_PRIVATE_KEY,ulBitLen);	//Update by zhoushenshen 20100310
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportECCPrivateKey ==>AddContainerKeySep() Error,rtn = %#08x", rtn);
        return rtn;
    }

    ///////
    //赋值返回ID
    memcpy(pcPriKeyID,fileName+2,2);

    AK_Log(AK_LOG_DEBUG, "USBKEY_ImportECCPrivateKey Success!");
    AK_UnlockMutex(gpMutex);
    return USBKEY_OK;
}

//Add by zhoushenshen 20111102
//	导入ECC公钥.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ImportECCPublicKey(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcContainerName,	//[in]密钥容器
        USBKEY_ULONG ulKeyUsage,		//[in]密钥用途，KEY_EXCHANGE密钥或者KEY_SIGNATURE密钥
        USBKEY_UCHAR_PTR pcECCPublicKey,//[in]导入的密钥
        USBKEY_ULONG ulECCPublicKeyLen,	//[in]导入的密钥长度
        USBKEY_UCHAR_PTR pcPubKeyID)	//[out]返回的公钥ID
{
    //USBKEY_UCHAR request[550];
    USBKEY_UCHAR response[550];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0,recNO,ulAccessCondition = USBKEY_USER_USE_PRIV,ulFileSize;
    USBKEY_UCHAR data[550],fileName[4];
    USBKEY_UCHAR p1,p2,lc,le;
    USBKEY_ULONG ulBitLen;

    USBKEY_UCHAR head256[4]= {0x00, 0x00, 0x01, 0x00};
    USBKEY_UCHAR head384[4] = {0x00, 0x00, 0x01, 0x80};

    AK_Log(AK_LOG_DEBUG, "USBKEY_ImportECCPublicKey");

    if (strlen((char *)pcContainerName) > 254)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportECCPublicKey ==> pcContainerName Length Error,Max Name Len is 255!");
        return USBKEY_PARAM_ERROR;
    }

    if ((ulKeyUsage != KEY_EXCHANGE) && (ulKeyUsage != KEY_SIGNATURE) )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportECCPublicKey ==>ulKeyUsage Error,ulKeyUsage = %#08x", ulKeyUsage);
        return USBKEY_PARAM_ERROR;
    }

    if ( ((ulECCPublicKeyLen == 68) && (0 != memcmp(pcECCPublicKey,head256,4)))||
            ((ulECCPublicKeyLen == 100) && (0 != memcmp(pcECCPublicKey,head384,4))) )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportECCPublicKey ==>pcECCPublicKey bits Error,rtn = %#08x", USBKEY_PARAM_ERROR);
        return USBKEY_PARAM_ERROR;
    }

    AK_LockMutex(gpMutex);

    if(ulECCPublicKeyLen == 68)
        ulBitLen = 256;
    else if (ulECCPublicKeyLen == 100)
        ulBitLen = 384;
    else
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportECCPublicKey ==>ulECCPublicKeyLen Error,ulECCPublicKeyLen = %#08x", ulECCPublicKeyLen);
        return USBKEY_PARAM_ERROR;
    }

    /*rtn = SelectFile(hDeviceHandle,pcAppName,NULL);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage16Val("====>>USBKEY_ImportRSAPublicKey->SelectFile() Error, rtn = 0x",rtn);
      return rtn;
      }*/

    rtn = SearchContainerRecNo(hDeviceHandle,pcContainerName,&recNO);
    if( (rtn != 0) || (recNO == 0) )
    {
        AK_UnlockMutex(gpMutex);
        if(recNO == 0)
        {
            AK_Log(AK_LOG_ERROR, "USBKEY_ImportECCPublicKey ==> not find container record.");
            return USBKEY_RECORD_NOT_FOUND;
        }
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportECCPublicKey ==> SearchContainerRecNo() Error,rtn = %#08x", rtn);
        return rtn;
    }

    ////////
    //生成文件名
    ////////
    memcpy(fileName,&recNO,1);
    //memcpy(fileName+2,&recNO,1);
    if (ulKeyUsage == KEY_EXCHANGE)
    {
        memset(fileName+1,0x01,1);
        //memset(fileName+3,0x02,1);
    }
    else if (ulKeyUsage == KEY_SIGNATURE)
    {
        memset(fileName+1,0x04,1);
        //memset(fileName+3,0x05,1);
    }

    //创建公钥文件
    p1 = 0x00;
    p2 = 0x10;
    //lc = 9;
    lc = 0x07;						//lc updtae in 20120106
    le = 0x00;

    memset(data,0,9);
    memcpy(data,fileName,2);
    le = (USBKEY_UCHAR)ulAccessCondition;
    data[3] = USBKEY_ALL_PRIV;
    memcpy(data+4,&le,1);
    //updtae in 20120106 文件长度由4字节改为2字节
    ulFileSize = USBKEY_PUBLIC_KEY_FILE_LEN;		
    le = (USBKEY_UCHAR)(ulFileSize/256);
    //memcpy(data+7,&le,1);
    memcpy(data+5,&le,1);
    le = (USBKEY_UCHAR)ulFileSize;
    //memcpy(data+8,&le,1);
    memcpy(data+6,&le,1);

    //Delete by zhoushenshen in 20121107
    /*rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_CreateFile,request,&reqLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_ImportECCPublicKey ==> PackageData() Error,rtn = %#08x", rtn);
      return rtn;
      }

      respLen = 550;
      rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_ImportECCPublicKey ==> DeviceExecCmd() Error,rtn = %#08x", rtn);
      return rtn;
      }*/
    //Del end

    //Add by zhoushenshen in 20121107
    respLen = 550;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_CreateFile,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportECCPublicKey ==> PackageData() Error, rtn = %#08x", rtn);
        return rtn;
    }
    //Add end

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        case DEVICE_CMD_DATA_ERROR:
            AK_Log(AK_LOG_DEBUG, "USBKEY_ImportECCPublicKey Public Key File Exist.");
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_ImportECCPublicKey ==> DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
            break;
    }

    AK_Log(AK_LOG_DEBUG, "USBKEY_ImportECCPublicKey Create Public Key File Success.");

    rtn = WritePublicKey(hDeviceHandle,fileName,0,pcECCPublicKey,&ulECCPublicKeyLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportECCPublicKey ==> WritePublicKey() Error,rtn = %#08x", rtn);
        return rtn;
    }

    rtn = AddContainerKeySep(hDeviceHandle,recNO,ulKeyUsage,USBKEY_PUBLIC_KEY,ulBitLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportECCPublicKey ==> AddContainerKeySep() Error,rtn = %#08x", rtn);
        return rtn;
    }

    ///////
    //赋值返回ID
    memcpy(pcPubKeyID,fileName,2);

    AK_Log(AK_LOG_DEBUG, "USBKEY_ImportECCPublicKey Success!");
    AK_UnlockMutex(gpMutex);

    return USBKEY_OK;
}


//	ECC公钥加密
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ECCEncrypt(
        USBKEY_HANDLE			hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR		pcPubKeyID,		//[in] 公钥ID
        USBKEY_UCHAR_PTR		pcInData,		//[in]待加密的明文数据
        USBKEY_ULONG			ulInDataLen,	//[in]待加密的明文数据长度
        USBKEY_UCHAR_PTR		pcOutData,		//[out]密文缓冲区，如果该参数为NULL，则由pulOutDataLen返回密文实际长度
        USBKEY_ULONG_PTR		pulOutDataLen)	//[in,out] in：pcOutData缓冲区大小，out：密文实际长度
{
    //USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    USBKEY_UCHAR data[300];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0;
    USBKEY_UCHAR p1,p2,lc,le;

    AK_Log(AK_LOG_DEBUG, "USBKEY_ECCEncrypt");

    //参数校验
    if( NULL == hDeviceHandle || NULL == pcPubKeyID || NULL == pcInData || NULL == pulOutDataLen )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ECCEncrypt ==> Param is NULL! rtn = %#08x", USBKEY_PARAM_ERROR);
        return USBKEY_PARAM_ERROR;
    }
    if( ulInDataLen != 0x20 && ulInDataLen != 0x30 )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ECCEncrypt ==> ulInDataLen Error,ulInDataLen = %#08x", ulInDataLen);
        return USBKEY_PARAM_ERROR;
    }
    if( NULL == pcOutData )
    {
        *pulOutDataLen = ulInDataLen * 4;
        return USBKEY_OK;
    }
    else if(*pulOutDataLen < ulInDataLen * 4)
    {
        *pulOutDataLen = ulInDataLen * 4;
        AK_Log(AK_LOG_ERROR, "USBKEY_ECCEncrypt ==> pulOutDataLen not enough,pulOutDataLen = %#08x", *pulOutDataLen);
        return USBKEY_BUFFER_TOO_SHORT;
    }

    AK_LockMutex(gpMutex);

    //Delete by zhoushenshen in 20121212
    /*//组包
      memset(request, 0, 300);
      request[0] = USBKEY_CMD_TYPE_PKI;
      request[1] = USBKEY_CMD_CODE_ECC_ENCRYPT;
      request[2] = 0x01;	//卡内公钥
      if( 0x20 == ulInDataLen )
      {
      request[3] = 0x02;	//模长为256
      }
      else
      {
      request[3] = 0x03;	//模长为384
      }
      request[4] = (USBKEY_UCHAR)(ulInDataLen + 2);
      memcpy(request+5, pcInData, ulInDataLen);	//明文
      memcpy(request+5+ulInDataLen, pcPubKeyID, 2);
      reqLen = 5 + request[4];

    //发送命令
    respLen = 300;
    rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
    if (rtn != 0)
    {
    AK_UnlockMutex(gpMutex);
    AK_Log(AK_LOG_ERROR, "USBKEY_ECCEncrypt ==>DeviceExecCmd() Error,rtn = %#08x", rtn);
    return rtn;
    }*/
    //Del end

    //Add by zhoushenshen in 20121212
    //组包
    memset(data, 0, 300);
    p1 = 0x01;	//卡内公钥
    if( 0x20 == ulInDataLen )
    {
        p2 = 0x02;	//模长为256
    }
    else
    {
        p2 = 0x03;	//模长为384
    }
    lc = (USBKEY_UCHAR)(ulInDataLen + 2);
    le = 4*ulInDataLen;
    memcpy(data, pcInData, ulInDataLen);	//明文
    memcpy(data+ulInDataLen, pcPubKeyID, 2);

    //发送命令
    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_ECCPubKeyEncrypt,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ECCEncrypt ==>PackageData() Error,rtn = %#08x", rtn);
        return rtn;
    }
    //Add end

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_ECCEncrypt ==>DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
    }

    //设置输出参数
    *pulOutDataLen = respLen - 2;
    memcpy(pcOutData, response, *pulOutDataLen);

    AK_Log(AK_LOG_DEBUG, "USBKEY_ECCEncrypt Success!");
    AK_UnlockMutex(gpMutex);

    return USBKEY_OK;
}


//	ECC私钥解密
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ECCDecrypt(
        USBKEY_HANDLE			hDeviceHandle,		//[in]设备句柄
        USBKEY_UCHAR_PTR		pcPriKeyID,			//[in]用来运算的私钥ID
        USBKEY_UCHAR_PTR		pcInData,			//[in]待解密的密文数据
        USBKEY_ULONG			ulInDataLen,		//[in]待解密的密文数据长度
        USBKEY_UCHAR_PTR		pcOutData,			//[out]明文缓冲区，如果该参数为NULL，则由pulOutDataLen返回明文实际长度
        USBKEY_ULONG_PTR		pulOutDataLen)		//[in,out] in：pcOutData缓冲区大小，out：明文实际长度
{
    //USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    USBKEY_UCHAR data[300];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0;
    USBKEY_UCHAR p1,p2,lc,le;

    AK_Log(AK_LOG_DEBUG, "USBKEY_ECCDecrypt");

    //参数校验
    if( NULL == hDeviceHandle || NULL == pcPriKeyID || NULL == pcInData || NULL == pulOutDataLen )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ECCDecrypt ==> Param is NULL! rtn = %#08x", USBKEY_PARAM_ERROR);
        return USBKEY_PARAM_ERROR;
    }

    if( ulInDataLen != 0x80 && ulInDataLen != 0xc0 )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ECCDecrypt ==> ulInDataLen Error,ulInDataLen = %#08x", ulInDataLen);
        return USBKEY_PARAM_ERROR;
    }
    if( NULL == pcOutData )
    {
        *pulOutDataLen = ulInDataLen / 4;
        return USBKEY_OK;
    }
    else if(*pulOutDataLen < ulInDataLen / 4)
    {
        *pulOutDataLen = ulInDataLen;
        AK_Log(AK_LOG_ERROR, "USBKEY_ECCDecrypt ==> pulOutDataLen not enough,pulOutDataLen = %#08x", *pulOutDataLen);
        return USBKEY_BUFFER_TOO_SHORT;
    }

    AK_LockMutex(gpMutex);

    //Delete by zhoushenshen in 20121212
    /*//组包
      memset(request, 0, 300);
      request[0] = USBKEY_CMD_TYPE_PKI;
      request[1] = USBKEY_CMD_CODE_ECC_DECRYPT;
      request[2] = 0x01;	//卡内私钥
      if( 0x80 == ulInDataLen )
      {
      request[3] = 0x02;	//模长为256
      }
      else
      {
      request[3] = 0x03;	//模长为384
      }
      request[4] = (USBKEY_UCHAR)(ulInDataLen +2);
      memcpy(request+5, pcInData, ulInDataLen);
      memcpy(request+5+ulInDataLen, pcPriKeyID,2);
      reqLen = 5 + request[4];

    //发送命令
    respLen = 300;
    rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
    if (rtn != 0)
    {
    AK_UnlockMutex(gpMutex);
    AK_Log(AK_LOG_ERROR, "USBKEY_ExtECCDecrypt ==>DeviceExecCmd() Error,rtn = %#08x", rtn);
    return rtn;
    }*/
    //Del end

    //Add by zhoushenshen in 20121212
    //组包
    memset(data, 0, 300);
    p1 = 0x01;	//卡内私钥
    if( 0x80 == ulInDataLen )
    {
        p2 = 0x02;	//模长为256
    }
    else
    {
        p2 = 0x03;	//模长为384
    }
    lc = (USBKEY_UCHAR)(ulInDataLen +2);
    memcpy(data, pcInData, ulInDataLen);
    memcpy(data+ulInDataLen, pcPriKeyID,2);
    le = ulInDataLen / 4;

    //发送命令
    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_ECCPriKeyDecrypt,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ExtECCDecrypt ==>PackageData() Error,rtn = %#08x", rtn);
        return rtn;
    }
    //Add end

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_ExtECCDecrypt ==>DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
    }

    //设置输出参数
    *pulOutDataLen = respLen - 2;
    memcpy(pcOutData, response, *pulOutDataLen);

    AK_Log(AK_LOG_DEBUG, "USBKEY_ExtECCDecrypt Success!");
    AK_UnlockMutex(gpMutex);

    return USBKEY_OK;
}

//	ECC签名.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ECCSign(
        USBKEY_HANDLE		hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR	pcPriKeyID,		//[in]用来运算的私钥ID
        USBKEY_UCHAR_PTR	pcData,			//[in]待签名数据,必须是已经做过HASH并且填充好的数据
        USBKEY_ULONG		ulDataLen,		//[in]待签名数据长度,必须等于私钥长度（即模长）
        USBKEY_UCHAR_PTR	pcSignature,	//[out]返回的签名
        USBKEY_ULONG_PTR	pulSignLen)		//[out]返回的签名长度
{
    //USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    USBKEY_UCHAR data[300];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0;
    USBKEY_UCHAR p1,p2,lc,le;

    AK_Log(AK_LOG_DEBUG, "USBKEY_ECCSign");

    if( NULL == hDeviceHandle || NULL == pcPriKeyID || NULL == pcData || NULL == pcSignature || NULL == pulSignLen )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ECCSign ==> Param is NULL! rtn = %#08x", USBKEY_PARAM_ERROR);
        return USBKEY_PARAM_ERROR;
    }

    if( ulDataLen != 0x20 && ulDataLen != 0x30 )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ECCSign ==>ulDataLen Error,ulDataLen = %#08x", ulDataLen);
        return USBKEY_PARAM_ERROR;
    }

    AK_LockMutex(gpMutex);

    //Delete by zhoushenshen in 20121212
    /*request[0] = USBKEY_CMD_TYPE_PKI;
      request[1] = USBKEY_CMD_CODE_ECC_SIGN;
      request[2] = 0x01;	//使用内部私钥
      if( 0x20 == ulDataLen )
      {
      request[3] = 0x02;	//模长为256
      request[4] = 0x22;
      reqLen = 0x27;
      }
      else
      {
      request[3] = 0x03;	//模长为384
      request[4] = 0x32;
      reqLen = 0x37;
      }
      memcpy(request+5, pcData, ulDataLen);
      memcpy(request+5+ulDataLen, pcPriKeyID, 2);

      respLen = 300;
      rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_ECCSign ==>DeviceExecCmd() Error,rtn = %#08x", rtn);
      return rtn;
      }*/
    //Del end

    //Add by zhoushenshen in 20121212
    p1 = 0x01;	//使用内部私钥
    if( 0x20 == ulDataLen )
    {
        p2 = 0x02;	//模长为256
        lc = 0x22;
    }
    else
    {
        p2 = 0x03;	//模长为384
        lc = 0x32;
    }
    memcpy(data, pcData, ulDataLen);
    memcpy(data+ulDataLen, pcPriKeyID, 2);
    le = 2 * ulDataLen;

    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_ECCSign,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ECCSign ==>PackageData() Error,rtn = %#08x", rtn);
        return rtn;
    }
    //Add end

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_ECCSign ==>DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
    }

    if(*pulSignLen < respLen-2)
    {
        AK_UnlockMutex(gpMutex);
        *pulSignLen = respLen-2;
        return USBKEY_BUFFER_TOO_SHORT;
    }
    *pulSignLen = respLen-2;
    memcpy(pcSignature, response, respLen-2);

    AK_Log(AK_LOG_DEBUG, "USBKEY_ECCSign Success!");
    AK_UnlockMutex(gpMutex);

    return USBKEY_OK;
}


//  ECC外部公钥验证签名
//	返回值：
//		0：	验证成功.
//		>0:	失败，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ExtECCVerify(
        USBKEY_HANDLE			hDeviceHandle,		//[in]设备句柄
        USBKEY_ECC_PUBLIC_KEY*	pPublicKey,			//[in]公钥结构
        USBKEY_UCHAR_PTR		pcData,				//[IN] 待验证签名数据。
        USBKEY_ULONG			ulDataLen,			//[IN] 待验证签名数据长度
        USBKEY_UCHAR_PTR		pcSignature,		//[IN] 待验证签名值
        USBKEY_ULONG			ulSignLen)			//[IN] 待验证签名值长度
{
    //USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    USBKEY_UCHAR data[300];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0;
    USBKEY_UCHAR p1,p2,lc,le;

    AK_Log(AK_LOG_DEBUG, "USBKEY_ExtECCVerify");

    //参数校验
    if( NULL == hDeviceHandle || NULL == pPublicKey || NULL == pcData || NULL == pcSignature )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ExtECCVerify ==> Param is NULL! rtn = %#08x", USBKEY_PARAM_ERROR);
        return USBKEY_PARAM_ERROR;
    }
    if( 256 != pPublicKey->bits && 384 != pPublicKey->bits )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ExtECCVerify ==>pPublicKey->bits Error,pPublicKey->bits = %#08x", pPublicKey->bits);
        return USBKEY_PARAM_ERROR;
    }
    if( 256 == pPublicKey->bits && ulDataLen != 0x20 )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ExtECCVerify ==>ulDataLen Error,ulDataLen = %#08x", ulDataLen);
        return USBKEY_PARAM_ERROR;
    }
    if( 384 == pPublicKey->bits && ulDataLen != 0x30 )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ExtECCVerify ==>ulDataLen Error,ulDataLen = %#08x", ulDataLen);
        return USBKEY_PARAM_ERROR;
    }
    if( 256 == pPublicKey->bits && ulSignLen != 0x40 )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ExtECCVerify ==>ulSignLen Error,ulSignLen = %#08x", ulSignLen);
        return USBKEY_PARAM_ERROR;
    }
    if( 384 == pPublicKey->bits && ulSignLen != 0x60 )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ExtECCVerify ==>ulSignLen Error,ulSignLen = %#08x", ulSignLen);
        return USBKEY_PARAM_ERROR;
    }

    AK_LockMutex(gpMutex);

    //Delete by zhoushenshen in 20121213
    /*//组包
      request[0] = USBKEY_CMD_TYPE_PKI;
      request[1] = USBKEY_CMD_CODE_ECC_VERIFY;
      request[2] = 0x00;	//外部输入公钥
      if( 0x20 == ulDataLen )
      {
      request[3] = 0x02;	//模长为256
    //request[4] = 0xc0;
    //reqLen = 0xc5;
    //memcpy(request+5, pcSignature, 0x20);
    //memcpy(request+0x25, pcData, 0x20);
    //memcpy(request+0x45, pcSignature, 0x40);
    //memcpy(request+0x85, pPublicKey->XCoordinate, 0x20);
    //memcpy(request+0xa5, pPublicKey->YCoordinate, 0x20);
    //Update by zhoushenshen in 20110530
    request[4] = 0xa0;
    reqLen = 0xa5;
    memcpy(request+5, pcData, 0x20);
    memcpy(request+0x25, pcSignature, 0x40);
    memcpy(request+0x65, pPublicKey->XCoordinate, 0x20);
    memcpy(request+0x85, pPublicKey->YCoordinate, 0x20);
    }
    else
    {
    request[3] = 0x03;	//模长为384
    //request[4] = 0xff;
    //request[5] = 0x01;
    //request[6] = 0x20;
    //reqLen = 0x0127;
    //memcpy(request+7, pcSignature, 0x30);
    //memcpy(request+0x37, pcData, 0x30);
    //memcpy(request+0x67, pcSignature, 0x60);
    //memcpy(request+0xc7, pPublicKey->XCoordinate, 0x30);
    //memcpy(request+0xf7, pPublicKey->YCoordinate, 0x30);
    //Update by zhoushenshen in 20110530
    request[4] = 0xf0;
    reqLen = 0xf5;
    memcpy(request+5, pcData, 0x30);
    memcpy(request+0x35, pcSignature, 0x60);
    memcpy(request+0x95, pPublicKey->XCoordinate, 0x30);
    memcpy(request+0xc5, pPublicKey->YCoordinate, 0x30);
    }

    //发送命令
    respLen = 300;
    rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
    if (rtn != 0)
    {
    AK_UnlockMutex(gpMutex);
    AK_Log(AK_LOG_ERROR, "USBKEY_ExtECCVerify ==>DeviceExecCmd() Error,rtn = %#08x", rtn);
    return rtn;
    }*/
    //Del end

    //Add by zhoushenshen in 20121213
    //组包
    p1 = 0x00;	//外部输入公钥
    if( 0x20 == ulDataLen )
    {
        p2 = 0x02;	//模长为256
        //Update by zhoushenshen in 20110530
        lc = 0xa0;
        le = 0x00; 
        memcpy(data, pcData, 0x20);
        memcpy(data+0x20, pcSignature, 0x40);
        memcpy(data+0x60, pPublicKey->XCoordinate, 0x20);
        memcpy(data+0x80, pPublicKey->YCoordinate, 0x20);
    }
    else
    {
        p2 = 0x03;	//模长为384
        //Update by zhoushenshen in 20110530
        lc = 0xf0;
        le = 0x00;
        memcpy(data, pcData, 0x30);
        memcpy(data+0x30, pcSignature, 0x60);
        memcpy(data+0x90, pPublicKey->XCoordinate, 0x30);
        memcpy(data+0xc0, pPublicKey->YCoordinate, 0x30);
    }

    //发送命令
    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_ECCVerify,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ExtECCVerify ==>PackageData() Error,rtn = %#08x", rtn);
        return rtn;
    }
    //Add end

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_ExtECCVerify ==>DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
    }

    AK_Log(AK_LOG_DEBUG, "USBKEY_ExtECCVerify Success!");
    AK_UnlockMutex(gpMutex);

    return USBKEY_OK;

}

//  ECC公钥验证签名
//	返回值：
//		0：	验证成功.
//		>0:	失败，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ECCVerify(
        USBKEY_HANDLE		hDeviceHandle,	//[in] 设备句柄
        USBKEY_UCHAR_PTR	pcPubKeyID,		//[in] 公钥ID
        USBKEY_UCHAR_PTR	pcData,			//[IN] 待验证签名数据。
        USBKEY_ULONG		ulDataLen,		//[IN] 待验证签名数据长度
        USBKEY_UCHAR_PTR	pcSignature,	//[IN] 待验证签名值
        USBKEY_ULONG		ulSignLen)		//[IN] 待验证签名值长度
{
    //USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    USBKEY_UCHAR data[300];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0;
    USBKEY_UCHAR p1,p2,lc,le;

    AK_Log(AK_LOG_DEBUG, "USBKEY_ECCVerify");

    //参数校验
    if( NULL == hDeviceHandle || NULL == pcPubKeyID || NULL == pcData || NULL == pcSignature )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ECCVerify ==> Param is NULL! rtn = %#08x", USBKEY_PARAM_ERROR);
        return USBKEY_PARAM_ERROR;
    }
    if( ulDataLen != 0x20 && ulDataLen != 0x30 )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ECCVerify ==>ulDataLen Error,ulDataLen = %#08x", ulDataLen);
        return USBKEY_PARAM_ERROR;
    }
    if( ulSignLen != 0x40 && ulSignLen != 0x60 )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ECCVerify ==>ulSignLen Error,ulSignLen = %#08x", ulSignLen);
        return USBKEY_PARAM_ERROR;
    }

    AK_LockMutex(gpMutex);

    //Delete by zhoushenshen in 20121213
    /*//组包
      request[0] = USBKEY_CMD_TYPE_PKI;
      request[1] = USBKEY_CMD_CODE_ECC_VERIFY;
      request[2] = 0x01;	//内部公钥验证签名
      if( 0x20 == ulDataLen )
      {
      request[3] = 0x02;	//模长为256
    //request[4] = 0x82;
    //reqLen = 0x87;
    //memcpy(request+5, pcSignature, 0x20);
    //memcpy(request+0x25, pcData, 0x20);
    //memcpy(request+0x45, pcSignature, 0x40);
    //memcpy(request+0x85, pcPubKeyID, 2);
    //Update by zhoushenshen in 20110530
    request[4] = 0x62;
    reqLen = 0x67;
    memcpy(request+5, pcData, 0x20);
    memcpy(request+0x25, pcSignature, 0x40);
    memcpy(request+0x65, pcPubKeyID, 2);
    }
    else
    {
    request[3] = 0x03;	//模长为384
    //request[4] = 0xc2;
    //request[5] = 0xc2;
    //reqLen = 0xc7;
    //memcpy(request+5, pcSignature, 0x30);
    //memcpy(request+0x35, pcData, 0x30);
    //memcpy(request+0x65, pcSignature, 0x60);
    //memcpy(request+0xc5, pcPubKeyID, 2);
    //Update by zhoushenshen in 20110530
    request[4] = 0x92;
    reqLen = 0x97;
    memcpy(request+5, pcData, 0x30);
    memcpy(request+0x35, pcSignature, 0x60);
    memcpy(request+0x95, pcPubKeyID, 2);
    }

    //发送命令
    respLen = 300;
    rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
    if (rtn != 0)
    {
    AK_UnlockMutex(gpMutex);
    AK_Log(AK_LOG_ERROR, "USBKEY_ECCVerify ==>DeviceExecCmd() Error,rtn = %#08x", rtn);
    return rtn;
    }*/
    //Del end

    //Add by zhoushenshen in 20121213
    //组包
    p1 = 0x01;	//内部公钥验证签名
    if( 0x20 == ulDataLen )
    {
        p2 = 0x02;	//模长为256
        //Update by zhoushenshen in 20110530
        lc = 0x62;
        le = 0x00;
        memcpy(data, pcData, 0x20);
        memcpy(data+0x20, pcSignature, 0x40);
        memcpy(data+0x60, pcPubKeyID, 2);
    }
    else
    {
        p2 = 0x03;	//模长为384
        //Update by zhoushenshen in 20110530
        lc = 0x92;
        le = 0x00;
        memcpy(data, pcData, 0x30);
        memcpy(data+0x30, pcSignature, 0x60);
        memcpy(data+0x90, pcPubKeyID, 2);
    }

    //发送命令
    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_ECCVerify,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ECCVerify ==>PackageData() Error,rtn = %#08x", rtn);
        return rtn;
    }
    //Add end

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_ECCVerify ==>DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
    }

    AK_Log(AK_LOG_DEBUG, "USBKEY_ECCVerify Success!");
    AK_UnlockMutex(gpMutex);

    return USBKEY_OK;

}

//	外部公钥加密导出对称密钥.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ExtPubKeyExportSymmetricKey(
        USBKEY_HANDLE		hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR	pcSymKeyID,		//[in]被导出对称密钥
        USBKEY_ULONG		ulAsymAlgID,	//[in]用来保护导出密钥的非对称算法ID，目前支持USBKEY_TYPE_ECC
        USBKEY_UCHAR_PTR	pcPublicKey,	//[in]用来保护导出密钥的公钥数据结构，目前支持USBKEY_ECC_PUBLIC_KEY
        USBKEY_UCHAR_PTR	pcSymKey,		//[out]返回的密钥，如果该参数为NULL，则由pulSymKeyLen返回密文实际长度
        USBKEY_ULONG_PTR	pulSymKeyLen)	//[in,out] in：pcSymKey缓冲区大小，out：密文实际长度
{
    //USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    USBKEY_UCHAR data[300];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0;
    USBKEY_UCHAR p1,p2,lc,le;

    AK_Log(AK_LOG_DEBUG, "USBKEY_ExtPubKeyExportSymmetricKey");

    //参数校验
    if( NULL == hDeviceHandle || NULL == pcSymKeyID || NULL == pcPublicKey || NULL == pulSymKeyLen )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ExtPubKeyExportSymmetricKey ==> Param is NULL! rtn = %#08x", USBKEY_PARAM_ERROR);
        return USBKEY_PARAM_ERROR;
    }
    if( USBKEY_TYPE_ECC != ulAsymAlgID )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ExtPubKeyExportSymmetricKey ==> ulAsymAlgID Error,ulAsymAlgID = %#08x", ulAsymAlgID);
        return USBKEY_PARAM_ERROR;
    }
    if( 256 != ((USBKEY_ECC_PUBLIC_KEY*)pcPublicKey)->bits && 384 != ((USBKEY_ECC_PUBLIC_KEY*)pcPublicKey)->bits )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ExtPubKeyExportSymmetricKey ==> module len Error,bits = %#08x", ((USBKEY_ECC_PUBLIC_KEY*)pcPublicKey)->bits);
        return USBKEY_PARAM_ERROR;
    }
    if( NULL == pcSymKey )
    {
        *pulSymKeyLen = (((USBKEY_ECC_PUBLIC_KEY*)pcPublicKey)->bits/8)*3;
        return USBKEY_OK;
    }
    else if(*pulSymKeyLen < (((USBKEY_ECC_PUBLIC_KEY*)pcPublicKey)->bits/8)*3)
    {
        *pulSymKeyLen = (((USBKEY_ECC_PUBLIC_KEY*)pcPublicKey)->bits/8)*3;
        return USBKEY_BUFFER_TOO_SHORT;
    }

    AK_LockMutex(gpMutex);

    //Delete by zhoushenshen in 20121213
    /*//组包
      memset(request, 0, 300);
      request[0] = USBKEY_CMD_TYPE_PKI;
      request[1] = USBKEY_CMD_CODE_ECC_EXPORT_SYMKEY;
      request[2] = 0x00;	//外送公钥
      if( 256 == ((USBKEY_ECC_PUBLIC_KEY*)pcPublicKey)->bits )
      {
      request[3] = 0x02;
      }
      else
      {
      request[3] = 0x03;
      }
    //LC= 8+私钥长度+公钥数据长度
    request[4] = (USBKEY_UCHAR)(8 + (((USBKEY_ECC_PUBLIC_KEY*)pcPublicKey)->bits/8)*3); 
    if(0x10 == pcSymKeyID[0])
    request[5] = 0x07;//固定密钥
    else if(0x11 == pcSymKeyID[0])
    request[5] = 0x08;//临时密钥
    else
    {
    AK_UnlockMutex(gpMutex);
    return USBKEY_KEY_ID_ERROR;
    }
    request[6] = pcSymKeyID[1];	//密钥id
    memcpy(request+13+((USBKEY_ECC_PUBLIC_KEY*)pcPublicKey)->bits/8, ((USBKEY_ECC_PUBLIC_KEY*)pcPublicKey)->XCoordinate, ((USBKEY_ECC_PUBLIC_KEY*)pcPublicKey)->bits/8);
    memcpy(request+13+((USBKEY_ECC_PUBLIC_KEY*)pcPublicKey)->bits/4, ((USBKEY_ECC_PUBLIC_KEY*)pcPublicKey)->YCoordinate, ((USBKEY_ECC_PUBLIC_KEY*)pcPublicKey)->bits/8);
    reqLen = 5 + request[4];

    //发送命令
    respLen = 300;
    rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
    if (rtn != 0)
    {
    AK_UnlockMutex(gpMutex);
    AK_Log(AK_LOG_ERROR, "USBKEY_ExtPubKeyExportSymmetricKey ==>DeviceExecCmd() Error,rtn = %#08x", rtn);
    return rtn;
    }*/
    //Del end

    //Add by zhoushenshen in 20121213
    //组包
    memset(data, 0, 300);
    p1 = 0x00;	//外送公钥
    if( 256 == ((USBKEY_ECC_PUBLIC_KEY*)pcPublicKey)->bits )
    {
        p2 = 0x02;
    }
    else
    {
        p2 = 0x03;
    }
    //LC= 8+私钥长度+公钥数据长度
    lc = (USBKEY_UCHAR)(8 + (((USBKEY_ECC_PUBLIC_KEY*)pcPublicKey)->bits/8)*3); 
    le = lc;
    if(0x10 == pcSymKeyID[0])
        data[0] = 0x07;//固定密钥
    else if(0x11 == pcSymKeyID[0])
        data[0] = 0x08;//临时密钥
    else
    {
        AK_UnlockMutex(gpMutex);
        return USBKEY_KEY_ID_ERROR;
    }
    data[1] = pcSymKeyID[1];	//密钥id
    memcpy(data+8+((USBKEY_ECC_PUBLIC_KEY*)pcPublicKey)->bits/8, ((USBKEY_ECC_PUBLIC_KEY*)pcPublicKey)->XCoordinate, ((USBKEY_ECC_PUBLIC_KEY*)pcPublicKey)->bits/8);
    memcpy(data+8+((USBKEY_ECC_PUBLIC_KEY*)pcPublicKey)->bits/4, ((USBKEY_ECC_PUBLIC_KEY*)pcPublicKey)->YCoordinate, ((USBKEY_ECC_PUBLIC_KEY*)pcPublicKey)->bits/8);

    //发送命令
    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_ExportSymmKeybyEcc,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ExtPubKeyExportSymmetricKey ==> PackageData() Error, rtn = %#08x", rtn);
        return rtn;
    }
    //Add end

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_ExtPubKeyExportSymmetricKey ==>DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
    }

    //设置输出参数
    *pulSymKeyLen = respLen - 10;
    memcpy(pcSymKey, response+8, *pulSymKeyLen);

    AK_Log(AK_LOG_DEBUG, "USBKEY_ExtPubKeyExportSymmetricKey Success!");
    AK_UnlockMutex(gpMutex);

    return USBKEY_OK;
}

//	ECC外部公钥加密
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ExtECCEncrypt(
        USBKEY_HANDLE			hDeviceHandle,	//[in]设备句柄
        USBKEY_ECC_PUBLIC_KEY*	pPublicKey,		//[in]公钥结构
        USBKEY_UCHAR_PTR		pcInData,		//[in]待加密的明文数据
        USBKEY_ULONG			ulInDataLen,	//[in]待加密的明文数据长度
        USBKEY_UCHAR_PTR		pcOutData,		//[out]密文缓冲区，如果该参数为NULL，则由pulOutDataLen返回密文实际长度
        USBKEY_ULONG_PTR		pulOutDataLen)	//[in,out] in：pcOutData缓冲区大小，out：密文实际长度
{
    //USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    USBKEY_UCHAR data[300];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0;
    USBKEY_UCHAR p1,p2,lc,le;

    AK_Log(AK_LOG_DEBUG, "USBKEY_ExtECCEncrypt");

    //参数校验
    if( NULL == hDeviceHandle || NULL == pPublicKey || NULL == pcInData || NULL == pulOutDataLen )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ExtECCEncrypt ==> Param is NULL! rtn = %#08x", USBKEY_PARAM_ERROR);
        return USBKEY_PARAM_ERROR;
    }
    if( ulInDataLen != pPublicKey->bits/8 )//目前实现为单分组加密???
        //if( 0 != ulInDataLen%(pPublicKey->bits/8) )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ExtECCEncrypt ==> ulInDataLen Error,ulInDataLen = %#08x", ulInDataLen);
        return USBKEY_PARAM_ERROR;
    }
    if( NULL == pcOutData )
    {
        *pulOutDataLen = (pPublicKey->bits/8)*4;
        return USBKEY_OK;
    }
    else if(*pulOutDataLen < (pPublicKey->bits/8)*4)
    {
        *pulOutDataLen = (pPublicKey->bits/8)*4;
        return USBKEY_BUFFER_TOO_SHORT;
    }

    AK_LockMutex(gpMutex);

    //Delete by zhoushenshen in 20121213
    /*//组包
      memset(request, 0, 300);
      request[0] = USBKEY_CMD_TYPE_PKI;
      request[1] = USBKEY_CMD_CODE_ECC_ENCRYPT;
      request[2] = 0x00;	//外送公钥
      if( 256 == pPublicKey->bits )
      {
      request[3] = 0x02;
      }
      else
      {
      request[3] = 0x03;
      }
      request[4] = (USBKEY_UCHAR)(ulInDataLen + (pPublicKey->bits/8)*2);
      memcpy(request+5, pcInData, pPublicKey->bits/8);	//明文
      memcpy(request+5+pPublicKey->bits/8, pPublicKey->XCoordinate, pPublicKey->bits/8);
      memcpy(request+5+pPublicKey->bits/4, pPublicKey->YCoordinate, pPublicKey->bits/8);
      reqLen = 5 + request[4];

    //发送命令
    respLen = 300;
    rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
    if (rtn != 0)
    {
    AK_UnlockMutex(gpMutex);
    AK_Log(AK_LOG_ERROR, "USBKEY_ExtECCEncrypt ==>DeviceExecCmd() Error,rtn = %#08x", rtn);
    return rtn;
    }*/
    //Del end

    //Add by zhoushenshen in 20121213
    //组包
    memset(data, 0, 300);
    p1 = 0x00;	//外送公钥
    if( 256 == pPublicKey->bits )
    {
        p2 = 0x02;
    }
    else
    {
        p2 = 0x03;
    }
    lc = (USBKEY_UCHAR)(ulInDataLen + (pPublicKey->bits/8)*2);
    memcpy(data, pcInData, pPublicKey->bits/8);	//明文
    memcpy(data+pPublicKey->bits/8, pPublicKey->XCoordinate, pPublicKey->bits/8);
    memcpy(data+pPublicKey->bits/4, pPublicKey->YCoordinate, pPublicKey->bits/8);
    le = (pPublicKey->bits/8)*4;

    //发送命令
    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_ECCPubKeyEncrypt,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ExtECCEncrypt ==>PackageData() Error,rtn = %#08x", rtn);
        return rtn;
    }
    //Add end

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_ExtECCEncrypt ==>DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
    }

    //设置输出参数
    *pulOutDataLen = respLen - 2;
    memcpy(pcOutData, response, *pulOutDataLen);

    AK_Log(AK_LOG_DEBUG, "USBKEY_ExtECCEncrypt Success!");
    AK_UnlockMutex(gpMutex);

    return USBKEY_OK;
}

//	ECC外部私钥解密
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ExtECCDecrypt(
        USBKEY_HANDLE			hDeviceHandle,		//[in]设备句柄
        USBKEY_ECC_PRIVATE_KEY* pPrivateKey,		//[in]私钥结构
        USBKEY_UCHAR_PTR		pcInData,			//[in]待解密的密文数据
        USBKEY_ULONG			ulInDataLen,		//[in]待解密的密文数据长度
        USBKEY_UCHAR_PTR		pcOutData,			//[out]明文缓冲区，如果该参数为NULL，则由pulOutDataLen返回明文实际长度
        USBKEY_ULONG_PTR		pulOutDataLen)		//[in,out] in：pcOutData缓冲区大小，out：明文实际长度
{
    //USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    USBKEY_UCHAR data[300];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0;
    USBKEY_UCHAR p1,p2,lc,le;

    AK_Log(AK_LOG_DEBUG, "USBKEY_ExtECCDecrypt");

    //参数校验
    if( NULL == hDeviceHandle || NULL == pPrivateKey || NULL == pcInData || NULL == pulOutDataLen )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ExtECCDecrypt ==> Param is NULL! rtn = %#08x", USBKEY_PARAM_ERROR);
        return USBKEY_PARAM_ERROR;
    }

    if( ulInDataLen != (pPrivateKey->bits/8)*4 )//目前实现为单分组加密???
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ExtECCDecrypt ==> ulInDataLen Error,ulInDataLen = %#08x", ulInDataLen);
        return USBKEY_PARAM_ERROR;
    }
    if( NULL == pcOutData )
    {
        *pulOutDataLen = pPrivateKey->bits/8;
        return USBKEY_OK;
    }
    else if(*pulOutDataLen < pPrivateKey->bits/8)
    {
        *pulOutDataLen = pPrivateKey->bits/8;
        return USBKEY_BUFFER_TOO_SHORT;
    }

    AK_LockMutex(gpMutex);

    //Delete by zhoushenshen in 20121217
    /*//组包
      memset(request, 0, 300);
      request[0] = USBKEY_CMD_TYPE_PKI;
      request[1] = USBKEY_CMD_CODE_ECC_DECRYPT;
      request[2] = 0x00;	//外送公钥
      if( 256 == pPrivateKey->bits )
      {
      request[3] = 0x02;
      }
      else
      {
      request[3] = 0x03;
      }
      request[4] = (USBKEY_UCHAR)((pPrivateKey->bits/8) * 5);
      memcpy(request+5, pcInData, ulInDataLen);
      memcpy(request+5+ulInDataLen, pPrivateKey->PrivateKey, pPrivateKey->bits/8);
      reqLen = 5 + request[4];

    //发送命令
    respLen = 300;
    rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
    if (rtn != 0)
    {
    AK_UnlockMutex(gpMutex);
    AK_Log(AK_LOG_ERROR, "USBKEY_ExtECCDecrypt ==>DeviceExecCmd() Error,rtn = %#08x", rtn);
    return rtn;
    }*/
    //Del end

    //Add by zhoushenshen in 20121217
    //组包
    memset(data, 0, 300);
    p1 = 0x00;	//外送公钥
    if( 256 == pPrivateKey->bits )
    {
        p2 = 0x02;
    }
    else
    {
        p2 = 0x03;
    }
    lc = (USBKEY_UCHAR)((pPrivateKey->bits/8) * 5);
    memcpy(data, pcInData, ulInDataLen);
    memcpy(data+ulInDataLen, pPrivateKey->PrivateKey, pPrivateKey->bits/8);
    le = pPrivateKey->bits/8;

    //发送命令
    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_ECCPriKeyDecrypt,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ExtECCDecrypt ==>PackageData() Error,rtn = %#08x", rtn);
        return rtn;
    }
    //Add end

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_ExtECCDecrypt ==>DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
    }

    //设置输出参数
    *pulOutDataLen = respLen - 2;
    memcpy(pcOutData, response, *pulOutDataLen);

    AK_Log(AK_LOG_DEBUG, "USBKEY_ExtECCDecrypt Success!");
    AK_UnlockMutex(gpMutex);

    return USBKEY_OK;
}

//	ECC外部私钥签名.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ExtECCSign(
        USBKEY_HANDLE			hDeviceHandle,	//[in]设备句柄
        USBKEY_ECC_PRIVATE_KEY* pPrivateKey,	//[in]私钥结构
        USBKEY_UCHAR_PTR		pcData,			//[in]待签名数据,必须是已经做过HASH并且填充好的数据
        USBKEY_ULONG			ulDataLen,		//[in]待签名数据长度,必须等于私钥长度（即模长）
        USBKEY_UCHAR_PTR		pcSignature,	//[out]返回的签名，如果为NULL，pulSignLen返回签名长度
        USBKEY_ULONG_PTR		pulSignLen)		//[in,out] in：pcSignature缓冲区大小，out：签名实际长度
{
    //USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    USBKEY_UCHAR data[300];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0;
    USBKEY_UCHAR p1,p2,lc,le;

    AK_Log(AK_LOG_DEBUG, "USBKEY_ExtECCSign");

    //参数校验
    if( NULL == hDeviceHandle || NULL == pPrivateKey || NULL == pcData || NULL == pulSignLen )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ExtECCSign ==> Param is NULL! rtn = %#08x", USBKEY_PARAM_ERROR);
        return USBKEY_PARAM_ERROR;
    }
    if( ulDataLen != pPrivateKey->bits/8 )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ExtECCSign ==> ulDataLen Error,ulDataLen = %#08x", ulDataLen);
        return USBKEY_PARAM_ERROR;
    }
    if( NULL == pcSignature )
    {
        *pulSignLen = (pPrivateKey->bits/8)*2;
        return USBKEY_OK;
    }
    else if(*pulSignLen < (pPrivateKey->bits/8)*2)
    {
        *pulSignLen = (pPrivateKey->bits/8)*2;
        return USBKEY_BUFFER_TOO_SHORT;
    }

    AK_LockMutex(gpMutex);

    //Delete by zhoushenshen in 20121217
    /*//组包
      memset(request, 0, 300);
      request[0] = USBKEY_CMD_TYPE_PKI;
      request[1] = USBKEY_CMD_CODE_ECC_SIGN;
      request[2] = 0x00;	//外送私钥
      if( 256 == pPrivateKey->bits )
      {
      request[3] = 0x02;
      }
      else
      {
      request[3] = 0x03;
      }
      request[4] = (USBKEY_UCHAR)(pPrivateKey->bits/4);
      memcpy(request+5, pcData, ulDataLen);
      memcpy(request+5+ulDataLen, pPrivateKey->PrivateKey, pPrivateKey->bits/8);
      reqLen = 5 + request[4];

    //发送命令
    respLen = 300;
    rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
    if (rtn != 0)
    {
    AK_UnlockMutex(gpMutex);
    AK_Log(AK_LOG_ERROR, "USBKEY_ExtECCSign ==>DeviceExecCmd() Error,rtn = %#08x", rtn);
    return rtn;
    }*/
    //Del end

    //Add by zhoushenshen in 20121217
    //组包
    memset(data, 0, 300);
    p1 = 0x00;	//外送私钥
    if( 256 == pPrivateKey->bits )
    {
        p2 = 0x02;
    }
    else
    {
        p2 = 0x03;
    }
    lc = (USBKEY_UCHAR)(pPrivateKey->bits/4);
    memcpy(data, pcData, ulDataLen);
    memcpy(data+ulDataLen, pPrivateKey->PrivateKey, pPrivateKey->bits/8);
    le = (pPrivateKey->bits/8)*2;

    //发送命令
    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_ECCSign,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ExtECCSign ==>PackageData() Error,rtn = %#08x", rtn);
        return rtn;
    }
    //Add end

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_ExtECCSign ==>DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
    }

    //设置输出参数
    *pulSignLen = respLen - 2;
    memcpy(pcSignature, response, *pulSignLen);

    AK_Log(AK_LOG_DEBUG, "USBKEY_ExtECCSign Success!");
    AK_UnlockMutex(gpMutex);

    return USBKEY_OK;
}


//	ECC生成密钥协商参数并输出	USBKEY_ExportAgreementDataWithECC 
//	功能描述					使用ECC密钥协商算法，为计算会话密钥而产生协商参数，返回临时ECC密钥对的公钥
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
//	
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ExportAgreementDataWithECC (
        USBKEY_HANDLE			hDeviceHandle,			//[in]设备句柄
        USBKEY_ULONG			ulBitLen,				//[in]模长
        USBKEY_UCHAR_PTR		pcPriKeyID,				//[in]私钥ID
        USBKEY_ECC_PUBLIC_KEY*	pTempECCPubKeyBlob,		//[OUT] 临时ECC公钥
        BYTE*					pbID,					//[IN] 自身的ID
        ULONG					ulIDLen)				//[IN] 自身ID的长度，不大于32
{
    //USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    USBKEY_UCHAR data[300];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0;
    USBKEY_UCHAR p1,p2,lc,le;

    AK_Log(AK_LOG_DEBUG, "USBKEY_ExportAgreementDataWithECC");

    //参数校验
    if( NULL == hDeviceHandle || NULL == pcPriKeyID || NULL == pTempECCPubKeyBlob || NULL == pbID )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ExportAgreementDataWithECC ==> Param is NULL! rtn = %#08x", USBKEY_PARAM_ERROR);
        return USBKEY_PARAM_ERROR;
    }
    if( 256 != ulBitLen && 384 != ulBitLen )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ExportAgreementDataWithECC ==> ulBitLen Error,ulBitLen = %#08x", ulBitLen);
        return USBKEY_PARAM_ERROR;
    }
    if( ulIDLen > 32 )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ExportAgreementDataWithECC ==> ulIDLen Error,ulIDLen = %#08x", ulIDLen);
        return USBKEY_PARAM_ERROR;
    }

    AK_LockMutex(gpMutex);

    //Delete by zhoushenshen in 20121217
    /*//组包
      memset(request, 0, 300);
      request[0] = USBKEY_CMD_TYPE_PKI;
      request[1] = USBKEY_CMD_CODE_EXPORT_AGREEMENT;
      request[2] = 0x01;	//内部密钥
      if( 256 == ulBitLen )
      request[3] = 0x02;
      else
      request[3] = 0x03;
      request[4] = 2;
      memcpy(request+5, pcPriKeyID, 2);
      reqLen = 7;

    //发送命令
    respLen = 300;
    rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
    if (rtn != 0)
    {
    AK_UnlockMutex(gpMutex);
    AK_Log(AK_LOG_ERROR, "USBKEY_ExportAgreementDataWithECC ==>DeviceExecCmd() Error,rtn = %#08x", rtn);
    return rtn;
    }*/
    //Del end

    //Add by zhoushenshen in 20121217
    //组包
    memset(data, 0, 300);
    p1 = 0x01;	//内部密钥
    if( 256 == ulBitLen )
        p2 = 0x02;
    else
        p2 = 0x03;
    lc = 2;
    memcpy(data, pcPriKeyID, 2);
    le = sizeof(USBKEY_ECC_PUBLIC_KEY);

    //发送命令
    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_ECCExportAgreeData,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ExportAgreementDataWithECC ==>PackageData() Error,rtn = %#08x", rtn);
        return rtn;
    }
    //Add end

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_ExportAgreementDataWithECC ==>DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
    }

    //设置输出参数
    pTempECCPubKeyBlob->bits = ulBitLen;
    memcpy(pTempECCPubKeyBlob->XCoordinate, response, ulBitLen/8);
    memcpy(pTempECCPubKeyBlob->YCoordinate, response+ulBitLen/8, ulBitLen/8);

    AK_Log(AK_LOG_DEBUG, "USBKEY_ExportAgreementDataWithECC Success!");
    AK_UnlockMutex(gpMutex);

    return USBKEY_OK;
}

//	ECC导入协商参数并计算会话密钥		USBKEY_ImportAgreementDataWithECC
//	功能描述							使用ECC密钥协商算法，导入协商参数，计算会话密钥，并返回产生的密钥ID。
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
//
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ImportAgreementDataWithECC(
        USBKEY_HANDLE			hDeviceHandle,				//[in]设备句柄
        USBKEY_UCHAR_PTR		pcPriKeyID,					//[in]私钥ID
        ULONG					ulAlgId,					//[IN] 会话密钥算法标识
        USBKEY_ECC_PUBLIC_KEY*	pSponsorTempECCPubKeyBlob,	//[IN] 对方的临时ECC公钥
        BYTE*					pbID,						//[IN] 对方的ID
        ULONG					ulIDLen,					//[IN] 对方ID的长度，不大于32
        USBKEY_UCHAR_PTR		pcKeyID)					//[OUT] 返回的密钥ID
{
    //USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    USBKEY_UCHAR data[300];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0;
    USBKEY_UCHAR ucSymKeyID;
    USBKEY_UCHAR pcConSymKeyIDRec[USBKEY_MAX_SYM_KEY_ID];
    USBKEY_UCHAR p1,p2,lc,le;

    AK_Log(AK_LOG_DEBUG, "USBKEY_ImportAgreementDataWithECC");

    //参数校验
    if( NULL == hDeviceHandle || NULL == pcPriKeyID || NULL == pSponsorTempECCPubKeyBlob\
            || NULL == pbID || NULL == pcKeyID )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportAgreementDataWithECC ==> Param is NULL! rtn = %#08x", USBKEY_PARAM_ERROR);
        return USBKEY_PARAM_ERROR;
    }
    if( 256 != pSponsorTempECCPubKeyBlob->bits && 384 != pSponsorTempECCPubKeyBlob->bits )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportAgreementDataWithECC ==> ulBitLen Error,ulBitLen = %#08x", pSponsorTempECCPubKeyBlob->bits);
        return USBKEY_PARAM_ERROR;
    }
    if( ulIDLen > 32 )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportAgreementDataWithECC ==> ulIDLen Error,ulIDLen = %#08x", ulIDLen);
        return USBKEY_PARAM_ERROR;
    }
    if( ulAlgId != ALG_TYPE_DES && ulAlgId != ALG_TYPE_3DES_2KEY && ulAlgId != ALG_TYPE_3DES_3KEY\
            && ulAlgId != ALG_TYPE_SSF33 && ulAlgId != ALG_TYPE_SCB2 )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportAgreementDataWithECC ==> ulAlgId Error,ulAlgId = %#08x", ulAlgId);
        return USBKEY_PARAM_ERROR;
    }

    AK_LockMutex(gpMutex);


    //获取一个空闲的容器对称密钥id
    rtn = GetFreeConSymKeyID(hDeviceHandle, &ucSymKeyID, pcConSymKeyIDRec);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportAgreementDataWithECC ==>GetFreeConSymKeyID() Error,rtn = %#08x", rtn);
        return rtn;
    }

    //Delete by zhoushenshen in 20121217
    /*//组包
      memset(request, 0, 300);
      request[0] = USBKEY_CMD_TYPE_PKI;
      request[1] = USBKEY_CMD_CODE_IMPORT_AGREEMENT;
      request[2] = 0x00;
      if( 256 == pSponsorTempECCPubKeyBlob->bits )
      request[3] = 0x02;
      else
      request[3] = 0x03;
      request[4] = (USBKEY_UCHAR)(8 + (pSponsorTempECCPubKeyBlob->bits/8)*3);	//Lc

    //密钥属性（密钥标识 07/08 03/04/05/06/07 01/00 使用权限 更改权限 00 00 ）|| 私钥ID || (0x00||…||0x00)(私钥长度-2个 0x00) || 协商参数(长度为公钥长度)
    request[5] = ucSymKeyID;//密钥标识
    request[6] = 0x07;	//固定对称密钥
    request[7] = (USBKEY_UCHAR)ulAlgId;
    request[8] = 0x01;	//不可导出
    request[9] = USBKEY_USERTYPE_USER;	//使用权限
    request[10] = USBKEY_USERTYPE_USER;	//更改权限
    memcpy(request+13, pcPriKeyID, 2);
    memcpy(request+13+pSponsorTempECCPubKeyBlob->bits/8, pSponsorTempECCPubKeyBlob->XCoordinate, pSponsorTempECCPubKeyBlob->bits/8);
    memcpy(request+13+pSponsorTempECCPubKeyBlob->bits/4, pSponsorTempECCPubKeyBlob->YCoordinate, pSponsorTempECCPubKeyBlob->bits/8);
    reqLen = 5 + request[4];

    //发送命令，导入协商参数并产生对称密钥
    respLen = 300;
    rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
    if (rtn != 0)
    {
    AK_UnlockMutex(gpMutex);
    AK_Log(AK_LOG_ERROR, "USBKEY_ImportAgreementDataWithECC ==>DeviceExecCmd() Error,rtn = %#08x", rtn);
    return rtn;
    }*/
    //Del end

    //Add by zhoushenshen in 20121217
    //组包
    memset(data, 0, 300);
    p1 = 0x00;
    if( 256 == pSponsorTempECCPubKeyBlob->bits )
        p2 = 0x02;
    else
        p2 = 0x03;
    lc = (USBKEY_UCHAR)(8 + (pSponsorTempECCPubKeyBlob->bits/8)*3);	//Lc
    le = 0x00;

    //密钥属性（密钥标识 07/08 03/04/05/06/07 01/00 使用权限 更改权限 00 00 ）|| 私钥ID || (0x00||…||0x00)(私钥长度-2个 0x00) || 协商参数(长度为公钥长度)
    data[0] = ucSymKeyID;//密钥标识
    data[1] = 0x07;	//固定对称密钥
    data[2] = (USBKEY_UCHAR)ulAlgId;
    data[3] = 0x01;	//不可导出
    data[4] = USBKEY_USERTYPE_USER;	//使用权限
    data[5] = USBKEY_USERTYPE_USER;	//更改权限
    memcpy(data+8, pcPriKeyID, 2);
    memcpy(data+8+pSponsorTempECCPubKeyBlob->bits/8, pSponsorTempECCPubKeyBlob->XCoordinate, pSponsorTempECCPubKeyBlob->bits/8);
    memcpy(data+8+pSponsorTempECCPubKeyBlob->bits/4, pSponsorTempECCPubKeyBlob->YCoordinate, pSponsorTempECCPubKeyBlob->bits/8);

    //发送命令，导入协商参数并产生对称密钥
    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_ECCImportAgreeData,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportAgreementDataWithECC ==>PackageData() Error,rtn = %#08x", rtn);
        return rtn;
    }
    //Add end

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_ImportAgreementDataWithECC ==>DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
    }

    //把对称密钥增加到容器
    rtn = AddContainerSymKey(hDeviceHandle, ucSymKeyID, pcPriKeyID[0], pcConSymKeyIDRec);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportAgreementDataWithECC ==>AddContainerSymKey() Error,rtn = %#08x", rtn);
        return rtn;
    }		

    //设置输出参数
    *pcKeyID = 0x10;			//固定对称密钥
    *(pcKeyID+1) = ucSymKeyID;


    AK_Log(AK_LOG_DEBUG, "USBKEY_ImportAgreementDataWithECC Success!");
    AK_UnlockMutex(gpMutex);

    return USBKEY_OK;
}




//	导出ECC公钥.
//	功能描述	使用ECC密钥协商算法，为计算会话密钥而产生协商参数，并返回临时ECC密钥对的公钥和协商句柄
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ExportECCPublicKey(
        USBKEY_HANDLE hDeviceHandle,		//[in]设备句柄
        USBKEY_UCHAR_PTR pcPubKeyID,		//[in]被导出的公钥ID
        USBKEY_ECC_PUBLIC_KEY* pPublicKey)	//[out]返回的公钥
{
    USBKEY_ULONG rtn;
    USBKEY_UCHAR pcBuf[USBKEY_PUBLIC_KEY_FILE_LEN];
    USBKEY_UCHAR head256[4]= {0x00, 0x00, 0x01, 0x00};
    USBKEY_UCHAR head384[4] = {0x00, 0x00, 0x01, 0x80};

    AK_Log(AK_LOG_DEBUG, "USBKEY_ExportECCPublicKey");

    //参数校验
    if( NULL == hDeviceHandle || NULL == pcPubKeyID || NULL == pPublicKey )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ExportECCPublicKey ==> Param is NULL! rtn = %#08x", USBKEY_PARAM_ERROR);
        return USBKEY_PARAM_ERROR;
    }

    AK_LockMutex(gpMutex);

    //选择公钥文件
    rtn = SelectFile(hDeviceHandle,NULL, pcPubKeyID);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ExportECCPublicKey ==>SelectFile() Error,rtn = %#08x", rtn);
        return rtn;
    }

    //读公钥文件
    rtn = ReadCurBinaryFile(hDeviceHandle, 0, USBKEY_PUBLIC_KEY_FILE_LEN, pcBuf);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ExportECCPublicKey ==>ReadCurBinaryFile() Error,rtn = %#08x", rtn);
        return rtn;
    }

    if( memcmp(head256, pcBuf, 4) == 0 )
    {
        pPublicKey->bits = 256;	
    }
    else if( memcmp(head384, pcBuf, 4) == 0 )
    {
        pPublicKey->bits = 384;
    }
    else
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ExportECCPublicKey ==> The public key is not valid!");
        return USBKEY_GENERAL_ERROR;
    }

    memcpy(pPublicKey->XCoordinate, pcBuf+4, pPublicKey->bits/8);
    memcpy(pPublicKey->YCoordinate, pcBuf+4+ pPublicKey->bits/8, pPublicKey->bits/8);

    AK_Log(AK_LOG_DEBUG, "USBKEY_ExportECCPublicKey Success!");
    AK_UnlockMutex(gpMutex);

    return USBKEY_OK;
}


//	产生临时ECC非对称密钥，不存储.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_GenerateExtECCKeyPair(
        USBKEY_HANDLE			hDeviceHandle,	//[in]设备句柄
        USBKEY_ULONG			ulBitLen,		//[in]密钥模长
        USBKEY_ECC_PUBLIC_KEY	*pPublicKey,	//[out]返回的公钥结构
        USBKEY_ECC_PRIVATE_KEY	*pPrivateKey)	//[out]返回的私钥结构
{
    //USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    USBKEY_UCHAR data[300];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0;
    USBKEY_UCHAR p1,p2,lc,le;

    AK_Log(AK_LOG_DEBUG, "USBKEY_GenerateExtECCKeyPair");

    //参数校验
    if( NULL == hDeviceHandle || NULL == pPublicKey || NULL == pPrivateKey )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_GenerateExtECCKeyPair ==> Param is NULL! rtn = %#08x", USBKEY_PARAM_ERROR);
        return USBKEY_PARAM_ERROR;
    }
    if( 256 != ulBitLen && 384 != ulBitLen )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_GenerateExtECCKeyPair ==> ulBitLen Error,ulBitLen = %#08x", ulBitLen);
        return USBKEY_PARAM_ERROR; 
    }

    AK_LockMutex(gpMutex);

    //Delete by zhoushenshen in 20121217
    /*//组包
      request[0] = USBKEY_CMD_TYPE_PKI;					//cla
      request[1] = USBKEY_CMD_DODE_GEN_ECC_KEY_PAIR;		//ins
      request[2] = 0;	//内部不保存密钥对					//p1
      if( 256 == ulBitLen )
      request[3] = 0x02;								//p2
      else
      request[3] = 0x03;								//p2
      request[4] = (USBKEY_UCHAR)((ulBitLen/8) * 3);						//le
      reqLen = 5;

    //发送命令
    respLen = 300;
    rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
    if (rtn != 0)
    {
    AK_Log(AK_LOG_ERROR, "USBKEY_GenerateExtECCKeyPair ==>DeviceExecCmd() Error,rtn = %#08x", rtn);
    return rtn;
    }*/
    //Del end

    //Add by zhoushenshen in 20121217
    //组包
    p1 = 0;	//内部不保存密钥对					//p1
    if( 256 == ulBitLen )
        p2 = 0x02;								//p2
    else
        p2 = 0x03;								//p2
    lc = 0x00;
    le = (USBKEY_UCHAR)((ulBitLen/8) * 3);						//le

    //发送命令
    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_GenECCKeyPair,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_GenerateExtECCKeyPair ==>PackageData() Error,rtn = %#08x", rtn);
        return rtn;
    }
    //Add end

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_GenerateExtECCKeyPair ==>DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
    }

    pPrivateKey->bits = ulBitLen;
    memcpy(pPrivateKey->PrivateKey, response, ulBitLen/8);
    pPublicKey->bits = ulBitLen;
    memcpy(pPublicKey->XCoordinate, response+ulBitLen/8, ulBitLen/8);
    memcpy(pPublicKey->YCoordinate, response+ulBitLen/4, ulBitLen/8);

    AK_Log(AK_LOG_DEBUG, "USBKEY_GenerateExtECCKeyPair Success!");
    AK_UnlockMutex(gpMutex);

    return USBKEY_OK;
}

//	关闭应用.
//	返回值
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_CloseApplication(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcAppName)	//[in]送入的应用名称
{
    USBKEY_ULONG rtn;
    AK_Log(AK_LOG_DEBUG, "USBKEY_CloseApplication");

    if (strlen((char *)pcAppName) > 16)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_CloseApplication ==> pcAppName Length Error,rtn = %#08x", USBKEY_PARAM_ERROR);
        return USBKEY_PARAM_ERROR;
    }

    AK_LockMutex(gpMutex);
    rtn = SelectFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"/",NULL);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_CloseApplication ==>SelectFile() Error,rtn = %#08x", rtn);
        return rtn;
    }

    AK_Log(AK_LOG_DEBUG, "USBKEY_CloseApplication Success!");
    AK_UnlockMutex(gpMutex);

    return USBKEY_OK;
}


/*------------------------------------------------税控应用API-------------------------------------------------*/
//	解锁隐藏分区用户PIN.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_UnlockHiDiskPin(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_ULONG ulPINType,			//[in]需解锁PIN类型
        USBKEY_UCHAR_PTR pcAdminPIN,	//[in]送入的管理员PIN
        USBKEY_ULONG ulAdminLen,		//[in]送入的PIN长度
        USBKEY_UCHAR_PTR pcNewPIN,		//[in]送入的用户新PIN
        USBKEY_ULONG ulNewLen,			//[in]送入的用户新PIN长度
        USBKEY_ULONG_PTR pulRetryNum)	//[out]管理员PIN出错后重试次数
{
#ifdef _SEC_GTAX_APP
    USBKEY_ULONG rtn;
    USBKEY_ULONG ulAdPinType;

    AK_Log(AK_LOG_DEBUG, "USBKEY_UnlockHiDiskPin");

    if( (ulAdminLen > USBKEY_PIN_MAX_LEN) || (ulAdminLen < USBKEY_PIN_MIN_LEN) )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_UnlockHiDiskPin ==> Admin PIN Length Error,ulAdminLen = %#08x", ulAdminLen);
        return USBKEY_PARAM_ERROR;
    }
    if( (ulNewLen > USBKEY_PIN_MAX_LEN) || (ulNewLen < USBKEY_PIN_MIN_LEN) )
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_UnlockHiDiskPin ==> New PIN Length Error,ulUserNewLen = %#08x", ulNewLen);
        return USBKEY_PARAM_ERROR;
    }

    if(ulPINType == USBKEY_USERTYPE_HIDISK_ADMIN)
    {
        ulAdPinType = USBKEY_USERTYPE_ADMIN;
    }
    else if(ulPINType == USBKEY_USERTYPE_HIDISK_USER)
    {
        ulAdPinType = USBKEY_USERTYPE_HIDISK_ADMIN;
    }
    else
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_UnlockHiDiskPin ==> ulPINType Error,ulPINType = %#08x", ulPINType);
        return USBKEY_PARAM_ERROR;
    }

    //验证PIN
    rtn = USBKEY_CheckPin(hDeviceHandle,ulAdPinType,pcAdminPIN,ulAdminLen,pulRetryNum);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_UnlockHiDiskPin ==> USBKEY_CheckPin() Error,rtn = %#08x", rtn);
        return rtn;
    }

    //初始化PIN
    AK_LockMutex(gpMutex);
    if(ulPINType == USBKEY_USERTYPE_HIDISK_ADMIN)
    {
        //**************************初始化隐藏区高级管理用户PIN（税控金税卡应用）*************************/
        rtn = CreateAppPin(
                hDeviceHandle,					//[in]设备句柄
                pcNewPIN,						//[in]送入的PIN
                ulNewLen,						//[in]送入的PIN长度
                USBKEY_HIDISK_ADMIN_PIN_FLAG,	//[in]PIN码标识
                USBKEY_HIDISK_ADMIN_PRIV,		//[in]PIN验证后安全状态
                KEY_RETRY_MAX_TIMES,			//[in]重试次数
                USBKEY_ALL_PRIV,				//[in]使用权限
                0x65);							//[in]更改权限
        if (rtn != 0)
        {
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_UnlockHiDiskPin ==> CreateAppPin() APP HiDisk Admin Key Error,rtn = %#08x", rtn);
            return rtn;
        }
        AK_Log(AK_LOG_DEBUG, "USBKEY_UnlockHiDiskPin Unlock APP HidDisk Admin PIN Success!");
    }
    else
    {
        //**************************初始化隐藏区普通用户PIN（税控金税卡应用）*************************/
        rtn = CreateAppPin(
                hDeviceHandle,					//[in]设备句柄
                pcNewPIN,						//[in]送入的PIN
                ulNewLen,						//[in]送入的PIN长度
                USBKEY_HIDISK_USER_PIN_FLAG,	//[in]PIN码标识
                USBKEY_HIDISK_USER_PRIV,		//[in]PIN验证后安全状态
                KEY_RETRY_MAX_TIMES,			//[in]重试次数
                USBKEY_ALL_PRIV,				//[in]使用权限
                0x54);							//[in]更改权限
        if (rtn != 0)
        {
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_UnlockHiDiskPin ==> CreateAppPin() APP HiDisk User Key Error,rtn = %#08x", rtn);
            return rtn;
        }
        AK_Log(AK_LOG_DEBUG, "USBKEY_UnlockHiDiskPin Unlock APP HidDisk User PIN Success!");
    }


    AK_Log(AK_LOG_DEBUG, "USBKEY_UnlockHiDiskPin Success!");
    AK_UnlockMutex(gpMutex);
    return USBKEY_OK;
#else
    AK_Log(AK_LOG_ERROR, "USBKEY_UnlockHiDiskPin ==> the fun not support,rtn = %#08x", USBKEY_FUNCTION_NOT_SUPPORT);
    return USBKEY_FUNCTION_NOT_SUPPORT;
#endif
}

//	读税控应用存放税控信息文件.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ReadTaxInfoFile(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pucFileID,		//[in]文件ID
        USBKEY_ULONG ulOffset,			//[in]读取偏移量
        USBKEY_UCHAR_PTR pcOutData,		//[out]返回数据
        USBKEY_ULONG_PTR pulSize)		//[in,out]输入读取数据长度，输出实际读取长度
{
#ifdef _SEC_GTAX_APP
    USBKEY_ULONG rtn;
    USBKEY_UCHAR pcFileID[2];

    AK_Log(AK_LOG_DEBUG, "USBKEY_ReadTaxInfoFile Success!");

    //COS只接受2字节偏移量
    if(ulOffset >= 256*256)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_ReadTaxInfoFile ==> ulOffset too long,ulOffset = %#08x", ulOffset);
        return USBKEY_PARAM_ERROR;
    }	

    AK_LockMutex(gpMutex);

    //选择对称密钥ID文件
    pcFileID[0] = pucFileID[0];	//0x80
    pcFileID[1] = pucFileID[1];	//0xff
    rtn = SelectFile(hDeviceHandle,NULL, pcFileID);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "====>>USBKEY_ReadTaxInfoFile->SelectFile() Error,rtn = %#08x",rtn);
        return rtn;
    }


    if(*pulSize + ulOffset > USBKEY_TAX_INFO_ID_FILE_LEN)
        *pulSize = USBKEY_TAX_INFO_ID_FILE_LEN - ulOffset;

    rtn = ReadCurBinaryFile(hDeviceHandle, ulOffset, *pulSize, pcOutData);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "====>>USBKEY_ReadTaxInfoFile->ReadCurBinaryFile() Error,rtn = %#08x",rtn);
        return rtn;
    }
    AK_Log(AK_LOG_DEBUG, "USBKEY_ReadTaxInfoFile Success!");
    AK_UnlockMutex(gpMutex);
    return USBKEY_OK;
#else
    AK_Log(AK_LOG_ERROR, "USBKEY_ReadTaxInfoFile ==> the fun not support,rtn = %#08x", USBKEY_FUNCTION_NOT_SUPPORT);
    return USBKEY_FUNCTION_NOT_SUPPORT;
#endif
}


//	写税控应用存放税控信息文件.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_WriteTaxInfoFile(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pucFileID,		//[in]文件ID
        USBKEY_ULONG ulOffset,			//[in]写入偏移量
        USBKEY_UCHAR_PTR pcInData,		//[in]写入数据
        USBKEY_ULONG ulSize)			//[in]写入数据长度
{
#ifdef _SEC_GTAX_APP
    USBKEY_ULONG rtn;
    USBKEY_UCHAR pcFileID[2];

    AK_Log(AK_LOG_DEBUG, "USBKEY_WriteTaxInfoFile Success!");

    //COS只接受2字节偏移量
    if(ulOffset >= 256*256)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_WriteTaxInfoFile ==> ulOffset too long,ulOffset = %#08x", ulOffset);
        return USBKEY_PARAM_ERROR;
    }
    if(ulOffset + ulSize > USBKEY_TAX_INFO_ID_FILE_LEN)
    {
        AK_Log(AK_LOG_ERROR, "USBKEY_WriteTaxInfoFile ==> write data too long,ulSize = %#08x", ulSize);
        return USBKEY_PARAM_ERROR;
    }

    AK_LockMutex(gpMutex);

    //选择对称密钥ID文件
    pcFileID[0] = pucFileID[0];	//0x80
    pcFileID[1] = pucFileID[1];	//0xff
    rtn = SelectFile(hDeviceHandle,NULL, pcFileID);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "====>>USBKEY_WriteTaxInfoFile->SelectFile() Error,rtn = %#08x",rtn);
        return rtn;
    }

    rtn = WriteCurBinaryFile(hDeviceHandle, ulOffset, ulSize, pcInData);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "====>>USBKEY_WriteTaxInfoFile->WriteCurBinaryFile() Error,rtn = %#08x",rtn);
        return rtn;
    }
    AK_Log(AK_LOG_DEBUG, "USBKEY_WriteTaxInfoFile Success!");
    AK_UnlockMutex(gpMutex);
    return USBKEY_OK;
#else
    AK_Log(AK_LOG_ERROR, "USBKEY_WriteTaxInfoFile ==> the fun not support,rtn = %#08x", USBKEY_FUNCTION_NOT_SUPPORT);
    return USBKEY_FUNCTION_NOT_SUPPORT;
#endif
}


//	产生税控对称密钥.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_GenTaxKey(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_ULONG ulKeyLen,			//[in]密钥长度
        USBKEY_ULONG ulAlgID,			//[in]密钥算法标示
        USBKEY_ULONG ulKeyFlag,			//[in]导入密钥标志，0为0号密钥，1为1号密钥
        USBKEY_UCHAR_PTR phKey)			//[out]返回的密钥句柄
{
#ifdef _ATAX_APP
    USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    USBKEY_ULONG rtn,reqLen,respLen=0;
    USBKEY_UCHAR data[24];
    USBKEY_UCHAR p1,p2,lc,le;
    USBKEY_UCHAR ucTaxKeyID_0 = 0,ucTaxKeyID_1 = 0;
    USBKEY_UCHAR pcConSymKeyIDRec[USBKEY_MAX_SYM_KEY_ID];

    AK_Log(AK_LOG_DEBUG, "USBKEY_GenTaxKey");

    /*if (strlen(pcAppName) > 16)
      {
      LogMessage("====>>USBKEY_GenTaxKey->pcAppName Length Error!");
      return USBKEY_PARAM_ERROR;
      }*/

    AK_LockMutex(gpMutex);

    //检查是否存在容器名为TAX_KEY_FLAG的记录
    rtn = SearchTaxSymmetricKey(hDeviceHandle,&ucTaxKeyID_0,&ucTaxKeyID_1);
    if( rtn != 0 )
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_GenTaxKey ==> SearchTaxSymmetricKey() Error,rtn = %#08x", rtn);
        return rtn;
    }

    //若不存在TAX_KEY记录，则创建新记录
    if(-1 == ucTaxKeyID_0)
    {
        //获取空闲的对称密钥ID	(flag为0)
        rtn = GetFreeTaxKeyID(hDeviceHandle, &ucTaxKeyID_0, &ucTaxKeyID_1, pcConSymKeyIDRec);
        if( rtn != 0 )
        {
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_GenTaxKey ==> GetFreeTaxKeyID() Error,rtn = %#08x", rtn);
            return rtn;
        }
    }

    /*rtn = SelectFile(hDeviceHandle,pcAppName,NULL);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage16Val("====>>USBKEY_GenTaxKey->SelectFile() Error,rtn = 0x",rtn);
      return rtn;
      }*/

    p1 = 0x00;
    p2 = 0x00;
    lc = 0x06;
    le = 0x00;

    memset(data,0,sizeof(data));

    //设置密钥属性：密钥标识（1字节）、密钥类型（1字节）、算法标识（1字节）、控制标志（1字节）、使用权限（1字节）、更改权限（1字节）
    if(ulKeyFlag == 0)
        data[0] = ucTaxKeyID_0;	//密钥标识
    else
        data[0] = ucTaxKeyID_1;		//密钥标识
    data[1] = 7;			//密钥类型：卡内固定密钥
    data[2] = (USBKEY_UCHAR)ulAlgID;		//算法标识
    data[3] = 1;			//控制标志: 1，密钥不可以导出；0，密钥可以导出
    data[4] = USBKEY_USER_USE_PRIV;		//使用权限
    data[5] = USBKEY_USER_USE_PRIV;		//更改权限,管理员权限可以修改

    /*rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_GenSymmKey,request,&reqLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_GenTaxKey ==>PackageData() Error,rtn = %#08x", rtn);
      return rtn;
      }

      respLen = 300;
      rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_GenTaxKey ==>DeviceExecCmd() Error,rtn = %#08x", rtn);
      return rtn;
      }*/

    //Add by zhoushenshen in 20121114
    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_GenSymmKey,response,&respLen);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_GenTaxKey ==> PackageData() Error, rtn = %#08x", rtn);
        return rtn;
    }
    //Add end
    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_GenTaxKey ==>DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
            break;
    }

    //把对称密钥增加到容器
    rtn = AddTaxSymKey(hDeviceHandle, ucTaxKeyID_0, ucTaxKeyID_1, pcConSymKeyIDRec);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_GenTaxKey ==> AddTaxSymKey() Error,rtn = %#08x", rtn);
        return rtn;
    }

    *phKey = 0x10;
    *(phKey+1) = data[0];

    AK_Log(AK_LOG_DEBUG, "USBKEY_GenTaxKey Success!");
    AK_UnlockMutex(gpMutex);
    return USBKEY_OK;
#else
    AK_Log(AK_LOG_ERROR, "USBKEY_GenTaxKey ==> the fun not support,rtn = %#08x", USBKEY_FUNCTION_NOT_SUPPORT);
    return USBKEY_FUNCTION_NOT_SUPPORT;
#endif
}

//	导入税控应用密钥.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ImportTaxKey(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcWrapKeyID,	//[in]用来保护导入密钥的密钥ID
        USBKEY_ULONG ulAsymAlgID,		//[in]用来保护导入密钥的密钥算法：USBKEY_TYPE_RSA或者USBKEY_TYPE_ECC；如果明文导入设置为0
        USBKEY_ULONG ulAlgID,			//[in]密钥算法标识
        USBKEY_ULONG ulKeyFlag,			//[in]导入密钥标志，0为0号密钥，1为1号密钥
        USBKEY_UCHAR_PTR pcSymKey,		//[in]导入的密钥
        USBKEY_ULONG ulSymKeyLen,		//[in]导入的密钥长度
        USBKEY_UCHAR_PTR pcSymKeyID)	//[out]返回导入的对称密钥ID
{
#ifdef _ATAX_APP
    USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    USBKEY_ULONG rtn,reqLen,respLen=0;
    USBKEY_UCHAR data[300];
    USBKEY_UCHAR p1,p2,lc,le;
    USBKEY_UCHAR keyAttr[6];	//密钥属性 add by heli-20090520
    USBKEY_UCHAR ucTaxKeyID_0 = 0,ucTaxKeyID_1 = 0;
    USBKEY_UCHAR pcConSymKeyIDRec[USBKEY_MAX_SYM_KEY_ID];

    //明文导入：
    //密钥标识（1字节）、密钥类型（1字节）、算法标识（1字节）、控制标志（1字节）、使用权限（1字节）、更改权限（1字节）、密钥值（8字节/16字节/24字节）

    //密文导入：
    //密钥标识（1字节）、密钥类型（1字节）、算法标识（1字节）、控制标志（1字节）、使用权限（1字节）、更改权限（1字节）、用指定的公钥对密钥值加密所得的密文。

    AK_Log(AK_LOG_DEBUG, "USBKEY_ImportTaxKey");
    /*if (strlen(pcAppName) > 16)
      {
      LogMessage("====>>USBKEY_ImportTaxKey->pcAppName Length Error!");
      return USBKEY_PARAM_ERROR;
      }*/

    AK_LockMutex(gpMutex);

    //检查是否存在容器名为TAX_KEY_FLAG的记录
    rtn = SearchTaxSymmetricKey(hDeviceHandle,&ucTaxKeyID_0,&ucTaxKeyID_1);
    if( rtn != 0 )
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportTaxKey ==> SearchTaxSymmetricKey() Error,rtn = %#08x", rtn);
        return rtn;
    }

    //若不存在TAX_KEY记录，则创建新记录
    if(-1 == ucTaxKeyID_0)
    {
        //获取空闲的对称密钥ID	(flag为0)
        rtn = GetFreeTaxKeyID(hDeviceHandle, &ucTaxKeyID_0, &ucTaxKeyID_1, pcConSymKeyIDRec);
        if( rtn != 0 )
        {
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_ImportTaxKey ==> GetFreeTaxKeyID() Error,rtn = %#08x", rtn);
            return rtn;
        }
    }

    /*rtn = SelectFile(hDeviceHandle,pcAppName,NULL);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      LogMessage16Val("====>>USBKEY_ImportTaxKey->SelectFile() Error,rtn = 0x",rtn);
      return rtn;
      }*/

    //设置密钥属性：密钥标识（1字节）、密钥类型（1字节）、算法标识（1字节）、控制标志（1字节）、使用权限（1字节）、更改权限（1字节）
    if(ulKeyFlag == 0)
        keyAttr[0] = ucTaxKeyID_0;	//密钥标识
    else
        keyAttr[0] = ucTaxKeyID_1;	//密钥标识
    keyAttr[1] = 7;						//密钥类型：卡内固定密钥
    keyAttr[2] = (USBKEY_UCHAR)ulAlgID;				//算法标识
    keyAttr[3] = 1;						//控制标志: 1，密钥不可以导出；0，密钥可以导出
    keyAttr[4] = USBKEY_USER_USE_PRIV;		//使用权限
    keyAttr[5] = USBKEY_USER_USE_PRIV;		//更改权限,管理员权限可以修改

    //Delete by zhoushenshen in 20111124
    /*if (pcWrapKeyID == NULL)
      {
      p1 = 0x00;
      p2 = 0x00;
      if ((ulSymKeyLen != 8) && (ulSymKeyLen != 16) && (ulSymKeyLen != 24))
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_ImportTaxKey ==>ulSymKeyLen Error,ulSymKeyLen = %#08x", ulSymKeyLen);
      return USBKEY_PARAM_ERROR;
      }
      else
      lc = (USBKEY_UCHAR)(ulSymKeyLen+6);
      memcpy(data,keyAttr,6);
      memcpy(data+6,pcSymKey,ulSymKeyLen);
      }
      else
      {
      memcpy(&p1,pcWrapKeyID,1);
      memcpy(&p2,pcWrapKeyID+1,1);
      if (ulSymKeyLen == 128)
      {
      lc = 0x86;
      memcpy(data,keyAttr,6);
      memcpy(data+6,pcSymKey,ulSymKeyLen);
      }
      else if (ulSymKeyLen == 256)
      {
      lc = 0xff;
      memset(data,0x01,1);
      memset(data+1,0x06,1);
      memcpy(data+2,keyAttr,6);
      memcpy(data+8,pcSymKey,ulSymKeyLen);
      }
      else
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_ImportTaxKey ==>ulSymKeyLen Error,ulSymKeyLen = %#08x", ulSymKeyLen);
      return USBKEY_PARAM_ERROR;
      }
      }
      le = 0x00;

      rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_ImportDivSymmKey,request,&reqLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_ImportTaxKey ==>PackageData() Error,rtn = %#08x", rtn);
      return rtn;
      }

      respLen = 300;
      rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
      if (rtn != 0)
      {
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_ImportTaxKey ==>DeviceExecCmd() Error,rtn = %#08x", rtn);
      return rtn;
      }
      rtn = *(response+respLen-2);
      rtn = rtn*256 + *(response+respLen-1);
      switch(rtn)
      {
      case DEVICE_SUCCESS:
      break;
      default:
      AK_UnlockMutex(gpMutex);
      AK_Log(AK_LOG_ERROR, "USBKEY_ImportTaxKey ==>DeviceExecCmd() Error,code = %#08x", rtn);
      return rtn;
      break;
      }*/

    //Add by zhoushenshen in 20111124
    if( 0 == ulAsymAlgID || USBKEY_TYPE_RSA == ulAsymAlgID )
    {
        //rsa加密导入或者明文导入对称密钥
        rtn = RSAImportSessionKey(hDeviceHandle, pcWrapKeyID, keyAttr, pcSymKey, ulSymKeyLen);
        if (rtn != 0)
        {
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_ImportTaxKey ==>RSAImportSessionKey() Error,rtn = %#08x", rtn);
            return rtn;
        }
    }
    else
    {
        //ECC加密导入对称密钥
        rtn = ECCImportSessionKey(hDeviceHandle, pcWrapKeyID, keyAttr, pcSymKey, ulSymKeyLen);
        if (rtn != 0)
        {
            AK_UnlockMutex(gpMutex);
            AK_Log(AK_LOG_ERROR, "USBKEY_ImportTaxKey ==>ECCImportSessionKey() Error,rtn = %#08x", rtn);
            return rtn;
        }
    }

    //把对称密钥增加到容器
    rtn = AddTaxSymKey(hDeviceHandle, ucTaxKeyID_0, ucTaxKeyID_1, pcConSymKeyIDRec);
    if (rtn != 0)
    {
        AK_UnlockMutex(gpMutex);
        AK_Log(AK_LOG_ERROR, "USBKEY_ImportTaxKey ==> AddTaxSymKey() Error,rtn = %#08x", rtn);
        return rtn;
    }

    *pcSymKeyID = 0x10;
    *(pcSymKeyID+1) = keyAttr[0];

    AK_Log(AK_LOG_DEBUG, "USBKEY_ImportTaxKey Success!");
    AK_UnlockMutex(gpMutex);

    return USBKEY_OK;
#else
    AK_Log(AK_LOG_ERROR, "USBKEY_ImportTaxKey ==> the fun not support,rtn = %#08x", USBKEY_FUNCTION_NOT_SUPPORT);
    return USBKEY_FUNCTION_NOT_SUPPORT;
#endif
}


///////////////////////////////////////////////////////////////////////////////
//	END
///////////////////////////////////////////////////////////////////////////////
