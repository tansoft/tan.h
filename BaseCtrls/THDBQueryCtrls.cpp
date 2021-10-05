// THDBQueryTreeCtrl.cpp : implementation file
//

#include "stdafx.h"
#include <THStruct.h>
#include <THTime.h>
#include <THFile.h>
#include "THDBQueryCtrls.h"
#include ".\thdbqueryctrls.h"

// THDBQueryTreeCtrl

IMPLEMENT_DYNAMIC(THDBQueryTreeCtrl, CTreeCtrl)
THDBQueryTreeCtrl::THDBQueryTreeCtrl()
{
	m_qdb=NULL;
	m_hRoot=NULL;
	m_bUseImage=FALSE;
}

THDBQueryTreeCtrl::~THDBQueryTreeCtrl()
{
}


BEGIN_MESSAGE_MAP(THDBQueryTreeCtrl, CTreeCtrl)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnTvnSelchanged)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

// THDBQueryTreeCtrl message handlers

void THDBQueryTreeCtrl::Init(THDBQuery *qdb,THString sRoot,THBitmap *bm,COLORREF crMask)
{
	m_qdb=qdb;
	if (m_qdb) m_qdb->AddNotifyCb(this);
	m_hRoot=InsertItem(sRoot);
	if (bm)
	{
		int x,y;
		bm->GetBmpSize(x,y);
		m_list.Create(y,y,ILC_COLOR32|ILC_MASK,10,10);
		CBitmap cbm;
		cbm.Attach(bm->GetBitmap());
		m_list.Add(&cbm,crMask);
		cbm.Detach();
		SetImageList(&m_list,TVSIL_NORMAL);
		SetItemImage(m_hRoot,0,0);
		m_bUseImage=TRUE;
	}
}

void THDBQueryTreeCtrl::UnInit()
{
	if (m_qdb) m_qdb->RemoveNotifyCb(this);
	m_qdb=NULL;
}

HTREEITEM THDBQueryTreeCtrl::FindItemByData(int data,HTREEITEM hParent)
{
	if (!m_qdb) return NULL;
	HTREEITEM hItem=GetNextItem(hParent,TVGN_CHILD);
	HTREEITEM ret=NULL;
	THString strdata=THi2s(data);
	while(hItem)
	{
		if (m_qdb->GetContent((int)GetItemData(hItem),0)==strdata)
			return hItem;
		ret=FindItemByData(data,hItem);
		if (ret) return ret;
		hItem=GetNextItem(hItem,TVGN_NEXT);
	}
	return NULL;
}

void THDBQueryTreeCtrl::OnQueryDataFinish(THDBQuery *cls,int nRow,int nCol)
{
	HTREEITEM hTreeItem,hTreeItem1;
	hTreeItem=GetNextItem(m_hRoot,TVGN_CHILD);
	while(hTreeItem)
	{
		hTreeItem1=GetNextItem(hTreeItem,TVGN_NEXT);
		if (hTreeItem!=NULL)
			DeleteItem(hTreeItem);
		hTreeItem=hTreeItem1;
	}
	if (nCol>=3 && m_qdb)
	{
		THList<int>tmp,tmp1;
		for(int i=0;i<nRow;i++)
			tmp.AddHead(i);
		int idx;
		HTREEITEM hItem;
		while(tmp.GetSize()>0)
		{
			THPosition pos=tmp.GetStartPosition();
			BOOL bFound=FALSE;
			while(!pos.IsEmpty())
			{
				if (tmp.GetNextPosition(pos,idx))
				{
					int parent=THs2i(m_qdb->GetContent(idx,1));
					if (parent==0)
					{
						hItem=InsertItem(m_qdb->GetContent(idx,2),m_hRoot);
						Expand(m_hRoot,TVE_EXPAND);
						SetItemData(hItem,idx);//THs2i(m_qdb->GetContent(idx,0))
						if (nCol>=4 && m_bUseImage)
						{
							int imgidx=THs2i(m_qdb->GetContent(idx,3));
							SetItemImage(hItem,imgidx,imgidx);
						}
						bFound=TRUE;
					}
					else
					{
						hItem=FindItemByData(parent,m_hRoot);
						if (hItem)
						{
							HTREEITEM hSubItem=InsertItem(m_qdb->GetContent(idx,2),hItem);
							Expand(hItem,TVE_EXPAND);
							SetItemData(hSubItem,idx);//THs2i(m_qdb->GetContent(idx,0))
							if (nCol>=4 && m_bUseImage)
							{
								int imgidx=THs2i(m_qdb->GetContent(idx,3));
								SetItemImage(hSubItem,imgidx,imgidx);
							}
							bFound=TRUE;
						}
						else
						{
							tmp1.AddHead(idx);
						}
					}
				}
			}
			if (bFound==FALSE)
			{
				MessageBox(_T("警告：读取数据库数据出错，数据可能不能完全显示！"));
				break;
			}
			tmp.RemoveAll();
			tmp.Copy(&tmp1);
			tmp1.RemoveAll();
		}
	}
}

void THDBQueryTreeCtrl::OnSelectionChange(THDBQuery *cls,int nOld,int nNew)
{
	if (nNew==-1)
	{
		//当新建一个对象时，tree很可能需要用来确定上级，因此不进行选中还原
		if (m_qdb && m_qdb->GetModifyFlag()) return;
		SelectItem(m_hRoot);
	}
}

void THDBQueryTreeCtrl::OnModifyChange(THDBQuery *cls,BOOL bModify)
{
	if (bModify==FALSE)
	{
		//可能的情况是，保存对象后，这时数据库没有指对应数据时，还原没有选中状态
		if (m_qdb && !m_qdb->GetModifyFlag() && m_qdb->GetSelection()==-1)
			SelectItem(m_hRoot);
	}
}

BOOL THDBQueryTreeCtrl::OnSaveModify(THDBQuery *cls,int idx)
{
	if (m_qdb)
	{
		if (idx==-1)
		{
			//new 模式下,提供parent
			THString idx=_T("0");
			HTREEITEM hItem=GetSelectedItem();
			if (hItem && hItem!=m_hRoot)
				idx=m_qdb->GetContent((int)GetItemData(hItem),0);
			m_qdb->SetContentChange(1,idx);
		}
	}
	return TRUE;
}

BOOL THDBQueryTreeCtrl::OnDeleteRecord(THDBQuery *cls,int idx)
{
	HTREEITEM hItem=GetSelectedItem();
	if (hItem)
	{
		int myidx=(int)GetItemData(hItem);
		if (myidx==idx)
		{
			if (GetNextItem(hItem,TVGN_CHILD))
			{
				THString str;
				THString text=GetItemText(hItem);
				str.Format(_T("不能删除\"%s\"！\"%s\"下还包含数据，请先删除其下的所有数据。"),text,text);
				MessageBox(str,_T("不能删除"),MB_OK|MB_ICONQUESTION);
				return FALSE;
			}
		}
	}
	return TRUE;
}

void THDBQueryTreeCtrl::OnTvnSelchanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	HTREEITEM hItem=GetSelectedItem();
	if (hItem)
	{
		int idx=(int)GetItemData(hItem);
		if (hItem==m_hRoot) idx=-1;
		if (m_qdb->GetSelection()!=idx)
			m_qdb->ChangeSelection(idx);
	}
	*pResult = 0;
}

void THDBQueryTreeCtrl::OnDestroy()
{
	UnInit();
	__super::OnDestroy();
}


IMPLEMENT_DYNAMIC(THDBQueryComboBox, CComboBox)
THDBQueryComboBox::THDBQueryComboBox()
{
	m_qdb=NULL;
	m_bTreeMode=FALSE;
}

THDBQueryComboBox::~THDBQueryComboBox()
{
}


BEGIN_MESSAGE_MAP(THDBQueryComboBox, CComboBox)
	ON_CONTROL_REFLECT(CBN_SELCHANGE, OnCbnSelchange)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

void THDBQueryComboBox::Init(THDBQuery *qdb,BOOL bTreeMode)
{
	m_qdb=qdb;
	m_bTreeMode=bTreeMode;
	if (m_qdb) m_qdb->AddNotifyCb(this);
}

void THDBQueryComboBox::UnInit()
{
	if (m_qdb) m_qdb->RemoveNotifyCb(this);
	m_qdb=NULL;
}

THString THDBQueryComboBox::FindParentString(int parent)
{
	THString str;
	THString strdata=THi2s(parent);
	for(int i=0;i<GetCount();i++)
	{
		if (m_qdb->GetContent((int)GetItemData(i),0)==strdata)
		{
			GetLBText(i,str);
			return str;
		}
	}
	return _T("");
}

void THDBQueryComboBox::OnQueryDataFinish(THDBQuery *cls,int nRow,int nCol)
{
	ResetContent();
	if (m_qdb)
	{
		if (m_bTreeMode)
		{
			if (nCol>=3)
			{
				THList<int>tmp,tmp1;
				for(int i=0;i<nRow;i++)
					tmp.AddHead(i);
				int idx;
				while(tmp.GetSize()>0)
				{
					THPosition pos=tmp.GetStartPosition();
					BOOL bFound=FALSE;
					while(!pos.IsEmpty())
					{
						if (tmp.GetNextPosition(pos,idx))
						{
							int parent=THs2i(m_qdb->GetContent(idx,1));
							if (parent==0)
							{
								int item=AddString(m_qdb->GetContent(idx,2));
								SetItemData(item,idx);
								bFound=TRUE;
							}
							else
							{
								THString strtmp=FindParentString(parent);
								if (!strtmp.IsEmpty())
								{
									int item=AddString(strtmp+_T("->")+m_qdb->GetContent(idx,2));
									SetItemData(item,idx);
									bFound=TRUE;
								}
								else
								{
									tmp1.AddHead(idx);
								}
							}
						}
					}
					if (bFound==FALSE)
					{
						MessageBox(_T("警告：读取数据库数据出错，数据可能不能完全显示！"));
						break;
					}
					tmp.RemoveAll();
					tmp.Copy(&tmp1);
					tmp1.RemoveAll();
				}
			}
		}
		else
		{
			if (nCol>=2)
			{
				for(int i=0;i<nRow;i++)
				{
					int item=AddString(m_qdb->GetContent(i,1));
					SetItemData(item,i);
				}
			}
		}
	}
}

void THDBQueryComboBox::OnCbnSelchange()
{
	// TODO: Add your control notification handler code here
	UpdateData();
	if (m_qdb)
	{
		int idx=GetCurSel();
		if (idx==-1)
			m_qdb->ChangeSelection(idx);
		m_qdb->ChangeSelection((int)GetItemData(idx));
	}
}

void THDBQueryComboBox::OnDestroy()
{
	UnInit();
	__super::OnDestroy();
}

IMPLEMENT_DYNAMIC(THDBQueryMultiSelectComboBox, CComboBox)
THDBQueryMultiSelectComboBox::THDBQueryMultiSelectComboBox()
{
	m_qdb=NULL;
	m_nCol=0;
	m_bNeedSelect=FALSE;
	m_bCanEdit=TRUE;
}

THDBQueryMultiSelectComboBox::~THDBQueryMultiSelectComboBox()
{
}

BEGIN_MESSAGE_MAP(THDBQueryMultiSelectComboBox, CComboBox)
	ON_WM_DESTROY()
	ON_CONTROL_REFLECT(CBN_CLOSEUP, OnCbnCloseup)
	ON_CONTROL_REFLECT(CBN_DROPDOWN, OnCbnDropdown)
	ON_MESSAGE(WM_CHANGETEXTMSG, OnChangeTextMessage)
END_MESSAGE_MAP()

void THDBQueryMultiSelectComboBox::Init(THDBQuery *qdb,int nCol,BOOL bNeedSelect,BOOL bCanEdit,THString sSpliter)
{
	m_qdb=qdb;
	m_nCol=nCol;
	m_bNeedSelect=bNeedSelect;
	m_bCanEdit=bCanEdit;
	m_sSpliter=sSpliter;
	if (m_qdb) m_qdb->AddNotifyCb(this);
	EnableWindow(FALSE);
}

void THDBQueryMultiSelectComboBox::UnInit()
{
	if (m_qdb) m_qdb->RemoveNotifyCb(this);
	m_qdb=NULL;
}

void THDBQueryMultiSelectComboBox::OnQueryDataFinish(THDBQuery *cls,int nRow,int nCol)
{
	ReLoadEnum();
	EnableWindow(FALSE);
}

void THDBQueryMultiSelectComboBox::ReLoadEnum()
{
	if (m_qdb)
	{
		ResetContent();
		m_sEnumArray.RemoveAll();
		THString data;
		for(int i=0;i<m_qdb->GetRecordCount();i++)
		{
			data=m_qdb->GetContent(i,m_nCol);
			THStringToken t(data,m_sSpliter);
			while(t.IsMoreTokens())
			{
				data=t.GetNextToken();
				BOOL bFound=FALSE;
				for(int j=0;j<m_sEnumArray.GetSize();j++)
				{
					if (m_sEnumArray[j]==data)
					{
						bFound=TRUE;
						break;
					}
				}
				if (!bFound)
					m_sEnumArray.Add(data);
			}
		}
		for(int i=0;i<m_sEnumArray.GetSize();i++)
			AddString(m_sEnumArray[i]);
		int idx=m_qdb->GetSelection();
		if (idx==-1)
			m_select.Empty();
		else
			m_select=m_qdb->GetContent(idx,m_nCol);
		SetSelectText(_T(""),m_select);
	}
}

void THDBQueryMultiSelectComboBox::OnSelectionChange(THDBQuery *cls,int nOld,int nNew)
{
	if (nNew!=-1)
	{
		m_select=m_qdb->GetContent(nNew,m_nCol);
		SetSelectText(_T(""),m_select);
		return;
	}
	SetSelectText(_T(""),_T(""));
}

void THDBQueryMultiSelectComboBox::OnModifyChange(THDBQuery *cls,BOOL bModify)
{
	EnableWindow(m_bCanEdit && bModify);
}

BOOL THDBQueryMultiSelectComboBox::OnSaveModify(THDBQuery *cls,int idx)
{
	if (!m_bCanEdit) return TRUE;
	THString str;
	GetWindowText(str);
	if (str.IsEmpty())
	{
		if (m_bNeedSelect)
		{
			THString out;
			if (m_qdb)
				out.Format(_T("请选择“%s”！"),m_qdb->GetHeader(m_nCol));
			else
				out=_T("请选择项！");
			MessageBox(out,_T("内容为空"),MB_OK|MB_ICONSTOP);
			SetFocus();
			return FALSE;
		}
		//return TRUE;
	}
	if (m_qdb) m_qdb->SetContentChange(m_nCol,str);
	return TRUE;
}

void THDBQueryMultiSelectComboBox::SetSelectText(THString oldtext,THString addtext)
{
	THStringToken t(oldtext,m_sSpliter);
	THStringArray ar;
	while(t.IsMoreTokens())
	{
		THString str=t.GetNextToken();
		ar.Add(str);
	}
	THStringToken t1(addtext,m_sSpliter);
	while(t1.IsMoreTokens())
	{
		THString str=t1.GetNextToken();
		BOOL bFound=FALSE;
		for(int i=0;i<ar.GetSize();i++)
		{
			if (ar[i]==str)
			{
				bFound=TRUE;
				break;
			}
		}
		if (!bFound)
		{
			ar.Add(str);
		}
	}
	m_select.Empty();
	for(int i=0;i<ar.GetSize();i++)
	{
		if (i==0)
			m_select=ar[0];
		else
			m_select+=m_sSpliter+ar[i];
	}
	if (::SendMessage(m_hWnd,CB_SELECTSTRING,(WPARAM)-1,(LPARAM)(LPCTSTR)m_select)==CB_ERR)
		SetWindowText(m_select);
}

void THDBQueryMultiSelectComboBox::OnDestroy()
{
	UnInit();
	__super::OnDestroy();
}

void THDBQueryMultiSelectComboBox::OnCbnCloseup()
{
	// TODO: Add your control notification handler code here
	THString tmp;
	if (GetCurSel()!=-1)
	{
		GetLBText(GetCurSel(),tmp);
		SetSelectText(m_select,tmp);
		PostMessage(WM_CHANGETEXTMSG);
	}
}

void THDBQueryMultiSelectComboBox::OnCbnDropdown()
{
	// TODO: Add your control notification handler code here
	GetWindowText(m_select);
}

LRESULT THDBQueryMultiSelectComboBox::OnChangeTextMessage(WPARAM wParam,LPARAM lParam)
{
	SetSelectText(m_select,_T(""));
	return 0;
}

IMPLEMENT_DYNAMIC(THDBQueryEditComboBox, CComboBox)
THDBQueryEditComboBox::THDBQueryEditComboBox()
{
	m_qdb=NULL;
	m_nCol=0;
	m_bNeedSelect=FALSE;
	m_bReDrawTitle=FALSE;
	m_bInit=FALSE;
	m_bCanEdit=TRUE;
	m_bUseDBEnum=TRUE;
}

THDBQueryEditComboBox::~THDBQueryEditComboBox()
{
}


BEGIN_MESSAGE_MAP(THDBQueryEditComboBox, CComboBox)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

void THDBQueryEditComboBox::Init(THDBQuery *qdb,int nCol,BOOL bNeedSelect,BOOL bReDrawTitle,BOOL bCanEdit,BOOL bUseDBEnum)
{
	m_qdb=qdb;
	m_nCol=nCol;
	m_bNeedSelect=bNeedSelect;
	m_bReDrawTitle=bReDrawTitle;
	m_bCanEdit=bCanEdit;
	m_bUseDBEnum=bUseDBEnum;
	if (m_qdb) m_qdb->AddNotifyCb(this);
	EnableWindow(FALSE);
}

void THDBQueryEditComboBox::UnInit()
{
	if (m_qdb) m_qdb->RemoveNotifyCb(this);
	m_qdb=NULL;
}

void THDBQueryEditComboBox::OnQueryDataFinish(THDBQuery *cls,int nRow,int nCol)
{
	if (m_bReDrawTitle)
		ResetContent();
	if (m_qdb)
	{
		if (!m_bInit || m_bReDrawTitle)
		{
			m_bInit=TRUE;
			ReLoadEnum();
		}
	}
	EnableWindow(FALSE);
}

void THDBQueryEditComboBox::ReLoadEnum()
{
	if (m_qdb)
	{
		ResetContent();
		m_sEnumArray.RemoveAll();
		if (m_bUseDBEnum)
			m_qdb->GetColEnumState(m_nCol,&m_sEnumArray);
		else
		{
			for(int i=0;i<m_qdb->GetRecordCount();i++)
			{
				THString str=m_qdb->GetContent(i,m_nCol);
				BOOL bFound=FALSE;
				for(int j=0;j<m_sEnumArray.GetSize();j++)
				{
					if (m_sEnumArray[j]==str)
					{
						bFound=TRUE;
						break;
					}
				}
				if (!bFound) m_sEnumArray.Add(str);
			}
		}
		for(int i=0;i<m_sEnumArray.GetSize();i++)
			AddString(m_sEnumArray[i]);
		OnSelectionChange(NULL,-1,m_qdb->GetSelection());
	}
}

void THDBQueryEditComboBox::OnSelectionChange(THDBQuery *cls,int nOld,int nNew)
{
	if (nNew!=-1)
	{
		THString str1=m_qdb->GetContent(nNew,m_nCol);
		if (!str1.IsEmpty())
		{
			if (::SendMessage(m_hWnd,CB_SELECTSTRING,(WPARAM)-1,(LPARAM)(LPCTSTR)str1)==CB_ERR)
				SetWindowText(str1);
			return;
/*			THString str;
			for(int i=0;i<GetCount();i++)
			{
				GetLBText(i,str);
				if (str==str1)
				{
					SetCurSel(i);
					return;
				}
			}*/
		}
	}
	SetCurSel(-1);
}

void THDBQueryEditComboBox::OnModifyChange(THDBQuery *cls,BOOL bModify)
{
	EnableWindow(m_bCanEdit && bModify);
}

BOOL THDBQueryEditComboBox::OnSaveModify(THDBQuery *cls,int idx)
{
	if (!m_bCanEdit) return TRUE;
	THString str;
	BOOL bOk=FALSE;
	GetWindowText(str);
/*	if (GetCurSel()!=-1)
	{
		GetLBText(GetCurSel(),str);
		if (!str.IsEmpty())
		{
			for(int i=0;i<m_sEnumArray.GetSize();i++)
			{
				if (str==m_sEnumArray[i])
				{
					bOk=TRUE;
					break;
				}
			}
		}
	}
	if (!bOk)*/
	if (str.IsEmpty())
	{
		if (m_bNeedSelect)
		{
			THString out;
			if (m_qdb)
				out.Format(_T("请选择“%s”！"),m_qdb->GetHeader(m_nCol));
			else
				out=_T("请选择项！");
			MessageBox(out,_T("内容为空"),MB_OK|MB_ICONSTOP);
			SetFocus();
			return FALSE;
		}
		//return TRUE;
	}
	if (m_qdb) m_qdb->SetContentChange(m_nCol,str);
	return TRUE;
}

void THDBQueryEditComboBox::OnDestroy()
{
	UnInit();
	__super::OnDestroy();
}

IMPLEMENT_DYNAMIC(THDBQueryEdit, CEdit)
THDBQueryEdit::THDBQueryEdit()
{
	m_qdb=NULL;
	m_col=0;
	m_bCheckEmpty=TRUE;
	m_bSelectChange=FALSE;
	m_bCanEdit=TRUE;
}

THDBQueryEdit::~THDBQueryEdit()
{
}

BEGIN_MESSAGE_MAP(THDBQueryEdit, CEdit)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

void THDBQueryEdit::Init(THDBQuery *qdb,int nCol,BOOL bCheckEmpty,BOOL bSelectChange,BOOL bCanEdit,THString sDefValue)
{
	m_qdb=qdb;
	m_col=nCol;
	m_bSelectChange=bSelectChange;
	m_bCheckEmpty=bCheckEmpty;
	m_bCanEdit=bCanEdit;
	m_sDefValue=sDefValue;
	if (m_qdb) m_qdb->AddNotifyCb(this);
	SetWindowText(m_sDefValue);
	SetReadOnly();
}

void THDBQueryEdit::UnInit()
{
	if (m_qdb) m_qdb->RemoveNotifyCb(this);
	m_qdb=NULL;
}

void THDBQueryEdit::OnSelectionChange(THDBQuery *cls,int nOld,int nNew)
{
	if (m_bSelectChange)
	{
		if (nNew!=-1 && m_qdb)
			SetWindowText(m_qdb->GetContent(nNew,m_col));
		else
			SetWindowText(m_sDefValue);
	}
}

void THDBQueryEdit::OnQueryDataFinish(THDBQuery *cls,int nRow,int nCol)
{
	SetWindowText(m_sDefValue);
	SetReadOnly();
}

void THDBQueryEdit::SetCanEdit(BOOL bCanEdit)
{
	m_bCanEdit=bCanEdit;
	if (m_qdb)
		OnModifyChange(m_qdb,m_qdb->GetModifyFlag());
	else
		OnModifyChange(NULL,FALSE);
}

void THDBQueryEdit::OnModifyChange(THDBQuery *cls,BOOL bModify)
{
	SetReadOnly(!(m_bCanEdit && bModify));
	if (bModify)
	{
		if (m_qdb && m_qdb->GetSelection()!=-1)
			SetWindowText(m_qdb->GetContent(m_qdb->GetSelection(),m_col));
		else
			SetWindowText(m_sDefValue);
	}
}

BOOL THDBQueryEdit::OnSaveModify(THDBQuery *cls,int idx)
{
	if (!m_bCanEdit) return TRUE;
	THString str;
	GetWindowText(str);
	if (str.IsEmpty()) str=m_sDefValue;
	if (str.IsEmpty() && m_bCheckEmpty)
	{
		THString out;
		if (m_qdb)
			out.Format(_T("请输入“%s”！"),m_qdb->GetHeader(m_col));
		else
			out=_T("请输入修改的内容！");
		MessageBox(out,_T("内容为空"),MB_OK|MB_ICONSTOP);
		SetFocus();
		return FALSE;
	}
	if (m_qdb) m_qdb->SetContentChange(m_col,str);
	return TRUE;
}

void THDBQueryEdit::OnDestroy()
{
	UnInit();
	__super::OnDestroy();
}

IMPLEMENT_DYNAMIC(THDBQueryStatic, CStatic)
THDBQueryStatic::THDBQueryStatic()
{
	m_qdb=NULL;
	m_nCol=0;
}

THDBQueryStatic::~THDBQueryStatic()
{
}

BEGIN_MESSAGE_MAP(THDBQueryStatic, CStatic)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

void THDBQueryStatic::Init(THDBQuery *qdb,int nCol,THString sPreString)
{
	m_qdb=qdb;
	m_nCol=nCol;
	m_sPreString=sPreString;
	if (m_qdb) m_qdb->AddNotifyCb(this);
	SetWindowText(m_sPreString);
}

void THDBQueryStatic::UnInit()
{
	if (m_qdb) m_qdb->RemoveNotifyCb(this);
	m_qdb=NULL;
}

void THDBQueryStatic::OnSelectionChange(THDBQuery *cls,int nOld,int nNew)
{
	if (nNew==-1 || m_qdb==NULL)
		SetWindowText(m_sPreString);
	else
		SetWindowText(m_sPreString+m_qdb->GetContent(nNew,m_nCol));
}

void THDBQueryStatic::OnDestroy()
{
	UnInit();
	__super::OnDestroy();
}

void THDBQueryStatic::OnModifyChange(THDBQuery *cls,BOOL bModify)
{
	if (m_qdb==NULL || m_qdb->GetSelection()==-1)
		SetWindowText(m_sPreString);
	else
		SetWindowText(m_sPreString+m_qdb->GetContent(m_qdb->GetSelection(),m_nCol));
}

IMPLEMENT_DYNAMIC(THDBQueryCheckBox, CButton)
THDBQueryCheckBox::THDBQueryCheckBox()
{
	m_qdb=NULL;
	m_col=0;
	m_bSelectChange=FALSE;
	m_bCanEdit=TRUE;
	m_bDefValue=FALSE;
}

THDBQueryCheckBox::~THDBQueryCheckBox()
{
}

BEGIN_MESSAGE_MAP(THDBQueryCheckBox, CButton)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

void THDBQueryCheckBox::Init(THDBQuery *qdb,int nCol,BOOL bSelectChange,BOOL bCanEdit,BOOL bDefValue)
{
	m_qdb=qdb;
	m_col=nCol;
	m_bSelectChange=bSelectChange;
	m_bCanEdit=bCanEdit;
	m_bDefValue=bDefValue;
	if (m_qdb) m_qdb->AddNotifyCb(this);
	SetCheck(m_bDefValue?BST_CHECKED:BST_UNCHECKED);
	EnableWindow(FALSE);
}

void THDBQueryCheckBox::UnInit()
{
	if (m_qdb) m_qdb->RemoveNotifyCb(this);
	m_qdb=NULL;
}

void THDBQueryCheckBox::OnSelectionChange(THDBQuery *cls,int nOld,int nNew)
{
	if (m_bSelectChange)
	{
		BOOL bCheck=m_bDefValue;
		if (nNew!=-1 && m_qdb)
		{
			THString str=m_qdb->GetContent(nNew,m_col).Trim().MakeLower();
			if (str==_T("是") || str==_T("y") || str==_T("yes"))
				bCheck=TRUE;
			else if (str==_T("否") || str==_T("n") || str==_T("no"))
				bCheck=FALSE;
			else
				bCheck=THs2i(str);
		}
		SetCheck(bCheck?BST_CHECKED:BST_UNCHECKED);
	}
}

void THDBQueryCheckBox::OnQueryDataFinish(THDBQuery *cls,int nRow,int nCol)
{
	EnableWindow(FALSE);
}

void THDBQueryCheckBox::OnModifyChange(THDBQuery *cls,BOOL bModify)
{
	EnableWindow(m_bCanEdit && bModify);
	BOOL bCheck=m_bDefValue;
	if (bModify && m_qdb && m_qdb->GetSelection()!=-1)
	{
		THString str=m_qdb->GetContent(m_qdb->GetSelection(),m_col).Trim().MakeLower();
		if (str==_T("是") || str==_T("y") || str==_T("yes"))
			bCheck=TRUE;
		else if (str==_T("否") || str==_T("n") || str==_T("no"))
			bCheck=FALSE;
		else
			bCheck=THs2i(str);
	}
	SetCheck(bCheck?BST_CHECKED:BST_UNCHECKED);
}

BOOL THDBQueryCheckBox::OnSaveModify(THDBQuery *cls,int idx)
{
	if (!m_bCanEdit) return TRUE;
	THString str=_T("0");
	int chk=GetCheck();
	if (chk==BST_CHECKED) str=_T("1");
	if (m_qdb) m_qdb->SetContentChange(m_col,str);
	return TRUE;
}

void THDBQueryCheckBox::OnDestroy()
{
	UnInit();
	__super::OnDestroy();
}

IMPLEMENT_DYNAMIC(THDBQueryListBox, CListBox)
THDBQueryListBox::THDBQueryListBox()
{
	m_qdb=NULL;
	m_nCol=0;
}

THDBQueryListBox::~THDBQueryListBox()
{
}


BEGIN_MESSAGE_MAP(THDBQueryListBox, CListBox)
	ON_WM_DESTROY()
	ON_CONTROL_REFLECT(LBN_SELCHANGE, OnLbnSelchange)
END_MESSAGE_MAP()

void THDBQueryListBox::Init(THDBQuery *qdb,int nCol)
{
	m_qdb=qdb;
	m_nCol=nCol;
	if (m_qdb) m_qdb->AddNotifyCb(this);
}

void THDBQueryListBox::UnInit()
{
	if (m_qdb) m_qdb->RemoveNotifyCb(this);
	m_qdb=NULL;
}

void THDBQueryListBox::OnQueryDataFinish(THDBQuery *cls,int nRow,int nCol)
{
	ResetContent();
	if (m_qdb && nCol>m_nCol)
	{
		for(int i=0;i<nRow;i++)
		{
			int idx=AddString(m_qdb->GetContent(i,m_nCol));
			SetItemData(idx,i);//THs2i(m_qdb->GetContent(i,0)));
		}
	}
}

void THDBQueryListBox::OnDestroy()
{
	UnInit();
	__super::OnDestroy();
}


void THDBQueryListBox::OnLbnSelchange()
{
	int cur=GetCurSel();
	if (cur!=-1 && m_qdb)
		m_qdb->ChangeSelection((int)GetItemData(cur));
}

IMPLEMENT_DYNAMIC(THDBQueryListCtrl, CListCtrl)
THDBQueryListCtrl::THDBQueryListCtrl()
{
	m_qdb=NULL;
	m_bUseImage=FALSE;
	m_bReDrawTitle=FALSE;
	m_bInit=FALSE;
	m_nCol=-1;
}

THDBQueryListCtrl::~THDBQueryListCtrl()
{
}


BEGIN_MESSAGE_MAP(THDBQueryListCtrl, THSortListCtrl)
	ON_WM_DESTROY()
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, OnLvnItemchanged)
END_MESSAGE_MAP()

void THDBQueryListCtrl::Init(THDBQuery *qdb,THString sWidth,int nCol,BOOL bReDrawTitle,THBitmap *bm,COLORREF crMask)
{
	m_qdb=qdb;
	m_bReDrawTitle=bReDrawTitle;
	m_nCol=nCol;
	m_Width.RemoveAll();
	THStringToken t(sWidth,_T("|"));
	while(t.IsMoreTokens())
		m_Width.AddTail(THs2i(t.GetNextToken()));
	if (m_qdb) m_qdb->AddNotifyCb(this);
	SetCompareCallBack(this);
	if (bm)
	{
		int x,y;
		bm->GetBmpSize(x,y);
		m_list.Create(y,y,ILC_COLOR32|ILC_MASK,10,10);
		CBitmap cbm;
		cbm.Attach(bm->GetBitmap());
		m_list.Add(&cbm,crMask);
		cbm.Detach();
		SetImageList(&m_list,LVSIL_SMALL);
		m_bUseImage=TRUE;
	}
	UseSortHeader();
}

void THDBQueryListCtrl::UnInit()
{
	if (m_qdb) m_qdb->RemoveNotifyCb(this);
	m_qdb=NULL;
}

void THDBQueryListCtrl::OnQueryDataFinish(THDBQuery *cls,int nRow,int nCol)
{
	DeleteAllItems();
	if (m_bReDrawTitle)
		while(DeleteColumn(0));
	if (m_qdb)
	{
		int StartIdx=1;
		if (m_bUseImage) StartIdx=2;
		//修正限定的列数
		if (m_nCol!=-1)
			nCol=m_nCol+StartIdx;
		int nWidth;
		if (m_bReDrawTitle || m_bInit==FALSE)
		{
			m_bInit=TRUE;
			for(int i=StartIdx;i<nCol;i++)
			{
				nWidth=m_Width[i-StartIdx];
				if (nWidth<=0) nWidth=100;
				InsertColumn(i,m_qdb->GetHeader(i),LVCFMT_LEFT,nWidth);
			}
		}
		if ((nCol>=2 && m_bUseImage) ||
			(nCol>=1 && m_bUseImage==FALSE))
		{
			int item;
			for(int i=0;i<nRow;i++)
			{
				if (m_bUseImage)
					item=InsertItem(GetItemCount(),m_qdb->GetContent(i,StartIdx),THs2i(m_qdb->GetContent(i,1)));
				else
					item=InsertItem(GetItemCount(),m_qdb->GetContent(i,StartIdx));
				SetItemData(item,i);//THs2i(m_qdb->GetContent(i,0)));
				SetLparam(item,(LPARAM)i);
				for(int j=1;j<nCol;j++)
					SetItemText(item,j,m_qdb->GetContent(i,StartIdx+j));
			}
		}
	}
}

void THDBQueryListCtrl::OnSelectionChange(THDBQuery *cls,int nOld,int nNew)
{
}

void THDBQueryListCtrl::OnModifyChange(THDBQuery *cls,BOOL bModify)
{
}

BOOL THDBQueryListCtrl::OnSaveModify(THDBQuery *cls,int idx)
{
	return TRUE;
}

BOOL THDBQueryListCtrl::OnDeleteRecord(THDBQuery *cls,int idx)
{
	return TRUE;
}

int THDBQueryListCtrl::SortListCompare(THSortListCtrl *pCls,LPARAM lParam1,LPARAM lParam2,int nSortIdx,BOOL bAcc)
{
	if (m_qdb)
	{
		int StartIdx=1+nSortIdx;
		if (m_bUseImage) StartIdx++;
		int idx1=(int)lParam1;
		int idx2=(int)lParam2;
		if (!bAcc)
		{
			int tmp=idx2;
			idx2=idx1;
			idx1=tmp;
		}
		THString keytype=m_qdb->GetColValueType(StartIdx);
		if (keytype==_T("f"))
		{
			return (int)(THs2f(m_qdb->GetContent(idx1,StartIdx))-THs2f(m_qdb->GetContent(idx2,StartIdx)));
		}
		else if (keytype==_T("d"))
		{
			THString str1=m_qdb->GetContent(idx1,StartIdx);
			THString str2=m_qdb->GetContent(idx2,StartIdx);
			if (THStringConv::IsStringNumber(str1) && THStringConv::IsStringNumber(str2))
				return (int)(THs2i(str1)-THs2i(str2));
			//break and take the "s" type compare
		}
		//else //_T("s") _T("t")
		return _tcscmp(m_qdb->GetContent(idx1,StartIdx),m_qdb->GetContent(idx2,StartIdx));
	}
	return 0;
}

BOOL THDBQueryListCtrl::ExportToCsv(THString sPath,BOOL bExportHeader)
{
	THFile file;
	BOOL bRet=FALSE;
	if (file.Open(sPath,THFile::modeCreate|THFile::modeWrite))
	{
		CHeaderCtrl *ctrl=GetHeaderCtrl();
		THString str,tmp;
		if (ctrl)
		{
			int cnt=ctrl->GetItemCount();
			int ccnt=GetItemCount();
			TCHAR cch[512];
			if (bExportHeader)
			{
				HDITEM item;
				for(int i=0;i<cnt;i++)
				{
					cch[0]='\0';
					item.cchTextMax = 512;
					item.pszText = cch;
					item.mask = HDI_TEXT;
					ctrl->GetItem(i,&item);
					if (i==0)
						tmp=item.pszText;
					else
						tmp.AppendFormat(_T(",%s"),item.pszText);
				}
				tmp+=_T("\r\n");
				str+=tmp;
			}
			for(int j=0;j<ccnt;j++)
			{
				for(int i=0;i<cnt;i++)
				{
					
					if (i==0)
						tmp=GetItemText(j,i);
					else
						tmp+=_T(",")+GetItemText(j,i);
				}
				tmp+=_T("\r\n");
				str+=tmp;
			}
			if (file.WriteStringA(str))
				bRet=TRUE;
		}
		file.Close();
	}
	return bRet;
}

void THDBQueryListCtrl::OnDestroy()
{
	UnInit();
	__super::OnDestroy();
}

void THDBQueryListCtrl::OnLvnItemchanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	if (m_qdb)
	{
		POSITION pos = GetFirstSelectedItemPosition();
		if (pos == NULL)
			m_qdb->ChangeSelection(-1);
		else
		{
			int nItem = GetNextSelectedItem(pos);
			m_qdb->ChangeSelection((int)GetItemData(nItem));
		}
	}
	*pResult = 0;
}

IMPLEMENT_DYNAMIC(THDBQueryEditPanel, CStatic)
THDBQueryEditPanel::THDBQueryEditPanel()
{
	m_pd=NULL;
	m_InitType=0;
	m_deltext=_T("确认删除这条记录吗？");
}

THDBQueryEditPanel::~THDBQueryEditPanel()
{
}

#define IDC_DBQUERYBUTTON1		15220
#define IDC_DBQUERYBUTTON2		15221
#define IDC_DBQUERYBUTTON3		15222
#define IDC_DBQUERYBUTTON4		15223
#define IDC_DBQUERYBUTTON5		15224

BEGIN_MESSAGE_MAP(THDBQueryEditPanel, CStatic)
	ON_BN_CLICKED(IDC_DBQUERYBUTTON1, OnBnClickedButton1)
	ON_BN_CLICKED(IDC_DBQUERYBUTTON2, OnBnClickedButton2)
	ON_BN_CLICKED(IDC_DBQUERYBUTTON3, OnBnClickedButton3)
	ON_BN_CLICKED(IDC_DBQUERYBUTTON4, OnBnClickedButton4)
	ON_BN_CLICKED(IDC_DBQUERYBUTTON5, OnBnClickedButton5)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

void THDBQueryEditPanel::Init(THDBQuery *pd,int InitType)
{
	RECT rect;
	GetClientRect(&rect);
	int nWidth=rect.right/5;
	TCHAR btnText[][10]={_T("新建(&N)"),_T("编辑(&E)"),_T("删除(&D)"),_T("保存(&S)"),_T("取消(&C)")};
	for(int i=0;i<5;i++)
	{
		rect.left=i*nWidth;
		rect.right=(i+1)*nWidth;
		m_btn[i].Create(btnText[i],WS_CHILD|WS_VISIBLE,rect,this,IDC_DBQUERYBUTTON1+i);
	}
	m_InitType=InitType;
	if (pd)
	{
		m_pd=pd;
		OnModifyChange(m_pd,m_pd->GetModifyFlag());
		m_pd->AddNotifyCb(this);
	}
}

void THDBQueryEditPanel::SetButtonState(int StateType)
{
	m_InitType=StateType;
	OnUpdateButtons();
}

void THDBQueryEditPanel::UnInit()
{
	if (m_pd) m_pd->RemoveNotifyCb(this);
	m_pd=NULL;
}

void THDBQueryEditPanel::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	//new
	if (m_pd) m_pd->SetModifyFlag(TRUE,TRUE);
}

void THDBQueryEditPanel::OnBnClickedButton2()
{
	// TODO: Add your control notification handler code here
	//edit
	if (m_pd) m_pd->SetModifyFlag(TRUE);
}

void THDBQueryEditPanel::SetDeleteNotifyText(THString text)
{
	m_deltext=text;
}

void THDBQueryEditPanel::OnBnClickedButton3()
{
	// TODO: Add your control notification handler code here
	//del
	if (m_pd)
	{
		if (GetParent()->MessageBox(m_deltext,_T("确认删除"),MB_YESNO|MB_ICONQUESTION)==IDYES)
			m_pd->DelRecord();
	}
}

void THDBQueryEditPanel::OnBnClickedButton4()
{
	// TODO: Add your control notification handler code here
	//save
	if (m_pd) m_pd->SaveModify();
}

void THDBQueryEditPanel::OnBnClickedButton5()
{
	// TODO: Add your control notification handler code here
	//cancel
	if (m_pd) m_pd->SetModifyFlag(FALSE);
}

void THDBQueryEditPanel::OnQueryDataFinish(THDBQuery *cls,int nRow,int nCol)
{
	OnUpdateButtons();
}

void THDBQueryEditPanel::OnPreSelectionChange(THDBQuery *cls,int nOld,int nNew)
{
	if (m_pd)
	{
		if (m_pd->GetModifyFlag())
		{
			if (GetParent()->MessageBox(_T("当前数据正在修改中，是否保存？"),_T("数据已修改"),MB_YESNO|MB_ICONQUESTION)==IDYES)
				m_pd->SaveModify();
			else
				m_pd->SetModifyFlag(FALSE);
		}
	}
}

void THDBQueryEditPanel::OnSelectionChange(THDBQuery *cls,int nOld,int nNew)
{
	OnUpdateButtons();
}

void THDBQueryEditPanel::OnModifyChange(THDBQuery *cls,BOOL bModify)
{
	OnUpdateButtons();
}

void THDBQueryEditPanel::OnUpdateButtons()
{
	if (m_pd)
	{
		if (m_pd->GetModifyFlag())
		{
			GetDlgItem(IDC_DBQUERYBUTTON1)->EnableWindow(FALSE);
			GetDlgItem(IDC_DBQUERYBUTTON2)->EnableWindow(FALSE);
			GetDlgItem(IDC_DBQUERYBUTTON3)->EnableWindow(FALSE);
			if (m_InitType&(DBQUERYEDITPANEL_ADD|DBQUERYEDITPANEL_EDIT))
			{
				GetDlgItem(IDC_DBQUERYBUTTON4)->EnableWindow(TRUE);
				GetDlgItem(IDC_DBQUERYBUTTON5)->EnableWindow(TRUE);
			}
			else
			{
				GetDlgItem(IDC_DBQUERYBUTTON4)->EnableWindow(FALSE);
				GetDlgItem(IDC_DBQUERYBUTTON5)->EnableWindow(FALSE);
			}
		}
		else
		{
			if (m_InitType&DBQUERYEDITPANEL_ADD)
				GetDlgItem(IDC_DBQUERYBUTTON1)->EnableWindow(TRUE);
			else
				GetDlgItem(IDC_DBQUERYBUTTON1)->EnableWindow(FALSE);
			if (m_pd->GetSelection()!=-1)
			{
				if (m_InitType&DBQUERYEDITPANEL_EDIT)
					GetDlgItem(IDC_DBQUERYBUTTON2)->EnableWindow(TRUE);
				else
					GetDlgItem(IDC_DBQUERYBUTTON2)->EnableWindow(FALSE);
				if (m_InitType&DBQUERYEDITPANEL_DELETE)
					GetDlgItem(IDC_DBQUERYBUTTON3)->EnableWindow(TRUE);
				else
					GetDlgItem(IDC_DBQUERYBUTTON3)->EnableWindow(FALSE);
			}
			else
			{
				GetDlgItem(IDC_DBQUERYBUTTON2)->EnableWindow(FALSE);
				GetDlgItem(IDC_DBQUERYBUTTON3)->EnableWindow(FALSE);
			}
			GetDlgItem(IDC_DBQUERYBUTTON4)->EnableWindow(FALSE);
			GetDlgItem(IDC_DBQUERYBUTTON5)->EnableWindow(FALSE);
		}
		if (m_pd->GetReadOnly())
		{
			GetDlgItem(IDC_DBQUERYBUTTON1)->EnableWindow(FALSE);
			GetDlgItem(IDC_DBQUERYBUTTON2)->EnableWindow(FALSE);
			GetDlgItem(IDC_DBQUERYBUTTON3)->EnableWindow(FALSE);
			GetDlgItem(IDC_DBQUERYBUTTON4)->EnableWindow(FALSE);
			GetDlgItem(IDC_DBQUERYBUTTON5)->EnableWindow(FALSE);
		}
	}
}

void THDBQueryEditPanel::OnDestroy()
{
	UnInit();
	__super::OnDestroy();
	// TODO: Add your message handler code here
}

IMPLEMENT_DYNAMIC(THDBQueryDateTimeCtrl, CDateTimeCtrl)
THDBQueryDateTimeCtrl::THDBQueryDateTimeCtrl()
{
	m_qdb=NULL;
	m_col=0;
	m_bSelectChange=FALSE;
	m_bCanEdit=TRUE;
}

THDBQueryDateTimeCtrl::~THDBQueryDateTimeCtrl()
{
}

BEGIN_MESSAGE_MAP(THDBQueryDateTimeCtrl, CDateTimeCtrl)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

void THDBQueryDateTimeCtrl::Init(THDBQuery *qdb,int nCol,THString sFormat,BOOL bSelectChange,BOOL bCanEdit)
{
	m_qdb=qdb;
	m_col=nCol;
	m_sFormat=sFormat;
	m_bCanEdit=bCanEdit;
	m_bSelectChange=bSelectChange;
	if (m_qdb) m_qdb->AddNotifyCb(this);
	THTime ti;
	SYSTEMTIME st=ti.GetTimeAsSystemTime();
	SetTime(&st);
	EnableWindow(FALSE);
}

void THDBQueryDateTimeCtrl::UnInit()
{
	if (m_qdb) m_qdb->RemoveNotifyCb(this);
	m_qdb=NULL;
}

void THDBQueryDateTimeCtrl::OnSelectionChange(THDBQuery *cls,int nOld,int nNew)
{
	if (m_bSelectChange)
	{
		THTime ti;
		if (nNew!=-1 && m_qdb)
			ti.SetTime(m_qdb->GetContent(nNew,m_col),m_sFormat);
		SYSTEMTIME st=ti.GetTimeAsSystemTime();
		SetTime(&st);
	}
}

void THDBQueryDateTimeCtrl::OnQueryDataFinish(THDBQuery *cls,int nRow,int nCol)
{
	EnableWindow(FALSE);
}

void THDBQueryDateTimeCtrl::OnModifyChange(THDBQuery *cls,BOOL bModify)
{
	EnableWindow(m_bCanEdit && bModify);
	if (bModify && m_qdb && m_qdb->GetSelection()!=-1)
	{
		THTime ti;
		ti.SetTime(m_qdb->GetContent(m_qdb->GetSelection(),m_col),m_sFormat);
		SYSTEMTIME st=ti.GetTimeAsSystemTime();
		SetTime(&st);
	}
}

BOOL THDBQueryDateTimeCtrl::OnSaveModify(THDBQuery *cls,int idx)
{
	if (!m_bCanEdit) return TRUE;
	SYSTEMTIME st;
	GetTime(&st);
	THTime ti;
	ti.SetTime(st);
	if (m_qdb) m_qdb->SetContentChange(m_col,ti.Format(m_sFormat));
	return TRUE;
}

void THDBQueryDateTimeCtrl::OnDestroy()
{
	UnInit();
	__super::OnDestroy();
}
