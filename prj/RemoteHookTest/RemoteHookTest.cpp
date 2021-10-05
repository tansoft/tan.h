// RemoteHookTest.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"

HANDLE g_ThreadHandle=NULL;

BOOL APIENTRY DllMain( HANDLE hModule,
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
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

extern "C" __declspec(dllexport) void THRemoteHookRun()
{
	//OutputDebugString("Calling Hook!!!!!!");
	Sleep(10000);
	//OutputDebugString("Hook Exit!!!!!!");
	QuitMyThread();
}
