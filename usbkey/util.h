/*	util.h
Author  :   caizhun@aisino.com
CO.     :   AISINO
date    :   2008年5月26日，
描述    :   USBKEY底层API工具函数接口	*/

#ifndef _UTIL_H
#define _UTIL_H
#include "./inc/global.h"

#ifdef __cplusplus
extern "C" {
#endif


//内部文件ID
#define USBKEY_MF_ID					0x3f00		//根目录
#define USBKEY_SYSTEM_KEY_FILE_ID		0x3f01		//系统密钥
#define USBKEY_USER_KEY_FILE_ID			0x3f02		//用户密钥
#define USBKEY_DIR_REC_FILE_ID			0x8000		//目录记录文件
#define USBKEY_FILE_REC_FILE_ID			0x8001		//文件记录文件
#define USBKEY_CONTAINER_REC_FILE_ID	0x8002		//容器记录文件
#define USBKEY_SYM_KEY_FILE_ID			0x8003		//对称密钥ID文件

//对称密钥ID文件用二进制文件实现，文件长度为255
//文件内容为对称密钥所属的容器文件ID，如果id为0xff，则为没分配的ID号
//#define USBKEY_SYM_KEY_ID_FILE_LEN		255			//对称密钥ID文件长度
#define USBKEY_SYM_KEY_ID_FILE_LEN		254			//对称密钥ID文件长度 (金融IC卡最大文件块长度254)
#define USBKEY_KEY_REC_LEN				32			//密钥记录长度

#ifdef _SEC_GTAX_APP
#define USBKEY_TAX_INFO_FILE_ID			0x80ff		//税控应用存放税控信息文件
#define USBKEY_TAX_INFO_ID_FILE_LEN		1024*2		//税控应用存放税控信息文件长度 2K
#endif

//#define USBKEY_FILE_MAX_BLOCK_LEN		2048			//文件处理块长(UsbKeyAPI.h头文件中同义宏不使用)
#define USBKEY_FILE_MAX_BLOCK_LEN			254			//文件处理块长(金融IC卡)
#define USBKEY_RANDOM_MAX_BLCOK_LEN		8			//随机数生成最大块长度(只能生成4、8长度随机数)

//PIN码大小
#define USBKEY_PIN_MAX_LEN				16
#define USBKEY_PIN_MIN_LEN				2		

//隐藏区只读PIN
#define USBKEY_HIDISK_ADMIN_PIN			"1111"
#define HIDISK_ADMIN_PIN_LEN				4
#define USBKEY_HIDISK_USER_PIN			"123456"
#define HIDISK_USER_PIN_LEN					6
#define USBKEY_HIDISK_TAX_PIN			"111111"
#define HIDISK_TAX_PIN_LEN					6

//PIN标识
#define USBKEY_DEVICE_PIN_FLAG			0x01
#define USBKEY_ADMIN_PIN_FLAG			0x02
#define USBKEY_USER_PIN_FLAG			0x03

#ifdef _SEC_GTAX_APP
#define USBKEY_HIDISK_ADMIN_PIN_FLAG	0x04
#define USBKEY_HIDISK_USER_PIN_FLAG		0x05
#define USBKEY_HIDISK_TAX_PIN_FLAG		0x06
#endif

//设备操作权限
#define USBKEY_DEVICE_PRIV				0x99		//设备认证
#define USBKEY_ADMIN_PRIV				0x66		//管理员
#define USBKEY_USER_PRIV				0x33		//用户

#ifdef _SEC_GTAX_APP
#define USBKEY_HIDISK_ADMIN_PRIV		0x55		//隐藏区管理用户
#define USBKEY_HIDISK_USER_PRIV			0x44		//隐藏区普通用户
#define USBKEY_HIDISK_TAX_PRIV			0x11		//隐藏区税局用户
#endif

#define USBKEY_ALL_PRIV					0xf0		//无权限限制
#define USBKEY_NOT_USE					0x0f		//不允许使用
#define USBKEY_ADMIN_USE_PRIV			0x66		//管理员使用权限
#ifdef _SEC_GTAX_APP
#define USBKEY_USER_USE_PRIV			0x53		//用户使用权限
#else
#define USBKEY_USER_USE_PRIV			0x33		//用户使用权限
#endif

#define USBKEY_FILE_RECORD_LEN			40			//文件记录长度
#define PATH_MAX						512
//#define USBKEY_MAX_SYM_KEY_ID			255			//最大对称密钥ID
#define USBKEY_MAX_SYM_KEY_ID			254			//最大对称密钥ID  (金融IC卡最大文件块长度254)

#define USBKEY_TAX_PIN_RETRY_TIMES		8			//税控PIN重试次数

//初始化信息
#define APP_VER_MAJOR					1
#define APP_VER_MINOR					0

#define USBKEY_PUBLIC_KEY_FILE_LEN		516			//公钥文件长度
#define USBKEY_PRIVATE_KEY_FILE_LEN		1412		//私钥文件长度
//#define USBKEY_MAX_COM_BUFFER			2048		//最大命令数据/响应数据buffer

//COS命令码
//#define USBKEY_CMD_READ_BINARY						0xb0		//读二进制文件
//#define USBKEY_CMD_WRITE_BINARY						0xd6		//写二进制文件
//#define USBKEY_CMD_CREATE_FILE						0xe0		//创建文件

//#define USBKEY_CMD_TYPE_PKI							0xb0		//pki命令类别码
//#define USBKEY_CMD_DODE_GEN_ECC_KEY_PAIR			0x3b		//产生ECC密钥对
//#define USBKEY_CMD_CODE_ECC_SIGN					0x3e		//ECC签名
//#define USBKEY_CMD_CODE_ECC_VERIFY					0x3f		//ECC验证签名	
//#define USBKEY_CMD_CODE_ECC_IMPORT_SYMKEY			0x40		//ECC加密导入对称密钥
//#define USBKEY_CMD_CODE_ECC_EXPORT_SYMKEY			0x41		//ECC加密导出对称密钥
//#define USBKEY_CMD_CODE_ECC_ENCRYPT					0x3d		//ECC加密
//#define USBKEY_CMD_CODE_ECC_DECRYPT					0x3c		//ECC解密
//#define USBKEY_CMD_CODE_EXPORT_AGREEMENT			0x50		//导出ECC密钥协商参数
//#define USBKEY_CMD_CODE_IMPORT_AGREEMENT			0x51		//导入ECC密钥协商参数


//////////////////////////////////////////////////////////////////////////
//工具函数
//==============================================================
//函数名称:		IsNewInDevice
//函数参数:		pDeviceName：设备名称
//返回值:		TRUE：新设备
//				FALSE：原有设备
//函数功能: 	判断是否为新设备。
//==============================================================
BOOL IsNewInDevice(char ppchDevNameList[DEVICE_MAX_QUANTITY][MAX_PATH], char* pDeviceName);

//==============================================================
//函数名称:		DealDeviceIn
//函数参数:		无			
//返回值:		无
//函数功能: 	处理设备插入事件
//==============================================================
void  DealDeviceIn(char ppchDevNameList[DEVICE_MAX_QUANTITY][MAX_PATH], char* pchDevName, USBKEY_ULONG* pulLen, USBKEY_ULONG* pulEventType);

//===============================================================
//函数名称:		DealDeviceOut
//函数参数:		无			
//返回值:		无
//函数功能: 	处理设备拔出事件。
//===============================================================
void  DealDeviceOut(char ppchDevNameList[DEVICE_MAX_QUANTITY][MAX_PATH], char* pchDevName, USBKEY_ULONG* pulLen, USBKEY_ULONG* pulEventType);



//回调函数
LRESULT CALLBACK CallUsbProc( int nCode, WPARAM wParam, LPARAM lParam );


USBKEY_ULONG CheckValid(USBKEY_UCHAR_PTR name);

USBKEY_ULONG CheckPrivillege(USBKEY_HANDLE hDeviceHandle);

USBKEY_ULONG SelectFile(USBKEY_HANDLE hDeviceHandle,USBKEY_UCHAR_PTR pcDirName, USBKEY_UCHAR_PTR pcFileID);

USBKEY_ULONG CreateDirRecord(USBKEY_HANDLE hDeviceHandle,USBKEY_UCHAR_PTR pcDirName);

USBKEY_ULONG CreateFileRecord(USBKEY_HANDLE hDeviceHandle,USBKEY_UCHAR ulAccessCondition);

USBKEY_ULONG CreateKeyRecord(USBKEY_HANDLE hDeviceHandle,USBKEY_UCHAR ulAccessCondition);

USBKEY_ULONG CreateContainerRecord(USBKEY_HANDLE hDeviceHandle,USBKEY_UCHAR ulAccessCondition);

USBKEY_ULONG ClearContainer(USBKEY_HANDLE hDeviceHandle, USBKEY_UCHAR_PTR pcContainerFileID);

USBKEY_ULONG AddContainerKey(USBKEY_HANDLE hDeviceHandle, USBKEY_ULONG ulContainerRecNO,USBKEY_ULONG ulKeyUsage,USBKEY_ULONG ulBitLen);

USBKEY_ULONG AddContainerKeySep(USBKEY_HANDLE hDeviceHandle, USBKEY_ULONG ulContainerRecNO,USBKEY_ULONG ulKeyUsage, USBKEY_ULONG ulKeyType,USBKEY_ULONG ulBitLen);

//Add by zhoushenshen 20110118
USBKEY_ULONG ReadContainerKeyRec(USBKEY_HANDLE hDeviceHandle,USBKEY_ULONG ulContainerRecNO,USBKEY_ULONG ulKeyUsage, USBKEY_ULONG ulKeyType,USBKEY_UCHAR_PTR pKeyRec,USBKEY_ULONG_PTR pulRecLen);

USBKEY_ULONG DeleteContainerKey(USBKEY_HANDLE hDeviceHandle,USBKEY_ULONG ulContainerRecNO,USBKEY_ULONG ulKeyUsage, USBKEY_ULONG ulKeyType);

USBKEY_ULONG AddContainerCert(USBKEY_HANDLE hDeviceHandle,USBKEY_ULONG ulContainerRecNO,USBKEY_ULONG ulKeyUsage);

USBKEY_ULONG DeleteContainerCert(USBKEY_HANDLE hDeviceHandle,USBKEY_ULONG ulContainerRecNO,USBKEY_ULONG ulKeyUsage);

USBKEY_ULONG ReadRecord(USBKEY_HANDLE hDeviceHandle, USBKEY_UCHAR_PTR pcFileID, USBKEY_ULONG recNO, USBKEY_UCHAR_PTR pcRecord, USBKEY_ULONG_PTR pulRecordLen);

USBKEY_ULONG AppendRecord(USBKEY_HANDLE hDeviceHandle, USBKEY_UCHAR_PTR pcFileID,USBKEY_UCHAR_PTR pcRecord, USBKEY_ULONG ulRecordLen, USBKEY_ULONG ulType);

USBKEY_ULONG UpdateRecord(USBKEY_HANDLE hDeviceHandle, USBKEY_UCHAR_PTR pcFileID,USBKEY_ULONG recNO, USBKEY_UCHAR_PTR pcRecord, USBKEY_ULONG ulRecordLen);

USBKEY_ULONG DeleRecord(USBKEY_HANDLE hDeviceHandle, USBKEY_UCHAR_PTR pcFileID, USBKEY_UCHAR_PTR pcRecordID, USBKEY_ULONG ulRecordLen,  USBKEY_ULONG ulType);

USBKEY_ULONG GetRecNo(USBKEY_HANDLE hDeviceHandle, USBKEY_UCHAR_PTR pcFileID, USBKEY_ULONG_PTR plRecordNum);

USBKEY_ULONG GetFileRecNo(USBKEY_HANDLE hDeviceHandle, USBKEY_UCHAR_PTR pcFileID, USBKEY_ULONG_PTR plRecordNum);

USBKEY_ULONG GetConRecNo(USBKEY_HANDLE hDeviceHandle, USBKEY_UCHAR_PTR pcFileID, USBKEY_ULONG_PTR plRecordNum);

USBKEY_ULONG GetKeyRecNo(USBKEY_HANDLE hDeviceHandle, USBKEY_UCHAR_PTR pcFileID, USBKEY_ULONG_PTR plRecordNum);

USBKEY_ULONG AddFileRecord(USBKEY_HANDLE hDeviceHandle, USBKEY_UCHAR_PTR pcFileID);

USBKEY_ULONG DeleFileRecord(USBKEY_HANDLE hDeviceHandle, USBKEY_UCHAR_PTR pcFileName);

USBKEY_ULONG GetFileInfo(USBKEY_HANDLE hDeviceHandle, USBKEY_UCHAR_PTR pcFileName, USBKEY_UCHAR_PTR pcFileInfo);

USBKEY_ULONG SearchFileRecNo(USBKEY_HANDLE hDeviceHandle,USBKEY_UCHAR_PTR pcFileName, USBKEY_ULONG_PTR plRecordNum);

USBKEY_ULONG AddKeyRecord(USBKEY_HANDLE hDeviceHandle, USBKEY_UCHAR_PTR pcKeyID);

USBKEY_ULONG DeleKeyRecord(USBKEY_HANDLE hDeviceHandle, USBKEY_UCHAR_PTR pcKeyID);

USBKEY_ULONG AddDirRecord(USBKEY_HANDLE hDeviceHandle,USBKEY_UCHAR_PTR pcDirName);

USBKEY_ULONG DeleDirRecord(USBKEY_HANDLE hDeviceHandle,USBKEY_UCHAR_PTR pcDirName);

USBKEY_ULONG AddContainerRecord(USBKEY_HANDLE hDeviceHandle, USBKEY_UCHAR_PTR pcContainerName);

USBKEY_ULONG DeleContainerRecord(USBKEY_HANDLE hDeviceHandle, USBKEY_UCHAR_PTR pcContainerName);

USBKEY_ULONG DeleteSymKey(USBKEY_HANDLE hDeviceHandle, USBKEY_UCHAR ucSymKeyID);

//删除容器对称密钥
USBKEY_ULONG DeleteContainerSymmetricKey(USBKEY_HANDLE hDeviceHandle,USBKEY_ULONG ulContainerRecNO);

USBKEY_ULONG SearchContainerRecNo(USBKEY_HANDLE hDeviceHandle,USBKEY_UCHAR_PTR pcContainerName, USBKEY_ULONG_PTR plRecordNum);

USBKEY_ULONG GetDeviceInfo(USBKEY_HANDLE hDeviceHandle,USBKEY_UCHAR_PTR pcDeviceInfo,USBKEY_ULONG ulInfoType);

USBKEY_ULONG EraseUsbKey(USBKEY_HANDLE hDeviceHandle);

USBKEY_ULONG Erase45Key(USBKEY_HANDLE hDeviceHandle);

USBKEY_ULONG EraseAC3Key(USBKEY_HANDLE hDeviceHandle);

USBKEY_ULONG WritePublicKey(USBKEY_HANDLE hDeviceHandle,USBKEY_UCHAR_PTR pcFileName,USBKEY_ULONG ulOffset,USBKEY_UCHAR_PTR pcData,USBKEY_ULONG_PTR pulSize);	

//USBKEY_ULONG WriteProtectedPrivateKey(USBKEY_HANDLE hDeviceHandle,USBKEY_UCHAR_PTR pcFileName,USBKEY_ULONG ulOffset,USBKEY_UCHAR_PTR pcData,USBKEY_ULONG_PTR pulSize);

//检查权限 flag 权限的标志。USBKEY_USERTYPE_USER为检查用户权限 USBKEY_USERTYPE_ADMIN为检查管理员权限 0为检查管理员或用户权限
USBKEY_ULONG TaxCheckPrivillege(USBKEY_HANDLE hDeviceHandle,USBKEY_ULONG flag);

USBKEY_ULONG SearchTaxSymmetricKey(USBKEY_HANDLE hDeviceHandle,USBKEY_UCHAR_PTR pucTaxKeyID_0,USBKEY_UCHAR_PTR pucTaxKeyID_1);

// 获取税控密钥ID
// 返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_ULONG GetFreeTaxKeyID(
		USBKEY_HANDLE		hDeviceHandle,
		USBKEY_UCHAR_PTR	pucTaxKeyID_0,
		USBKEY_UCHAR_PTR	pucTaxKeyID_1,
		USBKEY_UCHAR		pcConSymKeyIDRec[USBKEY_MAX_SYM_KEY_ID]);

// 把税控对称密钥增加到密钥文件
// 返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_ULONG AddTaxSymKey(
	USBKEY_HANDLE		hDeviceHandle,
	USBKEY_UCHAR		ucTaxKeyID_0,
	USBKEY_UCHAR		ucTaxKeyID_1,
	USBKEY_UCHAR		pcConSymKeyIDRec[USBKEY_MAX_SYM_KEY_ID]);

void Reverser(USBKEY_VOID_PTR buf, USBKEY_ULONG len);
//////////////////////////////////////////////////////////////////////////

// 创建公私钥文件
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_ULONG CreateKeyPairFile(
	USBKEY_HANDLE		hDeviceHandle,
	USBKEY_UCHAR_PTR	pcPubKeyFileID, 
	USBKEY_UCHAR_PTR	pcPriKeyFileID,
	USBKEY_UCHAR		ucPubKeyFileRight,
	USBKEY_UCHAR		ucPriKeyFileRight);

// 产生ECC密钥对
// 返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_ULONG GenEccKeyPair(
	USBKEY_HANDLE			hDeviceHandle,
	USBKEY_ULONG			ulBitLen, 
	USBKEY_UCHAR_PTR		pcPubKeyFileID, 
	USBKEY_UCHAR_PTR		pcPriKeyFileID,
	USBKEY_ECC_PUBLIC_KEY*	pPublicKey);

//读当前选择的二进制文件
USBKEY_ULONG ReadCurBinaryFile(
	USBKEY_HANDLE		hDeviceHandle, 
	USBKEY_ULONG		ulOffset, 
	USBKEY_ULONG		ulLen,
	USBKEY_UCHAR_PTR	pOutBuf);

////读当前选择的税控文件
//USBKEY_ULONG ReadCurTaxFile(
//	USBKEY_HANDLE		hDeviceHandle, 
//	USBKEY_ULONG		ulOffset, 
//	USBKEY_ULONG_PTR	pulLen,
//	USBKEY_UCHAR_PTR	pOutBuf);

//写当前选择的二进制文件
USBKEY_ULONG WriteCurBinaryFile(
	USBKEY_HANDLE		hDeviceHandle, 
	USBKEY_ULONG		ulOffset, 
	USBKEY_ULONG		ulLen,
	USBKEY_UCHAR_PTR	pInBuf);

// 获取一个空闲的容器对称密钥id
// 返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_ULONG GetFreeConSymKeyID(
		USBKEY_HANDLE		hDeviceHandle,
		USBKEY_UCHAR_PTR	pucSymKeyID,
		USBKEY_UCHAR		pcConSymKeyIDRec[USBKEY_MAX_SYM_KEY_ID]);

// 把一个对称密钥增加到容器
// 返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_ULONG AddContainerSymKey(
	USBKEY_HANDLE		hDeviceHandle,
	USBKEY_UCHAR		ucSymKeyID,
	USBKEY_UCHAR		ucRecNo,
	USBKEY_UCHAR		pcConSymKeyIDRec[USBKEY_MAX_SYM_KEY_ID]);

// RSA导入对称密钥，pcWrapKeyID为NULL则明文导入，否则密文导入
// 返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_ULONG RSAImportSessionKey(
	USBKEY_HANDLE		hDeviceHandle,
	USBKEY_UCHAR_PTR	pcWrapKeyID,
	USBKEY_UCHAR		pcKeyAttr[6],
	USBKEY_UCHAR_PTR	pcSymKey,	
	USBKEY_ULONG		ulSymKeyLen);

// ECC加密导入对称密钥
// 返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_ULONG ECCImportSessionKey(
	USBKEY_HANDLE		hDeviceHandle,
	USBKEY_UCHAR_PTR	pcWrapKeyID,
	USBKEY_UCHAR		pcKeyAttr[6],
	USBKEY_UCHAR_PTR	pcSymKey,	
	USBKEY_ULONG		ulSymKeyLen);

// RSA加密导出对称密钥
// 返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_ULONG RSAExportSessionKey(
	USBKEY_HANDLE		hDeviceHandle,
	USBKEY_UCHAR_PTR	pcSymKeyID,
	USBKEY_UCHAR_PTR	pcWrapKeyID,
	USBKEY_UCHAR_PTR	pcSymKey,		
	USBKEY_ULONG_PTR	pulSymKeyLen);

// ECC加密导出对称密钥
// 返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_ULONG ECCExportSessionKey(
	USBKEY_HANDLE		hDeviceHandle,
	USBKEY_UCHAR_PTR	pcSymKeyID,
	USBKEY_UCHAR_PTR	pcWrapKeyID,
	USBKEY_UCHAR_PTR	pcSymKey,		
	USBKEY_ULONG_PTR	pulSymKeyLen);

// 创建二进制文件
// 返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_ULONG CreateBinaryFile(
	USBKEY_HANDLE		hDeviceHandle,
	USBKEY_UCHAR_PTR	pcFileID,
	USBKEY_UCHAR		ucMode,
	USBKEY_UCHAR		ucReadRight,		
	USBKEY_UCHAR		ucWriteRight,
	USBKEY_ULONG		ulFileLen);

// 创建应用PIN码
// 返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_ULONG CreateAppPin(
	USBKEY_HANDLE		hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR	pucPIN,			//[in]送入的PIN
	USBKEY_ULONG		ulPINLen,		//[in]送入的PIN长度
	USBKEY_UCHAR		ucPinFlag,		//[in]PIN码标识
	USBKEY_UCHAR		ucSecState,		//[in]PIN验证后安全状态
	USBKEY_UCHAR		ucRetryCounts,	//[in]重试次数
	USBKEY_UCHAR		ulUseRights,	//[in]使用权限
	USBKEY_UCHAR		ulChangeRights);	//[in]更改权限

USBKEY_ULONG EraseAC4Key(USBKEY_HANDLE hDeviceHandle);

#ifdef __cplusplus
}
#endif
#endif
