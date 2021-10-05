// SkinDemo.cpp : implementation file
//

#include "stdafx.h"
#include "AllTest.h"
#include "SkinDemo.h"
#include ".\skindemo.h"
#include <THSystem.h>
#include <THSysMisc.h>

// CSkinDemo dialog

IMPLEMENT_DYNAMIC(CSkinDemo, CDialog)
CSkinDemo::CSkinDemo(CWnd* pParent /*=NULL*/)
	: CDialog(CSkinDemo::IDD, pParent)
{
}

CSkinDemo::~CSkinDemo()
{
}

void CSkinDemo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBOBOXEX1, m_comboex1);
	DDX_Control(pDX, IDC_COMBOBOXEX2, m_comboex2);
}


BEGIN_MESSAGE_MAP(CSkinDemo, CDialog)
	ON_BN_CLICKED(IDC_BUTTON5, OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON6, OnBnClickedButton6)
END_MESSAGE_MAP()


// CSkinDemo message handlers

BOOL CSkinDemo::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	GetDlgItem(IDC_BUTTON2)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON4)->EnableWindow(FALSE);
	GetDlgItem(IDC_CHECK4)->EnableWindow(FALSE);
	GetDlgItem(IDC_CHECK5)->EnableWindow(FALSE);
	GetDlgItem(IDC_CHECK6)->EnableWindow(FALSE);
	GetDlgItem(IDC_CHECK10)->EnableWindow(FALSE);
	GetDlgItem(IDC_CHECK11)->EnableWindow(FALSE);
	GetDlgItem(IDC_CHECK12)->EnableWindow(FALSE);
	GetDlgItem(IDC_CHECK16)->EnableWindow(FALSE);
	GetDlgItem(IDC_CHECK17)->EnableWindow(FALSE);
	GetDlgItem(IDC_CHECK18)->EnableWindow(FALSE);
	GetDlgItem(IDC_CHECK22)->EnableWindow(FALSE);
	GetDlgItem(IDC_CHECK23)->EnableWindow(FALSE);
	GetDlgItem(IDC_CHECK24)->EnableWindow(FALSE);
	((CButton *)GetDlgItem(IDC_CHECK2))->SetCheck(BST_CHECKED);
	((CButton *)GetDlgItem(IDC_CHECK8))->SetCheck(BST_CHECKED);
	((CButton *)GetDlgItem(IDC_CHECK14))->SetCheck(BST_CHECKED);
	((CButton *)GetDlgItem(IDC_CHECK20))->SetCheck(BST_CHECKED);
	((CButton *)GetDlgItem(IDC_CHECK5))->SetCheck(BST_CHECKED);
	((CButton *)GetDlgItem(IDC_CHECK11))->SetCheck(BST_CHECKED);
	((CButton *)GetDlgItem(IDC_CHECK17))->SetCheck(BST_CHECKED);
	((CButton *)GetDlgItem(IDC_CHECK23))->SetCheck(BST_CHECKED);

	((CButton *)GetDlgItem(IDC_CHECK3))->SetCheck(BST_INDETERMINATE);
	((CButton *)GetDlgItem(IDC_CHECK9))->SetCheck(BST_INDETERMINATE);
	((CButton *)GetDlgItem(IDC_CHECK15))->SetCheck(BST_INDETERMINATE);
	((CButton *)GetDlgItem(IDC_CHECK21))->SetCheck(BST_INDETERMINATE);
	((CButton *)GetDlgItem(IDC_CHECK6))->SetCheck(BST_INDETERMINATE);
	((CButton *)GetDlgItem(IDC_CHECK12))->SetCheck(BST_INDETERMINATE);
	((CButton *)GetDlgItem(IDC_CHECK18))->SetCheck(BST_INDETERMINATE);
	((CButton *)GetDlgItem(IDC_CHECK24))->SetCheck(BST_INDETERMINATE);

	GetDlgItem(IDC_EDIT1)->SetWindowText(_T("文本1"));
	GetDlgItem(IDC_EDIT2)->SetWindowText(_T("Password"));
	GetDlgItem(IDC_EDIT3)->SetWindowText(_T("文本3"));
	GetDlgItem(IDC_EDIT4)->SetWindowText(_T("文本4"));
	GetDlgItem(IDC_EDIT5)->SetWindowText(_T("文本5\r\n文本5"));
	GetDlgItem(IDC_EDIT6)->SetWindowText(_T("文本6\r\n文本6"));
	GetDlgItem(IDC_EDIT7)->SetWindowText(_T("文本7\r\n文本7"));
	GetDlgItem(IDC_EDIT4)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT7)->EnableWindow(FALSE);

	GetDlgItem(IDC_RADIO3)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIO4)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIO7)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIO8)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIO11)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIO12)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIO15)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIO16)->EnableWindow(FALSE);

	((CButton *)GetDlgItem(IDC_RADIO2))->SetCheck(BST_CHECKED);
	((CButton *)GetDlgItem(IDC_RADIO4))->SetCheck(BST_CHECKED);
	((CButton *)GetDlgItem(IDC_RADIO6))->SetCheck(BST_CHECKED);
	((CButton *)GetDlgItem(IDC_RADIO8))->SetCheck(BST_CHECKED);
	((CButton *)GetDlgItem(IDC_RADIO10))->SetCheck(BST_CHECKED);
	((CButton *)GetDlgItem(IDC_RADIO12))->SetCheck(BST_CHECKED);
	((CButton *)GetDlgItem(IDC_RADIO14))->SetCheck(BST_CHECKED);
	((CButton *)GetDlgItem(IDC_RADIO16))->SetCheck(BST_CHECKED);

	THString str;
	for(int j=0;j<10;j++)
	{
		str.Format(_T("文本%d"),j+1);
		((CComboBox *)GetDlgItem(IDC_COMBO1))->AddString(str);
		((CComboBox *)GetDlgItem(IDC_COMBO2))->AddString(str);
		((CComboBox *)GetDlgItem(IDC_COMBO3))->AddString(str);
		((CComboBox *)GetDlgItem(IDC_COMBO4))->AddString(str);
		((CComboBox *)GetDlgItem(IDC_COMBO5))->AddString(str);
		((CComboBox *)GetDlgItem(IDC_COMBO6))->AddString(str);
	}
	((CComboBox *)GetDlgItem(IDC_COMBO1))->SetCurSel(0);
	((CComboBox *)GetDlgItem(IDC_COMBO2))->SetCurSel(0);
	((CComboBox *)GetDlgItem(IDC_COMBO3))->SetCurSel(0);
	((CComboBox *)GetDlgItem(IDC_COMBO4))->SetCurSel(0);
	((CComboBox *)GetDlgItem(IDC_COMBO5))->SetCurSel(0);
	((CComboBox *)GetDlgItem(IDC_COMBO6))->SetCurSel(0);

	GetDlgItem(IDC_COMBO2)->EnableWindow(FALSE);
	GetDlgItem(IDC_COMBO4)->EnableWindow(FALSE);
	GetDlgItem(IDC_COMBO6)->EnableWindow(FALSE);
	COMBOBOXEXITEM item;
	for(j=0;j<10;j++)
	{
		str.Format(_T("文本%d"),j+1);
		item.mask=CBEIF_TEXT;
		item.iItem=0;
		item.iImage=0;
		item.pszText=str.GetBuffer();
		m_comboex1.InsertItem(&item);
		m_comboex2.InsertItem(&item);
		str.ReleaseBuffer();
	}
	m_comboex1.SetCurSel(0);
	m_comboex2.SetCurSel(0);
	GetDlgItem(IDC_COMBOBOXEX2)->EnableWindow(FALSE);

	GetDlgItem(IDC_SPIN2)->EnableWindow(FALSE);
	GetDlgItem(IDC_SPIN4)->EnableWindow(FALSE);
	GetDlgItem(IDC_SLIDER2)->EnableWindow(FALSE);
	GetDlgItem(IDC_HOTKEY2)->EnableWindow(FALSE);
	GetDlgItem(IDC_PROGRESS2)->EnableWindow(FALSE);
	GetDlgItem(IDC_PROGRESS4)->EnableWindow(FALSE);
	GetDlgItem(IDC_PROGRESS6)->EnableWindow(FALSE);
	((CProgressCtrl *)GetDlgItem(IDC_PROGRESS1))->SetRange(0,100);
	((CProgressCtrl *)GetDlgItem(IDC_PROGRESS1))->SetPos(60);
	((CProgressCtrl *)GetDlgItem(IDC_PROGRESS2))->SetRange(0,100);
	((CProgressCtrl *)GetDlgItem(IDC_PROGRESS2))->SetPos(60);
	((CProgressCtrl *)GetDlgItem(IDC_PROGRESS3))->SetRange(0,100);
	((CProgressCtrl *)GetDlgItem(IDC_PROGRESS3))->SetPos(60);
	((CProgressCtrl *)GetDlgItem(IDC_PROGRESS4))->SetRange(0,100);
	((CProgressCtrl *)GetDlgItem(IDC_PROGRESS4))->SetPos(60);
	((CProgressCtrl *)GetDlgItem(IDC_PROGRESS5))->SetRange(0,100);
	((CProgressCtrl *)GetDlgItem(IDC_PROGRESS5))->SetPos(60);
	((CProgressCtrl *)GetDlgItem(IDC_PROGRESS6))->SetRange(0,100);
	((CProgressCtrl *)GetDlgItem(IDC_PROGRESS6))->SetPos(60);

	GetDlgItem(IDC_STATIC1)->EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC2)->EnableWindow(FALSE);

	GetDlgItem(IDC_TREE2)->EnableWindow(FALSE);
	GetDlgItem(IDC_TREE4)->EnableWindow(FALSE);
	GetDlgItem(IDC_TREE6)->EnableWindow(FALSE);

	CTreeCtrl *ctrl[6];
	ctrl[0]=(CTreeCtrl *)GetDlgItem(IDC_TREE1);
	ctrl[1]=(CTreeCtrl *)GetDlgItem(IDC_TREE2);
	ctrl[2]=(CTreeCtrl *)GetDlgItem(IDC_TREE3);
	ctrl[3]=(CTreeCtrl *)GetDlgItem(IDC_TREE4);
	ctrl[4]=(CTreeCtrl *)GetDlgItem(IDC_TREE5);
	ctrl[5]=(CTreeCtrl *)GetDlgItem(IDC_TREE6);
	HTREEITEM htree;
	for(int i=0;i<6;i++)
	{
		htree=ctrl[i]->InsertItem(_T("根项"));
		for(j=0;j<5;j++)
			ctrl[i]->InsertItem(_T("子项"),htree);
		ctrl[i]->Expand(htree,TVE_EXPAND);
	}

	GetDlgItem(IDC_DATETIMEPICKER2)->EnableWindow(FALSE);
	GetDlgItem(IDC_DATETIMEPICKER4)->EnableWindow(FALSE);

	for(i=0;i<10;i++)
	{
		str.Format(_T("文本%d"),i+1);
		((CListBox *)GetDlgItem(IDC_LIST1))->InsertString(i,str);
		((CListBox *)GetDlgItem(IDC_LIST2))->InsertString(i,str);
	}
	((CListBox *)GetDlgItem(IDC_LIST1))->SetCurSel(0);
	((CListBox *)GetDlgItem(IDC_LIST2))->SetCurSel(0);
	GetDlgItem(IDC_LIST2)->EnableWindow(FALSE);
	GetDlgItem(IDC_LIST4)->EnableWindow(FALSE);
	GetDlgItem(IDC_LIST6)->EnableWindow(FALSE);
	GetDlgItem(IDC_LIST8)->EnableWindow(FALSE);
	GetDlgItem(IDC_LIST10)->EnableWindow(FALSE);

	CListCtrl *lctrl[8];
	lctrl[0]=(CListCtrl *)GetDlgItem(IDC_LIST3);
	lctrl[1]=(CListCtrl *)GetDlgItem(IDC_LIST4);
	lctrl[2]=(CListCtrl *)GetDlgItem(IDC_LIST5);
	lctrl[3]=(CListCtrl *)GetDlgItem(IDC_LIST6);
	lctrl[4]=(CListCtrl *)GetDlgItem(IDC_LIST7);
	lctrl[5]=(CListCtrl *)GetDlgItem(IDC_LIST8);
	lctrl[6]=(CListCtrl *)GetDlgItem(IDC_LIST9);
	lctrl[7]=(CListCtrl *)GetDlgItem(IDC_LIST10);

	for(i=0;i<8;i++)
	{
		lctrl[i]->InsertColumn(0,_T("标题1"),LVCFMT_LEFT,100);
		for(j=0;j<10;j++)
		{
			str.Format(_T("文本%d"),j+1);
			lctrl[i]->InsertItem(0,str);
		}
	}

	CTabCtrl *tctrl[6];
	tctrl[0]=(CTabCtrl *)GetDlgItem(IDC_TAB1);
	tctrl[1]=(CTabCtrl *)GetDlgItem(IDC_TAB2);
	tctrl[2]=(CTabCtrl *)GetDlgItem(IDC_TAB3);
	tctrl[3]=(CTabCtrl *)GetDlgItem(IDC_TAB4);
	tctrl[4]=(CTabCtrl *)GetDlgItem(IDC_TAB5);
	tctrl[5]=(CTabCtrl *)GetDlgItem(IDC_TAB6);
	for(i=0;i<6;i++)
	{
		for(j=0;j<10;j++)
		{
			str.Format(_T("文本%d"),j+1);
			tctrl[i]->InsertItem(j,str);
		}
	}
	GetDlgItem(IDC_TAB2)->EnableWindow(FALSE);
	GetDlgItem(IDC_TAB4)->EnableWindow(FALSE);
	GetDlgItem(IDC_TAB6)->EnableWindow(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CSkinDemo::OnBnClickedButton5()
{
	// TODO: Add your control notification handler code here
	THString str=THSysDialog::FileDialogOpenExistOne(NULL,_T(""),_T("皮肤文件(*.ssk)|*.ssk||"),_T("ssk"),_T(""),_T("请选择应用的皮肤文件"));
	if (str.IsEmpty()) return;
	char *chTmp=THCharset::t2a(str);
	if (chTmp)
	{
		skinppLoadSkin(chTmp);
		THCharset::free(chTmp);
	}
}

void CSkinDemo::OnBnClickedButton6()
{
	// TODO: Add your control notification handler code here
	skinppRemoveSkin();
}
