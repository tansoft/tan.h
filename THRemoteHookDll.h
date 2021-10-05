#pragma once

#include <THString.h>
#include <THCharset.h>
#include <THDllLoader.h>

typedef BOOL (*RemoteHookInjectToProcess)(DWORD pid,char *dllname);

/**
* @brief 远程插入钩子Dll封装类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-08-18 新建类
*/
/**<pre>
用法：
	THRemoteHookDll m_hook;
	m_hook.InjectToProcessByName(_T("QQ.exe"),_T("F:\\我的工程050926\\svn\\tan.h\\prj\\RemoteHookTest\\RemoteHookTest.dll"));

	//实现RemoteHookTest.dll：
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

	//实现"THRemoteHookRun"接口，成功插入后将会调用该函数
	extern "C" __declspec(dllexport) void THRemoteHookRun()
	{
		Sleep(10000);
		//....函数调用
		//必须调用QuitMyThread进行释放，否则会引起非操
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