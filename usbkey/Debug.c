/*	Debug.c
	Author  :   caizhun@aisino.com
	CO.     :   AISINO
	date    :   2008年6月6日，
	描述    :   Debug 信息 */

#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>

#include "../inc/UsbKeyAPI.h"
#include "../inc/global.h"


void LogMessage(USBKEY_UCHAR_PTR s)
{
	SYSTEMTIME time;
	FILE *fp;
#ifdef _USBKEY_DEBUG_INFO_
	fp = fopen(_USBKEY_DEBUG_FILE_,"a+");
	if(fp == NULL){
		return;
	}

	GetLocalTime(&time);
	fprintf(fp,"%d年%d月%d日  %d:%d:%d  %s\n",time.wYear,time.wMonth,time.wDay,time.wHour,time.wMinute,time.wSecond,s);
	//fprintf(fp,"%s\n",s);
	fclose(fp);
#endif

	return;

}

void LogMessageVal(USBKEY_UCHAR_PTR s , USBKEY_ULONG d)
{
	SYSTEMTIME time;
	FILE *fp;
#ifdef _USBKEY_DEBUG_INFO_
	fp = fopen(_USBKEY_DEBUG_FILE_,"a+");
	if(fp == NULL){
		return;
	}

	GetLocalTime(&time);
	fprintf(fp,"%d年%d月%d日  %d:%d:%d  %s",time.wYear,time.wMonth,time.wDay,time.wHour,time.wMinute,time.wSecond,s);
	fprintf(fp,"%d .\n",d);
	fclose(fp);
#endif
	return;
}

void LogMessage16Val(USBKEY_UCHAR_PTR s , USBKEY_ULONG d)
{
	SYSTEMTIME time;
	FILE *fp;
#ifdef _USBKEY_DEBUG_INFO_
	fp = fopen(_USBKEY_DEBUG_FILE_,"a+");
	if(fp == NULL){
		return;
	}

	GetLocalTime(&time);
	fprintf(fp,"%d年%d月%d日  %d:%d:%d  %s",time.wYear,time.wMonth,time.wDay,time.wHour,time.wMinute,time.wSecond,s);
	fprintf(fp,"%04x .\n",d);
	fclose(fp);
#endif

	return;
}