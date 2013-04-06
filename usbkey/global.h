/*	global.h
Author  :   caizhun@aisino.com
CO.     :   AISINO
date    :   2008年5月26日，
描述    :   通用全局宏定义	*/

#ifndef _GLOBAL_H
#define _GLOBAL_H

//操作系统
#define _USBKEY_WIN32_
#define _USBKEY_API_DLL_

//Debug信息
//#define _USBKEY_DEBUG_INFO_
//#define _USBKEY_DEBUG_FILE_ "c:\\log.log"

//芯片类型宏定义
//#define AKEY_45CHIP
//#define AKEY_AC4CHIP
#define AKEY_AC3CHIP

//出厂宏定义
#define _USBKEY_INIT

//金税卡报税盘应用宏定义
//#define _SEC_GTAX_APP

//淄博发票应用（添加两个与正常使用不同的税控对称密钥）
//#define _ATAX_APP

//电子产品等非管理终端用户
//#define _USBKEY_UNFORMAT

//设备管理员PIN
#ifdef _USBKEY_UNFORMAT
#define USBKEY_DEVICE_ADMIN_PIN		"key123456"
#else
//#define USBKEY_DEVICE_ADMIN_PIN		"Ai1@$USB6#&Key./"
#define USBKEY_DEVICE_ADMIN_PIN		"123456"
#endif


extern unsigned char gDeviceType;

extern HHOOK g_hHook;  
extern HANDLE g_mutext;
extern int g_DevNum;

extern HANDLE g_hDevEvent;
extern HINSTANCE g_hModule;

#endif