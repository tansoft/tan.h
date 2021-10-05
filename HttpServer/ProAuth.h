#pragma once
#include "afxwin.h"


// CProAuth dialog

class CProAuth : public CPropertyPage
{
	DECLARE_DYNAMIC(CProAuth)

public:
	CProAuth();
	virtual ~CProAuth();

// Dialog Data
	enum { IDD = IDD_PROPPAGE_AUTH };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	int m_nAuthType;
	CString m_hostname;
	CListBox m_userlist;
	BOOL m_bLimitIp;
	CString m_allowip;
	CString m_blockip;
	THStringArray m_arUserList;
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton5();
	afx_msg void OnBnClickedButton6();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton2();
	virtual BOOL OnInitDialog();
	virtual BOOL OnKillActive();
	virtual void OnOK();
	afx_msg void OnBnClickedCheck1();
};
