#pragma once


// CProCustom dialog

class CProCustom : public CPropertyPage
{
	DECLARE_DYNAMIC(CProCustom)

public:
	CProCustom();
	virtual ~CProCustom();

// Dialog Data
	enum { IDD = IDD_PROPPAGE_CUSTOM };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
