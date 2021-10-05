#pragma once

#include <THString.h>

/**
* @brief 硬盘配置文件读写类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-05-28 新建类
* @2007-06-14 加入枚举键值及子键功能
* @2007-07-10 加入判断是否初始化函数
*/
/**<pre>
  使用Sample：
  CMyIni ini(_T("c:\\abc.ini"));
  ini.GetString(_T("section1"),_T("key1"),_T("defval"));
  ini.WriteInt(_T("section1"),_T("key1"),1);
  CStringArray ar,ar1;
  if (ini.EnumAllStruct(&ar))
    for(int i=0;i<ar.GetSize();i++)
      TRACE(_T("Value:%s"),ar[i]);
  if (theApp.m_ini.EnumStruct(_T("sect1"),&ar,&ar1))
    for(int i=0;i<ar.GetSize();i++)
      TRACE(_T("%s=%s"),ar[i],ar1[i]);
</pre>*/
class THIni
{
public:
	/**
	* @brief 初始化路径
	*/
	THIni(){}
	/**
	* @brief 初始化路径
	* @param sPath		路径
	*/
	THIni(CString sPath){Init(sPath);}

	virtual ~THIni(){}

	/**
	* @brief 初始化路径
	* @param sIniFile		路径
	*/
	inline BOOL Init(CString sIniFile=_T(""))
	{
		m_sPath=sIniFile;
		return TRUE;
	}


	/**
	* @brief 获取字符串
	* @param sApp		段名称
	* @param sKey		字段名称
	* @param sDefValue	默认值
	* @return 获取的值
	*/
	inline CString GetString(CString sApp,CString sKey,CString sDefValue=_T(""))
	{
		if (m_bUseEncMode)
			return DecBuffer(IniGetString(m_sPath,EncBuffer(sApp),EncBuffer(sKey),EncBuffer(sDefValue)));
		return IniGetString(m_sPath,sApp,sKey,sDefValue);
	}

	/**
	* @brief 获取整数值
	* @param sApp		段名称
	* @param sKey		字段名称
	* @param iDefValue	默认值
	* @return 获取的值
	*/
	inline int GetInt(CString sApp,CString sKey,int iDefValue=0)
	{
		CString ret;
		ret.Format(_T("%d"),iDefValue);
		return _ttoi(GetString(sApp,sKey,ret));
	}

	/**
	* @brief 获取布尔值
	* @param sApp		段名称
	* @param sKey		字段名称
	* @param bDefValue	默认值
	* @return 获取的值
	*/
	inline BOOL GetBool(CString sApp,CString sKey,BOOL bDefValue=FALSE)
	{return (GetInt(sApp,sKey,bDefValue)!=0)?TRUE:FALSE;}

	/**
	* @brief 保存字符串
	* @param sApp		段名称
	* @param sKey		字段名称
	* @param sValue		值
	* @return 是否成功
	*/
	inline BOOL WriteString(CString sApp,CString sKey,CString sValue)
	{
		if (m_bUseEncMode)
			return IniWriteString(m_sPath,EncBuffer(sApp),EncBuffer(sKey),EncBuffer(sValue));
		return IniWriteString(m_sPath,sApp,sKey,sValue);
	}

	/**
	* @brief 增加字符串
	* @param sApp		段名称
	* @param sKey		字段名称
	* @param sValue		值
	* @return 是否成功
	*/
	inline BOOL AppendString(CString sApp,CString sKey,CString sValue)
	{return WriteString(sApp,sKey,GetString(sApp,sKey)+sValue);}

	/**
	* @brief 保存整数值
	* @param sApp		段名称
	* @param sKey		字段名称
	* @param iValue		值
	* @return 是否成功
	*/
	inline BOOL WriteInt(CString sApp,CString sKey,int iValue)
	{
		CString str;
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
	inline BOOL WriteBool(CString sApp,CString sKey,BOOL bValue)
	{return WriteString(sApp,sKey,bValue?_T("1"):_T("0"));}

	/**
	* @brief 返回Ini文件的路径
	* @return Ini路径
	*/
	inline CString GetIniPath(){return m_sPath;}

	/**
	* @brief 返回是否已经初始化
	* @return Ini路径
	*/
	inline BOOL IsInited(){return (m_sPath.IsEmpty()==FALSE);}

	/**
	* @brief 是否使用轻量加密模式
	* @return 是否使用轻量加密模式
	*/
	inline BOOL IsEncMode(){return m_bUseEncMode;}

	/**
	* @brief 设置密码
	* @param sPass		加密密码，Ini进入轻量加密模式，为空为使用正常模式
	*/
	void InitPass(CString sPass)
	{
	}

protected:
	CString m_sPath;
	unsigned char PassSeed[8];
	BOOL m_bUseEncMode;

	virtual CString EncBuffer(CString sBuf)
	{
		return sBuf;
	}
	virtual CString DecBuffer(CString sBuf)
	{
		return sBuf;
	}
	virtual CString IniGetString(CString sFile,CString sApp,CString sKey,CString sDefValue=_T(""))
	{
		CString buffer;
		int InitSize=4096;
		while(GetPrivateProfileString(sApp,sKey,sDefValue,buffer.GetBuffer(InitSize),InitSize,sFile)==InitSize)
		{
			//缓冲区可能不够，加大缓冲区重试
			buffer.ReleaseBuffer();
			InitSize*=2;
		}
		buffer.ReleaseBuffer();
		return buffer;
	}
	virtual BOOL IniWriteString(CString sFile,CString sApp,CString sKey,CString sValue)
	{
		return WritePrivateProfileString(sApp,sKey,sValue,sFile);
	}
};
