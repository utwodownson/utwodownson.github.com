/*
  文件名称: AKP11_Log.h
  Copyright (c) 2009 Aerospace Information. Co., Ltd.
  作	者: heli@aisino.com
  创建日期: 2009-3-19
  描	述: 定义写日志函数
*/

#ifndef _AIP11_LOG_H_
#define _AIP11_LOG_H_

//条件编译宏
//#define LOG_TO_STDOUT				//写日志到标准输出
#define LOG_TO_FILE					//写日志到文件

#define AK_MAX_BUFFER_SIZE			4096

#define AK_LOG_FILE_NAME			"c:\\log.log"

#define AK_LOG_LEVEL				2		//0：不记录日志；1：只记录错误日志；2：错误日志和dubug信息都记录
#define AK_LOG_ERROR				1
#define AK_LOG_DEBUG				2

void AK_Log(int type, const char *fmt, ...);


#endif