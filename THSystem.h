//#pragma once

//disable warning C4996
#define _CRT_SECURE_NO_WARNINGS

/**
* @brief ϵͳ���ö���
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-05-28 �½���
*/

#ifdef THSYSTEM_INCLUDE_STDIO
	#include <stdio.h>
#endif

//֧��·����ȡ����
#ifdef THSYSTEM_INCLUDE_SHELLAPI
	#include <shlwapi.h>
#endif

//֧��pow�η�����
//֧��sqrt��������
#ifdef THSYSTEM_INCLUDE_MATH
	#include <math.h>
#endif

//֧��FLT_EPSILON��DBL_EPSILON
#ifdef THSYSTEM_FLOAT
	#include <float.h>
#endif

//֧�ֻ�ȡ�ļ�������
#ifdef THSYSTEM_INCLUDE_PSAPI
	#include <psapi.h>
	#pragma comment(lib, "psapi.lib")
#endif

//֧�����绷��
#ifdef THSYSTEM_INCLUDE_SOCKET2
	#include <winsock2.h>
	#pragma comment(lib,"ws2_32.lib")
#endif

#ifdef THSYSTEM_INCLUDE_IPTYPES
	#include <iptypes.h>
#endif

//֧������ץ����ip��ַ�����ȡ
#ifdef THSYSTEM_INCLUDE_TCPIP
	#include <mstcpip.h>
#endif
#ifdef THSYSTEM_INCLUDE_IPHELPAPI
	#include <iphlpapi.h>
	#pragma comment(lib,"Iphlpapi.lib")
#endif

//֧��INet��
#ifdef THSYSTEM_INCLUDE_WININET
	#include <wininet.h>
	#pragma comment(lib,"wininet.lib")
#endif

#ifdef THSYSTEM_INCLUDE_WINSOCK
	#include <winsock.h>
#endif

//֧��ʱ�䴦��
#ifdef THSYSTEM_INCLUDE_WINDOWS
	#include <windows.h>
#endif

//֧��min
#ifdef THSYSTEM_INCLUDE_WINDEF
	#include <windef.h>
#endif

//֧��ϵͳѡ���
#ifdef THSYSTEM_INCLUDE_COMMDLG
	#include <commdlg.h>
#endif

//֧�ֽ���ö��
#ifdef THSYSTEM_INCLUDE_TOOLHELP
	#include <tlhelp32.h>
#endif

//֧���ļ��汾��Ϣ
#ifdef THSYSTEM_INCLUDE_VERSION
	#include <winver.h>
	#pragma comment(lib, "version.lib")
#endif

//֧���ļ��Ի���
#ifdef THSYSTEM_INCLUDE_SHELLOBJ
	#include <shlobj.h>
#endif

//֧��ע�����
#ifdef THSYSTEM_INCLUDE_REG
	#include <winreg.h>
#endif

//֧��COM����
#ifdef THSYSTEM_INCLUDE_OBJIDL
	#include <objidl.h>
#endif

//֧��ShellCom����
#ifdef THSYSTEM_INCLUDE_SHELLOBJIDL
	#include <shobjidl.h>
#endif

//֧��Ole����
#ifdef THSYSTEM_INCLUDE_OLECTRL
	#include <olectl.h>
#endif

//֧��BSTR
#ifdef THSYSTEM_INCLUDE_OLEAUTO
	#include <oleauto.h>
#endif

//֧��webbrowser����
#ifdef THSYSTEM_INCLUDE_MSHTML
	#include <mshtml.h>
#endif

//֧��webbrowser����
#ifdef THSYSTEM_INCLUDE_OLEACC
	#include <oleacc.h>
#endif

#ifdef THSYSTEM_INCLUDE_WINNT
	#include <winnt.h>
#endif

#ifdef THSYSTEM_INCLUDE_WINBASE
	#include <winbase.h>
#endif

#ifdef THSYSTEM_INCLUDE_WINIOCTL
	#include <winioctl.h>
#endif

#ifdef THSYSTEM_INCLUDE_STL_STRUCT
	#include <THStlEx.h>
#endif

#ifdef THSYSTEM_INCLUDE_TCHAR
	#include <tchar.h>
#endif

#ifdef THSYSTEM_INCLUDE_TIME
	#include <time.h>
#endif

#ifdef THSYSTEM_INCLUDE_DSHOW
	#include <qedit.h>
	#include <dshow.h>
	#pragma comment(lib,"Strmiids.lib")
#endif

typedef unsigned __int64 U64;
typedef unsigned int U32;
typedef unsigned short U16;
typedef unsigned char U8;
typedef __int64 I64;
typedef signed int I32;
typedef signed short I16;
typedef signed char I8;
typedef __int64 S64;
typedef signed int S32;
typedef signed short S16;
typedef signed char S8;
typedef unsigned __int64 uint64_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
typedef __int64 int64_t;
typedef signed int int32_t;
typedef signed short int16_t;
typedef signed char int8_t;

#ifndef ntoh64
#define ntoh64(x)                      \
    ((U64)(((((U64)x) & 0x00000000000000ffulL) << 56)    \
     | ((((U64)x) & 0x000000000000ff00ull) << 40)  \
     | ((((U64)x) & 0x0000000000ff0000ull) << 24)  \
     | ((((U64)x) & 0x00000000ff000000ull) << 8)   \
     | ((((U64)x) & 0x000000ff00000000ull) >> 8)   \
     | ((((U64)x) & 0x0000ff0000000000ull) >> 24)  \
     | ((((U64)x) & 0x00ff000000000000ull) >> 40)  \
     | ((((U64)x) & 0xff00000000000000ull) >> 56)))
#endif

#ifndef hton64
#define hton64(x) ntoh64(x)
#endif
