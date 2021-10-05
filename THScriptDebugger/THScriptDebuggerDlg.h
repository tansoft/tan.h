// THScriptDebuggerDlg.h : 头文件
//

#pragma once

#include <THScript.h>
#include <THDebug.h>
#include <THThread.h>
#include "afxcmn.h"

#define STATE_STEPINTO	0		//单步执行，所有情况都break掉
#define STATE_STEPOVER	1		//单步跳过，当遇到function开头时，运行，直到堆栈中没有子函数为止
#define STATE_RUN		2		//运行
#define STATE_RUNTORET	3		//执行到函数返回，即遇到end
#define STATE_SKIP		4		//跳过当前执行

#define WM_UPDATE_VAR			WM_USER+1432

// CTHScriptDebuggerDlg 对话框
class CTHScriptDebuggerDlg : public CDialog ,public ITHScriptDebugger,public THAsynCall<CTHScriptDebuggerDlg>
{
// 构造
public:
	CTHScriptDebuggerDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_THSCRIPTDEBUGGER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

public:
	//返回TRUE表示忽略该错误
	virtual BOOL OnScriptErrorHandler(int nErrCode,THString sErrText);
	//当开始调用函数时触发，nRecursion表示递归次数，注意在Timer,Schedule及外部函数调用时，递归都由0开始计数
	//返回FALSE表示退出该函数执行
	virtual BOOL OnEnterFunction(THString sCurrentFn,int nRecursion,int localidx,THString *tempsetting);
	//当结束调用函数时触发，sRet表示函数返回值
	virtual void OnLeaveFunction(THString sCurrentFn,int nRecursion,THString *sRet,int localidx);
	//返回FALSE表示不执行该句脚本
	virtual BOOL OnRunScript(THString script,int localidx,THString *tempsetting);
	//进入tag
	virtual void OnEnterTag(THString fn,THString tag,THString *script);
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
