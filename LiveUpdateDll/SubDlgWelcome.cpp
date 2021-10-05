// SubDlgWelcome.cpp : implementation file
//

#include "stdafx.h"
#include "LiveUpdateDll.h"
#include "SubDlgWelcome.h"


// CSubDlgWelcome dialog

IMPLEMENT_DYNAMIC(CSubDlgWelcome, CDialog)
CSubDlgWelcome::CSubDlgWelcome(CWnd* pParent /*=NULL*/)
	: CDialog(CSubDlgWelcome::IDD, pParent)
{
}

CSubDlgWelcome::~CSubDlgWelcome()
{
}

void CSubDlgWelcome::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSubDlgWelcome, CDialog)
END_MESSAGE_MAP()


// CSubDlgWelcome message handlers
