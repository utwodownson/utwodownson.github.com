/*
   �ļ�����: pkcs5_padding.cpp
   Copyright (c) 2008 Aerospace Information. Co., Ltd.
   ��	��: heli
   ��������: 2009-06-04
   ��	��: ʵ��PKCS5���ģʽ
 */

#include "stdafx.h"


//==============================================================
//��������:		pkcs5_padding
//��������:		pucData[in/out]:	in:ԭʼ���ݣ�out:���������
//				pulDataLen[in/out]:	in:ԭʼ���ݳ��ȣ�out:��������ݳ���
//				ulBlockLen[in]:		���鳤��
//����ֵ:		0:	�ɹ�
//				~0: ʧ��
//��������: 	�����ݽ���PKCS5���
//==============================================================
int pkcs5_padding(unsigned char* pucData, unsigned long* pulDataLen, unsigned long ulBlockLen)
{
    unsigned long ulPadNum;
    int i;

    ulPadNum = ulBlockLen - *pulDataLen%ulBlockLen;	//����ֵ�����ĸ���
    for(i=0; i<ulPadNum; i++)
    {
        pucData[i+*pulDataLen] = ulPadNum;
    }
    *pulDataLen = *pulDataLen + ulPadNum;

    return 0;
}

//==============================================================
//��������:		pkcs5_no_padding
//��������:		pucData[in]:	in:��������
//				pulDataLen[in/out]:	in:�������ݳ��ȣ�out:ȥ���������ݳ���
//����ֵ:		0:	�ɹ�
//				~0: ʧ��
//��������: 	�����ݵ�PKCS5���ȥ��
//==============================================================
int pkcs5_no_padding(unsigned char* pucData, unsigned long* pulDataLen)
{
    //unsigned long ulPadNum;

    if( pucData[*pulDataLen-1] >= *pulDataLen )
        return 1;

    *pulDataLen = *pulDataLen - pucData[*pulDataLen-1];

    return 0;
}
