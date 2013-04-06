/*
  文件名称: UsbKeyComDLL.h
  Copyright (c) 2008 Aerospace Information. Co., Ltd.
  作	者: heli
  创建日期: 2009-05-12
  描	述: 实现USBKey的PC端通信函数
*/

#ifndef _USBKEYCOMDLL_H
#define _USBKEYCOMDLL_H

#include <windows.h>

// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 USBKEYCOMDLL_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// USBKEYCOMDLL_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifdef USBKEYCOMDLL_EXPORTS
#define USBKEYCOMDLL_API __declspec(dllexport)
#else
#define USBKEYCOMDLL_API __declspec(dllimport)
#endif

//=================================================错误码定义==================================================
//1到994为windows的GetLastError()返回的错误码
#define AKCOM_OK							0					//操作成功
#define AKCOM_GENERAL_ERROR					0x10000001			//通用错误
#define AKCOM_PARAM_ERROR					0x10000002			//参数错误
#define AKCOM_BUFFER_TOO_SMALL				0x10000003			//输出缓冲区太小
#define AKCOM_LOCK_ERROR					0x10000004			//加锁失败
#define AKCOM_GET_DEVICE_NAME_ERROR			0x10000005			//获取设备名称失败
#define AKCOM_CREATE_DEVICE_HANDLE_ERROR	0x10000006			//创建设备句柄失败
#define AKCOM_GET_DEVICE_INFO_ERROR			0x10000007			//获取设备信息失败
#define AKCOM_CREATE_LOCK_ERROR				0x10000008			//创建锁失败
#define AKCOM_UNLOCK_ERROR					0x10000009			//解锁失败

//=================================================常量定义====================================================
#define MAX_DEVICE_NUM						10	
#define AK_IOCTL_DATA_OUT					0
#define AK_IOCTL_DATA_IN					1
#define MAX_SEND_PACKAGE					2048
#define DEVICE_TYPE_USBKEY					1
#define	DEVICE_TYPE_CARD					2
#define CARD_MARK					"Reader:Smart Card"
#define USB_MARK					"Aision:UsbKey Com"

#ifdef __cplusplus
	extern "C" {
#endif


//=======================================================================================
//函数名称:	DeviceEnum
//函数参数:	ppcDeviceIdS[out]				设备ID列表
//				pucDeviceQuantity[out]          设备数
//					
//返回值:		0  :ok
//函数功能: 	枚举设备，如果ppcDeviceIdS为NULL，pucDeviceQuantity返回当前连接的设备数
//=========================================================================================		
USBKEYCOMDLL_API DWORD DeviceEnum(
	char ppcDeviceIdS[MAX_DEVICE_NUM][MAX_PATH],		//[out]设备ID列表
	int	*pucDeviceQuantity);	//[out]设备数


//==============================================================
//函数名称:	DeviceOpen
//函数参数:	pcDeviceId:   //[in]设备ID
//				ucShareMode:  //[in]共享模式
//				phDeviceHandle://[out]设备句柄
//				char	*pcDeviceType		//设备类型
//返回值:		0:	成功
//				~0: 失败
//函数功能: 	打开设备
//==============================================================
USBKEYCOMDLL_API DWORD DeviceOpen(
	char    *pcDeviceId,		//[in]设备的ID
	UCHAR	ucShareMode,		//[in]共享模式
	HANDLE	*phDeviceHandle,    //[out]设备句柄
	BYTE	*pcDeviceType		//设备类型
	);	

//==============================================================
//函数名称:	GetDeviceList
//函数参数:	pHDevList[out]: 设备句柄列表
//				piDevNum[out]: 设备数
//返回值:		0:	成功
//				~0: 失败
//函数功能: 	取设备句柄列表
//==============================================================
USBKEYCOMDLL_API DWORD GetDeviceList(HANDLE *pHDevList, int *piDevNum,BYTE *type);

//==============================================================
//函数名称:	DeviceExecCmd
//函数参数:	hDeviceHandle[in]:		设备句柄
//				pbCmd[in]:				命令
//				dwCmdLen[in]:			命令长度
//				pbResult[out]:			响应
//				pdwResultLen[in/out]:	in: 响应buffer的长度，out: 响应长度
//返回值:		0:	成功
//				~0: 失败
//函数功能: 	让当前连接的某个设备执行某个命令
//==============================================================
USBKEYCOMDLL_API DWORD DeviceExecCmd(
	HANDLE	hDeviceHandle,	
	BYTE	*pbCmd,			
	int		dwCmdLen,		
	BYTE	*pbResult,		
	int		*pdwResultLen);

//==============================================================
//函数名称:		DeviceClose
//函数参数:		hDeviceHandle[in]:	设备句柄
//返回值:		0:	成功
//				~0: 失败
//函数功能: 	关闭设备句柄
//==============================================================
USBKEYCOMDLL_API DWORD DeviceClose(HANDLE  hDeviceHandle,BYTE pcDeviceType);

//==============================================================
//函数名称:	SendDeviceCmd
//函数参数:	hDeviceHandle[in]:				设备句柄
//				pbCDB[in]:						cdb命令
//				dwCDBLen[in]:					cdb命令长度
//				dwDataTransferLength[in]:		要发送/接收的数据长度
//				pbDataBuffer[out]:				发送/接收数据buffer
//				pdwRetLen[out]:					实际发送/接收数据的长度
//				pbStatusCode[out]:				接收到的状态码，0：成功，其他为错误码
//				bTransferDir[in]:				传输方向，AK_IOCTL_DATA_IN: 从设备读数据，AK_IOCTL_DATA_OUT: 写数据到设备	
//返回值:		0:	成功
//				~0: 失败
//函数功能: 	发送命令
//==============================================================
USBKEYCOMDLL_API DWORD SendDeviceCmd(
			HANDLE		hDeviceHandle,
			BYTE		*pbCDB,
			UCHAR		cdbLen,
			DWORD		dwDataTransferLength,
			BYTE		*pbDataBuffer,
			DWORD		*pdwRetLen,
			BYTE		*pbStatusCode,
			BYTE		bTransferDir);

//==============================================================
//函数名称:	DeviceExecCmdExt
//函数参数:	hDeviceHandle[in]:		设备句柄
//				pcDeviceType			设备类型
//				Cla[in]:				命令首部
//				Ins[in]:				命令首部
//				P1[in]:				    命令首部
//				P2[in]:				    命令首部
//				Lc[in]:				    数据长度
//				pCmdData[in]			命令数据
//				Le[in]:				    期望返回数据长度
//				dwCmdLen[in]:			命令长度
//				pbResult[out]:			响应
//				pdwResultLen[in/out]:	in: 响应buffer的长度，out: 响应长度
//返回值:		0:	成功
//				~0: 失败
//函数功能: 	让当前连接的某个设备执行某个命令
//==============================================================
USBKEYCOMDLL_API DWORD DeviceExecCmdExt(
			HANDLE hDeviceHandle,
			BYTE pcDeviceType,
			BYTE Cla,
			BYTE Ins,
			BYTE P1,
			BYTE P2,
			unsigned int Lc,
			BYTE *pCmdData,
			unsigned int Le,
			BYTE *pbResult,
			int *pdwResultLen
			);

#ifdef __cplusplus
	}
#endif

#endif