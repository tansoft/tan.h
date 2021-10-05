#pragma once
#include "afxcmn.h"


// CSkinDemo dialog

class CSkinDemo : public CDialog
{
	DECLARE_DYNAMIC(CSkinDemo)

public:
	CSkinDemo(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSkinDemo();

// Dialog Data
	enum { IDD = IDD_DEMOSSK };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButton5();
	afx_msg void OnBnClickedButton6();
	CComboBoxEx m_comboex1;
	CComboBoxEx m_comboex2;
};
