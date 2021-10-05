#pragma once


// CSubDlgWelcome dialog

class CSubDlgWelcome : public CDialog
{
	DECLARE_DYNAMIC(CSubDlgWelcome)

public:
	CSubDlgWelcome(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSubDlgWelcome();

// Dialog Data
	enum { IDD = IDD_SUBDIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
