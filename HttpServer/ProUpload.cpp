// ProUpload.cpp : implementation file
//

#include "stdafx.h"
#include "HttpServer.h"
#include "ProUpload.h"
#include ".\proupload.h"


// CProUpload dialog

IMPLEMENT_DYNAMIC(CProUpload, CPropertyPage)
CProUpload::CProUpload()
	: CPropertyPage(CProUpload::IDD)
	, m_sUploadPath(_T(""))
{
}

CProUpload::~CProUpload()
{
}

void CProUpload::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_sUploadPath);
}


BEGIN_MESSAGE_MAP(CProUpload, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
END_MESSAGE_MAP()


// CProUpload message handlers

void CProUpload::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
}

BOOL CProUpload::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// TODO:  Add extra initialization here
	//m_sUploadPath=

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CProUpload::OnKillActive()
{
	// TODO: Add your specialized code here and/or call the base class

	return CPropertyPage::OnKillActive();
}

void CProUpload::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class

	CPropertyPage::OnOK();
}
