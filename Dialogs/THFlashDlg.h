#pragma once

#include <Dialogs\THFlash.h>
// THMagneticDlg dialog
#define IDC_THFLASH1	2035

class THFlashDlg : public CDialog
{
	DECLARE_DYNAMIC(THFlashDlg)

public:
	THFlashDlg(UINT nIDTemplate, CWnd* pParentWnd=NULL);   // standard constructor
	virtual ~THFlashDlg();

	THFlash m_flash;
	THString m_flashfile;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void FlashCallShockwaveflash(LPCTSTR request);
	virtual void FSCommandShockwaveflash(LPCTSTR command, LPCTSTR args);
	virtual void OnProgressShockwaveflash(long percentDone);
	virtual void OnReadyStateChangeShockwaveflash(long newState);
	virtual void OnFlashCommand(THString cmd,THString args);

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL OnInitDialog();
};
