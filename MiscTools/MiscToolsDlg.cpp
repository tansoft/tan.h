// MiscToolsDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "MiscTools.h"
#include "MiscToolsDlg.h"
#include ".\misctoolsdlg.h"
#include <THSysMisc.h>

#ifdef _DEBUG
//#define new DEBUG_NEW
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


// CMiscToolsDlg �Ի���



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


// CMiscToolsDlg ��Ϣ�������

BOOL CMiscToolsDlg::OnInitDialog()
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

	return TRUE;  // ���������˿ؼ��Ľ��㣬���򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CMiscToolsDlg::OnPaint() 
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
	THString str=THSysDialog::FileDialogOpenExistOne(m_hWnd,_T(""),_T("�����ļ�(*.*)|*.*||"),_T(""),_T(""),_T("��ѡ����Ҫ�ָ���ļ�"));
	if (str.IsEmpty()) return;
	THString s1=THSysDialog::FileDialogSaveOneOverWritePrompt(m_hWnd,_T(""),_T("�����ļ�(*.*)|*.*||"),_T(""),_T(""),_T("��ѡ��ָ��ǰ�벿�ֱ��浽��ȡ��Ϊ������ò���"));
	THString s2=THSysDialog::FileDialogSaveOneOverWritePrompt(m_hWnd,_T(""),_T("�����ļ�(*.*)|*.*||"),_T(""),_T(""),_T("��ѡ��ָ�ĺ�벿�ֱ��浽��ȡ��Ϊ������ò���"));
	if (s1.IsEmpty() && s2.IsEmpty()) return;
	unsigned int flen;
	void *buf=THFile::GetContentToBuffer(str,&flen);
	if (!buf)
	{
		MessageBox(_T("���ļ�����"));
		return;
	}
	if (flen<len)
	{
		MessageBox(_T("�ļ����ȱȷָ��С"));
		THFile::FreeContentBuffer(buf);
		return;
	}
	if (!s1.IsEmpty())
	{
		if (!THFile::SaveFile(s1,buf,len))
			MessageBox(_T("����ǰ�벿�ֳ���"));
	}
	if (!s2.IsEmpty())
	{
		if (!THFile::SaveFile(s2,(U8 *)buf+len,flen-len))
			MessageBox(_T("�����벿�ֳ���"));
	}
	THFile::FreeContentBuffer(buf);
}

void CMiscToolsDlg::OnBnClickedButton2()
{
	// TODO: Add your control notification handler code here
	//spilt
	UpdateData();
	unsigned int len=THStringConv::s2num_u(m_off);
	THString str=THSysDialog::FileDialogOpenExistOne(m_hWnd,_T(""),_T("�����ļ�(*.*)|*.*||"),_T(""),_T(""),_T("��ѡ����Ҫ�ָ���ļ�"));
	if (str.IsEmpty()) return;
	THFile file;
	if (!file.Open(str,THFile::modeRead))
	{
		MessageBox(_T("���ļ�����"));
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
			MessageBox(_T("�����ļ�����"));
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
	THString str=THSysDialog::FileDialogOpenExistMulti(m_hWnd,_T(""),_T("�����ļ�(*.*)|*.*||"),_T(""),_T(""),_T("��ѡ����Ҫ�ϲ��Ķ���ļ�"));
	if (str.IsEmpty()) return;
	THStringArray ar;
	THStringToken::FillStringArray(ar,str,_T("|"));
	if (ar.GetSize()<=1)
	{
		MessageBox(_T("ѡ����ļ�����"));
		return;
	}
	THString save=THSysDialog::FileDialogSaveOneOverWritePrompt(m_hWnd,_T(""),_T("�����ļ�(*.*)|*.*||"),_T(""),_T(""),_T("��ѡ�񱣴�Ϊ���ļ�"));
	if (save.IsEmpty()) return;
	THFile file;
	if (!file.Open(save,THFile::modeCreate|THFile::modeWrite))
	{
		MessageBox(_T("���ܱ����ļ�"));
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
	THString ret=input.ShowInputDialogQuick(_T("����������ַ���"),_T("�������ַ���"),theApp.m_hInstance,m_hWnd);
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
	THString pass=dlg.ShowInputDialogQuick(_T("���������룺"),_T("���������룺"),theApp.m_hInstance,m_hWnd);
	if (!str.IsEmpty())
	{
		THIni ini,ini2;
		ini.Init(str);
		ini2.Init(str);
		if (!pass.IsEmpty())
		{
			ini2.InitPass(pass);
			ini2.UseEncodeCrcMode(MessageBox(_T("�Ƿ�ʹ��crcģʽ��Ĭ����"),_T("�Ƿ�ʹ��crcģʽ��Ĭ����"),MB_YESNO|MB_ICONQUESTION)==IDOK);
		}
		ini2.CopyIni(&ini);
	}
}

void CMiscToolsDlg::OnBnClickedButton6()
{
	//decode ini
	THString str=THSysDialog::FileDialogOpenExistOne();
	THBaseInputDialog dlg;
	THString pass=dlg.ShowInputDialogQuick(_T("���������룺"),_T("���������룺"),theApp.m_hInstance,m_hWnd);
	if (!str.IsEmpty())
	{
		THIni ini,ini2;
		ini.Init(str);
		ini2.Init(str);
		if (!pass.IsEmpty())
		{
			ini.InitPass(pass);
			ini.UseEncodeCrcMode(MessageBox(_T("�Ƿ�ʹ��crcģʽ��Ĭ����"),_T("�Ƿ�ʹ��crcģʽ��Ĭ����"),MB_YESNO|MB_ICONQUESTION)==IDOK);
		}
		ini2.CopyIni(&ini);
	}
}

#include <THArith.h>

void CMiscToolsDlg::OnBnClickedButton7()
{
	THBaseInputDialog dlg;
	THString text=dlg.ShowInputDialogQuick(_T("���������ģ�"),_T("���������ģ�"),theApp.m_hInstance,m_hWnd);
	if (!text.IsEmpty())
		MessageBox(THBase64::EncodeStr(text),_T("������"));
	text=dlg.ShowInputDialogQuick(_T("������Base64��"),_T("������Base64��"),theApp.m_hInstance,m_hWnd);
	if (!text.IsEmpty())
		MessageBox(THBase64::EncodeStr(text),_T("������"));
}


void CMiscToolsDlg::OnBnClickedButton8()
{
	THString nsifile=THSysDialog::FileDialogOpenExistOne(m_hWnd,_T(""),_T("Nsiģ��(*.nst)|*.nst||"),_T("nst"),_T(""),_T("ѡ����Ҫִ�е�Nsi�ű�ģ���ļ���"));
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
				MessageBox(_T("���ܴ������ļ�")+confile);
		}
		else
			MessageBox(_T("���ܴ������ļ�")+confile);
	}
}

#include <THSwfParser.h>

void CMiscToolsDlg::OnBnClickedButton9()
{
	// TODO: Add your control notification handler code here
	THString str=THSysDialog::FileDialogOpenExistOne(m_hWnd,_T(""),_T("Swf�ļ�(*.swf)|*.swf||"),_T(""),_T(""),_T("��ѡ����Ҫ��ѹ���ļ�"));
	if (!str.IsEmpty())
	{
		THSwfParser p;
		THSwfParserError err=p.OpenSwfFile(str);
		if (err!=THSwfParserError_None) MessageBox(_T("�������"));
		p.DumpSwf(str+_T(".swf"));
	}
}

#include <THFileStore.h>

void CMiscToolsDlg::OnBnClickedButton10()
{
	//file2img
	THString path=THSysDialog::DirDialog(m_hWnd,_T("��ѡ����Ҫӳ���Ŀ¼"));
	if (path.IsEmpty()) return;
	THString outfile=THSysDialog::FileDialogSaveOneOverWritePrompt(m_hWnd,_T("New.img"),_T("����ӳ���ļ�(*.img)|*.img||"),_T("img"),_T(""),_T("��ѡ��ӳ���ļ������Ŀ¼"));
	if (outfile.IsEmpty()) return;
	THBaseInputDialog input;
	THString pass=input.ShowInputDialogQuick(_T("��������"),_T("������ӳ���ļ����룬Ϊ��Ϊ��ʹ�����룺"),theApp.m_hInstance,m_hWnd,FALSE);
	if (pass.IsEmpty()) pass=_T("no");
	else pass=_T("pass")+pass;
	MessageBox(_T("���������")+File2Img(path,outfile,_T("gzip"),pass));
}

void CMiscToolsDlg::OnBnClickedButton11()
{
	//img2file
	THString infile=THSysDialog::FileDialogOpenExistOne(m_hWnd,_T(""),_T("����ӳ���ļ�(*.img)|*.img||"),_T("img"),_T(""),_T("��ѡ��ӳ���ļ�"));
	if (infile.IsEmpty()) return;
	THString path=THSysDialog::DirDialog(m_hWnd,_T("��ѡ��ӳ�������Ŀ¼"));
	if (path.IsEmpty()) return;
	THBaseInputDialog input;
	THString pass=input.ShowInputDialogQuick(_T("��������"),_T("������ӳ���ļ����룬Ϊ��Ϊû�����룺"),theApp.m_hInstance,m_hWnd,FALSE);
	if (pass.IsEmpty()) pass=_T("no");
	else pass=_T("pass")+pass;
	MessageBox(_T("���������")+Img2File(infile,path,_T("gzip"),pass));
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
		ret=_T("�ɹ���");
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
