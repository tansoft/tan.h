#pragma once

#define THSYSTEM_INCLUDE_VERSION
#include <THSystem.h>
#include <THTime.h>

/**
* @brief 文件版本信息读取类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-10-15 新建类
*/
/**<pre>
  使用Sample：
</pre>*/
class THFileVersionInfo
{
public:
	THFileVersionInfo(){Reset();}
	virtual ~THFileVersionInfo(){}

	BOOL Init(HMODULE hModule = NULL){return Init(THSysMisc::GetModuleFileName(hModule));}

	BOOL Init(THString sFileName)
	{
		Reset();
		DWORD dwHandle;
		DWORD dwFileVersionInfoSize = GetFileVersionInfoSize(sFileName.GetBuffer(),&dwHandle);
		sFileName.ReleaseBuffer();
		if (!dwFileVersionInfoSize)	return FALSE;

		LPVOID	lpData = (LPVOID)new BYTE[dwFileVersionInfoSize];
		if (!lpData) return FALSE;
		try
		{
			if (!GetFileVersionInfo((LPCTSTR)sFileName, dwHandle, dwFileVersionInfoSize, lpData)) throw FALSE;
			// catch default information
			LPVOID lpInfo;
			UINT unInfoLen;
			if (VerQueryValue(lpData, _T("\\"), &lpInfo, &unInfoLen))
			{
				if (unInfoLen == sizeof(m_FileInfo))
					memcpy(&m_FileInfo, lpInfo, unInfoLen);
			}
			// find best matching language and codepage
			VerQueryValue(lpData, _T("\\VarFileInfo\\Translation"), &lpInfo, &unInfoLen);
			DWORD dwLangCode = 0;
			if (!GetTranslationId(lpInfo, unInfoLen, GetUserDefaultLangID(), dwLangCode, FALSE))
			{
				if (!GetTranslationId(lpInfo, unInfoLen, GetUserDefaultLangID(), dwLangCode, TRUE))
				{
					if (!GetTranslationId(lpInfo, unInfoLen, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), dwLangCode, TRUE))
					{
						if (!GetTranslationId(lpInfo, unInfoLen, MAKELANGID(LANG_ENGLISH, SUBLANG_NEUTRAL), dwLangCode, TRUE))
							// use the first one we can get
							dwLangCode = *((DWORD*)lpInfo);
					}
				}
			}
			THString strSubBlock;
			strSubBlock.Format(_T("\\StringFileInfo\\%04X%04X\\"), dwLangCode&0x0000FFFF, (dwLangCode&0xFFFF0000)>>16);
			// catch string table
			if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock+_T("CompanyName")), &lpInfo, &unInfoLen))
				m_strCompanyName = THString((LPCTSTR)lpInfo);
			if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock+_T("FileDescription")), &lpInfo, &unInfoLen))
				m_strFileDescription = THString((LPCTSTR)lpInfo);
			if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock+_T("FileVersion")), &lpInfo, &unInfoLen))
				m_strFileVersion = THString((LPCTSTR)lpInfo);
			if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock+_T("InternalName")), &lpInfo, &unInfoLen))
				m_strInternalName = THString((LPCTSTR)lpInfo);
			if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock+_T("LegalCopyright")), &lpInfo, &unInfoLen))
				m_strLegalCopyright = THString((LPCTSTR)lpInfo);
			if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock+_T("OriginalFileName")), &lpInfo, &unInfoLen))
				m_strOriginalFileName = THString((LPCTSTR)lpInfo);
			if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock+_T("ProductName")), &lpInfo, &unInfoLen))
				m_strProductName = THString((LPCTSTR)lpInfo);
			if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock+_T("ProductVersion")), &lpInfo, &unInfoLen))
				m_strProductVersion = THString((LPCTSTR)lpInfo);
			if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock+_T("Comments")), &lpInfo, &unInfoLen))
				m_strComments = THString((LPCTSTR)lpInfo);
			if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock+_T("LegalTrademarks")), &lpInfo, &unInfoLen))
				m_strLegalTrademarks = THString((LPCTSTR)lpInfo);
			if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock+_T("PrivateBuild")), &lpInfo, &unInfoLen))
				m_strPrivateBuild = THString((LPCTSTR)lpInfo);
			if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock+_T("SpecialBuild")), &lpInfo, &unInfoLen))
				m_strSpecialBuild = THString((LPCTSTR)lpInfo);
			delete[] lpData;
		}
		catch (BOOL)
		{
			delete[] lpData;
			return FALSE;
		}
		return TRUE;
	}

	//least significant
	DWORD GetFileVersionLS() const {return m_FileInfo.dwFileVersionLS;}
	//most significant
	DWORD GetFileVersionMS() const {return m_FileInfo.dwFileVersionMS;}
	WORD GetFileVersion(int nIndex) const
	{
		if (nIndex == 0) return (WORD)(m_FileInfo.dwFileVersionLS & 0x0000FFFF);
		else if (nIndex == 1) return (WORD)((m_FileInfo.dwFileVersionLS & 0xFFFF0000) >> 16);
		else if (nIndex == 2) return (WORD)(m_FileInfo.dwFileVersionMS & 0x0000FFFF);
		else if (nIndex == 3) return (WORD)((m_FileInfo.dwFileVersionMS & 0xFFFF0000) >> 16);
		return 0;
	}

	WORD GetProductVersion(int nIndex) const
	{
		if (nIndex == 0) return (WORD)(m_FileInfo.dwProductVersionLS & 0x0000FFFF);
		else if (nIndex == 1) return (WORD)((m_FileInfo.dwProductVersionLS & 0xFFFF0000) >> 16);
		else if (nIndex == 2) return (WORD)(m_FileInfo.dwProductVersionMS & 0x0000FFFF);
		else if (nIndex == 3) return (WORD)((m_FileInfo.dwProductVersionMS & 0xFFFF0000) >> 16);
		return 0;
	}

	DWORD GetFileFlagsMask() const {return m_FileInfo.dwFileFlagsMask;}
	DWORD GetFileFlags() const {return m_FileInfo.dwFileFlags;}
	DWORD GetFileOs() const {return m_FileInfo.dwFileOS;}
	DWORD GetFileType() const {return m_FileInfo.dwFileType;}
	DWORD GetFileSubtype() const {return m_FileInfo.dwFileSubtype;}

	THTime GetFileDate() const
	{
		FILETIME ft;
		ft.dwLowDateTime = m_FileInfo.dwFileDateLS;
		ft.dwHighDateTime = m_FileInfo.dwFileDateMS;
		return THTime(ft);
	}

	THString GetCompanyName() const {return m_strCompanyName;}
	THString GetFileDescription() const {return m_strFileDescription;}
	THString GetFileVersionFormatDot() const
	{
		THString ret;
		ret.Format(_T("%u.%u.%u.%u"),GetFileVersion(3),GetFileVersion(2),GetFileVersion(1),GetFileVersion(0));
		return ret;
	}
	THString GetFileVersion() const {return m_strFileVersion;}
	THString GetInternalName() const {return m_strInternalName;}
	THString GetLegalCopyright() const {return m_strLegalCopyright;}
	THString GetOriginalFileName() const {return m_strOriginalFileName;}
	THString GetProductName() const {return m_strProductName;}
	THString GetProductVersion() const {return m_strProductVersion;}
	THString GetComments() const {return m_strComments;}
	THString GetLegalTrademarks() const {return m_strLegalTrademarks;}
	THString GetPrivateBuild() const {return m_strPrivateBuild;}
	THString GetSpecialBuild() const {return m_strSpecialBuild;}
protected:
	virtual void Reset()
	{
		ZeroMemory(&m_FileInfo, sizeof(m_FileInfo));
		m_strCompanyName.Empty();
		m_strFileDescription.Empty();
		m_strFileVersion.Empty();
		m_strInternalName.Empty();
		m_strLegalCopyright.Empty();
		m_strOriginalFileName.Empty();
		m_strProductName.Empty();
		m_strProductVersion.Empty();
		m_strComments.Empty();
		m_strLegalTrademarks.Empty();
		m_strPrivateBuild.Empty();
		m_strSpecialBuild.Empty();
	}
	BOOL GetTranslationId(LPVOID lpData, UINT unBlockSize, WORD wLangId, DWORD &dwId, BOOL bPrimaryEnough = FALSE)
	{
		for (LPWORD lpwData = (LPWORD)lpData; (LPBYTE)lpwData < ((LPBYTE)lpData)+unBlockSize; lpwData+=2)
		{
			if (*lpwData == wLangId)
			{
				dwId = *((DWORD*)lpwData);
				return TRUE;
			}
		}
		if (!bPrimaryEnough) return FALSE;
		for (lpwData = (LPWORD)lpData; (LPBYTE)lpwData < ((LPBYTE)lpData)+unBlockSize; lpwData+=2)
		{
			if (((*lpwData)&0x00FF) == (wLangId&0x00FF))
			{
				dwId = *((DWORD*)lpwData);
				return TRUE;
			}
		}
		return FALSE;
	}
private:
	VS_FIXEDFILEINFO m_FileInfo;
	THString m_strCompanyName;
	THString m_strFileDescription;
	THString m_strFileVersion;
	THString m_strInternalName;
	THString m_strLegalCopyright;
	THString m_strOriginalFileName;
	THString m_strProductName;
	THString m_strProductVersion;
	THString m_strComments;
	THString m_strLegalTrademarks;
	THString m_strPrivateBuild;
	THString m_strSpecialBuild;
};
