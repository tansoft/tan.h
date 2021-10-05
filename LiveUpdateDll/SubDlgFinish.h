#pragma once


// CSubDlgFinish dialog

class CSubDlgFinish : public CDialog
{
	DECLARE_DYNAMIC(CSubDlgFinish)

public:
	CSubDlgFinish(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSubDlgFinish();

// Dialog Data
	enum { IDD = IDD_SUBDIALOG5 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
