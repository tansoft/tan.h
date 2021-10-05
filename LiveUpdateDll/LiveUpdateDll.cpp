// LiveUpdateDll.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "LiveUpdateDll.h"
#include "LiveUpdateDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

// CLiveUpdateDllApp

BEGIN_MESSAGE_MAP(CLiveUpdateDllApp, CWinApp)
END_MESSAGE_MAP()


CLiveUpdateDlg g_dlg;
// CLiveUpdateDllApp construction

CLiveUpdateDllApp::CLiveUpdateDllApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	//g_dlg.Create(IDD_ALLTEST_DIALOG);
}

extern "C" __declspec(dllexport) BOOL ShowLiveUpdateProcessBlock()
{
	CLiveUpdateDlg m_dlg;
	m_dlg.DoModal();
	return TRUE;
}

extern "C" __declspec(dllexport) BOOL ShowLiveUpdateProcess(const char* sPath,const char* sTitle,const char* sUrl,UINT nReTryMin,UINT nFailedReTryMin,UINT nFailReTryCount)
{
	g_dlg.m_sPath=THCharset::a2t(sPath);
	g_dlg.m_sTitle=THCharset::a2t(sTitle);
	g_dlg.m_sUrl=THCharset::a2t(sUrl);
	g_dlg.m_nReTryMin=nReTryMin;
	g_dlg.m_nFailedReTryMin=nFailedReTryMin;
	g_dlg.m_nFailReTry=nFailReTryCount;
	g_dlg.m_nType=UPDATEDLGTYPE_PAGEWELCOME|UPDATEDLGTYPE_PAGESELECT;
	g_dlg.m_nState=0;
	g_dlg.DoModal();
	//g_dlg.ShowWindow(SW_SHOW);
	return TRUE;
}

// The one and only CLiveUpdateDllApp object

CLiveUpdateDllApp theApp;


// CLiveUpdateDllApp initialization

BOOL CLiveUpdateDllApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}
