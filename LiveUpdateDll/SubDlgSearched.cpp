// SubDlgSearched.cpp : implementation file
//

#include "stdafx.h"
#include "LiveUpdateDll.h"
#include "SubDlgSearched.h"


// CSubDlgSearched dialog

IMPLEMENT_DYNAMIC(CSubDlgSearched, CDialog)
CSubDlgSearched::CSubDlgSearched(CWnd* pParent /*=NULL*/)
	: CDialog(CSubDlgSearched::IDD, pParent)
{
}

CSubDlgSearched::~CSubDlgSearched()
{
}

void CSubDlgSearched::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE1, m_TreeCtrl);
}


BEGIN_MESSAGE_MAP(CSubDlgSearched, CDialog)
END_MESSAGE_MAP()


// CSubDlgSearched message handlers
