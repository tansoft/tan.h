// THScriptDebuggerDlg.h : ͷ�ļ�
//

#pragma once

#include <THScript.h>
#include <THDebug.h>
#include <THThread.h>
#include "afxcmn.h"

#define STATE_STEPINTO	0		//����ִ�У����������break��
#define STATE_STEPOVER	1		//����������������function��ͷʱ�����У�ֱ����ջ��û���Ӻ���Ϊֹ
#define STATE_RUN		2		//����
#define STATE_RUNTORET	3		//ִ�е��������أ�������end
#define STATE_SKIP		4		//������ǰִ��

#define WM_UPDATE_VAR			WM_USER+1432

// CTHScriptDebuggerDlg �Ի���
class CTHScriptDebuggerDlg : public CDialog ,public ITHScriptDebugger,public THAsynCall<CTHScriptDebuggerDlg>
{
// ����
public:
	CTHScriptDebuggerDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_THSCRIPTDEBUGGER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

public:
	//����TRUE��ʾ���Ըô���
	virtual BOOL OnScriptErrorHandler(int nErrCode,THString sErrText);
	//����ʼ���ú���ʱ������nRecursion��ʾ�ݹ������ע����Timer,Schedule���ⲿ��������ʱ���ݹ鶼��0��ʼ����
	//����FALSE��ʾ�˳��ú���ִ��
	virtual BOOL OnEnterFunction(THString sCurrentFn,int nRecursion,int localidx,THString *tempsetting);
	//���������ú���ʱ������sRet��ʾ��������ֵ
	virtual void OnLeaveFunction(THString sCurrentFn,int nRecursion,THString *sRet,int localidx);
	//����FALSE��ʾ��ִ�иþ�ű�
	virtual BOOL OnRunScript(THString script,int localidx,THString *tempsetting);
	//����tag
	virtual void OnEnterTag(THString fn,THString tag,THString *script);
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
	afx_msg void OnBnClickedButton4();
	THScript m_script;
	afx_msg void OnBnClickedButton5();
	int m_nLocalIdx;
	afx_msg void OnEnChangeEdit1();
	void UpdateVar();
	void UpdateGlobalState();
	void UpdateWindowTitle();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton3();
	struct _GlbalSetting{
		THString m_sFile;
		THString m_sFn;
		THString m_sTag;
		THString m_sScript;
		THString m_sCurScript;
		THString m_sCurTempVar;
		THString m_sReturnRet;
	}m_gset;
	afx_msg void OnBnClickedButton6();
	afx_msg void OnBnClickedButton7();
	afx_msg void OnBnClickedCancel();
	CListCtrl m_varslist;
	void InsertVarItem(THString stype,THString sxml);
	void LoadScript();
	void StartDebug();
	void FreeScript();
	THEvent m_DebugEvent;
	afx_msg void OnBnClickedButton8();
	afx_msg void OnBnClickedButton9();
	volatile int m_nLastState;
	volatile int m_nStepovering;
	volatile int m_nRunningState;
	afx_msg void OnBnClickedButton13();
	afx_msg void OnBnClickedButton10();
	void Continue();
	afx_msg void OnBnClickedButton14();
	afx_msg LRESULT OnUpdateVar(WPARAM w,LPARAM l);
	CListCtrl m_scriptlist;
	THString m_sCallStack;
	THString m_sBreakPoint;
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton11();
	afx_msg void OnBnClickedButton15();
	afx_msg void OnBnClickedButton12();
	afx_msg void OnBnClickedButton16();
	afx_msg void OnBnClickedButton17();
};
