/*	global.h
Author  :   caizhun@aisino.com
CO.     :   AISINO
date    :   2008��5��26�գ�
����    :   ͨ��ȫ�ֺ궨��	*/

#ifndef _GLOBAL_H
#define _GLOBAL_H

//����ϵͳ
#define _USBKEY_WIN32_
#define _USBKEY_API_DLL_

//Debug��Ϣ
//#define _USBKEY_DEBUG_INFO_
//#define _USBKEY_DEBUG_FILE_ "c:\\log.log"

//оƬ���ͺ궨��
//#define AKEY_45CHIP
//#define AKEY_AC4CHIP
#define AKEY_AC3CHIP

//�����궨��
#define _USBKEY_INIT

//��˰����˰��Ӧ�ú궨��
//#define _SEC_GTAX_APP

//�Ͳ���ƱӦ�ã��������������ʹ�ò�ͬ��˰�ضԳ���Կ��
//#define _ATAX_APP

//���Ӳ�Ʒ�ȷǹ����ն��û�
//#define _USBKEY_UNFORMAT

//�豸����ԱPIN
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