// PluginHttpsServer.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "PluginHttpsServer.h"
#include <THPlugin.h>
#include <THHttpServer.h>
#include <THSsl.h>

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

// CPluginHttpsServerApp

BEGIN_MESSAGE_MAP(CPluginHttpsServerApp, CWinApp)
END_MESSAGE_MAP()


// CPluginHttpsServerApp construction

CPluginHttpsServerApp::CPluginHttpsServerApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CPluginHttpsServerApp object

CPluginHttpsServerApp theApp;


// CPluginHttpsServerApp initialization

BOOL CPluginHttpsServerApp::InitInstance()
{
	CWinApp::InitInstance();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	return TRUE;
}

CString g_pem;
CString g_key;


class MySocketLayer : public ITHHttpServerPluginTypeSocketLayerReplacer
{
	THRawSocket *GetSocketLayer()
	{
		if (g_pem.IsEmpty() || g_key.IsEmpty()) return NULL;
		THRawSslSocket *s=new THRawSslSocket;
		s->InitSsl(THSSL_TYPE_NORMALSERVER,g_pem,g_key);
		return (THRawSocket *)s;
	}
	void ReleaseSocketLayer(THRawSocket *layer)
	{
		THRawSslSocket *l=(THRawSslSocket *)layer;
		if (l)
		{
			l->StopListen();
			delete l;
		}
	}
};

class MyInterface : public ITHPluginInterface
{
public:
	//返回失败为加载出错，可能提供的系统环境不支持该插件
	virtual BOOL Init(){return TRUE;}
	virtual BOOL Free(){return TRUE;}
	virtual void *QueryInterface(LPCSTR fnName)
	{
		if (strcmp(fnName,"ITHHttpServerPluginTypeSocketLayerReplacer")==0)
			return (void *)(ITHHttpServerPluginTypeSocketLayerReplacer *)&m_layer;
		return NULL;
	}
	virtual void OnEvent(LPCSTR event,void *param1,void *param2)
	{
		if (strcmp(event,"SslSocketPemAndKey")==0)
		{
			g_pem=THString((LPCSTR)param1);
			g_key=THString((LPCSTR)param2);
		}
	}
protected:
	MySocketLayer m_layer;
};

class MyInterface g_face;
ITHPluginSystemInterface *g_sys=NULL;

ITHPluginInterface *PluginInterface(ITHPluginSystemInterface *sys)
{
	g_sys=sys;
	return &g_face;
}
