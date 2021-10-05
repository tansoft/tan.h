// THScriptDebuggerDlg.cpp : ʵ���ļ�
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


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// CTHScriptDebuggerDlg �Ի���



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


// CTHScriptDebuggerDlg ��Ϣ�������

BOOL CTHScriptDebuggerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ��\������...\���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	m_script.SetDebugger(this);
	m_DebugEvent.Create(NULL,TRUE,FALSE,NULL);

	m_varslist.InsertColumn(0,_T("����"),LVCFMT_LEFT,60);
	m_varslist.InsertColumn(1,_T("����"),LVCFMT_LEFT,100);
	m_varslist.InsertColumn(2,_T("ֵ"),LVCFMT_LEFT,250);
	m_varslist.SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);

	m_scriptlist.InsertColumn(0,_T("����"),LVCFMT_LEFT,60);
	m_scriptlist.InsertColumn(1,_T("ֵ"),LVCFMT_LEFT,200);
	m_scriptlist.InsertColumn(2,_T("����ֵ"),LVCFMT_LEFT,150);
	m_scriptlist.SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);

	return TRUE;  // ���������˿ؼ��Ľ��㣬���򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CTHScriptDebuggerDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ��������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù����ʾ��
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
		InsertVarItem(_T("���ر���"),m_gset.m_sCurTempVar);
	}
	InsertVarItem(_T("ȫ�ֱ���"),m_script.GetGlobalSettings(_T("")));
	InsertVarItem(_T("�û�����"),m_script.GetLocalSettings(m_nLocalIdx,_T("")));
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
	InsertVarItem(_T("ϵͳ����"),ret);
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
				stype=_T("����");
			else
				stype.Format(_T("����%d"),idx);
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
		ret.Format(_T("������%s\r\n��ţ�%s\r\n��ǰ�ű���\r\n%s\r\n"),m_gset.m_sFn,m_gset.m_sTag,m_gset.m_sCurScript);
	else if (m_nRunningState==3)
		ret.Format(_T("�������أ�%s\r\n"),m_gset.m_sReturnRet);
	GetDlgItem(IDC_GLOBALSTATE)->SetWindowText(ret);
	if (m_nRunningState==1 || m_nRunningState==2)
		ret.Format(_T("�ű���\r\n%s\r\n"),m_gset.m_sScript);
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
	THString str=THSysDialog::FileDialogOpenExistOne(m_hWnd,_T(""),_T("THScript File(*.txt)|*.txt||"),_T("txt"),_T(""),_T("��ѡ����صĽű��ļ�"));
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
	if (input.ShowInputDialog(&fn,_T("�����뺯������"),_T("<Hint0>�����뺯�����ƣ�</Hint0><Hint1>�����뺯���������Զ��ŷָ���</Hint1>"),theApp.m_hInstance,INPUTDIALOGTYPE_HINTUP|INPUTDIALOGTYPE_NOTCHECK,2,m_hWnd)==IDOK)
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

//����ʼ���ú���ʱ������nRecursion��ʾ�ݹ������ע����Timer,Schedule���ⲿ��������ʱ���ݹ鶼��0��ʼ����
//����FALSE��ʾ�˳��ú���ִ��
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

//���������ú���ʱ������sRet��ʾ��������ֵ
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

//����FALSE��ʾ��ִ�иþ�ű�
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

//����tag
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
	if (input.ShowInputDialog(&fn,_T("����ȫ�ֱ���"),_T("<Hint0>�������ƣ�</Hint0><Hint1>�������ݣ�</Hint1>"),theApp.m_hInstance,INPUTDIALOGTYPE_HINTLEFT|INPUTDIALOGTYPE_NOTCHECK,2,m_hWnd)==IDOK)
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
	if (input.ShowInputDialog(&fn,_T("�����û�����"),_T("<Hint0>�������ƣ�</Hint0><Hint1>�������ݣ�</Hint1>"),theApp.m_hInstance,INPUTDIALOGTYPE_HINTLEFT|INPUTDIALOGTYPE_NOTCHECK,2,m_hWnd)==IDOK)
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
	InsertVarItem(_T("�ϵ�"),m_sBreakPoint);
}

void CTHScriptDebuggerDlg::OnBnClickedButton16()
{
	THString fn;
	THBaseInputDialog input;
	if (input.ShowInputDialog(&fn,_T("������ϵ�"),_T("<Hint0>������ϵ㺯�����ƣ�Ϊ��Ϊ����ƥ��ı�Ŷ��жϣ�</Hint0><Hint1>������ϵ��ţ�Ϊ��Ϊ������ʼλ�ã���start��</Hint1>"),theApp.m_hInstance,INPUTDIALOGTYPE_HINTUP|INPUTDIALOGTYPE_NOTCHECK,2,m_hWnd)==IDOK)
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
