/*	util.c
Author  :   caizhun@aisino.com
CO.     :   AISINO
date    :   2008年5月26日，
描述    :   USBKEY底层API工具函数接口	*/

#include "stdafx.h"
#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include <Dbt.h>

#include "./inc/UsbKeyAPI.h"
//#include "../inc/Device.h"
#include "./inc/UsbKeyComDLL.h"

#include "./inc/DataPackage.h"
//#include "./inc/Debug.h"
#include "./inc/global.h"
#include "./inc/util.h"
#include "./inc/AK_Log.h"

//////////////////////////////////////////////////////////////////////////
//工具函数
//==============================================================
//函数名称:		IsNewInDevice
//函数参数:		pDeviceName：设备名称
//返回值:		TRUE：新设备
//				FALSE：原有设备
//函数功能: 	判断是否为新设备。
//==============================================================
BOOL IsNewInDevice(char ppchDevNameList[DEVICE_MAX_QUANTITY][MAX_PATH], char* pDeviceName)
{
    int i;
    for( i=0; i<DEVICE_MAX_QUANTITY; i++ )
    {
        if( '\0' != ppchDevNameList[i] )
            continue;
        if( 0 == strcmp(ppchDevNameList[i], pDeviceName) )
        {
            break;	//该设备不是新插入的设备
        }
    }

    if( i == DEVICE_MAX_QUANTITY )
        return TRUE;
    else
        return FALSE;
}

//==============================================================
//函数名称:		DealDeviceIn
//函数参数:		无			
//返回值:		无
//函数功能: 	处理设备插入事件
//==============================================================
void  DealDeviceIn(char ppchDevNameList[DEVICE_MAX_QUANTITY][MAX_PATH], char* pchDevName, USBKEY_ULONG* pulLen, USBKEY_ULONG* pulEventType)
{
    int iDeviceNum = 0;
    int i;
    char ppchDeviceName[DEVICE_MAX_QUANTITY][MAX_PATH];

    //枚举设备
    if( 0 != USBKEY_DeviceEnum(ppchDeviceName, &iDeviceNum) )
    {
        return;
    }

    if( iDeviceNum <= g_DevNum )
        return;

    AK_Log(AK_LOG_DEBUG, "device insert.");


    //查找插入的设备
    char* pTmpDevName = pchDevName;
    *pulLen = 0;
    for(i=0; i<DEVICE_MAX_QUANTITY; i++)
    {
        if( '\0' == ppchDeviceName[i][0] )
            continue;

        if(!IsNewInDevice(ppchDevNameList, ppchDeviceName[i]))
            continue;

        //记录插入设备名称
        strcpy(pTmpDevName, ppchDeviceName[i]);
        *pulLen += (USBKEY_ULONG)strlen(ppchDeviceName[i]) + 1;
        pTmpDevName += strlen(ppchDeviceName[i]) + 1;
    }

    *pulEventType = DEV_IN;

    return;
}

//===============================================================
//函数名称:		DealDeviceOut
//函数参数:		无			
//返回值:		无
//函数功能: 	处理设备拔出事件。
//===============================================================
void  DealDeviceOut(char ppchDevNameList[DEVICE_MAX_QUANTITY][MAX_PATH], char* pchDevName, USBKEY_ULONG* pulLen, USBKEY_ULONG* pulEventType)
{
    char ppchDeviceName[DEVICE_MAX_QUANTITY][MAX_PATH];
    int iDeviceNum = 0;
    int i,j;

    //枚举设备
    if( 0 != USBKEY_DeviceEnum(ppchDeviceName, &iDeviceNum) )
    {
        return;
    }

    if( iDeviceNum >= g_DevNum )
        return;

    AK_Log(AK_LOG_DEBUG, "device remove.");

    //查找拔出的设备
    char* pTmpDevName = pchDevName;
    *pulLen = 0;
    for(i=0; i<DEVICE_MAX_QUANTITY; i++)
    {
        if( '\0' == ppchDevNameList[i][0] )
            continue;

        for(j=0; j<DEVICE_MAX_QUANTITY; j++)
        {
            if( '\0' == ppchDeviceName[i][0] )
                continue;
            if( 0 == strcmp(ppchDevNameList[i], ppchDeviceName[j]) )
            {
                break;	
            }
        }

        if( j == DEVICE_MAX_QUANTITY )//找到拔出的设备
        {
            //记录拔出设备名称
            strcpy(pTmpDevName, (char*)ppchDevNameList[i]);
            *pulLen += (USBKEY_ULONG)strlen((char*)ppchDevNameList[i]) + 1;
            pTmpDevName += strlen((char*)ppchDevNameList[i]) + 1;
        }

    }

    *pulEventType = DEV_OUT;

    return;
}



//回调函数
LRESULT CALLBACK CallUsbProc( int nCode, WPARAM wParam, LPARAM lParam )
{
    int iDeviceNum;
    //	char chmsg[255];
    USBKEY_CHAR ppcDeviceIdS[10][260];

    if( nCode == HC_ACTION )
    {
        CWPSTRUCT *msg = (CWPSTRUCT *)lParam;

        if (msg->message == WM_DEVICECHANGE)
        {

            switch(msg->wParam)
            {
                case DBT_DEVICEARRIVAL://设备插入			
                case DBT_DEVICEREMOVECOMPLETE://设备拔出
                case DBT_DEVNODES_CHANGED:	//设备插拔
                    USBKEY_DeviceEnum(ppcDeviceIdS, &iDeviceNum);
                    if( iDeviceNum != g_DevNum )
                    {	
                        //sprintf(chmsg, "WM_DEVICECHANGE: msg->wParam = 0x%x, msg->lParam = 0x%x", msg->wParam, msg->lParam);
                        //MessageBox(NULL,chmsg,"",MB_OK);
                        SetEvent(g_hDevEvent);
                        return TRUE;
                    }
                    break;
            }
        }
    }

    return CallNextHookEx( g_hHook, nCode, wParam, lParam); 
}



USBKEY_ULONG CheckValid(USBKEY_UCHAR_PTR name)
{
    USBKEY_ULONG reserved,rtn = 0;
    //根目录文件
    reserved = 0x003f;
    if (memcmp(name,&reserved,2) == 0) //(strlen(name) != 2) ||
    {
        rtn = 1;
    }
    //目录系统密钥文件
    reserved = 0x013f;
    if (memcmp(name,&reserved,2) == 0) //(strlen(name) != 2) ||
    {
        rtn = 1;
    }
    //目录用户密钥文件
    reserved = 0x023f;
    if (memcmp(name,&reserved,2) == 0) //(strlen(name) != 2) ||
    {
        rtn = 1;
    }
    //目录记录文件
    reserved = 0x0080;
    if (memcmp(name,&reserved,2) == 0) //(strlen(name) != 2) ||
    {
        rtn = 1;
    }
    //文件记录文件
    reserved = 0x0180;
    if (memcmp(name,&reserved,2) == 0)
    {
        rtn = 1;
    }
    //Container记录文件
    reserved = 0x0280;
    if (memcmp(name,&reserved,2) == 0)
    {
        rtn = 1;
    }
    //会话密钥记录文件
    reserved = 0x0380;
    if (memcmp(name,&reserved,2) == 0)
    {
        rtn = 1;
    }
    return rtn;
}

USBKEY_ULONG SelectFile(USBKEY_HANDLE hDeviceHandle,USBKEY_UCHAR_PTR pcDirName, USBKEY_UCHAR_PTR pcFileID)
{
    USBKEY_UCHAR response[300];
    USBKEY_UCHAR data[300],dirName[3];
    USBKEY_ULONG rtn,respLen=300;
    USBKEY_UCHAR p1,p2,lc,le;

    AK_Log(AK_LOG_DEBUG, "====>>SelectFile");

    if (pcDirName != NULL)
    {
        //选择目录
        if (strcmp((char *)pcDirName,"/") == 0)
        {
            dirName[0] = 0x3f;
            dirName[1] = 0x00;
            dirName[2] = 0x00;
            p1 = 0x00;
            p2 = 0x00;
            lc = 2;
            le = 0x00;
            memset(data,0,300);
            memcpy(data,dirName,lc);
        }
        else
        {
            p1 = 0x04;
            p2 = 0x00;
            lc = (USBKEY_UCHAR)strlen((char *)pcDirName);
            if (lc > 16)
            {
                AK_Log(AK_LOG_ERROR, "====>>SelectFile->DirName Too Long Error,Max len is 16.len = %#08x", lc);
                return USBKEY_PARAM_ERROR;
            }
            le = 0x00;
            memset(data,0,300);
            memcpy(data,pcDirName,lc);
            if (lc < 5)
            {
                memset(data+lc,0x01,5-lc);
                lc = 0x05;
            }
        }

        respLen = 300;
        rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_SelectFile,response,&respLen);
        if (rtn != 0)
        {
            AK_Log(AK_LOG_ERROR, "====>>SelectFile->PackageData() Error, rtn = %#08x", rtn);
            return rtn;
        }

        rtn = *(response+respLen-2);
        rtn = rtn*256 + *(response+respLen-1);
        switch(rtn)
        {
            case DEVICE_SUCCESS:
                break;
            case DEVICE_FILE_NOT_FIND:
                AK_Log(AK_LOG_ERROR, "====>>SelectFile Dir file not find, code = %#08x", rtn);
                return USBKEY_FILE_NOT_FOUND;
                break;
            default:
                AK_Log(AK_LOG_ERROR, "====>>SelectFile->DeviceExecCmd() Dir Error,code = %#08x", rtn);
                return rtn;
                break;
        }

    }

    if (pcFileID != NULL)
    {
        //选择文件
        p1 = 0x00;
        p2 = 0x00;
        lc = 0x02;
        le = 0x00;
        memset(data,0,300);
        memcpy(data,pcFileID,2);

        respLen = 300;
        rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_SelectFile,response,&respLen);
        if (rtn != 0)
        {
            AK_Log(AK_LOG_ERROR, "====>>SelectFile->PackageData() fileID Error, rtn = %#08x", rtn);
            return rtn;
        }

        rtn = *(response+respLen-2);
        rtn = rtn*256 + *(response+respLen-1);
        switch(rtn)
        {
            case DEVICE_SUCCESS:
                break;
            default:
                AK_Log(AK_LOG_ERROR, "====>>SelectFile->DeviceExecCmd() fileID Error,code = %#08x", rtn);
                return rtn;
                break;
        }
    }

    AK_Log(AK_LOG_DEBUG, "====>>SelectFile Success!");
    return USBKEY_OK;
}

USBKEY_ULONG CreateDirRecord(USBKEY_HANDLE hDeviceHandle,USBKEY_UCHAR_PTR pcDirName)
{
    USBKEY_UCHAR response[300];
    USBKEY_UCHAR data[300];
    USBKEY_ULONG rtn,respLen=0,recNO;
    USBKEY_UCHAR p1,p2,lc,le;
    USBKEY_UCHAR pcFileName[2];
    USBKEY_UCHAR ulAccessCondition;

    AK_Log(AK_LOG_DEBUG, "====>>CreateDirRecord");

    p1 = 0x00;
    p2 = 0x03;
    lc = 0x07;
    le = 0x00;

    //创建文件
    pcFileName[0] = 0x80;
    pcFileName[1] = 0x00;
    memset(data,0,300);
    memcpy(data,pcFileName,2);
    le = 0x00;
    memcpy(data+2,&le,1);
    ulAccessCondition = USBKEY_ALL_PRIV;
    memcpy(data+3,&ulAccessCondition,1);
    //ulAccessCondition = USBKEY_DEVICE_PRIV;		//ADD by zhoushenshen 1117
    ulAccessCondition = USBKEY_ALL_PRIV;	//update in 20120108 因权限即时生效暂时改为建立文件无权限要求
    memcpy(data+4,&ulAccessCondition,1);
    le = 0x10;
    memcpy(data+5,&le,1);
    le = APPLICATION_MAX_QUANTITY;
    memcpy(data+6,&le,1);

    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_CreateFile,response,&respLen);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>CreateDirRecord->PackageData() Error, rtn = %#08x", rtn);
        return rtn;
    }

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_Log(AK_LOG_ERROR, "====>>CreateDirRecord->DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
            break;
    }

    memset(data,0xff,sizeof(data));
    for( recNO = 1; recNO <= APPLICATION_MAX_QUANTITY; recNO++ )
    {
        rtn = UpdateRecord(hDeviceHandle,pcFileName,recNO, data,0x10);	//Update by zhoushenshen 20100309
        if (rtn != 0)
        {
            AK_Log(AK_LOG_ERROR, "====>>CreateDirRecord->UpdateRecord() Error,rtn = %#08x", rtn);
            return rtn;
        }
    }

    AK_Log(AK_LOG_DEBUG, "====>>CreateDirRecord Success!");
    return USBKEY_OK;
}

USBKEY_ULONG CreateFileRecord(USBKEY_HANDLE hDeviceHandle,USBKEY_UCHAR ulAccessCondition)
{
    USBKEY_UCHAR response[300];
    USBKEY_UCHAR data[300];
    USBKEY_ULONG rtn,respLen=0,recNO;
    USBKEY_UCHAR p1,p2,lc,le;
    USBKEY_UCHAR pcFileName[2];

    AK_Log(AK_LOG_DEBUG, "====>>CreateFileRecord");

    p1 = 0x00;
    p2 = 0x03;
    lc = 0x07;
    le = 0x00;

    //创建文件
    pcFileName[0] = 0x80;
    pcFileName[1] = 0x01;
    memset(data,0,300);
    memcpy(data,pcFileName,2);
    le = 0x00;
    memcpy(data+2,&le,1);
    le = USBKEY_ALL_PRIV;
    memcpy(data+3,&le,1);
    memcpy(data+4,&ulAccessCondition,1);
    le = USBKEY_FILE_RECORD_LEN;
    memcpy(data+5,&le,1);
    le = FILE_MAX_QUANTITY;
    memcpy(data+6,&le,1);

    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_CreateFile,response,&respLen);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>CreateFileRecord->PackageData() Error, rtn = %#08x", rtn);
        return rtn;
    }

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_Log(AK_LOG_ERROR, "====>>CreateFileRecord->DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
            break;
    }

    //recover in 20130106 金融IC卡初始状态为0，设为-1
    memset(data,0xff,sizeof(data));
    for( recNO = 1; recNO <= FILE_MAX_QUANTITY; recNO++ )
    {
        rtn = UpdateRecord(hDeviceHandle,pcFileName,recNO, data,USBKEY_FILE_RECORD_LEN);
        if (rtn != 0)
        {
            AK_Log(AK_LOG_ERROR, "====>>CreateFileRecord->UpdateRecord() Error, rtn = %#08x", rtn);
            return rtn;
        }
    }

    AK_Log(AK_LOG_DEBUG, "====>>CreateFileRecord Success!");
    return USBKEY_OK;
}

//Update by zhoushenshen 20100317
USBKEY_ULONG CreateKeyRecord(USBKEY_HANDLE hDeviceHandle,USBKEY_UCHAR ulAccessCondition)
{
    //USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    USBKEY_UCHAR data[300];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0;
    USBKEY_UCHAR p1,p2,lc,le;
    USBKEY_UCHAR pcFileName[2];

    AK_Log(AK_LOG_DEBUG, "====>>CreateKeyRecord");

    p1 = 0x00;
    p2 = 0x03;
    lc = 0x07;
    le = 0x00;

    ////////
    //创建文件
    ////////
    pcFileName[0] = 0x80;
    pcFileName[1] = 0x03;
    memset(data,0,300);
    memcpy(data,pcFileName,2);
    le = 0x00;
    memcpy(data+2,&le,1);
    le = USBKEY_ALL_PRIV;
    memcpy(data+3,&le,1);
    memcpy(data+4,&ulAccessCondition,1);
    le = 0x10;
    memcpy(data+5,&le,1);
    le = KEY_MAX_QUANTITY;
    memcpy(data+6,&le,1);

    //Delete by zhoushenshen in 20121107
    /*rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_CreateFile,request,&reqLen);
      if (rtn != 0)
      {
      AK_Log(AK_LOG_ERROR, "====>>CreateKeyRecord->PackageData() Error,rtn = %#08x", rtn);
      return rtn;
      }

      respLen = 300;
      rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
      if (rtn != 0)
      {
      AK_Log(AK_LOG_ERROR, "====>>CreateKeyRecord->DeviceExecCmd() Error,rtn = %#08x", rtn);
      return rtn;
      }*/
    //Del end

    //Add by zhoushenshen in 20121107
    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_CreateFile,response,&respLen);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>CreateKeyRecord->PackageData() Error, rtn = %#08x", rtn);
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
            AK_Log(AK_LOG_ERROR, "====>>CreateKeyRecord->DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
            break;
    }

    /*memset(data,0xff,sizeof(data));
      for( recNO = 1; recNO <= KEY_MAX_QUANTITY; recNO++ )
      {
      rtn = UpdateRecord(hDeviceHandle,"/",pcFileName,recNO, data,0x10);
      if (rtn != 0)
      {
      LogMessage16Val("====>>CreateKeyRecord->UpdateRecord() Error,rtn = 0x",rtn);
      return rtn;
      }
      }*/

    AK_Log(AK_LOG_DEBUG, "====>>CreateKeyRecord Success!");
    return USBKEY_OK;
}

USBKEY_ULONG CreateContainerRecord(USBKEY_HANDLE hDeviceHandle,USBKEY_UCHAR ulAccessCondition)
{
    USBKEY_UCHAR response[300];
    USBKEY_UCHAR data[300];
    USBKEY_ULONG rtn,respLen=0,recNO;
    USBKEY_UCHAR p1,p2,lc,le;
    USBKEY_UCHAR pcFileName[2];

    AK_Log(AK_LOG_DEBUG, "====>>CreateContainerRecord");

    p1 = 0x00;
    p2 = 0x03;
    lc = 0x07;
    le = 0x00;

    //创建文件
    pcFileName[0] = 0x80;
    pcFileName[1] = 0x02;
    memset(data,0,300);
    memcpy(data,pcFileName,2);
    le = 0x00;
    memcpy(data+2,&le,1);
    le = USBKEY_ALL_PRIV;
    memcpy(data+3,&le,1);
    memcpy(data+4,&ulAccessCondition,1);
    le = 0xfe;
    memcpy(data+5,&le,1);
    le = CONTAINER_MAX_QUANTITY;
    memcpy(data+6,&le,1);

    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_CreateFile,response,&respLen);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>CreateContainerRecord->PackageData() Error, rtn = %#08x", rtn);
        return rtn;
    }

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_Log(AK_LOG_ERROR, "====>>CreateContainerRecord->DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
            break;
    }

    //recover in 20130106 金融IC卡初始状态为0，设为-1
    memset(data,0xff,sizeof(data));
    for( recNO = 1; recNO <= CONTAINER_MAX_QUANTITY; recNO++ )
    {
        rtn = UpdateRecord(hDeviceHandle,pcFileName,recNO, data,0xfe);
        if (rtn != 0)
        {
            AK_Log(AK_LOG_ERROR, "====>>CreateContainerRecord->UpdateRecord() Error, rtn = %#08x", rtn);
            return rtn;
        }
    }

    AK_Log(AK_LOG_DEBUG, "====>>CreateContainerRecord Success!");
    return USBKEY_OK;
}

USBKEY_ULONG ReadRecord(USBKEY_HANDLE hDeviceHandle,USBKEY_UCHAR_PTR pcFileID, USBKEY_ULONG recNO, USBKEY_UCHAR_PTR pcRecord, USBKEY_ULONG_PTR pulRecordLen)
{
    USBKEY_UCHAR response[300];
    USBKEY_UCHAR data[300];
    USBKEY_ULONG rtn,respLen=0;
    USBKEY_UCHAR p1,p2,lc,le;

    AK_Log(AK_LOG_DEBUG, "====>>ReadRecord");

    p1 = (USBKEY_UCHAR)recNO;
    p2 = 0x04;
    lc = 0x00;
    le = (USBKEY_UCHAR)(*pulRecordLen);

    memset(data,0,300);

    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_ReadRecord,response,&respLen);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>ReadRecord->PackageData() Error, rtn = %#08x", rtn);
        return rtn;
    }

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_Log(AK_LOG_ERROR, "====>>ReadRecord->DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
            break;
    }

    memcpy(pcRecord,response,respLen-2);
    *pulRecordLen = respLen - 2;

    AK_Log(AK_LOG_DEBUG, "====>>ReadRecord Success!,the record size is %d",respLen-2);
    return USBKEY_OK;
}

// 更新记录
USBKEY_ULONG AppendRecord(
			USBKEY_HANDLE hDeviceHandle, 
			USBKEY_UCHAR_PTR pcFileID,
			USBKEY_UCHAR_PTR pcRecord, 
			USBKEY_ULONG ulRecordLen, 
			USBKEY_ULONG ulType)
{
    USBKEY_UCHAR response[300];
    USBKEY_UCHAR data[300];
    USBKEY_ULONG rtn,respLen=0,recNO;
    USBKEY_UCHAR p1,p2,lc,le;

    AK_Log(AK_LOG_DEBUG, "====>>AppendRecord");

    if (ulType == 0)
        rtn = GetRecNo(hDeviceHandle,pcFileID,&recNO);		//Update by zhoushenshen 20100309
    else if (ulType == 2)
        rtn = GetConRecNo(hDeviceHandle,pcFileID,&recNO);	//Update by zhoushenshen 20100309
    else if (ulType == 3)
        rtn = GetKeyRecNo(hDeviceHandle,pcFileID,&recNO);
    else
        rtn = GetFileRecNo(hDeviceHandle,pcFileID,&recNO);	//Update by zhoushenshen 20100309

    if ( (rtn != 0) || (recNO == 0) )
    {
        if(recNO == 0)
        {
            AK_Log(AK_LOG_ERROR, "====>>AppendRecord->not find record.");
            return USBKEY_RECORD_NOT_FOUND;
        }
        AK_Log(AK_LOG_ERROR, "====>>AppendRecord->GetRecNo() Error,rtn = %#08x", rtn);
        return rtn;
    }

    // 创建文件
    p1 = (USBKEY_UCHAR)recNO;
    p2 = 0x04;

    if (ulType == 2)
        lc = 254;
    else if (ulType == 1)
        lc = USBKEY_FILE_RECORD_LEN;
    else
        lc = 16;
    le = 0x00;

    memset(data,0x00,300);
    memcpy(data,pcRecord,ulRecordLen);

    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_UpdateRecord,response,&respLen);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>AppendRecord->PackageData() Error, rtn = %#08x", rtn);
        return rtn;
    }

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_Log(AK_LOG_ERROR, "====>>AppendRecord->DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
            break;
    }

    AK_Log(AK_LOG_DEBUG, "====>>AppendRecord Success!");
    return USBKEY_OK;
}


USBKEY_ULONG UpdateRecord(
			USBKEY_HANDLE hDeviceHandle, 
			USBKEY_UCHAR_PTR pcFileID,
			USBKEY_ULONG recNO, 
			USBKEY_UCHAR_PTR pcRecord, 
			USBKEY_ULONG ulRecordLen)
{
    USBKEY_UCHAR response[300];
    USBKEY_UCHAR data[300];
    USBKEY_ULONG rtn,respLen=0;
    USBKEY_UCHAR p1,p2,lc,le;

    AK_Log(AK_LOG_DEBUG, "====>>UpdateRecord");

    p1 = (USBKEY_UCHAR)recNO;
    p2 = 0x04;
    lc = (USBKEY_UCHAR)ulRecordLen;
    le = 0x00;

    //创建文件
    memset(data,0x00,300);
    memcpy(data,pcRecord,ulRecordLen);

    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_UpdateRecord,response,&respLen);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>UpdateRecord->PackageData() Error, rtn = %#08x", rtn);
        return rtn;
    }
    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_Log(AK_LOG_ERROR, "====>>UpdateRecord->DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
            break;
    }

    AK_Log(AK_LOG_DEBUG, "====>>UpdateRecord Success!");
    return USBKEY_OK;
}

USBKEY_ULONG DeleRecord(USBKEY_HANDLE hDeviceHandle, USBKEY_UCHAR_PTR pcFileID, USBKEY_UCHAR_PTR pcRecordID, USBKEY_ULONG ulRecordLen,  USBKEY_ULONG ulType)
{
    USBKEY_UCHAR response[300];
    USBKEY_UCHAR data[300];
    USBKEY_ULONG rtn,respLen=0,recNO,reclen;
    USBKEY_UCHAR p1,p2,lc,le;
    USBKEY_ULONG max;

    AK_Log(AK_LOG_DEBUG, "====>>DeleRecord");

    if(ulType == 0)
        max = APPLICATION_MAX_QUANTITY;
    else if (ulType == 2)
        max = CONTAINER_MAX_QUANTITY;
    else
        max = FILE_MAX_QUANTITY;

    //查找相应的记录号
    for (recNO = 1;recNO <= max; recNO++)
    {
        if (ulType == 2)
            reclen = 254;
        else if(ulType == 1)
            reclen = USBKEY_FILE_RECORD_LEN;
        else
            reclen = 16;
        rtn = ReadRecord(hDeviceHandle,pcFileID,recNO,data,&reclen);	//Update by zhoushenshen 20100309
        if (rtn != 0)
        {
            AK_Log(AK_LOG_ERROR, "====>>DeleRecord->ReadRecord() Error,rtn = %#08x", rtn);
            return rtn;
        }
        if(ulType == 1)	//文件记录 格式：文件ID+读权限+写权限+大小+文件名
        {
            if ( (reclen >= ulRecordLen+8) && (memcmp(data+8,pcRecordID,ulRecordLen) == 0) )
                break;
        }
        else if ( (reclen >= ulRecordLen) && (memcmp(data,pcRecordID,ulRecordLen) == 0) )
        {
            break;
        }
    }

    p1 = (USBKEY_UCHAR)recNO;
    p2 = 0x04;
    if (ulType == 2)
        lc = 254;
    else if(ulType == 1)
        lc = USBKEY_FILE_RECORD_LEN;
    else
        lc = 16;
    le = 0x00;

    // 删除文件记录
    memset(data,0xff,sizeof(data));

    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_UpdateRecord,response,&respLen);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>DeleRecord->PackageData() Error, rtn = %#08x", rtn);
        return rtn;
    }
    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_Log(AK_LOG_ERROR, "====>>DeleRecord->DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
            break;
    }

    AK_Log(AK_LOG_DEBUG, "====>>DeleRecord Success!");
    return USBKEY_OK;
}

USBKEY_ULONG GetRecNo(USBKEY_HANDLE hDeviceHandle, USBKEY_UCHAR_PTR pcFileID, USBKEY_ULONG_PTR plRecordNum)
{
    USBKEY_UCHAR data[300];
    USBKEY_ULONG rtn,recNO,recLen;
    USBKEY_UCHAR msg[300];

    AK_Log(AK_LOG_DEBUG, "====>>GetRecNo");

    memset(data,0xff,sizeof(data));
    for (recNO = 1; recNO <= APPLICATION_MAX_QUANTITY ; recNO ++)
    {
        memset(msg,0xff,sizeof(msg));
        recLen = 16;
        rtn = ReadRecord(hDeviceHandle,pcFileID,recNO,msg,&recLen);	//Update by zhoushenshen 20100309
        if (rtn != 0)
        {
            AK_Log(AK_LOG_ERROR, "====>>GetRecNo->ReadRecord() Error,rtn = %#08x", rtn);
            return rtn;
        }
        if (memcmp(data,msg,recLen) == 0)
        {
            *plRecordNum = recNO;
            return USBKEY_OK;
        }
    }
    *plRecordNum = 0;

    AK_Log(AK_LOG_DEBUG, "====>>GetRecNo Success!");
    return USBKEY_OK;
}

//Update by zhoushenshen 20100309
USBKEY_ULONG GetFileRecNo(USBKEY_HANDLE hDeviceHandle, USBKEY_UCHAR_PTR pcFileID, USBKEY_ULONG_PTR plRecordNum)
{
    USBKEY_UCHAR data[300];
    USBKEY_ULONG rtn,recNO,recLen;
    USBKEY_UCHAR msg[300];

    AK_Log(AK_LOG_DEBUG, "====>>GetFileRecNo");

    memset(data,0xff,sizeof(data));
    for (recNO = 1; recNO <= FILE_MAX_QUANTITY ; recNO ++)
    {
        memset(msg,0xff,sizeof(msg));
        recLen = USBKEY_FILE_RECORD_LEN;
        rtn = ReadRecord(hDeviceHandle,pcFileID,recNO,msg,&recLen);		//Update by zhoushenshen 20100309
        if (rtn != 0)
        {
            AK_Log(AK_LOG_ERROR, "====>>GetFileRecNo->ReadRecord() Error,rtn = %#08x", rtn);
            return rtn;
        }
        if (memcmp(data,msg,recLen) == 0)
        {
            *plRecordNum = recNO;
            return USBKEY_OK;
        }
    }
    *plRecordNum = 0;

    AK_Log(AK_LOG_DEBUG, "====>>GetFileRecNo Success!");
    return USBKEY_OK;
}

//Update by zhoushenshen 20100309
USBKEY_ULONG GetConRecNo(USBKEY_HANDLE hDeviceHandle, USBKEY_UCHAR_PTR pcFileID, USBKEY_ULONG_PTR plRecordNum)
{
    USBKEY_UCHAR data[300];
    USBKEY_ULONG rtn,recNO,recLen;
    USBKEY_UCHAR msg[300];

    AK_Log(AK_LOG_DEBUG, "====>>GetConRecNo");

    memset(data,0xff,sizeof(data));
    for (recNO = 1; recNO <= CONTAINER_MAX_QUANTITY; recNO ++)
    {
        memset(msg,0xff,sizeof(msg));
        recLen = 254;
        rtn = ReadRecord(hDeviceHandle,pcFileID,recNO,msg,&recLen);		//Update by zhoushenshen 20100309
        if (rtn != 0)
        {
            AK_Log(AK_LOG_ERROR, "====>>GetConRecNo->ReadRecord() Error,rtn = %#08x", rtn);
            return rtn;
        }
        if (memcmp(data,msg,recLen) == 0)
        {
            *plRecordNum = recNO;
            return USBKEY_OK;
        }
    }
    *plRecordNum = 0;

    AK_Log(AK_LOG_DEBUG, "====>>GetConRecNo Success!");
    return USBKEY_OK;
}

//Update by zhoushenshen 20100309
USBKEY_ULONG GetKeyRecNo(USBKEY_HANDLE hDeviceHandle, USBKEY_UCHAR_PTR pcFileID, USBKEY_ULONG_PTR plRecordNum)
{
    USBKEY_UCHAR data[300];
    USBKEY_ULONG rtn,recNO,recLen;
    USBKEY_UCHAR msg[300];

    AK_Log(AK_LOG_DEBUG, "====>>GetKeyRecNo");

    memset(data,0xff,sizeof(data));
    for (recNO = 1; recNO <= KEY_MAX_QUANTITY; recNO++)
    {
        memset(msg,0xff,sizeof(msg));
        recLen = 16;
        rtn = ReadRecord(hDeviceHandle,pcFileID,recNO,msg,&recLen);		//Update by zhoushenshen 20100309
        if (rtn != 0)
        {
            AK_Log(AK_LOG_ERROR, "====>>GetKeyRecNo->ReadRecord() Error,rtn = %#08x", rtn);
            return rtn;
        }
        if (memcmp(data,msg,recLen) == 0)
        {
            *plRecordNum = recNO;
            return USBKEY_OK;
        }
    }
    *plRecordNum = 0;

    AK_Log(AK_LOG_DEBUG, "====>>GetKeyRecNo Success!");
    return USBKEY_OK;
}

//Update by zhoushenshen 20100309
USBKEY_ULONG AddFileRecord(USBKEY_HANDLE hDeviceHandle, USBKEY_UCHAR_PTR pcFileID)
{
    USBKEY_ULONG rtn;
    USBKEY_UCHAR fileID[2];

    AK_Log(AK_LOG_DEBUG, "====>>AddFileRecord");

    fileID[0] = 0x80;
    fileID[1] = 0x01;
    rtn = SelectFile(hDeviceHandle,NULL,fileID);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>AddFileRecord->SelectFile() Error,rtn = %#08x", rtn);
        return rtn;
    }
    rtn = AppendRecord(hDeviceHandle,(USBKEY_UCHAR_PTR)&fileID,pcFileID,USBKEY_FILE_RECORD_LEN, 1);		//Update by zhoushenshen 20100309
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>AddFileRecord->AppendRecord() Error,rtn = %#08x", rtn);
        return rtn;
    }

    AK_Log(AK_LOG_DEBUG, "====>>AddFileRecord Success!");
    return USBKEY_OK;
}
//Update by zhoushenshen 20100317
USBKEY_ULONG DeleFileRecord(USBKEY_HANDLE hDeviceHandle, USBKEY_UCHAR_PTR pcFileName)
{
    USBKEY_ULONG rtn;
    USBKEY_UCHAR fileID[2];

    AK_Log(AK_LOG_DEBUG, "====>>DeleFileRecord");

    fileID[0] = 0x80;
    fileID[1] = 0x01;
    rtn = SelectFile(hDeviceHandle,NULL,fileID);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>DeleFileRecord->SelectFile() Error,rtn = %#08x", rtn);
        return rtn;
    }
    rtn = DeleRecord(hDeviceHandle,(USBKEY_UCHAR_PTR)&fileID,pcFileName,(USBKEY_ULONG)strlen((char *)pcFileName),1);	//Update by zhoushenshen 20100309
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>DeleFileRecord->DeleRecord() Error,rtn = %#08x", rtn);
        return rtn;
    }

    AK_Log(AK_LOG_DEBUG, "====>>DeleFileRecord Success!");
    return USBKEY_OK;
}

//Update by zhoushenshen 20100309
USBKEY_ULONG GetFileInfo(USBKEY_HANDLE hDeviceHandle, USBKEY_UCHAR_PTR pcFileName, USBKEY_UCHAR_PTR pcFileInfo)
{
    USBKEY_ULONG rtn,recNO,reclen;
    USBKEY_UCHAR fileID[2],data[USBKEY_FILE_RECORD_LEN];

    AK_Log(AK_LOG_DEBUG, "====>>GetFileInfo");

    //文件ID: fe+记录号
    //文件信息格式:文件ID+读权限+写权限+大小+文件名
    fileID[0] = 0x80;
    fileID[1] = 0x01;
    rtn = SelectFile(hDeviceHandle,NULL,fileID);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>GetFileInfo->SelectFile() Error,rtn = %#08x", rtn);
        return rtn;
    }
    //查找相应的记录号
    for (recNO = 1;recNO <= FILE_MAX_QUANTITY; recNO++)
    {
        reclen = USBKEY_FILE_RECORD_LEN;
        rtn = ReadRecord(hDeviceHandle,NULL,recNO,data,&reclen);	//Update by zhoushenshen 20100309
        if (rtn != 0)
        {
            AK_Log(AK_LOG_ERROR, "====>>GetFileInfo->ReadRecord() Error,rtn = %#08x", rtn);
            return rtn;
        }
        if ( (reclen > 8) && (strcmp((char *)data+8,(char *)pcFileName) == 0) )
        {
            memcpy(pcFileInfo,data,USBKEY_FILE_RECORD_LEN);
            return USBKEY_OK;
        }
    }

    AK_Log(AK_LOG_DEBUG, "====>>GetFileInfo Success!");
    return USBKEY_NO_SUCH_RECORD;
}

//ADD by zhoushenshen 20091119
USBKEY_ULONG SearchFileRecNo(USBKEY_HANDLE hDeviceHandle,USBKEY_UCHAR_PTR pcFileName, USBKEY_ULONG_PTR plRecordNum)
{
    USBKEY_UCHAR data[300];
    USBKEY_ULONG rtn,recNO,recLen;
    USBKEY_UCHAR msg[300],fileID[2];

    AK_Log(AK_LOG_DEBUG, "====>>SearchFileRecNo");

    fileID[0] = 0x80;
    fileID[1] = 0x01;
    rtn = SelectFile(hDeviceHandle,NULL,fileID);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>SearchFileRecNo->SelectFile() Error,rtn = %#08x", rtn);
        return rtn;
    }

    memset(data,0,sizeof(data));
    memcpy(data,pcFileName,strlen((char *)pcFileName));
    for (recNO = 1; recNO <= FILE_MAX_QUANTITY; recNO ++)
    {
        memset(msg,0xff,sizeof(msg));
        recLen = USBKEY_FILE_RECORD_LEN;
        rtn = ReadRecord(hDeviceHandle,NULL,recNO,msg,&recLen);		//Update by zhoushenshen 20100309
        if (rtn != 0)
        {
            AK_Log(AK_LOG_ERROR, "====>>SearchFileRecNo->ReadRecord() Error,rtn = %#08x", rtn);
            return rtn;
        }
        if (memcmp(data,msg+8,recLen-8) == 0)
        {
            *plRecordNum = recNO;
            return USBKEY_OK;
        }
    }
    *plRecordNum = 0;

    AK_Log(AK_LOG_DEBUG, "====>>SearchFileRecNo Success!");
    return USBKEY_OK;
}

//Update by zhoushenshen 20100317
USBKEY_ULONG AddKeyRecord(USBKEY_HANDLE hDeviceHandle,USBKEY_UCHAR_PTR pcKeyID)
{
    USBKEY_ULONG rtn;
    USBKEY_UCHAR fileID[2];

    AK_Log(AK_LOG_DEBUG, "====>>AddKeyRecord");

    fileID[0] = 0x80;
    fileID[1] = 0x03;
    rtn = SelectFile(hDeviceHandle,NULL,fileID);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>AddKeyRecord->SelectFile() Error,rtn = %#08x", rtn);
        return rtn;
    }
    rtn = AppendRecord(hDeviceHandle,(USBKEY_UCHAR_PTR)&fileID,pcKeyID,2,3);	//Update by zhoushenshen 20100309
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>AddKeyRecord->AppendRecord() Error,rtn = %#08x", rtn);
        return rtn;
    }

    AK_Log(AK_LOG_DEBUG, "====>>AddKeyRecord Success!");
    return USBKEY_OK;
}

//Update by zhoushenshen 20100317
USBKEY_ULONG DeleKeyRecord(USBKEY_HANDLE hDeviceHandle, USBKEY_UCHAR_PTR pcKeyID)
{
    USBKEY_ULONG rtn;
    USBKEY_UCHAR fileID[2];

    AK_Log(AK_LOG_DEBUG, "====>>DeleKeyRecord");

    fileID[0] = 0x80;
    fileID[1] = 0x03;
    rtn = SelectFile(hDeviceHandle,NULL,fileID);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>DeleKeyRecord->SelectFile() Error,rtn = %#08x", rtn);
        return rtn;
    }
    rtn = DeleRecord(hDeviceHandle,(USBKEY_UCHAR_PTR)&fileID,pcKeyID,1,0);	//Update by zhoushenshen 20100309
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>DeleKeyRecord->DeleRecord() Error,rtn = %#08x", rtn);
        return rtn;
    }

    AK_Log(AK_LOG_DEBUG, "====>>DeleKeyRecord Success!");
    return USBKEY_OK;
}

USBKEY_ULONG AddDirRecord(USBKEY_HANDLE hDeviceHandle,USBKEY_UCHAR_PTR pcDirName)
{
    USBKEY_ULONG rtn;
    USBKEY_UCHAR fileID[2];

    AK_Log(AK_LOG_DEBUG, "====>>AddDirRecord");

    fileID[0] = 0x80;
    fileID[1] = 0x00;
    rtn = SelectFile(hDeviceHandle,NULL,fileID);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>AddDirRecord->SelectFile() Error,rtn = %#08x", rtn);
        return rtn;
    }
    rtn = AppendRecord(hDeviceHandle,(USBKEY_UCHAR_PTR)&fileID,pcDirName,(USBKEY_ULONG)strlen((char *)pcDirName),0);	//Update by zhoushenshen 20100309
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>AddDirRecord->AppendRecord() Error,rtn = %#08x", rtn);
        return rtn;
    }

    AK_Log(AK_LOG_DEBUG, "====>>AddDirRecord Success!");
    return USBKEY_OK;
}

USBKEY_ULONG DeleDirRecord(USBKEY_HANDLE hDeviceHandle,USBKEY_UCHAR_PTR pcDirName)
{
    USBKEY_ULONG rtn;
    USBKEY_UCHAR fileID[2];

    AK_Log(AK_LOG_DEBUG, "====>>DeleDirRecord");

    fileID[0] = 0x80;
    fileID[1] = 0x00;
    rtn = SelectFile(hDeviceHandle,NULL,fileID);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>DeleDirRecord->SelectFile() Error,rtn = %#08x", rtn);
        return rtn;
    }
    rtn = DeleRecord(hDeviceHandle,(USBKEY_UCHAR_PTR)&fileID,pcDirName,(USBKEY_ULONG)strlen((char *)pcDirName),0);	//Update by zhoushenshen 20100309
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>DeleDirRecord->DeleRecord() Error,rtn = %#08x", rtn);
        return rtn;
    }

    AK_Log(AK_LOG_DEBUG, "====>>DeleDirRecord Success!");
    return USBKEY_OK;
}

//Update by zhoushenshen 20100309
USBKEY_ULONG AddContainerRecord(USBKEY_HANDLE hDeviceHandle, USBKEY_UCHAR_PTR pcContainerName)
{
    USBKEY_ULONG rtn;
    USBKEY_UCHAR fileID[2];

    AK_Log(AK_LOG_DEBUG, "====>>AddContainerRecord");

    fileID[0] = 0x80;
    fileID[1] = 0x02;
    rtn = SelectFile(hDeviceHandle,NULL,fileID);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>AddContainerRecord->SelectFile() Error,rtn = %#08x", rtn);
        return rtn;
    }
    rtn = AppendRecord(hDeviceHandle,(USBKEY_UCHAR_PTR)&fileID,pcContainerName,(USBKEY_ULONG)strlen((char *)pcContainerName),2);	//Update by zhoushenshen 20100309
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>AddContainerRecord->AppendRecord() Error,rtn = %#08x", rtn);
        return rtn;
    }

    AK_Log(AK_LOG_DEBUG, "====>>AddContainerRecord Success!");
    return USBKEY_OK;
}

//Update by zhoushenshen 20100309
USBKEY_ULONG DeleContainerRecord(USBKEY_HANDLE hDeviceHandle, USBKEY_UCHAR_PTR pcContainerName)
{
    USBKEY_ULONG rtn;
    USBKEY_UCHAR fileID[2];

    AK_Log(AK_LOG_DEBUG, "====>>DeleContainerRecord");

    fileID[0] = 0x80;
    fileID[1] = 0x02;
    rtn = SelectFile(hDeviceHandle,NULL,fileID);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>DeleContainerRecord->SelectFile() Error,rtn = %#08x", rtn);
        return rtn;
    }
    rtn = DeleRecord(hDeviceHandle,(USBKEY_UCHAR_PTR)&fileID,pcContainerName,(USBKEY_ULONG)strlen((char *)pcContainerName),2);	//Update by zhoushenshen 20100309
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>DeleContainerRecord->DeleRecord() Error,rtn = %#08x", rtn);
        return rtn;
    }

    AK_Log(AK_LOG_DEBUG, "====>>DeleContainerRecord Success!");
    return USBKEY_OK;
}

//Update by zhoushenshen 20100309
USBKEY_ULONG AddContainerKey(USBKEY_HANDLE hDeviceHandle, USBKEY_ULONG ulContainerRecNO,USBKEY_ULONG ulKeyUsage,USBKEY_ULONG ulBitLen)
{
    USBKEY_ULONG rtn;
    USBKEY_UCHAR fileID[16],containerFile[2];

    AK_Log(AK_LOG_DEBUG, "====>>AddContainerKey");

    containerFile[0] = 0xef;
    containerFile[1] = (USBKEY_UCHAR)ulContainerRecNO;
    rtn = SelectFile(hDeviceHandle,NULL,containerFile);		//Update by zhoushenshen 20100309
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>AddContainerKey->SelectFile() Error,rtn = %#08x", rtn);
        return rtn;
    }

    memset(fileID,0,sizeof(fileID));
    switch(ulKeyUsage)
    {
        case KEY_EXCHANGE:
            memcpy(fileID,&ulContainerRecNO,1);
            fileID[1] = 0x01;
            memcpy(fileID+2,&ulBitLen,4);
            rtn = UpdateRecord(hDeviceHandle,containerFile,1, fileID,0x10);	//Update by zhoushenshen 20100309
            if (rtn != 0)
            {
                AK_Log(AK_LOG_ERROR, "====>>AddContainerKey->UpdateRecord() 1 Error,rtn = %#08x", rtn);
                return rtn;
            }
            fileID[1] = 0x02;
            memcpy(fileID+2,&ulBitLen,4);
            rtn = UpdateRecord(hDeviceHandle,containerFile,2, fileID,0x10);	//Update by zhoushenshen 20100309
            if (rtn != 0)
            {
                AK_Log(AK_LOG_ERROR, "====>>AddContainerKey->UpdateRecord() 2 Error,rtn = %#08x", rtn);
                return rtn;
            }
            break;
        case KEY_SIGNATURE:
            memcpy(fileID,&ulContainerRecNO,1);
            fileID[1] = 0x04;
            memcpy(fileID+2,&ulBitLen,4);
            rtn = UpdateRecord(hDeviceHandle,containerFile,4, fileID,0x10);	//Update by zhoushenshen 20100309
            if (rtn != 0)
            {
                AK_Log(AK_LOG_ERROR, "====>>AddContainerKey->UpdateRecord() 4 Error,rtn = %#08x", rtn);
                return rtn;
            }
            fileID[1] = 0x05;
            memcpy(fileID+2,&ulBitLen,4);
            rtn = UpdateRecord(hDeviceHandle,containerFile,5, fileID,0x10);	//Update by zhoushenshen 20100309
            if (rtn != 0)
            {
                AK_Log(AK_LOG_ERROR, "====>>AddContainerKey->UpdateRecord() 5 Error,rtn = %#08x", rtn);
                return rtn;
            }
            break;
        default:
            break;
    }

    AK_Log(AK_LOG_DEBUG, "====>>AddContainerKey Success!");
    return USBKEY_OK;
}

//Update by zhoushenshen 20100310
USBKEY_ULONG AddContainerKeySep(USBKEY_HANDLE hDeviceHandle,USBKEY_ULONG ulContainerRecNO,USBKEY_ULONG ulKeyUsage, USBKEY_ULONG ulKeyType,USBKEY_ULONG ulBitLen)
{
    USBKEY_ULONG rtn;
    USBKEY_UCHAR fileID[16],containerFile[2];

    AK_Log(AK_LOG_DEBUG, "====>>AddContainerKeySep");

    containerFile[0] = 0xef;
    containerFile[1] = (USBKEY_UCHAR)ulContainerRecNO;
    rtn = SelectFile(hDeviceHandle,NULL,containerFile);		//Update by zhoushenshen 20100310
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>AddContainerKeySep->SelectFile() Error,rtn = %#08x", rtn);
        return rtn;
    }

    memset(fileID,0,sizeof(fileID));
    switch(ulKeyUsage)
    {
        case KEY_EXCHANGE:
            memcpy(fileID,&ulContainerRecNO,1);
            if (ulKeyType == USBKEY_RSA_PUBLICKEY)
            {
                fileID[1] = 0x01;
                memcpy(fileID+2,&ulBitLen,4);
                rtn = UpdateRecord(hDeviceHandle,containerFile,1, fileID,0x10);	//Update by zhoushenshen 20100309
                if (rtn != 0)
                {
                    AK_Log(AK_LOG_ERROR, "====>>AddContainerKeySep->UpdateRecord() 1 Error,rtn = %#08x", rtn);
                    return rtn;
                }
            }
            else if (ulKeyType == USBKEY_RSA_PRIVATEKEY)
            {
                fileID[1] = 0x02;
                memcpy(fileID+2,&ulBitLen,4);
                rtn = UpdateRecord(hDeviceHandle,containerFile,2, fileID,0x10);		//Update by zhoushenshen 20100309
                if (rtn != 0)
                {
                    AK_Log(AK_LOG_ERROR, "====>>AddContainerKeySep->UpdateRecord() 2 Error,rtn = %#08x", rtn);
                    return rtn;
                }
            }
            break;
        case KEY_SIGNATURE:
            memcpy(fileID,&ulContainerRecNO,1);
            if (ulKeyType == USBKEY_RSA_PUBLICKEY)
            {
                fileID[1] = 0x04;
                memcpy(fileID+2,&ulBitLen,4);
                rtn = UpdateRecord(hDeviceHandle,containerFile,4, fileID,0x10);		//Update by zhoushenshen 20100309
                if (rtn != 0)
                {
                    AK_Log(AK_LOG_ERROR, "====>>AddContainerKeySep->UpdateRecord() 4 Error,rtn = %#08x", rtn);
                    return rtn;
                }
            }
            else if (ulKeyType == USBKEY_RSA_PRIVATEKEY)
            {
                fileID[1] = 0x05;
                memcpy(fileID+2,&ulBitLen,4);
                rtn = UpdateRecord(hDeviceHandle,containerFile,5, fileID,0x10);		//Update by zhoushenshen 20100309
                if (rtn != 0)
                {
                    AK_Log(AK_LOG_ERROR, "====>>AddContainerKeySep->UpdateRecord() 5 Error,rtn = %#08x", rtn);
                    return rtn;
                }
            }
            break;
        default:
            break;
    }

    AK_Log(AK_LOG_DEBUG, "====>>AddContainerKeySep Success!");
    return USBKEY_OK;
}


//Add by zhoushenshen 20110118
USBKEY_ULONG ReadContainerKeyRec(USBKEY_HANDLE hDeviceHandle,USBKEY_ULONG ulContainerRecNO,USBKEY_ULONG ulKeyUsage, USBKEY_ULONG ulKeyType,USBKEY_UCHAR_PTR pKeyRec,USBKEY_ULONG_PTR pulRecLen)
{
    USBKEY_ULONG rtn;
    USBKEY_UCHAR containerFile[2];
    USBKEY_ULONG ulRecLen = 16;

    AK_Log(AK_LOG_DEBUG, "====>>ReadContainerKeyRec");

    if(*pulRecLen < 16)
    {
        AK_Log(AK_LOG_ERROR, "====>>ReadContainerKeyRec->pulRecLen not enough,rtn = %#08x", USBKEY_BUFFER_TOO_SHORT);
        return USBKEY_BUFFER_TOO_SHORT;
    }
    *pulRecLen = 16;


    containerFile[0] = 0xef;
    containerFile[1] = (USBKEY_UCHAR)ulContainerRecNO;
    rtn = SelectFile(hDeviceHandle,NULL,containerFile);	
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>ReadContainerKeyRec->SelectFile() Error,rtn = %#08x", rtn);
        return rtn;
    }

    switch(ulKeyUsage)
    {
        case KEY_EXCHANGE:
            if (ulKeyType == USBKEY_RSA_PUBLICKEY)
            {
                rtn = ReadRecord(hDeviceHandle,containerFile,1, pKeyRec,pulRecLen);
                if (rtn != 0)
                {
                    AK_Log(AK_LOG_ERROR, "====>>ReadContainerKeyRec->ReadRecord() 1 Error,rtn = %#08x", rtn);
                    return rtn;
                }
            }
            else if (ulKeyType == USBKEY_RSA_PRIVATEKEY)
            {
                rtn = ReadRecord(hDeviceHandle,containerFile,2, pKeyRec,pulRecLen);
                if (rtn != 0)
                {
                    AK_Log(AK_LOG_ERROR, "====>>ReadContainerKeyRec->ReadRecord() 2 Error,rtn = %#08x", rtn);
                    return rtn;
                }
            }
            //fileID[1] = 0x02;
            break;
        case KEY_SIGNATURE:
            if (ulKeyType == USBKEY_RSA_PUBLICKEY)
            {

                rtn = ReadRecord(hDeviceHandle,containerFile,4, pKeyRec,pulRecLen);
                if (rtn != 0)
                {
                    AK_Log(AK_LOG_ERROR, "====>>ReadContainerKeyRec->ReadRecord() 4 Error,rtn = %#08x", rtn);
                    return rtn;
                }
            }
            else if (ulKeyType == USBKEY_RSA_PRIVATEKEY)
            {
                rtn = ReadRecord(hDeviceHandle,containerFile,5, pKeyRec,pulRecLen);
                if (rtn != 0)
                {
                    AK_Log(AK_LOG_ERROR, "====>>ReadContainerKeyRec->ReadRecord() 5 Error,rtn = %#08x", rtn);
                    return rtn;
                }
            }
            break;
        default:
            break;
    }

    AK_Log(AK_LOG_DEBUG, "====>>ReadContainerKeyRec Success!");
    return USBKEY_OK;
}
//Add end

USBKEY_ULONG DeleteContainerKey(USBKEY_HANDLE hDeviceHandle,USBKEY_ULONG ulContainerRecNO,USBKEY_ULONG ulKeyUsage, USBKEY_ULONG ulKeyType)
{
    USBKEY_ULONG rtn;
    USBKEY_UCHAR fileID[16],containerFile[2];

    AK_Log(AK_LOG_DEBUG, "====>>DeleteContainerKey");

    containerFile[0] = 0xef;
    containerFile[1] = (USBKEY_UCHAR)ulContainerRecNO;
    rtn = SelectFile(hDeviceHandle,NULL,containerFile);		//Update by zhoushenshen 20100317
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>DeleteContainerKey->SelectFile() Error,rtn = %#08x", rtn);
        return rtn;
    }

    memset(fileID,0xff,sizeof(fileID));
    switch(ulKeyUsage)
    {
        case KEY_EXCHANGE:
            if (ulKeyType == USBKEY_RSA_PUBLICKEY)
            {
                rtn = UpdateRecord(hDeviceHandle,containerFile,1, fileID,0x10);		//Update by zhoushenshen 20100309
                if (rtn != 0)
                {
                    AK_Log(AK_LOG_ERROR, "====>>DeleteContainerKey->UpdateRecord() 1 Error,rtn = %#08x", rtn);
                    return rtn;
                }
            }
            else if (ulKeyType == USBKEY_RSA_PRIVATEKEY)
            {
                rtn = UpdateRecord(hDeviceHandle,containerFile,2, fileID,0x10);		//Update by zhoushenshen 20100309
                if (rtn != 0)
                {
                    AK_Log(AK_LOG_ERROR, "====>>DeleteContainerKey->UpdateRecord() 2 Error,rtn = %#08x", rtn);
                    return rtn;
                }
            }
            //fileID[1] = 0x02;
            break;
        case KEY_SIGNATURE:
            if (ulKeyType == USBKEY_RSA_PUBLICKEY)
            {

                rtn = UpdateRecord(hDeviceHandle,containerFile,4, fileID,0x10);		//Update by zhoushenshen 20100309
                if (rtn != 0)
                {
                    AK_Log(AK_LOG_ERROR, "====>>DeleteContainerKey->UpdateRecord() 4 Error,rtn = %#08x", rtn);
                    return rtn;
                }
            }
            else if (ulKeyType == USBKEY_RSA_PRIVATEKEY)
            {
                rtn = UpdateRecord(hDeviceHandle,containerFile,5, fileID,0x10);		//Update by zhoushenshen 20100309
                if (rtn != 0)
                {
                    AK_Log(AK_LOG_ERROR, "====>>DeleteContainerKey->UpdateRecord() 5 Error,rtn = %#08x", rtn);
                    return rtn;
                }
            }
            break;
        default:
            break;
    }

    AK_Log(AK_LOG_DEBUG, "====>>DeleteContainerKey Success!");
    return USBKEY_OK;
}

//Update by zhoushenshen 20100317
USBKEY_ULONG AddContainerCert(USBKEY_HANDLE hDeviceHandle,USBKEY_ULONG ulContainerRecNO,USBKEY_ULONG ulKeyUsage)
{
    USBKEY_ULONG rtn;
    USBKEY_UCHAR fileID[16],containerFile[2];

    AK_Log(AK_LOG_DEBUG, "====>>AddContainerCert");

    containerFile[0] = 0xef;
    containerFile[1] = (USBKEY_UCHAR)ulContainerRecNO;
    rtn = SelectFile(hDeviceHandle,NULL,containerFile);	//Update by zhoushenshen 20100317
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>AddContainerCert->SelectFile() Error,rtn = %#08x", rtn);
        return rtn;
    }

    memset(fileID,0,sizeof(fileID));
    switch(ulKeyUsage)
    {
        case KEY_EXCHANGE:
            memcpy(fileID,&ulContainerRecNO,1);
            fileID[1] = 0x03;
            rtn = UpdateRecord(hDeviceHandle,containerFile,3, fileID,0x10);		//Update by zhoushenshen 20100309
            if (rtn != 0)
            {
                AK_Log(AK_LOG_ERROR, "====>>AddContainerCert->UpdateRecord() 3 Error,rtn = %#08x", rtn);
                return rtn;
            }
            break;
        case KEY_SIGNATURE:
            memcpy(fileID,&ulContainerRecNO,1);
            fileID[1] = 0x06;
            rtn = UpdateRecord(hDeviceHandle,containerFile,6, fileID,0x10);		//Update by zhoushenshen 20100309
            if (rtn != 0)
            {
                AK_Log(AK_LOG_ERROR, "====>>AddContainerCert->UpdateRecord() 6 Error,rtn = %#08x", rtn);
                return rtn;
            }
            break;
        default:
            break;
    }

    AK_Log(AK_LOG_DEBUG, "====>>AddContainerCert Success!");
    return USBKEY_OK;
}

//Update by zhoushenshen 20100309
USBKEY_ULONG DeleteContainerCert(USBKEY_HANDLE hDeviceHandle, USBKEY_ULONG ulContainerRecNO,USBKEY_ULONG ulKeyUsage)
{
    USBKEY_ULONG rtn;
    USBKEY_UCHAR fileID[16],containerFile[2];

    AK_Log(AK_LOG_DEBUG, "====>>DeleteContainerCert");

    containerFile[0] = 0xef;
    containerFile[1] = (USBKEY_UCHAR)ulContainerRecNO;
    rtn = SelectFile(hDeviceHandle,NULL,containerFile);	//Update by zhoushenshen 20100309
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>DeleteContainerCert->SelectFile() Error,rtn = %#08x", rtn);
        return rtn;
    }

    memset(fileID,0xff,sizeof(fileID));
    switch(ulKeyUsage)
    {
        case KEY_EXCHANGE:
            //memcpy(fileID,&ulContainerRecNO,1);
            //fileID[1] = 0x03;
            rtn = UpdateRecord(hDeviceHandle,containerFile,3, fileID,0x10);		//Update by zhoushenshen 20100309
            if (rtn != 0)
            {
                AK_Log(AK_LOG_ERROR, "====>>DeleteContainerCert->UpdateRecord() 3 Error,rtn = %#08x", rtn);
                return rtn;
            }
            break;
        case KEY_SIGNATURE:
            //memcpy(fileID,&ulContainerRecNO,1);
            //fileID[1] = 0x06;
            rtn = UpdateRecord(hDeviceHandle,containerFile,6, fileID,0x10);		//Update by zhoushenshen 20100309
            if (rtn != 0)
            {
                AK_Log(AK_LOG_ERROR, "====>>DeleteContainerCert->UpdateRecord() 6 Error,rtn = %#08x", rtn);
                return rtn;
            }
            break;
        default:
            break;
    }

    AK_Log(AK_LOG_DEBUG, "====>>DeleteContainerCert Success!");
    return USBKEY_OK;
}

USBKEY_ULONG DeleteSymKey(USBKEY_HANDLE hDeviceHandle, USBKEY_UCHAR ucSymKeyID)
{
    //USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0;
    USBKEY_UCHAR data[32];
    USBKEY_UCHAR p1,p2,lc,le;
    USBKEY_UCHAR keyAttr[6];	//密钥属性

    AK_Log(AK_LOG_DEBUG, "====>>DeleteSymKey");

    p1 = 0x00;
    p2 = 0x00;
    lc = 24+6;	//密钥长度，取最大值
    le = 0x00;

    //设置密钥属性：密钥标识（1字节）、密钥类型（1字节）、算法标识（1字节）、控制标志（1字节）、使用权限（1字节）、更改权限（1字节）
    keyAttr[0] = ucSymKeyID;			//密钥标识
    keyAttr[1] = 7;		//密钥类型：卡内固定密钥
    keyAttr[2] = ALG_TYPE_3DES_3KEY;	//算法标识
    keyAttr[3] = 0;			//控制标志: 1，密钥不可以导出；0，密钥可以导出
    keyAttr[4] = USBKEY_ALL_PRIV;		//使用权限
    keyAttr[5] = USBKEY_ALL_PRIV;

    //删除密钥，设置密钥值 全0
    memcpy(data,keyAttr,6);
    memset(data+6,0,24);

    //Delete by zhoushenshen in 20121109
    /*rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_ImportDivSymmKey,request,&reqLen);
      if (rtn != 0)
      {
      AK_Log(AK_LOG_ERROR, "====>>DeleteSymKey->PackageData() Error,rtn = %#08x",rtn);
      return rtn;
      }

      respLen = 300;
      rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
      if (rtn != 0)
      {
      AK_Log(AK_LOG_ERROR, "====>>DeleteSymKey->DeviceExecCmd() Error,rtn = %#08x",rtn);
      return rtn;
      }*/
    //Del end

    //Add by zhoushenshen in 20121109
    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_ImportDivSymmKey,response,&respLen);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>DeleteSymKey->PackageData() Error, rtn = %#08x", rtn);
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
            AK_Log(AK_LOG_ERROR, "====>>DeleteSymKey->DeviceExecCmd() Error,code = %#08x",rtn);
            return rtn;
            break;
    }

    AK_Log(AK_LOG_DEBUG, "====>>DeleteSymKey Success!");
    return USBKEY_OK;
}

//删除容器对称密钥
USBKEY_ULONG DeleteContainerSymmetricKey(USBKEY_HANDLE hDeviceHandle,USBKEY_ULONG ulContainerRecNO)
{
    USBKEY_ULONG rtn;
    USBKEY_UCHAR pcConSymKeyIDRec[USBKEY_MAX_SYM_KEY_ID];
    USBKEY_UCHAR ucFlag = 0;
    USBKEY_UCHAR pcFileID[2];
    int i;

    AK_Log(AK_LOG_DEBUG, "====>>DeleteContainerSymmetricKey");

    //选择对称密钥ID文件
    pcFileID[0] = (USBKEY_SYM_KEY_FILE_ID&0xff00)>>8;
    pcFileID[1] = USBKEY_SYM_KEY_FILE_ID&0x00ff;
    rtn = SelectFile(hDeviceHandle,NULL, pcFileID);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>DeleteContainerSymmetricKey->SelectFile() Error,rtn = %#08x",rtn);
        return rtn;
    }

    //读对称密钥ID文件
    rtn = ReadCurBinaryFile(hDeviceHandle, 0, USBKEY_SYM_KEY_ID_FILE_LEN, pcConSymKeyIDRec);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>DeleteContainerSymmetricKey->ReadCurBinaryFile() Error,rtn = %#08x",rtn);
        return rtn;
    }

    for(i=0; i<USBKEY_MAX_SYM_KEY_ID; i++)
    {
        if(pcConSymKeyIDRec[i] == ulContainerRecNO)
        {
            rtn = DeleteSymKey(hDeviceHandle, i+1);
            if (rtn != 0)
            {
                AK_Log(AK_LOG_ERROR, "====>>DeleteContainerSymmetricKey->DeleteSymKey() Error,rtn = %#08x",rtn);
                return rtn;
            }
            pcConSymKeyIDRec[i] = 0xff;
            if(0 == ucFlag)
                ucFlag = 1;
        }
    }

    if(1 == ucFlag)
    {
        //写对称密钥ID文件
        rtn = WriteCurBinaryFile(hDeviceHandle, 0, USBKEY_SYM_KEY_ID_FILE_LEN, (USBKEY_UCHAR_PTR)pcConSymKeyIDRec);
        if (rtn != 0)
        {
            AK_Log(AK_LOG_ERROR, "====>>DeleteContainerSymmetricKey->WriteCurBinaryFile() Error,rtn = %#08x",rtn);
            return rtn;
        }
    }

    AK_Log(AK_LOG_DEBUG, "====>>DeleteContainerSymmetricKey Success!");
    return USBKEY_OK;
}


//Update by zhoushenshen 20100309
USBKEY_ULONG ClearContainer(USBKEY_HANDLE hDeviceHandle, USBKEY_UCHAR_PTR pcContainerFileID)
{
    //USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0;
    USBKEY_UCHAR p1,p2,lc,le;
    USBKEY_ULONG recNO,reclen;
    USBKEY_UCHAR record[16],data[16];

    AK_Log(AK_LOG_DEBUG, "====>>ClearContainer");

    memset(record,0xff,sizeof(record));
    for( recNO = 1; recNO <= 6; recNO++ )
    {
        rtn = SelectFile(hDeviceHandle,NULL,pcContainerFileID);		//Update by zhoushenshen 20100309
        if (rtn != 0)
        {
            AK_Log(AK_LOG_ERROR, "====>>ClearContainer->SelectFile() Error,rtn = %#08x",rtn);
            return rtn;
        }
        reclen = 16;
        rtn = ReadRecord(hDeviceHandle,pcContainerFileID,recNO,data,&reclen);	//Update by zhoushenshen 20100309
        if (rtn != 0)
        {
            AK_Log(AK_LOG_ERROR, "====>>ClearContainer->ReadRecord() Error,rtn = %#08x",rtn);
            return rtn;
        }

        if ( (reclen >= 2) && (memcmp(data,record,2) != 0) )
        {
            //删除文件
            p1 = 0x00;
            p2 = 0x00;
            lc = 2;
            le = 0x00;

            /*rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_DeleteFile,request,&reqLen);
              if (rtn != 0)
              {
              AK_Log(AK_LOG_ERROR, "====>>ClearContainer->PackageData() Error,rtn = %#08x",rtn);
              return rtn;
              }

              respLen = 300;
              rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
              if (rtn != 0)
              {
              AK_Log(AK_LOG_ERROR, "====>>ClearContainer->DeviceExecCmd() Error,rtn = %#08x",rtn);
              return rtn;
              }*/

            //Add by zhoushenshen in 20121109
            respLen = 300;
            rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_DeleteFile,response,&respLen);
            if (rtn != 0)
            {
                AK_Log(AK_LOG_ERROR, "====>>ClearContainer->PackageData() Error, rtn = %#08x", rtn);
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
                    AK_Log(AK_LOG_DEBUG, "====>>ClearContainer->DeviceExecCmd() Error,code = %#08x",rtn);
                    //return rtn;
                    break;
            }

            if( (recNO == 3) || (recNO == 6) )
            {
                data[2] = 0;
                rtn = DeleFileRecord(hDeviceHandle,data);
                if (rtn != 0)
                {
                    AK_Log(AK_LOG_DEBUG, "====>>ClearContainer->DeleFileRecord() Error,rtn = %#08x",rtn);
                    //return rtn;
                }
            }
        }

    }
    AK_Log(AK_LOG_DEBUG, "====>>ClearContainer Success!");
    return USBKEY_OK;
}

USBKEY_ULONG GetDeviceInfo(USBKEY_HANDLE hDeviceHandle,USBKEY_UCHAR_PTR pcDeviceInfo,USBKEY_ULONG ulInfoType)
{
    //USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0;
    USBKEY_UCHAR data[24];
    USBKEY_UCHAR p1,p2,lc,le;

    AK_Log(AK_LOG_DEBUG, "====>>GetDeviceInfo");

    p1 = 0x00;
    p2 = 0x00;
    lc = 0x00;
    le = 0xb2;

    //Delete by zhoushenshen in 20121108
    /*rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_GetDeviceInfo,request,&reqLen);
      if (rtn != 0)
      {
      AK_Log(AK_LOG_ERROR, "====>>GetDeviceInfo->PackageData() Error,rtn = %#08x",rtn);
      return rtn;
      }

      respLen = 300;
      rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
      if (rtn != 0)
      {
      AK_Log(AK_LOG_ERROR, "====>>GetDeviceInfo->DeviceExecCmd() Error,rtn = %#08x",rtn);
      return rtn;
      }*/
    //Del end

    //Add by zhoushenshen in 20121108
    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_GetDeviceInfo,response,&respLen);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>GetDeviceInfo->PackageData() Error, rtn = %#08x", rtn);
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
            AK_Log(AK_LOG_ERROR, "====>>GetDeviceInfo->DeviceExecCmd() Error,code = %#08x",rtn);
            return rtn;
            break;
    }

    switch(ulInfoType)
    {
        case DEVICEINFO_CUR_PRIV:
            break;
        default:
            AK_Log(AK_LOG_ERROR, "====>>GetDeviceInfo->ulInfoType Error,ulInfoType = %#08x", ulInfoType);
            return USBKEY_PARAM_ERROR;
            break;
    }

    memcpy(pcDeviceInfo,response,respLen-2);

    AK_Log(AK_LOG_DEBUG, "====>>GetDeviceInfo Success!");
    return USBKEY_OK;
}

USBKEY_ULONG SearchContainerRecNo(USBKEY_HANDLE hDeviceHandle,USBKEY_UCHAR_PTR pcContainerName, USBKEY_ULONG_PTR plRecordNum)
{
    USBKEY_UCHAR data[300];
    USBKEY_ULONG rtn,recNO,recLen;
    USBKEY_UCHAR msg[300],fileID[2];

    AK_Log(AK_LOG_DEBUG, "====>>SearchContainerRecNo");

    fileID[0] = 0x80;
    fileID[1] = 0x02;
    rtn = SelectFile(hDeviceHandle,NULL,fileID);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>SearchContainerRecNo->SelectFile() Error,rtn = %#08x",rtn);
        return rtn;
    }

    memset(data,0,sizeof(data));
    memcpy(data,pcContainerName,strlen((char *)pcContainerName));
    for (recNO = 1; recNO <= CONTAINER_MAX_QUANTITY; recNO ++)
    {
        memset(msg,0xff,sizeof(msg));
        recLen = 254;
        rtn = ReadRecord(hDeviceHandle,NULL,recNO,msg,&recLen);		//Update by zhoushenshen 20100309
        if (rtn != 0)
        {
            AK_Log(AK_LOG_ERROR, "====>>SearchContainerRecNo->ReadRecord() Error,rtn = %#08x",rtn);
            return rtn;
        }
        if (memcmp(data,msg,recLen) == 0)
        {
            *plRecordNum = recNO;
            return USBKEY_OK;
        }
    }
    *plRecordNum = 0;

    AK_Log(AK_LOG_DEBUG, "====>>SearchContainerRecNo Success!");
    return USBKEY_OK;
}

USBKEY_ULONG CheckPrivillege(USBKEY_HANDLE hDeviceHandle)
{
    USBKEY_ULONG rtn = 0;
    USBKEY_UCHAR msg[256];

    AK_Log(AK_LOG_DEBUG, "====>>CheckPrivillege");

    memset(msg,0,sizeof(msg));
    rtn = GetDeviceInfo(hDeviceHandle,msg,DEVICEINFO_CUR_PRIV);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>CheckPrivillege->GetDeviceInfo() Error,rtn = %#08x",rtn);
        return rtn;
    }
    //if (msg[172] < 0x08)
    if( (msg[172]&0x08) == 0 )	//edit by heli----20090623
    {
        AK_Log(AK_LOG_ERROR, "====>>CheckPrivillege->User must be logged in!");
        return USBKEY_NO_PRIVILEGE;
    }

    AK_Log(AK_LOG_DEBUG, "====>>CheckPrivillege Success!");
    return USBKEY_OK;
}

//Update by zhoushenshen 20100310
USBKEY_ULONG WritePublicKey(
        USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR pcFileName,	//[in]文件名称
        USBKEY_ULONG ulOffset,			//[in]写入偏移量
        USBKEY_UCHAR_PTR pcData,		//[in]写入数据
        USBKEY_ULONG_PTR pulSize)		//[in,out]输入写入数据缓冲区大小，输出实际写入大小
{
    //USBKEY_UCHAR request[550];
    USBKEY_UCHAR response[550];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0,loop = 0/*,offset,len,i*/;
    //USBKEY_UCHAR p1,p2,lc,le;
    USBKEY_UCHAR p1,p2,le;
    USBKEY_INT32 lc;
    USBKEY_UCHAR data[550];

    AK_Log(AK_LOG_DEBUG, "====>>WritePublicKey");

    /*if (strlen(pcAppName) > 16)
      {
      LogMessage("====>>WritePublicKey->pcAppName Length Error!");
      return USBKEY_PARAM_ERROR;
      }*/

    /*rtn = CheckValid(pcFileName);
      if (rtn != 0)
      {
      LogMessage("====>>WritePublicKey->File Name Error!");
      return rtn;
      }*/

    if (*pulSize > 2048)
    {
        AK_Log(AK_LOG_ERROR, "====>>WritePublicKey->pulSize Error,pulSize = %#08x", *pulSize);
        return USBKEY_PARAM_ERROR;
    }

    /*rtn = SelectFile(hDeviceHandle, pcAppName, NULL);
      if (rtn != 0)
      {
      LogMessage16Val("====>>WritePublicKey->SelectFile() Error,rtn = 0x",rtn);
      return rtn;
      }*/

    /////////检查权限///////////////////////////////////////////////////
    /*rtn = CheckPrivillege(hDeviceHandle);
      if (rtn != 0)
      {
      LogMessage16Val("====>>WritePublicKey->CheckPrivillege() Error,rtn = 0x",rtn);
      return rtn;
      }*/
    //////////////////////////////////////////////////////////////////////////
    rtn = SelectFile(hDeviceHandle, NULL, pcFileName);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>WritePublicKey->SelectFile() Error,rtn = %#08x",rtn);
        return rtn;
    }
    /*
       if ( (*pulSize) > 254)
       {
       loop = (*pulSize)/254;
       }
       offset = ulOffset;
       for (i=0; i<loop; i++)
       {
       len = offset/256;
       memcpy(&p2,&offset,1);
       memcpy(&p1,&len,1);
       lc = 0xFE;
       le = 0x00;
       memcpy(data,pcData+i*254,254);

       rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_UpdateBinary,request,&reqLen);
       if (rtn != 0)
       {
       LogMessage16Val("====>>WritePublicKey->PackageData() Error,rtn = 0x",rtn);
       return rtn;
       }

       respLen = 300;
       rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,&respLen);
       if (rtn != 0)
       {
       LogMessage16Val("====>>WritePublicKey->DeviceExecCmd() Error,rtn = 0x",rtn);
       return rtn;
       }

       rtn = *(response+respLen-2);
       rtn = rtn*256 + *(response+respLen-1);
       switch(rtn)
       {
       case DEVICE_SUCCESS:
       break;
       default:
       LogMessage16Val("====>>WritePublicKey->DeviceExecCmd() Error,code = 0x",rtn);
       return rtn;
       break;
       }
       offset+=254;
       }

       len = offset/256;
       memcpy(&p2,&offset,1);
       memcpy(&p1,&len,1);
       len = (*pulSize) - loop*254;
       lc = len;
       le = 0x00;
       memcpy(data,pcData+loop*254,len);

       rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_UpdateBinary,request,&reqLen);
       if (rtn != 0)
       {
       LogMessage16Val("====>>WritePublicKey->PackageData() Error,rtn = 0x",rtn);
       return rtn;
       }
     */

    //Delete by zhoushenshen in 20121204
    //一次写入公钥------edit by heli 20090619
    /*p1 = 0;
      p2 = 0;
      le = 0x00;

      if ( (*pulSize) > 254)
      {
      lc = 0xff;
      data[0] = (USBKEY_UCHAR)(*pulSize/256);
      data[1] = (USBKEY_UCHAR)(*pulSize%256);
      memcpy(data+2,pcData,*pulSize);
      }
      else
      {
      lc = (USBKEY_UCHAR)(*pulSize);
      memcpy(data,pcData,*pulSize);
      }

    //组织数据包
    rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_UpdatePublicKeyBinary,request,&reqLen);
    if (rtn != 0)
    {
    AK_Log(AK_LOG_ERROR, "====>>WritePublicKey->PackageData() Error,rtn = %#08x",rtn);
    return rtn;
    }
    //end------edit by heli 20090619

    respLen = 300;
    rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
    if (rtn != 0)
    {
    AK_Log(AK_LOG_ERROR, "====>>WritePublicKey->DeviceExecCmd() Error,rtn = %#08x",rtn);
    return rtn;
    }

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
    case DEVICE_SUCCESS:
    break;
    default:
    AK_Log(AK_LOG_ERROR, "====>>WritePublicKey->DeviceExecCmd() code,rtn = %#08x",rtn);
    return rtn;
    break;
    }*/
    //Del end

    rtn = WriteCurBinaryFile(hDeviceHandle,0,*pulSize,pcData);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>WritePublicKey->WriteCurBinaryFile() Error,rtn = %#08x",rtn);
        return rtn;
    }

    //offset+=len;
    //*pulSize = offset - ulOffset;

    AK_Log(AK_LOG_DEBUG, "====>>WritePublicKey Success!");
    return USBKEY_OK;
}

//Delete by zhoushenshen 20121212
//Update by zhoushenshen 20100317
/*USBKEY_ULONG WriteProtectedPrivateKey(
  USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
  USBKEY_UCHAR_PTR pcFileName,	//[in]文件名称
  USBKEY_ULONG ulOffset,			//[in]写入偏移量
  USBKEY_UCHAR_PTR pcData,		//[in]写入数据
  USBKEY_ULONG_PTR pulSize)		//[in,out]输入写入数据缓冲区大小,输出实际写入大小
  {
  USBKEY_UCHAR request[300];
  USBKEY_UCHAR response[300];
  USBKEY_ULONG rtn,reqLen,respLen=0,loop = 0,offset,len,i;
  USBKEY_UCHAR p1,p2,lc,le;
  USBKEY_UCHAR data[300];

  AK_Log(AK_LOG_DEBUG, "====>>WriteProtectedPrivateKey");

//if (strlen(pcAppName) > 16)
//{
//	LogMessage("====>>WriteProtectedPrivateKey->pcAppName Length Error!");
//	return USBKEY_PARAM_ERROR;
//}

//rtn = CheckValid(pcFileName);
//if (rtn != 0)
//{
//	LogMessage("====>>WriteProtectedPrivateKey->File Name Error!");
//	return rtn;
//}

if (*pulSize > 2048)
{
AK_Log(AK_LOG_ERROR, "====>>WriteProtectedPrivateKey->pulSize Error,pulSize = %#08x", *pulSize);
return USBKEY_PARAM_ERROR;
}

//rtn = SelectFile(hDeviceHandle, pcAppName, NULL);
//if (rtn != 0)
//{
//	LogMessage16Val("====>>WriteProtectedPrivateKey->SelectFile() Error,rtn = 0x",rtn);
//	return rtn;
//}

/////////检查权限///////////////////////////////////////////////////
rtn = CheckPrivillege(hDeviceHandle);
if (rtn != 0)
{
AK_Log(AK_LOG_ERROR, "====>>WriteProtectedPrivateKey->CheckPrivillege() Error,rtn = %#08x",rtn);
return rtn;
}
//////////////////////////////////////////////////////////////////////////

rtn = SelectFile(hDeviceHandle, NULL, pcFileName);
if (rtn != 0)
{
AK_Log(AK_LOG_ERROR, "====>>WriteProtectedPrivateKey->SelectFile() Error,rtn = %#08x",rtn);
return rtn;
}

if ( (*pulSize) > 254)
{
loop = (*pulSize)/254;
}
offset = ulOffset;
for (i=0; i<loop; i++)
{
len = offset/256;
memcpy(&p2,&offset,1);
memcpy(&p1,&len,1);
lc = 0xFE;
le = 0x00;
memcpy(data,pcData+i*254,254);

rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_UpdateProtectedBinary,request,&reqLen);
if (rtn != 0)
{
    AK_Log(AK_LOG_ERROR, "====>>WriteProtectedPrivateKey->PackageData() loop Error,rtn = %#08x",rtn);
    return rtn;
}

respLen = 300;
rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
if (rtn != 0)
{
    AK_Log(AK_LOG_ERROR, "====>>WriteProtectedPrivateKey->DeviceExecCmd() loop Error,rtn = %#08x",rtn);
    return rtn;
}

rtn = *(response+respLen-2);
rtn = rtn*256 + *(response+respLen-1);
switch(rtn)
{
    case DEVICE_SUCCESS:
        break;
    default:
        AK_Log(AK_LOG_ERROR, "====>>WriteProtectedPrivateKey->DeviceExecCmd() loop Error,code = %#08x",rtn);
        return rtn;
        break;
}
offset+=254;
}

len = offset/256;
memcpy(&p2,&offset,1);
memcpy(&p1,&len,1);
len = (*pulSize) - loop*254;
lc = (USBKEY_UCHAR)len;
le = 0x00;
memcpy(data,pcData+loop*254,len);

rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_UpdateProtectedBinary,request,&reqLen);
if (rtn != 0)
{
    AK_Log(AK_LOG_ERROR, "====>>WriteProtectedPrivateKey->PackageData() Error,rtn = %#08x",rtn);
    return rtn;
}

respLen = 300;
rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
if (rtn != 0)
{
    AK_Log(AK_LOG_ERROR, "====>>WriteProtectedPrivateKey->DeviceExecCmd() Error,rtn = %#08x",rtn);
    return rtn;
}

rtn = *(response+respLen-2);
rtn = rtn*256 + *(response+respLen-1);
switch(rtn)
{
    case DEVICE_SUCCESS:
        break;
    default:
        AK_Log(AK_LOG_ERROR, "====>>WriteProtectedPrivateKey->DeviceExecCmd() Error,code = %#08x",rtn);
        return rtn;
        break;
}

offset+=len;
*pulSize = offset - ulOffset;

AK_Log(AK_LOG_DEBUG, "====>>WriteProtectedPrivateKey Success!");
return USBKEY_OK;
}*/

USBKEY_ULONG EraseUsbKey(USBKEY_HANDLE hDeviceHandle)
{
    /*	USBKEY_UCHAR request[300];
        USBKEY_UCHAR response[300];
        USBKEY_ULONG rtn,reqLen,respLen=0,keylen = 24;
        USBKEY_UCHAR p1,p2,lc,le;
        USBKEY_UCHAR_PTR data;
        USBKEY_UCHAR msg[300],rand[300];
        FILE *fp,*fp1;
        USBKEY_KEYHEAD keyhead;

        LogMessage("->EraseUsbKey");

        fp = fopen("c:\\key","r+");
        fread(rand,1,24,fp);
        fclose(fp);

        keyhead.ApplicationType = KEY_EXTERNAL_AUTHENTICATE_KEY;
        keyhead.ErrorCounter = 0x0f;
        keyhead.ulAccessControl = 0xf0;
        keyhead.ulChangeControl = 0xf3;
        rtn = USBKEY_WriteKey(hDeviceHandle,(USBKEY_UCHAR_PTR)&keyhead,rand,keylen);
        if (rtn != 0)
        {
        LogMessageVal("====>>EraseUsbKey->USBKEY_WriteKey() Error,rtn = 0x",rtn);
        return rtn;
        }

        rtn = USBKEY_GenRandom(hDeviceHandle,rand,8);
        if (rtn != 0)
        {
        LogMessage16Val("====>>EraseUsbKey->USBKEY_GenRandom() Error,rtn = 0x",rtn);
        return rtn;
        }

        fp = fopen("c:\\rand","w+");
        fwrite(rand,1,8,fp);
        fclose(fp);

        fp1 = fopen("c:\\enc","r+");
        fread(rand,1,16,fp1);
        fclose(fp1);
        rtn = USBKEY_ExtAuthenticate(hDeviceHandle,rand,16);
        if (rtn != 0)
        {
        LogMessage16Val("====>>EraseUsbKey->USBKEY_ExtAuthenticate() Error,rtn = 0x",rtn);
        return rtn;
        }


    //*
    p1 = 0x00;
    p2 = 0x00;
    lc = 0x02;
    le = 0x00;
    data = malloc(lc);

    ////生成文件标识
    //memcpy(data+2,ulAccessCondition,1);
    le = 0x3f;
    memset(data,0,lc);
    memcpy(data,&le,1);

    rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_DeleteFile,request,&reqLen);
    if (rtn != 0)
    {
    LogMessage16Val("====>>EraseUsbKey->PackageData() Error,rtn = 0x",rtn);
    free(data);
    return rtn;
    }

    respLen = 300;
    rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,&respLen);
    if (rtn != 0)
    {
        LogMessage16Val("====>>EraseUsbKey->DeviceExecCmd() Error,rtn = 0x",rtn);
        free(data);
        return rtn;
    }

    rtn = 0x0090;
    if (memcmp(response+respLen-2,&rtn,2) != 0)
    {
        LogMessage16Val("====>>EraseUsbKey->DeviceExecCmd() Error,code = %02x%02x",*(response+respLen-2),*(response+respLen-1));
        free(data);
        return USBKEY_GENERAL_ERROR;
    }

    free(data);
    //*/

    USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    USBKEY_ULONG rtn,i,respLen=0,reqLen[10] = {21,21,11,11,11,11,11,11,11,5};
    USBKEY_UCHAR code[10][300] = 
    {
        {0xB8 ,0x00 ,0x00 ,0x00 ,0x10 ,0x69 ,0x72 ,0x2d ,0x4e ,0x84 ,0x76 ,0xfe, 0x56 ,0x3b ,0x75 ,0x05 ,0x96 ,0xfb ,0x8b ,0x02 ,0x30},
        {0xb8 ,0x00 ,0x00 ,0x01 ,0x10 ,0xff, 0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff},
        {0xb8 ,0x00 ,0x00 ,0x02 ,0x06 ,0x00 ,0x00 ,0x00 ,0x08 ,0x01 ,0x00},
        {0xb8 ,0x00 ,0x00 ,0x02 ,0x06 ,0x01 ,0x00 ,0x00 ,0x08 ,0x01 ,0x00},
        {0xb8 ,0x00 ,0x00 ,0x02 ,0x06 ,0x02 ,0x00 ,0x00 ,0x08 ,0x01 ,0x00},
        {0xb8 ,0x00 ,0x00 ,0x02 ,0x06 ,0x03 ,0x00 ,0x00 ,0x08 ,0x01 ,0x00},
        {0xb8 ,0x00 ,0x00 ,0x02 ,0x06 ,0x04 ,0x08 ,0x00 ,0x08 ,0x01 ,0x00},
        {0xb8 ,0x00 ,0x00 ,0x02 ,0x06 ,0x05 ,0x08 ,0x00 ,0x08 ,0x01 ,0x00},
        {0xb8 ,0x00 ,0x00 ,0x02 ,0x06 ,0x60 ,0x11 ,0x00 ,0x08 ,0x01 ,0x01},
        {0xb8 ,0x01 ,0x00 ,0x00 ,0x00}
    };

    AK_Log(AK_LOG_DEBUG, "====>>EraseUsbKey");

    for (i = 0; i < 10; i++)
    {
        memset(request,0,sizeof(request));
        memcpy(request,code[i],reqLen[i]);
        respLen = 300;
        rtn = DeviceExecCmd(hDeviceHandle,request,reqLen[i],response,(int *)&respLen);
        if (rtn != 0)
        {
            AK_Log(AK_LOG_ERROR, "====>>EraseUsbKey->DeviceExecCmd() Error,rtn = %#08x",rtn);
            return rtn;
        }

        rtn = *(response+respLen-2);
        rtn = rtn*256 + *(response+respLen-1);
        switch(rtn)
        {
            case DEVICE_SUCCESS:
                break;
            default:
                AK_Log(AK_LOG_ERROR, "====>>EraseUsbKey->DeviceExecCmd() Error,code = %#08x",rtn);
                return rtn;
                break;
        }
        AK_Log(AK_LOG_DEBUG, "====>>EraseUsbKey->DeviceExecCmd() Success,i = %#08x", i);
    }

    AK_Log(AK_LOG_DEBUG, "====>>EraseUsbKey Success!");
    return USBKEY_OK;
}

USBKEY_ULONG Erase45Key(USBKEY_HANDLE hDeviceHandle)
{
    USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    USBKEY_ULONG rtn,i,respLen=0,reqLen[6] = {21,21,18,14,11,14};
    USBKEY_UCHAR code[6][300] = 
    {
        {0xB8 ,0x00 ,0x00 ,0x00 ,0x10 ,0x69 ,0x72 ,0x2d ,0x4e ,0x84 ,0x76 ,0xfe ,0x56 ,0x3b ,0x75 ,0x05 ,0x96 ,0xfb ,0x8b ,0x02 ,0x30},
        {0xb8 ,0x00 ,0x00 ,0x01 ,0x10 ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff},
        {0xb8 ,0x00 ,0x00 ,0x02 ,0x0d ,0x00 ,0x00 ,0x84 ,0xe0 ,0x08 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00},
        {0xb8 ,0x00 ,0x00 ,0x02 ,0x09 ,0x08 ,0x50 ,0x84 ,0xe0 ,0x04 ,0x00 ,0x00 ,0x00 ,0x00},
        {0xb8 ,0x00 ,0x00 ,0x02 ,0x06 ,0x08 ,0x90 ,0x84 ,0xe0 ,0x01 ,0x01},
        {0xb8 ,0x00 ,0x00 ,0x02 ,0x09 ,0x82 ,0x60 ,0x84 ,0xe0 ,0x04 ,0x00 ,0x00 ,0x00 ,0x00}
    };

    AK_Log(AK_LOG_DEBUG, "====>>Erase45Key");

    for (i = 0; i < 6; i++)
    {
        memset(request,0,sizeof(request));
        memcpy(request,code[i],reqLen[i]);
        respLen = 300;	//add by heli
        rtn = DeviceExecCmd(hDeviceHandle,request,reqLen[i],response,(int *)&respLen);
        if (rtn != 0)
        {
            AK_Log(AK_LOG_ERROR, "====>>Erase45Key->DeviceExecCmd() Error,rtn = %#08x",rtn);
            return rtn;
        }

        rtn = *(response+respLen-2);
        rtn = rtn*256 + *(response+respLen-1);
        switch(rtn)
        {
            case DEVICE_SUCCESS:
                break;
            default:
                AK_Log(AK_LOG_ERROR, "====>>Erase45Key->DeviceExecCmd() Error,code = %#08x",rtn);
                return rtn;
                break;
        }
        AK_Log(AK_LOG_DEBUG, "====>>Erase45Key->DeviceExecCmd() Success,i = %#08x", i);
    }

    AK_Log(AK_LOG_DEBUG, "====>>Erase45Key Success!");
    return USBKEY_OK;
}

USBKEY_ULONG EraseAC4Key(USBKEY_HANDLE hDeviceHandle)
{
    USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    USBKEY_ULONG rtn,i,respLen=0,reqLen[5] = {21,21,18,14,11};
    USBKEY_UCHAR code[6][300] = 
    {
        {0xB8 ,0x00 ,0x00 ,0x00 ,0x10 ,0x69 ,0x72 ,0x2d ,0x4e ,0x84 ,0x76 ,0xfe ,0x56 ,0x3b ,0x75 ,0x05 ,0x96 ,0xfb ,0x8b ,0x02 ,0x30},
        {0xb8 ,0x00 ,0x00 ,0x01 ,0x10 ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff},
        {0xb8 ,0x00 ,0x00 ,0x02 ,0x0d ,0x00 ,0x00 ,0x84 ,0x00 ,0x08 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00},
        {0xb8 ,0x00 ,0x00 ,0x02 ,0x09 ,0xFC ,0x17 ,0x84 ,0x00 ,0x04 ,0x00 ,0x00 ,0x00 ,0x00},
        {0xb8 ,0x00 ,0x00 ,0x02 ,0x06 ,0x08 ,0x24 ,0x84 ,0x00 ,0x01 ,0x01}
    };

    AK_Log(AK_LOG_DEBUG, "====>>EraseAC4Key");

    for (i = 0; i < 5; i++)
    {
        memset(request,0,sizeof(request));
        memcpy(request,code[i],reqLen[i]);
        respLen = 300;	//add by heli
        rtn = DeviceExecCmd(hDeviceHandle,request,reqLen[i],response,(int *)&respLen);
        if (rtn != 0)
        {
            AK_Log(AK_LOG_ERROR, "====>>EraseAC4Key->DeviceExecCmd() Error,rtn = %#08x",rtn);
            return rtn;
        }

        rtn = *(response+respLen-2);
        rtn = rtn*256 + *(response+respLen-1);
        switch(rtn)
        {
            case DEVICE_SUCCESS:
                break;
            default:
                AK_Log(AK_LOG_ERROR, "====>>EraseAC4Key->DeviceExecCmd() Error,code = %#08x",rtn);
                return rtn;
                break;
        }
        AK_Log(AK_LOG_DEBUG, "====>>EraseAC4Key->DeviceExecCmd() Success,i = %#08x", i);
    }

    AK_Log(AK_LOG_DEBUG, "====>>EraseAC4Key Success!");
    return USBKEY_OK;
}


// add by yangshuang-20121220
USBKEY_ULONG EraseAC3Key(USBKEY_HANDLE hDeviceHandle)
{
    USBKEY_ULONG rtn = 0;
    USBKEY_ULONG i = 0;
    USBKEY_ULONG respLen = 0;

    USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    USBKEY_UCHAR cla[6] = {0xb8, 0xb8, 0xb8, 0xb8, 0xb8, 0xb8};
    USBKEY_UCHAR ins[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    USBKEY_UCHAR p1[6]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    USBKEY_UCHAR p2[6]  = {0x00, 0x01, 0x02, 0x02, 0x02, 0x02};
    USBKEY_UCHAR lc[6]  = {0x10, 0x10, 0x0B, 0x15, 0x06, 0x06};
    USBKEY_UCHAR le[6]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    USBKEY_UCHAR pCmdData[6][100] = 
    {
        {0x73 ,0x84 ,0x35 ,0x5f ,0xef ,0xdc ,0xab ,0x4a ,
            0x8c ,0x9f ,0x39 ,0x21 ,0x7b ,0x63 ,0x72 ,0x8a},
        {0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,
            0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff},
        {0x14 ,0xB0 ,0x01 ,0x78 ,0x06 ,0x01 ,0x02 ,0x03 ,
            0x04 ,0x05 ,0x06},
        {0x1C ,0xB0 ,0x01 ,0x78 ,0x10 ,0x02 ,0x80 ,0x40 ,
            0x08 ,0x04 ,0x02 ,0x20 ,0x10 ,0x08 ,0x04 ,0x02 ,
            0x01 ,0x80 ,0x80 ,0x40 ,0x20},
        {0x34 ,0xB0 ,0x01 ,0x78 ,0x01 ,0x03},
        {0x06 ,0x01 ,0x02 ,0x78 ,0x01 ,0x01}	
    };

    AK_Log(AK_LOG_DEBUG, "====>>EraseAC3Key");

    for (i = 0; i < 6; ++i)
    {
        memset(request, 0, sizeof(request));
        respLen = 300;	//add by heli

        // add by yangshuang 20121221 
        rtn = DeviceExecCmdExt(  
                hDeviceHandle, 
                gDeviceType, 
                cla[i], 
                ins[i], 
                p1[i], 
                p2[i], 
                lc[i],
                pCmdData[i],
                le[i],
                response,
                (int *)&respLen); 

        if (rtn != 0)
        {
            AK_Log(AK_LOG_ERROR, 
                    "====>>EraseAC3Key->DeviceExecCmdExt() \
                    Error,rtn = %#08x \t\tthe value i is %d\n", 
                    rtn, i);
            return rtn;
        }
        rtn = *(response + respLen - 2);
        rtn = rtn * 256 + *(response + respLen - 1);
        switch(rtn)
        {
            case DEVICE_SUCCESS:
                break;
            default:
                AK_Log(AK_LOG_ERROR, "====>>EraseAC3Key->DeviceExecCmdExt() Error,code = %#08x", rtn);
                return rtn;
                break;
        }

        AK_Log(AK_LOG_DEBUG, "====>>EraseAC3Key->DeviceExecCmd() Success,i = %#08x", i);
    }
    //printf("EraseAC3Key End\n");
    AK_Log(AK_LOG_DEBUG, "====>>EraseAC3Key Success!");
    return USBKEY_OK;
}
// add end


//flag 权限的标志。USBKEY_USERTYPE_USER为检查用户权限 USBKEY_USERTYPE_ADMIN为检查管理员权限 0为检查管理员或用户权限
USBKEY_ULONG TaxCheckPrivillege(USBKEY_HANDLE hDeviceHandle,USBKEY_ULONG flag) 
{
#ifndef _ATAX_APP
    AK_Log(AK_LOG_ERROR, "====>>TaxCheckPrivillege Error,rtn = %#08x",DEVICE_FUNC_NOT_SUPPORT);
    return DEVICE_FUNC_NOT_SUPPORT;

#else
    USBKEY_ULONG rtn = 0;
    USBKEY_UCHAR msg[256];

    AK_Log(AK_LOG_DEBUG, "====>>TaxCheckPrivillege");

    memset(msg,0,sizeof(msg));
    rtn = GetDeviceInfo(hDeviceHandle,msg,DEVICEINFO_CUR_PRIV);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>TaxCheckPrivillege->GetDeviceInfo() Error,rtn = %#08x",rtn);
        return rtn;
    }
    //if (msg[172] < 0x08)

    //edit by zhoushenshen----20091030
    if( USBKEY_USERTYPE_USER == flag && (msg[172]&0x08) == 0)	
    {
        AK_Log(AK_LOG_ERROR, "====>>TaxCheckPrivillege->User must be logged in!");
        return USBKEY_NO_PRIVILEGE;
    }
    if( USBKEY_USERTYPE_USER == flag && (msg[172]&0x40) != 0)	
    {
        AK_Log(AK_LOG_ERROR, "====>>TaxCheckPrivillege->User must be logged in!");
        return USBKEY_NO_PRIVILEGE;
    }
    if( USBKEY_USERTYPE_ADMIN == flag && (msg[172]&0x40) == 0 )	
    {
        AK_Log(AK_LOG_ERROR, "====>>TaxCheckPrivillege->Administrator must be logged in!");
        return USBKEY_NO_PRIVILEGE;
    }
    if( 0 == flag && (msg[172]&0x08) == 0)	//edit by heli----20090623
    {
        AK_Log(AK_LOG_ERROR, "====>>TaxCheckPrivillege->must be logged in!");
        return USBKEY_NO_PRIVILEGE;
    }

    AK_Log(AK_LOG_DEBUG, "====>>TaxCheckPrivillege Success!");
    return USBKEY_OK;
#endif
}

//==============================================================
//函数名称:		AK_Reverser
//函数参数:		buf[in/out],in：待转换数据，out：转换结果
//				len[in], 要转换的数据长度
//返回值:	    Cryptoki定义的错误码
//				
//函数功能:		把buf中的数据倒转 
//==============================================================
void Reverser(USBKEY_VOID_PTR buf, USBKEY_ULONG len)
{
    USBKEY_UCHAR t; 
    USBKEY_ULONG i;
    USBKEY_UCHAR_PTR pBuf;

    pBuf = (USBKEY_UCHAR_PTR)buf;
    for(i = 0; i<len/2; i++)
    {
        t = pBuf[i];
        pBuf[i] = pBuf[len-1-i];
        pBuf[len-1-i] = t;
    }

    return;
}
//////////////////////////////////////////////////////////////////////////

// 创建公私钥文件
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_ULONG CreateKeyPairFile(
        USBKEY_HANDLE		hDeviceHandle,
        USBKEY_UCHAR_PTR	pcPubKeyFileID, 
        USBKEY_UCHAR_PTR	pcPriKeyFileID,
        USBKEY_UCHAR		ucPubKeyFileRight,
        USBKEY_UCHAR		ucPriKeyFileRight)
{
    //USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0;
    USBKEY_UCHAR data[16];
    USBKEY_UCHAR p1,p2,lc,le;

    AK_Log(AK_LOG_DEBUG, "====>>CreateKeyPairFile");

    //创建公钥文件
    p1 = 0x00;
    p2 = 0x10;
    //lc = 9;
    lc = 0x07;						//lc updtae in 20120106
    le = 0x00;

    memset(data,0,9);
    memcpy(data,pcPubKeyFileID,2);
    data[3] = USBKEY_ALL_PRIV;
    memcpy(data+4,&ucPubKeyFileRight,1);
    //updtae in 20120106 文件长度由4字节改为2字节
    le = USBKEY_PUBLIC_KEY_FILE_LEN/256;
    //memcpy(data+7,&le,1);
    memcpy(data+5,&le,1);
    le = (USBKEY_UCHAR)USBKEY_PUBLIC_KEY_FILE_LEN;
    //memcpy(data+8,&le,1);
    memcpy(data+6,&le,1);

    //Delete by zhoushenshen in 20121107
    /*//组包
      rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_CreateFile,request,&reqLen);
      if (rtn != 0)
      {
      AK_Log(AK_LOG_ERROR, "====>>CreateKeyPairFile->PackageData() Create PubKey File Error,rtn = %#08x",rtn);
      return rtn;
      }

    //发送命令
    respLen = 300;
    rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
    if (rtn != 0)
    {
    AK_Log(AK_LOG_ERROR, "====>>CreateKeyPairFile->DeviceExecCmd() Create PubKey File Error,rtn = %#08x",rtn);
    return rtn;
    }*/
    //Del end

    //Add by zhoushenshen in 20121107
    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_CreateFile,response,&respLen);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>CreateKeyPairFile->PackageData() Create PubKey File Error, rtn = %#08x", rtn);
        return rtn;
    }
    //Add end

    //处理响应码
    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        case DEVICE_CMD_DATA_ERROR:
            AK_Log(AK_LOG_DEBUG, "====>>CreateKeyPairFile Public Key File Exist.");
            break;
        default:
            AK_Log(AK_LOG_ERROR, "====>>CreateKeyPairFile->DeviceExecCmd() Create PubKey File Error,code = %#08x",rtn);
            return rtn;
    }

    AK_Log(AK_LOG_DEBUG, "====>>CreateKeyPairFile Create Public Key File Success.");

    //创建私钥文件
    p1 = 0x00;
    p2 = 0x11;
    //lc = 9;
    lc = 0x07;						//lc updtae in 20120106
    le = 0x00;

    memset(data,0,9);
    memcpy(data,pcPriKeyFileID,2);
    data[2] = 0x02;
    le = ucPriKeyFileRight;	//设置读权限
    memcpy(data+3,&le,1);
    memcpy(data+4,&ucPriKeyFileRight,1);
    //updtae in 20120106 文件长度由4字节改为2字节
    le = USBKEY_PRIVATE_KEY_FILE_LEN/256;
    //memcpy(data+7,&le,1);
    memcpy(data+5,&le,1);
    le = (USBKEY_UCHAR)USBKEY_PRIVATE_KEY_FILE_LEN;
    //memcpy(data+8,&le,1);
    memcpy(data+6,&le,1);

    //Delete by zhoushenshen in 20121107
    /*//组包
      rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_CreateFile,request,&reqLen);
      if (rtn != 0)
      {
      AK_Log(AK_LOG_ERROR, "====>>CreateKeyPairFile->PackageData() Create PriKey File Error,rtn = %#08x",rtn);
      return rtn;
      }

    //发送命令
    respLen = 300;
    rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
    if (rtn != 0)
    {
    AK_Log(AK_LOG_ERROR, "====>>CreateKeyPairFile->DeviceExecCmd() Create PriKey File Error,rtn = %#08x",rtn);
    return rtn;
    }*/
    //Del end

    //Add by zhoushenshen in 20121107
    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_CreateFile,response,&respLen);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>CreateKeyPairFile->PackageData() Create PriKey File Error, rtn = %#08x", rtn);
        return rtn;
    }
    //Add end

    //处理响应码
    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        case DEVICE_CMD_DATA_ERROR:
            AK_Log(AK_LOG_DEBUG, "====>>CreateKeyPairFile Private Key File Exist.");
            break;
        default:
            AK_Log(AK_LOG_ERROR, "====>>CreateKeyPairFile->DeviceExecCmd() Create PriKey File Error,code = %#08x",rtn);
            return rtn;
    }
    AK_Log(AK_LOG_DEBUG, "====>>CreateKeyPairFile Create Private Key File Success.");


    AK_Log(AK_LOG_DEBUG, "====>>CreateKeyPairFile Success!");
    return USBKEY_OK;
}


// 产生ECC密钥对
// 返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_ULONG GenEccKeyPair(
        USBKEY_HANDLE			hDeviceHandle,
        USBKEY_ULONG			ulBitLen, 
        USBKEY_UCHAR_PTR		pcPubKeyFileID, 
        USBKEY_UCHAR_PTR		pcPriKeyFileID,
        USBKEY_ECC_PUBLIC_KEY*	pPublicKey)
{
    //USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    USBKEY_UCHAR data[300];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0;
    USBKEY_UCHAR p1,p2,le,lc;

    AK_Log(AK_LOG_DEBUG, "====>>GenEccKeyPair");

    //Delete by zhoushenshen in 20121212
    /*//生成密钥
      request[0] = USBKEY_CMD_TYPE_PKI;
      request[1] = USBKEY_CMD_DODE_GEN_ECC_KEY_PAIR;
      request[2] = 0x01;	//写文件
      if (ulBitLen == 256)
      {
      request[3] = 0x02;
      }
      else
      {
      request[3] = 0x03;
      }
      request[4] = 0x04;
      memcpy(request+5, pcPubKeyFileID, 2);
      memcpy(request+7, pcPriKeyFileID, 2);
      reqLen = 9;

      respLen = 300;
      rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
      if (rtn != 0)
      {
      AK_Log(AK_LOG_ERROR, "====>>GenEccKeyPair->DeviceExecCmd() Error,rtn = %#08x",rtn);
      return rtn;
      }*/
    //Del end

    //Add by zhoushenshen in 20121212
    //生成密钥
    p1 = 0x01;	//写文件
    if (ulBitLen == 256)
    {
        p2 = 0x02;
    }
    else
    {
        p2 = 0x03;
    }
    lc = 0x04;
    le = ulBitLen/8*2;	//公钥长度
    memcpy(data, pcPubKeyFileID, 2);
    memcpy(data+2, pcPriKeyFileID, 2);

    //发送命令
    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_GenECCKeyPair,response,&respLen);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>GenEccKeyPair->PackageData() Error,rtn = %#08x", rtn);
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
            AK_Log(AK_LOG_ERROR, "====>>GenEccKeyPair->DeviceExecCmd() Error,code = %#08x",rtn);
            return rtn;
    }

    pPublicKey->bits = ulBitLen;
    memcpy(pPublicKey->XCoordinate, response, ulBitLen/8);
    memcpy(pPublicKey->YCoordinate, response+ulBitLen/8, ulBitLen/8);

    AK_Log(AK_LOG_DEBUG, "====>>GenEccKeyPair Success!");
    return USBKEY_OK;
}

//读当前选择的二进制文件
USBKEY_ULONG ReadCurBinaryFile(
        USBKEY_HANDLE		hDeviceHandle, 
        USBKEY_ULONG		ulOffset, 
        USBKEY_ULONG		ulLen,
        USBKEY_UCHAR_PTR	pOutBuf)
{
    //USBKEY_UCHAR request[10];
    USBKEY_UCHAR response[USBKEY_FILE_MAX_BLOCK_LEN+2];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0;
    USBKEY_UCHAR p1,p2,lc;
    USBKEY_INT32 le;
    USBKEY_UCHAR data[10];
    USBKEY_ULONG loop = 0,offset,len,i;

    AK_Log(AK_LOG_DEBUG, "====>>ReadCurBinaryFile");

    if( ulOffset >= 0x8000 )
    {
        AK_Log(AK_LOG_ERROR, "====>>ReadCurBinaryFile->ulOffset Error,ulOffset = %#08x", ulOffset);
        return USBKEY_PARAM_ERROR;
    }
    /*if( ulLen > USBKEY_FILE_MAX_BLOCK_LEN )
      {
      AK_Log(AK_LOG_ERROR, "====>>ReadCurBinaryFile->ulLen Error,ulLen = %#08x", ulLen);
      return USBKEY_PARAM_ERROR;
      }*/

    //Delete by zhoushenshen in 20121109
    /*//组包
      request[0] = 0x00;		//CLA
      request[1] = USBKEY_CMD_READ_BINARY;	//INS
      request[2] = (USBKEY_UCHAR)((ulOffset&0xff00)>>8);	//P1
      request[3] = (USBKEY_UCHAR)(ulOffset&0xff);			//P2
      if(ulLen < 255)
      {
      request[4] = (USBKEY_UCHAR)ulLen;
      reqLen = 5;
      }
      else
      {
      request[4] = 0xff;
      request[5] = (USBKEY_UCHAR)((ulLen&0xff00)>>8);
      request[6] = (USBKEY_UCHAR)(ulLen&0xff);
      reqLen = 7;
      }

    //发送命令
    respLen = USBKEY_MAX_COM_BUFFER+2;
    rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
    if (rtn != 0)
    {
    AK_Log(AK_LOG_ERROR, "====>>ReadCurBinaryFile->DeviceExecCmd() Error,rtn = %#08x",rtn);
    return rtn;
    }*/
    //Del end

    //Add by zhoushenshen in 20121109
    if ( ulLen > USBKEY_FILE_MAX_BLOCK_LEN)
    {
        loop = ulLen/USBKEY_FILE_MAX_BLOCK_LEN;
    }
    offset = ulOffset;

    for (i=0; i<loop; i++)
    {
        len = offset/256;
        memcpy(&p2,&offset,1);
        memcpy(&p1,&len,1);
        lc = 0x00;
        le = USBKEY_FILE_MAX_BLOCK_LEN;

        respLen = USBKEY_FILE_MAX_BLOCK_LEN+2;
        rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_ReadBinary,response,&respLen);
        if (rtn != 0)
        {
            AK_Log(AK_LOG_ERROR, "====>>ReadCurBinaryFile->PackageData() Error, rtn = %#08x", rtn);
            return rtn;
        }

        rtn = *(response+respLen-2);
        rtn = rtn*256 + *(response+respLen-1);
        switch(rtn)
        {
            case DEVICE_SUCCESS:
                break;
            default:
                AK_Log(AK_LOG_ERROR, "====>>ReadCurBinaryFile->DeviceExecCmd() loop Error,code = %#08x",rtn);
                return rtn;
        }

        memcpy(pOutBuf+i*USBKEY_FILE_MAX_BLOCK_LEN,response,respLen-2);
        offset+=USBKEY_FILE_MAX_BLOCK_LEN;
    }

    len = offset/256;
    memcpy(&p2,&offset,1);
    memcpy(&p1,&len,1);
    lc = 0x00;
    len = ulLen - loop*USBKEY_FILE_MAX_BLOCK_LEN;
    if(0 == len)
    {
        AK_Log(AK_LOG_DEBUG, "====>>ReadCurBinaryFile Success!");
        return USBKEY_OK;
    }
    le = len;

    respLen = len+2;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_ReadBinary,response,&respLen);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>ReadCurBinaryFile->PackageData() Error, rtn = %#08x", rtn);
        return rtn;
    }

    //p1 = (USBKEY_UCHAR)((ulOffset&0xff00)>>8);	//P1
    //p2 = (USBKEY_UCHAR)(ulOffset&0xff);			//P2
    //lc = 0x00;
    //le = ulLen;
    //respLen = USBKEY_FILE_MAX_BLOCK_LEN+2;

    //Add end


    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_Log(AK_LOG_ERROR, "====>>ReadCurBinaryFile->DeviceExecCmd() Error,code = %#08x",rtn);
            return rtn;
    }

    //memcpy(pOutBuf, response, respLen-2);
    memcpy(pOutBuf+loop*USBKEY_FILE_MAX_BLOCK_LEN,response,respLen-2);
    AK_Log(AK_LOG_DEBUG, "====>>ReadCurBinaryFile Success!");
    return USBKEY_OK;
}

//写当前选择的二进制文件
USBKEY_ULONG WriteCurBinaryFile(
        USBKEY_HANDLE		hDeviceHandle, 
        USBKEY_ULONG		ulOffset, 
        USBKEY_ULONG		ulLen,
        USBKEY_UCHAR_PTR	pInBuf)
{
    //USBKEY_UCHAR request[USBKEY_MAX_COM_BUFFER+7];
    USBKEY_UCHAR response[5];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0;
    USBKEY_UCHAR data[USBKEY_FILE_MAX_BLOCK_LEN];
    USBKEY_UCHAR p1,p2,le;
    USBKEY_INT32 lc;
    USBKEY_ULONG loop = 0,offset,len,i;

    AK_Log(AK_LOG_DEBUG, "====>>WriteCurBinaryFile");

    if( ulOffset >= 0x8000 )
    {
        AK_Log(AK_LOG_ERROR, "====>>WriteCurBinaryFile->ulOffset Error,ulOffset = %#08x", ulOffset);
        return USBKEY_GENERAL_ERROR;
    }
    /*if( ulLen > USBKEY_FILE_MAX_BLOCK_LEN )
      {
      AK_Log(AK_LOG_ERROR, "====>>WriteCurBinaryFile->ulLen Error,ulLen = %#08x", ulLen);
      return USBKEY_GENERAL_ERROR;
      }*/

    //Delete by zhoushenshen in 20121109
    /*//组包
      request[0] = 0x00;		//CLA
      request[1] = USBKEY_CMD_WRITE_BINARY;	//INS
      request[2] = (USBKEY_UCHAR)((ulOffset&0xff00)>>8);	//P1
      request[3] = (USBKEY_UCHAR)(ulOffset&0xff);			//P2
      if(ulLen < 255)
      {
      request[4] = (USBKEY_UCHAR)ulLen;
      memcpy(request+5, pInBuf, ulLen);
      reqLen = 5 + ulLen;
      }
      else
      {
      request[4] = 0xff;
      request[5] = (USBKEY_UCHAR)((ulLen&0xff00)>>8);
      request[6] = (USBKEY_UCHAR)(ulLen&0xff);
      memcpy(request+7, pInBuf, ulLen);
      reqLen = 7 + ulLen;
      }

    //发送命令
    respLen = 5;
    rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
    if (rtn != 0)
    {
    AK_Log(AK_LOG_ERROR, "====>>WriteCurBinaryFile->DeviceExecCmd() Error,rtn = %#08x",rtn);
    return rtn;
    }*/
    //Del end

    //Add by zhoushenshen in 20121109
    if ( ulLen > USBKEY_FILE_MAX_BLOCK_LEN)
    {
        loop = ulLen/USBKEY_FILE_MAX_BLOCK_LEN;
    }
    offset = ulOffset;

    for (i=0; i<loop; i++)
    {
        len = offset/256;
        memcpy(&p2,&offset,1);
        memcpy(&p1,&len,1);

        lc = USBKEY_FILE_MAX_BLOCK_LEN;
        le = 0x00;
        memcpy(data,pInBuf+i*USBKEY_FILE_MAX_BLOCK_LEN,USBKEY_FILE_MAX_BLOCK_LEN);

        respLen = 5;
        rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_UpdateBinary,response,&respLen);
        if (rtn != 0)
        {
            AK_Log(AK_LOG_ERROR, "====>>WriteCurBinaryFile->PackageData() loop Error, rtn = %#08x", rtn);
            return rtn;
        }

        rtn = *(response+respLen-2);
        rtn = rtn*256 + *(response+respLen-1);
        switch(rtn)
        {
            case DEVICE_SUCCESS:
                break;
            default:
                AK_Log(AK_LOG_ERROR, "====>>WriteCurBinaryFile->DeviceExecCmd() loop Error,code = %#08x",rtn);
                return rtn;
        }

        offset+=USBKEY_FILE_MAX_BLOCK_LEN;
    }

    len = offset/256;
    memcpy(&p2,&offset,1);
    memcpy(&p1,&len,1);
    len = ulLen - loop*USBKEY_FILE_MAX_BLOCK_LEN;
    if(0 == len)
    {
        AK_Log(AK_LOG_DEBUG, "====>>WriteCurBinaryFile Success!");
        return USBKEY_OK;
    }

    lc = len;
    le = 0x00;
    memcpy(data,pInBuf+loop*USBKEY_FILE_MAX_BLOCK_LEN,len);

    respLen = 5;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_UpdateBinary,response,&respLen);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>WriteCurBinaryFile->PackageData() Error, rtn = %#08x", rtn);
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
            AK_Log(AK_LOG_ERROR, "====>>WriteCurBinaryFile->DeviceExecCmd() Error,code = %#08x",rtn);
            return rtn;
    }

    AK_Log(AK_LOG_DEBUG, "====>>WriteCurBinaryFile Success!");
    return USBKEY_OK;
}


// 获取一个空闲的容器对称密钥id
// 返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_ULONG GetFreeConSymKeyID(
        USBKEY_HANDLE		hDeviceHandle,
        USBKEY_UCHAR_PTR	pucSymKeyID,
        USBKEY_UCHAR		pcConSymKeyIDRec[USBKEY_MAX_SYM_KEY_ID])
{
    USBKEY_ULONG rtn;
    USBKEY_UCHAR pcFileID[2];
    int i;

    AK_Log(AK_LOG_DEBUG, "====>>GetFreeConSymKeyID");

    //选择对称密钥ID文件
    pcFileID[0] = (USBKEY_SYM_KEY_FILE_ID&0xff00)>>8;
    pcFileID[1] = USBKEY_SYM_KEY_FILE_ID&0x00ff;
    rtn = SelectFile(hDeviceHandle,NULL, pcFileID);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>GetFreeConSymKeyID->SelectFile() Error,rtn = %#08x",rtn);
        return rtn;
    }

    //读对称密钥ID文件
    rtn = ReadCurBinaryFile(hDeviceHandle, 0, USBKEY_SYM_KEY_ID_FILE_LEN, (USBKEY_UCHAR_PTR)pcConSymKeyIDRec);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>GetFreeConSymKeyID->ReadCurBinaryFile() Error,rtn = %#08x",rtn);
        return rtn;
    }

    //查找空闲密钥ID
    for (i = 0; i < USBKEY_MAX_SYM_KEY_ID; i++)
    {
        if( 0xff == pcConSymKeyIDRec[i] )
            break;
    }
    if( USBKEY_MAX_SYM_KEY_ID == i )
    {
        AK_Log(AK_LOG_ERROR, "====>>GetFreeConSymKeyID->No Free Sym Key ID");
        return USBKEY_MAX_NUM_OF_SYMKEY;
    }
    *pucSymKeyID = i+1;

    AK_Log(AK_LOG_DEBUG, "====>>GetFreeConSymKeyID Success!");
    return USBKEY_OK;
}

// 把一个对称密钥增加到容器
// 返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_ULONG AddContainerSymKey(
        USBKEY_HANDLE		hDeviceHandle,
        USBKEY_UCHAR		ucSymKeyID,
        USBKEY_UCHAR		ucRecNo,
        USBKEY_UCHAR		pcConSymKeyIDRec[USBKEY_MAX_SYM_KEY_ID])
{
    USBKEY_ULONG rtn;
    USBKEY_UCHAR pcFileID[2];

    AK_Log(AK_LOG_DEBUG, "====>>AddContainerSymKey");

    //选择对称密钥ID文件
    pcFileID[0] = (USBKEY_SYM_KEY_FILE_ID&0xff00)>>8;
    pcFileID[1] = USBKEY_SYM_KEY_FILE_ID&0x00ff;
    rtn = SelectFile(hDeviceHandle,NULL, pcFileID);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>AddContainerSymKey->SelectFile() Error,rtn = %#08x",rtn);
        return rtn;
    }

    pcConSymKeyIDRec[ucSymKeyID-1] = ucRecNo;

    //写对称密钥ID文件
    rtn = WriteCurBinaryFile(hDeviceHandle, 0, USBKEY_SYM_KEY_ID_FILE_LEN, (USBKEY_UCHAR_PTR)pcConSymKeyIDRec);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>AddContainerSymKey->WriteCurBinaryFile() Error,rtn = %#08x",rtn);
        return rtn;
    }

    AK_Log(AK_LOG_DEBUG, "====>>AddContainerSymKey Success!");
    return USBKEY_OK;
}

// RSA导入对称密钥，pcWrapKeyID为NULL则明文导入，否则密文导入
// 返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_ULONG RSAImportSessionKey(
        USBKEY_HANDLE		hDeviceHandle,
        USBKEY_UCHAR_PTR	pcWrapKeyID,
        USBKEY_UCHAR		pcKeyAttr[6],
        USBKEY_UCHAR_PTR	pcSymKey,	
        USBKEY_ULONG		ulSymKeyLen)
{
    //USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0;
    USBKEY_UCHAR data[300];
    //USBKEY_UCHAR p1,p2,lc,le;
    USBKEY_UCHAR p1,p2,le;
    USBKEY_INT32 lc;

    AK_Log(AK_LOG_DEBUG, "====>>RSAImportSessionKey");

    if (pcWrapKeyID == NULL)
    {
        //明文导入对称密钥组包
        p1 = 0x00;
        p2 = 0x00;
        if ((ulSymKeyLen != 8) && (ulSymKeyLen != 16) && (ulSymKeyLen != 24))
        {
            AK_Log(AK_LOG_ERROR, "====>>RSAImportSessionKey->ulSymKeyLen Error,ulSymKeyLen = %#08x", ulSymKeyLen);
            return USBKEY_PARAM_ERROR;
        }
        else
            lc = ulSymKeyLen+6;
        memcpy(data,pcKeyAttr,6);
        memcpy(data+6,pcSymKey,ulSymKeyLen);
    }
    else
    {
        //密文导入对称密钥组包
        memcpy(&p1,pcWrapKeyID,1);
        memcpy(&p2,pcWrapKeyID+1,1);
        if (ulSymKeyLen == 128)
        {
            lc = 0x86;
            memcpy(data,pcKeyAttr,6);
            memcpy(data+6,pcSymKey,ulSymKeyLen);
        }
        else if (ulSymKeyLen == 256)
        {
            //Delete by zhoushenshen in 20121109
            /*lc = 0xff;
              memset(data,0x01,1);
              memset(data+1,0x06,1);
              memcpy(data+2,pcKeyAttr,6);
              memcpy(data+8,pcSymKey,ulSymKeyLen);*/
            //Del end
            lc = 262;	//0x106
            memcpy(data,pcKeyAttr,6);
            memcpy(data+6,pcSymKey,ulSymKeyLen);
        }
        else
        {
            AK_Log(AK_LOG_ERROR, "====>>RSAImportSessionKey->ulSymKeyLen Error,ulSymKeyLen = %#08x", ulSymKeyLen);
            return USBKEY_PARAM_ERROR;
        }
    }
    le = 0x00;

    //Delete by zhoushenshen in 20121109
    /*rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_ImportDivSymmKey,request,&reqLen);
      if (rtn != 0)
      {
      AK_Log(AK_LOG_ERROR, "====>>RSAImportSessionKey->PackageData() Error,rtn = %#08x",rtn);
      return rtn;
      }

      respLen = 300;
      rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
      if (rtn != 0)
      {
      AK_Log(AK_LOG_ERROR, "====>>RSAImportSessionKey->DeviceExecCmd() Error,rtn = %#08x",rtn);
      return rtn;
      }*/
    //Del end

    //Add by zhoushenshen in 20121109
    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_ImportDivSymmKey,response,&respLen);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>RSAImportSessionKey->PackageData() Error, rtn = %#08x", rtn);
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
            AK_Log(AK_LOG_ERROR, "====>>RSAImportSessionKey->DeviceExecCmd() Error,code = %#08x",rtn);
            AK_Log(AK_LOG_ERROR, "====>>RSAImportSessionKey->KeyID = 0x%02x",pcKeyAttr[0]);
            return rtn;
    }

    AK_Log(AK_LOG_DEBUG, "====>>RSAImportSessionKey Success!");
    return USBKEY_OK;
}

// ECC加密导入对称密钥
// 返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_ULONG ECCImportSessionKey(
        USBKEY_HANDLE		hDeviceHandle,
        USBKEY_UCHAR_PTR	pcWrapKeyID,
        USBKEY_UCHAR		pcKeyAttr[6],
        USBKEY_UCHAR_PTR	pcSymKey,	
        USBKEY_ULONG		ulSymKeyLen)
{
    //USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0;
    USBKEY_UCHAR p1,p2,le;
    USBKEY_INT32 lc;
    USBKEY_UCHAR data[300];

    AK_Log(AK_LOG_DEBUG, "====>>ECCImportSessionKey");

    //Delete by zhoushenshen in 20121204
    //组包
    /*request[0] = USBKEY_CMD_TYPE_PKI;
      request[1] = USBKEY_CMD_CODE_ECC_IMPORT_SYMKEY;
      request[2] = 0x01;	//密文导入
      if ( 128 == ulSymKeyLen )
      request[3] = 0x02;//模长为256;
      else if( 192 == ulSymKeyLen )
      request[3] = 0x03;//模长为384;
      else
      {
      AK_Log(AK_LOG_ERROR, "====>>ECCImportSessionKey->ulSymKeyLen Error,ulSymKeyLen = %#08x", ulSymKeyLen);
      return USBKEY_PARAM_ERROR;
      }
      request[4] = (USBKEY_UCHAR)(8 + ulSymKeyLen + 2);
      memcpy(request+5, pcKeyAttr, 6);
      memcpy(request+13, pcSymKey, ulSymKeyLen);
      memcpy(request+13+ulSymKeyLen, pcWrapKeyID, 2);
      reqLen = 5 + request[4];

    //发送命令
    respLen = 300;
    rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
    if (rtn != 0)
    {
    AK_Log(AK_LOG_ERROR, "====>>ECCImportSessionKey->DeviceExecCmd() Error,rtn = %#08x",rtn);
    return rtn;
    }*/
    //Del end

    //Add by zhoushenshen in 20121204
    //组包
    p1 = 0x01;	//密文导入
    if ( 128 == ulSymKeyLen )
        p2 = 0x02;//模长为256;
    else if( 192 == ulSymKeyLen )
        p2 = 0x03;//模长为384;
    else
    {
        AK_Log(AK_LOG_ERROR, "====>>ECCImportSessionKey->ulSymKeyLen Error,ulSymKeyLen = %#08x", ulSymKeyLen);
        return USBKEY_PARAM_ERROR;
    }
    lc = 8 + ulSymKeyLen + 2;
    le = 0x00;
    memset(data, 0, sizeof(data));
    memcpy(data, pcKeyAttr, 6);
    memcpy(data+8, pcSymKey, ulSymKeyLen);
    memcpy(data+8+ulSymKeyLen, pcWrapKeyID, 2);

    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_ImportECCEnSymmKey,response,&respLen);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>ECCImportSessionKey->PackageData() Error, rtn = %#08x", rtn);
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
            AK_Log(AK_LOG_ERROR, "====>>ECCImportSessionKey->DeviceExecCmd() Error,code = %#08x",rtn);
            return rtn;
    }

    AK_Log(AK_LOG_DEBUG, "====>>ECCImportSessionKey Success!");
    return USBKEY_OK;
}

// RSA加密导出对称密钥
// 返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_ULONG RSAExportSessionKey(
        USBKEY_HANDLE		hDeviceHandle,
        USBKEY_UCHAR_PTR	pcSymKeyID,
        USBKEY_UCHAR_PTR	pcWrapKeyID,
        USBKEY_UCHAR_PTR	pcSymKey,		
        USBKEY_ULONG_PTR	pulSymKeyLen)	
{
    //USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0;
    USBKEY_UCHAR data[24];
    USBKEY_UCHAR p1,p2,lc,le;

    AK_Log(AK_LOG_DEBUG, "====>>RSAExportSessionKey");

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
        AK_Log(AK_LOG_ERROR, "====>>RSAExportSessionKey->pcSymKeyID Error,pcSymKeyID = %#08x", *pcSymKeyID);
        return USBKEY_PARAM_ERROR;
    }
    p2 = *(pcSymKeyID+1);
    lc = 0x02;
    le = 0x00;

    memset(data,0,sizeof(data));
    memcpy(data,pcWrapKeyID,2);

    //Delete by zhoushenshen in 20121116
    /*rtn = PackageData(p1,p2,lc,data,le,USBKEY_OP_ExportDivSymmKey,request,&reqLen);
      if (rtn != 0)
      {
      AK_Log(AK_LOG_ERROR, "====>>RSAExportSessionKey->PackageData() Error,rtn = %#08x",rtn);
      return rtn;
      }

      respLen = 300;
      rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
      if (rtn != 0)
      {
      AK_Log(AK_LOG_ERROR, "====>>RSAExportSessionKey->DeviceExecCmd() Error,rtn = %#08x",rtn);
      return rtn;
      }*/
    //Del end

    //Add by zhoushenshen in 20121116
    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_ExportDivSymmKey,response,&respLen);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>RSAExportSessionKey->PackageData() Error, rtn = %#08x", rtn);
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
            AK_Log(AK_LOG_ERROR, "====>>RSAExportSessionKey->DeviceExecCmd() Error,code = %#08x",rtn);
            return rtn;
    }

    memcpy(pcSymKey, response+6, respLen-8);	//不拷贝密钥属性
    *pulSymKeyLen = respLen - 8;

    AK_Log(AK_LOG_DEBUG, "====>>RSAExportSessionKey Success!");
    return USBKEY_OK;
}

// ECC加密导出对称密钥
// 返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_ULONG ECCExportSessionKey(
        USBKEY_HANDLE		hDeviceHandle,
        USBKEY_UCHAR_PTR	pcSymKeyID,
        USBKEY_UCHAR_PTR	pcWrapKeyID,
        USBKEY_UCHAR_PTR	pcSymKey,		
        USBKEY_ULONG_PTR	pulSymKeyLen)	
{
    //USBKEY_UCHAR request[300];
    USBKEY_UCHAR response[300];
    //USBKEY_ULONG reqLen;
    USBKEY_ULONG rtn,respLen=0;
    USBKEY_UCHAR head[4];
    USBKEY_UCHAR head256[4] = {0x00, 0x00, 0x01, 0x00};
    USBKEY_UCHAR head384[4] = {0x00, 0x00, 0x01, 0x80};
    USBKEY_ULONG ulBits;
    USBKEY_UCHAR data[300];
    USBKEY_UCHAR p1,p2;
    USBKEY_INT32 lc,le;

    AK_Log(AK_LOG_DEBUG, "====>>ECCExportSessionKey");

    memset(data, 0, 300);
    rtn = SelectFile(hDeviceHandle,NULL,pcWrapKeyID);	
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>ECCExportSessionKey->SelectFile() Error,rtn = %#08x",rtn);
        return rtn;
    }
    else
    {
        //读公钥文件的模长
        rtn = ReadCurBinaryFile(hDeviceHandle, 0, 4, head);
        if (rtn != 0)
        {

            AK_Log(AK_LOG_ERROR, "====>>ECCExportSessionKey->ReadCurBinaryFile() Error,rtn = %#08x",rtn);
            return rtn;
        }

        if(memcmp(head,head256,4) == 0)
        {
            ulBits = 256;
            //request[3] = 0x02;
            p2 = 0x02;
        }
        else if(memcmp(head,head384,4) == 0)
        {
            ulBits = 384;
            //request[3] = 0x03;
            p2 = 0x03;
        }
        else
        {
            AK_Log(AK_LOG_ERROR, "====>>ECCExportSessionKey->public file invalid! not 1024 or 2048 bits!");
            return USBKEY_GENERAL_ERROR;
        }
    }

    //Delete by zhoushenshen in 20121116
    //组包
    /*request[0] = USBKEY_CMD_TYPE_PKI;
      request[1] = USBKEY_CMD_CODE_ECC_EXPORT_SYMKEY;
      request[2] = 0x01;	//使用内部公钥加密
      request[4] = (USBKEY_UCHAR)(8 + ulBits/8 + 2);	//lc
      if (*pcSymKeyID == 0x10)
      {
      request[5] = 0x07;
      }
      else if (*pcSymKeyID == 0x11)
      {
      request[5] = 0x08;
      }
      else
      {
      AK_Log(AK_LOG_ERROR, "====>>ECCExportSessionKey->pcSymKeyID Error,pcSymKeyID = %#08x", *pcSymKeyID);
      return USBKEY_PARAM_ERROR;
      }
      request[6] = *(pcSymKeyID+1);
      memcpy(request+5+8+ulBits/8, pcWrapKeyID, 2);
      reqLen = 5 + request[4];

    //发送命令
    respLen = 300;
    rtn = DeviceExecCmd(hDeviceHandle,request,reqLen,response,(int *)&respLen);
    if (rtn != 0)
    {
    AK_Log(AK_LOG_ERROR, "====>>ECCExportSessionKey->DeviceExecCmd() Error,rtn = %#08x",rtn);
    return rtn;
    }*/
    //Del end

    //Add by zhoushenshen in 20121116
    p1 = 0x01;		//使用内部公钥加密
    lc = 8 + ulBits/8 + 2;
    le = 8 + ulBits/8*3;	//8 + 密文长度（值为3个私钥长度）

    if (*pcSymKeyID == 0x10)
    {
        data[0] = 0x07;
    }
    else if (*pcSymKeyID == 0x11)
    {
        data[0] = 0x08;
    }
    else
    {
        AK_Log(AK_LOG_ERROR, "====>>ECCExportSessionKey->pcSymKeyID Error,pcSymKeyID = %#08x", *pcSymKeyID);
        return USBKEY_PARAM_ERROR;
    }
    data[1] = *(pcSymKeyID+1);
    memcpy(data+8+ulBits/8, pcWrapKeyID, 2);

    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_ExportSymmKeybyEcc,response,&respLen);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>ECCExportSessionKey->PackageData() Error, rtn = %#08x", rtn);
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
            AK_Log(AK_LOG_ERROR, "====>>ECCExportSessionKey->DeviceExecCmd() Error,code = %#08x",rtn);
            return rtn;
    }

    memcpy(pcSymKey, response+8, respLen-10);
    *pulSymKeyLen = respLen-10;

    AK_Log(AK_LOG_DEBUG, "====>>ECCExportSessionKey Success!");
    return USBKEY_OK;
}

/**
 * @创建二进制文件
 * @返回值
 *		0：	正常.
 *		>0:	FAIL，返回的是错误码
 *	@param
 *		pcFileID filename
 */
USBKEY_ULONG CreateBinaryFile(
        USBKEY_HANDLE		hDeviceHandle,
        USBKEY_UCHAR_PTR	pcFileID,
        USBKEY_UCHAR		ucMode,
        USBKEY_UCHAR		ucReadRight,		
        USBKEY_UCHAR		ucWriteRight,
        USBKEY_ULONG		ulFileLen)	
{
    USBKEY_UCHAR response[2];
    USBKEY_ULONG rtn,respLen=0;
    USBKEY_UCHAR data[10],dataBuf[USBKEY_FILE_MAX_BLOCK_LEN];
    USBKEY_UCHAR p1,p2,lc,le;

    AK_Log(AK_LOG_DEBUG, "====>>CreateBinaryFile");

    //组包
    p1 = 0x00;						//p1
    p2 = 0x02;						//p2
    lc = 0x07;						//lc updtae in 20120106
    le = 0x00;

    memset(data,0,9);
    data[0] = pcFileID[0];
    data[1] = pcFileID[1];
    data[2] = ucMode;
    data[3] = ucReadRight;
    data[4] = ucWriteRight;
    data[5] = (USBKEY_UCHAR)((ulFileLen&0x0000ff00)>>8);
    data[6] = (USBKEY_UCHAR)(ulFileLen&0xff);

    //发送命令
    respLen = 2;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_CreateFile,response,&respLen);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>CreateBinaryFile->PackageData() Error, rtn = %#08x", rtn);
        return rtn;
    }

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_Log(AK_LOG_ERROR, "====>>CreateBinaryFile->DeviceExecCmd() Error,code = %#08x",rtn);
            return rtn;
    }

    //金融IC卡内存初始值为0，不为-1，将初始值设为-1
    memset(dataBuf,0xff,sizeof(dataBuf));
    rtn = WriteCurBinaryFile(hDeviceHandle, 0, ulFileLen, (USBKEY_UCHAR_PTR)dataBuf);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>CreateBinaryFile->WriteCurBinaryFile() Error,rtn = %#08x",rtn);
        return rtn;
    }

    AK_Log(AK_LOG_DEBUG, "====>>CreateBinaryFile Success!");
    return USBKEY_OK;
}


//查找税控对称密钥
USBKEY_ULONG SearchTaxSymmetricKey(USBKEY_HANDLE hDeviceHandle,USBKEY_UCHAR_PTR pucTaxKeyID_0,USBKEY_UCHAR_PTR pucTaxKeyID_1)
{
    USBKEY_ULONG rtn;
    USBKEY_UCHAR pcConSymKeyIDRec[USBKEY_MAX_SYM_KEY_ID];
    USBKEY_UCHAR pcFileID[2],pKeyID[2];
    int i,j;

    AK_Log(AK_LOG_DEBUG, "====>>SearchTaxSymmetricKey");

    //选择对称密钥ID文件
    pcFileID[0] = (USBKEY_SYM_KEY_FILE_ID&0xff00)>>8;
    pcFileID[1] = USBKEY_SYM_KEY_FILE_ID&0x00ff;
    rtn = SelectFile(hDeviceHandle,NULL, pcFileID);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>SearchTaxSymmetricKey->SelectFile() Error,rtn = %#08x",rtn);
        return rtn;
    }

    //读对称密钥ID文件
    rtn = ReadCurBinaryFile(hDeviceHandle, 0, USBKEY_SYM_KEY_ID_FILE_LEN, pcConSymKeyIDRec);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>SearchTaxSymmetricKey->ReadCurBinaryFile() Error,rtn = %#08x",rtn);
        return rtn;
    }

    j = 0;
    for(i=0; i<USBKEY_MAX_SYM_KEY_ID; i++)
    {
        if(pcConSymKeyIDRec[i] == TAX_KEY_FLAG)		//如果容器号为0
        {
            pKeyID[j] = i + 1;
            j++;		
        }
    }

    if(j != 0 && j != 2)		//如果税控密钥个数不为2，无税控密钥
    {
        AK_Log(AK_LOG_ERROR, "====>>SearchTaxSymmetricKey->Tax Key Number Error, num = %#08x", j);
        return USBKEY_KEY_ID_ERROR;
    }

    if(j != 0)
    {
        if(pucTaxKeyID_0)
            *pucTaxKeyID_0 = pKeyID[0];
        if(pucTaxKeyID_1)
            *pucTaxKeyID_1 = pKeyID[1];
    }
    else
    {
        if(pucTaxKeyID_0)
            *pucTaxKeyID_0 = -1;
        if(pucTaxKeyID_1)
            *pucTaxKeyID_1 = -1;
    }

    AK_Log(AK_LOG_DEBUG, "====>>SearchTaxSymmetricKey Success!");
    return USBKEY_OK;
}


// 获取税控密钥ID
// 返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_ULONG GetFreeTaxKeyID(
        USBKEY_HANDLE		hDeviceHandle,
        USBKEY_UCHAR_PTR	pucTaxKeyID_0,
        USBKEY_UCHAR_PTR	pucTaxKeyID_1,
        USBKEY_UCHAR		pcConSymKeyIDRec[USBKEY_MAX_SYM_KEY_ID])
{
    USBKEY_ULONG rtn;
    USBKEY_UCHAR pcFileID[2];
    int i,j;

    AK_Log(AK_LOG_DEBUG, "====>>GetFreeTaxKeyID");

    //选择对称密钥ID文件
    pcFileID[0] = (USBKEY_SYM_KEY_FILE_ID&0xff00)>>8;
    pcFileID[1] = USBKEY_SYM_KEY_FILE_ID&0x00ff;
    rtn = SelectFile(hDeviceHandle,NULL, pcFileID);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>GetFreeTaxKeyID->SelectFile() Error,rtn = %#08x",rtn);
        return rtn;
    }

    //读对称密钥ID文件
    rtn = ReadCurBinaryFile(hDeviceHandle, 0, USBKEY_SYM_KEY_ID_FILE_LEN, (USBKEY_UCHAR_PTR)pcConSymKeyIDRec);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>GetFreeTaxKeyID->ReadCurBinaryFile() Error,rtn = %#08x",rtn);
        return rtn;
    }

    //查找空闲密钥ID
    for (i = 0; i < USBKEY_MAX_SYM_KEY_ID; i++)
    {
        if( 0xff == pcConSymKeyIDRec[i] )
            break;
    }
    if( USBKEY_MAX_SYM_KEY_ID == i )
    {
        AK_Log(AK_LOG_ERROR, "====>>GetFreeTaxKeyID->No Free Sym Key ID for Tax_0");
        return USBKEY_MAX_NUM_OF_SYMKEY;
    }
    *pucTaxKeyID_0 = i+1;

    //查找下一个空闲密钥ID
    for (j = i+1; j < USBKEY_MAX_SYM_KEY_ID; j++)
    {
        if( 0xff == pcConSymKeyIDRec[j] )
            break;
    }
    if( USBKEY_MAX_SYM_KEY_ID == j )
    {
        AK_Log(AK_LOG_ERROR, "====>>GetFreeTaxKeyID->No Free Sym Key ID for Tax_0");
        return USBKEY_MAX_NUM_OF_SYMKEY;
    }

    *pucTaxKeyID_1 = j+1;

    AK_Log(AK_LOG_DEBUG, "====>>GetFreeTaxKeyID Success!");
    return USBKEY_OK;
}


// 把税控对称密钥增加到密钥文件
// 返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_ULONG AddTaxSymKey(
        USBKEY_HANDLE		hDeviceHandle,
        USBKEY_UCHAR		ucTaxKeyID_0,
        USBKEY_UCHAR		ucTaxKeyID_1,
        USBKEY_UCHAR		pcConSymKeyIDRec[USBKEY_MAX_SYM_KEY_ID])
{
    USBKEY_ULONG rtn;
    USBKEY_UCHAR pcFileID[2];

    AK_Log(AK_LOG_DEBUG, "====>>AddTaxSymKey");

    //选择对称密钥ID文件
    pcFileID[0] = (USBKEY_SYM_KEY_FILE_ID&0xff00)>>8;
    pcFileID[1] = USBKEY_SYM_KEY_FILE_ID&0x00ff;
    rtn = SelectFile(hDeviceHandle,NULL, pcFileID);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>AddTaxSymKey->SelectFile() Error,rtn = %#08x",rtn);
        return rtn;
    }

    //读对称密钥ID文件
    AK_Log(AK_LOG_ERROR, "====>>AddTaxSymKey->ReadCurBinaryFile() success");
    rtn = ReadCurBinaryFile(hDeviceHandle, 0, USBKEY_SYM_KEY_ID_FILE_LEN, (USBKEY_UCHAR_PTR)pcConSymKeyIDRec);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>AddTaxSymKey->ReadCurBinaryFile() Error,rtn = %#08x",rtn);
        return rtn;
    }

    pcConSymKeyIDRec[ucTaxKeyID_0-1] = TAX_KEY_FLAG;
    pcConSymKeyIDRec[ucTaxKeyID_1-1] = TAX_KEY_FLAG;

    //写对称密钥ID文件
    rtn = WriteCurBinaryFile(hDeviceHandle, 0, USBKEY_SYM_KEY_ID_FILE_LEN, (USBKEY_UCHAR_PTR)pcConSymKeyIDRec);
    if (rtn != 0)
    {
        AK_Log(AK_LOG_ERROR, "====>>AddTaxSymKey->WriteCurBinaryFile() Error,rtn = %#08x",rtn);
        return rtn;
    }

    AK_Log(AK_LOG_DEBUG, "====>>AddTaxSymKey Success!");
    return USBKEY_OK;
}


// 创建应用PIN码
// 返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_ULONG CreateAppPin(
        USBKEY_HANDLE		hDeviceHandle,	//[in]设备句柄
        USBKEY_UCHAR_PTR	pucPIN,			//[in]送入的PIN
        USBKEY_ULONG		ulPINLen,		//[in]送入的PIN长度
        USBKEY_UCHAR		ucPinFlag,		//[in]PIN码标识
        USBKEY_UCHAR		ucSecState,		//[in]PIN验证后安全状态
        USBKEY_UCHAR		ucRetryCounts,	//[in]重试次数
        USBKEY_UCHAR		ulUseRights,	//[in]使用权限
        USBKEY_UCHAR		ulChangeRights)	//[in]更改权限
{
    USBKEY_UCHAR response[300];
    USBKEY_ULONG rtn,respLen=300;
    USBKEY_UCHAR p1,p2,lc,le;
    USBKEY_UCHAR_PTR data;

    AK_Log(AK_LOG_DEBUG, "====>>CreateAppPin");

    p1 = 0x00;
    p2 = 0x00;

    //生成数据

    lc = (USBKEY_UCHAR)(6+ulPINLen);
    data = (USBKEY_UCHAR_PTR)malloc(lc);

    memcpy(data,&ucPinFlag,1);	//设置PIN标识
    //le = 0x05;				//设置密钥类型，PIN为5
    le = 0x02;					//命令修改 02代表PIN
    memcpy(data+1,&le,1);
    memcpy(data+2,&ucSecState,1);	//设置获得状态
    memcpy(data+3,&ucRetryCounts,1);//设置重试次数
    le = ulUseRights;				//设置使用权限
    memcpy(data+4,&le,1);
    le = ulChangeRights;			//设置更改权限
    memcpy(data+5,&le,1);
    le = 0x00;

    memcpy(data+6,pucPIN,ulPINLen);

    respLen = 300;
    rtn = PackageData(hDeviceHandle,p1,p2,lc,data,le,USBKEY_OP_WriteKey,response,&respLen);
    if (rtn != 0)
    {
        free(data);
        AK_Log(AK_LOG_ERROR, "CreateAppPin ==> PackageData() Error, rtn = %#08x", rtn);
        return rtn;
    }

    rtn = *(response+respLen-2);
    rtn = rtn*256 + *(response+respLen-1);
    switch(rtn)
    {
        case DEVICE_SUCCESS:
            break;
        default:
            AK_Log(AK_LOG_ERROR, "CreateAppPin ==> DeviceExecCmd() Error,code = %#08x", rtn);
            return rtn;
            break;
    }

    AK_Log(AK_LOG_DEBUG, "====>>CreateAppPin Success!");
    return USBKEY_OK;
}
