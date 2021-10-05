
/**
* @brief Ftp文件储存器描述类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-10-18 新建类
*/
/*
class THFtpFileStore : public ITHFileStore
{
public:
	THFtpFileStore():ITHFileStore()
	{
		m_nFileListCount=0;
		m_FileList.SetFreeProc(MyFreeFile);
	}
	virtual ~THFtpFileStore()
	{
		m_FileList.RemoveAll();
	}
	virtual ITHFileStore *CreateObject()
	{
		return new THFileStore();
	}

	virtual BOOL FindFiles(THString sPath,THStringArray *arDir,THStringArray *arFile)
	{
		sPath=GetFullPath(sPath);
		return THSysMisc::FindFiles(sPath,arFile,arDir,_T("*.*"),FALSE,FALSE);
	}

	virtual BOOL GetFileAttrib(THString sFile,UINT *size,THTime *ct,THTime *mt)
	{
		THFileStatus st;
		if (THFile::GetStatus(GetFullPath(sFile),st))
		{
			if (size) *size=(UINT)st.m_size;
			if (ct) *ct=st.m_ctime;
			if (mt) *mt=st.m_mtime;
			return TRUE;
		}
		return FALSE;
	}

	virtual UINT GetFileSize(int idx)
	{
		THFile *file;
		if (m_FileList.GetAt(idx,file))
			return (UINT)file->GetLength();
		return 0;
	}

	virtual THString GetStoreName()
	{
		return _T("磁盘文件");
	}

	virtual THString FileDialogOpenExistOne(HWND hOwner=NULL,THString sFileName=_T(""),THString sFilter=_T("所有文件(*.*)|*.*||"),THString sExt=_T(""),THString sInitDir=_T(""),THString sTitle=_T(""))
	{return THSysDialog::FileDialogOpenExistOne(hOwner,sFileName,sFilter,sExt,sInitDir,sTitle);}

	virtual THString FileDialogSaveOne(HWND hOwner=NULL,THString sFileName=_T("未命名"),THString sFilter=_T("所有文件(*.*)|*.*||"),THString sExt=_T(""),THString sInitDir=_T(""),THString sTitle=_T(""))
	{return THSysDialog::FileDialogSaveOne(hOwner,sFileName,sFilter,sExt,sInitDir,sTitle);}

	virtual THString DirDialog(HWND hWnd=NULL,THString sTitle=_T("请选择目录"))
	{return THSysDialog::DirDialog(hWnd,sTitle);}
protected:
	virtual int _OpenFile(THString sFile,int nMode=FILEMODE_READ)
	{
		UINT mode=0;
		int nOpenMode=nMode&FILEMODE_MASK;
		if (nOpenMode==FILEMODE_READ)
			mode=THFile::modeRead;
		else if (nOpenMode==FILEMODE_READWRITE)
			mode=THFile::modeReadWrite;
		else if (nOpenMode==FILEMODE_CREATEOREXIST)
			mode=THFile::modeNoTruncate|THFile::modeCreate|THFile::modeReadWrite;
		else
			return -1;
		mode|=THFile::shareDenyNone;
		THFile *file=new THFile;
		if (!file) return 0;
		if (!file->Open(GetFullPath(sFile),mode))
		{
			delete file;
			return -1;
		}
		m_nFileListCount++;
		m_FileList.SetAt(m_nFileListCount,file);
		return m_nFileListCount;
	}

	virtual void _CloseFile(int idx){m_FileList.RemoveAt(idx);}

	virtual void _CloseAllFile(){m_FileList.RemoveAll();}
	
	virtual UINT _GetPosition(int idx)
	{
		THFile *file;
		if (m_FileList.GetAt(idx,file))
			return (UINT)file->GetPosition();
		return 0;
	}

	virtual UINT _SeekFile(int idx,int nStart,int nPos)
	{
		THFile *file;
		if (m_FileList.GetAt(idx,file))
			return (UINT)file->Seek(nPos,nStart);
		return 0;
	}

	virtual UINT _ReadFile(int idx,void *buf,UINT len,int pos=-1)
	{
		THFile *file;
		if (m_FileList.GetAt(idx,file))
		{
			if (pos!=-1) file->Seek(pos,SEEK_SET);
			return file->Read(buf,len);
		}
		return 0;
	}

	virtual UINT _WriteFile(int idx,const void *buf,UINT len,int pos=-1)
	{
		THFile *file;
		if (m_FileList.GetAt(idx,file))
		{
			if (pos!=-1) file->Seek(pos,SEEK_SET);
			try{
				file->Write(buf,len);
			}
			catch(...)
			{
				len=0;
			}
		}
		return len;
	}

	THMap<int,THFile *> m_FileList;
	int m_nFileListCount;
	static void MyFreeFile(void *key,void *value,void *adddata)
	{
		delete (THFile *)value;
	}
};
*/