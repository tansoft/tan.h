
#pragma once

#include <THString.h>
#include <THSystem.h>
#include <THFileStore.h>
#include <THArith.h>

//写入值和读取值的定义宏，方便使用
#define THIniWInt(ini,se,key,ret,def)	ini->WriteInt(se,key,ret)
#define THIniRInt(ini,se,key,ret,def)	ret=ini->GetInt(se,key,def)
#define THIniWBool(ini,se,key,ret,def)	ini->WriteInt(se,key,ret)
#define THIniRBool(ini,se,key,ret,def)	ret=ini->GetInt(se,key,def)
#define THIniWString(ini,se,key,ret,def)	ini->WriteString(se,key,ret)
#define THIniRString(ini,se,key,ret,def)	ret=ini->GetString(se,key,def)
#define THIniWStrAr(ini,se,key,ret) ini->WriteStringArray(se,key,ret)
#define THIniRStrAr(ini,se,key,ret)	ini->GetStringArray(se,key,ret)
#define THIniWBin(ini,se,key,buf,len)	ini->WriteBin(se,key,buf,len)
#define THIniRBin(ini,se,key,buf,len)	ini->ReadBin(se,key,buf,len)

//写入值和读取值自动匹配键值定义宏，方便使用
#define THIniAutoWInt(ini,se,ret,def) THIniWInt(ini,se,_T(#ret),m_##ret,def)
#define THIniAutoRInt(ini,se,ret,def) THIniRInt(ini,se,_T(#ret),m_##ret,def)
#define THIniAutoWBool(ini,se,ret,def) THIniWBool(ini,se,_T(#ret),m_##ret,def)
#define THIniAutoRBool(ini,se,ret,def)	 THIniRBool(ini,se,_T(#ret),m_##ret,def)
#define THIniAutoWString(ini,se,ret,def) THIniWString(ini,se,_T(#ret),m_##ret,def)
#define THIniAutoRString(ini,se,ret,def) THIniRString(ini,se,_T(#ret),m_##ret,def)
#define THIniAutoWStrAr(ini,se,ret) THIniWStrAr(ini,se,_T(#ret),m_##ret)
#define THIniAutoRStrAr(ini,se,ret) THIniRStrAr(ini,se,_T(#ret),m_##ret)
#define THIniAutoWBin(ini,se,ret,len) THIniWBin(ini,se,_T(#ret),m_##ret,len)
#define THIniAutoRBin(ini,se,ret,len) THIniRBin(ini,se,_T(#ret),m_##ret,len)

/**
* @brief Ini配置文件读写接口类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-09-08 新建类
*/
/**<pre>
  使用Sample：
  ITHIni *ini=THIni::CreateObject();
  ini->Init(_T("c:\\abc.txt"));
  ini->GetString(_T("section1"),_T("key1"),_T("defval"));
  ini->WriteInt(_T("section1"),_T("key1"),1);
  THStringArray ar,ar1;
  if (ini->EnumAllStruct(&ar))
    for(int i=0;i<ar.GetSize();i++)
      THDebug(_T("Value:%s"),ar[i]);
  if (ini->EnumStruct(_T("sect1"),&ar,&ar1))
    for(int i=0;i<ar.GetSize();i++)
      THDebug(_T("%s=%s"),ar[i],ar1[i]);
</pre>*/
class ITHIni
{
public:
	ITHIni()
	{
		m_pStore=NULL;
		m_bUseEncMode=FALSE;
		m_bCrc=FALSE;
	}
	/**
	* @brief 初始化路径,必须由实例类实现该接口
	* @param sPath		路径
	*/
	ITHIni(THString sPath)
	{
		ASSERT(FALSE);
	}

	ITHIni(ITHFileStore *pStore,THString sIniFile=_T(""))
	{
		InitStore(pStore,FALSE,sIniFile);
	}
	virtual ~ITHIni()
	{
		ReleaseStore();
	}

	BOOL InitStore(ITHFileStore *pStore,BOOL bFree,THString sIniFile=_T(""))
	{
		ReleaseStore();
		if (!pStore) return FALSE;
		m_pStore=pStore;
		if (bFree) m_pStore->SetFreeor(this);
		m_bUseEncMode=FALSE;
		m_bCrc=FALSE;
		return Init(sIniFile);
	}

	BOOL Init(THString sIniFile=_T(""))
	{
		m_sPath.Empty();
		if (!m_pStore) return FALSE;
		m_sPath=m_pStore->GetFullPath(sIniFile);
		return TRUE;
	}

	void ReleaseStore()
	{
		if (m_pStore)
		{
			m_pStore->DeleteObjectByFreeor(this);
			//纯虚函数问题
			//m_pStore->CloseAllFile();
			/*if (m_bFreeStore)
			{
				delete m_pStore;
				m_pStore=NULL;
			}*/
			m_pStore=NULL;
		}
	}

	/**
	* @brief 生成实例函数
	* @param sPath		Ini配置文件路径
	* @return 新建具体实例实现类的实例指针
	*/
	virtual ITHIni *CreateObject(THString sPath=_T("")){return new ITHIni(sPath);};

	/**
	* @brief 复制ini，原ini与目标ini可以是同一个文件，主要用于ini文件整个加密/解密，重新优化ini键值位置用
	* @param srcini		欲复制的ini对象
	* @return 是否成功
	*/
	virtual BOOL CopyIni(ITHIni *srcini)
	{
		if (!m_pStore || !srcini) return FALSE;
		THStringArray arKey;
		if (!srcini->EnumAllStruct(&arKey)) return FALSE;
		int cnt=(int)arKey.GetSize();
		THStringArray *va=new THStringArray[cnt*2];
		int i=0;
		for(i=0;i<cnt;i++)
		{
			THString k=arKey[i];
			if (!srcini->EnumStruct(k,va+i*2,va+i*2+1))
			{
				delete [] va;
				return FALSE;
			}
		}
		if (!m_pStore->DeleteFile(m_sPath))
		{
			delete [] va;		
			return FALSE;
		}
		for(i=0;i<cnt;i++)
		{
			THString k=arKey[i];
			THStringArray *ak,*av;
			ak=(va+i*2);
			av=(va+i*2+1);
			for(int j=0;j<ak->GetSize();j++)
				if (!WriteString(k,(*ak)[j],(*av)[j]))
				{
					delete [] va;
					return FALSE;
				}
		}
		return TRUE;
	}

	/**
	* @brief 获取字符串
	* @param sApp		段名称
	* @param sKey		字段名称
	* @param sDefValue	默认值
	* @return 获取的值
	*/
	virtual THString GetString(THString sApp,THString sKey,THString sDefValue=_T(""))
	{
		if (!m_pStore) return sDefValue;
		if (m_bUseEncMode)
			return DecBuffer(m_pStore->IniGetString(m_sPath,EncBuffer(sApp),EncBuffer(sKey),EncBuffer(sDefValue)));
		return m_pStore->IniGetString(m_sPath,sApp,sKey,sDefValue);
	}

	/**
	* @brief 获取字符串数组
	* @param sApp		段名称
	* @param sKey		字段名称
	* @param arDef	返回数组
	* @param bEmptyAr 是否先清空数组
	* @return 是否成功
	*/
	virtual BOOL GetStringArray(THString sApp,THString sKey,THStringArray &arDef,BOOL bEmptyAr=TRUE)
	{
		THString ret=GetString(sApp,sKey);
		BOOL bOk=FALSE;
		if (!ret.IsEmpty())
		{
			THStringToken t(ret,_T("|"));
			THString part;
			while(t.IsMoreTokens())
			{
				if (bOk==FALSE && bEmptyAr) arDef.RemoveAll();
				bOk=TRUE;
				part=t.GetNextToken();
				part=THBase64::DecodeStr(part);
				arDef.Add(part);
			}
		}
		return bOk;
	}

	/**
	* @brief 获取整数值
	* @param sApp		段名称
	* @param sKey		字段名称
	* @param iDefValue	默认值
	* @return 获取的值
	*/
	virtual int GetInt(THString sApp,THString sKey,int iDefValue=0)
	{
		return THStringConv::s2i(GetString(sApp,sKey,THStringConv::i2s(iDefValue)));
	}

	/**
	* @brief 获取布尔值
	* @param sApp		段名称
	* @param sKey		字段名称
	* @param bDefValue	默认值
	* @return 获取的值
	*/
	virtual BOOL GetBool(THString sApp,THString sKey,BOOL bDefValue=FALSE)
	{
		return (GetInt(sApp,sKey,bDefValue)!=0)?TRUE:FALSE;
	}

	/**
	* @brief 获取二进制数据
	* @param sApp		段名称
	* @param sKey		字段名称
	* @param pBuffer	数据指针
	* @param uSize		数据大小
	* @return 获取的值
	*/
	virtual BOOL GetBin(THString sApp,THString sKey,void *pBuffer,UINT uSize)
	{
		if (!m_pStore) return FALSE;
		if (m_bUseEncMode)
			return m_pStore->IniGetBin(m_sPath,EncBuffer(sApp),EncBuffer(sKey),pBuffer,uSize);
		return m_pStore->IniGetBin(m_sPath,sApp,sKey,pBuffer,uSize);
	}

	/**
	* @brief 保存字符串
	* @param sApp		段名称
	* @param sKey		字段名称
	* @param sValue		值
	* @return 是否成功
	*/
	virtual BOOL WriteString(THString sApp,THString sKey,THString sValue)
	{
		if (!m_pStore) return FALSE;
		if (m_bUseEncMode)
			return m_pStore->IniWriteString(m_sPath,EncBuffer(sApp),EncBuffer(sKey),EncBuffer(sValue));
		return m_pStore->IniWriteString(m_sPath,sApp,sKey,sValue);
	}

	/**
	* @brief 写入字符串数组
	* @param sApp		段名称
	* @param sKey		字段名称
	* @param arDef	默认值
	* @return 是否成功
	*/
	virtual BOOL WriteStringArray(THString sApp,THString sKey,THStringArray &arDef)
	{
		THString ret;
		for(int i=0;i<arDef.GetSize();i++)
		{
			if (!ret.IsEmpty()) ret+=_T("|");
			ret+=THBase64::EncodeStr(arDef[i]);
		}
		return WriteString(sApp,sKey,ret);
	}

	/**
	* @brief 增加字符串
	* @param sApp		段名称
	* @param sKey		字段名称
	* @param sValue		值
	* @return 是否成功
	*/
	virtual BOOL AppendString(THString sApp,THString sKey,THString sValue)
	{
		return WriteString(sApp,sKey,GetString(sApp,sKey)+sValue);
	}

	/**
	* @brief 保存整数值
	* @param sApp		段名称
	* @param sKey		字段名称
	* @param iValue		值
	* @return 是否成功
	*/
	virtual BOOL WriteInt(THString sApp,THString sKey,int iValue)
	{
		THString str;
		str.Format(_T("%d"),iValue);
		return WriteString(sApp,sKey,str);
	}

	/**
	* @brief 保存布尔值
	* @param sApp		段名称
	* @param sKey		字段名称
	* @param bValue		值
	* @return 是否成功
	*/
	virtual BOOL WriteBool(THString sApp,THString sKey,BOOL bValue)
	{
		return WriteString(sApp,sKey,bValue?_T("1"):_T("0"));
	}

	/**
	* @brief 保存二进制数据
	* @param sApp		段名称
	* @param sKey		字段名称
	* @param pBuffer	数据指针
	* @param uSize		数据大小
	* @return 是否成功
	*/
	virtual BOOL WriteBin(THString sApp,THString sKey,void *pBuffer,UINT uSize)
	{
		if (!m_pStore) return FALSE;
		if (m_bUseEncMode)
			return m_pStore->IniWriteBin(m_sPath,EncBuffer(sApp),EncBuffer(sKey),pBuffer,uSize);
		return m_pStore->IniWriteBin(m_sPath,sApp,sKey,pBuffer,uSize);
	}

	/**
	* @brief 枚举文件中的所有段
	* @param arKey		返回键值名数组
	* @return 是否成功
	*/
	virtual BOOL EnumAllStruct(THStringArray *arKey)
	{
		if (!arKey) return FALSE;
		if (!m_pStore) return FALSE;
		if (!m_pStore->IniEnumAllStruct(m_sPath,arKey)) return FALSE;
		if (m_bUseEncMode)
		{
			for(int i=0;i<arKey->GetSize();i++)
			{
				THString str=arKey->GetAt(i);
				arKey->SetAt(i,DecBuffer(str));
			}
		}
		return TRUE;
	}

	/**
	* @brief 枚举段中的所有键值名
	* @param sKey		键值名
	* @param arKey		返回键值名数组
	* @param arValue	返回键值内容数组
	* @return 是否成功
	*/
	virtual BOOL EnumStruct(const THString sKey,THStringArray *arKey,THStringArray *arValue)
	{
		if (!arKey || !arValue) return FALSE;
		if (!m_pStore) return FALSE;
		if (!m_pStore->IniEnumStruct(m_sPath,EncBuffer(sKey),arKey,arValue)) return FALSE;
		if (m_bUseEncMode)
		{
			for(int i=0;i<arKey->GetSize();i++)
			{
				THString str=arKey->GetAt(i);
				arKey->SetAt(i,DecBuffer(str));
				str=arValue->GetAt(i);
				arValue->SetAt(i,DecBuffer(str));
			}
		}
		return TRUE;
	}

	/**
	* @brief 返回Ini文件的路径
	* @return Ini路径
	*/
	THString GetIniPath(){return m_sPath;}

	/**
	* @brief 返回是否已经初始化
	* @return Ini路径
	*/
	BOOL IsInited()
	{
		if (!m_pStore) return FALSE;
		return (m_sPath.IsEmpty()==FALSE);
	}

	/**
	* @brief 是否使用轻量加密模式
	* @return 是否使用轻量加密模式
	*/
	BOOL IsEncMode(){return m_bUseEncMode;}

	BOOL IsEncCrcMode(){return m_bUseEncMode && m_bCrc;}

	/**
	* @brief 设置密码
	* @param sPass		加密密码，Ini进入轻量加密模式，为空为使用正常模式
	*/
	void InitPass(THString sPass)
	{
		m_bUseEncMode=!sPass.IsEmpty();
		if (m_bUseEncMode)
		{
			PassSeed[0]=0x51;PassSeed[1]=0xcc;
			PassSeed[2]=0xc6;PassSeed[3]=0x1f;
			PassSeed[4]=0xa5;PassSeed[5]=0x21;
			PassSeed[6]=0x85;PassSeed[7]=0x2b;
			int idx=0;
			for(int i=0;i<sPass.GetLength();i++)
			{
				PassSeed[(idx++)%8]^=sPass.GetAt(i);
			}
		}
	}

	void UseEncodeCrcMode(BOOL bCrc=TRUE){m_bCrc=bCrc;}
protected:
	ITHFileStore *m_pStore;
	THString m_sPath;
	unsigned char PassSeed[8];
	BOOL m_bUseEncMode;
	BOOL m_bCrc;

	virtual THString EncBuffer(THString sBuf)
	{
		if (m_bUseEncMode)
		{
			THString ret;
			int idx=0;
			int len=sBuf.GetLength()*sizeof(TCHAR);
			unsigned char *buf=(unsigned char *)sBuf.GetBuffer();
			for(int i=0;i<len;i++)
				ret.AppendFormat(_T("%02x"),(unsigned char)(*(buf+i))^PassSeed[(idx++)%8]);
			if (m_bCrc)
				ret.AppendFormat(_T("%04x"),THCrc::Crc16(buf,len));
			sBuf.ReleaseBuffer();
			return ret;
		}
		return sBuf;
	}

	virtual THString DecBuffer(THString sBuf)
	{
		if (m_bUseEncMode)
		{
			int idx=0;
			THString ret;
			unsigned char *buf=(unsigned char *)ret.GetBuffer(sBuf.GetLength()+2);
			int len=sBuf.GetLength();
			if (m_bCrc) len-=4;
			if (len<0) return _T("");
			for(int i=0;i<len;i+=2)
			{
				unsigned char ch=THStringConv::s2h(sBuf.Mid(i).Left(2));
				*(buf+idx++)=ch^PassSeed[(idx)%8];
			}
			if (m_bCrc)
			{
				U16 ch=THStringConv::s2h(sBuf.Mid(len).Left(4));
				if (THCrc::Crc16(buf,idx)!=ch) return _T("");
			}
			*(buf+idx++)='\0';
			*(buf+idx++)='\0';
			ret.ReleaseBuffer();
			return ret;
		}
		return sBuf;
	}
};

/**
* @brief 硬盘配置文件读写类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-05-28 新建类
* @2007-06-14 加入枚举键值及子键功能
* @2007-07-10 加入判断是否初始化函数
* @2007-09-08 更改协议接口，支持多种模式下的储存器
* @2009-08-29 支持整个ini加密/解密还原
*/
/**<pre>
  使用Sample：
  THIni ini(_T("c:\\abc.ini"));
  ini.GetString(_T("section1"),_T("key1"),_T("defval"));
  ini.WriteInt(_T("section1"),_T("key1"),1);
  THStringArray ar,ar1;
  if (ini.EnumAllStruct(&ar))
    for(int i=0;i<ar.GetSize();i++)
      THDebug(_T("Value:%s"),ar[i]);
  if (theApp.m_ini.EnumStruct(_T("sect1"),&ar,&ar1))
    for(int i=0;i<ar.GetSize();i++)
      THDebug(_T("%s=%s"),ar[i],ar1[i]);
</pre>*/
class THIni : public ITHIni
{
public:
	/**
	* @brief 初始化路径
	* @param sPath		路径
	*/
	THIni():ITHIni()
	{
		THFileStore store;
		InitStore(store.CreateObject(),TRUE);
	}

	/**
	* @brief 初始化路径
	* @param sPath		路径
	*/
	THIni(THString sPath):ITHIni()
	{
		THFileStore store;
		InitStore(store.CreateObject(),TRUE,sPath);
	}

	virtual ~THIni(){}

	ITHIni *CreateObject(THString sPath=_T(""))
	{
		return new THIni(sPath);
	}
};
