// MfcCtrlsTestDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#include <THDBOdbc.h>
#include <BaseCtrls\THDBQueryCtrls.h>
#include "afxwin.h"
//#include "..\..\BaseCtrls\THSortListCtrl.h"

// CMfcCtrlsTestDlg 对话框
class CMfcCtrlsTestDlg : public CDialog
{
// 构造
public:
	CMfcCtrlsTestDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_MFCCTRLSTEST_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	THDBQueryListCtrl m_List;
	//THSortListCtrl m_List;
	THDBQuery m_query;
	THDBOdbc m_database;
	THMulDataBase m_dbs;
	afx_msg void OnDestroy();
	THDBQueryEditPanel m_panel;
	THDBQueryEdit m_edit;
	THDBQueryDateTimeCtrl m_date;
	THDBQueryMultiSelectComboBox m_mcombo;
	THDBQueryEditComboBox m_combo;
	THDBQueryListBox m_listbox;
	THDBQueryStatic m_static;
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
};
