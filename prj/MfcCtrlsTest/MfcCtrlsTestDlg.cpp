// MfcCtrlsTestDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "MfcCtrlsTest.h"
#include "MfcCtrlsTestDlg.h"
#include <THString.h>
#include ".\mfcctrlstestdlg.h"
#include <THDebug.h>
#include <THDatabase.h>

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


// CMfcCtrlsTestDlg �Ի���



CMfcCtrlsTestDlg::CMfcCtrlsTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMfcCtrlsTestDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMfcCtrlsTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_List);
	DDX_Control(pDX, IDC_STATIC1, m_panel);
	DDX_Control(pDX, IDC_STATIC2, m_static);
	DDX_Control(pDX, IDC_EDIT1, m_edit);
	DDX_Control(pDX, IDC_DATETIMEPICKER1, m_date);
	DDX_Control(pDX, IDC_COMBO1, m_mcombo);
	DDX_Control(pDX, IDC_COMBO2, m_combo);
	DDX_Control(pDX, IDC_LIST2, m_listbox);
}

BEGIN_MESSAGE_MAP(CMfcCtrlsTestDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, OnBnClickedButton2)
END_MESSAGE_MAP()


// CMfcCtrlsTestDlg ��Ϣ�������

BOOL CMfcCtrlsTestDlg::OnInitDialog()
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

	THString sHost,sigrdb,sUser,sPass;
	int nPort=4431;
	sHost=_T("61.235.64.96");
	sUser=_T("sa");
	sPass=_T("zhxit");
	THDBOdbc m_dbf21;
	THMulDataBase m_dbs;
	if (!m_dbf21.OpenDataBase(_T("SQL Server"),sHost,nPort,_T("f18master"),sUser,sPass))
	{
		MessageBox(_T("���ܴ�F21�������ݿ⣡"),_T("���д���"),MB_OK|MB_ICONSTOP);
	}
	THString disdb;
	ITHRecordSet *set=m_dbf21.StartQuery(TRUE,_T("select dbalia,f18name from f18books"));
	ITHDataBase *pdb;
	if (!set)
	{
		MessageBox(_T("���ܴ�F21�������ݿ⣡"),_T("���д���"),MB_OK|MB_ICONSTOP);
		exit(0);
	}
	THString dbs;
	while(!set->IsEOF())
	{
		dbs=set->GetColValue(0).Trim();
		pdb=m_dbf21.NewObject();
		if (pdb)
		{
			if (!pdb->OpenDataBase(_T("SQL Server"),sHost,nPort,dbs,sUser,sPass))
			{
				MessageBox(_T("���ܴ������ݿ�")+dbs+_T("��"),_T("���д���"),MB_OK|MB_ICONSTOP);
				m_dbf21.ReleaseObject(pdb);
			}
			else
			{
				disdb+=set->GetColValue(1).Trim()+_T(" ");
				m_dbs.AddDataBase(pdb);
			}
		}
		set->MoveNext();
	}
	m_dbf21.EndQuery(set);
	ITHRecordSet *set1=m_dbs.StartQuery(TRUE,_T("select clothingid from j_clothing where clothingid like '%1234%'"));
	//_T("select drName from j_brand"));
	//_T("select names from j_company"));
	if (set1)
	{
		while(!set1->IsEOF())
		{
			THDebug(set1->GetColValue(0));
			set1->MoveNext();
		}
		m_dbs.EndQuery(set1);
	}

	m_database.OpenDataBase(_T("SQL Server"),_T("127.0.0.1"),1433,_T("fdb"),_T("abc"),_T("123456"));

	THString tmpstr;
	//ITHDataBase *pdb;
	for(int i=0;i<4;i++)
	{
		tmpstr.Format(_T("clF21_%d"),i+1);
		pdb=m_database.NewObject();
		if (!pdb->OpenDataBase(_T("SQL Server"),_T("127.0.0.1"),1433,tmpstr,_T("abc"),_T("123456")))
		{
			m_database.ReleaseObject(pdb);
		}
		else
			m_dbs.AddDataBase(pdb);
	}

	set=m_dbs.StartQuery(TRUE,_T("select top 20 clothingid from j_clothing where clothingid like '%%v%%'"));
	//_T("select names from j_company"));
	if (set)
	{
		while(!set->IsEOF())
		{
			THDebug(set->GetColValue(0));
			set->MoveNext();
		}
		m_dbs.EndQuery(set);
	}

	set=m_dbs.StartQuery(TRUE,_T("select top 20 clothingid from j_clothing where clothingid like '%N63361099245%'"));
	if (set)
	{
		if (set->GetRecordCount()>0)
		{
			while(!set->IsEOF())
			{
				THDebug(set->GetColValue(0));
				set->MoveNext();
			}
		}
		m_dbs.EndQuery(set);
	}
	//m_database.OpenDataBase(_T("SQL Server"),_T("192.168.0.208"),_T("1433"),_T("fdb"),_T("abc"),_T("123456"));
	m_query.SetDataBase(&m_database);
	m_panel.Init(&m_query);
	m_static.Init(&m_query,0,_T("21:"));
	m_List.Init(&m_query,_T("300|100"));
	//m_edit.Init(&m_query,3,FALSE,TRUE);
	//m_date.Init(&m_query,0);
	m_mcombo.Init(&m_query,0);
	m_combo.Init(&m_query,0,FALSE,FALSE,TRUE,FALSE);
	m_listbox.Init(&m_query,0);

	//if (!m_query.StartQuery(_T("select id,storeparent,storename from sysstore"),_T("sysstore|id,storeparent,storename|d,d,s")))
	if (!m_query.StartQuery(_T("select id,age from [user]"),_T("[user]|age|d,d")))
	{
		MessageBox(_T("��ѯ���ݿ����"));
	}

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
//	DWORD nStyle = WS_CHILD|WS_VISIBLE|WS_BORDER|LVS_REPORT|LVS_AUTOARRANGE|LVS_SHOWSELALWAYS;
//	CRect rect(0,0,300,200);
//	m_List.Create(nStyle,rect,this,100);
//	m_List.SetExtendedStyle(m_List.GetExtendedStyle()|LVS_EX_FULLROWSELECT);
/*	m_List.InsertColumn(0,_T("123"),LVCFMT_LEFT,100);
	m_List.InsertColumn(0,_T("32123"),LVCFMT_LEFT,100);
	m_List.InsertColumn(0,_T("15421123"),LVCFMT_LEFT,100);
	for(int i=0;i<10;i++)
	{
		THString str;
		str.Format(_T("test%d"),rand());
		int idx=m_List.InsertItem(m_List.GetItemCount(),str);
		str.Format(_T("test%d"),rand());
		m_List.SetItemText(idx,1,str);
		str.Format(_T("test%d"),rand());
		m_List.SetItemText(idx,2,str);
	}
*/	
	return TRUE;  // ���������˿ؼ��Ľ��㣬���򷵻� TRUE
}

void CMfcCtrlsTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CMfcCtrlsTestDlg::OnPaint() 
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
HCURSOR CMfcCtrlsTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMfcCtrlsTestDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: Add your message handler code here
}

void CMfcCtrlsTestDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	m_List.InsertColumn(0,_T("321"),LVCFMT_LEFT,100);
	m_List.InsertColumn(1,_T("321"),LVCFMT_LEFT,100);
	m_List.InsertColumn(2,_T("321"),LVCFMT_LEFT,100);
	m_List.InsertColumn(3,_T("321"),LVCFMT_LEFT,100);
	m_List.InsertColumn(4,_T("321"),LVCFMT_LEFT,100);
	m_List.InsertColumn(5,_T("321"),LVCFMT_LEFT,100);
	m_List.InsertColumn(6,_T("321"),LVCFMT_LEFT,100);
	m_List.InsertColumn(7,_T("321"),LVCFMT_LEFT,100);
	m_List.InsertColumn(8,_T("321"),LVCFMT_LEFT,100);
	m_List.InsertColumn(9,_T("321"),LVCFMT_LEFT,100);
	for(int i=0;i<10;i++)
	{
		int idx=m_List.InsertItem(m_List.GetItemCount(),THi2s(rand()));
		for(int j=1;j<10;j++)
		{
			m_List.SetItemText(idx,j,THi2s(rand()));
		}
	}
}

void CMfcCtrlsTestDlg::OnBnClickedButton2()
{
	// TODO: Add your control notification handler code here
	m_query.ChangeSelection(-1);
}
