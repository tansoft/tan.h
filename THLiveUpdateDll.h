#pragma once

#include <THString.h>
#include <THDllLoader.h>

typedef BOOL (*LiveUpdateShowLiveUpdateProcess)(const char* sPath,const char* sTitle,const char* sUrl,UINT nReTryMin,UINT nFailedReTryMin,UINT nFailReTryCount);

/**
* @brief �Զ����½���dll�����װ��
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-08-20 �½���
*/
/**<pre>
�÷���
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
	* @brief ��ʼ���Զ�����
	* @param sPath				�Զ�����ʹ�õ�Ŀ¼
	* @param sTitle				�Զ�����ģ�����ƣ��磺����ͨѶ¼
	* @param sUrl				�Զ����������ļ�url
	* @param nReTryMin			�Զ����¶�ʱ���¼������λΪ���ӣ�0Ϊ�����ж�ʱ����
	* @param nFailedReTryMin	�Զ�����ʧ�ܶ�ʱ���¼������λΪ���ӣ�0Ϊ�����ж�ʱ����
	* @param nFailReTryCount	���¹���ʧ�ܺ����Դ���
	* @return �Ƿ�ɹ�
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