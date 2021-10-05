#pragma once

#include <THString.h>
#include <THDebug.h>
#include <THStruct.h>
#include <THSysMisc.h>

#define THCTRLTYPE_BUTTON		_T("Button")
#define THCTRLTYPE_COMBOBOX		_T("ComboBox")
#define THCTRLTYPE_LISTBOX		_T("ListBox")
#define THCTRLTYPE_LISTCTRL		_T("SysListView32")
#define THCTRLTYPE_TABCTRL		_T("SysTabControl32")
#define THCTRLTYPE_STATIC		_T("Static")
#define THCTRLTYPE_EDIT			_T("Edit")

#define THCTRLTYPE_DELPHI_PANEL		_T("TPanel")
#define THCTRLTYPE_DELPHI_BUTTON	_T("TButton")
#define THCTRLTYPE_DELPHI_EDIT		_T("TEdit")
#define THCTRLTYPE_DELPHI_STBAR		_T("TStatusBar")

#define THGETWND(hwnd)				(CWnd::FromHandle(hwnd))
//#define THGETPOINTBYCLS(cls,hwnd)	(dynamic_cast<cls * >(CWnd::FromHandle(hwnd)))
#define THGETPOINTBYCLS(cls,hwnd)	((cls *)CWnd::FromHandle(hwnd))
#define THGETTABCTRL(hwnd)			THGETPOINTBYCLS(CTabCtrl,hwnd)
#define THGETBUTTON(hwnd)			THGETPOINTBYCLS(CButton,hwnd)
#define THGETEDIT(hwnd)				THGETPOINTBYCLS(CEdit,hwnd)
#define THGETSTATIC(hwnd)			THGETPOINTBYCLS(CStatic,hwnd)

class THDlgItemButton;
class THDlgItemEdit;
class THDlgItemTabCtrl;
class THDlgItemListCtrl;

/**
* @brief 对话框元素封装类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-06-24 新建类
*/
/**<pre>
</pre>*/
class THDlgItem
{
public:
	THDlgItem()
	{
		hWnd=hParentWnd=NULL;
		nCtrlId=0;
		nLevel=0;
		nIdx=0;
		rClientRect.left=rClientRect.top=rClientRect.right=rClientRect.bottom=0;
		m_remotebuf1=NULL;
		m_remotebuf2=NULL;
	}

	virtual ~THDlgItem() {
		if (m_remotebuf1) {
			m_man.FreeBuffer(m_remotebuf1);
			m_remotebuf1=NULL;
		}
		if (m_remotebuf2) {
			m_man.FreeBuffer(m_remotebuf2);
			m_remotebuf2=NULL;
		}
	}

	HWND hWnd;
	int nCtrlId;
	THString sClass;
	THString sWindow;
	int nLevel;
	int nIdx;
	HWND hParentWnd;
	RECT rClientRect;
	THRemoteAccessManager m_man;
	void *m_remotebuf1;
	void *m_remotebuf2;

	THDlgItemTabCtrl* ToTabCtrl()
	{
		ASSERT(sClass==THCTRLTYPE_TABCTRL);
		return (THDlgItemTabCtrl *)this;
	}
	THDlgItemEdit* ToEdit()
	{
		ASSERT(sClass==THCTRLTYPE_EDIT);
		return (THDlgItemEdit *)this;
	}
	THDlgItemButton* ToButton()
	{
		ASSERT(sClass==THCTRLTYPE_BUTTON);
		return (THDlgItemButton *)this;
	}
	THDlgItemListCtrl* ToListCtrl()
	{
		ASSERT(sClass==THCTRLTYPE_LISTCTRL);
		return (THDlgItemListCtrl *)this;
	}
	inline BOOL SetWindowText(THString sText)
	{
		if (::SetWindowText(hWnd,sText))
		{
			sWindow=sText;
			return TRUE;
		}
		return FALSE;
	}
	THString GetWindowText()
	{
		THString str;
		int len=::GetWindowTextLength(hWnd);
		::GetWindowText(hWnd,str.GetBuffer(len+1),len+1);
		str.ReleaseBuffer();
		if (!str.IsEmpty()) sWindow=str;
		return str;
	}
	THString GetDlgItemText()
	{
		THString str;
		::GetDlgItemText(hParentWnd,nCtrlId,str.GetBuffer(1024),1024);
		str.ReleaseBuffer();
		if (!str.IsEmpty()) sWindow=str;
		return str;
	}
	inline HWND SetFocus(){return ::SetFocus(hWnd);}
	inline LRESULT SendCtrlMessage(UINT nNotify){return ::SendMessage(hParentWnd,WM_COMMAND,MAKEWPARAM(nCtrlId,nNotify),(LPARAM)NULL);}
	inline LRESULT PostCtrlMessage(UINT nNotify){return ::PostMessage(hParentWnd,WM_COMMAND,MAKEWPARAM(nCtrlId,nNotify),(LPARAM)NULL);}
	inline LRESULT SendMessage(UINT nMsg,WPARAM wParam,LPARAM lParam){return ::SendMessage(hWnd,nMsg,wParam,lParam);}
	inline LRESULT PostMessage(UINT nMsg,WPARAM wParam,LPARAM lParam){return ::PostMessage(hWnd,nMsg,wParam,lParam);}
	inline LRESULT SendLButtonClickMessage(int x=1,int y=1)
	{
		SendMessage(WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(x,y));
		return SendMessage(WM_LBUTTONUP,0,MAKELPARAM(x,y));
	}
	inline LRESULT PostLButtonClickMessage(int x=1,int y=1)
	{
		PostMessage(WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(x,y));
		return PostMessage(WM_LBUTTONUP,0,MAKELPARAM(x,y));
	}
	inline LRESULT SendLButtonDoubleClickMessage(int x=1,int y=1)
	{
		SendMessage(WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(x,y));
		SendMessage(WM_LBUTTONDBLCLK,MK_LBUTTON,MAKELPARAM(x,y));
		return SendMessage(WM_LBUTTONUP,0,MAKELPARAM(x,y));
	}
	inline void SendKeyBoardEvent(unsigned char ch)
	{
		LPARAM l=MAKELPARAM(1,MapVirtualKey(ch,0));
		SendMessage(WM_KEYDOWN,ch,l);
		SendMessage(WM_CHAR,ch,l);
		SendMessage(WM_KEYUP,ch,l);
	}
	inline void LowSendKeyBoardEvent(unsigned char ch)
	{
		keybd_event(ch,0,0,0);
		keybd_event(ch,0,KEYEVENTF_KEYUP,0);
	}
	void RemoteProcessOpen(){
		if (!m_man.GetProcessHandle()) m_man.OpenRemoteProcess(THSysMisc::GetPidFromHwnd(hWnd));
	}
};

class THDlgItemTabCtrl : public THDlgItem
{
public:
	inline LRESULT SelectTab(int nSelect)
	{
		//如果是 CPropertySheet 应在CPropertySheet类中SendMessage(PSM_SETCURSEL, nPage)切换
		NMHDR selch;
		selch.hwndFrom=hWnd;
		selch.idFrom=nCtrlId;
		selch.code=TCN_SELCHANGING;
		::SendMessage(hParentWnd,WM_NOTIFY,MAKELONG(TCN_SELCHANGING,0),(LPARAM)&selch);
		selch.code=TCN_SELCHANGE;
		::SendMessage(hParentWnd,WM_NOTIFY,MAKELONG(TCN_SELCHANGE,0),(LPARAM)&selch);
		selch.code=NM_CLICK;
		return ::SendMessage(hParentWnd,WM_NOTIFY,MAKELONG(NM_CLICK,0),(LPARAM)&selch);
	}
};

class THDlgItemButton : public THDlgItem
{
public:
	inline LRESULT SendClickedMessage(){return SendCtrlMessage(BN_CLICKED);}
	inline LRESULT PostClickedMessage(){return PostCtrlMessage(BN_CLICKED);}
};

class THDlgItemEdit : public THDlgItem
{
public:
	inline void SetText(THString text){SendMessage(WM_SETTEXT,0,(LPARAM)(LPCTSTR)text);}
};

class THDlgItemListCtrl : public THDlgItem
{
public:
	inline int GetItemCount(){return (int)SendMessage(LVM_GETITEMCOUNT,0,0L);}
	inline HWND GetHeaderCtrl(){return (HWND) SendMessage(LVM_GETHEADER,0,0);}
	inline void SelectItem(int nItem,BOOL bSelect=TRUE)
	{
		SetItemState(nItem,bSelect?(LVIS_SELECTED|LVIS_FOCUSED):0,LVIS_SELECTED|LVIS_FOCUSED);
		NMITEMACTIVATE nia;
		nia.hdr.code=NM_CLICK;
		nia.hdr.hwndFrom=hWnd;
		nia.hdr.idFrom=nCtrlId;
		nia.iItem=nItem;
		nia.iSubItem=0;
		nia.lParam=NULL;
		nia.ptAction.x=0;
		nia.ptAction.y=0;
		nia.uChanged=0;
		nia.uKeyFlags=0;
		nia.uNewState=0;
		nia.uOldState=0;
		::SendMessage(hParentWnd,WM_NOTIFY,MAKELONG(NM_CLICK,0),(LPARAM)&nia);
	}
	BOOL SetItemState(int nItem,int nState,int nStateMask)
	{
		LVITEM lvi;
		lvi.stateMask = nStateMask;
		lvi.state = nState;
		return (BOOL)SendMessage(LVM_SETITEMSTATE, nItem, (LPARAM)&lvi);
	}
	THString GetItemText(int nItem,int nSubItem){
		THString str;
		LVITEM lvi;
		memset(&lvi, 0, sizeof(LVITEM));
		lvi.iSubItem = nSubItem;
		RemoteProcessOpen();
		if (!m_remotebuf1) m_remotebuf1=m_man.AllocBuffer(sizeof(LVITEM));
		if (!m_remotebuf2) m_remotebuf2=m_man.AllocBuffer(4096);
		lvi.cchTextMax = 4096;
		lvi.pszText = (LPTSTR)m_remotebuf2;
		m_man.CommitBuffer(&lvi,m_remotebuf1,sizeof(LVITEM));
		int nRes  = (int)SendMessage(LVM_GETITEMTEXT, (WPARAM)nItem, (LPARAM)m_remotebuf1);
		m_man.UpdateBuffer(m_remotebuf2,str.GetBufferSetLength(4096),nRes+1);
		str.ReleaseBuffer();
		return str;
	}
	inline BOOL DeleteItem(int nItem){ return (BOOL)SendMessage(LVM_DELETEITEM, nItem, 0L);}
	inline UINT GetItemState(int nItem, UINT nMask) { return (UINT)SendMessage(LVM_GETITEMSTATE, nItem, nMask);}
};

/**
* @brief 对话框元素枚举回调响应类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-06-24 新建类
*/
/**<pre>
</pre>*/
class ITHDlgItemEnumCallBack
{
public:
	/**
	* @brief 响应对话框控件枚举
	* @param hWnd			控件句柄
	* @param nCtrlId		控件ID
	* @param sClass			类名
	* @param sWindow		标题
	* @param rClientRect	控件矩形
	* @param nLevel			控件所在层
	* @param nIdx			控件唯一标识
	* @param pAddData		附加参数
	* @return 是否取消枚举
	*/
	virtual BOOL OnEnumWindow(HWND hWnd,int nCtrlId,THString sClass,THString sWindow,RECT rClientRect,int nLevel,int nIdx,void *pAddData)
	{
		THDebug(_T("Level:%d,Idx:%d,hWnd:0x%X,CtrlId:%d,Class:%s,Window:%s,Rect:%d,%d,%d,%d"),nLevel,nIdx,hWnd,nCtrlId,sClass,sWindow,rClientRect.left,rClientRect.top,rClientRect.right,rClientRect.bottom);
		return FALSE;
	}
};

/**
* @brief 对话框元素枚举类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-06-24 新建类
*/
/**<pre>
</pre>*/
class THDlgItemEnumer
{
public:
	THDlgItemEnumer()
	{
	}

	virtual ~THDlgItemEnumer()
	{
	}

	/**
	* @brief 枚举对话框的所有子控件
	* @param hDlgWnd	主窗口句柄
	* @param cb			枚举回调函数
	* @param adddata	附加参数
	*/
	static void EnumAllSubWindows(HWND hDlgWnd,ITHDlgItemEnumCallBack *cb=NULL,void *adddata=NULL)
	{
		int idx=0;
		ITHDlgItemEnumCallBack ncb;
		if (!cb) cb=&ncb;
		_EnumAllSubWindows(hDlgWnd,cb,0,idx,adddata,hDlgWnd);
	}

	/**
	* @brief 枚举对话框的重要子控件
	* @param hDlgWnd	主窗口句柄
	* @param cb			枚举回调函数
	* @param adddata	附加参数
	*/
	static void EnumMainSubWindows(HWND hDlgWnd,ITHDlgItemEnumCallBack *cb=NULL,void *adddata=NULL)
	{
		int idx=0;
		ITHDlgItemEnumCallBack ncb;
		if (!cb) cb=&ncb;
		_EnumMainSubWindows(hDlgWnd,cb,0,idx,adddata,hDlgWnd);
	}
protected:
	static BOOL _EnumAllSubWindows(HWND hDlgWnd,ITHDlgItemEnumCallBack *cb,int level,int &idx,void *adddata,HWND hRootWnd)
	{
		if (cb)
		{
			HWND hch=NULL;
			while((hch=::FindWindowEx(hDlgWnd,hch,NULL,NULL))!=NULL)
			{
				TCHAR buffer[4096],buffer1[4096];
				::GetClassName(hch,buffer,4096);
				::GetWindowText(hch,buffer1,4096);
				int id=::GetDlgCtrlID(hch);
				RECT rect;
				::GetWindowRect(hch,&rect);
				ScreenToClient(hRootWnd,(LPPOINT)&rect);
				ScreenToClient(hRootWnd,((LPPOINT)&rect)+1);
				if (cb->OnEnumWindow(hch,id,buffer,buffer1,rect,level,idx,adddata))
					return TRUE;
				idx++;
				if (_EnumAllSubWindows(hch,cb,level+1,idx,adddata,hRootWnd)) return TRUE;
			}
		}
		return FALSE;
	}

	static BOOL _EnumMainSubWindows(HWND hDlgWnd,ITHDlgItemEnumCallBack *cb,int level,int &idx,void *adddata,HWND hRootWnd)
	{
		if (cb)
		{
			HWND hch=NULL;
			while((hch=::FindWindowEx(hDlgWnd,hch,NULL,NULL))!=NULL)
			{
				if (IsWindowVisible(hch))
				{
					RECT rect;
					::GetWindowRect(hch,&rect);
					if (!::IsRectEmpty(&rect))
					{
						int oldidx=idx;
						//如果窗口较大，进入下一层枚举
						if (rect.right-rect.left>100 || rect.bottom-rect.top>100)
						{
							if (_EnumMainSubWindows(hch,cb,level+1,idx,adddata,hRootWnd)) return TRUE;
						}
						//如果下一层没有子窗口，或当前窗口就比较小了，进入cb
						if (idx==oldidx)
						{
							TCHAR buffer[4096],buffer1[4096];
							::GetClassName(hch,buffer,4096);
							::GetWindowText(hch,buffer1,4096);
							int id=::GetDlgCtrlID(hch);
							ScreenToClient(hRootWnd,(LPPOINT)&rect);
							ScreenToClient(hRootWnd,((LPPOINT)&rect)+1);
							if (cb->OnEnumWindow(hch,id,buffer,buffer1,rect,level,idx,adddata)) return TRUE;
						}
						idx++;
					}
				}
			}
		}
		return FALSE;
	}
};

class THDlgItemEnumBuffered : private ITHDlgItemEnumCallBack
{
public:
	THDlgItemEnumBuffered()
	{
		m_info.SetFreeProc(FreeInfo);
	}
	virtual ~THDlgItemEnumBuffered()
	{
	}

	void LoadDialog(HWND hWnd)
	{
		m_info.RemoveAll();
		THDlgItemEnumer::EnumAllSubWindows(hWnd,this,(void *)1);
	}

	void LoadDialogOnlyMain(HWND hWnd)
	{
		m_info.RemoveAll();
		THDlgItemEnumer::EnumMainSubWindows(hWnd,this,(void *)0);
	}

	THDlgItem *FindControlByType(THString sType,int nIdx=0)
	{
		for(UINT i=0;i<m_info.GetSize();i++)
		{
			if (m_info[i]->sClass==sType)
			{
				if (nIdx==0)
					return m_info[i];
				else
					nIdx--;
			}
		}
		return NULL;
	}

	THDlgItem *FindControlByCtrlId(int nId)
	{
		for(UINT i=0;i<m_info.GetSize();i++)
		{
			if (m_info[i]->nCtrlId==nId)
			{
				return m_info[i];
			}
		}
		return NULL;
	}

	THDlgItem *FindControlByWindowText(THString sWindow,THString sType=_T(""),int nIdx=0)
	{
		for(UINT i=0;i<m_info.GetSize();i++)
		{
			if (m_info[i]->sWindow==sWindow && m_info[i]->sClass.Find(sType)!=-1)
			{
				if (nIdx==0)
					return m_info[i];
				else
					nIdx--;
			}
		}
		return NULL;
	}

	THDlgItem *GetBegin() {return GetNext(NULL);}

	THDlgItem *GetNext(THDlgItem *prev)
	{
		for(UINT i=0;i<m_info.GetSize();i++)
		{
			if (prev==NULL) return m_info[i];
			if (m_info[i]==prev) prev=NULL;
		}
		return NULL;
	}

	inline UINT GetCtrlCount() {return m_info.GetSize();}
private:
	virtual BOOL OnEnumWindow(HWND hWnd,int nCtrlId,THString sClass,THString sWindow,RECT rClientRect,int nLevel,int nIdx,void *pAddData)
	{
		if (pAddData) THDebug(_T("Level:%d,Idx:%d,hWnd:0x%X,CtrlId:%d,Class:%s,Window:%s,Rect:%d,%d,%d,%d"),nLevel,nIdx,hWnd,nCtrlId,sClass,sWindow,rClientRect.left,rClientRect.top,rClientRect.right,rClientRect.bottom);
		THDlgItem *info=new THDlgItem;
		if (info)
		{
			info->hWnd=hWnd;
			info->nCtrlId=nCtrlId;
			info->nIdx=nIdx;
			info->nLevel=nLevel;
			info->sClass=sClass;
			info->sWindow=sWindow;
			info->hParentWnd=::GetParent(hWnd);
			info->rClientRect=rClientRect;
			m_info.AddTail(info);
		}
		return FALSE;
	}
	static void FreeInfo(void *key,void *value,void *adddata)
	{
		delete (THDlgItem *)value;
	}
	THList<THDlgItem *> m_info;
};
