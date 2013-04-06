/*	util.h
Author  :   caizhun@aisino.com
CO.     :   AISINO
date    :   2008��5��26�գ�
����    :   USBKEY�ײ�API���ߺ����ӿ�	*/

#ifndef _UTIL_H
#define _UTIL_H
#include "./inc/global.h"

#ifdef __cplusplus
extern "C" {
#endif


//�ڲ��ļ�ID
#define USBKEY_MF_ID					0x3f00		//��Ŀ¼
#define USBKEY_SYSTEM_KEY_FILE_ID		0x3f01		//ϵͳ��Կ
#define USBKEY_USER_KEY_FILE_ID			0x3f02		//�û���Կ
#define USBKEY_DIR_REC_FILE_ID			0x8000		//Ŀ¼��¼�ļ�
#define USBKEY_FILE_REC_FILE_ID			0x8001		//�ļ���¼�ļ�
#define USBKEY_CONTAINER_REC_FILE_ID	0x8002		//������¼�ļ�
#define USBKEY_SYM_KEY_FILE_ID			0x8003		//�Գ���ԿID�ļ�

//�Գ���ԿID�ļ��ö������ļ�ʵ�֣��ļ�����Ϊ255
//�ļ�����Ϊ�Գ���Կ�����������ļ�ID�����idΪ0xff����Ϊû�����ID��
//#define USBKEY_SYM_KEY_ID_FILE_LEN		255			//�Գ���ԿID�ļ�����
#define USBKEY_SYM_KEY_ID_FILE_LEN		254			//�Գ���ԿID�ļ����� (����IC������ļ��鳤��254)
#define USBKEY_KEY_REC_LEN				32			//��Կ��¼����

#ifdef _SEC_GTAX_APP
#define USBKEY_TAX_INFO_FILE_ID			0x80ff		//˰��Ӧ�ô��˰����Ϣ�ļ�
#define USBKEY_TAX_INFO_ID_FILE_LEN		1024*2		//˰��Ӧ�ô��˰����Ϣ�ļ����� 2K
#endif

//#define USBKEY_FILE_MAX_BLOCK_LEN		2048			//�ļ�����鳤(UsbKeyAPI.hͷ�ļ���ͬ��겻ʹ��)
#define USBKEY_FILE_MAX_BLOCK_LEN			254			//�ļ�����鳤(����IC��)
#define USBKEY_RANDOM_MAX_BLCOK_LEN		8			//������������鳤��(ֻ������4��8���������)

//PIN���С
#define USBKEY_PIN_MAX_LEN				16
#define USBKEY_PIN_MIN_LEN				2		

//������ֻ��PIN
#define USBKEY_HIDISK_ADMIN_PIN			"1111"
#define HIDISK_ADMIN_PIN_LEN				4
#define USBKEY_HIDISK_USER_PIN			"123456"
#define HIDISK_USER_PIN_LEN					6
#define USBKEY_HIDISK_TAX_PIN			"111111"
#define HIDISK_TAX_PIN_LEN					6

//PIN��ʶ
#define USBKEY_DEVICE_PIN_FLAG			0x01
#define USBKEY_ADMIN_PIN_FLAG			0x02
#define USBKEY_USER_PIN_FLAG			0x03

#ifdef _SEC_GTAX_APP
#define USBKEY_HIDISK_ADMIN_PIN_FLAG	0x04
#define USBKEY_HIDISK_USER_PIN_FLAG		0x05
#define USBKEY_HIDISK_TAX_PIN_FLAG		0x06
#endif

//�豸����Ȩ��
#define USBKEY_DEVICE_PRIV				0x99		//�豸��֤
#define USBKEY_ADMIN_PRIV				0x66		//����Ա
#define USBKEY_USER_PRIV				0x33		//�û�

#ifdef _SEC_GTAX_APP
#define USBKEY_HIDISK_ADMIN_PRIV		0x55		//�����������û�
#define USBKEY_HIDISK_USER_PRIV			0x44		//��������ͨ�û�
#define USBKEY_HIDISK_TAX_PRIV			0x11		//������˰���û�
#endif

#define USBKEY_ALL_PRIV					0xf0		//��Ȩ������
#define USBKEY_NOT_USE					0x0f		//������ʹ��
#define USBKEY_ADMIN_USE_PRIV			0x66		//����Աʹ��Ȩ��
#ifdef _SEC_GTAX_APP
#define USBKEY_USER_USE_PRIV			0x53		//�û�ʹ��Ȩ��
#else
#define USBKEY_USER_USE_PRIV			0x33		//�û�ʹ��Ȩ��
#endif

#define USBKEY_FILE_RECORD_LEN			40			//�ļ���¼����
#define PATH_MAX						512
//#define USBKEY_MAX_SYM_KEY_ID			255			//���Գ���ԿID
#define USBKEY_MAX_SYM_KEY_ID			254			//���Գ���ԿID  (����IC������ļ��鳤��254)

#define USBKEY_TAX_PIN_RETRY_TIMES		8			//˰��PIN���Դ���

//��ʼ����Ϣ
#define APP_VER_MAJOR					1
#define APP_VER_MINOR					0

#define USBKEY_PUBLIC_KEY_FILE_LEN		516			//��Կ�ļ�����
#define USBKEY_PRIVATE_KEY_FILE_LEN		1412		//˽Կ�ļ�����
//#define USBKEY_MAX_COM_BUFFER			2048		//�����������/��Ӧ����buffer

//COS������
//#define USBKEY_CMD_READ_BINARY						0xb0		//���������ļ�
//#define USBKEY_CMD_WRITE_BINARY						0xd6		//д�������ļ�
//#define USBKEY_CMD_CREATE_FILE						0xe0		//�����ļ�

//#define USBKEY_CMD_TYPE_PKI							0xb0		//pki���������
//#define USBKEY_CMD_DODE_GEN_ECC_KEY_PAIR			0x3b		//����ECC��Կ��
//#define USBKEY_CMD_CODE_ECC_SIGN					0x3e		//ECCǩ��
//#define USBKEY_CMD_CODE_ECC_VERIFY					0x3f		//ECC��֤ǩ��	
//#define USBKEY_CMD_CODE_ECC_IMPORT_SYMKEY			0x40		//ECC���ܵ���Գ���Կ
//#define USBKEY_CMD_CODE_ECC_EXPORT_SYMKEY			0x41		//ECC���ܵ����Գ���Կ
//#define USBKEY_CMD_CODE_ECC_ENCRYPT					0x3d		//ECC����
//#define USBKEY_CMD_CODE_ECC_DECRYPT					0x3c		//ECC����
//#define USBKEY_CMD_CODE_EXPORT_AGREEMENT			0x50		//����ECC��ԿЭ�̲���
//#define USBKEY_CMD_CODE_IMPORT_AGREEMENT			0x51		//����ECC��ԿЭ�̲���


//////////////////////////////////////////////////////////////////////////
//���ߺ���
//==============================================================
//��������:		IsNewInDevice
//��������:		pDeviceName���豸����
//����ֵ:		TRUE�����豸
//				FALSE��ԭ���豸
//��������: 	�ж��Ƿ�Ϊ���豸��
//==============================================================
BOOL IsNewInDevice(char ppchDevNameList[DEVICE_MAX_QUANTITY][MAX_PATH], char* pDeviceName);

//==============================================================
//��������:		DealDeviceIn
//��������:		��			
//����ֵ:		��
//��������: 	�����豸�����¼�
//==============================================================
void  DealDeviceIn(char ppchDevNameList[DEVICE_MAX_QUANTITY][MAX_PATH], char* pchDevName, USBKEY_ULONG* pulLen, USBKEY_ULONG* pulEventType);

//===============================================================
//��������:		DealDeviceOut
//��������:		��			
//����ֵ:		��
//��������: 	�����豸�γ��¼���
//===============================================================
void  DealDeviceOut(char ppchDevNameList[DEVICE_MAX_QUANTITY][MAX_PATH], char* pchDevName, USBKEY_ULONG* pulLen, USBKEY_ULONG* pulEventType);



//�ص�����
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

//ɾ�������Գ���Կ
USBKEY_ULONG DeleteContainerSymmetricKey(USBKEY_HANDLE hDeviceHandle,USBKEY_ULONG ulContainerRecNO);

USBKEY_ULONG SearchContainerRecNo(USBKEY_HANDLE hDeviceHandle,USBKEY_UCHAR_PTR pcContainerName, USBKEY_ULONG_PTR plRecordNum);

USBKEY_ULONG GetDeviceInfo(USBKEY_HANDLE hDeviceHandle,USBKEY_UCHAR_PTR pcDeviceInfo,USBKEY_ULONG ulInfoType);

USBKEY_ULONG EraseUsbKey(USBKEY_HANDLE hDeviceHandle);

USBKEY_ULONG Erase45Key(USBKEY_HANDLE hDeviceHandle);

USBKEY_ULONG EraseAC3Key(USBKEY_HANDLE hDeviceHandle);

USBKEY_ULONG WritePublicKey(USBKEY_HANDLE hDeviceHandle,USBKEY_UCHAR_PTR pcFileName,USBKEY_ULONG ulOffset,USBKEY_UCHAR_PTR pcData,USBKEY_ULONG_PTR pulSize);	

//USBKEY_ULONG WriteProtectedPrivateKey(USBKEY_HANDLE hDeviceHandle,USBKEY_UCHAR_PTR pcFileName,USBKEY_ULONG ulOffset,USBKEY_UCHAR_PTR pcData,USBKEY_ULONG_PTR pulSize);

//���Ȩ�� flag Ȩ�޵ı�־��USBKEY_USERTYPE_USERΪ����û�Ȩ�� USBKEY_USERTYPE_ADMINΪ������ԱȨ�� 0Ϊ������Ա���û�Ȩ��
USBKEY_ULONG TaxCheckPrivillege(USBKEY_HANDLE hDeviceHandle,USBKEY_ULONG flag);

USBKEY_ULONG SearchTaxSymmetricKey(USBKEY_HANDLE hDeviceHandle,USBKEY_UCHAR_PTR pucTaxKeyID_0,USBKEY_UCHAR_PTR pucTaxKeyID_1);

// ��ȡ˰����ԿID
// ����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_ULONG GetFreeTaxKeyID(
		USBKEY_HANDLE		hDeviceHandle,
		USBKEY_UCHAR_PTR	pucTaxKeyID_0,
		USBKEY_UCHAR_PTR	pucTaxKeyID_1,
		USBKEY_UCHAR		pcConSymKeyIDRec[USBKEY_MAX_SYM_KEY_ID]);

// ��˰�ضԳ���Կ���ӵ���Կ�ļ�
// ����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_ULONG AddTaxSymKey(
	USBKEY_HANDLE		hDeviceHandle,
	USBKEY_UCHAR		ucTaxKeyID_0,
	USBKEY_UCHAR		ucTaxKeyID_1,
	USBKEY_UCHAR		pcConSymKeyIDRec[USBKEY_MAX_SYM_KEY_ID]);

void Reverser(USBKEY_VOID_PTR buf, USBKEY_ULONG len);
//////////////////////////////////////////////////////////////////////////

// ������˽Կ�ļ�
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_ULONG CreateKeyPairFile(
	USBKEY_HANDLE		hDeviceHandle,
	USBKEY_UCHAR_PTR	pcPubKeyFileID, 
	USBKEY_UCHAR_PTR	pcPriKeyFileID,
	USBKEY_UCHAR		ucPubKeyFileRight,
	USBKEY_UCHAR		ucPriKeyFileRight);

// ����ECC��Կ��
// ����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_ULONG GenEccKeyPair(
	USBKEY_HANDLE			hDeviceHandle,
	USBKEY_ULONG			ulBitLen, 
	USBKEY_UCHAR_PTR		pcPubKeyFileID, 
	USBKEY_UCHAR_PTR		pcPriKeyFileID,
	USBKEY_ECC_PUBLIC_KEY*	pPublicKey);

//����ǰѡ��Ķ������ļ�
USBKEY_ULONG ReadCurBinaryFile(
	USBKEY_HANDLE		hDeviceHandle, 
	USBKEY_ULONG		ulOffset, 
	USBKEY_ULONG		ulLen,
	USBKEY_UCHAR_PTR	pOutBuf);

////����ǰѡ���˰���ļ�
//USBKEY_ULONG ReadCurTaxFile(
//	USBKEY_HANDLE		hDeviceHandle, 
//	USBKEY_ULONG		ulOffset, 
//	USBKEY_ULONG_PTR	pulLen,
//	USBKEY_UCHAR_PTR	pOutBuf);

//д��ǰѡ��Ķ������ļ�
USBKEY_ULONG WriteCurBinaryFile(
	USBKEY_HANDLE		hDeviceHandle, 
	USBKEY_ULONG		ulOffset, 
	USBKEY_ULONG		ulLen,
	USBKEY_UCHAR_PTR	pInBuf);

// ��ȡһ�����е������Գ���Կid
// ����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_ULONG GetFreeConSymKeyID(
		USBKEY_HANDLE		hDeviceHandle,
		USBKEY_UCHAR_PTR	pucSymKeyID,
		USBKEY_UCHAR		pcConSymKeyIDRec[USBKEY_MAX_SYM_KEY_ID]);

// ��һ���Գ���Կ���ӵ�����
// ����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_ULONG AddContainerSymKey(
	USBKEY_HANDLE		hDeviceHandle,
	USBKEY_UCHAR		ucSymKeyID,
	USBKEY_UCHAR		ucRecNo,
	USBKEY_UCHAR		pcConSymKeyIDRec[USBKEY_MAX_SYM_KEY_ID]);

// RSA����Գ���Կ��pcWrapKeyIDΪNULL�����ĵ��룬�������ĵ���
// ����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_ULONG RSAImportSessionKey(
	USBKEY_HANDLE		hDeviceHandle,
	USBKEY_UCHAR_PTR	pcWrapKeyID,
	USBKEY_UCHAR		pcKeyAttr[6],
	USBKEY_UCHAR_PTR	pcSymKey,	
	USBKEY_ULONG		ulSymKeyLen);

// ECC���ܵ���Գ���Կ
// ����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_ULONG ECCImportSessionKey(
	USBKEY_HANDLE		hDeviceHandle,
	USBKEY_UCHAR_PTR	pcWrapKeyID,
	USBKEY_UCHAR		pcKeyAttr[6],
	USBKEY_UCHAR_PTR	pcSymKey,	
	USBKEY_ULONG		ulSymKeyLen);

// RSA���ܵ����Գ���Կ
// ����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_ULONG RSAExportSessionKey(
	USBKEY_HANDLE		hDeviceHandle,
	USBKEY_UCHAR_PTR	pcSymKeyID,
	USBKEY_UCHAR_PTR	pcWrapKeyID,
	USBKEY_UCHAR_PTR	pcSymKey,		
	USBKEY_ULONG_PTR	pulSymKeyLen);

// ECC���ܵ����Գ���Կ
// ����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_ULONG ECCExportSessionKey(
	USBKEY_HANDLE		hDeviceHandle,
	USBKEY_UCHAR_PTR	pcSymKeyID,
	USBKEY_UCHAR_PTR	pcWrapKeyID,
	USBKEY_UCHAR_PTR	pcSymKey,		
	USBKEY_ULONG_PTR	pulSymKeyLen);

// �����������ļ�
// ����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_ULONG CreateBinaryFile(
	USBKEY_HANDLE		hDeviceHandle,
	USBKEY_UCHAR_PTR	pcFileID,
	USBKEY_UCHAR		ucMode,
	USBKEY_UCHAR		ucReadRight,		
	USBKEY_UCHAR		ucWriteRight,
	USBKEY_ULONG		ulFileLen);

// ����Ӧ��PIN��
// ����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_ULONG CreateAppPin(
	USBKEY_HANDLE		hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR	pucPIN,			//[in]�����PIN
	USBKEY_ULONG		ulPINLen,		//[in]�����PIN����
	USBKEY_UCHAR		ucPinFlag,		//[in]PIN���ʶ
	USBKEY_UCHAR		ucSecState,		//[in]PIN��֤��ȫ״̬
	USBKEY_UCHAR		ucRetryCounts,	//[in]���Դ���
	USBKEY_UCHAR		ulUseRights,	//[in]ʹ��Ȩ��
	USBKEY_UCHAR		ulChangeRights);	//[in]����Ȩ��

USBKEY_ULONG EraseAC4Key(USBKEY_HANDLE hDeviceHandle);

#ifdef __cplusplus
}
#endif
#endif
