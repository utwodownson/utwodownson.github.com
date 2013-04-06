#ifndef _UTILTEST_H
#define _UTILTEST_H

#include "./UsbKeyAPI.h"


#define APP_PKI "AKEY PKI APP"
//#define USBKEY_DEVICE_ADMIN_PIN		"Ai1@$USB6#&Key./"
#define USBKEY_DEVICE_ADMIN_PIN		"123456"


void AK_Hexdump(USBKEY_VOID_PTR pBuf, USBKEY_ULONG ulLen);

USBKEY_ULONG InitUKey_Test(USBKEY_HANDLE hDeviceHandle);

USBKEY_ULONG AuthenticateUKey_Test(USBKEY_HANDLE hDeviceHandle);

USBKEY_ULONG GetUKeyInfo_Test(USBKEY_HANDLE hDeviceHandle);

USBKEY_ULONG OptUKeyPIN_Test(USBKEY_HANDLE hDeviceHandle);

USBKEY_ULONG FileOpt_Test(USBKEY_HANDLE hDeviceHandle);

USBKEY_ULONG GenUKeyRSAPair_Test(USBKEY_HANDLE hDeviceHandle);

USBKEY_ULONG ImportUKeyRSAPair_Test(USBKEY_HANDLE hDeviceHandle);

USBKEY_ULONG TempKeyRSAPairOpt_Test(USBKEY_HANDLE hDeviceHandle);

USBKEY_ULONG UKeySymmetricKeyOpt_Test(USBKEY_HANDLE hDeviceHandle);

USBKEY_ULONG TempSymmetricKeyOpt_Test(USBKEY_HANDLE hDeviceHandle);

USBKEY_ULONG UKeyCertOpt_Test(USBKEY_HANDLE hDeviceHandle);

USBKEY_ULONG GenUKeyECCPair_Test(USBKEY_HANDLE hDeviceHandle);

USBKEY_ULONG ImportUKeyEccPair_Test(USBKEY_HANDLE hDeviceHandle);

USBKEY_ULONG TempKeyECCPairOpt_Test(USBKEY_HANDLE hDeviceHandle);

#endif