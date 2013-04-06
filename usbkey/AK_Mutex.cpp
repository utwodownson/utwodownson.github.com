/*
   �ļ�����: AK_Mutex.cpp
   Copyright (c) 2009 Aerospace Information. Co., Ltd.
   ��	��: heli@aisino.com
   ��������: 2009-3-18
   ��	��: �������ڶ��̷߳��ʵĻ��⺯��
 */

#include "stdafx.h"
#include "./inc/AK_Mutex.h"
#include <windows.h>

//==============================================================
//��������:		AK_CreateMutex
//��������:		pMutexName[in]:������������Ϊ��������������ΪNULL
//				ppMutex[out]: ������
//����ֵ:	    0: �ɹ�
//				����:	ʧ��	
//��������:     ����һ��������
//==============================================================
int AK_CreateMutex(char* pMutexName, void** ppMutex)
{
    void* pMutex = NULL;

    //�������Ϸ���
    if( NULL == ppMutex )
        return 1;

    //������
    pMutex = CreateMutex(NULL, FALSE, (LPCWSTR)pMutexName);
    if (!pMutex)
    {
        return 1;
    }
    *ppMutex = pMutex;

    return 0;
}

//==============================================================
//��������:		AK_LockMutex
//��������:		pMutex[in]: ������   
//����ֵ:	    CKR_OK: �ɹ�
//				����:	ʧ��	
//��������:     ����һ��������
//==============================================================
int AK_LockMutex(void* pMutex)
{
    int rv;

    //�������Ϸ���
    if( NULL == pMutex )
        return 1;

    //�ȴ���������
    rv = WaitForSingleObject(pMutex, INFINITE);
    if (WAIT_FAILED == rv)
    {
        return 1;
    }

    return 0;

}

//==============================================================
//��������:		AK_UnlockMutex
//��������:		pMutex[in]: ������   
//����ֵ:	    CKR_OK: �ɹ�
//				����:	ʧ��	
//��������:     ����һ��������
//==============================================================
int AK_UnlockMutex(void* pMutex)
{
    //�������Ϸ���
    if( NULL == pMutex )
        return 1;

    //����
    ReleaseMutex(pMutex); 

    return 0;
}

//==============================================================
//��������:		AK_DestroyMutex
//��������:		pMutex[in]: ������   
//����ֵ:	    CKR_OK: �ɹ�
//				����:	ʧ��	
//��������:     ����һ��������
//==============================================================
int AK_DestroyMutex(void* pMutex)
{
    //�������Ϸ���
    if( NULL == pMutex )
        return 1;

    // �ͷž��
    CloseHandle(pMutex); 

    return 0;
}
