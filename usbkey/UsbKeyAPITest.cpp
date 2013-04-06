// UsbKeyAPITest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "stdio.h"
#include "stdlib.h"
#include "memory.h"
#include <windows.h>
#include <commctrl.h>

#include "../UsbKeyAPI/inc/UsbKeyAPI.h"
#include "pkcs1_padding.h"

#define APP_PKI "AKEY PKI APP"
#define CON_NAME "testContainer1"
//设备管理员PIN
#define USBKEY_DEVICE_ADMIN_PIN		"Ai1@$USB6#&Key./"

int AK_AddParityCheck(USBKEY_UCHAR_PTR pBuf, USBKEY_ULONG ulLen)
{
	USBKEY_ULONG num = 0;
	USBKEY_UCHAR TempByte;
		
	USBKEY_ULONG i;
	USBKEY_ULONG j;

	
	for(i = 0; i<ulLen; i++)
	{
		TempByte = *(pBuf +i);
		//检查字节内1个数
		for(j = 0; j < 8; j++)
		{
			if(TempByte & 0x01)
				num++;
			TempByte>>=1;
		}

		//若此字节中1个数为奇数个，则不变，若为偶数个，最后一位取反
		if(!(num & 0x01))
		{
			if(*(pBuf +i) & 0x01)
				*(pBuf +i) &= 0xfe;
			else
				*(pBuf +i) |= 0x01;
				
		}

		num = 0;
	}
	
	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	unsigned long rtn = 0;
	int i;
	USBKEY_HANDLE hDeviceHandle;
	USBKEY_UCHAR_PTR pcDeviceIdS;
	int pucDeviceQuantity = 0;
	USBKEY_UCHAR NameList[256];
	USBKEY_ULONG ulDevNameLen,DeviceQuantity,hDevState;

	USBKEY_ULONG len,num,ulEncryptedLen;
	USBKEY_DEVINFO devinfo;
	USBKEY_UCHAR temp[4096],out[4096];
	USBKEY_FILEATTRIBUTE fileinfo;

	USBKEY_UCHAR pubKeyID[2],pFileID[2];
	USBKEY_UCHAR priKeyID[2],phKey[2];
	USBKEY_RSA_KEY rsakey;
	USBKEY_UCHAR pcIV[16];

	USBKEY_KEYHEAD pKeyHead;
	USBKEY_UCHAR pcRandom[16],pTempKey[16],pTest[17] = {0x00,0xf8,0x7b,0x5f,0xb7,0xcd,0xf3,0x0e,0xa0,0xfc,0x25,0xf8,0x7d,0xc7,0x5e,0xe0,0x50};

	USBKEY_RSA_PRIVATE_KEY_1024 rsakeyPair;
	USBKEY_RSA_PRIVATE_KEY_IMPORT_1024 priKeyR;
	USBKEY_RSA_PUBLIC_KEY_1024 pubKeyR;

	USBKEY_RSA_PRIVATE_KEY_1024 RsaPrivate;
	USBKEY_RSA_PUBLIC_KEY_1024 RsaPublic;
	USBKEY_RSA_PRIVATE_KEY_IMPORT_1024 RsaImportPrivate;
	USBKEY_CHAR ppcDeviceIdS[10][260];

	/*rtn = USBKEY_CheckDeviceEvent(NULL,&ulDevNameLen,&DeviceQuantity);
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
	}*/

	pucDeviceQuantity = 10;
	rtn += USBKEY_DeviceEnum(ppcDeviceIdS,&pucDeviceQuantity);

	rtn += USBKEY_DeviceOpen(ppcDeviceIdS[0], 1, &hDeviceHandle);

	//rtn += USBKEY_DeviceCheckExist(pcDeviceIdS+1,&hDevState);
 	if (rtn !=0)
	{
		printf("USBKEY_DeviceCheckExist error!");
	}
	
	//初始化设备
	USBKEY_VERSION appVer;
	appVer.major = 1;
	appVer.minor = 0;
	USBKEY_UCHAR pcManufacturer[64] = "Aero-info";
	USBKEY_UCHAR pcLabel[64] = "AKey20";
	memset(pcManufacturer+9, ' ', 64-strlen("Aero-info"));
	memset(pcLabel+6, ' ', 64-strlen("AKey20"));
	USBKEY_UCHAR pPin[6],pNewPin[6];
	memcpy(pPin,"1111",4);
	memcpy(pNewPin,"1234",4);

	memset(pTempKey,0x01,16);
	memset(pcIV,1,16);
	memset(pcRandom,1,16);
 	
	//rtn += USBKEY_CheckDeviceAdminPin(hDeviceHandle,(USBKEY_UCHAR_PTR)USBKEY_DEVICE_ADMIN_PIN,6,&len);

	//rtn += USBKEY_DeviceInit(hDeviceHandle, CHIP_TYPE_45, appVer, pcManufacturer, pcLabel);

	
	/*//生成随机数
 	rtn += USBKEY_GenRandom(hDeviceHandle,pcRandom,16);

	//临时对称加密
	memset(pTempKey,0x01,16);
	rtn += USBKEY_TempDataEncrypt(hDeviceHandle,pTempKey,ALG_TYPE_SCB2,USBKEY_ECB|USBKEY_NO_PADDING,NULL,pcRandom,16,out,&ulEncryptedLen);
	//外部认证
	rtn += USBKEY_ExtAuthenticate(hDeviceHandle,out,ulEncryptedLen);
	rtn += USBKEY_TempDataDecrypt(hDeviceHandle,pTempKey,ALG_TYPE_SCB2,USBKEY_ECB|USBKEY_NO_PADDING,NULL,out,16,temp,&ulEncryptedLen);
	*/

	//重置设备
 /*	rtn += USBKEY_DeviceReset(hDeviceHandle);

 	rtn = USBKEY_DeviceGetStat(hDeviceHandle, &devinfo);

	//获取设备信息
 	rtn = USBKEY_DeviceGetStat(hDeviceHandle, &devinfo);
*/	
	
	
	/*//更改外部认证密钥
	pKeyHead.ApplicationType = KEY_EXTERNAL_AUTHENTICATE_KEY;
	pKeyHead.ErrorCounter = 5;
	pKeyHead.KeyLen = 16;
	pKeyHead.ulAlg = ALG_TYPE_SCB2;
	pKeyHead.ulPrivil = USBKEY_DEVICE_PRIVILEGE;
	memset(pcRandom,2,16);
	rtn += USBKEY_WriteKey(hDeviceHandle,(USBKEY_UCHAR_PTR)&pKeyHead,pcRandom);

	//临时对称加密
	memset(pTempKey,0x02,16);
	rtn += USBKEY_TempDataEncrypt(hDeviceHandle,pTempKey,ALG_TYPE_SCB2,USBKEY_ECB|USBKEY_NO_PADDING,NULL,pcRandom,16,out,&ulEncryptedLen);
	//外部认证
	rtn += USBKEY_ExtAuthenticate(hDeviceHandle,out,ulEncryptedLen);*/
	

	//创建应用
	/*rtn += USBKEY_CheckDeviceAdminPin(hDeviceHandle,(USBKEY_UCHAR_PTR)USBKEY_DEVICE_ADMIN_PIN,16,&len);
	rtn += USBKEY_ChangeDeviceAdminPin(hDeviceHandle,(USBKEY_UCHAR_PTR)USBKEY_DEVICE_ADMIN_PIN,16,(USBKEY_UCHAR_PTR)"1111",4,&len);
	rtn += USBKEY_DeviceReset(hDeviceHandle);
	rtn += USBKEY_CheckDeviceAdminPin(hDeviceHandle,(USBKEY_UCHAR_PTR)"1111",4,&len);
	rtn += USBKEY_CreateApplication(hDeviceHandle,(USBKEY_UCHAR_PTR)APP_PKI,pPin,4,5,(USBKEY_UCHAR_PTR)"123456",6,5,USBKEY_USERTYPE_USER);*/
	//rtn += USBKEY_CheckPin(hDeviceHandle,USBKEY_USERTYPE_USER,(USBKEY_UCHAR_PTR)"123456",6,&len);
	rtn += USBKEY_DeviceReset(hDeviceHandle);
	rtn += USBKEY_CheckDeviceAdminPin(hDeviceHandle,(USBKEY_UCHAR_PTR)"1111",4,&len);
	//打开应用
  	rtn += USBKEY_OpenApplication(hDeviceHandle,(USBKEY_UCHAR_PTR)APP_PKI);
	//rtn += USBKEY_CheckPin(hDeviceHandle,USBKEY_USERTYPE_USER,(USBKEY_UCHAR_PTR)"123456",6,&len);
	//rtn += USBKEY_DeviceReset(hDeviceHandle);
	//
	////check pin
	rtn += USBKEY_CheckPin(hDeviceHandle,USBKEY_USERTYPE_ADMIN,(USBKEY_UCHAR_PTR)"1111",4,&len);

	//len = 1024;
	//pFileID[0] = (0x80ff&0xff00)>>8;
	//pFileID[1] = 0x80ff&0x00ff;
	//memset(temp,4,len);
	//rtn += USBKEY_WriteTaxInfoFile(hDeviceHandle,pFileID,0,temp,len);
	//len = 4096;
	//rtn += USBKEY_ReadTaxInfoFile(hDeviceHandle,pFileID,0,out,&len);
	//初始化PIN
	//rtn += USBKEY_InitPin(hDeviceHandle,(USBKEY_CHAR_PTR)APP_PKI,pPin,4);

	//check pin
	//rtn += USBKEY_CheckPin(hDeviceHandle,USBKEY_USERTYPE_USER,(USBKEY_UCHAR_PTR)"123456",6,&len);

	//memset(temp,0,sizeof(temp));
	//temp[1] = 0x01;
	//memset(temp+2,0xff,109);
	//memcpy(temp+111,pTest,sizeof(pTest));
	//memset(out,0,sizeof(out));
	//len = 4096;
 //	rtn += USBKEY_GenRandom(hDeviceHandle,pTempKey,24);
 //	rtn += USBKEY_GenRandom(hDeviceHandle,pcIV,16);
	//AK_AddParityCheck(pTempKey,24);

	/*rtn += USBKEY_CreateContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)CON_NAME);
	rtn += USBKEY_GenerateRSAKeyPair(hDeviceHandle,(USBKEY_UCHAR_PTR)CON_NAME,KEY_EXCHANGE,1024,USBKEY_CRYPT_USER_PROTECTED,pubKeyID,priKeyID);

	rtn += USBKEY_GenSymmetricKey(hDeviceHandle,8,ALG_TYPE_3DES_2KEY,USBKEY_CRYPT_EXPORTABLE|USBKEY_CRYPT_ONDEVICE|USBKEY_CRYPT_USER_PROTECTED,phKey);
	rtn += USBKEY_ExportSymmetricKey(hDeviceHandle,phKey,pubKeyID,USBKEY_TYPE_RSA,temp,&len);
	rtn += USBKEY_RSAPrivateKeyOperation(hDeviceHandle,priKeyID,temp,128,out,&len);
	rsa_pkcs1_No_padding(out,&len,128);
	AK_AddParityCheck(out,len);
	memcpy(pTempKey,out,len);*/

	//len = 1024;
	////rtn += USBKEY_ImportSymmetricKey(hDeviceHandle,NULL,0,USBKEY_CRYPT_EXPORTABLE|USBKEY_CRYPT_ONDEVICE,ALG_TYPE_3DES_2KEY,temp,16,phKey);
	//rtn += USBKEY_TempDataEncrypt(hDeviceHandle,pTempKey,ALG_TYPE_3DES_3KEY,USBKEY_CBC|USBKEY_NO_PADDING,pcIV,temp,128,out,&len);
	//rtn += USBKEY_TempDataDecrypt(hDeviceHandle,pTempKey,ALG_TYPE_3DES_3KEY,USBKEY_CBC|USBKEY_NO_PADDING,pcIV,out,len,temp,&ulEncryptedLen);

	//rtn += USBKEY_DeviceReset(hDeviceHandle);
	//枚举容器
 	//rtn += USBKEY_EnumContainer(hDeviceHandle,NULL,&len);
 	//rtn += USBKEY_EnumContainer(hDeviceHandle,NameList,&len);

	//rtn = USBKEY_DeleteContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)CON_NAME);
	//rtn += USBKEY_CreateContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)CON_NAME);

/*	rtn = USBKEY_IsCertificateExist(hDeviceHandle,(USBKEY_UCHAR_PTR)CON_NAME,KEY_SIGNATURE);
	len = 11;
	rtn = USBKEY_WriteCertificate(hDeviceHandle,(USBKEY_UCHAR_PTR)CON_NAME,KEY_SIGNATURE,(USBKEY_UCHAR_PTR)"testCertSig",&len);
	rtn = USBKEY_IsCertificateExist(hDeviceHandle,(USBKEY_UCHAR_PTR)CON_NAME,KEY_SIGNATURE);


	rtn += USBKEY_JudgeEmptyContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)CON_NAME);
*/

	/*len = 1024;
	rtn += USBKEY_GenerateExtRSAKeyPair(hDeviceHandle,len,(USBKEY_UCHAR_PTR)&RsaPrivate);
	//RsaPublic.bits = RsaPrivate.bits;
	//memcpy(RsaPublic.exponent,RsaPrivate.publicExponent,MAX_RSA_MODULUS_LEN_2048);
	//memcpy(RsaPublic.modulus,RsaPrivate.modulus,MAX_RSA_MODULUS_LEN_2048);
	RsaImportPrivate.bits = RsaPrivate.bits;
 	memcpy(RsaImportPrivate.prime,RsaPrivate.prime,MAX_RSA_MODULUS_LEN_1024);
	memcpy(RsaImportPrivate.primeExponent,RsaPrivate.primeExponent,MAX_RSA_MODULUS_LEN_1024);
	memcpy(RsaImportPrivate.coefficient,RsaPrivate.coefficient,MAX_RSA_PRIME_LEN_1024);

	USBKEY_UCHAR pbEncryptedData[2048];
	USBKEY_UCHAR pbRsaKeyPairs[2048];
	USBKEY_ULONG ulRsaKeyPairsLen = 0;
	USBKEY_UCHAR *pbTmp;
	ulEncryptedLen = 1024;
	pbTmp = pbRsaKeyPairs;
	pbTmp += 4; //前4个字节放SEQUENCE：30 82 LH LL

	//version: A0 01 03
	pbTmp[0] = 0xa0;
	pbTmp[1] = 0x01;
	pbTmp[2] = 0x03;
	ulRsaKeyPairsLen += 3;
	pbTmp += 3;

	//02 81 80 n，长度为128+3=131
	pbTmp[0] = 0x02;
	pbTmp[1] = 0x81;
	pbTmp[2] = 0x80;
	pbTmp += 3;
	memcpy(pbTmp, RsaPrivate.modulus, len/8);
	ulRsaKeyPairsLen += 3 + len/8;
	pbTmp += len/8;

	//e: 02 04 00 01 00 01
	pbTmp[0] = 0x02;
	pbTmp[1] = 0x04;
	pbTmp += 2;
	memcpy(pbTmp, RsaPrivate.publicExponent+len/8-4, 4);
	ulRsaKeyPairsLen += 6;
	pbTmp += 4;

	///02 81 80 d, 长度为128+3=131
	pbTmp[0] = 0x02;
	pbTmp[1] = 0x81;
	pbTmp[2] = 0x80;
	pbTmp += 3;
	memcpy(pbTmp, RsaPrivate.publicExponent, len/8);
	ulRsaKeyPairsLen += 3 + len/8;
	pbTmp += len/8;

	//02 40 p, 长度为64+2=66
	pbTmp[0] = 0x02;
	pbTmp[1] = 0x40;
	pbTmp += 2;
	memcpy(pbTmp, RsaPrivate.prime[0], len/16);
	ulRsaKeyPairsLen += 2 + len/16;
	pbTmp += len/16;

	//02 40 q, 长度为64+2=66
	pbTmp[0] = 0x02;
	pbTmp[1] = 0x40;
	pbTmp += 2;
	memcpy(pbTmp, RsaPrivate.prime[2], len/16);
	ulRsaKeyPairsLen += 2 +len/16;
	pbTmp += len/16;

	//02 40 dp, 长度为64+2=66
	pbTmp[0] = 0x02;
	pbTmp[1] = 0x40;
	pbTmp += 2;
	memcpy(pbTmp, RsaPrivate.primeExponent[0], len/16);
	ulRsaKeyPairsLen += 2 + len/16;
	pbTmp += len/16;

	//02 40 dq, 长度为64+2=66
	pbTmp[0] = 0x02;
	pbTmp[1] = 0x40;
	pbTmp += 2;
	memcpy(pbTmp, RsaPrivate.primeExponent[1], len/16);
	ulRsaKeyPairsLen += 2 + len/16;
	pbTmp += len/16;
	
	//02 40 qinv, 长度为64+2=66
	pbTmp[0] = 0x02;
	pbTmp[1] = 0x40;
	pbTmp += 2;
	memcpy(pbTmp, RsaPrivate.coefficient, len/16);
	ulRsaKeyPairsLen += 2 + len/16;
	pbTmp += len/16;
	
	//sequence = "30 82 LenH LenL "+version+n+e+d+p+q+dp+dq+qinv
	pbRsaKeyPairs[0] = 0x30;
	pbRsaKeyPairs[1] = 0x82;
	pbRsaKeyPairs[2] = (ulRsaKeyPairsLen&0xff00)>>8;
	pbRsaKeyPairs[3] = ulRsaKeyPairsLen;

	ulRsaKeyPairsLen += 4;
	rtn = USBKEY_ImportRSAKeyPair(
						hDeviceHandle,
						NULL,
						USBKEY_ECB,
						NULL,
						0,
						(USBKEY_UCHAR_PTR)CON_NAME,
						KEY_EXCHANGE,
						pbRsaKeyPairs,
						ulRsaKeyPairsLen,
						pubKeyID,
						priKeyID);
	rtn = USBKEY_ImportRSAKeyPair(
						hDeviceHandle,
						NULL,
						USBKEY_ECB,
						NULL,
						0,
						(USBKEY_UCHAR_PTR)CON_NAME,
						KEY_EXCHANGE,
						pbRsaKeyPairs,
						ulRsaKeyPairsLen,
						pubKeyID,
						priKeyID);
	rtn = USBKEY_ImportRSAKeyPair(
						hDeviceHandle,
						NULL,
						USBKEY_ECB,
						NULL,
						0,
						(USBKEY_UCHAR_PTR)CON_NAME,
						KEY_EXCHANGE,
						pbRsaKeyPairs,
						ulRsaKeyPairsLen,
						pubKeyID,
						priKeyID);

	rtn = USBKEY_ImportRSAKeyPair(
						hDeviceHandle,
						NULL,
						USBKEY_ECB,
						NULL,
						0,
						(USBKEY_UCHAR_PTR)CON_NAME,
						KEY_EXCHANGE,
						pbRsaKeyPairs,
						ulRsaKeyPairsLen,
						pubKeyID,
						priKeyID);

*/
	
	/*memset(temp,'1',sizeof(temp));
	memset(out,0,sizeof(out));
	memset(pcIV, 0x3, 16);
	ulEncryptedLen = 4096;
	rtn += USBKEY_MassiveDataEncrypt(
				hDeviceHandle,					//[in]设备句柄
				phKey,							//[in]用来加密的密钥ID
				ALG_TYPE_DES,					//[in]加密算法
				USBKEY_CBC|USBKEY_PKCS5_PADDING,	//[in]加密模式和填充模式
				pcIV,							//[in]初始化向量，如果是ecb模式，设置为NULL
				(USBKEY_UCHAR_PTR)&RsaImportPrivate,							//[in]待加密数据
				sizeof(RsaImportPrivate),							//[in]待加密数据长度
				out,							//[out]返回的加密后数据
				&ulEncryptedLen);//[in/out]in: pcEncryptedData缓冲区大小，out: 加密后数据长度或需要的pcEncryptedData缓冲区大小
*/
	//rtn += USBKEY_ImportRSAPrivateKey(hDeviceHandle,phKey,USBKEY_CBC,pcIV,8,(USBKEY_UCHAR_PTR)CON_NAME,
		//						KEY_EXCHANGE,out,ulEncryptedLen,priKeyID);

	//rtn += USBKEY_ImportRSAPrivateKey(hDeviceHandle,phKey,USBKEY_CBC,pcIV,8,(USBKEY_UCHAR_PTR)CON_NAME,
		//						KEY_EXCHANGE,out,ulEncryptedLen,priKeyID);

	//rtn += USBKEY_ImportRSAPrivateKey(hDeviceHandle,NULL,USBKEY_ECB|USBKEY_PKCS5_PADDING,NULL,0,(USBKEY_UCHAR_PTR)CON_NAME,
							//	KEY_EXCHANGE,(USBKEY_UCHAR_PTR)&RsaImportPrivate,sizeof(RsaImportPrivate),priKeyID);

	//rtn += USBKEY_ImportRSAPrivateKey(hDeviceHandle,NULL,USBKEY_ECB|USBKEY_PKCS5_PADDING,NULL,0,(USBKEY_UCHAR_PTR)CON_NAME,
							//	KEY_EXCHANGE,(USBKEY_UCHAR_PTR)&RsaImportPrivate,sizeof(RsaImportPrivate),priKeyID);

	//rtn = USBKEY_JudgeEmptyContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)CON_NAME);

	//rtn = USBKEY_DeleteContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)CON_NAME);
	//修改PIN
	//rtn += USBKEY_ChangePin(hDeviceHandle,(USBKEY_CHAR_PTR)APP_PKI,USBKEY_USERTYPE_USER,pPin,4,
	//	pNewPin,4,&len);

	//解锁PIN
	//rtn += USBKEY_UnlockPin(hDeviceHandle,(USBKEY_CHAR_PTR)APP_PKI,pPin,4,pNewPin,4,&len);

	//check pin
	//rtn += USBKEY_CheckPin(hDeviceHandle,(USBKEY_CHAR_PTR)APP_PKI,USBKEY_USERTYPE_USER,pNewPin,4,&len);

	//重置设备
 	//rtn += USBKEY_DeviceReset(hDeviceHandle);

	//获取设备信息
 	//rtn = USBKEY_DeviceGetStat(hDeviceHandle, &devinfo);

	//rtn += USBKEY_DeleteApplication(hDeviceHandle,_T("TestAPP"));
	//rtn += USBKEY_EnumApplication(hDeviceHandle,NULL,&len,&num);
	//rtn += USBKEY_EnumApplication(hDeviceHandle,NameList,&len,&num);

	//rtn += USBKEY_OpenApplication(hDeviceHandle,NameList);

	//check pin
	//rtn += USBKEY_CheckPin(hDeviceHandle,(USBKEY_CHAR_PTR)_T("TestAPP"),USBKEY_USERTYPE_USER,pPin,4,&len);

	/*文件操作*/
	//创建文件
 	/*rtn += USBKEY_CreateFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"12",10,USBKEY_USERTYPE_USER,USBKEY_USERTYPE_USER);
	rtn += USBKEY_CreateFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"13",10,USBKEY_USERTYPE_USER,USBKEY_USERTYPE_USER);
 	rtn += USBKEY_CreateFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"14",10,USBKEY_USERTYPE_USER,USBKEY_USERTYPE_USER);


	//枚举文件
 	rtn += USBKEY_EnumFile(hDeviceHandle,NULL,&len);
	rtn += USBKEY_EnumFile(hDeviceHandle,temp,&len);

	//删除文件
 	rtn = USBKEY_DeleteFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"12");

	//枚举文件
 	rtn += USBKEY_EnumFile(hDeviceHandle,NULL,&len);
	rtn += USBKEY_EnumFile(hDeviceHandle,temp,&len);


	//读写文件
 	len = 8;
 	rtn += USBKEY_WriteFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"13",0,(USBKEY_UCHAR_PTR)"testData",&len);
 	memset(temp,0,128);
 	len = 8;
 	rtn += USBKEY_ReadFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"13",0,temp,&len);

	//获取设备信息
	 memset(&fileinfo,0,sizeof(fileinfo));
	 rtn = USBKEY_GetFileAttribute(hDeviceHandle,(USBKEY_UCHAR_PTR)"13",(USBKEY_UCHAR_PTR)&fileinfo);


	//删除文件
  	rtn += USBKEY_DeleteFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"13");
  	rtn += USBKEY_DeleteFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"14");
 	rtn += USBKEY_DeleteFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"12");*/

	/*容器操作*/
	//添加容器
	/*rtn += USBKEY_CreateContainer(hDeviceHandle,APP_PKI,_T("testContainer1"));
	rtn += USBKEY_CreateContainer(hDeviceHandle,APP_PKI,_T("testContainer2"));

	//枚举容器
 	rtn += USBKEY_EnumContainer(hDeviceHandle,APP_PKI,NULL,&len);
 	rtn += USBKEY_EnumContainer(hDeviceHandle,APP_PKI,NameList,&len);

	//删除容器
 	rtn += USBKEY_DeleteContainer(hDeviceHandle,APP_PKI,_T("testContainer1"));

	//枚举容器
 	rtn += USBKEY_EnumContainer(hDeviceHandle,APP_PKI,NULL,&len);
 	rtn += USBKEY_EnumContainer(hDeviceHandle,APP_PKI,NameList,&len);

 	rtn += USBKEY_DeleteContainer(hDeviceHandle,APP_PKI,_T("testContainer2"));*/

	/*证书操作*/
	/*rtn += USBKEY_CreateContainer(hDeviceHandle,APP_PKI,CON_NAME);
	
	len = 11;
	rtn += USBKEY_WriteCertificate(hDeviceHandle,APP_PKI,CON_NAME,KEY_SIGNATURE,(USBKEY_UCHAR_PTR)"testCertSig",&len);
	rtn += USBKEY_WriteCertificate(hDeviceHandle,APP_PKI,CON_NAME,KEY_EXCHANGE,(USBKEY_UCHAR_PTR)"testCertExc",&len);

	memset(temp,0,sizeof(temp));
	rtn += USBKEY_ReadCertificate(hDeviceHandle,APP_PKI,CON_NAME,KEY_EXCHANGE,NULL,&len);
	rtn += USBKEY_ReadCertificate(hDeviceHandle,APP_PKI,CON_NAME,KEY_EXCHANGE,temp,&len);

	memset(temp,0,sizeof(temp));
	rtn += USBKEY_ReadCertificate(hDeviceHandle,APP_PKI,CON_NAME,KEY_SIGNATURE,NULL,&len);
	rtn += USBKEY_ReadCertificate(hDeviceHandle,APP_PKI,CON_NAME,KEY_SIGNATURE,temp,&len);

	rtn += USBKEY_DeleteCertificate(hDeviceHandle,APP_PKI,CON_NAME,KEY_EXCHANGE);

	rtn += USBKEY_ReadCertificate(hDeviceHandle,APP_PKI,CON_NAME,KEY_EXCHANGE,NULL,&len);

 	rtn += USBKEY_DeleteContainer(hDeviceHandle,APP_PKI,CON_NAME);*/

	/*非对称密钥操作*/
	//rtn += USBKEY_DeleteContainer(hDeviceHandle,APP_PKI,CON_NAME);
	/*rtn += USBKEY_CreateContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)CON_NAME);

	for(i = 0; i < 20; i++)
{
	rtn += USBKEY_GenerateRSAKeyPair(hDeviceHandle,(USBKEY_UCHAR_PTR)CON_NAME,KEY_EXCHANGE,2048,USBKEY_CRYPT_USER_PROTECTED,pubKeyID,priKeyID);
}
	//rtn += USBKEY_GetAsymmetricKeyID(hDeviceHandle,APP_PKI,CON_NAME,KEY_EXCHANGE,(USBKEY_UCHAR_PTR)&rsakey);
	//rtn += USBKEY_GetAsymmetricKeyID(hDeviceHandle,APP_PKI,CON_NAME,KEY_SIGNATURE,(USBKEY_UCHAR_PTR)&rsakey);
	
	//rtn += USBKEY_ExportRSAPublicKey(hDeviceHandle,APP_PKI,pubKeyID,temp,&len);

	//rtn += USBKEY_DeleteAsymmetricKey(hDeviceHandle,APP_PKI,CON_NAME,KEY_EXCHANGE,USBKEY_RSA_PUBLICKEY);
 
  	//rtn += USBKEY_ImportRSAPublicKey(hDeviceHandle,APP_PKI,CON_NAME,KEY_EXCHANGE,temp,len,pubKeyID);

	//公钥运算
	len = 256;
	memset(temp,'1',sizeof(temp));
	memset(out,0,sizeof(out));
	//rtn += USBKEY_RSAPrivateKeyOperation(hDeviceHandle,APP_PKI,priKeyID,temp,256,out,&len);
	
	memset(temp,0,sizeof(temp));
	//rtn += USBKEY_RSAPublicKeyOperation(hDeviceHandle,APP_PKI,pubKeyID,out,len,temp,&len);

 	rtn += USBKEY_DeleteContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)CON_NAME);*/

	/*临时非对称密钥操作*/
	//产生临时RSA密钥
	//rtn += USBKEY_GenerateExtRSAKeyPair(hDeviceHandle,1024,(USBKEY_UCHAR_PTR)&rsakeyPair);

	////临时RSA加密
	//memset(temp,0,sizeof(temp));
	//temp[1] = 0x01;
	//memset(temp+2,0xff,109);
	//memcpy(temp+111,pTest,sizeof(pTest));
	//memset(out,0,sizeof(out));
	//len = 4096;
	//priKeyR.bits = rsakeyPair.bits;
	//memcpy((USBKEY_UCHAR_PTR)&pubKeyR,(USBKEY_UCHAR_PTR)&rsakeyPair,sizeof(pubKeyR));
	//memcpy((USBKEY_UCHAR_PTR)&priKeyR+4,(USBKEY_UCHAR_PTR)&rsakeyPair+260,sizeof(priKeyR)-4);
	////rtn += USBKEY_TempRSAPublicKeyOperation(hDeviceHandle,(USBKEY_UCHAR_PTR)&pubKeyR,sizeof(pubKeyR),temp,128,out,&len);

	//rtn += USBKEY_TempRSAPrivateKeyOperation(hDeviceHandle,(USBKEY_UCHAR_PTR)&priKeyR,sizeof(priKeyR),temp,128,out,&len);
	//
	////临时RSA解密
	//memset(temp,0,sizeof(temp));
	//memcpy((USBKEY_UCHAR_PTR)&pubKeyR,(USBKEY_UCHAR_PTR)&rsakeyPair,sizeof(pubKeyR));
	//rtn += USBKEY_TempRSAPublicKeyOperation(hDeviceHandle,(USBKEY_UCHAR_PTR)&pubKeyR,sizeof(pubKeyR),out,len,temp,&len);
	//len = 4096;

	/*对称密钥操作*/
	//产生对称密钥
	/*rtn += USBKEY_GenSymmetricKey(hDeviceHandle,APP_PKI,8,ALG_TYPE_DES,USBKEY_CRYPT_EXPORTABLE|USBKEY_CRYPT_ONDEVICE|USBKEY_CRYPT_USER_PROTECTED,phKey);
	
	rtn += USBKEY_CreateContainer(hDeviceHandle,APP_PKI,CON_NAME);

	rtn += USBKEY_GenerateRSAKeyPair(hDeviceHandle,APP_PKI,CON_NAME,KEY_EXCHANGE,1024,USBKEY_CRYPT_USER_PROTECTED,pubKeyID,priKeyID);

	//导出密钥
	memset(temp,0,sizeof(temp));
	rtn += USBKEY_ExportSymmetricKey(hDeviceHandle,APP_PKI,phKey,pubKeyID,temp,&len);

	//导入密钥
	rtn += USBKEY_ImportSymmetricKey(hDeviceHandle,APP_PKI,priKeyID,USBKEY_CRYPT_EXPORTABLE,ALG_TYPE_DES,temp,len,phKey);


	//rtn += USBKEY_GenRandom(hDeviceHandle,temp,16);

	//rtn += USBKEY_ImportSymmetricKey(hDeviceHandle,APP_PKI,NULL,USBKEY_CRYPT_EXPORTABLE|USBKEY_CRYPT_ONDEVICE,ALG_TYPE_SCB2,temp,16,phKey);

	//批量数据加密
	memset(temp,'1',sizeof(temp));
	memset(out,0,sizeof(out));
	memset(pcIV, 0x3, 16);
	ulEncryptedLen = 4096;
	rtn += USBKEY_MassiveDataEncrypt(
				hDeviceHandle,					//[in]设备句柄
				APP_PKI,						//[in]应用名称
				phKey,							//[in]用来加密的密钥ID
				ALG_TYPE_DES,					//[in]加密算法
				USBKEY_CBC|USBKEY_NO_PADDING,	//[in]加密模式和填充模式
				pcIV,							//[in]初始化向量，如果是ecb模式，设置为NULL
				temp,							//[in]待加密数据
				1024,							//[in]待加密数据长度
				out,							//[out]返回的加密后数据
				&ulEncryptedLen);//[in/out]in: pcEncryptedData缓冲区大小，out: 加密后数据长度或需要的pcEncryptedData缓冲区大小

	//批量数据解密
	memset(temp,0,sizeof(temp));
	len = 4096;
	rtn += USBKEY_MassiveDataDecrypt(
				hDeviceHandle,						//[in]设备句柄
				APP_PKI,	//[in]应用名称
				phKey,							//[in]用来加密的密钥ID
				ALG_TYPE_DES,						//[in]加密算法
				USBKEY_CBC|USBKEY_NO_PADDING,	//[in]加密模式和填充模式
				pcIV,								//[in]初始化向量，如果是ecb模式，设置为NULL
				out,								//[in]密文数据
				ulEncryptedLen,						//[in]密文数据长度
				temp,								//[out]返回的明文
				&len);								//[in/out]in: pcData缓冲区长度 out:返回的明文长度

	rtn += USBKEY_DeleteContainer(hDeviceHandle,APP_PKI,CON_NAME);*/
	
	rtn += USBKEY_DeviceClose(hDeviceHandle);
	printf("Create app success!\n");
	Sleep(3000);
	return rtn;
}

