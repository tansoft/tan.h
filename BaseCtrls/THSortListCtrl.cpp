
#include "Stdafx.h"
#include <THString.h>
#include "THSortListCtrl.h"
#include ".\thsortlistctrl.h"

THSortHeaderCtrl::THSortHeaderCtrl()
	: m_iSortColumn( -1 )
	, m_bSortAscending( TRUE )
{
	m_crBk = RGB(242,242,242);
}

THSortHeaderCtrl::~THSortHeaderCtrl()
{
}

BEGIN_MESSAGE_MAP(THSortHeaderCtrl, CHeaderCtrl)
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

BOOL THSortHeaderCtrl::OnEraseBkgnd(CDC* pDC)
{
	CBrush brush(m_crBk);
	CRect rect;
	GetClientRect(&rect);
	pDC->FillRect( rect, &brush );
	return TRUE;
}

void THSortHeaderCtrl::SetBkColor(COLORREF cr)
{
	m_crBk = cr;
}

void THSortHeaderCtrl::SetSortArrow( const int iSortColumn, const BOOL bSortAscending )
{
	m_iSortColumn = iSortColumn;
	m_bSortAscending = bSortAscending;
	/*HD_ITEM hditem;
	hditem.mask = HDI_FORMAT;
	VERIFY( GetItem( iSortColumn, &hditem ) );
	hditem.fmt |= HDF_OWNERDRAW;
	VERIFY( SetItem( iSortColumn, &hditem ) );
	Invalidate();*/
}


void THSortHeaderCtrl::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
{
	CDC dc;
	VERIFY( dc.Attach( lpDrawItemStruct->hDC ) );

	const int iSavedDC = dc.SaveDC();

	CRect rc( lpDrawItemStruct->rcItem );

	CRgn rgn;
	VERIFY( rgn.CreateRectRgnIndirect( &rc ) );
	(void)dc.SelectObject( &rgn );
	VERIFY( rgn.DeleteObject() );

	//CBrush brush( GetSysColor( COLOR_3DFACE ) );
	//dc.FillRect( rc, &brush );
	CBrush brush(m_crBk);
	dc.FillRect( rc, &brush );
	dc.SetTextColor(RGB(0,0,0));
	dc.SetBkMode(TRANSPARENT);

	TCHAR szText[ 256 ];
	HD_ITEM hditem;

	hditem.mask = HDI_TEXT | HDI_FORMAT;
	hditem.pszText = szText;
	hditem.cchTextMax = 255;

	VERIFY( GetItem( lpDrawItemStruct->itemID, &hditem ) );

	UINT uFormat = DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP | DT_VCENTER | DT_END_ELLIPSIS ;

	if( hditem.fmt & HDF_CENTER)
		uFormat |= DT_CENTER;
	else if( hditem.fmt & HDF_RIGHT)
		uFormat |= DT_RIGHT;
	else
		uFormat |= DT_LEFT;

	if( lpDrawItemStruct->itemState == ODS_SELECTED )
	{
		rc.left++;
		rc.top += 2;
		rc.right++;
	}

	CRect rcIcon( lpDrawItemStruct->rcItem );
	const int iOffset = ( rcIcon.bottom - rcIcon.top ) / 4;

	if( lpDrawItemStruct->itemID == (UINT)m_iSortColumn )
		rc.right -= 3 * iOffset;

	rc.left += iOffset;
	rc.right -= iOffset;

	if( rc.left < rc.right )
		(void)dc.DrawText( szText, -1, rc, uFormat );

	if( lpDrawItemStruct->itemID == (UINT)m_iSortColumn )
	{
		CPen penLight( PS_SOLID, 1, GetSysColor( COLOR_3DHILIGHT ) );
		CPen penShadow( PS_SOLID, 1, GetSysColor( COLOR_3DSHADOW ) );
		CPen* pOldPen = dc.SelectObject( &penLight );

		if( m_bSortAscending )
		{
			dc.MoveTo( rcIcon.right - 2 * iOffset, iOffset);
			dc.LineTo( rcIcon.right - iOffset, rcIcon.bottom - iOffset - 1 );
			dc.LineTo( rcIcon.right - 3 * iOffset - 2, rcIcon.bottom - iOffset - 1 );
			(void)dc.SelectObject( &penShadow );
			dc.MoveTo( rcIcon.right - 3 * iOffset - 1, rcIcon.bottom - iOffset - 1 );
			dc.LineTo( rcIcon.right - 2 * iOffset, iOffset - 1);		
		}
		else
		{
			dc.MoveTo( rcIcon.right - iOffset - 1, iOffset );
			dc.LineTo( rcIcon.right - 2 * iOffset - 1, rcIcon.bottom - iOffset );
			(void)dc.SelectObject( &penShadow );
			dc.MoveTo( rcIcon.right - 2 * iOffset - 2, rcIcon.bottom - iOffset );
			dc.LineTo( rcIcon.right - 3 * iOffset - 1, iOffset );
			dc.LineTo( rcIcon.right - iOffset - 1, iOffset );		
		}
		(void)dc.SelectObject( pOldPen );
	}
	VERIFY( dc.RestoreDC( iSavedDC ) );
	(void)dc.Detach();
}

THSortListCompareCallBack::THSortListCompareCallBack()
{
	//不使用第0项数据
	m_sSorting.Add(_T("Empty"));
}

THSortListCompareCallBack::~THSortListCompareCallBack()
{
}

void THSortListCompareCallBack::OnSetItemText(THSortListCtrl *pCls,int idx,int subtext,THString sTitle)
{
	THString ret;
	sTitle.Replace(_T("|"),_T("shu"));
	int index=(int)pCls->GetLparam(idx);
	if (index==0)
	{
		//insert
		index=(int)m_sSorting.Add(_T(""));
		pCls->SetLparam(idx,index);
	}
	else
	{
		if (index<m_sSorting.GetSize())
			ret=m_sSorting.GetAt(index);
	}
	THStringToken t(ret,_T("|"));
	THStringArray saret;
	for(int i=0;i<subtext;i++)
		saret.Add(t.GetNextToken());
	t.GetNextToken();
	saret.Add(sTitle);
	while(t.IsMoreTokens())
		saret.Add(t.GetNextToken());
	ret.Empty();
	for(i=0;i<saret.GetSize();i++)
	{
		if (i!=0)
			ret+=_T("|");
		ret+=saret[i];
	}
	m_sSorting.SetAtGrow(index,ret);
}

int THSortListCompareCallBack::SortListCompare(THSortListCtrl *pCls,LPARAM lParam1,LPARAM lParam2,int nSortIdx,BOOL bAcc)
{
	int idx1=(int)lParam1;
	int idx2=(int)lParam2;
	if (idx1<0 || idx1>=m_sSorting.GetSize()) return 0;
	if (idx2<0 || idx2>=m_sSorting.GetSize()) return 0;
	THStringToken t1(m_sSorting[idx1],_T("|"));
	THStringToken t2(m_sSorting[idx2],_T("|"));
	THString st1,st2;
	for(int i=0;i<nSortIdx+1;i++)
	{
		st1=t1.GetNextToken();
		st2=t2.GetNextToken();
	}
	if (bAcc) return st1.Compare(st2);
	return st2.Compare(st1);
}

THSortListCtrl::THSortListCtrl()
{
	m_iSortColumn = -1;
	m_bSortAscending = TRUE;
	m_bInit = FALSE;
	m_pCB = &m_defaultcb;
}

THSortListCtrl::~THSortListCtrl()
{
}

BEGIN_MESSAGE_MAP(THSortListCtrl, CListCtrl)
	ON_WM_CREATE()
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnLvnColumnclick)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

int THSortListCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CListCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;
	UseSortHeader();
	return 0;
}

void THSortListCtrl::SetHeaderColor(COLORREF cr)
{
	m_header.SetBkColor(cr);
}

void THSortListCtrl::UseSortHeader()
{
	if (!m_bInit)
	{
		m_bInit=TRUE;
		HWND hHeaderCtrl = GetHeaderCtrl()->GetSafeHwnd();
		if(hHeaderCtrl)
			m_header.SubclassWindow(hHeaderCtrl);
		SetExtendedStyle(GetExtendedStyle()|LVS_EX_FULLROWSELECT);
	}
}

void THSortListCtrl::OnLvnColumnclick(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMListView = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	const int iColumn = pNMListView->iSubItem;
	Sort( iColumn, iColumn == m_iSortColumn ? !m_bSortAscending : TRUE);
	*pResult = 0;
}
void THSortListCtrl::Sort( int iColumn, BOOL bAscending )
{
	m_iSortColumn = iColumn;
	m_bSortAscending = bAscending;
	m_header.SetSortArrow( m_iSortColumn, m_bSortAscending );
	
	VERIFY( SortItems( CompareFunction, LPARAM(this) ));
}
int CALLBACK THSortListCtrl::CompareFunction( LPARAM lParam1, LPARAM lParam2, LPARAM lParamData)
{
	THSortListCtrl* pListCtrl = (THSortListCtrl *)lParamData;
	if (pListCtrl && pListCtrl->m_pCB)
		return pListCtrl->m_pCB->SortListCompare(pListCtrl,lParam1,lParam2,pListCtrl->m_iSortColumn,pListCtrl->m_bSortAscending);
	return 0;
}
int THSortListCtrl::InsertColumn(int nCol, const LVCOLUMN* pColumn)
{
	int idx=CListCtrl::InsertColumn(nCol,pColumn);
	UseSortHeader();
	HD_ITEM hditem;
	hditem.mask = HDI_FORMAT;
	m_header.GetItem(idx,&hditem);
	hditem.fmt |= HDF_OWNERDRAW;
	m_header.SetItem(idx,&hditem);
	return idx;
}
int THSortListCtrl::InsertColumn(int nCol, LPCTSTR lpszColumnHeading,int nFormat, int nWidth, int nSubItem)
{
	int idx=CListCtrl::InsertColumn(nCol,lpszColumnHeading,nFormat,nWidth,nSubItem);
	UseSortHeader();
	HD_ITEM hditem;
	hditem.mask = HDI_FORMAT;
	m_header.GetItem(idx,&hditem);
	hditem.fmt |= HDF_OWNERDRAW;
	m_header.SetItem(idx,&hditem);
	return idx;
}
void THSortListCtrl::SetLparam(int idx,LPARAM lParam)
{
	CListCtrl::SetItem(idx,0,LVIF_PARAM,NULL,0,0,0,lParam);
}
LPARAM THSortListCtrl::GetLparam(int idx)
{
	LVITEM item;
	item.lParam=NULL;
	item.iItem=idx;
	item.iSubItem=0;
	item.mask=LVIF_PARAM;
	GetItem(&item);
	return item.lParam;
}
void THSortListCtrl::SetCompareCallBack(ITHSortListCompareCallBack *pCB)
{
	if (pCB)
		m_pCB=pCB;
	else
		m_pCB=&m_defaultcb;
}

void THSortListCtrl::OnDestroy()
{
	SetCompareCallBack(NULL);
	CListCtrl::OnDestroy();
	// TODO: Add your message handler code here
}

int THSortListCtrl::InsertItem(const LVITEM* pItem)
{
	int idx=CListCtrl::InsertItem(pItem);
	if (m_pCB && pItem && pItem->mask&LVIF_TEXT) m_pCB->OnSetItemText(this,idx,0,pItem->pszText);
	return idx;
}

int THSortListCtrl::InsertItem(int nItem, LPCTSTR lpszItem)
{
	int idx=CListCtrl::InsertItem(nItem,lpszItem);
	if (m_pCB) m_pCB->OnSetItemText(this,idx,0,lpszItem);
	return idx;
}

int THSortListCtrl::InsertItem(int nItem, LPCTSTR lpszItem, int nImage)
{
	int idx=CListCtrl::InsertItem(nItem,lpszItem,nImage);
	if (m_pCB) m_pCB->OnSetItemText(this,idx,0,lpszItem);
	return idx;
}

int THSortListCtrl::InsertItem(UINT nMask, int nItem, LPCTSTR lpszItem, UINT nState, UINT nStateMask, int nImage, LPARAM lParam)
{
	int idx=CListCtrl::InsertItem(nMask,nItem,lpszItem,nState,nStateMask,nImage,lParam);
	if (m_pCB && nMask&LVIF_TEXT) m_pCB->OnSetItemText(this,idx,0,lpszItem);
	return idx;
}

BOOL THSortListCtrl::SetItem(int nItem, int nSubItem, UINT nMask, LPCTSTR lpszItem, int nImage, UINT nState, UINT nStateMask, LPARAM lParam)
{
	BOOL ret=CListCtrl::SetItem(nItem,nSubItem,nMask,lpszItem,nImage,nState,nStateMask,lParam);
	if (m_pCB && nMask&LVIF_TEXT) m_pCB->OnSetItemText(this,nItem,nSubItem,lpszItem);
	return ret;
}

BOOL THSortListCtrl::SetItem(const LVITEM* pItem)
{
	BOOL ret=CListCtrl::SetItem(pItem);
	if (m_pCB && pItem && pItem->mask&LVIF_TEXT) m_pCB->OnSetItemText(this,pItem->iItem,pItem->iSubItem,pItem->pszText);
	return ret;
}

BOOL THSortListCtrl::SetItem(int nItem, int nSubItem, UINT nMask, LPCTSTR lpszItem,	int nImage, UINT nState, UINT nStateMask, LPARAM lParam, int nIndent)
{
	BOOL ret=CListCtrl::SetItem(nItem,nSubItem,nMask,lpszItem,nImage,nState,nStateMask,lParam,nIndent);
	if (m_pCB && nMask&LVIF_TEXT) m_pCB->OnSetItemText(this,nItem,nSubItem,lpszItem);
	return ret;
}

BOOL THSortListCtrl::SetItemText(int nItem, int nSubItem, LPCTSTR lpszText)
{
	BOOL ret=CListCtrl::SetItemText(nItem,nSubItem,lpszText);
	if (m_pCB) m_pCB->OnSetItemText(this,nItem,nSubItem,lpszText);
	return ret;
}
