// DataMaker.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error �ڰ������� PCH �Ĵ��ļ�֮ǰ������stdafx.h��
#endif

#include "resource.h"		// ������


// CDataMakerApp:
// �йش����ʵ�֣������ DataMaker.cpp
//

class CDataMakerApp : public CWinApp
{
public:
	CDataMakerApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CDataMakerApp theApp;
