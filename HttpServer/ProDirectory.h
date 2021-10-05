#pragma once


// CProDirectory dialog

class CProDirectory : public CPropertyPage
{
	DECLARE_DYNAMIC(CProDirectory)

public:
	CProDirectory();
	virtual ~CProDirectory();

// Dialog Data
	enum { IDD = IDD_PROPPAGE_DIRECTORY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
