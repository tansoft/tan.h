// MfcCtrlsTest.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error �ڰ������� PCH �Ĵ��ļ�֮ǰ������stdafx.h��
#endif

#include "resource.h"		// ������


// CMfcCtrlsTestApp:
// �йش����ʵ�֣������ MfcCtrlsTest.cpp
//

class CMfcCtrlsTestApp : public CWinApp
{
public:
	CMfcCtrlsTestApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CMfcCtrlsTestApp theApp;
