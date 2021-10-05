#pragma once
#include "afxwin.h"


// CProScript dialog

class CProScript : public CPropertyPage
{
	DECLARE_DYNAMIC(CProScript)

public:
	CProScript();
	virtual ~CProScript();

// Dialog Data
	enum { IDD = IDD_PROPPAGE_SCRIPT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	BOOL m_bUseScript;
	BOOL m_bScriptLimitTime;
	UINT m_nScriptLimitTime;
	BOOL m_bUseSession;
	UINT m_nSessionTimeout;
	BOOL m_bSessionCookieLessMode;
	virtual BOOL OnKillActive();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	CListBox m_extlist;
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedCheck3();
	afx_msg void OnBnClickedButton1();
};
