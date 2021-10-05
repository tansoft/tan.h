// ProLog.cpp : implementation file
//

#include "stdafx.h"
#include "HttpServer.h"
#include "ProLog.h"
#include ".\prolog.h"
#include <THSysMisc.h>

// CProLog dialog

IMPLEMENT_DYNAMIC(CProLog, CPropertyPage)
CProLog::CProLog()
	: CPropertyPage(CProLog::IDD)
	, m_bRecordError(FALSE)
	, m_bRecordInfo(FALSE)
	, m_bRecordDetail(FALSE)
	, m_sRecordPath(_T(""))
	, m_nRecordType(0)
{
}

CProLog::~CProLog()
{
}

void CProLog::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK3, m_bRecordError);
	DDX_Check(pDX, IDC_CHECK4, m_bRecordInfo);
	DDX_Check(pDX, IDC_CHECK5, m_bRecordDetail);
	DDX_Text(pDX, IDC_EDIT1, m_sRecordPath);
	DDX_Radio(pDX, IDC_RADIO1, m_nRecordType);
}


BEGIN_MESSAGE_MAP(CProLog, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
END_MESSAGE_MAP()


// CProLog message handlers

void CProLog::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	UpdateData();
	THString str;
	str=_T("请选择日志保存位置");
	if (m_nRecordType!=0)
		str+=_T("，每天的日期会分别加到文件结尾");
	str=THSysDialog::FileDialogSaveOneOverWritePrompt(m_hWnd,_T("HttpServer.log"),_T("记录文件(*.log)|*.log|文本文件(*.txt)|*.txt|所有文件(*.*)|*.*||"),_T(""),_T(""),str);
	if (!str.IsEmpty())
	{
		m_sRecordPath=str;
		UpdateData(FALSE);
	}
}

BOOL CProLog::OnKillActive()
{
	// TODO: Add your specialized code here and/or call the base class
	UpdateData();
	return CPropertyPage::OnKillActive();
}

BOOL CProLog::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	int nMode=theApp.m_server.GetWebLogMode();
	if (nMode&LOG_ERROR)	m_bRecordError=TRUE;
	if (nMode&LOG_NORMAL) m_bRecordInfo=TRUE;
	if (nMode&LOG_DETAIL) m_bRecordDetail=TRUE;

	THIniAutoRString((&theApp.m_ini),_T("THHttpServerSettings"),sRecordPath,theApp.m_sExePath+_T("\\HttpServer.log"));
	THIniAutoRInt((&theApp.m_ini),_T("THHttpServerSettings"),nRecordType,0);
	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CProLog::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class
	UpdateData();
	int mode=0;
	if (m_bRecordError) mode|=LOG_ERROR;
	if (m_bRecordInfo) mode|=LOG_NORMAL;
	if (m_bRecordDetail) mode|=LOG_DETAIL;
	THIniAutoWString((&theApp.m_ini),_T("THHttpServerSettings"),sRecordPath,theApp.m_sExePath+_T("\\HttpServer.log"));
	THIniAutoWInt((&theApp.m_ini),_T("THHttpServerSettings"),nRecordType,0);
	if (m_nRecordType==0)
	{
		theApp.m_server.SetWebLogMode(m_sRecordPath,mode);
		theApp.StopAllSchedule();
	}
	else
	{
		theApp.m_server.SetWebLogMode(theApp.m_server.GetWebLogFile(),mode);
		//改变当天的文件名
		theApp.StartLogSchedule();
	}
	CPropertyPage::OnOK();
}
