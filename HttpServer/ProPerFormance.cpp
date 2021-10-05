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
		MessageBox(_T("基准线程数必需大于0，并建议起码设置为3以上。"));
		return FALSE;
	}
	if (m_bLimitMaxThread && m_nLimitMaxThread<=m_BaseThread)
	{
		MessageBox(_T("您设置了限制最多线程数，请设置正确的最多允许线程数，最多允许线程数应比基准线程数多。"));
		return FALSE;
	}
	if (m_bUseCache)
	{
		if (m_nCacheLimitType==0)
		{
			//size
			if (m_nCacheSize<1)
			{
				MessageBox(_T("您设置了使用文件缓存，请设置正确的缓存大小。"));
				return FALSE;
			}
		}
		else
		{
			//range
			if (m_nCacheRange<10 || m_nCacheRange>80)
			{
				MessageBox(_T("您设置了使用文件缓存，请设置正确的缓存比例，缓存比例只能在10%-80%之间。"));
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
	MessageBox(_T("线程越多并发处理能力越强，当并发请求过多，线程没有处理完成时，会新建线程进行处理，直到达到线程最大值为止。\r\n如果没有空闲线程，连接将会阻塞等待直到有空闲线程处理。\r\n当线程空闲时，会结束掉增加的线程，直到基准线程数为止。\r\n越多的基准线程性能越好，新建线程需要耗费CPU时间；但同时常占的内存亦会越多。\r\n最大线程数应尽量设置得大点，以保证并发时不会拒绝服务。\r\n通过设置限制脚本运行时间可能会提升性能，不至于被非常耗时的脚本占用了所有线程，但需要确认中断脚本是否会有问题。"),_T("关于线程"),MB_OK|MB_ICONINFORMATION);
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
	MessageBox(_T("使用缓存可以加速一些大文件的处理，减少磁盘读取次数，突破磁盘效率限制；对于小文件，如：脚本文件，则没有显著效果。\r\n设置过小的缓存使用反而会降低效率，因为缓存容易被不断更新。"),_T("关于文件缓存"),MB_OK|MB_ICONINFORMATION);
}
