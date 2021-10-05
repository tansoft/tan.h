#include <windows.h>
#include <tchar.h>
//#include <afxwin.h>         // MFC 核心和标准组件
#include <THMapFile.h>

extern "C"{

static DWORD *lastTime = NULL;
static HHOOK keyHook = NULL;
static HHOOK mouseHook = NULL;
HINSTANCE g_hInstance = NULL;
static POINT g_point;
static THMapFile m_map;

void RemoveCallWndProcHook();
void RemoveWindowHook();

static DWORD* setup_shared_mem()
{
	lastTime=(DWORD *)m_map.MapBuffer();
	
	if (lastTime==NULL) return NULL;
	
	*lastTime=GetTickCount();
	
	return lastTime;
}

LRESULT CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam)
{
	if (code < 0)
		return CallNextHookEx(keyHook, code, wParam, lParam);
	if (lastTime == NULL)
		lastTime = setup_shared_mem();
	
	if (lastTime)
		*lastTime = GetTickCount();

	return CallNextHookEx(keyHook, code, wParam, lParam);
}

LRESULT CALLBACK MouseProc(int code, WPARAM wParam, LPARAM lParam)
{
	if (code < 0)
		return CallNextHookEx(mouseHook, code, wParam, lParam);

	/* We need to verify that the Mouse pointer has actually moved. */
	if((g_point.x == ((MOUSEHOOKSTRUCT*)lParam)->pt.x) &&
	   (g_point.y == ((MOUSEHOOKSTRUCT*)lParam)->pt.y))
		return 0;

	g_point.x = ((MOUSEHOOKSTRUCT*)lParam)->pt.x;
	g_point.y = ((MOUSEHOOKSTRUCT*)lParam)->pt.y;
	
	if (lastTime == NULL)
		lastTime = setup_shared_mem();
	
	if (lastTime)
		*lastTime = GetTickCount();
	
	return CallNextHookEx(mouseHook, code, wParam, lParam);
}

__declspec(dllexport) DWORD GetLastIdle()
{
	if (lastTime == NULL)
		lastTime = setup_shared_mem();
	
	if (lastTime)
		return *lastTime;

	return 0;
}

__declspec(dllexport) BOOL SetIdleHooks()
{
	// Set up the shared memory.
	lastTime = setup_shared_mem();
	if (lastTime==NULL) return FALSE;
	*lastTime = GetTickCount();
	
	// Set up the keyboard hook.
	keyHook = SetWindowsHookExA(WH_KEYBOARD, KeyboardProc, g_hInstance, 0);
	if (keyHook == NULL) {
		m_map.Close();
		lastTime=NULL;
		return FALSE;
	}
	
	// Set up the mouse hook.
	mouseHook = SetWindowsHookExA(WH_MOUSE, MouseProc, g_hInstance, 0);
	if (mouseHook == NULL) {
		UnhookWindowsHookEx(keyHook);
		m_map.Close();
		lastTime=NULL;
		return FALSE;
	}
	
	return TRUE;
}

__declspec(dllexport) void RemoveIdleHooks()
{
	if (keyHook)
	{
		UnhookWindowsHookEx(keyHook);
		keyHook=NULL;
	}
	if (mouseHook)
	{
		UnhookWindowsHookEx(mouseHook);
		mouseHook=NULL;
	}
	m_map.Close();
	lastTime=NULL;
}

int WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	switch(dwReason) {
		case DLL_PROCESS_ATTACH:
			g_hInstance = hInstance;
			g_point.x = 0;
			g_point.y = 0;
			m_map.Init(_T("MouseKbIdleCounter"),sizeof(DWORD));
			break;
		case DLL_PROCESS_DETACH:
			RemoveIdleHooks();
			RemoveCallWndProcHook();
			RemoveWindowHook();
			break;
	}
	return TRUE;
}

}
