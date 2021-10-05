#pragma once
#include "afxcmn.h"


// CSubDlgSearched dialog

class CSubDlgSearched : public CDialog
{
	DECLARE_DYNAMIC(CSubDlgSearched)

public:
	CSubDlgSearched(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSubDlgSearched();

// Dialog Data
	enum { IDD = IDD_SUBDIALOG3 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CTreeCtrl m_TreeCtrl;
};
