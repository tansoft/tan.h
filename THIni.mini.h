#pragma once

#include <THString.h>

/**
* @brief Ӳ�������ļ���д��
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-05-28 �½���
* @2007-06-14 ����ö�ټ�ֵ���Ӽ�����
* @2007-07-10 �����ж��Ƿ��ʼ������
*/
/**<pre>
  ʹ��Sample��
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
	* @brief ��ʼ��·��
	*/
	THIni(){}
	/**
	* @brief ��ʼ��·��
	* @param sPath		·��
	*/
	THIni(CString sPath){Init(sPath);}

	virtual ~THIni(){}

	/**
	* @brief ��ʼ��·��
	* @param sIniFile		·��
	*/
	inline BOOL Init(CString sIniFile=_T(""))
	{
		m_sPath=sIniFile;
		return TRUE;
	}


	/**
	* @brief ��ȡ�ַ���
	* @param sApp		������
	* @param sKey		�ֶ�����
	* @param sDefValue	Ĭ��ֵ
	* @return ��ȡ��ֵ
	*/
	inline CString GetString(CString sApp,CString sKey,CString sDefValue=_T(""))
	{
		if (m_bUseEncMode)
			return DecBuffer(IniGetString(m_sPath,EncBuffer(sApp),EncBuffer(sKey),EncBuffer(sDefValue)));
		return IniGetString(m_sPath,sApp,sKey,sDefValue);
	}

	/**
	* @brief ��ȡ����ֵ
	* @param sApp		������
	* @param sKey		�ֶ�����
	* @param iDefValue	Ĭ��ֵ
	* @return ��ȡ��ֵ
	*/
	inline int GetInt(CString sApp,CString sKey,int iDefValue=0)
	{
		CString ret;
		ret.Format(_T("%d"),iDefValue);
		return _ttoi(GetString(sApp,sKey,ret));
	}

	/**
	* @brief ��ȡ����ֵ
	* @param sApp		������
	* @param sKey		�ֶ�����
	* @param bDefValue	Ĭ��ֵ
	* @return ��ȡ��ֵ
	*/
	inline BOOL GetBool(CString sApp,CString sKey,BOOL bDefValue=FALSE)
	{return (GetInt(sApp,sKey,bDefValue)!=0)?TRUE:FALSE;}

	/**
	* @brief �����ַ���
	* @param sApp		������
	* @param sKey		�ֶ�����
	* @param sValue		ֵ
	* @return �Ƿ�ɹ�
	*/
	inline BOOL WriteString(CString sApp,CString sKey,CString sValue)
	{
		if (m_bUseEncMode)
			return IniWriteString(m_sPath,EncBuffer(sApp),EncBuffer(sKey),EncBuffer(sValue));
		return IniWriteString(m_sPath,sApp,sKey,sValue);
	}

	/**
	* @brief �����ַ���
	* @param sApp		������
	* @param sKey		�ֶ�����
	* @param sValue		ֵ
	* @return �Ƿ�ɹ�
	*/
	inline BOOL AppendString(CString sApp,CString sKey,CString sValue)
	{return WriteString(sApp,sKey,GetString(sApp,sKey)+sValue);}

	/**
	* @brief ��������ֵ
	* @param sApp		������
	* @param sKey		�ֶ�����
	* @param iValue		ֵ
	* @return �Ƿ�ɹ�
	*/
	inline BOOL WriteInt(CString sApp,CString sKey,int iValue)
	{
		CString str;
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
	inline BOOL WriteBool(CString sApp,CString sKey,BOOL bValue)
	{return WriteString(sApp,sKey,bValue?_T("1"):_T("0"));}

	/**
	* @brief ����Ini�ļ���·��
	* @return Ini·��
	*/
	inline CString GetIniPath(){return m_sPath;}

	/**
	* @brief �����Ƿ��Ѿ���ʼ��
	* @return Ini·��
	*/
	inline BOOL IsInited(){return (m_sPath.IsEmpty()==FALSE);}

	/**
	* @brief �Ƿ�ʹ����������ģʽ
	* @return �Ƿ�ʹ����������ģʽ
	*/
	inline BOOL IsEncMode(){return m_bUseEncMode;}

	/**
	* @brief ��������
	* @param sPass		�������룬Ini������������ģʽ��Ϊ��Ϊʹ������ģʽ
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
			//���������ܲ������Ӵ󻺳�������
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
