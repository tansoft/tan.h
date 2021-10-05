#pragma once


// CProPerFormance dialog

class CProPerFormance : public CPropertyPage
{
	DECLARE_DYNAMIC(CProPerFormance)

public:
	CProPerFormance();
	virtual ~CProPerFormance();

// Dialog Data
	enum { IDD = IDD_PROPPAGE_PERFORMANCE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	UINT m_BaseThread;
	BOOL m_bLimitMaxThread;
	UINT m_nLimitMaxThread;
	afx_msg void OnBnClickedCheck1();
	virtual BOOL OnInitDialog();
	virtual BOOL OnKillActive();
	virtual void OnOK();
	afx_msg void OnBnClickedButton4();
	BOOL m_bUseCache;
	afx_msg void OnBnClickedCheck2();
	int m_nCacheLimitType;
	UINT m_nCacheSize;
	UINT m_nCacheRange;
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedRadio1();
	afx_msg void OnBnClickedRadio2();
	afx_msg void OnBnClickedRadio4();
	afx_msg void OnBnClickedButton7();
};
