// AllTest.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error �ڰ������� PCH �Ĵ��ļ�֮ǰ������stdafx.h��
#endif

#include "resource.h"		// ������

#include <THSkinWinApp.h>
// CAllTestApp:
// �йش����ʵ�֣������ AllTest.cpp
//

class CAllTestApp : public THSkinWinApp
{
public:
	CAllTestApp();

// ��д
	public:
	virtual BOOL InitInstance();

	THString GetSkinFile(THString sExePath);

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CAllTestApp theApp;
