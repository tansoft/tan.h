// ProPlugin.cpp : implementation file
//

#include "stdafx.h"
#include "HttpServer.h"
#include "ProPlugin.h"


// CProPlugin dialog

IMPLEMENT_DYNAMIC(CProPlugin, CPropertyPage)
CProPlugin::CProPlugin()
	: CPropertyPage(CProPlugin::IDD)
{
}

CProPlugin::~CProPlugin()
{
}

void CProPlugin::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CProPlugin, CPropertyPage)
END_MESSAGE_MAP()


// CProPlugin message handlers
