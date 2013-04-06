/*
  Name: pkcs1_padding.h
  Copyright (c) 2008 Aerospace Information. Co., Ltd.
  Author: heli
  Date: 2008-9-19
  Description: ʵ��pkcs1���
*/

#ifndef _PKCS1_PADDING_H_
#define _PKCS1_PADDING_H_

#define PKCS1_PADDING_TYPE1			1	//����˽Կ�������
#define PKCS1_PADDING_TYPE2			2	//���ڹ�Կ�������

int rsa_pkcs1_padding(
		unsigned char 	*pucData,
		unsigned long 	ulDataLen,
		unsigned long 	ulRSASize,              // key->BlockLen/8
		unsigned long 	ulPaddingType);

int rsa_pkcs1_No_padding(
		unsigned char 	*pucData,
		unsigned long 		*pulDataLen,
		unsigned long 	ulRSASize);

#endif