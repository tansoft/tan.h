// HttpServerDlg.h : ͷ�ļ�
//

#pragma once

#include "afxwin.h"
#include "Settings.h"

#include <THTrayIcon.h>

#define WM_TRAY_MESSAGE		WM_USER+0x1643

// CHttpServerDlg �Ի���
class CHttpServerDlg : public CDialog
{
// ����
public:
	CHttpServerDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_HTTPSERVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


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
	//void UpdateAllData(BOOL bUpdate=TRUE);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	THTrayIcon m_trayicon;
	CStatic m_stsLight;
	//BOOL m_bRunState;
	afx_msg void OnBnClickedBtnrun();
	//CString m_sFilePath;
	//CString m_sImgFile;
	//int m_nFileStoreType;
	//CSettings *m_set;
	//afx_msg void OnBnClickedRadio1();
	afx_msg void OnBnClickedBtnrun2();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg LRESULT OnTrayIconMsg(WPARAM wParam,LPARAM lParam);
	afx_msg void OnTrayExit();
protected:
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
};
