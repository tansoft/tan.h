#pragma once

#define THSYSTEM_INCLUDE_STDIO
#define THSYSTEM_INCLUDE_MATH
#include <THSystem.h>
#include <THString.h>
#include <THStruct.h>

#define DEFAULT_DIALOG_STYLE			WS_POPUP|WS_BORDER|WS_SYSMENU|DS_MODALFRAME|WS_CAPTION
#define DEFAULT_BUTTON_STYLE			WS_CHILD|WS_VISIBLE|WS_TABSTOP
#define DEFAULT_EDIT_STYLE				WS_CHILD|WS_VISIBLE|ES_LEFT|WS_BORDER|WS_TABSTOP|ES_AUTOHSCROLL
#define DEFAULT_STATIC_STYLE			WS_CHILD|WS_VISIBLE|SS_LEFT
#define DEFAULT_GROUPBOX_STYLE			WS_CHILD|WS_VISIBLE|BS_GROUPBOX
#define	DEFAULT_TREECTRL_STYLE			WS_CHILD|WS_VISIBLE|WS_TABSTOP

#pragma pack(push, 1)
typedef struct
{
	WORD dlgVer;
	WORD signature;
	DWORD helpID;
	DWORD exStyle;
	DWORD style;
	WORD cDlgItems;
	short x;
	short y;
	short cx;
	short cy;
	// Everything else in this structure is variable length,
	// and therefore must be determined dynamically

	// sz_Or_Ord menu;			// name or ordinal of a menu resource
	// sz_Or_Ord windowClass;	// name or ordinal of a window class
	// WCHAR title[titleLen];	// title string of the dialog box
	// short pointsize;			// only if DS_SETFONT is set
	// short weight;			// only if DS_SETFONT is set
	// short bItalic;			// only if DS_SETFONT is set
	// WCHAR font[fontLen];		// typeface name, if DS_SETFONT is set
} DLGTEMPLATEEX;

typedef struct
{
	DWORD helpID;
	DWORD exStyle;
	DWORD style;
	short x;
	short y;
	short cx;
	short cy;
	DWORD id;
} DLGITEMTEMPLATEEX;
#pragma pack(pop)

class THBaseDialog;
static THMap<HWND,THBaseDialog *> g_windowmap;

/**
* @brief 基本对话框封装类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-03-05 新建类
*/
/**<pre>
	用法： 重载 OnDialogProc 实现消息截获，函数返回TRUE时，不调用类中默认处理过程。
	AddXXX 函数生成对应的组件
	ShowModalDialog 显示模式对话框
	ShowDialog	显示非模式对话框
	子类继承后，显示对话框的一般流程如下：
		if (!AllocBuffer()) return 0;
		InitDialog(sWindowText,200,80);
		AddButton(150,15,40,15,_T("确定"),IDOK,DEFAULT_BUTTON_STYLE|BS_DEFPUSHBUTTON);
		...
		return ShowModalDialog(hinst,hwndOwner);
</pre>*/
class THBaseDialog
{
public:
	THBaseDialog()
	{
		m_hgbl=NULL;
		m_lpw=0;
	}
	virtual ~THBaseDialog(){}

	virtual BOOL OnDialogProc(LRESULT &lResult,HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)=0;

	static LRESULT CALLBACK DialogProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
	{
		if (uMsg==WM_INITDIALOG)
		{
			g_windowmap.SetAt(hWnd,(THBaseDialog *)(UINT_PTR)lParam);
		}
		THBaseDialog *tmp=g_windowmap.GetAt(hWnd);
		if (tmp)
		{
			LRESULT ret;
			if (tmp->OnDialogProc(ret,hWnd,uMsg,wParam,lParam))
				return ret;
		}
		switch(uMsg)
		{
			case WM_COMMAND:
			{
				switch (LOWORD(wParam))
				{
					case IDOK:
						g_windowmap.RemoveAt(hWnd);
						EndDialog(hWnd,IDOK);
						break;
					case IDCANCEL:
						g_windowmap.RemoveAt(hWnd);
						EndDialog(hWnd,IDCANCEL);
						break;
				}
			}
				break;
			case WM_DESTROY:
			{
				g_windowmap.RemoveAt(hWnd);
			}
			break;
		}
		return FALSE;
	}
protected:
	HGLOBAL m_hgbl;
	DLGTEMPLATEEX *m_lpdt;
	LPWORD m_lpw;

	BOOL AllocBuffer()
	{
		if (m_hgbl) FreeBuffer();
		m_hgbl=GlobalAlloc(GMEM_ZEROINIT|GMEM_FIXED,1024);
		if (!m_hgbl) return FALSE;
		m_lpdt=(DLGTEMPLATEEX *)GlobalLock(m_hgbl);
		return m_lpdt!=NULL;
	}

	BOOL FreeBuffer()
	{
		if (m_hgbl)
		{
			GlobalUnlock(m_hgbl);
			GlobalFree(m_hgbl);
			m_hgbl=NULL;
		}
		return TRUE;
	}

	LPWORD lpwAlign(LPWORD lpIn)
	{
		ULONG ul;
		ul = (ULONG)(UINT_PTR)lpIn;
		ul +=3;
		ul >>=2;
		ul <<=2;
		return (LPWORD)(UINT_PTR) ul;
	}

	void InitDialog(THString sWindowText,int cx,int cy,DWORD dwStyle=DEFAULT_DIALOG_STYLE)
	{
		LPWSTR lpwsz;
		WCHAR nchar;
		m_lpdt->dlgVer = 1;m_lpdt->signature = 0xffff;
		m_lpdt->helpID = 0;m_lpdt->exStyle = 0;
		m_lpdt->style = dwStyle|DS_SETFONT;
		m_lpdt->cDlgItems = 0; // number of controls
		m_lpdt->x = 0; m_lpdt->y = 0;
		m_lpdt->cx = cx; m_lpdt->cy = cy;
		m_lpw = (LPWORD) (m_lpdt + 1);
		*m_lpw++ = 0; // no menu
		*m_lpw++ = 0; // predefined dialog box class (by default)
		// WCHAR title[titleLen];	// title string of the dialog box
		// short pointsize;			// only if DS_SETFONT is set
		// short weight;			// only if DS_SETFONT is set
		// short bItalic;			// only if DS_SETFONT is set
		// WCHAR font[fontLen];		// typeface name, if DS_SETFONT is set
		lpwsz = (LPWSTR) m_lpw;
		THCharset::t2w(sWindowText,lpwsz,50);
		nchar = 1+ (WCHAR)wcslen(lpwsz);
		m_lpw += nchar;
		short *tmp=(short *)m_lpw;
		*tmp++=9;//9 pt
		*tmp++=FW_NORMAL;//normal weight
		*tmp++=0;//bItalic=false
		m_lpw = (LPWORD)tmp;
		lpwsz = (LPWSTR) tmp;//font
		THCharset::t2w(_T("宋体"),lpwsz,50);
		nchar = 1+ (WCHAR)wcslen(lpwsz);
		m_lpw += nchar;
	}

	void AddControlByClassName(int x,int y,int cx,int cy,THString text,UINT id,THString className,DWORD dwStyle,DWORD dwExStyle)
	{
		LPWSTR lpwsz;
		WCHAR nchar;
		DLGITEMTEMPLATEEX *lpdit;
		m_lpw = lpwAlign (m_lpw);
		lpdit = (DLGITEMTEMPLATEEX *) m_lpw;
		lpdit->x = x; lpdit->y = y;
		lpdit->cx = cx; lpdit->cy = cy;
		lpdit->id = id;
		lpdit->style = dwStyle;
		lpdit->exStyle = dwExStyle;
		lpdit->helpID = 0;
		m_lpw = (LPWORD) (lpdit + 1); 
		//class
		lpwsz = (LPWSTR) m_lpw;
		THCharset::t2w(className,lpwsz,50);
		nchar = 1+ (WCHAR)wcslen(lpwsz);
		m_lpw += nchar;
		//title
		lpwsz = (LPWSTR) m_lpw;
		THCharset::t2w(text,lpwsz,50);
		nchar = 1+ (WCHAR)wcslen(lpwsz);
		m_lpw += nchar;
		*m_lpw++ = 0; // no creation data
		m_lpdt->cDlgItems++;
	}

	/*clsType
		0x0080 Button
		0x0081 Edit
		0x0082 Static
		0x0083 List box
		0x0084 Scroll bar
		0x0085 Combo box
	*/
	void AddControl(int x,int y,int cx,int cy,THString text,UINT id,WORD clsType,DWORD dwStyle,DWORD dwExStyle)
	{
		LPWSTR lpwsz;
		WCHAR nchar;
		DLGITEMTEMPLATEEX *lpdit;
		m_lpw = lpwAlign (m_lpw);
		lpdit = (DLGITEMTEMPLATEEX *) m_lpw;
		lpdit->x = x; lpdit->y = y;
		lpdit->cx = cx; lpdit->cy = cy;
		lpdit->id = id;
		lpdit->style = dwStyle;
		lpdit->exStyle = dwExStyle;
		lpdit->helpID = 0;
		m_lpw = (LPWORD) (lpdit + 1); 
		*m_lpw++ = 0xFFFF; 
		*m_lpw++ = clsType;// button class
		lpwsz = (LPWSTR) m_lpw;
		THCharset::t2w(text,lpwsz,50);
		nchar = 1+ (WCHAR)wcslen(lpwsz);
		m_lpw += nchar;
		*m_lpw++ = 0; // no creation data
		m_lpdt->cDlgItems++;
	}
	inline void AddButton(int x,int y,int cx,int cy,THString text,UINT id,DWORD dwStyle=DEFAULT_BUTTON_STYLE,DWORD dwExStyle=0){AddControl(x,y,cx,cy,text,id,0x0080,dwStyle,dwExStyle);}
	inline void AddEdit(int x,int y,int cx,int cy,THString text,UINT id,DWORD dwStyle=DEFAULT_EDIT_STYLE,DWORD dwExStyle=0){AddControl(x,y,cx,cy,text,id,0x0081,dwStyle,dwExStyle);}
	inline void AddStatic(int x,int y,int cx,int cy,THString text,UINT id=IDC_STATIC,DWORD dwStyle=DEFAULT_STATIC_STYLE,DWORD dwExStyle=0){AddControl(x,y,cx,cy,text,id,0x0082,dwStyle,dwExStyle);}
	inline void AddGroupBox(int x,int y,int cx,int cy,THString text,UINT id=IDC_STATIC,DWORD dwStyle=DEFAULT_GROUPBOX_STYLE,DWORD dwExStyle=0){AddButton(x,y,cx,cy,text,id,dwStyle,dwExStyle);}
	inline void AddTreeCtrl(int x,int y,int cx,int cy,UINT id,DWORD dwStyle=DEFAULT_TREECTRL_STYLE,DWORD dwExStyle=0){AddControlByClassName(x,y,cx,cy,_T(""),id,_T("SysTreeView32"),dwStyle,dwExStyle);}

	int ShowModalDialog(HINSTANCE hinst,HWND hwndOwner)
	{
		if (!m_hgbl) return 0;
		GlobalUnlock(m_hgbl);
		int ret = (int)DialogBoxIndirectParam(hinst,(LPDLGTEMPLATE)m_hgbl,hwndOwner,(DLGPROC)DialogProc,(LPARAM)this);
		FreeBuffer();
		return ret;
	}

	HWND ShowDialog(HINSTANCE hinst,HWND hwndOwner)
	{
		if (!m_hgbl) return NULL;
		GlobalUnlock(m_hgbl);
		HWND hwnd=CreateDialogIndirectParam(hinst,(LPDLGTEMPLATE)m_hgbl,hwndOwner,(DLGPROC)DialogProc,(LPARAM)this);
		if (!hwnd) return NULL;
		ShowWindow(hwnd,SW_SHOW);
		UpdateWindow(hwnd);
		FreeBuffer();
		return hwnd;
	}

	BOOL DestroyDialog(HWND hWnd)
	{
		if (!hWnd) return FALSE;
		return DestroyWindow(hWnd);
	}
};

class ITHBasePassDialogEventHandler
{
public:
	virtual BOOL IsInputPassRight(THString sUser,THString sOrg,THString sInput,void *addParam)=0;
};

#define	PASSDIALOGTYPE_INPUTPASS	1		///<输入密码对话框，提供的密码为空时，用户输入密码即返回，否则比较一样才退出
#define PASSDIALOGTYPE_CONFIRMPASS	2		///<输入密码并确认对话框
#define PASSDIALOGTYPE_CHANGEPASS	3		///<更改密码对话框

/**
* @brief 基本密码输入框封装类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-03-07 新建类
*/
/**<pre>
	THBasePassDialog m_pass;
	THString ret;
	if (m_pass.ShowPassDialog(&ret,_T("输入密码"),theApp.m_hInstance,PASSDIALOGTYPE_INPUTPASS,NULL)==IDOK);
	if (m_pass.ShowPassDialog(&ret,_T("输入密码"),theApp.m_hInstance,PASSDIALOGTYPE_INPUTPASS,NULL)==IDOK);
	if (m_pass.ShowPassDialog(&ret,_T("设置密码"),theApp.m_hInstance,PASSDIALOGTYPE_CONFIRMPASS,NULL)==IDOK);
	MessageBox(ret);
	if (m_pass.ShowPassDialog(&ret,_T("更改密码"),theApp.m_hInstance,PASSDIALOGTYPE_CHANGEPASS,NULL)==IDOK);
	MessageBox(ret);
</pre>*/
class THBasePassDialog :public THBaseDialog,private ITHBasePassDialogEventHandler
{
public:
	THBasePassDialog()
	{
		m_sPassData=NULL;
		m_nType=0;
		m_handler=this;
		m_adddata=NULL;
	}

	virtual ~THBasePassDialog()
	{
	}

	void SetEventHandler(ITHBasePassDialogEventHandler *handler,void *adddata=NULL)
	{
		if (handler==NULL)
			handler=this;
		m_handler=handler;
		m_adddata=adddata;
	}

	virtual BOOL OnDialogProc(LRESULT &lResult,HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
	{
		if (uMsg==WM_COMMAND)
		{
			if (LOWORD(wParam)==IDOK)
			{
				if (m_sPassData)
				{
					THString user=GetUser(hWnd);
					//check
					if (m_nType==PASSDIALOGTYPE_INPUTPASS)
					{
						THString str;
						HWND hPass=GetDlgItem(hWnd,1200);
						GetWindowText(hPass,str.GetBuffer(4096),4096);
						str.ReleaseBuffer();
						if (str.IsEmpty())
						{
							MessageBox(hWnd,_T("密码不能为空！"),_T("请输入密码"),MB_ICONINFORMATION|MB_OK);
							SetFocus(hPass);
							return TRUE;
						}
						if (!m_sPassData->IsEmpty())
						{
							if (!m_handler->IsInputPassRight(user,*m_sPassData,str,m_adddata))
							{
								MessageBox(hWnd,_T("密码错误，请重新输入！"),_T("请输入密码"),MB_ICONINFORMATION|MB_OK);
								SetFocus(hPass);
								return TRUE;
							}
						}
						m_sPassData->SetString(str);
					}
					else if (m_nType==PASSDIALOGTYPE_CONFIRMPASS)
					{
						THString str,str1;
						HWND hPass=GetDlgItem(hWnd,1200);
						GetWindowText(hPass,str.GetBuffer(4096),4096);
						str.ReleaseBuffer();
						if (str.IsEmpty())
						{
							MessageBox(hWnd,_T("密码不能为空！"),_T("请输入密码"),MB_ICONINFORMATION|MB_OK);
							SetFocus(hPass);
							return TRUE;
						}
						HWND hPass1=GetDlgItem(hWnd,1201);
						GetWindowText(hPass1,str1.GetBuffer(4096),4096);
						str1.ReleaseBuffer();
						if (str1.IsEmpty())
						{
							MessageBox(hWnd,_T("确认密码不能为空！"),_T("请输入密码"),MB_ICONINFORMATION|MB_OK);
							SetFocus(hPass1);
							return TRUE;
						}
						if (str1.Compare(str)!=0)
						{
							MessageBox(hWnd,_T("两次输入的密码不相同，请重新输入！"),_T("请输入密码"),MB_ICONINFORMATION|MB_OK);
							SetFocus(hPass);
							return TRUE;
						}
						m_sPassData->SetString(str);
					}
					else if (m_nType==PASSDIALOGTYPE_CHANGEPASS)
					{
						THString str,str1,str2;
						HWND hPass2=GetDlgItem(hWnd,1200);
						GetWindowText(hPass2,str2.GetBuffer(4096),4096);
						str2.ReleaseBuffer();
						if (!m_handler->IsInputPassRight(user,*m_sPassData,str2,m_adddata))
						{
							MessageBox(hWnd,_T("原密码输入错误，请重新输入！"),_T("请输入密码"),MB_ICONINFORMATION|MB_OK);
							SetFocus(hPass2);
							return TRUE;
						}
						HWND hPass=GetDlgItem(hWnd,1201);
						GetWindowText(hPass,str.GetBuffer(4096),4096);
						str.ReleaseBuffer();
						if (str.IsEmpty())
						{
							MessageBox(hWnd,_T("新密码不能为空！"),_T("请输入密码"),MB_ICONINFORMATION|MB_OK);
							SetFocus(hPass);
							return TRUE;
						}
						HWND hPass1=GetDlgItem(hWnd,1202);
						GetWindowText(hPass1,str1.GetBuffer(4096),4096);
						str1.ReleaseBuffer();
						if (str1.IsEmpty())
						{
							MessageBox(hWnd,_T("确认密码不能为空！"),_T("请输入密码"),MB_ICONINFORMATION|MB_OK);
							SetFocus(hPass1);
							return TRUE;
						}
						if (str1.Compare(str)!=0)
						{
							MessageBox(hWnd,_T("两次输入的新密码不相同，请重新输入！"),_T("请输入密码"),MB_ICONINFORMATION|MB_OK);
							SetFocus(hPass);
							return TRUE;
						}
						m_sPassData->SetString(str);
					}
				}
			}
		}
		else if (uMsg==WM_INITDIALOG)
		{
			SetFocus(GetDlgItem(hWnd,1200));
			lResult=FALSE;
			return TRUE;
		}
		return FALSE;
	}

	int ShowPassDialog(THString *sPassData,THString sWindowText,HINSTANCE hinst,int Type=PASSDIALOGTYPE_INPUTPASS,HWND hwndOwner=GetDesktopWindow())
	{
		m_sPassData=sPassData;
		m_nType=Type;
		if (!AllocBuffer()) return 0;
		int w,h;
		GetDialogWH(w,h);
		InitDialog(sWindowText,w,h);
		AddButton(150,15,40,15,_T("确定"),IDOK,DEFAULT_BUTTON_STYLE|BS_DEFPUSHBUTTON);
		AddButton(150,40,40,15,_T("取消"),IDCANCEL);
		int y=GetBaseY();
		if (Type==PASSDIALOGTYPE_INPUTPASS)
		{
			AddEdit(55,y+15,75,12,_T(""),1200,DEFAULT_EDIT_STYLE|ES_PASSWORD);
			AddStatic(15,y+17,30,10,_T("密码："),IDC_STATIC);
		}
		else if (Type==PASSDIALOGTYPE_CONFIRMPASS)
		{
			AddEdit(65,y+5,65,12,_T(""),1200,DEFAULT_EDIT_STYLE|ES_PASSWORD);
			AddEdit(65,y+30,65,12,_T(""),1201,DEFAULT_EDIT_STYLE|ES_PASSWORD);
			AddStatic(15,y+7,40,10,_T("密码："),IDC_STATIC);
			AddStatic(15,y+32,40,10,_T("重复密码："),IDC_STATIC);
		}
		else if (Type==PASSDIALOGTYPE_CHANGEPASS)
		{
			AddEdit(65,y,65,12,_T(""),1200,DEFAULT_EDIT_STYLE|ES_PASSWORD);
			AddEdit(65,y+20,65,12,_T(""),1201,DEFAULT_EDIT_STYLE|ES_PASSWORD);
			AddEdit(65,y+40,65,12,_T(""),1202,DEFAULT_EDIT_STYLE|ES_PASSWORD);
			AddStatic(15,y+2,40,10,_T("原密码："),IDC_STATIC);
			AddStatic(15,y+22,40,10,_T("新密码："),IDC_STATIC);
			AddStatic(15,y+42,40,10,_T("重复密码："),IDC_STATIC);
		}
		return ShowModalDialog(hinst,hwndOwner);
	}
protected:
	THString *m_sPassData;
	UINT m_nType;
	ITHBasePassDialogEventHandler *m_handler;
	void *m_adddata;

	virtual BOOL IsInputPassRight(THString sUser,THString sOrg,THString sInput,void *addParam)
	{
		return (sOrg.Compare(sInput)==0);
	}

	virtual void GetDialogWH(int &w,int &h)
	{
		w=200;
		h=80;
	}

	virtual int GetBaseY()
	{
		return 13;
	}

	virtual THString GetUser(HWND hWnd){return _T("");}
};

/**
* @brief 基本帐户密码输入框封装类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-03-07 新建类
*/
/**<pre>
	THString sRet=_T("barry:12345|barry1:54321|");
	THBaseUserPassDialog dlg;
	dlg.ShowPassDialog(&sRet,_T("请输入密码"),theApp.m_hInstance,PASSDIALOGTYPE_INPUTPASS,m_hWnd);
	THDebug(sRet);
	dlg.ShowPassDialog(&sRet,_T("请输入密码"),theApp.m_hInstance,PASSDIALOGTYPE_CONFIRMPASS,m_hWnd);
	THDebug(sRet);
	dlg.ShowPassDialog(&sRet,_T("请输入密码"),theApp.m_hInstance,PASSDIALOGTYPE_CHANGEPASS,m_hWnd);
	THDebug(sRet);
</pre>*/
class THBaseUserPassDialog :public THBasePassDialog
{
public:
	virtual BOOL OnDialogProc(LRESULT &lResult,HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
	{
		if (uMsg==WM_INITDIALOG)
		{
			SetFocus(GetDlgItem(hWnd,1199));
			lResult=FALSE;
			return TRUE;
		}
		else if (uMsg==WM_COMMAND)
		{
			if (LOWORD(wParam)==IDOK)
			{
				if (m_sPassData)
				{
					THString str=GetUser(hWnd);
					if (str.IsEmpty())
					{
						MessageBox(hWnd,_T("用户名不能为空！"),_T("请输入用户名"),MB_ICONINFORMATION|MB_OK);
						return TRUE;
					}
					if (THBasePassDialog::OnDialogProc(lResult,hWnd,uMsg,wParam,lParam)) return TRUE;
					/*if (m_nType==PASSDIALOGTYPE_INPUTPASS)
					THString str;
					HWND hUser=GetDlgItem(hWnd,1199);
					GetWindowText(hUser,str.GetBuffer(4096),4096);
					str.ReleaseBuffer();*/
					m_sPassData->SetString(str+_T(":")+*m_sPassData);
				}
			}
		}
		return FALSE;
	}
protected:
	virtual void GetDialogWH(int &w,int &h)
	{
		w=200;
		if (m_nType==PASSDIALOGTYPE_INPUTPASS) h=80;
		else h=100;
	}

	virtual int GetBaseY()
	{
		if (m_nType==PASSDIALOGTYPE_INPUTPASS)
		{
			AddEdit(55,18,75,12,_T(""),1199);
			AddStatic(15,18+2,40,10,_T("用户名："),IDC_STATIC);
			return 28;
		}
		AddEdit(65,13,65,12,_T(""),1199);
		AddStatic(15,13+2,40,10,_T("用户名："),IDC_STATIC);
		return 33;
	}

	virtual THString GetUser(HWND hWnd)
	{
		THString str;
		HWND hUser=GetDlgItem(hWnd,1199);
		GetWindowText(hUser,str.GetBuffer(4096),4096);
		str.ReleaseBuffer();
		return str;
	}

	//sOrg 为帐户阵列，barry:123456|barry1:54321|
	virtual BOOL IsInputPassRight(THString sUser,THString sOrg,THString sInput,void *addParam)
	{
		THStringToken t(sOrg,_T("|"));
		while(t.IsMoreTokens())
		{
			if (t.GetNextToken()==sUser+_T(":")+sInput)
				return TRUE;
		}
		return FALSE;
	}
};

class ITHBaseInputDialogEventHandler
{
public:
	virtual BOOL IsInputRight(int nCnt,THString sInput,void *addParam)=0;
};

#define	INPUTDIALOGTYPE_NOHINT		0		///<没有提示语句
#define INPUTDIALOGTYPE_HINTUP		1		///<提示语句在上方
#define INPUTDIALOGTYPE_HINTLEFT	2		///<提示语句在左方
#define INPUTDIALOGTYPE_NOTCHECK	4		///<确定时不判断是否有输入，在没有设置SetEventHandler时生效

/**
* @brief 基本信息输入框封装类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-03-07 新建类
*/
/**<pre>
</pre>*/
class THBaseInputDialog : public THBaseDialog,public ITHBaseInputDialogEventHandler
{
public:
	THBaseInputDialog()
	{
		m_sRetData=NULL;
		m_nCnt=0;
		m_nMode=0;
		m_handler=this;
		m_adddata=NULL;
	}

	virtual ~THBaseInputDialog()
	{
	}

	void SetEventHandler(ITHBaseInputDialogEventHandler *handler,void *adddata=NULL)
	{
		if (handler==NULL)
			handler=this;
		m_handler=handler;
		m_adddata=adddata;
	}

	THString ShowInputDialogQuick(THString title,THString hint,HINSTANCE hinst,HWND hWnd,BOOL bCheck=TRUE)
	{
		THString ret;
		THString theret;
		int hitmode=INPUTDIALOGTYPE_HINTUP;
		if (!bCheck) hitmode|=INPUTDIALOGTYPE_NOTCHECK;
		if (ShowInputDialog(&theret,title,THSimpleXml::MakeParam(_T("Hint0"),hint),hinst,hitmode,1,hWnd)==IDOK)
		{
			ret=THSimpleXml::GetParam(theret,_T("ret0"));
		}
		return ret;
	}
#define DIALOG_W	200
#define BUTTON_W	40
#define EDIT_H		12
#define U_EDIT_W	95
#define U_EDIT_S	20
#define L_EDIT_W	75
#define L_EDIT_S	50
#define STATIC_H	10
#define L_STATIC_W	30
#define L_STATIC_S	10
#define U_STATIC_W	100
#define U_STATIC_S	15
#define FONT_MAP_X	5
#define FONT_MAP_Y	5
#define START_H		20
#define END_H		20
#define H_PRE		10
	/**
	* @brief 开始基本输入框
	* @param sRetData	返回数据，以ret0开始为key，对应值为对应的返回内容
	* @param sTitle		对话框标题
	* @param sHint		对话框提示，以Hint0开始为key，对应值为对应的提示内容
	* @param hinst		Instance句柄
	* @param nHintMode	提示放置地方
	* @param nDisCnt	输入项目个数
	* @param hwndOwner	父窗口
	* @return 对话框返回值
	*/
	int ShowInputDialog(THString *sRetData,THString sTitle,THString sHint,HINSTANCE hinst,int nHintMode=INPUTDIALOGTYPE_HINTUP,UINT nDisCnt=1,HWND hwndOwner=GetDesktopWindow())
	{
		m_sRetData=sRetData;
		m_nCnt=nDisCnt;
		m_nMode=nHintMode;
		if (!AllocBuffer()) return 0;
		int nHeight=0;
		THStringArray ar;
		THString tmp;
		UINT len,line;
		for(UINT i=0;i<nDisCnt;i++)
		{
			tmp.Format(_T("Hint%d"),i);
			tmp=THSimpleXml::GetParam(sHint,tmp);
			ar.Add(tmp);
			if (nHintMode&INPUTDIALOGTYPE_HINTUP)
			{
				len=THStringConv::GetStringSize(tmp)*FONT_MAP_X;
				line=(len+U_STATIC_W-1)/U_STATIC_W;
				nHeight+=line*STATIC_H+EDIT_H+H_PRE*2;
			}
			else if (nHintMode&INPUTDIALOGTYPE_HINTLEFT)
			{
				len=THStringConv::GetStringSize(tmp)*FONT_MAP_X;
				line=(len+L_STATIC_W-1)/L_STATIC_W;
				nHeight+=max(line*STATIC_H,EDIT_H)+H_PRE;
			}
			else
			{
				nHeight+=EDIT_H+10;
			}
		}
		nHeight+=START_H+END_H;
		InitDialog(sTitle,DIALOG_W,nHeight);
		AddButton(DIALOG_W-BUTTON_W-10,15,BUTTON_W,15,_T("确定"),IDOK,DEFAULT_BUTTON_STYLE|BS_DEFPUSHBUTTON);
		AddButton(DIALOG_W-BUTTON_W-10,40,BUTTON_W,15,_T("取消"),IDCANCEL);
		nHeight=START_H;
		UINT nID=1200;
		for(i=0;i<nDisCnt;i++)
		{
			tmp=ar[i];
			if (nHintMode&INPUTDIALOGTYPE_HINTUP)
			{
				len=THStringConv::GetStringSize(tmp)*FONT_MAP_X;
				line=(len+U_STATIC_W-1)/U_STATIC_W;
				AddStatic(U_STATIC_S,nHeight,U_STATIC_W,line*STATIC_H,tmp,IDC_STATIC);
				nHeight+=line*STATIC_H+H_PRE;
				AddEdit(U_EDIT_S,nHeight,U_EDIT_W,EDIT_H,_T(""),nID+i);
				nHeight+=EDIT_H+H_PRE;
			}
			else if (nHintMode&INPUTDIALOGTYPE_HINTLEFT)
			{
				len=THStringConv::GetStringSize(tmp)*FONT_MAP_X;
				line=(len+L_STATIC_W-1)/L_STATIC_W;
				AddStatic(L_STATIC_S,nHeight,L_STATIC_W,line*STATIC_H,tmp,IDC_STATIC);
				AddEdit(L_EDIT_S,nHeight,L_EDIT_W,EDIT_H,_T(""),nID+i);
				nHeight+=max(line*STATIC_H,EDIT_H)+H_PRE;
			}
			else
			{
				AddEdit(U_EDIT_S,nHeight,U_EDIT_W,EDIT_H,_T(""),nID+i);
				nHeight+=EDIT_H+H_PRE;
			}
		}
		return ShowModalDialog(hinst,hwndOwner);
	}


	virtual BOOL OnDialogProc(LRESULT &lResult,HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
	{
		if (uMsg==WM_COMMAND)
		{
			if (LOWORD(wParam)==IDOK)
			{
				THString ret;
				for(UINT i=0;i<m_nCnt;i++)
				{
					THString str;
					HWND hEd=GetDlgItem(hWnd,1200+i);
					GetWindowText(hEd,str.GetBuffer(4096),4096);
					str.ReleaseBuffer();
					if (m_handler)
					{
						if (!m_handler->IsInputRight(i,str,m_adddata))
						{
							SetFocus(hEd);
							return TRUE;
						}
					}
					ret+=THSimpleXml::MakeParam(_T("ret")+THu2s(i),str);
				}
				if (m_sRetData)
				{
					*m_sRetData=ret;
				}
			}
		}
		else if (uMsg==WM_INITDIALOG)
		{
			SetFocus(GetDlgItem(hWnd,1200));
			lResult=FALSE;
			return TRUE;
		}
		return FALSE;
	}
protected:
	THString *m_sRetData;
	UINT m_nCnt;
	UINT m_nMode;
	ITHBaseInputDialogEventHandler *m_handler;
	void *m_adddata;

	virtual BOOL IsInputRight(int nCnt,THString sInput,void *addParam)
	{
		if (m_nMode&INPUTDIALOGTYPE_NOTCHECK) return TRUE;
		if (sInput.IsEmpty())
		{
			MessageBox(NULL,_T("输入不能为空！"),_T("输入错误"),MB_ICONINFORMATION|MB_OK);
			return FALSE;
		}
		return TRUE;
	}
};

/**
* @brief 基本等待中对话框封装类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-03-07 新建类
*/
/**<pre>
</pre>*/
class THBaseWaitingDialog : private THBaseDialog
{
public:
	THBaseWaitingDialog()
	{
		m_hWnd=NULL;
	}

	virtual ~THBaseWaitingDialog()
	{
		EndWaitingDialog();
	}

	virtual BOOL OnDialogProc(LRESULT &lResult,HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam){return FALSE;}

	BOOL ShowWaitingDialog(HINSTANCE hinst,HWND hwndOwner,THString sWindowText,THString sWaiting)
	{
		if (m_hWnd) EndWaitingDialog();
		if (!AllocBuffer()) return FALSE;
		InitDialog(sWindowText,200,100);
		AddStatic(0,40,200,20,sWaiting,1200,WS_CHILD|WS_VISIBLE|SS_CENTER);
		m_hWnd=ShowDialog(hinst,hwndOwner);
		return m_hWnd!=NULL;
	}

	void SetWaitingText(THString str)
	{
		if (IsWindow(m_hWnd))
			SetWindowText(GetDlgItem(m_hWnd,1200),str);
	}

	void EndWaitingDialog()
	{
		if (m_hWnd)
		{
			EndDialog(m_hWnd,0);
			g_windowmap.RemoveAt(m_hWnd);
			DestroyDialog(m_hWnd);
		}
		m_hWnd=NULL;
	}
private:
	HWND m_hWnd;
};

#define MULSELECTDIALOGTYPE_RADIOBUTTON		1		///<单选框形式
#define MULSELECTDIALOGTYPE_CHECKBOX			2		///<复选框形式
#define MULSELECTDIALOGTYPE_PUSHBUTTON		3		///<按钮形式，选择某个后返回
#define MULSELECTDIALOGTYPE_CHECKBUTTON		4		///<按钮形式，有按下效果
/**
* @brief 基本多选对话框封装类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-08-07 新建类
*/
/**<pre>
</pre>*/
class THBaseMulSelectDialog : private THBaseDialog
{
public:
	THBaseMulSelectDialog()
	{
		m_nItemCount=0;
		m_sRetData=NULL;
		m_nMode=0;
	}

	virtual ~THBaseMulSelectDialog()
	{
	}

	virtual BOOL OnDialogProc(LRESULT &lResult,HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
	{
		if (uMsg==WM_INITDIALOG)
		{
			THStringToken t(m_initset,_T(","));
			while(t.IsMoreTokens())
			{
				int id=THs2i(t.GetNextToken());
				if (id>0)
				{
					HWND hBtnWnd=GetDlgItem(hWnd,id);
					if (hBtnWnd)
					{
						if (m_nMode!=MULSELECTDIALOGTYPE_PUSHBUTTON)
						{
							::SendMessage(hBtnWnd,BM_SETCHECK,BST_CHECKED,0);
							//radio button is only select one
							if (m_nMode==MULSELECTDIALOGTYPE_RADIOBUTTON) break;
						}
					}
				}
			}
		}
		else if (uMsg==WM_COMMAND)
		{
			UINT id=LOWORD(wParam);
			if (m_nMode==MULSELECTDIALOGTYPE_PUSHBUTTON)
			{
				if (id>=1200 && id<1200+m_nItemCount)
				{
					m_sRetData->AppendFormat(_T("<ret%d>1</ret%d>"),id-1200,id-1200);
					//to close the dialog
					::PostMessage(hWnd,WM_COMMAND,MAKEWPARAM(IDOK,BN_CLICKED),NULL);
				}
			}
			else if (id==IDOK)
			{
				if (m_sRetData)
				{
					THString str;
					for(UINT i=1200;i<1200+m_nItemCount;i++)
					{
						HWND hBtnWnd=GetDlgItem(hWnd,i);
						if (hBtnWnd)
						{
							if ((int)::SendMessage(hBtnWnd, BM_GETCHECK, 0, 0)==BST_CHECKED)
							{
								m_sRetData->AppendFormat(_T("<ret%d>1</ret%d>"),i-1200,i-1200);
								//radio button is only select one
								if(m_nMode==MULSELECTDIALOGTYPE_RADIOBUTTON) break;
							}
						}
					}
				}
			}
		}
		return FALSE;
	}

	/**
	* @brief 开始多选输入框
	* @param hinst					Instance句柄
	* @param hwndOwner			父窗口
	* @param sWindowText		对话框标题
	* @param sItemData			每项显示内容，以item0开始为key，对应值为对应的显示内容，以selected0开始，为对应项是否需要选中的内容，可忽略
	* @param sRetData				返回数据，以ret0开始为key，对应值为对应的返回内容
	* @param nItemCount			输入项目个数
	* @param sItemTitle				外框内容，为空为不显示外框
	* @param nMode					选择方式
	* @return 对话框返回值
	*/
	int ShowMulSelectDialog(HINSTANCE hinst,HWND hwndOwner,THString sWindowText,
		THString sItemData,THString *sRetData,UINT nItemCount,THString sItemTitle=_T(""),int nMode=MULSELECTDIALOGTYPE_RADIOBUTTON)
	{
		m_nItemCount=nItemCount;
		if (sRetData) sRetData->Empty();
		m_sRetData=sRetData;
		m_nMode=nMode;
		if (!AllocBuffer()) return FALSE;
		int x=200;
		int y=50+nItemCount*20;
		InitDialog(sWindowText,x,y);
		AddGroupBox(5,5,x-10,y-10,sItemTitle);
		DWORD dwStyle=DEFAULT_BUTTON_STYLE;
		if (nMode==MULSELECTDIALOGTYPE_RADIOBUTTON)
			dwStyle|=BS_AUTORADIOBUTTON;
		else if (nMode==MULSELECTDIALOGTYPE_CHECKBOX)
			dwStyle|=BS_AUTOCHECKBOX;
		else if (nMode==MULSELECTDIALOGTYPE_CHECKBUTTON)
			dwStyle|=BS_AUTOCHECKBOX|BS_PUSHLIKE;
		//else if (nMode==MULSELECTDIALOGTYPE_PUSHBUTTON)
		THString tmp;
		int nStart;
		int EndSize;
		if (nMode==MULSELECTDIALOGTYPE_PUSHBUTTON || nMode==MULSELECTDIALOGTYPE_CHECKBUTTON)
			nStart=10;
		else
			nStart=20;
		if (nMode==MULSELECTDIALOGTYPE_PUSHBUTTON)
			EndSize=x-nStart*2;
		else
			EndSize=x-nStart*2-45;
		for(UINT i=0;i<nItemCount;i++)
		{
			tmp.Format(_T("item%d"),i);
			tmp=THSimpleXml::GetParam(sItemData,tmp);
			AddButton(nStart,25+i*20,EndSize,20,tmp,1200+i,dwStyle);
			tmp.Format(_T("selected%d"),i);
			if (THs2i(THSimpleXml::GetParam(sItemData,tmp))!=0)
				m_initset.AppendFormat(_T("%d,"),1200+i);
		}
		if (nMode!=MULSELECTDIALOGTYPE_PUSHBUTTON)
		{
			AddButton(x-40-nStart,15,40,15,_T("确定"),IDOK,DEFAULT_BUTTON_STYLE|BS_DEFPUSHBUTTON);
			AddButton(x-40-nStart,40,40,15,_T("取消"),IDCANCEL);
		}
		return ShowModalDialog(hinst,hwndOwner);
	}
private:
	THString m_initset;		///<保存initdialog时需要设置默认值的item
	UINT m_nItemCount;
	THString *m_sRetData;
	int m_nMode;
};

/**
* @brief 基本树控件消息封装类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-10-26 新建类
*/
/**<pre>
</pre>*/
class THTreeCtrl
{
public:
	THTreeCtrl(){m_hWnd=NULL;}
	virtual ~THTreeCtrl(){}
	void SetHWnd(HWND hWnd){m_hWnd=hWnd;}

/*	inline HTREEITEM InsertItem(LPTVINSERTSTRUCT lpInsertStruct)
	{return (HTREEITEM)::SendMessage(m_hWnd, TVM_INSERTITEM, 0, (LPARAM)lpInsertStruct);}
	inline HTREEITEM InsertItem(LPCTSTR lpszItem, int nImage,int nSelectedImage, HTREEITEM hParent, HTREEITEM hInsertAfter)
	{return InsertItem(TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE, lpszItem, nImage, nSelectedImage, 0, 0, 0, hParent, hInsertAfter); }
	inline HTREEITEM InsertItem(LPCTSTR lpszItem, HTREEITEM hParent, HTREEITEM hInsertAfter)
	{return InsertItem(TVIF_TEXT, lpszItem, 0, 0, 0, 0, 0, hParent, hInsertAfter); }
	inline BOOL DeleteItem(HTREEITEM hItem)
	{return (BOOL)::SendMessage(m_hWnd, TVM_DELETEITEM, 0, (LPARAM)hItem); }
	inline BOOL DeleteAllItems()
	{return (BOOL)::SendMessage(m_hWnd, TVM_DELETEITEM, 0, (LPARAM)TVI_ROOT);}
	inline BOOL Expand(HTREEITEM hItem, UINT nCode)
	{return (BOOL)::SendMessage(m_hWnd, TVM_EXPAND, nCode, (LPARAM)hItem); }
	inline UINT GetCount() const
	{return (UINT)::SendMessage(m_hWnd, TVM_GETCOUNT, 0, 0); }
	inline UINT GetIndent() const
	{return (UINT)::SendMessage(m_hWnd, TVM_GETINDENT, 0, 0); }
	inline void SetIndent(UINT nIndent)
	{::SendMessage(m_hWnd, TVM_SETINDENT, nIndent, 0); }
	inline HIMAGELIST GetImageList(UINT nImageList) const
	{return (HIMAGELIST)::SendMessage(m_hWnd, TVM_GETIMAGELIST, (UINT)nImageList, 0); }
	inline HIMAGELIST SetImageList(HIMAGELIST hImageList, int nImageListType)
	{return (HIMAGELIST)::SendMessage(m_hWnd, TVM_SETIMAGELIST, (UINT)nImageListType, (LPARAM)HIMAGELIST); }
	inline UINT SetScrollTime(UINT uScrollTime)
	{return (UINT) ::SendMessage(m_hWnd, TVM_SETSCROLLTIME, (WPARAM) uScrollTime, 0); }
	inline HTREEITEM GetNextItem(HTREEITEM hItem, UINT nCode) const
	{return (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, nCode, (LPARAM)hItem); }
	inline HTREEITEM GetChildItem(HTREEITEM hItem) const
	{return (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)hItem); }
	inline HTREEITEM GetNextSiblingItem(HTREEITEM hItem) const
	{return (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)hItem); }
	inline HTREEITEM GetPrevSiblingItem(HTREEITEM hItem) const
	{return (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_PREVIOUS, (LPARAM)hItem); }
	inline HTREEITEM GetParentItem(HTREEITEM hItem) const
	{return (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_PARENT, (LPARAM)hItem); }
	inline HTREEITEM GetFirstVisibleItem() const
	{return (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_FIRSTVISIBLE, 0); }
	inline HTREEITEM GetNextVisibleItem(HTREEITEM hItem) const
	{return (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_NEXTVISIBLE, (LPARAM)hItem); }
	inline HTREEITEM GetPrevVisibleItem(HTREEITEM hItem) const
	{return (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_PREVIOUSVISIBLE, (LPARAM)hItem); }
	inline HTREEITEM GetSelectedItem() const
	{return (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_CARET, 0); }
	inline UINT GetScrollTime() const
	{return (UINT) ::SendMessage(m_hWnd, TVM_GETSCROLLTIME, 0, 0); }
	inline HTREEITEM GetDropHilightItem() const
	{return (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_DROPHILITE, 0); }
	inline HTREEITEM GetRootItem() const
	{return (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_ROOT, 0); }
	inline BOOL Select(HTREEITEM hItem, UINT nCode)
	{return (BOOL)::SendMessage(m_hWnd, TVM_SELECTITEM, nCode, (LPARAM)hItem); }
	inline BOOL SelectItem(HTREEITEM hItem)
	{return (BOOL)::SendMessage(m_hWnd, TVM_SELECTITEM, TVGN_CARET, (LPARAM)hItem); }
	inline BOOL SelectDropTarget(HTREEITEM hItem)
	{return (BOOL)::SendMessage(m_hWnd, TVM_SELECTITEM, TVGN_DROPHILITE, (LPARAM)hItem); }
	inline BOOL SelectSetFirstVisible(HTREEITEM hItem)
	{return (BOOL)::SendMessage(m_hWnd, TVM_SELECTITEM, TVGN_FIRSTVISIBLE, (LPARAM)hItem); }
	inline BOOL GetItem(TVITEM* pItem) const
	{return (BOOL)::SendMessage(m_hWnd, TVM_GETITEM, 0, (LPARAM)pItem); }
	inline BOOL SetItem(TVITEM* pItem)
	{return (BOOL)::SendMessage(m_hWnd, TVM_SETITEM, 0, (LPARAM)pItem); }
	inline BOOL SetItemText(HTREEITEM hItem, LPCTSTR lpszItem)
	{return SetItem(hItem, TVIF_TEXT, lpszItem, 0, 0, 0, 0, NULL); }
	inline BOOL SetItemImage(HTREEITEM hItem, int nImage, int nSelectedImage)
	{return SetItem(hItem, TVIF_IMAGE|TVIF_SELECTEDIMAGE, NULL, nImage, nSelectedImage, 0, 0, NULL); }
	inline BOOL SetItemState(HTREEITEM hItem, UINT nState, UINT nStateMask)
	{return SetItem(hItem, TVIF_STATE, NULL, 0, 0, nState, nStateMask, NULL); }
	inline BOOL SetItemData(HTREEITEM hItem, DWORD_PTR dwData)
	{return SetItem(hItem, TVIF_PARAM, NULL, 0, 0, 0, 0, (LPARAM)dwData); }
	inline HTREEITEM HitTest(TVHITTESTINFO* pHitTestInfo) const
	{return (HTREEITEM)::SendMessage(m_hWnd, TVM_HITTEST, 0, (LPARAM)pHitTestInfo); }
	inline HWND GetEditControl() const
	{return (HWND)::SendMessage(m_hWnd, TVM_GETEDITCONTROL, 0, 0); }
	inline UINT GetVisibleCount() const
	{return (UINT)::SendMessage(m_hWnd, TVM_GETVISIBLECOUNT, 0, 0); }
	inline BOOL SortChildren(HTREEITEM hItem)
	{return (BOOL)::SendMessage(m_hWnd, TVM_SORTCHILDREN, 0, (LPARAM)hItem); }
	inline BOOL EnsureVisible(HTREEITEM hItem)
	{return (BOOL)::SendMessage(m_hWnd, TVM_ENSUREVISIBLE, 0, (LPARAM)hItem); }
	inline BOOL SortChildrenCB(LPTVSORTCB pSort)
	{return (BOOL)::SendMessage(m_hWnd, TVM_SORTCHILDRENCB, 0, (LPARAM)pSort); }
	inline HWND GetToolTips() const
	{return (HWND)::SendMessage(m_hWnd, TVM_GETTOOLTIPS, 0, 0L); }
	inline HWND SetToolTips(HWND hWndTip)
	{return (HWND)::SendMessage(m_hWnd, TVM_SETTOOLTIPS, (WPARAM)hWndTip, 0L); }
	inline COLORREF GetBkColor() const
	{return (COLORREF) ::SendMessage(m_hWnd, TVM_GETBKCOLOR, 0, 0L); }
	inline COLORREF SetBkColor(COLORREF clr)
	{return (COLORREF) ::SendMessage(m_hWnd, TVM_SETBKCOLOR, 0, (LPARAM)clr); }
	inline SHORT GetItemHeight() const
	{return (SHORT) ::SendMessage(m_hWnd, TVM_GETITEMHEIGHT, 0, 0L); }
	inline SHORT SetItemHeight(SHORT cyHeight)
	{return (SHORT) ::SendMessage(m_hWnd, TVM_SETITEMHEIGHT, cyHeight, 0L); }
	inline COLORREF GetTextColor() const
	{return (COLORREF) ::SendMessage(m_hWnd, TVM_GETTEXTCOLOR, 0, 0L); }
	inline COLORREF SetTextColor(COLORREF clr)
	{return (COLORREF) ::SendMessage(m_hWnd, TVM_SETTEXTCOLOR, 0, (LPARAM)clr); }
	inline BOOL SetInsertMark(HTREEITEM hItem, BOOL fAfter)
	{return (BOOL) ::SendMessage(m_hWnd, TVM_SETINSERTMARK, fAfter, (LPARAM)hItem); }
	inline COLORREF GetInsertMarkColor() const
	{return (COLORREF) ::SendMessage(m_hWnd, TVM_GETINSERTMARKCOLOR, 0, 0L); }
	inline COLORREF SetInsertMarkColor(COLORREF clrNew)
	{return (COLORREF) ::SendMessage(m_hWnd, TVM_SETINSERTMARKCOLOR, 0, (LPARAM)clrNew); }
*/
protected:
	HWND m_hWnd;
};

class ITHTreeCtrlMessageHandler
{
public:
	virtual void OnInit(THTreeCtrl *ctrl){}
	//fixme,cause the event
	virtual BOOL IsAllowOkNow(THTreeCtrl *ctrl,HTREEITEM hItem,void *pItemData){return TRUE;}
	virtual BOOL IsAllowCancelNow(THTreeCtrl *ctrl,HTREEITEM hItem,void *pItemData){return TRUE;}
	virtual void OnOK(THTreeCtrl *ctrl,HTREEITEM hItem,void *pItemData){}
	virtual void OnCancel(){}
	virtual void OnItemExpand(THTreeCtrl *ctrl,HTREEITEM hItem,void *pItemData){}
	virtual void OnItemSelect(THTreeCtrl *ctrl,HTREEITEM hItem,void *pItemData){}
	virtual void OnItemClick(THTreeCtrl *ctrl,HTREEITEM hItem,void *pItemData){}
	virtual void OnItemDbClick(THTreeCtrl *ctrl,HTREEITEM hItem,void *pItemData){}
	virtual void OnItemRClick(THTreeCtrl *ctrl,HTREEITEM hItem,void *pItemData){}
};

/**
* @brief 基本树控件对话框封装类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-10-26 新建类
*/
/**<pre>
</pre>*/
class THBaseTreeDialog : THBaseDialog
{
public:
	THBaseTreeDialog()
	{
		m_handler=NULL;
	}

	virtual ~THBaseTreeDialog()
	{
	}

	virtual BOOL OnDialogProc(LRESULT &lResult,HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
	{
		if (uMsg==WM_INITDIALOG)
		{
			m_ctrl.SetHWnd(GetDlgItem(hWnd,1200));
			if (m_handler) m_handler->OnInit(&m_ctrl);
		}
		return FALSE;
	}

	int ShowTreeDialog(HINSTANCE hinst,HWND hwndOwner,THString sWindowText,
		THString sSubTitle=_T(""),ITHTreeCtrlMessageHandler *handler=NULL)
	{
		if (!AllocBuffer()) return FALSE;
		m_handler=handler;
		int x=200;
		int y=230;
		InitDialog(sWindowText,x,y);
		AddStatic(10,10,180,10,sSubTitle);
		AddTreeCtrl(15,25,170,170,1200);
		AddButton(40,205,40,15,_T("确定"),IDOK,DEFAULT_BUTTON_STYLE|BS_DEFPUSHBUTTON);
		AddButton(120,205,40,15,_T("取消"),IDCANCEL);
		return ShowModalDialog(hinst,hwndOwner);
	}
protected:
	THTreeCtrl m_ctrl;
	ITHTreeCtrlMessageHandler *m_handler;
};
