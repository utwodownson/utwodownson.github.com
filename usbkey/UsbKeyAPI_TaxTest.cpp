#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "../UsbKeyAPI/inc/UsbKeyAPI.h"

BOOL ReadFile(char * FileName, USBKEY_UCHAR** pValue, USBKEY_ULONG *pulLen)
{
	FILE *fp;
	USBKEY_ULONG ulSize;
	USBKEY_UCHAR* pBuf;

	if( (fp = fopen(FileName, "rb")) == NULL)
	{
		printf("cannot open the file\n");
		return 1;
	}

	fseek(fp, 0, SEEK_END);
	ulSize = ftell(fp);
	pBuf = (USBKEY_UCHAR_PTR)malloc(ulSize);
	if(!pBuf)
		return 1;
	fseek(fp, 0, SEEK_SET);

	if(fread(pBuf,1,ulSize,fp) != ulSize)
	{
		printf("read error");
		free(pBuf);
		return 1;
	}

	*pValue = pBuf;
	*pulLen = ulSize;
	return 0;
}

BOOL WriteFile(char * FileName, USBKEY_UCHAR* pValue, USBKEY_ULONG ulSize)
{
	FILE *fp;

	if( (fp = fopen(FileName, "wb+")) == NULL)
	{
		printf("cannot open the file\n");
		return 1;
	}

	if(fwrite(pValue,1,ulSize,fp) != ulSize)
	{
		printf("write error");
		return 1;
	}

	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	long rtn = 0;;

	USBKEY_HANDLE hDeviceHandle;
	USBKEY_UCHAR** ppcDeviceIdS;
	USBKEY_UCHAR pcDirName[3],pcFileName[2];
	USBKEY_DEVINFO devinfo;
	USBKEY_UCHAR temp[4096],out[4096];
	USBKEY_ULONG i;
	USBKEY_UCHAR SecKey[16];
	USBKEY_UCHAR pcIV[16];
	USBKEY_UCHAR * pcData;
	USBKEY_ULONG ulRandomLen = 16;
	USBKEY_ULONG ulDataLen,ulEncryptedLen,ulIVLen,Num;
	int pucDeviceQuantity = 0;

	pcDirName[0] = 0x3f;
	pcDirName[1] = 0x03;
	pcFileName[0] = 0x80;
	pcFileName[1] = 0x00;

	//
	
	
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

	rtn += USBKEY_DeviceEnum((USBKEY_UCHAR **)ppcDeviceIdS,(USBKEY_INT32_PTR)&pucDeviceQuantity);

	rtn += USBKEY_DeviceOpen(ppcDeviceIdS[0], 1, &hDeviceHandle);

	//check pin
	rtn = USBKEY_CheckPin(hDeviceHandle,(USBKEY_UCHAR_PTR)"AKEY PKI APP",USBKEY_USERTYPE_ADMIN,(USBKEY_UCHAR_PTR)"1111",4,&Num);

	//获取设备信息
 	rtn = USBKEY_DeviceGetStat(hDeviceHandle, &devinfo);
	//产生随机数
	memset(SecKey,0,16);
	rtn = USBKEY_GenRandom(
					hDeviceHandle,//[in]设备句柄
					SecKey,			//[out]返回的随机数
					ulRandomLen);			//[in]需要的随机数长度
	if( 0 != rtn )
	{
		printf("USBKEY_ImportTaxKey error!");
		return 1;
	}

	memset(pcIV,0,16);
	ulIVLen = 16;

	rtn = ReadFile("p11.txt", &pcData, &ulDataLen);
	if(0 != rtn)
	{
		printf("ReadFile error, %8x\n",rtn);
		return 1;
	}
	
	rtn = USBKEY_ImportTaxKey(
						hDeviceHandle,						//[in]设备句柄
						(USBKEY_UCHAR_PTR)"AKEY PKI APP",	//[in]应用名称
						NULL,								//[in]用来保护导入密钥的密钥ID
						ALG_TYPE_SCB2,						//[in]密钥算法标识
						SecKey,								//[in]导入的密钥
						ulRandomLen);						//[in]导入的密钥长度
	if( 0 != rtn )
	{
		printf("USBKEY_ImportTaxKey error!");
		return 1;
	}
	
	//复位设备
	rtn = USBKEY_DeviceReset(hDeviceHandle);
	if( 0 != rtn )
	{
		printf("USBKEY_DeviceReset error!");
		return 1;
	}
	//check pin
	rtn = USBKEY_CheckPin(hDeviceHandle,(USBKEY_UCHAR_PTR)"AKEY PKI APP",USBKEY_USERTYPE_USER,(USBKEY_UCHAR_PTR)"1111",4,&Num);
	//获取设备信息
 	rtn = USBKEY_DeviceGetStat(hDeviceHandle, &devinfo);
	//	税控数据对称加密.
	rtn = USBKEY_TaxDataEncrypt(
					hDeviceHandle,						//[in]设备句柄
					(USBKEY_UCHAR_PTR)"AKEY PKI APP",	//[in]应用名称
					ALG_TYPE_SCB2,						//[in]加密算法
					USBKEY_CBC|USBKEY_PKCS5_PADDING,	//[in]加密模式和填充模式：USBKEY_ECB|USBKEY_NO_PADDING, USBKEY_ECB|USBKEY_PKCS5_PADDING, USBKEY_CBC|USBKEY_NO_PADDING, USBKEY_CBC|USBKEY_PKCS5_PADDING
					pcIV,			//[in]初始化向量，如果是ecb模式，设置为NULL
					ulIVLen,		//[in]初始化向量长度，如果是ecb模式，设置为0
					pcData,			//[in]待加密数据
					ulDataLen,		//[in]待加密数据长度
					temp,//[out]返回的加密后数据
					&ulEncryptedLen);//[in/out]in: pcEncryptedData缓冲区大小，out: 加密后数据长度或需要的pcEncryptedData缓冲区大小
	if( 0 != rtn )
	{
		printf("USBKEY_TaxDataEncrypt error!");
		return 1;
	}

	ulDataLen = ulEncryptedLen;
	//	税控数据对称解密.
	rtn = USBKEY_TaxDataDecrypt(
					hDeviceHandle,	//[in]设备句柄
					(USBKEY_UCHAR_PTR)"AKEY PKI APP",		//[in]应用名称
					ALG_TYPE_SCB2,			//[in]解密算法
					USBKEY_CBC|USBKEY_PKCS5_PADDING,			//[in]解密模式和填充模式：USBKEY_ECB|USBKEY_NO_PADDING, USBKEY_ECB|USBKEY_PKCS5_PADDING, USBKEY_CBC|USBKEY_NO_PADDING, USBKEY_CBC|USBKEY_PKCS5_PADDING
					pcIV,			//[in]初始化向量，如果是ecb模式，设置为NULL
					ulIVLen,		//[in]初始化向量长度，如果是ecb模式，设置为0
					temp,//[in]密文数据
					ulEncryptedLen,	//[in]密文数据长度
					out,			//[out]返回的明文
					&ulDataLen);		//[in/out]in: pcData缓冲区长度 out:返回的明文长度
	if( 0 != rtn )
	{
		printf("USBKEY_TaxDataDecrypt error!");
		return 1;
	}

	rtn += USBKEY_DeviceClose(hDeviceHandle);

	free(ppcDeviceIdS);
	free(pcData);
	printf("return val : %d\r\n",rtn);

	scanf("");

	return 0;
}
