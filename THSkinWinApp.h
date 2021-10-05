#pragma once

#include <THWinApp.h>
#include <THCharset.h>
#include <SkinPlusPlus\SkinPPWTL.h>
#pragma comment(lib,"SkinPPWTL.lib")

/**
* @brief WinApp ���࣬��Skin++ Lib������·���������ļ���
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-05-28 �½���
*/
/**<pre>
ע��㣺
	Exe·�� m_sExePath Ϊ��󲻴� \\ �ģ���ʱ��Ҫ����
�÷���
	ֱ�Ӱ�WinApp�̳���THSkinWinApp
	ʵ��virtual THString GetSkinFile(THString sExePath);����
	��WinApp��InitInstance��������THSkinWinApp��InitInstance
	�����ExitInstance����������THSkinWinApp��ExitInstance
	//ȥ���ӿؼ�Ƥ��
	skinppSetNoSkinHwnd(HWND hWnd,BOOL bChildNoSkin = TRUE);
</pre>*/
class THSkinWinApp : public THWinApp
{
protected:
	THSkinWinApp():THWinApp(){}
	virtual ~THSkinWinApp(){}

public:
	virtual THString GetSkinFile(THString sExePath)=0;
	virtual BOOL InitInstance()
	{
		THString sPath=THSysMisc::GetFilePath(THSysMisc::GetModuleFileName());
		char *chTmp=THCharset::t2a(GetSkinFile(sPath));
		if (chTmp)
		{
			skinppLoadSkin(chTmp);
			THCharset::free(chTmp);
		}
		return THWinApp::InitInstance();
	}
	virtual int ExitInstance()
	{
		skinppExitSkin();
		return THWinApp::ExitInstance();
	}
};
