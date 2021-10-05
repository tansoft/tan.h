#pragma once

#include <THString.h>
#include <THCharset.h>
#include <THDllLoader.h>

typedef BOOL (*RemoteHookInjectToProcess)(DWORD pid,char *dllname);

/**
* @brief Զ�̲��빳��Dll��װ��
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-08-18 �½���
*/
/**<pre>
�÷���
	THRemoteHookDll m_hook;
	m_hook.InjectToProcessByName(_T("QQ.exe"),_T("F:\\�ҵĹ���050926\\svn\\tan.h\\prj\\RemoteHookTest\\RemoteHookTest.dll"));

	//ʵ��RemoteHookTest.dll��
	HANDLE g_ThreadHandle=NULL;
	BOOL APIENTRY DllMain(HANDLE hModule,DWORD ul_reason_for_call,LPVOID lpReserved)
	{
		if (ul_reason_for_call==DLL_PROCESS_ATTACH)
			g_ThreadHandle=hModule;
		return TRUE;
	}

	static void QuitMyThread()
	{
		if (g_ThreadHandle)
		{
			__asm
			{
				push 0
				push g_ThreadHandle
				push ExitThread
				push FreeLibrary
				ret
			}
		}
	}

	//ʵ��"THRemoteHookRun"�ӿڣ��ɹ�����󽫻���øú���
	extern "C" __declspec(dllexport) void THRemoteHookRun()
	{
		Sleep(10000);
		//....��������
		//�������QuitMyThread�����ͷţ����������ǲ�
		QuitMyThread();
	}

</pre>*/
class THRemoteHookDll :public THDllLoader
{
public:
	THRemoteHookDll():THDllLoader(_T("RemoteHook.dll")){EmptyDllPointer();}
	virtual ~THRemoteHookDll(){FreeDll();}

	virtual void EmptyDllPointer()
	{
		m_InjectToProcess=NULL;
	}

	virtual BOOL InitDllPoinier()
	{
		m_InjectToProcess=(RemoteHookInjectToProcess)GetProcAddress(m_module,"InjectToProcess");
		if (!m_InjectToProcess)
			return FALSE;
		return TRUE;
	}

	BOOL InjectToProcess(DWORD pid,THString sCbDll)
	{
		if (pid==0) return FALSE;
		if (sCbDll.IsEmpty()) return FALSE;
		if (!InitDll()) return FALSE;
		if (!m_InjectToProcess) return FALSE;
		char *tmp=THCharset::t2a(sCbDll);
		if (!tmp) return FALSE;
		BOOL ret=(m_InjectToProcess)(pid,tmp);
		THCharset::free(tmp);
		return ret;
	}

	BOOL InjectToProcessByName(THString sProcessName,THString sCbDll)
	{
		DWORD pid=THToolHelp::GetProcessIdByName(sProcessName);
		return InjectToProcess(pid,sCbDll);
	}
private:
	RemoteHookInjectToProcess    m_InjectToProcess;
};