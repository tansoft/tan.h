
#include <THMembuf.h>
#include <THSyn.h>

//fixme,SetBaseDirectory 要处理支持选择exe文件后，选择里面的具体目录功能，初步设想加入标识符'|'完成
//fixme加入标准的选择基准目录内文件函数，用TreeCtrl每级响应回调函数完成

/**
* @brief Exe资源文件储存器描述类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-10-18 新建类
*/
class THResFileStore : public ITHFileStore
{
public:
	THResFileStore():ITHFileStore()
	{
		m_files.SetFreeProc(MyFreeFile);
	}
	virtual ~THResFileStore()
	{
		CloseAllFile();
	}
	virtual ITHFileStore *CreateObject()
	{
		return new THResFileStore();
	}

	virtual BOOL IsPathDir(THString sPath)
	{
		int pos=sPath.Find(_T("?"),0);
		if (pos!=-1) sPath=sPath.Left(pos);
		sPath=GetFullPath(sPath);
		sPath.MakeLower();
		if (sPath.IsEmpty()) return FALSE;
		THString sp,sf;
		if (!_GetResPathAndFile(sPath,sp,sf)) return FALSE;
		//the root path sp is empty,so only test sf will be ok
		if (sf.IsEmpty()) return TRUE;
		return FALSE;
	}

	virtual BOOL IsPathFile(THString sPath)
	{
		int pos=sPath.Find(_T("?"),0);
		if (pos!=-1) sPath=sPath.Left(pos);
		sPath=GetFullPath(sPath);
		sPath.MakeLower();
		if (sPath.IsEmpty()) return FALSE;
		THString sp,sf;
		if (!_GetResPathAndFile(sPath,sp,sf)) return FALSE;
		if (sp.IsEmpty() || sf.IsEmpty()) return FALSE;
		THStringArray ar;
		if (!FindFiles(sp,NULL,&ar)) return FALSE;
		if (sf.Left(1)==_T("#")) sf=_T("ResID_")+sf.Mid(1);
		for(int i=0;i<ar.GetSize();i++)
			if (ar[i].CompareNoCase(sf)==0) return TRUE;
		return FALSE;
	}

	virtual BOOL FindFiles(THString sPath,THStringArray *arDir,THStringArray *arFile)
	{
		int pos=sPath.Find(_T("?"),0);
		if (pos!=-1) sPath=sPath.Left(pos);
		sPath=GetFullPath(sPath);
		sPath.MakeLower();
		if (sPath.IsEmpty()) return FALSE;
		THString sp,sf;
		if (!_GetResPathAndFile(sPath,sp,sf)) return FALSE;
		HANDLE hExe=LoadLibrary(m_basepath);
		if (!hExe) return FALSE;
		if (arDir) arDir->RemoveAll();
		if (arFile) arFile->RemoveAll();
		if (sp.IsEmpty())
		{
			//Enum All Types
			if (arDir)
				EnumResourceTypes((HMODULE)hExe,(ENUMRESTYPEPROC)EnumTypesFunc,(LONG_PTR)arDir);
		}
		else
		{
			//only for sp type
			if (arFile)
				EnumResourceNames((HMODULE)hExe,sp,(ENUMRESNAMEPROC)EnumNamesFunc,(LONG_PTR)arFile);
		}
		return TRUE;
	}

	virtual BOOL GetFileAttrib(THString sFile,UINT *size,THTime *ct,THTime *mt)
	{
		if (m_basepath.IsEmpty()) return FALSE;
		THFileStatus st;
		if (!THFile::GetStatus(m_basepath,st)) return FALSE;
		if (ct) *ct=st.m_ctime;
		if (mt) *mt=st.m_mtime;
		if (size)
		{
			int idx=OpenFile(sFile);
			if (idx==-1) return FALSE;
			*size=GetFileSize(idx);
			CloseFile(idx);
		}
		return TRUE;
	}

	virtual UINT GetFileSize(int idx)
	{
		THSingleLock s(&m_mtx);
		THResFileInfo *info;
		if (!m_files.GetAt(idx,info)) return 0;
		return m_man.GetBufferedSize(info->bufidx);
	}

	virtual THString GetStoreName()
	{
		return _T("Exe资源文件");
	}

	virtual THString FileDialogOpenExistOne(HWND hOwner=NULL,THString sFileName=_T(""),THString sFilter=_T("所有文件(*.*)|*.*||"),THString sExt=_T(""),THString sInitDir=_T(""),THString sTitle=_T(""))
	{//fixme
	return THSysDialog::FileDialogOpenExistOne(hOwner,sFileName,sFilter,sExt,sInitDir,sTitle);}

	virtual THString FileDialogSaveOne(HWND hOwner=NULL,THString sFileName=_T("未命名"),THString sFilter=_T("所有文件(*.*)|*.*||"),THString sExt=_T(""),THString sInitDir=_T(""),THString sTitle=_T(""))
	{//fixme
	return THSysDialog::FileDialogSaveOne(hOwner,sFileName,sFilter,sExt,sInitDir,sTitle);}

	virtual THString DirDialog(HWND hWnd=NULL,THString sTitle=_T("请选择目录"))
	{//fixme
	return THSysDialog::DirDialog(hWnd,sTitle);}

	virtual THString BaseDirDialog(HWND hWnd=NULL,THString sTitle=_T("请选择基准Exe文件"))
	{return THSysDialog::FileDialogOpenExistOne(hWnd,_T(""),_T("可执行文件(*.exe,*.dll,*.ocx)|*.exe;*.dll;*.ocx||"),_T(""),_T(""),sTitle);}
protected:
	#define KNOWNNAME const static TCHAR knownname[][20]={_T(""),_T("RT_CURSOR"),_T("RT_BITMAP"),_T("RT_ICON"),_T("RT_MENU"),					\
														_T("RT_DIALOG"),_T("RT_STRING"),_T("RT_FONTDIR"),_T("RT_FONT"),_T("RT_ACCELERATOR"),	\
														_T("RT_RCDATA"),_T("RT_MESSAGETABLE"),_T("RT_GROUP_CURSOR"),_T(""),_T("RT_GROUP_ICON"),	\
														_T(""),_T("RT_VERSION"),_T("RT_DLGINCLUDE"),_T(""),_T("RT_PLUGPLAY"),					\
														_T("RT_VXD"),_T("RT_ANICURSOR"),_T("RT_ANIICON"),_T("RT_HTML"),_T("RT_MANIFEST")}

	static THString _GetUidByName(THString name,BOOL bCheckSysName=TRUE)
	{
		KNOWNNAME;
		THString str=name;
		if (str.Left(6).CompareNoCase(_T("ResID_"))==0)
			str=_T("#")+str.Mid(6);
		else
		{
			if (bCheckSysName && !name.IsEmpty())
			{
				for(int i=1;i<=24;i++)
				{
					if (name.CompareNoCase(knownname[i])==0)
					{
						str.Format(_T("#%u"),i);
						break;
					}
				}
			}
		}
		return str;
	}
	static THString _GetFriendlyName(USHORT type)
	{
		KNOWNNAME;
		THString str;
		if (type<=24) str=knownname[type];
		else str.Format(_T("ResID_%u"),type);
		return str;
	}
	static BOOL EnumTypesFunc(HANDLE hModule,LPTSTR lpType,LONG lParam)
	{
		THString str;
		if ((ULONG)(ULONG_PTR)lpType & 0xFFFF0000)
			//string type
			str=lpType;
		else
			str=_GetFriendlyName((USHORT)lpType);
		((THStringArray *)(LONG_PTR)lParam)->Add(str);
		return TRUE;
	}

	static BOOL EnumNamesFunc(HANDLE hModule,LPCTSTR lpType,LPTSTR lpName,LONG lParam)
	{
		THString str;
		if ((ULONG)(ULONG_PTR)lpName & 0xFFFF0000)
			//string type
			str=lpName;
		else
			str.Format(_T("ResID_%u"),(USHORT)lpName);
		((THStringArray *)(LONG_PTR)lParam)->Add(str);
		return TRUE;
	}

	BOOL _GetResPathAndFile(THString sFile,THString &sp,THString &sf)
	{
		if (sFile.GetLength()>=m_basepath.GetLength())
		{
			sp=sFile.Mid(m_basepath.GetLength());
			sp.Replace(_T("/"),_T("\\"));
			sp.Trim(_T("\\"));
			int pos=sp.Find(_T("\\"),0);
			if (pos!=-1)
			{
				sf=sp.Mid(pos);
				sp=sp.Left(pos);
				sp.Trim(_T("\\"));
				sf.Trim(_T("\\"));
			}
			sp=_GetUidByName(sp);
			sf=_GetUidByName(sf,FALSE);
			return TRUE;
		}
		return FALSE;
	}

	//file name maybe string or #id or ResID_id
	virtual int _OpenFile(THString sFile,int nMode=FILEMODE_READ)
	{
		//ingore the param
		int pos=sFile.Find(_T("?"),0);
		if (pos!=-1) sFile=sFile.Left(pos);
		if (sFile.IsEmpty()) return -1;
		sFile=GetFullPath(sFile);
		sFile.MakeLower();
		THString sp,sf;
		if (!_GetResPathAndFile(sFile,sp,sf)) return -1;
		sFile=m_basepath+_T("\\")+sp+_T("\\")+sf;
		BOOL bMustExist=FALSE;
		UINT mode=0;
		int nOpenMode=nMode&FILEMODE_MASK;
		//mode FILEMODE_READ and FILEMODE_READWRITE is not have different
		if (nOpenMode==FILEMODE_READ)
			bMustExist=TRUE;
		else if (nOpenMode==FILEMODE_READWRITE)
			bMustExist=TRUE;
		else if (nOpenMode==FILEMODE_CREATEOREXIST)
			bMustExist=FALSE;
		else if (nOpenMode==FILEMODE_ALWAYSCREATE)
			bMustExist=FALSE;
		else
			return -1;
		m_mtx.Lock();
		int edidx=-1;
		for(int i=0;i<m_arFileList.GetSize();i++)
		{
			if (m_arFileList[i].CompareNoCase(sFile)==0)
			{
				edidx=i;
				break;
			}
		}
		if (edidx!=-1)
		{
			//share exclusive
			if (mode&FILEMODE_MASK_SHAREEXCLUSIVE)
			{
				m_mtx.Unlock();
				return -1;
			}
			//有正在打开的文件，取消打开
			if (nOpenMode==FILEMODE_ALWAYSCREATE)
			{
				m_mtx.Unlock();
				return -1;
			}
		}
		if (nOpenMode==FILEMODE_ALWAYSCREATE)
		{
			edidx=-1;
			DeleteFile(sFile);
		}
		THResFileInfo *info=new THResFileInfo;
		if (!info)
		{
			m_mtx.Unlock();
			return -1;
		}
		info->pos=0;
		if (nOpenMode==FILEMODE_READ)
			info->mode=1;
		else
			info->mode=2;
		if (edidx!=-1)
		{
			//get the open bufidx
			info->bufidx=m_files[edidx]->bufidx;
			edidx=(int)m_arFileList.Add(sFile);
		}
		else
		{
			//open file
			BOOL bErr=TRUE;
			HANDLE hExe=LoadLibrary(m_basepath);
			if (hExe)
			{
				//HRSRC res=FindResourceEx((HMODULE)hExe,sp,sf,MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL));
				HRSRC res=FindResource((HMODULE)hExe,sf,sp);
				if (res)
				{
					HGLOBAL gl=::LoadResource((HMODULE)hExe,res);
					if(gl)
					{
						LPVOID lp=::LockResource(gl);
						if (lp)
						{
							DWORD size=::SizeofResource((HMODULE)hExe,res);
							if (size>0)
							{
								//add ref
								edidx=(int)m_arFileList.Add(sFile);
								info->bufidx=edidx;
								m_man.AddBuf(edidx,lp,size);
								bErr=FALSE;
							}
							//not need to UnlockResource
						}
						::FreeResource(gl);
					}
				}
				::FreeLibrary((HMODULE)hExe);
			}
			if (bErr)
			{
				if (bMustExist)
				{
					delete info;
					m_mtx.Unlock();
					return -1;
				}
				else
				{
					//add ref
					edidx=(int)m_arFileList.Add(sFile);
					info->bufidx=edidx;
					m_man.AddBuf(edidx,NULL,0);
					bErr=FALSE;
				}
			}
		}
		info->filename=sFile;
		m_files.SetAt(edidx,info);
		m_mtx.Unlock();
		return edidx;
	}

	virtual void _CloseFile(int idx)
	{
		THSingleLock s(&m_mtx);
		THResFileInfo *info;
		if (!m_files.GetAt(idx,info)) return;
		if (info->mode==3)
		{
			//save file to exe file
			UINT len;
			void *buf=m_man.GetBuf(info->bufidx,&len);
			//not need to check buf,if null to delete the resource
			//if (buf && len>0)
			{
				THString sp,sf;
				if (_GetResPathAndFile(info->filename,sp,sf))
				{
					HANDLE hUpdateRes=BeginUpdateResource(m_basepath,FALSE);
					if (hUpdateRes)
					{
						UpdateResource(hUpdateRes,sp,sf,
							MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL),
							buf,len);
						EndUpdateResource(hUpdateRes,FALSE);
					}
				}
			}
		}
		//set the m_arFileList 's info->filename to null
		m_arFileList[idx]=_T("");
		//check if not any more m_arFileList named it,clean buf
		BOOL bFound=FALSE;
		for(int i=0;i<m_arFileList.GetSize();i++)
		{
			if (m_arFileList[i].CompareNoCase(info->filename))
			{
				bFound=TRUE;
				break;
			}
		}
		if (bFound==FALSE)
			m_man.Empty(info->bufidx);
		m_files.RemoveAt(idx);
	}

	virtual void _CloseAllFile()
	{
		THSingleLock s(&m_mtx);
		m_arFileList.RemoveAll();
		m_files.RemoveAll();
		m_man.RemoveAll();
	}
	
	virtual UINT _GetPosition(int idx)
	{
		THSingleLock s(&m_mtx);
		THResFileInfo *info;
		if (m_files.GetAt(idx,info))
			return info->pos;
		return 0;
	}

	//fixme,not test
	virtual UINT _SeekFile(int idx,int nStart,int nPos)
	{
		THSingleLock s(&m_mtx);
		THResFileInfo *info;
		UINT pos=0;
		if (m_files.GetAt(idx,info))
		{
			info->pos=_GetRealSeekPos(idx,nStart,nPos);
			return info->pos;
		}
		return 0;
	}

	virtual UINT _ReadFile(int idx,void *buf,UINT len,int pos=-1)
	{
		THSingleLock s(&m_mtx);
		THResFileInfo *info;
		if (m_files.GetAt(idx,info))
		{
			UINT glen;
			U8 *gbuf=(U8 *)m_man.GetBuf(info->bufidx,&glen);
			if (gbuf && glen>0)
			{
				if (pos!=-1) _SeekFile(idx,SEEK_SET,pos);
				UINT upos=info->pos;
				if (upos+len>glen)
				{
					if (glen>upos)
						len=glen-upos;
					else
						len=0;
				}
				if (len>0)
				{
					memcpy(buf,gbuf+upos,len);
					info->pos+=len;
				}
			}
			else
				len=0;
			m_man.ReleaseGetBufLock(info->bufidx);
			return len;
		}
		return 0;
	}

	//fixme,not test
	virtual UINT _WriteFile(int idx,const void *buf,UINT len,int pos=-1)
	{
		THSingleLock s(&m_mtx);
		THResFileInfo *info;
		if (m_files.GetAt(idx,info))
		{
			if (info->mode==2 || info->mode==3)
			{
				UINT glen;
				UINT len1=0;
				U8 *gbuf=(U8 *)m_man.GetBuf(info->bufidx,&glen);
				if (gbuf && glen>0)
				{
					if (pos!=-1) _SeekFile(idx,SEEK_SET,pos);
					UINT upos=info->pos;
					if (upos+len>glen)
					{
						if (glen>upos)
							len1=glen-upos;
						else
							len1=0;
					}
					else
						len1=len;
					if (len1>0)
					{
						memcpy(gbuf+upos,buf,len1);
						info->pos+=len1;
						info->mode=3;
					}
				}
				else
					len=0;
				m_man.ReleaseGetBufLock(info->bufidx);
				if (len-len1>0)
				{
					//the append buffer
					len1=len-len1;
					m_man.AddBuf(info->bufidx,(U8 *)buf+len1,len1);
					info->pos+=len1;
					info->mode=3;
				}
				return len;
			}
		}
		return 0;
	}

	//fixme,not imp
	virtual BOOL _CreateDirectory(THString sPath){return FALSE;}
	virtual BOOL _DeleteDirectory(THString sPath){return FALSE;}

	//fixme,not test
	virtual BOOL _DeleteFile(THString sFile)
	{
		THSingleLock s(&m_mtx);
		BOOL bOk=FALSE;
		//ingore the param
		int pos=sFile.Find(_T("?"),0);
		if (pos!=-1) sFile=sFile.Left(pos);
		if (sFile.IsEmpty()) return -1;
		sFile=GetFullPath(sFile);
		sFile.MakeLower();
		THString sp,sf;
		if (_GetResPathAndFile(sFile,sp,sf))
		{
			HANDLE hUpdateRes=BeginUpdateResource(m_basepath,FALSE);
			if (hUpdateRes)
			{
				if (UpdateResource(hUpdateRes,sp,sf,
					MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL),
					NULL,0)) bOk=TRUE;
				EndUpdateResource(hUpdateRes,FALSE);
			}
		}
		if (bOk)
		{
			for(int i=0;i<m_arFileList.GetSize();i++)
			{
				if (m_arFileList[i].CompareNoCase(sFile)==0)
				{
					THResFileInfo *info;
					if (m_files.GetAt(i,info))
					{
						//clean the cache buffer
						m_man.Empty(info->bufidx);
						break;
					}
				}
			}
		}
		return bOk;
	}

	THMemBufManager<int> m_man;		//对应第一次文件名开辟对应索引的空间，但文件关闭时不进行释放，因为有可能有其他引用该内存的索引
	THStringArray m_arFileList;		//保存对应idx的文件名，用于冲突发现，文件关闭后把对应的ar置空不删除
	typedef struct _THResFileInfo{
		volatile UINT pos;
		int mode;					//Mode 1 读，2 读写，未改动；3 读写，已改动
		int bufidx;
		THString filename;			//用于修改后写入文件时确定文件名
	}THResFileInfo;
	THMap<int,THResFileInfo *> m_files;
	THMutex m_mtx;

	static void MyFreeFile(void *key,void *value,void *adddata)
	{
		delete (THResFileInfo *)value;
	}
};
