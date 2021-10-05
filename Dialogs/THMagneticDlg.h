#pragma once


// THMagneticDlg dialog

class THMagneticDlg : public CDialog
{
	DECLARE_DYNAMIC(THMagneticDlg)

public:
	THMagneticDlg(UINT nIDTemplate, CWnd* pParentWnd=NULL);   // standard constructor
	virtual ~THMagneticDlg();

// Dialog Data
//	enum { IDD = IDD_THMAGNETICDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
