/*
  文件名称: pkcs5_padding.h
  Copyright (c) 2008 Aerospace Information. Co., Ltd.
  作	者: heli
  创建日期: 2009-06-04
  描	述: 实现PKCS5填充模式
*/

#ifndef _PKCS5_PADDING_H
#define _PKCS5_PADDING_H

//==============================================================
//函数名称:		pkcs5_padding
//函数参数:		pucData[in/out]:	in:原始数据，out:填充后的数据
//				pulDataLen[in/out]:	in:原始数据长度，out:填充后的数据长度
//				ulBlockLen[in]:		分组长度
//返回值:		0:	成功
//				~0: 失败
//函数功能: 	对数据进行PKCS5填充
//==============================================================
int pkcs5_padding(unsigned char* pucData, unsigned long* pulDataLen, unsigned long ulBlockLen);

//==============================================================
//函数名称:		pkcs5_padding
//函数参数:		pucData[in]:	in:填充后数据
//				pulDataLen[in/out]:	in:填充后数据长度，out:去掉填充的数据长度
//返回值:		0:	成功
//				~0: 失败
//函数功能: 	把数据的PKCS5填充去掉
//==============================================================
int pkcs5_no_padding(unsigned char* pucData, unsigned long* pulDataLen);

#endif