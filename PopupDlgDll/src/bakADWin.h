#pragma once
#include "atlbase.h"
#include "atlwin.h"
#include "exdispid.h"
#include "..\..\common\TypeDef.h"

#define WEBBROWSEREVENTID 0
typedef HRESULT (*ADWIN_CB)(DWORD CB_id, LPVOID lpData , DWORD dwErrCode);
class CADMainWin;
typedef IDispEventImpl<WEBBROWSEREVENTID, CADMainWin , &DIID_DWebBrowserEvents2> CBrowserEvents;

class CADMainWin :
	public CWindowImpl<CADMainWin,CAxWindow2>,
	public CBrowserEvents
{
public:
	CADMainWin(void);
	~CADMainWin(void);
public:
	DECLARE_WND_SUPERCLASS(NULL, CAxWindow2::GetWndClassName())
	BEGIN_MSG_MAP(CADMainWin)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
//		MESSAGE_HANDLER(WM_CREATE, OnCreate)
	END_MSG_MAP()

	BEGIN_SINK_MAP(CADMainWin)
		SINK_ENTRY_EX(WEBBROWSEREVENTID, DIID_DWebBrowserEvents2, DISPID_NAVIGATECOMPLETE2, OnNavigateComplete2)
		SINK_ENTRY_EX(WEBBROWSEREVENTID, DIID_DWebBrowserEvents2, DISPID_BEFORENAVIGATE2, OnBeforeNavigate2)
		SINK_ENTRY_EX(WEBBROWSEREVENTID, DIID_DWebBrowserEvents2, DISPID_NAVIGATEERROR, OnNavigateError)
	END_SINK_MAP()
private:
	ADWIN_CB m_funShowADCB;
	CComPtr<IWebBrowser2> m_spWebBrowser2;
	BOOL bError;
	ADPARAM m_ADInfo;
public:
	void __stdcall OnNavigateComplete2(IDispatch *pDisp,VARIANT *URL );
	void __stdcall OnBeforeNavigate2(IDispatch *pDisp,
		VARIANT *&url,
		VARIANT *&Flags,
		VARIANT *&TargetFrameName,
		VARIANT *&PostData,
		VARIANT *&Headers,
		VARIANT_BOOL *&Cancel
		);
	void __stdcall OnNavigateError(IDispatch *pDisp,
		VARIANT *URL,
		VARIANT *TargetFrameName,
		VARIANT *StatusCode,
		VARIANT_BOOL *&Cancel
		);

public:
	LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	BOOL SetADInfo(LPADPARAM pADInfo);

};
