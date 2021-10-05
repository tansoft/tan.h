#pragma once

#define THSYSTEM_INCLUDE_SHELLOBJ
#include <THSystem.h>
#include <THIni.h>

/**
* @brief WinApp 基类，程序路径及配置文件等
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-05-28 新建类
* @2010-08-30 增加Application Data操作ini，实现兼容Win7/Vista
*/
/**<pre>
注意点：
	Exe路径 m_sExePath 为最后不带 \\ 的，用时需要加上
用法：
	直接把WinApp继承自THWinApp
	把WinApp的InitInstance函数调用THWinApp的InitInstance
</pre>*/
class THWinApp : public CWinApp
{
protected:
	THWinApp():CWinApp(){}
	virtual ~THWinApp(){}

public:
	virtual BOOL InitInstance()
	{
		JustInitInstance();
		return CWinApp::InitInstance();
	}
	//用于MFC DLL等，不需要调用上级CWinApp::InitInstance的调用
	virtual void JustInitInstance()
	{
		m_sExeFile=THSysMisc::GetModuleFileName();
		m_sIniFile=THSysMisc::RenameFileExt(m_sExeFile,_T(".ini"));
		m_sExePath=THSysMisc::GetFilePath(m_sExeFile);
		m_ini.Init(m_sIniFile);
		SHGetSpecialFolderPath(NULL,m_sIniFileEx.GetBuffer(MAX_PATH),CSIDL_APPDATA,FALSE);
		m_sIniFileEx.ReleaseBuffer();
		if (!m_sIniFileEx.IsEmpty()) m_sIniFileEx+=_T("\\");
		m_sIniFileEx+=THSysMisc::RenameFileExt(THSysMisc::GetFileName(m_sExeFile),_T(".ini"));
		m_iniex.Init(m_sIniFileEx);
	}
	virtual int ExitInstance()
	{
		return CWinApp::ExitInstance();
	}
	THString m_sExeFile;				///<程序文件名
	THString m_sExePath;				///<程序所在路径
	THString m_sIniFile;				///<默认配置文件路径
	THIni m_ini;						///<程序配置读取器
	THString m_sIniFileEx;				///<兼容Win7/Vista的ini路径
	THIni m_iniex;						///<兼容Win7/Vista的配置读取器
};
