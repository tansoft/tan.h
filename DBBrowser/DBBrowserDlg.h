// DBBrowserDlg.h : ͷ�ļ�
//

#pragma once

#include <THDataBase.h>
#include <THDBOdbc.h>
#include <THDBQuery.h>
#include <THMfcMiscCtrls.h>
#include <THNetwork.h>
#include <THIni.h>

// CDBBrowserDlg �Ի���
class CDBBrowserDlg : public CDialog
{
// ����
public:
	CDBBrowserDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_DBBROWSER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

	//THDBOdbc m_dbodbc;
	//THMulDataBase m_dbs;
	ITHDataBase *m_pdb;
	THDBQuery m_query;
	THDBQueryListCtrl m_list;
// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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
