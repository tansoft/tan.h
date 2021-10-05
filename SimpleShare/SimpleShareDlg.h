// SimpleShareDlg.h : 头文件
//

#pragma once

#include <THSimpleShare.h>
#include <THBaseDialog.h>
#include "afxwin.h"
// CSimpleShareDlg 对话框
class CSimpleShareDlg : public CDialog , public THSimpleShare , private ITHBaseInputDialogEventHandler
{
// 构造
public:
	CSimpleShareDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_SIMPLESHARE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	virtual void HandleMessage(THStringA str,U8 cmd,U16 seq,U32 peerip,U16 peerport);
	virtual BOOL IsInputRight(int nCnt,THString sInput,void *addParam){return TRUE;}

	THBaseInputDialog m_input;
	
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
	CListBox m_box;
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButton5();
	afx_msg void OnBnClickedButton6();
};
