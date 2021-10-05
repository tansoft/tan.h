#pragma once

#include <THString.h>
#include <THStruct.h>

/**
* @brief 对话框控件位置调整类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-07-01 新建类
*/
class THResizeDialogRect
{
public:
	THResizeDialogRect()
	{
		m_hWnd=NULL;
		m_BaseSize.cx=0;
		m_BaseSize.cy=0;
		m_CurSize=m_BaseSize;
		m_ctrllist.SetFreeProc(FreeCtrlList);
	}

	virtual ~THResizeDialogRect()
	{
	}

	typedef enum _THResizeType{
		TopLeft=			0x11,		TopCenter=		0x12,		TopRight=		0x14,
		MiddleLeft=		0x21,		MiddleCenter=	0x22,		MiddleRight=	0x24,
		BottomLeft=		0x41,		BottomCenter=0x42,		BottomRight=	0x44
	}THResizeType;

	void OnSize(int x,int y)
	{
		m_CurSize.cx=x;
		m_CurSize.cy=y;
		//fixme,set dialog size with m_hWnd,m_CurSize
		THPosition pos=m_ctrllist.GetStartPosition();
		THResizeControl *ctrl;
		while(!pos.IsEmpty())
		{
			if (m_ctrllist.GetNextPosition(pos,ctrl))
			{
				ReCalcSize(ctrl->tl,ctrl->rect.left,ctrl->currect.left,ctrl->rect.top,ctrl->currect.top);
				ReCalcSize(ctrl->br,ctrl->rect.right,ctrl->currect.right,ctrl->rect.bottom,ctrl->currect.bottom);
				//fixme,set control size with ctrl->hWnd,ctrl->currect
			}
		}
	}

	BOOL IsControlRect(int x,int y)
	{
		THPosition pos=m_ctrllist.GetStartPosition();
		THResizeControl *ctrl;
		while(!pos.IsEmpty())
		{
			if (m_ctrllist.GetNextPosition(pos,ctrl))
			{
				if (ctrl->currect.left<=x && ctrl->currect.right>=x &&
					ctrl->currect.top<=y && ctrl->currect.bottom>=y)
					return TRUE;
			}
		}
		return FALSE;
	}

	void SetBaseSize(HWND hWnd)
	{
		m_hWnd=hWnd;
		RECT r;
		GetWindowRect(hWnd,&r);
		m_BaseSize.cx=r.right-r.left;
		m_BaseSize.cy=r.bottom-r.top;
		m_CurSize=m_BaseSize;
	}

	void SetBaseSize(int x,int y)
	{
		m_BaseSize.cx=x;
		m_BaseSize.cy=y;
		m_CurSize=m_BaseSize;
		m_hWnd=NULL;
	}

	BOOL AddAnchor(HWND hWnd,THResizeType topleft,THResizeType bottomright)
	{
		THResizeControl *ctrl=new THResizeControl;
		if (!ctrl) return FALSE;
		ctrl->hWnd=hWnd;
		GetClientRect(hWnd,&ctrl->rect);
		ctrl->currect=ctrl->rect;
		ctrl->tl=topleft;
		ctrl->br=bottomright;
		return m_ctrllist.AddHead(ctrl);
	}

	BOOL AddAnchor(UINT nID,THResizeType topleft,THResizeType bottomright)
	{
		THResizeControl *ctrl=new THResizeControl;
		if (!ctrl) return FALSE;
		ctrl->hWnd=GetDlgItem(m_hWnd,nID);
		GetClientRect(ctrl->hWnd,&ctrl->rect);
		ctrl->currect=ctrl->rect;
		ctrl->tl=topleft;
		ctrl->br=bottomright;
		return m_ctrllist.AddHead(ctrl);
	}

	BOOL AddAnchor(RECT rect,THResizeType topleft,THResizeType bottomright)
	{
		THResizeControl *ctrl=new THResizeControl;
		if (!ctrl) return FALSE;
		ctrl->hWnd=NULL;
		ctrl->rect=rect;
		ctrl->tl=topleft;
		ctrl->br=bottomright;
		ctrl->currect=ctrl->rect;
		return m_ctrllist.AddHead(ctrl);
	}

protected:
	typedef struct _THResizeControl{
		HWND hWnd;
		RECT rect;
		RECT currect;
		THResizeType tl;
		THResizeType br;
	}THResizeControl;
	HWND m_hWnd;
	SIZE m_BaseSize;
	SIZE m_CurSize;
	THList<THResizeControl *>m_ctrllist;
	static void FreeCtrlList(void *key,void *value,void *adddata)
	{
		delete (THResizeControl *)value;
	}

	//orgsize cursize : dialogsize, orgx orgy curx cury control size
	void ReCalcSize(THResizeType mode,LONG orgx,LONG &curx,LONG orgy,LONG &cury)
	{
		if (mode & 0x01) curx=orgx;
		else if ((mode & 0x02))
		{
			if (m_BaseSize.cx!=0)
				curx=m_CurSize.cx*orgx/m_BaseSize.cx;
			else
				curx=0;
		}
		else if ((mode & 0x04))
			curx=orgx+(m_CurSize.cx-m_BaseSize.cx);

		if (mode & 0x10) cury=orgy;
		else if ((mode & 0x20))
		{
			if (m_BaseSize.cy!=0)
				cury=m_CurSize.cy*orgy/m_BaseSize.cy;
			else
				cury=0;
		}
		else if ((mode & 0x40))
			cury=orgy+(m_CurSize.cy-m_BaseSize.cy);
	}
};
