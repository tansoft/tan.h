#pragma once

#include <THSystem.h>
#include <THReg.h>
#include <THPhysicalAccess.h>
#include <THNetWork.h>
#include <THArith.h>

/**
* @brief 获取系统信息类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-05-29 新建类
*/
/**<pre>
</pre>*/
class THSysInfo
{
public:
	THSysInfo()
	{
	}
	virtual ~THSysInfo()
	{
	}

	static THString MakeUnqiHardCode()
	{
		return THMd5::CalcStrMd5Str(GetBiosId()+GetMacString()+GetHardDiskSn()+GetCpuId()+GetOSProductId()+GetOSVersion());
	}

	static THString GetBiosId()
	{
		return THPhysicalBiosAccess::GetBiosId();
	}

	static THString GetMacString()
	{
		THAdaptersInfo info;
		return info.GetAddressStringTrim(info.GetBestIntAdapter());
	}

	//MB
	static UINT GetHardDiskSizeMB()
	{
		return (UINT)(THPhysicalHardDiskAccess::GetHdSize()/1024/1024);
	}
	static THString GetHardDiskSn()
	{
		return THPhysicalHardDiskAccess::GetHdSn();
	}

	static THString GetCpuInfoProcessor(){return GetCpuInfo(_T("ProcessorNameString"));}
	static THString GetCpuInfoVendor(){return GetCpuInfo(_T("VendorIdentifier"));}
	static THString GetCpuInfoIdentifier(){return GetCpuInfo(_T("Identifier"));}
	static THString GetCpuInfo(THString info)
	{
		THString ret;
		THReg reg;
		if (reg.Open(HKEY_LOCAL_MACHINE,_T("HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0")))
		{
			ret=reg.ReadString(info);
			reg.Close();
		}
		return ret;
	}

	static THString GetCpuId()
	{
		return THPhysicalCpuAccess::GetCpuId();
	}

	static THString GetOSProductId()
	{
		THString ret;
		THReg reg;
		if (reg.Open(HKEY_LOCAL_MACHINE,_T("SOFTWARE\\MICROSOFT\\Windows\\CurrentVersion")))
		{
			ret=reg.ReadString(_T("ProductId"));
			reg.Close();
		}
/*		//method 2
		if (reg.Open(HKEY_LOCAL_MACHINE,_T("SOFTWARE\\MICROSOFT\\Windows NT\\CurrentVersion")))
		{
			char buffer[1024];
			DWORD len=1024;
			if (reg.ReadBuffer(_T("DigitalProductId"),buffer,&len))
			{
				if (len>8)
					ret=THCharset::a2t(buffer+8);
			}
			reg.Close();
		}*/
		return ret;
	}

	static THString GetOSProductName()
	{
		THString ret;
		THReg reg;
		if (reg.Open(HKEY_LOCAL_MACHINE,_T("SOFTWARE\\MICROSOFT\\Windows NT\\CurrentVersion")))
		{
			ret=reg.ReadString(_T("ProductName"));
			reg.Close();
		}
		return ret;
	}

	static THString GetComputerName()
	{
		THString str;
		DWORD Size=1024;
		::GetComputerName(str.GetBuffer(1024),&Size);
		str.ReleaseBuffer();
		return str;
	}

	static THString GetUserName()
	{
		THString str;
		DWORD Size=1024;
		::GetUserName(str.GetBuffer(1024),&Size);
		str.ReleaseBuffer();
		return str;
	}

#ifndef SM_SERVERR2
	#define SM_SERVERR2 89
#endif
#ifndef VER_SUITE_STORAGE_SERVER
	#define VER_SUITE_STORAGE_SERVER 0x00002000
#endif
#ifndef VER_SUITE_COMPUTE_SERVER
	#define VER_SUITE_COMPUTE_SERVER 0x00004000
#endif

	static THString GetOSVersion()
	{
		OSVERSIONINFOEX os={0};
		os.dwOSVersionInfoSize=sizeof(OSVERSIONINFOEX);
		::GetVersionEx((OSVERSIONINFO *)&os);
		THString SysString=_T("Unknown");
		int BuildNumber=os.dwBuildNumber&0xffff;
		if (os.dwMajorVersion==3 && os.dwMinorVersion==51)
			SysString=_T("Windows NT 3.51");
		else if (os.dwMajorVersion==4)
		{
			if (os.dwMinorVersion==0)
			{
				if (os.dwPlatformId==VER_PLATFORM_WIN32_NT)
					SysString=_T("Windows NT 4.0");
				else
					SysString=_T("Windows 95");
			}
			if (os.dwMinorVersion==10)
			{
				SysString=_T("Windows 98");
				if (BuildNumber==2222) SysString+=_T(" SE");
			}
			else if (os.dwMinorVersion==90) SysString=_T("Windows ME");
		}
		else if (os.dwMajorVersion==5)
		{
			if (os.dwMinorVersion==0)
			{
				SysString=_T("Windows 2000");
				if(os.wSuiteMask==VER_SUITE_ENTERPRISE) SysString+=_T(" Advanced Server");
			}
			else if (os.dwMinorVersion==1)
			{
				SysString=_T("Windows XP");
				if(os.wSuiteMask==VER_SUITE_EMBEDDEDNT) SysString+=_T(" Embedded");
				else if(os.wSuiteMask==VER_SUITE_PERSONAL) SysString+=_T(" Home Edition");
				else SysString+=_T(" Professional");
			}
			else if (os.dwMinorVersion==2)
			{
				if (os.wProductType==VER_NT_WORKSTATION)
				{
					SysString=_T("Windows XP");
					if(os.wSuiteMask==VER_SUITE_EMBEDDEDNT) SysString+=_T(" Embedded");
					else if(os.wSuiteMask==VER_SUITE_PERSONAL) SysString+=_T(" Home Edition");
					else SysString+=_T(" Professional");
				}
				else
				{
					if(GetSystemMetrics(SM_SERVERR2)==0)
					{
						SysString=_T("Windows 2003");
						if (os.wSuiteMask==VER_SUITE_BLADE) SysString+=_T(" Web Edition");
						else if(os.wSuiteMask==VER_SUITE_COMPUTE_SERVER) SysString+=_T(" Compute Cluster Edition");
						else if(os.wSuiteMask==VER_SUITE_STORAGE_SERVER) SysString+=_T(" Storage Server");
						else if(os.wSuiteMask==VER_SUITE_DATACENTER) SysString+=_T(" Datacenter Edition");
						else if(os.wSuiteMask==VER_SUITE_ENTERPRISE) SysString+=_T(" Enterprise Edition");
					}
					else
					{
						SysString=_T("Windows Server 2003 R2");
						if (os.wSuiteMask==VER_SUITE_STORAGE_SERVER) SysString+=_T("Storage Server");
					}
				}
			}
		}
		else if (os.dwMajorVersion==6)
		{
			if (os.dwMinorVersion==0)
			{
				if(os.wProductType==VER_NT_WORKSTATION)
				{
					SysString=_T("Windows Vista");
					if (os.wSuiteMask==VER_SUITE_PERSONAL) SysString+=_T(" Home");
				}
				else
				{
					SysString=_T("Windows Server 2008");
					if (os.wSuiteMask==VER_SUITE_DATACENTER) SysString+=_T(" Datacenter Server");
					else if(os.wSuiteMask==VER_SUITE_ENTERPRISE) SysString+=_T(" Enterprise");
				}
			}
			else if (os.dwMinorVersion==1)
			{
				if(os.wProductType==VER_NT_WORKSTATION)
					SysString=_T("Windows 7");
				else
					SysString=_T("Windows Server 2008 R2");
			}
		}
		THString TmpStr=os.szCSDVersion;
		if (!TmpStr.IsEmpty())
		{
			if (TmpStr.CompareNoCase(_T("A"))==0)
				SysString+=_T(" SE");
			else if (TmpStr.CompareNoCase(_T("C"))==0)
				SysString+=_T(" OSR2");
			else
				SysString+=_T(" ")+TmpStr;
		}
		SysString.AppendFormat(_T("(%d.%d.%d)"),os.dwMajorVersion,os.dwMinorVersion,BuildNumber);
		return SysString;
	}

	static void GetMemoryStatus(MEMORYSTATUS &st)
	{
		memset(&st,0,sizeof(st));
		st.dwLength=sizeof(MEMORYSTATUS);
		::GlobalMemoryStatus(&st);
	}

	static THString GetMemoryStatus()
	{
		MEMORYSTATUS MemoryStatus;
		GetMemoryStatus(MemoryStatus);
		THString str;
		str.Format(_T("%d MB( %d%% Ava.)"),MemoryStatus.dwTotalPhys/1024/1024,(MemoryStatus.dwAvailPhys/(MemoryStatus.dwTotalPhys/100)+MemoryStatus.dwAvailVirtual/(MemoryStatus.dwTotalVirtual/100))/2);
		return str;
	}

};