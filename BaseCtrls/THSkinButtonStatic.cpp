#include "stdafx.h"
#include "THSkinButtonStatic.h"

IMPLEMENT_DYNAMIC(THSkinButtonStatic, THSkinStatic)
THSkinButtonStatic::THSkinButtonStatic()
{
	m_bMouseDown=FALSE;
	m_nLastHitButton=0;
	m_hWnd=NULL;
	m_nFristCmd=0;
}

THSkinButtonStatic::~THSkinButtonStatic()
{
}


BEGIN_MESSAGE_MAP(THSkinButtonStatic, THSkinStatic)
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()

void THSkinButtonStatic::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	for(UINT i=0;i<m_rtEmpty.GetCount();i++)
	{
		long pt=m_rtEmpty[i];
		int x=LOWORD(pt);
		int y=HIWORD(pt);
		dc.ExcludeClipRect(x,y,x+1,y+1);
	}
	CDC bufferDC;
	if (!bufferDC.CreateCompatibleDC(&dc)) return;
	RECT rect;
	GetClientRect(&rect);
	CBitmap bitmap;
	if (!bitmap.CreateCompatibleBitmap(&dc,rect.right,rect.bottom))
	{
		bufferDC.DeleteDC();
		return;
	}
	HGDIOBJ hOldBitmap=bufferDC.SelectObject(bitmap);
	PrivateDraw(rect,&bufferDC);
	if (m_bTransparent)
		dc.TransparentBlt(0,0,rect.right,rect.bottom,&bufferDC,0,0,rect.right,rect.bottom,m_clrTransColor);
	else
		dc.StretchBlt(0,0,rect.right,rect.bottom,&bufferDC,0,0,rect.right,rect.bottom,SRCCOPY);
	if (hOldBitmap) bufferDC.SelectObject(hOldBitmap);
	bitmap.DeleteObject();
	bufferDC.DeleteDC();
	for(UINT i=0;i<m_hWndOver.GetCount();i++)
		::InvalidateRect(m_hWndOver[i],NULL,FALSE);
}

void THSkinButtonStatic::OnTimer(UINT nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if (nIDEvent==m_nTimer)
	{
		m_nTimerIndex++;
		if (m_nTimerIndex>=m_displayqueue.GetCount())
			m_nTimerIndex=0;
		m_nIndex=m_displayqueue[m_nTimerIndex];
		Invalidate();
	}
	THSkinStatic::OnTimer(nIDEvent);
}

void THSkinButtonStatic::PrivateDraw(RECT rect,CDC *pDC)
{
	THSkinStatic::PrivateDraw(rect,pDC);
	THPosition pos=m_textinfo.GetStartPosition();
	int key;
	TextInfo *info;
	CRect btRect;
	CPoint ptpos;
	BOOL binit=FALSE;
	while(!pos.IsEmpty())
	{
		if (m_textinfo.GetNextPosition(pos,key,info))
		{
			if (info->type==2)
			{
				if (info->typeinfo.bmp.bmp)
				{
					if (!binit)
					{
						binit=TRUE;
						GetWindowRect(btRect);
						GetCursorPos(&ptpos);
						ptpos.x-=btRect.left;
						ptpos.y-=btRect.top;
					}
					THBitmap bit(info->typeinfo.bmp.bmp);
					int offest=0;
					if(info->rect.PtInRect(ptpos))
					{
						if (m_bMouseDown)
							offest=2;
						else
							offest=1;
						m_nLastHitButton=key;
					}
					bit.DrawBitmap(pDC->m_hDC,info->rect.left,info->rect.top,info->rect.Width(),info->rect.Height(),info->typeinfo.bmp.x+offest*info->typeinfo.bmp.w,info->typeinfo.bmp.y,info->typeinfo.bmp.w,info->typeinfo.bmp.h,info->typeinfo.bmp.btran,info->typeinfo.bmp.trancolor);

					if (info->align!=StaticTextAlignNone)
					{
						COLORREF oldClr=pDC->SetTextColor(info->typeinfo.font.fontcolor);
						HFONT hOldFont=(HFONT)pDC->SelectObject(info->typeinfo.font.font?info->typeinfo.font.font:(::GetStockObject(DEFAULT_GUI_FONT)));
						UINT format;
						if (info->align==StaticTextAlignCenter)
							format=DT_CENTER;
						else if (info->align==StaticTextAlignRight)
							format=DT_RIGHT;
						else
							format=DT_LEFT;
						pDC->DrawText(info->text,&info->rect,format|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
						if (hOldFont) pDC->SelectObject(hOldFont);
						pDC->SetTextColor(oldClr);
					}
				}
			}
		}
	}
}

BOOL THSkinButtonStatic::AddBmpButtonArea( int id,HBITMAP hBmp,int x,int y,int width,int height,CRect btnRect,StaticTextAlign align,BOOL btran,COLORREF trancolor,HFONT font,COLORREF fontcolor)
{
	TextInfo *info=NULL;
	BOOL bpret=m_textinfo.GetAt(id,info);
	if (bpret==FALSE || info==NULL)
	{
		info=new TextInfo;
		m_textinfo.SetAt(id,info);
	}
	info->type=2;
	info->typeinfo.bmp.bmp=hBmp;
	info->typeinfo.bmp.x=x;
	info->typeinfo.bmp.y=y;
	info->typeinfo.bmp.w=width;
	info->typeinfo.bmp.h=height;
	info->typeinfo.bmp.btran=btran;
	info->typeinfo.bmp.trancolor=trancolor;
	info->typeinfo.font.font=font;
	info->typeinfo.font.fontcolor=fontcolor;
	info->align=align;
	info->rect=btnRect;
	return TRUE;
}
void THSkinButtonStatic::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	Invalidate(FALSE);

	THSkinStatic::OnMouseMove(nFlags, point);
}

void THSkinButtonStatic::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	m_bMouseDown=TRUE;
	Invalidate(FALSE);
	THSkinStatic::OnLButtonDown(nFlags, point);
}

void THSkinButtonStatic::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	m_bMouseDown=FALSE;
	Invalidate(FALSE);
	::SendMessage(m_hWnd,WM_COMMAND,m_nFristCmd+m_nLastHitButton,0);
	THSkinStatic::OnLButtonUp(nFlags, point);
}

void THSkinButtonStatic::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	m_bMouseDown=TRUE;
	Invalidate(FALSE);
	THSkinStatic::OnLButtonDblClk(nFlags, point);
}

void THSkinButtonStatic::SetButtonMsgHandler( HWND hwnd,UINT fristCmd ) 
{
	m_hWnd=hwnd;
	m_nFristCmd=fristCmd;
}