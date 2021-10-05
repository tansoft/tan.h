// DBBrowserDlg.h : 头文件
//

#pragma once

#include <THDataBase.h>
#include <THDBOdbc.h>
#include <THDBQuery.h>
#include <THMfcMiscCtrls.h>
#include <THNetwork.h>
#include <THIni.h>

// CDBBrowserDlg 对话框
class CDBBrowserDlg : public CDialog
{
// 构造
public:
	CDBBrowserDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_DBBROWSER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	//THDBOdbc m_dbodbc;
	//THMulDataBase m_dbs;
	ITHDataBase *m_pdb;
	THDBQuery m_query;
	THDBQueryListCtrl m_list;
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
	afx_msg void OnBnClickedOk();
	CString m_sql;
	CString m_ip;
	CString m_db;
	CString m_usr;
	CString m_pas;
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedOk2();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton4();
};
