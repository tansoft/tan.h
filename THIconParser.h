#pragma once

#include <THSystem.h>
#include <THMemBuf.h>

/**
* @brief Icon文件处理类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2011-02-19 新建类
*/
class THIconParser
{
public:
	static BOOL SaveIcon(HICON Icon,THMemBuf *buf,int nBit=32)
	{
		TIconRec List;
		ICONINFO IconInfo;
		TCursorOrIcon CI;
		DWORD MonoInfoSize;
		DWORD ColorInfoSize;
		DWORD MonoBitsSize;
		DWORD ColorBitsSize;
		switch(nBit)
		{
			case 0:case 1:case 4:case 8:case 16:case 24:case 32:break;
			default:return FALSE;
		}
		if (!Icon || !buf) return FALSE; 
		memset(&CI,0,sizeof(CI));
		memset(&List,0,sizeof(List));
		GetIconInfo(Icon,&IconInfo);
		GetDIBSizes(IconInfo.hbmMask,MonoInfoSize,MonoBitsSize,1);
		GetDIBSizes(IconInfo.hbmColor,ColorInfoSize,ColorBitsSize,nBit);
		BITMAPINFO *MonoInfo=(BITMAPINFO*)malloc(MonoInfoSize);
		BITMAPINFO *ColorInfo=(BITMAPINFO*)malloc(ColorInfoSize);
		void *MonoBits=malloc(MonoBitsSize);
		void *ColorBits=malloc(ColorBitsSize);
		GetDIB(IconInfo.hbmMask,MonoInfo,MonoBits,1);
		GetDIB(IconInfo.hbmColor,ColorInfo,ColorBits,nBit);
		CI.wType=(WORD)0x10001;CI.Count=(WORD)1;
		buf->AddBuf(&CI,sizeof(CI));
		List.Width=(BYTE)ColorInfo->bmiHeader.biWidth;
		List.Height=(BYTE)ColorInfo->bmiHeader.biHeight;
		List.Colors=ColorInfo->bmiHeader.biPlanes*ColorInfo->bmiHeader.biBitCount;
		List.DIBSize=ColorInfoSize+ColorBitsSize+MonoBitsSize;
		List.DIBOffset=sizeof(CI)+sizeof(List);
		buf->AddBuf(&List,sizeof(List));
		ColorInfo->bmiHeader.biHeight*=2;
		buf->AddBuf(ColorInfo,ColorInfoSize);
		buf->AddBuf(ColorBits,ColorBitsSize);
		buf->AddBuf(MonoBits,MonoBitsSize);
		free(ColorInfo);free(MonoInfo);free(ColorBits);free(MonoBits);
		DeleteObject(IconInfo.hbmColor);
		DeleteObject(IconInfo.hbmMask);
		return TRUE;
	}
protected:
	struct TCursorOrIcon
	{
		WORD Reserved;
		WORD wType;
		WORD Count;
	};
	struct TIconRec
	{
		BYTE Width;
		BYTE Height;
		WORD Colors;
		WORD Reserved1;
		WORD Reserved2;
		long DIBSize;
		long DIBOffset;
	};
	static void InitBmpInfoHeader(HBITMAP Bitmap,BITMAPINFOHEADER& BI,int nBit)
	{
		int Bytes;
		DIBSECTION DS;
		DS.dsBmih.biSize=0;
		Bytes=GetObject(Bitmap,sizeof(DS),&DS);
		if (Bytes>=sizeof(DS.dsBm)+sizeof(DS.dsBmih) && DS.dsBmih.biSize>=sizeof(DS.dsBmih))
			memcpy(&BI,&DS.dsBmih,sizeof(BITMAPINFOHEADER));
		else
		{
			memset(&BI,0,sizeof(BI));
			BI.biSize=sizeof(BI);
			BI.biWidth=DS.dsBm.bmWidth;
			BI.biHeight=DS.dsBm.bmHeight;
			BI.biBitCount=DS.dsBm.bmPlanes*DS.dsBm.bmBitsPixel;
		}
		if (nBit!=0) BI.biBitCount=nBit;
		if (BI.biBitCount<=8) BI.biClrUsed=1<<BI.biBitCount;
		BI.biPlanes=1;
		if (BI.biClrImportant>BI.biClrUsed) BI.biClrImportant=BI.biClrUsed;
		if (BI.biSizeImage==0) BI.biSizeImage=((BI.biWidth*BI.biBitCount+31)/32)*4*BI.biHeight;
	}
	static void GetDIBSizes(HBITMAP Bitmap,DWORD& InfoSize,DWORD& ImageSize,int nBit)
	{
		BITMAPINFOHEADER BI;
		InitBmpInfoHeader(Bitmap,BI,nBit);
		InfoSize=sizeof(BITMAPINFOHEADER);
		if (BI.biBitCount>8)
		{
			if (BI.biCompression&BI_BITFIELDS) InfoSize+=12;
		}
		else
			InfoSize+=sizeof(RGBQUAD)*(BI.biClrUsed!=0?BI.biClrUsed:(1<<BI.biBitCount));
		ImageSize=BI.biSizeImage;
	}
	static void GetDIB(HBITMAP Bitmap,BITMAPINFO* BmpInfo,void* Bits,int nBit)
	{
		HDC DC=CreateCompatibleDC(NULL);
		InitBmpInfoHeader(Bitmap,BmpInfo->bmiHeader,nBit);
		GetDIBits(DC,Bitmap,0,BmpInfo->bmiHeader.biHeight,Bits,BmpInfo,DIB_RGB_COLORS);
		DeleteDC(DC);
	}

	/*使用OleCreatePictureIndirect保存的颜色有问题
	static BOOL Ico2Buf(HICON hIcon,THMemBuf *buf)
	{
		if (!hIcon || !buf) return FALSE;
		BOOL bRet=FALSE;
		PICTDESC picdesc;
		picdesc.cbSizeofstruct=sizeof(PICTDESC);
		picdesc.picType=PICTYPE_ICON;
		picdesc.icon.hicon=hIcon;
		IPicture* pPicture=NULL;
		if (SUCCEEDED(OleCreatePictureIndirect(&picdesc,IID_IPicture,FALSE,(VOID**)&pPicture)))
		{
			LPSTREAM pStream;
			if (SUCCEEDED(CreateStreamOnHGlobal(NULL,TRUE,&pStream)))
			{
				LONG size;
				if (SUCCEEDED(pPicture->SaveAsFile(pStream,TRUE,&size)))
				{
					LARGE_INTEGER li;
					li.HighPart=0;
					li.LowPart=0;
					ULARGE_INTEGER ulnewpos;
					if (SUCCEEDED(pStream->Seek(li,STREAM_SEEK_SET,&ulnewpos)))
					{
						char pbuf[1024];
						ULONG uReadCount=1;
						while(uReadCount>0)
						{
							if (FAILED(pStream->Read(pbuf,sizeof(pbuf),&uReadCount))) break;
							if(uReadCount>0) buf->AddBuf(pbuf,uReadCount);
						}
						bRet=TRUE;
					}
				}
				pStream->Release();
			}
			pPicture->Release();
		}
		return bRet;
	}*/
};
