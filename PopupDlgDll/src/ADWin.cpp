#include "StdAfx.h"
#include <ATLTYPES.H>
#include <shellapi.h>
#include <atlstr.h>
#include <atlsimpcoll.h>
#include <time.h>
#include ".\adwin.h"

#define CMD_FLAG _T("popupcmd://")
#define CMD_CLOSE _T("CloseWindow")
#define CMD_MOVE _T("MoveWindow")
#define CMD_MIN _T("MinWindow")
#define CMD_MAX	_T("MaxWindow")

extern HMODULE g_hModule;
CSimpleValArray<HWND> m_ADHwndArr;

VOID funShowAD(LPARAM lParam)
{
	::CoInitialize(NULL);
	CADMainWin *pADWin = new CADMainWin;

	RECT rt;
	rt.left = 0;
	rt.top = 0;
	rt.right = 10;
	rt.bottom = 10;
	HWND hWnd = pADWin->Create(GetDesktopWindow(),rt,THSimpleXml::GetParam((char *)lParam,"url","[","]"),WS_POPUP);
	if(hWnd)
	{

		m_ADHwndArr.Add(hWnd);
		if (pADWin->SetInterface() && pADWin->SetADInfo((char *)lParam))
		{
			pADWin->OnStart();
			MSG msg;
			BOOL bRet;
			while(bRet = ::GetMessage(&msg, NULL, 0, 0))
			{

				if(bRet == -1)
				{
					ATLTRACE(_T("::GetMessage returned -1 (error)\n"));
					continue;   // error, don't process
				}
				else 
				{
					::TranslateMessage(&msg);
					::DispatchMessage(&msg);				
				}		
			}
		}
	}
	//delete pADWin;
	free((char *)lParam);
	//::CoUninitialize();
}

BOOL CADMainWin::ParseCCMD(LPTSTR szUrl , VARIANT_BOOL *pbCancel)
{
	INT nLastIndex =  (INT)_tcslen(szUrl) - 1;
	if(nLastIndex < 0)
		return FALSE;
	if(szUrl[nLastIndex] == _T('/') )
		szUrl[nLastIndex] = NULL;
	CString strUrl(szUrl);
	strUrl.Replace(_T("%20"),_T(" "));

	UINT uStartIndex = (UINT)_tcslen(CMD_FLAG);
	INT nFind = -1;
	if(strUrl.Find(CMD_FLAG,0) != 0)
	{
		//call ext url fn
		if (m_ADInfo.cbNavUrl)
			*pbCancel = m_ADInfo.cbNavUrl(szUrl,m_ADInfo.cbAddData);
		else
			*pbCancel = FALSE;
		return FALSE;
	}
	*pbCancel = TRUE;
	if( (nFind = strUrl.Find(CMD_CLOSE,uStartIndex)) == uStartIndex)
	{
		DestroyWindow();
	}
	else if( (nFind = strUrl.Find(CMD_MOVE,uStartIndex)) == uStartIndex)
	{
		SendMessage(WM_NCLBUTTONDOWN, HTCAPTION, 0);
	}
	else if( (nFind = strUrl.Find(CMD_MIN,uStartIndex)) == uStartIndex)
	{
		ShowWindow(SW_MINIMIZE);
	}
	else if( (nFind = strUrl.Find(CMD_MAX,uStartIndex)) == uStartIndex)
	{
		if(IsZoomed())
			ShowWindow(SW_RESTORE);
		else
			ShowWindow(SW_MAXIMIZE);
	}
	else
	{
		//call ext cmd fn
		if (m_ADInfo.cbNavCmd)
			*pbCancel = m_ADInfo.cbNavCmd(strUrl.Mid(uStartIndex),m_ADInfo.cbAddData);
		else
			*pbCancel = FALSE;
	}
	return TRUE;
}

void CADMainWin::OnStart()
{
	if (m_ADInfo.cbWindowStart)
		m_ADInfo.cbWindowStart(m_ADInfo.szUrl,m_ADInfo.cbAddData);
}

CADMainWin::CADMainWin(void):m_bError(FALSE),m_WinStat(ADWS_HIDE),m_uCountTime(0),m_lWinWidth(0),m_lWinHeight(0),m_bShowFlag(FALSE),
m_lNCWidth(0),m_lNCHeight(0)
{
	//RECT rt;
	//::GetWindowRect(GetDesktopWindow(),&rt);
	//m_lScreenBottom = rt.bottom;
	//m_lScreenRight = rt.right;
	m_ADInfo.bIsTaskBT=FALSE;
	m_ADInfo.CloseType=DISPLAY_TYPE_U2D;
	m_ADInfo.DispType=DISPLAY_TYPE_D2U;
	m_ADInfo.hWndInsertAfter=HWND_TOPMOST;
	m_ADInfo.lADHeight=0;
	m_ADInfo.lADWidth=0;
	m_ADInfo.nIcon=0;
	m_ADInfo.nTimeOut=0;
	m_ADInfo.szUrl[0]='\0';
	m_ADInfo.uDefer=0;
	m_ADInfo.uDlgType=0;
	m_ADInfo.x=0;
	m_ADInfo.y=0;
	m_ADInfo.posx[0]='\0';
	m_ADInfo.posy[0]='\0';
	m_ADInfo.bMustHaveHtmlTag=FALSE;
	m_ADInfo.cbNavUrl=NULL;
	m_ADInfo.cbNavCmd=NULL;
	m_ADInfo.cbNavErr=NULL;
	m_ADInfo.cbNavNewWindow=NULL;
	m_ADInfo.cbWindowClose=NULL;
	m_ADInfo.cbWindowStart=NULL;
	m_ADInfo.cbSetTitle=NULL;
	m_ADInfo.cbWindowOpen=NULL;
	m_ADInfo.cbAddData=NULL;
}

CADMainWin::~CADMainWin(void)
{

}

LRESULT CADMainWin::OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// TODO: Add your message handler code here and/or call default
	CRect rt;
	POINT pt;
	GetWindowRect(&rt);
	GetCursorPos(&pt);
	switch(wParam) 
	{
	case TIMER_PTR_COUNT:
		m_uCountTime ++;
		if((int)m_uCountTime > m_ADInfo.nTimeOut)
		{
			if(!rt.PtInRect(pt))
				CloseAD();
		}
		break;
	case TIMER_PTR_SGLIDE:
		{
			if(rt.PtInRect(pt) && !m_bOPShow)
				return 0;
			GlideNotify();
		}
		break;
	}
	
	return 0;
}

void __stdcall CADMainWin::OnNavigateComplete2(IDispatch *pDisp,VARIANT *URL )
{
	CComPtr<IDispatch> spDisp;
	CComQIPtr<IHTMLDocument2> spDoc2;
	CComBSTR bstrTitle;
	m_spWebBrowser2->get_Document(&spDisp);
	spDoc2 = spDisp;
	spDoc2->get_title(&bstrTitle);
	CW2A szTitle(bstrTitle);
	SetWindowText(szTitle);
	if (m_ADInfo.cbSetTitle)
		m_ADInfo.cbSetTitle(szTitle,m_ADInfo.cbAddData);
}
void __stdcall CADMainWin::OnBeforeNavigate2(IDispatch *pDisp,
					 VARIANT *url,
					 VARIANT *Flags,
					 VARIANT *TargetFrameName,
					 VARIANT *PostData,
					 VARIANT *Headers,
					 VARIANT_BOOL *Cancel
					 )
{
	CW2T szURL(url->bstrVal);
	ParseCCMD(szURL,Cancel);
}

void __stdcall CADMainWin::OnNavigateError(          IDispatch *pDisp,
				   VARIANT *URL,
				   VARIANT *TargetFrameName,
				   VARIANT *StatusCode,
				   VARIANT_BOOL *Cancel
				   )
{
	//callback ext nav err fn
	if (m_ADInfo.cbNavErr)
	{
		CString strURL(V_BSTR(URL));
		//CString strFrame(V_BSTR(pvFrame));
		DWORD dwError = V_I4(StatusCode);
		m_ADInfo.cbNavErr(strURL,dwError,m_ADInfo.cbAddData);
	}
	m_bError = TRUE;
}
void __stdcall CADMainWin::OnNewWindow2(          IDispatch **ppDisp,
						  VARIANT_BOOL *Cancel
						  )
{
	if(m_ADInfo.DispType == DISPLAY_TYPE_HIDE)
		return;
	//callback ext new window fn
	if (m_ADInfo.cbNavNewWindow)
		m_ADInfo.cbNavNewWindow(m_ADInfo.szUrl,m_ADInfo.cbAddData);
}
void __stdcall CADMainWin:: OnDocumentComplete(          IDispatch *pDisp,
											   VARIANT *URL
											   )
{
	if(m_bShowFlag) return;
	READYSTATE ReadyState;
	m_spWebBrowser2->get_ReadyState(&ReadyState);
	if(ReadyState != READYSTATE_COMPLETE) return;
	if (!m_spWebBrowser2) return;
	IHTMLElementCollection *p;
	CComPtr<IDispatch> spDisp;
	CComQIPtr<IHTMLDocument2> spDoc2;
	m_spWebBrowser2->get_Document(&spDisp);
	spDoc2 = spDisp;
	if (!spDoc2) return;
	HRESULT hr=spDoc2->get_all(&p);
	if(m_bError)
	{
		DestroyWindow();
		return;
	}
	BOOL bHaveHtmlTag=FALSE;
	if (SUCCEEDED(hr) && p)
	{
		CComVariant varName;
		CComVariant varIndex;
		CComPtr<IDispatch> spdispElem;
		varName.vt = VT_BSTR;
		varName.bstrVal = T2BSTR_EX(_T("PopupSettings"));
		hr=p->item(varName,varIndex,&spdispElem);
		if (SUCCEEDED(hr) && spdispElem)
		{
			CComPtr<IHTMLElement> spP1;
			spP1 = spdispElem;
			if (spP1)
			{
				BSTR ret;
				hr=spP1->get_innerHTML(&ret);
				if (SUCCEEDED(hr))
				{
					m_bShowFlag=TRUE;
					CString str(ret);
					SetADInfo(str);
					bHaveHtmlTag=TRUE;
				}
			}
		}
	}
	//html tag not found
	if (bHaveHtmlTag==FALSE && m_ADInfo.bMustHaveHtmlTag==TRUE)
	{
		DestroyWindow();
		return;
	}
	if (m_ADInfo.cbWindowOpen)
		if (m_ADInfo.cbWindowOpen(m_ADInfo.szUrl,m_ADInfo.cbAddData))
		{
			DestroyWindow();
			return;
		}
	Sleep(1000 * m_ADInfo.uDefer);
	if(!OnShowAD(&m_ADInfo)) return;
}

LRESULT CADMainWin::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// TODO: Add your message handler code here and/or call default
	if(CBrowserEvents::m_dwEventCookie != 0xFEFEFEFE)
	{
		CBrowserEvents::Unadvise(m_spWebBrowser2);
		//call back close window fn
		if (m_ADInfo.cbWindowClose)
			m_ADInfo.cbWindowClose(m_ADInfo.szUrl,m_ADInfo.cbAddData);
	}
	PostQuitMessage(1);
	m_ADHwndArr.Remove(m_hWnd);
	return 0;
}

BOOL CADMainWin::SetScreenValue(THString *strSrc)
{
	LONG lw;
	LONG lh;
	USHORT uStyle;
	BOOL bIsTaskBt;
	TCHAR szVar[16] = {'\0'};

	lw=m_ADInfo.lADWidth;
	lh=m_ADInfo.lADHeight;
	uStyle=m_ADInfo.uDlgType;
	bIsTaskBt=m_ADInfo.bIsTaskBT;

	int bw,bh,th;
	bw=::GetSystemMetrics(SM_CXEDGE) + ::GetSystemMetrics(SM_CXBORDER);
	bh = ::GetSystemMetrics(SM_CYEDGE) + ::GetSystemMetrics(SM_CYBORDER);
	if(bIsTaskBt)
		th = ::GetSystemMetrics(SM_CYCAPTION);
	else
		th = ::GetSystemMetrics(SM_CYSMCAPTION);

	if (uStyle & DLGTYPE_RESIZE)
	{
		if(!(uStyle & DLGTYPE_TITLE || uStyle & DLGTYPE_MIN || uStyle & DLGTYPE_MAX))
		{
			bw += ::GetSystemMetrics(SM_CXDLGFRAME);
			bh += ::GetSystemMetrics(SM_CYDLGFRAME);
		}
		else
		{
			bw += ::GetSystemMetrics(SM_CXBORDER);
			bh += ::GetSystemMetrics(SM_CYBORDER);
		}
	}

	_itoa(bw,szVar,10);
	strSrc->Replace("bw",szVar);
	_itoa(bh,szVar,10);
	strSrc->Replace("bh",szVar);
	_itoa(th,szVar,10);
	strSrc->Replace("th",szVar);

	THSysMisc::srand();
	int r;
	r = rand();
	_itoa(r,szVar,10);
	strSrc->Replace("rand",szVar);

	//_itoa(GetSystemMetrics(SM_CXFULLSCREEN),szVar,10);
	//strSrc->Replace("a",szVar);
	//_itoa(GetSystemMetrics(SM_CYFULLSCREEN),szVar,10);
	//strSrc->Replace("b",szVar);

	RECT rect;
	SystemParametersInfo(SPI_GETWORKAREA,0,&rect,0);
	_itoa(rect.left,szVar,10);
	strSrc->Replace("sx",szVar);
	_itoa(rect.top,szVar,10);
	strSrc->Replace("sy",szVar);
	_itoa(rect.right,szVar,10);
	strSrc->Replace("ex",szVar);
	_itoa(rect.bottom,szVar,10);
	strSrc->Replace("ey",szVar);

	_itoa(GetSystemMetrics(SM_CXSCREEN),szVar,10);
	strSrc->Replace("scx",szVar);
	_itoa(GetSystemMetrics(SM_CYSCREEN),szVar,10);
	strSrc->Replace("scy",szVar);

	_ltoa(lw,szVar,10);
	strSrc->Replace("w",szVar);
	_ltoa(lh,szVar,10);
	strSrc->Replace("h",szVar);
	return TRUE;
}

BOOL CADMainWin::SetADInfo(const char *settings)
{
	THString set=settings;
	THString tmp;
	tmp=THSimpleXml::GetParam(set,"taskbar","[","]");
	if (!tmp.IsEmpty())
		m_ADInfo.bIsTaskBT=THStringConv::s2i(tmp);
	tmp=THSimpleXml::GetParam(set,"dlgtype","[","]");
	if (!tmp.IsEmpty())
		m_ADInfo.uDlgType=THStringConv::s2i(tmp);
	tmp=THSimpleXml::GetParam(set,"closetype","[","]");
	if (!tmp.IsEmpty())
		m_ADInfo.CloseType=(DISPLAY_TYPE)THStringConv::s2i(tmp);
	tmp=THSimpleXml::GetParam(set,"distype","[","]");
	if (!tmp.IsEmpty())
		m_ADInfo.DispType=(DISPLAY_TYPE)THStringConv::s2i(tmp);
	tmp=THSimpleXml::GetParam(set,"showtype","[","]");
	if (!tmp.IsEmpty())
		m_ADInfo.hWndInsertAfter=(HWND)(INT_PTR)THStringConv::s2i(tmp);
	double ret;
	tmp=THSimpleXml::GetParam(set,"height","[","]");
	if (!tmp.IsEmpty())
	{
		if (SetScreenValue(&tmp) && THLogicCalc::LogicCalc(tmp,&ret))
			m_ADInfo.lADHeight=(int)ret;
	}
	tmp=THSimpleXml::GetParam(set,"width","[","]");
	if (!tmp.IsEmpty())
	{
		if (SetScreenValue(&tmp) && THLogicCalc::LogicCalc(tmp,&ret))
			m_ADInfo.lADWidth=(int)ret;
	}
	tmp=THSimpleXml::GetParam(set,"icon","[","]");
	if (!tmp.IsEmpty())
		m_ADInfo.nIcon=THStringConv::s2i(tmp);
	tmp=THSimpleXml::GetParam(set,"timeout","[","]");
	if (!tmp.IsEmpty())
		m_ADInfo.nTimeOut=THStringConv::s2i(tmp);
	tmp=THSimpleXml::GetParam(set,"url","[","]");
	if (!tmp.IsEmpty())
	{
		strcpy(m_ADInfo.szUrl,tmp);
		//add query settings
		if (strstr(m_ADInfo.szUrl,"?")==0)
			strcat(m_ADInfo.szUrl,"?");
		else
			strcat(m_ADInfo.szUrl,"&");
		char tmp[30];
		sprintf(tmp,"querytime=%d",time(NULL));
		strcat(m_ADInfo.szUrl,tmp);
	}
	tmp=THSimpleXml::GetParam(set,"defer","[","]");
	if (!tmp.IsEmpty())
		m_ADInfo.uDefer=THStringConv::s2i(tmp);
	//m_ADInfo.x 和 m_ADInfo.y 待OnShowAD时才计算
	strcpy(m_ADInfo.posx,THSimpleXml::GetParam(set,"posx","[","]"));
	strcpy(m_ADInfo.posy,THSimpleXml::GetParam(set,"posy","[","]"));
	tmp=THSimpleXml::GetParam(set,"musthtmltag","[","]");
	if (!tmp.IsEmpty())
		m_ADInfo.bMustHaveHtmlTag=THStringConv::s2i(tmp);
	tmp=THSimpleXml::GetParam(set,"cbfn","[","]");
	if (!tmp.IsEmpty())
	{
		CString addr;
#define SETPOINTER(a)	addr=THSimpleXml::GetParam(tmp,"fn"#a,"(",")");if (addr) m_ADInfo.cb##a=(fn##a)(INT_PTR)THStringConv::s2u(addr)
		SETPOINTER(NavUrl);
		SETPOINTER(NavCmd);
		SETPOINTER(NavErr);
		SETPOINTER(NavNewWindow);
		SETPOINTER(WindowClose);
		SETPOINTER(WindowStart);
		SETPOINTER(SetTitle);
		SETPOINTER(WindowOpen);
		SETPOINTER(AddData);
	}
	return TRUE;
}

BOOL CADMainWin::SetInterface()
{
	if(QueryControl(IID_IWebBrowser2,(void**)&m_spWebBrowser2)!=S_OK) goto ERR;
	if(CBrowserEvents::Advise(m_spWebBrowser2)!=S_OK) goto ERR;
	m_spWebBrowser2->put_RegisterAsDropTarget(VARIANT_FALSE);
	m_spWebBrowser2->put_Silent(TRUE);
	return TRUE;
ERR:
	DestroyWindow();
	return FALSE;
}
VOID CADMainWin::SetWinSizeFromADSize(LONG *lHeight , LONG *lWidth)
{
	if((*lHeight == 0) || (*lWidth == 0))
	{
		GetHTMLSize(lWidth , lHeight);
	}
	m_lWinWidth = (*lWidth) + m_lNCWidth;
	m_lWinHeight = (*lHeight) + m_lNCHeight;
}

BOOL CADMainWin::SetWinStyle()
{
	LONG_PTR dwStyle,dwStyleEx;
	HICON hIcon , hIconSmall;
	TCHAR szIconFile[MAX_PATH] = {'\0'};
	if(m_ADInfo.uDlgType)
	{
		dwStyleEx = 0;
		dwStyle =  WS_SYSMENU | WS_OVERLAPPED ;

		if(m_ADInfo.uDlgType & DLGTYPE_TITLE || m_ADInfo.uDlgType & DLGTYPE_MIN || m_ADInfo.uDlgType & DLGTYPE_MAX)
		{
			dwStyle = dwStyle |  WS_CAPTION ;
			if(m_ADInfo.uDlgType & DLGTYPE_MIN )
				dwStyle = dwStyle | WS_MINIMIZEBOX ;
			if(m_ADInfo.uDlgType & DLGTYPE_MAX)
				dwStyle = dwStyle | WS_MAXIMIZEBOX ;
			if(m_ADInfo.bIsTaskBT)
				m_lNCHeight += ::GetSystemMetrics(SM_CYCAPTION);
			else
				m_lNCHeight += ::GetSystemMetrics(SM_CYSMCAPTION);
			m_lNCWidth += (::GetSystemMetrics(SM_CXEDGE) + ::GetSystemMetrics(SM_CXBORDER))*2;
			m_lNCHeight += (::GetSystemMetrics(SM_CYEDGE) + ::GetSystemMetrics(SM_CYBORDER))*2;
		}
		if(m_ADInfo.uDlgType & DLGTYPE_RESIZE)
		{
			dwStyle = dwStyle | WS_THICKFRAME ;
			if(!(m_ADInfo.uDlgType & DLGTYPE_TITLE || m_ADInfo.uDlgType & DLGTYPE_MIN || m_ADInfo.uDlgType & DLGTYPE_MAX))
			{
				m_lNCWidth += ::GetSystemMetrics(SM_CXDLGFRAME) *2;
				m_lNCHeight += ::GetSystemMetrics(SM_CYDLGFRAME) *2;
			}
			else
			{
				m_lNCWidth += ::GetSystemMetrics(SM_CXBORDER) *2;
				m_lNCHeight += ::GetSystemMetrics(SM_CYBORDER) *2;
			}
		}
		if(m_ADInfo.uDlgType & DLGTYPE_BORDER)
		{
			dwStyle = dwStyle | WS_BORDER;
			dwStyleEx = dwStyleEx | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE ;
			if(!(m_ADInfo.uDlgType & DLGTYPE_TITLE || m_ADInfo.uDlgType & DLGTYPE_MIN || m_ADInfo.uDlgType & DLGTYPE_MAX))
			{
				m_lNCWidth += (::GetSystemMetrics(SM_CXEDGE) + ::GetSystemMetrics(SM_CXBORDER))*2;
				m_lNCHeight += (::GetSystemMetrics(SM_CYEDGE) + ::GetSystemMetrics(SM_CYBORDER))*2;
			}
			else
			{
				m_lNCWidth += ::GetSystemMetrics(SM_CXEDGE)*2;
				m_lNCHeight += ::GetSystemMetrics(SM_CYEDGE)*2;
			}
		}
		SetWindowLongPtr(GWL_STYLE,dwStyle);
		SetWindowLongPtr(GWL_EXSTYLE,dwStyleEx);
	}
	if(!m_ADInfo.bIsTaskBT)
	{
		//FixHere
		ModifyStyleEx(WS_EX_APPWINDOW,WS_EX_TOOLWINDOW); 
		//if(m_spTaskBar.CoCreateInstance(CLSID_TaskbarList) == S_OK)
		//{
		//	m_spTaskBar->DeleteTab(m_hWnd);
		//}
	}
	if(!(m_ADInfo.uDlgType & DLGTYPE_TITLE) && !m_ADInfo.uDlgType)
		return TRUE;
	if(!m_ADInfo.nIcon)
	{
		hIcon = (HICON)::LoadImage((HINSTANCE)g_hModule, MAKEINTRESOURCE(IDR_MAINFRAME), 
			IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
		hIconSmall = (HICON)::LoadImage((HINSTANCE)g_hModule, MAKEINTRESOURCE(IDR_MAINFRAME), 
			IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	}
	else
	{		
		_tcsncpy(szIconFile,m_ADInfo.szUrl,MAX_PATH - 1);
		::PathRemoveFileSpec(szIconFile);
		_tcsncat(szIconFile,_T("\\icon.ico"),MAX_PATH - _tcslen(szIconFile) - 1);
		hIcon = (HICON)::LoadImage(NULL, szIconFile,IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_LOADFROMFILE);
		hIconSmall = (HICON)::LoadImage(NULL, szIconFile, IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_LOADFROMFILE);
	}
	SetIcon(hIcon, TRUE);
	SetIcon(hIconSmall, FALSE);
	return TRUE;
}
BOOL CADMainWin::OnShowAD(LPADPARAM pADWinInfo)
{
	if(!SetWinStyle())
		return FALSE;
	if(pADWinInfo->DispType == DISPLAY_TYPE_HIDE)
	{
		if(pADWinInfo->CloseType ==DISPLAY_TYPE_HIDE)
		{
			ShowWindow(SW_MINIMIZE);
			SetForegroundWindow(m_hWnd);
		}
		SetCount(TRUE);
		return TRUE;
	}
	SetWinSizeFromADSize(&pADWinInfo->lADHeight,&pADWinInfo->lADWidth);
	//calc x and y
	double ret;
	THString tmp=pADWinInfo->posx;
	if (!tmp.IsEmpty())
	{
		if (SetScreenValue(&tmp) && THLogicCalc::LogicCalc(tmp,&ret))
			pADWinInfo->x=(int)ret;
	}
	tmp=pADWinInfo->posy;
	if (!tmp.IsEmpty())
	{
		if (SetScreenValue(&tmp) && THLogicCalc::LogicCalc(tmp,&ret))
			pADWinInfo->y=(int)ret;
	}
	SetADWinRect();
	ShowAD();
	return TRUE;
}
VOID CADMainWin::SetCount(BOOL bCount)
{
	KillTimer(TIMER_PTR_SGLIDE);
	if(bCount)
	{
		if(m_ADInfo.nTimeOut > 0)
			SetTimer(TIMER_PTR_COUNT , COUNTELAPSE );
		else if(m_ADInfo.nTimeOut == 0)
		{
			CloseAD();
		}
	}
	else
	{
		m_uCountTime -= 3;
		KillTimer(TIMER_PTR_COUNT);
	}
}
VOID CADMainWin::GetHTMLSize(LONG* lWidth,LONG* lHeight)
{
	CComPtr<IDispatch> spDisp;
	CComQIPtr<IHTMLDocument2> spDoc2;
	CComPtr<IHTMLElement> spBodyElement;
	CComPtr<IHTMLElement2> spBodyElement2;
	if(m_spWebBrowser2->get_Document(&spDisp) != S_OK)
		return;
	if(spDisp->QueryInterface(IID_IHTMLDocument2,(void**)&spDoc2) != S_OK)
		return;
	if(spDoc2->get_body(&spBodyElement) != S_OK)
		return ;
	if(spBodyElement->QueryInterface(IID_IHTMLElement2,(void**)&spBodyElement2) != S_OK)
		return ;
	if(!(*lWidth)) spBodyElement2->get_scrollWidth(lWidth);
	if(!(*lHeight)) spBodyElement2->get_scrollHeight(lHeight);
}

BOOL CADMainWin::Popup(BOOL bShow)
{
	if(!bShow)
	{
		SetCount(FALSE);
		return DestroyWindow();
	}
	if(m_ADInfo.hWndInsertAfter != HWND_BOTTOM)
	{		
		ShowWindow(SW_MINIMIZE);
		SetForegroundWindow(m_hWnd);
	}
	SetWindowPos(m_ADInfo.hWndInsertAfter,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
	SetCount(TRUE);
	//FixHere
	if(m_ADInfo.hWndInsertAfter != HWND_BOTTOM)
	{
		ShowWindow(SW_NORMAL);
		BringWindowToTop();
	}
//	FlashWindow(TRUE);
	return TRUE;
}
BOOL CADMainWin::ShowAD()
{
	switch(m_ADInfo.DispType)
	{
	case DISPLAY_TYPE_POPUP:
		return Popup(TRUE);
	case DISPLAY_TYPE_D2U:
		SetStat(ADWS_GLIDING_UP);
		SetTimer(TIMER_PTR_SGLIDE,GLIDEELAPSE);
		return TRUE;
	case DISPLAY_TYPE_U2D:
		SetStat(ADWS_GLIDING_DOWN);
		SetTimer(TIMER_PTR_SGLIDE,GLIDEELAPSE);
		return TRUE;
	case DISPLAY_TYPE_L2R:
		SetStat(ADWS_GLIDING_RIGHT);
		SetTimer(TIMER_PTR_SGLIDE,GLIDEELAPSE);
		return TRUE;
	case DISPLAY_TYPE_R2L:
		SetStat(ADWS_GLIDING_LEFT);
		SetTimer(TIMER_PTR_SGLIDE,GLIDEELAPSE);
		return TRUE;
	}
	return FALSE;
}

VOID CADMainWin::GlideNotify()
{
	RECT rt;
	GetWindowRect(&rt);
	switch(GetStat())
	{
	case ADWS_GLIDING_UP:
		rt.top -= ADGLIDESTEP;
		if(m_bOPShow)
		{
			if( rt.bottom - rt.top  < m_lWinHeight)
			{
				MoveWindow(&rt);
				return;
			}
			else
			{
				SetWindowPos(m_ADInfo.hWndInsertAfter,m_ADInfo.x,m_ADInfo.y,m_lWinWidth,m_lWinHeight,SWP_NOACTIVATE);
				SetCount(TRUE);
				return ;
			}
		}
		else
		{
			if(rt.bottom > 0)
			{
				rt.bottom = rt.bottom - ADGLIDESTEP;
				MoveWindow(&rt);
				return;
			}
			else
			{
				KillTimer(TIMER_PTR_SGLIDE);
				DestroyWindow();
				return;
			}
		}
		break;
	case ADWS_GLIDING_DOWN:	
		if(m_bOPShow)
		{
			rt.bottom += ADGLIDESTEP;
			if( rt.bottom - rt.top  < m_lWinHeight )
			{			

				MoveWindow(&rt);
				return;
			}
			else
			{
				SetWindowPos(m_ADInfo.hWndInsertAfter,m_ADInfo.x,m_ADInfo.y,m_lWinWidth,m_lWinHeight,SWP_NOACTIVATE);
				SetCount(TRUE);
				return;
			}
		}
		else
		{
			rt.top += ADGLIDESTEP;
			if( rt.top + m_lNCHeight < rt.bottom)
			{
				MoveWindow(&rt);
				return;
			}
			else
			{
				KillTimer(TIMER_PTR_SGLIDE);
				DestroyWindow();
				return;
			}
		}
		break;
	case ADWS_GLIDING_LEFT:
		rt.left -= ADGLIDESTEP;
		if(m_bOPShow)
		{
			if( rt.right - rt.left  < m_lWinWidth )
			{
				MoveWindow(&rt);
				return;
			}
			else
			{
				SetWindowPos(m_ADInfo.hWndInsertAfter,m_ADInfo.x,m_ADInfo.y,m_lWinWidth,m_lWinHeight,SWP_NOACTIVATE);
				SetCount(TRUE);
				return;
			}
		}
		else
		{
			if( rt.right > 0 )
			{
				rt.right -= ADGLIDESTEP;
				MoveWindow(&rt);
				return;
			}
			else
			{
				KillTimer(TIMER_PTR_SGLIDE);
				DestroyWindow();
				return;
			}
		}
		break;
	case ADWS_GLIDING_RIGHT:
		rt.right += ADGLIDESTEP;
		if(m_bOPShow)
		{
			if( rt.right - rt.left  < m_lWinWidth )
			{
				MoveWindow(&rt);
				return;
			}
			else
			{
				SetWindowPos(m_ADInfo.hWndInsertAfter,m_ADInfo.x,m_ADInfo.y,m_lWinWidth,m_lWinHeight,SWP_NOACTIVATE);
				SetCount(TRUE);
				return;
			}
		}
		else
		{
			if(rt.left + m_lNCWidth < rt.right)// rt.left < m_lScreenRight
			{
				rt.left += ADGLIDESTEP;
				MoveWindow(&rt);
				return;
			}
			else
			{
				KillTimer(TIMER_PTR_SGLIDE);
				DestroyWindow();
				return;
			}
		}
		break;
	}
}
BOOL CADMainWin::CloseAD()
{
	SetCount(FALSE);
	switch(m_ADInfo.CloseType)
	{
	case DISPLAY_TYPE_HIDE:
	case DISPLAY_TYPE_POPUP:
		return Popup(FALSE);
	case DISPLAY_TYPE_U2D:
		SetStat(ADWS_GLIDING_DOWN,FALSE);
		SetTimer(TIMER_PTR_SGLIDE,GLIDEELAPSE);
		return TRUE;
	case DISPLAY_TYPE_D2U:
		SetStat(ADWS_GLIDING_UP,FALSE);
		SetTimer(TIMER_PTR_SGLIDE,GLIDEELAPSE);
		return TRUE;
	case DISPLAY_TYPE_L2R:
		SetStat(ADWS_GLIDING_RIGHT,FALSE);
		SetTimer(TIMER_PTR_SGLIDE,GLIDEELAPSE);
		return TRUE;
	case DISPLAY_TYPE_R2L:
		SetStat(ADWS_GLIDING_LEFT,FALSE);
		SetTimer(TIMER_PTR_SGLIDE,GLIDEELAPSE);
		return TRUE;
	}
	return FALSE;
}
BOOL CADMainWin::SetADWinRect()
{
	RECT rt;
	rt.left=m_ADInfo.x;
	rt.top=m_ADInfo.y;
	rt.right=m_ADInfo.x+m_lWinWidth;
	rt.bottom=m_ADInfo.y+m_lWinHeight;
	switch(m_ADInfo.DispType)
	{
	case DISPLAY_TYPE_D2U:
		rt.top = rt.bottom - m_lNCHeight;
		break;
	case DISPLAY_TYPE_U2D:
		rt.bottom = rt.top + m_lNCHeight;
		break;
	case DISPLAY_TYPE_L2R:
		rt.right = rt.left + m_lNCWidth;
		break;
	case DISPLAY_TYPE_R2L:
		rt.left = rt.right - m_lNCWidth;
		break;
	case DISPLAY_TYPE_POPUP:
		break;
	case DISPLAY_TYPE_HIDE:
	default:
		return FALSE;
	}
//	MoveWindow(&rt,FALSE);
	SetWindowPos(m_ADInfo.hWndInsertAfter,&rt,SWP_NOACTIVATE);
	ShowWindow(SW_SHOW);
	return TRUE;
}

LRESULT CADMainWin::OnCopyData(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// TODO: Add your message handler code here and/or call default
	PCOPYDATASTRUCT pCopyData = (PCOPYDATASTRUCT)lParam;
	switch(pCopyData->dwData)
	{
	case COPYDATA_TYPE_NAVIGATE:
		{
			LPCSTR tmp = (LPCSTR)pCopyData->lpData;
			CComVariant VarURL(tmp);
			m_bShowFlag=FALSE;
			m_bError=FALSE;
			if(m_spWebBrowser2 && m_spWebBrowser2->Navigate2(&VarURL,NULL,NULL,NULL,NULL)==S_OK)
				return TRUE;
		}
		break;
	}
	return FALSE;
}
