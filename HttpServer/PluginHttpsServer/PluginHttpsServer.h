// PluginHttpsServer.h : main header file for the PluginHttpsServer DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols


// CPluginHttpsServerApp
// See PluginHttpsServer.cpp for the implementation of this class
//

class CPluginHttpsServerApp : public CWinApp
{
public:
	CPluginHttpsServerApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
