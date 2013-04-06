#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "./UsbKeyAPI.h"
#include "./UtilTest.h"

int _tmain(int argc, _TCHAR* argv[])
{
	unsigned long rtn = 0;				// the return of function

	USBKEY_HANDLE hDeviceHandle;		// the handle of usb
	USBKEY_CHAR ppcDeviceIdS[10][260];	// the name of device
	int pucDeviceQuantity = 0;			// the number of devices

	USBKEY_ULONG ulReNum = 0;			
	USBKEY_ULONG ulLen = 0;
	USBKEY_ULONG ulNum = 0;

	USBKEY_UCHAR AppNameList[256];
	USBKEY_UCHAR NameList[256];
	USBKEY_UCHAR_PTR pNamelist;
	USBKEY_RSA_KEY rsakey;

	USBKEY_UCHAR testdata[] = {48, -126, 2, 119, 2, 1, 0, 48, 13, 6, 9, 42, -122, 72, -122, -9, 13, 1, 1, 1, 5, 0, 4, -126, 2, 97, 48, -126, 2, 93, 2, 1, 0, 2, -127, -127, 0, -78, -101, 65, -90, -78, -105, 93, -107, -80, -51, -79, -9, -38, -114, -106, 18, -20, -99, -28, -113, 112, 76, 121, -29, 65, 29, 97, -4, 49, -37, 65, -64, 66, 4, -63, 91, 16, -52, 13, -113, -97, 18, -102, -1, -19, -40, 10, -89, -50, 99, 115, -6, -24, 126, 70, 90, -84, -48, -44, -14, 118, 16, -42, -30, 102, 38, 0, -118, -85, -79, -64, -18, 103, 62, -3, -70, 85, 21, -58, -102, -44, -60, -15, -109, 95, -127, 3, 45, 79, 90, 62, 78, 105, 39, 91, 63, 97, -15, -7, 53, 42, -19, -48, 105, 114, 69, 23, -40, -22, -121, -118, 8, -92, 81, 27, -112, 83, 68, -45, -65, -62, 80, -81, 61, -12, 58, -122, -85, 2, 3, 1, 0, 1, 2, -127, -128, 87, -106, 113, -60, -104, -14, 71, -17, -97, -85, -49, -23, -41, 97, 115, 81, 70, 126, 75, -42, -41, -32, 121, 41, -84, 6, 7, -58, 80, -128, -48, -90, -62, -7, 104, 5, 11, -96, -75, -55, -57, -35, -84, -21, 115, 45, 46, 84, -118, -113, 21, 18, 103, -81, 125, 58, -4, 98, -48, -5, 50, 81, 33, -65, 68, -70, 41, 54, -111, 120, -111, -60, 73, -52, -116, 103, -29, -28, -116, -112, 40, -69, 59, 52, 26, 12, 27, 23, -27, 32, 67, -17, -52, 55, -123, 44, -128, 18, -123, 59, 37, -102, 72, -112, 106, -19, -82, 119, -39, -101, -29, 90, -56, -80, -3, 119, -111, 71, 56, -57, 81, -43, 77, -5, 27, -74, -99, 1, 2, 65, 0, -26, -77, 116, -53, 95, -107, -20, -91, -49, -4, -127, 117, -6, 32, -55, -29, -102, -33, 75, 120, -44, -97, 43, -21, 116, 12, 36, 38, -37, 112, -81, 1, 100, 31, -11, -67, 109, 126, 1, 125, -120, -80, 50, -41, 40, 76, 95, -59, -119, 4, 19, 104, 41, 14, 51, 2, -60, 10, 94, -7, 86, -8, -30, 43, 2, 65, 0, -58, 49, 85, 104, -47, -56, 112, 4, 39, -43, 65, 22, -85, -35, -77, 95, 101, -74, 44, -14, -127, -92, 3, 37, -121, -86, -57, 115, 2, 50, 113, -82, 44, -103, -38, -27, 6, -49, -126, -32, -44, 33, 72, 21, -111, 107, -43, 127, 56, -99, -119, 114, 99, 118, 95, -31, 48, -19, -4, -104, -98, -83, 45, -127, 2, 64, 38, 15, 93, 41, 96, 47, -97, -57, 11, 53, 33, 54, -111, 121, 111, 85, 57, 107, -75, 75, -51, -24, 36, -102, -57, -98, 105, 40, 111, -13, 123, 43, -61, -108, -14, 57, -70, 34, -101, -92, 86, 52, -101, 110, -101, -101, 94, 1, 124, -59, 75, 94, 87, -57, -101, -22, -52, -28, -39, 30, 81, -106, -27, -39, 2, 65, 0, -77, 25, -15, -20, -1, 102, -6, -109, 70, -53, -40, 61, -4, -94, 36, 110, -103, 35, -64, 10, -122, 77, 8, -22, -23, 102, -45, 70, -125, 61, -62, -83, 123, 112, -83, -9, 57, -24, 92, -123, -128, 45, 115, 78, -54, 83, -128, 99, -81, 109, -19, -65, -119, 82, 119, -30, -80, -84, -32, 15, -85, -107, 36, 1, 2, 65, 0, -51, -102, -33, 26, 102, -63, 104, 8, 3, -74, 97, -76, 61, 42, 61, 91, -57, 75, 35, 116, -14, -36, -23, -101, -69, 108, -78, 47, -4, 47, -3, 77, -88, -61, -68, -88, 69, 108, -103, -33, 48, 59, -27, 42, 117, 46, 84, -108, -91, 111, 107, 81, -100, -73, -117, -85, 85, 125, 6, -79, 76, 108, -101, -30};
	AK_Hexdump(testdata, sizeof(testdata));

	printf("--UKey Test Begin\n");

	// 枚举设备
	rtn = USBKEY_DeviceEnum(ppcDeviceIdS, &pucDeviceQuantity);
	if (rtn !=0)
	{
		printf("USBKEY_DeviceEnum error! rtn = %#08x\n", rtn);
		return rtn;
	}
	printf("USBKEY_DeviceEnum success! the num of ukey is %d\n", pucDeviceQuantity);

	// 打开设备
	rtn = USBKEY_DeviceOpen(ppcDeviceIdS[0], 1, &hDeviceHandle);
 	if (rtn !=0)
	{
		printf("USBKEY_DeviceOpen error! rtn = %#08x\n", rtn);
		return rtn;
	}
	printf("==========================USBKEY_DeviceOpen success!==========================\n");

	// 初始化设备
	rtn = InitUKey_Test(hDeviceHandle);
	if (rtn !=0)
	{
		printf("Init UKey Test error! rtn = %#08x\n", rtn);
		return rtn;
	}
	printf("============================InitUKey_Test success!============================\n");
 	
	/***************************************************************
	//外部认证
	rtn =  AuthenticateUKey_Test(hDeviceHandle);
	if (rtn !=0)
	{
		printf("AuthenticateUKey_Test error! rtn = %#08x\n",rtn);
		return rtn;
	}
	****************************************************************/
	//
	//获得key信息
	rtn = GetUKeyInfo_Test(hDeviceHandle);
	if (rtn !=0)
	{
		printf("GetUKeyInfo_Test error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("=========================Get UKey Info Test success!==========================\n");

	//枚举应用
	rtn = USBKEY_EnumApplication(hDeviceHandle,NULL,&ulLen,&ulNum);
	if (rtn !=0)
	{
		printf("USBKEY_EnumApplication error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("USBKEY_EnumApplication success! the ulLen is %d,ulNum is %d\n",ulLen,ulNum);

	memset(AppNameList,0,sizeof(AppNameList));
	rtn = USBKEY_EnumApplication(hDeviceHandle,AppNameList,&ulLen,&ulNum);
	if (rtn !=0)
	{
		printf("USBKEY_EnumApplication error! rtn = %#08x\n",rtn);
		return rtn;
	}

	//打开应用
  	rtn = USBKEY_OpenApplication(hDeviceHandle,AppNameList);
	if (rtn !=0)
	{
		printf("USBKEY_OpenApplication error! rtn = %#08x\n",rtn);
		return rtn;
	}
	
	//Pin码操作
	rtn = OptUKeyPIN_Test(hDeviceHandle);
	if (rtn !=0)
	{
		printf("OptUKeyPIN_Test error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("=======================UKey PIN Operation Test success!=======================\n");

	//check pin
	rtn = USBKEY_CheckPin(hDeviceHandle,USBKEY_USERTYPE_USER,(USBKEY_UCHAR_PTR)"123456",6,&ulReNum);
	if (rtn !=0)
	{
		printf("USBKEY_CheckPin error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("==============================Check PIN success!==============================\n");
	
	/*rtn = USBKEY_GetAsymmetricKeyID(hDeviceHandle,pNamelist,KEY_SIGNATURE,(USBKEY_UCHAR_PTR)&rsakey);
	if (rtn !=0)
	{
		printf("USBKEY_GetAsymmetricKeyID error! rtn = %#08x\n",rtn);
		return rtn;
	}*/

	rtn = FileOpt_Test(hDeviceHandle);
	if (rtn !=0)
	{
		printf("FileOpt_Test error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("=========================File Operation Test success!=========================\n");

	//RSA公私钥对生成及容器操作
	rtn = GenUKeyRSAPair_Test(hDeviceHandle);
	if (rtn !=0)
	{
		printf("GenUKeyRSAPair_Test error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("========================Gen UKey RSAPair Test success!========================\n");

	//导入RSA公私钥对
	rtn = ImportUKeyRSAPair_Test(hDeviceHandle);
	if (rtn !=0)
	{
		printf("ImportUKeyRSAPair_Test error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("======================Import UKey RSAPair Test success!=======================\n");

	//临时RSA公私钥对操作
	rtn = TempKeyRSAPairOpt_Test(hDeviceHandle);
	if (rtn !=0)
	{
		printf("TempKeyRSAPairOpt_Test error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("===================Temp RSAPair Key Operation Test success!===================\n");

	//对称密钥及容器对称密钥操作
	rtn = UKeySymmetricKeyOpt_Test(hDeviceHandle);
	if (rtn !=0)
	{
		printf("UKeySymmetricKeyOpt_Test error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("==================UKey Symmetric Key Operation Test success!==================\n");

	//临时对称密钥操作
	rtn = TempSymmetricKeyOpt_Test(hDeviceHandle);
	if (rtn !=0)
	{
		printf("TempSymmetricKeyOpt_Test error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("==================Temp Symmetric Key Operation Test success!==================\n");

	//证书操作
	rtn = UKeyCertOpt_Test(hDeviceHandle);
	if (rtn !=0)
	{
		printf("UKeyCertOpt_Test error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("=======================UKey Cert Operation Test success!======================\n");

	//卡内ECC操作
	rtn = GenUKeyECCPair_Test(hDeviceHandle);
	if (rtn !=0)
	{
		printf("UKeyCertOpt_Test error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("===================UKey ECCPair Key Operation Test success!===================\n");

	//导入ECC操作
	rtn = ImportUKeyEccPair_Test(hDeviceHandle);
	if (rtn !=0)
	{
		printf("UKeyCertOpt_Test error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("===================UKey ECCPair Key Import Operation Test success!===================\n");


	//临时ECC操作
	rtn = TempKeyECCPairOpt_Test(hDeviceHandle);
	if (rtn !=0)
	{
		printf("UKeyCertOpt_Test error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("===================Temp ECCPair Key Operation Test success!===================\n");

	rtn = USBKEY_DeviceClose(hDeviceHandle);
	if (rtn !=0)
	{
		printf("USBKEY_DeviceClose error! rtn = %#08x\n",rtn);
		return rtn;
	}
//}

	printf("#############################UKey Test End Success!#############################\n");
	return rtn;
}