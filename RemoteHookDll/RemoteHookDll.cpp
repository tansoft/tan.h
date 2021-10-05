// THRemoteHookImp.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include <process.h>
#include <stdio.h>
#include <tlhelp32.h>

#pragma data_seg(".InfoShare")
	char g_InjectInfo[4096]={0};
	HHOOK g_hHook=NULL;
#pragma data_seg()
#pragma comment(linker,"/section:.InfoShare,RWS")

static unsigned __stdcall StartThread(void *lparam);

HANDLE g_ThreadHandle=NULL;
char g_dllpath[MAX_PATH]={0};
HMODULE g_hmod=NULL;

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	if (ul_reason_for_call==DLL_PROCESS_ATTACH)
	{
		DWORD proid=GetCurrentProcessId();
		g_ThreadHandle=hModule;
		char tmp[4096];
		strcpy(tmp,g_InjectInfo);
		char sproid[20];
		sprintf(sproid,"[%d:",proid);
		//OutputDebugString("Init Call");
		//OutputDebugString(g_InjectInfo);
		//OutputDebugString(sproid);
		char *pos=strstr(tmp,sproid);
		if (pos)
		{
			//it is the inject thread
			//OutputDebugString("Inject Thread");
			int len=(int)((pos-tmp)+strlen(sproid));
			int len2=min(4096-len,MAX_PATH-1);
			strncpy(g_dllpath,tmp+len,len2);
			g_dllpath[len2]='\0';
			char *endtmp=strstr(g_dllpath,"]");
			if (endtmp)
				*endtmp='\0';
			endtmp=strstr(pos,"]");
			if (endtmp)
				strcpy(pos,endtmp+1);
			else
				*pos='\0';
			strcpy(g_InjectInfo,tmp);
			//OutputDebugString("Inject Info:");
			//OutputDebugString(g_InjectInfo);
			//OutputDebugString(g_dllpath);
			g_hmod=LoadLibrary(g_dllpath);
			if (g_hmod)
			{
				unsigned int threadid;
				_beginthreadex(0,0,StartThread,NULL,0,&threadid);
			}
		}
	}
    return TRUE;
}

//回调其实用不着
LRESULT CALLBACK GetMsgProc(int nCode,WPARAM wParam,LPARAM lParam)
{
	if (g_hHook)
		return CallNextHookEx(g_hHook,nCode,wParam,lParam);
	return 0;
}

DWORD GetMainThreadInProcess(DWORD ProcessId)
{
	HANDLE hProcessSnap=NULL;
	THREADENTRY32 pe32;
	pe32.dwSize=sizeof(THREADENTRY32);
	hProcessSnap=CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);
	if(hProcessSnap==(HANDLE)-1)
	{
		return 0;
	}
	if(Thread32First(hProcessSnap,&pe32))
	{
		do{
			if (pe32.th32OwnerProcessID==ProcessId)
			{
				CloseHandle(hProcessSnap);
				return pe32.th32ThreadID;
			}
		}while(Thread32Next(hProcessSnap,&pe32));
	}
	CloseHandle(hProcessSnap);
	return 0;
}

BOOL InsertThreadByHookMsg(DWORD ProcessId,HANDLE Md,char *DllName)
{
	if (!g_hHook)
	{
		DWORD ThreadId=GetMainThreadInProcess(ProcessId);
		g_hHook=SetWindowsHookEx(WH_GETMESSAGE,GetMsgProc,(HINSTANCE)Md,ThreadId);//利用钩子函数创建远程线程
		if(!g_hHook)
		{
			//OutputDebugString("SetWindowsHook Error");
			return FALSE;
		}
		PostThreadMessage(ThreadId,WM_ENTERIDLE,0,0);
		Sleep(1000);
		UnhookWindowsHookEx(g_hHook);
		g_hHook=NULL;
	}
	else
	{
		while(g_hHook!=NULL)
		{
			//OutputDebugString("Waiting Hook");
			Sleep(1000);
		}
		return InsertThreadByHookMsg(ProcessId,Md,DllName);
	}
	return TRUE;
}

extern "C" __declspec(dllexport) BOOL InjectToProcess(DWORD pid,char *dllname)
{
	char buf[8092];
	sprintf(buf,"%s[%d:%s]",g_InjectInfo,pid,dllname);
	if (strlen(buf)>4096) return FALSE;
	strcpy(g_InjectInfo,buf);
	sprintf(buf,"[%d:%s]",pid,dllname);
	//OutputDebugString("Inject Thread");
	//OutputDebugString(g_InjectInfo);
	char filename[MAX_PATH]={0};
	GetModuleFileNameA((HMODULE)g_ThreadHandle,filename,sizeof(filename));
	while(1)
	{
		if (!InsertThreadByHookMsg(pid,g_ThreadHandle,filename)) return FALSE;
		Sleep(1000);
		//如果还找不到，尝试再次插入
		if (strstr(g_InjectInfo,buf)==NULL) return TRUE;
	}
}

static void QuitMyThread()
{
	if (/*g_LibUnWant && */g_ThreadHandle)
	{
		__asm
		{
/*			push 0
			push g_LibUnWant
			push ExitThread
			push FreeLibrary*/
			push 0
			push g_ThreadHandle
			push ExitThread
			push FreeLibrary
			ret
		}
	}
}

typedef void (*RemoteHookRun)();

static unsigned __stdcall StartThread(void *lparam)
{
	//OutputDebugString("StartThread");
	if (g_hmod)
	{
		//OutputDebugString("Loaded Library");
		RemoteHookRun hook=(RemoteHookRun)GetProcAddress(g_hmod,"THRemoteHookRun");
		if (hook)
		{
			//OutputDebugString("Hook");
			hook();
		}
		//OutputDebugString("Free Call");
		//FreeLibrary(g_hmod);
	}
	//OutputDebugString("QuitMyThread");
	//QuitMyThread();
	return 0;
}