#pragma once

#define THSYSTEM_INCLUDE_WINIOCTL
#include <THSystem.h>

#define MAX_LOADSTRING 100

typedef struct _UNICODE_STRING {
  USHORT  Length;//长度
  USHORT  MaximumLength;//最大长度
  PWSTR  Buffer;//缓存指针，访问物理内存时，此处指向UNICODE字符串"\device\physicalmemory"
} UNICODE_STRING,*PUNICODE_STRING;

typedef struct _OBJECT_ATTRIBUTES {
    ULONG Length;//长度 18h
    HANDLE RootDirectory;//  00000000
    PUNICODE_STRING ObjectName;//指向对象名的指针
    ULONG Attributes;//对象属性00000040h
    PVOID SecurityDescriptor;        // Points to type SECURITY_DESCRIPTOR，0
    PVOID SecurityQualityOfService;  // Points to type SECURITY_QUALITY_OF_SERVICE，0
} OBJECT_ATTRIBUTES;
typedef OBJECT_ATTRIBUTES *POBJECT_ATTRIBUTES;

typedef DWORD (__stdcall *ZWOS)(PHANDLE,ACCESS_MASK,POBJECT_ATTRIBUTES);
typedef DWORD (__stdcall *ZWMV)(HANDLE,HANDLE,PVOID,ULONG,ULONG,PLARGE_INTEGER,PSIZE_T,DWORD,ULONG,ULONG);
typedef DWORD (__stdcall *ZWUMV)(HANDLE,PVOID);

#ifndef _NTDEF_
typedef LONG NTSTATUS, *PNTSTATUS;
#endif
#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)
#define PHYSICAL_ADDRESS LARGE_INTEGER

/**
* @brief 物理内存访问类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-05-29 新建类
*/
/**<pre>
</pre>*/
class THPhysicalMemoryAccess
{
public:
	THPhysicalMemoryAccess()
	{
	}
	virtual ~THPhysicalMemoryAccess()
	{
	}

	static BOOL ReadPhysicalMemory(DWORD dwReadAddress,DWORD dwLength,void *pReceiveBuf)
	{
		//Init Dll Entry
		HMODULE hinstLib=LoadLibrary(_T("ntdll.dll"));
		if (!hinstLib) return FALSE;
		ZWOS ZWopenS=NULL;
		ZWMV ZWmapV=NULL;
		ZWUMV ZWunmapV=NULL;
		ZWopenS=(ZWOS)GetProcAddress(hinstLib,"ZwOpenSection");
		ZWmapV=(ZWMV)GetProcAddress(hinstLib,"ZwMapViewOfSection");
		ZWunmapV=(ZWUMV)GetProcAddress(hinstLib,"ZwUnmapViewOfSection");
		if (!ZWopenS || !ZWmapV || !ZWunmapV) return FALSE;
		HANDLE physmem;
		DWORD dwVAddress=0;
		SIZE_T rsize;

		UNICODE_STRING physmemString;
		WCHAR physmemName[] = L"\\Device\\PhysicalMemory";
		physmemString.Buffer=physmemName;
		physmemString.Length=0x2c;//注意大小是按字节算
		physmemString.MaximumLength =0x2e;//也是字节
		//RtlInitUnicodeString( &physmemString, physmemName);

		OBJECT_ATTRIBUTES attributes;
		attributes.Attributes =64;//属性0x40,OBJ_CASE_INSENSITIVE
		attributes.Length = sizeof(OBJECT_ATTRIBUTES);//OBJECT_ATTRIBUTES类型的长度
		attributes.ObjectName=&physmemString;//指向对象的指针
		attributes.RootDirectory=0;
		attributes.SecurityDescriptor=0;
		attributes.SecurityQualityOfService =0;
		//InitializeObjectAttributes(&attributes, &physmemString, OBJ_CASE_INSENSITIVE , NULL, NULL);

		if(!NT_SUCCESS(ZWopenS(&physmem,4,&attributes))) return FALSE;
		//if(!NT_SUCCESS(ZwOpenSection(&physmem,SECTION_MAP_READ,&attributes))) return FALSE;

		PHYSICAL_ADDRESS ViewBaseAddress;
		ViewBaseAddress.QuadPart=(ULONGLONG)(dwReadAddress);
		rsize=dwLength;
		if (!NT_SUCCESS(ZWmapV(physmem,(HANDLE)-1,(PVOID)&dwVAddress,0,dwLength,&ViewBaseAddress,&rsize,1,0,2))) return FALSE;
		//if (!NT_SUCCESS(ZwMapViewOfSection(physmem,(HANDLE)-1,(PVOID)&dwVAddress,0,dwLength,&ViewBaseAddress,&dwLength,ViewShare,0,PAGE_READONLY))) return FALSE;

		if (pReceiveBuf && rsize>=dwLength) memcpy(pReceiveBuf,(const void *)(UINT_PTR)dwVAddress,dwLength);

		if (!NT_SUCCESS(ZWunmapV((HANDLE)-1,(PVOID)(UINT_PTR)dwVAddress))) return FALSE;
		//if (!NT_SUCCESS(ZwUnmapViewOfSection((HANDLE)-1,(PVOID)dwVAddress))) return FALSE;
		FreeLibrary(hinstLib);
		return rsize>=dwLength;
	} 
};

/**
* @brief Bois访问类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-05-29 新建类
*/
/**<pre>
</pre>*/
class THPhysicalBiosAccess
{
public:
	THPhysicalBiosAccess(){}
	virtual ~THPhysicalBiosAccess(){}

	static THString GetBiosId()
	{
		THString ret;
		char *buffer=new char[0xffff];
		if (buffer)
		{
			//据说 0xFEC71 地址为Bios串号
			//读取地址需要对齐，这里取0xF0000
			if (THPhysicalMemoryAccess::ReadPhysicalMemory(0xF0000,0xffff,buffer))
			{
				BYTE *pBiosSerial=(BYTE *)buffer;
				UINT uBiosSerialLen = FindAwardBios( &pBiosSerial );
				if( uBiosSerialLen == 0U )
				{
					uBiosSerialLen = FindAmiBios( &pBiosSerial );
					if( uBiosSerialLen == 0U )
					{
						uBiosSerialLen = FindPhoenixBios( &pBiosSerial );
						if( uBiosSerialLen == 0U)
							uBiosSerialLen = FindIbmBios( &pBiosSerial );
					}
				}
				if( uBiosSerialLen != 0U )
					ret=THCharset::a2t((char *)pBiosSerial,uBiosSerialLen);
			}
			delete [] buffer;
		}
		return ret;
	}

	static void *GetBiosInfo()
	{
		char *buffer=new char[0xffff];
		if (buffer)
		{
			//读取地址需要对齐，这里取0xF0000
			if (THPhysicalMemoryAccess::ReadPhysicalMemory(0xF0000,0xffff,buffer))
				return buffer;
				//ret=THStringConv::BinToHexString(buffer+0xEC71,100);
			delete [] buffer;
		}
		return NULL;
	}

	static FreeBuffer(void *buf)
	{
		delete [] buf;
	}
private:
	static UINT FindAwardBios( BYTE** ppBiosAddr )
	{
    	BYTE* pBiosAddr = * ppBiosAddr + 0xEC71;
    	BYTE szBiosData[128];
    	CopyMemory( szBiosData, pBiosAddr, 127 );
    	szBiosData[127] = 0;
    	int iLen = lstrlenA( ( char* )szBiosData );
    	if( iLen > 0 && iLen < 128 )
    	{
        	//AWard: 07/08/2002-i845G-ITE8712-JF69VD0CC-00
        	//Phoenix-Award: 03/12/2002-sis645-p4s333
        	if( szBiosData[2] == '/' && szBiosData[5] == '/' )
        	{
            	BYTE* p = szBiosData;
            	while( * p )
            	{
                	if( * p < ' ' || * p >= 127 )
                	{
                    	break;
                	}
                	++ p;
            	}
            	if( * p == 0 )
            	{
                	* ppBiosAddr = pBiosAddr;
                	return ( UINT )iLen;
            	}
        	}
    	}
    	return 0;
	}

	static UINT FindAmiBios( BYTE** ppBiosAddr )
	{
    	BYTE* pBiosAddr = * ppBiosAddr + 0xF478;
    	BYTE szBiosData[128];
    	CopyMemory( szBiosData, pBiosAddr, 127 );
    	szBiosData[127] = 0;
    	int iLen = lstrlenA( ( char* )szBiosData );
    	if( iLen > 0 && iLen < 128 )
    	{
        	// Example: "AMI: 51-2300-000000-00101111-030199-"
        	if( szBiosData[2] == '-' && szBiosData[7] == '-' )
        	{
            	BYTE* p = szBiosData;
            	while( * p )
            	{
                	if( * p < ' ' || * p >= 127 )
                	{
                    	break;
                	}
                	++ p;
            	}
            	if( * p == 0 )
            	{
                	* ppBiosAddr = pBiosAddr;
                	return ( UINT )iLen;
            	}
        	}
    	}
    	return 0;
	}

	static UINT FindPhoenixBios( BYTE** ppBiosAddr )
	{
    	UINT uOffset[3] = { 0x6577, 0x7196, 0x7550 };
    	for( UINT i = 0; i < 3; ++ i )
    	{
        	BYTE* pBiosAddr = * ppBiosAddr + uOffset[i];
        	BYTE szBiosData[128];
        	CopyMemory( szBiosData, pBiosAddr, 127 );
        	szBiosData[127] = 0;
        	int iLen = lstrlenA( ( char* )szBiosData );
        	if( iLen > 0 && iLen < 128 )
        	{
            	// Example: Phoenix "NITELT0.86B.0044.P11.9910111055"
            	if( szBiosData[7] == '.' && szBiosData[11] == '.' )
            	{
                	BYTE* p = szBiosData;
                	while( * p )
                	{
                    	if( * p < ' ' || * p >= 127 )
                    	{
                        	break;
                    	}
                    	++ p;
                	}
                	if( * p == 0 )
                	{
                    	* ppBiosAddr = pBiosAddr;
                    	return ( UINT )iLen;
                	}
            	}
        	}
    	}
    	return 0;
	}

	static UINT FindIbmBios( BYTE** ppBiosAddr )
	{
    	BYTE* pBiosAddr = * ppBiosAddr + 0x6891;
    	BYTE szBiosData[128];
    	CopyMemory( szBiosData, pBiosAddr, 127 );
    	szBiosData[127] = 0;
    	int iLen = 0;
    	//if( iLen > 0 && iLen < 128 )
    	{
        	// Example: "UVPD0RESERVE7NETA7WW LVP75ML1ZE3B81B1ES7673B93g_SM_"
        	//if( szBiosData[2] == '-' && szBiosData[7] == '-' )
        	{
            	BYTE* p = szBiosData;
            	while( * p )
            	{
                	if( * p < ' ' || * p >= 127 )
                	{
                    	break;
                	}
                	++ p;
					iLen++;
            	}
            	if( iLen >= 20)
            	{
                	* ppBiosAddr = pBiosAddr;
                	return ( UINT )iLen;
            	}
        	}
    	}
    	return 0;
	}
};

#define MAX_IDE_DISK	4	//?
#define MAX_SCSI_DISK	26	//?

 // Valid values for the bCommandReg member of IDEREGS. 
#define IDE_ATAPI_IDENTIFY	0xA1 // Returns ID sector for ATAPI. 
#define IDE_ATA_IDENTIFY	0xEC // Returns ID sector for ATA. 
#define FILE_DEVICE_SCSI	0x0000001b
#define IOCTL_SCSI_MINIPORT_IDENTIFY	((FILE_DEVICE_SCSI << 16) + 0x0501) 
#define IOCTL_SCSI_MINIPORT	0x0004D008 // see NTDDSCSI.H for definition 
#define IDENTIFY_BUFFER_SIZE　512
#define SENDIDLENGTH		(sizeof( SENDCMDOUTPARAMS ) + IDENTIFY_BUFFER_SIZE )

#pragma pack(push,1)

typedef struct _SRB_IO_CONTROL 
{ 
	ULONG HeaderLength; 
	UCHAR Signature[8]; 
	ULONG Timeout; 
	ULONG ControlCode; 
	ULONG ReturnCode; 
	ULONG Length; 
}SRB_IO_CONTROL, *PSRB_IO_CONTROL; 

typedef struct _IDSECTOR 
{ 
	USHORT wGenConfig; 
	USHORT wNumCyls; 
	USHORT wReserved; 
	USHORT wNumHeads; 
	USHORT wBytesPerTrack; 
	USHORT wBytesPerSector; 
	USHORT wSectorsPerTrack; 
	USHORT wVendorUnique[3]; 
	CHAR sSerialNumber[20]; 
	USHORT wBufferType; 
	USHORT wBufferSize; 
	USHORT wECCSize; 
	CHAR sFirmwareRev[8]; 
	CHAR sModelNumber[40]; 
	USHORT wMoreVendorUnique; 
	USHORT wDoubleWordIO; 
	USHORT wCapabilities; 
	USHORT wReserved1; 
	USHORT wPIOTiming; 
	USHORT wDMATiming; 
	USHORT wBS; 
	USHORT wNumCurrentCyls; 
	USHORT wNumCurrentHeads; 
	USHORT wNumCurrentSectorsPerTrack; 
	ULONG ulCurrentSectorCapacity; 
	USHORT wMultSectorStuff; 
	ULONG ulTotalAddressableSectors; 
	USHORT wSingleWordDMA; 
	USHORT wMultiWordDMA; 
	BYTE bReserved[128]; 
}IDSECTOR, *PIDSECTOR; 

typedef struct _GETVERSIONOUTPARAMS 
{ 
	BYTE bVersion; // Binary driver version. 
	BYTE bRevision; // Binary driver revision. 
	BYTE bReserved; // Not used. 
	BYTE bIDEDeviceMap; // Bit map of IDE devices. 
	DWORD fCapabilities; // Bit mask of driver capabilities. 
	DWORD dwReserved[4]; // For future use. 
}GETVERSIONOUTPARAMS, *PGETVERSIONOUTPARAMS, *LPGETVERSIONOUTPARAMS; 

#pragma pack(pop)

#define  MAX_IDE_DRIVES  16

/**
* @brief 硬盘访问类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-05-29 新建类
*/
/**<pre>
	只使用了最常用的smart读取方法，只支持nt读取，需要9x或其他方法读取，请查看THPhysicalAccessMisc.txt
</pre>*/
class THPhysicalHardDiskAccess
{
public:
	THPhysicalHardDiskAccess()
	{
	}
	virtual ~THPhysicalHardDiskAccess()
	{
	}
	typedef struct _THHdInfo{
		THString sn;
		THString model;
		THString ctrlrno;//Drive Controller Revision Number
		__int64 size;
		long bufsize;//缓冲区大小
		DWORD drvtype;//0x0080=Removable,0x0040=Fixed
		int ctrlidx;//控制器索引，0为主控制器，0-3
		int drvidx;//磁盘索引，0为主磁盘，1为副磁盘
	}THHdInfo;

	static THString GetHdSn()
	{
		THHdInfo info;
		for(int i=0;i<MAX_IDE_DRIVES;i++)
		{
			if (GetHdInfo(i,info))
				return info.sn;
		}
		return _T("");
	}

	static __int64 GetHdSize()
	{
		THHdInfo info;
		for(int i=0;i<MAX_IDE_DRIVES;i++)
		{
			if (GetHdInfo(i,info))
				return info.size;
		}
		return 0;
	}

	static THString GetHdSizeString()
	{
		THString ret;
		ret.Format(_T("%I64u"),GetHdSize());
		return ret;
	}
	//0-15
	static BOOL GetHdInfo(int drive,THHdInfo &info)
	{
		BOOL bRet=FALSE;
		TCHAR driveName [256];
		_stprintf(driveName,_T("\\\\.\\PhysicalDrive%d"),drive);
		HANDLE hPhysicalDriveIOCTL = CreateFile(driveName,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
		if (hPhysicalDriveIOCTL != INVALID_HANDLE_VALUE)
		{
			GETVERSIONINPARAMS GetVersionParams;
			DWORD cbBytesReturned = 0;
			memset((void*)&GetVersionParams,0,sizeof(GetVersionParams));
			if (DeviceIoControl(hPhysicalDriveIOCTL,SMART_GET_VERSION,NULL,0,&GetVersionParams,sizeof(GETVERSIONINPARAMS),&cbBytesReturned,NULL))
			{
				ULONG CommandSize = sizeof(SENDCMDINPARAMS) + IDENTIFY_BUFFER_SIZE;
				char buf[sizeof(SENDCMDINPARAMS) + IDENTIFY_BUFFER_SIZE];
				PSENDCMDINPARAMS Command = (PSENDCMDINPARAMS)buf;
				Command->irDriveRegs.bCommandReg=0xEC;
				DWORD BytesReturned = 0;
				if (DeviceIoControl(hPhysicalDriveIOCTL,SMART_RCV_DRIVE_DATA,Command,sizeof(SENDCMDINPARAMS),Command,CommandSize,&BytesReturned,NULL))
				{
					DWORD diskdata [256];
					USHORT *pIdSector = (USHORT *)/*(PIDENTIFY_DATA)*/((PSENDCMDOUTPARAMS)Command)->bBuffer;
					for (int ijk = 0; ijk < 256; ijk++) diskdata[ijk]=pIdSector[ijk];
					PrintIdeInfo(drive,diskdata,info);
					bRet=TRUE;
				}
				CloseHandle(hPhysicalDriveIOCTL);
			}
		}
		return bRet;
	}
protected:
	static char *ConvertToString (DWORD diskdata [256],int firstIndex,int lastIndex,char* buf)
	{
		int index = 0;
		int position = 0;
		for (index = firstIndex; index <= lastIndex; index++)
		{
			buf [position++] = (char) (diskdata [index] / 256);
			buf [position++] = (char) (diskdata [index] % 256);
		}
		buf[position] = '\0';
		for (index = position - 1; index > 0 && isspace(buf [index]); index--)
			buf [index] = '\0';
		return buf;
	}

	static void PrintIdeInfo (int drive,DWORD diskdata [256],THHdInfo &info)
	{
		char serialNumber [1024];
		char modelNumber [1024];
		char revisionNumber [1024];
		__int64 sectors = 0;
		ConvertToString (diskdata, 10, 19, serialNumber);
		ConvertToString (diskdata, 27, 46, modelNumber);
		ConvertToString (diskdata, 23, 26, revisionNumber);
		info.sn=THCharset::a2t(serialNumber);
		info.model=THCharset::a2t(modelNumber);
		info.ctrlrno=THCharset::a2t(revisionNumber);
		info.bufsize=diskdata[21]*512;
		info.ctrlidx=drive/2;
		info.drvidx=drive%2;
		info.drvtype=diskdata[0];
		if (diskdata [83] & 0x400) 
			sectors = diskdata [103] * 65536I64 * 65536I64 * 65536I64 + diskdata [102] * 65536I64 * 65536I64 + diskdata [101] * 65536I64 + diskdata [100];
		else
			sectors = diskdata [61] * 65536 + diskdata [60];
		info.size = sectors * 512;
	}
};

/**
* @brief Cpu访问类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-05-29 新建类
*/
/**<pre>
</pre>*/
class THPhysicalCpuAccess
{
public:
	THPhysicalCpuAccess(){}
	virtual ~THPhysicalCpuAccess(){}

	static THString GetCpuId()
	{
		BOOL bException=FALSE;
		BYTE szCpu[16]={0};
		UINT uCpuID=0U;
		THString ret;
		try
		{
			_asm
			{
				mov eax, 0
				cpuid
				mov dword ptr szCpu[0], ebx
				mov dword ptr szCpu[4], edx
				mov dword ptr szCpu[8], ecx
				mov eax, 1
				cpuid
				mov uCpuID, edx
			}
		}
		catch(...)
		{
			bException = TRUE;
		}
		if(!bException)
		{
			ret.Format(_T("%s %u"),THCharset::a2t((char *)szCpu),uCpuID);
		}
		return ret;
	}
};
