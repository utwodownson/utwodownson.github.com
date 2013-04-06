#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include "./UtilTest.h"
#include "./pkcs1_padding.h"


static unsigned char Test_rsa_n[128] =
{	//	PubId=0xaa845249
	0xe5, 0xf3, 0xb6, 0xb6, 0xc0, 0x52, 0x51, 0x36, 0xd7, 0xdc, 0x50, 0xac, 0x4c, 0xbe, 0xc2, 0xc0,
	0xbd, 0x6f, 0x85, 0x5c, 0xad, 0xe5, 0x97, 0x03, 0x58, 0x1e, 0xfc, 0xe7, 0x0b, 0x01, 0xec, 0xd4,
	0x05, 0xab, 0x60, 0x76, 0x2e, 0x05, 0x9e, 0x88, 0x58, 0x84, 0x1e, 0x3c, 0x3f, 0x2a, 0xe3, 0x7b,
	0xb0, 0x21, 0x93, 0x56, 0xff, 0xf9, 0x17, 0xdf, 0x26, 0xd6, 0x49, 0x94, 0xed, 0x76, 0x47, 0xc1,
	0x12, 0x74, 0x62, 0xd7, 0x71, 0xc2, 0x06, 0x36, 0x76, 0xee, 0xa2, 0x8f, 0x82, 0x6d, 0x37, 0x6c,
	0xd7, 0x69, 0x48, 0x9f, 0x0c, 0xb7, 0x9d, 0x67, 0x34, 0x50, 0xe9, 0x57, 0x70, 0x89, 0xda, 0xb7,
	0xab, 0x82, 0x8a, 0x4f, 0xe8, 0x3e, 0xc1, 0x4c, 0xc0, 0xc0, 0x47, 0xf9, 0xaa, 0xd0, 0x5b, 0xed,
	0x59, 0x14, 0x42, 0xbc, 0x11, 0xf2, 0x36, 0x93, 0x4e, 0x31, 0x03, 0xbc, 0xd4, 0x68, 0x00, 0xfd
};
static unsigned char Test_rsa_p[64] =
{	//	PubId=0xaa845249
	0xfe, 0x34, 0xec, 0x62, 0x6b, 0xac, 0xbd, 0x8b, 0x73, 0x60, 0xb9, 0x66, 0xcb, 0xf6, 0xcc, 0x44,
	0xa1, 0xf0, 0x77, 0x55, 0x11, 0x29, 0x75, 0x8f, 0x92, 0x8a, 0x46, 0xc1, 0x74, 0x51, 0xc2, 0xc4,
	0xf9, 0xed, 0xf3, 0x7c, 0xad, 0xf4, 0x25, 0x6e, 0x3b, 0xfb, 0x2a, 0x58, 0x8d, 0x61, 0x00, 0xb4,
	0x2a, 0xfe, 0x3f, 0xa7, 0x70, 0x10, 0x57, 0x30, 0xe5, 0xe9, 0x7b, 0x03, 0x90, 0xb0, 0x14, 0xf7
};
static unsigned char Test_rsa_q[64] =
{	//	PubId=0xaa845249
	0xe7, 0x92, 0xfd, 0x00, 0xd4, 0xe8, 0x92, 0x8f, 0x49, 0xac, 0xbb, 0xa3, 0xe5, 0xbd, 0x0e, 0x5c,
	0xfd, 0x45, 0x1f, 0x01, 0x60, 0x5a, 0x63, 0xd7, 0xd3, 0xa8, 0x8c, 0x06, 0x93, 0xcf, 0x1f, 0x15,
	0xd8, 0x7c, 0xf6, 0x99, 0x0d, 0xd3, 0xc3, 0x52, 0x4d, 0xff, 0xca, 0xa2, 0x5d, 0xca, 0x2f, 0x25,
	0x4d, 0xbd, 0x30, 0xff, 0x4b, 0xef, 0x71, 0xce, 0x39, 0x5b, 0xc8, 0xcb, 0x97, 0x06, 0x00, 0xab
};
static unsigned char Test_rsa_d[128] =
{	//	PubId=0xaa845249
	0xde, 0xe5, 0x5c, 0x48, 0xcd, 0x27, 0x7d, 0xbd, 0xb1, 0x83, 0xc7, 0x57, 0x1c, 0x95, 0x14, 0x66,
	0x8b, 0xb9, 0x9c, 0x05, 0xf8, 0x32, 0x42, 0x2d, 0x65, 0xaf, 0xb3, 0xe9, 0xa9, 0x0b, 0xd6, 0xaa,
	0xb6, 0x4c, 0x26, 0x96, 0xa7, 0x89, 0x9e, 0xdf, 0xff, 0x1d, 0x21, 0xc9, 0xb8, 0x59, 0x34, 0x64,
	0x62, 0x98, 0x49, 0x1f, 0xdd, 0xc3, 0x70, 0x56, 0x48, 0x69, 0x12, 0xae, 0x1f, 0x77, 0xb5, 0x98,
	0x24, 0x0f, 0x93, 0x33, 0x03, 0xaf, 0x99, 0x5e, 0x4a, 0xd1, 0x70, 0xb3, 0xfb, 0x36, 0x92, 0x32,
	0x12, 0x6a, 0x59, 0xf8, 0x73, 0x28, 0xbc, 0x66, 0x2b, 0x3f, 0x93, 0x12, 0xb3, 0x5c, 0x7d, 0x4e,
	0x65, 0xc8, 0xb6, 0xf1, 0x6c, 0xc4, 0x4e, 0xa7, 0xab, 0x37, 0x9e, 0x79, 0x7c, 0xfe, 0xf8, 0x38,
	0xd4, 0x10, 0x84, 0xa3, 0x55, 0xca, 0x46, 0x59, 0x8b, 0x19, 0xcd, 0xd2, 0xee, 0xc1, 0x0f, 0xa9
};
static unsigned char Test_rsa_dmp1[64] =
{	//	PubId=0xaa845249
	0x6d, 0xc5, 0x54, 0xbe, 0x3b, 0x58, 0xe0, 0x90, 0xb3, 0x45, 0xf5, 0x49, 0x9a, 0x91, 0xfc, 0x2b,
	0x36, 0xf2, 0xab, 0xd5, 0x24, 0x10, 0x0f, 0x1f, 0xec, 0x5b, 0xa1, 0xaa, 0x30, 0x1c, 0x8e, 0x4a,
	0x98, 0xd2, 0x53, 0xee, 0x66, 0xff, 0xa2, 0x9a, 0x33, 0x74, 0x86, 0x18, 0xaf, 0x14, 0x50, 0x45,
	0x7c, 0xbb, 0x45, 0xa9, 0x7c, 0x0e, 0xd2, 0x57, 0xdf, 0x30, 0x2f, 0x5a, 0x9e, 0xc4, 0x2a, 0x89
};
static unsigned char Test_rsa_dmq1[64] =
{	//	PubId=0xaa845249
	0xcc, 0x8b, 0xe9, 0x2e, 0x72, 0xe1, 0x7b, 0x93, 0x55, 0x60, 0x5d, 0xdd, 0xc7, 0x6f, 0xd6, 0xe0,
	0x0c, 0x56, 0xa1, 0xac, 0x75, 0x8f, 0x61, 0xe1, 0xa1, 0xf3, 0x9e, 0xbb, 0xb0, 0x93, 0xa2, 0xc1,
	0x89, 0x17, 0xd8, 0x3a, 0x19, 0x1c, 0x93, 0xca, 0x7f, 0x5b, 0x11, 0x81, 0x99, 0x96, 0x9d, 0x4e,
	0x35, 0xfc, 0x86, 0xea, 0xfa, 0x07, 0xe6, 0x5e, 0x62, 0x0b, 0xd0, 0xb1, 0x04, 0x15, 0x29, 0x41
};
static unsigned char Test_rsa_iqmp[64] =
{	//	PubId=0xaa845249
	0xb6, 0x86, 0xb7, 0x47, 0x38, 0x36, 0x91, 0x7b, 0x5b, 0xe4, 0x55, 0x9f, 0xa9, 0xaf, 0x25, 0x85,
	0x40, 0xc6, 0x67, 0x95, 0xc8, 0x0e, 0x0b, 0xc4, 0xe4, 0xaf, 0xb8, 0xf1, 0x08, 0x39, 0x3a, 0xa8,
	0xa1, 0x84, 0x15, 0x61, 0x5c, 0x5b, 0x02, 0xa6, 0x42, 0x5e, 0x47, 0xa5, 0x56, 0xb9, 0xcb, 0xc6,
	0x52, 0x8b, 0x53, 0x9b, 0xb5, 0x95, 0xa6, 0x19, 0x19, 0x67, 0x97, 0xc8, 0xc2, 0x3d, 0x0c, 0x77
};



//==============================================================
//函数名称:		AK_Reverser
//函数参数:		buf[in/out],in：待转换数据，out：转换结果
//				len[in], 要转换的数据长度
//返回值:	    Cryptoki定义的错误码
//				
//函数功能:		把buf中的数据倒转 
//==============================================================
void AK_Reverser(USBKEY_VOID_PTR buf, USBKEY_ULONG len)
{
	USBKEY_UCHAR t; 
	USBKEY_ULONG i;
	USBKEY_UCHAR_PTR pBuf;
	
	pBuf = (USBKEY_UCHAR_PTR)buf;
	for(i = 0; i<len/2; i++)
	{
		t = pBuf[i];
		pBuf[i] = pBuf[len-1-i];
		pBuf[len-1-i] = t;
	}

	return;
}

//==============================================================
//函数名称:		AK_Hexdump
//函数参数:		pBuf[in]: 需打印内存
//				ulLen[in]: 打印长度
//				
//返回值:		无返回值
//函数功能:		打印内存字符十六进制及ASKII表示。不可打印字符用“.”表示
//==============================================================
void AK_Hexdump(USBKEY_VOID_PTR pBuf, USBKEY_ULONG ulLen)
{
	USBKEY_UCHAR_PTR line = (USBKEY_UCHAR_PTR)pBuf;
	USBKEY_ULONG thisline, offset = 0;
	USBKEY_ULONG i;

	while (offset < ulLen)
	{
		printf("%04x ", offset);
		thisline = ulLen - offset;
		if (thisline > 16)
			thisline = 16;

		for (i = 0; i < thisline; i++)
			printf("%02x ", line[i]);

		for (; i < 16; i++)
				printf("   ");

		for (i = 0; i < thisline; i++)
			printf("%c",(line[i] >= 0x20 && line[i] < 0x7f) ? line[i] : '.');

		printf("\n");
		offset += thisline;
		line += thisline;
	}
	return;
}

/**
 *	@brief
 *		初始化key测试，初始化key并创建应用
 *		USBKEY_DeviceInit
 *		USBKEY_CheckDeviceAdminPin
 *		USBKEY_ChangeDeviceAdminPin
 *		USBKEY_CreateApplication
 *	@param
 *		hDeviceHandle [in]the handle of device
 */
USBKEY_ULONG InitUKey_Test(USBKEY_HANDLE hDeviceHandle)
{
	unsigned long rtn = 0;
	
	USBKEY_VERSION appVer;	// cos version
	USBKEY_UCHAR pcManufacturer[64] = "Aero-info";
	USBKEY_UCHAR pcLabel[64] = "Aisino USBKey";
	USBKEY_UCHAR pcApplicationNameList[32];
	USBKEY_ULONG ulAppCount = 0;
	USBKEY_ULONG ulSize = 0;
	USBKEY_ULONG ulen = 0;

	appVer.major = 1;
	appVer.minor = 0;
	memset(pcManufacturer + 9, ' ', 64 - strlen("Aero-info"));
	memset(pcLabel + 13, ' ', 64 - 13);
	
	rtn = USBKEY_DeviceReset(hDeviceHandle);
	if (rtn !=0)
	{
		printf("USBKEY_DeviceReset error! rtn = %#08x\n", rtn);
		return rtn;
	}
	printf("Change Device AdminPin success!\n");

	//枚举应用，获得应用名称
	memset(pcApplicationNameList, 0, sizeof(pcApplicationNameList));
	//rtn = USBKEY_EnumApplication(hDeviceHandle, NULL, &ulSize, &ulAppCount);
	rtn = USBKEY_FILE_NOT_FOUND; 
	if( 0 == rtn && 0 != ulAppCount )
	{
		rtn = USBKEY_EnumApplication(hDeviceHandle, pcApplicationNameList, &ulSize, &ulAppCount);
		if( 0 != rtn )
		{
			printf("USBKEY_EnumApplication get value error! rtn = %#08x, ulAppCount = %d\n",rtn,ulAppCount);
			return rtn;
		}

		rtn = USBKEY_CheckDeviceAdminPin(hDeviceHandle,(USBKEY_UCHAR_PTR)"1111",4,&ulen);
		if (rtn !=0)
		{
			printf("USBKEY_CheckDeviceAdminPin error! rtn = %#08x, ulen = %d\n",rtn,ulen);
			return rtn;
		}

		//删除应用，即删除所有存放在KEY中的对象
		rtn = USBKEY_DeleteApplication(hDeviceHandle, pcApplicationNameList);
		if( 0 != rtn )
		{
			printf("USBKEY_DeleteApplication error! rtn = %#08x\n",rtn);
			return rtn;
		}
	}
	else if( USBKEY_FILE_NOT_FOUND == rtn )	//没有建立MF
	{
		//初始化设备，建立MF,设置label label为"Aisino USBKey"。
		rtn = USBKEY_DeviceInit(hDeviceHandle, CHIP_TYPE_AC3, appVer, pcManufacturer, pcLabel);
		if (rtn !=0)
		{
			printf("USBKEY_DeviceInit error! rtn = %#08x\n",rtn);
			return rtn;
		}
		printf("Device Init success!\n");

		//创建应用
		rtn = USBKEY_CheckDeviceAdminPin(hDeviceHandle,(USBKEY_UCHAR_PTR)USBKEY_DEVICE_ADMIN_PIN,6,&ulen);
		if (rtn !=0)
		{
			printf("USBKEY_CheckDeviceAdminPin error! rtn = %#08x, ulen = %d\n",rtn,ulen);
			return rtn;
		}

		rtn = USBKEY_ChangeDeviceAdminPin(hDeviceHandle,
				(USBKEY_UCHAR_PTR)USBKEY_DEVICE_ADMIN_PIN,6,(USBKEY_UCHAR_PTR)"1111",4,&ulen);
		if (rtn !=0)
		{
			printf("USBKEY_ChangeDeviceAdminPin error! rtn = %#08x, ulen = %d\n",rtn,ulen);
			return rtn;
		}
	}
	else if(0 != rtn || 0 != ulAppCount)	/*包括rvCKR_OK == rv 和ulAppCount为0的情况，此情况应跳到应用创建部分*/
	{
		printf("USBKEY_EnumApplication error! rtn = %#08x, ulAppCount = %d\n",rtn,ulAppCount);
		return rtn;
	}
	

	rtn = USBKEY_CreateApplication(hDeviceHandle,
			(USBKEY_UCHAR_PTR)APP_PKI,
			(USBKEY_UCHAR_PTR)"1111",4,5,
			(USBKEY_UCHAR_PTR)"123456",6,5,
			USBKEY_USERTYPE_USER);
	if (rtn !=0)
	{
		printf("USBKEY_CreateApplication error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Create Application success!\n");

	rtn = USBKEY_CheckPin(hDeviceHandle,USBKEY_USERTYPE_USER,(USBKEY_UCHAR_PTR)"123456",6,&ulen);
	if (rtn !=0)
	{
		printf("USBKEY_CheckUserPin error! not in the dir. rtn = %#08x\n",rtn);
		return rtn;
	}

	rtn = USBKEY_DeviceReset(hDeviceHandle);
	if (rtn !=0)
	{
		printf("USBKEY_CreateApplication error! rtn = %#08x\n",rtn);
		return rtn;
	}

	return 0;
}



//认证key测试，包括外部认证与内部认证(USBKEY_ExtAuthenticate/USBKEY_WriteKey)
USBKEY_ULONG AuthenticateUKey_Test(USBKEY_HANDLE hDeviceHandle)
{
	unsigned long rtn = 0;

	USBKEY_UCHAR pcRandom[16],pTempKey[16];
	USBKEY_UCHAR pcIV[16];
	USBKEY_UCHAR temp[4096],out[4096];
	USBKEY_KEYHEAD pKeyHead;

	USBKEY_ULONG ulEncryptedLen;

	memset(pTempKey,0x01,16);
	memset(pcIV,1,16);
	memset(pcRandom,1,16);


	//临时对称加密
	memset(pTempKey,0x01,16);
	rtn = USBKEY_TempDataEncrypt(hDeviceHandle,pTempKey,ALG_TYPE_SCB2,USBKEY_ECB|USBKEY_NO_PADDING,NULL,pcRandom,16,out,&ulEncryptedLen);
	if (rtn !=0)
	{
		printf("USBKEY_TempDataEncrypt error! rtn = %#08x\n",rtn);
		return rtn;
	}

	//生成随机数
 	rtn = USBKEY_GenRandom(hDeviceHandle,pcRandom,16);
	if (rtn !=0)
	{
		printf("USBKEY_GenRandom error! rtn = %#08x\n",rtn);
		return rtn;
	}

	//外部认证
	rtn = USBKEY_ExtAuthenticate(hDeviceHandle,out,ulEncryptedLen);
	if (rtn !=0)
	{
		printf("USBKEY_ExtAuthenticate error! rtn = %#08x\n",rtn);
		return rtn;
	}

	rtn = USBKEY_TempDataDecrypt(hDeviceHandle,pTempKey,ALG_TYPE_SCB2,USBKEY_ECB|USBKEY_NO_PADDING,NULL,out,16,temp,&ulEncryptedLen);
	if (rtn !=0)
	{
		printf("USBKEY_TempDataDecrypt error! rtn = %#08x\n",rtn);
		return rtn;
	}


	//更改外部认证密钥
	pKeyHead.ApplicationType = KEY_EXTERNAL_AUTHENTICATE_KEY;
	pKeyHead.ErrorCounter = 5;
	pKeyHead.KeyLen = 16;
	pKeyHead.ulAlg = ALG_TYPE_SCB2;
	pKeyHead.ulPrivil = USBKEY_DEVICE_PRIVILEGE;
	memset(pcRandom,2,16);
	rtn = USBKEY_WriteKey(hDeviceHandle,(USBKEY_UCHAR_PTR)&pKeyHead,pcRandom);
	if (rtn !=0)
	{
		printf("USBKEY_WriteKey error! rtn = %#08x\n",rtn);
		return rtn;
	}

	//临时对称加密
	memset(pTempKey,0x02,16);
	rtn = USBKEY_TempDataEncrypt(hDeviceHandle,pTempKey,ALG_TYPE_SCB2,USBKEY_ECB|USBKEY_NO_PADDING,NULL,pcRandom,16,out,&ulEncryptedLen);
	if (rtn !=0)
	{
		printf("USBKEY_TempDataEncrypt error! rtn = %#08x\n",rtn);
		return rtn;
	}

	//外部认证
	rtn = USBKEY_ExtAuthenticate(hDeviceHandle,out,ulEncryptedLen);
	if (rtn !=0)
	{
		printf("USBKEY_ExtAuthenticate error! rtn = %#08x\n",rtn);
		return rtn;
	}

	return 0;

}

//获得key信息(USBKEY_DeviceGetStat)
USBKEY_ULONG GetUKeyInfo_Test(USBKEY_HANDLE hDeviceHandle)
{
	unsigned long rtn = 0;
	USBKEY_DEVINFO devinfo;

	//重置设备
 	rtn = USBKEY_DeviceReset(hDeviceHandle);
	if (rtn !=0)
	{
		printf("USBKEY_DeviceReset error! rtn = %#08x\n",rtn);
		return rtn;
	}

	//获取设备信息
 	rtn = USBKEY_DeviceGetStat(hDeviceHandle, &devinfo);
	if (rtn !=0)
	{
		printf("USBKEY_DeviceGetStat error! rtn = %#08x\n",rtn);
		return rtn;
	}

	printf("Device GetStat success!\n");
	//printf("the Device Info is:\n");
	//AK_Hexdump(&devinfo,sizeof(devinfo));
	
	return 0;
}


//Pin码操作(USBKEY_InitPin/USBKEY_CheckPin/USBKEY_ChangePin/USBKEY_UnlockPin/USBKEY_GetPinInfo)
USBKEY_ULONG OptUKeyPIN_Test(USBKEY_HANDLE hDeviceHandle)
{
	unsigned long rtn = 0;
	USBKEY_ULONG ulReNum = 0;
	USBKEY_ULONG ulMaxNum;

	rtn = USBKEY_CheckPin(hDeviceHandle,USBKEY_USERTYPE_USER,(USBKEY_UCHAR_PTR)"123456",6,&ulReNum);
	if (rtn !=0)
	{
		printf("USBKEY_CheckPin error! rtn = %#08x\n",rtn);
		return rtn;
	}

	//check pin
	rtn = USBKEY_CheckPin(hDeviceHandle,USBKEY_USERTYPE_ADMIN,(USBKEY_UCHAR_PTR)"1111",4,&ulReNum);
	if (rtn !=0)
	{
		printf("USBKEY_CheckPin error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Check admin Pin success!\n");


	//初始化PIN
	rtn = USBKEY_InitPin(hDeviceHandle,(USBKEY_UCHAR_PTR)"1234",4);
	if (rtn !=0)
	{
		printf("USBKEY_InitPin error! rtn = %#08x\n",rtn);
		return rtn;
	}

	//check pin
	rtn = USBKEY_CheckPin(hDeviceHandle,USBKEY_USERTYPE_USER,(USBKEY_UCHAR_PTR)"1234",4,&ulReNum);
	if (rtn !=0)
	{
		printf("USBKEY_CheckPin error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Init Pin and check success!\n");

	//修改PIN
	rtn = USBKEY_ChangePin(hDeviceHandle,USBKEY_USERTYPE_USER,(USBKEY_UCHAR_PTR)"1234",4,(USBKEY_UCHAR_PTR)"1111",4,&ulReNum);
	if (rtn !=0)
	{
		printf("USBKEY_ChangePin error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Change Pin success!\n");

	//解锁PIN
	rtn = USBKEY_UnlockPin(hDeviceHandle,(USBKEY_UCHAR_PTR)"1111",4,(USBKEY_UCHAR_PTR)"123456",6,&ulReNum);
	if (rtn !=0)
	{
		printf("USBKEY_UnlockPin error! rtn = %#08x\n",rtn);
		return rtn;
	}

	//check pin
	rtn = USBKEY_CheckPin(hDeviceHandle,USBKEY_USERTYPE_USER,(USBKEY_UCHAR_PTR)"123456",6,&ulReNum);
	if (rtn !=0)
	{
		printf("USBKEY_CheckPin error! rtn = %#08x\n",rtn);
		return rtn;
	}

	printf("Unlock Pin and check success!\n");

	rtn = USBKEY_GetPinInfo(hDeviceHandle,USBKEY_USERTYPE_USER,&ulMaxNum,&ulReNum);
	if (rtn !=0)
	{
		printf("USBKEY_GetPinInfo error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("USBKEY_GetPinInfo success! the ulMaxNum is %d,ulReNum is %d\n",ulMaxNum,ulReNum);

	return 0;
}


// 文件操作
// USBKEY_CreateFile
// USBKEY_EnumFile
// USBKEY_DeleteFile
// USBKEY_WriteFile
// USBKEY_ReadFile
// USBKEY_GetFileAttribute
USBKEY_ULONG FileOpt_Test(USBKEY_HANDLE hDeviceHandle)
{
	unsigned long rtn = 0;
	USBKEY_UCHAR temp[256],out[256];
	USBKEY_ULONG ulLen;
	USBKEY_FILEATTRIBUTE fileinfo;

	/*文件操作*/
	//创建文件
 	rtn = USBKEY_CreateFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"testfile1",100,USBKEY_USERTYPE_USER,USBKEY_USERTYPE_USER);
	if (rtn !=0)
	{
		printf("USBKEY_CreateFile error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Create File testfile1 success!\n");

	rtn = USBKEY_CreateFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"testfile2",128,USBKEY_USERTYPE_USER,USBKEY_USERTYPE_USER);
 	if (rtn !=0)
	{
		printf("USBKEY_CreateFile error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Create File testfile2 success!\n");

	rtn = USBKEY_CreateFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"testfile3",256,USBKEY_USERTYPE_USER,USBKEY_USERTYPE_USER);
	if (rtn !=0)
	{
		printf("USBKEY_CreateFile error! rtn = %#08x\n",rtn);
		//return rtn;
	}
	printf("Create File testfile3 success!\n");


	//枚举文件
 	rtn = USBKEY_EnumFile(hDeviceHandle,NULL,&ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_EnumFile error! rtn = %#08x\n",rtn);
		return rtn;
	}

	rtn = USBKEY_EnumFile(hDeviceHandle,temp,&ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_EnumFile error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Enum File success!\n");
	AK_Hexdump(temp,ulLen);

	//删除文件
 	rtn = USBKEY_DeleteFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"testfile2");
	if (rtn !=0)
	{
		printf("USBKEY_DeleteFile testfile2 error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Delete File testfile2 success!\n");

	//枚举文件
	memset(temp,0,sizeof(temp));
 	rtn = USBKEY_EnumFile(hDeviceHandle,temp,&ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_EnumFile error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Enum File success!\n");
	AK_Hexdump(temp,ulLen);

	//读写文件
 	ulLen = 130;
	memset(temp,0,sizeof(temp));
	rtn = USBKEY_GenRandom(hDeviceHandle,temp,186);
	if (rtn !=0)
	{
		printf("USBKEY_GenRandom error! rtn = %#08x\n",rtn);
		return rtn;
	}
 	rtn = USBKEY_WriteFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"testfile3",0,temp,&ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_WriteFile error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Write File testfile3 success!\n");

 	memset(out,0,sizeof(out));
 	ulLen = 62;
 	rtn = USBKEY_ReadFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"testfile3",0,out,&ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_ReadFile error! rtn = %#08x\n",rtn);
		return rtn;
	}
	if(62 == ulLen && 0 == memcmp(temp,out,ulLen))
		printf("Read File testfile3 success,the write data and the read data is same\n");
	else
	{
		printf("Read File testfile3 error! the write data and the read data is different\n");
		Sleep(3000);
		return -1;
	}

	memset(out,0,128);
 	ulLen = 62;
 	rtn = USBKEY_ReadFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"testfile3",62,out,&ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_ReadFile error! rtn = %#08x\n",rtn);
		return rtn;
	}
	if(62 == ulLen && 0 == memcmp(temp+62,out,ulLen))
		printf("Read File testfile3 offset 62 success,the write data and the read data is same\n");
	else
	{
		printf("Read File testfile3 offset 62 error! the write data and the read data is different\n");
		Sleep(3000);
		return -1;
	}

	memset(out,0,128);
 	ulLen = 62;
 	rtn = USBKEY_ReadFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"testfile3",124,out,&ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_ReadFile error! rtn = %#08x\n",rtn);
		return rtn;
	}
	if(62 == ulLen && 0 == memcmp(temp+124,out,130-124))
		printf("Read File testfile3 offset 124 success,the write data and the read data is same\n");
	else
	{
		printf("Read File testfile3 offset 124 error! the write data and the read data is different\n");
		Sleep(3000);
		return -1;
	}

	//获取设备信息
	memset(&fileinfo,0,sizeof(fileinfo));
	rtn = USBKEY_GetFileAttribute(hDeviceHandle,(USBKEY_UCHAR_PTR)"testfile3",(USBKEY_UCHAR_PTR)&fileinfo);
	if (rtn !=0)
	{
		printf("USBKEY_WriteFile error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Get File Attribute success!\n");
	AK_Hexdump(&fileinfo,sizeof(fileinfo));


	//删除文件
  	rtn = USBKEY_DeleteFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"testfile3");
	if (rtn !=0)
	{
		printf("USBKEY_DeleteFile testfile3 error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Delete File testfile3 success!\n");

  	rtn = USBKEY_DeleteFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"testfile2");
	if (rtn !=0)
	{
		printf("USBKEY_DeleteFile testfile2 error! rtn = %#08x\n",rtn);
		//return rtn;
	}
	printf("Delete File testfile2,this file has deleted!\n");

 	rtn = USBKEY_DeleteFile(hDeviceHandle,(USBKEY_UCHAR_PTR)"testfile1");
	if (rtn !=0)
	{
		printf("USBKEY_DeleteFile testfile1 error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Delete File testfile1 success!\n");

	//枚举文件
 	rtn = USBKEY_EnumFile(hDeviceHandle,NULL,&ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_EnumFile error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Enum File success!,ulLen is %d\n",ulLen);

	return 0;
}


// RSA公私钥对生成及容器操作
// USBKEY_CreateContainer
// USBKEY_EnumContainer 
// USBKEY_DeleteContainer
// USBKEY_JudgeEmptyContainer
// USBKEY_GenerateRSAKeyPair
// USBKEY_GetAsymmetricKeyID
// USBKEY_ExportRSAPublicKey 
// USBKEY_DeleteAsymmetricKey
// USBKEY_RSAPrivateKeyOperation
// USBKEY_RSAPublicKeyOperation
// USBKEY_GetPriKeyBitLen

USBKEY_ULONG GenUKeyRSAPair_Test(USBKEY_HANDLE hDeviceHandle)
{
	unsigned long rtn = 0;
	USBKEY_UCHAR NameList[256];
	USBKEY_ULONG ulLen;

	USBKEY_RSA_KEY rsakey;
	USBKEY_UCHAR pubKeyID[2],priKeyID[2];
	USBKEY_UCHAR data[1024],temp[1024],out[1024];

	rtn = USBKEY_CreateContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1");
	if (rtn !=0)
	{
		printf("USBKEY_CreateContainer testContainer1 error! rtn = %#08x\n",rtn);
		//return rtn;
	}

	rtn = USBKEY_CreateContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer2");
	if (rtn !=0)
	{
		printf("USBKEY_CreateContainer testContainer2 error! rtn = %#08x\n",rtn);
		//return rtn;
	}
	printf("Create Containers success!\n");

	//枚举容器
 	rtn = USBKEY_EnumContainer(hDeviceHandle,NULL,&ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_EnumContainer error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Enum Container success!,ulLen is %d\n",ulLen);

 	rtn = USBKEY_EnumContainer(hDeviceHandle,NameList,&ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_EnumContainer error! rtn = %#08x\n",rtn);
		return rtn;
	}
	AK_Hexdump(NameList,ulLen);

	//删除容器
 	rtn = USBKEY_DeleteContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1");
	if (rtn !=0)
	{
		printf("USBKEY_DeleteContainer testContainer1 error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Delete Containers testContainer1 success!\n");

 	//枚举容器
 	rtn = USBKEY_EnumContainer(hDeviceHandle,NULL,&ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_EnumContainer 2 error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Enum Container 2 success!,ulLen is %d\n",ulLen);

 	rtn = USBKEY_EnumContainer(hDeviceHandle,NameList,&ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_EnumContainer 2 error! rtn = %#08x\n",rtn);
		return rtn;
	}
	AK_Hexdump(NameList,ulLen);

 	rtn = USBKEY_DeleteContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer2");
	if (rtn !=0)
	{
		printf("USBKEY_DeleteContainer testContainer2 error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Delete Container testContainer2 success!\n");

	rtn = USBKEY_CreateContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1");
	if (rtn !=0)
	{
		printf("USBKEY_CreateContainer testContainer1 error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Create Container testContainer1 success!\n");

	rtn = USBKEY_GenerateRSAKeyPair(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1",KEY_EXCHANGE,2048,USBKEY_CRYPT_USER_PROTECTED,pubKeyID,priKeyID);
	if (rtn !=0)
	{
		printf("USBKEY_GenerateRSAKeyPair error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Generate RSA KeyPair KEY_EXCHANGE success!\n");

	rtn = USBKEY_GetAsymmetricKeyID(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1",KEY_EXCHANGE,(USBKEY_UCHAR_PTR)&rsakey);
	if (rtn !=0)
	{
		printf("USBKEY_GetAsymmetricKeyID KEY_EXCHANGE error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Get KEY_EXCHANGE AsymmetricKey ID success!\n");
	AK_Hexdump(&rsakey,sizeof(rsakey));

	rtn = USBKEY_GetAsymmetricKeyID(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1",KEY_SIGNATURE,(USBKEY_UCHAR_PTR)&rsakey);
	if (rtn !=0)
	{
		printf("USBKEY_GetAsymmetricKeyID KEY_SIGNATURE error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Get KEY_SIGNATURE AsymmetricKey ID success!\n");
	AK_Hexdump(&rsakey,sizeof(rsakey));
	
	memset(temp,0,sizeof(temp));
	rtn = USBKEY_ExportRSAPublicKey(hDeviceHandle,pubKeyID,temp,&ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_ExportRSAPublicKey error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Export RSA PublicKey success!\n");
	//AK_Hexdump(temp,ulLen);
 
	//公私钥运算
	ulLen = 256;
	memset(data,0,sizeof(data));
	memset(out,0,sizeof(out));
	ulLen = sizeof(out);
	rtn = USBKEY_GenRandom(hDeviceHandle,data+2,254);
	if (rtn !=0)
	{
		printf("USBKEY_GenRandom error! rtn = %#08x\n",rtn);
		return rtn;
	}
	data[1]= 0x02;
	
	rtn = USBKEY_RSAPrivateKeyOperation(hDeviceHandle,priKeyID,data,256,out,&ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_RSAPrivateKeyOperation error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("RSA PrivateKey Operation success!\n");
		
	memset(temp,0,sizeof(temp));
	rtn = USBKEY_RSAPublicKeyOperation(hDeviceHandle,pubKeyID,out,ulLen,temp,&ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_RSAPublicKeyOperation error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("RSA PublicKey Operation success!\n");

	if(256 == ulLen && 0 == memcmp(temp,data,ulLen))
		printf("RSA Private & Public Operation success,the data and the decrpyt data same\n");
	else
	{
		printf("RSA Private & Public Operation error! the data and the decrpyt data is different\n");
		Sleep(3000);
		return -1;
	}

	rtn = USBKEY_DeleteAsymmetricKey(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1",KEY_EXCHANGE,USBKEY_RSA_PUBLICKEY);
	if (rtn !=0)
	{
		printf("USBKEY_DeleteAsymmetricKey PublicKey error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Delete AsymmetricKey PublicKey KEY_EXCHANGE success!\n");

	rtn = USBKEY_GetAsymmetricKeyID(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1",KEY_EXCHANGE,(USBKEY_UCHAR_PTR)&rsakey);
	if (rtn !=0)
	{
		printf("USBKEY_GetAsymmetricKeyID KEY_SIGNATURE error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Get KEY_EXCHANGE AsymmetricKey ID again success!\n");
	AK_Hexdump(&rsakey,sizeof(rsakey));

	rtn = USBKEY_GetPriKeyBitLen(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1",KEY_EXCHANGE,&ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_GetPriKeyBitLen error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Get PriKey BitLen success! ulLen = %d\n",ulLen);

	rtn = USBKEY_JudgeEmptyContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1");
	if( -1 == rtn)
		printf("Judge Empty Container success! Container is not empty\n");
	else
	{
		printf("Judge Empty Container error! rtn = %d\n",rtn);
		Sleep(3000);
		return rtn;
	}

	rtn = USBKEY_DeleteAsymmetricKey(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1",KEY_EXCHANGE,USBKEY_RSA_PRIVATEKEY);
	if (rtn !=0)
	{
		printf("USBKEY_DeleteAsymmetricKey PrivateKey error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Delete AsymmetricKey PrivateKey success!\n");

	rtn = USBKEY_JudgeEmptyContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1");
	if( 0 == rtn)
		printf("Judge Empty Container success! Container is empty\n");
	else
	{
		printf("Judge Empty Container error! rtn = %d\n",rtn);
		Sleep(3000);
		return rtn;
	}

 	rtn = USBKEY_DeleteContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1");
	if (rtn !=0)
	{
		printf("USBKEY_DeleteContainer error! rtn = %#08x\n",rtn);
		return rtn;
	}

	printf("GenUKeyRSAPair_Test success!\n");
	return 0;
}

//导入RSA公私钥对(USBKEY_CreateContainer/USBKEY_DeleteContainer/USBKEY_GenerateExtRSAKeyPair/USBKEY_ImportRSAKeyPair/USBKEY_ImportRSAPublicKey/USBKEY_ImportRSAPrivateKey/USBKEY_RSAPrivateKeyOperation/USBKEY_RSAPublicKeyOperation)
USBKEY_ULONG ImportUKeyRSAPair_Test(USBKEY_HANDLE hDeviceHandle)
{
	unsigned long rtn = 0;
	USBKEY_ULONG ulLen;

	USBKEY_UCHAR pubKeyID[2],priKeyID[2];
	USBKEY_UCHAR data[1024],temp[1024],out[1024];

	USBKEY_ULONG ulRsaKeyPairsLen = 0;

	USBKEY_RSA_PRIVATE_KEY_1024 RsaPrivate;
	USBKEY_RSA_PUBLIC_KEY_1024 RsaPublic;
	USBKEY_RSA_PRIVATE_KEY_IMPORT_1024 RsaImportPrivate;

	USBKEY_UCHAR pbEncryptedData[2048];
	USBKEY_UCHAR pbRsaKeyPairs[2048];
	USBKEY_UCHAR *pbTmp;

	rtn = USBKEY_CreateContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1");
	if (rtn !=0)
	{
		printf("USBKEY_CreateContainer testContainer1 error! rtn = %#08x\n",rtn);
		return rtn;
	}

	//生成外部公私钥对
	ulLen = 1024;
	rtn = USBKEY_GenerateExtRSAKeyPair(hDeviceHandle,ulLen,(USBKEY_UCHAR_PTR)&RsaPrivate);
	if (rtn !=0)
	{
		printf("USBKEY_GenerateExtRSAKeyPair error! rtn = %#08x\n",rtn);
		return rtn;
	}
	//AK_Hexdump(&RsaPrivate,sizeof(RsaPrivate));
	printf("Generate Ext RSAKeyPair success!\n");

	//导入公私钥对
	RsaPublic.bits = RsaPrivate.bits;
	memcpy(RsaPublic.exponent,RsaPrivate.publicExponent,MAX_RSA_MODULUS_LEN_1024);
	memcpy(RsaPublic.modulus,RsaPrivate.modulus,MAX_RSA_MODULUS_LEN_1024);
	RsaImportPrivate.bits = RsaPrivate.bits;
 	memcpy(RsaImportPrivate.prime,RsaPrivate.prime,MAX_RSA_MODULUS_LEN_1024);
	memcpy(RsaImportPrivate.primeExponent,RsaPrivate.primeExponent,MAX_RSA_MODULUS_LEN_1024);
	memcpy(RsaImportPrivate.coefficient,RsaPrivate.coefficient,MAX_RSA_PRIME_LEN_1024);

	/*pbTmp = pbRsaKeyPairs;
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
	memcpy(pbTmp, Test_rsa_n, sizeof(Test_rsa_n));
	ulRsaKeyPairsLen += 3 + ulLen/8;
	pbTmp += ulLen/8;

	//e: 02 04 00 01 00 01
	pbTmp[0] = 0x02;
	pbTmp[1] = 0x04;
	pbTmp += 2;
	memcpy(pbTmp, RsaPrivate.publicExponent+ulLen/8-4, 4);
	ulRsaKeyPairsLen += 6;
	pbTmp += 4;

	///02 81 80 d, 长度为128+3=131
	pbTmp[0] = 0x02;
	pbTmp[1] = 0x81;
	pbTmp[2] = 0x80;
	pbTmp += 3;
	memcpy(pbTmp, Test_rsa_d, sizeof(Test_rsa_d));
	ulRsaKeyPairsLen += 3 + ulLen/8;
	pbTmp += ulLen/8;

	//02 40 p, 长度为64+2=66
	pbTmp[0] = 0x02;
	pbTmp[1] = 0x40;
	pbTmp += 2;
	memcpy(pbTmp, Test_rsa_p, sizeof(Test_rsa_dmp1));
	ulRsaKeyPairsLen += 2 + ulLen/16;
	pbTmp += ulLen/16;

	//02 40 q, 长度为64+2=66
	pbTmp[0] = 0x02;
	pbTmp[1] = 0x40;
	pbTmp += 2;
	memcpy(pbTmp, Test_rsa_q, sizeof(Test_rsa_q));
	ulRsaKeyPairsLen += 2 +ulLen/16;
	pbTmp += ulLen/16;

	//02 40 dp, 长度为64+2=66
	pbTmp[0] = 0x02;
	pbTmp[1] = 0x40;
	pbTmp += 2;
	memcpy(pbTmp, Test_rsa_dmp1, sizeof(Test_rsa_dmp1));
	ulRsaKeyPairsLen += 2 + ulLen/16;
	pbTmp += ulLen/16;

	//02 40 dq, 长度为64+2=66
	pbTmp[0] = 0x02;
	pbTmp[1] = 0x40;
	pbTmp += 2;
	memcpy(pbTmp, Test_rsa_dmq1, sizeof(Test_rsa_dmq1));
	ulRsaKeyPairsLen += 2 + ulLen/16;
	pbTmp += ulLen/16;
	
	//02 40 qinv, 长度为64+2=66
	pbTmp[0] = 0x02;
	pbTmp[1] = 0x40;
	pbTmp += 2;
	memcpy(pbTmp, Test_rsa_iqmp, sizeof(Test_rsa_iqmp));
	ulRsaKeyPairsLen += 2 + ulLen/16;
	pbTmp += ulLen/16;
	
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
						(USBKEY_UCHAR_PTR)"testContainer1",
						KEY_EXCHANGE,
						pbRsaKeyPairs,
						ulRsaKeyPairsLen,
						priKeyID,
						pubKeyID);

	printf("Import RSAKeyPair success!\n");
	AK_Hexdump(pubKeyID,2);
	AK_Hexdump(priKeyID,2);

	//公私钥运算
	ulLen = 256;
	memset(temp,0,sizeof(temp));
	memset(temp,'3',127);
	memset(out,0,sizeof(out));
	ulLen = sizeof(out);
	temp[0] = 0;
	printf("RSA Operation data is:\n");
	AK_Hexdump(temp,128);

	rtn = USBKEY_RSAPrivateKeyOperation(hDeviceHandle,priKeyID,temp,128,out,&ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_RSAPrivateKeyOperation error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("RSA PrivateKey Operation success!\n");
	AK_Hexdump(out,ulLen);
	
	memset(temp,0,sizeof(temp));
	rtn = USBKEY_RSAPublicKeyOperation(hDeviceHandle,pubKeyID,out,ulLen,temp,&ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_RSAPublicKeyOperation error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("RSA PublicKey Operation success!\n");
	AK_Hexdump(temp,ulLen);
*/
	//导入公、私钥
  	rtn = USBKEY_ImportRSAPublicKey(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1",KEY_SIGNATURE,(USBKEY_UCHAR_PTR)&RsaPublic,sizeof(RsaPublic),pubKeyID);
	if (rtn !=0)
	{
		printf("USBKEY_ImportRSAPublicKey KEY_SIGNATURE error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Import RSA PublicKey success!\n");
	
	rtn = USBKEY_ImportRSAPrivateKey(hDeviceHandle,USBKEY_PRIKEY_TYPE_AI,NULL,USBKEY_ECB,NULL,0,(USBKEY_UCHAR_PTR)"testContainer1",
							KEY_SIGNATURE,(USBKEY_UCHAR_PTR)&RsaImportPrivate,sizeof(RsaImportPrivate),priKeyID);
	if (rtn !=0)
	{
		printf("USBKEY_ImportRSAPrivateKey KEY_SIGNATURE error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Import RSA PrivateKey success!\n");

	//公私钥运算
	memset(out,0,sizeof(out));
	ulLen = sizeof(out);
	memset(data,0,sizeof(data));
	rtn = USBKEY_GenRandom(hDeviceHandle,data,128);
	if (rtn !=0)
	{
		printf("USBKEY_GenRandom error! rtn = %#08x\n",rtn);
		return rtn;
	}
	data[0] = 0x00;
	data[1]= 0x02;

	rtn = USBKEY_RSAPrivateKeyOperation(hDeviceHandle,priKeyID,data,128,out,&ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_RSAPrivateKeyOperation error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("RSA PrivateKey Operation success!\n");
	
	memset(temp,0,sizeof(temp));
	rtn = USBKEY_RSAPublicKeyOperation(hDeviceHandle,pubKeyID,out,ulLen,temp,&ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_RSAPublicKeyOperation error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("RSA PublicKey Operation success!\n");

	if(128 == ulLen && 0 == memcmp(temp,data,ulLen))
		printf("RSA Private & Public Operation success,the data and the decrpyt data same\n");
	else
	{
		printf("RSA Private & Public Operation error! the data and the decrpyt data is different\n");
		printf("the data is:\n");
		AK_Hexdump(data,128);
		printf("the decrpyt data is:\n");
		AK_Hexdump(temp,ulLen);
		Sleep(3000);
		return -1;
	}

	rtn = USBKEY_DeleteContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1");
	if (rtn !=0)
	{
		printf("USBKEY_DeleteContainer error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("ImportUKeyRSAPair_Test success!\n");
	return 0;
}


//临时RSA公私钥对操作(USBKEY_GenerateExtRSAKeyPair/USBKEY_TempRSAPublicKeyOperation/USBKEY_TempRSAPrivateKeyOperation)
USBKEY_ULONG TempKeyRSAPairOpt_Test(USBKEY_HANDLE hDeviceHandle)
{
	unsigned long rtn = 0;
	USBKEY_ULONG ulLen;

	USBKEY_UCHAR data[1024],temp[1024],out[1024];
	USBKEY_RSA_PRIVATE_KEY_2048 rsakeyPair;
	//USBKEY_RSA_PRIVATE_KEY_1024 rsakeyPair;
	USBKEY_RSA_PRIVATE_KEY_IMPORT_2048 priKeyR;
	//USBKEY_RSA_PRIVATE_KEY_IMPORT_1024 priKeyR;
	USBKEY_RSA_PUBLIC_KEY_2048 pubKeyR;
	//USBKEY_RSA_PUBLIC_KEY_1024 pubKeyR;

	//产生临时RSA密钥
	rtn = USBKEY_GenerateExtRSAKeyPair(hDeviceHandle,2048,(USBKEY_UCHAR_PTR)&rsakeyPair);
	if (rtn !=0)
	{
		printf("USBKEY_GenerateExtRSAKeyPair error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Generate Ext RSAKeyPair success!\n");

	priKeyR.bits = rsakeyPair.bits;
	memcpy((USBKEY_UCHAR_PTR)&pubKeyR,(USBKEY_UCHAR_PTR)&rsakeyPair,sizeof(pubKeyR));
	memcpy((USBKEY_UCHAR_PTR)&priKeyR+4,(USBKEY_UCHAR_PTR)&rsakeyPair+516,sizeof(priKeyR)-4);

	//临时RSA加密
	memset(data,0,sizeof(data));
	memset(out,0,sizeof(out));
	ulLen = sizeof(out);
	rtn = USBKEY_GenRandom(hDeviceHandle,data+20,236);
	if (rtn !=0)
	{
		printf("USBKEY_GenRandom error! rtn = %#08x\n",rtn);
		return rtn;
	}

	rtn = USBKEY_TempRSAPublicKeyOperation(hDeviceHandle,(USBKEY_UCHAR_PTR)&pubKeyR,sizeof(pubKeyR),data,256,out,&ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_TempRSAPublicKeyOperation error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Temp RSA PublicKey Operation success!\n");

	memset(temp,0,sizeof(temp));
	rtn = USBKEY_TempRSAPrivateKeyOperation(hDeviceHandle,(USBKEY_UCHAR_PTR)&priKeyR,sizeof(priKeyR),out,ulLen,temp,&ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_TempRSAPrivateKeyOperation error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Temp RSA PrivateKey Operation success!\n");

	if(256 == ulLen && 0 == memcmp(temp,data,ulLen))
		printf("Temp RSA Public & Private Operation success,the data and the decrpyt data same\n");
	else
	{
		printf("Temp RSA Public & Private  Operation error! the data and the decrpyt data is different\n");
		Sleep(3000);
		return -1;
	}

	//临时RSA私钥加密
	memset(data,0,sizeof(data));
	memset(out,0,sizeof(out));
	ulLen = sizeof(out);
	rtn = USBKEY_GenRandom(hDeviceHandle,data,256);
	if (rtn !=0)
	{
		printf("USBKEY_GenRandom error! rtn = %#08x\n",rtn);
		return rtn;
	}
	data[0] = 0x00;

	rtn = USBKEY_TempRSAPrivateKeyOperation(hDeviceHandle,(USBKEY_UCHAR_PTR)&priKeyR,sizeof(priKeyR),data,256,out,&ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_TempRSAPrivateKeyOperation error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Temp RSA PrivateKey Operation success!\n");

	memset(temp,0,sizeof(temp));
	rtn = USBKEY_TempRSAPublicKeyOperation(hDeviceHandle,(USBKEY_UCHAR_PTR)&pubKeyR,sizeof(pubKeyR),out,ulLen,temp,&ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_TempRSAPublicKeyOperation error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Temp RSA PublicKey Operation success!\n");

	if(256 == ulLen && 0 == memcmp(temp,data,ulLen))
		printf("Temp RSA Private & Public Operation success,the data and the decrpyt data same\n");
	else
	{
		printf("Temp RSA Private & Public Operation error! the data and the decrpyt data is different\n");
		printf("the data is:\n");
		AK_Hexdump(data,256);
		printf("the decrpyt data is:\n");
		AK_Hexdump(temp,ulLen);
		Sleep(3000);
		Sleep(3000);
		return -1;
	}

	printf("TempKeyRSAPairOpt_Test success!\n");
	return 0;
}


//ukey对称密钥及容器对称密钥操作(USBKEY_GenSymmetricKey/USBKEY_GenRandom/USBKEY_ImportSymmetricKey/USBKEY_ExportSymmetricKey/USBKEY_MassiveDataEncrypt/USBKEY_MassiveDataDecrypt
//USBKEY_GenContainerSymmetricKey/USBKEY_ImportContainerSymmetricKey)
USBKEY_ULONG UKeySymmetricKeyOpt_Test(USBKEY_HANDLE hDeviceHandle)
{
	unsigned long rtn = 0;
	USBKEY_ULONG ulLen,ulEncryptedLen;

	//USBKEY_UCHAR uckey[16] = {0x27,0xfb,0x01,0x48,0x10,0x4f,0x90,0x94,0x10,0xa9,0x6e,0x3a,0xdf,0x44,0x3d,0x30};
	USBKEY_UCHAR temp[4096],out[4096];
	USBKEY_UCHAR pubKeyID[2],priKeyID[2];
	USBKEY_UCHAR phKey[2];
	//USBKEY_UCHAR pcIV[16];
	USBKEY_UCHAR pcIV[16],uckey[16];
	/*USBKEY_UCHAR data[ ] = {0x51,0xdb,0xc9,0x4c,0x2e,0x7c,0xfe,0x66,0xd8,0x27,0x3f,0x52,0xf8,0x15,0xbd,0x7f,
0x2b,0xdc,0x36,0x9a,0xdb,0x2b,0x2e,0x0f,0x4a,0xd9,0x88,0xb1,0xb4,0x01,0xc1,0x4e,
0xbb,0x4e,0xca,0x5e,0xa1,0x97,0xcb,0xa6,0xfd,0x1a,0xea,0x10,0xd7,0x65,0x6c,0xbf,
0x23,0xc0,0x06,0x59,0xc0,0x5a,0x61,0xd5,0x93,0xaf,0x3d,0xd3,0x35,0x83,0xfb,0xcb,
0x42,0xe8,0x5e,0x8c,0x8f,0x76,0x07,0xcf,0x14,0x2d,0xfb,0x7a,0x2b,0xfd,0xdb,0xbf,
0x82,0x6c,0x6c,0xf6,0x1b,0xde,0x73,0xe2,0x52,0xd0,0xe5,0x63,0x85,0x5f,0x83,0xbe,
0x4b,0x05,0x8d,0xd7,0x73,0x9d,0x03,0x15,0xab,0xd7,0x5b,0x07,0x33,0xe0,0xd7,0x7e,
0x3f,0x8a,0x61,0x97,0x82,0x2a,0x2c,0xb3,0x12,0xdd,0x46,0xbc,0x76,0xf8,0xbb,0xb3,
0x3b,0x80,0x89,0xd1,0x05,0x02,0x29,0x15,0xf1,0x4b,0xef,0x41,0x74,0xd5,0x0a,0xcc,
0x03,0x0f,0x89,0x75,0x6b,0xdd,0xa6,0x35,0xa3,0xf8,0xee,0xf0,0xea,0xfa,0x34,0x99,
0x16,0xc4,0x34,0x8a,0x25,0xc9,0x52,0x4e,0x29,0xe4,0x10,0x62,0x33,0x22,0x65,0x2a,
0x7b,0x41,0xb2,0x1a,0x04,0x8a,0xf5,0x67,0x50,0x9d,0x70,0x45,0xb4,0xb3,0x93,0x58,
0xca,0x2c,0x0b,0x01,0xa5,0x9d,0x8e,0x91,0x89,0xc0,0xf2,0x73,0x50,0x4d,0x1d,0xbc,
0x57,0xd3,0xb7,0x6e,0x87,0x44,0xf7,0xaa,0x98,0xb5,0xca,0x11,0xdd,0x04,0x51,0x7e,
0xe4,0x21,0xf6,0x39,0x5f,0x2e,0x35,0xdf,0x66,0x93,0xd4,0x46,0x93,0xfd,0x9e,0x00,
0xd0,0x35,0x08,0xac,0x43,0x81,0x82,0xe2,0x4b,0xe9,0xd8,0xb8,0x38,0x15,0xb2,0x60,
0xc5,0xf6,0x87,0xb2,0x6f,0xba,0xc5,0xeb,0x6a,0x1d,0x0b,0x2d,0x5d,0xff,0xa6,0xb5,
0x19,0x1b,0x5c,0x76,0xe2,0x76,0x18,0xf2,0x81,0x01,0xbf,0x88,0xa1,0x25,0x12,0x43,
0xf7,0x86,0x70,0xbe,0x9a,0x09,0x25,0x36,0x8f,0xa8,0x75,0xb0,0xc0,0xb6,0x02,0xbd,
0x13,0x58,0x85,0x18,0x17,0x39,0xdc,0x6d,0x62,0x9c,0xc3,0xd2,0xec,0x13,0x09,0xe0,
0xc4,0x3c,0x38,0xd8,0x76,0xd2,0xb4,0xbb,0xca,0x5a,0x15,0x14,0x5b,0x8a,0x74,0x26,
0x3e,0x50,0x8f,0xc3,0x53,0x6f,0xfa,0xfb,0xee,0xf3,0xbb,0x12,0xf9,0x3d,0xf1,0x58,
0x8f,0xe0,0xee,0xce,0xe5,0x00,0xd4,0xb3,0x33,0x16,0x3a,0xa4,0x09,0xe2,0x7b,0xe0,
0xd7,0xfe,0x60,0x41,0xcf,0xf3,0x94,0xc1,0x65,0xe0,0x04,0xcc,0x63,0xa9,0x18,0x66,
0xe5,0xf8,0x16,0x5b,0x59,0x6a,0x12,0xb3,0x4d,0xf2,0x46,0xad,0x01,0x30,0xea,0xeb,
0x55,0x21,0x4e,0x53,0x92,0x4e,0xb6,0xed,0xa8,0x49,0xc9,0x17,0xea,0xbf,0x28,0x4c,
0x37,0x38,0x03,0x63,0xe6,0xdf,0xc2,0x70,0xa2,0x4c,0x9e,0xf9,0x66,0xee,0x25,0x22,
0x4a,0x59,0xeb,0x66,0x56,0x44,0x72,0x94,0xc8,0x0f,0x44,0x85,0x0b,0xf8,0x0b,0xa1,
0x24,0x13,0x0d,0x91,0x1f,0x40,0x2b,0x16,0x80,0x05,0x3a,0x00,0xa1,0xb1,0x85,0x3b,
0xec,0xa7,0xeb,0xc3,0x9d,0x10,0x27,0x4b,0x2b,0x74,0x89,0x3d,0x0f,0xd4,0xd7,0x88,
0xed,0x71,0x60,0x1e,0x5a,0xbd,0xf2,0xd6,0x70,0x60,0xba,0x00,0xe0,0x9c,0xdb,0x2a,
0x0e,0x7a,0x1c,0x53,0xfc,0xe0,0xaa,0xe9,0x14,0xbc,0xf2,0xee,0x5d,0x9c,0x4b,0x87,
0x03,0x76,0xa3,0x4b,0x93,0x8c,0x1f,0x86,0xa7,0xbb,0xcf,0xab,0x5f,0xcc,0xb6,0x76,
0xe9,0xd5,0xda,0x0a,0xb1,0x16,0xe4,0x58,0xea,0x3e,0xbf,0x7e,0x39,0x1b,0xed,0x9d,
0xed,0xc7,0x13,0x84,0x0f,0xe2,0x95,0xc8,0xf8,0xa3,0xdb,0x95,0x4b,0xa9,0x28,0x08,
0xc6,0xb2,0x6c,0xfc,0x34,0xe4,0xf7,0x55,0x24,0xa5,0xf8,0xd0,0xcb,0xe5,0x2e,0xbc,
0x71,0xeb,0xed,0x74,0xb5,0x8e,0xf1,0x61,0x29,0x11,0x9d,0x46,0xa6,0x0a,0x0c,0x7b,
0x50,0xfc,0x7e,0xb7,0x8c,0x37,0x10,0xbb,0xc5,0xaf,0x1d,0xcf,0xf5,0x3b,0x7e,0xb5,
0x99,0x8a,0xd1,0xc2,0xd4,0x68,0x04,0x6f,0xa4,0x67,0x6a,0x6e,0xdc,0x7a,0x12,0x88,
0x49,0x1f,0x0b,0x19,0xe7,0x13,0x99,0x0d,0xcd,0x27,0x8e,0xb0,0x42,0xcb,0x8d,0x47};*/
	USBKEY_UCHAR data[4096];
	//USBKEY_UCHAR temp[4096],out[4096];


	//生成对称密钥
	rtn = USBKEY_GenSymmetricKey(hDeviceHandle,8,ALG_TYPE_3DES_2KEY,USBKEY_CRYPT_EXPORTABLE|USBKEY_CRYPT_ONDEVICE|USBKEY_CRYPT_USER_PROTECTED,phKey);
	if (rtn !=0)
	{
		printf("USBKEY_GenSymmetricKey error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("UKEY Gen SymmetricKey success!\n");

	//对称密钥加解密
	//批量数据加密
	memset(data,0,sizeof(data));
	memset(out,0,sizeof(out));
	memset(pcIV, 0x3, 16);
	ulEncryptedLen = sizeof(out);
	rtn = USBKEY_GenRandom(hDeviceHandle,data,1024);
	if (rtn !=0)
	{
		printf("USBKEY_GenRandom error! rtn = %#08x\n",rtn);
		return rtn;
	}

	rtn = USBKEY_MassiveDataEncrypt(
				hDeviceHandle,					//[in]设备句柄
				phKey,							//[in]用来加密的密钥ID
				ALG_TYPE_3DES_2KEY,				//[in]加密算法
				USBKEY_CBC|USBKEY_NO_PADDING,	//[in]加密模式和填充模式
				pcIV,							//[in]初始化向量，如果是ecb模式，设置为NULL
				data,							//[in]待加密数据
				1024,							//[in]待加密数据长度
				out,							//[out]返回的加密后数据
				&ulEncryptedLen);//[in/out]in: pcEncryptedData缓冲区大小，out: 加密后数据长度或需要的pcEncryptedData缓冲区大小
	if (rtn !=0)
	{
		printf("USBKEY_MassiveDataEncrypt error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Massive DataEncrypt success!\n");

	//批量数据解密
	memset(temp,0,sizeof(temp));
	ulLen = sizeof(temp);
	rtn = USBKEY_MassiveDataDecrypt(
				hDeviceHandle,						//[in]设备句柄
				phKey,							//[in]用来加密的密钥ID
				ALG_TYPE_3DES_2KEY,					//[in]加密算法
				USBKEY_CBC|USBKEY_NO_PADDING,	//[in]加密模式和填充模式
				pcIV,								//[in]初始化向量，如果是ecb模式，设置为NULL
				out,								//[in]密文数据
				ulEncryptedLen,						//[in]密文数据长度
				temp,								//[out]返回的明文
				&ulLen);								//[in/out]in: pcData缓冲区长度 out:返回的明文长度
	if (rtn !=0)
	{
		printf("USBKEY_MassiveDataDecrypt error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Massive DataDecrypt success!\n");

	if(1024 == ulLen && 0 == memcmp(temp,data,ulLen))
		printf("Massive SymmetricKey Operation success(1024 bytes),the data and the decrpyt data same\n");
	else
	{
		printf("Massive SymmetricKey Operation error(1024 bytes)! the data and the decrpyt data is different\n");
		Sleep(3000);
		return -1;
	}

	//导入对称密钥
	memset(uckey,0,sizeof(uckey));
	rtn = USBKEY_GenRandom(hDeviceHandle,uckey,16);
	if (rtn !=0)
	{
		printf("USBKEY_GenRandom error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Gen Ext SymmetricKey success!\n");
	memset(pcIV,0,sizeof(pcIV));
	rtn = USBKEY_ImportSymmetricKey(hDeviceHandle,NULL,USBKEY_TYPE_RSA,USBKEY_CRYPT_EXPORTABLE|USBKEY_CRYPT_ONDEVICE,ALG_TYPE_SCB2,uckey,16,phKey);
	if (rtn !=0)
	{
		printf("USBKEY_ImportSymmetricKey error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Import SymmetricKey success!\n");


	//导出对称密钥
	rtn = USBKEY_CreateContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1");
	if (rtn !=0)
	{
		printf("USBKEY_CreateContainer testContainer1 error! rtn = %#08x\n",rtn);
		return rtn;
	}

	rtn = USBKEY_GenerateRSAKeyPair(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1",KEY_EXCHANGE,1024,USBKEY_CRYPT_USER_PROTECTED,pubKeyID,priKeyID);
	if (rtn !=0)
	{
		printf("USBKEY_GenerateRSAKeyPair error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Generate RSA KeyPair success!\n");

	memset(temp,0,sizeof(temp));
	ulLen = sizeof(temp);
	rtn = USBKEY_ExportSymmetricKey(hDeviceHandle,phKey,pubKeyID,USBKEY_TYPE_RSA,temp,&ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_ExportSymmetricKey error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Export SymmetricKey success\n");

	memset(out,0,sizeof(out));
	rtn = USBKEY_RSAPrivateKeyOperation(hDeviceHandle,priKeyID,temp,ulLen,out,&ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_RSAPrivateKeyOperation error! rtn = %#08x\n",rtn);
		return rtn;
	}

	rtn = rsa_pkcs1_No_padding(out, &ulLen, 128);
	if (rtn !=0)
	{
		printf("rsa_pkcs1_No_padding error! rtn = %#08x\n",rtn);
		return rtn;
	}
	if(16 == ulLen && 0 == memcmp(uckey,out,ulLen))
		printf("Export SymmetricKey decrypt success,it is the same with SymmetricKey\n");
	else
	{
		printf("Export SymmetricKey decrypt error! it is different with SymmetricKey\n");
		Sleep(3000);
		return -1;
	}

	//对称密钥加解密
	//批量数据加密
	memset(out,0,sizeof(out));
	memset(pcIV, 0x3, 16);
	ulEncryptedLen = sizeof(out);
	rtn = USBKEY_GenRandom(hDeviceHandle,data,55);
	if (rtn !=0)
	{
		printf("USBKEY_GenRandom error! rtn = %#08x\n",rtn);
		return rtn;
	}

	rtn = USBKEY_MassiveDataEncrypt(
				hDeviceHandle,					//[in]设备句柄
				phKey,							//[in]用来加密的密钥ID
				ALG_TYPE_SCB2,					//[in]加密算法
				USBKEY_CBC|USBKEY_PKCS5_PADDING,	//[in]加密模式和填充模式
				pcIV,							//[in]初始化向量，如果是ecb模式，设置为NULL
				data,							//[in]待加密数据
				55,							//[in]待加密数据长度
				out,							//[out]返回的加密后数据
				&ulEncryptedLen);//[in/out]in: pcEncryptedData缓冲区大小，out: 加密后数据长度或需要的pcEncryptedData缓冲区大小
	if (rtn !=0)
	{
		printf("USBKEY_MassiveDataEncrypt error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Massive DataEncrypt success!\n");

	//批量数据解密
	memset(temp,0,sizeof(temp));
	ulLen = sizeof(temp);
	rtn = USBKEY_MassiveDataDecrypt(
				hDeviceHandle,						//[in]设备句柄
				phKey,							//[in]用来加密的密钥ID
				ALG_TYPE_SCB2,					//[in]加密算法
				USBKEY_CBC|USBKEY_PKCS5_PADDING,	//[in]加密模式和填充模式
				pcIV,								//[in]初始化向量，如果是ecb模式，设置为NULL
				out,								//[in]密文数据
				ulEncryptedLen,						//[in]密文数据长度
				temp,								//[out]返回的明文
				&ulLen);								//[in/out]in: pcData缓冲区长度 out:返回的明文长度
	if (rtn !=0)
	{
		printf("USBKEY_MassiveDataDecrypt error! rtn = %#08x\n",rtn);
		return rtn;
	}
	AK_Hexdump(temp,ulLen);
	printf("Massive DataDecrypt success!\n");
	
	if(55 == ulLen && 0 == memcmp(temp,data,ulLen))
		printf("Massive SymmetricKey Operation success(55 bytes),the data and the decrpyt data same\n");
	else
	{
		printf("Massive SymmetricKey Operation error(55 bytes)! the data and the decrpyt data is different\n");
		Sleep(3000);
		return -1;
	}

	//生成容器对称密钥
	rtn = USBKEY_GenContainerSymmetricKey(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1",16,ALG_TYPE_SSF33,USBKEY_CRYPT_EXPORTABLE|USBKEY_CRYPT_ONDEVICE|USBKEY_CRYPT_USER_PROTECTED,phKey);
	if (rtn !=0)
	{
		printf("USBKEY_GenContainerSymmetricKey error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Gen Container SymmetricKey success!\n");

	//导入对称密钥
	memset(uckey,0,sizeof(uckey));
	rtn = USBKEY_GenRandom(hDeviceHandle,uckey,16);
	if (rtn !=0)
	{
		printf("USBKEY_GenRandom error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Gen Ext Container SymmetricKey success!\n");
	
	rtn = USBKEY_ImportContainerSymmetricKey(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1",NULL,USBKEY_TYPE_RSA,USBKEY_CRYPT_EXPORTABLE|USBKEY_CRYPT_ONDEVICE,ALG_TYPE_SSF33,temp,16,phKey);
	if (rtn !=0)
	{
		printf("USBKEY_ImportContainerSymmetricKey error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Import Container SymmetricKey success!\n");
	

	//对称密钥加解密
	//批量数据加密
	memset(data,0,sizeof(data));
	memset(out,0,sizeof(out));
	memset(pcIV, 0x3, 16);
	ulEncryptedLen = sizeof(out);
	rtn = USBKEY_GenRandom(hDeviceHandle,data,252);
	if (rtn !=0)
	{
		printf("USBKEY_GenRandom error! rtn = %#08x\n",rtn);
		return rtn;
	}

	rtn = USBKEY_MassiveDataEncrypt(
				hDeviceHandle,					//[in]设备句柄
				phKey,							//[in]用来加密的密钥ID
				ALG_TYPE_SSF33,					//[in]加密算法
				USBKEY_CBC|USBKEY_PKCS5_PADDING,	//[in]加密模式和填充模式
				pcIV,							//[in]初始化向量，如果是ecb模式，设置为NULL
				data,							//[in]待加密数据
				252,							//[in]待加密数据长度
				out,							//[out]返回的加密后数据
				&ulEncryptedLen);//[in/out]in: pcEncryptedData缓冲区大小，out: 加密后数据长度或需要的pcEncryptedData缓冲区大小
	if (rtn !=0)
	{
		printf("USBKEY_MassiveDataEncrypt error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Massive DataEncrypt success!\n");

	//批量数据解密
	memset(temp,0,sizeof(temp));
	ulLen = sizeof(temp);
	rtn = USBKEY_MassiveDataDecrypt(
				hDeviceHandle,						//[in]设备句柄
				phKey,							//[in]用来加密的密钥ID
				ALG_TYPE_SSF33,					//[in]加密算法
				USBKEY_CBC|USBKEY_PKCS5_PADDING,	//[in]加密模式和填充模式
				pcIV,								//[in]初始化向量，如果是ecb模式，设置为NULL
				out,								//[in]密文数据
				ulEncryptedLen,						//[in]密文数据长度
				temp,								//[out]返回的明文
				&ulLen);								//[in/out]in: pcData缓冲区长度 out:返回的明文长度
	if (rtn !=0)
	{
		printf("USBKEY_MassiveDataDecrypt error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Massive DataDecrypt success!\n");

	if(252 == ulLen && 0 == memcmp(temp,data,ulLen))
		printf("Massive SymmetricKey Operation success(252 bytes),the data and the decrpyt data same\n");
	else
	{
		printf("Massive SymmetricKey Operation error(252 bytes)! the data and the decrpyt data is different\n");
		Sleep(3000);
		return -1;
	}

	rtn = USBKEY_DeleteContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1");
	if (rtn !=0)
	{
		printf("USBKEY_DeleteContainer error! rtn = %#08x\n",rtn);
		return rtn;
	}

	printf("UKeySymmetricKeyOpt_Test success!\n");
	return 0;
}

//临时对称密钥操作(USBKEY_GenRandom/USBKEY_TempDataEncrypt/USBKEY_TempDataDecrypt)
USBKEY_ULONG TempSymmetricKeyOpt_Test(USBKEY_HANDLE hDeviceHandle)
{
	unsigned long rtn = 0;
	USBKEY_ULONG ulEncryptedLen;

	USBKEY_UCHAR data[1024],temp[1024],out[1024];
	USBKEY_UCHAR pTempKey[16];

	memset(temp,0,sizeof(temp));
	rtn = USBKEY_GenRandom(hDeviceHandle,pTempKey,16);
	if (rtn !=0)
	{
		printf("USBKEY_GenRandom error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Gen Temp SymmetricKey success\n");

	//临时对称加密
	memset(data,0,sizeof(data));
	memset(out,0,sizeof(out));
	ulEncryptedLen = sizeof(out);
	rtn = USBKEY_GenRandom(hDeviceHandle,data,256);
	if (rtn !=0)
	{
		printf("USBKEY_GenRandom error! rtn = %#08x\n",rtn);
		return rtn;
	}

	rtn = USBKEY_TempDataEncrypt(hDeviceHandle,pTempKey,ALG_TYPE_SCB2,USBKEY_ECB|USBKEY_NO_PADDING,NULL,data,256,out,&ulEncryptedLen);
	if (rtn !=0)
	{
		printf("USBKEY_TempDataEncrypt error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Temp Data Encrypt success:\n");

	//外部解密
	memset(temp,0,sizeof(temp));
	rtn = USBKEY_TempDataDecrypt(hDeviceHandle,pTempKey,ALG_TYPE_SCB2,USBKEY_ECB|USBKEY_NO_PADDING,NULL,out,ulEncryptedLen,temp,&ulEncryptedLen);
	if (rtn !=0)
	{
		printf("USBKEY_TempDataDecrypt error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Temp Data Decrypt success:\n");

	if(256 == ulEncryptedLen && 0 == memcmp(temp,data,ulEncryptedLen))
		printf("Temp SymmetricKey Operation success(256 bytes),the data and the decrpyt data same\n");
	else
	{
		printf("Temp SymmetricKey Operation error(256 bytes)! the data and the decrpyt data is different\n");
		Sleep(3000);
		return -1;
	}

	printf("TempSymmetricKeyOpt_Test success!\n");
	return 0;
}

//证书操作(USBKEY_WriteCertificate/USBKEY_ReadCertificate/USBKEY_DeleteCertificate/USBKEY_IsCertificateExist)
USBKEY_ULONG UKeyCertOpt_Test(USBKEY_HANDLE hDeviceHandle)
{
	unsigned long rtn = 0;
	USBKEY_ULONG ulLen;

	USBKEY_UCHAR temp[4096];


	rtn = USBKEY_CreateContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1");
	if (rtn !=0)
	{
		printf("USBKEY_CreateContainer error! rtn = %#08x\n",rtn);
		return rtn;
	}
	
	ulLen = 11;
	rtn = USBKEY_WriteCertificate(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1",KEY_SIGNATURE,(USBKEY_UCHAR_PTR)"testCertSig",&ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_WriteCertificate KEY_SIGNATURE error! rtn = %#08x\n",rtn);
		return rtn;
	}

	rtn = USBKEY_WriteCertificate(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1",KEY_EXCHANGE,(USBKEY_UCHAR_PTR)"testCertExc",&ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_WriteCertificate KEY_EXCHANGE error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Write Certificates success!\n");

	rtn = USBKEY_IsCertificateExist(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1",KEY_SIGNATURE);
	if( 0 == rtn)
		printf("Judge Certificate Exist success! KEY_SIGNATURE Certificate is Exist\n");
	else
	{
		printf("Judge KEY_SIGNATURE Certificate Exist error! rtn = %d\n",rtn);
		Sleep(3000);
		return rtn;
	}

	rtn = USBKEY_IsCertificateExist(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1",KEY_EXCHANGE);
	if( 0 == rtn)
		printf("Judge Certificate Exist success! KEY_EXCHANGE Certificate is Exist\n");
	else
	{
		printf("Judge KEY_EXCHANGE Certificate Exist error! rtn = %d\n",rtn);
		Sleep(3000);
		return rtn;
	}
	
	rtn = USBKEY_ReadCertificate(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1",KEY_EXCHANGE,NULL,&ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_ReadCertificate KEY_EXCHANGE error! rtn = %#08x\n",rtn);
		return rtn;
	}
	memset(temp,0,sizeof(temp));
	rtn = USBKEY_ReadCertificate(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1",KEY_EXCHANGE,temp,&ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_ReadCertificate KEY_EXCHANGE error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Read Certificate KEY_EXCHANGE success:\n");
	AK_Hexdump(temp,ulLen);

	rtn = USBKEY_ReadCertificate(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1",KEY_SIGNATURE,NULL,&ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_ReadCertificate KEY_SIGNATURE error! rtn = %#08x\n",rtn);
		return rtn;
	}
	memset(temp,0,sizeof(temp));
	rtn = USBKEY_ReadCertificate(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1",KEY_SIGNATURE,temp,&ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_ReadCertificate KEY_SIGNATURE error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Read Certificate KEY_SIGNATURE success:\n");
	AK_Hexdump(temp,ulLen);

	rtn = USBKEY_DeleteCertificate(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1",KEY_EXCHANGE);
	if (rtn !=0)
	{
		printf("USBKEY_WriteCertificate KEY_EXCHANGE error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Delete Certificate KEY_EXCHANGE success!\n");

	rtn = USBKEY_ReadCertificate(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1",KEY_EXCHANGE,NULL,&ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_ReadCertificate KEY_EXCHANGE error! the cert has deleted.rtn = %#08x\n",rtn);
		//return rtn;
	}

	rtn = USBKEY_IsCertificateExist(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1",KEY_EXCHANGE);
	if( -1 == rtn)
		printf("Judge Certificate Exist again success! KEY_EXCHANGE Certificate is not Exist\n");
	else
	{
		printf("Judge KEY_EXCHANGE Certificate Exist again error! rtn = %d\n",rtn);
		Sleep(3000);
		return rtn;
	}
	
	rtn = USBKEY_IsCertificateExist(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1",KEY_SIGNATURE);
	if( 0 == rtn)
		printf("Judge Certificate Exist again success! KEY_SIGNATURE Certificate is Exist\n");
	else
	{
		printf("Judge KEY_SIGNATURE Certificate Exist again error! rtn = %d\n",rtn);
		Sleep(3000);
		return rtn;
	}

	rtn = USBKEY_DeleteContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1");
	if (rtn !=0)
	{
		printf("USBKEY_DeleteContainer error! rtn = %#08x\n",rtn);
		return rtn;
	}

	printf("UKeyCertOpt_Test success!\n");
	return 0;
}


//ECC公私钥对生成及相关操作(USBKEY_CreateContainer/USBKEY_DeleteContainer/USBKEY_JudgeEmptyContainer/USBKEY_GenerateECCKeyPair/
//USBKEY_ExportECCPublicKey/USBKEY_ECCSign/USBKEY_ECCVerify/USBKEY_GenRandom/USBKEY_ImportSymmetricKey/USBKEY_ExportSymmetricKey)
USBKEY_ULONG GenUKeyECCPair_Test(USBKEY_HANDLE hDeviceHandle)
{
	unsigned long rtn = 0;
	USBKEY_ULONG ulLen;

	USBKEY_UCHAR pubKeyID[2],priKeyID[2],phKey[2];
	USBKEY_UCHAR data[128],temp[128],out[128];
	USBKEY_ECC_PUBLIC_KEY stEccPubkey,stExpPubkey;
	USBKEY_ECC_PRIVATE_KEY priKeyE;
	USBKEY_ECC_PUBLIC_KEY pubKeyE;

	rtn = USBKEY_CreateContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1");
	if (rtn !=0)
	{
		printf("USBKEY_CreateContainer testContainer1 error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Create Containers success!\n");

	rtn = USBKEY_GenerateECCKeyPair(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1",KEY_EXCHANGE,256,USBKEY_CRYPT_USER_PROTECTED,pubKeyID,priKeyID,&stEccPubkey);
	if (rtn !=0)
	{
		printf("USBKEY_GenerateECCKeyPair error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Generate ECC KeyPair success!\n");

	rtn = USBKEY_ExportECCPublicKey(hDeviceHandle,pubKeyID,&stExpPubkey);
	if (rtn !=0)
	{
		printf("USBKEY_ExportECCPublicKey error! rtn = %#08x\n",rtn);
		return rtn;
	}

	if( 0 == memcmp(&stExpPubkey,&stEccPubkey,sizeof(stEccPubkey)) )
		printf("Export ECC PublicKey success,it is the same with Gen\n");
	else
	{
		printf("Export ECC PublicKey  error! it is different with Gen\n");
		Sleep(3000);
		return -1;
	}

	//ECC签名验签运算
	memset(data,0,sizeof(temp));
	memset(out,0,sizeof(out));
	ulLen = sizeof(out);
	rtn = USBKEY_GenRandom(hDeviceHandle,data,32);
	if (rtn !=0)
	{
		printf("USBKEY_GenRandom error! rtn = %#08x\n",rtn);
		return rtn;
	}

	rtn = USBKEY_ECCSign(hDeviceHandle,priKeyID,data,32,out,&ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_ECCSign error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("ECC Sign Operation success!\n");
	
	rtn = USBKEY_ECCVerify(hDeviceHandle,pubKeyID,data,32,out,ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_ECCVerify error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("ECC Verify Operation success!\n");

	rtn = USBKEY_JudgeEmptyContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1");
	if( -1 == rtn)
		printf("Judge Empty Container success! Container is not empty\n");
	else
	{
		printf("Judge Empty Container error! rtn = %d\n",rtn);
		Sleep(3000);
		return rtn;
	}

	//ECC加解密运算
	memset(out,0,sizeof(out));
	ulLen = sizeof(out);
	rtn = USBKEY_ECCEncrypt(hDeviceHandle,pubKeyID,data,32,out,&ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_ECCEncrypt error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("ECC Encrypt success!\n");

	memset(temp,0,sizeof(temp));
	rtn = USBKEY_ECCDecrypt(hDeviceHandle,priKeyID,out,ulLen,temp,&ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_ECCDecrypt error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("ECC Decrypt success!\n");

	if(32 == ulLen && 0 == memcmp(temp,data,ulLen))
		printf("ECC KeyPair Operation success(1024 bytes),the data and the decrpyt data same\n");
	else
	{
		printf("ECC KeyPair Operation error(1024 bytes)! the data and the decrpyt data is different\n");
		Sleep(3000);
		return -1;
	}

	//导入对称密钥
	memset(temp,0,sizeof(temp));
	temp[0] = 0x02;
	memset(temp+1,0xff,11);
	temp[12] = 0x1a;
	temp[13] = 0x04;
	temp[14] = 0x00;
	temp[15] = 0x03;
	rtn = USBKEY_GenRandom(hDeviceHandle,temp+16,16);
	if (rtn !=0)
	{
		printf("USBKEY_GenRandom error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Gen SymmetricKey success!\n");

	//ECC加解密运算
	memset(out,0,sizeof(out));
	ulLen = sizeof(out);
	rtn = USBKEY_ECCEncrypt(hDeviceHandle,pubKeyID,temp,32,out,&ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_ExtECCEncrypt error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("ECC Encrypt success!\n");

	rtn = USBKEY_ImportSymmetricKey(hDeviceHandle,priKeyID,USBKEY_TYPE_ECC,USBKEY_CRYPT_EXPORTABLE|USBKEY_CRYPT_ONDEVICE,ALG_TYPE_SCB2,out,ulLen,phKey);
	if (rtn !=0)
	{
		printf("USBKEY_ImportSymmetricKey error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Import SymmetricKey by ECC protected success!\n");

	//导出对称密钥
	memset(data,0,sizeof(data));
	ulLen = sizeof(data);
	rtn = USBKEY_ExportSymmetricKey(hDeviceHandle,phKey,pubKeyID,USBKEY_TYPE_ECC,data,&ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_ExportSymmetricKey error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Export SymmetricKey by ECC protected success!\n");

	memset(out,0,sizeof(out));
	rtn = USBKEY_ECCDecrypt(hDeviceHandle,priKeyID,data,ulLen,out,&ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_ExtECCDecrypt error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("ECC Decrypt success!\n");

	if( 0 == memcmp(temp+16,out+ulLen-16,16) )
		printf("Export SymmetricKey by ECC success,it is the same with SymmetricKey\n");
	else
	{
		printf("Export SymmetricKey by ECC error! it is different with SymmetricKey\n");
		Sleep(3000);
		return -1;
	}
	
 	rtn = USBKEY_DeleteContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1");
	if (rtn !=0)
	{
		printf("USBKEY_DeleteContainer error! rtn = %#08x\n",rtn);
		return rtn;
	}

	printf("GenUKeyECCPair_Test success!\n");
	return 0;
}

//卡内ECC公私钥对导入操作(USBKEY_GenerateExtRSAKeyPair/)
USBKEY_ULONG ImportUKeyEccPair_Test(USBKEY_HANDLE hDeviceHandle)
{
	unsigned long rtn = 0;
	USBKEY_ULONG ulLen,uloff;

	USBKEY_UCHAR pubKeyID[2],priKeyID[2],phKey[2];
	USBKEY_UCHAR temp[128],out[128];
	USBKEY_ECC_PUBLIC_KEY stEccPubkey,stExpPubkey;
	USBKEY_ECC_PRIVATE_KEY priKeyE;
	USBKEY_ECC_PUBLIC_KEY pubKeyE;
	USBKEY_UCHAR ucEccPubKey[132];
	USBKEY_UCHAR data[32];
	/*USBKEY_UCHAR data1[32] = {95, 124, 77, 5, 23, -122, -53, -91, -121, 120, -5, 36, -10, -97, 69, 99, -49, -30, -75, 82, -12, 105, 74, 13, 116, -95, 58, 9, -76, -105, 70, 55};
	USBKEY_UCHAR data2[32] = {10, -80, 65, 118, -79, 46, -94, -103, 9, -9, 74, 74, -70, 107, 113, -19, -53, -97, -1, 34, 48, 8, -20, -120, 112, 25, -53, -16, 88, 42, -35, -10};
	USBKEY_UCHAR data[32] = {0xd3,0xd2,0xcd,0xf0,0xb7,0xb7,0x5b,0xfa,0x3c,0x83,0xfb,0xfd,0x0a,0xef,0x12,0x8a,0xa1,0x4e,0x62,0x01,0x8e,0x14,0xc7,0x62,0x70,0x24,0x11,0x09,0x9b,0x2f,0x01,0xfa};*/


	//产生临时ECC密钥
	rtn = USBKEY_GenerateExtECCKeyPair(hDeviceHandle,256,&pubKeyE,&priKeyE);
	if (rtn !=0)
	{
		printf("USBKEY_GenerateExtECCKeyPair error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Generate Ext ECCKeyPair success!\n");

	/*pubKeyE.bits = 256;
	AK_Reverser(&pubKeyE.bits,4);
	memcpy(pubKeyE.XCoordinate,data1,32);
	memcpy(pubKeyE.YCoordinate,data2,32);*/
	AK_Reverser(&priKeyE.bits,4);
	//导入公私钥对
	rtn = USBKEY_CreateContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1");
	if (rtn !=0)
	{
		printf("USBKEY_CreateContainer testContainer1 error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Create Containers success!\n");

	rtn = USBKEY_ImportECCPrivateKey(hDeviceHandle,NULL,USBKEY_ECB,NULL,0,(USBKEY_UCHAR_PTR)"testContainer1",KEY_EXCHANGE,(USBKEY_UCHAR_PTR)&priKeyE,256/8+4,priKeyID);
	if (rtn !=0)
	{
		printf("USBKEY_ImportECCPrivateKey error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Import ECC Private Key success!\n");

	AK_Reverser(&pubKeyE.bits,4);
	memcpy(ucEccPubKey,&pubKeyE.bits,sizeof(pubKeyE.bits));
	uloff = sizeof(pubKeyE.bits);
	memcpy(ucEccPubKey+uloff,pubKeyE.XCoordinate,256/8);
	uloff += 256/8;
	memcpy(ucEccPubKey+uloff,pubKeyE.YCoordinate,256/8);
	uloff += 256/8;

	rtn = USBKEY_ImportECCPublicKey(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1",KEY_EXCHANGE,ucEccPubKey,uloff,pubKeyID);
	if (rtn !=0)
	{
		printf("USBKEY_ImportECCPublicKey error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Import ECC Public Key success!\n");

	rtn = USBKEY_ExportECCPublicKey(hDeviceHandle,pubKeyID,&stExpPubkey);
	if (rtn !=0)
	{
		printf("USBKEY_ExportECCPublicKey error! rtn = %#08x\n",rtn);
		return rtn;
	}

	/*rtn = USBKEY_ECCEncrypt(hDeviceHandle,pubKeyID,data,32,out,&ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_ExtECCEncrypt error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("ECC Encrypt success!\n");*/

	//ECC签名验签运算
	memset(data,0,sizeof(data));
	memset(out,0,sizeof(out));
	ulLen = sizeof(out);
	rtn = USBKEY_GenRandom(hDeviceHandle,data,32);
	if (rtn !=0)
	{
		printf("USBKEY_GenRandom error! rtn = %#08x\n",rtn);
		return rtn;
	}

	rtn = USBKEY_ECCSign(hDeviceHandle,priKeyID,data,32,out,&ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_ECCSign error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("ECC Sign Operation success!\n");
	
	rtn = USBKEY_ECCVerify(hDeviceHandle,pubKeyID,data,32,out,ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_ECCVerify error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("ECC Verify Operation success!\n");

	rtn = USBKEY_DeleteContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1");
	if (rtn !=0)
	{
		printf("USBKEY_DeleteContainer error! rtn = %#08x\n",rtn);
		return rtn;
	}

	printf("ImportUKeyEccPair_Test success!\n");
	return 0;

}

//临时ECC公私钥对操作(USBKEY_GenerateExtRSAKeyPair/USBKEY_TempRSAPublicKeyOperation/USBKEY_TempRSAPrivateKeyOperation)
USBKEY_ULONG TempKeyECCPairOpt_Test(USBKEY_HANDLE hDeviceHandle)
{
	unsigned long rtn = 0;
	USBKEY_ULONG ulLen;

	USBKEY_UCHAR temp[128],out[128];
	USBKEY_ECC_PRIVATE_KEY priKeyE;
	USBKEY_ECC_PUBLIC_KEY pubKeyE;
	USBKEY_UCHAR data[32];
	/*USBKEY_CHAR data1[32] = {95, 124, 77, 5, 23, -122, -53, -91, -121, 120, -5, 36, -10, -97, 69, 99, -49, -30, -75, 82, -12, 105, 74, 13, 116, -95, 58, 9, -76, -105, 70, 55};
	USBKEY_CHAR data2[32] = {10, -80, 65, 118, -79, 46, -94, -103, 9, -9, 74, 74, -70, 107, 113, -19, -53, -97, -1, 34, 48, 8, -20, -120, 112, 25, -53, -16, 88, 42, -35, -10};
	USBKEY_CHAR data[32] = {0xd3,0xd2,0xcd,0xf0,0xb7,0xb7,0x5b,0xfa,0x3c,0x83,0xfb,0xfd,0x0a,0xef,0x12,0x8a,0xa1,0x4e,0x62,0x01,0x8e,0x14,0xc7,0x62,0x70,0x24,0x11,0x09,0x9b,0x2f,0x01,0xfa};*/

	//产生临时ECC密钥
	rtn = USBKEY_GenerateExtECCKeyPair(hDeviceHandle,256,&pubKeyE,&priKeyE);
	if (rtn !=0)
	{
		printf("USBKEY_GenerateExtECCKeyPair error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Generate Ext ECCKeyPair success!\n");
	/*memset(&pubKeyE,0,sizeof(USBKEY_ECC_PUBLIC_KEY));
	pubKeyE.bits = 256;
	memcpy(pubKeyE.XCoordinate,data1,32);
	memcpy(pubKeyE.YCoordinate,data2,32);*/

	//临时ECC加密
	memset(data,0,sizeof(data));
	memset(out,0,sizeof(out));
	ulLen = sizeof(out);
	rtn = USBKEY_GenRandom(hDeviceHandle,data,32);
	if (rtn !=0)
	{
		printf("USBKEY_GenRandom error! rtn = %#08x\n",rtn);
		return rtn;
	}

	rtn = USBKEY_ExtECCEncrypt(hDeviceHandle,&pubKeyE,(USBKEY_UCHAR_PTR)data,32,(USBKEY_UCHAR_PTR)out,&ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_ExtECCEncrypt error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Ext ECC Encrypt success!\n");

	memset(temp,0,sizeof(temp));
	rtn = USBKEY_ExtECCDecrypt(hDeviceHandle,&priKeyE,out,ulLen,temp,&ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_ExtECCDecrypt error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Ext ECC Decrypt success!\n");
	
	if(32 == ulLen && 0 == memcmp(temp,data,ulLen))
		printf("Ext ECC KeyPair Operation success,the data and the decrpyt data same\n");
	else
	{
		printf("Ext ECC KeyPair Operation error! the data and the decrpyt data is different\n");
		Sleep(3000);
		return -1;
	}

	//临时ECC签名
	memset(data,0,sizeof(data));
	memset(out,0,sizeof(out));
	ulLen = sizeof(out);
	rtn = USBKEY_GenRandom(hDeviceHandle,data,32);
	if (rtn !=0)
	{
		printf("USBKEY_GenRandom error! rtn = %#08x\n",rtn);
		return rtn;
	}

	rtn = USBKEY_ExtECCSign(hDeviceHandle,&priKeyE,(USBKEY_UCHAR_PTR)data,32,out,&ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_ExtECCSign error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Ext ECC Sign success!\n");

	rtn = USBKEY_ExtECCVerify(hDeviceHandle,&pubKeyE,(USBKEY_UCHAR_PTR)data,32,out,ulLen);
	if (rtn !=0)
	{
		printf("USBKEY_TempRSAPublicKeyOperation error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("Ext ECC Verify success!\n");

	printf("TempKeyECCPairOpt_Test success!\n");
	return 0;
}
