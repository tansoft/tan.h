#pragma once

#include <THString.h>
#include <MsgHookDll\commondefine.h>
#include <THDllLoader.h>

typedef DWORD (*MsgHookGetLastIdle)();
typedef BOOL (*MsgHookSetIdleHooks)();
typedef void (*MsgHookRemoveIdleHooks)();
typedef BOOL (*MsgHookSetWindowHook)(HWND hRetWnd,int CheckType);
typedef void (*MsgHookRemoveWindowHook)();
typedef BOOL (*MsgHookSetCallWndProcHook)(HWND hWnd,HWND hRetWnd,int CheckType);
typedef void (*MsgHookRemoveCallWndProcHook)();

/**
* @brief 外部事件钩子处理封装类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-06-19 新建类
*/
/**<pre>
用法：
	//CHECKTYPE_XXX 详见commondefine.h
	THMsgHookDll m_dll;
	m_dll.SetWindowHook(HWND retHwnd,int CheckType);
	m_dll.SetCallWndProcHook(HWND hWnd,HWND hRetWnd,int CheckType);
	m_dll.RemoveWindowHook();
	m_dll.RemoveCallWndProcHook();
*/
class THMsgHookDll :public THDllLoader
{
public:
	THMsgHookDll():THDllLoader(_T("MsgHook.dll")){EmptyDllPointer();}
	virtual ~THMsgHookDll(){FreeDll();}

	virtual void EmptyDllPointer()
	{
		m_GetLastIdle=NULL;
		m_SetIdleHooks=NULL;
		m_RemoveIdleHooks=NULL;
		m_SetWindowHook=NULL;
		m_RemoveWindowHook=NULL;
		m_SetCallWndProcHook=NULL;
		m_RemoveCallWndProcHook=NULL;
	}

	virtual BOOL InitDllPoinier()
	{
		m_GetLastIdle=(MsgHookGetLastIdle)GetProcAddress(m_module,"GetLastIdle");
		m_SetIdleHooks=(MsgHookSetIdleHooks)GetProcAddress(m_module,"SetIdleHooks");
		m_RemoveIdleHooks=(MsgHookRemoveIdleHooks)GetProcAddress(m_module,"RemoveIdleHooks");
		m_SetWindowHook=(MsgHookSetWindowHook)GetProcAddress(m_module,"SetWindowHook");
		m_RemoveWindowHook=(MsgHookRemoveWindowHook)GetProcAddress(m_module,"RemoveWindowHook");
		m_SetCallWndProcHook=(MsgHookSetCallWndProcHook)GetProcAddress(m_module,"SetCallWndProcHook");
		m_RemoveCallWndProcHook=(MsgHookRemoveCallWndProcHook)GetProcAddress(m_module,"RemoveCallWndProcHook");
		if (!m_GetLastIdle || !m_SetIdleHooks || !m_RemoveIdleHooks || !m_SetWindowHook || !m_RemoveWindowHook || !m_SetCallWndProcHook || !m_RemoveCallWndProcHook)
			return FALSE;
		return TRUE;
	}

	BOOL SetIdleHooks()
	{
		if (!InitDll()) return FALSE;
		if (!m_SetIdleHooks) return FALSE;
		return (m_SetIdleHooks)();
	}

	BOOL RemoveIdleHooks()
	{
		if (!InitDll()) return FALSE;
		if (!m_RemoveIdleHooks) return FALSE;
		(m_RemoveIdleHooks)();
		return TRUE;
	}

	DWORD GetLastIdle()
	{
		if (!InitDll()) return FALSE;
		if (!m_GetLastIdle) return FALSE;
		return (m_GetLastIdle)();
	}

	//对自己的窗口事件处理
	BOOL SetWindowHook(HWND retHwnd,int CheckType)
	{
		if (!InitDll()) return FALSE;
		if (!m_SetWindowHook) return FALSE;
		return (m_SetWindowHook)(retHwnd,CheckType);
	}

	BOOL RemoveWindowHook()
	{
		if (!InitDll()) return FALSE;
		if (!m_RemoveWindowHook) return FALSE;
		(m_RemoveWindowHook)();
		return TRUE;
	}

	//对指定句柄的窗口事件处理
	BOOL SetCallWndProcHook(HWND hWnd,HWND hRetWnd,int CheckType)
	{
		if (!InitDll()) return FALSE;
		if (!m_SetCallWndProcHook) return FALSE;
		return (m_SetCallWndProcHook)(hWnd,hRetWnd,CheckType);
	}

	BOOL RemoveCallWndProcHook()
	{
		if (!InitDll()) return FALSE;
		if (!m_RemoveCallWndProcHook) return FALSE;
		(m_RemoveCallWndProcHook)();
		return TRUE;
	}
private:
	MsgHookGetLastIdle           m_GetLastIdle;
	MsgHookSetIdleHooks          m_SetIdleHooks;
	MsgHookRemoveIdleHooks       m_RemoveIdleHooks;
	MsgHookSetWindowHook         m_SetWindowHook;
	MsgHookRemoveWindowHook      m_RemoveWindowHook;
	MsgHookSetCallWndProcHook    m_SetCallWndProcHook;
	MsgHookRemoveCallWndProcHook m_RemoveCallWndProcHook;
};