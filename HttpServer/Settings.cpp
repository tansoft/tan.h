// Settings.cpp : implementation file
//

#include "stdafx.h"
#include "HttpServer.h"
#include "Settings.h"


// CSettings

IMPLEMENT_DYNAMIC(CSettings, CPropertySheet)
CSettings::CSettings(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
	Init();
}

CSettings::CSettings(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
	Init();
}

CSettings::~CSettings()
{
}


BEGIN_MESSAGE_MAP(CSettings, CPropertySheet)
END_MESSAGE_MAP()

void CSettings::Init()
{
	AddPage(&m_base);
	AddPage(&m_directory);
	AddPage(&m_auth);
	AddPage(&m_formance);
	AddPage(&m_upload);
	AddPage(&m_script);
	AddPage(&m_mulservice);
	AddPage(&m_log);
	AddPage(&m_custom);
	AddPage(&m_plugin);
}
// CSettings message handlers
