// THScriptDebugger.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error �ڰ������� PCH �Ĵ��ļ�֮ǰ������stdafx.h��
#endif

#include "resource.h"		// ������


// CTHScriptDebuggerApp:
// �йش����ʵ�֣������ THScriptDebugger.cpp
//

class CTHScriptDebuggerApp : public CWinApp
{
public:
	CTHScriptDebuggerApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CTHScriptDebuggerApp theApp;
