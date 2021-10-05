#include <windows.h>
#include "commondefine.h"

extern "C"
{
typedef LRESULT (*WndProc)(int nCode,WPARAM wParam,LPARAM lParam,void *adddata);
extern HINSTANCE g_hInstance;
#pragma data_seg(".WinMsgShare")
	static HHOOK cbtHook=NULL;
	static HWND retHwnd=NULL;
	static int checktype=0;
#pragma data_seg()
#pragma comment(linker,"/section:.WinMsgShare,RWS")

LRESULT CALLBACK CBT_Proc(int nCode,WPARAM wParam,LPARAM lParam)
{
	if(nCode>0)
	{
		//(nCode==HCBT_CREATEWND || HCBT_DESTROYWND)
	}
	return CallNextHookEx(cbtHook,nCode,wParam,lParam);
}

__declspec(dllexport) BOOL SetWindowHook(HWND hRetWnd,int CheckType)
{
	// Set up the keyboard hook.
	cbtHook = SetWindowsHookExA(WH_CBT,(HOOKPROC)CBT_Proc,g_hInstance,0);
	retHwnd=hRetWnd;
	checktype=CheckType;
	return (cbtHook!=NULL);
}

__declspec(dllexport) void RemoveWindowHook()
{
	retHwnd=NULL;
	checktype=0;
	if (cbtHook)
	{
		UnhookWindowsHookEx(cbtHook);
		cbtHook=NULL;
	}
}

}