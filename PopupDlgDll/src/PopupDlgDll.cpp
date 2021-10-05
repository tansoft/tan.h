// PluginADShow.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "ADWin.h"
#include <time.h>

extern "C"
{
__declspec(dllexport) BOOL ShowPopup(char *settings);
__declspec(dllexport) BOOL CloseAllPopup();
__declspec(dllexport) BOOL FindPopupWindow(LPSTR szCaption,HWND *hWndTarget);
}

CComModule _Module;
HMODULE g_hModule = NULL;
extern VOID funShowAD(LPARAM lParam);
extern CSimpleValArray<HWND> m_ADHwndArr;

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	if(ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		g_hModule = (HMODULE)hModule;
		_Module.Init(NULL, (HINSTANCE)g_hModule, &LIBID_ATLLib); // LIBID_ATLLib must be added to support IE Control
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		CloseAllPopup();
		_Module.Term();
	}
    return TRUE;
}

BOOL ShowPopup(char *settings)
{
	DWORD dwThreadID;
	CreateThread(NULL,NULL,(LPTHREAD_START_ROUTINE)funShowAD,(LPVOID)strdup(settings),NULL,&dwThreadID);
	return TRUE;
}

BOOL CloseAllPopup()
{
	UINT uCount = m_ADHwndArr.GetSize();
	for(;uCount > 0;)
	{
		SendMessage(m_ADHwndArr[--uCount],WM_CLOSE,0,0);
		Sleep(100);
	}
	return TRUE;
}

BOOL FindPopupWindow(LPSTR szCaption,HWND *hWndTarget)
{
	UINT uCount = m_ADHwndArr.GetSize();
	if(!hWndTarget)
		return FALSE;
	char szTitle[MAX_PATH] = {'\0'};
	for(;uCount > 0;)
	{
		HWND hWnd = m_ADHwndArr[--uCount];
		if(GetWindowText(hWnd,szTitle,MAX_PATH)<0)
			continue;
		if(!strcmp(szTitle,szCaption))
		{
			*hWndTarget = hWnd;
			return TRUE;
		}
	}
	return FALSE;
}
