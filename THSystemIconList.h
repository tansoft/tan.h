#pragma once

/**
* @brief 提供浏览器默认的icon list
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-07-01 新建类
*/
class THSystemIconList
{
public:
	THSystemIconList(void)
	{
		if(GetRefCount()==0)
		{
			::CoInitialize(NULL);
			SHFILEINFO sfi;
			HIMAGELIST hSystemImageList=(HIMAGELIST)SHGetFileInfo(_T("c:\\"),0,&sfi,sizeof(SHFILEINFO),SHGFI_SYSICONINDEX|SHGFI_SMALLICON);
			VERIFY(m_ImageList.Attach(hSystemImageList));
			hSystemImageList=(HIMAGELIST)SHGetFileInfo(_T("c:\\"),0,&sfi,sizeof(SHFILEINFO),SHGFI_SYSICONINDEX);
			VERIFY(m_BigImageList.Attach(hSystemImageList));
		} 
		GetRefCount()++;
	}
	virtual ~THSystemIconList(void)
	{
		GetRefCount()--; 
		if(m_nRefCount==0)
		{
			//Detach from the image list to prevent problems on 95/98 where
			//the system image list is share dacross processes
			m_ImageList.Detach();
			m_BigImageList.Detach();
			::CoUninitialize();
		}
	}

	/**
	* @brief 获取系统ImageList
	* @param bSmall		是否返回小图标列表
	* @return 返回ImageList指针
	*/
	CImageList *GetImageList(BOOL bSmall=TRUE)
	{
		if (bSmall) return &m_ImageList;
		return &m_BigImageList;
	}

	/**
	* @brief 获取文件类型对应的图标索引
	* @param sFilename	文件名
	* @param bSmall		是否为小图标
	* @param pnOpen		可为空，非空值返回打开图标的索引
	* @return 返回图标的索引
	*/
	int GetIconIndex(const CString &sFilename,BOOL bSmall=TRUE,int *pnOpen=NULL)
	{
		UINT flag=SHGFI_SYSICONINDEX;
		if (bSmall) flag|=SHGFI_SMALLICON;
		SHFILEINFO sfi;
		if (pnOpen)
		{
			if (SHGetFileInfo(sFilename,0,&sfi,sizeof(SHFILEINFO),flag|SHGFI_OPENICON)!=0)
				*pnOpen=sfi.iIcon;
			else
				*pnOpen=-1;
		}
		if (SHGetFileInfo(sFilename,0,&sfi,sizeof(SHFILEINFO),flag)!=0)
			return sfi.iIcon;
		return -1;
	}

	/**
	* @brief 获取特殊文件夹对应的图标索引
	* @param idx		文件夹标识
	* @param bSmall		是否为小图标
	* @param pnOpen		可为空，非空值返回打开图标的索引
	* @return 返回图标的索引
	*/
	int GetSystemPathIndex(int idx,BOOL bSmall=TRUE,int *pnOpen=NULL)
	{
		LPITEMIDLIST idl;
		if(SUCCEEDED(SHGetSpecialFolderLocation(NULL,nFolder,&idl)))
		{
			UINT flag=SHGFI_SYSICONINDEX|SHGFI_PIDL;
			if (bSmall) flag|=SHGFI_SMALLICON;
			if (pnOpen)
			{
				if (SHGetFileInfo(sFilename,0,&sfi,sizeof(SHFILEINFO),flag|SHGFI_OPENICON)!=0)
					*pnOpen=sfi.iIcon;
				else
					*pnOpen=-1;
			}
			if (SHGetFileInfo(sFilename,0,&sfi,sizeof(SHFILEINFO),flag)!=0)
				return sfi.iIcon;
		}
		return -1;
	}
protected:
	CImageList m_ImageList;
	CImageList m_BigImageList;
	static int& GetRefCount()
	{
		static int m_nRefCount=0;
		return m_nRefCount;
	}
};
