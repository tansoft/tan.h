#pragma once

#include <THSystem.h>

/**
* @brief ��������װ��
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-03-05 �½���
* @2009-06-10 �����ַ����������
*/
/**<pre>
�÷���
	afx_msg LRESULT OnTrayIconMsg(WPARAM wParam,LPARAM lParam);
	ON_MESSAGE(WM_TRAY_MESSAGE,OnTrayIconMsg)

	THTrayIcon m_tray;
	m_tray.SetNotifyWnd(m_hWnd,WM_TRAY_MESSAGE);
	m_tray.SetToolTip(_T("Display Text"));
	m_tray.SetIcon(m_hIcon);
	m_tray.ShowTray();

	��ҪExplorer�ǲٺ�ͼ���ؽ�֧������ϵͳ������Ϣѭ��DefWindowProc�е��ã�m_tray.UpdateRebuildState(UINT msg);
	����OnTrayIconMsg�е���TrayIcon�ĺ���ʵ��Ĭ�ϴ���
	LRESULT OnParseDefaultMessage(WPARAM wParam,LPARAM lParam)
	{
		switch(lParam)
		{
			case WM_LBUTTONUP:
				break;
			case WM_LBUTTONDBLCLK:
				break;
			case WM_RBUTTONUP:
				break;
			case WM_RBUTTONDBLCLK:
				break;
			case WM_MOUSEMOVE:
				break;
			case NIN_BALLOONSHOW:
				break;
			case NIN_BALLOONUSERCLICK://�û�����������ʾ
				break;
			case NIN_BALLOONTIMEOUT://��ʱ���ߵ����رհ�ť
				break;
		}
	}
</pre>*/
class THTrayIcon
{
public: 
	THTrayIcon()
	{
		memset(&m_nid, 0, sizeof(NOTIFYICONDATA));
		m_nid.cbSize = sizeof(NOTIFYICONDATA);
		m_hMenu=NULL;
		m_bShow=FALSE;
		m_adddata=NULL;
		m_rebuildmsg=::RegisterWindowMessage(_T("TaskbarCreated"));
	}
	~THTrayIcon()
	{
		ShowTray(FALSE);
		if (m_hMenu)
			DestroyMenu(m_hMenu);
	}
	BOOL SetMenuId(UINT id)
	{
		m_hMenu=LoadMenu(NULL,MAKEINTRESOURCE(id));
		return (m_hMenu!=NULL);
	}
	BOOL SetMenuText(UINT nCmdId,THString text)
	{
		if (!::IsMenu(m_hMenu)) return FALSE;
		return ::ModifyMenu(m_hMenu,nCmdId,MF_BYCOMMAND,nCmdId,text);
	}
	BOOL SetMenuCheck(UINT nCmdId,BOOL bCheck)
	{
		if (!::IsMenu(m_hMenu)) return FALSE;
		::CheckMenuItem(m_hMenu,nCmdId,MF_BYCOMMAND|(bCheck?MF_CHECKED:MF_UNCHECKED));
		return TRUE;
	}
	BOOL SetMenuEnable(UINT nCmdId,BOOL bEnable)
	{
		if (!::IsMenu(m_hMenu)) return FALSE;
		::EnableMenuItem(m_hMenu,nCmdId,MF_BYCOMMAND|(bEnable?MF_ENABLED:MF_GRAYED));
		return TRUE;
	}
	
	// Set notify message window 
	BOOL SetNotifyWnd(HWND hWnd, UINT nMessage)
	{
		if(!::IsWindow(hWnd)) return FALSE; 
		m_nid.hWnd = hWnd; 
		if(nMessage > 0) 
		{ 
			m_nid.uCallbackMessage = nMessage; 
			if(!(m_nid.uFlags & NIF_MESSAGE)) 
				m_nid.uFlags |= NIF_MESSAGE; 
		}
		else 
		{ 
			m_nid.uCallbackMessage = 0; 
			if(m_nid.uFlags & NIF_MESSAGE) 
				m_nid.uFlags &= ~NIF_MESSAGE; 
		}
		if (m_bShow)
			return Shell_NotifyIcon(NIM_MODIFY, &m_nid); 
		return TRUE;
	}
	BOOL SetIcon(HICON hIcon)
	{
		if (hIcon==NULL) return FALSE;
		if(!(m_nid.uFlags & NIF_ICON))
			m_nid.uFlags |= NIF_ICON;
		m_nid.hIcon = hIcon;
		if (m_bShow)
			return Shell_NotifyIcon(NIM_MODIFY, &m_nid);
		return TRUE;
	}
	inline BOOL SetIcon(UINT nID){return SetIcon(::LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(nID)));}
	HICON GetIcon() const {return m_nid.hIcon;}

	BOOL SetToolTip(LPCTSTR lpszText)
	{
		if(!(m_nid.uFlags & NIF_TIP)) m_nid.uFlags |= NIF_TIP; 
		_tcsncpy(m_nid.szTip, lpszText,sizeof(m_nid.szTip)/sizeof(TCHAR));
		m_nid.szTip[sizeof(m_nid.szTip)/sizeof(TCHAR)-1]='\0';
		if (m_bShow)
			return Shell_NotifyIcon(NIM_MODIFY,&m_nid); 
		return TRUE;
	}
	BOOL SetToolTip(UINT nID)
	{
		TCHAR szText[128];
		::LoadString(GetModuleHandle(NULL),nID,szText,128);
		return SetToolTip(szText);
	}
	UINT GetToolTip(LPTSTR lpszText, int nLength) const
	{
		_tcscpy(lpszText, m_nid.szTip); 
		return lstrlen(lpszText); 
	}

#if (_WIN32_IE >= 0x0500) 
	//dwInfoFlags=NIIF_INFO:��ʾ��Ϣͼ�ꣻNIIF_ERROR:����NIIF_WARNING:��ʾ���棬NIIF_NONE:����ʾͼ��
	BOOL SetBalloonTip(LPCTSTR lpszTitle, LPCTSTR lpszMsg, UINT nTimeout=3000 ,DWORD dwInfoFlags=NIIF_INFO,void *pAddData=NULL)
	{
		m_adddata=pAddData;
		if(!(m_nid.uFlags & NIF_INFO)) m_nid.uFlags |= NIF_INFO;
		m_nid.uTimeout = nTimeout;
		_tcsncpy(m_nid.szInfoTitle, lpszTitle,sizeof(m_nid.szInfoTitle)/sizeof(TCHAR));
		m_nid.szInfoTitle[sizeof(m_nid.szInfoTitle)/sizeof(TCHAR)-1]='\0';
		_tcsncpy(m_nid.szInfo, lpszMsg,sizeof(m_nid.szInfo)/sizeof(TCHAR));
		m_nid.szInfo[sizeof(m_nid.szInfo)/sizeof(TCHAR)-1]='\0';
		m_nid.dwInfoFlags=dwInfoFlags;
		if (m_bShow)
			return Shell_NotifyIcon(NIM_MODIFY, &m_nid);
		return TRUE;
	}
	BOOL UnSetBalloonTip()
	{
		m_adddata=NULL;
		if(!(m_nid.uFlags & NIF_INFO)) m_nid.uFlags |= NIF_INFO;
		m_nid.uTimeout = 0;
		m_nid.szInfoTitle[0]='\0';
		m_nid.szInfo[0]='\0';
		m_nid.dwInfoFlags=0;
		if (m_bShow)
			return Shell_NotifyIcon(NIM_MODIFY, &m_nid);
		return TRUE;
	}
	BOOL GetBalloonTip(LPTSTR lpszTitle, LPTSTR lpszMsg, UINT* pTimeout) const
	{
		_tcscpy(lpszTitle, m_nid.szInfoTitle); 
		_tcscpy(lpszMsg, m_nid.szInfo); 
		*pTimeout = m_nid.uTimeout; 
		return TRUE; 
	}
	void *GetBalloonAddData(){return m_adddata;}
#endif 
	BOOL ShowTray(BOOL bShow=TRUE)
	{
		if (m_bShow!=bShow)
		{
			m_bShow=bShow;
			return Shell_NotifyIcon(bShow?NIM_ADD:NIM_DELETE,&m_nid);
		}
		return TRUE;
	}

	//Tray rebuild support
	void UpdateRebuildState(UINT msg)
	{
		if (m_rebuildmsg==msg && m_bShow) Shell_NotifyIcon(NIM_ADD,&m_nid);
	}

	LRESULT OnParseDefaultMessage(WPARAM wParam,LPARAM lParam)
	{
		switch(lParam)
		{
			case WM_LBUTTONUP:
				break;
			case WM_LBUTTONDBLCLK:
			{
				ShowWindow(m_nid.hWnd,SW_RESTORE);
				SetWindowPos(m_nid.hWnd,HWND_TOP,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE|SWP_SHOWWINDOW);
				SetForegroundWindow(m_nid.hWnd);
			}
				break;
			case WM_RBUTTONUP:
			{
				if (m_hMenu)
				{
					POINT p;
					GetCursorPos(&p);
					//����ȡ���˵��¼�����������״̬Ϊ����Ҳ��
					::SetForegroundWindow(m_nid.hWnd);
					TrackPopupMenu(GetSubMenu(m_hMenu,0),TPM_LEFTALIGN|TPM_RIGHTBUTTON,p.x,p.y,0,m_nid.hWnd,NULL);
				}
			}
				break;
			case WM_RBUTTONDBLCLK:
				break;
			case WM_MOUSEMOVE:
				break;
#if (_WIN32_IE >= 0x0501)
			//case NIN_BALLOONHIDE:
			//	break;
			case NIN_BALLOONSHOW://���򵯳�
				break;
			case NIN_BALLOONUSERCLICK://�û�����������ʾ
				break;
			case NIN_BALLOONTIMEOUT://��ʱ���ߵ����رհ�ť
				break;
#endif
		}
		return 0;
	}
private:
	NOTIFYICONDATA m_nid; 
	HMENU m_hMenu;
	BOOL m_bShow;
	void *m_adddata;
	UINT m_rebuildmsg;
};
