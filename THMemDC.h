#pragma once

class THMemDC
{
public:
	THMemDC()
	{
		hdcMem		= 0;
		fOwnBitmap	= 0;
		size.cx		= 0;
		size.cy		= 0;
		sdc			= NULL;
	}

	THMemDC(HDC hdc,HBITMAP hBitmap)
	{
		Init(hdc,hBitmap);
	}

	THMemDC(HDC hdc,int width, int height)
	{
		Init(hdc,width,height);
	}

	~THMemDC()
	{
		Cleanup();
	}

	void Init(HDC hdc, HBITMAP hBitmap)
	{
		Cleanup();

		hdcMem		= CreateCompatibleDC(hdc);
		hbmOld		= SelectObject(hdcMem, hBitmap);
		fOwnBitmap	= FALSE;
		sdc			= hdc;
		GetBitmapDimensionEx(hBitmap,&size);
	}

	void Init(HDC hdc, int width, int height)
	{
		Cleanup();

		hdcMem		= CreateCompatibleDC(hdc);
		hbmMem		= CreateCompatibleBitmap(hdc, width, height);
		hbmOld		= SelectObject(hdcMem, hbmMem);
		fOwnBitmap	= TRUE;
		sdc			= hdc;
		size.cx		= width;
		size.cy		= height;
	}

	void Cleanup()
	{
		if(hbmMem)
		{
			SelectObject(hdcMem, hbmOld);
			
			if(fOwnBitmap)
				DeleteObject(hbmMem);

			hbmMem = 0;
		}

		if(hdcMem)
		{
			DeleteDC(hdcMem);
			hdcMem = 0;
		}
	}

	HBITMAP GetBitmap()
	{
		return hbmMem;
	}

	operator HDC() { return hdcMem; }
	operator DWORD() { return (DWORD)hdcMem; }

	void CopyBitmapToDc()
	{
		::BitBlt(sdc,0,0,size.cx,size.cy,hdcMem,0,0,SRCCOPY);
	}
	void CopyBitmapToDc(HDC hdc,int x=0,int y=0)
	{
		::BitBlt(hdc,x,y,size.cx,size.cy,hdcMem,0,0,SRCCOPY);
	}
private:
	HBITMAP	hbmMem;
	HDC		hdcMem;
	HANDLE	hbmOld;
	BOOL	fOwnBitmap;
	SIZE	size;
	HDC		sdc;
};
