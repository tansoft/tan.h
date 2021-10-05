// MiscToolsDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "MiscTools.h"
#include "MiscToolsDlg.h"
#include ".\misctoolsdlg.h"
#include <THSysMisc.h>

#ifdef _DEBUG
//#define new DEBUG_NEW
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


// CMiscToolsDlg 对话框



CMiscToolsDlg::CMiscToolsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMiscToolsDlg::IDD, pParent)
	, m_off(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMiscToolsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_off);
}

BEGIN_MESSAGE_MAP(CMiscToolsDlg, CDialog)
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
END_MESSAGE_MAP()


// CMiscToolsDlg 消息处理程序

BOOL CMiscToolsDlg::OnInitDialog()
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
	THCommandLineParser parser;
	parser.AddParseRule(_T("img2file"),THCMDLINERULE_MASK_NOTNEED);
	parser.AddParseRule(_T("file2img"),THCMDLINERULE_MASK_NOTNEED);
	parser.AddParseRule(_T("i"),THCMDLINERULE_MASK_NORMAL|THCMDLINERULE_MASK_ICASE|THCMDLINERULE_MASK_NOTNULL);
	parser.AddParseRule(_T("o"),THCMDLINERULE_MASK_NORMAL|THCMDLINERULE_MASK_ICASE|THCMDLINERULE_MASK_NOTNULL);
	parser.AddParseRule(_T("zipmode"),THCMDLINERULE_MASK_NORMAL|THCMDLINERULE_MASK_ICASE|THCMDLINERULE_MASK_NOTNULL,_T("gzip"));
	parser.AddParseRule(_T("authmode"),THCMDLINERULE_MASK_NORMAL|THCMDLINERULE_MASK_ICASE|THCMDLINERULE_MASK_NOTNULL,_T("no"));
	if (parser.Parse())
	{
		THString i=parser.GetParamValue(_T("i"));
		THString o=parser.GetParamValue(_T("o"));
		THString z=parser.GetParamValue(_T("zipmode"));
		THString a=parser.GetParamValue(_T("authmode"));
		if (parser.GetParamValue(_T("img2file"))==_T("1"))
		{
			Img2File(i,o,z,a);
			exit(0);
		}
		else if (parser.GetParamValue(_T("file2img"))==_T("1"))
		{
			File2Img(i,o,z,a);
			exit(0);
		}
	}

	return TRUE;  // 除非设置了控件的焦点，否则返回 TRUE
}

void CMiscToolsDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CMiscToolsDlg::OnPaint() 
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
HCURSOR CMiscToolsDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

#include <THString.h>
#include <THSysMisc.h>
#include <THFile.h>

void CMiscToolsDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	//cut
	UpdateData();
	unsigned int len=THStringConv::s2num_u(m_off);
	THString str=THSysDialog::FileDialogOpenExistOne(m_hWnd,_T(""),_T("所有文件(*.*)|*.*||"),_T(""),_T(""),_T("请选择需要分割的文件"));
	if (str.IsEmpty()) return;
	THString s1=THSysDialog::FileDialogSaveOneOverWritePrompt(m_hWnd,_T(""),_T("所有文件(*.*)|*.*||"),_T(""),_T(""),_T("请选择分割的前半部分保存到，取消为不保存该部分"));
	THString s2=THSysDialog::FileDialogSaveOneOverWritePrompt(m_hWnd,_T(""),_T("所有文件(*.*)|*.*||"),_T(""),_T(""),_T("请选择分割的后半部分保存到，取消为不保存该部分"));
	if (s1.IsEmpty() && s2.IsEmpty()) return;
	unsigned int flen;
	void *buf=THFile::GetContentToBuffer(str,&flen);
	if (!buf)
	{
		MessageBox(_T("打开文件错误"));
		return;
	}
	if (flen<len)
	{
		MessageBox(_T("文件长度比分割长度小"));
		THFile::FreeContentBuffer(buf);
		return;
	}
	if (!s1.IsEmpty())
	{
		if (!THFile::SaveFile(s1,buf,len))
			MessageBox(_T("保存前半部分出错"));
	}
	if (!s2.IsEmpty())
	{
		if (!THFile::SaveFile(s2,(U8 *)buf+len,flen-len))
			MessageBox(_T("保存后半部分出错"));
	}
	THFile::FreeContentBuffer(buf);
}

void CMiscToolsDlg::OnBnClickedButton2()
{
	// TODO: Add your control notification handler code here
	//spilt
	UpdateData();
	unsigned int len=THStringConv::s2num_u(m_off);
	THString str=THSysDialog::FileDialogOpenExistOne(m_hWnd,_T(""),_T("所有文件(*.*)|*.*||"),_T(""),_T(""),_T("请选择需要分割的文件"));
	if (str.IsEmpty()) return;
	THFile file;
	if (!file.Open(str,THFile::modeRead))
	{
		MessageBox(_T("打开文件错误"));
		return;
	}
	U8 *buf=new U8[len];
	int i=0;
	THString sf;
	while(1)
	{
		UINT rlen=file.Read(buf,len);
		if (rlen==0) break;
		sf.Format(_T("%s.%d"),str,i++);
		if (!THFile::SaveFile(sf,buf,rlen))
		{
			MessageBox(_T("保存文件出错"));
			break;
		}
	}
	file.Close();
	delete [] buf;
}

void CMiscToolsDlg::OnBnClickedButton3()
{
	// TODO: Add your control notification handler code here
	//combin
	UpdateData();
	THString str=THSysDialog::FileDialogOpenExistMulti(m_hWnd,_T(""),_T("所有文件(*.*)|*.*||"),_T(""),_T(""),_T("请选择需要合并的多个文件"));
	if (str.IsEmpty()) return;
	THStringArray ar;
	THStringToken::FillStringArray(ar,str,_T("|"));
	if (ar.GetSize()<=1)
	{
		MessageBox(_T("选择的文件不够"));
		return;
	}
	THString save=THSysDialog::FileDialogSaveOneOverWritePrompt(m_hWnd,_T(""),_T("所有文件(*.*)|*.*||"),_T(""),_T(""),_T("请选择保存为的文件"));
	if (save.IsEmpty()) return;
	THFile file;
	if (!file.Open(save,THFile::modeCreate|THFile::modeWrite))
	{
		MessageBox(_T("不能保存文件"));
		return;
	}
	unsigned int len;
	void *buf;
	for(int i=0;i<ar.GetSize();i++)
	{
		buf=THFile::GetContentToBuffer(ar[i],&len);
		if (buf)
		{
			if (len>0)
				file.Write(buf,len);
			THFile::FreeContentBuffer(buf);
		}
	}
	file.Close();
}

#include <THBaseDialog.h>
#include <THDebug.h>

void CMiscToolsDlg::OnBnClickedButton4()
{
	// TODO: Add your control notification handler code here
	THBaseInputDialog input;
	THString ret=input.ShowInputDialogQuick(_T("输入二进制字符串"),_T("二进制字符串"),theApp.m_hInstance,m_hWnd);
	if (!ret.IsEmpty())
	{
		unsigned int len;
		void *buf=THStringConv::HexStringToBin(ret,&len);
		if (buf)
		{
			THString ret=THStringConv::FormatString(buf,len);
			THDebug(ret);
			MessageBox(ret);
			THStringConv::Free(buf);
		}
	}
}

#include <THIni.h>
#include <THBaseDialog.h>

void CMiscToolsDlg::OnBnClickedButton5()
{
	//encode ini
	THString str=THSysDialog::FileDialogOpenExistOne();
	THBaseInputDialog dlg;
	THString pass=dlg.ShowInputDialogQuick(_T("请输入密码："),_T("请输入密码："),theApp.m_hInstance,m_hWnd);
	if (!str.IsEmpty())
	{
		THIni ini,ini2;
		ini.Init(str);
		ini2.Init(str);
		if (!pass.IsEmpty())
		{
			ini2.InitPass(pass);
			ini2.UseEncodeCrcMode(MessageBox(_T("是否使用crc模式？默认是"),_T("是否使用crc模式？默认是"),MB_YESNO|MB_ICONQUESTION)==IDOK);
		}
		ini2.CopyIni(&ini);
	}
}

void CMiscToolsDlg::OnBnClickedButton6()
{
	//decode ini
	THString str=THSysDialog::FileDialogOpenExistOne();
	THBaseInputDialog dlg;
	THString pass=dlg.ShowInputDialogQuick(_T("请输入密码："),_T("请输入密码："),theApp.m_hInstance,m_hWnd);
	if (!str.IsEmpty())
	{
		THIni ini,ini2;
		ini.Init(str);
		ini2.Init(str);
		if (!pass.IsEmpty())
		{
			ini.InitPass(pass);
			ini.UseEncodeCrcMode(MessageBox(_T("是否使用crc模式？默认是"),_T("是否使用crc模式？默认是"),MB_YESNO|MB_ICONQUESTION)==IDOK);
		}
		ini2.CopyIni(&ini);
	}
}

#include <THArith.h>

void CMiscToolsDlg::OnBnClickedButton7()
{
	THBaseInputDialog dlg;
	THString text=dlg.ShowInputDialogQuick(_T("请输入明文："),_T("请输入明文："),theApp.m_hInstance,m_hWnd);
	if (!text.IsEmpty())
		MessageBox(THBase64::EncodeStr(text),_T("编码结果"));
	text=dlg.ShowInputDialogQuick(_T("请输入Base64："),_T("请输入Base64："),theApp.m_hInstance,m_hWnd);
	if (!text.IsEmpty())
		MessageBox(THBase64::EncodeStr(text),_T("解码结果"));
}


void CMiscToolsDlg::OnBnClickedButton8()
{
	THString nsifile=THSysDialog::FileDialogOpenExistOne(m_hWnd,_T(""),_T("Nsi模板(*.nst)|*.nst||"),_T("nst"),_T(""),_T("选择需要执行的Nsi脚本模板文件："));
	if (!nsifile.IsEmpty())
	{
		THString confile=THSysMisc::RenameFileExt(nsifile,_T(".ini"));
		if (THSysMisc::IsFile(confile))
		{
			THIni ini;
			if (ini.Init(confile))
			{
				THStringArray allfile;
				if (ini.EnumAllStruct(&allfile))
				{
					for(int i=0;i<allfile.GetSize();i++)
					{
						if (allfile[i]==_T("General")) continue;
						THString cnt=THFile::GetContentAToString(nsifile);
						THStringArray key,value;
						ini.EnumStruct(_T("General"),&key,&value);
						for(int j=0;j<key.GetSize();j++)
							cnt.Replace(_T("<--")+key[j]+_T("-->"),value[j]);
						key.RemoveAll();value.RemoveAll();
						ini.EnumStruct(allfile[i],&key,&value);
						for(j=0;j<key.GetSize();j++)
							cnt.Replace(_T("<--")+key[j]+_T("-->"),value[j]);
						THFile::SaveStringToFileA(THSysMisc::RenameFileExt(nsifile,allfile[i]+_T(".nsi")),cnt);
					}
				}
			}
			else
				MessageBox(_T("不能打开配置文件")+confile);
		}
		else
			MessageBox(_T("不能打开配置文件")+confile);
	}
}

#include <THSwfParser.h>

void CMiscToolsDlg::OnBnClickedButton9()
{
	// TODO: Add your control notification handler code here
	THString str=THSysDialog::FileDialogOpenExistOne(m_hWnd,_T(""),_T("Swf文件(*.swf)|*.swf||"),_T(""),_T(""),_T("请选择需要解压的文件"));
	if (!str.IsEmpty())
	{
		THSwfParser p;
		THSwfParserError err=p.OpenSwfFile(str);
		if (err!=THSwfParserError_None) MessageBox(_T("处理出错"));
		p.DumpSwf(str+_T(".swf"));
	}
}

#include <THFileStore.h>

void CMiscToolsDlg::OnBnClickedButton10()
{
	//file2img
	THString path=THSysDialog::DirDialog(m_hWnd,_T("请选择需要映像的目录"));
	if (path.IsEmpty()) return;
	THString outfile=THSysDialog::FileDialogSaveOneOverWritePrompt(m_hWnd,_T("New.img"),_T("天塑映像文件(*.img)|*.img||"),_T("img"),_T(""),_T("请选择映像文件保存的目录"));
	if (outfile.IsEmpty()) return;
	THBaseInputDialog input;
	THString pass=input.ShowInputDialogQuick(_T("输入密码"),_T("请输入映像文件密码，为空为不使用密码："),theApp.m_hInstance,m_hWnd,FALSE);
	if (pass.IsEmpty()) pass=_T("no");
	else pass=_T("pass")+pass;
	MessageBox(_T("拷贝结果：")+File2Img(path,outfile,_T("gzip"),pass));
}

void CMiscToolsDlg::OnBnClickedButton11()
{
	//img2file
	THString infile=THSysDialog::FileDialogOpenExistOne(m_hWnd,_T(""),_T("天塑映像文件(*.img)|*.img||"),_T("img"),_T(""),_T("请选择映像文件"));
	if (infile.IsEmpty()) return;
	THString path=THSysDialog::DirDialog(m_hWnd,_T("请选择映像输出的目录"));
	if (path.IsEmpty()) return;
	THBaseInputDialog input;
	THString pass=input.ShowInputDialogQuick(_T("输入密码"),_T("请输入映像文件密码，为空为没有密码："),theApp.m_hInstance,m_hWnd,FALSE);
	if (pass.IsEmpty()) pass=_T("no");
	else pass=_T("pass")+pass;
	MessageBox(_T("拷贝结果：")+Img2File(infile,path,_T("gzip"),pass));
}

THString CMiscToolsDlg::Img2File(THString input,THString output,THString zipmode,THString authmode)
{
	THString ret;
	::DeleteFile(output);
	THFileStore fs;
	THImgFileStore ifs;
	ITHFileStore *in=ifs.CreateObject();
	ITHFileStore *out=fs.CreateObject();
	in->SetFreeor(this);
	out->SetFreeor(this);
	in->SetStoreAttrib(_T("zipmode"),zipmode);
	in->SetStoreAttrib(_T("authmode"),authmode);
	in->SetBaseDirectory(input);
	out->SetBaseDirectory(output);
	if (in->CopyFile(_T(""),_T(""),out))
		ret=_T("成功！");
	in->DeleteObjectByFreeor(this);
	out->DeleteObjectByFreeor(this);
	return ret;
}

THString CMiscToolsDlg::File2Img(THString input,THString output,THString zipmode,THString authmode)
{
	THString ret;
	::DeleteFile(output);
	THFileStore fs;
	THImgFileStore ifs;
	ITHFileStore *in=fs.CreateObject();
	ITHFileStore *out=ifs.CreateObject();
	in->SetFreeor(this);
	out->SetFreeor(this);
	in->SetBaseDirectory(input);
	out->SetBaseDirectory(output);
	out->SetStoreAttrib(_T("zipmode"),zipmode);
	out->SetStoreAttrib(_T("authmode"),authmode);
	if (in->CopyFile(_T(""),_T(""),out))
		ret=out->GetStoreAttrib(_T("storestate"));
	in->DeleteObjectByFreeor(this);
	out->DeleteObjectByFreeor(this);
	return ret;
}

#include <THDialogOper.h>

void CMiscToolsDlg::OnBnClickedButton12()
{
	// TODO: Add your control notification handler code here
	POINT pt;
	HWND hWnd=m_hWnd;
	if (::GetCursorPos(&pt)) {
		hWnd=::WindowFromPoint(pt);
		/*HWND hWndP;
		while(true) {
			::IsWindow(hWndP);
			hWndP=::GetParent(hWnd);
			if (!hWndP) break;
			hWnd=hWndP;
		}*/
	}
	THDlgItemEnumer enumer;
	enumer.EnumAllSubWindows(hWnd);
}
