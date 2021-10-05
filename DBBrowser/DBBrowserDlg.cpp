// DBBrowserDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "DBBrowser.h"
#include "DBBrowserDlg.h"
#include ".\dbbrowserdlg.h"

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


// CDBBrowserDlg �Ի���



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


// CDBBrowserDlg ��Ϣ�������

BOOL CDBBrowserDlg::OnInitDialog()
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
	
	return TRUE;  // ���������˿ؼ��Ľ��㣬���򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CDBBrowserDlg::OnPaint() 
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
		str.Format(_T("%d ����¼����ʱ %0.3f �롣"),m_query.GetRecordCount(),(GetTickCount()-ti)/1000.0);
		GetDlgItem(IDC_STATICDISPLAY)->SetWindowText(str);
	}
	else
		GetDlgItem(IDC_STATICDISPLAY)->SetWindowText(_T("�������"));
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
			GetDlgItem(IDC_STATICDISPLAY)->SetWindowText(_T("���ݿ��Ѵ򿪡�"));
			m_query.SetDataBase(m_pdb);
			m_list.Init(&m_query,_T(""),-1,TRUE);
		}
		else
			GetDlgItem(IDC_STATICDISPLAY)->SetWindowText(_T("���ݿ�򿪴���"));
	}
	else
		GetDlgItem(IDC_STATICDISPLAY)->SetWindowText(_T("���ݿ�򿪴���"));
}

void CDBBrowserDlg::OnBnClickedOk2()
{
	// TODO: Add your control notification handler code here
	UpdateData();
	if (!m_pdb->ExecuteSql(m_sql))
		GetDlgItem(IDC_STATICDISPLAY)->SetWindowText(_T("ִ��Sql������"));
}

void CDBBrowserDlg::OnBnClickedButton3()
{
	// TODO: Add your control notification handler code here
	UpdateData();
	THString file=THSysDialog::FileDialogOpenExistOne(m_hWnd,_T(""),_T("�����ļ�(*.ini)|*.ini||"),_T("ini"),_T(""),_T("��ѡ�������ļ�"));
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
	THString file=THSysDialog::FileDialogSaveOneOverWritePrompt(m_hWnd,_T(""),_T("�����ļ�(*.ini)|*.ini||"),_T("ini"),_T(""),_T("��ѡ�������ļ�"));
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
	THString strFile=THSysDialog::FileDialogSaveOneOverWritePrompt(m_hWnd,_T(""),_T("Excel CSV ��ʽ(*.csv)|*.csv|�ı���ʽ(*.txt)|*.txt||"),_T(".csv"),_T(""),_T("�����뵼���ļ����ļ���"));
	if (!strFile.IsEmpty())
		if (m_list.ExportToCsv(strFile,TRUE))
		{
			GetDlgItem(IDC_STATICDISPLAY)->SetWindowText(_T("������ɣ�"));
			if (MessageBox(_T("�����ɹ������ھ�Ҫ���ļ���"),_T("��ϲ"),MB_ICONQUESTION|MB_YESNO)==IDYES)
				ShellExecute(m_hWnd,_T("open"),strFile,NULL,NULL,SW_SHOW);
		}
		else
			GetDlgItem(IDC_STATICDISPLAY)->SetWindowText(_T("��������"));
}
