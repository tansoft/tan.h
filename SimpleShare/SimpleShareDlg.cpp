// SimpleShareDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "SimpleShare.h"
#include "SimpleShareDlg.h"
#include ".\simplesharedlg.h"

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


// CSimpleShareDlg �Ի���



CSimpleShareDlg::CSimpleShareDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSimpleShareDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSimpleShareDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_box);
}

BEGIN_MESSAGE_MAP(CSimpleShareDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON6, OnBnClickedButton6)
END_MESSAGE_MAP()


// CSimpleShareDlg ��Ϣ�������

BOOL CSimpleShareDlg::OnInitDialog()
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
	m_input.SetEventHandler(this);
	OnBnClickedOk();

	return TRUE;  // ���������˿ؼ��Ľ��㣬���򷵻� TRUE
}

void CSimpleShareDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CSimpleShareDlg::OnPaint() 
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
HCURSOR CSimpleShareDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CSimpleShareDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	StartService();
	//OnOK();
}
#include <THDebug.h>
void CSimpleShareDlg::HandleMessage(THStringA str,U8 cmd,U16 seq,U32 peerip,U16 peerport)
{
	if (cmd==THSIMPLESHARE_CMD_BOARDCAST)
	{
		int cnt=m_box.GetCount();
		int idx=-1;
		for(int i=0;i<cnt;i++)
		{
			if (m_box.GetItemData(i)==peerip)
				idx=i;
		}
		THString distext;
		THStringToken t(str,_T(","));
		static THNetWork net;
		if (net.IsLocalIpN(peerip))
			distext=_T("127.0.0.1");
		else
			distext.Format(_T("%s"),THNetWork::GetAddrStringN(peerip));
		distext.AppendFormat(_T(":%d,h:%s,t:%s"),ntohs(peerport),t.GetNextToken(),t.GetNextToken());
		if (idx==-1)
		{
			idx=m_box.AddString(distext);
			m_box.SetItemData(idx,peerip);
		}
		else
		{
			THDebug(_T("Delete:%d"),idx);
			m_box.DeleteString(idx);
			idx=m_box.InsertString(idx,distext);
			m_box.SetItemData(idx,peerip);
		}
	}
	THSimpleShare::HandleMessage(str,cmd,seq,peerip,peerport);
}

void CSimpleShareDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	//openh
	int i=m_box.GetCurSel();
	if (i!=-1)
	{
		THString text;
		m_box.GetText(i,text);
		THStringToken t(text,_T(","));
		U16 port;
		U32 ip=THNetWork::GetIpFromHostNameN(t.GetNextToken(),&port);
		if (m_input.ShowInputDialog(&text,_T("���빲������"),_T("<Hint0>�û�����</Hint0><Hint1>���룺</Hint1>"),theApp.m_hInstance,INPUTDIALOGTYPE_HINTLEFT,2,m_hWnd)==IDOK)
			CtrlPeerHttp(TRUE,ip,port,9347,THSimpleXml::GetParam(text,_T("ret0")),THSimpleXml::GetParam(text,_T("ret1")));
	}
}

void CSimpleShareDlg::OnBnClickedButton2()
{
	// TODO: Add your control notification handler code here
	//closeh
	int i=m_box.GetCurSel();
	if (i!=-1)
	{
		THString text;
		m_box.GetText(i,text);
		THStringToken t(text,_T(","));
		U16 port;
		U32 ip=THNetWork::GetIpFromHostNameN(t.GetNextToken(),&port);
		CtrlPeerHttp(FALSE,ip,port);
	}
}

void CSimpleShareDlg::OnBnClickedButton3()
{
	// TODO: Add your control notification handler code here
	//opent
	int i=m_box.GetCurSel();
	if (i!=-1)
	{
		THString text;
		m_box.GetText(i,text);
		THStringToken t(text,_T(","));
		U16 port;
		U32 ip=THNetWork::GetIpFromHostNameN(t.GetNextToken(),&port);
		if (m_input.ShowInputDialog(&text,_T("���빲������"),_T("<Hint0>���룺</Hint0>"),theApp.m_hInstance,INPUTDIALOGTYPE_HINTUP,1,m_hWnd)==IDOK)
			CtrlPeerTelnet(TRUE,ip,port,9593,THSimpleXml::GetParam(text,_T("ret0")));
	}
}

void CSimpleShareDlg::OnBnClickedButton4()
{
	// TODO: Add your control notification handler code here
	//closet
	int i=m_box.GetCurSel();
	if (i!=-1)
	{
		THString text;
		m_box.GetText(i,text);
		THStringToken t(text,_T(","));
		U16 port;
		U32 ip=THNetWork::GetIpFromHostNameN(t.GetNextToken(),&port);
		CtrlPeerTelnet(FALSE,ip,port);
	}
}

void CSimpleShareDlg::OnBnClickedButton5()
{
	// TODO: Add your control notification handler code here
	//browserH
	int i=m_box.GetCurSel();
	if (i!=-1)
	{
		THString text;
		m_box.GetText(i,text);
		THStringToken t(text,_T(","));
		U32 ip=THNetWork::GetIpFromHostNameN(t.GetNextToken(),NULL);
		text=t.GetNextToken();
		text.Replace(_T("h:"),_T(""));
		U16 port=THs2u(text);
		if (port!=0)
		{
			text.Format(_T("http://%s:%d/"),THNetWork::GetAddrStringN(ip),port);
			ShellExecute(NULL,_T("open"),text,NULL,NULL,SW_SHOW);
		}
	}
}

void CSimpleShareDlg::OnBnClickedButton6()
{
	// TODO: Add your control notification handler code here
	//browserT
	int i=m_box.GetCurSel();
	if (i!=-1)
	{
		THString text;
		m_box.GetText(i,text);
		THStringToken t(text,_T(","));
		U32 ip=THNetWork::GetIpFromHostNameN(t.GetNextToken(),NULL);
		t.GetNextToken();
		text=t.GetNextToken();
		text.Replace(_T("t:"),_T(""));
		U16 port=THs2u(text);
		if (port!=0)
		{
			text.Format(_T("%s %d"),THNetWork::GetAddrStringN(ip),port);
			ShellExecute(NULL,_T("open"),_T("telnet"),text,NULL,SW_SHOW);
		}
	}
}
