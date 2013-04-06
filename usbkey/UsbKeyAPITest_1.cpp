// UsbKeyAPITest.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "stdio.h"
#include "stdlib.h"
#include "windows.h"
#include "memory.h"
#include "wincrypt.h"

#include "../UsbKeyAPI/inc/UsbKeyAPI.h"

int _tmain(int argc, _TCHAR* argv[])
{
	long rtn = 0;;

	USBKEY_UCHAR pubKeyID[2];
	USBKEY_UCHAR pID[4];
	USBKEY_UCHAR priKeyID[2];
	USBKEY_HANDLE hDeviceHandle,hDeviceHandle1;
	USBKEY_UCHAR** ppcDeviceIdS;
	USBKEY_UCHAR_PTR	pcDeviceIdS;
	USBKEY_UCHAR pcDirName[3],pcFileName[2];
	USBKEY_DEVINFO devinfo;
	USBKEY_FILEATTRIBUTE fileinfo;
	USBKEY_UCHAR temp[4096],out[4096];
	USBKEY_ULONG len,num;
	USBKEY_RSA_KEY rsakey;
	USBKEY_ULONG i;
	USBKEY_UCHAR pcIV[16],pcRandom[16],pTempKey[16];
	USBKEY_ULONG ulEncryptedLen;
	USBKEY_ULONG ulDevNameLen,DeviceQuantity,hDevState;
	USBKEY_KEYHEAD pKeyHead;
	USBKEY_FILEATTRIBUTE pFileAttr;
	USBKEY_ULONG uMaxRetryNum,bDefaultPIN;		//������Դ���
	USBKEY_ULONG ulRemainRetryNum;	//ʣ�����Դ���
	int pucDeviceQuantity = 0;
	USBKEY_RSA_PRIVATE_KEY_2048 RsaPrivate;
	USBKEY_RSA_PUBLIC_KEY_2048 RsaPublic;
	USBKEY_RSA_PRIVATE_KEY_IMPORT_2048 RsaImportPrivate;
	pcDirName[0] = 0x3f;
	pcDirName[1] = 0x03;
	pcFileName[0] = 0x80;
	pcFileName[1] = 0x00;

	//
	
	rtn = USBKEY_CheckDeviceEvent(NULL,&ulDevNameLen,&DeviceQuantity);
	if (rtn !=0)
	{
		printf("USBKEY_CheckDeviceEvent error!");
	}
	
	if(ulDevNameLen != 0)
	{
		pcDeviceIdS = (USBKEY_UCHAR*)malloc(ulDevNameLen*sizeof(USBKEY_UCHAR));
		memset(pcDeviceIdS,0,ulDevNameLen);
		rtn = USBKEY_CheckDeviceEvent(pcDeviceIdS,&ulDevNameLen,&DeviceQuantity);
		if (rtn !=0)
		{
			printf("USBKEY_CheckDeviceEvent error!");
		}
	}
	rtn = USBKEY_DeviceEnum(NULL,(USBKEY_INT32_PTR)&pucDeviceQuantity);
	if (rtn !=0)
	{
		printf("enum error!");
	}
	ppcDeviceIdS = (USBKEY_UCHAR**)malloc(pucDeviceQuantity*sizeof(USBKEY_UCHAR*));
	for(i=0; i<pucDeviceQuantity; i++)
	{
		ppcDeviceIdS[i] = (USBKEY_UCHAR*)malloc(255);
	}

	rtn += USBKEY_DeviceEnum((USBKEY_UCHAR **)ppcDeviceIdS,&pucDeviceQuantity);
	
	rtn += USBKEY_DeviceCheckExist(pcDeviceIdS+1,&hDevState);
 	if (rtn !=0)
	{
		printf("USBKEY_DeviceCheckExist error!");
	}
	
	rtn = USBKEY_CheckDeviceEvent(NULL,&ulDevNameLen,&DeviceQuantity);
	if (rtn !=0)
	{
		printf("USBKEY_CheckDeviceEvent error!");
	}
	
	if(ulDevNameLen != 0)
	{
		pcDeviceIdS = (USBKEY_UCHAR*)malloc(ulDevNameLen*sizeof(USBKEY_UCHAR));
		memset(pcDeviceIdS,0,ulDevNameLen);
		rtn = USBKEY_CheckDeviceEvent(pcDeviceIdS,&ulDevNameLen,&DeviceQuantity);
		if (rtn !=0)
		{
			printf("USBKEY_CheckDeviceEvent error!");
		}
	}

	rtn += USBKEY_DeviceOpen(ppcDeviceIdS[0], 1, &hDeviceHandle);

	rtn += USBKEY_DeviceOpen(ppcDeviceIdS[0], 1, &hDeviceHandle1);
	
	//��ʼ���豸
	USBKEY_VERSION appVer;
	appVer.major = 1;
	appVer.minor = 0;
	USBKEY_UCHAR pcManufacturer[64] = "Aero-info";
	USBKEY_UCHAR pcLabel[64] = "AKey20";
	memset(pcManufacturer+strlen("Aero-info"), ' ', 64-strlen("Aero-info"));
	memset(pcLabel+strlen("AKey20"), ' ', 64-strlen("AKey20"));

	rtn += USBKEY_DeviceInit(hDeviceHandle, CHIP_TYPE_45, appVer, pcManufacturer, pcLabel);

	//���������
 	rtn += USBKEY_GenRandom(hDeviceHandle,pcRandom,16);

	//��ʱ�ԳƼ���
	memset(pTempKey,0x01,16);
	rtn += USBKEY_TempDataEncrypt(hDeviceHandle,pTempKey,ALG_TYPE_SCB2,USBKEY_ECB|USBKEY_NO_PADDING,NULL,pcRandom,16,out,&ulEncryptedLen);
	//�ⲿ��֤
	rtn += USBKEY_ExtAuthenticate(hDeviceHandle,out,ulEncryptedLen);
	rtn += USBKEY_TempDataDecrypt(hDeviceHandle,pTempKey,ALG_TYPE_SCB2,USBKEY_ECB|USBKEY_NO_PADDING,NULL,out,16,temp,&ulEncryptedLen);
	
	//rtn += USBKEY_CheckDeviceAdminPin(hDeviceHandle,(USBKEY_UCHAR_PTR)USBKEY_DEVICE_ADMIN_PIN,6,&len);
	
	
	
	/*�����ⲿ��֤��Կ
	pKeyHead.ApplicationType = KEY_EXTERNAL_AUTHENTICATE_KEY;
	pKeyHead.ErrorCounter = 5;
	pKeyHead.KeyLen = 16;
	pKeyHead.ulAlg = ALG_TYPE_SCB2;
	pKeyHead.ulPrivil = USBKEY_DEVICE_PRIVILEGE;
	memset(pcRandom,1,16);
	rtn += USBKEY_WriteKey(hDeviceHandle,(USBKEY_UCHAR_PTR)&pKeyHead,pcRandom);*/

	
	//����Ӧ��
	rtn += USBKEY_CreateApplication(hDeviceHandle,(USBKEY_UCHAR_PTR)"AKEY PKI APP",(USBKEY_UCHAR_PTR)"1111",4,5,(USBKEY_UCHAR_PTR)"1111",4,5,USBKEY_USERTYPE_USER);
	
	//check pin
	//rtn = USBKEY_CheckPin(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",USBKEY_USERTYPE_USER,(USBKEY_UCHAR_PTR)"1111",4,&len);

	//�����豸��Ϣ
	rtn += USBKEY_DeviceSetLabel(hDeviceHandle,pcLabel);

	rtn +=  USBKEY_OpenApplication(hDeviceHandle,(USBKEY_UCHAR_PTR)"AKEY PKI APP");

	//���PIN��Ϣ
	rtn = USBKEY_CheckPin(hDeviceHandle,USBKEY_USERTYPE_USER,(USBKEY_UCHAR_PTR)"1111",4,&len);
	rtn = USBKEY_GetPinInfo(hDeviceHandle,USBKEY_USERTYPE_USER,&uMaxRetryNum,&ulRemainRetryNum);
	rtn = USBKEY_GetPinInfo(hDeviceHandle,USBKEY_USERTYPE_ADMIN,&uMaxRetryNum,&ulRemainRetryNum);
	/*//�����ʱ˽Կ
	len = 2048;
	memset(&RsaPrivate,0,sizeof(USBKEY_RSA_PRIVATE_KEY_2048));
	rtn += USBKEY_GenerateExtRSAKey(hDeviceHandle,len,(USBKEY_UCHAR_PTR)&RsaPrivate);
	RsaPublic.bits = RsaPrivate.bits;
	memcpy(RsaPublic.exponent,RsaPrivate.publicExponent,MAX_RSA_MODULUS_LEN_2048);
	memcpy(RsaPublic.modulus,RsaPrivate.modulus,MAX_RSA_MODULUS_LEN_2048);
	RsaImportPrivate.bits = RsaPrivate.bits;
	memcpy(RsaImportPrivate.prime,RsaPrivate.prime,MAX_RSA_MODULUS_LEN_2048);
	memcpy(RsaImportPrivate.primeExponent,RsaPrivate.primeExponent,MAX_RSA_MODULUS_LEN_2048);
	memcpy(RsaImportPrivate.coefficient,RsaPrivate.coefficient,MAX_RSA_PRIME_LEN_2048);*/

	//ö������
	/*	rtn = USBKEY_EnumContainer(hDeviceHandle,NULL,&len);
		rtn = USBKEY_EnumContainer(hDeviceHandle,temp,&len);

	rtn = USBKEY_DeleteContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"keystore1239869624");*/
	//��ʼ��PIN
	//rtn = USBKEY_CheckPin(hDeviceHandle,USBKEY_USERTYPE_ADMIN,(USBKEY_UCHAR_PTR)"1111",4,&len);
	//rtn = USBKEY_InitPin(hDeviceHandle,(USBKEY_UCHAR_PTR)"111111",6);

//	rtn += USBKEY_CreateContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1");

// 
	//�����Գ���Կ
//	memset(temp,0,128);
//	rtn += USBKEY_GenSymmetricKey(hDeviceHandle,8, 
//				ALG_TYPE_DES, USBKEY_CRYPT_EXPORTABLE|USBKEY_CRYPT_ONDEVICE|USBKEY_CRYPT_USER_PROTECTED, (USBKEY_UCHAR_PTR)&pcFileName);

	//���ĵ���Գ���Կ
/*	rtn += USBKEY_ImportSymmetricKey(
				hDeviceHandle,						//[in]�豸���
				NULL,								//[in]��������������Կ����ԿID
				USBKEY_CRYPT_ONDEVICE,			//[in]��Կ���ͣ���ʾ��Կ�ǿ��ڹ̶���Կ������ʱ��Կ����ָ��ΪUSBKEY_CRYPT_ONDEVICE����Ϊ0
				USBKEY_ULONG ulAlgID,			//[in]��Կ�㷨��ʶ
				USBKEY_UCHAR_PTR pcSymKey,		//[in]�������Կ
				USBKEY_ULONG ulSymKeyLen,		//[in]�������Կ����
				USBKEY_UCHAR_PTR pcSymKeyID)	//[out]���ص���ĶԳ���ԿID
*/
	//���ݼ���
/*	memset(temp,'1',sizeof(temp));
	memset(out,0,sizeof(out));
	rtn += USBKEY_DataEncrypt(hDeviceHandle,pcFileName,temp,8,out,&len);

	//���ݽ���
	memset(temp,0,sizeof(temp));
	len = 8;
 	rtn += USBKEY_DataDecrypt(hDeviceHandle,pcFileName,out,len,temp,&len);
*/
	//�������ݼ���
/*	memset(temp,'1',sizeof(temp));
	memset(out,0,sizeof(out));
	memset(pcIV, 0x3, 16);
	ulEncryptedLen = 4096;
	rtn += USBKEY_MassiveDataEncrypt(
				hDeviceHandle,						//[in]�豸���
				pcFileName,							//[in]�������ܵ���ԿID
				ALG_TYPE_DES,						//[in]�����㷨
				USBKEY_CBC|USBKEY_NO_PADDING,	//[in]����ģʽ�����ģʽ
				pcIV,								//[in]��ʼ�������������ecbģʽ������ΪNULL
				temp,								//[in]����������
				1032,								//[in]���������ݳ���
				out,								//[out]���صļ��ܺ�����
				&ulEncryptedLen);//[in/out]in: pcEncryptedData��������С��out: ���ܺ����ݳ��Ȼ���Ҫ��pcEncryptedData��������С
	if( 0 != rtn )
	{
		printf("USBKEY_MassiveDataEncrypt error!");
		return 1;
	}
		

	//�������ݽ���
	memset(temp,0,sizeof(temp));
	len = 4096;
	rtn += USBKEY_MassiveDataDecrypt(
				hDeviceHandle,						//[in]�豸���
				pcFileName,							//[in]�������ܵ���ԿID
				ALG_TYPE_DES,						//[in]�����㷨
				USBKEY_CBC|USBKEY_NO_PADDING,	//[in]����ģʽ�����ģʽ
				pcIV,								//[in]��ʼ�������������ecbģʽ������ΪNULL
				out,								//[in]��������
				ulEncryptedLen,						//[in]�������ݳ���
				temp,								//[out]���ص�����
				&len);								//[in/out]in: pcData���������� out:���ص����ĳ���
	if( 0 != rtn )
	{
		printf("USBKEY_MassiveDataDecrypt error!");
		return 1;
	}
*/

//ö��Ӧ��
	rtn = USBKEY_EnumApplication(hDeviceHandle,NULL,&len,&num);
	rtn = USBKEY_EnumApplication(hDeviceHandle,temp,&len,&num);

	rtn +=  USBKEY_OpenApplication(hDeviceHandle,(USBKEY_UCHAR_PTR)"AKEY PKI APP");
	//rtn = USBKEY_CheckPin(hDeviceHandle,USBKEY_USERTYPE_USER,(USBKEY_UCHAR_PTR)"111111",6,&len);

/*//ö���ļ�
	rtn = USBKEY_EnumFile(hDeviceHandle,NULL,&len);
	rtn = USBKEY_EnumFile(hDeviceHandle,temp,&len);
//ö������
	rtn = USBKEY_EnumContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",NULL,&len);
	rtn = USBKEY_EnumContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",temp,&len);
	
//��ȡ�豸��Ϣ
	 	memset(&fileinfo,0,sizeof(fileinfo));
	 	rtn = USBKEY_GetFileAttribute(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"12",(USBKEY_UCHAR_PTR)&fileinfo);

*/

/*
	//��ȡ�豸��Ϣ
 	rtn = USBKEY_DeviceGetStat(hDeviceHandle, &devinfo);

	//�����豸
 	rtn = USBKEY_DeviceReset(hDeviceHandle);

// 	rtn = USBKEY_DeleteApplication(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1");
 //	rtn = USBKEY_CreateApplication(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"111111",6,(USBKEY_UCHAR_PTR)"111111",6,temp,16);

	//��֤PIN
	rtn = USBKEY_CheckPin(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",USBKEY_USERTYPE_USER,(USBKEY_UCHAR_PTR)"111111",6,&len);


	//����PIN
	rtn = USBKEY_UnlockPin(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"111111",6,
		(USBKEY_UCHAR_PTR)"222222",6,&len);

	//��֤PIN
	rtn = USBKEY_CheckPin(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",USBKEY_USERTYPE_USER,(USBKEY_UCHAR_PTR)"222222",6,&len);

	//��֤PIN
	rtn = USBKEY_CheckPin(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",USBKEY_USERTYPE_ADMIN,(USBKEY_UCHAR_PTR)"111111",6,&len);

	//��ʼ��PIN
	rtn = USBKEY_InitPin(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"222222",6);

	//��֤PIN
	rtn = USBKEY_CheckPin(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",USBKEY_USERTYPE_USER,(USBKEY_UCHAR_PTR)"111111",6,&len);

	//��֤PIN
	rtn = USBKEY_CheckPin(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",USBKEY_USERTYPE_USER,(USBKEY_UCHAR_PTR)"222222",6,&len);

	//�޸�PIN
	rtn = USBKEY_ChangePin(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",USBKEY_USERTYPE_USER,(USBKEY_UCHAR_PTR)"222222",6,
		(USBKEY_UCHAR_PTR)"111111",6,&len);
*/

//*

// 	rtn = USBKEY_CreateApplication(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"111111",6,(USBKEY_UCHAR_PTR)"111111",6,temp,16);
// 	rtn = USBKEY_CreateFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"12",10,USBKEY_USERTYPE_USER);
// 	rtn = USBKEY_CreateApplication(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp2",(USBKEY_UCHAR_PTR)"111111",6,(USBKEY_UCHAR_PTR)"111111",6,temp,16);
// 	rtn = USBKEY_CreateFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp2",(USBKEY_UCHAR_PTR)"12",10,USBKEY_USERTYPE_USER);
// 	rtn = USBKEY_DeleteFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"12");
//  	rtn = USBKEY_DeleteApplication(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1");
// 	rtn = USBKEY_DeleteFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp2",(USBKEY_UCHAR_PTR)"12");
// 	rtn = USBKEY_DeleteApplication(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp2");

/*
	//���Ӧ��
	rtn += USBKEY_CreateApplication(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"111111",6,(USBKEY_UCHAR_PTR)"111111",6);
//	rtn += USBKEY_CreateApplication(hDeviceHandle,(USBKEY_UCHAR_PTR)"App2",(USBKEY_UCHAR_PTR)"111111",6,(USBKEY_UCHAR_PTR)"111111",6);
//	rtn += USBKEY_CreateApplication(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp3",(USBKEY_UCHAR_PTR)"111111",6,(USBKEY_UCHAR_PTR)"111111",6);

	rtn = USBKEY_CheckPin(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",USBKEY_USERTYPE_USER,(USBKEY_UCHAR_PTR)"111111",6,&len);

	//ɾ��Ӧ��
//	rtn = USBKEY_DeleteApplication(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1");
// 	rtn = USBKEY_DeleteApplication(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp2");
// 	rtn = USBKEY_DeleteApplication(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp3");

	//ö��Ӧ��
// 	rtn = USBKEY_EnumApplication(hDeviceHandle,NULL,&len);
// 	rtn = USBKEY_EnumApplication(hDeviceHandle,temp,&len);
//*/



		rtn = USBKEY_CheckPin(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",USBKEY_USERTYPE_USER,(USBKEY_UCHAR_PTR)"1111",4,&len);

		//��ȡ�ǶԳ���ԿID
		//rtn += USBKEY_GetAsymmetricKeyID(hDeviceHandle,(USBKEY_UCHAR_PTR)"App1",(USBKEY_UCHAR_PTR)"Container1",AT_KEYEXCHANGE,(USBKEY_UCHAR_PTR)&rsakey);

		/*//�����Գ���Կ
		memset(temp,0,128);
		rtn += USBKEY_GenSymmetricKey(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",24, 
		ALG_TYPE_3DES_3KEY, USBKEY_CRYPT_EXPORTABLE|USBKEY_CRYPT_ONDEVICE|USBKEY_CRYPT_USER_PROTECTED, (USBKEY_UCHAR_PTR)&pcFileName);

		//���ݼ���
		memset(temp,'1',sizeof(temp));
		memset(out,0,sizeof(out));
		//rtn += USBKEY_DataEncrypt(hDeviceHandle,(USBKEY_UCHAR_PTR)"App1",pcFileName,temp,8,out,&len);


		//������Կ
		memset(temp,0,sizeof(temp));
		rtn += USBKEY_ExportSymmetricKey(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",pcFileName,pubKeyID,temp,&len);

		//������Կ
		priKeyID[0] = 0x01;
		priKeyID[1] = 0x02;
		//rtn += USBKEY_ImportSymmetricKey(hDeviceHandle,(USBKEY_UCHAR_PTR)"App1",priKeyID,USBKEY_CRYPT_ONDEVICE,temp,len,pcFileName);
//		*/




	//�����豸
 	//rtn = USBKEY_DeviceReset(hDeviceHandle);
	//rtn += USBKEY_CreateFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"Testfile1",1024,USBKEY_ALL_PRIVILEGE,USBKEY_ALL_PRIVILEGE);
	//rtn = USBKEY_CheckPin(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",USBKEY_USERTYPE_USER,(USBKEY_UCHAR_PTR)"1111",4,&len);
	
	//�����ļ�
 	//rtn += USBKEY_CreateFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"Testfile1",1024,USBKEY_ALL_PRIVILEGE,USBKEY_ALL_PRIVILEGE);
	//rtn += USBKEY_CreateFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"Testfile2",10,USBKEY_USERTYPE_USER,USBKEY_USERTYPE_USER);
 	//rtn += USBKEY_CreateFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"Testfile3",10,USBKEY_USERTYPE_USER,USBKEY_USERTYPE_USER);
// 	rtn += USBKEY_CreateFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp2",(USBKEY_UCHAR_PTR)"12",10,USBKEY_USERTYPE_USER);
// 	rtn += USBKEY_CreateFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp2",(USBKEY_UCHAR_PTR)"13",10,USBKEY_USERTYPE_USER);
// 	rtn += USBKEY_CreateFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp2",(USBKEY_UCHAR_PTR)"14",10,USBKEY_USERTYPE_USER);	
// 	rtn += USBKEY_CreateFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp3",(USBKEY_UCHAR_PTR)"12",10,USBKEY_USERTYPE_USER);
// 	rtn += USBKEY_CreateFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp3",(USBKEY_UCHAR_PTR)"13",10,USBKEY_USERTYPE_USER);
// 	rtn += USBKEY_CreateFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp3",(USBKEY_UCHAR_PTR)"14",10,USBKEY_USERTYPE_USER);	
	
	//ö���ļ�
 	//rtn = USBKEY_EnumFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",NULL,&len);
 	//rtn = USBKEY_EnumFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",temp,&len);

	//�������
	//rtn += USBKEY_CreateContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"testContainer1");
	//rtn += USBKEY_CreateContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"testContainer1");
	//rtn += USBKEY_CreateContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"testContainer3");

	/*//����ļ�����
	rtn = USBKEY_GetFileAttribute(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1", (USBKEY_UCHAR_PTR)"Testfile2",(USBKEY_UCHAR_PTR)&pFileAttr);
	
	//��д�ļ�
 	len = 8;
 	rtn = USBKEY_WriteFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"Testfile2",0,(USBKEY_UCHAR_PTR)"testData",&len);
 	memset(temp,0,128);
 	len = 8;
 	rtn = USBKEY_ReadFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"Testfile2",0,temp,&len);

	//ɾ���ļ�
 	rtn = USBKEY_DeleteFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"Testfile1");
  	rtn = USBKEY_DeleteFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"Testfile2");
  	rtn = USBKEY_DeleteFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"Testfile3");

	//ö���ļ�
 	rtn = USBKEY_EnumFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",NULL,&len);
 	rtn = USBKEY_EnumFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",temp,&len);
	*/
	//�������
	rtn = USBKEY_CheckPin(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",USBKEY_USERTYPE_USER,(USBKEY_UCHAR_PTR)"1111",4,&len);
	rtn += USBKEY_CreateContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"testContainer1");
	/*//rtn = USBKEY_DeleteContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"testContainer1");
	//���������
 	rtn += USBKEY_GenRandom(hDeviceHandle,pcRandom,16);
	rtn = USBKEY_ImportContainerSymmetricKey(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"testContainer1",
									NULL,USBKEY_CRYPT_ONDEVICE|USBKEY_CRYPT_USER_PROTECTED,ALG_TYPE_SCB2,pcRandom,16,pcFileName);
	
	rtn = USBKEY_DeleteContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"testContainer1");
	*/
	//rtn += USBKEY_CreateContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"testContainer1");
	//rtn += USBKEY_CreateContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"testContainer3");

	// 	rtn += USBKEY_CreateContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp2",(USBKEY_UCHAR_PTR)"testContainer4");
// 	rtn += USBKEY_CreateContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp2",(USBKEY_UCHAR_PTR)"testContainer5");
// 	rtn += USBKEY_CreateContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp2",(USBKEY_UCHAR_PTR)"testContainer6");
// 	rtn += USBKEY_CreateContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp3",(USBKEY_UCHAR_PTR)"testContainer7");
// 	rtn += USBKEY_CreateContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp3",(USBKEY_UCHAR_PTR)"testContainer8");
// 	rtn += USBKEY_CreateContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp3",(USBKEY_UCHAR_PTR)"testContainer9");
	//ɾ������
// 	rtn = USBKEY_DeleteContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"testContainer1");
// 	rtn = USBKEY_DeleteContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"testContainer2");
// 	rtn = USBKEY_DeleteContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"testContainer3");
// 	rtn = USBKEY_DeleteContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp2",(USBKEY_UCHAR_PTR)"testContainer4");
// 	rtn = USBKEY_DeleteContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp2",(USBKEY_UCHAR_PTR)"testContainer5");
// 	rtn = USBKEY_DeleteContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp2",(USBKEY_UCHAR_PTR)"testContainer6");
// 	rtn = USBKEY_DeleteContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp3",(USBKEY_UCHAR_PTR)"testContainer7");
// 	rtn = USBKEY_DeleteContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp3",(USBKEY_UCHAR_PTR)"testContainer8");
// 	rtn = USBKEY_DeleteContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp3",(USBKEY_UCHAR_PTR)"testContainer9");
	//ö������
// 	rtn = USBKEY_EnumContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",NULL,&len);
// 	rtn = USBKEY_EnumContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",temp,&len);

//rtn = USBKEY_CheckPin(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",USBKEY_USERTYPE_USER,(USBKEY_UCHAR_PTR)"111111",6,&len);

// 	//�����Գ���Կ
// 	memset(temp,0,128);
// 	rtn += USBKEY_GenSymmetricKey(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",24, 
// 	ALG_TYPE_3DES_3KEY, USBKEY_CRYPT_EXPORTABLE|USBKEY_CRYPT_ONDEVICE|USBKEY_CRYPT_USER_PROTECTED, (USBKEY_UCHAR_PTR)&pcFileName);
// 
// 	//���ݼ���
// 	memset(temp,'1',sizeof(temp));
// 	memset(out,0,sizeof(out));
// 	rtn += USBKEY_DataEncrypt(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",pcFileName,temp,8,out,&len);
// 
// 
// 	//������Կ
// 	memset(temp,0,sizeof(temp));
// 	rtn += USBKEY_ExportSymmetricKey(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",pcFileName,pubKeyID,temp,&len);
// 
// 	//������Կ
// 	priKeyID[0] = 0x01;
// 	priKeyID[1] = 0x02;
// 	rtn += USBKEY_ImportSymmetricKey(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",priKeyID,CRYPT_ONDEVICE,temp,len,pcFileName);
// 
// 	//���ݽ���
// 	memset(temp,0,sizeof(temp));
// 	len = 8;
// 	rtn += USBKEY_DataDecrypt(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",pcFileName,out,len,temp,&len);
//*/

/*
		rtn = USBKEY_DeleteApplication(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1");
		rtn += USBKEY_CreateApplication(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"111111",6,(USBKEY_UCHAR_PTR)"111111",6);
		rtn = USBKEY_CheckPin(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",USBKEY_USERTYPE_USER,(USBKEY_UCHAR_PTR)"111111",6,&len);
	 	//rtn = USBKEY_DeleteContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"testContainer1");
	 	rtn += USBKEY_CreateContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"testContainer1");
		//����RSA��Կ��
		rtn += USBKEY_GenerateRSAKeyPair(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"testContainer1",
			AT_KEYEXCHANGE,1024,CRYPT_USER_PROTECTED,pubKeyID,priKeyID);
	
		//
//����RSA��Կ��
//rtn += USBKEY_GenerateRSAKeyPair(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"testContainer1",
//								 AT_KEYEXCHANGE,1024,CRYPT_USER_PROTECTED,pubKeyID,priKeyID);
//*	
rtn = USBKEY_CheckPin(hDeviceHandle,(USBKEY_UCHAR_PTR)"Aisino App",USBKEY_USERTYPE_USER,(USBKEY_UCHAR_PTR)"1111",4,&len);

//  	rtn = USBKEY_DeleteContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"testContainer1");
// rtn += USBKEY_CreateContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"testContainer1");
// 
// len = 11;
// rtn = USBKEY_WriteCertificate(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"testContainer1",
// 	AT_SIGNATURE,(USBKEY_UCHAR_PTR)"testCertSig",&len);
// rtn = USBKEY_WriteCertificate(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"testContainer1",
// 	AT_KEYEXCHANGE,(USBKEY_UCHAR_PTR)"testCertExc",&len);
memset(temp,0,sizeof(temp));
rtn = USBKEY_ReadCertificate(hDeviceHandle,(USBKEY_UCHAR_PTR)"Aisino App",(USBKEY_UCHAR_PTR)"Aisino Cont",
	AT_KEYEXCHANGE,NULL,&len);
memset(temp,0,sizeof(temp));
rtn = USBKEY_ReadCertificate(hDeviceHandle,(USBKEY_UCHAR_PTR)"Aisino App",(USBKEY_UCHAR_PTR)"Aisino Cont",
	AT_KEYEXCHANGE,temp,&len);

 	rtn = USBKEY_DeleteContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"testContainer1");


rtn = USBKEY_CheckPin(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",USBKEY_USERTYPE_USER,(USBKEY_UCHAR_PTR)"111111",6,&len);
 rtn += USBKEY_GenerateRSAKeyPair(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"testContainer1",
 								 AT_KEYEXCHANGE,1024,CRYPT_USER_PROTECTED,pubKeyID,priKeyID);	

	//��ȡ�ǶԳ���ԿID
	rtn += USBKEY_GetAsymmetricKeyID(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"testContainer1",
	AT_KEYEXCHANGE,(USBKEY_UCHAR_PTR)&rsakey);

	pubKeyID[0] = 0x01;
	pubKeyID[1] = 0x01;
	priKeyID[0] = 0x01;
	priKeyID[1] = 0x02;
	//�����Գ���Կ
	memset(temp,0,128);
	rtn += USBKEY_GenSymmetricKey(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",24, 
	ALG_TYPE_3DES_3KEY, USBKEY_CRYPT_EXPORTABLE|USBKEY_CRYPT_ONDEVICE|USBKEY_CRYPT_USER_PROTECTED, (USBKEY_UCHAR_PTR)&pcFileName);

	//���ݼ���
	memset(temp,'1',sizeof(temp));
	memset(out,0,sizeof(out));
	rtn += USBKEY_DataEncrypt(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",pcFileName,temp,8,out,&len);


	//������Կ
	memset(temp,0,sizeof(temp));
	rtn += USBKEY_ExportSymmetricKey(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",pcFileName,pubKeyID,temp,&len);

	//������Կ
	priKeyID[0] = 0x01;
	priKeyID[1] = 0x02;
	rtn += USBKEY_ImportSymmetricKey(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",priKeyID,USBKEY_CRYPT_ONDEVICE,ALG_TYPE_3DES_3KEY, temp,len,pcFileName);

	//˽Կ����
	memset(temp,'1',sizeof(temp));
	memset(out,0,sizeof(out));
	rtn += USBKEY_RSAPrivateKeyOperation(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",
	priKeyID,temp,128,out,&len);

 	rtn = USBKEY_ExportRSAPublicKey(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",pubKeyID,temp,&len);
 
 
 	rtn = USBKEY_ImportRSAPublicKey(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"testContainer1",
 		AT_KEYEXCHANGE,temp,len,pubKeyID);
	*/

	/*//��Կ����
	len = 256;
	memset(temp,0,sizeof(temp));
	//rtn += USBKEY_RSAPublicKeyOperation(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",pubKeyID,out,len,temp,&len);

	//������ʱRSA��Կ
	rtn+= USBKEY_GenerateTempRSAKeyPair(hDeviceHandle);

	rtn = USBKEY_ImportTempRSAPublicKey(
				hDeviceHandle,	//[in]�豸���
				(USBKEY_UCHAR_PTR)&RsaPublic,//[in]�������Կ
				sizeof(RsaPublic),	//[in]�������Կ����
				pubKeyID);	//[out]���صĹ�ԿID
	memset(temp,0,sizeof(temp));
	rtn = USBKEY_ImportTempRSAPrivateKey(
				hDeviceHandle,	//[in]�豸���
				(USBKEY_UCHAR_PTR)&RsaImportPrivate,	//[in]�������Կ
				sizeof(RsaPrivate),	//[in]�������Կ����
				priKeyID);	//[out]���ص�˽ԿIDD
	//��ʱRSA����
	memset(temp,'1',sizeof(temp));
	memset(out,0,sizeof(out));
	rtn += USBKEY_TempRSAEncrypt(hDeviceHandle,
	temp,256,out,&len);

	//��ʱRSA����
	memset(temp,0,sizeof(temp));
	rtn += USBKEY_TempRSADecrypt(hDeviceHandle,
	out,len,temp,&len);

	rtn = USBKEY_ImportRSAPublicKey(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"testContainer1",
								AT_KEYEXCHANGE,(USBKEY_UCHAR_PTR)&RsaPublic,sizeof(RsaPublic),pubKeyID);
	rtn = USBKEY_ImportRSAPrivateKey(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",NULL,(USBKEY_UCHAR_PTR)"testContainer1",
								AT_KEYEXCHANGE,(USBKEY_UCHAR_PTR)&RsaImportPrivate,sizeof(RsaPrivate),priKeyID);
	//˽Կ����
	memset(temp,'1',sizeof(temp));
	memset(out,0,sizeof(out));
	rtn += USBKEY_RSAPrivateKeyOperation(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",
										 priKeyID,temp,256,out,&len);

	//��Կ����
	memset(temp,0,sizeof(temp));
	rtn += USBKEY_RSAPublicKeyOperation(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",
										pubKeyID,out,len,temp,&len);*/

/*	
//
//��֤PIN
rtn = USBKEY_CheckPin(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",USBKEY_USERTYPE_USER,(USBKEY_UCHAR_PTR)"111111",6,&len);

//rtn = USBKEY_DeleteFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"14");
//rtn += USBKEY_CreateFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"14",1024,USBKEY_USERTYPE_USER);
//ɾ���ļ�

// //��д�ļ�
// 	memset(temp,'1',sizeof(temp));
// 	len = sizeof(temp);
// 	temp[1] = '2';
// 	temp[2] = '3';
// 	temp[299] = '9';
// 	rtn = USBKEY_WriteFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"14",0,(USBKEY_UCHAR_PTR)temp,&len);
// 	memset(temp,0,sizeof(temp));
// 	//len = 300;
// 	rtn = USBKEY_ReadFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"14",0,temp,&len);


// pcFileName[0] = 0x20;
// //���ݼ���
// memset(temp,'1',sizeof(temp));
// memset(out,0,sizeof(out));
// rtn += USBKEY_DataEncrypt(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",pcFileName,temp,8,out,&len);

pubKeyID[0] = 0x01;
pubKeyID[1] = 0x01;
priKeyID[0] = 0x01;
priKeyID[1] = 0x02;

//˽Կ����
memset(temp,'1',sizeof(temp));
memset(out,0,sizeof(out));
rtn += USBKEY_RSAPrivateKeyOperation(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",
									 priKeyID,temp,128,out,&len);

//��Կ����
memset(temp,0,sizeof(temp));
rtn += USBKEY_RSAPublicKeyOperation(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",
									pubKeyID,out,len,temp,&len);
*/
rtn = USBKEY_GenerateRSAKeyPair(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"testContainer1",
 								 AT_KEYEXCHANGE,1024,CRYPT_USER_PROTECTED|USBKEY_CRYPT_ONDEVICE,pubKeyID,priKeyID);
rtn = USBKEY_DeviceReset(hDeviceHandle);
//��ȡ�ǶԳ���ԿID
rtn = USBKEY_GetAsymmetricKeyID(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"testContainer1",
	AT_KEYEXCHANGE,(USBKEY_UCHAR_PTR)&rsakey);

//rtn = USBKEY_ExportRSAPublicKey(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",pubKeyID,temp,&len);


//rtn = USBKEY_ImportRSAPublicKey(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"testContainer1",
//								AT_KEYEXCHANGE,temp,len,pubKeyID);

rtn = USBKEY_DeleteContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",(USBKEY_UCHAR_PTR)"testContainer1");
/*//��Կ����
memset(temp,0,sizeof(temp));
len = 128;
rtn += USBKEY_RSAPublicKeyOperation(hDeviceHandle,(USBKEY_UCHAR_PTR)"testApp1",
									pubKeyID,out,len,temp,&len);
*/
rtn += USBKEY_DeviceClose(hDeviceHandle);

	free(ppcDeviceIdS);

	printf("return val : %d\r\n",rtn);

	scanf("");

	return 0;
}

