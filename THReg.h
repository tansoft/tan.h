#pragma once

#include <THSystem.h>
#include <THString.h>

/**
* @brief 注册表封装类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-03-10 新建类
*/
/**<pre>
  使用Sample：
</pre>*/
class THReg
{
public:
	THReg(){m_hKey = NULL;}
	virtual ~THReg(){Close();}

public:
	BOOL Open(HKEY hKeyRoot,THString sPath)
	{
		DWORD dw;
		m_sPath=sPath;
		return RegCreateKeyEx(hKeyRoot,sPath,0L,NULL,REG_OPTION_VOLATILE,KEY_ALL_ACCESS,NULL,&m_hKey,&dw)==ERROR_SUCCESS;
	}

	void Close()
	{
		if (m_hKey)
		{
			RegCloseKey (m_hKey);
			m_hKey = NULL;
		}
	}

	BOOL WriteDword(THString sKey, DWORD dwVal)
	{
		if (!m_hKey) return FALSE;
		return RegSetValueEx(m_hKey,sKey,0L,REG_DWORD,(CONST BYTE*)&dwVal,sizeof(DWORD))==ERROR_SUCCESS;
	}

	BOOL WriteString(THString sKey, THString sVal)
	{
		if (!m_hKey) return FALSE;
		return RegSetValueEx(m_hKey,sKey,0L,REG_SZ,(CONST BYTE*)(LPCTSTR)sVal,(sVal.GetLength()+1)*sizeof(TCHAR))==ERROR_SUCCESS;
	}
	BOOL WriteBuffer(THString sKey, const void* pData,DWORD dwLength)
	{
		if (!m_hKey || !pData || dwLength==0) return FALSE;
		return RegSetValueEx(m_hKey,sKey,0L,REG_BINARY,(CONST BYTE*)pData,dwLength)==ERROR_SUCCESS;
	}

	DWORD ReadDword(THString sKey,DWORD defValue=0)
	{
		if (m_hKey)
		{
			DWORD dwType;
			DWORD dwSize = sizeof(DWORD);
			DWORD dwDest;
			LONG lRet = RegQueryValueEx(m_hKey,sKey,NULL,&dwType,(BYTE *)&dwDest,&dwSize);
			if (lRet == ERROR_SUCCESS && dwType==REG_DWORD) defValue=dwDest;
		}
		return defValue;
	}

	THString ReadString(THString sKey,THString defStr=_T(""))
	{
		if (m_hKey)
		{
			DWORD dwType;
			DWORD dwSize = 65536;
			THString str;
			LONG lRet = RegQueryValueEx(m_hKey,sKey,NULL,&dwType,(BYTE *)str.GetBuffer(65536),&dwSize);
			str.ReleaseBuffer();
			if (lRet == ERROR_SUCCESS && (dwType==REG_SZ || dwType==REG_EXPAND_SZ)) defStr=str;
		}
		return defStr;
	}

	BOOL ReadBuffer(THString sKey, void* pData, DWORD *dwLength)
	{
		DWORD dwType;
		LONG lRet=RegQueryValueEx(m_hKey,sKey,NULL,&dwType,(BYTE *)pData,dwLength);
		if (lRet==ERROR_SUCCESS && dwType==REG_BINARY) return TRUE;
		return FALSE;
	}


	BOOL GetEnumKeys(THStringArray *pArray)
	{
		if (!pArray || !m_hKey) return FALSE;
		pArray->RemoveAll();
		long lRetCode=ERROR_SUCCESS;
		TCHAR szValue[MAX_PATH];
		for (int nIndex=0;lRetCode==ERROR_SUCCESS;nIndex++)
		{
			lRetCode=RegEnumKey(m_hKey,nIndex,szValue,MAX_PATH);
			if (lRetCode==ERROR_SUCCESS)
				pArray->Add(szValue);
		}
		return TRUE;
	}

	static BOOL GetEnumKeys(HKEY hKeyRoot,THString sPath,THStringArray *pArray)
	{
		THReg reg;
		if (!reg.Open(hKeyRoot,sPath)) return FALSE;
		reg.GetEnumKeys(pArray);
		reg.Close();
		return TRUE;
	}

	BOOL GetEnumValues(THStringArray *pArray)
	{
		if (!pArray || !m_hKey) return FALSE;
		pArray->RemoveAll();
		long lRetCode=ERROR_SUCCESS;
		TCHAR szValue[MAX_PATH];
		DWORD dwValue;
		for (int nIndex=0;lRetCode==ERROR_SUCCESS;nIndex++)
		{
			szValue[0]='\0';
			dwValue=MAX_PATH;
			lRetCode=RegEnumValue(m_hKey,nIndex,szValue,&dwValue,NULL,NULL,NULL,NULL);
			if (lRetCode==ERROR_SUCCESS)
				pArray->Add(szValue);
		}
		return TRUE;
	}

	static BOOL GetEnumValues(HKEY hKeyRoot,THString sPath,THStringArray *pArray)
	{
		THReg reg;
		if (!reg.Open(hKeyRoot,sPath)) return FALSE;
		reg.GetEnumValues(pArray);
		reg.Close();
		return TRUE;
	}

	static BOOL DeleteKey(HKEY hKey,THString sKey,THString sValueName=_T(""))
	{
		HKEY hSubKey;
		LONG lRet =	RegOpenKeyEx(hKey,sKey,0,KEY_WRITE | KEY_READ,&hSubKey);
		if( lRet != ERROR_SUCCESS) return FALSE ;
		if (!sValueName.IsEmpty())
		{
			//仅删除该键
			lRet=RegDeleteValue(hSubKey,sValueName);
			RegCloseKey(hSubKey);
			RegCloseKey(hKey);
			if(lRet!=ERROR_SUCCESS) return FALSE;
		} 
		else 
		{
			DWORD dwSubKeyCnt=0;
			do 
			{	
				// 得到子键的信息
				DWORD dwMaxSubKey;
				LONG lRet =	RegQueryInfoKey(
					hSubKey,
					0,					// buffer for class name
					0,					// length of class name string
					0,					// reserved
					&dwSubKeyCnt,		// # of subkeys
					&dwMaxSubKey,		// length of longest subkey
					0,					// length of longest class name string
					0,					// # of values
					0,					// length of longest value name
					0,					// length of longest value data
					0,					// security descriptor
					0					// last write time
					) ;
				if(lRet != ERROR_SUCCESS)
				{
					RegCloseKey(hSubKey);
					return FALSE;
				}
				if(dwSubKeyCnt>0)
				{
					LPTSTR pszKeyName=new TCHAR[dwMaxSubKey+1];
					DWORD dwKeyNameLen = dwMaxSubKey;
					lRet =	RegEnumKey(hSubKey,0,pszKeyName,dwKeyNameLen+1);
					if(lRet!=ERROR_SUCCESS) 
					{
						delete [] pszKeyName;
						RegCloseKey(hSubKey);
						return FALSE;
					}
					if(!DeleteKey(hSubKey,pszKeyName,sValueName))
					{
						delete [] pszKeyName;
						RegCloseKey(hSubKey);
						return FALSE;
					}
					delete [] pszKeyName;
				}
			}while(dwSubKeyCnt>0);
			RegCloseKey(hSubKey);
			lRet = RegDeleteKey(hKey,sKey);
			if(lRet!=ERROR_SUCCESS) return FALSE;
		}
		return TRUE;
	}
protected:
	HKEY m_hKey;
	THString m_sPath;
};

/**
* @brief 注册表自启动项设置封装类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-03-10 新建类
*/
/**<pre>
  使用Sample：
</pre>*/
class THAutoRunReg
{
public:
	THAutoRunReg(){}
	virtual ~THAutoRunReg(){}

	static BOOL SetAutoRun(THString sRegKey,THString sCmdLine,BOOL bSet=TRUE,BOOL bCurrentUser=FALSE,BOOL bRunOnce=FALSE)
	{
		if (sRegKey.IsEmpty() || (sCmdLine.IsEmpty() && bSet==TRUE)) return FALSE;
		HKEY hRoot;
		if (bCurrentUser)
			hRoot=HKEY_CURRENT_USER;
		else
			hRoot=HKEY_LOCAL_MACHINE;
		THString sPath=_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
		if (bRunOnce) sPath+=_T("Once");
		if (bSet)
		{
			THReg reg;
			if (!reg.Open(hRoot,sPath)) return FALSE;
			//not need update, avoid the firewall warning
			if (sCmdLine.CompareNoCase(reg.ReadString(sRegKey))==0) return TRUE;
			return reg.WriteString(sRegKey,sCmdLine);
		}
		else
		{
			return THReg::DeleteKey(hRoot,sPath,sRegKey);
		}
	}
};