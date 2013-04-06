/*
   �ļ�����: AKP11_Log.cpp
   Copyright (c) 2009 Aerospace Information. Co., Ltd.
   ��	��: heli@aisino.com
   ��������: 2009-3-19
   ��	��: ����д��־����
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

    //�������Ϸ���
    if( AK_LOG_ERROR != type && AK_LOG_DEBUG != type )
        return;

    //�ж��Ƿ���Ҫд��־
    if( type > AK_LOG_LEVEL )
        return;

    pTmp = buffer;

    time(&timep);
    ptm = localtime(&timep); //ȡ�õ���ʱ��

    if( AK_LOG_ERROR == type )
        sprintf(pTmp, "[%04d%02d%02d %02d:%02d:%02d error]	", 1900+ptm->tm_year, 1+ptm->tm_mon, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
    else
        sprintf(pTmp, "[%04d%02d%02d %02d:%02d:%02d debug]	", 1900+ptm->tm_year, 1+ptm->tm_mon, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

    pTmp += strlen(buffer);

    va_start(pvar, fmt);
    vsprintf(pTmp, fmt, pvar);
    va_end(pvar);

    //д��־����׼���
#ifdef LOG_TO_STDOUT
    printf(buffer);
    printf("\n");
#endif


    //д��־���ļ�
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

