// BmpButton.cpp : implementation file
//

#include "stdafx.h"
#include "THSkinButton.h"

IMPLEMENT_DYNAMIC(THSkinButton, CButton)
THSkinButton::THSkinButton()
{
	memset(&m_bmp,0,sizeof(BITMAP));
	m_bMouseIn=FALSE;
	m_bMouseDown=FALSE;
	m_hIcon=NULL;
	m_hCursor=NULL;
	m_hBmp=NULL;
	m_nWidth=0;
	m_nHeight=0;
	m_nLeft=0;
	m_crText=RGB(0,0,0);
	m_nTop=0;
	m_nTimer=0;
	m_nTypeRadio=0;
	m_mask=1.0;
	m_bTransparent=FALSE;
	m_pbitmap=NULL;
	m_clrTransColor=RGB(255,0,255);
	m_TextAlign=ButtonTextAlignNone;
	m_hFont=NULL;
	GetCursorPos(&m_prePos);
}

THSkinButton::~THSkinButton()
{
	if(m_hIcon)
		DeleteObject(m_hIcon);
	if(m_hCursor)
		DeleteObject(m_hCursor);
	if (m_pbitmap) delete m_pbitmap;
}

BEGIN_MESSAGE_MAP(THSkinButton, CButton)
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	ON_WM_KILLFOCUS()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_SETCURSOR()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_CREATE()
END_MESSAGE_MAP()

BOOL THSkinButton::OnEraseBkgnd( CDC * cdc)
{
	return TRUE;
}

void THSkinButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC buttonDC;				//按钮设备场景
	CDC memDC;
	CRect rc,textrc;	//按钮区域,选中,正常,活动
	UINT state;
	CBitmap bmp,*oldBmp;
	int icon_x,icon_y;
	HGDIOBJ hFont,hOldFont;

	rc=lpDrawItemStruct->rcItem;
	textrc=rc;

	if(m_hIcon)
		textrc.left=rc.left+4+18+4;
	else
		textrc.left=rc.left+4;

	icon_x=rc.left+4;
	icon_y=rc.top+(rc.Height()-18)/2;

	hFont=m_hFont;
	if (!hFont)
		hFont=::GetStockObject(DEFAULT_GUI_FONT);
	buttonDC.Attach(lpDrawItemStruct->hDC);

	for(UINT i=0;i<m_rtEmpty.GetSize();i++)
	{
		long pt=m_rtEmpty[i];
		int x=LOWORD(pt);
		int y=HIWORD(pt);
		buttonDC.ExcludeClipRect(x,y,x+1,y+1);
	}

	if (!memDC.CreateCompatibleDC(&buttonDC)) return;

	if (!bmp.CreateCompatibleBitmap(&buttonDC,rc.Width(),rc.Height()))
	{
		memDC.DeleteDC();
		return;
	}

	oldBmp=memDC.SelectObject(&bmp);
	hOldFont=memDC.SelectObject(hFont);
	memDC.SetBkMode(TRANSPARENT);
	int offset=0;

	state=lpDrawItemStruct->itemState;
	if(!(state&ODS_DISABLED))
	{
		if(m_bMouseIn)				//击活
		{
			if(state&ODS_SELECTED)		//鼠标按下
				offset=m_nTop+m_nHeight*2;
			else					//处于击活状态
				offset=m_nTop+m_nHeight*1;
		}
		else
			offset=m_nTop+m_nHeight*0;
		if (m_nTypeRadio==2)
			offset=m_nTop+m_nHeight*2;
	}
	else
		offset=m_nTop+m_nHeight*3;

	if (m_pbitmap)
		m_pbitmap->DrawBitmap(memDC,0,0,rc.Width(),rc.Height(),m_nLeft,offset,m_nWidth,m_nHeight);

	if (m_TextAlign!=ButtonTextAlignNone)
	{
		COLORREF oldClr=memDC.SetTextColor(m_crText);
		CString text;
		GetWindowText(text);
		UINT format;
		if (m_TextAlign==ButtonTextAlignCenter)
			format=DT_CENTER;
		else if (m_TextAlign==ButtonTextAlignRight)
			format=DT_RIGHT;
		else
			format=DT_LEFT;
		memDC.DrawText(text,&textrc,format|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
		memDC.SetTextColor(oldClr);
	}

	if (m_bTransparent)
		buttonDC.TransparentBlt(0,0,rc.Width(),rc.Height(),&memDC,0,0,rc.Width(),rc.Height(),m_clrTransColor);
	else
		buttonDC.BitBlt(0,0,rc.Width(),rc.Height(),&memDC,0,0,SRCCOPY);
	if(m_hIcon)
		DrawIconEx(buttonDC.m_hDC,icon_x,icon_y,m_hIcon,16,16,0,NULL,DI_NORMAL);

	if(hOldFont)
		memDC.SelectObject(hOldFont);

	if(oldBmp)
		memDC.SelectObject(oldBmp);

	memDC.DeleteDC();
	bmp.DeleteObject();
	buttonDC.Detach();
}

void THSkinButton::PreSubclassWindow()
{
	SetButtonStyle(GetButtonStyle()|BS_OWNERDRAW|WS_EX_TRANSPARENT);
	CButton::PreSubclassWindow();
}

void THSkinButton::SetBitmap(HBITMAP hBitmap,int left,int top,int width,int height,BOOL bExpand,BOOL bTrans,COLORREF crTran,BOOL bNormalRect,COLORREF crNormal)
{
	if(!hBitmap) return;
	if (m_pbitmap) delete m_pbitmap;
	m_hBmp=hBitmap;
	m_pbitmap=new THBitmap(m_hBmp);
	m_pbitmap->SetMask(m_mask,bTrans,crTran);
	::GetObject(m_hBmp,sizeof(m_bmp),&m_bmp);
	m_nWidth=width;
	m_nHeight=height;
	m_nLeft=left;
	m_nTop=top;
	m_bTransparent=bTrans;
	m_clrTransColor=crTran;
	if(bExpand)
		SetWindowPos(NULL,0,0,width,height,SWP_NOMOVE|SWP_NOZORDER);
	if(!bNormalRect)
	{
		if (m_pbitmap->BitmapRegion(crNormal,&m_rtEmpty,&m_rgnTotal,m_nTop,m_nLeft,m_nWidth,m_nHeight))
			SetWindowRgn(m_rgnTotal,TRUE);
	}
}

void THSkinButton::OnTimer(UINT nIDEvent)
{
	CRect btRect;				//按钮区域
	GetWindowRect(btRect);
	CPoint pos;
	GetCursorPos(&pos);
	if(!btRect.PtInRect(pos))	//鼠标不在按钮内
	{
		GetCursorPos(&m_prePos);
		m_bMouseIn=FALSE;
		m_bMouseDown=FALSE;
		KillTimer(m_nTimer);
		m_nTimer=0;
		OnKillFocus(NULL);
		Invalidate(FALSE);
	}

	CButton::OnTimer(nIDEvent);
}

void THSkinButton::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_bMouseDown=TRUE;
	Invalidate(FALSE);
	CButton::OnLButtonDown(nFlags, point);
}

void THSkinButton::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	m_bMouseDown=TRUE;
	Invalidate(FALSE);
	CButton::OnLButtonDblClk(nFlags, point);
}

void THSkinButton::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_nTypeRadio==1)
		m_nTypeRadio=2;
	else if (m_nTypeRadio==2)
		m_nTypeRadio=1;
	m_bMouseDown=FALSE;
	Invalidate(FALSE);
	CButton::OnLButtonUp(nFlags, point);
}

BOOL THSkinButton::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if(m_hCursor)
	{
		::SetCursor(m_hCursor);
		return TRUE;
	}
	return CButton::OnSetCursor(pWnd,nHitTest,message);
}

void THSkinButton::OnKillFocus(CWnd* pNewWnd)
{
	m_bMouseIn=FALSE;
	Invalidate(FALSE);
	CButton::OnKillFocus(pNewWnd);
}

void THSkinButton::OnRButtonDown(UINT nFlags, CPoint point)
{
	CButton::OnRButtonDown(nFlags, point);
}

void THSkinButton::OnMouseMove(UINT nFlags, CPoint point)
{
	CRect btRect;
	GetWindowRect(btRect);
	CPoint pos;
	GetCursorPos(&pos);

	if(btRect.PtInRect(pos))	//鼠标是否在按钮内
	{
		m_bMouseIn=TRUE;
		if(!btRect.PtInRect(m_prePos))
		{
			//SetTimer(1,10,NULL);
			Invalidate(FALSE);
			GetCursorPos(&m_prePos);
		}
	}
	else						//不在
		m_bMouseIn=FALSE;
	if(!m_nTimer)
		m_nTimer=(UINT)SetTimer(251,10,NULL);
	CButton::OnMouseMove(nFlags, point);
}

void THSkinButton::OnRButtonUp(UINT nFlags, CPoint point)
{
	CButton::OnRButtonUp(nFlags, point);
}

//because of the Skinplusplus library to discard the message when in noskin mode,so parse these message in pretranslatemessage
BOOL THSkinButton::PreTranslateMessage(MSG* pMsg)
{
	switch(pMsg->message)
	{
	case WM_MOUSEMOVE:
		OnMouseMove((UINT)pMsg->wParam,CPoint(pMsg->lParam));break;
	case WM_LBUTTONDOWN:
		OnLButtonDown((UINT)pMsg->wParam,CPoint(pMsg->lParam));break;
	case WM_LBUTTONUP:
		OnLButtonUp((UINT)pMsg->wParam,CPoint(pMsg->lParam));break;
	case WM_RBUTTONDOWN:
		OnRButtonDown((UINT)pMsg->wParam,CPoint(pMsg->lParam));break;
	case WM_RBUTTONUP:
		OnRButtonUp((UINT)pMsg->wParam,CPoint(pMsg->lParam));break;
	case WM_LBUTTONDBLCLK:
		OnLButtonDblClk((UINT)pMsg->wParam,CPoint(pMsg->lParam));break;
	case WM_TIMER:
		OnTimer((UINT)pMsg->wParam);break;
	case WM_KILLFOCUS:
		OnKillFocus(CWnd::FromHandle((HWND)pMsg->wParam));break;
	case WM_SETCURSOR:
		OnSetCursor(CWnd::FromHandle((HWND)pMsg->wParam),LOWORD(pMsg->lParam),HIWORD(pMsg->lParam));break;
	case WM_ERASEBKGND:
		OnEraseBkgnd(CDC::FromHandle((HDC)pMsg->wParam));break;
	case WM_CREATE:
		OnCreate((LPCREATESTRUCT)pMsg->lParam);break;
	}
	return CButton::PreTranslateMessage(pMsg);
}

int THSkinButton::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CButton::OnCreate(lpCreateStruct)==-1) return -1;
	//设置背景
	SetClassLong(m_hWnd,GCL_HBRBACKGROUND,NULL);
	return 0;
}

void THSkinButton::SetShowText(ButtonTextAlign align,COLORREF crText,HFONT hFont)
{
	m_TextAlign=align;
	m_crText=crText;
	m_hFont=hFont;
}

void THSkinButton::SetRadioMode( BOOL bPress/*=FALSE*/ ) 
{
	m_nTypeRadio=bPress?2:1;
	Invalidate();
}

void THSkinButton::SetMask(double mask)
{
	m_mask=mask;
	if (m_pbitmap) m_pbitmap->SetMask(m_mask,m_bTransparent,m_clrTransColor);
	Invalidate();
}
