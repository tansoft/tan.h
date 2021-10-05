// AllTestDlg.h : 头文件
//

#pragma once

#define _USE_RESIZE_DLG
#ifdef _USE_RESIZE_DLG
#include <THResizeDlg.h>
#else
#define THResizeDlg CDialog
#endif

#include <THPopupDlgDll.h>

#include <THSqliteDll.h>

#include <THINet.h>

#include <THSerialPort.h>

#include <THModem.h>

#include <THLiveUpdate.h>
#include <THLiveUpdateUi.h>

#include <THSoftwareAuth.h>

#include <THCmdProcess.h>

#define WM_COMINGSMS		WM_USER+1213

// CAllTestDlg 对话框
class CAllTestDlg : public THResizeDlg , public THSqliteDataHandler, public ITHSerialPortBufferedEventHandler , public ITHGsmModemEventHandler ,public THGsmModemMachineEventHandler ,public THFtpEventHandler,public ITHSoftwareAuthResult ,public ITHCmdProcessEvent
{
// 构造
public:
	CAllTestDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_ALLTEST_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	virtual BOOL OnEnumData(THStringArray *titles,THStringArray *args);

	THPopupDlgDll m_popupdll;
	THSqliteDll m_sqldll;
	THINetHttp m_http;
	THINetFtp m_ftp;
	int m_threadid;
	THSerialPort m_port;
	THGsmModem gsm;
	THGsmModemMachine m_machine;
	THLiveUpdateUi m_liveupdateui;
	THLiveUpdate m_liveupdate;
	THSoftwareAuth m_auth;
	THCmdProcess m_process;

	virtual void OnHandleError(int nErrorType,int nPortNum);
	virtual void OnHandleFlagEvent(DWORD dwCommEvent,int nPortNum);
	virtual unsigned int OnHandleBufferCharEvent(const void *pData,unsigned int nLen,int nPortNum);

	virtual void OnResponseRequest(THString str,int status);
	virtual void OnComingMessage(THString str,int status);
	virtual void OnComingSMS(int idx);

	virtual void OnSMSComing(const int idx,const THString sender,const THString msg,const THTime ti);

	BOOL OnConnected(int threadid,const THString host,class THINetFtp *cls,void *adddata);
	void OnCommandFinish(int threadid,const THString host,class THINetFtp *cls,void *adddata,THString cmd,THString ret,THString val1,THString val2,int cmdadddata);
	void OnFinish(int threadid,const THString host,class THINetFtp *cls,void *adddata,int ErrCode);
	BOOL OnProcessing(int threadid,const THString host,class THINetFtp *cls,void *adddata,unsigned int current,unsigned int down,unsigned int total,THString cmd,THString val1,THString val2,int cmdadddata);

	void OnAuthResult(THSoftwareAuth *cls,THAuthState Result,THString strErrMsg,void *adddata);

	LRESULT OnComingSMSMsg(WPARAM wParam,LPARAM lParam);

	BOOL OnProcessingOutput(THCmdProcess *pProcess,THProcessInfo *info,int nThreadId);

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
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButton5();
	afx_msg void OnBnClickedButton6();
	afx_msg void OnBnClickedButton7();
	afx_msg void OnBnClickedButton8();
	afx_msg void OnBnClickedButton9();
	afx_msg void OnBnClickedButton10();
	afx_msg void OnBnClickedButton11();
	afx_msg void OnBnClickedButton12();
	afx_msg void OnBnClickedButton13();
	afx_msg void OnBnClickedButton14();
	afx_msg void OnBnClickedButton15();
	afx_msg void OnBnClickedButton16();
	afx_msg void OnBnClickedButton17();
	afx_msg void OnBnClickedButton18();
	afx_msg void OnBnClickedButton19();
	afx_msg void OnBnClickedButton20();
};
