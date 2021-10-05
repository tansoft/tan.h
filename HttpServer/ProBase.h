#pragma once


// CProBase dialog

class CProBase : public CPropertyPage
{
	DECLARE_DYNAMIC(CProBase)

public:
	CProBase();
	virtual ~CProBase();

// Dialog Data
	enum { IDD = IDD_PROPPAGE_BASE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	UINT m_nListenPort;
	UINT m_nDynIpSid;
	afx_msg void OnBnClickedButton4();
	virtual BOOL OnSetActive();
	BOOL m_bAutoRun;
	BOOL m_bAutoStart;
	BOOL m_bStartHide;
	BOOL m_bHttpsService;
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual BOOL OnKillActive();
};
