// MiscTools.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error �ڰ������� PCH �Ĵ��ļ�֮ǰ������stdafx.h��
#endif

#include "resource.h"		// ������


// CMiscToolsApp:
// �йش����ʵ�֣������ MiscTools.cpp
//

class CMiscToolsApp : public CWinApp
{
public:
	CMiscToolsApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CMiscToolsApp theApp;
