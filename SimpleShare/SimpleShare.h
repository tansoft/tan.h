// SimpleShare.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error �ڰ������� PCH �Ĵ��ļ�֮ǰ������stdafx.h��
#endif

#include "resource.h"		// ������


// CSimpleShareApp:
// �йش����ʵ�֣������ SimpleShare.cpp
//

class CSimpleShareApp : public CWinApp
{
public:
	CSimpleShareApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CSimpleShareApp theApp;
