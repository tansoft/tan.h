// LiveUpdateDll.h : main header file for the LiveUpdateDll DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

// CLiveUpdateDllApp
// See LiveUpdateDll.cpp for the implementation of this class
//

class CLiveUpdateDllApp : public CWinApp
{
public:
	CLiveUpdateDllApp();
// Overrides
public:
	virtual BOOL InitInstance();
	DECLARE_MESSAGE_MAP()
};
