#pragma once

#include <THLiveUpdate.h>
#include "SubDlgWelcome.h"
#include "SubDlgSearching.h"
#include "SubDlgSearched.h"
#include "SubDlgDownloading.h"
#include "SubDlgFinish.h"

// CLiveUpdateDlg dialog

class CLiveUpdateDlg : public CDialog , public ITHLiveUpdateEventHandler
{
	DECLARE_DYNAMIC(CLiveUpdateDlg)

public:
	CLiveUpdateDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLiveUpdateDlg();

	virtual BOOL OnStartLiveUpdate(THLiveUpdate *pUpdate);

	virtual void OnFinishLiveUpdate(THLiveUpdate *pUpdate,int nReason);

	virtual void OnProcessStateMessage(THLiveUpdate *pUpdate,THString sMsg);

	virtual void OnProcessPercent(THLiveUpdate *pUpdate,UINT nPosAll,UINT nPosCurrent,UINT nTotal,UINT nFinished,UINT nSpeedPreSecond);

	virtual BOOL OnFoundedUpdate(THLiveUpdate *pUpdate,int nMax);

	virtual BOOL OnStartInstall(THLiveUpdate *pUpdate);

	THLiveUpdate m_update;

	void UpdateNowState();
//fill data
	THString m_sPath,m_sTitle,m_sUrl;
	UINT m_nReTryMin,m_nFailedReTryMin,m_nFailReTry;
	THString m_Company;
#define UPDATEDLGTYPE_PAGEWELCOME		0x1		//使用欢迎页
#define UPDATEDLGTYPE_PAGESELECT		0x2		//使用选择安装页
	int m_nType;	//安装形式

#define UPDATEDLGSTATE_WELCOME		0
#define UPDATEDLGSTATE_SEARCHING	1
#define UPDATEDLGSTATE_AVAUPDATE	2
#define UPDATEDLGSTATE_DOWNLOADING	3
#define UPDATEDLGSTATE_FINISH		4
	int m_nState;	//0 显示欢迎页 1 显示正在查询页 2 显示可用更新页 3 下载更新页 4 完成页

	int m_nUpdateCount;
	THEvent m_event;
// Dialog Data
	enum { IDD = IDD_DIALOG1 };
	CSubDlgWelcome m_welcome;
	CSubDlgSearching m_searching;
	CSubDlgSearched m_searched;
	CSubDlgDownloading m_downloading;
	CSubDlgFinish m_finish;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};
