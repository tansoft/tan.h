#pragma once

/**
* @brief �ṩ�����Ĭ�ϵ�icon list
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-07-01 �½���
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
	* @brief ��ȡϵͳImageList
	* @param bSmall		�Ƿ񷵻�Сͼ���б�
	* @return ����ImageListָ��
	*/
	CImageList *GetImageList(BOOL bSmall=TRUE)
	{
		if (bSmall) return &m_ImageList;
		return &m_BigImageList;
	}

	/**
	* @brief ��ȡ�ļ����Ͷ�Ӧ��ͼ������
	* @param sFilename	�ļ���
	* @param bSmall		�Ƿ�ΪСͼ��
	* @param pnOpen		��Ϊ�գ��ǿ�ֵ���ش�ͼ�������
	* @return ����ͼ�������
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
	* @brief ��ȡ�����ļ��ж�Ӧ��ͼ������
	* @param idx		�ļ��б�ʶ
	* @param bSmall		�Ƿ�ΪСͼ��
	* @param pnOpen		��Ϊ�գ��ǿ�ֵ���ش�ͼ�������
	* @return ����ͼ�������
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
