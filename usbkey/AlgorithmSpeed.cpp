#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>

#include "./UsbKeyAPI.h"


int Tes_ReadFile(char *FileName, unsigned char *pFileValue,  unsigned long *pulFileLen)
{
	FILE *fp;
	unsigned long ulSize;

	if( (fp = fopen(FileName, "rb+")) == NULL)
	{
		printf("Tes_ReadFile:FileName not open! CODE: 0x%08x", 1);
		return 1;
	}

	fseek(fp, 0, SEEK_END);
	ulSize = ftell(fp);
	if(!pFileValue)
	{
		fclose(fp);
		*pulFileLen = ulSize;
		return 0;
	}

	if( *pulFileLen < ulSize)
	{
		*pulFileLen = ulSize;
		fclose(fp);
		printf("Tes_ReadFile: pulFileLen not enough! CODE: 0x%08x", 2);
		return 2;
	}

	fseek(fp, 0, SEEK_SET);
	if(fread(pFileValue,1,ulSize,fp) != ulSize)
	{
		fclose(fp);
		printf("Tes_ReadFile: read file error! CODE: 0x%08x", 3);
		return 3;
	}
	*pulFileLen = ulSize;
	fclose(fp);
	return 0;
}


int Tes_WriteFile(char *FileName, unsigned char *pFileValue, unsigned long ulFileLen)
{
	FILE *fp;

	if( (fp = fopen(FileName, "wb+")) == NULL)
	{
		printf("Tes_WriteFile:FileName not open! CODE: 0x%08x", 1);
		return 1;
	}

	if(fwrite(pFileValue,1,ulFileLen,fp) != ulFileLen)
	{
		fclose(fp);
		printf("Tes_WriteFile:write file error! CODE: 0x%08x", 2);
		return 2;
	}

	fclose(fp);
	return 0;
}

//����RSA�ӽ����ٶ�
USBKEY_ULONG RSAUKeyPair_Test(USBKEY_HANDLE hDeviceHandle)
{
	unsigned long rtn = 0,i;
	USBKEY_ULONG ulLen;

	USBKEY_UCHAR pubKeyID[2],priKeyID[2];
	USBKEY_UCHAR data[1024],temp[1024],out[1024];

	clock_t start, finish;
	double   duration;
	int sp;

	rtn = USBKEY_CreateContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1");
	if (rtn !=0)
	{
		printf("USBKEY_CreateContainer testContainer1 error! rtn = %#08x\n",rtn);
		return rtn;
	}

	start = clock();
	for(i = 0; i < 1; i++)
	{
		rtn = USBKEY_GenerateRSAKeyPair(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1",KEY_EXCHANGE,1024,USBKEY_CRYPT_USER_PROTECTED,pubKeyID,priKeyID);
		if (rtn !=0)
		{
			printf("USBKEY_GenerateRSAKeyPair error! rtn = %#08x\n",rtn);
			return rtn;
		}
	}
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	printf("1024λRSA��˽Կ�������ٶ�Ϊ%.3f��/��\n",duration);

	//��˽Կ����
	ulLen = 256;
	memset(data,0,sizeof(data));
	memset(out,0,sizeof(out));
	ulLen = sizeof(out);
	rtn = USBKEY_GenRandom(hDeviceHandle,data,256);
	if (rtn !=0)
	{
		printf("USBKEY_GenRandom error! rtn = %#08x\n",rtn);
		return rtn;
	}
	data[0] = 0;

	memset(temp,0,sizeof(temp));
	start = clock();
	for(i = 0; i < 1; i++)
	{
		rtn = USBKEY_RSAPrivateKeyOperation(hDeviceHandle,priKeyID,data,128,out,&ulLen);
		if (rtn !=0)
		{
			printf("USBKEY_RSAPrivateKeyOperation error! rtn = %#08x\n",rtn);
			return rtn;
		}

		rtn = USBKEY_RSAPublicKeyOperation(hDeviceHandle,pubKeyID,out,ulLen,temp,&ulLen);
		if (rtn !=0)
		{
			printf("USBKEY_RSAPublicKeyOperation error! rtn = %#08x\n",rtn);
			return rtn;
		}

		if(128 == ulLen && 0 == memcmp(temp,data,ulLen))
		{
			printf("1024λRSA��˽Կ����ɹ���ԭ�������������һ��\n");
			//Sleep(1000);
		}
		else
		{
			Tes_WriteFile("2.txt",data,ulLen);
			Tes_WriteFile("2_de.txt",temp,ulLen);
			printf("1024λRSA��˽Կ����ʧ��! ������������ݲ�ͬ\n");
			//Sleep(1000);
			//return -1;
		}
	}
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	sp = 1/duration;
	printf("1024λRSA��Կ�����ٶ�Ϊ%d��/s\n",sp);
		
	/*start = clock();
	for(i = 0; i < 10; i++)
	{
		rtn = USBKEY_RSAPrivateKeyOperation(hDeviceHandle,priKeyID,out,ulLen,temp,&ulLen);
		if (rtn !=0)
		{
			printf("USBKEY_RSAPrivateKeyOperation error! rtn = %#08x\n",rtn);
			return rtn;
		}
	}
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	sp = 10/duration;
	printf("1024λRSA˽Կ�����ٶ�Ϊ%d��/s\n",sp);*/

	//if(128 == ulLen && 0 == memcmp(temp,data,ulLen))
	//{
	//	printf("1024λRSA��˽Կ����ɹ���ԭ�������������һ��\n");
	//	Sleep(1000);
	//}
	//else
	//{
	//	Tes_WriteFile("2.txt",data,ulLen);
	//	Tes_WriteFile("2_de.txt",temp,ulLen);
	//	printf("1024λRSA��˽Կ����ʧ��! ������������ݲ�ͬ\n");
	//	Sleep(1000);
	//	//return -1;
	//}

	rtn = USBKEY_DeleteAsymmetricKey(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1",KEY_EXCHANGE,USBKEY_RSA_PUBLICKEY);
	if (rtn !=0)
	{
		printf("USBKEY_DeleteAsymmetricKey PublicKey error! rtn = %#08x\n",rtn);
		return rtn;
	}

	rtn = USBKEY_DeleteAsymmetricKey(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1",KEY_EXCHANGE,USBKEY_RSA_PRIVATEKEY);
	if (rtn !=0)
	{
		printf("USBKEY_DeleteAsymmetricKey PrivateKey error! rtn = %#08x\n",rtn);
		return rtn;
	}

	rtn = USBKEY_JudgeEmptyContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1");
	if( 0 != rtn)
	{
		printf("Judge Empty Container error! rtn = %d\n",rtn);
		//Sleep(3000);
		return rtn;
	}


	//2048λ
	start = clock();
	for(i = 0; i < 1; i++)
	{
		rtn = USBKEY_GenerateRSAKeyPair(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1",KEY_EXCHANGE,2048,USBKEY_CRYPT_USER_PROTECTED,pubKeyID,priKeyID);
		if (rtn !=0)
		{
			printf("USBKEY_GenerateRSAKeyPair error! rtn = %#08x\n",rtn);
			return rtn;
		}
	}
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	printf("2048λRSA��˽Կ�������ٶ�Ϊ%.3f��/��\n",duration);

	//��˽Կ����
	ulLen = 256;
	memset(data,0,sizeof(data));
	memset(out,0,sizeof(out));
	ulLen = sizeof(out);
	rtn = USBKEY_GenRandom(hDeviceHandle,data,256);
	if (rtn !=0)
	{
		printf("USBKEY_GenRandom error! rtn = %#08x\n",rtn);
		return rtn;
	}
	data[0] = 0;

	memset(temp,0,sizeof(temp));
	start = clock();
	for(i = 0; i < 1; i++)
	{
		rtn = USBKEY_RSAPrivateKeyOperation(hDeviceHandle,priKeyID,data,256,out,&ulLen);
		if (rtn !=0)
		{
			printf("USBKEY_RSAPrivateKeyOperation error! rtn = %#08x\n",rtn);
			return rtn;
		}

		rtn = USBKEY_RSAPublicKeyOperation(hDeviceHandle,pubKeyID,out,ulLen,temp,&ulLen);
		if (rtn !=0)
		{
			printf("USBKEY_RSAPublicKeyOperation error! rtn = %#08x\n",rtn);
			return rtn;
		}
		if(256 == ulLen && 0 == memcmp(temp,data,ulLen))
		{
			printf("2048λRSA��˽Կ����ɹ���ԭ�������������һ��\n");
			//Sleep(1000);
		}
		else
		{
			Tes_WriteFile("2.txt",data,ulLen);
			Tes_WriteFile("2_de.txt",temp,ulLen);
			printf("2048λRSA��˽Կ����ʧ��! ������������ݲ�ͬ\n");
			//Sleep(1000);
			//return -1;
		}
	}
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	sp = 1/duration;
	printf("2048λRSA��Կ�����ٶ�Ϊ%d��/s\n",sp);

	/*start = clock();
	for(i = 0; i < 10; i++)
	{
		rtn = USBKEY_RSAPrivateKeyOperation(hDeviceHandle,priKeyID,out,ulLen,temp,&ulLen);
		if (rtn !=0)
		{
			printf("USBKEY_RSAPrivateKeyOperation error! rtn = %#08x\n",rtn);
			return rtn;
		}
	}
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	sp = 10/duration;
	printf("2048λRSA˽Կ�����ٶ�Ϊ%d��/s\n",sp);*/


	//if(256 == ulLen && 0 == memcmp(temp,data,ulLen))
	//{
	//	printf("2048λRSA��˽Կ����ɹ���ԭ�������������һ��\n");
	//	Sleep(1000);
	//}
	//else
	//{
	//	Tes_WriteFile("2.txt",data,ulLen);
	//	Tes_WriteFile("2_de.txt",temp,ulLen);
	//	printf("2048λRSA��˽Կ����ʧ��! ������������ݲ�ͬ\n");
	//	Sleep(1000);
	//	//return -1;
	//}

 	rtn = USBKEY_DeleteContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1");
	if (rtn !=0)
	{
		printf("USBKEY_DeleteContainer error! rtn = %#08x\n",rtn);
		return rtn;
	}

	return 0;
}


//����ECC�ӽ����ٶ�
USBKEY_ULONG ECCUKeyPair_Test(USBKEY_HANDLE hDeviceHandle)
{
	unsigned long rtn = 0,i;
	USBKEY_ULONG ulLen,ulLen1;

	USBKEY_UCHAR pubKeyID[2],priKeyID[2];
	USBKEY_UCHAR data[128],temp[128],out[128];

	clock_t start, finish;
	double   duration;
	int sp;

	/*rtn = USBKEY_CreateContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1");
	if (rtn !=0)
	{
		printf("USBKEY_CreateContainer testContainer1 error! rtn = %#08x\n",rtn);
		return rtn;
	}

	start = clock();
	for(i = 0; i < 1; i++)
	{
		rtn = USBKEY_GenerateECCKeyPair(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1",KEY_EXCHANGE,256,USBKEY_CRYPT_EXPORTABLE,pubKeyID,priKeyID,NULL);
		if (rtn !=0)
		{
			printf("USBKEY_GenerateECCKeyPair error! rtn = %#08x\n",rtn);
			return rtn;
		}
	}
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	printf("SM2��˽Կ�������ٶ�Ϊ%.3f��/��\n",duration);*/

	USBKEY_RSA_KEY rsakey;
	rtn = USBKEY_GetAsymmetricKeyID(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1",KEY_EXCHANGE,(USBKEY_UCHAR_PTR)&rsakey);
	if (rtn !=0)
	{
		printf("USBKEY_GetAsymmetricKeyID error! rtn = %#08x\n",rtn);
		return rtn;
	}
	
	//ECCǩ����ǩ����
	memset(data,0,sizeof(data));
	memset(out,0,sizeof(out));
	ulLen = sizeof(out);
	rtn = USBKEY_GenRandom(hDeviceHandle,data,32);
	if (rtn !=0)
	{
		printf("USBKEY_GenRandom error! rtn = %#08x\n",rtn);
		return rtn;
	}

	start = clock();
	for(i = 0; i < 10; i++)
	{
		rtn = USBKEY_ECCSign(hDeviceHandle,rsakey.privateKeyID,data,32,out,&ulLen);
		if (rtn !=0)
		{
			printf("USBKEY_ECCSign error! rtn = %#08x\n",rtn);
			return rtn;
		}
	}
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	sp = 10/duration;
	printf("SM2�㷨ǩ���ٶ�Ϊ%d��/s\n",sp);
	
	start = clock();
	for(i = 0; i < 10; i++)
	{
		rtn = USBKEY_ECCVerify(hDeviceHandle,rsakey.publicKeyID,data,32,out,ulLen);
		if (rtn !=0)
		{
			printf("USBKEY_ECCVerify error! rtn = %#08x\n",rtn);
			return rtn;
		}
	}
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	sp = 10/duration;
	printf("SM2�㷨��ǩ�ٶ�Ϊ%d��/s\n",sp);

	//ECC�ӽ�������
	memset(out,0,sizeof(out));
	memset(temp,0,sizeof(temp));
	ulLen = sizeof(out);
	ulLen1 = sizeof(out);
	start = clock();
	for(i = 0; i < 10; i++)
	{
		rtn = USBKEY_ECCEncrypt(hDeviceHandle,pubKeyID,data,32,out,&ulLen);
		if (rtn !=0)
		{
			printf("USBKEY_ECCEncrypt error! rtn = %#08x\n",rtn);
			return rtn;
		}
	}
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	sp = 10/duration;
	printf("SM2�㷨�����ٶ�Ϊ%d��/s\n",sp);

	start = clock();
	for(i = 0; i < 10; i++)
	{
		rtn = USBKEY_ECCDecrypt(hDeviceHandle,priKeyID,out,ulLen,temp,&ulLen1);
		if (rtn !=0)
		{
			printf("USBKEY_ECCDecrypt error! rtn = %#08x\n",rtn);
			return rtn;
		}
	}
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	sp = 10/duration;
	printf("SM2�㷨�����ٶ�Ϊ%d��/s\n",sp);

	if(32 == ulLen1 && 0 == memcmp(temp,data,ulLen1))
		printf("SM2�㷨�ӽ�������ɹ���ԭ�������������һ��\n");
	else
	{
		printf("SM2�㷨�ӽ�������ʧ�ܣ�������������ݲ�ͬ\n");
		Sleep(3000);
		return -1;
	}

 	rtn = USBKEY_DeleteContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1");
	if (rtn !=0)
	{
		printf("USBKEY_DeleteContainer error! rtn = %#08x\n",rtn);
		return rtn;
	}

	return 0;
}


USBKEY_ULONG UKeySymmKeyCBCOpt_Test(USBKEY_HANDLE hDeviceHandle)
{
	unsigned long rtn = 0;
	USBKEY_ULONG ulLen,ulEncryptedLen;
	USBKEY_ULONG ulFileLen;

	USBKEY_UCHAR_PTR pucData,pucTemp,pucOut;

	//USBKEY_UCHAR data[1024],temp[1024],out[1024];
	USBKEY_UCHAR phKey[2];
	USBKEY_UCHAR pcIV[16];

	clock_t start, finish;
	double dsize;
	double   duration;

	
	rtn = Tes_ReadFile("1.txt",NULL,&ulFileLen);
	if (rtn !=0)
	{
		printf("���ļ�1.txtʧ�ܣ���ȷ������Ŀ¼���Ƿ���ڴ��ļ���rtn = %#08x\n",rtn);
		return rtn;
	}
	pucData = (unsigned char *)malloc(ulFileLen);
	rtn = Tes_ReadFile("1.txt",pucData,&ulFileLen);
	if(0!= rtn)
	{
		printf("Tes_ReadFile error! CODE: 0x%08x", rtn);
		return rtn;
	}
	pucTemp = (unsigned char *)malloc(ulFileLen+24);
	pucOut = (unsigned char *)malloc(ulFileLen+24);
	ulEncryptedLen = ulFileLen+24;
	memset(pucOut,0,ulEncryptedLen);
	ulLen = ulFileLen+24;
	memset(pucTemp,0,ulLen);

	//============================SCB2�ٶȲ���================================
	rtn = USBKEY_GenSymmetricKey(hDeviceHandle,16,ALG_TYPE_SCB2,USBKEY_CRYPT_EXPORTABLE|USBKEY_CRYPT_ONDEVICE|USBKEY_CRYPT_USER_PROTECTED,phKey);
	if (rtn !=0)
	{
		printf("USBKEY_GenSymmetricKey error! rtn = %#08x\n",rtn);
		return rtn;
	}


	//�Գ���Կ�ӽ���
	//�������ݼ���
	memset(pcIV, 0x3, 16);

	start = clock();
	rtn = USBKEY_MassiveDataEncrypt(
				hDeviceHandle,					//[in]�豸���
				phKey,							//[in]�������ܵ���ԿID
				ALG_TYPE_SCB2,					//[in]�����㷨
				USBKEY_CBC|USBKEY_PKCS5_PADDING,	//[in]����ģʽ�����ģʽ
				pcIV,							//[in]��ʼ�������������ecbģʽ������ΪNULL
				pucData,							//[in]����������
				ulFileLen,							//[in]���������ݳ���
				pucOut,							//[out]���صļ��ܺ�����
				&ulEncryptedLen);//[in/out]in: pcEncryptedData��������С��out: ���ܺ����ݳ��Ȼ���Ҫ��pcEncryptedData��������С
	if (rtn !=0)
	{
		printf("USBKEY_MassiveDataEncrypt error! rtn = %#08x\n",rtn);
		return rtn;
	}
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	dsize = 16.0/duration;
	printf("SCB2�㷨�����ٶ�Ϊ%.3fMbps\n",dsize);


	//�������ݽ���
	start = clock();
	rtn = USBKEY_MassiveDataDecrypt(
				hDeviceHandle,						//[in]�豸���
				phKey,							//[in]�������ܵ���ԿID
				ALG_TYPE_SCB2,					//[in]�����㷨
				USBKEY_CBC|USBKEY_PKCS5_PADDING,	//[in]����ģʽ�����ģʽ
				pcIV,								//[in]��ʼ�������������ecbģʽ������ΪNULL
				pucOut,								//[in]��������
				ulEncryptedLen,						//[in]�������ݳ���
				pucTemp,								//[out]���ص�����
				&ulLen);								//[in/out]in: pcData���������� out:���ص����ĳ���
	if (rtn !=0)
	{
		printf("USBKEY_MassiveDataDecrypt error! rtn = %#08x\n",rtn);
		return rtn;
	}
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	dsize = 16.0/duration;
	printf("SCB2�㷨�����ٶ�Ϊ%.3fMbps\n",dsize);

	if(ulFileLen == ulLen && 0 == memcmp(pucTemp,pucData,ulLen))
		printf("SCB2�㷨�ӽ�������ɹ���ԭ�������������һ��\n");
	else
	{
		printf("SCB2�㷨�ӽ�������ʧ�ܣ�������������ݲ�ͬ\n");
		Sleep(3000);
		return -1;
	}


	//============================SSF33�ٶȲ���================================
	rtn = USBKEY_GenSymmetricKey(hDeviceHandle,16,ALG_TYPE_SSF33,USBKEY_CRYPT_EXPORTABLE|USBKEY_CRYPT_ONDEVICE|USBKEY_CRYPT_USER_PROTECTED,phKey);
	if (rtn !=0)
	{
		printf("USBKEY_GenSymmetricKey error! rtn = %#08x\n",rtn);
		return rtn;
	}

	ulEncryptedLen = ulFileLen+24;
	memset(pucOut,0,ulEncryptedLen);
	ulLen = ulFileLen+24;
	memset(pucTemp,0,ulLen);
	
	//�Գ���Կ�ӽ���
	//�������ݼ���
	memset(pcIV, 0x3, 16);

	start = clock();
	rtn = USBKEY_MassiveDataEncrypt(
				hDeviceHandle,					//[in]�豸���
				phKey,							//[in]�������ܵ���ԿID
				ALG_TYPE_SSF33,					//[in]�����㷨
				USBKEY_CBC|USBKEY_PKCS5_PADDING,	//[in]����ģʽ�����ģʽ
				pcIV,							//[in]��ʼ�������������ecbģʽ������ΪNULL
				pucData,							//[in]����������
				ulFileLen,							//[in]���������ݳ���
				pucOut,							//[out]���صļ��ܺ�����
				&ulEncryptedLen);//[in/out]in: pcEncryptedData��������С��out: ���ܺ����ݳ��Ȼ���Ҫ��pcEncryptedData��������С
	if (rtn !=0)
	{
		printf("USBKEY_MassiveDataEncrypt error! rtn = %#08x\n",rtn);
		return rtn;
	}
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	dsize = 16.0/duration;
	printf("SSF33�㷨�����ٶ�Ϊ%.3fMbps\n",dsize);


	//�������ݽ���
	start = clock();
	rtn = USBKEY_MassiveDataDecrypt(
				hDeviceHandle,						//[in]�豸���
				phKey,							//[in]�������ܵ���ԿID
				ALG_TYPE_SSF33,					//[in]�����㷨
				USBKEY_CBC|USBKEY_PKCS5_PADDING,	//[in]����ģʽ�����ģʽ
				pcIV,								//[in]��ʼ�������������ecbģʽ������ΪNULL
				pucOut,								//[in]��������
				ulEncryptedLen,						//[in]�������ݳ���
				pucTemp,								//[out]���ص�����
				&ulLen);								//[in/out]in: pcData���������� out:���ص����ĳ���
	if (rtn !=0)
	{
		printf("USBKEY_MassiveDataDecrypt error! rtn = %#08x\n",rtn);
		return rtn;
	}
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	dsize = 16.0/duration;
	printf("SSF33�㷨�����ٶ�Ϊ%.3fMbps\n",dsize);

	if(ulFileLen == ulLen && 0 == memcmp(pucTemp,pucData,ulLen))
		printf("SSF33�㷨�ӽ�������ɹ���ԭ�������������һ��\n");
	else
	{
		printf("SSF33�㷨�ӽ�������ʧ�ܣ�������������ݲ�ͬ\n");
		Sleep(3000);
		return -1;
	}

	//============================3DES�ٶȲ���================================
	rtn = USBKEY_GenSymmetricKey(hDeviceHandle,24,ALG_TYPE_3DES_3KEY,USBKEY_CRYPT_EXPORTABLE|USBKEY_CRYPT_ONDEVICE|USBKEY_CRYPT_USER_PROTECTED,phKey);
	if (rtn !=0)
	{
		printf("USBKEY_GenSymmetricKey error! rtn = %#08x\n",rtn);
		return rtn;
	}

	ulEncryptedLen = ulFileLen+24;
	memset(pucOut,0,ulEncryptedLen);
	ulLen = ulFileLen+24;
	memset(pucTemp,0,ulLen);
	
	//�Գ���Կ�ӽ���
	//�������ݼ���
	memset(pcIV, 0x3, 8);

	start = clock();
	rtn = USBKEY_MassiveDataEncrypt(
				hDeviceHandle,					//[in]�豸���
				phKey,							//[in]�������ܵ���ԿID
				ALG_TYPE_3DES_3KEY,					//[in]�����㷨
				USBKEY_CBC|USBKEY_PKCS5_PADDING,	//[in]����ģʽ�����ģʽ
				pcIV,							//[in]��ʼ�������������ecbģʽ������ΪNULL
				pucData,							//[in]����������
				ulFileLen,							//[in]���������ݳ���
				pucOut,							//[out]���صļ��ܺ�����
				&ulEncryptedLen);//[in/out]in: pcEncryptedData��������С��out: ���ܺ����ݳ��Ȼ���Ҫ��pcEncryptedData��������С
	if (rtn !=0)
	{
		printf("USBKEY_MassiveDataEncrypt error! rtn = %#08x\n",rtn);
		return rtn;
	}
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	dsize = 16.0/duration;
	printf("3DES�㷨�����ٶ�Ϊ%.3fMbps\n",dsize);


	//�������ݽ���
	start = clock();
	rtn = USBKEY_MassiveDataDecrypt(
				hDeviceHandle,						//[in]�豸���
				phKey,							//[in]�������ܵ���ԿID
				ALG_TYPE_3DES_3KEY,					//[in]�����㷨
				USBKEY_CBC|USBKEY_PKCS5_PADDING,	//[in]����ģʽ�����ģʽ
				pcIV,								//[in]��ʼ�������������ecbģʽ������ΪNULL
				pucOut,								//[in]��������
				ulEncryptedLen,						//[in]�������ݳ���
				pucTemp,								//[out]���ص�����
				&ulLen);								//[in/out]in: pcData���������� out:���ص����ĳ���
	if (rtn !=0)
	{
		printf("USBKEY_MassiveDataDecrypt error! rtn = %#08x\n",rtn);
		return rtn;
	}
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	dsize = 16.0/duration;
	printf("3DES�㷨�����ٶ�Ϊ%.3fMbps\n",dsize);


	if(ulFileLen == ulLen && 0 == memcmp(pucTemp,pucData,ulLen))
		printf("3DES�㷨�ӽ�������ɹ���ԭ�������������һ��\n");
	else
	{
		printf("3DES�㷨�ӽ�������ʧ�ܣ�������������ݲ�ͬ\n");
		Sleep(3000);
		return -1;
	}


	//============================3DES_2KEY�ٶȲ���================================
	//���ɶԳ���Կ
	rtn = USBKEY_GenSymmetricKey(hDeviceHandle,16,ALG_TYPE_3DES_2KEY,USBKEY_CRYPT_EXPORTABLE|USBKEY_CRYPT_ONDEVICE|USBKEY_CRYPT_USER_PROTECTED,phKey);
	if (rtn !=0)
	{
		printf("USBKEY_GenSymmetricKey error! rtn = %#08x\n",rtn);
		return rtn;
	}
	
	ulEncryptedLen = ulFileLen+24;
	memset(pucOut,0,ulEncryptedLen);
	ulLen = ulFileLen+24;
	memset(pucTemp,0,ulLen);
	
	//�Գ���Կ�ӽ���
	//�������ݼ���
	memset(pcIV, 0x3, 8);

	start = clock();
	rtn = USBKEY_MassiveDataEncrypt(
				hDeviceHandle,					//[in]�豸���
				phKey,							//[in]�������ܵ���ԿID
				ALG_TYPE_3DES_2KEY,				//[in]�����㷨
				USBKEY_CBC|USBKEY_PKCS5_PADDING,	//[in]����ģʽ�����ģʽ
				pcIV,							//[in]��ʼ�������������ecbģʽ������ΪNULL
				pucData,							//[in]����������
				ulFileLen,							//[in]���������ݳ���
				pucOut,							//[out]���صļ��ܺ�����
				&ulEncryptedLen);//[in/out]in: pcEncryptedData��������С��out: ���ܺ����ݳ��Ȼ���Ҫ��pcEncryptedData��������С
	if (rtn !=0)
	{
		printf("USBKEY_MassiveDataEncrypt error! rtn = %#08x\n",rtn);
		return rtn;
	}
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	dsize = 16.0/duration;
	printf("3DES_2KEY�㷨�����ٶ�Ϊ%.3fMbps\n",dsize);

	//�������ݽ���
	start = clock();
	rtn = USBKEY_MassiveDataDecrypt(
				hDeviceHandle,						//[in]�豸���
				phKey,							//[in]�������ܵ���ԿID
				ALG_TYPE_3DES_2KEY,					//[in]�����㷨
				USBKEY_CBC|USBKEY_PKCS5_PADDING,	//[in]����ģʽ�����ģʽ
				pcIV,								//[in]��ʼ�������������ecbģʽ������ΪNULL
				pucOut,								//[in]��������
				ulEncryptedLen,						//[in]�������ݳ���
				pucTemp,								//[out]���ص�����
				&ulLen);								//[in/out]in: pcData���������� out:���ص����ĳ���
	if (rtn !=0)
	{
		printf("USBKEY_MassiveDataDecrypt error! rtn = %#08x\n",rtn);
		return rtn;
	}
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	dsize = 16.0/duration;
	printf("3DES_2KEY�㷨�����ٶ�Ϊ%.3fMbps\n",dsize);

	if(ulFileLen == ulLen && 0 == memcmp(pucTemp,pucData,ulLen))
		printf("3DES_2KEY�㷨�ӽ�������ɹ���ԭ�������������һ��\n");
	else
	{
		printf("3DES_2KEY�㷨�ӽ�������ʧ�ܣ�������������ݲ�ͬ\n");
		Sleep(3000);
		return -1;
	}



	//============================DES�ٶȲ���================================
	rtn = USBKEY_GenSymmetricKey(hDeviceHandle,8,ALG_TYPE_DES,USBKEY_CRYPT_EXPORTABLE|USBKEY_CRYPT_ONDEVICE|USBKEY_CRYPT_USER_PROTECTED,phKey);
	if (rtn !=0)
	{
		printf("USBKEY_GenSymmetricKey error! rtn = %#08x\n",rtn);
		return rtn;
	}

	ulEncryptedLen = ulFileLen+24;
	memset(pucOut,0,ulEncryptedLen);
	ulLen = ulFileLen+24;
	memset(pucTemp,0,ulLen);
	
	//�Գ���Կ�ӽ���
	//�������ݼ���
	memset(pcIV, 0x3, 8);

	start = clock();
	rtn = USBKEY_MassiveDataEncrypt(
				hDeviceHandle,					//[in]�豸���
				phKey,							//[in]�������ܵ���ԿID
				ALG_TYPE_DES,					//[in]�����㷨
				USBKEY_CBC|USBKEY_PKCS5_PADDING,	//[in]����ģʽ�����ģʽ
				pcIV,							//[in]��ʼ�������������ecbģʽ������ΪNULL
				pucData,							//[in]����������
				ulFileLen,							//[in]���������ݳ���
				pucOut,							//[out]���صļ��ܺ�����
				&ulEncryptedLen);//[in/out]in: pcEncryptedData��������С��out: ���ܺ����ݳ��Ȼ���Ҫ��pcEncryptedData��������С
	if (rtn !=0)
	{
		printf("USBKEY_MassiveDataEncrypt error! rtn = %#08x\n",rtn);
		return rtn;
	}
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	dsize = 16.0/duration;
	printf("DES�㷨�����ٶ�Ϊ%.3fMbps\n",dsize);


	//�������ݽ���
	start = clock();
	rtn = USBKEY_MassiveDataDecrypt(
				hDeviceHandle,						//[in]�豸���
				phKey,							//[in]�������ܵ���ԿID
				ALG_TYPE_DES,					//[in]�����㷨
				USBKEY_CBC|USBKEY_PKCS5_PADDING,	//[in]����ģʽ�����ģʽ
				pcIV,								//[in]��ʼ�������������ecbģʽ������ΪNULL
				pucOut,								//[in]��������
				ulEncryptedLen,						//[in]�������ݳ���
				pucTemp,								//[out]���ص�����
				&ulLen);								//[in/out]in: pcData���������� out:���ص����ĳ���
	if (rtn !=0)
	{
		printf("USBKEY_MassiveDataDecrypt error! rtn = %#08x\n",rtn);
		return rtn;
	}
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	dsize = 16.0/duration;
	printf("DES�㷨�����ٶ�Ϊ%.3fMbps\n",dsize);

	if(ulFileLen == ulLen && 0 == memcmp(pucTemp,pucData,ulLen))
		printf("DES�㷨�ӽ�������ɹ���ԭ�������������һ��\n");
	else
	{
		printf("DES�㷨�ӽ�������ʧ�ܣ�������������ݲ�ͬ\n");
		Sleep(3000);
		return -1;
	}

	free(pucData);
	free(pucTemp);
	free(pucOut);
	return 0;
}


USBKEY_ULONG UKeySymmKeyECBOpt_Test(USBKEY_HANDLE hDeviceHandle)
{
	unsigned long rtn = 0,i;
	USBKEY_ULONG ulLen,ulEncryptedLen;
	USBKEY_ULONG ulFileLen,ulDataLen;

	//USBKEY_UCHAR_PTR pucData,pucTemp,pucOut;

	USBKEY_UCHAR pucData[1024*8],pucTemp[1024*8],pucOut[1024*8];
	USBKEY_UCHAR phKey[2];

	clock_t start, finish;
	double dsize;
	double   duration;

	/*rtn = Tes_ReadFile("1.txt",NULL,&ulFileLen);
	if (rtn !=0)
	{
		printf("���ļ�1.txtʧ�ܣ���ȷ������Ŀ¼���Ƿ���ڴ��ļ���rtn = %#08x\n",rtn);
		return rtn;
	}
	pucData = (unsigned char *)malloc(ulFileLen);
	rtn = Tes_ReadFile("1.txt",pucData,&ulFileLen);
	if(0!= rtn)
	{
		printf("Tes_ReadFile error! CODE: 0x%08x", rtn);
		return rtn;
	}
	pucTemp = (unsigned char *)malloc(ulFileLen+24);
	pucOut = (unsigned char *)malloc(ulFileLen+24);
	ulEncryptedLen = ulFileLen+24;
	memset(pucOut,0,ulEncryptedLen);
	ulLen = ulFileLen+24;
	memset(pucTemp,0,ulLen);*/

	ulDataLen = sizeof(pucData);
	rtn = USBKEY_GenRandom(hDeviceHandle,pucData,ulDataLen);
	if (rtn !=0)
	{
		printf("USBKEY_GenRandom error! rtn = %#08x\n",rtn);
		return rtn;
	}

	//============================SCB2�ٶȲ���================================
	rtn = USBKEY_GenSymmetricKey(hDeviceHandle,16,ALG_TYPE_SCB2,USBKEY_CRYPT_EXPORTABLE|USBKEY_CRYPT_ONDEVICE|USBKEY_CRYPT_USER_PROTECTED,phKey);
	if (rtn !=0)
	{
		printf("USBKEY_GenSymmetricKey error! rtn = %#08x\n",rtn);
		return rtn;
	}
	
	ulEncryptedLen = sizeof(pucOut);
	ulLen = sizeof(pucTemp);
	memset(pucOut,0,ulEncryptedLen);
	memset(pucTemp,0,ulLen);

	//�Գ���Կ�ӽ���
	//�������ݼ���
	start = clock();
	for(i = 0; i < 250; i++)
	{
		rtn = USBKEY_MassiveDataEncrypt(
					hDeviceHandle,					//[in]�豸���
					phKey,							//[in]�������ܵ���ԿID
					ALG_TYPE_SCB2,					//[in]�����㷨
					USBKEY_ECB|USBKEY_NO_PADDING,	//[in]����ģʽ�����ģʽ
					NULL,							//[in]��ʼ�������������ecbģʽ������ΪNULL
					pucData,							//[in]����������
					ulDataLen,							//[in]���������ݳ���
					pucOut,							//[out]���صļ��ܺ�����
					&ulEncryptedLen);//[in/out]in: pcEncryptedData��������С��out: ���ܺ����ݳ��Ȼ���Ҫ��pcEncryptedData��������С
		if (rtn !=0)
		{
			printf("USBKEY_MassiveDataEncrypt error! rtn = %#08x\n",rtn);
			return rtn;
		}
	}
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	dsize = 16.0/duration;
	printf("SCB2�㷨�����ٶ�Ϊ%.3fMbps\n",dsize);


	//�������ݽ���
	start = clock();
	for(i = 0; i < 250; i++)
	{
		rtn = USBKEY_MassiveDataDecrypt(
					hDeviceHandle,						//[in]�豸���
					phKey,							//[in]�������ܵ���ԿID
					ALG_TYPE_SCB2,					//[in]�����㷨
					USBKEY_ECB|USBKEY_NO_PADDING,	//[in]����ģʽ�����ģʽ
					NULL,								//[in]��ʼ�������������ecbģʽ������ΪNULL
					pucOut,								//[in]��������
					ulEncryptedLen,						//[in]�������ݳ���
					pucTemp,								//[out]���ص�����
					&ulLen);								//[in/out]in: pcData���������� out:���ص����ĳ���
		if (rtn !=0)
		{
			printf("USBKEY_MassiveDataDecrypt error! rtn = %#08x\n",rtn);
			return rtn;
		}
	}
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	dsize = 16.0/duration;
	printf("SCB2�㷨�����ٶ�Ϊ%.3fMbps\n",dsize);


	if(ulDataLen == ulLen && 0 == memcmp(pucTemp,pucData,ulLen))
		printf("SCB2�㷨�ӽ�������ɹ���ԭ�������������һ��\n");
	else
	{
		printf("SCB2�㷨�ӽ�������ʧ�ܣ�������������ݲ�ͬ\n");
		Sleep(3000);
		return -1;
	}


	//============================SSF33�ٶȲ���================================
	rtn = USBKEY_GenSymmetricKey(hDeviceHandle,16,ALG_TYPE_SSF33,USBKEY_CRYPT_EXPORTABLE|USBKEY_CRYPT_ONDEVICE|USBKEY_CRYPT_USER_PROTECTED,phKey);
	if (rtn !=0)
	{
		printf("USBKEY_GenSymmetricKey error! rtn = %#08x\n",rtn);
		return rtn;
	}

	ulEncryptedLen = sizeof(pucOut);
	ulLen = sizeof(pucTemp);
	memset(pucOut,0,ulEncryptedLen);
	memset(pucTemp,0,ulLen);
	
	//�Գ���Կ�ӽ���
	//�������ݼ���

	start = clock();
	for(i = 0; i < 250; i++)
	{
		rtn = USBKEY_MassiveDataEncrypt(
					hDeviceHandle,					//[in]�豸���
					phKey,							//[in]�������ܵ���ԿID
					ALG_TYPE_SSF33,					//[in]�����㷨
					USBKEY_ECB|USBKEY_NO_PADDING,	//[in]����ģʽ�����ģʽ
					NULL,							//[in]��ʼ�������������ecbģʽ������ΪNULL
					pucData,							//[in]����������
					ulDataLen,							//[in]���������ݳ���
					pucOut,							//[out]���صļ��ܺ�����
					&ulEncryptedLen);//[in/out]in: pcEncryptedData��������С��out: ���ܺ����ݳ��Ȼ���Ҫ��pcEncryptedData��������С
		if (rtn !=0)
		{
			printf("USBKEY_MassiveDataEncrypt error! rtn = %#08x\n",rtn);
			return rtn;
		}
	}
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	dsize = 16.0/duration;
	printf("SSF33�㷨�����ٶ�Ϊ%.3fMbps\n",dsize);


	//�������ݽ���
	start = clock();
	for(i = 0; i < 250; i++)
	{
		rtn = USBKEY_MassiveDataDecrypt(
					hDeviceHandle,						//[in]�豸���
					phKey,							//[in]�������ܵ���ԿID
					ALG_TYPE_SSF33,					//[in]�����㷨
					USBKEY_ECB|USBKEY_NO_PADDING,	//[in]����ģʽ�����ģʽ
					NULL,								//[in]��ʼ�������������ecbģʽ������ΪNULL
					pucOut,								//[in]��������
					ulEncryptedLen,						//[in]�������ݳ���
					pucTemp,								//[out]���ص�����
					&ulLen);								//[in/out]in: pcData���������� out:���ص����ĳ���
		if (rtn !=0)
		{
			printf("USBKEY_MassiveDataDecrypt error! rtn = %#08x\n",rtn);
			return rtn;
		}
	}
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	dsize = 16.0/duration;
	printf("SSF33�㷨�����ٶ�Ϊ%.3fMbps\n",dsize);


	if(ulDataLen == ulLen && 0 == memcmp(pucTemp,pucData,ulLen))
		printf("SSF33�㷨�ӽ�������ɹ���ԭ�������������һ��\n");
	else
	{
		printf("SSF33�㷨�ӽ�������ʧ�ܣ�������������ݲ�ͬ\n");
		Sleep(3000);
		return -1;
	}

	//============================3DES�ٶȲ���================================
	rtn = USBKEY_GenSymmetricKey(hDeviceHandle,24,ALG_TYPE_3DES_3KEY,USBKEY_CRYPT_EXPORTABLE|USBKEY_CRYPT_ONDEVICE|USBKEY_CRYPT_USER_PROTECTED,phKey);
	if (rtn !=0)
	{
		printf("USBKEY_GenSymmetricKey error! rtn = %#08x\n",rtn);
		return rtn;
	}

	ulEncryptedLen = sizeof(pucOut);
	ulLen = sizeof(pucTemp);
	memset(pucOut,0,ulEncryptedLen);
	memset(pucTemp,0,ulLen);
	
	//�Գ���Կ�ӽ���
	//�������ݼ���
	start = clock();
	for(i = 0; i < 250; i++)
	{
		rtn = USBKEY_MassiveDataEncrypt(
					hDeviceHandle,					//[in]�豸���
					phKey,							//[in]�������ܵ���ԿID
					ALG_TYPE_3DES_3KEY,					//[in]�����㷨
					USBKEY_ECB|USBKEY_NO_PADDING,	//[in]����ģʽ�����ģʽ
					NULL,							//[in]��ʼ�������������ecbģʽ������ΪNULL
					pucData,							//[in]����������
					ulDataLen,							//[in]���������ݳ���
					pucOut,							//[out]���صļ��ܺ�����
					&ulEncryptedLen);//[in/out]in: pcEncryptedData��������С��out: ���ܺ����ݳ��Ȼ���Ҫ��pcEncryptedData��������С
		if (rtn !=0)
		{
			printf("USBKEY_MassiveDataEncrypt error! rtn = %#08x\n",rtn);
			return rtn;
		}
	}
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	dsize = 16.0/duration;
	printf("3DES�㷨�����ٶ�Ϊ%.3fMbps\n",dsize);


	//�������ݽ���
	start = clock();
	for(i = 0; i < 250; i++)
	{
		rtn = USBKEY_MassiveDataDecrypt(
					hDeviceHandle,						//[in]�豸���
					phKey,							//[in]�������ܵ���ԿID
					ALG_TYPE_3DES_3KEY,					//[in]�����㷨
					USBKEY_ECB|USBKEY_NO_PADDING,	//[in]����ģʽ�����ģʽ
					NULL,								//[in]��ʼ�������������ecbģʽ������ΪNULL
					pucOut,								//[in]��������
					ulEncryptedLen,						//[in]�������ݳ���
					pucTemp,								//[out]���ص�����
					&ulLen);								//[in/out]in: pcData���������� out:���ص����ĳ���
		if (rtn !=0)
		{
			printf("USBKEY_MassiveDataDecrypt error! rtn = %#08x\n",rtn);
			return rtn;
		}
	}
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	dsize = 16.0/duration;
	printf("3DES�㷨�����ٶ�Ϊ%.3fMbps\n",dsize);

	
	if(ulDataLen == ulLen && 0 == memcmp(pucTemp,pucData,ulLen))
		printf("3DES�㷨�ӽ�������ɹ���ԭ�������������һ��\n");
	else
	{
		printf("3DES�㷨�ӽ�������ʧ�ܣ�������������ݲ�ͬ\n");
		Sleep(3000);
		return -1;
	}


	//============================3DES_2KEY�ٶȲ���================================
	//���ɶԳ���Կ
	rtn = USBKEY_GenSymmetricKey(hDeviceHandle,16,ALG_TYPE_3DES_2KEY,USBKEY_CRYPT_EXPORTABLE|USBKEY_CRYPT_ONDEVICE|USBKEY_CRYPT_USER_PROTECTED,phKey);
	if (rtn !=0)
	{
		printf("USBKEY_GenSymmetricKey error! rtn = %#08x\n",rtn);
		return rtn;
	}

	ulEncryptedLen = sizeof(pucOut);
	ulLen = sizeof(pucTemp);
	memset(pucOut,0,ulEncryptedLen);
	memset(pucTemp,0,ulLen);

	//�Գ���Կ�ӽ���
	//�������ݼ���

	start = clock();
	for(i = 0; i < 250; i++)
	{
		rtn = USBKEY_MassiveDataEncrypt(
					hDeviceHandle,					//[in]�豸���
					phKey,							//[in]�������ܵ���ԿID
					ALG_TYPE_3DES_2KEY,				//[in]�����㷨
					USBKEY_ECB|USBKEY_NO_PADDING,	//[in]����ģʽ�����ģʽ
					NULL,							//[in]��ʼ�������������ecbģʽ������ΪNULL
					pucData,							//[in]����������
					ulDataLen,							//[in]���������ݳ���
					pucOut,							//[out]���صļ��ܺ�����
					&ulEncryptedLen);//[in/out]in: pcEncryptedData��������С��out: ���ܺ����ݳ��Ȼ���Ҫ��pcEncryptedData��������С
		if (rtn !=0)
		{
			printf("USBKEY_MassiveDataEncrypt error! rtn = %#08x\n",rtn);
			return rtn;
		}
	}
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	dsize = 16.0/duration;
	printf("3DES_2KEY�㷨�����ٶ�Ϊ%.3fMbps\n",dsize);

	//�������ݽ���
	start = clock();
	for(i = 0; i < 250; i++)
	{
		rtn = USBKEY_MassiveDataDecrypt(
					hDeviceHandle,						//[in]�豸���
					phKey,							//[in]�������ܵ���ԿID
					ALG_TYPE_3DES_2KEY,					//[in]�����㷨
					USBKEY_ECB|USBKEY_NO_PADDING,	//[in]����ģʽ�����ģʽ
					NULL,								//[in]��ʼ�������������ecbģʽ������ΪNULL
					pucOut,								//[in]��������
					ulEncryptedLen,						//[in]�������ݳ���
					pucTemp,								//[out]���ص�����
					&ulLen);								//[in/out]in: pcData���������� out:���ص����ĳ���
		if (rtn !=0)
		{
			printf("USBKEY_MassiveDataDecrypt error! rtn = %#08x\n",rtn);
			return rtn;
		}
	}
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	dsize = 16.0/duration;
	printf("3DES_2KEY�㷨�����ٶ�Ϊ%.3fMbps\n",dsize);

	if(ulDataLen == ulLen && 0 == memcmp(pucTemp,pucData,ulLen))
		printf("3DES_2KEY�㷨�ӽ�������ɹ���ԭ�������������һ��\n");
	else
	{
		printf("3DES_2KEY�㷨�ӽ�������ʧ�ܣ�������������ݲ�ͬ\n");
		Sleep(3000);
		return -1;
	}


	//============================DES�ٶȲ���================================
	rtn = USBKEY_GenSymmetricKey(hDeviceHandle,8,ALG_TYPE_DES,USBKEY_CRYPT_EXPORTABLE|USBKEY_CRYPT_ONDEVICE|USBKEY_CRYPT_USER_PROTECTED,phKey);
	if (rtn !=0)
	{
		printf("USBKEY_GenSymmetricKey error! rtn = %#08x\n",rtn);
		return rtn;
	}

	ulEncryptedLen = sizeof(pucOut);
	ulLen = sizeof(pucTemp);
	memset(pucOut,0,ulEncryptedLen);
	memset(pucTemp,0,ulLen);
	
	//�Գ���Կ�ӽ���
	//�������ݼ���
	start = clock();
	for(i = 0; i < 250; i++)
	{
		rtn = USBKEY_MassiveDataEncrypt(
					hDeviceHandle,					//[in]�豸���
					phKey,							//[in]�������ܵ���ԿID
					ALG_TYPE_DES,					//[in]�����㷨
					USBKEY_ECB|USBKEY_NO_PADDING,	//[in]����ģʽ�����ģʽ
					NULL,							//[in]��ʼ�������������ecbģʽ������ΪNULL
					pucData,							//[in]����������
					ulDataLen,							//[in]���������ݳ���
					pucOut,							//[out]���صļ��ܺ�����
					&ulEncryptedLen);//[in/out]in: pcEncryptedData��������С��out: ���ܺ����ݳ��Ȼ���Ҫ��pcEncryptedData��������С
		if (rtn !=0)
		{
			printf("USBKEY_MassiveDataEncrypt error! rtn = %#08x\n",rtn);
			return rtn;
		}
	}
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	dsize = 16.0/duration;
	printf("DES�㷨�����ٶ�Ϊ%.3fMbps\n",dsize);


	//�������ݽ���
	start = clock();
	for(i = 0; i < 250; i++)
	{
		rtn = USBKEY_MassiveDataDecrypt(
					hDeviceHandle,						//[in]�豸���
					phKey,							//[in]�������ܵ���ԿID
					ALG_TYPE_DES,					//[in]�����㷨
					USBKEY_ECB|USBKEY_NO_PADDING,	//[in]����ģʽ�����ģʽ
					NULL,								//[in]��ʼ�������������ecbģʽ������ΪNULL
					pucOut,								//[in]��������
					ulEncryptedLen,						//[in]�������ݳ���
					pucTemp,								//[out]���ص�����
					&ulLen);								//[in/out]in: pcData���������� out:���ص����ĳ���
		if (rtn !=0)
		{
			printf("USBKEY_MassiveDataDecrypt error! rtn = %#08x\n",rtn);
			return rtn;
		}
	}
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	dsize = 16.0/duration;
	printf("DES�㷨�����ٶ�Ϊ%.3fMbps\n",dsize);


	if(ulDataLen == ulLen && 0 == memcmp(pucTemp,pucData,ulLen))
		printf("DES�㷨�ӽ�������ɹ���ԭ�������������һ��\n");
	else
	{
		printf("DES�㷨�ӽ�������ʧ�ܣ�������������ݲ�ͬ\n");
		Sleep(3000);
		return -1;
	}

	//free(pucData);
	//free(pucTemp);
	//free(pucOut);
	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	unsigned long rtn = 0;

	USBKEY_HANDLE hDeviceHandle;
	USBKEY_CHAR ppcDeviceIdS[10][260];
	int pucDeviceQuantity = 0;
	USBKEY_ULONG ulReNum = 0;
	USBKEY_ULONG ulLen,ulNum;

	USBKEY_DEVINFO devinfo;
	USBKEY_UCHAR AppNameList[256];

	printf("################################���ܲ��Կ�ʼ��################################\n");

	rtn = sizeof(USBKEY_RSA_PUBLIC_KEY_1024);
	rtn = sizeof(USBKEY_RSA_PRIVATE_KEY_1024);
	rtn = sizeof(USBKEY_RSA_PUBLIC_KEY_2048);
	rtn = sizeof(USBKEY_RSA_PRIVATE_KEY_2048);

	rtn = USBKEY_DeviceEnum(ppcDeviceIdS,&pucDeviceQuantity);
	if (rtn !=0)
	{
		printf("USBKEY_DeviceEnum error! rtn = %#08x\n",rtn);
		Sleep(5000);
		return rtn;
	}
	printf("ö���豸�ɹ�! ukey��ĿΪ��%d��\n",pucDeviceQuantity);
	//Sleep(1000);

	rtn = USBKEY_DeviceOpen(ppcDeviceIdS[0], 1, &hDeviceHandle);
 	if (rtn !=0)
	{
		printf("USBKEY_DeviceOpen error! rtn = %#08x\n",rtn);
		//Sleep(5000);
		return rtn;
	}
	printf("==============================����豸�ɹ���==============================\n");

	rtn = USBKEY_DeviceGetStat(hDeviceHandle, &devinfo);
	if (rtn !=0)
	{
		printf("USBKEY_DeviceGetStat error! rtn = %#08x\n",rtn);
		return rtn;
	}
	printf("==============================����豸��Ϣ��==============================\n");

	if(devinfo.HWVersion.major <= 3 && devinfo.FirmwareVersion.major < 5)
	{
		printf("��֧���豸���й̼����豸�̼��汾�����\n");
		Sleep(5000);
		return -1;
	}

	//ö��Ӧ��
	rtn = USBKEY_EnumApplication(hDeviceHandle,NULL,&ulLen,&ulNum);
	if (rtn !=0)
	{
		printf("USBKEY_EnumApplication error! rtn = %#08x\n",rtn);
		Sleep(3000);
		return rtn;
	}
	memset(AppNameList,0,sizeof(AppNameList));
	rtn = USBKEY_EnumApplication(hDeviceHandle,AppNameList,&ulLen,&ulNum);
	if (rtn !=0)
	{
		printf("USBKEY_EnumApplication error! rtn = %#08x\n",rtn);
		Sleep(2000);
		return rtn;
	}

	//��Ӧ��
  	rtn = USBKEY_OpenApplication(hDeviceHandle,AppNameList);
	if (rtn !=0)
	{
		printf("USBKEY_OpenApplication error! rtn = %#08x\n",rtn);
		Sleep(2000);
		return rtn;
	}

	//check pin
	rtn = USBKEY_CheckPin(hDeviceHandle,USBKEY_USERTYPE_USER,(USBKEY_UCHAR_PTR)"123456",6,&ulReNum);
	if (rtn !=0)
	{
		printf("USBKEY_CheckPin error! rtn = %#08x\n",rtn);
		Sleep(2000);
		return rtn;
	}
	printf("==============================�豸ע��ɹ���==============================\n");

	//����RSA�ӽ����ٶ�
	rtn = RSAUKeyPair_Test(hDeviceHandle);
	if (rtn !=0)
	{
		printf("RSAUKeyPair_Test error! rtn = %#08x\n",rtn);
		USBKEY_DeleteContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1");
		Sleep(5000);
		//return rtn;
	}
	printf("===========================RSA�㷨���ܲ�����ɣ�===========================\n");
	//Sleep(1000);

	//����SM2�ӽ����ٶ�
	//if(devinfo.HWVersion.major >= 5 || (devinfo.FirmwareVersion.major >= 5 && devinfo.FirmwareVersion.minor > 1))
	//{
	//	rtn = ECCUKeyPair_Test(hDeviceHandle);
	//	if (rtn !=0)
	//	{
	//		printf("ECCUKeyPair_Test error! rtn = %#08x\n",rtn);
	//		USBKEY_DeleteContainer(hDeviceHandle,(USBKEY_UCHAR_PTR)"testContainer1");
	//		Sleep(5000);
	//		//return rtn;
	//	}
	//	printf("============================SM2�㷨���ܲ�����ɣ�============================\n");
	//	Sleep(1000);
	//}

	//���ԶԳ�ECB�ӽ����ٶ�
	//rtn = UKeySymmKeyECBOpt_Test(hDeviceHandle);
	//if (rtn !=0)
	//{
	//	printf("UKeySymmKeyECBOpt_Test error! rtn = %#08x\n",rtn);
	//	Sleep(5000);
	//	//return rtn;
	//}
	//printf("===========================�Գ��㷨�������ܲ������===========================\n");
	//Sleep(5000);

	//���ԶԳ�CBC�ӽ����ٶ�
 	/*rtn = UKeySymmKeyCBCOpt_Test(hDeviceHandle);
	if (rtn !=0)
	{
		printf("UKeySymmKeyCBCOpt_Test error! rtn = %#08x\n",rtn);
		Sleep(5000);
		return rtn;
	}
	printf("=========================�Գ��㷨CBC�������ܲ�����ɣ�==========================\n");
	Sleep(5000);*/

	/*rtn = USBKEY_DeviceClose(hDeviceHandle);
	if (rtn !=0)
	{
		printf("USBKEY_DeviceClose error! rtn = %#08x\n",rtn);
		Sleep(5000);
		return rtn;
	}*/

	printf("#############################UKey Test End Success!#############################\n");
	//Sleep(5000);
	return rtn;
}
