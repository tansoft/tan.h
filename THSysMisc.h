#pragma once

#define THSYSTEM_INCLUDE_TOOLHELP
#define THSYSTEM_INCLUDE_PSAPI
#define THSYSTEM_INCLUDE_SHELLOBJ
#define THSYSTEM_INCLUDE_STL_STRUCT
#define THSYSTEM_INCLUDE_COMMDLG
#define THSYSTEM_INCLUDE_SHELLAPI
#include <THSystem.h>
#include <THString.h>
#include <THCharset.h>
#include <THStlEx.h>

#define PATHMODE_FILENAME	0		//<<<·���������ļ���
#define PATHMODE_FULLPATH	1		//<<<·��Ϊȫ·��
#define PATHMODE_RELPATH	2		//<<<·��Ϊ���·��

typedef int (__stdcall *TIMEOUTMESSAGEBOXA)(HWND hWnd,LPCSTR sText,LPCSTR sCaption,UINT nType,WORD wLangId,DWORD dwMilliSeconds);
typedef int (__stdcall *TIMEOUTMESSAGEBOXW)(HWND hWnd,LPCWSTR sText,LPCWSTR sCaption,UINT nType,WORD wLangId,DWORD dwMilliSeconds);
#define MB_TIMEDOUT	32000

/**
* @brief ϵͳ����
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-07-01 �½���
*/
/**<pre>
  ʹ��Sample��
</pre>*/
class THSysMisc
{
public:
	static void srand()
	{
		static int rndseed=0;
		unsigned int rnd=GetTickCount();
		rnd+=(unsigned int)GetCurrentThreadId();
		rnd+=rndseed++;
		::srand(rnd);
	}
	/**
	* @brief �������뺯��
	* @param dVal			ԭ��ֵ
	* @return ���ؽ��
	*/
	static int cuttail(double val)
	{
		if (val>=0.0)
			return (int)(val+0.5);
		return (int)(val-0.5);
	}

	static THString GetComputerName()
	{
		TCHAR chTemp[MAX_COMPUTERNAME_LENGTH+1];
		chTemp[0]='\0';
		DWORD Size=MAX_COMPUTERNAME_LENGTH+1;
		::GetComputerName(chTemp,&Size);
		chTemp[MAX_COMPUTERNAME_LENGTH]='\0';
		return THString(chTemp);
	}

	static DWORD GetThreadIdFromHwnd(HWND hWnd)
	{
		return ::GetWindowThreadProcessId(hWnd,NULL);
	}

	static DWORD GetPidFromHwnd(HWND hWnd)
	{
		DWORD ret;
		::GetWindowThreadProcessId(hWnd,&ret);
		return ret;
	}

	static THString GetWindowText(HWND hWnd)
	{
		THString str;
		int len=::GetWindowTextLength(hWnd);
		::GetWindowText(hWnd,str.GetBuffer(len+1),len+1);
		str.ReleaseBuffer();
		return str;
	}

	static THString GetFileNameFromPid(DWORD pid)
	{
		THString szModName;
		HANDLE hProc=OpenProcess(PROCESS_ALL_ACCESS,FALSE,pid);
		if(hProc)
		{
			TCHAR *ch=szModName.GetBuffer(4096);
			//e.g.[System] pid=4 will fail and copy some chars into the buffer
			if (GetModuleFileNameEx(hProc,0,ch,4096)==0)
				*ch='\0';
			szModName.ReleaseBuffer();
			CloseHandle(hProc);
		}
		return szModName;
	}

	/**
	* @brief ��ȡ������������
	* @param sfile		�ļ���
	* @param spath		���ָ������ʾ�����ӦĿ¼�µĳ���
	* @return ���̸���
	*/
	static UINT GetProcessCountByName(THString sfile,THString spath=_T(""))
	{
		UINT cnt=0;
		spath.MakeLower();
		sfile.MakeLower();
		list<DWORD> prolist;
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
		if (hSnapshot)
		{
			PROCESSENTRY32 ppe;
			ppe.dwSize=sizeof(PROCESSENTRY32);
			BOOL fOk = Process32First(hSnapshot, &ppe);
			while(fOk)
			{
				CString str=ppe.szExeFile;
				str.MakeLower();
				if (str.Find(sfile)!=-1 && GetCurrentProcessId()!=ppe.th32ProcessID)
					prolist.push_back(ppe.th32ProcessID);
				fOk = Process32Next(hSnapshot,&ppe); // Remove the "[System Process]" (PID = 0)
			}
			CloseHandle(hSnapshot);
		}
		cnt=(UINT)prolist.size();
		if (!spath.IsEmpty())
		{
			list<DWORD>::iterator it=prolist.begin();
			for(;it!=prolist.end();it++)
			{
				HANDLE hProc=OpenProcess(PROCESS_ALL_ACCESS,false,*it);
				if(hProc)
				{
					TCHAR buffer[4096];
					buffer[0]='\0';
					if (GetModuleFileNameEx(hProc,0,buffer,4096))
					{
						_tcslwr(buffer);
						if (_tcsstr(buffer,spath)==NULL) cnt--;
					}
					CloseHandle(hProc);
				}
			}
		}
		return cnt;
	}

	/**
	* @brief �������̣���������
	* @param sfile		�ļ���
	* @param spath		���ָ������ʾֻ������ӦĿ¼�µĳ���
	* @return �����ļ���
	*/
	static void KillProcessByName(THString sfile,THString spath=_T(""))
	{
		spath.MakeLower();
		sfile.MakeLower();
		list<DWORD> dellist;
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
		if (hSnapshot)
		{
			PROCESSENTRY32 ppe;
			ppe.dwSize=sizeof(PROCESSENTRY32);
			BOOL fOk = Process32First(hSnapshot, &ppe);
			while(fOk)
			{
				CString str=ppe.szExeFile;
				str.MakeLower();
				if (str.Find(sfile)!=-1 && GetCurrentProcessId()!=ppe.th32ProcessID)
					dellist.push_back(ppe.th32ProcessID);
				fOk = Process32Next(hSnapshot,&ppe); // Remove the "[System Process]" (PID = 0)
			}
			CloseHandle(hSnapshot);
		}
		list<DWORD>::iterator it=dellist.begin();
		for(;it!=dellist.end();it++)
		{
			HANDLE hProc=OpenProcess(PROCESS_ALL_ACCESS,false,*it);
			if(hProc)
			{
				TCHAR buffer[4096];
				buffer[0]='\0';
				if (spath.IsEmpty() || GetModuleFileNameEx(hProc,0,buffer,4096))
				{
					_tcslwr(buffer);
					if (spath.IsEmpty() || _tcsstr(buffer,spath))
						TerminateProcess(hProc,0);
				}
				CloseHandle(hProc);
			}
		}
	}

	static THString GetFileNameFromHwnd(HWND hWnd)
	{
		THString ret;
		DWORD dwProcessId = 0;
		GetWindowThreadProcessId(hWnd, &dwProcessId);
		if(dwProcessId!=0)
		{
			HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
			if (hProcess != NULL)
			{
				TCHAR *buf=ret.GetBuffer(MAX_PATH);
				if (0 != GetModuleFileNameEx(hProcess, NULL, buf, MAX_PATH))
				{
					//ת��һ�Σ��Ѷ�·����ת��
					GetLongPathName(buf,buf,MAX_PATH);
				}
				ret.ReleaseBuffer();
				CloseHandle(hProcess);
			}
		}
		return ret;
	}

	static THString GetModuleFileName(HMODULE hModule=NULL)
	{
		THString ret;
		::GetModuleFileName(hModule,ret.GetBuffer(MAX_PATH),MAX_PATH);
		ret.ReleaseBuffer();
		return ret;
	}
 
	/**
	* @brief ��ȡ���ļ���Ϊ�����ģ������ڵ��ļ���
	* @param sFileName	�ļ���
	* @param sPreFix	����ظ������ļ��������ӵ���Ÿ�ʽ
	* @return �����ļ���
	*/
	static THString EnsureFileNotExist(THString sFileName,THString sPreFix=_T("_%u"))
	{
		if (::PathFileExists(sFileName))
		{
			THString sExt=GetFileExt(sFileName);
			THString sName=RemoveFileExt(sFileName);
			int i=1;
			do{
				sFileName.Format(_T("%s")+sPreFix+_T("%s"),sName,i++,sExt);
			}while(::PathFileExists(sFileName));
		}
		return sFileName;
	}

	static THString RenameFileExt(THString path,THString newext)
	{
		//example for a.log.txt will failed
		if (::PathRenameExtension(path.GetBuffer(MAX_PATH),newext))
		{
			path.ReleaseBuffer();
			return path;
		}
		path.ReleaseBuffer();
		int pos=path.ReverseFind('.');
		if (pos!=-1)
			path=path.Left(pos)+newext;
		return path;
	}

	static THString RemoveFileExt(THString sPathName)
	{
		::PathRemoveExtension(sPathName.GetBuffer());
		sPathName.ReleaseBuffer();
		return sPathName;
	}

	static THString GetFileExt(THString sPathName)
	{
		return ::PathFindExtension(sPathName);
	}

	static THString GetGuid()
	{
		GUID gID = GUID_NULL;
		::CoInitialize(NULL);
		CoCreateGuid(&gID);
		::CoUninitialize();
		return THStringConv::BinToHexString(&gID,16);
	}

	//���ļ�������չ��ǰ�����ı�
	static THString AddTailFileName(THString sFileName,THString sAddStr)
	{
		THString ret=GetFilePath(sFileName);
		if (!ret.IsEmpty()) ret+=_T("\\");
		ret+=RemoveFileExt(GetFileName(sFileName));
		ret+=sAddStr;
		ret+=GetFileExt(sFileName);
		return ret;
	}

	/**
	* @brief ��ʱ�Ի���������ܼ��أ����û�Ĭ�ϵ�MessageBox
	* @param hWnd			���
	* @param sText			�ı�
	* @param sCaption		����
	* @param nType			����
	* @param dwMilliSeconds	��ʱʱ��
	* @return ���ؽ��������MB_TIMEDOUTΪ��ʱ��Ӧ
	*/
	static int TimeoutMessageBox(HWND hWnd,LPCTSTR sText,LPCTSTR sCaption=NULL,UINT nType=MB_OK,DWORD dwMilliSeconds=8000)
	{
		HMODULE hMod=::LoadLibrary(_T("user32.dll"));
		BOOL bOk=FALSE;
		int ret=0;
		if (hMod)
		{
#ifdef _UNICODE
			TIMEOUTMESSAGEBOXW msgbox=(TIMEOUTMESSAGEBOXW)GetProcAddress(hMod,"MessageBoxTimeoutW");
#else
			TIMEOUTMESSAGEBOXA msgbox=(TIMEOUTMESSAGEBOXA)GetProcAddress(hMod,"MessageBoxTimeoutA");
#endif
			if (msgbox)
			{
				bOk=TRUE;
				ret=msgbox(hWnd,sText,sCaption,nType,0,dwMilliSeconds);
			}
			::FreeLibrary(hMod);
		}
		if (!bOk) ret=MessageBox(hWnd,sText,sCaption,nType);
		return ret;
	}

	/**
	* @brief ����Ŀ¼�µ������ļ�
	* @param sPath			����·�����磺c:\\data
	* @param arFile			�����ҵ����ļ�����Ϊ��
	* @param arDir			�����ҵ�����Ŀ¼����Ϊ��
	* @param sKey			���ҵ�ƥ��ʽ���磺*.*��ע�������޶���ƥ��ʽ�������Ŀ¼������ȫö��
	* @param bAllSubDir		�Ƿ����������Ŀ¼ö��
	* @param nPathMode		·����ʽ��ΪPATHMODE_FULLPATH����ȫ·����ΪPATHMODE_FILENAME�����ļ�����ΪPATHMODE_RELPATH�����뿪ʼ��Ŀ¼�����·��
	* @return ���ؽ��
	*/
	static BOOL FindFiles(THString sPath,THStringArray *arFile,THStringArray *arDir=NULL,THString sKey=_T("*.*"),BOOL bAllSubDir=FALSE,int nPathMode=PATHMODE_FULLPATH)
	{
		WIN32_FIND_DATA FindFileData;
		if (sPath.GetAt(sPath.GetLength()-1)=='\\')
			sPath=sPath.Left(sPath.GetLength()-1);
		HANDLE hFind = FindFirstFile(sPath+_T("\\")+sKey,&FindFileData);
		if (hFind == INVALID_HANDLE_VALUE) return FALSE;
		BOOL bRet=TRUE;
		while(bRet)
		{
			if (_tcscmp(FindFileData.cFileName,_T("."))!=0 &&
				_tcscmp(FindFileData.cFileName,_T(".."))!=0)
			{
				if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if (arDir) arDir->Add((nPathMode?(sPath+_T("\\")):_T(""))+FindFileData.cFileName);
					if (bAllSubDir)
						if (!FindFiles(sPath+_T("\\")+FindFileData.cFileName,arFile,arDir,sKey,bAllSubDir,nPathMode==PATHMODE_RELPATH?PATHMODE_FULLPATH:nPathMode))//���nPathModeΪ���ģʽ��������ȫ·��ģʽ���ã����Žض�
							return FALSE;
				}
				else
				{
					if (arFile) arFile->Add((nPathMode?(sPath+_T("\\")):_T(""))+FindFileData.cFileName);
				}
			}
			bRet = FindNextFile(hFind,&FindFileData);
		}
		FindClose(hFind);
		if (nPathMode==PATHMODE_RELPATH)
		{
			int len=sPath.GetLength()+1;
			//�ض�·��Ϊ���·��
			if (arFile)
			{
				for(int i=0;i<arFile->GetSize();i++)
				{
					THString str=arFile->GetAt(i);
					if (str.GetLength()>len) str=str.Mid(len);
					arFile->SetAt(i,str);
				}
			}
			if (arDir)
			{
				for(int i=0;i<arDir->GetSize();i++)
				{
					THString str=arDir->GetAt(i);
					if (str.GetLength()>len) str=str.Mid(len);
					arDir->SetAt(i,str);
				}
			}
		}
		return TRUE;
	}
	static BOOL DeleteSubDirectory(THString sPath)
	{
		BOOL ret=TRUE;
		BOOL bRet=TRUE;
		if (sPath.GetAt(sPath.GetLength()-1)=='\\') sPath=sPath.Left(sPath.GetLength()-1);
		WIN32_FIND_DATA FindFileData;
		HANDLE hFind = FindFirstFile(sPath+_T("\\*.*"),&FindFileData);
		if (hFind == INVALID_HANDLE_VALUE) return FALSE;
		while(bRet)
		{
			if (_tcscmp(FindFileData.cFileName,_T("."))!=0 &&
				_tcscmp(FindFileData.cFileName,_T(".."))!=0)
			{
				if (FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
				{
					if (!DeleteSubDirectory(sPath+_T("\\")+FindFileData.cFileName))
						ret=FALSE;
				}
				else
				{
					SetFileAttributes(sPath+_T("\\")+FindFileData.cFileName,FILE_ATTRIBUTE_NORMAL);
					if (!DeleteFile(sPath+_T("\\")+FindFileData.cFileName))
						ret=FALSE;
				}
			}
			bRet = FindNextFile(hFind,&FindFileData);
		}
		FindClose(hFind);
		if (!RemoveDirectory(sPath))
			ret=FALSE;
		return ret;
	}
	/*	BOOL ResolveShortcut(CWnd* pWnd, LPCTSTR lpszFileIn, LPTSTR lpszFileOut, int cchPath)
	{
		USES_CONVERSION;
		AFX_COM com;
		IShellLink* psl = NULL;
		*lpszFileOut = 0;   // assume failure
		
		if (!pWnd)
			return FALSE;

		SHFILEINFO info;
		if ((SHGetFileInfo(lpszFileIn, 0, &info, sizeof(info),
			SHGFI_ATTRIBUTES) == 0) || !(info.dwAttributes & SFGAO_LINK))
		{
			return FALSE;
		}

		if (FAILED(com.CreateInstance(CLSID_ShellLink, NULL, IID_IShellLink,
			(LPVOID*)&psl)) || psl == NULL)
		{
			return FALSE;
		}

		IPersistFile *ppf = NULL;
		if (SUCCEEDED(psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf)))
		{
			if (ppf != NULL && SUCCEEDED(ppf->Load(T2COLE(lpszFileIn), STGM_READ)))
			{
				/ * Resolve the link, this may post UI to find the link * /
				if (SUCCEEDED(psl->Resolve(pWnd->GetSafeHwnd(),
					SLR_ANY_MATCH)))
				{
					psl->GetPath(lpszFileOut, cchPath, NULL, 0);
					ppf->Release();
					psl->Release();
					return TRUE;
				}
			}
			if (ppf != NULL)
				ppf->Release();
		}
		psl->Release();
		return FALSE;
	}*/

	// turn a file, relative path or other into an absolute path
	static THString GetFullPath(THString sFileIn)
		// lpszPathOut = buffer of _MAX_PATH
		// lpszFileIn = file, relative path or absolute path
		// (both in ANSI character set)
	{
		// first, fully qualify the path name
		LPTSTR lpszFilePart;
		TCHAR lpszPathOut[MAX_PATH];
		DWORD dwRet = GetFullPathName(sFileIn,MAX_PATH,lpszPathOut,&lpszFilePart);
		if (dwRet == 0 || dwRet >= MAX_PATH)
			return sFileIn;

		// determine the root name of the volume
		CString strRoot=GetRoot(lpszPathOut);

		if (!::PathIsUNC(strRoot))
		{
			// get file system information for the volume
			DWORD dwFlags, dwDummy;
			if (!GetVolumeInformation(strRoot, NULL, 0, NULL, &dwDummy, &dwFlags,
				NULL, 0))
				return sFileIn;

			// not all characters have complete uppercase/lowercase
			if (!(dwFlags & FS_CASE_IS_PRESERVED))
				CharUpper(lpszPathOut);

			// assume non-UNICODE file systems, use OEM character set
			if (!(dwFlags & FS_UNICODE_STORED_ON_DISK))
			{
				WIN32_FIND_DATA data;
				HANDLE h = FindFirstFile(sFileIn, &data);
				if (h != INVALID_HANDLE_VALUE)
				{
					FindClose(h);
					if(lpszFilePart != NULL && lpszFilePart > lpszPathOut)
					{
						int nFileNameLen = lstrlen(data.cFileName);
						int nIndexOfPart = (int)(lpszFilePart - lpszPathOut);
						if ((nFileNameLen + nIndexOfPart) < _MAX_PATH)
							lstrcpy(lpszFilePart, data.cFileName);
						else
							return sFileIn; // Path doesn't fit in the buffer.
					}
					else
						return sFileIn;
				}
			}
		}
		return lpszPathOut;
	}

	static THString GetRoot(const THString sPath)
	{
		THString strRoot;
		LPTSTR lpszRoot = strRoot.GetBuffer(MAX_PATH);
		memset(lpszRoot, 0, MAX_PATH);
		lstrcpyn(lpszRoot, sPath, MAX_PATH);
		PathStripToRoot(lpszRoot);
		strRoot.ReleaseBuffer();
		return strRoot;
	}

	static BOOL IsDirectory(THString str)
	{
		return PathIsDirectory(str);
	}

	static BOOL IsFile(THString sPath)
	{
		//PathFileExists is true if it is a dir or file
		if (::PathFileExists(sPath))
			if (!IsDirectory(sPath))
				return TRUE;
		return FALSE;
	}

	static BOOL ComparePath(LPCTSTR lpszPath1, LPCTSTR lpszPath2)
	{
		// use case insensitive compare as a starter
		if (lstrcmpi(lpszPath1, lpszPath2) != 0)
			return FALSE;

		// on non-DBCS systems, we are done
		if (!GetSystemMetrics(SM_DBCSENABLED))
			return TRUE;

		// on DBCS systems, the file name may not actually be the same
		// in particular, the file system is case sensitive with respect to
		// "full width" roman characters.
		// (ie. fullwidth-R is different from fullwidth-r).
		int nLen = lstrlen(lpszPath1);
		if (nLen != lstrlen(lpszPath2))
			return FALSE;
		ASSERT(nLen < _MAX_PATH);

		// need to get both CT_CTYPE1 and CT_CTYPE3 for each filename
		LCID lcid = GetThreadLocale();
		WORD aCharType11[_MAX_PATH];
		VERIFY(GetStringTypeEx(lcid, CT_CTYPE1, lpszPath1, -1, aCharType11));
		WORD aCharType13[_MAX_PATH];
		VERIFY(GetStringTypeEx(lcid, CT_CTYPE3, lpszPath1, -1, aCharType13));
		WORD aCharType21[_MAX_PATH];
		VERIFY(GetStringTypeEx(lcid, CT_CTYPE1, lpszPath2, -1, aCharType21));
	//#ifdef _DEBUG
	//	WORD aCharType23[_MAX_PATH];
	//	VERIFY(GetStringTypeEx(lcid, CT_CTYPE3, lpszPath2, -1, aCharType23));
	//#endif

		// for every C3_FULLWIDTH character, make sure it has same C1 value
		int i = 0;
		for (LPCTSTR lpsz = lpszPath1; *lpsz != 0; lpsz = _tcsinc(lpsz))
		{
			// check for C3_FULLWIDTH characters only
			if (aCharType13[i] & C3_FULLWIDTH)
			{
	//#ifdef _DEBUG
	//			ASSERT(aCharType23[i] & C3_FULLWIDTH); // should always match!
	//#endif

				// if CT_CTYPE1 is different then file system considers these
				// file names different.
				if (aCharType11[i] != aCharType21[i])
					return FALSE;
			}
			++i; // look at next character type
		}
		return TRUE; // otherwise file name is truly the same
	}

	static THString GetFilePath(THString path)
	{
		::PathRemoveFileSpec(path.GetBuffer(MAX_PATH));
		path.ReleaseBuffer();
		return path;
	}

	static THString GetFileName(THString path)
	{
		return THString(PathFindFileName(path));
	}

	static THString GetFileNameAndRemoveExt(THString sFileName)
	{
		return RemoveFileExt(GetFileName(sFileName));
	}
	static THString GetWindowsPath()
	{
		THString ret;
		::GetWindowsDirectory(ret.GetBuffer(MAX_PATH),MAX_PATH);
		ret.ReleaseBuffer();
		return ret;
	}

	//CSIDL_DESKTOP CSIDL_STARTUP
	static THString GetSpecialFolderPath(int nFolder)
	{
		THString ret;
		::SHGetSpecialFolderPath(NULL,ret.GetBuffer(MAX_PATH),nFolder,FALSE);
		ret.ReleaseBuffer();
		return ret;
	}
	static THString GetSystemPath()
	{
		THString ret;
		::GetSystemDirectory(ret.GetBuffer(MAX_PATH),MAX_PATH);
		ret.ReleaseBuffer();
		return ret;
	}

	static THString GetTempPath(BOOL bAddSubDir=FALSE)
	{
		THString ret;
		::GetTempPath(MAX_PATH,ret.GetBuffer(MAX_PATH));
		ret.ReleaseBuffer();
		if (bAddSubDir)
		{
			srand();
			ret.AppendFormat(_T("\\%d%d%d%d"),rand(),rand(),rand(),rand());
			CreateDirectory(ret,NULL);
		}
		return ret;
	}

	static THString GenTempFileName()
	{
		THSysMisc::srand();
		THString ret;
		do
		{
			ret.Empty();
			GetTempFileName(GetTempPath(),_T("~"),rand(),ret.GetBuffer(MAX_PATH));
			ret.ReleaseBuffer();
		}while(::PathFileExists(ret)==TRUE);
		return ret;
	}

	/**
	* @brief ������̿ռ��ַ
	* @param pid			����id
	* @param size			�����С
	* @param process		���ؽ��̾��
	* @return �������뵽��ָ��
	*/
	static void *AllocProcessMemory(DWORD pid,UINT size,HANDLE *process)
	{
		*process=OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_ALL_ACCESS,true,pid);
		if (*process==NULL) return NULL;
		return VirtualAllocEx(*process,NULL,(SIZE_T)size,MEM_COMMIT,PAGE_READWRITE);
	}

	/**
	* @brief �ͷŽ��̿ռ��ַ
	* @param process		���̾��
	* @param buffer			���뵽��ָ��
	*/
	static void FreeProcessMemory(HANDLE process,void *buffer)
	{
		if (process)
		{
			if (buffer) VirtualFreeEx(process,buffer,0,MEM_RELEASE);
			CloseHandle(process);
		}
	}

	/**
	* @brief ��ȡ���ش�����Ϣ
	* @param nDriveType	��ȡ�Ĵ������ͣ�Ĭ��Ϊ���ش��̣�DRIVE_REMOVABLE Ϊ���̣�u�̣�DRIVE_REMOTE Ϊ�����̣�DRIVE_CDROM Ϊ������DRIVE_RAMDISK Ϊ�ڴ���
	* @param buffer			���뵽��ָ��
	* @return ���ش����б���,����
	*/
	static THString GetLocalDrives(UINT nDriveType=DRIVE_FIXED)
	{
		DWORD dwDrives=GetLogicalDrives();
		THString sDrive,tmp;
		for(char drive='A';drive<='Z'; ++drive)
		{
			if ((dwDrives>>(drive-'A'))&1)
			{
				tmp.Format(_T("%c:"),drive);
				if (::GetDriveType(tmp)==nDriveType)
					sDrive+=tmp+_T(",");
			}
		}
		return sDrive;
	}
};

#if (_WIN32_IE >= 0x0500)
	#define DIRDIALOG_FLAG_NORMALFS		BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE
	#define DIRDIALOG_FLAG_COMPUTERS	BIF_BROWSEFORCOMPUTER|BIF_NEWDIALOGSTYLE
	#define DIRDIALOG_FLAG_PRINTERS		BIF_BROWSEFORPRINTER|BIF_NEWDIALOGSTYLE
#else
	#define DIRDIALOG_FLAG_NORMALFS		BIF_RETURNONLYFSDIRS
	#define DIRDIALOG_FLAG_COMPUTERS	BIF_BROWSEFORCOMPUTER
	#define DIRDIALOG_FLAG_PRINTERS		BIF_BROWSEFORPRINTER
#endif


/**
* @brief ϵͳ�Ի����װ��
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-08-16 �½���
*/
/**<pre>
  ʹ��Sample��
</pre>*/
class THSysDialog
{
public:
	/**
	* @brief ʹ�ô��ļ��Ի���ѡ��һ�����ڵ��ļ�
	* @param hOwner			�����ھ��
	* @param sFileName		Ĭ�ϵ��ļ���
	* @param sFilter		�ļ���������������д����Text File|*.txt;*.log|All File|*.*||
	* @param sExt			Ĭ�Ϻ�׺��
	* @param sInitDir		��ʼ·��
	* @param sTitle			�Ի������
	* @return ����ѡ�е��ļ��������ȡ�����ߴ��󣬷��ؿ�
	*/
	static THString FileDialogOpenExistOne(HWND hOwner=NULL,THString sFileName=_T(""),THString sFilter=_T("�����ļ�(*.*)|*.*||"),THString sExt=_T(""),THString sInitDir=_T(""),THString sTitle=_T(""))
	{return FileDialog(FALSE,hOwner,sFilter,sFileName,sInitDir,sTitle,sExt,OFN_EXPLORER|OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY|OFN_LONGNAMES);}

	/**
	* @brief ʹ�ô��ļ��Ի���ѡ�������ڵ��ļ�
	* @param hOwner			�����ھ��
	* @param sFilter		�ļ���������������д����Text File|*.txt;*.log|All File|*.*||
	* @param sFileName		Ĭ�ϵ��ļ���
	* @param sInitDir		��ʼ·��
	* @param sTitle			�Ի������
	* @param sExt			Ĭ�Ϻ�׺��
	* @return ����ѡ�е��ļ��������ȡ�����ߴ��󣬷��ؿգ�������ڶ���ļ����������ļ�������'|'����
	*/
	static THString FileDialogOpenExistMulti(HWND hOwner=NULL,THString sFileName=_T(""),THString sFilter=_T("�����ļ�(*.*)|*.*||"),THString sExt=_T(""),THString sInitDir=_T(""),THString sTitle=_T(""))
	{return FileDialog(FALSE,hOwner,sFilter,sFileName,sInitDir,sTitle,sExt,OFN_EXPLORER|OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY|OFN_LONGNAMES|OFN_ALLOWMULTISELECT);}

	/**
	* @brief ʹ�ñ����ļ��Ի���ѡ��һ���ļ�
	* @param hOwner			�����ھ��
	* @param sFilter		�ļ���������������д����Text File|*.txt;*.log|All File|*.*||
	* @param sFileName		Ĭ�ϵ��ļ���
	* @param sInitDir		��ʼ·��
	* @param sTitle			�Ի������
	* @param sExt			Ĭ�Ϻ�׺��
	* @return ����ѡ�е��ļ��������ȡ�����ߴ��󣬷��ؿ�
	*/
	static THString FileDialogSaveOne(HWND hOwner=NULL,THString sFileName=_T("δ����"),THString sFilter=_T("�����ļ�(*.*)|*.*||"),THString sExt=_T(""),THString sInitDir=_T(""),THString sTitle=_T(""))
	{return FileDialog(TRUE,hOwner,sFilter,sFileName,sInitDir,sTitle,sExt,OFN_PATHMUSTEXIST|OFN_EXPLORER|OFN_HIDEREADONLY|OFN_LONGNAMES);}

	/**
	* @brief ʹ�ñ����ļ��Ի���ѡ��һ���ļ���������ʾ
	* @param hOwner			�����ھ��
	* @param sFilter		�ļ���������������д����Text File|*.txt;*.log|All File|*.*||
	* @param sFileName		Ĭ�ϵ��ļ���
	* @param sInitDir		��ʼ·��
	* @param sTitle			�Ի������
	* @param sExt			Ĭ�Ϻ�׺��
	* @return ����ѡ�е��ļ��������ȡ�����ߴ��󣬷��ؿ�
	*/
	static THString FileDialogSaveOneOverWritePrompt(HWND hOwner=NULL,THString sFileName=_T("δ����"),THString sFilter=_T("�����ļ�(*.*)|*.*||"),THString sExt=_T(""),THString sInitDir=_T(""),THString sTitle=_T(""))
	{return FileDialog(TRUE,hOwner,sFilter,sFileName,sInitDir,sTitle,sExt,OFN_PATHMUSTEXIST|OFN_EXPLORER|OFN_HIDEREADONLY|OFN_LONGNAMES|OFN_OVERWRITEPROMPT);}

	/**
	* @brief ʹ�ñ����ļ��Ի���ѡ��һ���ļ����½��ļ���ʾ
	* @param hOwner			�����ھ��
	* @param sFilter		�ļ���������������д����Text File|*.txt;*.log|All File|*.*||
	* @param sFileName		Ĭ�ϵ��ļ���
	* @param sInitDir		��ʼ·��
	* @param sTitle			�Ի������
	* @param sExt			Ĭ�Ϻ�׺��
	* @return ����ѡ�е��ļ��������ȡ�����ߴ��󣬷��ؿ�
	*/
	static THString FileDialogSaveOneCreatePrompt(HWND hOwner=NULL,THString sFileName=_T("δ����"),THString sFilter=_T("�����ļ�(*.*)|*.*||"),THString sExt=_T(""),THString sInitDir=_T(""),THString sTitle=_T(""))
	{return FileDialog(TRUE,hOwner,sFilter,sFileName,sInitDir,sTitle,sExt,OFN_PATHMUSTEXIST|OFN_EXPLORER|OFN_HIDEREADONLY|OFN_LONGNAMES|OFN_CREATEPROMPT);}

	/**
	* @brief ʹ�ô��ļ��Ի���ѡ���ļ�
	* @param bSave			�Ƿ�Ϊ����Ի���
	* @param hOwner			�����ھ��
	* @param sFilter		�ļ���������������д����Text File|*.txt;*.log|All File|*.*||
	* @param sFileName		Ĭ�ϵ��ļ���
	* @param sInitDir		��ʼ·��
	* @param sTitle			�Ի������
	* @param sExt			Ĭ�Ϻ�׺��
	* @param nFlags			�����ԣ�OFN_XXX
	* @return ����ѡ�е��ļ��������ȡ�����ߴ��󣬷��ؿգ�������ڶ���ļ����������ļ�������'|'����
	*/
	static THString FileDialog(BOOL bSave,HWND hOwner,THString sFilter,THString sFileName,THString sInitDir,THString sTitle,THString sExt,int nFlags)
	{
		TCHAR *pszFilter=NULL;
		TCHAR szFile[40960];
		TCHAR szFileTitle[MAX_PATH];
		OPENFILENAME ofn;
		BOOL bRet;
		THString ret;
		memset(&ofn,0,sizeof(OPENFILENAME));
		ofn.lStructSize=sizeof(OPENFILENAME);
		ofn.hwndOwner=hOwner;
		if (sFilter.IsEmpty())
			ofn.lpstrFilter=NULL;	//ע�⣬Ϊnullʱ��lnk�ļ�����lnk�ļ���ѡ�У�����ָ���Ŀ��
		else
		{
			pszFilter=new TCHAR[sFilter.GetLength()+1];
			_tcscpy(pszFilter,sFilter);
			TCHAR *tmp=pszFilter;
			while(*tmp!='\0')
			{
				if (*tmp=='|')
					*tmp='\0';
				tmp++;
			}
			ofn.lpstrFilter=pszFilter;
		}
		_tcscpy(szFile,sFileName);
		ofn.lpstrFile=szFile;
		ofn.nMaxFile=40960;
		ofn.lpstrFileTitle=szFileTitle;
		ofn.nMaxFileTitle=MAX_PATH;
		if (sInitDir.IsEmpty())
			ofn.lpstrInitialDir=NULL;
		else
			ofn.lpstrInitialDir=sInitDir;
		if (sTitle.IsEmpty())
			ofn.lpstrTitle=NULL;
		else
			ofn.lpstrTitle=sTitle;
		ofn.Flags=nFlags;
		if (sExt.IsEmpty())
			ofn.lpstrDefExt=NULL;
		else
		{
			sExt.Replace(_T("."),_T(""));
			ofn.lpstrDefExt=sExt;
		}
		if (bSave)
			bRet=GetSaveFileName(&ofn);
		else
			bRet=GetOpenFileName(&ofn);
		if (bRet)
		{
			if (nFlags&OFN_ALLOWMULTISELECT)
			{
				LPCTSTR pf=ofn.lpstrFile;
				THString first=pf;
				pf+=_tcslen(pf)+1;
				while(*pf!='\0')
				{
					THString next=pf;
					pf+=_tcslen(pf)+1;
					ret+=first+_T("\\")+next+_T("|");
				}
				if (ret.IsEmpty()) ret=first;
			}
			else
				ret=ofn.lpstrFile;
		}
		if (pszFilter) delete [] pszFilter;
		return ret;
	}

	/**
	* @brief ʹ�ô�Ŀ¼�Ի���ѡ��Ŀ¼
	* @param hWnd			�����ھ��
	* @param sTitle			�Ի������
	* @param nFlags			����
	* @return ����ѡ�е�Ŀ¼�����ȡ�����ߴ��󣬷��ؿ�
	*/
	static THString DirDialog(HWND hWnd=NULL,THString sTitle=_T("��ѡ��Ŀ¼"),UINT nFlags=DIRDIALOG_FLAG_NORMALFS)
	{
		//for new style
		::CoInitialize(NULL);
		THString str;
		BROWSEINFO bi;
		TCHAR szPathName[MAX_PATH],szDisName[MAX_PATH];
		ZeroMemory(&bi,sizeof(BROWSEINFO));
		bi.hwndOwner = hWnd;
		bi.pszDisplayName = szDisName;
		bi.lpszTitle = sTitle;
		bi.ulFlags = nFlags;
		LPITEMIDLIST idl = SHBrowseForFolder(&bi);
		if(idl != NULL)
		{
			if (SHGetPathFromIDList(idl,szPathName))
			{
				str=szPathName;
				if (str.GetAt(str.GetLength()-1)=='\\') str=str.Left(str.GetLength()-1);
			}
			CoTaskMemFree(idl);
		}
		::CoUninitialize();
		return str;
	}
};

// This macro returns TRUE if a number is between two others
#ifndef chINRANGE
#define chINRANGE(low,Num,High) (((low)<=(Num))&&((Num)<=(High)))
#endif

/**
* @brief ϵͳ������
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-08-16 �½���
*/
/**<pre>
  ʹ��Sample��
</pre>*/
class THToolHelp
{
public:
	THToolHelp(DWORD dwFlags = 0, DWORD dwProcessID = 0)
	{
		m_hSnapshot = INVALID_HANDLE_VALUE;
		CreateSnapshot(dwFlags, dwProcessID);
	}
	virtual ~THToolHelp()
	{
		if (m_hSnapshot != INVALID_HANDLE_VALUE) CloseHandle(m_hSnapshot);
	}

	static DWORD GetProcessIdByName(THString sName)
	{
		if (sName.IsEmpty()) return 0;
		PROCESSENTRY32 pe32;
		THToolHelp toolhelp;
		pe32.dwSize=sizeof(PROCESSENTRY32);
		if (!toolhelp.CreateSnapshot()) return 0;
		for (BOOL fOk = toolhelp.ProcessFirst(&pe32); fOk; fOk = toolhelp.ProcessNext(&pe32))
		{
			if (sName.CompareNoCase(pe32.szExeFile)==0)
				return pe32.th32ProcessID;
		}
		return 0;
	}

	static DWORD GetMainThreadIdInProcess(DWORD ProcessId)
	{
		if (ProcessId==0) return 0;
		THREADENTRY32 pe32;
		THToolHelp toolhelp;
		pe32.dwSize=sizeof(THREADENTRY32);
		if (!toolhelp.CreateSnapshot(TH32CS_SNAPTHREAD)) return 0;
		for (BOOL fOk = toolhelp.ThreadFirst(&pe32); fOk; fOk = toolhelp.ThreadNext(&pe32))
		{
			if (pe32.th32OwnerProcessID==ProcessId)
				return pe32.th32ThreadID;
		}
		return 0;
	}

	BOOL CreateSnapshot(DWORD dwFlags=TH32CS_SNAPPROCESS,DWORD dwProcessID = 0)
	{
		if (m_hSnapshot != INVALID_HANDLE_VALUE) CloseHandle(m_hSnapshot);
		if (dwFlags == 0)
			m_hSnapshot = INVALID_HANDLE_VALUE;
		else
			m_hSnapshot = CreateToolhelp32Snapshot(dwFlags,dwProcessID);
		return(m_hSnapshot != INVALID_HANDLE_VALUE);
	}

	//ppe.dwSize=sizeof(PROCESSENTRY32);
	BOOL ProcessFirst(PPROCESSENTRY32 ppe) const
	{
		BOOL fOk = Process32First(m_hSnapshot, ppe);
		if (fOk && (ppe->th32ProcessID == 0))
			fOk = ProcessNext(ppe); // Remove the "[System Process]" (PID = 0)
		return(fOk);
	}
	//ppe.dwSize=sizeof(PROCESSENTRY32);
	BOOL ProcessNext(PPROCESSENTRY32 ppe) const
	{
		BOOL fOk = Process32Next(m_hSnapshot, ppe);
		if (fOk && (ppe->th32ProcessID == 0))
			fOk = ProcessNext(ppe); // Remove the "[System Process]" (PID = 0)
		return(fOk);
	}
	//ppe.dwSize=sizeof(PROCESSENTRY32);
	BOOL ProcessFind(DWORD dwProcessId, PPROCESSENTRY32 ppe) const
	{
		BOOL fFound = FALSE;
		for (BOOL fOk = ProcessFirst(ppe); fOk; fOk = ProcessNext(ppe))
		{
			fFound = (ppe->th32ProcessID == dwProcessId);
			if (fFound) break;
		}
		return(fFound);
	}

	//pme.dwSize=sizeof(MODULEENTRY32);
	BOOL ModuleFirst(PMODULEENTRY32 pme) const
	{return(Module32First(m_hSnapshot, pme));}
	//pme.dwSize=sizeof(MODULEENTRY32);
	BOOL ModuleNext(PMODULEENTRY32 pme) const
	{return(Module32Next(m_hSnapshot, pme));}
	//pme.dwSize=sizeof(MODULEENTRY32);
	BOOL ModuleFind(PVOID pvBaseAddr, PMODULEENTRY32 pme) const
	{
		BOOL fFound = FALSE;
		for (BOOL fOk = ModuleFirst(pme); fOk; fOk = ModuleNext(pme))
		{
			fFound = (pme->modBaseAddr == pvBaseAddr);
			if (fFound) break;
		}
		return(fFound);
	}
	//pme.dwSize=sizeof(MODULEENTRY32);
	BOOL ModuleFind(PTSTR pszModName, PMODULEENTRY32 pme) const
	{
		BOOL fFound = FALSE;
		for (BOOL fOk = ModuleFirst(pme); fOk; fOk = ModuleNext(pme))
		{
			fFound = (lstrcmpi(pme->szModule,  pszModName) == 0) || (lstrcmpi(pme->szExePath, pszModName) == 0);
			if (fFound) break;
		}
		return(fFound);
	}

	//pte.dwSize=sizeof(THREADENTRY32);
	BOOL ThreadFirst(PTHREADENTRY32 pte) const
	{return(Thread32First(m_hSnapshot, pte));}
	//pte.dwSize=sizeof(THREADENTRY32);
	BOOL ThreadNext(PTHREADENTRY32 pte) const
	{return(Thread32Next(m_hSnapshot, pte));}

	//phl.dwSize=sizeof(HEAPLIST32);
	BOOL HeapListFirst(PHEAPLIST32 phl) const
	{return(Heap32ListFirst(m_hSnapshot, phl));}
	//phl.dwSize=sizeof(HEAPLIST32);
	BOOL HeapListNext(PHEAPLIST32 phl) const
	{return(Heap32ListNext(m_hSnapshot, phl));}
	int HowManyHeaps() const
	{
		int nHowManyHeaps = 0;
		HEAPLIST32 hl = { sizeof(hl) };
		for (BOOL fOk = HeapListFirst(&hl); fOk; fOk = HeapListNext(&hl)) nHowManyHeaps++;
		return(nHowManyHeaps);
	}

	// Note: The heap block functions do not reference a snapshot and
	// just walk the process's heap from the beginning each time. Infinite 
	// loops can occur if the target process changes its heap while the
	// functions below are enumerating the blocks in the heap.
	//phe.dwSize=sizeof(HEAPENTRY32);
	BOOL HeapFirst(PHEAPENTRY32 phe, DWORD dwProcessID, UINT_PTR dwHeapID) const
	{return(Heap32First(phe, dwProcessID, dwHeapID));}
	//phe.dwSize=sizeof(HEAPENTRY32);
	BOOL HeapNext(PHEAPENTRY32 phe) const
	{return(Heap32Next(phe));}
	int HowManyBlocksInHeap(DWORD dwProcessID, DWORD dwHeapID) const
	{
		int nHowManyBlocksInHeap = 0;
		HEAPENTRY32 he = { sizeof(he) };
		BOOL fOk = HeapFirst(&he, dwProcessID, dwHeapID);
		for (; fOk; fOk = HeapNext(&he)) nHowManyBlocksInHeap++;
		return(nHowManyBlocksInHeap);
	}
	BOOL IsAHeap(HANDLE hProcess, PVOID pvBlock, PDWORD pdwFlags) const
	{
		HEAPLIST32 hl = { sizeof(hl) };
		for (BOOL fOkHL = HeapListFirst(&hl); fOkHL; fOkHL = HeapListNext(&hl))
		{
			HEAPENTRY32 he = { sizeof(he) };
			BOOL fOkHE = HeapFirst(&he, hl.th32ProcessID, hl.th32HeapID);
			for (; fOkHE; fOkHE = HeapNext(&he))
			{
				MEMORY_BASIC_INFORMATION mbi;
				VirtualQueryEx(hProcess, (PVOID) he.dwAddress, &mbi, sizeof(mbi));
				if (chINRANGE(mbi.AllocationBase, pvBlock, (PBYTE) mbi.AllocationBase + mbi.RegionSize))
				{
					*pdwFlags = hl.dwFlags;
					return(TRUE);
				}
			}
		}
		return(FALSE);
	}
public:
	//ʹ������Բ鿴������̵������Ϣ
	static BOOL EnableDebugPrivilege(BOOL fEnable = TRUE)
	{
		// Enabling the debug privilege allows the application to see
		// information about service applications
		BOOL fOk = FALSE;    // Assume function fails
		HANDLE hToken;

		// Try to open this process's access token
		if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
		{
			// Attempt to modify the "Debug" privilege
			TOKEN_PRIVILEGES tp;
			tp.PrivilegeCount = 1;
			LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tp.Privileges[0].Luid);
			tp.Privileges[0].Attributes = fEnable ? SE_PRIVILEGE_ENABLED : 0;
			AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);
			fOk = (GetLastError() == ERROR_SUCCESS);
			CloseHandle(hToken);
		}
		return(fOk);
	}

	static BOOL ReadProcessMemory(DWORD dwProcessID, LPCVOID pvBaseAddress, PVOID pvBuffer, DWORD cbRead, PDWORD pdwNumberOfBytesRead = NULL)
	{return(Toolhelp32ReadProcessMemory(dwProcessID, pvBaseAddress, pvBuffer, cbRead, pdwNumberOfBytesRead));}
private:
	HANDLE m_hSnapshot;
};

/**
* @brief �Ի���ö����
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-08-16 �½���
*/
/**<pre>
  ע�⣬ö�ٵĶԻ����Ƕ���Ի���
</pre>*/
class THEnumWindow
{
public:
	THEnumWindow()
	{
	}
	virtual ~THEnumWindow()
	{
	}
	void Snapshot()
	{
		m_hwndset.clear();
		EnumWindows(EnumWindowsProc,(LPARAM)this);
	}
	HWND FindWindowByProcessId(DWORD processid,HWND hLast)
	{
		set<HWND>::iterator it;
		if (hLast)
		{
			it=m_hwndset.find(hLast);
			if (it==m_hwndset.end())
				it=m_hwndset.begin();
			else
				it++;
		}
		else
			it=m_hwndset.begin();
		for(;it!=m_hwndset.end();it++)
		{
			if (THSysMisc::GetPidFromHwnd(*it)==processid)
				return *it;
		}
		return NULL;
	}
	/**
	* @brief ����ָ�������У��Ƿ���ָ������ĶԻ���
	* @param processid		����id
	* @param title			�Ի������
	* @param bTitleInclude	ΪFALSE��ʾ�������һ����ΪTRUE��ʾ�����к��м��ɣ���TRUEģʽʱ������ж����Ҫƥ������;�ָ�
	* @return ���ضԻ�����
	*/
	HWND CheckProcessHaveDialog(DWORD processid,THString title,BOOL bTitleInclude=TRUE)
	{
		HWND h=NULL;
		THString text;
		do{
			h=FindWindowByProcessId(processid,h);
			if (h)
			{
				text=THSysMisc::GetWindowText(h);
				if (bTitleInclude)
				{
					THStringToken t(title,_T(";"));
					while(t.IsMoreTokens())
					{
						THString s=t.GetNextToken();
						if (!s.IsEmpty() && text.Find(s)!=-1) return h;
					}
				}
				else
				{
					if (text==title) return h;
				}
			}
		}while(h);
		return NULL;
	}
	const set<HWND> &GetAllHwnd(){return m_hwndset;}
protected:
	set<HWND> m_hwndset;
	static BOOL CALLBACK EnumWindowsProc(HWND hwnd,LPARAM lParam)
	{
		THEnumWindow *w=(THEnumWindow *)lParam;
		w->m_hwndset.insert(set<HWND>::value_type(hwnd));
		return TRUE;
	}
};

#define THCMDLINERULE_MASK_NORMAL	0		///<��ͨģʽ
#define THCMDLINERULE_MASK_NUMBER	0x1		///<����
#define THCMDLINERULE_MASK_NOTNEED	0x2		///<����Ҫ���Ӳ���
#define THCMDLINERULE_MASK_ALL		0x4		///<����Ĳ���ȫ����Ϊֵʹ��
#define THCMDLINERULE_MASK_ICASE	0x8		///<�����ִ�Сд
#define THCMDLINERULE_MASK_NOTNULL	0x10	///<������Ϊ��

/**
* @brief �����в���������
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2011-02-11 �½���
*/
class THCommandLineParser
{
public:
	THCommandLineParser() {_Init();}
	virtual ~THCommandLineParser()
	{
	}
	THString GetExePath(){if (m_sCmdLines.GetSize()>0) return m_sCmdLines[0];return _T("");}
	//AddParseRule(_T("p"),THCMDLINERULE_MASK_NUMBER|THCMDLINERULE_MASK_ICASE,_T("80"));	=	-p 88 or -P 88 or /p 88
	//AddParseRule(_T("h"),THCMDLINERULE_MASK_NORMAL,_T("test.abc.com"));	=	-h www.test.com
	//AddParseRule(_T("d"));												=	-d
	//AddParseRule(_T("m"),THCMDLINERULE_MASK_ALL,_T("abc def ghj"));		=	-m abc def ghj klo
	void AddParseRule(THString sParamKey,int nMode=THCMDLINERULE_MASK_NOTNEED,THString sDefValue=_T(""))
	{
		if (nMode&THCMDLINERULE_MASK_ICASE) sParamKey.MakeLower();
		m_params.erase(sParamKey);
		ParserInfo info;
		info.nMode=nMode;
		info.sDefValue=info.sCurValue=sDefValue;
		m_params.insert(map<THString,ParserInfo>::value_type(sParamKey,info));
	}
	void EmptyAllParseRule() {m_params.clear();}
	THString GetParamValue(THString sParamKey)
	{
		map<THString,ParserInfo>::iterator it=m_params.find(sParamKey);
		if (it==m_params.end()) return _T("");
		return it->second.sCurValue;
	}
	BOOL Parse()
	{
		if (m_sCmdLines.GetSize()==0) return FALSE;
		map<THString,ParserInfo>::iterator it;
		for(int i=1;i<m_sCmdLines.GetSize();i++)
		{
			TCHAR ch=m_sCmdLines[i].GetAt(0);
			if (ch=='-' || ch=='/')
			{
				THString key1=m_sCmdLines[i].Mid(1);
				THString key2=key1;
				key2.MakeLower();
				it=m_params.find(key1);
				if (it==m_params.end()) it=m_params.find(key2);
				if (it!=m_params.end())
				{
					ParserInfo &info=it->second;
					if (info.nMode&THCMDLINERULE_MASK_ICASE || key1==it->first)
					{
						if (info.nMode&THCMDLINERULE_MASK_NOTNEED)
							info.sCurValue=_T("1");
						else
						{
							i++;
							if (i>=m_sCmdLines.GetSize()) return FALSE;
							info.sCurValue.Empty();
							for(;i<m_sCmdLines.GetSize();i++)
							{
								if (info.nMode&THCMDLINERULE_MASK_NUMBER)
									if (!THStringConv::IsStringNumberAndOperator(m_sCmdLines[i])) return FALSE;
								if (info.nMode&THCMDLINERULE_MASK_NOTNULL)
									if (m_sCmdLines[i].IsEmpty()) return FALSE;
								if (!info.sCurValue.IsEmpty()) info.sCurValue+=_T(" ");
								info.sCurValue+=m_sCmdLines[i];
								if (!(info.nMode&THCMDLINERULE_MASK_ALL)) break;
							}
						}
					}
				}
			}
		}
		return TRUE;
	}
protected:
	typedef struct _ParserInfo{
		int nMode;
		THString sDefValue;
		THString sCurValue;
	}ParserInfo;
	void _Init()
	{
		LPWSTR *szArglist;
		int nArgs;
		szArglist = CommandLineToArgvW(GetCommandLineW(),&nArgs);
		if(szArglist)
		{
			for(int i=0; i<nArgs; i++)
				m_sCmdLines.Add(THCharset::w2t(szArglist[i]));
			LocalFree(szArglist);
		}
	}
	THStringArray m_sCmdLines;
	map<THString,ParserInfo> m_params;
};

/**
* @brief Զ�̷��ʲ�����
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2013-04-11 �½���
*/
/**<pre>
</pre>*/
class THRemoteAccessManager
{
public:
	THRemoteAccessManager(){m_hProcess=NULL;}
	virtual ~THRemoteAccessManager(){Close();}

	BOOL OpenRemoteProcess(DWORD pid){
		Close();
		m_hProcess=OpenProcess(PROCESS_ALL_ACCESS,FALSE,pid);
		return m_hProcess!=NULL;
	}
	void *AllocBuffer(unsigned int size){
		if (!m_hProcess) return NULL;
		return VirtualAllocEx(m_hProcess, NULL, size, MEM_COMMIT, PAGE_READWRITE);
	}
	BOOL CommitBuffer(const void *localbuf,void *remotebuf,unsigned int size){
		if (!m_hProcess) return FALSE;
		return WriteProcessMemory(m_hProcess,remotebuf,localbuf,size,NULL);
	}
	BOOL UpdateBuffer(const void *remotebuf,void *localbuf,unsigned int size){
		if (!m_hProcess) return FALSE;
		return ReadProcessMemory(m_hProcess,remotebuf,localbuf,size,NULL);
	}
	BOOL FreeBuffer(void *buf){
		if (!m_hProcess) return FALSE;
		return VirtualFreeEx(m_hProcess, buf, 0, MEM_RELEASE);
	}
	void Close(){
		if (m_hProcess) {
			CloseHandle(m_hProcess);
			m_hProcess=NULL;
		}
	}
	HANDLE GetProcessHandle(){return m_hProcess;}
private:
	HANDLE m_hProcess;
};
