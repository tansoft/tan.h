#pragma once

#include <THStruct.h>

/**
* @brief 图片及内存DC准备类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-06-04 新建类
*/
/**<pre>
使用Sample：
	THBitmap bitmap(IDB_BITMAP1);
	bitmap.DrawBitmap(&bufferDC,0,0);
	bitmap.DrawBitmap(&bufferDC,0,0,100,100,0,0,100,100,TRUE,RGB(255,0,255));
</pre>*/
class THBitmap
{
public:
	THBitmap(void)
	{
		m_bAutoFree=FALSE;
		m_hBmp=NULL;
		m_hMaskBmp=NULL;
		m_mask=1.0;
		m_bTran=FALSE;
		m_crTran=RGB(255,0,255);
		memset(&m_bmp,0,sizeof(BITMAP));
	}
	/**
	* @brief 加载图片
	* @param BmpFile 图片文件名
	*/
	THBitmap(THString BmpFile,BOOL bAutoFree=TRUE)
	{
		m_hBmp=NULL;
		m_hMaskBmp=NULL;
		m_mask=1.0;
		m_bTran=FALSE;
		m_crTran=RGB(255,0,255);
		memset(&m_bmp,0,sizeof(BITMAP));
		Load(BmpFile,bAutoFree);
	}
	/**
	* @brief 加载图片
	* @param nID 图片资源ID
	*/
	THBitmap(UINT nID,BOOL bAutoFree=TRUE)
	{
		m_hBmp=NULL;
		m_hMaskBmp=NULL;
		m_mask=1.0;
		m_bTran=FALSE;
		m_crTran=RGB(255,0,255);
		memset(&m_bmp,0,sizeof(BITMAP));
		Load(nID,bAutoFree);
	}

	/**
	* @brief 加载图片
	* @param hBitmap 图片句柄
	*/
	THBitmap(HBITMAP hBitmap,BOOL bAutoFree=FALSE)
	{
		m_hBmp=NULL;
		m_hMaskBmp=NULL;
		m_mask=1.0;
		m_bTran=FALSE;
		m_crTran=RGB(255,0,255);
		memset(&m_bmp,0,sizeof(BITMAP));
		Load(hBitmap,bAutoFree);
	}

	virtual ~THBitmap(void)
	{
		if (m_bAutoFree && m_hBmp)
		{
			::DeleteObject(m_hBmp);
			m_hBmp=NULL;
		}
		if (m_mask!=1.0 && m_hMaskBmp)
		{
			::DeleteObject(m_hMaskBmp);
			m_hMaskBmp=NULL;
		}
	}

	/**
	* @brief 加载图片
	* @param BmpFile	图片文件名
	* @param bAutoFree	是否释构对象时释放绘图句柄
	* @return			是否成功加载
	*/
	inline BOOL Load(THString BmpFile,BOOL bAutoFree=TRUE){return Load((HBITMAP)::LoadImage(NULL,BmpFile,IMAGE_BITMAP,0,0,LR_LOADFROMFILE),bAutoFree);}

	/**
	* @brief 加载图片
	* @param nID		图片资源ID
	* @param bAutoFree	是否释构对象时释放绘图句柄
	* @return			是否成功加载
	*/
	inline BOOL Load(UINT nID,BOOL bAutoFree=TRUE){return Load(LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(nID)),bAutoFree);}

	/**
	* @brief 加载图片
	* @param hBitmap	图片句柄
	* @param bAutoFree	是否释构对象时释放绘图句柄
	* @return			是否成功加载
	*/
	BOOL Load(HBITMAP hBitmap,BOOL bAutoFree=FALSE)
	{
		if (m_bAutoFree && m_hBmp)
		{
			DeleteObject(m_hBmp);
			m_hBmp=NULL;
		}
		if (m_mask!=1.0 && m_hMaskBmp)
		{
			::DeleteObject(m_hMaskBmp);
			m_hMaskBmp=NULL;
		}
		if (!hBitmap) return FALSE;
		m_hBmp=hBitmap;
		m_bAutoFree=bAutoFree;
		::GetObject(m_hBmp,sizeof(m_bmp),&m_bmp);
		SetMask(m_mask,m_bTran,m_crTran);
		return TRUE;
	}

	/**
	* @brief 图片是否无效
	* @return 返回图片是否无效
	*/
	BOOL IsEmpty(){return m_hBmp==NULL;}

	/**
	* @brief 获取位图大小
	* @param x
	* @param y
	*/
	void GetBmpSize(int &x,int &y)
	{
		x=m_bmp.bmWidth;
		y=m_bmp.bmHeight;
	}

	/**
	* @brief 获取BITMAP结构指针
	* @return 返回BITMAP结构指针
	*/
	BITMAP *GetBmpInfo(){return &m_bmp;}

	/**
	* @brief 绘画Bitmap
	* @param hDC		目标DC
	* @param dx			目标坐标x
	* @param dy			目标坐标y
	* @param bTran		是否使用透明色
	* @param crTran		透明色
	* @return			是否成功绘画
	*/
	inline BOOL DrawBitmap(HDC hDC,int dx,int dy,BOOL bTran=FALSE,COLORREF crTran=RGB(255,0,255)){return DrawBitmap(hDC,dx,dy,m_bmp.bmWidth,m_bmp.bmHeight,0,0,m_bmp.bmWidth,m_bmp.bmHeight,bTran,crTran);}
	/**
	* @brief 绘画Bitmap
	* @param hDC		目标DC
	* @param dx			目标坐标x
	* @param dy			目标坐标y
	* @param dcx		目标宽
	* @param dcy		目标高
	* @param sx			源坐标x
	* @param sy			源坐标y
	* @param bTran		是否使用透明色
	* @param crTran		透明色
	* @return			是否成功绘画
	*/
	inline BOOL DrawBitmap(HDC hDC,int dx,int dy,int dcx,int dcy,int sx,int sy,BOOL bTran=FALSE,COLORREF crTran=RGB(255,0,255)){return DrawBitmap(hDC,dx,dy,dcx,dcy,sx,sy,m_bmp.bmWidth-sx,m_bmp.bmHeight-sy,bTran,crTran);}
	/**
	* @brief 绘画Bitmap
	* @param hDC		目标DC
	* @param dx			目标坐标x
	* @param dy			目标坐标y
	* @param dcx		目标宽
	* @param dcy		目标高
	* @param sx			源坐标x
	* @param sy			源坐标y
	* @param scx		源宽
	* @param scy		源高
	* @param bTran		是否使用透明色
	* @param crTran		透明色
	* @return			是否成功绘画
	*/
	BOOL DrawBitmap(HDC hDC,int dx,int dy,int dcx,int dcy,int sx,int sy,int scx,int scy,BOOL bTran=FALSE,COLORREF crTran=RGB(255,0,255))
	{
		if (!hDC) return FALSE;
		HDC memDC=::CreateCompatibleDC(hDC);
		if (!memDC) return FALSE;
		HBITMAP hOldBmp=(HBITMAP)::SelectObject(memDC,m_hMaskBmp);
		int nOldMode=::SetBkMode(memDC,TRANSPARENT);
		if (bTran)
			::TransparentBlt(hDC,dx,dy,dcx,dcy,memDC,sx,sy,scx,scy,crTran);
		else
			::StretchBlt(hDC,dx,dy,dcx,dcy,memDC,sx,sy,scx,scy,SRCCOPY);
		if (hOldBmp) ::SelectObject(memDC,hOldBmp);
		::SetBkMode(memDC,nOldMode);
		::DeleteDC(memDC);
		return TRUE;
	}
	/**
	* @brief 设置图片灰度
	* @param mask		灰度值，1.0为原图效果，0.X表示增加图像灰度，数值越小图像越黑
	* @param bTran		是否使用透明色
	* @param crTran		透明色
	*/
	void SetMask(double mask,BOOL bTran=FALSE,COLORREF crTran=RGB(255,0,255))
	{
		if (m_mask!=1.0 && m_hMaskBmp)
		{
			::DeleteObject(m_hMaskBmp);
			m_hMaskBmp=NULL;
		}
		m_mask=mask;
		m_bTran=bTran;
		m_crTran=crTran;
		if (!m_hBmp) return;
		if (m_mask==1.0)
			m_hMaskBmp=m_hBmp;
		else
		{
			HDC hDC=::GetDC(NULL);
			if (!hDC) return;
			HDC hDC1,hDC2;
			hDC1=::CreateCompatibleDC(hDC);
			if (!hDC1) return;
			hDC2=::CreateCompatibleDC(hDC);
			if (!hDC2)
			{
				::DeleteDC(hDC1);
				return;
			}
			m_hMaskBmp=::CreateCompatibleBitmap(hDC,m_bmp.bmWidth,m_bmp.bmHeight);
			if (!m_hMaskBmp)
			{
				::DeleteDC(hDC1);
				::DeleteDC(hDC2);
				return;
			}
			HGDIOBJ hOld1=SelectObject(hDC1,m_hMaskBmp);
			HGDIOBJ hOld2=SelectObject(hDC2,m_hBmp);
			BitBlt(hDC1,0,0,m_bmp.bmWidth,m_bmp.bmHeight,hDC2,0,0,SRCCOPY);
			ParseMask(hDC1,m_bmp.bmWidth,m_bmp.bmHeight);
			if (hOld1) SelectObject(hDC1,hOld1);
			if (hOld2) SelectObject(hDC2,hOld2);
			::DeleteDC(hDC1);
			::DeleteDC(hDC2);
		}
	}
	/**
	* @brief 使用图片制作不规则矩形
	* @param cTransparentColor	透明色
	* @param rtEmpty			需要置空的坐标数组
	* @param rgnTotal			不规则Rgn指针
	* @param left				坐标x
	* @param top				坐标y
	* @param width				宽
	* @param height				高
	* @return					是否成功
	*/
	BOOL BitmapRegion(COLORREF cTransparentColor,THList<long> *rtEmpty,CRgn *rgnTotal,int left,int top,int width,int height)
	{
		if (!m_hBmp) return FALSE;
		HDC hDC=::GetDC(NULL);
		if (!hDC) return FALSE;
		HDC memDC=::CreateCompatibleDC(hDC);
		if (!memDC) return FALSE;

		rtEmpty->RemoveAll();
		CRgn rgnTmp;

		HBITMAP hOldBmp=(HBITMAP)SelectObject(memDC,m_hBmp);
		rgnTotal->DeleteObject();
		rgnTotal->CreateRectRgn(left,top,left+width,top+height);
		int x,y;
		for(int i=left;i<left+width;i++)
			for(int j=top;j<top+height;j++)
			{
				if (::GetPixel(memDC,i,j)==cTransparentColor)
				{
					x=i-left;
					y=j-top;
					rgnTmp.CreateRectRgn(x,y,x+1,y+1);
					rtEmpty->AddHead(MAKELONG(x,y));
					rgnTotal->CombineRgn(rgnTotal,&rgnTmp,RGN_XOR);
					rgnTmp.DeleteObject();
				}
			}
		if (hOldBmp) ::SelectObject(memDC,hOldBmp);
		::DeleteDC(memDC);
		return TRUE;
	}

	/**
	* @brief 获取位图的句柄
	* @return			位图的句柄
	*/
	HBITMAP GetBitmap(){return m_hMaskBmp;};

	/**
	* @brief 返回位图的句柄
	* @return			位图的句柄
	*/
	operator HBITMAP() const
	{
		return m_hMaskBmp;
	}
private:
	HBITMAP		m_hBmp;
	BITMAP		m_bmp;
	double		m_mask;
	HBITMAP		m_hMaskBmp;
	BOOL		m_bTran;
	COLORREF	m_crTran;
	BOOL		m_bAutoFree;
	void ParseMask(HDC hDC,int x,int y)
	{
		if (m_mask==1.0) return;
		COLORREF cr;
		for(int i=0;i<x;i++)
		{
			for(int j=0;j<y;j++)
			{
				cr=::GetPixel(hDC,i,j);
				if (m_bTran && cr==m_crTran)
					continue;
				int r=GetRValue(cr);
				int g=GetGValue(cr);
				int b=GetBValue(cr);
				::SetPixel(hDC,i,j,RGB((int)(r*m_mask),(int)(g*m_mask),(int)(b*m_mask)));
			}
		}
	}
};
