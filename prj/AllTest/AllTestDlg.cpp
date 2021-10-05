// AllTestDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "AllTest.h"
#include "AllTestDlg.h"
#include ".\alltestdlg.h"

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


// CAllTestDlg 对话框



CAllTestDlg::CAllTestDlg(CWnd* pParent /*=NULL*/)
	: THResizeDlg(CAllTestDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAllTestDlg::DoDataExchange(CDataExchange* pDX)
{
	THResizeDlg::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAllTestDlg, THResizeDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON6, OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON7, OnBnClickedButton7)
	ON_BN_CLICKED(IDC_BUTTON8, OnBnClickedButton8)
	ON_BN_CLICKED(IDC_BUTTON9, OnBnClickedButton9)
	ON_BN_CLICKED(IDC_BUTTON10, OnBnClickedButton10)
	ON_BN_CLICKED(IDC_BUTTON11, OnBnClickedButton11)
	ON_BN_CLICKED(IDC_BUTTON12, OnBnClickedButton12)
	ON_BN_CLICKED(IDC_BUTTON13, OnBnClickedButton13)
	ON_BN_CLICKED(IDC_BUTTON14, OnBnClickedButton14)
	ON_BN_CLICKED(IDC_BUTTON15, OnBnClickedButton15)
	ON_BN_CLICKED(IDC_BUTTON16, OnBnClickedButton16)
	ON_BN_CLICKED(IDC_BUTTON17, OnBnClickedButton17)
	ON_BN_CLICKED(IDC_BUTTON18, OnBnClickedButton18)
	ON_BN_CLICKED(IDC_BUTTON19, OnBnClickedButton19)
	ON_BN_CLICKED(IDC_BUTTON20, OnBnClickedButton20)
	ON_MESSAGE(WM_COMINGSMS,OnComingSMSMsg)
END_MESSAGE_MAP()


// CAllTestDlg 消息处理程序

BOOL CAllTestDlg::OnInitDialog()
{
	THResizeDlg::OnInitDialog();

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
#ifdef _USE_RESIZE_DLG
	AddAnchor(IDOK,TOP_LEFT,TOP_RIGHT);

	EnableSaveRestore(FALSE);
#endif
	GetDlgItem(IDC_EDIT1)->SetWindowText(_T("0x00402df5"));
	return TRUE;  // 除非设置了控件的焦点，否则返回 TRUE
}

void CAllTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		THResizeDlg::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CAllTestDlg::OnPaint() 
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
		THResizeDlg::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标显示。
HCURSOR CAllTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

BOOL CAllTestDlg::OnProcessingOutput(THCmdProcess *pProcess,THProcessInfo *info,int nThreadId)
{
	UINT a=info->bufout.GetBufferedSize();
	return FALSE;
}

#include <THDebug.h>
#include <THSysInfo.h>
#include <THExceptionHandler.h>
#include <THBaseDialog.h>
#include <THTimeClient.h>
//#include <THBlockFile.h>
void CAllTestDlg::OnBnClickedButton1()
{
	THIndexArray iar;
	iar.fromString(_T("-5~-10,-2,-8~-15,1,2~5,7,10,22,11~2,20~25,21~23"));
	THDebug(iar.toString());
	iar.IndexSets(-1,0);
	THDebug(iar.toString());
	iar.IndexSet(1);
	THDebug(iar.toString());
	iar.IndexSets(2,5);
	THDebug(iar.toString());
	iar.IndexSet(7);
	THDebug(iar.toString());
	iar.IndexSet(6);
	THDebug(iar.toString());
	iar.IndexSet(10);
	THDebug(iar.toString());
	iar.IndexSets(6,9);
	THDebug(iar.toString());
	iar.IndexSet(22);
	THDebug(iar.toString());
	iar.IndexSets(11,2);
	THDebug(iar.toString());
	iar.IndexSets(20,25);
	THDebug(iar.toString());
	iar.IndexSets(21,23);
	THDebug(iar.toString());
	iar.IndexSets(18,23);
	THDebug(iar.toString());
	iar.IndexSets(23,28);
	THDebug(iar.toString());
	iar.IndexSets(13,18);
	THDebug(iar.toString());
	iar.IndexSets(12,19);
	THDebug(iar.toString());
	iar.IndexSets(10,15,0);
	THDebug(iar.toString());
	iar.IndexSets(-3,4,0);
	THDebug(iar.toString());
	iar.IndexSets(18,30,0);
	THDebug(iar.toString());
	iar.IndexSets(10,20,0);
	THDebug(iar.toString());
	ASSERT(iar.IsIndexSeted(4)==FALSE);
	ASSERT(iar.IsIndexSeted(5)==TRUE);
	ASSERT(iar.IsIndexSeted(10)==FALSE);
	ASSERT(iar.IsIndexSeted(9)==TRUE);
	return;
	map<int,int> test;
	test[1]=1;
	test[2]=2;
	test[4]=4;
	ASSERT(test.lower_bound(1)->second==1);
	ASSERT(test.upper_bound(1)->second==2);
	ASSERT((map_lower_key< map<int,int> >(test,1)->second==1));
	ASSERT(test.lower_bound(3)->second==4);
	ASSERT(test.upper_bound(3)->second==4);
	ASSERT((map_lower_key< map<int,int> >(test,3)->second==2));
	multimap<int,int> mtest;
	mtest.insert(multimap<int,int>::value_type(1,10));
	mtest.insert(multimap<int,int>::value_type(1,11));
	mtest.insert(multimap<int,int>::value_type(2,20));
	mtest.insert(multimap<int,int>::value_type(2,21));
	mtest.insert(multimap<int,int>::value_type(4,40));
	ASSERT(mtest.lower_bound(1)->second==10);
	ASSERT(mtest.upper_bound(1)->second==20);
	ASSERT((map_lower_key< multimap<int,int> >(mtest,1)->second==11));
	ASSERT(mtest.lower_bound(3)->second==40);
	ASSERT(mtest.upper_bound(3)->second==40);
	ASSERT((map_lower_key< multimap<int,int> >(mtest,3)->second==21));
	return;
	THTimeClient client;
	THTime ti;
	if (client.GetTime(&ti))
	{
		ti.SetSystemTime();
		THDebug(_T("%s"),ti.Format());
	}
	THDebugSetupPrintStyle(0);
	THDebugSetupReceiveCtrl(GetDlgItem(IDC_STATICR)->m_hWnd,1);
	THDebug(_T("dedede"));
	THDebug(_T("ded"));
	THDebug(_T("abc"));
	return;
	THBaseTreeDialog dlg;
	dlg.ShowTreeDialog(AfxGetApp()->m_hInstance,m_hWnd,_T("Window"),_T("SubWindow"));
	THIni ini;
	ini.Init(_T("c:\\a.ini"));
	//ini.WriteString(_T("test"),_T("gre"),_T("测试1"));
	THDebug(ini.GetString(_T("TesT"),_T("GrE")));
	THStringArray k;
	THStringArray v;

	ini.EnumAllStruct(&k);
	ini.EnumStruct(_T("te"),&k,&v);
	char *buf="\x2f\xfd\xee";
	char buf1[3];
	THDebug(_T("%d"),ini.GetBin(_T("te"),_T("sfd"),&buf1,3));
	//ini.WriteBin(_T("te"),_T("sfd1"),buf,3);
	//ini.GetBin(_T("")
	return;
	THString returnstr;
	THBaseMulSelectDialog dialog;
	dialog.ShowMulSelectDialog(::AfxGetInstanceHandle(),NULL,_T("test"),
		_T("<item0>测试一(&y)</item0><item1>测试二(&e)</item1><item2>测试三(&s)</item2><selected1>1</selected1><selected2>1</selected2>"),&returnstr,3,_T("1"),1);
	THDebug(returnstr);
	dialog.ShowMulSelectDialog(::AfxGetInstanceHandle(),NULL,_T("test"),
		_T("<item0>测试一(&y)</item0><item1>测试二(&e)</item1><item2>测试三(&s)</item2><selected1>1</selected1>"),&returnstr,3,_T("2"),2);
	THDebug(returnstr);
	dialog.ShowMulSelectDialog(::AfxGetInstanceHandle(),NULL,_T("test"),
		_T("<item0>测试一(&y)</item0><item1>测试二(&e)</item1><item2>测试三(&s)</item2><selected1>1</selected1>"),&returnstr,3,_T("3"),3);
	THDebug(returnstr);
	dialog.ShowMulSelectDialog(::AfxGetInstanceHandle(),NULL,_T("test"),
		_T("<item0>测试一(&y)</item0><item1>测试二(&e)</item1><item2>测试三(&s)</item2><selected1>1</selected1>"),&returnstr,3,_T("4"),4);
	THDebug(returnstr);
	return;
	THString str1=THExceptionHandler::MakeExceptionReport(NULL);
	m_process.RunCmd(_T("c:\\windows\\system32\\cmd.exe"),TRUE,NULL,FALSE,500,0,this);
	return;
	THDebug(THSysInfo::GetBiosId());
	THDebug(THSysInfo::GetComputerName());
	THDebug(THSysInfo::GetMemoryStatus());
	THDebug(THSysInfo::GetOSProductId());
	THDebug(THSysInfo::GetOSProductName());
	THDebug(THSysInfo::GetOSVersion());
	THDebug(THSysInfo::GetUserName());
	THDebug(THSysInfo::GetMacString());
	THDebug(THSysInfo::GetHardDiskSn());
	THDebug(THSysInfo::GetCpuId());
	//THDebug(THSysInfo:);
	return;
	// TODO: Add your control notification handler code here
	THString str=_T("Print");
	THDebug(_T("Test Debug String %s"),str);
	theApp.m_ini.GetString(_T("fw"),_T("fre"),_T("fre"));
	THStringArray ar,ar1;
	if (theApp.m_ini.EnumAllStruct(&ar))
		for(int i=0;i<ar.GetSize();i++)
			THDebug(_T("Value:%s"),ar[i]);
	if (theApp.m_ini.EnumStruct(_T("sect1"),&ar,&ar1))
		for(int i=0;i<ar.GetSize();i++)
			THDebug(_T("%s=%s"),ar[i],ar1[i]);
	//encode ini
	//THIni ini;
	ini.Init(_T("c:\\a.ini"));
	ini.InitPass(_T("pass"));
	ini.WriteString(_T("test"),_T("gre"),_T("测试1"));
	THDebug(ini.GetString(_T("test"),_T("gre"),_T("测试")));
	THUrlExplain url;
	url.Parse(_T("http://barrytan:123456@tansoft.com:2142/fasd/dir/afile.asp?a=12&fwg=2&fe#tag"));
	THDebug(_T("url:%s\r\ntype:%s,user:%s,pass:%s,host:%s:%d,params:%s,fullpath:%s,dir:%s,file:%s,ext:%s,tag:%s"),
		url.GetUrl(),url.GetType(),url.GetUser(),url.GetPass(),url.GetHost(),url.GetPort(80),
		url.GetParams(),url.GetFullPath(),url.GetDir(),url.GetFile(),url.GetExt(),url.GetTag());
	THStringArray *arkey,*arval;
	url.GetParamsArray(&arkey,&arval);
	for(int i=0;i<arkey->GetSize();i++)
		THDebug(_T("key:%s val:%s"),arkey->GetAt(i),arval->GetAt(i));
	url.Parse(_T("http://tansoft.com"));
	THDebug(_T("url:%s\r\ntype:%s,user:%s,pass:%s,host:%s:%d,params:%s,fullpath:%s,dir:%s,file:%s,ext:%s,tag:%s"),
		url.GetUrl(),url.GetType(),url.GetUser(),url.GetPass(),url.GetHost(),url.GetPort(80),
		url.GetParams(),url.GetFullPath(),url.GetDir(),url.GetFile(),url.GetExt(),url.GetTag());
	url.Parse(_T("http://tansoft.com/#tag"));
	THDebug(_T("url:%s\r\ntype:%s,user:%s,pass:%s,host:%s:%d,params:%s,fullpath:%s,dir:%s,file:%s,ext:%s,tag:%s"),
		url.GetUrl(),url.GetType(),url.GetUser(),url.GetPass(),url.GetHost(),url.GetPort(80),
		url.GetParams(),url.GetFullPath(),url.GetDir(),url.GetFile(),url.GetExt(),url.GetTag());
	url.Parse(_T("http://tansoft.com/?a=2&tre&e=2#tag"));
	THDebug(_T("url:%s\r\ntype:%s,user:%s,pass:%s,host:%s:%d,params:%s,fullpath:%s,dir:%s,file:%s,ext:%s,tag:%s"),
		url.GetUrl(),url.GetType(),url.GetUser(),url.GetPass(),url.GetHost(),url.GetPort(80),
		url.GetParams(),url.GetFullPath(),url.GetDir(),url.GetFile(),url.GetExt(),url.GetTag());
	url.GetParamsArray(&arkey,&arval);
	for(i=0;i<arkey->GetSize();i++)
		THDebug(_T("key:%s val:%s"),arkey->GetAt(i),arval->GetAt(i));
	//THSysDialog::FileDialog(TRUE,NULL,_T("e"),_T("s"),_T("s"),_T("s"),_T("d"),0);
	//THDebugFile(_T("this is a test"));
	THEncDebugFile(_T("this is a test"));
	THEncDebugFile(_T("this is anohter test"));
	THDebugLog::DecodeLogFile(theApp.m_sExePath+_T("\\AllTest.log"));
	THString strret=THSysDialog::FileDialogOpenExistOne(m_hWnd);
	if (!strret.IsEmpty())
		THDebugLog::DecodeLogFile(strret);
	THString sDir=THSysDialog::DirDialog(m_hWnd,_T("选择要处理的目录"));
	if (!sDir.IsEmpty())
	{
		THStringArray arFile;
		if (THSysMisc::FindFiles(sDir,&arFile,NULL,_T("*.log"),TRUE,TRUE))
		{
			for(int i=0;i<arFile.GetSize();i++)
			{
				THDebugLog::DecodeLogFile(arFile[i]);
			}
		}
	}
	//strret=THSysDialog::FileDialogSaveOne(m_hWnd,_T(""),_T(""),_T("txt"));
	//THDebug(strret);
	THFile file;
	THString ret;
	while(1)
	{
		if (!file.GetStringAndRemoveA(_T("c:\\test.txt"),ret)) break;
		//THDebug(_T("%s"),ret);
	}
	//THBlockFile bf;
}

int GetEncodedU32(unsigned char*& pos)
{
	int result = pos[0];
	if (!(result & 0x00000080))
	{
	pos++;
	return result;
	}
	result = (result & 0x0000007f) | pos[1]<<7;
	if (!(result & 0x00004000))
	{
	pos += 2;
	return result;
	}
	result = (result & 0x00003fff) | pos[2]<<14;
	if (!(result & 0x00200000))
	{
	pos += 3;
	return result;
	}
	result = (result & 0x001fffff) | pos[3]<<21;
	if (!(result & 0x10000000))
	{
	pos += 4;
	return result;
	}
	result = (result & 0x0fffffff) | pos[4]<<28;
	pos += 5;
	return result;
}

#include <THArith.h>
#include <THBitParser.h>
void CAllTestDlg::OnBnClickedButton2()
{
	THString str2=THSysDialog::FileDialogOpenExistOne();
	unsigned int len1;
	unsigned char *buff=(unsigned char *)THFile::GetContentToBuffer(str2,&len1);
	void *retbuf;
	unsigned long ret1;
	unsigned int i=0;
	while(i<len1)
	{
		THGZip::ZipDecode(buff+i,len1-i,&retbuf,&ret1);
		i++;
	}
/*	unsigned char *buffer=buff;
	int outbuffer[4096];
	int i=0;
	while(buffer<buff+len1)
	{
		outbuffer[i++]=GetEncodedU32(buffer);
	}
	THDebug(THStringConv::FormatString(outbuffer,len1));*/
	THFile::FreeContentBuffer(buff);
	THString str1=THFile::GetContentAToString(THSysDialog::FileDialogOpenExistOne());
	str1.Replace(_T("\r\n"),_T(""));
	void *data1;
	int size;
	if (THBase64::Decode(str1,&data1,&size))
	{
		str1.SetString((LPCTSTR)data1,size);
		THDebug(str1);
		THBase64::FreeBuffer(data1);
	}
	return;
	m_auth.SetEventHandler(this,NULL);
	m_auth.StartCheckAuth(1234,_T("822BBE9F3123cds32"));
	m_auth.StartDemoCodeQuery(1234);
	return;
	THStringA a;
	// TODO: Add your control notification handler code here
	THBitParser parser;
	parser.SetBit(1);
	parser.SetBit(0);
	parser.SetBit(1);
	parser.SetBit(1);
	parser.SetBit(0);
	parser.SetBit(0);
	parser.SetBit(0);
	parser.SetBit(1);
	parser.SetBit(0);
	parser.SetBit(1);
	parser.SetBit(0);
	parser.SetBit(0);
	for(int i=0;i<12;i++)
		THDebug(_T("%d"),parser.GetBit(i));
	return;
	THDebug(_T("Md5 ret:%s"),THMd5::CalcFileMd5Str(_T("c:\\a.dat")));
//	THDebug(_T("Sha ret:%s"),THSha::CalcShaStr("dewf",4));
	THString ret=_T("H4sIAAAAAAAEAO29B2AcSZYlJi9tynt/SvVK1+B0oQiAYBMk2JBAEOzBiM3mkuwdaUcjKasqgcplVmVdZhZAzO2dvPfee++999577733ujudTif33/8/XGZkAWz2zkrayZ4hgKrIHz9+fB8/Ih6fVMvLvG6ytqiWr/O2LZYXR49fz6ur51mbN+0XedNkF3lz9Cwrm/zx3cg33PpNsaDPssXqlKBdf9FcHL2p19q+/93j02Wb12+q1/lypg39Tx4fr9vqVb4qr0+X2aTMZ6b33ueu5dlylr872vHayCeP70ZH+EVVzV7O66zJm/SuA1NgPBYE//W8Wl68/uK1efNp0aDrx3c7nz8+WTdttcjrl3Xe5MspA4p89kVWLL9bLGfVlX3z87par55cf1HNcgzA//MxdbcqM/lj9/Fd/0+m+5fLsljmNMQ2m7b0x7U/Uf0v+Z2neZsVZTCl5iP+/ouqziNt/I8f342Mg+auqWr75/FVdv3l8mxW5jrF3gfel18UyzXx09F9v4H5UNu9ntZ5vnyd0TwGsPzPH395fo7hBl2Gn4VNvl2t6+Zor9NIPn38ZN0Q/GfrspQ+FF7vYyJNOOo3xapDFpmFF1VbnF9jfoO/H7+o/L+/O8+X6EO7G/jy8Yv8SsVPwWjz7scEPfigBz7+7eO7vVH8P+mlJWQoBAAA");
	THDebug(_T("Base64 ret:%s"),THBase64::Encode((LPCTSTR)ret,ret.GetLength()*sizeof(TCHAR)));
	void *data;
	int len;
	THBase64::Decode(ret,&data,&len);
	CFile file;
	file.Open(_T("c:\\a.gz"),CFile::modeCreate|CFile::modeWrite);
	file.Write(data,len);
	file.Close();
	if (data)
		THDebug(_T("Base64 ret:%s"),THBase64::Encode(data,len));
	THBase64::FreeBuffer(data);
	void *buf,*buf1;
	unsigned long retsize,retsize1;
	if (THGZip::ZipEncode(_T("1242566"),7*sizeof(TCHAR),&buf,&retsize))
	{
		THDebug(THStringConv::FormatString(buf,retsize));
		if (THGZip::ZipDecode(buf,retsize,&buf1,&retsize1))
		{
			THDebug(THStringConv::FormatString(buf1,retsize1));
			THGZip::Free(buf1);
		}
		THGZip::Free(buf);
	}
	THGZip::GZipEncodeFile(_T("c:\\FetionFx.txt"),_T("c:\\a.gz"));
	THGZip::GZipDecodeFile(_T("c:\\a.gz"),_T("c:\\FetionFx1.exe"));
	THAes aes;
	aes.EncryptFile(_T("c:\\CopyLocation.dll"),_T("c:\\CopyLocation.aes"),_T("123456"));
	aes.DecryptFile(_T("c:\\CopyLocation.aes"),_T("c:\\CopyLocation1.dll"),_T("123456"));
	aes.EncryptFile(_T("c:\\CopyLocation.txt"),_T("c:\\CopyLocation1.aes"),_T("123456"));
	aes.DecryptFile(_T("c:\\CopyLocation1.aes"),_T("c:\\CopyLocation1.txt"),_T("123456"));
	THString str=aes.EncryptText(_T("mytext"),_T("123456"));
	THDebug(_T("Encode:%s,Decode:%s"),str,aes.DecryptText(str,_T("123456")));
	int retlen;
	void *pret=aes.EncryptBuffer("124325",6,&retlen,_T("123456"));
	if (pret)
	{
		THDebug(THStringConv::FormatString(pret,retlen));
		void *ret1=aes.DecryptBuffer(pret,retlen,&retlen,_T("123456"));
		if (ret1)
		{
			THDebug(THStringConv::FormatString(ret1,retlen));
			aes.Free(ret1);
		}
		aes.Free(pret);
	}
	THTiniRsa trsa;
	trsa.GenKey();
	THString tstr=trsa.Encode("测试",4);
	THDebug(_T("%s"),tstr);
	unsigned int tret;
	void *tmp=trsa.Decode(tstr,&tret);
	if (tmp)
	{
		THDebug(_T("%s"),THStringConv::FormatString(tmp,tret));
		trsa.Free(tmp);
	}
	THString url=THUrlExplain::UrlEncode(_T("http://eq.ew.com/的法1发2/的误区。2.tar"));
	THDebug(_T("%s"),url);
	THDebug(THUrlExplain::UrlDecode(url));
}

#include <THStruct.h>
void FreeListCallBack(void *key,void *value,void *adddata)
{
	THDebug(_T("Free list:%d"),value);
	delete [] value;
}

void FreeMapCallBack(void *key,void *value,void *adddata)
{
	THDebug(_T("Free map key:%d 's value %d"),(int)(INT_PTR)key,value);
	delete [] value;
}

void FreeDMapCallBack(void *key,void *value,void *adddata)
{
	if (value==NULL)
	{
		THDebug(_T("Free DMap Key1:%d"),key);
	}
	else
	{
		THDebug(_T("Free DMap Key2:%d and Value:%d"),key,value);
		delete [] value;
	}
}

void FreeDynMapCallBack(void *key,void *value,void *adddata)
{
	THDebug(_T("Free dyn map key:%d 's value %d"),(int)(INT_PTR)key,value);
	delete [] value;
}

void CAllTestDlg::OnBnClickedButton3()
{
	// TODO: Add your control notification handler code here
	THList<char *> m_list;
	m_list.SetFreeProc(FreeListCallBack);
	m_list.AddHead(new char[123]);
	m_list.AddTail(new char[123]);
	m_list.AddHead(new char[123]);
	m_list.AddHead(new char[123]);
	THDebug(_T("size:%d"),m_list.GetSize());
	//这里必须使用bRemove=FALSE,因为数据类型是指针，且设置了FreeProc
	char *tmp=m_list.GetHead(FALSE);
	//use pointer tmp
	m_list.RemoveHead();
	THPosition pos=m_list.GetStartPosition();
	THPosition lastpos;
	while(!pos.IsEmpty())
	{
		lastpos=pos;
		if (m_list.GetNextPosition(pos,tmp))
		{
			THDebug(_T("org item:%d"),tmp);
			m_list.SetAt(lastpos,new char[123]);//更改某个键值内容
			m_list.GetAt(lastpos,tmp,FALSE);
			THDebug(_T("new item changed:%d"),tmp);
		}
	}
	for(UINT i=0;i<m_list.GetSize();i++)
	{
		THDebug(_T("item[%d]:%d"),i,m_list[i]);
	}
	m_list.RemoveAll();
	THMap<int,char *> m_map;
	m_map.SetFreeProc(FreeMapCallBack);
	m_map.SetAt(10,new char[123]);
	m_map.RemoveAt(10);
	m_map.SetAt(11,new char[124]);
	m_map.SetAt(12,new char[124]);
	m_map.SetAt(13,new char[124]);
	m_map.SetAt(21,new char[124]);
	m_map.SetAt(101,new char[124]);
	pos=m_map.GetStartPosition();
	int key;
	char *val;
	while(!pos.IsEmpty())
	{
		if (m_map.GetNextPosition(pos,key,val))
		{
			THDebug(_T("map member key:%d,val:%d"),key,val);
		}
	}
	THDebug(_T("list map member key:%d,val:%d"),21,m_map[21]);
	m_map.RemoveAll();
	THDoubleKeyMap<int,short,char *> m_dmap;
	m_dmap.SetFreeProc(FreeDMapCallBack);
	m_dmap.SetAt(2,4,new char[124]);
	m_dmap.SetAt(3,4,new char[124]);
	m_dmap.SetAt(4,43,new char[124]);
	m_dmap.SetAt(5,123,new char[124]);
	m_dmap.SetAt(6,83,new char[124]);
	m_dmap.SetAt(2,4,new char[124]);
	m_dmap.SetAt(124,21,new char[124]);
	THDebug(_T("double map member 2,4 is:%d"),m_dmap[2][4]);
	m_dmap.RemoveAt(2,4);
	THDebug(_T("double map member 2,4 is now:%d"),m_dmap.GetAt(2,4));
	pos=m_dmap.GetStartPosition();
	short key2;
	while(!pos.IsEmpty())
	{
		if (m_dmap.GetNextPosition(pos,key,key2,val))
		{
			THDebug(_T("double map member key:%d,%d val:%d"),key,key2,val);
		}
	}
	m_dmap.RemoveAll();
	THDynMapPool<int,char *,3> m_dynmap;
	m_dynmap.SetFreeProc(FreeDynMapCallBack);
	m_dynmap.SetAt(1,new char[124]);
	m_dynmap.SetAt(3,new char[124]);
	m_dynmap.SetAt(4,new char[124]);
	m_dynmap.SetAt(5,new char[124]);
	m_dynmap.SetAt(7,new char[124]);
	THDebug(_T("dyn map member key:%d val:%d"),7,m_dynmap[7]);
	m_dynmap.RemoveAt(5);
	pos=m_dynmap.GetStartPosition();
	while(!pos.IsEmpty())
	{
		if (m_dynmap.GetNextPosition(pos,key,val))
		{
			THDebug(_T("dyn map member key:%d,val:%d"),key,val);
		}
	}
	m_dynmap.RemoveAll();
	const void *ret=THBinSearch::BinSearch("\0\3\0\0\21\12\41\142\12\32\23",11,"\12\41",2);
	THBinSearch::StrSearch(_T("\0\3\0\0\21\12abcdegf\41\142\12\32\23"),36,_T("abcd"));
	THMemBufOld buf;
	buf.AddBuf("123214211241",12);
	buf.AddBuf("123214211241",12);
	char mybuf[1024];
	unsigned int myret;
	if (buf.GetBuf(mybuf,-1,&myret,TRUE))
	{
		if (buf.GetBuf(mybuf,5,&myret,FALSE))
			if (buf.GetBuf(mybuf,5,&myret,FALSE))
				if (buf.GetBuf(mybuf,5,&myret,FALSE))
					if (buf.GetBuf(mybuf,5,&myret,FALSE))
						if (buf.GetBuf(mybuf,4,&myret,FALSE))
							THDebug(_T("ok"));
	}
}

#include <THString.h>
void CAllTestDlg::OnBnClickedButton4()
{
	// TODO: Add your control notification handler code here
	THString str=_T("<item1>23</item1><item2></item2><aaa><bbb>dert</bbb><ccc>gf</ccc></aaa>");
	THDebug(str);
	THSimpleXml::SetParam(&str,_T("bbb"),_T("test1412"));
	THDebug(str);
	THDebug(THSimpleXml::GetParam(str,_T("ccc")));
	THDebug(THSimpleXml::GetAndRemoveParam(&str,_T("aaa")));
	THDebug(str);
	THSimpleXml::RemoveParam(&str,_T("item2"));
	THDebug(str);
	THString ret=THStringConv::FormatString(_T("\x11\xdd\x1d\xff\x1a\x00\x01\x41"),8,THStringConv::FormatType_Hex,8,THStringConv::FormatType_Hex,TRUE,TRUE);
	THDebug(ret);
	ret=THStringConv::FormatString(_T("\x11\xdd\x1d\xff\x1a\x00\x01\x41"),8,THStringConv::FormatType_Dec,4,THStringConv::FormatType_Dec,TRUE,FALSE);
	THDebug(ret);
	ret=THStringConv::FormatString(_T("\x11\xdd\x1d\xff\x1a\x00\x01\x41"),8,THStringConv::FormatType_Oct,7,THStringConv::FormatType_None,TRUE,TRUE);
	THDebug(ret);
	ret=THStringConv::FormatString(_T("\x11\xdd\x1d\xff\x1a\x00\x01\x41"),8,THStringConv::FormatType_Bin,8,THStringConv::FormatType_Hex,FALSE);
	THDebug(ret);
	THString strxml=_T("<results><contacts><contact uri=\"sip:441119006@fetion.com.cn;p=3911\" status-code=\"200\"><personal version=\"19\" nickname=\"\" /></contact><contact uri=\"sip:443143025@fetion.com.cn;p=4920\" status-code=\"200\"><personal version=\"13\" nickname=\"\"/></contact></contacts></results>");
	THTiniXml txml;
	txml.SetContent(strxml);
	//<results><contacts><contact uri="sip:441119006@fetion.com.cn;p=3911" status-code="200"><personal version="19" nickname="鏋佸湴鍊欓笩-鏃犵晫绡悆" impresa="" ivr-enabled="1" portrait-crc="898625511" provisioning="1" mobile-no="13801062635" name=""/></contact><contact uri="sip:443143025@fetion.com.cn;p=4920" status-code="200"><personal version="13" nickname="椋炰俊绮剧伒" impresa="" ivr-enabled="1" portrait-crc="0" provisioning="1" mobile-no="13901927625" name=""/></contact></contacts></results>
	THStringArray ar;
	txml.SplitAllKeys(&ar);
	THDebug(_T("xml:%s"),strxml);
	for(int i=0;i<ar.GetSize();i++)
		THDebug(_T("sub %d:%s"),i,ar[i]);
	txml.IntoXml(_T("contacts"));
	txml.SplitAllKeys(&ar);
	THDebug(_T("xml:%s"),txml.GetCurrent());
	for(int i=0;i<ar.GetSize();i++)
		THDebug(_T("sub %d:%s"),i,ar[i]);
	THString mystr;
	mystr=_T("测试1242563SAB;/[");
	for(int i=0;i<mystr.GetLength();i++)
	{
		int val=mystr.GetAt(i);
		THDebug(_T("%d"),val);
	}
	THDebug(_T("%s"),THStringConv::LimitStringSize(mystr,2));
	THDebug(_T("%s"),THStringConv::LimitStringSize(mystr,3));
	THDebug(_T("%s"),THStringConv::LimitStringSize(mystr,4));
	THDebug(_T("%s"),THStringConv::LimitStringSize(mystr,5));
	mystr=_T("124测试2563SAB;/[");
	THDebug(_T("%s"),THStringConv::LimitStringSize(mystr,4));
	THDebug(_T("%s"),THStringConv::LimitStringSize(mystr,5));
	mystr=_T("1241测试2563SAB;/[");
	THDebug(_T("%s"),THStringConv::LimitStringSize(mystr,4));
	THDebug(_T("%s"),THStringConv::LimitStringSize(mystr,5));
	THDebug(_T("%s"),THStringConv::LimitStringSize(mystr,6));
	THDebug(_T("%s"),THStringConv::LimitStringSize(mystr,7));
	mystr=_T("124346");
	THDebug(_T("%s"),THStringConv::LimitStringCount(mystr,7));
	THDebug(_T("%s"),THStringConv::LimitStringCount(mystr,6));
	THDebug(_T("%s"),THStringConv::LimitStringCount(mystr,5));
	mystr=_T("124测试346");
	THDebug(_T("%s"),THStringConv::LimitStringCount(mystr,3));
	THDebug(_T("%s"),THStringConv::LimitStringCount(mystr,4));
	THDebug(_T("%s"),THStringConv::LimitStringCount(mystr,5));
	THDebug(_T("%s"),THStringConv::LimitStringCount(mystr,6));
	THDebug(_T("%s"),THStringConv::LimitStringCount(mystr,7));
}

#include <THThread.h>
void CAllTestDlg::OnBnClickedButton5()
{
	// TODO: Add your control notification handler code here
	THThread m_thread;
	m_thread.StartThread(1,this);
}

void CAllTestDlg::OnBnClickedButton6()
{
	// TODO: Add your control notification handler code here
	m_popupdll.ShowPopup(_T("http://127.0.0.1/html/template1.html"),_T("[musthtmltag]1[/musthtmltag]"));//确保成功才显示
	//m_popupdll.ShowPopup(_T("http://127.0.0.1/w/656/index.htm"));
	//m_popupdll.ShowPopup(_T("http://127.0.0.1/w/656/index.htm"),PopupSettingsRightDownPopupTimeoutImage);
	//Sleep(1000);
	//m_popupdll.ShowPopup(_T("F:\\waitdel\\656\\index.htm"),PopupSettingsRightDownPopupTimeoutImage);
	//m_popupdll.ShowPopup(_T("http://127.0.0.1/w/656/index.htm"),_T("[dlgtype]0[/dlgtype]")PopupSettingsIe);
	//m_popupdll.ShowPopup(_T("http://127.0.0.1/w/656/index.htm"),PopupSettingsIe);
	/*m_popupdll.ShowPopup(_T("http://tansoft.cn/tools"),PopupSettingsRightDownPopupAlwaysImage);
	Sleep(1000);
	m_popupdll.ShowPopup(_T("http://tansoft.cn/tools"),PopupSettingsRightDownPopupTimeoutImage);
	Sleep(1000);
	m_popupdll.ShowPopup(_T("http://tansoft.cn/tools"),PopupSettingsRightDownPopupAlwaysWindow);
	Sleep(1000);
	m_popupdll.ShowPopup(_T("http://tansoft.cn/tools"),PopupSettingsRightDownPopupTimeoutWindow);
	Sleep(1000);
	m_popupdll.ShowPopup(_T("http://tansoft.cn/tools"),PopupSettingsIe);*/
}

#include <THLogicCalc.h>
void CAllTestDlg::OnBnClickedButton7()
{
	// TODO: Add your control notification handler code here
	double ret;
	THString str=_T("((1.0+2.3)*12.2-6.5*23-2^5+8.2)/2.3+5~3");//490.26086956521739130434782608696
	THLogicCalc::LogicCalc(str,&ret);
	THDebug(_T("LogicCalc %s=%f"),str,ret);
}

#include <THRegExpDll.h>
void CAllTestDlg::OnBnClickedButton8()
{
	// TODO: Add your control notification handler code here
	THRegExpDll m_regexp;
	THString str1=_T("barrytan@21cn.com");
	THString str2=_T("127.0.0.1");
	THString str3=_T("http://127.0.0.1/");
	if (m_regexp.CheckRegExp(str1,RegExpEmail))
		THDebug(_T("%s match with email RegExp"),str1);
	if (m_regexp.CheckRegExp(str2,RegExpIpAddress))
		THDebug(_T("%s match with ip addr RegExp"),str2);
	if (m_regexp.CheckRegExp(str3,RegExpUrl))
		THDebug(_T("%s match with post code RegExp"),str3);
}

#include <THScript.h>
void CAllTestDlg::OnBnClickedButton9()
{
	THScript script;
	script.Init(_T("c:\\a.txt"));
	script.SetGlobalSettings(_T("ss"),_T("dewef"));
	script.SetLocalSettings(0,_T("ss"),_T("dewef"));
	script.RunScript(_T("logic"),0);
}

#include <THNetWork.h>
class MyData : public THNetSnifferDataHandler
{
public:
	MyData():THNetSnifferDataHandler()
	{
		//m_sS
	}
	virtual void OnError(int nErrCode)
	{
		THDebug(_T("OnError:%d,LastError:%d"),nErrCode,WSAGetLastError());
	}
	virtual void OnData(U8 protocol,U32 saddr,U32 daddr,const void *data,unsigned int len)
	{
		THDebug(_T("Protocol:%d,Len:%d\r\n%s"),protocol,len,THStringConv::FormatString(data,len));
	}
};

void CAllTestDlg::OnBnClickedButton10()
{
	//THNetWork net;
	THNetSniffer *sniffer=new THNetSniffer;
	MyData *data=new MyData;
	data->SetSnifferAddr(sniffer->GetLocalIp(1));
	//data->AddFilterAddr(sniffer->GetIpFromHostName(_T("www.google.com")));
	//data->AddFilterProtocol(IPPROTO_TCP);
	//data->AddFilterProtocol(IPPROTO_UDP);
	sniffer->StartSniffer(data);
}

void CAllTestDlg::OnBnClickedButton11()
{
	if (m_sqldll.OpenDataBase(_T("c:\\aaa1.db")))
	{
		m_sqldll.ExecSql(_T("Create table contacts (id INTEGER PRIMARY KEY,first_name CHAR,last_name CHAR,email CHAR);"));
		m_sqldll.ExecSql(_T("insert into contacts(first_name,last_name,email) values('dwq','dwqf','wqfqw@21cn.com')"));
		m_sqldll.ExecSql(_T("SELECT * FROM contacts"),this);
		if (m_sqldll.StartQuery(_T("SELECT * FROM contacts")))
		{
			while(m_sqldll.GetNext())
			{
				THDebug(_T("%s,%s"),m_sqldll.GetValue(0),m_sqldll.GetValueByKey(_T("email")));
			}
		}
		m_sqldll.EndQuery();
	}
}

BOOL CAllTestDlg::OnEnumData(THStringArray *titles,THStringArray *args)
{
	THDebug(FindValues(_T("first_name"),titles,args));
	return FALSE;
}

void CAllTestDlg::OnBnClickedButton12()
{
	//m_http.StartFileDownload(_T("http://127.0.0.1/a.rar"),_T("c:\\a.rar"));
	//m_http.StartMemDownload(_T("http://127.0.0.1/a.rar"),1024);
	m_ftp.SetHandler(this);
	m_threadid=m_ftp.ConnectToFtp(_T("211.155.23.177"),21,_T("webmaster@tansoft.cn"),_T("810918810905"));
}

BOOL CAllTestDlg::OnConnected(int threadid,const THString host,class THINetFtp *cls,void *adddata)
{
	m_ftp.ChangeDirectory(threadid,_T("htdocs/tools"));
	//m_ftp.MakeDirectory(threadid,_T("test"));
	//m_ftp.RenameFile(threadid,_T("test"),_T("test1"));
	//m_ftp.DeleteDirectory(threadid,_T("test1"));
	//m_ftp.ListDirectory(threadid);
	m_ftp.DownloadFile(threadid,_T("Bluetooth.rar"),_T("c:\\aaa.rar"),TRUE);
	//m_ftp.UploadFile(threadid,_T("c:\\aaa.rar"),_T("abc.rar"),TRUE);
	return FALSE;
}

void CAllTestDlg::OnCommandFinish(int threadid,const THString host,class THINetFtp *cls,void *adddata,THString cmd,THString ret,THString val1,THString val2,int cmdadddata)
{
	if (cmd==FTPCOMMAND_LISTDIR)
	{
		THString result=THSimpleXml::GetParam(ret,_T("result"));
		THString dirs=THSimpleXml::GetParam(ret,_T("dirs"));
		THString files=THSimpleXml::GetParam(ret,_T("files"));
	}
}

BOOL CAllTestDlg::OnProcessing(int threadid,const THString host,class THINetFtp *cls,void *adddata,unsigned int current,unsigned int down,unsigned int total,THString cmd,THString val1,THString val2,int cmdadddata)
{
	THDebug(_T("%d,%d,%d"),current,down,total);
	return FALSE;
}

void CAllTestDlg::OnFinish(int threadid,const THString host,class THINetFtp *cls,void *adddata,int ErrCode)
{
	MessageBox(_T("OnFinish"),_T(""),0);
}

void CAllTestDlg::OnBnClickedButton13()
{
/*	//THSerialPort 测试
	//异步模式
	m_port.SetHandler(this);
	if (m_port.InitPort(1,115200))
	{
		if (m_port.IsCanWrite())
			m_port.WriteToPort(_T("AT+CMGR=4\r\n"));
		//or
		m_port.WriteToPort(_T("AT+CMGR=3\r\n"),TRUE,TRUE);
	}
	m_port.ClosePort();
	//同步模式
	if (m_port.InitPort(1,115200,0))
	{
		m_port.WriteToPort(_T("AT+CMGR=3\r\n"),TRUE,TRUE);
		THDebug(_T("%s"),m_port.ReadFromPort(1024));
	}
	m_port.ClosePort();*/
/*	THModem m;
	if (m.OpenModem(1))
	{
		if (m.IsReady())
		{
			THDebug(_T("Modem Ready!"));
			m.SetEcho();
		}
		else
			THDebug(_T("Not Have Modem Here!"));
		m.CloseModem();
	}*/
/*	if (gsm.OpenModem(1,9600))
	{
		if (gsm.IsReady())
		{
			THDebug(_T("Modem Ready!"));
			gsm.SetEcho();
			gsm.SetPduMode();
			gsm.SetGsmHandler(this);
			gsm.SetMsgComingNotifyMode();
			THString sender,msg;
			THTime ti;
			for(int idx=1;idx<20;idx++)
			{
				if (gsm.ReadSMS(idx,&sender,&msg,&ti))
				{
					THDebug(_T("Receive Msg %d: sender:%s msg:%s time:%s"),idx,sender,msg,ti.Format());
				}
			}
			//gsm.SendSMS(_T("13928557836"),_T("测试测试"));//,_T("13800757500"));
			gsm.DeleteSMS(4);
		}
		else
			THDebug(_T("Not Have Modem Here!"));
		//gsm.CloseModem();
	}*/
	m_machine.SetReceiveMessageHandler(this);
	if (m_machine.InitModem(1,9600))
	{
		THDebug(_T("machine start now!"));
		m_machine.SendSMS(_T("13535601188"),_T("这是测试"));
		m_machine.SendSMS(_T("16113535601188"),_T("这是测试"));
		m_machine.SendSMS(_T("161"),_T("FS#13535601188#这是测试2"));
//		m_machine.SendSMS(_T("16113925901884"),_T("这是测试"));
//		m_machine.SendSMS(_T("161"),_T("FS#13925901884#这是测试2"));
		//m_machine.SendSMS(_T("13925901884"),_T("这是好长的测试3"));
	}
	else
	{
		THDebug(_T("machine init error!"));
	}
}

void CAllTestDlg::OnSMSComing(const int idx,const THString sender,const THString msg,const THTime ti)
{
	THDebug(_T("Receive msg:idx:%d,sender:%s,msg:%s,time:%s"),idx,sender,msg,ti.Format());
}

void CAllTestDlg::OnResponseRequest(THString str,int status)
{
	THDebug(_T("OnResponseRequest:%s,%d"),str,status);
}

void CAllTestDlg::OnComingMessage(THString str,int status)
{
	THDebug(_T("OnComingMessage:%s,%d"),str,status);
}

LRESULT CAllTestDlg::OnComingSMSMsg(WPARAM wParam,LPARAM lParam)
{
	int idx=(int)wParam;
	THString sender,msg;
	THTime time;
	if (gsm.ReadSMS(idx,&sender,&msg,&time))
	{
		THDebug(_T("Receive Msg %d: sender:%s msg:%s time:%s"),idx,sender,msg,time.Format());
		gsm.DeleteSMS(idx);
	}
	return 0;
}

void CAllTestDlg::OnComingSMS(int idx)
{
	THDebug(_T("OnComingSMS:%d"),idx);
	::PostMessage(m_hWnd,WM_COMINGSMS,idx,0);
}

void CAllTestDlg::OnHandleError(int nErrorType,int nPortNum)
{
	THDebug(_T("Seiral Port:%d Error %d"),nPortNum,nErrorType);
}

void CAllTestDlg::OnHandleFlagEvent(DWORD dwCommEvent,int nPortNum)
{
	THDebug(_T("Seiral Port:%d Event:%d"),nPortNum,dwCommEvent);
}

unsigned int CAllTestDlg::OnHandleBufferCharEvent(const void *pData,unsigned int nLen,int nPortNum)
{
	THDebug(_T("Seiral Port:%d Data:%s"),nPortNum,THStringConv::FormatString(pData,nLen));
	return nLen;
}

#include <THRemoteHookDll.h>
void CAllTestDlg::OnBnClickedButton14()
{
	THRemoteHookDll m_hook;
	THString filename=THSysDialog::FileDialogOpenExistOne(m_hWnd,_T("RemoteHookTest.dll"),_T("dll文件(*.dll)|*.dll||"),_T("dll"),_T("..\\..\\RemoteHookTest"),_T("请选择加载入qq.exe的dll："));
	if (!filename.IsEmpty())
		THDebug(_T("Try to Inject %s to QQ.exe,ret:%d"),filename,m_hook.InjectToProcessByName(_T("QQ.exe"),filename));
}

#include <THLiveUpdateDll.h>
void CAllTestDlg::OnBnClickedButton15()
{
	THLiveUpdateDll m_dll;
	m_dll.ShowLiveUpdateProcess(theApp.m_sExePath,_T("测试"),_T("http://127.0.0.1/SimpleSettings/liveupdate.ini"),2,1,3);
	//m_liveupdate.ShowLiveUpdateProcess();
	//m_liveupdate.InitLiveUpdate(theApp.m_sExePath,_T("测试"),_T("http://127.0.0.1/SimpleSettings/liveupdate.ini"),2,1,3);
}

#include <THDebugMap.h>
void CAllTestDlg::OnBnClickedButton16()
{
	THString str;
	GetDlgItem(IDC_EDIT1)->GetWindowText(str);
	if (str.IsEmpty())
	{
		MessageBox(_T("请输入崩溃地址"));
		return;
	}
	unsigned int addr=THStringConv::s2num_u(str);
	THString mapfile=THSysDialog::FileDialogOpenExistOne(m_hWnd,_T(""),_T("Map 文件(*.map)|*.map||"),_T(""),_T(""),_T("请选择对应的Map文件"));
	if (mapfile.IsEmpty()) return;
	//THString mapdata=THFile::GetContentToString(mapfile);
	CStdioFile f;
	if (!f.Open(mapfile,CStdioFile::modeRead))
	{
		MessageBox(_T("打开文件错误"));
		return;
	}
	THString data,tmp;
	THStringArray ar;
	int state=0;//0 读取基址模式 1 读取程序段模式 2 读取函数偏移模式 3 读取文件偏移模式
	unsigned int baseaddr=0;	//基地址
	THStringArray arSection;	//程序段数据
	THString last;
	THString baseseg;
	//THStringArray arFunction;	//函数数据
	//THStringArray
	THString curfile;
	unsigned int offest=0;
	while(f.ReadString(data))
	{
		data.Trim();
		ar.RemoveAll();
		if (state==0)
		{
			tmp=data;
			if (THMatchSearch::MatchSearch(&tmp,_T("Preferred load address is %s"),&ar))
			{
				baseaddr=THStringConv::s2h(ar[0]);
			}
			else
			{
				tmp=data;
				ar.RemoveAll();
				if (THMatchSearch::MatchSearch(&tmp,_T("Start%sLength%sName%sClass"),&ar))
				{
					state=1;
				}
			}
		}
		else if (state==1)
		{
			tmp=data;
			if (THMatchSearch::MatchSearch(&tmp,_T("Address%sPublics by Value%sRva+Base%sLib:Object"),&ar))
			{
				state=2;
			}
			else
			{
				tmp=data;
				ar.RemoveAll();
				if (THMatchSearch::MatchSearch(&tmp,_T(" %s %s %s %s"),&ar))
				{
					//not need to parse
				}
			}
		}
		else if (state==2)
		{
			tmp=data;
			if (THMatchSearch::MatchSearch(&tmp,_T("Line numbers for %s(%s) segment %s"),&ar))
			{
				state=4;
			}
			else
			{
				tmp=data;
				ar.RemoveAll();
				if (THMatchSearch::MatchSearch(&tmp,_T("%s[ ]%s[ ]%s[ ]%s"),&ar))
				{
					//for(int i=0;i<ar.GetSize();i++)
					//	THDebug(_T("%s\r\n"),ar[i]);
					if (THStringConv::s2h(ar[2])>addr)
					{
						THDebug(_T("Found %s"),last);
						tmp=last;
						ar.RemoveAll();
						if (!THMatchSearch::MatchSearch(&tmp,_T("%s:%s[ ]%s[ ]%s[ ]%s"),&ar))
						{
							THDebug(_T("错误，不能解释上一条语句 %s"),last);
						}
						else
						{
							baseseg=ar[0];
							unsigned int saddr=THStringConv::s2h(ar[3]);
							unsigned int raddr=THStringConv::s2h(ar[1]);
							offest=addr-saddr+raddr;
						}
						state=3;
					}
					last=data;
				}
			}
		}
		else if (state==3)
		{
			tmp=data;
			if (THMatchSearch::MatchSearch(&tmp,_T("Line numbers for %x(%s) segment %x"),&ar))
			{
				curfile=ar[0];
				last.Empty();
				state=4;
			}
		}
		else if (state==4)
		{
			tmp=data;
			if (THMatchSearch::MatchSearch(&tmp,_T("Line numbers for %s(%s) segment %s"),&ar))
			{
				curfile=ar[0];
				last.Empty();
			}
			else
			{
				tmp=data;
				ar.RemoveAll();
				while(THMatchSearch::MatchSearch(&tmp,_T("%s %s:%s[ ]"),&ar))
				//261 0001:0000f7f0   264 0001:0000f80f   293 0001:0000f822
				{
					if (baseseg==ar[1] && THStringConv::s2h(ar[2])>offest)
					{
						THDebug(_T("Found:%s(%s)"),curfile,last);
						return;
					}
					last=ar[0]+_T("")+ar[1]+_T(":")+ar[2];
				}
			}
		}
	}
}

#include "SkinDemo.h"
void CAllTestDlg::OnBnClickedButton17()
{
	CSkinDemo skin;
	skin.DoModal();
}

#include <HardWare\THCardReader.h>
void CAllTestDlg::OnBnClickedButton18()
{
	THERIcCardDll dll;
	if (dll.OpenPort(1,0))
	{
		THDebug(_T("Serial No:%s"),dll.ReadCardSerialNo());
		if (dll.WriteCard(_T("1001001"),_T("黄小虎")))
		{
			THString str,str1;
			if (dll.ReadCard(&str,&str1))
				THDebug(_T("Id:%s Name:%s"),str,str1);
		}
		dll.ClosePort();
	}
}

void CAllTestDlg::OnAuthResult(THSoftwareAuth *cls,THAuthState Result,THString strErrMsg,void *adddata)
{
	THDebug(_T("%d,%s"),Result,strErrMsg);
}

#include <THFlvParser.h>
#include <THMp4Parser.h>
#include <THJsonParser.h>
#include <THSwfParser.h>
#include <THFileStore.h>

void CAllTestDlg::OnBnClickedButton19()
{
	THString strret=THSysDialog::FileDialogOpenExistOne(m_hWnd);
	if (!strret.IsEmpty())
	{
		THSwfParser::DumpSwfFromFile(strret);
	}
	return;
	THResFileStore fs;
	THString sPath=fs.BaseDirDialog(m_hWnd,_T("fre"));
	fs.SetBaseDirectory(sPath);
	THStringArray arf,ard,ard1;
	if (fs.FindFiles(_T("\\"),&ard,&arf))
	{
		THDebug(_T("Success"));
		for(int i=0;i<arf.GetSize();i++)
			THDebug(_T("%s"),arf[i]);
		for(i=0;i<ard.GetSize();i++)
		{
			THDebug(_T("Dir:%s"),ard[i]);
			if (fs.FindFiles(ard[i],NULL,&ard1))
			{
				for(int j=0;j<ard1.GetSize();j++)
					THDebug(_T("%s"),ard1[j]);
			}
		}
	}
	return;
	strret=THSysDialog::FileDialogOpenExistOne(m_hWnd);
	if (!strret.IsEmpty())
	{
		THSwfParser swf;
		THSwfParserError err=swf.OpenSwfFile(strret);
		if (err==THSwfParserError_SwfStreamError)
			THDebug(_T("Warning:The File Stream Have Error"));
		THString str=swf.PrintDetail();
		UINT nlen=str.GetLength();
		THDebug(str);
		THDebugFile(str);
	}
/*	THStringArray ar;
	ar.Add(_T("d:\\aaa.flv"));
	ar.Add(_T("d:\\bbb.flv"));
	ar.Add(_T("d:\\ccc.flv"));
	THFlvParser::CombinFlvFile(&ar,_T("d:\\ddd2.flv"));*/
//	THMp4Parser mp4;
//	mp4.ParseFile(_T("d:\\1.mp4"));
//	mp4.ParseFile(_T("d:\\3.mp4"));
	//mp4.ParseFile(_T("d:\\a.mp4"));
/*	THString strret=THSysDialog::FileDialogOpenExistOne(m_hWnd);
	if (!strret.IsEmpty())
	{
		THString str=THFile::GetContentAToString(strret);
		THJsonParser parser;
		THJsonObject *obj=parser.ParseJsonBuffer(str);
		if (obj)
		{
			THDebug(obj->PrintDetail());
			THDebug(obj->MakeJsonBuffer());
		}
		else
		{
			THDebug(parser.GetLastError());
		}
	}*/
}

#include <THStringParser.h>
#include <THLibMfcCtrls.h>
//#include <Dialogs\THFlashDlg.h>

void CAllTestDlg::OnBnClickedButton20()
{
	THRandomInfo info;

/*	THFlashDlg dlg(IDD);
	dlg.m_flashfile=_T("c:\\testlive.swf");
	dlg.DoModal();*/
}
