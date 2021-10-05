// ProScript.cpp : implementation file
//

#include "stdafx.h"
#include "HttpServer.h"
#include "ProScript.h"
#include ".\proscript.h"
#include <THBaseDialog.h>

// CProScript dialog

IMPLEMENT_DYNAMIC(CProScript, CPropertyPage)
CProScript::CProScript()
	: CPropertyPage(CProScript::IDD)
	, m_bUseScript(FALSE)
	, m_bScriptLimitTime(FALSE)
	, m_nScriptLimitTime(0)
	, m_bUseSession(FALSE)
	, m_nSessionTimeout(0)
	, m_bSessionCookieLessMode(FALSE)
{
}

CProScript::~CProScript()
{
}

void CProScript::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK1, m_bUseScript);
	DDX_Check(pDX, IDC_CHECK2, m_bScriptLimitTime);
	DDX_Text(pDX, IDC_EDIT1, m_nScriptLimitTime);
	DDX_Check(pDX, IDC_CHECK3, m_bUseSession);
	DDX_Text(pDX, IDC_EDIT2, m_nSessionTimeout);
	DDX_Check(pDX, IDC_CHECK5, m_bSessionCookieLessMode);
	DDX_Control(pDX, IDC_LIST1, m_extlist);
}


BEGIN_MESSAGE_MAP(CProScript, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON2, OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, OnBnClickedButton4)
	ON_BN_CLICKED(IDC_CHECK1, OnBnClickedCheck1)
	ON_BN_CLICKED(IDC_CHECK3, OnBnClickedCheck3)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
END_MESSAGE_MAP()


// CProScript message handlers

BOOL CProScript::OnKillActive()
{
	// TODO: Add your specialized code here and/or call the base class
	UpdateData();
	if (m_bUseScript && m_extlist.GetCount()==0)
	{
		MessageBox(_T("��ָ����ʹ�ýű���������ű��ĺ�׺��"));
		return FALSE;
	}
	if (m_bScriptLimitTime && m_nScriptLimitTime==0)
	{
		MessageBox(_T("��ָ���˽ű�����ʱ�䣬������ʱ��ֵ"));
		return FALSE;
	}
	if (m_bUseSession && m_nSessionTimeout==0)
	{
		MessageBox(_T("��ָ����ʹ��Session��������Session��ʱʱ��"));
		return FALSE;
	}
	return CPropertyPage::OnKillActive();
}

void CProScript::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class
	UpdateData();
	theApp.m_server.m_bUseScript=m_bUseScript;
	if (m_bScriptLimitTime)
		theApp.m_server.m_nRunLimitTime=m_nScriptLimitTime*1000;
	else
		theApp.m_server.m_nRunLimitTime=0;
	theApp.m_server.m_bUseSession=m_bUseSession;
	theApp.m_server.m_nSessionLimitTime=m_nSessionTimeout*1000;
	theApp.m_server.m_bSessionCookielessMode=m_bSessionCookieLessMode;
	//fixme script ext list
	THString ext=_T(".");
	for(int i=0;i<m_extlist.GetCount();i++)
	{
		THString str;
		m_extlist.GetText(i,str);
		ext+=str+_T(".");
	}
	theApp.m_server.m_sScriptExtList=ext;

	CPropertyPage::OnOK();
}

BOOL CProScript::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// TODO:  Add extra initialization here
	m_bUseScript=theApp.m_server.m_bUseScript;
	if (theApp.m_server.m_nRunLimitTime==0)
	{
		m_bScriptLimitTime=FALSE;
		m_nScriptLimitTime=90;
	}
	else
	{
		m_bScriptLimitTime=TRUE;
		m_nScriptLimitTime=theApp.m_server.m_nRunLimitTime/1000;
	}
	m_bUseSession=theApp.m_server.m_bUseSession;
	m_nSessionTimeout=theApp.m_server.m_nSessionLimitTime/1000;
	m_bSessionCookieLessMode=theApp.m_server.m_bSessionCookielessMode;
	UpdateData(FALSE);
	THStringToken t(theApp.m_server.m_sScriptExtList,_T("."));
	THString str;
	while(t.IsMoreTokens())
	{
		str=t.GetNextToken();
		if (!str.IsEmpty()) m_extlist.AddString(str);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CProScript::OnBnClickedButton2()
{
	// TODO: Add your control notification handler code here
	//add
	UpdateData();
	THBaseInputDialog dlg;
	THString str;
	if (dlg.ShowInputDialog(&str,_T("���ӽű���׺"),_T("�������׺��(��: tsp):"),theApp.m_hInstance,INPUTDIALOGTYPE_HINTUP,1,m_hWnd)==IDOK)
	{
		str=THSimpleXml::GetParam(str,_T("ret0"));
		str.Replace(_T("*"),_T(""));
		str.Replace(_T("."),_T(""));
		str.Trim();
		if (!str.IsEmpty()) m_extlist.AddString(str);
	}
}

void CProScript::OnBnClickedButton3()
{
	// TODO: Add your control notification handler code here
	//edit
	UpdateData();
	int idx=m_extlist.GetCurSel();
	if (idx==-1)
	{
		MessageBox(_T("��ѡ����Ҫ�༭�ĺ�׺��"));
		return;
	}
	THString str;
	m_extlist.GetText(idx,str);
	THBaseInputDialog dlg;
	if (dlg.ShowInputDialog(&str,_T("�༭�ű���׺"),_T("�������׺��(��: tsp):"),theApp.m_hInstance,INPUTDIALOGTYPE_HINTUP,1,m_hWnd)==IDOK)
	{
		str=THSimpleXml::GetParam(str,_T("ret0"));
		str.Replace(_T("*"),_T(""));
		str.Replace(_T("."),_T(""));
		str.Trim();
		if (!str.IsEmpty())
		{
			m_extlist.DeleteString(idx);
			m_extlist.AddString(str);
		}
	}
}

void CProScript::OnBnClickedButton4()
{
	// TODO: Add your control notification handler code here
	//del
	UpdateData();
	int idx=m_extlist.GetCurSel();
	if (idx==-1)
	{
		MessageBox(_T("��ѡ����Ҫ�༭�ĺ�׺��"));
		return;
	}
	THString str;
	m_extlist.GetText(idx,str);
	if (MessageBox(THFormat(_T("ȷ��ɾ����׺ %s ��"),str),_T("ȷ��ɾ��"),MB_YESNO|MB_ICONQUESTION)==IDYES)
		m_extlist.DeleteString(idx);
}

void CProScript::OnBnClickedCheck1()
{
	// TODO: Add your control notification handler code here
	UpdateData();
	GetDlgItem(IDC_BUTTON2)->EnableWindow(m_bUseScript);
	GetDlgItem(IDC_BUTTON3)->EnableWindow(m_bUseScript);
	GetDlgItem(IDC_BUTTON4)->EnableWindow(m_bUseScript);
	GetDlgItem(IDC_LIST1)->EnableWindow(m_bUseScript);
	GetDlgItem(IDC_CHECK2)->EnableWindow(m_bUseScript);
	GetDlgItem(IDC_EDIT1)->EnableWindow(m_bUseScript);
}

void CProScript::OnBnClickedCheck3()
{
	// TODO: Add your control notification handler code here
	UpdateData();
	GetDlgItem(IDC_EDIT2)->EnableWindow(m_bUseSession);
	GetDlgItem(IDC_CHECK5)->EnableWindow(m_bUseSession);
}

void CProScript::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	MessageBox(_T("Session��ָ�������ֲ�ͬ�ͻ��ı�ʶ�����ͻ���Session��ʱʱ���ڷ��ʷ�����ʱ���ÿͻ���SessionID��һ���ġ�\r\n\r\nSession Cookieģʽ��ָʹ��Cookie��¼Session��Ĭ��ʹ�ø�ֵ��\r\n\r\nSession CookieLessģʽ��ָ��ʹ��Cookie��¼Session������ͨ���ڷ��ʵ�url�м���Session��Ϊ��Ŀ¼������Session�ķ�ʽ��"),_T("����Session"),MB_OK|MB_ICONINFORMATION);
}
