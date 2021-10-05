#pragma once

#include <THSystem.h>
#include <THSysMisc.h>
#include <THString.h>
#include <THTime.h>
#include <THCharset.h>

struct THFileStatus
{
	THTime m_ctime;			// creation date/time of file
	THTime m_mtime;			// last modification date/time of file
	THTime m_atime;			// last access date/time of file
	ULONGLONG m_size;		// logical size of file in bytes
	BYTE m_attribute;		// logical OR of CFile::Attribute enum values
	BYTE _m_padding;		// pad the structure to a WORD
	THString m_szFullName;	// absolute path name
};

/**
* @brief 文件处理基类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-08-05 新建类
*/
/**<pre>
  使用Sample：
</pre>*/
class THFile
{
public:
// Flag values
	enum OpenFlags {
		modeRead =         (int) 0x00000,
		modeWrite =        (int) 0x00001,
		modeReadWrite =    (int) 0x00002,
		shareCompat =      (int) 0x00000,
		shareExclusive =   (int) 0x00010,
		shareDenyWrite =   (int) 0x00020,
		shareDenyRead =    (int) 0x00030,
		shareDenyNone =    (int) 0x00040,
		modeNoInherit =    (int) 0x00080,
		modeCreate =       (int) 0x01000,
		modeNoTruncate =   (int) 0x02000,
		typeText =         (int) 0x04000, // typeText and typeBinary are
		typeBinary =       (int) 0x08000, // used in derived classes only
		osNoBuffer =       (int) 0x10000,
		osWriteThrough =   (int) 0x20000,
		osRandomAccess =   (int) 0x40000,
		osSequentialScan = (int) 0x80000,
		};

	enum Attribute {
		normal =    0x00,
		readOnly =  0x01,
		hidden =    0x02,
		system =    0x04,
		volume =    0x08,
		directory = 0x10,
		archive =   0x20
		};

	enum SeekPosition { begin = 0x0, current = 0x1, end = 0x2 };

// Constructors
	THFile()
	{
		m_hFile = INVALID_HANDLE_VALUE;
		m_bCloseOnDelete = FALSE;
	}
	THFile(HANDLE hFile)
	{
		m_hFile = hFile;
		m_bCloseOnDelete = FALSE;
	}
	THFile(const THString sFileName, UINT nOpenFlags)
	{
		m_hFile = INVALID_HANDLE_VALUE;
		Open(sFileName, nOpenFlags);
	}
	virtual ~THFile()
	{
		if (m_hFile != INVALID_HANDLE_VALUE && m_bCloseOnDelete) Close();
	}
// Attributes
	HANDLE m_hFile;
	operator HANDLE() const{return m_hFile;}

	inline BOOL IsOpen(){return m_hFile!=INVALID_HANDLE_VALUE;}

	virtual ULONGLONG GetPosition() const
	{
		if (m_hFile==INVALID_HANDLE_VALUE) return 0;
		LARGE_INTEGER liPos;
		liPos.QuadPart = 0;
		liPos.LowPart = ::SetFilePointer(m_hFile, liPos.LowPart, &liPos.HighPart , FILE_CURRENT);
		if (liPos.LowPart == (DWORD)-1)
			if (::GetLastError() != NO_ERROR)
				return 0;
		return liPos.QuadPart;
	}
	BOOL GetStatus(THFileStatus& rStatus) const
	{
		//memset(&rStatus, 0, sizeof(THFileStatus));//because it have class member THTime & THString
		// copy file name from cached m_strFileName
		rStatus.m_szFullName=m_strFileName;
		if (m_hFile != INVALID_HANDLE_VALUE)
		{
			// get time current file size
			FILETIME ftCreate, ftAccess, ftModify;
			if (!::GetFileTime(m_hFile, &ftCreate, &ftAccess, &ftModify)) return FALSE;
			if ((rStatus.m_size = ::GetFileSize(m_hFile, NULL)) == (DWORD)-1L) return FALSE;

			if (m_strFileName.IsEmpty())
				rStatus.m_attribute = 0;
			else
			{
				DWORD dwAttribute = ::GetFileAttributes(m_strFileName);
				// don't return an error for this because previous versions of MFC didn't
				if (dwAttribute == 0xFFFFFFFF)
					rStatus.m_attribute = 0;
				else
					rStatus.m_attribute = (BYTE) dwAttribute;
			}

			// convert times as appropriate
			rStatus.m_ctime = THTime(ftCreate);
			rStatus.m_atime = THTime(ftAccess);
			rStatus.m_mtime = THTime(ftModify);

			if (rStatus.m_ctime.GetTimeAsTime64() == 0)
				rStatus.m_ctime = rStatus.m_mtime;

			if (rStatus.m_atime.GetTimeAsTime64() == 0)
				rStatus.m_atime = rStatus.m_mtime;
		}
		return TRUE;
	}
	virtual THString GetFileName() const{return m_strFileName;}
	virtual THString GetFileNameTitle() const{return THSysMisc::RemoveFileExt(m_strFileName);}
	virtual THString GetFilePath() const{return THSysMisc::GetFilePath(m_strFileName);};
	virtual void SetFilePath(const THString sNewName){m_strFileName = sNewName;}

// Operations
	virtual BOOL Open(THString sFileName, UINT nOpenFlags)
	{
		if ((nOpenFlags & typeText) != 0) return FALSE;   // text mode not supported

		// shouldn't open an already open file (it will leak)
		if (m_hFile != INVALID_HANDLE_VALUE)
			Close();

		// THFile objects are always binary and CreateFile does not need flag
		nOpenFlags &= ~(UINT)typeBinary;
		m_bCloseOnDelete = FALSE;
		m_hFile = INVALID_HANDLE_VALUE;
		m_strFileName.Empty();

		if (sFileName.GetLength() < MAX_PATH)
			sFileName=THSysMisc::GetFullPath(sFileName);
		else
			return FALSE; // path is too long
			
		m_strFileName = sFileName;

		if (shareCompat != 0) return FALSE;

		// map read/write mode
		ASSERT((modeRead|modeWrite|modeReadWrite) == 3);
		DWORD dwAccess = 0;
		switch (nOpenFlags & 3)
		{
		case modeRead:
			dwAccess = GENERIC_READ;
			break;
		case modeWrite:
			dwAccess = GENERIC_WRITE;
			break;
		case modeReadWrite:
			dwAccess = GENERIC_READ | GENERIC_WRITE;
			break;
		default:
			return FALSE;  // invalid share mode
		}

		// map share mode
		DWORD dwShareMode = 0;
		switch (nOpenFlags & 0x70)    // map compatibility mode to exclusive
		{
		default:
			return FALSE;  // invalid share mode?
		case shareCompat:
		case shareExclusive:
			dwShareMode = 0;
			break;
		case shareDenyWrite:
			dwShareMode = FILE_SHARE_READ;
			break;
		case shareDenyRead:
			dwShareMode = FILE_SHARE_WRITE;
			break;
		case shareDenyNone:
			dwShareMode = FILE_SHARE_WRITE | FILE_SHARE_READ;
			break;
		}

		// Note: typeText and typeBinary are used in derived classes only.

		// map modeNoInherit flag
		SECURITY_ATTRIBUTES sa;
		sa.nLength = sizeof(sa);
		sa.lpSecurityDescriptor = NULL;
		sa.bInheritHandle = (nOpenFlags & modeNoInherit) == 0;

		// map creation flags
		DWORD dwCreateFlag;
		if (nOpenFlags & modeCreate)
		{
			if (nOpenFlags & modeNoTruncate)
				dwCreateFlag = OPEN_ALWAYS;
			else
				dwCreateFlag = CREATE_ALWAYS;
		}
		else
			dwCreateFlag = OPEN_EXISTING;

		// special system-level access flags

		// Random access and sequential scan should be mutually exclusive
		if ((nOpenFlags&(osRandomAccess|osSequentialScan)) == (osRandomAccess|
			osSequentialScan)) return FALSE;

		DWORD dwFlags = FILE_ATTRIBUTE_NORMAL;
		if (nOpenFlags & osNoBuffer)
			dwFlags |= FILE_FLAG_NO_BUFFERING;
		if (nOpenFlags & osWriteThrough)
			dwFlags |= FILE_FLAG_WRITE_THROUGH;
		if (nOpenFlags & osRandomAccess)
			dwFlags |= FILE_FLAG_RANDOM_ACCESS;
		if (nOpenFlags & osSequentialScan)
			dwFlags |= FILE_FLAG_SEQUENTIAL_SCAN;

		// attempt file creation
		HANDLE hFile = ::CreateFile(sFileName, dwAccess, dwShareMode, &sa,
			dwCreateFlag, dwFlags, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
			return FALSE;
		m_hFile = hFile;
		m_bCloseOnDelete = TRUE;
		return TRUE;
	}

	static void Rename(const THString sOldName,const THString sNewName){::MoveFile(sOldName,sNewName);}
	static void Remove(const THString sFileName){::DeleteFile(sFileName);}
	static UINT GetFileSize(const THString sFileName)
	{
		if (sFileName.IsEmpty()) return 0;
		if (sFileName.GetLength()>=MAX_PATH) return 0;
		WIN32_FIND_DATA findFileData;
		HANDLE hFind = FindFirstFile(sFileName, &findFileData);
		if (hFind == INVALID_HANDLE_VALUE) return 0;
		FindClose(hFind);
		if (findFileData.nFileSizeHigh != 0) return 0;
		return findFileData.nFileSizeLow;
	}
	static BOOL GetStatus(const THString sFileName,THFileStatus& rStatus)
	{
		if (sFileName.IsEmpty()) return FALSE;
		if (sFileName.GetLength()>=MAX_PATH) return FALSE;
		// attempt to fully qualify path first
		rStatus.m_szFullName=THSysMisc::GetFullPath(sFileName);
		if (rStatus.m_szFullName.IsEmpty()) return FALSE;
		WIN32_FIND_DATA findFileData;
		HANDLE hFind = FindFirstFile(sFileName, &findFileData);
		if (hFind == INVALID_HANDLE_VALUE) return FALSE;
		FindClose(hFind);

		// strip attribute of NORMAL bit, our API doesn't have a "normal" bit.
		rStatus.m_attribute = (BYTE)(findFileData.dwFileAttributes & ~FILE_ATTRIBUTE_NORMAL);

		// get just the low DWORD of the file size
		if (findFileData.nFileSizeHigh != 0) return FALSE;
		rStatus.m_size = (LONG)findFileData.nFileSizeLow;

		// convert times as appropriate
		rStatus.m_ctime = THTime(findFileData.ftCreationTime);
		rStatus.m_atime = THTime(findFileData.ftLastAccessTime);
		rStatus.m_mtime = THTime(findFileData.ftLastWriteTime);

		if (rStatus.m_ctime.GetTimeAsTime64() == 0) rStatus.m_ctime = rStatus.m_mtime;
		if (rStatus.m_atime.GetTimeAsTime64() == 0) rStatus.m_atime = rStatus.m_mtime;
		return TRUE;
	}
	static void SetStatus(const THString sFileName,THFileStatus& status)
	{
		DWORD wAttr;
		FILETIME creationTime;
		FILETIME lastAccessTime;
		FILETIME lastWriteTime;
		LPFILETIME lpCreationTime = NULL;
		LPFILETIME lpLastAccessTime = NULL;
		LPFILETIME lpLastWriteTime = NULL;

		if ((wAttr = GetFileAttributes(sFileName)) == (DWORD)-1L) return;
		if ((DWORD)status.m_attribute != wAttr && (wAttr & readOnly))
		{
			// Set file attribute, only if currently readonly.
			// This way we will be able to modify the time assuming the
			// caller changed the file from readonly.

			if (!SetFileAttributes(sFileName, (DWORD)status.m_attribute))
				return;
		}
		// last modification time
		if (status.m_mtime.GetTimeAsTime64() != 0)
		{
			status.m_mtime.GetTimeAsFileTime(&lastWriteTime);
			lpLastWriteTime = &lastWriteTime;
			// last access time
			if (status.m_atime.GetTimeAsTime64() != 0)
			{
				status.m_atime.GetTimeAsFileTime(&lastAccessTime);
				lpLastAccessTime = &lastAccessTime;
			}
			// create time
			if (status.m_ctime.GetTimeAsTime64() != 0)
			{
				status.m_ctime.GetTimeAsFileTime(&creationTime);
				lpCreationTime = &creationTime;
			}
			HANDLE hFile = ::CreateFile(sFileName, GENERIC_READ|GENERIC_WRITE,
				FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hFile == INVALID_HANDLE_VALUE) return;
			if (!SetFileTime((HANDLE)hFile, lpCreationTime, lpLastAccessTime, lpLastWriteTime)) return;
			if (!::CloseHandle(hFile)) return;
		}

		if ((DWORD)status.m_attribute != wAttr && !(wAttr & readOnly))
		{
			if (!SetFileAttributes(sFileName, (DWORD)status.m_attribute))
				return;
		}
	}

	ULONGLONG SeekToEnd(){return Seek(0,THFile::end);}
	void SeekToBegin(){Seek(0,THFile::begin);}

// Overridables
	virtual THFile* Duplicate() const
	{
		if (m_hFile == INVALID_HANDLE_VALUE) return NULL;

		THFile* pFile = new THFile(INVALID_HANDLE_VALUE);
		HANDLE hFile;
		if (!::DuplicateHandle(::GetCurrentProcess(), m_hFile,
			::GetCurrentProcess(), &hFile, 0, FALSE, DUPLICATE_SAME_ACCESS))
		{
			delete pFile;
			return NULL;
		}
		pFile->m_hFile = hFile;
		if (pFile->m_hFile == INVALID_HANDLE_VALUE)
		{
			delete pFile;
			return NULL;
		}
		pFile->m_bCloseOnDelete = m_bCloseOnDelete;
		return pFile;
	}

	virtual ULONGLONG Seek(LONGLONG lOff, UINT nFrom)
	{
		if (m_hFile==INVALID_HANDLE_VALUE) return 0;
		if (nFrom != begin && nFrom != end && nFrom != current) return 0;
		LARGE_INTEGER liOff;
		liOff.QuadPart = lOff;
		liOff.LowPart = ::SetFilePointer(m_hFile, liOff.LowPart, &liOff.HighPart,(DWORD)nFrom);
		if (liOff.LowPart  == (DWORD)-1)
			if (::GetLastError() != NO_ERROR)
				return 0;
		return liOff.QuadPart;
	}

	virtual void SetLength(ULONGLONG dwNewLen)
	{
		if(m_hFile == INVALID_HANDLE_VALUE) return;
		Seek(dwNewLen, (UINT)begin);
		::SetEndOfFile(m_hFile);
	}
	virtual ULONGLONG GetLength() const
	{
		ULARGE_INTEGER liSize;
		liSize.LowPart = ::GetFileSize(m_hFile, &liSize.HighPart);
		if (liSize.LowPart == (DWORD)-1)
			if (::GetLastError() != NO_ERROR)
				return 0;
		return liSize.QuadPart;
	}

	virtual UINT Read(void* lpBuf, UINT nCount)
	{
		if(m_hFile == INVALID_HANDLE_VALUE) return 0;
		if (nCount == 0) return 0;   // avoid Win32 "null-read"
		if (lpBuf == NULL) return 0;
		DWORD dwRead;
		if (!::ReadFile(m_hFile, lpBuf, nCount, &dwRead, NULL))
			return 0;
		return (UINT)dwRead;
	}
	virtual UINT Write(const void* lpBuf, UINT nCount)
	{
		if(m_hFile == INVALID_HANDLE_VALUE) return 0;
		if (nCount == 0) return 0;   // avoid Win32 "null-read"
		if (lpBuf == NULL) return 0;
		DWORD nWritten;
		::WriteFile(m_hFile, lpBuf, nCount, &nWritten, NULL);
		return nWritten;
		// Win95 will not return an error all the time (usually DISK_FULL)
		//if (nWritten != nCount)
		//	AfxThrowFileException(CFileException::diskFull, -1, m_strFileName);
	}

	static BOOL WriteStringA(THString sFile,THString sVal)
	{
		THFile f;
		if (!f.Open(sFile,THFile::modeWrite|THFile::modeCreate)) return FALSE;
		if (!f.WriteStringA(sVal)) return FALSE;
		f.Close();
		return TRUE;
	}

	static BOOL WriteStringW(THString sFile,THString sVal)
	{
		THFile f;
		if (!f.Open(sFile,THFile::modeWrite|THFile::modeCreate)) return FALSE;
		if (!f.WriteStringW(sVal)) return FALSE;
		f.Close();
		return TRUE;
	}

	virtual BOOL WriteStringA(THString str)
	{
		char *chtmp=THCharset::t2a(str);
		if (chtmp)
		{
			Write(chtmp,str.GetLength());
			THCharset::free(chtmp);
			return TRUE;
		}
		return FALSE;
	}

	virtual BOOL WriteStringW(THString str)
	{
		wchar_t *chtmp=THCharset::t2w(str);
		if (chtmp)
		{
			Write(chtmp,str.GetLength()*sizeof(wchar_t));
			THCharset::free(chtmp);
			return TRUE;
		}
		return FALSE;
	}

	virtual void LockRange(ULONGLONG dwPos, ULONGLONG dwCount)
	{
		if(m_hFile == INVALID_HANDLE_VALUE) return;
		ULARGE_INTEGER liPos;
		ULARGE_INTEGER liCount;
		liPos.QuadPart = dwPos;
		liCount.QuadPart = dwCount;
		::LockFile(m_hFile, liPos.LowPart, liPos.HighPart, liCount.LowPart, liCount.HighPart);
	}
	virtual void UnlockRange(ULONGLONG dwPos, ULONGLONG dwCount)
	{
		if(m_hFile == INVALID_HANDLE_VALUE) return;
		ULARGE_INTEGER liPos;
		ULARGE_INTEGER liCount;
		liPos.QuadPart = dwPos;
		liCount.QuadPart = dwCount;
		::UnlockFile(m_hFile, liPos.LowPart, liPos.HighPart, liCount.LowPart,liCount.HighPart);
	}

	virtual void Abort()
	{
		if (m_hFile != INVALID_HANDLE_VALUE)
		{
			// close but ignore errors
			::CloseHandle(m_hFile);
			m_hFile = INVALID_HANDLE_VALUE;
		}
		m_strFileName.Empty();
	}
	virtual void Flush()
	{
		if (m_hFile == INVALID_HANDLE_VALUE) return;
		::FlushFileBuffers(m_hFile);
	}
	virtual void Close()
	{
		if(m_hFile == INVALID_HANDLE_VALUE) return;

		BOOL bError = FALSE;
		if (m_hFile != INVALID_HANDLE_VALUE)
			bError = !::CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
		m_bCloseOnDelete = FALSE;
		m_strFileName.Empty();
		//if (bError)
	}

	static void * GetContentToBuffer(THString filename,unsigned int *len)
	{
		if (len) *len=0;
		char *tmp=NULL;
		THFile file;
		if (file.Open(filename,THFile::modeRead))
		{
			int size=(int)file.GetLength();
			tmp=new char[size];
			if (tmp)
			{
				if (file.Read(tmp,size)!=(UINT)size)
				{
					delete [] tmp;
					tmp=NULL;
				}
				else
				{
					if (len) *len=size;
				}
			}
			file.Close();
		}
		return (void *)tmp;
	}

	static void FreeContentBuffer(void *p)
	{
		if (p) delete [] p;
	}

	static THString GetContentToString(THString filename)
	{
		THString str;
		char *tmp=NULL;
		THFile file;
		if (file.Open(filename,THFile::modeRead))
		{
			int size=(int)file.GetLength();
			tmp=(char *)str.GetBuffer((size+1)/sizeof(TCHAR));
			if (tmp)
			{
				if (file.Read(tmp,size)==(UINT)size)
				{
					*(tmp+size)='\0';
					*(tmp+size+1)='\0';
					str.ReleaseBuffer();
				}
				else
				{
					str.ReleaseBuffer();
					str.Empty();
				}
			}
			file.Close();
		}
		return str;
	}

	static THString GetContentAToString(THString filename)
	{
		THString ret;
		THFile file;
		if (file.Open(filename,THFile::modeRead))
		{
			UINT len=(UINT)file.GetLength();
			char *buf=new char[len+2];
			if (buf)
			{
				len=file.Read(buf,len);
				buf[len]='\0';
				ret=THCharset::a2t(buf);
				delete [] buf;
			}
			file.Close();
		}
		return ret;
	}

	static BOOL GetContentToStringArray(THString filename,THStringArray *ar)
	{
		THString str=GetContentToString(filename);
		if (str.IsEmpty()) return FALSE;
		THStringArray t;
		//因为\r\n是一个整体，返回中包含空行
		str.Replace(_T("\r\n"),_T("\n"));
		t.Add(_T("\r"));
		t.Add(_T("\n"));
		THStringConv::MultiTokenToArray(str,&t,ar);
		return TRUE;
	}

	static BOOL GetContentAToStringArray(THString filename,THStringArray *ar)
	{
		THString str=GetContentAToString(filename);
		if (str.IsEmpty()) return FALSE;
		THStringArray t;
		//因为\r\n是一个整体，返回中包含空行
		str.Replace(_T("\r\n"),_T("\n"));
		t.Add(_T("\r"));
		t.Add(_T("\n"));
		THStringConv::MultiTokenToArray(str,&t,ar);
		return TRUE;
	}

	/**
	* @brief 获取文件一行数据，并删除，文件为ASCII格式
	* @param filename	指定文件
	* @param ret		指定返回的字符串
	* @param token		指定分行命令
	* @return 返回是否成功
	*/
	static BOOL GetStringAndRemoveA(THString filename,THString &ret,THString token=_T("\r\n"))
	{
		ret.Empty();
		THFile file;
		if (!file.Open(filename,THFile::modeReadWrite)) return FALSE;
		THString tmpstr;
		char buf[2];
		buf[1]='\0';
		BOOL bEndFile=FALSE;
		UINT startpos=0;
		while(1)
		{
			if (file.Read(&buf,sizeof(char))!=sizeof(char))
			{
				//end of file
				bEndFile=TRUE;
				break;
			}
			tmpstr=THCharset::a2t(buf);
			if (!tmpstr) return FALSE;
			if (token.Find(tmpstr,0)!=-1)
			{
				//end of line
				while(1)
				{
					if (file.Read(&buf,sizeof(char))!=sizeof(char))
					{
						//end of file
						bEndFile=TRUE;
						break;
					}
					tmpstr=THCharset::a2t(buf);
					if (token.Find(tmpstr,0)==-1)
					{
						//it is next data
						startpos=(UINT)file.GetPosition()-1;
						break;
					}
				}
			}
			if (startpos!=0) break;
			ret+=tmpstr;
		}
		if (bEndFile)
		{
			file.Close();
			::DeleteFile(filename);
		}
		else
		{
			//seek the new position
			file.SeekToEnd();
			UINT endpos=(UINT)file.GetPosition();
			file.Seek(startpos,SEEK_SET);
			UINT size=endpos-startpos;
			char *tmpbuf=new char[size];
			if (!tmpbuf) return FALSE;
			if (file.Read(tmpbuf,size)!=size) return FALSE;
			file.SeekToBegin();
			file.Write(tmpbuf,size);
			delete [] tmpbuf;
			file.SetLength(size);
			file.Close();
		}
		return TRUE;
	}

	/**
	* @brief 获取文件一行数据，并删除，文件为UNICODE格式
	* @param filename	指定文件
	* @param ret		指定返回的字符串
	* @param token		指定分行命令
	* @return 返回是否成功
	*/
	static BOOL GetStringAndRemoveW(THString filename,THString &ret,THString token=_T("\r\n"))
	{
		ret.Empty();
		THFile file;
		if (!file.Open(filename,THFile::modeReadWrite)) return FALSE;
		THString tmpstr;
		wchar_t buf[2];
		buf[1]='\0';
		BOOL bEndFile=FALSE;
		while(1)
		{
			if (file.Read(&buf,sizeof(wchar_t))!=sizeof(wchar_t))
			{
				//end of file
				bEndFile=TRUE;
				break;
			}
			tmpstr=THCharset::w2t(buf);
			if (!tmpstr) return FALSE;
			if (token.Find(tmpstr,0)!=-1)
			{
				//end of line
				while(1)
				{
					if (file.Read(&buf,sizeof(wchar_t))!=sizeof(wchar_t))
					{
						//end of file
						bEndFile=TRUE;
						break;
					}
					tmpstr=THCharset::w2t(buf);
					if (token.Find(tmpstr,0)==-1)
					{
						//it is next data
						break;
					}
				}
			}
			ret+=tmpstr;
		}
		if (bEndFile)
		{
			file.Close();
			::DeleteFile(filename);
		}
		else
		{
			//seek the new position
			UINT startpos=(UINT)file.GetPosition()-1;
			file.SeekToEnd();
			UINT endpos=(UINT)file.GetPosition();
			file.Seek(startpos,SEEK_SET);
			UINT size=endpos-startpos;
			char *tmpbuf=new char[size];
			if (!tmpbuf) return FALSE;
			if (file.Read(tmpbuf,size)!=size) return FALSE;
			file.SeekToBegin();
			file.Write(tmpbuf,size);
			delete [] tmpbuf;
			file.SetLength(size);
			file.Close();
		}
		return TRUE;
	}

	static BOOL SaveFile(THString filename,const void *data,int len)
	{
		THFile file;
		if (!file.Open(filename,THFile::modeCreate|THFile::modeWrite)) return FALSE;
		UINT r=file.Write(data,len);
		file.Close();
		return r==(UINT)len;
	}

	static BOOL SaveStringToFileA(THString filename,THString str,BOOL bAppend=FALSE)
	{
		THFile file;
		UINT flag;
		if (bAppend)
			flag=THFile::modeCreate|THFile::modeNoTruncate|THFile::modeWrite;
		else
			flag=THFile::modeCreate|THFile::modeWrite;
		if (!file.Open(filename,flag)) return FALSE;
		if (bAppend)
			file.SeekToEnd();
		char *ret=THCharset::t2a(str);
		if (!ret) return FALSE;
		file.Write(ret,(UINT)strlen(ret));
		file.Close();
		THCharset::free(ret);
		return TRUE;
	}
	UINT CopyPartData(THFile &other,ULONGLONG off,UINT nLen)
	{
		if (other.Seek(off,SEEK_SET)!=off) return 0;
		return CopyPartData(other,nLen);
	}
	//由对方的当前位置读取len长度，写入本文件
	UINT CopyPartData(THFile &other,UINT nLen)
	{
		UINT readed=0;
		UINT nPart;
		char buf[4096];
		while(nLen>0)
		{
			nPart=(nLen>4096)?4096:nLen;
			nPart=other.Read(buf,nPart);
			if (nPart==0) break;
			if (Write(buf,nPart)!=nPart) break;
			readed+=nPart;
			nLen-=nPart;
		}
		return readed;
	}
// Implementation
public:
	enum BufferCommand { bufferRead, bufferWrite, bufferCommit, bufferCheck };
	enum BufferFlags 
	{ 
		bufferDirect = 0x01,
		bufferBlocking = 0x02
	};
	// THFile does not support direct buffering (THMemFile does)
	virtual UINT GetBufferPtr(UINT nCommand, UINT nCount = 0,void** ppBufStart = NULL, void** ppBufMax = NULL)
	{
		//ASSERT(nCommand == bufferCheck);
		//UNUSED(nCommand);    // not used in retail build
		return 0;   // no support
	}

protected:
	BOOL m_bCloseOnDelete;
	THString m_strFileName;
};
