/*	UsbKeyAPI.h
	Author  :   caizhun@aisino.com
	CO.     :   AISINO
	date    :   2008��5��26��
	����    :   USBKEY�ײ�API�ӿ�	*/

#ifndef _USBKEYAPI_H
#define _USBKEYAPI_H


///////////////////////////////////////////////////////////////////////////////
//	include����/
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//	�����궨��
///////////////////////////////////////////////////////////////////////////////
#define _USBKEY_WIN32_
//#define _USBKEY_LINUX_
#define _USBKEY_API_DLL_

#ifdef _USBKEY_WIN32_
#include <windows.h>
#ifdef _USBKEY_API_DLL_
#define USBKEY_API_MODE(ReturnType) __declspec(dllexport) ReturnType WINAPI
#else
#define USBKEY_API_MODE(ReturnType) __declspec(dllimport) ReturnType WINAPI
#endif
#else
#ifdef _USBKEY_API_DLL_
#define USBKEY_API_MODE(ReturnType) returnType
#else
#define USBKEY_API_MODE(ReturnType) extern returnType
#endif
#endif


///////////////////////////////////////////////////////////////////////////////
//	�����붨��
///////////////////////////////////////////////////////////////////////////////
#define USBKEY_OK							0x00000000	//�ɹ�
#define USBKEY_GENERAL_ERROR				0x00000001	//һ�����
#define USBKEY_RESERVED_NAME_ERROR			0x00000002	//������
#define USBKEY_NO_SUCH_RECORD				0x00000003	//û�д˼�¼
#define USBKEY_BUFFER_TOO_SHORT				0x00000004	//����������
#define USBKEY_NO_PRIVILEGE					0x00000005	//Ȩ�޲���
#define USBKEY_FILE_EXIST					0x00000006	//�ļ���Ŀ¼�Ѿ�����
#define USBKEY_FILE_NOT_FOUND				0x00000007	//�ļ�δ�ҵ�
#define USBKEY_RECORD_NOT_FOUND				0x00000008	//��¼δ�ҵ�
#define USBKEY_PARAM_ERROR					0x00000009	//��������
#define USBKEY_CONTAINER_NAME_EXIST			0x00000010	//���������Ѵ���
#define USBKEY_MALLOC_ERROR					0x00000011	//�ڴ����ʧ��
#define USBKEY_DISCARD_PADDING_ERROR		0x00000012	//ȥ�����ʧ��
#define USBKEY_MAX_NUM_OF_CONTAINER_ERROR	0x00000013	//�����ﵽ�������
#define USBKEY_MUTEX_BAD					0x00000014	//mutex����
#define USBKEY_MAX_NUM_OF_SYMKEY			0x00000015	//�Գ���Կ�ﵽ�������
#define USBKEY_KEY_ID_ERROR					0x00000016	//��ԿID����
#define USBKEY_TIME_OUT_ERROR				0x00000017	//ʱ�䳬ʱ
#define USBKEY_INVALID_HANDLE_ERROR			0x00000018	//�����Ч
#define USBKEY_MUTEX_ERROR					0x00000019	//�̴߳���
#define USBKEY_FUNCTION_NOT_SUPPORT			0x10000000	//������֧��

/////////////////////////////////////////////////////////////////////////////
//  �豸���ش���
/////////////////////////////////////////////////////////////////////////////
#define DEVICE_REMAIN_NT	 				0x63c0	//ʣ��ĳ��Դ�������0x63c2��ʾ�����Գ���2��
#define DEVICE_MEM_ERROR					0x6581	//�������
#define DEVICE_P3_ERROR   					0x6700	//P3����
#define DEVICE_NO_CURRENT_EF				0x6989	//û�е�ǰEF
#define DEVICE_CMD_DATA_ERROR				0x6a80	//�ļ���Ŀ¼�Ѵ���
#define DEVICE_FUNC_NOT_SUPPORT				0x6a81	//��֧�ִ˹���
#define DEVICE_FILE_NOT_FIND				0x6a82 	//û�ҵ��ļ�
#define DEVICE_RECORD_NOT_FIND				0x6a83	//û�ҵ���¼
#define DEVICE_MEM_NOT_ENOUGH				0x6a84	//����ռ䲻��
#define DEVICE_P1P2_ERROR  					0x6a86	//P1��P2����

#define DEVICE_NA_STATUS					0x6901	//״̬�����㣬����ûMF������´���df��ef
#define DEVICE_FILE_TYPE_ERROR				0x6981 	//�ļ����ʹ���
#define DEVICE_RIGHT_FAIL					0x6982	//Ȩ�޲�����
#define DEVICE_KEY_LOCKED					0x6983 	//��Կ����
#define DEVICE_NO_RANDOM					0x6984 	//û�������
#define DEVICE_APP_LOCK_TEMP	 			0x6985	//Ӧ����ʱ����
#define DEVICE_SEC_CONDITON_NOT_SATISFY 	0x6986	//��ȫ״̬������
#define DEVICE_USE_CONDITON_NOT_SATISFY 	0x6987	//ʹ������������

#define DEVICE_MAC_ERROR					0x6988	//MAC����

#define DEVICE_RANGE_ERROR  				0x6b00	//��/д�ļ�Խ��
#define DEVICE_6C00							0x6c00	//���ݳ��ȴ��󣬲�ָʾ��ȷ�ĳ��ȣ�����0x6c30��ʾ��ȷ�ĳ���Ӧ��Ϊ0x30
#define DEVICE_INS_ERROR  					0x6d00	//ָ�����
#define DEVICE_CLA_ERROR  					0x6e00	//����������
#define DEVICE_6F00							0x6f00	//û����Ӧ

#define DEVICE_SUCCESS     					0x9000	//�ɹ�
#define DEVICE_APP_LOCK_FOREVER	 			0x9303	//Ӧ����������

#define DEVICE_DF_LEVEL_UNSUPPORT			0x9401	//��֧�ִ�Ŀ¼����
#define DEVICE_KEY_NOT_FIND					0x9402	//��Կû���ҵ�
#define DEVICE_FILE_POS_ERROR				0x9403	//�ļ�ָ��λ�ô���
#define DEVICE_SYS_INIT_ERROR				0x9404	//ϵͳ��ʼ������
#define DEVICE_ALG_NOT_SUPPORT				0x9405	//�㷨��֧��

#define PKI_ERROR_PARAMETER                 0xFF01    //  �������� 
#define PKI_ERROR_BIG_NUMBER_EXP_MOD        0xFF02    //  ����ģ��ʧ�� AEmodN_for_RSA()
#define PKI_ERROR_RSA_RESULT_WRONG          0xFF03    //  RSA���������� 
#define PKI_ERROR_API_RETURN_FAIL           0xFF04    //  ����оƬAPI����ʧ�� 
#define PKI_ERROR_FILE_FORMAT               0xFF05    //  �ļ���ʽ�����ݴ��� 
#define PKI_ERROR_SESSION_KEY_LEN           0xFF06    //  Session Key���Ȳ���ȷ 
#define PKI_ERROR_ALG_NOT_SUPPORT           0xFF07    //  ��֧�ֵ���Կ�㷨 
#define PKI_ERROR_PINKEY_2_MACKEY           0xFF08    //  PIN��Կ����MAC��֤����Կ������֧�� 
#define PKI_ERROR_FILE_LENGTH               0xFF09    //  �ļ��������� 
#define PKI_ERROR_TEMP_SESSION_KEY_NO_PLACE 0xFF0A    //  g_session_key��û�п��пռ����ڲ�����Կ�� 
#define PKI_ERROR_KEY_CANNOT_EXPORT         0xFF0B    //  ��Կ���ܱ����� 
#define PKI_ERROR_ECC_RESULT_WRONG          0xFF0C    //  ECC���������� 
#define PKI_ERROR_ECC_VERSIG_FAIL           0xFF0D    //  ECCǩ����֤ʧ�� 

///////////////////////////////////////////////////////////////////////////////
//	��������
///////////////////////////////////////////////////////////////////////////////
#define DEVICE_MAX_QUANTITY			10	//����豸����
#define APPLICATION_MAX_QUANTITY	3	//���Ӧ������
#define CONTAINER_MAX_QUANTITY		6	//�����������
#define FILE_MAX_QUANTITY			20	//����ļ�����
#define KEY_MAX_QUANTITY			5	//���Ự��Կ���� С��RECORD_MAX_QUANTITY
#define NAME_ID_MAX_LEN				16	//���ơ�ID��󳤶�
//#define RECORD_MAX_QUANTITY		10	//��Կ�ļ�����¼��3f01 3f02
#define MAX_SYS_KEY_NUM				10	//���ϵͳ��Կ��3f01
#define MAX_USER_KEY_NUM			64	//����û���Կ��3f02
#define KEY_RETRY_MAX_TIMES			5	//��Կ��������Դ���
#define DEV_IN					1		//�豸����
#define DEV_OUT					2		//�豸�γ�

//������Կ��ʶ
 #define USBKEY_CRYPT_EXPORTABLE		0x00000001 
 #define USBKEY_CRYPT_CREATE_SALT		0x00000002 
 #define USBKEY_CRYPT_NO_SALT			0x00000004 
 #define USBKEY_CRYPT_USER_PROTECTED	0x00000008 
 #define USBKEY_CRYPT_ONDEVICE			0x00000020

//COS�㷨��ʶ
#define ALG_TYPE_DES			0x03
#define ALG_TYPE_3DES_2KEY		0x04
#define ALG_TYPE_3DES_3KEY		0x05
#define ALG_TYPE_SSF33			0x06
#define ALG_TYPE_SCB2			0x07

//���ܾ��㷨��ʶ
#define USBKEY_TYPE_SCB2		0x00000100
#define USBKEY_TYPE_SSF33		0x00000200
#define USBKEY_TYPE_RSA			0x00010000
#define USBKEY_TYPE_ECC			0x00020100

//�豸��Ϣ
#define DEVICEINFO_MANU_INFO	0x00020001		//������Ϣ
#define DEVICEINFO_DEV_LABEL	0x00020002		//�豸��ǩ
#define DEVICEINFO_DEV_VERSION  0x00020003		//�豸Ӳ���汾
#define DEVICEINFO_COS_VERSION  0x00020004		//cos�汾
#define DEVICEINFO_CUR_PRIV		0x00020005		//�豸��ǰȨ��
#define DEVICEINFO_CHIP_ID		0x00020006		//оƬid


//�豸״̬
#define DEVICE_IN				0x01		//�豸����
#define DEVICE_OUT				0x02		//�豸�γ�
#define DEVICE_ABSENT_STATE		0x00000000	//�豸������
#define DEVICE_PRESENT_STATE	0x00000001	//�豸����
#define USBKEY_INFINITE			INFINITE	//��ȡ����¼���ʱʱ��

//Ȩ��
#define USBKEY_USERTYPE_USER	0x00000010	//�û�
#define USBKEY_USERTYPE_ADMIN	0x00000001	//����Ա
#define USBKEY_USERTYPE_HIDISK_USER		0x00010000	//�������û�
#define USBKEY_USERTYPE_HIDISK_ADMIN	0x00001000	//����������
#define USBKEY_USERTYPE_HIDISK_TAX		0x00100000	//������˰��
#define USBKEY_ALL_PRIVILEGE	0x000000ff	//��Ȩ������
#define USBKEY_NEVER_USE		0x00000000	//������ʹ��					
#define USBKEY_DEVICE_PRIVILEGE	0x00000100	//�豸Ȩ��

//�豸����ԱPIN
#define DEVICE_ADMIN_PIN_LEN			16
#define	DEVICE_ADMIN_PIN_RETRY_TIMES	3

//�����ļ���
#define DIR_RECORD_FILE			0x8000 //Ŀ¼��¼�ļ�
#define FILE_RECORD_FILE		0x8001 //�ļ���¼�ļ�
#define CONTAINER_RECORD_FILE	0x8002 //Container��¼�ļ�
//#define DIR_RECORD_FILE 0x8003 //Ŀ¼��¼�ļ�

//��Կ��;
#define KEY_EXCHANGE					0x00000001
#define KEY_SIGNATURE					0x00000002
#define KEY_SESSION						0x00000003
#define KEY_INTERNAL_AUTHENTICATE_KEY	0x00000004
#define KEY_EXTERNAL_AUTHENTICATE_KEY	0x00000005
#define KEY_PIN							0x00000006

//оƬ����
#define CHIP_TYPE_20					0x00000001
#define CHIP_TYPE_AC3					0x00000002
#define CHIP_TYPE_45					0x00000003
#define CHIP_TYPE_AC4					0x00000005

//�������ģʽ
#define USBKEY_ECB						0x00000000		//ECBģʽ
#define USBKEY_CBC						0x00000010		//CBCģʽ

//�������ģʽ
#define USBKEY_NO_PADDING				0x00000000		//�����
#define USBKEY_PKCS5_PADDING			0x00000001		//PKCS5���

//RSA��Կ����
#define USBKEY_RSA_PUBLICKEY			0x00000001
#define USBKEY_RSA_PRIVATEKEY			0x00000002

//�ǶԳ���Կ����
#define USBKEY_PUBLIC_KEY				0x00000001
#define USBKEY_PRIVATE_KEY				0x00000002

//˽Կ��������
#define USBKEY_PRIKEY_TYPE_AI			0x00000000		//�Զ�������
#define USBKEY_PRIKEY_TYPE_CSP			0x00000001		//΢��CSP˽Կ���ĸ�ʽ

//#define USBKEY_MAX_COM_DATA_LEN			8*1024						//����������ݳ���
#define USBKEY_MAX_COM_DATA_LEN			254						//����������ݳ���(����IC��)
//#define USBKEY_MAX_COM_DATA_LEN			2048						//����������ݳ���
#define USBKEY_MAX_COM_LEN				(USBKEY_MAX_COM_DATA_LEN+7)	//��������

#define USBKEY_MAX_FILE_LEN				(256*256)					//����ļ�����
#define USBKEY_MAX_BLOCK_LEN			4096						//�ļ�����鳤

//˰��Ӧ��
#define	TAX_KEY_FLAG			0	//KEY_MAX_QUANTITY-1		//˰��ר����Կ��ʶ	add by zhoushenshen-20091030	Update by zhoushenshen-20100708

///////////////////////////////////////////////////////////////////////////////
//	���Ͷ���
///////////////////////////////////////////////////////////////////////////////
typedef unsigned int		USBKEY_UINT32;
typedef unsigned short		USBKEY_UINT16;
typedef unsigned char		USBKEY_UCHAR;
typedef char				USBKEY_CHAR;
typedef int					USBKEY_INT32;
typedef short				USBKEY_INT16;
typedef void				USBKEY_VOID;
typedef long				USBKEY_LONG;
typedef unsigned long		USBKEY_ULONG;
typedef USBKEY_UINT32*		USBKEY_UINT32_PTR;
typedef USBKEY_UINT16*		USBKEY_UINT16_PTR;
typedef USBKEY_UCHAR*		USBKEY_UCHAR_PTR;
typedef USBKEY_CHAR*		USBKEY_CHAR_PTR;
typedef USBKEY_INT32*		USBKEY_INT32_PTR;
typedef USBKEY_INT16*		USBKEY_INT16_PTR;
typedef USBKEY_VOID*		USBKEY_VOID_PTR;
typedef USBKEY_VOID_PTR		USBKEY_HANDLE;
typedef USBKEY_LONG *		USBKEY_LONG_PTR;
typedef USBKEY_ULONG *		USBKEY_ULONG_PTR;

typedef struct
{
	unsigned char Eventflag;		//�¼�����
	unsigned char *pcDeviceId;		//�����¼����豸��ID
}DEVICE_EVENT;

typedef struct
{
	unsigned int  bits;                 		/* length in bits of modulus */
	unsigned char publicKeyID[2];  				/* ��ԿID */
	unsigned char privateKeyID[2]; 				/* ˽ԿID */
} USBKEY_RSA_KEY;

typedef struct
{
	unsigned int  bits;                 		/* length in bits of modulus */
	unsigned char publicKeyID[2];  				/* ��ԿID */
	unsigned char privateKeyID[2]; 				/* ˽ԿID */
}USBKEY_KEY_PAIR_ID;

//	RSA modulus
#define MAX_RSA_MODULUS_LEN_1024	128		//1024 bits
#define MAX_RSA_PRIME_LEN_1024		64
#define MAX_RSA_MODULUS_LEN_2048	256		//2048 bits
#define MAX_RSA_PRIME_LEN_2048		128

typedef struct
{
	unsigned int  bits;                 		/* length in bits of modulus */
	unsigned char modulus[MAX_RSA_MODULUS_LEN_1024];  	/* modulus */
	unsigned char exponent[MAX_RSA_MODULUS_LEN_1024]; 	/* public exponent */
} USBKEY_RSA_PUBLIC_KEY_1024;

typedef struct 
{
	unsigned int  bits;                           	/* length in bits of modulus */
	unsigned char modulus[MAX_RSA_MODULUS_LEN_1024];       	/* n */
	unsigned char publicExponent[MAX_RSA_MODULUS_LEN_1024];	/* e */
	unsigned char prime[2][MAX_RSA_PRIME_LEN_1024];        	/* p,q */
	unsigned char primeExponent[2][MAX_RSA_PRIME_LEN_1024];	/* dp,dq */
	unsigned char coefficient[MAX_RSA_PRIME_LEN_1024];      	/* qInv */
} USBKEY_RSA_PRIVATE_KEY_1024;

typedef struct
{
	unsigned int  bits;                 		/* length in bits of modulus */
	unsigned char modulus[MAX_RSA_MODULUS_LEN_2048];  	/* modulus */
	unsigned char exponent[MAX_RSA_MODULUS_LEN_2048]; 	/* public exponent */
} USBKEY_RSA_PUBLIC_KEY_2048;

typedef struct 
{
	unsigned int  bits;                           	/* length in bits of modulus */
	unsigned char modulus[MAX_RSA_MODULUS_LEN_2048];       	/* n */
	unsigned char publicExponent[MAX_RSA_MODULUS_LEN_2048];	/* e */
	unsigned char prime[2][MAX_RSA_PRIME_LEN_2048];        	/* p,q */
	unsigned char primeExponent[2][MAX_RSA_PRIME_LEN_2048];	/* dp,dq */
	unsigned char coefficient[MAX_RSA_PRIME_LEN_2048];      	/* qInv */
} USBKEY_RSA_PRIVATE_KEY_2048;

typedef struct 
{
	unsigned int  bits;                           	/* length in bits of modulus */
	unsigned char prime[2][MAX_RSA_PRIME_LEN_1024];        	/* p,q */
	unsigned char primeExponent[2][MAX_RSA_PRIME_LEN_1024];	/* dp,dq */
	unsigned char coefficient[MAX_RSA_PRIME_LEN_1024];      	/* qInv */
} USBKEY_RSA_PRIVATE_KEY_IMPORT_1024;

typedef struct 
{
	unsigned int  bits;                           	/* length in bits of modulus */
	unsigned char prime[2][MAX_RSA_PRIME_LEN_2048];        	/* p,q */
	unsigned char primeExponent[2][MAX_RSA_PRIME_LEN_2048];	/* dp,dq */
	unsigned char coefficient[MAX_RSA_PRIME_LEN_2048];      	/* qInv */
} USBKEY_RSA_PRIVATE_KEY_IMPORT_2048;

////////////////////////////////////////////////////////////////////////////
#define MAX_ECC_XCOORDINATE_BITS_LEN			512
#define MAX_ECC_YCOORDINATE_BITS_LEN			512
#define MAX_ECC_MODULUS_BITS_LEN				512

typedef struct
{
	USBKEY_ULONG  bits;
	USBKEY_UCHAR XCoordinate[MAX_ECC_XCOORDINATE_BITS_LEN/8];
	USBKEY_UCHAR YCoordinate[MAX_ECC_YCOORDINATE_BITS_LEN/8];
}USBKEY_ECC_PUBLIC_KEY;

typedef struct
{
	USBKEY_ULONG  bits;
	USBKEY_UCHAR PrivateKey[MAX_ECC_MODULUS_BITS_LEN/8];
}USBKEY_ECC_PRIVATE_KEY;

//////////////////////////////////////////////////////////////////////////

typedef struct {
	UINT8 ApplicationType;//��֤��Կ��Ӧ�����ͣ��ⲿ��֤��Կ���ڲ���֤��Կ��	�ڲ���֤��Կ��AT_INTERNAL_AUTHENTICATE_KEY�ⲿ��֤��Կ��AT_EXTERNAL_AUTHENTICATE_KEY
	UINT8 ulAlg;			//��֤�㷨����
	ULONG ulPrivil;	//��֤��ԿȨ��
	UINT8 KeyLen;//��Կֵ���ȣ����ֽڼ���
	UINT8 ErrorCounter;//�����������
}USBKEY_KEYHEAD, *PUSBKEY_KEYHEAD;

typedef struct {
	USBKEY_UCHAR major;
	USBKEY_UCHAR minor;
}USBKEY_VERSION;

typedef struct {
	USBKEY_UCHAR Manufacturer[64];		//�豸����Ϣ
	USBKEY_UCHAR Issuer[64];			//��������Ϣ
	USBKEY_UCHAR Label[64];				//�豸��ǩ
	USBKEY_UCHAR SerialNumber[32];		//���к�
	USBKEY_VERSION HWVersion;			//Ӳ���汾
	USBKEY_VERSION FirmwareVersion;	//�豸�̼��汾
	USBKEY_VERSION AppVersion;			//Ӧ�ð汾
	USBKEY_UCHAR   MinPINLen;			//���PIN����
	USBKEY_UCHAR   MaxPINLen;			//���PIN����
	USBKEY_ULONG  AlgSymCap;			//���������㷨��ʶ
	USBKEY_ULONG  AlgAsymCap;			//�ǶԳ������㷨��ʶ
	USBKEY_ULONG  DevAuthAlgId;		//�豸��֤ʹ�õķ��������㷨��ʶ
	USBKEY_ULONG  TotalMemory;			//�豸�ܿռ��С
	USBKEY_ULONG  FreeMemory;			//�û����ÿռ��С
	USBKEY_ULONG  Reserved;				//������չ��LOGIN ��Ϣ
}USBKEY_DEVINFO,*PUSBKEY_DEVINFO;

typedef struct {
	CHAR	FileName[32];
	ULONG	FileSize;
	ULONG	ReadRights;
	ULONG	WriteRights;
} USBKEY_FILEATTRIBUTE, *PUSBKEY_FILEATTRIBUTE;



///////////////////////////////////////////////////////////////////////////////
//	API
///////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
	extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////
//	�ײ��豸���API
///////////////////////////////////////////////////////////////////////////////
//	�豸����¼�����	USBKEY_WaitForDevEvent
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_WaitForDevEvent(
	ULONG ulTimeOut,		//[IN]  ��ʱʱ�䣬��λ΢��,0����������USBKEY_INFINITE������
	LPSTR szDevName, 		//[OUT] �����¼����豸����
	ULONG *pulDevNameLen, 	//[IN,OUT] IN�����������ȣ�OUT���豸���Ƶ���Ч���ȣ����Ȱ����ַ���������
	ULONG *pulEvent);		//[OUT] �¼����͡�1��ʾ���룬2��ʾ�γ�

//	ȡ���ȴ��豸����¼�	USBKEY_CancelWaitForDevEvent
//	����ֵ��
//		0��	OK
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_CancelWaitForDevEvent();

//	ö�ټ�����������ӵ��豸��ID
//	���ppcDeviceIdSΪNULL�����pdwDeviceQuantity������Ҫ����char *[NAME_ID_MAX_LEN]����
//	���ppcDeviceIdS�ǿգ����ppcDeviceIdS���������豸��ID��
//		����pdwDeviceQuantity����ʵ���豸����
//	����ֵ��
//		0��	OK
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DeviceEnum(
	USBKEY_CHAR			ppcDeviceIdS[DEVICE_MAX_QUANTITY][MAX_PATH],			//[out]�豸ID���б�
	USBKEY_INT32_PTR	piDeviceQuantity);	//[out]�豸ID�ĸ���

//	����豸�Ƿ����
//	DEVICE_PRESENT_STATEΪ���ڣ�DEVICE_ABSENT_STATE��ʾ�豸������
//	����ֵ��
//		0��	OK
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DeviceCheckExist(
	USBKEY_CHAR_PTR pcDeviceId,	//[in]�豸��ID
	USBKEY_ULONG_PTR phDevState);	//[out]�豸״̬

//	��ĳ���豸
//	���ucShareMode=1 ��ʾ����ģʽ ����Ӧ�ó�����Է����豸
//	����ֵ��
//		0��	OK
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DeviceOpen(
	USBKEY_CHAR_PTR pcDeviceId,		//[in]�豸��ID
	USBKEY_INT32	iShareMode,		//[in]����ģʽ
	USBKEY_HANDLE	*phDeviceHandle);	//[out]�豸���

//	�ر�ĳ���豸
//	����ֵ��
//		0��	OK
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DeviceClose(
	USBKEY_HANDLE	hDeviceHandle );	//[in]�豸���
	
//	���õ�ǰ���ӵ�ĳ���豸��ʹ�豸�����ʼ״̬���豸�����һ�θձ��򿪣�
//	����ֵ��
//		0��	OK
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DeviceReset(
	USBKEY_HANDLE	hDeviceHandle		//[in]�豸���
	);		
	

//	��ʼ����ǰ���ӵ�ĳ���豸.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DeviceInit(
		USBKEY_HANDLE		hDeviceHandle,	//[in]�豸���
		USBKEY_ULONG		ulChipType,		//[in]оƬ���ͣ�CHIP_TYPE_20��CHIP_TYPE_45
		USBKEY_VERSION		appVer,			//Ӧ�ð汾
		USBKEY_UCHAR_PTR	pcManufacturer,	//�������ƣ��64���ַ�������64���ַ��Կհ��ַ�(ASCII��Ϊ0x20)��䣬������null��0x00��������
		USBKEY_UCHAR_PTR	pcLabel);		//�豸��ǩ���64���ַ�������64���ַ��Կհ��ַ�(ASCII��Ϊ0x20)��䣬������null��0x00��������


//	��ȡ��ǰ���ӵ�ĳ���豸��״̬.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DeviceGetStat(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_DEVINFO* pcDeviceInfo);//[Out]���ص���Ϣ�����սṹ��DEVINFO����

//	�����豸��ǩ����ǩ��СС��32�ֽ�
//	����ֵ��
//		0��	OK
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DeviceSetLabel(
	USBKEY_HANDLE	hDeviceHandle,		//[in]�豸���
	USBKEY_UCHAR_PTR pLabel);			//[in]�豸��ǩ�ַ��������ַ���ӦС��32�ֽ�

//	�����豸����
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_TransmitDeviceCmd(
	USBKEY_HANDLE		hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR	pbCommand,		//[in]�豸����
	USBKEY_ULONG		ulCommandLen,	//[in]�����	
	USBKEY_UCHAR_PTR	pbResponse,		//[out]��Ӧ����
	USBKEY_ULONG		*pulResponseLen);	//[in out]in: ��Ӧbuffer�ĳ��ȣ�out: ��Ӧ����

//	��֤�豸PIN.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_CheckDeviceAdminPin(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR pcPIN,			//[in]�����PINֵ
	USBKEY_ULONG ulLen,				//[in]�����PIN����
	USBKEY_ULONG_PTR pulRetryNum);	//[out]��������Դ���

//	�޸��豸PIN.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ChangeDeviceAdminPin(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR pcOldPIN,		//[in]�����ԭPIN
	USBKEY_ULONG ulOldLen,			//[in]�����ԭPIN����
	USBKEY_UCHAR_PTR pcNewPIN,		//[in]�������PIN
	USBKEY_ULONG ulNewLen,			//[in]�������PIN����
	USBKEY_ULONG_PTR pulRetryNum);	//[out]��������Դ���
///////////////////////////////////////////////////////////////////////////////
//	���ʿ������API
///////////////////////////////////////////////////////////////////////////////
//	��USBKEYд���ڲ���֤�����ⲿ��֤��Կ.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_WriteKey(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR pcKeyHead,		//[in]��Կͷ����Ϣ
	USBKEY_UCHAR_PTR pcKeyBody);	//[in]��Կֵ

//	�ڲ���֤���ն˶��豸����֤.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_IntAuthenticate(
	USBKEY_HANDLE hDeviceHandle,		//[in]�豸���
	USBKEY_UCHAR_PTR pcRandomNumber,	//[in]����������
	USBKEY_ULONG ulRandomNumberLen,		//[in]��������������
	USBKEY_UCHAR_PTR pcEncryptNumber,	//[out]���ܺ�������
	USBKEY_ULONG_PTR pulEncryptNumberLen);//[out]���ص����������

//	�ⲿ��֤.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ExtAuthenticate(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR pcRandomNumber,//[in]������ⲿ��֤��Կ���ܺ�������
	USBKEY_ULONG ulRandomNumberLen);	//[in]��������������

//	��ʼ���û�PIN.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_InitPin(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR pcPIN,		//[in]�����PIN
	USBKEY_ULONG ulLen);	//[in]PIN����

//	�޸�PIN.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ChangePin(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_ULONG ulPINType,			//[in]PIN����: USBKEY_USERTYPE_ADMIN �� USBKEY_USERTYPE_USER
	USBKEY_UCHAR_PTR pcOldPIN,		//[in]�����ԭPIN
	USBKEY_ULONG ulOldLen,			//[in]�����ԭPIN����
	USBKEY_UCHAR_PTR pcNewPIN,		//[in]�������PIN
	USBKEY_ULONG ulNewLen,			//[in]�������PIN����
	USBKEY_ULONG_PTR pulRetryNum);	//[out]��������Դ���

//	��֤PIN.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_CheckPin(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_ULONG ulPINType,			//[in]PIN����
	USBKEY_UCHAR_PTR pcPIN,			//[in]�����PINֵ
	USBKEY_ULONG ulLen,				//[in]�����PIN����
	USBKEY_ULONG_PTR pulRetryNum);	//[out]��������Դ���

//	�����û�PIN.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_UnlockPin(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR pcAdminPIN,	//[in]����Ĺ���ԱPIN
	USBKEY_ULONG ulAdminLen,		//[in]�����PIN����
	USBKEY_UCHAR_PTR pcUserNewPIN,	//[in]������û���PIN
	USBKEY_ULONG ulUserNewLen,		//[in]������û���PIN����
	USBKEY_ULONG_PTR pulRetryNum);	//[out]��������Դ���

//��ȡPIN��Ϣ 
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_GetPinInfo(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_ULONG ulPINType,			//[in]PIN����
	USBKEY_ULONG_PTR puMaxRetryNum,	//[out]������Դ���
	USBKEY_ULONG_PTR pulRemainRetryNum);	//[out]ʣ�����Դ���

///////////////////////////////////////////////////////////////////////////////
//	�ļ��������API
///////////////////////////////////////////////////////////////////////////////
//	����Ӧ��.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_CreateApplication(
	USBKEY_HANDLE hDeviceHandle,		//[in]�豸���
	USBKEY_UCHAR_PTR pcAppName,			//[in]�����Ӧ������
	USBKEY_UCHAR_PTR pcAdminPIN,		//[in]����Ĺ���ԱPIN
	USBKEY_ULONG ulAdminPINLen,			//[in]����Ĺ���ԱPIN����
	USBKEY_UCHAR ulAdminPinRetryCount,	//[in]����ԱPIN������Դ���
	USBKEY_UCHAR_PTR pcUserPIN,			//[in]������û�PIN
	USBKEY_ULONG ulUserPINLen,			//[in]������û�PIN����
	USBKEY_UCHAR ulUserPinRetryCount,	//[in]�û�PIN������Դ���
	USBKEY_ULONG ulCreateFileRight);	//[in]�ڸ�Ӧ���´����ļ���������Ȩ��

//	ɾ��Ӧ��.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DeleteApplication(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR pcAppName);		//[in]�����Ӧ������

//	ö��Ӧ��.
//	���pcApplicationNameListΪNULL�����pulSize������Ҫ����ռ�������pulAppCount����Ӧ�õĸ���
//	���pcApplicationNameList�ǿգ����pcApplicationNameList����Ӧ���б�Ӧ�ü���'\0'�ָ���
//		����pulSize����ʵ��ʹ�ÿռ��С��pulAppCount����Ӧ�õĸ���
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_EnumApplication(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR pcApplicationNameList,		//[in,out]Ӧ����Ϣ�б�
	USBKEY_ULONG_PTR pulSize,		//[in,out]����Ϊ��������С�����Ϊʵ�ʿռ��С
	USBKEY_ULONG_PTR pulAppCount);		//[out]���Ӧ�ø���

//	��Ӧ��.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_OpenApplication(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR pcAppName);	//[in]�����Ӧ������

//	�����ļ�.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_CreateFile(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR pcFileName,	//[in]�ļ�����
	USBKEY_ULONG ulFileSize,		//[in]�ļ���С
	USBKEY_ULONG ulReadRights,		//[in]�ļ���Ȩ��
	USBKEY_ULONG ulWriteRights);	//[in]�ļ�дȨ��

//	ɾ���ļ�.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DeleteFile(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR pcFileName);	//[in]�ļ�����

//	ö���ļ�.
//	���pcFileNameListΪNULL�����pulSize������Ҫ����ռ�����
//	���pcFileNameList�ǿգ����pcFileNameList�����ļ��б��ļ�������'\0'�ָ���
//		����pulSize����ʵ��ʹ�ÿռ�����
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_EnumFile(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR pcFileNameList,		//[in,out]�ļ���Ϣ�б�
	USBKEY_ULONG_PTR pulSize);	//[in,out]����Ϊ��������С�����Ϊʵ��ʹ�ÿռ��С

//	��ȡ�ļ���Ϣ.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_GetFileAttribute(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR pcFileName,	//[in]�ļ�����
	USBKEY_UCHAR_PTR pcFileInfo);	//[out]�ļ���Ϣ�����սṹ��FILEATTRIBUTE����

//	��ȡ�ļ�.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ReadFile(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR pcFileName,	//[in]�ļ�����
	USBKEY_ULONG ulOffset,			//[in]��ȡƫ����
	USBKEY_UCHAR_PTR pcOutData,		//[out]��������
	USBKEY_ULONG_PTR pulSize);		//[in,out]�����ȡ���ݻ�������С�����ʵ�ʶ�ȡ����

//	д�ļ�.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_WriteFile(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR pcFileName,	//[in]�ļ�����
	USBKEY_ULONG ulOffset,			//[in]д��ƫ����
	USBKEY_UCHAR_PTR pcData,		//[in]д������
	USBKEY_ULONG_PTR pulSize);		//[in,out]����д�����ݻ�������С�����ʵ��д���С


///////////////////////////////////////////////////////////////////////////////
//	�㷨�������API
///////////////////////////////////////////////////////////////////////////////
//	������Կ����.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_CreateContainer(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR pcContainerName);		//[in]�������Կ��������

//	ɾ����Կ����.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DeleteContainer(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR pcContainerName);	//[in]��Կ��������

//	�ж��Ƿ�Ϊ������.
//	����ֵ��
//		0:	Ϊ��
//		-1:��Ϊ��
//		>0:������
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_JudgeEmptyContainer(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR pcContainerName);	//[in]��Կ��������

//	ö����Կ����.
//	���pcContianerNameListΪNULL�����pulSize������Ҫ����ռ�����
//	���pcContianerNameList�ǿգ����pcContianerNameList������Կ�����б�ID����'\0'�ָ���
//		����pulSize����ʵ��ʹ�ÿռ��С
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_EnumContainer(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR pcContianerNameList,		//[in,out]��Կ���������б�
	USBKEY_ULONG_PTR pulSize);	//[in,out]����Ϊ��������С�����Ϊʵ��ʹ�ÿռ��С

//	���������.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_GenRandom(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR pcRandom,		//[out]���ص������
	USBKEY_ULONG ulRandomLen);		//[in]��Ҫ�����������

//	�����Գ���Կ.
//	����ֵ:
//		0:	����
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_GenSymmetricKey(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_ULONG ulKeyLen,			//[in]��Կ����
	USBKEY_ULONG ulAlgID,			//[in]��Կ�㷨��ʾ
	USBKEY_ULONG ulGenKeyFlags,		//[in]������Կ�Ŀ��Ʊ�־
	USBKEY_UCHAR_PTR phKey);		//[out]���ص���Կ���

//	���������Գ���Կ.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_GenContainerSymmetricKey(
	USBKEY_HANDLE hDeviceHandle,		//[in]�豸���
	USBKEY_UCHAR_PTR pcContainerName,	//[in]��Կ��������
	USBKEY_ULONG ulKeyLen,				//[in]��Կ����
	USBKEY_ULONG ulAlgID,				//[in]��Կ�㷨��ʾ
	USBKEY_ULONG ulGenKeyFlags,			//[in]������Կ�Ŀ��Ʊ�־
	USBKEY_UCHAR_PTR phKey);			//[out]���ص���ԿID

//	ɾ�������Գ���Կ.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DeleteContainerSymmetricKey(
	USBKEY_HANDLE hDeviceHandle,		//[in]�豸���
	USBKEY_UCHAR_PTR pcContainerName,	//[in]��Կ��������
	USBKEY_UCHAR_PTR phKey);				//[in]��ԿID

//	����RSA�ǶԳ���Կ.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_GenerateRSAKeyPair(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR pcContainerName,//[in]��Կ��������
	USBKEY_ULONG ulKeyUsage,		//[in]��Կ��;��KEY_EXCHANGE��Կ����KEY_SIGNATURE��Կ
	USBKEY_ULONG ulBitLen,			//[in]��Կģ��
	USBKEY_ULONG ulGenKeyFlags,		//[in]������Կ�Ŀ��Ʊ�־
	USBKEY_UCHAR_PTR pcPubKeyID,	//[out]���صĹ�ԿID
	USBKEY_UCHAR_PTR pcPriKeyID);	//[out]���ص�˽ԿID

//	��ȡ�ǶԳ���ԿID.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_GetAsymmetricKeyID(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR pcContainerName,//[in]��Կ��������
	USBKEY_ULONG ulKeyUsage,		//[in]��Կ��;
	USBKEY_UCHAR_PTR pcKeyPair);		//[out]���ص���Կ�Ծ��������USBKEY_KEY_PAIR_ID����

//	��ȡ�ǶԳ�˽Կģ��.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_GetPriKeyBitLen(
 	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR pcContainerName,//[in]��Կ��������
	USBKEY_ULONG ulKeyUsage,		//[in]��Կ��;
	USBKEY_ULONG_PTR pulBitLen);		//[out]���ص�˽Կ��ģ��

//	�����Գ���Կ.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ExportSymmetricKey(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR pcSymKeyID,	//[in]�������Գ���Կ
	USBKEY_UCHAR_PTR pcWrapKeyID,	//[in]��������������Կ����ԿID
	USBKEY_ULONG ulAsymAlgID,		//[in]��������������Կ����Կ�㷨��USBKEY_TYPE_RSA����USBKEY_TYPE_ECC
	USBKEY_UCHAR_PTR pcSymKey,		//[out]���ص���Կ
	USBKEY_ULONG_PTR pulSymKeyLen);	//[out]���ص���Կ����


//	����Գ���Կ.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ImportSymmetricKey(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR pcWrapKeyID,	//[in]��������������Կ����ԿID
	USBKEY_ULONG ulAsymAlgID,		//[in]��������������Կ����Կ�㷨��USBKEY_TYPE_RSA����USBKEY_TYPE_ECC
	USBKEY_ULONG ulKeyType,			//[in]��Կ���ͣ���ʾ��Կ�ǿ��ڹ̶���Կ������ʱ��Կ����ָ��ΪUSBKEY_CRYPT_ONDEVICE����Ϊ0
	USBKEY_ULONG ulSymAlgID,		//[in]��Կ�㷨��ʶ
	USBKEY_UCHAR_PTR pcSymKey,		//[in]�������Կ
	USBKEY_ULONG ulSymKeyLen,		//[in]�������Կ����
	USBKEY_UCHAR_PTR pcSymKeyID);	//[out]���ص���ĶԳ���ԿID

//	���������Գ���Կ.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ImportContainerSymmetricKey(
	USBKEY_HANDLE hDeviceHandle,		//[in]�豸���
	USBKEY_UCHAR_PTR pcContainerName,	//[in]��Կ��������
	USBKEY_UCHAR_PTR pcWrapKeyID,		//[in]��������������Կ����ԿID
	USBKEY_ULONG ulAsymAlgID,			//[in]��������������Կ����Կ�㷨��USBKEY_TYPE_RSA����USBKEY_TYPE_ECC
	USBKEY_ULONG ulKeyType,				//[in]��Կ���ͣ���ʾ��Կ�ǿ��ڹ̶���Կ������ʱ��Կ����ָ��ΪUSBKEY_CRYPT_ONDEVICE����Ϊ0
	USBKEY_ULONG ulAlgID,				//[in]��Կ�㷨��ʶ
	USBKEY_UCHAR_PTR pcSymKey,			//[in]�������Կ
	USBKEY_ULONG ulSymKeyLen,			//[in]�������Կ����
	USBKEY_UCHAR_PTR pcSymKeyID);		//[out]���ص���ĶԳ���ԿID

//	��ȡRSA��Կ.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ExportRSAPublicKey(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR pcPubKeyID,	//[in]��������Կ
	USBKEY_UCHAR_PTR pcPubKey,		//[out]���ص���Կ
	USBKEY_ULONG_PTR pulPubKeyLen);	//[out]���ص���Կ����

/*//	����RSA˽Կ.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ImportRSAPrivateKey(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR pcAppName,		//[in]Ӧ������
	USBKEY_UCHAR_PTR pcWrapKeyID,	//[in]��������������Կ����ԿID
	USBKEY_UCHAR_PTR pcContainerName,	//[in]��Կ����
	USBKEY_ULONG ulKeyUsage,		//[in]��Կ��;��KEY_EXCHANGE��Կ����KEY_SIGNATURE��Կ
	USBKEY_UCHAR_PTR pcRSAKeyPair,	//[in]�������Կ
	USBKEY_ULONG ulRSAKeyPairLen,	//[in]�������Կ����
	USBKEY_UCHAR_PTR pcPriKeyID);	//[out]���ص�˽ԿID*/

//	����RSA˽Կ.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ImportRSAPrivateKey(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_ULONG ulKeyType,			//[in]˽Կ���ͣ�USBKEY_PRIKEY_TYPE_CSP,USBKEY_PRIKEY_TYPE_AI
	USBKEY_UCHAR_PTR pcWrapKeyID,	//[in]��������������Կ����ԿID
	USBKEY_ULONG ulMode,			//[in]����ģʽ��USBKEY_ECB, USBKEY_CBC(����Ϊ�����),c���ޱ�����ԿID����ΪECBģʽ
	USBKEY_UCHAR_PTR pcIV,			//[in]��ʼ�������������ecbģʽ������ΪNULL
	USBKEY_ULONG ulIVLen,			//[in]��ʼ���������ȣ������ecbģʽ������Ϊ0
	USBKEY_UCHAR_PTR pcContainerName,//[in]��Կ����
	USBKEY_ULONG ulKeyUsage,		//[in]��Կ��;��KEY_EXCHANGE��Կ����KEY_SIGNATURE��Կ
	USBKEY_UCHAR_PTR pcRSAKeyPair,	//[in]�������Կ
	USBKEY_ULONG ulRSAKeyPairLen,	//[in]�������Կ����
	USBKEY_UCHAR_PTR pcPriKeyID);	//[out]���ص�˽ԿID

//	����RSA��Կ.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ImportRSAPublicKey(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR pcContainerName,	//[in]��Կ����
	USBKEY_ULONG ulKeyUsage,		//[in]��Կ��;��KEY_EXCHANGE��Կ����KEY_SIGNATURE��Կ
	USBKEY_UCHAR_PTR pcRSAPublicKey,//[in]�������Կ
	USBKEY_ULONG ulECCPublicKeyLen,	//[in]�������Կ����
	USBKEY_UCHAR_PTR pcPubKeyID);	//[out]���صĹ�ԿID

//	����RSA��˽Կ��.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ImportRSAKeyPair(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR pcWrapKeyID,	//[in]��������������Կ����ԿID
	USBKEY_ULONG ulMode,			//[in]����ģʽ(����Ϊ�����)��USBKEY_ECB �� USBKEY_CBC,���ޱ�����ԿID����ΪECBģʽ
	USBKEY_UCHAR_PTR pcIV,			//[in]��ʼ�������������ecbģʽ������ΪNULL
	USBKEY_ULONG ulIVLen,			//[in]��ʼ���������ȣ������ecbģʽ������Ϊ0
	USBKEY_UCHAR_PTR pcContainerName,//[in]��Կ����
	USBKEY_ULONG ulKeyUsage,		//[in]��Կ��;��KEY_EXCHANGE��Կ����KEY_SIGNATURE��Կ
	USBKEY_UCHAR_PTR pcRSAKeyPair,	//[in]�������Կ
	USBKEY_ULONG ulRSAKeyPairLen,	//[in]�������Կ����
	USBKEY_UCHAR_PTR pcPriKeyID,	//[out]���ص�˽ԿID
	USBKEY_UCHAR_PTR pcPubKeyID);	//[out]���صĹ�ԿID

//	RSA˽Կ����.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_RSAPrivateKeyOperation(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR pcKeyID,		//[in]�����������ԿID
	USBKEY_UCHAR_PTR pcInData,		//[in]��������
	USBKEY_ULONG ulInDataLen,		//[in]�������ݳ���
	USBKEY_UCHAR_PTR pcOutData,		//[out]���ص����
	USBKEY_ULONG_PTR pulOutDataLen);//[out]���ص��������

//	RSA��Կ����.
//	����ֵ��
//		0��	ǩ����֤�ɹ�.
//		>0:	ǩ����֤ʧ�ܣ����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_RSAPublicKeyOperation(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR pcKeyID,		//[in]�����������ԿID
	USBKEY_UCHAR_PTR pcInData,		//[in]��������
	USBKEY_ULONG ulInDataLen,		//[in]�������ݳ���
	USBKEY_UCHAR_PTR pcOutData,		//[out]�������
	USBKEY_ULONG_PTR pulOutDataLen);//[out]������ݳ���

//	�ԳƼ���.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DataEncrypt(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR pcKeyID,		//[in]�������ܵ���ԿID
	USBKEY_UCHAR_PTR pcData,		//[in]����������
	USBKEY_ULONG ulDataLen,			//[in]���������ݳ���
	USBKEY_UCHAR_PTR pcEncryptedData,//[out]���صļ��ܺ�����
	USBKEY_ULONG_PTR pulEncryptedLen);//[out]���صļ��ܺ����ݳ���

//	�Գƽ���.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DataDecrypt(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR pcKeyID,		//[in]�������ܵ���ԿID
	USBKEY_UCHAR_PTR pcEncryptedData,//[in]��������
	USBKEY_ULONG ulEncryptedLen,	//[in]�������ݳ���
	USBKEY_UCHAR_PTR pcData,		//[out]���ص�����
	USBKEY_ULONG_PTR pulDataLen);	//[out]���ص����ĳ���

//	�����ԳƼ���.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_MassiveDataEncrypt(
	USBKEY_HANDLE		hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR	pcKeyID,		//[in]�������ܵ���ԿID
	USBKEY_ULONG		ulAlg,			//[in]�����㷨
	USBKEY_ULONG		ulMode,			//[in]����ģʽ�����ģʽ��USBKEY_ECB|USBKEY_NO_PADDING, USBKEY_ECB|USBKEY_PKCS5_PADDING, USBKEY_CBC|USBKEY_NO_PADDING, USBKEY_CBC|USBKEY_PKCS5_PADDING
	USBKEY_UCHAR_PTR	pcIV,			//[in]��ʼ�������������ecbģʽ������ΪNULL
	USBKEY_UCHAR_PTR	pcData,			//[in]����������
	USBKEY_ULONG		ulDataLen,		//[in]���������ݳ���
	USBKEY_UCHAR_PTR	pcEncryptedData,//[out]���صļ��ܺ�����
	USBKEY_ULONG_PTR	pulEncryptedLen);//[in/out]in: pcEncryptedData��������С��out: ���ܺ����ݳ��Ȼ���Ҫ��pcEncryptedData��������С

//	�����Գƽ���.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_MassiveDataDecrypt(
	USBKEY_HANDLE		hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR	pcKeyID,		//[in]�������ܵ���ԿID
	USBKEY_ULONG		ulAlg,			//[in]�����㷨
	USBKEY_ULONG		ulMode,			//[in]����ģʽ�����ģʽ��USBKEY_ECB|USBKEY_NO_PADDING, USBKEY_ECB|USBKEY_PKCS5_PADDING, USBKEY_CBC|USBKEY_NO_PADDING, USBKEY_CBC|USBKEY_PKCS5_PADDING
	USBKEY_UCHAR_PTR	pcIV,			//[in]��ʼ�������������ecbģʽ������ΪNULL
	USBKEY_UCHAR_PTR	pcEncryptedData,//[in]��������
	USBKEY_ULONG		ulEncryptedLen,	//[in]�������ݳ���
	USBKEY_UCHAR_PTR	pcData,			//[out]���ص�����
	USBKEY_ULONG_PTR	pulDataLen);	//[in/out]in: pcData���������� out:���ص����ĳ���

//	��ʱ�Գ���Կ����	
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_TempDataEncrypt(
	USBKEY_HANDLE		hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR	pcKey,			//[in]�������ܵ���Կ
	USBKEY_ULONG		ulAlg,			//[in]�����㷨
	USBKEY_ULONG		ulMode,			//[in]����ģʽ�����ģʽ��USBKEY_ECB|USBKEY_NO_PADDING, USBKEY_ECB|USBKEY_PKCS5_PADDING, USBKEY_CBC|USBKEY_NO_PADDING, USBKEY_CBC|USBKEY_PKCS5_PADDING
	USBKEY_UCHAR_PTR	pcIV,			//[in]��ʼ�������������ecbģʽ������ΪNULL
	USBKEY_UCHAR_PTR	pcData,			//[in]����������
	USBKEY_ULONG		ulDataLen,		//[in]���������ݳ���
	USBKEY_UCHAR_PTR	pcEncryptedData,//[out]���صļ��ܺ�����
	USBKEY_ULONG_PTR	pulEncryptedLen);//[in/out]in: pcEncryptedData��������С��out: ���ܺ����ݳ��Ȼ���Ҫ��pcEncryptedData��������С

//	��ʱ�Գ���Կ����
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_TempDataDecrypt(
	USBKEY_HANDLE		hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR	pcKey,			//[in]�������ܵ���Կ
	USBKEY_ULONG		ulAlg,			//[in]�����㷨
	USBKEY_ULONG		ulMode,			//[in]����ģʽ�����ģʽ��USBKEY_ECB|USBKEY_NO_PADDING, USBKEY_ECB|USBKEY_PKCS5_PADDING, USBKEY_CBC|USBKEY_NO_PADDING, USBKEY_CBC|USBKEY_PKCS5_PADDING
	USBKEY_UCHAR_PTR	pcIV,			//[in]��ʼ�������������ecbģʽ������ΪNULL
	USBKEY_UCHAR_PTR	pcEncryptedData,//[in]��������
	USBKEY_ULONG		ulEncryptedLen,	//[in]�������ݳ���
	USBKEY_UCHAR_PTR	pcData,			//[out]���ص�����
	USBKEY_ULONG_PTR	pulDataLen);	//[in/out]in: pcData���������� out:���ص����ĳ���

//	������ʱRSA�ǶԳ���Կ�����洢.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_GenerateExtRSAKeyPair(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_ULONG ulBitLen,			//[in]��Կģ��
	USBKEY_UCHAR_PTR pPriKeyStrc);	//[out]���ص�˽Կ�ṹ
	
/*//	����1024��ʱRSA�ǶԳ���Կ.������ΪUSBKEY_GenerateExtRSAKey��
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_GenerateTempRSAKeyPair(
	USBKEY_HANDLE hDeviceHandle);	//[in]�豸���*/

//	������ʱRSA��Կ.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ImportTempRSAPublicKey(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR pcRSAPublicKey,//[in]�������Կ
	USBKEY_ULONG ulRSAPublicKeyLen,	//[in]�������Կ����
	USBKEY_UCHAR_PTR pcPubKeyID);	//[out]���صĹ�ԿID

//	���ĵ�����ʱRSA˽Կ.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ImportTempRSAPrivateKey(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR pcRSAPrivateKey,	//[in]�������Կ
	USBKEY_ULONG ulRSAPrivateKeyrLen,	//[in]�������Կ����
	USBKEY_UCHAR_PTR pcPriKeyID);	//[out]���ص�˽ԿID

//	��ʱRSA˽Կ����
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_TempRSAPrivateKeyOperation(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR pcPriKey,		//[in]˽Կ�ṹ	USBKEY_RSA_PRIVATE_KEY_IMPORT_2048/1024
	USBKEY_ULONG ulPriKeyLen,		//[in]˽Կ�ĳ���
	USBKEY_UCHAR_PTR pcInData,		//[in]��������
	USBKEY_ULONG ulInDataLen,		//[in]�������ݳ���
	USBKEY_UCHAR_PTR pcOutData,		//[out]���ص����
	USBKEY_ULONG_PTR pulOutDataLen);//[out]���ص��������

//	��ʱRSA��Կ����
//	����ֵ��
//		0��	�ɹ�.
//		>0:	ʧ�ܣ����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_TempRSAPublicKeyOperation(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR pcPubKey,		//[in]��Կ�ṹ	USBKEY_RSA_PUBLIC_KEY_2048/1024
	USBKEY_ULONG ulPubKeyLen,		//[in]��Կ�ĳ���
	USBKEY_UCHAR_PTR pcInData,		//[in]��������
	USBKEY_ULONG ulInDataLen,		//[in]�������ݳ���
	USBKEY_UCHAR_PTR pcOutData,		//[out]�������
	USBKEY_ULONG_PTR pulOutDataLen);//[out]������ݳ���

//	д����Կ����֤��.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_WriteCertificate(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR pcContainerName,//[in]��Կ��������
	USBKEY_ULONG ulKeyUsage,		//[in]��Կ��;��KEY_EXCHANGE��Կ����KEY_SIGNATURE��Կ
	USBKEY_UCHAR_PTR pcData,		//[in]֤������
	USBKEY_ULONG_PTR pulSize);		//[in,out]����д�����ݻ�������С�����ʵ��д���С

//	��ȡ��Կ����֤��.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ReadCertificate(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR pcContainerName,//[in]��Կ��������
	USBKEY_ULONG ulKeyUsage,		//[in]��Կ��;��KEY_EXCHANGE��Կ����KEY_SIGNATURE��Կ
	USBKEY_UCHAR_PTR pcData,		//[out]֤������
	USBKEY_ULONG_PTR pulSize);		//[in,out]�����ȡ���ݻ�������С�����ʵ�ʶ�ȡ����

//	�ж�֤���Ƿ����
//	����ֵ��
//		0:	����
//		-1:������
//		>0:������
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_IsCertificateExist(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR pcContainerName,//[in]��Կ��������
	USBKEY_ULONG ulKeyUsage);		//[in]��Կ��;��KEY_EXCHANGE��Կ����KEY_SIGNATURE��Կ

//	ɾ����Կ����֤��.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DeleteCertificate(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR pcContainerName,//[in]��Կ��������
	USBKEY_ULONG ulKeyUsage);		//[in]��Կ��;��KEY_EXCHANGE��Կ����KEY_SIGNATURE��Կ

//	ɾ���ǶԳ���Կ.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DeleteAsymmetricKey(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR pcContainerName,//[in]��Կ��������
	USBKEY_ULONG ulKeyUsage,		//[in]��Կ��;
	USBKEY_ULONG ulKeyType);		//[in]��Կ���ͣ�USBKEY_RSA_PUBLICKEY��USBKEY_RSA_PRIVATEKEY

//	����ECC�ǶԳ���Կ.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_GenerateECCKeyPair(
	USBKEY_HANDLE			hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR		pcContainerName,//[in]��Կ��������
	USBKEY_ULONG			ulKeyUsage,		//[in]��Կ��;��KEY_EXCHANGE��Կ����KEY_SIGNATURE��Կ
	USBKEY_ULONG			ulBitLen,		//[in]��Կģ����256����384
	USBKEY_ULONG			ulGenKeyFlags,	//[in]������Կ�Ŀ��Ʊ�־
	USBKEY_UCHAR_PTR		pcPubKeyID,		//[out]���صĹ�ԿID�����ΪNULL�򲻷��ع�ԿID
	USBKEY_UCHAR_PTR		pcPriKeyID,		//[out]���ص�˽ԿID�����ΪNULL�򲻷���˽ԿID
	USBKEY_ECC_PUBLIC_KEY*	pPublicKey);	//[out]���صĹ�Կ���ݣ����ΪNULL�򲻷��ع�Կ����

//	����ECC��˽Կ��.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ImportECCKeyPair(
	USBKEY_HANDLE		hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR	pcWrapKeyID,	//[in]��������������Կ����ԿID
	USBKEY_ULONG		ulMode,			//[in]����ģʽ(����Ϊ�����)��USBKEY_ECB �� USBKEY_CBC,���ޱ�����ԿID����ΪECBģʽ
	USBKEY_UCHAR_PTR	pcIV,			//[in]��ʼ�������������ecbģʽ������ΪNULL
	USBKEY_ULONG		ulIVLen,		//[in]��ʼ���������ȣ������ecbģʽ������Ϊ0
	USBKEY_UCHAR_PTR	pcContainerName,//[in]��Կ����
	USBKEY_ULONG		ulKeyUsage,		//[in]��Կ��;��KEY_EXCHANGE��Կ����KEY_SIGNATURE��Կ
	USBKEY_UCHAR_PTR	pcECCKeyPair,	//[in]�������Կ��
	USBKEY_ULONG		ulECCKeyPairLen,//[in]�������Կ����
	USBKEY_UCHAR_PTR	pcPubKeyID,		//[out]���ص�˽ԿID�����ΪNULL�򲻷��ع�ԿID
	USBKEY_UCHAR_PTR	pcPriKeyID);	//[out]���صĹ�ԿID�����ΪNULL�򲻷���˽ԿID


//	����ECC˽Կ.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ImportECCPrivateKey(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR pcWrapKeyID,	//[in]��������������Կ����ԿID
	USBKEY_ULONG ulMode,			//[in]����ģʽ��USBKEY_ECB, USBKEY_CBC(����Ϊ�����),���ޱ�����ԿID����ΪECBģʽ
	USBKEY_UCHAR_PTR pcIV,			//[in]��ʼ�������������ecbģʽ������ΪNULL
	USBKEY_ULONG ulIVLen,			//[in]��ʼ���������ȣ������ecbģʽ������Ϊ0
	USBKEY_UCHAR_PTR pcContainerName,//[in]��Կ����
	USBKEY_ULONG ulKeyUsage,		//[in]��Կ��;��KEY_EXCHANGE��Կ����KEY_SIGNATURE��Կ
	USBKEY_UCHAR_PTR pcECCPriKey,	//[in]�������Կ
	USBKEY_ULONG ulECCPriKeyLen,	//[in]�������Կ����
	USBKEY_UCHAR_PTR pcPriKeyID);	//[out]���ص�˽ԿID

//	����ECC��Կ.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ImportECCPublicKey(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR pcContainerName,	//[in]��Կ����
	USBKEY_ULONG ulKeyUsage,		//[in]��Կ��;��KEY_EXCHANGE��Կ����KEY_SIGNATURE��Կ
	USBKEY_UCHAR_PTR pcECCPublicKey,//[in]�������Կ
	USBKEY_ULONG ulRSAPublicKeyLen,	//[in]�������Կ����
	USBKEY_UCHAR_PTR pcPubKeyID);	//[out]���صĹ�ԿID

//	ECC��Կ����
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ECCEncrypt(
	USBKEY_HANDLE			hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR		pcPubKeyID,		//[in] ��ԿID
	USBKEY_UCHAR_PTR		pcInData,		//[in]�����ܵ���������
	USBKEY_ULONG			ulInDataLen,	//[in]�����ܵ��������ݳ���
	USBKEY_UCHAR_PTR		pcOutData,		//[out]���Ļ�����������ò���ΪNULL������pulOutDataLen��������ʵ�ʳ���
	USBKEY_ULONG_PTR		pulOutDataLen);	//[in,out] in��pcOutData��������С��out������ʵ�ʳ���


//	ECC˽Կ����
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ECCDecrypt(
	USBKEY_HANDLE			hDeviceHandle,		//[in]�豸���
	USBKEY_UCHAR_PTR		pcPriKeyID,			//[in]���������˽ԿID
	USBKEY_UCHAR_PTR		pcInData,			//[in]�����ܵ���������
	USBKEY_ULONG			ulInDataLen,		//[in]�����ܵ��������ݳ���
	USBKEY_UCHAR_PTR		pcOutData,			//[out]���Ļ�����������ò���ΪNULL������pulOutDataLen��������ʵ�ʳ���
	USBKEY_ULONG_PTR		pulOutDataLen);		//[in,out] in��pcOutData��������С��out������ʵ�ʳ���

//	ECCǩ��.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ECCSign(
	USBKEY_HANDLE		hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR	pcPriKeyID,		//[in]���������˽ԿID
	USBKEY_UCHAR_PTR	pcData,			//[in]��ǩ������,�������Ѿ�����HASH�������õ�����
	USBKEY_ULONG		ulDataLen,		//[in]��ǩ�����ݳ���,�������˽Կ���ȣ���ģ����
	USBKEY_UCHAR_PTR	pcSignature,	//[out]���ص�ǩ��
	USBKEY_ULONG_PTR	pulSignLen);	//[out]���ص�ǩ������

//  ECC�ⲿ��Կ��֤ǩ��
//	����ֵ��
//		0��	��֤�ɹ�.
//		>0:	ʧ�ܣ����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ExtECCVerify(
	USBKEY_HANDLE			hDeviceHandle,		//[in]�豸���
	USBKEY_ECC_PUBLIC_KEY*	pPublicKey,			//[in]��Կ�ṹ
	USBKEY_UCHAR_PTR		pcData,				//[IN] ����֤ǩ�����ݡ�
	USBKEY_ULONG			ulDataLen,			//[IN] ����֤ǩ�����ݳ���
	USBKEY_UCHAR_PTR		pcSignature,		//[IN] ����֤ǩ��ֵ
	USBKEY_ULONG			ulSignLen);			//[IN] ����֤ǩ��ֵ����

//  ECC��Կ��֤ǩ��
//	����ֵ��
//		0��	��֤�ɹ�.
//		>0:	ʧ�ܣ����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ECCVerify(
	USBKEY_HANDLE		hDeviceHandle,	//[in] �豸���
	USBKEY_UCHAR_PTR	pcPubKeyID,		//[in] ��ԿID
	USBKEY_UCHAR_PTR	pcData,			//[IN] ����֤ǩ�����ݡ�
	USBKEY_ULONG		ulDataLen,		//[IN] ����֤ǩ�����ݳ���
	USBKEY_UCHAR_PTR	pcSignature,	//[IN] ����֤ǩ��ֵ
	USBKEY_ULONG		ulSignLen);		//[IN] ����֤ǩ��ֵ����

//	�ⲿ��Կ���ܵ����Գ���Կ.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ExtPubKeyExportSymmetricKey(
	USBKEY_HANDLE		hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR	pcSymKeyID,		//[in]�������Գ���Կ
	USBKEY_ULONG		ulAsymAlgID,	//[in]��������������Կ�ķǶԳ��㷨ID��Ŀǰ֧��USBKEY_TYPE_ECC
	USBKEY_UCHAR_PTR	pcPublicKey,	//[in]��������������Կ�Ĺ�Կ���ݽṹ��Ŀǰ֧��USBKEY_ECC_PUBLIC_KEY
	USBKEY_UCHAR_PTR	pcSymKey,		//[out]���ص���Կ������ò���ΪNULL������pulSymKeyLen��������ʵ�ʳ���
	USBKEY_ULONG_PTR	pulSymKeyLen);	//[in,out] in��pcSymKey��������С��out������ʵ�ʳ���

//	ECC�ⲿ��Կ����
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ExtECCEncrypt(
	USBKEY_HANDLE			hDeviceHandle,	//[in]�豸���
	USBKEY_ECC_PUBLIC_KEY*	pPublicKey,		//[in]��Կ�ṹ
	USBKEY_UCHAR_PTR		pcInData,		//[in]�����ܵ���������
	USBKEY_ULONG			ulInDataLen,	//[in]�����ܵ��������ݳ���
	USBKEY_UCHAR_PTR		pcOutData,		//[out]���Ļ�����������ò���ΪNULL������pulOutDataLen��������ʵ�ʳ���
	USBKEY_ULONG_PTR		pulOutDataLen);	//[in,out] in��pcOutData��������С��out������ʵ�ʳ���

//	ECC�ⲿ˽Կ����
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ExtECCDecrypt(
	USBKEY_HANDLE			hDeviceHandle,		//[in]�豸���
	USBKEY_ECC_PRIVATE_KEY* pPrivateKey,		//[in]˽Կ�ṹ
	USBKEY_UCHAR_PTR		pcInData,			//[in]�����ܵ���������
	USBKEY_ULONG			ulInDataLen,		//[in]�����ܵ��������ݳ���
	USBKEY_UCHAR_PTR		pcOutData,			//[out]���Ļ�����������ò���ΪNULL������pulOutDataLen��������ʵ�ʳ���
	USBKEY_ULONG_PTR		pulOutDataLen);		//[in,out] in��pcOutData��������С��out������ʵ�ʳ���

//	ECC�ⲿ˽Կǩ��.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ExtECCSign(
	USBKEY_HANDLE			hDeviceHandle,	//[in]�豸���
	USBKEY_ECC_PRIVATE_KEY* pPrivateKey,	//[in]˽Կ�ṹ
	USBKEY_UCHAR_PTR		pcData,			//[in]��ǩ������,�������Ѿ�����HASH�������õ�����
	USBKEY_ULONG			ulDataLen,		//[in]��ǩ�����ݳ���,�������˽Կ���ȣ���ģ����
	USBKEY_UCHAR_PTR		pcSignature,	//[out]���ص�ǩ�������ΪNULL��pulSignLen����ǩ������
	USBKEY_ULONG_PTR		pulSignLen);	//[in,out] in��pcSignature��������С��out��ǩ��ʵ�ʳ���

//	ECC������ԿЭ�̲��������	USBKEY_ExportAgreementDataWithECC 
//	��������					ʹ��ECC��ԿЭ���㷨��Ϊ����Ự��Կ������Э�̲�����������ʱECC��Կ�ԵĹ�Կ
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
//	
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ExportAgreementDataWithECC (
	USBKEY_HANDLE			hDeviceHandle,			//[in]�豸���
	USBKEY_ULONG			ulBitLen,				//[in]ģ��
	USBKEY_UCHAR_PTR		pcPriKeyID,				//[in]˽ԿID
	USBKEY_ECC_PUBLIC_KEY*	pTempECCPubKeyBlob,		//[OUT] ��ʱECC��Կ
	BYTE*					pbID,					//[IN] �����ID
	ULONG					ulIDLen);				//[IN] ����ID�ĳ��ȣ�������32
	

//	ECC����Э�̲���������Ự��Կ		USBKEY_ImportAgreementDataWithECC
//	��������							ʹ��ECC��ԿЭ���㷨������Э�̲���������Ự��Կ�������ز�������ԿID��
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
//
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ImportAgreementDataWithECC(
	USBKEY_HANDLE			hDeviceHandle,				//[in]�豸���
	USBKEY_UCHAR_PTR		pcPriKeyID,					//[in]˽ԿID
	ULONG					ulAlgId,					//[IN] �Ự��Կ�㷨��ʶ
	USBKEY_ECC_PUBLIC_KEY*	pSponsorTempECCPubKeyBlob,	//[IN] �Է�����ʱECC��Կ
	BYTE*					pbID,						//[IN] �Է���ID
	ULONG					ulIDLen,					//[IN] �Է�ID�ĳ��ȣ�������32
	USBKEY_UCHAR_PTR		pcKeyID);					//[OUT] ���ص���ԿID


//	����ECC��Կ.
//	��������	ʹ��ECC��ԿЭ���㷨��Ϊ����Ự��Կ������Э�̲�������������ʱECC��Կ�ԵĹ�Կ��Э�̾��
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ExportECCPublicKey(
	USBKEY_HANDLE			hDeviceHandle,		//[in]�豸���
	USBKEY_UCHAR_PTR		pcPubKeyID,			//[in]�������Ĺ�ԿID
	USBKEY_ECC_PUBLIC_KEY*	pPublicKey);		//[out]���صĹ�Կ


//	������ʱECC�ǶԳ���Կ�����洢.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_GenerateExtECCKeyPair(
	USBKEY_HANDLE			hDeviceHandle,	//[in]�豸���
	USBKEY_ULONG			ulBitLen,		//[in]��Կģ��
	USBKEY_ECC_PUBLIC_KEY	*pPublicKey,	//[out]���صĹ�Կ�ṹ
	USBKEY_ECC_PRIVATE_KEY	*pPrivateKey);	//[out]���ص�˽Կ�ṹ

//	�ر�Ӧ��.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_CloseApplication(
	USBKEY_HANDLE		hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR	pcAppName);		//[in]�����Ӧ������




//	�������ط����û�PIN.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_UnlockHiDiskPin(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_ULONG ulPINType,			//[in]�����PIN����
	USBKEY_UCHAR_PTR pcAdminPIN,	//[in]����Ĺ���ԱPIN
	USBKEY_ULONG ulAdminLen,		//[in]�����PIN����
	USBKEY_UCHAR_PTR pcNewPIN,		//[in]������û���PIN
	USBKEY_ULONG ulNewLen,			//[in]������û���PIN����
	USBKEY_ULONG_PTR pulRetryNum);	//[out]����ԱPIN��������Դ���

//	��˰��Ӧ�ô��˰����Ϣ�ļ�.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ReadTaxInfoFile(
		USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
		USBKEY_UCHAR_PTR pucFileID,		//[in]�ļ�ID
		USBKEY_ULONG ulOffset,			//[in]��ȡƫ����
		USBKEY_UCHAR_PTR pcOutData,		//[out]��������
		USBKEY_ULONG_PTR pulSize);		//[in,out]�����ȡ���ݳ��ȣ����ʵ�ʶ�ȡ����

//	д˰��Ӧ�ô��˰����Ϣ�ļ�.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_WriteTaxInfoFile(
		USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
		USBKEY_UCHAR_PTR pucFileID,		//[in]�ļ�ID
		USBKEY_ULONG ulOffset,			//[in]д��ƫ����
		USBKEY_UCHAR_PTR pcInData,		//[in]д������
		USBKEY_ULONG ulSize);			//[in]д�����ݳ���



//=======================================�Ͳ���ͨ��Ʊϵͳʹ��================================
//	����˰�ضԳ���Կ.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_GenTaxKey(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_ULONG ulKeyLen,			//[in]��Կ����
	USBKEY_ULONG ulAlgID,			//[in]��Կ�㷨��ʾ
	USBKEY_ULONG ulKeyFlag,			//[in]������Կ��־��0Ϊ0����Կ��1Ϊ1����Կ
	USBKEY_UCHAR_PTR phKey);		//[out]���ص���Կ���

//	����˰��Ӧ����Կ.
//	����ֵ��
//		0��	����.
//		>0:	FAIL�����ص��Ǵ�����
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ImportTaxKey(
	USBKEY_HANDLE hDeviceHandle,	//[in]�豸���
	USBKEY_UCHAR_PTR pcWrapKeyID,	//[in]��������������Կ����ԿID
	USBKEY_ULONG ulAsymAlgID,		//[in]��������������Կ����Կ�㷨��USBKEY_TYPE_RSA����USBKEY_TYPE_ECC��������ĵ�������Ϊ0
	USBKEY_ULONG ulAlgID,			//[in]��Կ�㷨��ʶ
	USBKEY_ULONG ulKeyFlag,			//[in]������Կ��־��0Ϊ0����Կ��1Ϊ1����Կ
	USBKEY_UCHAR_PTR pcSymKey,		//[in]�������Կ
	USBKEY_ULONG ulSymKeyLen,		//[in]�������Կ����
	USBKEY_UCHAR_PTR pcSymKeyID);	//[out]���ص���ĶԳ���ԿID


///////////////////////////////////////////////////////////////////////////////
//	END
///////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
	}
#endif
#endif
