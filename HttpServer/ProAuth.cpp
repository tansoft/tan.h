// ProAuth.cpp : implementation file
//

#include "stdafx.h"
#include "HttpServer.h"
#include "ProAuth.h"
#include ".\proauth.h"
#include <THBaseDialog.h>

// CProAuth dialog

IMPLEMENT_DYNAMIC(CProAuth, CPropertyPage)
CProAuth::CProAuth()
	: CPropertyPage(CProAuth::IDD)
	, m_nAuthType(0)
	, m_hostname(_T(""))
	, m_bLimitIp(FALSE)
	, m_allowip(_T(""))
	, m_blockip(_T(""))
{
}

CProAuth::~CProAuth()
{
}

void CProAuth::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO1, m_nAuthType);
	DDX_Text(pDX, IDC_EDIT1, m_hostname);
	DDX_Control(pDX, IDC_LIST1, m_userlist);
	DDX_Check(pDX, IDC_CHECK1, m_bLimitIp);
	DDX_Text(pDX, IDC_EDIT2, m_allowip);
	DDX_Text(pDX, IDC_EDIT3, m_blockip);
}


BEGIN_MESSAGE_MAP(CProAuth, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON5, OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON6, OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON3, OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON2, OnBnClickedButton2)
	ON_BN_CLICKED(IDC_CHECK1, OnBnClickedCheck1)
END_MESSAGE_MAP()


// CProAuth message handlers
#include <THDebug.h>
void CProAuth::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	//add
	UpdateData();
	THString sRet;
	THBaseUserPassDialog dlg;
	if (dlg.ShowPassDialog(&sRet,_T("请输入账号密码"),theApp.m_hInstance,PASSDIALOGTYPE_CONFIRMPASS,m_hWnd)==IDOK)
	{
		int aridx=(int)m_arUserList.Add(sRet);
		THStringToken t(sRet,_T(":"));
		int idx=m_userlist.AddString(t.GetNextToken());
		m_userlist.SetItemData(idx,aridx);
	}
}

void CProAuth::OnBnClickedButton5()
{
	// TODO: Add your control notification handler code here
	//edit
	UpdateData();
	int idx=m_userlist.GetCurSel();
	if (idx==-1)
	{
		MessageBox(_T("请选择需要编辑的账号"));
		return;
	}
	int aridx=(int)m_userlist.GetItemData(idx);
	THStringToken t(m_arUserList[aridx],_T(":"));
	THString user=t.GetNextToken();
	THString sRet=t.GetNextToken();
	THBasePassDialog dlg;
	if (dlg.ShowPassDialog(&sRet,THFormat(_T("修改账号 %s 的密码"),user),theApp.m_hInstance,PASSDIALOGTYPE_CHANGEPASS,m_hWnd)==IDOK)
	{
		m_arUserList.SetAt(aridx,user+_T(":")+sRet);
	}
}

void CProAuth::OnBnClickedButton6()
{
	// TODO: Add your control notification handler code here
	//del
	UpdateData();
	int idx=m_userlist.GetCurSel();
	if (idx==-1)
	{
		MessageBox(_T("请选择需要删除的账号"));
		return;
	}
	THString str;
	m_userlist.GetText(idx,str);
	if (MessageBox(THFormat(_T("确认删除账号 %s 吗？"),str),_T("确认删除"),MB_YESNO|MB_ICONQUESTION)==IDYES)
		m_userlist.DeleteString(idx);
}

void CProAuth::OnBnClickedButton3()
{
	// TODO: Add your control notification handler code here
	//base auth ?
	MessageBox(_T("Http标准协议中的基本认证方式容易在传输时被监听，从而有泄露密码的风险。\r\n因此，在使用密码时，请尽量使用摘要认证方式。\r\n同时，在本地保存的用户名/密码加密是可逆加密，请确认您的机器有足够的安全性，注意密码有被盗用的风险。"),_T("关于风险警告"),MB_OK|MB_ICONINFORMATION);
}

void CProAuth::OnBnClickedButton2()
{
	// TODO: Add your control notification handler code here
	//ip range ?
	MessageBox(_T("填写范围请遵循以下格式：\r\nip1:mask1|ip2:mask2|\r\n其中mask可省略，默认为255.255.255.255\r\ne.g.: 192.168.0.0:255.255.255.0|10.1.2.0:255.255.0.0"),_T("ip范围设置格式"),MB_OK|MB_ICONINFORMATION);
}

BOOL CProAuth::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// TODO:  Add extra initialization here
	if (theApp.m_server.m_bAuth)
	{
		if (theApp.m_server.m_bDigestMode)
			m_nAuthType=2;
		else
			m_nAuthType=1;
	}
	else
		m_nAuthType=0;
	m_hostname=theApp.m_server.m_sAuthName;

	m_bLimitIp=theApp.m_server.m_bLimitIp;

	m_allowip=theApp.m_server.m_allowrange.SaveToString();
	m_blockip=theApp.m_server.m_blockrange.SaveToString();

	UpdateData(FALSE);
	m_arUserList.Copy(theApp.m_server.m_aAccountList);
	THStringToken t;
	for(int i=0;i<m_arUserList.GetSize();i++)
	{
		t.Init(m_arUserList[i],_T(":"));
		int idx=m_userlist.AddString(t.GetNextToken());
		m_userlist.SetItemData(idx,i);
	}

	OnBnClickedCheck1();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CProAuth::OnKillActive()
{
	// TODO: Add your specialized code here and/or call the base class
	UpdateData();
	if (m_bLimitIp)
	{
		if (m_allowip.IsEmpty())
		{
			if (MessageBox(_T("您设置了ip限制，但并没有设置允许ip列表，这样将会禁止所有的连接，是否把允许列表改为\"0.0.0.0:0.0.0.0|\"？"),_T("设置可能存在问题"),MB_YESNO|MB_ICONQUESTION)==IDYES)
			{
				m_allowip=_T("0.0.0.0:0.0.0.0|");
				UpdateData(FALSE);
			}
		}
	}
	return CPropertyPage::OnKillActive();
}

void CProAuth::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class
	UpdateData();
	if (m_nAuthType==0)
		theApp.m_server.m_bAuth=FALSE;
	else
	{
		theApp.m_server.m_bAuth=TRUE;
		if (m_nAuthType==1)
			theApp.m_server.m_bDigestMode=FALSE;
		else
			theApp.m_server.m_bDigestMode=TRUE;
	}
	theApp.m_server.m_sAuthName=m_hostname;
	if (m_nAuthType!=0 && m_hostname.IsEmpty()) theApp.m_server.m_sAuthName=_T("TansoftServer");
	theApp.m_server.m_bLimitIp=m_bLimitIp;
	theApp.m_server.m_allowrange.LoadFromString(m_allowip);
	theApp.m_server.m_blockrange.LoadFromString(m_blockip);
	THStringArray ar;
	for(int i=0;i<m_userlist.GetCount();i++)
	{
		ar.Add(m_arUserList[m_userlist.GetItemData(i)]);
	}
	theApp.m_server.m_aAccountList.RemoveAll();
	theApp.m_server.m_aAccountList.Copy(ar);
	CPropertyPage::OnOK();
}

void CProAuth::OnBnClickedCheck1()
{
	// TODO: Add your control notification handler code here
	UpdateData();
	GetDlgItem(IDC_EDIT2)->EnableWindow(m_bLimitIp);
	GetDlgItem(IDC_EDIT3)->EnableWindow(m_bLimitIp);
}
