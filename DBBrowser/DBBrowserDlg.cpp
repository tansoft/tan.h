// DBBrowserDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "DBBrowser.h"
#include "DBBrowserDlg.h"
#include ".\dbbrowserdlg.h"

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


// CDBBrowserDlg 对话框



CDBBrowserDlg::CDBBrowserDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDBBrowserDlg::IDD, pParent)
	, m_sql(_T(""))
	, m_ip(_T(""))
	, m_db(_T(""))
	, m_usr(_T(""))
	, m_pas(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pdb = NULL;
}

void CDBBrowserDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT2, m_sql);
	DDX_Control(pDX,IDC_LIST1,m_list);
	DDX_Text(pDX, IDC_EDIT1, m_ip);
	DDX_Text(pDX, IDC_EDIT3, m_db);
	DDX_Text(pDX, IDC_EDIT4, m_usr);
	DDX_Text(pDX, IDC_EDIT5, m_pas);
}

BEGIN_MESSAGE_MAP(CDBBrowserDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
	ON_BN_CLICKED(IDOK2, OnBnClickedOk2)
	ON_BN_CLICKED(IDC_BUTTON3, OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON2, OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON4, OnBnClickedButton4)
END_MESSAGE_MAP()


// CDBBrowserDlg 消息处理程序

BOOL CDBBrowserDlg::OnInitDialog()
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
	
	return TRUE;  // 除非设置了控件的焦点，否则返回 TRUE
}

void CDBBrowserDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CDBBrowserDlg::OnPaint() 
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
HCURSOR CDBBrowserDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CDBBrowserDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	//OnOK();
	UpdateData();
	DWORD ti=GetTickCount();
	if (m_query.StartQuery(m_sql))
	{
		THString str;
		str.Format(_T("%d 条记录，用时 %0.3f 秒。"),m_query.GetRecordCount(),(GetTickCount()-ti)/1000.0);
		GetDlgItem(IDC_STATICDISPLAY)->SetWindowText(str);
	}
	else
		GetDlgItem(IDC_STATICDISPLAY)->SetWindowText(_T("请求出错！"));
}

void CDBBrowserDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	UpdateData();
	if (m_pdb)
	{
		m_pdb->CloseDataBase();
		m_pdb->ReleaseObject(m_pdb);
	}
	m_pdb=new THDBOdbc();
	if (m_pdb)
	{
		U16 nPort=0;
		THString host=THNetWork::GetHostFromHostName(m_ip,&nPort);
		if (nPort==0) nPort=1433;
		if (m_pdb->OpenDataBase(_T("SQL Server"),host,nPort,m_db,m_usr,m_pas))
		{
			GetDlgItem(IDC_STATICDISPLAY)->SetWindowText(_T("数据库已打开。"));
			m_query.SetDataBase(m_pdb);
			m_list.Init(&m_query,_T(""),-1,TRUE);
		}
		else
			GetDlgItem(IDC_STATICDISPLAY)->SetWindowText(_T("数据库打开错误！"));
	}
	else
		GetDlgItem(IDC_STATICDISPLAY)->SetWindowText(_T("数据库打开错误！"));
}

void CDBBrowserDlg::OnBnClickedOk2()
{
	// TODO: Add your control notification handler code here
	UpdateData();
	if (!m_pdb->ExecuteSql(m_sql))
		GetDlgItem(IDC_STATICDISPLAY)->SetWindowText(_T("执行Sql语句出错！"));
}

void CDBBrowserDlg::OnBnClickedButton3()
{
	// TODO: Add your control notification handler code here
	UpdateData();
	THString file=THSysDialog::FileDialogOpenExistOne(m_hWnd,_T(""),_T("配置文件(*.ini)|*.ini||"),_T("ini"),_T(""),_T("请选择配置文件"));
	if (file)
	{
		THIni ini;
		if (ini.Init(file))
		{
			m_ip=ini.GetString(_T("DataBase"),_T("ip"));
			m_db=ini.GetString(_T("DataBase"),_T("db"));
			m_usr=ini.GetString(_T("DataBase"),_T("user"));
			m_pas=ini.GetString(_T("DataBase"),_T("pass"));
			UpdateData(FALSE);
		}
	}
}

void CDBBrowserDlg::OnBnClickedButton2()
{
	// TODO: Add your control notification handler code here
	UpdateData();
	THString file=THSysDialog::FileDialogSaveOneOverWritePrompt(m_hWnd,_T(""),_T("配置文件(*.ini)|*.ini||"),_T("ini"),_T(""),_T("请选择配置文件"));
	if (file)
	{
		THIni ini;
		if (ini.Init(file))
		{
			ini.WriteString(_T("DataBase"),_T("ip"),m_ip);
			ini.WriteString(_T("DataBase"),_T("db"),m_db);
			ini.WriteString(_T("DataBase"),_T("user"),m_usr);
			ini.WriteString(_T("DataBase"),_T("pass"),m_pas);
			UpdateData(FALSE);
		}
	}
}

void CDBBrowserDlg::OnBnClickedButton4()
{
	// TODO: Add your control notification handler code here
	//export
	THString strFile=THSysDialog::FileDialogSaveOneOverWritePrompt(m_hWnd,_T(""),_T("Excel CSV 格式(*.csv)|*.csv|文本格式(*.txt)|*.txt||"),_T(".csv"),_T(""),_T("请输入导出文件的文件名"));
	if (!strFile.IsEmpty())
		if (m_list.ExportToCsv(strFile,TRUE))
		{
			GetDlgItem(IDC_STATICDISPLAY)->SetWindowText(_T("导出完成！"));
			if (MessageBox(_T("导出成功，现在就要打开文件吗？"),_T("恭喜"),MB_ICONQUESTION|MB_YESNO)==IDYES)
				ShellExecute(m_hWnd,_T("open"),strFile,NULL,NULL,SW_SHOW);
		}
		else
			GetDlgItem(IDC_STATICDISPLAY)->SetWindowText(_T("导出错误！"));
}
