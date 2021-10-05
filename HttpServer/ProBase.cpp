// ProBase.cpp : implementation file
//

#include "stdafx.h"
#include "HttpServer.h"
#include "ProBase.h"
#include ".\probase.h"
#include <THReg.h>

// CProBase dialog

IMPLEMENT_DYNAMIC(CProBase, CPropertyPage)
CProBase::CProBase()
	: CPropertyPage(CProBase::IDD)
	, m_nListenPort(0)
	, m_nDynIpSid(0)
	, m_bAutoRun(FALSE)
	, m_bAutoStart(FALSE)
	, m_bStartHide(FALSE)
	, m_bHttpsService(FALSE)
{
}

CProBase::~CProBase()
{
}

void CProBase::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_nListenPort);
	DDX_Text(pDX, IDC_EDIT2, m_nDynIpSid);
	DDX_Check(pDX, IDC_CHECK1, m_bAutoRun);
	DDX_Check(pDX, IDC_CHECK2, m_bAutoStart);
	DDX_Check(pDX, IDC_CHECK3, m_bStartHide);
	DDX_Check(pDX, IDC_CHECK4, m_bHttpsService);
}


BEGIN_MESSAGE_MAP(CProBase, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON4, OnBnClickedButton4)
END_MESSAGE_MAP()


// CProBase message handlers

void CProBase::OnBnClickedButton4()
{
	if (MessageBox(_T("天塑 DynIp 服务是天塑在线提供的一种转接服务。\r\n通过DynIp接口，客户端通过指定的SID即可以随时连接上对应的动态服务器。\r\n需要打开DynIp首页了解详情吗？"),_T("关于 天塑 DynIp 服务"),MB_YESNO|MB_ICONQUESTION)==IDYES)
		ShellExecute(NULL,_T("open"),_T("http://dynip.tansoft.cn/"),NULL,NULL,SW_SHOW);
}

BOOL CProBase::OnSetActive()
{
	// TODO: Add your specialized code here and/or call the base class
	return CPropertyPage::OnSetActive();
}

BOOL CProBase::OnKillActive()
{
	// TODO: Add your specialized code here and/or call the base class
	UpdateData();
	if (m_nListenPort<=0 || m_nListenPort>65535)
	{
		MessageBox(_T("必须指定监听端口！"));
		return FALSE;
	}
	return CPropertyPage::OnKillActive();
}

BOOL CProBase::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// TODO:  Add extra initialization here
	THIniAutoRInt((&theApp.m_ini),_T("THHttpServerSettings"),nListenPort,80);
	THIniAutoRInt((&theApp.m_ini),_T("THHttpServerSettings"),nDynIpSid,0);
	THIniAutoRBool((&theApp.m_ini),_T("THHttpServerSettings"),bAutoRun,FALSE);
	THIniAutoRBool((&theApp.m_ini),_T("THHttpServerSettings"),bAutoStart,FALSE);
	THIniAutoRBool((&theApp.m_ini),_T("THHttpServerSettings"),bStartHide,FALSE);
	THIniAutoRBool((&theApp.m_ini),_T("THHttpServerSettings"),bHttpsService,FALSE);
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CProBase::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class
	UpdateData();
	THIniAutoWInt((&theApp.m_ini),_T("THHttpServerSettings"),nListenPort,80);
	THIniAutoWInt((&theApp.m_ini),_T("THHttpServerSettings"),nDynIpSid,0);
	THIniAutoWBool((&theApp.m_ini),_T("THHttpServerSettings"),bAutoRun,FALSE);
	THIniAutoWBool((&theApp.m_ini),_T("THHttpServerSettings"),bAutoStart,FALSE);
	THIniAutoWBool((&theApp.m_ini),_T("THHttpServerSettings"),bStartHide,FALSE);
	THIniAutoWBool((&theApp.m_ini),_T("THHttpServerSettings"),bHttpsService,FALSE);
	THAutoRunReg::SetAutoRun(_T("THHttpServer"),theApp.m_sExeFile+_T(" /start"),m_bAutoRun);
	CPropertyPage::OnOK();
}
