// THMagneticDlg.cpp : implementation file
//

#include "stdafx.h"
#include "THFlashDlg.h"
#include <THDebug.h>
#include ".\thflashdlg.h"

// THMagneticDlg dialog

IMPLEMENT_DYNAMIC(THFlashDlg, CDialog)
THFlashDlg::THFlashDlg(UINT nIDTemplate, CWnd* pParentWnd)
	: CDialog(nIDTemplate, pParentWnd)
{
}

THFlashDlg::~THFlashDlg()
{
}

void THFlashDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_THFLASH1, m_flash);
}


BEGIN_MESSAGE_MAP(THFlashDlg, CDialog)
	ON_WM_CREATE()
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(THFlashDlg, CDialog)
	ON_EVENT(THFlashDlg, IDC_THFLASH1, 197, FlashCallShockwaveflash, VTS_BSTR)
	ON_EVENT(THFlashDlg, IDC_THFLASH1, 150, FSCommandShockwaveflash, VTS_BSTR VTS_BSTR)
	ON_EVENT(THFlashDlg, IDC_THFLASH1, 1958, OnProgressShockwaveflash, VTS_I4)
	ON_EVENT(THFlashDlg, IDC_THFLASH1, DISPID_READYSTATECHANGE, OnReadyStateChangeShockwaveflash, VTS_I4)
END_EVENTSINK_MAP()
// THFlashDlg message handlers

int THFlashDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rt;
	rt.left=rt.top=0;
	rt.right=lpCreateStruct->cx-lpCreateStruct->x;
	rt.bottom=lpCreateStruct->cy-lpCreateStruct->y;
	m_flash.Create(_T("flash"),WS_CHILD|WS_VISIBLE,rt,this,IDC_THFLASH1);
	return 0;
}

void THFlashDlg::FlashCallShockwaveflash(LPCTSTR request)
{
	THString str=request;
	while(1)
	{
		int cmdpos=str.Find(_T("<invoke"),0);
		if (cmdpos==-1) break;
		int cmdendpos=str.Find(_T("</invoke>"),cmdpos);
		if (cmdendpos==-1) break;
		THString cmd=str.Left(cmdendpos+9).Mid(cmdpos+7);
		str=str.Mid(cmdendpos+9);
		THString arg=THSimpleXml::GetParam(cmd,_T("arguments"));
		arg=THSimpleXml::GetParam(arg,_T("string"));
		cmdpos=cmd.Find(_T("name="),0);
		if (cmdpos==-1) continue;
		cmdendpos=cmd.Find(_T(" "),cmdpos);
		cmd=cmd.Left(cmdendpos).Mid(cmdpos+5);
		cmd=cmd.Trim(_T(" \""));
		OnFlashCommand(cmd,arg);
	}
}

void THFlashDlg::FSCommandShockwaveflash(LPCTSTR command, LPCTSTR args)
{
	OnFlashCommand(command,args);
}

void THFlashDlg::OnProgressShockwaveflash(long percentDone)
{
	THDebug(_T("[Flash]:Progress:%d"),percentDone);
}

void THFlashDlg::OnReadyStateChangeShockwaveflash(long newState)
{
	THDebug(_T("[Flash]:ReadyStateChange:%d"),newState);
}

void THFlashDlg::OnFlashCommand(THString cmd,THString args)
{
	if (cmd==_T("debug"))
	{
		THDebug(_T("[Flash]:%s"),args);
	}
	else
	{
		THDebug(_T("[Flash]:Warning:Unhandle Flash cmd:%s,%s"),cmd,args);
	}
}

BOOL THFlashDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_flash.put_Movie(m_flashfile);
	m_flash.put_AllowScriptAccess(_T("always"));
	//m_flash.put_AllowFullScreen(_T("true"));
	//m_flash.SetVariable(_T("key"),_T("value"));
	m_flash.Play();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
