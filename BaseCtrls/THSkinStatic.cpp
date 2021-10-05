#include "stdafx.h"
#include "THSkinStatic.h"
// THSkinStatic

IMPLEMENT_DYNAMIC(THSkinStatic, CStatic)
THSkinStatic::THSkinStatic()
{
	m_nIndex=0;
	m_nTimer=0;
	m_mask=1.0;
	m_pbitmap=NULL;
	m_textinfo.SetFreeProc(MapItemFreeCallBack);
}

THSkinStatic::~THSkinStatic()
{
	if (m_pbitmap) delete m_pbitmap;
}

void THSkinStatic::MapItemFreeCallBack(void *key,void *value,void *adddata)
{
	delete (TextInfo *)value;
}

BEGIN_MESSAGE_MAP(THSkinStatic, CStatic)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_CREATE()
	ON_WM_TIMER()
END_MESSAGE_MAP()

BOOL THSkinStatic::OnEraseBkgnd( CDC * cdc)
{
	return TRUE;
}

void THSkinStatic::PrivateDraw(RECT rect,CDC *pDC)
{
	pDC->SetBkMode(TRANSPARENT);

	if (m_pbitmap)
		m_pbitmap->DrawBitmap(pDC->m_hDC,0,0,rect.right,rect.bottom,m_nLeft+m_nIndex*m_nWidth,m_nTop,m_nWidth,m_nHeight,m_bTransparent,m_clrTransColor);

	THPosition pos=m_textinfo.GetStartPosition();
	int key;
	TextInfo *info;

	while(!pos.IsEmpty())
	{
		if (m_textinfo.GetNextPosition(pos,key,info))
		{
			if (info->type==0)
			{
				if (info->align!=StaticTextAlignNone)
				{
					HFONT font=info->typeinfo.font.font;
					if (!font)
						font=(HFONT)::GetStockObject(DEFAULT_GUI_FONT);
					HGDIOBJ hOldFont=pDC->SelectObject(font);
					COLORREF crOld=pDC->SetTextColor(info->typeinfo.font.fontcolor);
					UINT format;
					if (info->align==StaticTextAlignCenter)
						format=DT_CENTER;
					else if (info->align==StaticTextAlignRight)
						format=DT_RIGHT;
					else
						format=DT_LEFT;
					pDC->DrawText(info->text,info->rect,format|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
					if (crOld) pDC->SetTextColor(crOld);
					if (hOldFont) pDC->SelectObject(hOldFont);
				}
			}
			else if (info->type==1)
			{
				if (info->typeinfo.bmp.bmp && info->align!=StaticTextAlignNone)
				{
					THBitmap bit(info->typeinfo.bmp.bmp);
					int startx=0;
					if (info->align==StaticTextAlignLeft)
						startx=info->rect.left;
					else if (info->align==StaticTextAlignCenter)
						startx=info->rect.left+(info->rect.Width()-(info->typeinfo.bmp.w*info->text.GetLength()))/2;
					else if (info->align==StaticTextAlignRight)
						startx=info->rect.right-info->typeinfo.bmp.w*info->text.GetLength();
					for(int i=0;i<info->text.GetLength();i++)
					{
						//³¬³ö·¶Î§£¬²»½øÐÐ»æ»­
						int x=startx+i*info->typeinfo.bmp.w;
						if (x<info->rect.left || x+info->typeinfo.bmp.w>info->rect.right)
							continue;
						int index=GetFilterIndex(info->typeinfo.bmp.filtertext,info->text.GetAt(i));
						if (index>=0)
							bit.DrawBitmap(pDC->m_hDC,x,info->rect.top,info->typeinfo.bmp.w,info->rect.Height(),
							info->typeinfo.bmp.x+info->typeinfo.bmp.w*index,info->typeinfo.bmp.y,info->typeinfo.bmp.w,info->typeinfo.bmp.h,info->typeinfo.bmp.btran,info->typeinfo.bmp.trancolor);
					}
				}
			}
		}
	}
}

void THSkinStatic::OnPaint()
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

void THSkinStatic::SetBitmap(HBITMAP hBitmap,int left,int top,int width,int height,BOOL bExpand,BOOL bTrans,COLORREF crTran,BOOL bNormalRect,COLORREF crNormal)
{
	if(!hBitmap) return;
	m_hBmp=hBitmap;
	if (m_pbitmap) delete m_pbitmap;
	m_pbitmap=new THBitmap(m_hBmp);
	m_pbitmap->SetMask(m_mask,bTrans,crTran);
	::GetObject(m_hBmp,sizeof(m_bmp),&m_bmp);
	m_nIndex=0;
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

int THSkinStatic::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CStatic::OnCreate(lpCreateStruct) == -1)
		return -1;
	//ÉèÖÃ±³¾°
	SetClassLong(m_hWnd,GCL_HBRBACKGROUND,NULL);
	return 0;
}

HFONT THSkinStatic::CreatePresentFont( CString Name,int nHeight/*=-16*/,int nWeight/*=FW_BOLD*/,BOOL bItalic/*=FALSE*/,BOOL bUnderline/*=FALSE*/ ) 
{
	return ::CreateFont(nHeight,0,0,0,nWeight,bItalic,bUnderline,0,0,0,0,0,0,Name);
}

BOOL THSkinStatic::AddTextArea( int id,HFONT font,CRect textRect,COLORREF fontcolor,StaticTextAlign align/*=TextAlignLeft*/ ) 
{
	TextInfo *info=NULL;
	BOOL bpret=m_textinfo.GetAt(id,info);
	if (bpret==FALSE || info==NULL)
	{
		info=new TextInfo;
		m_textinfo.SetAt(id,info);
	}
	info->type=0;
	info->typeinfo.font.font=font;
	info->typeinfo.font.fontcolor=fontcolor;
	info->align=align;
	info->rect=textRect;
	return TRUE;
}

BOOL THSkinStatic::AddBmpTextArea(int id,HBITMAP hBmp,int x,int y,int width,int height,CString filtertext,CRect textRect,StaticTextAlign align,BOOL btran,COLORREF trancolor)
{
	TextInfo *info=NULL;
	BOOL bpret=m_textinfo.GetAt(id,info);
	if (bpret==FALSE || info==NULL)
	{
		info=new TextInfo;
		m_textinfo.SetAt(id,info);
	}
	info->type=1;
	info->typeinfo.bmp.bmp=hBmp;
	info->typeinfo.bmp.x=x;
	info->typeinfo.bmp.y=y;
	info->typeinfo.bmp.w=width;
	info->typeinfo.bmp.h=height;
	info->typeinfo.bmp.btran=btran;
	info->typeinfo.bmp.trancolor=trancolor;
	info->typeinfo.bmp.filtertext=filtertext;
	info->align=align;
	info->rect=textRect;
	return TRUE;
}

BOOL THSkinStatic::RemoveTextArea( int id ) 
{
	TextInfo *info=NULL;
	if (!m_textinfo.GetAt(id,info)) return FALSE;
	if (!info) return FALSE;
	delete info;
	m_textinfo.RemoveAt(id);
	return TRUE;
}
BOOL THSkinStatic::SetAreaText( int id,CString text ) 
{
	TextInfo *info=NULL;
	BOOL bpret=m_textinfo.GetAt(id,info);
	if (bpret==FALSE || info==NULL) return FALSE;
	info->text=text;
	return TRUE;
}
void THSkinStatic::SetBitmapIndex(int index)
{
	m_nIndex=index;
	Invalidate();
}

void THSkinStatic::OnTimer(UINT nIDEvent)
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
	CStatic::OnTimer(nIDEvent);
}

void THSkinStatic::SetChangeTimer(int interval,const THList<int> *displayqueue)
{
	if (m_nTimer)
		KillTimer(m_nTimer);
	m_nTimerIndex=0;
	m_displayqueue.RemoveAll();
	if (interval>0 && displayqueue->GetCount()>0)
	{
		m_displayqueue.Copy(displayqueue);
		m_nTimer=(UINT)SetTimer(1546,interval,NULL);
		m_nIndex=m_displayqueue[m_nTimerIndex];
	}
	Invalidate();
}

void THSkinStatic::SetMask(double mask)
{
	m_mask=mask;
	if (m_pbitmap) m_pbitmap->SetMask(m_mask,m_bTransparent,m_clrTransColor);
	Invalidate();
}

void THSkinStatic::AddOverHwnd(HWND hWnd)
{
	m_hWndOver.AddHead(hWnd);
}

int THSkinStatic::GetFilterIndex(CString filter,TCHAR ch)
{
	return filter.Find(ch,0);
}