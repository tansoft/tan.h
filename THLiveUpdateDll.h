#pragma once

#include <THString.h>
#include <THDllLoader.h>

typedef BOOL (*LiveUpdateShowLiveUpdateProcess)(const char* sPath,const char* sTitle,const char* sUrl,UINT nReTryMin,UINT nFailedReTryMin,UINT nFailReTryCount);

/**
* @brief 自动更新界面dll处理封装类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-08-20 新建类
*/
/**<pre>
用法：
	THLiveUpdateDll m_dll;
*/
class THLiveUpdateDll :public THDllLoader
{
public:
	THLiveUpdateDll():THDllLoader(_T("LiveUpdate.dll")){EmptyDllPointer();}
	virtual ~THLiveUpdateDll(){FreeDll();}

	virtual void EmptyDllPointer()
	{
		m_ShowLiveUpdateProcess=NULL;
	}

	virtual BOOL InitDllPoinier()
	{
		m_ShowLiveUpdateProcess=(LiveUpdateShowLiveUpdateProcess)GetProcAddress(m_module,"ShowLiveUpdateProcess");
		if (!m_ShowLiveUpdateProcess) return FALSE;
		return TRUE;
	}

	/**
	* @brief 初始化自动更新
	* @param sPath				自动更新使用的目录
	* @param sTitle				自动更新模块名称，如：天塑通讯录
	* @param sUrl				自动更新配置文件url
	* @param nReTryMin			自动更新定时更新间隔，单位为分钟，0为不进行定时更新
	* @param nFailedReTryMin	自动更新失败定时更新间隔，单位为分钟，0为不进行定时更新
	* @param nFailReTryCount	更新过程失败后重试次数
	* @return 是否成功
	*/
	BOOL ShowLiveUpdateProcess(THString sPath,THString sTitle,THString sUrl,UINT nReTryMin=0,UINT nFailedReTryMin=0,UINT nFailReTryCount=2)
	{
		if (!InitDll()) return FALSE;
		if (!m_ShowLiveUpdateProcess) return FALSE;
		char *path=THCharset::t2a(sPath);
		char *title=THCharset::t2a(sTitle);
		char *url=THCharset::t2a(sUrl);
		if (!path || !title || !url) return FALSE;
		BOOL ret=(m_ShowLiveUpdateProcess)(path,title,url,nReTryMin,nFailedReTryMin,nFailReTryCount);
		THCharset::free(path);
		THCharset::free(title);
		THCharset::free(url);
		return ret;
	}
private:
	LiveUpdateShowLiveUpdateProcess m_ShowLiveUpdateProcess;
};