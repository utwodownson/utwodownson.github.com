/*
  �ļ�����: pkcs5_padding.h
  Copyright (c) 2008 Aerospace Information. Co., Ltd.
  ��	��: heli
  ��������: 2009-06-04
  ��	��: ʵ��PKCS5���ģʽ
*/

#ifndef _PKCS5_PADDING_H
#define _PKCS5_PADDING_H

//==============================================================
//��������:		pkcs5_padding
//��������:		pucData[in/out]:	in:ԭʼ���ݣ�out:���������
//				pulDataLen[in/out]:	in:ԭʼ���ݳ��ȣ�out:��������ݳ���
//				ulBlockLen[in]:		���鳤��
//����ֵ:		0:	�ɹ�
//				~0: ʧ��
//��������: 	�����ݽ���PKCS5���
//==============================================================
int pkcs5_padding(unsigned char* pucData, unsigned long* pulDataLen, unsigned long ulBlockLen);

//==============================================================
//��������:		pkcs5_padding
//��������:		pucData[in]:	in:��������
//				pulDataLen[in/out]:	in:�������ݳ��ȣ�out:ȥ���������ݳ���
//����ֵ:		0:	�ɹ�
//				~0: ʧ��
//��������: 	�����ݵ�PKCS5���ȥ��
//==============================================================
int pkcs5_no_padding(unsigned char* pucData, unsigned long* pulDataLen);

#endif