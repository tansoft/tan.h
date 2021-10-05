// ProMulService.cpp : implementation file
//

#include "stdafx.h"
#include "HttpServer.h"
#include "ProMulService.h"


// CProMulService dialog

IMPLEMENT_DYNAMIC(CProMulService, CPropertyPage)
CProMulService::CProMulService()
	: CPropertyPage(CProMulService::IDD)
{
}

CProMulService::~CProMulService()
{
}

void CProMulService::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CProMulService, CPropertyPage)
END_MESSAGE_MAP()


// CProMulService message handlers
