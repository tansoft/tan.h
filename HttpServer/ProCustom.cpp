// ProCustom.cpp : implementation file
//

#include "stdafx.h"
#include "HttpServer.h"
#include "ProCustom.h"


// CProCustom dialog

IMPLEMENT_DYNAMIC(CProCustom, CPropertyPage)
CProCustom::CProCustom()
	: CPropertyPage(CProCustom::IDD)
{
}

CProCustom::~CProCustom()
{
}

void CProCustom::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CProCustom, CPropertyPage)
END_MESSAGE_MAP()


// CProCustom message handlers
