/*
  Name: pkcs1_padding.c
  Copyright (c) 2008 Aerospace Information. Co., Ltd.
  Author: heli, zhoushenshen
  Date: 2008-9-19
  Description: 实现pkcs1填充
*/
#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "pkcs1_padding.h"
#define random(x) (rand()%x)

//00||BT||PS||00||DATA
//私钥运算BT=0x01，PS全为0xff
//公钥运算BT=0x02，PS为随机数
//成功返回0，失败返回1
int rsa_pkcs1_padding(
		unsigned char 	*pucData,
		unsigned long 	ulDataLen,
		unsigned long 	ulRSASize,
		unsigned long 	ulPaddingType)
{
	unsigned char  ps[256];
	unsigned long  pslen;
	unsigned int i;

	if (ulDataLen > ulRSASize - 11)
	{
		return 1;
	}

	pslen = ulRSASize - ulDataLen - 3;
	
	ps[0] = 0x00;
	switch (ulPaddingType)
	{
	case PKCS1_PADDING_TYPE1:
		ps[1] = 0x01;
		memset(ps+2, 0xff, pslen);
		break;
	case PKCS1_PADDING_TYPE2:
		ps[1] = 0x02;
		srand((int)time(0));
     	for(i=0; i<pslen; i++)
     	{
           ps[i+2] = random(256);
        }
		break;
	default: 
		return 2;
	}

	ps[2+pslen] = 0x00;
	memcpy(ps+3+pslen, pucData, ulDataLen);
	memcpy(pucData, ps, ulRSASize);
	
	return 0 ;
}


int rsa_pkcs1_No_padding(
		unsigned char 	*pucData,
		unsigned long 		*pulDataLen,
		unsigned long 	ulRSASize)
{
	unsigned char *pTemp;
	unsigned long i;
	
	pTemp = pucData;
	switch(pTemp[1])
	{
	case PKCS1_PADDING_TYPE1:
		for(i = 2; i < ulRSASize-1; i++)
		{
			if(0xff != pTemp[i])
				break;
		}
		
		if(0 != pTemp[i++])
			return 1;
		break;	
	case PKCS1_PADDING_TYPE2:
		for(i = 2; i < ulRSASize-1; i++)
		{
			if(0x00 == pTemp[i] )
				break;			
		}
		i++;
		break;
	default:
		return 2;
	}

	if(*pulDataLen < (ulRSASize - i) )
		return 1;
	
	*pulDataLen = ulRSASize - i;
	if(*pulDataLen + 11 > ulRSASize)
		return 1;
	memcpy(pucData, pTemp+i, *pulDataLen);

	return 0;
}