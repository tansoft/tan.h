// HttpServerDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "HttpServer.h"
#include "HttpServerDlg.h"
#include ".\httpserverdlg.h"

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


// CHttpServerDlg �Ի���



CHttpServerDlg::CHttpServerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CHttpServerDlg::IDD, pParent)
	/*, m_sFilePath(_T(""))
	, m_sImgFile(_T(""))
	, m_nFileStoreType(0)*/
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	//m_bRunState=FALSE;
}

void CHttpServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATELIGHT, m_stsLight);
	//DDX_Text(pDX, IDC_EDIT3, m_sFilePath);
	//DDX_Text(pDX, IDC_EDIT4, m_sImgFile);
	//DDX_Radio(pDX, IDC_RADIO1, m_nFileStoreType);
}

BEGIN_MESSAGE_MAP(CHttpServerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BTNRUN, OnBnClickedBtnrun)
//	ON_BN_CLICKED(IDC_RADIO1, OnBnClickedRadio1)
//	ON_BN_CLICKED(IDC_RADIO2, OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_BTNRUN2, OnBnClickedBtnrun2)
	ON_WM_TIMER()
	ON_MESSAGE(WM_TRAY_MESSAGE,OnTrayIconMsg)
	ON_COMMAND(ID_TRAY_EXIT,OnTrayExit)
END_MESSAGE_MAP()


// CHttpServerDlg ��Ϣ�������

BOOL CHttpServerDlg::OnInitDialog()
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
	CRgn rgn;
	rgn.CreateRectRgn(0,0,0,0);
	SetWindowRgn(rgn,TRUE);

	m_stsLight.SetIcon((HICON)::LoadImage(::AfxGetInstanceHandle(),MAKEINTRESOURCE(IDI_ICONOFF),IMAGE_ICON,16,16,0));

	m_trayicon.SetIcon(m_hIcon);
	m_trayicon.SetMenuId(IDR_MENU1);
	m_trayicon.SetNotifyWnd(m_hWnd,WM_TRAY_MESSAGE);
	m_trayicon.SetToolTip(_T("����΢��Web������ 1.3"));
	m_trayicon.ShowTray();
	SetWindowText(_T("����΢��Web������ 1.3"));

	theApp.m_server.m_bUsePlugin=TRUE;//theApp.m_ini.GetBool(_T("THHttpServerSettings"),_T("bUse)
	theApp.m_server.InitPlugin(theApp.m_sExePath+_T("\\plugins"));
	/*	THString cmd=THCharset::a2t(GetCommandLine());
	if (cmd.Find(_T("/start"),0)!=-1)
	{
		//autorun checking
	}*/
	if (theApp.m_ini.GetBool(_T("THHttpServerSettings"),_T("bAutoStart"))) OnBnClickedBtnrun();

	SetTimer(1,50,NULL);

	return TRUE;  // ���������˿ؼ��Ľ��㣬���򷵻� TRUE
}

void CHttpServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CHttpServerDlg::OnPaint() 
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
HCURSOR CHttpServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CHttpServerDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	//OnOK();
}

void CHttpServerDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	//OnCancel();
	ShowWindow(SW_HIDE);
}
/*
void CHttpServerDlg::UpdateAllData(BOOL bUpdate)
{
	UpdateData(bUpdate);
	if (IsWindow(m_set->m_base.GetSafeHwnd())) m_set->m_base.UpdateData(bUpdate);
	if (IsWindow(m_set->m_directory.GetSafeHwnd())) m_set->m_directory.UpdateData(bUpdate);
	if (IsWindow(m_set->m_auth.GetSafeHwnd())) m_set->m_auth.UpdateData(bUpdate);
	if (IsWindow(m_set->m_formance.GetSafeHwnd())) m_set->m_formance.UpdateData(bUpdate);
	if (IsWindow(m_set->m_upload.GetSafeHwnd())) m_set->m_upload.UpdateData(bUpdate);
	if (IsWindow(m_set->m_script.GetSafeHwnd())) m_set->m_script.UpdateData(bUpdate);
	if (IsWindow(m_set->m_log.GetSafeHwnd())) m_set->m_log.UpdateData(bUpdate);
	if (IsWindow(m_set->m_mulservice.GetSafeHwnd())) m_set->m_mulservice.UpdateData(bUpdate);
	if (IsWindow(m_set->m_custom.GetSafeHwnd())) m_set->m_custom.UpdateData(bUpdate);
	if (IsWindow(m_set->m_plugin.GetSafeHwnd())) m_set->m_plugin.UpdateData(bUpdate);
}
*/
void CHttpServerDlg::OnBnClickedBtnrun()
{
	if (theApp.m_server.IsStarted())
	{
		theApp.m_server.StopListen();
		theApp.StopAllSchedule();
	}
	else
	{
		theApp.m_server.LoadSettings(&theApp.m_ini);
		theApp.StartLogSchedule();
		int ListenPort=theApp.m_ini.GetInt(_T("THHttpServerSettings"),_T("nListenPort"),80);
		int DynIpSid=theApp.m_ini.GetInt(_T("THHttpServerSettings"),_T("nDynIpSid"),0);
		BOOL bHttpsService=theApp.m_ini.GetBool(_T("THHttpServerSettings"),_T("bHttpsService"));
		BOOL bStartHide=theApp.m_ini.GetBool(_T("THHttpServerSettings"),_T("bStartHide"));
		THString sFilePath;
		if (sFilePath.IsEmpty()) sFilePath=theApp.m_sExePath;
		if (bHttpsService)
		{
			if (!theApp.m_server.SetupHttpsServer(theApp.m_sExePath+_T("\\plugins\\127.0.0.1.pem"),theApp.m_sExePath+_T("\\plugins\\127.0.0.1.key")))
			{
				MessageBox(_T("û����ʼ��������ȷ��Https������PluginHttpsServer.plg��Openssl��������֤���Ƿ���ȷ��"),_T("��������"),MB_OK|MB_ICONSTOP);
				return;
			}
		}
		//fixme for resfile temp testing
		/*THResFileStore fs;
		ITHFileStore *pfs=fs.CreateObject();
		THString sPath=pfs->BaseDirDialog(m_hWnd,_T("fre"));
		pfs->SetBaseDirectory(sPath);
		if (!theApp.m_server.StartListen(ListenPort,DynIpSid,pfs,TRUE))*/

		//for test
		/*ITHFileStore *store;
		store=new THFileStore;
		store->SetBaseDirectory(_T("c:\\v"));
		theApp.m_server.AddVirtualDirMapping(_T("av1"),store,TRUE);
		store=new THFileStore;
		store->SetBaseDirectory(_T("d:\\v"));
		theApp.m_server.AddVirtualDirMapping(_T("av2"),store,TRUE);
		store=new THFileStore;
		store->SetBaseDirectory(_T("e:\\v"));
		theApp.m_server.AddVirtualDirMapping(_T("av3"),store,TRUE);*/

		if (!theApp.m_server.StartListen(ListenPort,DynIpSid,sFilePath))
			MessageBox(_T("û����ʼ��������ȷ�϶˿�û�б�ռ�ã�"),_T("��������"),MB_OK|MB_ICONSTOP);
	}
	m_stsLight.SetIcon((HICON)::LoadImage(::AfxGetInstanceHandle(),MAKEINTRESOURCE(theApp.m_server.IsStarted()?IDI_ICONON:IDI_ICONOFF),IMAGE_ICON,16,16,0));
	GetDlgItem(IDC_BTNRUN)->SetWindowText(theApp.m_server.IsStarted()?_T("ֹͣ(&S)"):_T("����(&S)"));
	// TODO: Add your control notification handler code here
	//UpdateAllData();
/*	if (m_bRunState==FALSE)
	{
		if (m_nFileStoreType==0)
		{
			m_server.m_bAuth=TRUE;
			m_server.m_bDigestMode=TRUE;
			m_server.m_aAccountList.Add(_T("barry:810918"));
			m_server.m_bLimitIp=TRUE;
			m_server.m_allowrange.AddIpRange(_T("127.0.0.1"),_T("255.255.255.0"));
			//m_server.m_blockrange.AddIpRange(_T("127.0.0.1"),_T("255.255.255.255"));
			THFileStore *store=new THFileStore;
			store->SetBaseDirectory(_T("c:\\"));
			m_server.AddHostMapping(_T("test.com"),store,TRUE);
			store=new THFileStore;
			store->SetBaseDirectory(_T("e:\\"));
			m_server.AddVirtualDirMapping(_T("cdr"),store,TRUE);
			//m_server.m_bUseSession=TRUE;
			//m_server.m_bSessionCookielessMode=TRUE;
			if (!m_server.StartListen(m_set->m_base.m_nListenPort,m_set->m_base.m_nDynIpSid,m_sFilePath))
				return;
		}
		else
			//fixme,img file support
			m_server.StartListen(m_set->m_base.m_nListenPort,m_set->m_base.m_nDynIpSid,m_sFilePath);
	}
	else
		m_server.StopListen();
	m_bRunState=!m_bRunState;
	//m_stsLight.ModifyStyle(0x0,SS_ICON|SS_CENTERIMAGE);
	m_stsLight.SetIcon((HICON)::LoadImage(::AfxGetInstanceHandle(),MAKEINTRESOURCE(m_bRunState?IDI_ICONON:IDI_ICONOFF),IMAGE_ICON,16,16,0));
	//m_stsLight.SetIcon(theApp.LoadIcon(m_bRunState?IDI_ICONON:IDI_ICONOFF));
	GetDlgItem(IDC_BTNRUN)->SetWindowText(m_bRunState?_T("ֹͣ(&S)"):_T("����(&S)"));*/
}
/*
void CHttpServerDlg::OnBnClickedRadio1()
{
	// TODO: Add your control notification handler code here
	UpdateData();
//	GetDlgItem(IDC_EDIT3)->EnableWindow(m_nFileStoreType==0);
//	GetDlgItem(IDC_BUTTON2)->EnableWindow(m_nFileStoreType==0);
//	GetDlgItem(IDC_EDIT4)->EnableWindow(m_nFileStoreType==1);
//	GetDlgItem(IDC_BUTTON3)->EnableWindow(m_nFileStoreType==1);
}
*/
void CHttpServerDlg::OnBnClickedBtnrun2()
{
	// TODO: Add your control notification handler code here
/*	m_set=new CSettings(_T("settings"),this);
	//WS_EX_CONTROLPARENT ʹCPropertySheet��ΪDialog���Ӵ��ڡ�
	m_set->Create(this,WS_CHILD|WS_VISIBLE,WS_EX_CONTROLPARENT);
	m_set->ShowWindow(SW_SHOW);*/
	CSettings settings(_T("��������"),this);
	if (settings.DoModal()==IDOK)
		theApp.m_server.SaveSettings(&theApp.m_ini);
}

void CHttpServerDlg::OnTimer(UINT nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if (nIDEvent==1)
	{
		if (theApp.m_ini.GetBool(_T("THHttpServerSettings"),_T("bStartHide")))
			ShowWindow(SW_HIDE);
		SetWindowRgn(NULL,TRUE);
		KillTimer(1);
	}

	CDialog::OnTimer(nIDEvent);
}

LRESULT CHttpServerDlg::OnTrayIconMsg(WPARAM wParam,LPARAM lParam)
{
	return m_trayicon.OnParseDefaultMessage(wParam,lParam);
}

LRESULT CHttpServerDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: Add your specialized code here and/or call the base class
	m_trayicon.UpdateRebuildState(message);
	return CDialog::DefWindowProc(message, wParam, lParam);
}

void CHttpServerDlg::OnTrayExit()
{
	theApp.m_server.StopListen();
	OnCancel();
}
