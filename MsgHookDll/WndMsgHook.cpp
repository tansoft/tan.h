#include <windows.h>
#include "commondefine.h"

extern "C"{
extern HINSTANCE g_hInstance;
#pragma data_seg(".WndMsgShare")
	static HHOOK cwpHook=NULL;
	static HWND retHwnd=NULL;
	static int checktype=0;
#pragma data_seg()
#pragma comment(linker,"/section:.WndMsgShare,RWS")

#include <stdio.h>

LRESULT CALLBACK CallWndProc(int nCode,WPARAM wParam,LPARAM lParam)
{
	if(nCode==HC_ACTION && retHwnd)
	{
		PCWPSTRUCT pMsg = (PCWPSTRUCT)lParam;
		if (checktype==CHECKTYPE_NEWWINDOW)
		{
			if (pMsg->message==WM_CREATE)
			{
				LPCREATESTRUCT c=(LPCREATESTRUCT)pMsg->lParam;
				if (c && c->hwndParent==NULL)
				{
					//dialog's parent is null
					//char buf[1024];
					//sprintf(buf,"Receive Check Window Msg %s,%s",c->lpszName?c->lpszName:"",c->lpszClass?c->lpszClass:"");
					//OutputDebugString(buf);
					SendMessage(retHwnd,WM_MSGHOOKMSG,CHECKTYPE_NEWWINDOW,(LPARAM)c->hwndParent);
				}
			}
/*			else if (pMsg->message==WM_WINDOWPOSCHANGING)
			{
				LPWINDOWPOS pos=(LPWINDOWPOS)pMsg->lParam;
				if (pos->flags & SWP_SHOWWINDOW)
					SendMessage(retHwnd,WM_MSGHOOKMSG,CHECKTYPE_NEWWINDOW,(LPARAM)pos->hwnd);
			}*/
		}
	}
	return CallNextHookEx(cwpHook,nCode,wParam,lParam);
}

__declspec(dllexport) BOOL SetCallWndProcHook(HWND hWnd,HWND hRetWnd,int CheckType)
{
	DWORD ProcessId;
	DWORD ThreadId=GetWindowThreadProcessId(hWnd,&ProcessId);
	cwpHook=SetWindowsHookEx(WH_CALLWNDPROC,CallWndProc,g_hInstance,ThreadId);
	retHwnd=hRetWnd;
	checktype=CheckType;
	return (cwpHook!=NULL);
}


__declspec(dllexport) void RemoveCallWndProcHook()
{
	retHwnd=NULL;
	checktype=0;
	if (cwpHook)
	{
		UnhookWindowsHookEx(cwpHook);
		cwpHook=NULL;
	}
}

}
