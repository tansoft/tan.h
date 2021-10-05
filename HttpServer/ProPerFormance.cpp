// ProPerFormance.cpp : implementation file
//

#include "stdafx.h"
#include "HttpServer.h"
#include "ProPerFormance.h"
#include ".\properformance.h"


// CProPerFormance dialog

IMPLEMENT_DYNAMIC(CProPerFormance, CPropertyPage)
CProPerFormance::CProPerFormance()
	: CPropertyPage(CProPerFormance::IDD)
	, m_BaseThread(0)
	, m_bLimitMaxThread(FALSE)
	, m_nLimitMaxThread(0)
	, m_bUseCache(FALSE)
	, m_nCacheLimitType(0)
	, m_nCacheSize(0)
	, m_nCacheRange(0)
{
}

CProPerFormance::~CProPerFormance()
{
}

void CProPerFormance::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_BaseThread);
	DDX_Check(pDX, IDC_CHECK1, m_bLimitMaxThread);
	DDX_Text(pDX, IDC_EDIT2, m_nLimitMaxThread);
	DDX_Check(pDX, IDC_CHECK2, m_bUseCache);
	DDX_Radio(pDX, IDC_RADIO7, m_nCacheLimitType);
	DDX_Text(pDX, IDC_EDIT4, m_nCacheSize);
	DDX_Text(pDX, IDC_EDIT5, m_nCacheRange);
}


BEGIN_MESSAGE_MAP(CProPerFormance, CPropertyPage)
	ON_BN_CLICKED(IDC_CHECK1, OnBnClickedCheck1)
	ON_BN_CLICKED(IDC_BUTTON4, OnBnClickedButton4)
	ON_BN_CLICKED(IDC_CHECK2, OnBnClickedCheck2)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
	ON_BN_CLICKED(IDC_RADIO1, OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO2, OnBnClickedRadio2)
	ON_BN_CLICKED(IDC_RADIO4, OnBnClickedRadio4)
	ON_BN_CLICKED(IDC_BUTTON7, OnBnClickedButton7)
END_MESSAGE_MAP()


// CProPerFormance message handlers

void CProPerFormance::OnBnClickedCheck1()
{
	// TODO: Add your control notification handler code here
	UpdateData();
	GetDlgItem(IDC_EDIT2)->EnableWindow(m_bLimitMaxThread);
}

BOOL CProPerFormance::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// TODO:  Add extra initialization here
	m_BaseThread=theApp.m_server.m_nParseThread;
	m_bLimitMaxThread=(theApp.m_server.m_nMaxParseThread!=0);
	m_nLimitMaxThread=theApp.m_server.m_nMaxParseThread;
	m_bUseCache=(theApp.m_server.m_nFileCacheMode!=0);
	m_nCacheLimitType=(theApp.m_server.m_nFileCacheMode==1 || m_bUseCache==FALSE)?0:1;
	m_nCacheSize=(theApp.m_server.m_nFileCacheMode==1)?theApp.m_server.m_nFileCacheSize/1024/1024:10;
	m_nCacheRange=(theApp.m_server.m_nFileCacheMode!=1)?theApp.m_server.m_nFileCacheMode:50;
	UpdateData(FALSE);
	OnBnClickedCheck1();
	OnBnClickedCheck2();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CProPerFormance::OnKillActive()
{
	// TODO: Add your specialized code here and/or call the base class
	UpdateData();
	if (m_BaseThread==0)
	{
		MessageBox(_T("��׼�߳����������0����������������Ϊ3���ϡ�"));
		return FALSE;
	}
	if (m_bLimitMaxThread && m_nLimitMaxThread<=m_BaseThread)
	{
		MessageBox(_T("����������������߳�������������ȷ����������߳�������������߳���Ӧ�Ȼ�׼�߳����ࡣ"));
		return FALSE;
	}
	if (m_bUseCache)
	{
		if (m_nCacheLimitType==0)
		{
			//size
			if (m_nCacheSize<1)
			{
				MessageBox(_T("��������ʹ���ļ����棬��������ȷ�Ļ����С��"));
				return FALSE;
			}
		}
		else
		{
			//range
			if (m_nCacheRange<10 || m_nCacheRange>80)
			{
				MessageBox(_T("��������ʹ���ļ����棬��������ȷ�Ļ���������������ֻ����10%-80%֮�䡣"));
				return FALSE;
			}
		}
	}
	return CPropertyPage::OnKillActive();
}

void CProPerFormance::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class
	UpdateData();
	theApp.m_server.m_nParseThread=m_BaseThread;
	theApp.m_server.m_nMaxParseThread=m_bLimitMaxThread?m_nLimitMaxThread:0;
	if (!m_bUseCache)
		theApp.m_server.m_nFileCacheMode=0;
	else
	{
		if (m_nCacheLimitType==0)
		{
			theApp.m_server.m_nFileCacheMode=1;
			theApp.m_server.m_nFileCacheSize=m_nCacheSize*1024*1024;
		}
		else
		{
			theApp.m_server.m_nFileCacheMode=m_nCacheRange;
		}
	}
	CPropertyPage::OnOK();
}

void CProPerFormance::OnBnClickedButton4()
{
	// TODO: Add your control notification handler code here
	MessageBox(_T("�߳�Խ�ಢ����������Խǿ��������������࣬�߳�û�д������ʱ�����½��߳̽��д���ֱ���ﵽ�߳����ֵΪֹ��\r\n���û�п����̣߳����ӽ��������ȴ�ֱ���п����̴߳���\r\n���߳̿���ʱ������������ӵ��̣߳�ֱ����׼�߳���Ϊֹ��\r\nԽ��Ļ�׼�߳�����Խ�ã��½��߳���Ҫ�ķ�CPUʱ�䣻��ͬʱ��ռ���ڴ����Խ�ࡣ\r\n����߳���Ӧ�������õô�㣬�Ա�֤����ʱ����ܾ�����\r\nͨ���������ƽű�����ʱ����ܻ��������ܣ������ڱ��ǳ���ʱ�Ľű�ռ���������̣߳�����Ҫȷ���жϽű��Ƿ�������⡣"),_T("�����߳�"),MB_OK|MB_ICONINFORMATION);
}

void CProPerFormance::OnBnClickedCheck2()
{
	// TODO: Add your control notification handler code here
	UpdateData();
	GetDlgItem(IDC_RADIO6)->EnableWindow(m_bUseCache);
	GetDlgItem(IDC_RADIO7)->EnableWindow(m_bUseCache);
	GetDlgItem(IDC_EDIT4)->EnableWindow(m_bUseCache);
	GetDlgItem(IDC_EDIT5)->EnableWindow(m_bUseCache);
	GetDlgItem(IDC_BUTTON1)->EnableWindow(m_bUseCache);
}

void CProPerFormance::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	theApp.m_server.EmptyCache();
}

void CProPerFormance::OnBnClickedRadio1()
{
	// TODO: Add your control notification handler code here
	//tiny
	UpdateData();
	m_BaseThread=5;
	m_bLimitMaxThread=FALSE;
	m_bUseCache=FALSE;
	UpdateData(FALSE);
}

void CProPerFormance::OnBnClickedRadio2()
{
	// TODO: Add your control notification handler code here
	//small
	UpdateData();
	m_BaseThread=50;
	m_bLimitMaxThread=FALSE;
	m_bUseCache=TRUE;
	m_nCacheLimitType=1;
	m_nCacheRange=50;
	UpdateData(FALSE);
}

void CProPerFormance::OnBnClickedRadio4()
{
	// TODO: Add your control notification handler code here
	//middle
	UpdateData();
	m_BaseThread=256;
	m_bLimitMaxThread=TRUE;
	m_nLimitMaxThread=512;
	m_bUseCache=TRUE;
	m_nCacheLimitType=1;
	m_nCacheRange=80;
	UpdateData(FALSE);
}

void CProPerFormance::OnBnClickedButton7()
{
	// TODO: Add your control notification handler code here
	MessageBox(_T("ʹ�û�����Լ���һЩ���ļ��Ĵ������ٴ��̶�ȡ������ͻ�ƴ���Ч�����ƣ�����С�ļ����磺�ű��ļ�����û������Ч����\r\n���ù�С�Ļ���ʹ�÷����ή��Ч�ʣ���Ϊ�������ױ����ϸ��¡�"),_T("�����ļ�����"),MB_OK|MB_ICONINFORMATION);
}
