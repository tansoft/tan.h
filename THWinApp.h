#pragma once

#define THSYSTEM_INCLUDE_SHELLOBJ
#include <THSystem.h>
#include <THIni.h>

/**
* @brief WinApp ���࣬����·���������ļ���
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-05-28 �½���
* @2010-08-30 ����Application Data����ini��ʵ�ּ���Win7/Vista
*/
/**<pre>
ע��㣺
	Exe·�� m_sExePath Ϊ��󲻴� \\ �ģ���ʱ��Ҫ����
�÷���
	ֱ�Ӱ�WinApp�̳���THWinApp
	��WinApp��InitInstance��������THWinApp��InitInstance
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
	//����MFC DLL�ȣ�����Ҫ�����ϼ�CWinApp::InitInstance�ĵ���
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
	THString m_sExeFile;				///<�����ļ���
	THString m_sExePath;				///<��������·��
	THString m_sIniFile;				///<Ĭ�������ļ�·��
	THIni m_ini;						///<�������ö�ȡ��
	THString m_sIniFileEx;				///<����Win7/Vista��ini·��
	THIni m_iniex;						///<����Win7/Vista�����ö�ȡ��
};
