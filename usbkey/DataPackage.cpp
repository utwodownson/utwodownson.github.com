/*	DataPackage.c
Author  :   caizhun@aisino.com
CO.     :   AISINO
date    :   2008年6月6日，
描述    :   USBKEY底层命令包打包函数 */

#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include "./inc/DataPackage.h"
#include "./inc/global.h"
#include "./inc/UsbKeyComDLL.h"


USBKEY_ULONG PackageData(USBKEY_HANDLE	hDeviceHandle,USBKEY_UCHAR cP1, USBKEY_UCHAR cP2,USBKEY_INT32 uLc,USBKEY_UCHAR_PTR pcData,USBKEY_INT32 uLe,
        USBKEY_ULONG ulOPType,USBKEY_UCHAR_PTR pResponsedData,USBKEY_ULONG_PTR pulDataLen)
{
    USBKEY_ULONG rtn = 0;
    USBKEY_UCHAR request[USBKEY_MAX_COM_LEN];
    USBKEY_UCHAR cCLA,cINS;

    switch(ulOPType)
    {
        case USBKEY_OP_ResetDevice:
            cCLA = USBKEY_DEVICE_CODE;
            cINS = USBKEY_CMD_DEVICE_RESET;
            break;
        case USBKEY_OP_ActivateSM:
            cCLA = USBKEY_DEFAULT_CODE;
            cINS = USBKEY_CMD_ACTIVATE_SECMACH;
            break;
        case USBKEY_OP_GetChallenge:
            cCLA = USBKEY_DEFAULT_CODE;
            cINS = USBKEY_CMD_GET_CHALLENGE;
            break;
        case USBKEY_OP_GetDeviceInfo:
            cCLA = USBKEY_DEVICE_CODE;
            cINS = USBKEY_CMD_GET_DEVICE_INFO;
            break;
        case USBKEY_OP_ExternalAuth:
            cCLA = USBKEY_DEFAULT_CODE;
            cINS = USBKEY_CMD_EXTERNAL_AUTHENT;
            break;
        case USBKEY_OP_InternalAuth:
            cCLA = USBKEY_DEFAULT_CODE;
            cINS = USBKEY_CMD_INTERNAL_AUTHENT;
            break;
        case USBKEY_OP_CreateFile:
            cCLA = USBKEY_FILE_CODE;
            cINS = USBKEY_CMD_CREATE_FILE;
            break;
        case USBKEY_OP_DeleteFile:
            cCLA = USBKEY_FILE_CODE;
            cINS = USBKEY_CMD_DELETE_FILE;
            break;
        case USBKEY_OP_SelectFile:
            cCLA = USBKEY_DEFAULT_CODE;
            cINS = USBKEY_CMD_SELECT_FILE;
            break;
        case USBKEY_OP_ReadBinary:
            cCLA = USBKEY_DEFAULT_CODE;
            cINS = USBKEY_CMD_READ_BINARY;
            break;
        case USBKEY_OP_UpdateBinary:
            cCLA = USBKEY_DEFAULT_CODE;
            cINS = USBKEY_CMD_UPDATE_BINARY;
            break;
        case USBKEY_OP_ReadRecord:
            cCLA = USBKEY_DEFAULT_CODE;
            cINS = USBKEY_CMD_READ_RECORD;
            break;
        case USBKEY_OP_UpdateRecord:
            cCLA = USBKEY_DEFAULT_CODE;
            cINS = USBKEY_CMD_UPDATE_RECORD;
            break;
        case USBKEY_OP_AppendRecord:
            cCLA = USBKEY_DEFAULT_CODE;
            cINS = USBKEY_CMD_APPEND_RECORD;
            break;
        case USBKEY_OP_VerifyPIN:
            cCLA = USBKEY_DEFAULT_CODE;
            cINS = USBKEY_CMD_VERIFY_PIN;
            break;
        case USBKEY_OP_ChangePIN:
            cCLA = USBKEY_FILE_CODE;
            cINS = USBKEY_CMD_CHANGE_PIN;
            break;
        case USBKEY_OP_UnlockPIN:
            cCLA = USBKEY_LOCK_CODE;
            cINS = USBKEY_CMD_UNLOCK_PIN;
            break;
        case USBKEY_OP_ReloadPIN:
            cCLA = USBKEY_FILE_CODE;
            cINS = USBKEY_CMD_RELOAD_PIN;
            break;
        case USBKEY_OP_AppBlock:
            cCLA = USBKEY_LOCK_CODE;
            cINS = USBKEY_CMD_LOCK_APP;
            break;
        case USBKEY_OP_AppUnblock:
            cCLA = USBKEY_LOCK_CODE;
            cINS = USBKEY_CMD_UNLOCK_APP;
            break;
        case USBKEY_OP_CardBlock:
            cCLA = USBKEY_LOCK_CODE;
            cINS = USBKEY_CMD_LOCK_DEVICE;
            break;
        case USBKEY_OP_GenRSAKeyPair:
            cCLA = USBKEY_PKI_CODE;
            cINS = USBKEY_CMD_GEN_RSA_KEY_PAIR;
            break;
        case USBKEY_OP_GenExtRSAKeyPair:
            cCLA = USBKEY_PKI_CODE;
            cINS = USBKEY_CMD_GEN_EXT_RSA_KEY_PAIR;
            break;
        case USBKEY_OP_WriteKey:
            cCLA = USBKEY_FILE_CODE;
            cINS = USBKEY_CMD_WRITE_KEY;
            break;
        case USBKEY_OP_PrivateKeyOP:
            cCLA = USBKEY_PKI_CODE;
            cINS = USBKEY_CMD_RSA_PRIKEY_OPERATION;
            break;
        case USBKEY_OP_PublicKeyOP:
            cCLA = USBKEY_PKI_CODE;
            cINS = USBKEY_CMD_RSA_PUBKEY_OPERATION;
            break;
        case USBKEY_OP_TempRSAOP:
            cCLA = USBKEY_PKI_CODE;
            cINS = USBKEY_CMD_TEMP_RSA_OPERATION;
            break;
        case USBKEY_OP_DataEncrypt:
            cCLA = USBKEY_PKI_CODE;
            cINS = USBKEY_CMD_SYM_KEY_ENCRYPT;
            break;
        case USBKEY_OP_DataDecrypt:
            cCLA = USBKEY_PKI_CODE;
            cINS = USBKEY_CMD_SYM_KEY_DECRYPT;
            break;
        case USBKEY_OP_MassiveDataEncrypt:
            cCLA = USBKEY_PKI_CODE;
            cINS = USBKEY_CMD_SYM_KEY_MASSIVE_ENCRYPT;
            break;
        case USBKEY_OP_MassiveDataDecrypt:
            cCLA = USBKEY_PKI_CODE;
            cINS = USBKEY_CMD_SYM_KEY_MASSIVE_DECRYPT;
            break;
        case USBKEY_OP_TempSymmKeyOP:
            cCLA = USBKEY_PKI_CODE;
            cINS = USBKEY_CMD_TEMP_SYM_KEY_OPERATION;
            break;

        case USBKEY_OP_GenSymmKey:
            cCLA = USBKEY_PKI_CODE;
            cINS = USBKEY_CMD_GEN_SYM_KEY;
            break;
        case USBKEY_OP_ImportDivSymmKey:
            cCLA = USBKEY_PKI_CODE;
            cINS = USBKEY_CMD_IMPORT_SYM_KEY;	
            break;
        case USBKEY_OP_ImportECCEnSymmKey:
            cCLA = USBKEY_PKI_CODE;
            cINS = USBKEY_CMD_ECC_IMPORT_SYMKEY;
            break;
        case USBKEY_OP_ExportDivSymmKey:
            cCLA = USBKEY_PKI_CODE;
            cINS = USBKEY_CMD_EXPORT_SYM_KEY;	
            break;
        case USBKEY_OP_ExportSymmKeybyEcc:
            cCLA = USBKEY_PKI_CODE;
            cINS = USBKEY_CMD_ECC_EXPORT_SYMKEY;	
            break;
        case USBKEY_OP_SetDevInfo:
            cCLA = USBKEY_DEVICE_CODE;
            cINS = USBKEY_CMD_DEVICE_SET_INFO;
            break;
        case USBKEY_OP_ImportPrivateKey:
            cCLA = USBKEY_PKI_CODE;
            cINS = USBKEY_CMD_IMPORT_PRIVATE_KEY;
            break;
        case USBKEY_OP_GenECCKeyPair:
            cCLA = USBKEY_PKI_CODE;
            cINS = USBKEY_CMD_GEN_ECC_KEY_PAIR;
            break;
        case USBKEY_OP_ECCPubKeyEncrypt:
            cCLA = USBKEY_PKI_CODE;
            cINS = USBKEY_CMD_ECC_ENCRYPT;
            break;
        case USBKEY_OP_ECCPriKeyDecrypt:
            cCLA = USBKEY_PKI_CODE;
            cINS = USBKEY_CMD_ECC_DECRYPT;
            break;
        case USBKEY_OP_ECCSign:
            cCLA = USBKEY_PKI_CODE;
            cINS = USBKEY_CMD_ECC_SIGN;
            break;
        case USBKEY_OP_ECCVerify:
            cCLA = USBKEY_PKI_CODE;
            cINS = USBKEY_CMD_ECC_VERIFY;
            break;
        case USBKEY_OP_ECCExportAgreeData:
            cCLA = USBKEY_PKI_CODE;
            cINS = USBKEY_CMD_EXPORT_AGREEMENT;
            break;
        case USBKEY_OP_ECCImportAgreeData:
            cCLA = USBKEY_PKI_CODE;
            cINS = USBKEY_CMD_IMPORT_AGREEMENT;
            break;
        default:
            break;
    }

    rtn = DeviceExecCmdExt(
            hDeviceHandle,		//设备句柄
            gDeviceType,		//设备类型
            cCLA,				//命令类别
            cINS,				//命令代码
            cP1,				//命令首部
            cP2,				//命令首部
            uLc,				//数据长度
            pcData,				//命令数据
            uLe,				//期望返回数据长度
            pResponsedData,		//[out] 响应
            (int *)pulDataLen	//in: 响应buffer的长度，out: 响应长度
            );
    return 0;
}

USBKEY_ULONG UnPackageData(USBKEY_UCHAR_PTR pcResponseData,USBKEY_ULONG ulOPType, USBKEY_UCHAR_PTR pcData)
{
    return 0;
}
