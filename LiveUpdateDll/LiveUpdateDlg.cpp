// LiveUpdateDlg.cpp : implementation file
//

#include "stdafx.h"
#include "LiveUpdateDll.h"
#include "LiveUpdateDlg.h"
#include ".\liveupdatedlg.h"


// CLiveUpdateDlg dialog

IMPLEMENT_DYNAMIC(CLiveUpdateDlg, CDialog)
CLiveUpdateDlg::CLiveUpdateDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLiveUpdateDlg::IDD, pParent)
{
	m_update.SetHandler(this);
	m_nReTryMin=0;
	m_nFailedReTryMin=0;
	m_nFailReTry=0;
	m_nState=UPDATEDLGSTATE_WELCOME;
	m_nUpdateCount=0;
}

CLiveUpdateDlg::~CLiveUpdateDlg()
{
}

void CLiveUpdateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CLiveUpdateDlg, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()


// CLiveUpdateDlg message handlers

BOOL CLiveUpdateDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	CRect rect;
	GetDlgItem(IDC_STATICSUBDLG)->GetWindowRect(rect);
	ScreenToClient(&rect);
	m_welcome.Create(IDD_SUBDIALOG1,this);
	m_welcome.MoveWindow(rect);
	m_searching.Create(IDD_SUBDIALOG2,this);
	m_searching.MoveWindow(rect);
	m_searched.Create(IDD_SUBDIALOG3,this);
	m_searched.MoveWindow(rect);
	m_downloading.Create(IDD_SUBDIALOG4,this);
	m_downloading.MoveWindow(rect);
	m_finish.Create(IDD_SUBDIALOG5,this);
	m_finish.MoveWindow(rect);

	UpdateNowState();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CLiveUpdateDlg::OnStartLiveUpdate(THLiveUpdate *pUpdate)
{
	return FALSE;
}

void CLiveUpdateDlg::OnFinishLiveUpdate(THLiveUpdate *pUpdate,int nReason)
{
}

void CLiveUpdateDlg::OnProcessStateMessage(THLiveUpdate *pUpdate,THString sMsg)
{
	if (m_nState==UPDATEDLGSTATE_SEARCHING)
	{
		m_searching.GetDlgItem(IDC_STATICPROCESS)->SetWindowText(sMsg);
	}
}

void CLiveUpdateDlg::OnProcessPercent(THLiveUpdate *pUpdate,UINT nPosAll,UINT nPosCurrent,UINT nTotal,UINT nFinished,UINT nSpeedPreSecond)
{
	if (m_nState==UPDATEDLGSTATE_SEARCHING)
	{
		//m_searching.m_process.SetPos
	}
}

BOOL CLiveUpdateDlg::OnFoundedUpdate(THLiveUpdate *pUpdate,int nMax)
{
	m_nState=UPDATEDLGSTATE_AVAUPDATE;
	m_nUpdateCount=nMax;
	UpdateNowState();
	m_event.Create(NULL,TRUE,FALSE,NULL);
	m_event.WaitForObject();
	return FALSE;
}

BOOL CLiveUpdateDlg::OnStartInstall(THLiveUpdate *pUpdate)
{
	return FALSE;
}

void CLiveUpdateDlg::UpdateNowState()
{
	if (m_nState==UPDATEDLGSTATE_WELCOME)
	{
		if (m_nType & UPDATEDLGTYPE_PAGEWELCOME)
		{
			m_searching.ShowWindow(SW_HIDE);
			m_searched.ShowWindow(SW_HIDE);
			m_downloading.ShowWindow(SW_HIDE);
			m_finish.ShowWindow(SW_HIDE);
			m_welcome.ShowWindow(SW_SHOW);
			GetDlgItem(IDOK)->SetWindowText(_T("下一步(&N)"));
			GetDlgItem(IDOK)->ShowWindow(SW_SHOW);
		}
		else
			//不显示欢迎页面，直接进入下一步
			OnBnClickedOk();
	}
	else if (m_nState==UPDATEDLGSTATE_SEARCHING)
	{
		m_welcome.ShowWindow(SW_HIDE);
		m_searched.ShowWindow(SW_HIDE);
		m_downloading.ShowWindow(SW_HIDE);
		m_finish.ShowWindow(SW_HIDE);
		m_searching.ShowWindow(SW_SHOW);
		GetDlgItem(IDOK)->ShowWindow(SW_HIDE);
		m_update.InitLiveUpdate(m_sPath,m_sTitle,m_sUrl,m_nReTryMin,m_nFailedReTryMin,m_nFailReTry);
	}
	else if (m_nState==UPDATEDLGSTATE_AVAUPDATE)
	{
		m_welcome.ShowWindow(SW_HIDE);
		m_searching.ShowWindow(SW_HIDE);
		m_downloading.ShowWindow(SW_HIDE);
		m_finish.ShowWindow(SW_HIDE);
		m_searched.ShowWindow(SW_SHOW);
		for(int i=0;i<m_nUpdateCount;i++)
			m_searched.m_TreeCtrl.InsertItem(m_update.GetUpdateInfo(i,_T("desc")));
		GetDlgItem(IDOK)->SetWindowText(_T("下一步(&N)"));
		GetDlgItem(IDOK)->ShowWindow(SW_SHOW);
	}
	else if (m_nState==UPDATEDLGSTATE_DOWNLOADING)
	{
		m_welcome.ShowWindow(SW_HIDE);
		m_searching.ShowWindow(SW_HIDE);
		m_searched.ShowWindow(SW_HIDE);
		m_finish.ShowWindow(SW_HIDE);
		m_downloading.ShowWindow(SW_SHOW);
	}
	else if (m_nState==UPDATEDLGSTATE_FINISH)
	{
	}
}

void CLiveUpdateDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	if (m_nState==UPDATEDLGSTATE_WELCOME)
	{
		m_nState=UPDATEDLGSTATE_SEARCHING;
		UpdateNowState();
	}
	else if (m_nState==UPDATEDLGSTATE_SEARCHING)
	{
	}
	else if (m_nState==UPDATEDLGSTATE_AVAUPDATE)
	{
	}
	else if (m_nState==UPDATEDLGSTATE_DOWNLOADING)
	{
	}
	else if (m_nState==UPDATEDLGSTATE_FINISH)
	{
	}
}

void CLiveUpdateDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	if (MessageBox(_T("是否取消当前的更新操作？"),_T("取消确认"),MB_ICONQUESTION|MB_YESNO)==IDYES)
		OnCancel();
}
