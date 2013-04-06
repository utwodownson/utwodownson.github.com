/*	Device.h
	Author  :   yangdayong@aisino.com
	CO.     :   AISINO
	date    :   2008��5��19�գ�
	����    :   Ӳ�������ӿڣ��������Ʋ�ʵ��	*/

#ifndef _DEVICE_H
#define _DEVICE_H


///////////////////////////////////////////////////////////////////////////////
//	include����
///////////////////////////////////////////////////////////////////////////////
//#include "windows.h"

///////////////////////////////////////////////////////////////////////////////
//	�����붨��
///////////////////////////////////////////////////////////////////////////////
#define HANDLEERROR     -1;
#define DATALENGTHERROR 99;

///////////////////////////////////////////////////////////////////////////////
//	�����궨��
///////////////////////////////////////////////////////////////////////////////
#define	DEVICE_MAX_QUANTITY	10	//����豸����
#define DEVICE_ID_MAX_LEN	255	//�豸ID��󳤶�
#define CDB_NO_DATABUF	    0	//cbd����û��������
#define CDB_HAS_DATABUF	    1	//cbd������������


#ifdef __cplusplus
extern "C" {
#endif

//	ö�ټ�����������ӵ��豸��ID
//	���ppcDeviceIdSΪNULL�����pdwDeviceQuantity������Ҫ����char *[DEVICE_ID_MAX_LEN]����
//	���ppcDeviceIdS�ǿգ����ppcDeviceIdS���������豸��ID��
//		����pdwDeviceQuantity����ʵ���豸����
//	����ֵ��
//		0��	OK
//		>0:	FAIL�����ص��Ǵ�����
 __declspec(dllexport) UCHAR DeviceEnum(
	char	**ppcDeviceIdS,		//[out]�豸ID���б�
	int	*pucDeviceQuantity);	//[out]�豸ID�ĸ���

//	��ĳ���豸
//	���ucShareMode=1 ��ʾ����ģʽ ����Ӧ�ó�����Է����豸
//	����ֵ��
//		0��	OK
//		>0:	FAIL�����ص��Ǵ�����
__declspec(dllexport) DWORD DeviceOpen(
	char    *pcDeviceId,		//[in]�豸��ID
	UCHAR	ucShareMode,		//[in]����ģʽ
	HANDLE	*phDeviceHandle);	//[out]�豸���

//	�ر�ĳ���豸
//	����ֵ��
//		0��	OK
//		>0:	FAIL�����ص��Ǵ�����
 __declspec(dllexport) DWORD DeviceClose(
	HANDLE	hDeviceHandle );	//[in]�豸���
	
//	���õ�ǰ���ӵ�ĳ���豸��ʹ�豸�����ʼ״̬���豸�����һ�θձ��򿪣�
//	����ֵ��
//		0��	OK
//		>0:	FAIL�����ص��Ǵ�����
 __declspec(dllexport) DWORD DeviceReset(
	HANDLE	hDeviceHandle,		//[in]�豸���
	BYTE	*pbCmd,			//[in]��λ��������
	DWORD	dwCmdLen );		//[in]�����
	
//	�õ�ǰ���ӵ�ĳ���豸ִ��ĳ������
//	����ֵ��
//		0��	OK
//		>0:	FAIL�����ص��Ǵ�����
 __declspec(dllexport) DWORD DeviceExecCmd(
	HANDLE	hDeviceHandle,		//[in]�豸���
	BYTE	*pbCmd,			//[in]����
	int	dwCmdLen,		//[in]�����
	BYTE	*pbResult,		//[out]���
	int	*pdwResultLen);		//[out]�������

//	��ȡ��ǰ���ӵ�ĳ���豸��״̬.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
 __declspec(dllexport) DWORD DeviceGetStat(
	HANDLE hDeviceHandle);		//[in]�豸���

#ifdef __cplusplus
}
#endif

#endif