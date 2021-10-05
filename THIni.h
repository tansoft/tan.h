
#pragma once

#include <THString.h>
#include <THSystem.h>
#include <THFileStore.h>
#include <THArith.h>

//д��ֵ�Ͷ�ȡֵ�Ķ���꣬����ʹ��
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

//д��ֵ�Ͷ�ȡֵ�Զ�ƥ���ֵ����꣬����ʹ��
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
* @brief Ini�����ļ���д�ӿ���
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-09-08 �½���
*/
/**<pre>
  ʹ��Sample��
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
	* @brief ��ʼ��·��,������ʵ����ʵ�ָýӿ�
	* @param sPath		·��
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
			//���麯������
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
	* @brief ����ʵ������
	* @param sPath		Ini�����ļ�·��
	* @return �½�����ʵ��ʵ�����ʵ��ָ��
	*/
	virtual ITHIni *CreateObject(THString sPath=_T("")){return new ITHIni(sPath);};

	/**
	* @brief ����ini��ԭini��Ŀ��ini������ͬһ���ļ�����Ҫ����ini�ļ���������/���ܣ������Ż�ini��ֵλ����
	* @param srcini		�����Ƶ�ini����
	* @return �Ƿ�ɹ�
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
	* @brief ��ȡ�ַ���
	* @param sApp		������
	* @param sKey		�ֶ�����
	* @param sDefValue	Ĭ��ֵ
	* @return ��ȡ��ֵ
	*/
	virtual THString GetString(THString sApp,THString sKey,THString sDefValue=_T(""))
	{
		if (!m_pStore) return sDefValue;
		if (m_bUseEncMode)
			return DecBuffer(m_pStore->IniGetString(m_sPath,EncBuffer(sApp),EncBuffer(sKey),EncBuffer(sDefValue)));
		return m_pStore->IniGetString(m_sPath,sApp,sKey,sDefValue);
	}

	/**
	* @brief ��ȡ�ַ�������
	* @param sApp		������
	* @param sKey		�ֶ�����
	* @param arDef	��������
	* @param bEmptyAr �Ƿ����������
	* @return �Ƿ�ɹ�
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
	* @brief ��ȡ����ֵ
	* @param sApp		������
	* @param sKey		�ֶ�����
	* @param iDefValue	Ĭ��ֵ
	* @return ��ȡ��ֵ
	*/
	virtual int GetInt(THString sApp,THString sKey,int iDefValue=0)
	{
		return THStringConv::s2i(GetString(sApp,sKey,THStringConv::i2s(iDefValue)));
	}

	/**
	* @brief ��ȡ����ֵ
	* @param sApp		������
	* @param sKey		�ֶ�����
	* @param bDefValue	Ĭ��ֵ
	* @return ��ȡ��ֵ
	*/
	virtual BOOL GetBool(THString sApp,THString sKey,BOOL bDefValue=FALSE)
	{
		return (GetInt(sApp,sKey,bDefValue)!=0)?TRUE:FALSE;
	}

	/**
	* @brief ��ȡ����������
	* @param sApp		������
	* @param sKey		�ֶ�����
	* @param pBuffer	����ָ��
	* @param uSize		���ݴ�С
	* @return ��ȡ��ֵ
	*/
	virtual BOOL GetBin(THString sApp,THString sKey,void *pBuffer,UINT uSize)
	{
		if (!m_pStore) return FALSE;
		if (m_bUseEncMode)
			return m_pStore->IniGetBin(m_sPath,EncBuffer(sApp),EncBuffer(sKey),pBuffer,uSize);
		return m_pStore->IniGetBin(m_sPath,sApp,sKey,pBuffer,uSize);
	}

	/**
	* @brief �����ַ���
	* @param sApp		������
	* @param sKey		�ֶ�����
	* @param sValue		ֵ
	* @return �Ƿ�ɹ�
	*/
	virtual BOOL WriteString(THString sApp,THString sKey,THString sValue)
	{
		if (!m_pStore) return FALSE;
		if (m_bUseEncMode)
			return m_pStore->IniWriteString(m_sPath,EncBuffer(sApp),EncBuffer(sKey),EncBuffer(sValue));
		return m_pStore->IniWriteString(m_sPath,sApp,sKey,sValue);
	}

	/**
	* @brief д���ַ�������
	* @param sApp		������
	* @param sKey		�ֶ�����
	* @param arDef	Ĭ��ֵ
	* @return �Ƿ�ɹ�
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
	* @brief �����ַ���
	* @param sApp		������
	* @param sKey		�ֶ�����
	* @param sValue		ֵ
	* @return �Ƿ�ɹ�
	*/
	virtual BOOL AppendString(THString sApp,THString sKey,THString sValue)
	{
		return WriteString(sApp,sKey,GetString(sApp,sKey)+sValue);
	}

	/**
	* @brief ��������ֵ
	* @param sApp		������
	* @param sKey		�ֶ�����
	* @param iValue		ֵ
	* @return �Ƿ�ɹ�
	*/
	virtual BOOL WriteInt(THString sApp,THString sKey,int iValue)
	{
		THString str;
		str.Format(_T("%d"),iValue);
		return WriteString(sApp,sKey,str);
	}

	/**
	* @brief ���沼��ֵ
	* @param sApp		������
	* @param sKey		�ֶ�����
	* @param bValue		ֵ
	* @return �Ƿ�ɹ�
	*/
	virtual BOOL WriteBool(THString sApp,THString sKey,BOOL bValue)
	{
		return WriteString(sApp,sKey,bValue?_T("1"):_T("0"));
	}

	/**
	* @brief �������������
	* @param sApp		������
	* @param sKey		�ֶ�����
	* @param pBuffer	����ָ��
	* @param uSize		���ݴ�С
	* @return �Ƿ�ɹ�
	*/
	virtual BOOL WriteBin(THString sApp,THString sKey,void *pBuffer,UINT uSize)
	{
		if (!m_pStore) return FALSE;
		if (m_bUseEncMode)
			return m_pStore->IniWriteBin(m_sPath,EncBuffer(sApp),EncBuffer(sKey),pBuffer,uSize);
		return m_pStore->IniWriteBin(m_sPath,sApp,sKey,pBuffer,uSize);
	}

	/**
	* @brief ö���ļ��е����ж�
	* @param arKey		���ؼ�ֵ������
	* @return �Ƿ�ɹ�
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
	* @brief ö�ٶ��е����м�ֵ��
	* @param sKey		��ֵ��
	* @param arKey		���ؼ�ֵ������
	* @param arValue	���ؼ�ֵ��������
	* @return �Ƿ�ɹ�
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
	* @brief ����Ini�ļ���·��
	* @return Ini·��
	*/
	THString GetIniPath(){return m_sPath;}

	/**
	* @brief �����Ƿ��Ѿ���ʼ��
	* @return Ini·��
	*/
	BOOL IsInited()
	{
		if (!m_pStore) return FALSE;
		return (m_sPath.IsEmpty()==FALSE);
	}

	/**
	* @brief �Ƿ�ʹ����������ģʽ
	* @return �Ƿ�ʹ����������ģʽ
	*/
	BOOL IsEncMode(){return m_bUseEncMode;}

	BOOL IsEncCrcMode(){return m_bUseEncMode && m_bCrc;}

	/**
	* @brief ��������
	* @param sPass		�������룬Ini������������ģʽ��Ϊ��Ϊʹ������ģʽ
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
* @brief Ӳ�������ļ���д��
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-05-28 �½���
* @2007-06-14 ����ö�ټ�ֵ���Ӽ�����
* @2007-07-10 �����ж��Ƿ��ʼ������
* @2007-09-08 ����Э��ӿڣ�֧�ֶ���ģʽ�µĴ�����
* @2009-08-29 ֧������ini����/���ܻ�ԭ
*/
/**<pre>
  ʹ��Sample��
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
	* @brief ��ʼ��·��
	* @param sPath		·��
	*/
	THIni():ITHIni()
	{
		THFileStore store;
		InitStore(store.CreateObject(),TRUE);
	}

	/**
	* @brief ��ʼ��·��
	* @param sPath		·��
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
