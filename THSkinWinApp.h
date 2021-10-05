#pragma once

#include <THWinApp.h>
#include <THCharset.h>
#include <SkinPlusPlus\SkinPPWTL.h>
#pragma comment(lib,"SkinPPWTL.lib")

/**
* @brief WinApp 基类，带Skin++ Lib，程序路径及配置文件等
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-05-28 新建类
*/
/**<pre>
注意点：
	Exe路径 m_sExePath 为最后不带 \\ 的，用时需要加上
用法：
	直接把WinApp继承自THSkinWinApp
	实现virtual THString GetSkinFile(THString sExePath);函数
	把WinApp的InitInstance函数调用THSkinWinApp的InitInstance
	如果有ExitInstance函数，调用THSkinWinApp的ExitInstance
	//去除子控件皮肤
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
