// DBBrowser.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error �ڰ������� PCH �Ĵ��ļ�֮ǰ������stdafx.h��
#endif

#include "resource.h"		// ������


// CDBBrowserApp:
// �йش����ʵ�֣������ DBBrowser.cpp
//

class CDBBrowserApp : public CWinApp
{
public:
	CDBBrowserApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CDBBrowserApp theApp;
