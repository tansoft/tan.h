// THScriptDebuggerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "THScriptDebugger.h"
#include "THScriptDebuggerDlg.h"
#include ".\thscriptdebuggerdlg.h"

#include <THSysMisc.h>
#include <THBaseDialog.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CTHScriptDebuggerDlg 对话框



CTHScriptDebuggerDlg::CTHScriptDebuggerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTHScriptDebuggerDlg::IDD, pParent)
	, m_nLocalIdx(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_nLastState=0;
	m_nStepovering=0;
	m_nRunningState=0;
}

void CTHScriptDebuggerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_nLocalIdx);
	DDX_Control(pDX, IDC_LIST2, m_varslist);
	DDX_Control(pDX, IDC_LIST3, m_scriptlist);
}

BEGIN_MESSAGE_MAP(CTHScriptDebuggerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON4, OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, OnBnClickedButton5)
	ON_EN_CHANGE(IDC_EDIT1, OnEnChangeEdit1)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON3, OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON6, OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON7, OnBnClickedButton7)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON8, OnBnClickedButton8)
	ON_BN_CLICKED(IDC_BUTTON9, OnBnClickedButton9)
	ON_BN_CLICKED(IDC_BUTTON13, OnBnClickedButton13)
	ON_BN_CLICKED(IDC_BUTTON10, OnBnClickedButton10)
	ON_BN_CLICKED(IDC_BUTTON14, OnBnClickedButton14)
	ON_MESSAGE(WM_UPDATE_VAR,OnUpdateVar)
	ON_BN_CLICKED(IDC_BUTTON2, OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON11, OnBnClickedButton11)
	ON_BN_CLICKED(IDC_BUTTON15, OnBnClickedButton15)
	ON_BN_CLICKED(IDC_BUTTON12, OnBnClickedButton12)
	ON_BN_CLICKED(IDC_BUTTON16, OnBnClickedButton16)
	ON_BN_CLICKED(IDC_BUTTON17, OnBnClickedButton17)
END_MESSAGE_MAP()


// CTHScriptDebuggerDlg 消息处理程序

BOOL CTHScriptDebuggerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将\“关于...\”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_script.SetDebugger(this);
	m_DebugEvent.Create(NULL,TRUE,FALSE,NULL);

	m_varslist.InsertColumn(0,_T("类型"),LVCFMT_LEFT,60);
	m_varslist.InsertColumn(1,_T("名称"),LVCFMT_LEFT,100);
	m_varslist.InsertColumn(2,_T("值"),LVCFMT_LEFT,250);
	m_varslist.SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);

	m_scriptlist.InsertColumn(0,_T("参数"),LVCFMT_LEFT,60);
	m_scriptlist.InsertColumn(1,_T("值"),LVCFMT_LEFT,200);
	m_scriptlist.InsertColumn(2,_T("处理值"),LVCFMT_LEFT,150);
	m_scriptlist.SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);

	return TRUE;  // 除非设置了控件的焦点，否则返回 TRUE
}

void CTHScriptDebuggerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CTHScriptDebuggerDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标显示。
HCURSOR CTHScriptDebuggerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CTHScriptDebuggerDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	//OnOK();
}

void CTHScriptDebuggerDlg::OnBnClickedButton4()
{
	// TODO: Add your control notification handler code here
	m_script.EmptyGlobalSettings();
	UpdateVar();
}

void CTHScriptDebuggerDlg::OnBnClickedButton5()
{
	// TODO: Add your control notification handler code here
	UpdateData();
	m_script.EmptyLocalSettings(m_nLocalIdx);
	UpdateVar();
}

void CTHScriptDebuggerDlg::OnEnChangeEdit1()
{
	UpdateData();
	UpdateVar();
}

void CTHScriptDebuggerDlg::InsertVarItem(THString stype,THString sxml)
{
	THStringArray key,value;
	THSimpleXml::toArray(sxml,&key,&value);
	for(int i=0;i<key.GetSize();i++)
	{
		int item=m_varslist.InsertItem(m_varslist.GetItemCount(),stype);
		m_varslist.SetItemText(item,1,key[i]);
		m_varslist.SetItemText(item,2,value[i]);
	}
}

void CTHScriptDebuggerDlg::UpdateVar()
{
	SendMessage(WM_UPDATE_VAR,0,0);
}

LRESULT CTHScriptDebuggerDlg::OnUpdateVar(WPARAM w,LPARAM l)
{
	UpdateData();
	m_varslist.DeleteAllItems();
	if (m_nRunningState==1 || m_nRunningState==2)
	{
		InsertVarItem(_T("本地变量"),m_gset.m_sCurTempVar);
	}
	InsertVarItem(_T("全局变量"),m_script.GetGlobalSettings(_T("")));
	InsertVarItem(_T("用户变量"),m_script.GetLocalSettings(m_nLocalIdx,_T("")));
	THStringArray slist;
	slist.Add(_T("s_TickCount"));
	slist.Add(_T("s_Time32"));
	slist.Add(_T("s_LastError"));
	slist.Add(_T("s_EngineVer"));
	slist.Add(_T("s_CurPath"));
	slist.Add(_T("s_CurScript"));
	slist.Add(_T("s_WinPath"));
	slist.Add(_T("s_SysPath"));
	slist.Add(_T("s_TempPath"));
	THString ret,tmp;
	for(int i=0;i<slist.GetSize();i++)
		ret+=THSimpleXml::MakeParam(slist[i],m_script.ParseVar(_T("$")+slist[i]+_T("$"),&tmp,m_nLocalIdx));
	InsertVarItem(_T("系统变量"),ret);
	m_scriptlist.DeleteAllItems();
	if (m_nRunningState==1 || m_nRunningState==2)
	{
		THString str=m_gset.m_sCurScript;
		if (str.Find(_T(":"))!=-1)
			str.SetAt(str.Find(_T(":")),',');
		THStringToken t(str,_T(","));
		int idx=0;
		while(t.IsMoreTokens())
		{
			THString stype,key,value;
			if (idx==0)
				stype=_T("命令");
			else
				stype.Format(_T("参数%d"),idx);
			idx++;
			key=t.GetNextToken();
			value=m_script.ParseVar(key,&m_gset.m_sCurTempVar,m_nLocalIdx);
			int item=m_scriptlist.InsertItem(m_scriptlist.GetItemCount(),stype);
			m_scriptlist.SetItemText(item,1,key);
			m_scriptlist.SetItemText(item,2,value);
		}
	}
	return 0;
}

void CTHScriptDebuggerDlg::UpdateGlobalState()
{
	THString ret;
	if (m_nRunningState==1 || m_nRunningState==2)
		ret.Format(_T("函数：%s\r\n标号：%s\r\n当前脚本：\r\n%s\r\n"),m_gset.m_sFn,m_gset.m_sTag,m_gset.m_sCurScript);
	else if (m_nRunningState==3)
		ret.Format(_T("函数返回：%s\r\n"),m_gset.m_sReturnRet);
	GetDlgItem(IDC_GLOBALSTATE)->SetWindowText(ret);
	if (m_nRunningState==1 || m_nRunningState==2)
		ret.Format(_T("脚本：\r\n%s\r\n"),m_gset.m_sScript);
	else
		ret.Empty();
	GetDlgItem(IDC_GLOBALSTATE2)->SetWindowText(ret);
	UpdateWindowTitle();
}

void CTHScriptDebuggerDlg::UpdateWindowTitle()
{
	THString mode;
	if (m_nRunningState==1) mode=_T("[Running]");
	else if (m_nRunningState==2) mode=_T("[Break]");
	else if (m_nRunningState==3) mode=_T("[Finish]");
	SetWindowText(THFormat(_T("THScriptDebugger %s %s %s"),m_gset.m_sFile.IsEmpty()?_T(""):_T("-"),m_gset.m_sFile,mode));
}

void CTHScriptDebuggerDlg::OnBnClickedButton1()
{
	StartAsynCall(&CTHScriptDebuggerDlg::LoadScript);
}

void CTHScriptDebuggerDlg::LoadScript()
{
	// TODO: Add your control notification handler code here
	THString str=THSysDialog::FileDialogOpenExistOne(m_hWnd,_T(""),_T("THScript File(*.txt)|*.txt||"),_T("txt"),_T(""),_T("请选择加载的脚本文件"));
	if (!str.IsEmpty())
	{
		m_gset.m_sFile=str;//THSysMisc::GetFileName(str);
		m_nLastState=STATE_STEPINTO;
		m_nRunningState=1;
		UpdateWindowTitle();
		m_script.Init(str);
		m_nRunningState=3;
		m_gset.m_sReturnRet.Empty();
		UpdateWindowTitle();
		UpdateGlobalState();
		UpdateVar();
	}
}

void CTHScriptDebuggerDlg::OnBnClickedButton3()
{
	// TODO: Add your control notification handler code here
	StartAsynCall(&CTHScriptDebuggerDlg::StartDebug);
}

void CTHScriptDebuggerDlg::StartDebug()
{
	THString fn;
	THBaseInputDialog input;
	if (input.ShowInputDialog(&fn,_T("请输入函数名称"),_T("<Hint0>请输入函数名称：</Hint0><Hint1>请输入函数参数，以逗号分隔：</Hint1>"),theApp.m_hInstance,INPUTDIALOGTYPE_HINTUP|INPUTDIALOGTYPE_NOTCHECK,2,m_hWnd)==IDOK)
	{
		THStringToken t(THSimpleXml::GetParam(fn,_T("ret1")),_T(","));
		fn=THSimpleXml::GetParam(fn,_T("ret0"));
		if (!fn.IsEmpty())
		{
			THString arg;
			int idx=1;
			while(t.IsMoreTokens() && idx<=20)
			{
				arg+=THSimpleXml::MakeParam(_T("arg")+THi2s(idx++),t.GetNextToken());
			}
			m_nLastState=STATE_STEPINTO;
			m_nRunningState=1;
			UpdateWindowTitle();
			THString ret=m_script.RunScript(fn,m_nLocalIdx,arg);
			m_nRunningState=3;
			m_gset.m_sReturnRet=ret;
			UpdateWindowTitle();
			UpdateGlobalState();
			UpdateVar();
		}
	}
}

BOOL CTHScriptDebuggerDlg::OnScriptErrorHandler(int nErrCode,THString sErrText)
{
	//THDebug(_T("OnScriptErrorHandler:%d,%s"),nErrCode,sErrText);
	THString text;
	text.Format(_T("Script Error %d:%s with info:%s\r\nDo you want to ignore it?"),nErrCode,m_script.GetErrorText((THScript::enErrCode)nErrCode),sErrText);
	if (MessageBox(text,_T("ASSERT!"),MB_ICONQUESTION|MB_YESNO)==IDYES)
		return TRUE;
	return FALSE;
}

//当开始调用函数时触发，nRecursion表示递归次数，注意在Timer,Schedule及外部函数调用时，递归都由0开始计数
//返回FALSE表示退出该函数执行
BOOL CTHScriptDebuggerDlg::OnEnterFunction(THString sCurrentFn,int nRecursion,int localidx,THString *tempsetting)
{
	//THDebug(_T("OnEnterFunction:%s,%d,%d,%s"),sCurrentFn,nRecursion,localidx,*tempsetting);
	THString idx=THi2s(localidx)+_T(" ")+THi2s(nRecursion);
	THSimpleXml::SetParam(&m_sCallStack,_T("CallIdx")+idx,sCurrentFn);
	THSimpleXml::SetParam(&m_sCallStack,_T("ParamIdx")+idx,*tempsetting);
	m_nStepovering++;
	m_gset.m_sCurTempVar=*tempsetting;
	UpdateGlobalState();
	return TRUE;
}

//当结束调用函数时触发，sRet表示函数返回值
void CTHScriptDebuggerDlg::OnLeaveFunction(THString sCurrentFn,int nRecursion,THString *sRet,int localidx)
{
	//THDebug(_T("OnLeaveFunction:%s,%d,%d,%s"),sCurrentFn,nRecursion,localidx,*sRet);
	THString idx=THi2s(localidx)+_T(" ")+THi2s(nRecursion);
	THSimpleXml::SetParam(&m_sCallStack,_T("CallIdx")+idx,_T(""));
	THSimpleXml::SetParam(&m_sCallStack,_T("ParamIdx")+idx,_T(""));
	m_nStepovering--;
	m_gset.m_sCurTempVar.Empty();
	UpdateVar();
}

//返回FALSE表示不执行该句脚本
BOOL CTHScriptDebuggerDlg::OnRunScript(THString script,int localidx,THString *tempsetting)
{
	THDebug(_T("OnRunScript:%s,%d,%s"),script,localidx,*tempsetting);
	m_gset.m_sCurScript=script;
	m_gset.m_sCurTempVar=*tempsetting;
	UpdateGlobalState();
	UpdateVar();
	BOOL bBreak=FALSE;
	if (m_nLastState==STATE_RUN)
	{
	}
	else if (m_nLastState==STATE_RUNTORET)
	{
		THStringToken t(script,_T(":"));
		if (t.GetNextToken()==_T("end"))
			bBreak=TRUE;
	}
	else if (m_nLastState==STATE_STEPINTO)
	{
		bBreak=TRUE;
	}
	else if (m_nLastState==STATE_STEPOVER)
	{
		if (m_nStepovering<=0) bBreak=TRUE;
	}
	if (bBreak)
	{
		m_nRunningState=2;
		UpdateWindowTitle();
		UpdateVar();
		m_DebugEvent.WaitForObject();
		m_nRunningState=1;
		UpdateWindowTitle();
		if (m_nLastState==STATE_SKIP)
		{
			m_nLastState=STATE_STEPINTO;
			return FALSE;
		}
	}
	return TRUE;
}

//进入tag
void CTHScriptDebuggerDlg::OnEnterTag(THString fn,THString tag,THString *script)
{
	//THDebug(_T("OnEnterTag:%s,%s,%s"),fn,tag,*script);
	THString bp,bp2;
	bp.Format(_T("%d %s:%s"),m_nLocalIdx,fn,tag);
	bp2.Format(_T("%d :%s"),m_nLocalIdx,tag);
	if (THSimpleXml::GetParam(m_sBreakPoint,bp)==_T("1") ||
		THSimpleXml::GetParam(m_sBreakPoint,bp2)==_T("1"))
		m_nLastState=STATE_STEPINTO;
	m_gset.m_sFn=fn;
	m_gset.m_sTag=tag;
	m_gset.m_sScript=*script;
	m_gset.m_sScript.Replace(_T(";"),_T("\r\n"));
	UpdateGlobalState();
}

void CTHScriptDebuggerDlg::OnBnClickedButton6()
{
	THString fn;
	THBaseInputDialog input;
	if (input.ShowInputDialog(&fn,_T("设置全局变量"),_T("<Hint0>变量名称：</Hint0><Hint1>变量内容：</Hint1>"),theApp.m_hInstance,INPUTDIALOGTYPE_HINTLEFT|INPUTDIALOGTYPE_NOTCHECK,2,m_hWnd)==IDOK)
	{
		THString key=THSimpleXml::GetParam(fn,_T("ret0"));
		THString value=THSimpleXml::GetParam(fn,_T("ret1"));
		if (!key.IsEmpty())
		{
			m_script.SetGlobalSettings(key,value);
			UpdateVar();
		}
	}
}

void CTHScriptDebuggerDlg::OnBnClickedButton7()
{
	UpdateData();
	THString fn;
	THBaseInputDialog input;
	if (input.ShowInputDialog(&fn,_T("设置用户变量"),_T("<Hint0>变量名称：</Hint0><Hint1>变量内容：</Hint1>"),theApp.m_hInstance,INPUTDIALOGTYPE_HINTLEFT|INPUTDIALOGTYPE_NOTCHECK,2,m_hWnd)==IDOK)
	{
		THString key=THSimpleXml::GetParam(fn,_T("ret0"));
		THString value=THSimpleXml::GetParam(fn,_T("ret1"));
		if (!key.IsEmpty())
		{
			m_script.SetLocalSettings(m_nLocalIdx,key,value);
			UpdateVar();
		}
	}
}

void CTHScriptDebuggerDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	m_script.SetDebugger(NULL);
	m_script.FreeScript();
	OnCancel();
}

void CTHScriptDebuggerDlg::FreeScript()
{
	m_nLastState=STATE_STEPINTO;
	m_nRunningState=1;
	UpdateWindowTitle();
	m_script.FreeScript();
	m_nRunningState=3;
	m_gset.m_sReturnRet.Empty();
	UpdateWindowTitle();
	UpdateGlobalState();
	UpdateVar();
}

void CTHScriptDebuggerDlg::OnBnClickedButton9()
{
	//Run
	m_nLastState=STATE_RUN;
	Continue();
}

void CTHScriptDebuggerDlg::OnBnClickedButton13()
{
	//Run To Ret
	m_nLastState=STATE_RUNTORET;
	Continue();
}

void CTHScriptDebuggerDlg::OnBnClickedButton8()
{
	//Step Into
	m_nLastState=STATE_STEPINTO;
	Continue();
}

void CTHScriptDebuggerDlg::OnBnClickedButton10()
{
	//Step Over
	m_nLastState=STATE_STEPOVER;
	m_nStepovering=0;
	Continue();
}

void CTHScriptDebuggerDlg::Continue()
{
	//Continue
	m_DebugEvent.Pulse();
}

void CTHScriptDebuggerDlg::OnBnClickedButton14()
{
	//Skip
	m_nLastState=STATE_SKIP;
	Continue();
}

void CTHScriptDebuggerDlg::OnBnClickedButton2()
{
	StartAsynCall(&CTHScriptDebuggerDlg::FreeScript);
}

void CTHScriptDebuggerDlg::OnBnClickedButton11()
{
	UpdateVar();
}

void CTHScriptDebuggerDlg::OnBnClickedButton15()
{
	//call stack
	m_varslist.DeleteAllItems();
	int nRecursion=0;
	while(1)
	{
		THString idx=THi2s(m_nLocalIdx)+_T(" ")+THi2s(nRecursion++);
		THString fn=THSimpleXml::GetParam(m_sCallStack,_T("CallIdx")+idx);
		THString param=THSimpleXml::GetParam(m_sCallStack,_T("ParamIdx")+idx);
		if (fn.IsEmpty()) break;
		int item=m_varslist.InsertItem(m_varslist.GetItemCount(),idx);
		m_varslist.SetItemText(item,1,fn);
		m_varslist.SetItemText(item,2,param);
	}
}

void CTHScriptDebuggerDlg::OnBnClickedButton12()
{
	//break point
	m_varslist.DeleteAllItems();
	InsertVarItem(_T("断点"),m_sBreakPoint);
}

void CTHScriptDebuggerDlg::OnBnClickedButton16()
{
	THString fn;
	THBaseInputDialog input;
	if (input.ShowInputDialog(&fn,_T("请输入断点"),_T("<Hint0>请输入断点函数名称，为空为所有匹配的标号都中断：</Hint0><Hint1>请输入断点标号，为空为函数起始位置，即start：</Hint1>"),theApp.m_hInstance,INPUTDIALOGTYPE_HINTUP|INPUTDIALOGTYPE_NOTCHECK,2,m_hWnd)==IDOK)
	{
		THString tag=THSimpleXml::GetParam(fn,_T("ret1"));
		fn=THSimpleXml::GetParam(fn,_T("ret0"));
		if (!fn.IsEmpty() || !tag.IsEmpty())
		{
			if (tag.IsEmpty()) tag=_T("start");
			THString bp;
			bp.Format(_T("%d %s:%s"),m_nLocalIdx,fn,tag);
			THSimpleXml::SetParam(&m_sBreakPoint,bp,_T("1"));//fixme disable bp,edit vars
		}
	}
	OnBnClickedButton12();
}

void CTHScriptDebuggerDlg::OnBnClickedButton17()
{
	//empty bp
	m_sBreakPoint.Empty();
	OnBnClickedButton12();
}
