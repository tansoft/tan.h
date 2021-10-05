#pragma once


// CSubDlgDownloading dialog

class CSubDlgDownloading : public CDialog
{
	DECLARE_DYNAMIC(CSubDlgDownloading)

public:
	CSubDlgDownloading(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSubDlgDownloading();

// Dialog Data
	enum { IDD = IDD_SUBDIALOG4 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
