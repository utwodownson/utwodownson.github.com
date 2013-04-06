/*	DataPackage.h
Author  :   caizhun@aisino.com
CO.     :   AISINO
date    :   2008��6��6�գ�
����    :   USBKEY�ײ������������� */


#include "./inc/UsbKeyAPI.h"

//����֧�ֵĲ�������
//////////////////////////////////////////////////////////////////////////
#define USBKEY_OP_ExternalAuth		0x00000001
#define USBKEY_OP_InternalAuth		0x00000002
#define USBKEY_OP_GetChallenge		0x00000003
#define USBKEY_OP_GetResponse		0x00000004
#define USBKEY_OP_CreateFile		0x00000005
#define USBKEY_OP_DeleteFile		0x00000006
#define USBKEY_OP_SelectFile		0x00000007
#define USBKEY_OP_ReadBinary		0x00000008
#define USBKEY_OP_UpdateBinary		0x00000009
#define USBKEY_OP_ReadRecord		0x00000010
#define USBKEY_OP_UpdateRecord		0x00000011
#define USBKEY_OP_AppendRecord		0x00000012
#define USBKEY_OP_VerifyPIN			0x00000013
#define USBKEY_OP_ChangePIN			0x00000014
#define USBKEY_OP_UnlockPIN			0x00000015
#define USBKEY_OP_ReloadPIN			0x00000016
#define USBKEY_OP_AppBlock			0x00000017
#define USBKEY_OP_AppUnblock		0x00000018
#define USBKEY_OP_CardBlock			0x00000019
#define USBKEY_OP_GetDeviceInfo		0x00000020
#define USBKEY_OP_ResetDevice		0x00000021
#define USBKEY_OP_ActivateSM		0x00000022
//#define USBKEY_OP_UpdatePublicKeyBinary			0x00000039
#define USBKEY_OP_UpdatePrivateKeyBinary		0x00000040
#define USBKEY_OP_UpdatePrivateKeyBinaryTemp	0x00000041
#define USBKEY_OP_UpdateProtectedBinary			0x00000042
//#define USBKEY_OP_UpdatePrivateKeyBinaryPlain	0x00000043

#define USBKEY_OP_GenRSAKeyPair		0x00000030
#define USBKEY_OP_WriteKey			0x00000031
#define USBKEY_OP_PrivateKeyOP		0x00000032
#define USBKEY_OP_PublicKeyOP		0x00000033
#define USBKEY_OP_DataEncrypt		0x00000034
#define USBKEY_OP_DataDecrypt		0x00000035
#define USBKEY_OP_GenSymmKey		0x00000036
#define USBKEY_OP_ExportSymmKey		0x00000037
#define USBKEY_OP_ImportSymmKey		0x00000038
#define USBKEY_OP_ExportDivSymmKey	0x00000044
#define USBKEY_OP_ImportDivSymmKey	0x00000045
#define USBKEY_OP_MassiveDataEncrypt		0x00000046
#define USBKEY_OP_MassiveDataDecrypt		0x00000047
// #define USBKEY_OP_EncodeEnv			0x00000037
// #define USBKEY_OP_DecodeEnv			0x00000038
// #define USBKEY_OP_GenSonKey			0x00000039
#define USBKEY_OP_SetDevInfo		0x00000048
#define USBKEY_OP_ImportPrivateKey	0x00000049
#define USBKEY_OP_ExportSymmKeybyEcc	0x00000050
#define USBKEY_OP_ImportECCEnSymmKey	0x00000051
#define USBKEY_OP_TempSymmKeyOP			0x00000052

#define USBKEY_OP_GenExtRSAKeyPair		0x00000053
#define USBKEY_OP_TempRSAOP				0x00000054

#define USBKEY_OP_GenECCKeyPair			0x00000055
#define USBKEY_OP_ECCPubKeyEncrypt		0x00000056
#define USBKEY_OP_ECCPriKeyDecrypt		0x00000057
#define USBKEY_OP_ECCSign				0x00000058
#define USBKEY_OP_ECCVerify				0x00000059
#define USBKEY_OP_ECCExportAgreeData	0x00000060
#define USBKEY_OP_ECCImportAgreeData	0x00000061
//������������	CLA
#define	USBKEY_PKI_CODE			0xb0		//pki���������
#define	USBKEY_DEVICE_CODE		0xb8		//�豸���������
#define	USBKEY_LOCK_CODE		0x84		//���������������
#define USBKEY_FILE_CODE		0x80		//�ļ����������
#define USBKEY_DEFAULT_CODE		0x00		//�������������

//������������	INS
#define USBKEY_CMD_DEVICE_RESET			0x01
#define USBKEY_CMD_GET_DEVICE_INFO		0x03
#define USBKEY_CMD_DEVICE_SET_INFO		0x04
#define USBKEY_CMD_CREATE_FILE			0xe0		//�����ļ�
#define USBKEY_CMD_DELETE_FILE			0x02
//#define USBKEY_CMD_WRITE_KEY			0x31
#define USBKEY_CMD_WRITE_KEY			0xd4		// install key
#define USBKEY_CMD_READ_BINARY			0xb0		//���������ļ�
#define USBKEY_CMD_UPDATE_BINARY		0xd6		//���¶������ļ�
#define USBKEY_CMD_READ_RECORD			0xb2
#define USBKEY_CMD_UPDATE_RECORD		0xdc
#define USBKEY_CMD_APPEND_RECORD		0xe2
#define USBKEY_CMD_ACTIVATE_SECMACH		0xac
#define USBKEY_CMD_SELECT_FILE			0xa4
#define USBKEY_CMD_VERIFY_PIN			0x20
#define USBKEY_CMD_CHANGE_PIN			0x5e
#define USBKEY_CMD_INTERNAL_AUTHENT		0x88
#define USBKEY_CMD_EXTERNAL_AUTHENT		0x82
#define USBKEY_CMD_GET_CHALLENGE		0x84
#define USBKEY_CMD_UNLOCK_PIN			0x24		//����PIN
#define USBKEY_CMD_LOCK_APP				0x1e		//Ӧ������
#define USBKEY_CMD_UNLOCK_APP			0x18		//����Ӧ��
#define USBKEY_CMD_LOCK_DEVICE			0x16		//�豸����
#define USBKEY_CMD_RELOAD_PIN			0x5e		//����PIN

#define USBKEY_CMD_IMPORT_SYM_KEY		0x44		//����Գ���Կ
#define USBKEY_CMD_EXPORT_SYM_KEY		0x45		//�����Գ���Կ
#define USBKEY_CMD_IMPORT_PRIVATE_KEY	0x4a		//�����ǶԳ�˽Կ
#define USBKEY_CMD_GEN_SYM_KEY			0x36		//���ɶԳ���Կ
#define USBKEY_CMD_GEN_RSA_KEY_PAIR		0x30		//����RSA��˽Կ��
#define USBKEY_CMD_RSA_PRIKEY_OPERATION	0x32		//RSA˽Կ����
#define USBKEY_CMD_RSA_PUBKEY_OPERATION	0x33		//RSA��Կ����
#define USBKEY_CMD_GEN_EXT_RSA_KEY_PAIR	0x4e		//�����ⲿRSA��˽Կ��
#define	USBKEY_CMD_TEMP_RSA_OPERATION	0x4f		//��ʱRSA��˽Կ����
#define USBKEY_CMD_SYM_KEY_ENCRYPT		0x34		//�Գ���Կ�����������
#define USBKEY_CMD_SYM_KEY_DECRYPT		0x35		//�Գ���Կ�����������
#define USBKEY_CMD_SYM_KEY_MASSIVE_ENCRYPT		0x42		//�Գ���Կ������������
#define USBKEY_CMD_SYM_KEY_MASSIVE_DECRYPT		0x43		//�Գ���Կ������������
#define USBKEY_CMD_TEMP_SYM_KEY_OPERATION	0x4b		//��ʱ�Գ���Կ�ӽ�������

#define USBKEY_CMD_ECC_IMPORT_SYMKEY	0x40		//ECC���ܵ���Գ���Կ
#define USBKEY_CMD_ECC_EXPORT_SYMKEY	0x41		//ECC���ܵ����Գ���Կ
#define USBKEY_CMD_GEN_ECC_KEY_PAIR		0x3b		//����ECC��Կ�ԣ�����/��ʱ��
#define USBKEY_CMD_ECC_ENCRYPT			0x3d		//ECC���ܣ�����/�ⲿ��
#define USBKEY_CMD_ECC_DECRYPT			0x3c		//ECC���ܣ�����/�ⲿ��
#define USBKEY_CMD_ECC_SIGN				0x3e		//ECCǩ��������/�ⲿ��
#define USBKEY_CMD_ECC_VERIFY			0x3f		//ECC��ǩ������/�ⲿ��
#define USBKEY_CMD_EXPORT_AGREEMENT		0x50		//����ECC��ԿЭ�̲���
#define USBKEY_CMD_IMPORT_AGREEMENT		0x51		//����ECC��ԿЭ�̲���
//////////////////////////////////////////////////////////////////////////

//USBKEY_ULONG PackageData(USBKEY_UCHAR cP1, USBKEY_UCHAR cP2,USBKEY_UCHAR cLc,USBKEY_UCHAR_PTR pcData,USBKEY_UCHAR cLe,USBKEY_ULONG ulOPType,USBKEY_UCHAR_PTR pcPackagedData,USBKEY_ULONG_PTR pulDataLen);

USBKEY_ULONG PackageData(USBKEY_HANDLE	hDeviceHandle,USBKEY_UCHAR cP1, USBKEY_UCHAR cP2,USBKEY_INT32 uLc,USBKEY_UCHAR_PTR pcData,USBKEY_INT32 uLe,
							USBKEY_ULONG ulOPType,USBKEY_UCHAR_PTR pResponsedData,USBKEY_ULONG_PTR pulDataLen);