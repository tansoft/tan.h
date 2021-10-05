#pragma once
#include "afxcmn.h"


// CSubDlgSearching dialog

class CSubDlgSearching : public CDialog
{
	DECLARE_DYNAMIC(CSubDlgSearching)

public:
	CSubDlgSearching(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSubDlgSearching();

// Dialog Data
	enum { IDD = IDD_SUBDIALOG2 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
};
