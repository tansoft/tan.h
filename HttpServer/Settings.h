#pragma once

#include "ProBase.h"
#include "ProDirectory.h"
#include "ProAuth.h"
#include "ProPerFormance.h"
#include "ProUpload.h"
#include "ProScript.h"
#include "ProLog.h"
#include "ProMulService.h"
#include "ProPlugin.h"
#include "ProCustom.h"

// CSettings

class CSettings : public CPropertySheet
{
	DECLARE_DYNAMIC(CSettings)

public:
	CSettings(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CSettings(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	virtual ~CSettings();
	CProBase m_base;
	CProDirectory m_directory;
	CProAuth m_auth;
	CProPerFormance m_formance;
	CProUpload m_upload;
	CProScript m_script;
	CProLog m_log;
	CProMulService m_mulservice;
	CProPlugin m_plugin;
	CProCustom m_custom;

protected:
	void Init();
	DECLARE_MESSAGE_MAP()
};


