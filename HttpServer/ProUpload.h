#pragma once


// CProUpload dialog

class CProUpload : public CPropertyPage
{
	DECLARE_DYNAMIC(CProUpload)

public:
	CProUpload();
	virtual ~CProUpload();

// Dialog Data
	enum { IDD = IDD_PROPPAGE_UPLOAD };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	virtual BOOL OnInitDialog();
	virtual BOOL OnKillActive();
	virtual void OnOK();
	CString m_sUploadPath;
};
