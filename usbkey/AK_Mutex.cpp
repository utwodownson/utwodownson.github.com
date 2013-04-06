/*
   文件名称: AK_Mutex.cpp
   Copyright (c) 2009 Aerospace Information. Co., Ltd.
   作	者: heli@aisino.com
   创建日期: 2009-3-18
   描	述: 定义用于多线程访问的互斥函数
 */

#include "stdafx.h"
#include "./inc/AK_Mutex.h"
#include <windows.h>

//==============================================================
//函数名称:		AK_CreateMutex
//函数参数:		pMutexName[in]:互斥量名；若为无名互斥量，则为NULL
//				ppMutex[out]: 互斥锁
//返回值:	    0: 成功
//				其他:	失败	
//函数功能:     创建一个互斥锁
//==============================================================
int AK_CreateMutex(char* pMutexName, void** ppMutex)
{
    void* pMutex = NULL;

    //检查参数合法性
    if( NULL == ppMutex )
        return 1;

    //创建锁
    pMutex = CreateMutex(NULL, FALSE, (LPCWSTR)pMutexName);
    if (!pMutex)
    {
        return 1;
    }
    *ppMutex = pMutex;

    return 0;
}

//==============================================================
//函数名称:		AK_LockMutex
//函数参数:		pMutex[in]: 互斥锁   
//返回值:	    CKR_OK: 成功
//				其他:	失败	
//函数功能:     锁定一个互斥锁
//==============================================================
int AK_LockMutex(void* pMutex)
{
    int rv;

    //检查参数合法性
    if( NULL == pMutex )
        return 1;

    //等待锁，加锁
    rv = WaitForSingleObject(pMutex, INFINITE);
    if (WAIT_FAILED == rv)
    {
        return 1;
    }

    return 0;

}

//==============================================================
//函数名称:		AK_UnlockMutex
//函数参数:		pMutex[in]: 互斥锁   
//返回值:	    CKR_OK: 成功
//				其他:	失败	
//函数功能:     解锁一个互斥锁
//==============================================================
int AK_UnlockMutex(void* pMutex)
{
    //检查参数合法性
    if( NULL == pMutex )
        return 1;

    //解锁
    ReleaseMutex(pMutex); 

    return 0;
}

//==============================================================
//函数名称:		AK_DestroyMutex
//函数参数:		pMutex[in]: 互斥锁   
//返回值:	    CKR_OK: 成功
//				其他:	失败	
//函数功能:     销毁一个互斥锁
//==============================================================
int AK_DestroyMutex(void* pMutex)
{
    //检查参数合法性
    if( NULL == pMutex )
        return 1;

    // 释放句柄
    CloseHandle(pMutex); 

    return 0;
}
