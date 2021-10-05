
/**
* @brief 硬盘文件储存器描述类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-09-07 新建类
*/
class THFileStore : public ITHFileStore
{
public:
	THFileStore():ITHFileStore()
	{
		m_nFileListCount=0;
		m_FileList.SetFreeProc(MyFreeFile);
	}
	virtual ~THFileStore()
	{
		m_FileList.RemoveAll();
	}
	virtual ITHFileStore *CreateObject()
	{
		return new THFileStore();
	}

	virtual BOOL IsPathDir(THString sPath)
	{
		return ::PathIsDirectory(GetFullPath(sPath));
	}

	virtual BOOL IsPathFile(THString sPath)
	{
		sPath=GetFullPath(sPath);
		//PathFileExists is true if it is a dir or file
		if (::PathFileExists(sPath))
			if (!IsPathDir(sPath))
				return TRUE;
		return FALSE;
	}

	virtual BOOL FindFiles(THString sPath,THStringArray *arDir,THStringArray *arFile)
	{
		sPath=GetFullPath(sPath);
		return THSysMisc::FindFiles(sPath,arFile,arDir,_T("*.*"),FALSE,PATHMODE_FILENAME);
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

	virtual THString IniGetString(THString sFile,THString sApp,THString sKey,THString sDefValue=_T(""))
	{
		THString buffer;
		int InitSize=4096;
		while(GetPrivateProfileString(sApp,sKey,sDefValue,buffer.GetBuffer(InitSize),InitSize,sFile)==InitSize)
		{
			//缓冲区可能不够，加大缓冲区重试
			buffer.ReleaseBuffer();
			InitSize*=2;
		}
		buffer.ReleaseBuffer();
		return buffer;
	}

	virtual BOOL IniWriteString(THString sFile,THString sApp,THString sKey,THString sValue){return WritePrivateProfileString(sApp,sKey,sValue,sFile);}

	virtual BOOL IniGetBin(THString sFile,THString sApp,THString sKey,void *pBuffer,UINT uSize){return GetPrivateProfileStruct(sApp,sKey,pBuffer,uSize,sFile);}

	virtual BOOL IniWriteBin(THString sFile,THString sApp,THString sKey,void *pBuffer,UINT uSize){return WritePrivateProfileStruct(sApp,sKey,pBuffer,uSize,sFile);}

	virtual BOOL IniEnumAllStruct(THString sFile,THStringArray *arKey)
	{
		if (!arKey) return FALSE;
		arKey->RemoveAll();
		TCHAR buf[4096];
		TCHAR *buffer=buf;
		DWORD rcnt=GetPrivateProfileSectionNames(buf,sizeof(buf),sFile);
		if (rcnt==0) return FALSE;
		THString tmp;
		while(*buffer!='\0')
		{
			tmp=buffer;
			buffer+=tmp.GetLength()+1;
			tmp.Trim();
			if (!tmp.IsEmpty())
				arKey->Add(tmp);
		}
		return TRUE;
	}

	virtual BOOL IniEnumStruct(THString sFile,const THString sKey,THStringArray *arKey,THStringArray *arValue)
	{
		if (!arKey || !arValue) return FALSE;
		arKey->RemoveAll();
		arValue->RemoveAll();
		const int maxlen=64*1024;
		TCHAR *buffer=new TCHAR[maxlen+2];
		TCHAR *sbuffer=buffer;
		if (!buffer) return FALSE;
		//microsoft bug Q198906
		GetPrivateProfileString(NULL,NULL,NULL,buffer,maxlen,sFile);
		DWORD rcnt=GetPrivateProfileSection(sKey,buffer,maxlen,sFile);
		if (rcnt==0)
		{
			delete [] sbuffer;
			return FALSE;
		}
		THString tmp;
		while(*buffer!='\0')
		{
			tmp=buffer;
			buffer+=tmp.GetLength()+1;
			if (!tmp.IsEmpty())
			{
				int pos=tmp.Find(_T("="),0);
				if (pos!=-1)
				{
					arKey->Add(tmp.Left(pos).Trim());
					arValue->Add(tmp.Mid(pos+1).Trim());
				}
			}
		}
		delete [] sbuffer;
		return TRUE;
	}

	virtual THString FileDialogOpenExistOne(HWND hOwner=NULL,THString sFileName=_T(""),THString sFilter=_T("所有文件(*.*)|*.*||"),THString sExt=_T(""),THString sInitDir=_T(""),THString sTitle=_T(""))
	{if (sInitDir.IsEmpty()) sInitDir=m_basepath;
	return THSysDialog::FileDialogOpenExistOne(hOwner,sFileName,sFilter,sExt,sInitDir,sTitle);}

	virtual THString FileDialogSaveOne(HWND hOwner=NULL,THString sFileName=_T("未命名"),THString sFilter=_T("所有文件(*.*)|*.*||"),THString sExt=_T(""),THString sInitDir=_T(""),THString sTitle=_T(""))
	{if (sInitDir.IsEmpty()) sInitDir=m_basepath;
	return THSysDialog::FileDialogSaveOne(hOwner,sFileName,sFilter,sExt,sInitDir,sTitle);}

	virtual THString DirDialog(HWND hWnd=NULL,THString sTitle=_T("请选择目录"))
	{return THSysDialog::DirDialog(hWnd,sTitle);}
	virtual THString BaseDirDialog(HWND hWnd=NULL,THString sTitle=_T("请选择基准路径"))
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
		else if (nOpenMode==FILEMODE_ALWAYSCREATE)
			mode=THFile::modeCreate|THFile::modeReadWrite;
		else
			return -1;
		if (mode&FILEMODE_MASK_SHAREEXCLUSIVE)
			mode|=THFile::shareExclusive;
		else
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
			return file->Write(buf,len);
		}
		return 0;
	}

	virtual BOOL _CreateDirectory(THString sPath)
	{
		return ::CreateDirectory(GetFullPath(sPath),NULL);
	}
	virtual BOOL _DeleteDirectory(THString sPath)
	{
		return ::RemoveDirectory(GetFullPath(sPath));
	}
	virtual BOOL _CopyFile(THString sPath,THString sNewPath,ITHFileStore *fs=NULL)
	{
		if (fs==NULL || fs==this) return ::CopyFile(GetFullPath(sPath),GetFullPath(sNewPath),FALSE);
		return ITHFileStore::_CopyFile(sPath,sNewPath,fs);
	}
	virtual BOOL _MoveFile(THString sPath,THString sNewPath,ITHFileStore *fs=NULL)
	{
		if (fs==NULL || fs==this) return ::MoveFile(GetFullPath(sPath),GetFullPath(sNewPath));
		return ITHFileStore::_MoveFile(sPath,sNewPath,fs);
	}
	virtual BOOL _DeleteFile(THString sFile)
	{
		return ::DeleteFile(GetFullPath(sFile));
	}

	THMap<int,THFile *> m_FileList;
	int m_nFileListCount;
	static void MyFreeFile(void *key,void *value,void *adddata)
	{
		delete (THFile *)value;
	}
};

