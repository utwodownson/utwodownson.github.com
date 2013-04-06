/*	UsbKeyAPI.h
	Author  :   caizhun@aisino.com
	CO.     :   AISINO
	date    :   2008年5月26日
	描述    :   USBKEY底层API接口	*/

#ifndef _USBKEYAPI_H
#define _USBKEYAPI_H


///////////////////////////////////////////////////////////////////////////////
//	include区域/
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//	环境宏定义
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
//	错误码定义
///////////////////////////////////////////////////////////////////////////////
#define USBKEY_OK							0x00000000	//成功
#define USBKEY_GENERAL_ERROR				0x00000001	//一般错误
#define USBKEY_RESERVED_NAME_ERROR			0x00000002	//保留名
#define USBKEY_NO_SUCH_RECORD				0x00000003	//没有此记录
#define USBKEY_BUFFER_TOO_SHORT				0x00000004	//缓冲区不足
#define USBKEY_NO_PRIVILEGE					0x00000005	//权限不足
#define USBKEY_FILE_EXIST					0x00000006	//文件、目录已经存在
#define USBKEY_FILE_NOT_FOUND				0x00000007	//文件未找到
#define USBKEY_RECORD_NOT_FOUND				0x00000008	//记录未找到
#define USBKEY_PARAM_ERROR					0x00000009	//参数错误
#define USBKEY_CONTAINER_NAME_EXIST			0x00000010	//容器名称已存在
#define USBKEY_MALLOC_ERROR					0x00000011	//内存分配失败
#define USBKEY_DISCARD_PADDING_ERROR		0x00000012	//去掉填充失败
#define USBKEY_MAX_NUM_OF_CONTAINER_ERROR	0x00000013	//容器达到最大数量
#define USBKEY_MUTEX_BAD					0x00000014	//mutex错误
#define USBKEY_MAX_NUM_OF_SYMKEY			0x00000015	//对称密钥达到最大数量
#define USBKEY_KEY_ID_ERROR					0x00000016	//密钥ID错误
#define USBKEY_TIME_OUT_ERROR				0x00000017	//时间超时
#define USBKEY_INVALID_HANDLE_ERROR			0x00000018	//句柄无效
#define USBKEY_MUTEX_ERROR					0x00000019	//线程错误
#define USBKEY_FUNCTION_NOT_SUPPORT			0x10000000	//函数不支持

/////////////////////////////////////////////////////////////////////////////
//  设备返回错误
/////////////////////////////////////////////////////////////////////////////
#define DEVICE_REMAIN_NT	 				0x63c0	//剩余的尝试次数，如0x63c2表示还可以尝试2次
#define DEVICE_MEM_ERROR					0x6581	//储存错误
#define DEVICE_P3_ERROR   					0x6700	//P3错误
#define DEVICE_NO_CURRENT_EF				0x6989	//没有当前EF
#define DEVICE_CMD_DATA_ERROR				0x6a80	//文件、目录已存在
#define DEVICE_FUNC_NOT_SUPPORT				0x6a81	//不支持此功能
#define DEVICE_FILE_NOT_FIND				0x6a82 	//没找到文件
#define DEVICE_RECORD_NOT_FIND				0x6a83	//没找到记录
#define DEVICE_MEM_NOT_ENOUGH				0x6a84	//储存空间不足
#define DEVICE_P1P2_ERROR  					0x6a86	//P1、P2错误

#define DEVICE_NA_STATUS					0x6901	//状态不满足，例如没MF的情况下创建df获ef
#define DEVICE_FILE_TYPE_ERROR				0x6981 	//文件类型错误
#define DEVICE_RIGHT_FAIL					0x6982	//权限不满足
#define DEVICE_KEY_LOCKED					0x6983 	//密钥锁定
#define DEVICE_NO_RANDOM					0x6984 	//没有随机数
#define DEVICE_APP_LOCK_TEMP	 			0x6985	//应用临时锁定
#define DEVICE_SEC_CONDITON_NOT_SATISFY 	0x6986	//安全状态不满足
#define DEVICE_USE_CONDITON_NOT_SATISFY 	0x6987	//使用条件不满足

#define DEVICE_MAC_ERROR					0x6988	//MAC错误

#define DEVICE_RANGE_ERROR  				0x6b00	//读/写文件越界
#define DEVICE_6C00							0x6c00	//数据长度错误，并指示正确的长度，例如0x6c30表示正确的长度应该为0x30
#define DEVICE_INS_ERROR  					0x6d00	//指令错误
#define DEVICE_CLA_ERROR  					0x6e00	//命令类别错误
#define DEVICE_6F00							0x6f00	//没有响应

#define DEVICE_SUCCESS     					0x9000	//成功
#define DEVICE_APP_LOCK_FOREVER	 			0x9303	//应用永久锁定

#define DEVICE_DF_LEVEL_UNSUPPORT			0x9401	//不支持此目录级别
#define DEVICE_KEY_NOT_FIND					0x9402	//密钥没有找到
#define DEVICE_FILE_POS_ERROR				0x9403	//文件指针位置错误
#define DEVICE_SYS_INIT_ERROR				0x9404	//系统初始化错误
#define DEVICE_ALG_NOT_SUPPORT				0x9405	//算法不支持

#define PKI_ERROR_PARAMETER                 0xFF01    //  参数错误 
#define PKI_ERROR_BIG_NUMBER_EXP_MOD        0xFF02    //  大数模密失败 AEmodN_for_RSA()
#define PKI_ERROR_RSA_RESULT_WRONG          0xFF03    //  RSA运算结果错误 
#define PKI_ERROR_API_RETURN_FAIL           0xFF04    //  调用芯片API返回失败 
#define PKI_ERROR_FILE_FORMAT               0xFF05    //  文件格式或内容错误 
#define PKI_ERROR_SESSION_KEY_LEN           0xFF06    //  Session Key长度不正确 
#define PKI_ERROR_ALG_NOT_SUPPORT           0xFF07    //  不支持的密钥算法 
#define PKI_ERROR_PINKEY_2_MACKEY           0xFF08    //  PIN密钥生成MAC验证的密钥方法不支持 
#define PKI_ERROR_FILE_LENGTH               0xFF09    //  文件长度问题 
#define PKI_ERROR_TEMP_SESSION_KEY_NO_PLACE 0xFF0A    //  g_session_key中没有空闲空间用于产生密钥了 
#define PKI_ERROR_KEY_CANNOT_EXPORT         0xFF0B    //  密钥不能被导出 
#define PKI_ERROR_ECC_RESULT_WRONG          0xFF0C    //  ECC运算结果错误 
#define PKI_ERROR_ECC_VERSIG_FAIL           0xFF0D    //  ECC签名验证失败 

///////////////////////////////////////////////////////////////////////////////
//	常量定义
///////////////////////////////////////////////////////////////////////////////
#define DEVICE_MAX_QUANTITY			10	//最大设备数量
#define APPLICATION_MAX_QUANTITY	3	//最大应用数量
#define CONTAINER_MAX_QUANTITY		6	//最大容器数量
#define FILE_MAX_QUANTITY			20	//最大文件数量
#define KEY_MAX_QUANTITY			5	//最大会话密钥数量 小于RECORD_MAX_QUANTITY
#define NAME_ID_MAX_LEN				16	//名称、ID最大长度
//#define RECORD_MAX_QUANTITY		10	//密钥文件最大记录数3f01 3f02
#define MAX_SYS_KEY_NUM				10	//最大系统密钥数3f01
#define MAX_USER_KEY_NUM			64	//最大用户密钥数3f02
#define KEY_RETRY_MAX_TIMES			5	//密钥最大允许尝试次数
#define DEV_IN					1		//设备插入
#define DEV_OUT					2		//设备拔出

//生成密钥标识
 #define USBKEY_CRYPT_EXPORTABLE		0x00000001 
 #define USBKEY_CRYPT_CREATE_SALT		0x00000002 
 #define USBKEY_CRYPT_NO_SALT			0x00000004 
 #define USBKEY_CRYPT_USER_PROTECTED	0x00000008 
 #define USBKEY_CRYPT_ONDEVICE			0x00000020

//COS算法标识
#define ALG_TYPE_DES			0x03
#define ALG_TYPE_3DES_2KEY		0x04
#define ALG_TYPE_3DES_3KEY		0x05
#define ALG_TYPE_SSF33			0x06
#define ALG_TYPE_SCB2			0x07

//国密局算法标识
#define USBKEY_TYPE_SCB2		0x00000100
#define USBKEY_TYPE_SSF33		0x00000200
#define USBKEY_TYPE_RSA			0x00010000
#define USBKEY_TYPE_ECC			0x00020100

//设备信息
#define DEVICEINFO_MANU_INFO	0x00020001		//厂商信息
#define DEVICEINFO_DEV_LABEL	0x00020002		//设备标签
#define DEVICEINFO_DEV_VERSION  0x00020003		//设备硬件版本
#define DEVICEINFO_COS_VERSION  0x00020004		//cos版本
#define DEVICEINFO_CUR_PRIV		0x00020005		//设备当前权限
#define DEVICEINFO_CHIP_ID		0x00020006		//芯片id


//设备状态
#define DEVICE_IN				0x01		//设备插入
#define DEVICE_OUT				0x02		//设备拔出
#define DEVICE_ABSENT_STATE		0x00000000	//设备不存在
#define DEVICE_PRESENT_STATE	0x00000001	//设备存在
#define USBKEY_INFINITE			INFINITE	//获取插拔事件超时时间

//权限
#define USBKEY_USERTYPE_USER	0x00000010	//用户
#define USBKEY_USERTYPE_ADMIN	0x00000001	//管理员
#define USBKEY_USERTYPE_HIDISK_USER		0x00010000	//隐藏区用户
#define USBKEY_USERTYPE_HIDISK_ADMIN	0x00001000	//隐藏区管理
#define USBKEY_USERTYPE_HIDISK_TAX		0x00100000	//隐藏区税局
#define USBKEY_ALL_PRIVILEGE	0x000000ff	//无权限限制
#define USBKEY_NEVER_USE		0x00000000	//不允许使用					
#define USBKEY_DEVICE_PRIVILEGE	0x00000100	//设备权限

//设备管理员PIN
#define DEVICE_ADMIN_PIN_LEN			16
#define	DEVICE_ADMIN_PIN_RETRY_TIMES	3

//保留文件名
#define DIR_RECORD_FILE			0x8000 //目录记录文件
#define FILE_RECORD_FILE		0x8001 //文件记录文件
#define CONTAINER_RECORD_FILE	0x8002 //Container记录文件
//#define DIR_RECORD_FILE 0x8003 //目录记录文件

//密钥用途
#define KEY_EXCHANGE					0x00000001
#define KEY_SIGNATURE					0x00000002
#define KEY_SESSION						0x00000003
#define KEY_INTERNAL_AUTHENTICATE_KEY	0x00000004
#define KEY_EXTERNAL_AUTHENTICATE_KEY	0x00000005
#define KEY_PIN							0x00000006

//芯片类型
#define CHIP_TYPE_20					0x00000001
#define CHIP_TYPE_AC3					0x00000002
#define CHIP_TYPE_45					0x00000003
#define CHIP_TYPE_AC4					0x00000005

//定义加密模式
#define USBKEY_ECB						0x00000000		//ECB模式
#define USBKEY_CBC						0x00000010		//CBC模式

//定义填充模式
#define USBKEY_NO_PADDING				0x00000000		//不填充
#define USBKEY_PKCS5_PADDING			0x00000001		//PKCS5填充

//RSA密钥类型
#define USBKEY_RSA_PUBLICKEY			0x00000001
#define USBKEY_RSA_PRIVATEKEY			0x00000002

//非对称密钥类型
#define USBKEY_PUBLIC_KEY				0x00000001
#define USBKEY_PRIVATE_KEY				0x00000002

//私钥密文类型
#define USBKEY_PRIKEY_TYPE_AI			0x00000000		//自定义类型
#define USBKEY_PRIKEY_TYPE_CSP			0x00000001		//微软CSP私钥密文格式

//#define USBKEY_MAX_COM_DATA_LEN			8*1024						//最大命令数据长度
#define USBKEY_MAX_COM_DATA_LEN			254						//最大命令数据长度(金融IC卡)
//#define USBKEY_MAX_COM_DATA_LEN			2048						//最大命令数据长度
#define USBKEY_MAX_COM_LEN				(USBKEY_MAX_COM_DATA_LEN+7)	//最大命令长度

#define USBKEY_MAX_FILE_LEN				(256*256)					//最大文件长度
#define USBKEY_MAX_BLOCK_LEN			4096						//文件处理块长

//税控应用
#define	TAX_KEY_FLAG			0	//KEY_MAX_QUANTITY-1		//税控专用密钥标识	add by zhoushenshen-20091030	Update by zhoushenshen-20100708

///////////////////////////////////////////////////////////////////////////////
//	类型定义
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
	unsigned char Eventflag;		//事件类型
	unsigned char *pcDeviceId;		//发生事件的设备的ID
}DEVICE_EVENT;

typedef struct
{
	unsigned int  bits;                 		/* length in bits of modulus */
	unsigned char publicKeyID[2];  				/* 公钥ID */
	unsigned char privateKeyID[2]; 				/* 私钥ID */
} USBKEY_RSA_KEY;

typedef struct
{
	unsigned int  bits;                 		/* length in bits of modulus */
	unsigned char publicKeyID[2];  				/* 公钥ID */
	unsigned char privateKeyID[2]; 				/* 私钥ID */
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
	UINT8 ApplicationType;//认证密钥的应用类型：外部认证密钥，内部认证密钥。	内部认证密钥：AT_INTERNAL_AUTHENTICATE_KEY外部认证密钥：AT_EXTERNAL_AUTHENTICATE_KEY
	UINT8 ulAlg;			//认证算法类型
	ULONG ulPrivil;	//认证密钥权限
	UINT8 KeyLen;//密钥值长度，按字节计算
	UINT8 ErrorCounter;//错误计数器，
}USBKEY_KEYHEAD, *PUSBKEY_KEYHEAD;

typedef struct {
	USBKEY_UCHAR major;
	USBKEY_UCHAR minor;
}USBKEY_VERSION;

typedef struct {
	USBKEY_UCHAR Manufacturer[64];		//设备商信息
	USBKEY_UCHAR Issuer[64];			//发行者信息
	USBKEY_UCHAR Label[64];				//设备标签
	USBKEY_UCHAR SerialNumber[32];		//序列号
	USBKEY_VERSION HWVersion;			//硬件版本
	USBKEY_VERSION FirmwareVersion;	//设备固件版本
	USBKEY_VERSION AppVersion;			//应用版本
	USBKEY_UCHAR   MinPINLen;			//最短PIN长度
	USBKEY_UCHAR   MaxPINLen;			//最大PIN长度
	USBKEY_ULONG  AlgSymCap;			//分组密码算法标识
	USBKEY_ULONG  AlgAsymCap;			//非对称密码算法标识
	USBKEY_ULONG  DevAuthAlgId;		//设备认证使用的分组密码算法标识
	USBKEY_ULONG  TotalMemory;			//设备总空间大小
	USBKEY_ULONG  FreeMemory;			//用户可用空间大小
	USBKEY_ULONG  Reserved;				//保留扩展，LOGIN 信息
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
//	底层设备相关API
///////////////////////////////////////////////////////////////////////////////
//	设备插拔事件函数	USBKEY_WaitForDevEvent
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_WaitForDevEvent(
	ULONG ulTimeOut,		//[IN]  超时时间，单位微秒,0：不阻塞，USBKEY_INFINITE：阻塞
	LPSTR szDevName, 		//[OUT] 发生事件的设备名称
	ULONG *pulDevNameLen, 	//[IN,OUT] IN：缓冲区长度，OUT：设备名称的有效长度，长度包含字符串结束符
	ULONG *pulEvent);		//[OUT] 事件类型。1表示插入，2表示拔出

//	取消等待设备插拔事件	USBKEY_CancelWaitForDevEvent
//	返回值：
//		0：	OK
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_CancelWaitForDevEvent();

//	枚举计算机上所连接的设备的ID
//	如果ppcDeviceIdS为NULL，则从pdwDeviceQuantity返回需要分配char *[NAME_ID_MAX_LEN]数量
//	如果ppcDeviceIdS非空，则从ppcDeviceIdS返回连接设备的ID，
//		并从pdwDeviceQuantity返回实际设备个数
//	返回值：
//		0：	OK
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DeviceEnum(
	USBKEY_CHAR			ppcDeviceIdS[DEVICE_MAX_QUANTITY][MAX_PATH],			//[out]设备ID的列表
	USBKEY_INT32_PTR	piDeviceQuantity);	//[out]设备ID的个数

//	检查设备是否存在
//	DEVICE_PRESENT_STATE为存在，DEVICE_ABSENT_STATE表示设备不存在
//	返回值：
//		0：	OK
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DeviceCheckExist(
	USBKEY_CHAR_PTR pcDeviceId,	//[in]设备的ID
	USBKEY_ULONG_PTR phDevState);	//[out]设备状态

//	打开某个设备
//	如果ucShareMode=1 表示共享模式 其它应用程序可以访问设备
//	返回值：
//		0：	OK
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DeviceOpen(
	USBKEY_CHAR_PTR pcDeviceId,		//[in]设备的ID
	USBKEY_INT32	iShareMode,		//[in]共享模式
	USBKEY_HANDLE	*phDeviceHandle);	//[out]设备句柄

//	关闭某个设备
//	返回值：
//		0：	OK
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DeviceClose(
	USBKEY_HANDLE	hDeviceHandle );	//[in]设备句柄
	
//	重置当前连接的某个设备，使设备进入初始状态（设备句柄第一次刚被打开）
//	返回值：
//		0：	OK
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DeviceReset(
	USBKEY_HANDLE	hDeviceHandle		//[in]设备句柄
	);		
	

//	初始化当前连接的某个设备.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DeviceInit(
		USBKEY_HANDLE		hDeviceHandle,	//[in]设备句柄
		USBKEY_ULONG		ulChipType,		//[in]芯片类型：CHIP_TYPE_20或CHIP_TYPE_45
		USBKEY_VERSION		appVer,			//应用版本
		USBKEY_UCHAR_PTR	pcManufacturer,	//厂商名称：最长64个字符，不足64个字符以空白字符(ASCII码为0x20)填充，不能以null（0x00）结束。
		USBKEY_UCHAR_PTR	pcLabel);		//设备标签：最长64个字符，不足64个字符以空白字符(ASCII码为0x20)填充，不能以null（0x00）结束。


//	获取当前连接的某个设备的状态.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DeviceGetStat(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_DEVINFO* pcDeviceInfo);//[Out]返回的信息，按照结构体DEVINFO返回

//	设置设备标签，标签大小小于32字节
//	返回值：
//		0：	OK
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DeviceSetLabel(
	USBKEY_HANDLE	hDeviceHandle,		//[in]设备句柄
	USBKEY_UCHAR_PTR pLabel);			//[in]设备标签字符串，此字符串应小于32字节

//	传输设备命令
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_TransmitDeviceCmd(
	USBKEY_HANDLE		hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR	pbCommand,		//[in]设备命令
	USBKEY_ULONG		ulCommandLen,	//[in]命令长度	
	USBKEY_UCHAR_PTR	pbResponse,		//[out]响应数据
	USBKEY_ULONG		*pulResponseLen);	//[in out]in: 响应buffer的长度，out: 响应长度

//	验证设备PIN.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_CheckDeviceAdminPin(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR pcPIN,			//[in]送入的PIN值
	USBKEY_ULONG ulLen,				//[in]送入的PIN长度
	USBKEY_ULONG_PTR pulRetryNum);	//[out]出错后重试次数

//	修改设备PIN.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ChangeDeviceAdminPin(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR pcOldPIN,		//[in]送入的原PIN
	USBKEY_ULONG ulOldLen,			//[in]送入的原PIN长度
	USBKEY_UCHAR_PTR pcNewPIN,		//[in]送入的新PIN
	USBKEY_ULONG ulNewLen,			//[in]送入的新PIN长度
	USBKEY_ULONG_PTR pulRetryNum);	//[out]出错后重试次数
///////////////////////////////////////////////////////////////////////////////
//	访问控制相关API
///////////////////////////////////////////////////////////////////////////////
//	向USBKEY写入内部认证或者外部认证密钥.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_WriteKey(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR pcKeyHead,		//[in]密钥头部信息
	USBKEY_UCHAR_PTR pcKeyBody);	//[in]密钥值

//	内部认证，终端对设备的认证.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_IntAuthenticate(
	USBKEY_HANDLE hDeviceHandle,		//[in]设备句柄
	USBKEY_UCHAR_PTR pcRandomNumber,	//[in]送入的随机数
	USBKEY_ULONG ulRandomNumberLen,		//[in]送入的随机数长度
	USBKEY_UCHAR_PTR pcEncryptNumber,	//[out]加密后的随机数
	USBKEY_ULONG_PTR pulEncryptNumberLen);//[out]返回的随机数长度

//	外部认证.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ExtAuthenticate(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR pcRandomNumber,//[in]送入的外部认证密钥加密后的随机数
	USBKEY_ULONG ulRandomNumberLen);	//[in]送入的随机数长度

//	初始化用户PIN.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_InitPin(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR pcPIN,		//[in]送入的PIN
	USBKEY_ULONG ulLen);	//[in]PIN长度

//	修改PIN.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ChangePin(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_ULONG ulPINType,			//[in]PIN类型: USBKEY_USERTYPE_ADMIN 或 USBKEY_USERTYPE_USER
	USBKEY_UCHAR_PTR pcOldPIN,		//[in]送入的原PIN
	USBKEY_ULONG ulOldLen,			//[in]送入的原PIN长度
	USBKEY_UCHAR_PTR pcNewPIN,		//[in]送入的新PIN
	USBKEY_ULONG ulNewLen,			//[in]送入的新PIN长度
	USBKEY_ULONG_PTR pulRetryNum);	//[out]出错后重试次数

//	验证PIN.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_CheckPin(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_ULONG ulPINType,			//[in]PIN类型
	USBKEY_UCHAR_PTR pcPIN,			//[in]送入的PIN值
	USBKEY_ULONG ulLen,				//[in]送入的PIN长度
	USBKEY_ULONG_PTR pulRetryNum);	//[out]出错后重试次数

//	解锁用户PIN.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_UnlockPin(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR pcAdminPIN,	//[in]送入的管理员PIN
	USBKEY_ULONG ulAdminLen,		//[in]送入的PIN长度
	USBKEY_UCHAR_PTR pcUserNewPIN,	//[in]送入的用户新PIN
	USBKEY_ULONG ulUserNewLen,		//[in]送入的用户新PIN长度
	USBKEY_ULONG_PTR pulRetryNum);	//[out]出错后重试次数

//获取PIN信息 
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_GetPinInfo(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_ULONG ulPINType,			//[in]PIN类型
	USBKEY_ULONG_PTR puMaxRetryNum,	//[out]最大重试次数
	USBKEY_ULONG_PTR pulRemainRetryNum);	//[out]剩余重试次数

///////////////////////////////////////////////////////////////////////////////
//	文件管理相关API
///////////////////////////////////////////////////////////////////////////////
//	创建应用.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_CreateApplication(
	USBKEY_HANDLE hDeviceHandle,		//[in]设备句柄
	USBKEY_UCHAR_PTR pcAppName,			//[in]送入的应用名称
	USBKEY_UCHAR_PTR pcAdminPIN,		//[in]送入的管理员PIN
	USBKEY_ULONG ulAdminPINLen,			//[in]送入的管理员PIN长度
	USBKEY_UCHAR ulAdminPinRetryCount,	//[in]管理员PIN最大重试次数
	USBKEY_UCHAR_PTR pcUserPIN,			//[in]送入的用户PIN
	USBKEY_ULONG ulUserPINLen,			//[in]送入的用户PIN长度
	USBKEY_UCHAR ulUserPinRetryCount,	//[in]用户PIN最大重试次数
	USBKEY_ULONG ulCreateFileRight);	//[in]在该应用下创建文件和容器的权限

//	删除应用.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DeleteApplication(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR pcAppName);		//[in]送入的应用名称

//	枚举应用.
//	如果pcApplicationNameList为NULL，则从pulSize返回需要分配空间数量，pulAppCount返回应用的个数
//	如果pcApplicationNameList非空，则从pcApplicationNameList返回应用列表，应用间以'\0'分隔，
//		并从pulSize返回实际使用空间大小，pulAppCount返回应用的个数
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_EnumApplication(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR pcApplicationNameList,		//[in,out]应用信息列表
	USBKEY_ULONG_PTR pulSize,		//[in,out]输入为缓冲区大小，输出为实际空间大小
	USBKEY_ULONG_PTR pulAppCount);		//[out]输出应用个数

//	打开应用.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_OpenApplication(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR pcAppName);	//[in]送入的应用名称

//	创建文件.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_CreateFile(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR pcFileName,	//[in]文件名称
	USBKEY_ULONG ulFileSize,		//[in]文件大小
	USBKEY_ULONG ulReadRights,		//[in]文件读权限
	USBKEY_ULONG ulWriteRights);	//[in]文件写权限

//	删除文件.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DeleteFile(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR pcFileName);	//[in]文件名称

//	枚举文件.
//	如果pcFileNameList为NULL，则从pulSize返回需要分配空间数量
//	如果pcFileNameList非空，则从pcFileNameList返回文件列表，文件名间以'\0'分隔，
//		并从pulSize返回实际使用空间数量
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_EnumFile(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR pcFileNameList,		//[in,out]文件信息列表
	USBKEY_ULONG_PTR pulSize);	//[in,out]输入为缓冲区大小，输出为实际使用空间大小

//	获取文件信息.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_GetFileAttribute(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR pcFileName,	//[in]文件名称
	USBKEY_UCHAR_PTR pcFileInfo);	//[out]文件信息，按照结构体FILEATTRIBUTE返回

//	读取文件.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ReadFile(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR pcFileName,	//[in]文件名称
	USBKEY_ULONG ulOffset,			//[in]读取偏移量
	USBKEY_UCHAR_PTR pcOutData,		//[out]返回数据
	USBKEY_ULONG_PTR pulSize);		//[in,out]输入读取数据缓冲区大小，输出实际读取长度

//	写文件.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_WriteFile(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR pcFileName,	//[in]文件名称
	USBKEY_ULONG ulOffset,			//[in]写入偏移量
	USBKEY_UCHAR_PTR pcData,		//[in]写入数据
	USBKEY_ULONG_PTR pulSize);		//[in,out]输入写入数据缓冲区大小，输出实际写入大小


///////////////////////////////////////////////////////////////////////////////
//	算法服务相关API
///////////////////////////////////////////////////////////////////////////////
//	创建密钥容器.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_CreateContainer(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR pcContainerName);		//[in]送入的密钥容器名称

//	删除密钥容器.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DeleteContainer(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR pcContainerName);	//[in]密钥容器名称

//	判断是否为空容器.
//	返回值：
//		0:	为空
//		-1:不为空
//		>0:错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_JudgeEmptyContainer(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR pcContainerName);	//[in]密钥容器名称

//	枚举密钥容器.
//	如果pcContianerNameList为NULL，则从pulSize返回需要分配空间数量
//	如果pcContianerNameList非空，则从pcContianerNameList返回密钥容器列表，ID间以'\0'分隔，
//		并从pulSize返回实际使用空间大小
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_EnumContainer(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR pcContianerNameList,		//[in,out]密钥容器名称列表
	USBKEY_ULONG_PTR pulSize);	//[in,out]输入为缓冲区大小，输出为实际使用空间大小

//	产生随机数.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_GenRandom(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR pcRandom,		//[out]返回的随机数
	USBKEY_ULONG ulRandomLen);		//[in]需要的随机数长度

//	产生对称密钥.
//	返回值:
//		0:	正常
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_GenSymmetricKey(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_ULONG ulKeyLen,			//[in]密钥长度
	USBKEY_ULONG ulAlgID,			//[in]密钥算法标示
	USBKEY_ULONG ulGenKeyFlags,		//[in]生成密钥的控制标志
	USBKEY_UCHAR_PTR phKey);		//[out]返回的密钥句柄

//	产生容器对称密钥.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_GenContainerSymmetricKey(
	USBKEY_HANDLE hDeviceHandle,		//[in]设备句柄
	USBKEY_UCHAR_PTR pcContainerName,	//[in]密钥容器名称
	USBKEY_ULONG ulKeyLen,				//[in]密钥长度
	USBKEY_ULONG ulAlgID,				//[in]密钥算法标示
	USBKEY_ULONG ulGenKeyFlags,			//[in]生成密钥的控制标志
	USBKEY_UCHAR_PTR phKey);			//[out]返回的密钥ID

//	删除容器对称密钥.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DeleteContainerSymmetricKey(
	USBKEY_HANDLE hDeviceHandle,		//[in]设备句柄
	USBKEY_UCHAR_PTR pcContainerName,	//[in]密钥容器名称
	USBKEY_UCHAR_PTR phKey);				//[in]密钥ID

//	产生RSA非对称密钥.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_GenerateRSAKeyPair(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR pcContainerName,//[in]密钥容器名称
	USBKEY_ULONG ulKeyUsage,		//[in]密钥用途，KEY_EXCHANGE密钥或者KEY_SIGNATURE密钥
	USBKEY_ULONG ulBitLen,			//[in]密钥模长
	USBKEY_ULONG ulGenKeyFlags,		//[in]生成密钥的控制标志
	USBKEY_UCHAR_PTR pcPubKeyID,	//[out]返回的公钥ID
	USBKEY_UCHAR_PTR pcPriKeyID);	//[out]返回的私钥ID

//	获取非对称密钥ID.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_GetAsymmetricKeyID(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR pcContainerName,//[in]密钥容器名称
	USBKEY_ULONG ulKeyUsage,		//[in]密钥用途
	USBKEY_UCHAR_PTR pcKeyPair);		//[out]返回的密钥对句柄，按照USBKEY_KEY_PAIR_ID返回

//	获取非对称私钥模长.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_GetPriKeyBitLen(
 	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR pcContainerName,//[in]密钥容器名称
	USBKEY_ULONG ulKeyUsage,		//[in]密钥用途
	USBKEY_ULONG_PTR pulBitLen);		//[out]返回的私钥的模长

//	导出对称密钥.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ExportSymmetricKey(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR pcSymKeyID,	//[in]被导出对称密钥
	USBKEY_UCHAR_PTR pcWrapKeyID,	//[in]用来保护导出密钥的密钥ID
	USBKEY_ULONG ulAsymAlgID,		//[in]用来保护导出密钥的密钥算法：USBKEY_TYPE_RSA或者USBKEY_TYPE_ECC
	USBKEY_UCHAR_PTR pcSymKey,		//[out]返回的密钥
	USBKEY_ULONG_PTR pulSymKeyLen);	//[out]返回的密钥长度


//	导入对称密钥.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ImportSymmetricKey(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR pcWrapKeyID,	//[in]用来保护导入密钥的密钥ID
	USBKEY_ULONG ulAsymAlgID,		//[in]用来保护导入密钥的密钥算法：USBKEY_TYPE_RSA或者USBKEY_TYPE_ECC
	USBKEY_ULONG ulKeyType,			//[in]密钥类型，表示密钥是卡内固定密钥还是临时密钥，可指定为USBKEY_CRYPT_ONDEVICE或者为0
	USBKEY_ULONG ulSymAlgID,		//[in]密钥算法标识
	USBKEY_UCHAR_PTR pcSymKey,		//[in]导入的密钥
	USBKEY_ULONG ulSymKeyLen,		//[in]导入的密钥长度
	USBKEY_UCHAR_PTR pcSymKeyID);	//[out]返回导入的对称密钥ID

//	导入容器对称密钥.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ImportContainerSymmetricKey(
	USBKEY_HANDLE hDeviceHandle,		//[in]设备句柄
	USBKEY_UCHAR_PTR pcContainerName,	//[in]密钥容器名称
	USBKEY_UCHAR_PTR pcWrapKeyID,		//[in]用来保护导入密钥的密钥ID
	USBKEY_ULONG ulAsymAlgID,			//[in]用来保护导入密钥的密钥算法：USBKEY_TYPE_RSA或者USBKEY_TYPE_ECC
	USBKEY_ULONG ulKeyType,				//[in]密钥类型，表示密钥是卡内固定密钥还是临时密钥，可指定为USBKEY_CRYPT_ONDEVICE或者为0
	USBKEY_ULONG ulAlgID,				//[in]密钥算法标识
	USBKEY_UCHAR_PTR pcSymKey,			//[in]导入的密钥
	USBKEY_ULONG ulSymKeyLen,			//[in]导入的密钥长度
	USBKEY_UCHAR_PTR pcSymKeyID);		//[out]返回导入的对称密钥ID

//	获取RSA公钥.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ExportRSAPublicKey(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR pcPubKeyID,	//[in]被导出公钥
	USBKEY_UCHAR_PTR pcPubKey,		//[out]返回的密钥
	USBKEY_ULONG_PTR pulPubKeyLen);	//[out]返回的密钥长度

/*//	导入RSA私钥.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ImportRSAPrivateKey(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR pcAppName,		//[in]应用名称
	USBKEY_UCHAR_PTR pcWrapKeyID,	//[in]用来保护导入密钥的密钥ID
	USBKEY_UCHAR_PTR pcContainerName,	//[in]密钥容器
	USBKEY_ULONG ulKeyUsage,		//[in]密钥用途，KEY_EXCHANGE密钥或者KEY_SIGNATURE密钥
	USBKEY_UCHAR_PTR pcRSAKeyPair,	//[in]导入的密钥
	USBKEY_ULONG ulRSAKeyPairLen,	//[in]导入的密钥长度
	USBKEY_UCHAR_PTR pcPriKeyID);	//[out]返回的私钥ID*/

//	导入RSA私钥.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ImportRSAPrivateKey(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_ULONG ulKeyType,			//[in]私钥类型：USBKEY_PRIKEY_TYPE_CSP,USBKEY_PRIKEY_TYPE_AI
	USBKEY_UCHAR_PTR pcWrapKeyID,	//[in]用来保护导入密钥的密钥ID
	USBKEY_ULONG ulMode,			//[in]解密模式：USBKEY_ECB, USBKEY_CBC(必须为带填充),c若无保护密钥ID，设为ECB模式
	USBKEY_UCHAR_PTR pcIV,			//[in]初始化向量，如果是ecb模式，设置为NULL
	USBKEY_ULONG ulIVLen,			//[in]初始化向量长度，如果是ecb模式，设置为0
	USBKEY_UCHAR_PTR pcContainerName,//[in]密钥容器
	USBKEY_ULONG ulKeyUsage,		//[in]密钥用途，KEY_EXCHANGE密钥或者KEY_SIGNATURE密钥
	USBKEY_UCHAR_PTR pcRSAKeyPair,	//[in]导入的密钥
	USBKEY_ULONG ulRSAKeyPairLen,	//[in]导入的密钥长度
	USBKEY_UCHAR_PTR pcPriKeyID);	//[out]返回的私钥ID

//	导入RSA公钥.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ImportRSAPublicKey(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR pcContainerName,	//[in]密钥容器
	USBKEY_ULONG ulKeyUsage,		//[in]密钥用途，KEY_EXCHANGE密钥或者KEY_SIGNATURE密钥
	USBKEY_UCHAR_PTR pcRSAPublicKey,//[in]导入的密钥
	USBKEY_ULONG ulECCPublicKeyLen,	//[in]导入的密钥长度
	USBKEY_UCHAR_PTR pcPubKeyID);	//[out]返回的公钥ID

//	导入RSA公私钥对.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ImportRSAKeyPair(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR pcWrapKeyID,	//[in]用来保护导入密钥的密钥ID
	USBKEY_ULONG ulMode,			//[in]解密模式(必须为带填充)：USBKEY_ECB 或 USBKEY_CBC,若无保护密钥ID，设为ECB模式
	USBKEY_UCHAR_PTR pcIV,			//[in]初始化向量，如果是ecb模式，设置为NULL
	USBKEY_ULONG ulIVLen,			//[in]初始化向量长度，如果是ecb模式，设置为0
	USBKEY_UCHAR_PTR pcContainerName,//[in]密钥容器
	USBKEY_ULONG ulKeyUsage,		//[in]密钥用途，KEY_EXCHANGE密钥或者KEY_SIGNATURE密钥
	USBKEY_UCHAR_PTR pcRSAKeyPair,	//[in]导入的密钥
	USBKEY_ULONG ulRSAKeyPairLen,	//[in]导入的密钥长度
	USBKEY_UCHAR_PTR pcPriKeyID,	//[out]返回的私钥ID
	USBKEY_UCHAR_PTR pcPubKeyID);	//[out]返回的公钥ID

//	RSA私钥运算.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_RSAPrivateKeyOperation(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR pcKeyID,		//[in]用来运算的密钥ID
	USBKEY_UCHAR_PTR pcInData,		//[in]输入数据
	USBKEY_ULONG ulInDataLen,		//[in]输入数据长度
	USBKEY_UCHAR_PTR pcOutData,		//[out]返回的输出
	USBKEY_ULONG_PTR pulOutDataLen);//[out]返回的输出长度

//	RSA公钥运算.
//	返回值：
//		0：	签名验证成功.
//		>0:	签名验证失败，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_RSAPublicKeyOperation(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR pcKeyID,		//[in]用来运算的密钥ID
	USBKEY_UCHAR_PTR pcInData,		//[in]输入数据
	USBKEY_ULONG ulInDataLen,		//[in]输入数据长度
	USBKEY_UCHAR_PTR pcOutData,		//[out]输出数据
	USBKEY_ULONG_PTR pulOutDataLen);//[out]输出数据长度

//	对称加密.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DataEncrypt(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR pcKeyID,		//[in]用来加密的密钥ID
	USBKEY_UCHAR_PTR pcData,		//[in]待加密数据
	USBKEY_ULONG ulDataLen,			//[in]待加密数据长度
	USBKEY_UCHAR_PTR pcEncryptedData,//[out]返回的加密后数据
	USBKEY_ULONG_PTR pulEncryptedLen);//[out]返回的加密后数据长度

//	对称解密.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DataDecrypt(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR pcKeyID,		//[in]用来解密的密钥ID
	USBKEY_UCHAR_PTR pcEncryptedData,//[in]密文数据
	USBKEY_ULONG ulEncryptedLen,	//[in]密文数据长度
	USBKEY_UCHAR_PTR pcData,		//[out]返回的明文
	USBKEY_ULONG_PTR pulDataLen);	//[out]返回的明文长度

//	批量对称加密.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_MassiveDataEncrypt(
	USBKEY_HANDLE		hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR	pcKeyID,		//[in]用来加密的密钥ID
	USBKEY_ULONG		ulAlg,			//[in]加密算法
	USBKEY_ULONG		ulMode,			//[in]加密模式和填充模式：USBKEY_ECB|USBKEY_NO_PADDING, USBKEY_ECB|USBKEY_PKCS5_PADDING, USBKEY_CBC|USBKEY_NO_PADDING, USBKEY_CBC|USBKEY_PKCS5_PADDING
	USBKEY_UCHAR_PTR	pcIV,			//[in]初始化向量，如果是ecb模式，设置为NULL
	USBKEY_UCHAR_PTR	pcData,			//[in]待加密数据
	USBKEY_ULONG		ulDataLen,		//[in]待加密数据长度
	USBKEY_UCHAR_PTR	pcEncryptedData,//[out]返回的加密后数据
	USBKEY_ULONG_PTR	pulEncryptedLen);//[in/out]in: pcEncryptedData缓冲区大小，out: 加密后数据长度或需要的pcEncryptedData缓冲区大小

//	批量对称解密.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_MassiveDataDecrypt(
	USBKEY_HANDLE		hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR	pcKeyID,		//[in]用来解密的密钥ID
	USBKEY_ULONG		ulAlg,			//[in]解密算法
	USBKEY_ULONG		ulMode,			//[in]解密模式和填充模式：USBKEY_ECB|USBKEY_NO_PADDING, USBKEY_ECB|USBKEY_PKCS5_PADDING, USBKEY_CBC|USBKEY_NO_PADDING, USBKEY_CBC|USBKEY_PKCS5_PADDING
	USBKEY_UCHAR_PTR	pcIV,			//[in]初始化向量，如果是ecb模式，设置为NULL
	USBKEY_UCHAR_PTR	pcEncryptedData,//[in]密文数据
	USBKEY_ULONG		ulEncryptedLen,	//[in]密文数据长度
	USBKEY_UCHAR_PTR	pcData,			//[out]返回的明文
	USBKEY_ULONG_PTR	pulDataLen);	//[in/out]in: pcData缓冲区长度 out:返回的明文长度

//	临时对称密钥加密	
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_TempDataEncrypt(
	USBKEY_HANDLE		hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR	pcKey,			//[in]用来加密的密钥
	USBKEY_ULONG		ulAlg,			//[in]加密算法
	USBKEY_ULONG		ulMode,			//[in]加密模式和填充模式：USBKEY_ECB|USBKEY_NO_PADDING, USBKEY_ECB|USBKEY_PKCS5_PADDING, USBKEY_CBC|USBKEY_NO_PADDING, USBKEY_CBC|USBKEY_PKCS5_PADDING
	USBKEY_UCHAR_PTR	pcIV,			//[in]初始化向量，如果是ecb模式，设置为NULL
	USBKEY_UCHAR_PTR	pcData,			//[in]待加密数据
	USBKEY_ULONG		ulDataLen,		//[in]待加密数据长度
	USBKEY_UCHAR_PTR	pcEncryptedData,//[out]返回的加密后数据
	USBKEY_ULONG_PTR	pulEncryptedLen);//[in/out]in: pcEncryptedData缓冲区大小，out: 加密后数据长度或需要的pcEncryptedData缓冲区大小

//	临时对称密钥解密
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_TempDataDecrypt(
	USBKEY_HANDLE		hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR	pcKey,			//[in]用来解密的密钥
	USBKEY_ULONG		ulAlg,			//[in]解密算法
	USBKEY_ULONG		ulMode,			//[in]解密模式和填充模式：USBKEY_ECB|USBKEY_NO_PADDING, USBKEY_ECB|USBKEY_PKCS5_PADDING, USBKEY_CBC|USBKEY_NO_PADDING, USBKEY_CBC|USBKEY_PKCS5_PADDING
	USBKEY_UCHAR_PTR	pcIV,			//[in]初始化向量，如果是ecb模式，设置为NULL
	USBKEY_UCHAR_PTR	pcEncryptedData,//[in]密文数据
	USBKEY_ULONG		ulEncryptedLen,	//[in]密文数据长度
	USBKEY_UCHAR_PTR	pcData,			//[out]返回的明文
	USBKEY_ULONG_PTR	pulDataLen);	//[in/out]in: pcData缓冲区长度 out:返回的明文长度

//	产生临时RSA非对称密钥，不存储.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_GenerateExtRSAKeyPair(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_ULONG ulBitLen,			//[in]密钥模长
	USBKEY_UCHAR_PTR pPriKeyStrc);	//[out]返回的私钥结构
	
/*//	产生1024临时RSA非对称密钥.（更改为USBKEY_GenerateExtRSAKey）
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_GenerateTempRSAKeyPair(
	USBKEY_HANDLE hDeviceHandle);	//[in]设备句柄*/

//	导入临时RSA公钥.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ImportTempRSAPublicKey(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR pcRSAPublicKey,//[in]导入的密钥
	USBKEY_ULONG ulRSAPublicKeyLen,	//[in]导入的密钥长度
	USBKEY_UCHAR_PTR pcPubKeyID);	//[out]返回的公钥ID

//	明文导入临时RSA私钥.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ImportTempRSAPrivateKey(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR pcRSAPrivateKey,	//[in]导入的密钥
	USBKEY_ULONG ulRSAPrivateKeyrLen,	//[in]导入的密钥长度
	USBKEY_UCHAR_PTR pcPriKeyID);	//[out]返回的私钥ID

//	临时RSA私钥运算
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_TempRSAPrivateKeyOperation(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR pcPriKey,		//[in]私钥结构	USBKEY_RSA_PRIVATE_KEY_IMPORT_2048/1024
	USBKEY_ULONG ulPriKeyLen,		//[in]私钥的长度
	USBKEY_UCHAR_PTR pcInData,		//[in]输入数据
	USBKEY_ULONG ulInDataLen,		//[in]输入数据长度
	USBKEY_UCHAR_PTR pcOutData,		//[out]返回的输出
	USBKEY_ULONG_PTR pulOutDataLen);//[out]返回的输出长度

//	临时RSA公钥运算
//	返回值：
//		0：	成功.
//		>0:	失败，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_TempRSAPublicKeyOperation(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR pcPubKey,		//[in]公钥结构	USBKEY_RSA_PUBLIC_KEY_2048/1024
	USBKEY_ULONG ulPubKeyLen,		//[in]公钥的长度
	USBKEY_UCHAR_PTR pcInData,		//[in]输入数据
	USBKEY_ULONG ulInDataLen,		//[in]输入数据长度
	USBKEY_UCHAR_PTR pcOutData,		//[out]输出数据
	USBKEY_ULONG_PTR pulOutDataLen);//[out]输出数据长度

//	写入密钥容器证书.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_WriteCertificate(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR pcContainerName,//[in]密钥容器名称
	USBKEY_ULONG ulKeyUsage,		//[in]密钥用途，KEY_EXCHANGE密钥或者KEY_SIGNATURE密钥
	USBKEY_UCHAR_PTR pcData,		//[in]证书数据
	USBKEY_ULONG_PTR pulSize);		//[in,out]输入写入数据缓冲区大小，输出实际写入大小

//	读取密钥容器证书.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ReadCertificate(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR pcContainerName,//[in]密钥容器名称
	USBKEY_ULONG ulKeyUsage,		//[in]密钥用途，KEY_EXCHANGE密钥或者KEY_SIGNATURE密钥
	USBKEY_UCHAR_PTR pcData,		//[out]证书数据
	USBKEY_ULONG_PTR pulSize);		//[in,out]输入读取数据缓冲区大小，输出实际读取长度

//	判断证书是否存在
//	返回值：
//		0:	存在
//		-1:不存在
//		>0:错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_IsCertificateExist(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR pcContainerName,//[in]密钥容器名称
	USBKEY_ULONG ulKeyUsage);		//[in]密钥用途，KEY_EXCHANGE密钥或者KEY_SIGNATURE密钥

//	删除密钥容器证书.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DeleteCertificate(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR pcContainerName,//[in]密钥容器名称
	USBKEY_ULONG ulKeyUsage);		//[in]密钥用途，KEY_EXCHANGE密钥或者KEY_SIGNATURE密钥

//	删除非对称密钥.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_DeleteAsymmetricKey(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR pcContainerName,//[in]密钥容器名称
	USBKEY_ULONG ulKeyUsage,		//[in]密钥用途
	USBKEY_ULONG ulKeyType);		//[in]密钥类型，USBKEY_RSA_PUBLICKEY或USBKEY_RSA_PRIVATEKEY

//	产生ECC非对称密钥.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_GenerateECCKeyPair(
	USBKEY_HANDLE			hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR		pcContainerName,//[in]密钥容器名称
	USBKEY_ULONG			ulKeyUsage,		//[in]密钥用途，KEY_EXCHANGE密钥或者KEY_SIGNATURE密钥
	USBKEY_ULONG			ulBitLen,		//[in]密钥模长：256或者384
	USBKEY_ULONG			ulGenKeyFlags,	//[in]生成密钥的控制标志
	USBKEY_UCHAR_PTR		pcPubKeyID,		//[out]返回的公钥ID，如果为NULL则不返回公钥ID
	USBKEY_UCHAR_PTR		pcPriKeyID,		//[out]返回的私钥ID，如果为NULL则不返回私钥ID
	USBKEY_ECC_PUBLIC_KEY*	pPublicKey);	//[out]返回的公钥数据，如果为NULL则不返回公钥数据

//	导入ECC公私钥对.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ImportECCKeyPair(
	USBKEY_HANDLE		hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR	pcWrapKeyID,	//[in]用来保护导入密钥的密钥ID
	USBKEY_ULONG		ulMode,			//[in]解密模式(必须为带填充)：USBKEY_ECB 或 USBKEY_CBC,若无保护密钥ID，设为ECB模式
	USBKEY_UCHAR_PTR	pcIV,			//[in]初始化向量，如果是ecb模式，设置为NULL
	USBKEY_ULONG		ulIVLen,		//[in]初始化向量长度，如果是ecb模式，设置为0
	USBKEY_UCHAR_PTR	pcContainerName,//[in]密钥容器
	USBKEY_ULONG		ulKeyUsage,		//[in]密钥用途，KEY_EXCHANGE密钥或者KEY_SIGNATURE密钥
	USBKEY_UCHAR_PTR	pcECCKeyPair,	//[in]导入的密钥对
	USBKEY_ULONG		ulECCKeyPairLen,//[in]导入的密钥长度
	USBKEY_UCHAR_PTR	pcPubKeyID,		//[out]返回的私钥ID，如果为NULL则不返回公钥ID
	USBKEY_UCHAR_PTR	pcPriKeyID);	//[out]返回的公钥ID，如果为NULL则不返回私钥ID


//	导入ECC私钥.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ImportECCPrivateKey(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR pcWrapKeyID,	//[in]用来保护导入密钥的密钥ID
	USBKEY_ULONG ulMode,			//[in]解密模式：USBKEY_ECB, USBKEY_CBC(必须为带填充),若无保护密钥ID，设为ECB模式
	USBKEY_UCHAR_PTR pcIV,			//[in]初始化向量，如果是ecb模式，设置为NULL
	USBKEY_ULONG ulIVLen,			//[in]初始化向量长度，如果是ecb模式，设置为0
	USBKEY_UCHAR_PTR pcContainerName,//[in]密钥容器
	USBKEY_ULONG ulKeyUsage,		//[in]密钥用途，KEY_EXCHANGE密钥或者KEY_SIGNATURE密钥
	USBKEY_UCHAR_PTR pcECCPriKey,	//[in]导入的密钥
	USBKEY_ULONG ulECCPriKeyLen,	//[in]导入的密钥长度
	USBKEY_UCHAR_PTR pcPriKeyID);	//[out]返回的私钥ID

//	导入ECC公钥.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ImportECCPublicKey(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR pcContainerName,	//[in]密钥容器
	USBKEY_ULONG ulKeyUsage,		//[in]密钥用途，KEY_EXCHANGE密钥或者KEY_SIGNATURE密钥
	USBKEY_UCHAR_PTR pcECCPublicKey,//[in]导入的密钥
	USBKEY_ULONG ulRSAPublicKeyLen,	//[in]导入的密钥长度
	USBKEY_UCHAR_PTR pcPubKeyID);	//[out]返回的公钥ID

//	ECC公钥加密
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ECCEncrypt(
	USBKEY_HANDLE			hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR		pcPubKeyID,		//[in] 公钥ID
	USBKEY_UCHAR_PTR		pcInData,		//[in]待加密的明文数据
	USBKEY_ULONG			ulInDataLen,	//[in]待加密的明文数据长度
	USBKEY_UCHAR_PTR		pcOutData,		//[out]密文缓冲区，如果该参数为NULL，则由pulOutDataLen返回密文实际长度
	USBKEY_ULONG_PTR		pulOutDataLen);	//[in,out] in：pcOutData缓冲区大小，out：密文实际长度


//	ECC私钥解密
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ECCDecrypt(
	USBKEY_HANDLE			hDeviceHandle,		//[in]设备句柄
	USBKEY_UCHAR_PTR		pcPriKeyID,			//[in]用来运算的私钥ID
	USBKEY_UCHAR_PTR		pcInData,			//[in]待解密的密文数据
	USBKEY_ULONG			ulInDataLen,		//[in]待解密的密文数据长度
	USBKEY_UCHAR_PTR		pcOutData,			//[out]明文缓冲区，如果该参数为NULL，则由pulOutDataLen返回明文实际长度
	USBKEY_ULONG_PTR		pulOutDataLen);		//[in,out] in：pcOutData缓冲区大小，out：明文实际长度

//	ECC签名.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ECCSign(
	USBKEY_HANDLE		hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR	pcPriKeyID,		//[in]用来运算的私钥ID
	USBKEY_UCHAR_PTR	pcData,			//[in]待签名数据,必须是已经做过HASH并且填充好的数据
	USBKEY_ULONG		ulDataLen,		//[in]待签名数据长度,必须等于私钥长度（即模长）
	USBKEY_UCHAR_PTR	pcSignature,	//[out]返回的签名
	USBKEY_ULONG_PTR	pulSignLen);	//[out]返回的签名长度

//  ECC外部公钥验证签名
//	返回值：
//		0：	验证成功.
//		>0:	失败，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ExtECCVerify(
	USBKEY_HANDLE			hDeviceHandle,		//[in]设备句柄
	USBKEY_ECC_PUBLIC_KEY*	pPublicKey,			//[in]公钥结构
	USBKEY_UCHAR_PTR		pcData,				//[IN] 待验证签名数据。
	USBKEY_ULONG			ulDataLen,			//[IN] 待验证签名数据长度
	USBKEY_UCHAR_PTR		pcSignature,		//[IN] 待验证签名值
	USBKEY_ULONG			ulSignLen);			//[IN] 待验证签名值长度

//  ECC公钥验证签名
//	返回值：
//		0：	验证成功.
//		>0:	失败，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ECCVerify(
	USBKEY_HANDLE		hDeviceHandle,	//[in] 设备句柄
	USBKEY_UCHAR_PTR	pcPubKeyID,		//[in] 公钥ID
	USBKEY_UCHAR_PTR	pcData,			//[IN] 待验证签名数据。
	USBKEY_ULONG		ulDataLen,		//[IN] 待验证签名数据长度
	USBKEY_UCHAR_PTR	pcSignature,	//[IN] 待验证签名值
	USBKEY_ULONG		ulSignLen);		//[IN] 待验证签名值长度

//	外部公钥加密导出对称密钥.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ExtPubKeyExportSymmetricKey(
	USBKEY_HANDLE		hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR	pcSymKeyID,		//[in]被导出对称密钥
	USBKEY_ULONG		ulAsymAlgID,	//[in]用来保护导出密钥的非对称算法ID，目前支持USBKEY_TYPE_ECC
	USBKEY_UCHAR_PTR	pcPublicKey,	//[in]用来保护导出密钥的公钥数据结构，目前支持USBKEY_ECC_PUBLIC_KEY
	USBKEY_UCHAR_PTR	pcSymKey,		//[out]返回的密钥，如果该参数为NULL，则由pulSymKeyLen返回密文实际长度
	USBKEY_ULONG_PTR	pulSymKeyLen);	//[in,out] in：pcSymKey缓冲区大小，out：密文实际长度

//	ECC外部公钥加密
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ExtECCEncrypt(
	USBKEY_HANDLE			hDeviceHandle,	//[in]设备句柄
	USBKEY_ECC_PUBLIC_KEY*	pPublicKey,		//[in]公钥结构
	USBKEY_UCHAR_PTR		pcInData,		//[in]待加密的明文数据
	USBKEY_ULONG			ulInDataLen,	//[in]待加密的明文数据长度
	USBKEY_UCHAR_PTR		pcOutData,		//[out]密文缓冲区，如果该参数为NULL，则由pulOutDataLen返回密文实际长度
	USBKEY_ULONG_PTR		pulOutDataLen);	//[in,out] in：pcOutData缓冲区大小，out：密文实际长度

//	ECC外部私钥解密
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ExtECCDecrypt(
	USBKEY_HANDLE			hDeviceHandle,		//[in]设备句柄
	USBKEY_ECC_PRIVATE_KEY* pPrivateKey,		//[in]私钥结构
	USBKEY_UCHAR_PTR		pcInData,			//[in]待解密的密文数据
	USBKEY_ULONG			ulInDataLen,		//[in]待解密的密文数据长度
	USBKEY_UCHAR_PTR		pcOutData,			//[out]明文缓冲区，如果该参数为NULL，则由pulOutDataLen返回明文实际长度
	USBKEY_ULONG_PTR		pulOutDataLen);		//[in,out] in：pcOutData缓冲区大小，out：明文实际长度

//	ECC外部私钥签名.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ExtECCSign(
	USBKEY_HANDLE			hDeviceHandle,	//[in]设备句柄
	USBKEY_ECC_PRIVATE_KEY* pPrivateKey,	//[in]私钥结构
	USBKEY_UCHAR_PTR		pcData,			//[in]待签名数据,必须是已经做过HASH并且填充好的数据
	USBKEY_ULONG			ulDataLen,		//[in]待签名数据长度,必须等于私钥长度（即模长）
	USBKEY_UCHAR_PTR		pcSignature,	//[out]返回的签名，如果为NULL，pulSignLen返回签名长度
	USBKEY_ULONG_PTR		pulSignLen);	//[in,out] in：pcSignature缓冲区大小，out：签名实际长度

//	ECC生成密钥协商参数并输出	USBKEY_ExportAgreementDataWithECC 
//	功能描述					使用ECC密钥协商算法，为计算会话密钥而产生协商参数，返回临时ECC密钥对的公钥
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
//	
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ExportAgreementDataWithECC (
	USBKEY_HANDLE			hDeviceHandle,			//[in]设备句柄
	USBKEY_ULONG			ulBitLen,				//[in]模长
	USBKEY_UCHAR_PTR		pcPriKeyID,				//[in]私钥ID
	USBKEY_ECC_PUBLIC_KEY*	pTempECCPubKeyBlob,		//[OUT] 临时ECC公钥
	BYTE*					pbID,					//[IN] 自身的ID
	ULONG					ulIDLen);				//[IN] 自身ID的长度，不大于32
	

//	ECC导入协商参数并计算会话密钥		USBKEY_ImportAgreementDataWithECC
//	功能描述							使用ECC密钥协商算法，导入协商参数，计算会话密钥，并返回产生的密钥ID。
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
//
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ImportAgreementDataWithECC(
	USBKEY_HANDLE			hDeviceHandle,				//[in]设备句柄
	USBKEY_UCHAR_PTR		pcPriKeyID,					//[in]私钥ID
	ULONG					ulAlgId,					//[IN] 会话密钥算法标识
	USBKEY_ECC_PUBLIC_KEY*	pSponsorTempECCPubKeyBlob,	//[IN] 对方的临时ECC公钥
	BYTE*					pbID,						//[IN] 对方的ID
	ULONG					ulIDLen,					//[IN] 对方ID的长度，不大于32
	USBKEY_UCHAR_PTR		pcKeyID);					//[OUT] 返回的密钥ID


//	导出ECC公钥.
//	功能描述	使用ECC密钥协商算法，为计算会话密钥而产生协商参数，并返回临时ECC密钥对的公钥和协商句柄
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ExportECCPublicKey(
	USBKEY_HANDLE			hDeviceHandle,		//[in]设备句柄
	USBKEY_UCHAR_PTR		pcPubKeyID,			//[in]被导出的公钥ID
	USBKEY_ECC_PUBLIC_KEY*	pPublicKey);		//[out]返回的公钥


//	产生临时ECC非对称密钥，不存储.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_GenerateExtECCKeyPair(
	USBKEY_HANDLE			hDeviceHandle,	//[in]设备句柄
	USBKEY_ULONG			ulBitLen,		//[in]密钥模长
	USBKEY_ECC_PUBLIC_KEY	*pPublicKey,	//[out]返回的公钥结构
	USBKEY_ECC_PRIVATE_KEY	*pPrivateKey);	//[out]返回的私钥结构

//	关闭应用.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_CloseApplication(
	USBKEY_HANDLE		hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR	pcAppName);		//[in]送入的应用名称




//	解锁隐藏分区用户PIN.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_UnlockHiDiskPin(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_ULONG ulPINType,			//[in]需解锁PIN类型
	USBKEY_UCHAR_PTR pcAdminPIN,	//[in]送入的管理员PIN
	USBKEY_ULONG ulAdminLen,		//[in]送入的PIN长度
	USBKEY_UCHAR_PTR pcNewPIN,		//[in]送入的用户新PIN
	USBKEY_ULONG ulNewLen,			//[in]送入的用户新PIN长度
	USBKEY_ULONG_PTR pulRetryNum);	//[out]管理员PIN出错后重试次数

//	读税控应用存放税控信息文件.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ReadTaxInfoFile(
		USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
		USBKEY_UCHAR_PTR pucFileID,		//[in]文件ID
		USBKEY_ULONG ulOffset,			//[in]读取偏移量
		USBKEY_UCHAR_PTR pcOutData,		//[out]返回数据
		USBKEY_ULONG_PTR pulSize);		//[in,out]输入读取数据长度，输出实际读取长度

//	写税控应用存放税控信息文件.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_WriteTaxInfoFile(
		USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
		USBKEY_UCHAR_PTR pucFileID,		//[in]文件ID
		USBKEY_ULONG ulOffset,			//[in]写入偏移量
		USBKEY_UCHAR_PTR pcInData,		//[in]写入数据
		USBKEY_ULONG ulSize);			//[in]写入数据长度



//=======================================淄博普通发票系统使用================================
//	产生税控对称密钥.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_GenTaxKey(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_ULONG ulKeyLen,			//[in]密钥长度
	USBKEY_ULONG ulAlgID,			//[in]密钥算法标示
	USBKEY_ULONG ulKeyFlag,			//[in]导入密钥标志，0为0号密钥，1为1号密钥
	USBKEY_UCHAR_PTR phKey);		//[out]返回的密钥句柄

//	导入税控应用密钥.
//	返回值：
//		0：	正常.
//		>0:	FAIL，返回的是错误码
USBKEY_API_MODE(USBKEY_ULONG) USBKEY_ImportTaxKey(
	USBKEY_HANDLE hDeviceHandle,	//[in]设备句柄
	USBKEY_UCHAR_PTR pcWrapKeyID,	//[in]用来保护导入密钥的密钥ID
	USBKEY_ULONG ulAsymAlgID,		//[in]用来保护导入密钥的密钥算法：USBKEY_TYPE_RSA或者USBKEY_TYPE_ECC；如果明文导入设置为0
	USBKEY_ULONG ulAlgID,			//[in]密钥算法标识
	USBKEY_ULONG ulKeyFlag,			//[in]导入密钥标志，0为0号密钥，1为1号密钥
	USBKEY_UCHAR_PTR pcSymKey,		//[in]导入的密钥
	USBKEY_ULONG ulSymKeyLen,		//[in]导入的密钥长度
	USBKEY_UCHAR_PTR pcSymKeyID);	//[out]返回导入的对称密钥ID


///////////////////////////////////////////////////////////////////////////////
//	END
///////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
	}
#endif
#endif
