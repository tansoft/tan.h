// ProDirectory.cpp : implementation file
//

#include "stdafx.h"
#include "HttpServer.h"
#include "ProDirectory.h"


// CProDirectory dialog

IMPLEMENT_DYNAMIC(CProDirectory, CPropertyPage)
CProDirectory::CProDirectory()
	: CPropertyPage(CProDirectory::IDD)
{
}

CProDirectory::~CProDirectory()
{
}

void CProDirectory::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CProDirectory, CPropertyPage)
END_MESSAGE_MAP()


// CProDirectory message handlers
