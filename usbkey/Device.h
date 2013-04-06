/*	Device.h
	Author  :   yangdayong@aisino.com
	CO.     :   AISINO
	date    :   2008年5月19日，
	描述    :   硬件操作接口，杨大勇设计并实现	*/

#ifndef _DEVICE_H
#define _DEVICE_H


///////////////////////////////////////////////////////////////////////////////
//	include区域
///////////////////////////////////////////////////////////////////////////////
//#include "windows.h"

///////////////////////////////////////////////////////////////////////////////
//	错误码定义
///////////////////////////////////////////////////////////////////////////////
#define HANDLEERROR     -1;
#define DATALENGTHERROR 99;

///////////////////////////////////////////////////////////////////////////////
//	环境宏定义
///////////////////////////////////////////////////////////////////////////////
#define	DEVICE_MAX_QUANTITY	10	//最大设备数量
#define DEVICE_ID_MAX_LEN	255	//设备ID最大长度
#define CDB_NO_DATABUF	    0	//cbd命令没有数据域
#define CDB_HAS_DATABUF	    1	//cbd命令有数据域


#ifdef __cplusplus
extern "C" {
#endif

//	枚举计算机上所连接的设备的ID
//	如果ppcDeviceIdS为NULL，则从pdwDeviceQuantity返回需要分配char *[DEVICE_ID_MAX_LEN]数量
//	如果ppcDeviceIdS非空，则从ppcDeviceIdS返回连接设备的ID，
//		并从pdwDeviceQuantity返回实际设备个数
//	返回值：
//		0：	OK
//		>0:	FAIL，返回的是错误码
 __declspec(dllexport) UCHAR DeviceEnum(
	char	**ppcDeviceIdS,		//[out]设备ID的列表
	int	*pucDeviceQuantity);	//[out]设备ID的个数

//	打开某个设备
//	如果ucShareMode=1 表示共享模式 其它应用程序可以访问设备
//	返回值：
//		0：	OK
//		>0:	FAIL，返回的是错误码
__declspec(dllexport) DWORD DeviceOpen(
	char    *pcDeviceId,		//[in]设备的ID
	UCHAR	ucShareMode,		//[in]共享模式
	HANDLE	*phDeviceHandle);	//[out]设备句柄

//	关闭某个设备
//	返回值：
//		0：	OK
//		>0:	FAIL，返回的是错误码
 __declspec(dllexport) DWORD DeviceClose(
	HANDLE	hDeviceHandle );	//[in]设备句柄
	
//	重置当前连接的某个设备，使设备进入初始状态（设备句柄第一次刚被打开）
//	返回值：
//		0：	OK
//		>0:	FAIL，返回的是错误码
 __declspec(dllexport) DWORD DeviceReset(
	HANDLE	hDeviceHandle,		//[in]设备句柄
	BYTE	*pbCmd,			//[in]复位操作命令
	DWORD	dwCmdLen );		//[in]命令长度
	
//	让当前连接的某个设备执行某个命令
//	返回值：
//		0：	OK
//		>0:	FAIL，返回的是错误码
 __declspec(dllexport) DWORD DeviceExecCmd(
	HANDLE	hDeviceHandle,		//[in]设备句柄
	BYTE	*pbCmd,			//[in]命令
	int	dwCmdLen,		//[in]命令长度
	BYTE	*pbResult,		//[out]结果
	int	*pdwResultLen);		//[out]结果长度

//	获取当前连接的某个设备的状态.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
 __declspec(dllexport) DWORD DeviceGetStat(
	HANDLE hDeviceHandle);		//[in]设备句柄

#ifdef __cplusplus
}
#endif

#endif