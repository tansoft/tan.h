#pragma once

#include <THSystem.h>

/**
* @brief Guid描述类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-08-30 新建类
*/
class THGuid
{
public:
	THGuid()
	{
		memset(&m_def,0,sizeof(THGuidDef));
	}
	virtual ~THGuid(){}

	THString GetString() const
	{
		THString str;
		str.Format(_T("%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X"),htonl(m_def.Data1),htons(m_def.Data2),htons(m_def.Data3)
			,m_def.Data4[0],m_def.Data4[1],m_def.Data4[2],m_def.Data4[3],m_def.Data4[4],m_def.Data4[5],m_def.Data4[6],m_def.Data4[7]);
		return str;
	}

	inline void GetBuf(U8 *buf) const
	{
		memcpy(buf,&m_def,sizeof(THGuidDef));
	}

	inline void SetBuf(U8 *buf)
	{
		memcpy(&m_def,buf,sizeof(THGuidDef));
	}

	void SetString(THString sGuid)
	{
		FormatString(sGuid);
		unsigned int len=0;
		void *buf=THStringConv::HexStringToBin(sGuid,&len);
		if (buf)
		{
			SetBuf((U8 *)buf);
			THStringConv::Free(buf);
		}
	}

	BOOL IsEmpty() const
	{
		THGuid guid;
		return IsSame(&guid);
	}

	BOOL IsSame(THGuid *guid1) const
	{
		return memcmp(&guid1->m_def,&m_def,sizeof(THGuid))==0;
	}

	static BOOL IsSame(const THGuid *guid1,const THGuid *guid2)
	{
		return memcmp(&guid1->m_def,&guid2->m_def,sizeof(THGuid))==0;
	}

	BOOL IsSame(THString guid1) const
	{
		return IsSame(GetString(),guid1);
	}

	static BOOL IsSame(THString guid1,THString guid2)
	{
		FormatString(guid1);
		FormatString(guid2);
		guid1.MakeLower();
		guid2.MakeLower();
		return guid1==guid2;
	}
protected:
	//16 bytes
	typedef struct _THGuidDef
	{
		U32 Data1;
		U16 Data2;
		U16 Data3;
		U8 Data4[8];
	}THGuidDef;
	THGuidDef m_def;

	static void FormatString(THString &sGuid)
	{
		sGuid.Replace(_T("-"),_T(""));
		sGuid.Replace(_T("{"),_T(""));
		sGuid.Replace(_T("}"),_T(""));
		sGuid.Replace(_T("0x"),_T(""));
		sGuid.Replace(_T(","),_T(""));
	}
};
