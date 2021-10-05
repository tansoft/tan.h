#pragma once


// CProLog dialog

class CProLog : public CPropertyPage
{
	DECLARE_DYNAMIC(CProLog)

public:
	CProLog();
	virtual ~CProLog();

// Dialog Data
	enum { IDD = IDD_PROPPAGE_LOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	BOOL m_bRecordError;
	BOOL m_bRecordInfo;
	BOOL m_bRecordDetail;
	CString m_sRecordPath;
	int m_nRecordType;
	afx_msg void OnBnClickedButton1();
	virtual BOOL OnKillActive();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
};
