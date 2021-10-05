#pragma once
#include "atlbase.h"
#include "atlwin.h"
#include "exdispid.h"
#include "resource.h"
#include <THSystem.h>
#include <THString.h>
#include <THLogicCalc.h>

#define WEBBROWSEREVENTID 0

#define ADGLIDESTEP	10
#define GLIDEELAPSE 100
#define COUNTELAPSE 1000

enum ADWINSTAT
{
	ADWS_GLIDING_UP,
	ADWS_GLIDING_DOWN,
	ADWS_GLIDING_LEFT,
	ADWS_GLIDING_RIGHT,
	ADWS_SHOW,
	ADWS_HIDE
};
enum TIMER_PTR
{
	TIMER_PTR_COUNT = 1,
	TIMER_PTR_SGLIDE
};
VOID funShowAD(LPARAM lParam);
class CADMainWin;
typedef IDispEventImpl<WEBBROWSEREVENTID, CADMainWin , &DIID_DWebBrowserEvents2> CBrowserEvents;
enum DISPLAY_TYPE
{
	DISPLAY_TYPE_HIDE = 0,	//0:����
	DISPLAY_TYPE_POPUP = 1, //1:ֱ�ӵ���
	DISPLAY_TYPE_U2D,		//2:�� -> ��
	DISPLAY_TYPE_D2U,		//3:�� -> �� 
	DISPLAY_TYPE_L2R,		//4:�� -> ��
	DISPLAY_TYPE_R2L		//5:�� -> ��
};
enum DLGTYPE
{
	DLGTYPE_TITLE = 0x1,
	DLGTYPE_BORDER = 0x2,
	DLGTYPE_RESIZE = 0x4,
	DLGTYPE_MIN = 0x8,
	DLGTYPE_MAX = 0x10
};

#define ADURLMAXSIZE	4096
#define COPYDATA_TYPE_NAVIGATE	1

//����bCancel
typedef BOOL (*fnNavUrl)(const char *url,void *adddata);
//����bCancel
typedef BOOL (*fnNavCmd)(const char *cmd,void *adddata);
typedef void (*fnNavErr)(const char *url,DWORD dwError,void *adddata);
typedef void (*fnNavNewWindow)(const char *url,void *adddata);
typedef void (*fnWindowClose)(const char *url,void *adddata);
typedef void (*fnWindowStart)(const char *url,void *adddata);
typedef void (*fnSetTitle)(const char *title,void *adddata);
//����bCancel
typedef BOOL (*fnWindowOpen)(const char *url,void *adddata);
typedef void* fnAddData;

typedef struct tagADParam		//������
{
	USHORT uDlgType;				//�Ƿ��б�����
	BOOL bIsTaskBT;				//�Ƿ�����������ť
	INT	 nIcon;					//ICON 0:ϵͳĬ��(IE),1:�����ļ��е�icon.ico
	TCHAR szUrl[ADURLMAXSIZE];	//AD URL
	TCHAR posx[100];			//������㹫ʽ����Ϊw��h��Ҫȷ���˲��ܼ���
	TCHAR posy[100];			//������㹫ʽ����Ϊw��h��Ҫȷ���˲��ܼ���
	LONG lADWidth;				//AD ��� ,Ϊ0ʱΪҳ����
	LONG lADHeight;				//AD �߶� ,Ϊ0ʱΪҳ��߶�
	LONG x;						//AD X����
	LONG y;						//AD Y����
	INT nTimeOut;				//AD ��ʾ��ʱ��,Ϊ0ʱ��ʾ�������ر�,-1�ǲ��ر�
	HWND hWndInsertAfter;		//AD ��ʾ������ SetWindowPos����
//	AD_TYPE Type;				//AD ������
	DISPLAY_TYPE DispType;		//AD ��ʾ����0:���ط�ʽ 1:ֱ�ӵ��� 2:�� -> �� 3:�� -> �� 4:�� -> �� 5:�� -> �� 
	DISPLAY_TYPE CloseType;		//AD �ر�����1:ֱ�ӵ��� 2:�� -> �� 3:�� -> �� 4:�� -> �� 5:�� -> �� 
	UINT uDefer;				//AD ��ʱ����
	BOOL bMustHaveHtmlTag;		//AD html���Ƿ�����ж�ӦHtmlTag
//	TCHAR szADID[ADIDMAXSIZE];		//AD ID 
	fnNavUrl cbNavUrl;
	fnNavCmd cbNavCmd;
	fnNavErr cbNavErr;
	fnNavNewWindow cbNavNewWindow;
	fnWindowClose cbWindowClose;
	fnWindowStart cbWindowStart;
	fnSetTitle cbSetTitle;
	fnWindowOpen cbWindowOpen;
	void *cbAddData;
}ADPARAM,*LPADPARAM;
/*
LPTSTR ParseFileName(LPCTSTR szFile)
{
	if(!_tcsstr(szFile,_T("\\")))
		return NULL;
	LPTSTR lpFileName = (LPTSTR)(szFile + _tcslen(szFile));
	while(*(--lpFileName) != _T('\\'));
	lpFileName++;
	return lpFileName;
}
LPTSTR ParseFileExtraName(LPCTSTR szFileName)
{
	UINT uTitleLen = (UINT)_tcslen(szFileName);
	while(uTitleLen > 0 && szFileName[--uTitleLen] != _T('.') );
	if( uTitleLen > 0)
	{
		return (LPTSTR)&(szFileName[uTitleLen]);
	}
	return NULL;
}
LPTSTR GetADID(LPCTSTR szURL , LPTSTR szADID,DWORD dwSize)
{
	LPTSTR lpNewTem = new TCHAR[_tcslen(szURL) + 1];
	_tcscpy(lpNewTem,szURL);
	LPTSTR lpOffset = ParseFileName(lpNewTem);
	if(!lpOffset)
		goto END;
	lpOffset--;
	*lpOffset = NULL;
	lpOffset = ParseFileName(lpNewTem);
	if(!lpOffset)
		goto END;
	_tcsncpy(szADID,lpOffset,dwSize - 1);
	lpOffset = szADID;
END:
	delete [] lpNewTem;
	return lpOffset;
}
*/
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
		MESSAGE_HANDLER(WM_COPYDATA, OnCopyData)
	END_MSG_MAP()

	BEGIN_SINK_MAP(CADMainWin)
		SINK_ENTRY_EX(WEBBROWSEREVENTID, DIID_DWebBrowserEvents2, DISPID_NAVIGATECOMPLETE2, OnNavigateComplete2)
		SINK_ENTRY_EX(WEBBROWSEREVENTID, DIID_DWebBrowserEvents2, DISPID_BEFORENAVIGATE2, OnBeforeNavigate2)
		SINK_ENTRY_EX(WEBBROWSEREVENTID, DIID_DWebBrowserEvents2, DISPID_NAVIGATEERROR, OnNavigateError)
		SINK_ENTRY_EX(WEBBROWSEREVENTID, DIID_DWebBrowserEvents2, DISPID_DOCUMENTCOMPLETE, OnDocumentComplete)
		SINK_ENTRY_EX(WEBBROWSEREVENTID, DIID_DWebBrowserEvents2, DISPID_NEWWINDOW2, OnNewWindow2)
	END_SINK_MAP()
private:
	CComPtr<IWebBrowser2> m_spWebBrowser2;
	BOOL m_bError;
	BOOL m_bShowFlag;
	ADPARAM m_ADInfo;
	ADWINSTAT m_WinStat;
	BOOL m_bOPShow;
	UINT m_uCountTime;
	LONG m_lWinWidth;
	LONG m_lWinHeight;
	LONG m_lNCWidth;
	LONG m_lNCHeight;
	//LONG m_lScreenBottom;
	//LONG m_lScreenRight;

public:
	void __stdcall OnNavigateComplete2(IDispatch *pDisp,VARIANT *URL );
	void __stdcall OnBeforeNavigate2(IDispatch *pDisp,
		VARIANT *url,
		VARIANT *Flags,
		VARIANT *TargetFrameName,
		VARIANT *PostData,
		VARIANT *Headers,
		VARIANT_BOOL *Cancel
		);
	void __stdcall OnNavigateError(IDispatch *pDisp,
		VARIANT *URL,
		VARIANT *TargetFrameName,
		VARIANT *StatusCode,
		VARIANT_BOOL *Cancel
		);
	void __stdcall OnDocumentComplete(          IDispatch *pDisp,
		VARIANT *URL
		);
	void __stdcall OnNewWindow2(          IDispatch **ppDisp,
		VARIANT_BOOL *Cancel
		);
public:
	LRESULT OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	BOOL SetADInfo(const char *settings);
	BOOL SetScreenValue(THString *val);
	BOOL SetInterface();
	BOOL OnShowAD(LPADPARAM pADWinInfo);
	BOOL SetWinStyle();
	void OnStart();

private:
	VOID SetStat(ADWINSTAT stat , BOOL bOPShow = TRUE)
	{
		m_WinStat = stat;
		m_bOPShow = bOPShow;
	}
	ADWINSTAT GetStat()
	{
		return m_WinStat;
	}
	VOID SetWinSizeFromADSize(LONG *lHeight , LONG *lWidth);
	VOID GetHTMLSize(LONG* lWidth,LONG* lHeight);
	BOOL ShowAD();
	BOOL Popup(BOOL bShow);
	BOOL CloseAD();
	VOID GlideNotify();
	VOID SetCount(BOOL bCount);
	BOOL SetADWinRect();
	BOOL ParseCCMD(LPTSTR szUrl ,VARIANT_BOOL *pbCancel);
public:
	LRESULT OnCopyData(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};
