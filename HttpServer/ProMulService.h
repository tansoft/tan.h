#pragma once


// CProMulService dialog

class CProMulService : public CPropertyPage
{
	DECLARE_DYNAMIC(CProMulService)

public:
	CProMulService();
	virtual ~CProMulService();

// Dialog Data
	enum { IDD = IDD_PROPPAGE_MULSERVICE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
