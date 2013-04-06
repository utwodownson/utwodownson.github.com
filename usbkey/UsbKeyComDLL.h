/*
  �ļ�����: UsbKeyComDLL.h
  Copyright (c) 2008 Aerospace Information. Co., Ltd.
  ��	��: heli
  ��������: 2009-05-12
  ��	��: ʵ��USBKey��PC��ͨ�ź���
*/

#ifndef _USBKEYCOMDLL_H
#define _USBKEYCOMDLL_H

#include <windows.h>

// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� USBKEYCOMDLL_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// USBKEYCOMDLL_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef USBKEYCOMDLL_EXPORTS
#define USBKEYCOMDLL_API __declspec(dllexport)
#else
#define USBKEYCOMDLL_API __declspec(dllimport)
#endif

//=================================================�����붨��==================================================
//1��994Ϊwindows��GetLastError()���صĴ�����
#define AKCOM_OK							0					//�����ɹ�
#define AKCOM_GENERAL_ERROR					0x10000001			//ͨ�ô���
#define AKCOM_PARAM_ERROR					0x10000002			//��������
#define AKCOM_BUFFER_TOO_SMALL				0x10000003			//���������̫С
#define AKCOM_LOCK_ERROR					0x10000004			//����ʧ��
#define AKCOM_GET_DEVICE_NAME_ERROR			0x10000005			//��ȡ�豸����ʧ��
#define AKCOM_CREATE_DEVICE_HANDLE_ERROR	0x10000006			//�����豸���ʧ��
#define AKCOM_GET_DEVICE_INFO_ERROR			0x10000007			//��ȡ�豸��Ϣʧ��
#define AKCOM_CREATE_LOCK_ERROR				0x10000008			//������ʧ��
#define AKCOM_UNLOCK_ERROR					0x10000009			//����ʧ��

//=================================================��������====================================================
#define MAX_DEVICE_NUM						10	
#define AK_IOCTL_DATA_OUT					0
#define AK_IOCTL_DATA_IN					1
#define MAX_SEND_PACKAGE					2048
#define DEVICE_TYPE_USBKEY					1
#define	DEVICE_TYPE_CARD					2
#define CARD_MARK					"Reader:Smart Card"
#define USB_MARK					"Aision:UsbKey Com"

#ifdef __cplusplus
	extern "C" {
#endif


//=======================================================================================
//��������:	DeviceEnum
//��������:	ppcDeviceIdS[out]				�豸ID�б�
//				pucDeviceQuantity[out]          �豸��
//					
//����ֵ:		0  :ok
//��������: 	ö���豸�����ppcDeviceIdSΪNULL��pucDeviceQuantity���ص�ǰ���ӵ��豸��
//=========================================================================================		
USBKEYCOMDLL_API DWORD DeviceEnum(
	char ppcDeviceIdS[MAX_DEVICE_NUM][MAX_PATH],		//[out]�豸ID�б�
	int	*pucDeviceQuantity);	//[out]�豸��


//==============================================================
//��������:	DeviceOpen
//��������:	pcDeviceId:   //[in]�豸ID
//				ucShareMode:  //[in]����ģʽ
//				phDeviceHandle://[out]�豸���
//				char	*pcDeviceType		//�豸����
//����ֵ:		0:	�ɹ�
//				~0: ʧ��
//��������: 	���豸
//==============================================================
USBKEYCOMDLL_API DWORD DeviceOpen(
	char    *pcDeviceId,		//[in]�豸��ID
	UCHAR	ucShareMode,		//[in]����ģʽ
	HANDLE	*phDeviceHandle,    //[out]�豸���
	BYTE	*pcDeviceType		//�豸����
	);	

//==============================================================
//��������:	GetDeviceList
//��������:	pHDevList[out]: �豸����б�
//				piDevNum[out]: �豸��
//����ֵ:		0:	�ɹ�
//				~0: ʧ��
//��������: 	ȡ�豸����б�
//==============================================================
USBKEYCOMDLL_API DWORD GetDeviceList(HANDLE *pHDevList, int *piDevNum,BYTE *type);

//==============================================================
//��������:	DeviceExecCmd
//��������:	hDeviceHandle[in]:		�豸���
//				pbCmd[in]:				����
//				dwCmdLen[in]:			�����
//				pbResult[out]:			��Ӧ
//				pdwResultLen[in/out]:	in: ��Ӧbuffer�ĳ��ȣ�out: ��Ӧ����
//����ֵ:		0:	�ɹ�
//				~0: ʧ��
//��������: 	�õ�ǰ���ӵ�ĳ���豸ִ��ĳ������
//==============================================================
USBKEYCOMDLL_API DWORD DeviceExecCmd(
	HANDLE	hDeviceHandle,	
	BYTE	*pbCmd,			
	int		dwCmdLen,		
	BYTE	*pbResult,		
	int		*pdwResultLen);

//==============================================================
//��������:		DeviceClose
//��������:		hDeviceHandle[in]:	�豸���
//����ֵ:		0:	�ɹ�
//				~0: ʧ��
//��������: 	�ر��豸���
//==============================================================
USBKEYCOMDLL_API DWORD DeviceClose(HANDLE  hDeviceHandle,BYTE pcDeviceType);

//==============================================================
//��������:	SendDeviceCmd
//��������:	hDeviceHandle[in]:				�豸���
//				pbCDB[in]:						cdb����
//				dwCDBLen[in]:					cdb�����
//				dwDataTransferLength[in]:		Ҫ����/���յ����ݳ���
//				pbDataBuffer[out]:				����/��������buffer
//				pdwRetLen[out]:					ʵ�ʷ���/�������ݵĳ���
//				pbStatusCode[out]:				���յ���״̬�룬0���ɹ�������Ϊ������
//				bTransferDir[in]:				���䷽��AK_IOCTL_DATA_IN: ���豸�����ݣ�AK_IOCTL_DATA_OUT: д���ݵ��豸	
//����ֵ:		0:	�ɹ�
//				~0: ʧ��
//��������: 	��������
//==============================================================
USBKEYCOMDLL_API DWORD SendDeviceCmd(
			HANDLE		hDeviceHandle,
			BYTE		*pbCDB,
			UCHAR		cdbLen,
			DWORD		dwDataTransferLength,
			BYTE		*pbDataBuffer,
			DWORD		*pdwRetLen,
			BYTE		*pbStatusCode,
			BYTE		bTransferDir);

//==============================================================
//��������:	DeviceExecCmdExt
//��������:	hDeviceHandle[in]:		�豸���
//				pcDeviceType			�豸����
//				Cla[in]:				�����ײ�
//				Ins[in]:				�����ײ�
//				P1[in]:				    �����ײ�
//				P2[in]:				    �����ײ�
//				Lc[in]:				    ���ݳ���
//				pCmdData[in]			��������
//				Le[in]:				    �����������ݳ���
//				dwCmdLen[in]:			�����
//				pbResult[out]:			��Ӧ
//				pdwResultLen[in/out]:	in: ��Ӧbuffer�ĳ��ȣ�out: ��Ӧ����
//����ֵ:		0:	�ɹ�
//				~0: ʧ��
//��������: 	�õ�ǰ���ӵ�ĳ���豸ִ��ĳ������
//==============================================================
USBKEYCOMDLL_API DWORD DeviceExecCmdExt(
			HANDLE hDeviceHandle,
			BYTE pcDeviceType,
			BYTE Cla,
			BYTE Ins,
			BYTE P1,
			BYTE P2,
			unsigned int Lc,
			BYTE *pCmdData,
			unsigned int Le,
			BYTE *pbResult,
			int *pdwResultLen
			);

#ifdef __cplusplus
	}
#endif

#endif