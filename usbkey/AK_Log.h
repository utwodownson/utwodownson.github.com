/*
  �ļ�����: AKP11_Log.h
  Copyright (c) 2009 Aerospace Information. Co., Ltd.
  ��	��: heli@aisino.com
  ��������: 2009-3-19
  ��	��: ����д��־����
*/

#ifndef _AIP11_LOG_H_
#define _AIP11_LOG_H_

//���������
//#define LOG_TO_STDOUT				//д��־����׼���
#define LOG_TO_FILE					//д��־���ļ�

#define AK_MAX_BUFFER_SIZE			4096

#define AK_LOG_FILE_NAME			"c:\\log.log"

#define AK_LOG_LEVEL				2		//0������¼��־��1��ֻ��¼������־��2��������־��dubug��Ϣ����¼
#define AK_LOG_ERROR				1
#define AK_LOG_DEBUG				2

void AK_Log(int type, const char *fmt, ...);


#endif