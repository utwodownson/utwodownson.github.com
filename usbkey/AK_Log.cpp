/*
   文件名称: AKP11_Log.cpp
   Copyright (c) 2009 Aerospace Information. Co., Ltd.
   作	者: heli@aisino.com
   创建日期: 2009-3-19
   描	述: 定义写日志函数
 */

#include "stdafx.h"
#include "./inc/AK_Log.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>

//[20080913 22:14:59 debug]	msg
//[20080913 22:14:59 error]	msg
void AK_Log(int type, const char *fmt, ...)
{ 
    va_list pvar;
    char buffer[AK_MAX_BUFFER_SIZE];
    char *pTmp;
    FILE *fp;
    time_t timep;
    struct tm *ptm;

    //检查参数合法性
    if( AK_LOG_ERROR != type && AK_LOG_DEBUG != type )
        return;

    //判断是否需要写日志
    if( type > AK_LOG_LEVEL )
        return;

    pTmp = buffer;

    time(&timep);
    ptm = localtime(&timep); //取得当地时间

    if( AK_LOG_ERROR == type )
        sprintf(pTmp, "[%04d%02d%02d %02d:%02d:%02d error]	", 1900+ptm->tm_year, 1+ptm->tm_mon, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
    else
        sprintf(pTmp, "[%04d%02d%02d %02d:%02d:%02d debug]	", 1900+ptm->tm_year, 1+ptm->tm_mon, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

    pTmp += strlen(buffer);

    va_start(pvar, fmt);
    vsprintf(pTmp, fmt, pvar);
    va_end(pvar);

    //写日志到标准输出
#ifdef LOG_TO_STDOUT
    printf(buffer);
    printf("\n");
#endif


    //写日志到文件
#ifdef LOG_TO_FILE
    fp = fopen(AK_LOG_FILE_NAME, "a+");
    if(fp == NULL){
        return;
    }
    fprintf(fp, buffer);
    fprintf(fp, "\n");
    fclose(fp);
#endif

}

