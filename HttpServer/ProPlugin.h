#pragma once


// CProPlugin dialog

class CProPlugin : public CPropertyPage
{
	DECLARE_DYNAMIC(CProPlugin)

public:
	CProPlugin();
	virtual ~CProPlugin();

// Dialog Data
	enum { IDD = IDD_PROPPAGE_PLUGIN };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
