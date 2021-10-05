#pragma once

#include <THStruct.h>
#include <THBitParser.h>

/**
* @brief �ļ�����������
*/
#define THFILERESULT_SUCCESS			0		///<�ɹ�
#define THFILERESULT_NOMOREDATA		1		///<û���㹻����
#define THFILERESULT_FILEERROR		2		///<�ļ�����

#define FLAGSIZE				1024	///<�ļ���Ƭ�Ĵ�С

#define BUFFERNEEDREQUERY		(5*1000)	///<��������buffer��ʱʱ��
#define MAXTRANSFERUNIT			64		///<ÿ�������������Ե����ƫ����

#define THFileResult_UpdateBitmap	1
#define THFileResult_WritingData	2
#define THFileResult_ReadingData	3
#define THFileResult_FailData		4

/**
* @brief �ļ������ص���
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-02-29 �½���
*/
class ITHFileResultCb
{
public:
	virtual BOOL OnResult(UINT ResultType,UINT p1,UINT p2)=0;
};

/**
* @brief �����ļ���
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-02-21 �½���
*/
/**<pre>
 ÿ���ļ���Ĵ����࣬�ļ��鱾��û�й涨��С����FileManager����������ÿ����Ĵ�С
</pre>*/
class THBlockFile
{
public:
	/**
	* @brief ���죬ָ���ļ�������
	* @param sFilePath	�ļ�·��
	*/
	THBlockFile(THString sFilePath)
	{
		m_sFilePath=sFilePath;
	}

	/**
	* @brief �͹����ر��ļ�
	*/
	virtual ~THBlockFile()
	{
		if (m_File.IsOpen()) m_File.Close();
	}

	/**
	* @brief ��ȡ�ļ�
	* @param BlockOffest	�����ڵ�ƫ����
	* @param buf			���ݻ���
	* @param len			�����С������ʵ�ʶ�ȡ�Ĵ�С
	* @return				��ȡ���ֵ
	*/
	int Read(UINT BlockOffest,char *buf,UINT &len)
	{
		m_cs.Lock();
		if (!m_File.IsOpen())
		{
			if (!m_File.Open(m_sFilePath,THFile::modeNoTruncate|THFile::modeCreate|THFile::modeReadWrite))
			{
				m_cs.Unlock();
				len=0;
				return RESULT_FILEERROR;
			}
		}
		if (m_File.Seek(BlockOffest,THFile::begin)!=BlockOffest)
		{
			m_cs.Unlock();
			len=0;
			return RESULT_FILEERROR;
		}
		UINT real;
		if ((real=m_File.Read(buf,len))!=len)
		{
			m_cs.Unlock();
			len=real;
			return RESULT_NOMOREDATA;
		}
		m_cs.Unlock();
		return RESULT_SUCCESS;
	}

	/**
	* @brief д���ļ�
	* @param BlockOffest	�����ڵ�ƫ����
	* @param buf			���ݻ���
	* @param len			�����С
	* @return				д����ֵ
	*/
	int Write(UINT BlockOffest,const char *buf,UINT len)
	{
		m_cs.Lock();
		if (!m_File.IsOpen())
		{
			if (!m_File.Open(m_sFilePath,THFile::modeNoTruncate|THFile::modeCreate|THFile::modeReadWrite))
			{
				m_cs.Unlock();
				return RESULT_FILEERROR;
			}
		}
		if (m_File.Seek(BlockOffest,THFile::begin)!=BlockOffest)
		{
			//may be the length not enough
			/*			if (BlockOffest>(UINT)m_File.GetLength())
			{
			m_File.SetLength(BlockOffest+len);
			if (m_File.Seek(BlockOffest,CFile::begin)!=BlockOffest)
			{
			m_cs.Unlock();
			return RESULT_FILEERROR;
			}
			}
			else*/
			{
				m_cs.Unlock();
				return RESULT_FILEERROR;
			}
		}
		try{
			m_File.Write(buf,len);
		}
		catch(...)
		{
			m_cs.Unlock();
			return RESULT_FILEERROR;
		}
		m_cs.Unlock();
		return RESULT_SUCCESS;
	}

	BOOL GetFileModifyTime(THFileStatus &fs) {return THFile::GetStatus(m_sFilePath,fs);}
protected:
	THMutex m_cs;
	THFile m_File;
	THString m_sFilePath;
};


//fixme �������δ����


//���������ļ�����
#define FLAGSTATE_EMPTY			0			///<�ñ�־����û��ʹ��
#define FLAGSTATE_HAVEDATA		1			///<�ñ�־����û������

#define FILESIZEOFFEST			0			///<�ļ���Сֵ����ƫ��
#define FILEINFOOFFEST			10			///<�ļ���Ϣ����ƫ��
#define FILEFLAGOFFEST			100			///<�ļ������־ƫ��

/**
* @brief ���������ļ���
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-02-21 �½���
*/
/**<pre>
 �ļ�������Ϣ�ı�����
</pre>*/
class CBlockFileSetting : public CBlockFile ,public IFileResultCb
{
public:
	CBlockFileSetting(CString sFilePath,UINT index):CBlockFile(sFilePath,index)
	{
		m_FileSize=GetFileSize();
		UINT filesize=m_FileSize/FLAGSIZE;
		if (m_FileSize%FLAGSIZE!=0) filesize++;
		m_LastFlag=filesize;
		m_PriorityFlag=0;
		m_FinishedFlag=-1;
		m_nSegSize=0;
		GetLastFinishedFlag();
		m_TryedFlag=m_FinishedFlag;
		m_cb=NULL;
	}
	virtual ~CBlockFileSetting(){}

	void SetCbHandler(IFileResultCb *cb){m_cb=cb;}
	virtual BOOL OnResult(UINT ResultType,UINT p1,UINT p2);

	/**
	* @brief ���õ�ǰ���ȶ�ȡ�ĵط�
	* @param �ļ�Ƭ��λ��
	*/
	void SetPriorityRequestFlag(UINT nFlag){m_PriorityFlag=nFlag;}

	/**
	* @brief ��ȡ�ļ��Ѿ���ɵ�Flag����
	* @return				�ļ��Ѿ���ɵ�Flag����,����-1��ʾû���κ����Ƭ��
	*/
	int GetLastFinishedFlag();

	//nEndΪ0Ϊ��ָ��end
	BOOL IsFinishedPart(UINT nStart,UINT nEnd);

	/**
	* @brief ��ȡ�ļ�������
	* @return	�ļ���Flag����
	*/
	inline int GetLastFlag(){return m_LastFlag;}

	/**
	* @brief ��ȡ������һ��Ϊֹδ�������Сƫ���������ڼ���
	* @param bMustRecved	ֵΪtrue����ʾ��ȡ����Сƫ����֮ǰ������ȫ���Ѿ���ɵ�Ƭ��
							ֵΪfalse����ʾ��ȡ����Сƫ����֮ǰ�����������Ƭ�ϻ����������е�Ƭ��
	* @return				ƫ������С
	*/
	int GetTryBaseFlag(BOOL bMustRecved);

	/**
	* @brief ��ȡ�ļ���С
	* @return				�ļ���С��δ���д�С��Ϣʱ������0
	*/
	UINT GetFileSize();
	/**
	* @brief д���ļ���С
	* @param size			�ļ���С
	* @return				�Ƿ�ɹ�
	*/
	BOOL SetFileSize(UINT size);

	/**
	* @brief ����Ƭ��״̬
	* @param FlagIndex		Ƭ������
	* @param State			Ƭ��״̬
	*/
	inline void SaveFlagState(UINT FlagIndex,char State){Write(FILEFLAGOFFEST+FlagIndex,&State,sizeof(char));}
	/**
	* @brief ��ȡƬ��״̬
	* @param FlagIndex		Ƭ������
	* @return				Ƭ��״̬
	*/
	char ReadFlagState(UINT FlagIndex);
	/**
	* @brief ��������Ƭ�����
	* @param StartFlag		��ʼƬ��
	* @param Count			������Ƭ����
	*/
	void SetFinishFlags(UINT StartFlag,UINT Count);

	/**
	* @brief Ƭ�������Ƿ����
	* @param StartFlag		��ʼƬ��
	* @param Count			������Ƭ����
	* @return				�Ƿ����
	*/
	BOOL IsFlagsFinish(UINT StartFlag,UINT Count);

	void FreeRequestingFlag(UINT FlagOffest);

	/**
	* @brief ��ȡ����ʵĻ���
	* @param StartFlag		��ʼ������Ƭ��
	* @param needindex		�û��巢�͸��Է������У�Խǰ�ĶԷ�Խ���ȴ���
	* @param flag			���ص�Ƭ�Ϻ�
	* @param len			Ƭ�ϳ���
	* @return				�Ƿ��к���Ƭ��
	*/
	BOOL GetNeedBuffer(UINT StartFlag,int needindex,UINT *flag,UINT *len);

	//��ȡ�ļ�bitmap����ָ��SegSize��ָ�������ٸ���
	void *GetFileBitmap(UINT nSegSize,UINT *plen);

private:
	UINT m_FileSize;
	UINT m_LastFlag;
	CMap<UINT,UINT,DWORD,DWORD>m_QueryingFlag;
	CMyMutex m_cs;
	int m_FinishedFlag;
	int m_TryedFlag;
	UINT m_PriorityFlag;
	UINT m_nSegSize;
	CBitParser m_parser;
	IFileResultCb *m_cb;
};

/**
* @brief �ļ�������
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-02-21 �½���
*/
/**<pre>
 ���ö���ļ���������ʽ��������,��������bufferС������Ҳnew�������ռ�
</pre>*/
class CFileManager : public IFileResultCb
{
public:
	/**
	 * @brief ���캯��
	 * @param sFilePath		�ļ�����·��
	 * @param nBlockSize	ÿ������Ĵ�С
	*/
	CFileManager(CString sFilePath,UINT nBlockSize=FLAGSIZE*2048)
	{
		m_arFileList.SetFreeProc(FreeListCB);
		m_sFilePath=sFilePath;
		CreateDirectory(m_sFilePath,NULL);
		m_nBlockSize=nBlockSize;
		m_blocksetting=new CBlockFileSetting(sFilePath,0);
		m_FileSize=m_blocksetting->GetFileSize();
		m_dwLastHit=0;
		m_cb=NULL;
		srand((unsigned int)time(NULL));
	}
	virtual ~CFileManager(void)
	{
		delete m_blocksetting;
	}

	void SetCbHandler(IFileResultCb *cb)
	{
		m_cb=cb;
		m_blocksetting->SetCbHandler(this);
	}
	virtual BOOL OnResult(UINT ResultType,UINT p1,UINT p2);
	/**
	 * @brief �رն��ļ���ռ��
	*/
	void CloseLock();

	/**
	 * @brief ���¼����ļ�
	*/
	void OpenLock(CString chFilePath);

	BOOL IsFinished();

	//nEndΪ0Ϊ��ָ��end
	BOOL IsFinishedPart(UINT nStart,UINT nEnd)
	{
		m_cs.Lock();
		BOOL ret=m_blocksetting->IsFinishedPart(nStart,nEnd);
		m_cs.Unlock();
		return ret;
	}

	/**
	 * @brief ��ȡ���������
	 * @return ��������������
	*/
	DWORD GetLastFinishedOffest()
	{
		m_cs.Lock();
		int flag=m_blocksetting->GetLastFinishedFlag();
		m_cs.Unlock();
		return (flag+1)*FLAGSIZE;
	}

	/**
	 * @brief �����������������ƫ��ֵ�����ͻ�����������ĵط�
	 * @return
	*/
	void SetPriorityRequestOffest(UINT off)
	{
		m_cs.Lock();
		UINT nFlag=off/FLAGSIZE;
		m_blocksetting->SetPriorityRequestFlag(nFlag);
		m_cs.Unlock();
	}

	/**
	 * @brief ��ȡ����
	 * @param uOffest		��ȡƫ��
	 * @param buf			���ݻ�����
	 * @param len			���ݻ��������ȣ�����ʵ�ʶ�ȡ����
	 * @param bForce		�Ƿ�ǿ�ƶ�ȡ�������ݲ�����ʱ���Ƿ񷵻���������
	 * @return �Ƿ�ɹ� RESULT_XXX
	*/
	int Read(UINT uOffest,char *buf,UINT &len,BOOL bForce=FALSE);
	/**
	 * @brief д������
	 * @param uOffest		д��ƫ�ƣ�����ΪFLAGSIZE��������
	 * @param buf			���ݻ�����
	 * @param len			���ݻ��������ȣ���������ΪFLAGSIZE�������������Ľ�������Ϊ��������
	 * @return �Ƿ�ɹ� RESULT_XXX
	*/
	int Write(UINT uOffest,const char *buf,UINT len);
	inline UINT GetFileSize(){return m_FileSize;}
	inline DWORD GetLastHit(){return m_dwLastHit;}
	BOOL SetFileSize(UINT FileSize)
	{
		m_cs.Lock();
		BOOL ret=m_blocksetting->SetFileSize(FileSize);
		if (ret)
			m_FileSize=FileSize;
		else
			m_FileSize=0;
		m_cs.Unlock();
		if (!ret) OnResult(FileResult_FailData,0,0);
		return ret;
	}

	//����ʵ��ռ�ô��̴�С
	UINT GetUsedFileSize(CString sPath=_T(""));

	/**
	 * @brief ������ݶλ�û��������ɣ�������ݶ���������ı�־
	 * @param StartOffest		��ʼƫ��
	 * @param list				����ƫ�ƶ���(����������ʼ�㣬������Ƭ����)
	 * @param len				���и���
	 * @return �����Ƿ�����Ҫ������
	*/
	void FreeRequestingFlag(UINT FlagOffest)
	{
		m_blocksetting->FreeRequestingFlag(FlagOffest);
	}

	/**
	 * @brief ��������Ļ�û�е����ݶ�
	 * @param StartOffest		��ʼƫ��
	 * @param list				����ƫ�ƶ���(����������ʼ�㣬������Ƭ����)
	 * @param len				���и���
	 * @return �����Ƿ�����Ҫ������
	*/
	BOOL GetRequestDataOffest(UINT StartOffest,UINT *list,int len);

	void *GetFileBitmap(UINT nSegSize,UINT *plen)
	{
		m_cs.Lock();
		void *ret=m_blocksetting->GetFileBitmap(nSegSize,plen);
		m_cs.Unlock();
		return ret;
	}

	CTime GetModifyTime()
	{
		CTime ti;
		ti=m_blocksetting->GetFileModifyTime();
		return ti;
	}
private:
	static void FreeListCB(void *key,void *value,void *adddata)
	{
		delete (CBlockFile *)value;
	}
	CBlockFileSetting *m_blocksetting;						///<�ļ�����Ϣ�����ļ�
	UINT m_nBlockSize;										///<�����С
	UINT m_FileSize;										///<�ļ���С
	CString m_sFilePath;									///<�ļ�·��
	//CMap<UINT,UINT,CBlockFile *,CBlockFile *>m_arFileList;	///<�ļ�ָ��
	CMyDynMapPool<UINT,CBlockFile *,20>m_arFileList;			///<��̬������С������
	CMyMutex m_cs;
	DWORD m_dwLastHit;
	IFileResultCb *m_cb;
};

class IFileManagerEventNotify
{
public:
	virtual void OnFileManagerRelease(CString smd5,PMD5BUFFER buf,CFileManager *pman)=0;
	virtual void OnFileManagerUpdateBitmap(CString smd5,PMD5BUFFER buf,UINT off)=0;
	virtual void OnFileManagerWriteData(CString smd5,PMD5BUFFER buf,UINT datsize)=0;
	virtual void OnFileManagerReadData(CString smd5,PMD5BUFFER buf,UINT datsize)=0;
	virtual void OnFileManagerFailData()=0;
};

/**
* @brief �ļ���������
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-02-21 �½���
*/
class CMd5FileManager :public IFileResultCb
{
public:
	CMd5FileManager(){m_limit=0;m_notify=NULL;}
	virtual ~CMd5FileManager()
	{
		for(int i=0;i<m_file.GetSize();i++)
		{
			if (m_file[i])
			{
				if (m_file[i]->manager) delete m_file[i]->manager;
				delete m_file[i];
			}
		}
		m_file.RemoveAll();
	}

	void Init(CString BasePath,UINT limit)
	{
		m_BasePath=BasePath;
		m_limit=limit;
	}
/*
	CString ReCalcManager(CFileManager *man)
	{
		for(int i=0;i<m_file.GetSize();i++)
		{
			if (m_file[i] && m_file[i]->manager==man)
			{
				//get Md5 by file
				//PrintLog("Start Calc the Md5...");
				UINT size=m_file[i]->manager->GetFileSize();
				char buffer[1024];
				UINT len;
				Md51 Md5ret;
				CMd5 cMd5;
				for(UINT j=0;j<size;j++)
				{
					len=(size>(1024+j))?1024:(size-j);
					if (m_file[i]->manager->Read(j,buffer,len)==RESULT_SUCCESS)
					{
						cMd5.Add(buffer,len);
						j+=len;
					}
					else
						return "";
				}
				cMd5.Finish();
				cMd5.GetHash(&Md5ret);
				CString csret=CMd5::HashToString(&Md5ret);
				csret.MakeLower();
				//PrintLog("Calc the Md5 Finish...");
				m_file[i]->manager->CloseLock();
				::MoveFile(m_BasePath+"\\"+m_file[i]->Md5,m_BasePath+"\\"+csret);
				m_file[i]->Md5=csret;
				m_file[i]->manager->OpenLock(m_BasePath+"\\"+csret);
				return csret;
			}
		}
		return "";
	}
*/
	void GetFileList(CStringArray *ar)
	{
		CString path=m_BasePath+_T("\\*.*");
		WIN32_FIND_DATA FindFileData;
		HANDLE hFind = FindFirstFile(path,&FindFileData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			BOOL bRet=TRUE;
			CString tmp;
			//int count=0;
			while(bRet)
			{
				if (_tcscmp(FindFileData.cFileName,_T("."))!=0 &&
					_tcscmp(FindFileData.cFileName,_T(".."))!=0)
				{
					if (FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
					{
						tmp=FindFileData.cFileName;
						tmp.MakeLower();
						//if (m_limit!=0)
						//	count++;
						//һ��ʼ��ʱ���Ȳ����з������ƣ���Ϊ����ȷ���ĸ����������Ƶ
						//if (count>m_limit)
						//{
							//PrintLog("File Space Limit,Delete Md5:"+tmp);
						//	DeleteFile(tmp);
						//}
						//else
						{
							ar->Add(tmp);
							_CreateOrOpenFileManager(tmp,NULL,FALSE);
						}
					}
				}
				bRet = FindNextFile(hFind,&FindFileData);
			}
			FindClose(hFind);
		}
	}
	CFileManager *FindFileManager(CString Md5);

	CFileManager *FindFileManager(PMD5BUFFER Md5);

	void *GetFileBitmap(CFileManager *man,UINT nSegSize,UINT *plen)
	{
		return man->GetFileBitmap(nSegSize,plen);
	}

	//nEndΪ0Ϊ��ָ��end
	BOOL IsCompleteFile(CString Md5,UINT nStart,UINT nEnd);

	CFileManager *CreateOrOpenFileManager(CString md5)
	{
		return _CreateOrOpenFileManager(md5,NULL);
	}

	CFileManager *CreateOrOpenFileManager(PMD5BUFFER buf)
	{
		if (!buf) return NULL;
		CFileManager *man=FindFileManager(buf);
		if (man) return man;
		return _CreateOrOpenFileManager(CMyMd5::GetMd5Str(buf),buf);
	}

	void DeleteFile(CString Md5)
	{
		for(int i=0;i<m_file.GetSize();i++)
		{
			Md5File *tmp=m_file[i];
			if (tmp)
			{
				if (Md5.CompareNoCase(tmp->Md5)==0)
				{
					m_file.RemoveAt(i);
					if (m_notify) m_notify->OnFileManagerRelease(tmp->Md5,&tmp->md5buf,tmp->manager);
					if (tmp->manager) delete tmp->manager;
					delete tmp;
					break;
				}
			}
		}
		DelSubDir(m_BasePath+_T("\\")+Md5);
	}
	void SetNotify(IFileManagerEventNotify *notify){m_notify=notify;}
	virtual BOOL OnResult(UINT ResultType,UINT p1,UINT p2);

	void DeleteOldestFile()
	{
		DWORD last=0;
		int idx=0;
		for(int i=1;i<m_file.GetSize();i++)
		{
			if (m_file[i]->manager->GetLastHit()<last)
			{
				last=m_file[i]->manager->GetLastHit();
				idx=i;
			}
		}
		DeleteFile(m_file[idx]->Md5);
	}

	inline UINT GetFileCount(){return (UINT)m_file.GetSize();}
	UINT GetAllFileSize()
	{
		UINT ret=0;
		for(int i=0;i<m_file.GetSize();i++)
			ret+=m_file[i]->manager->GetUsedFileSize();
		return ret;
	}

protected:
	typedef struct _Md5File{
		CString Md5;
		MD5BUFFER md5buf;
		CFileManager *manager;
	}Md5File;
	CArray<Md5File *,Md5File *>m_file;
	CString m_BasePath;
	UINT m_limit;
	IFileManagerEventNotify *m_notify;
	void DelSubDir(CString rootpath)
	{
		CString path=rootpath+_T("\\*.*");
		WIN32_FIND_DATA FindFileData;
		HANDLE hFind = FindFirstFile(path,&FindFileData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			BOOL bRet=TRUE;
			while(bRet)
			{
				if (_tcscmp(FindFileData.cFileName,_T("."))!=0 &&
					_tcscmp(FindFileData.cFileName,_T(".."))!=0)
				{
					if (FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
						DelSubDir(rootpath+_T("\\")+FindFileData.cFileName);
					else
						::DeleteFile(rootpath+_T("\\")+FindFileData.cFileName);
				}
				bRet = FindNextFile(hFind,&FindFileData);
			}
			FindClose(hFind);
		}
		::RemoveDirectory(rootpath);
	}

	CFileManager *_CreateOrOpenFileManager(CString md5,PMD5BUFFER buf=NULL,BOOL bLimit=TRUE);
};

BOOL CBlockFileSetting::OnResult(UINT ResultType,UINT p1,UINT p2)
{
	if (m_cb)
		return m_cb->OnResult(ResultType,p1,p2);
	return FALSE;
}

int CBlockFileSetting::GetLastFinishedFlag()
{
	for(UINT i=(UINT)(m_FinishedFlag+1);i<m_LastFlag;i++)
	{
		if (ReadFlagState(i)==FLAGSTATE_HAVEDATA)
			m_FinishedFlag=i;
		else
			break;
	}
	return m_FinishedFlag;
}

BOOL CBlockFileSetting::IsFinishedPart(UINT nStart,UINT nEnd)
{
	if (GetLastFlag()==0) return FALSE;
	if (nStart==0 && nEnd==0)
	{
		return (GetLastFinishedFlag()+1==GetLastFlag());
	}
	else
	{
		if (nEnd!=0)
			nEnd=(nEnd+FLAGSIZE-1)/FLAGSIZE;
		else
			nEnd=GetLastFlag()-1;
		if (nStart!=0) nStart=(nStart+FLAGSIZE-1)/FLAGSIZE;
		if (nEnd<nStart) return FALSE;
		return IsFlagsFinish(nStart,nEnd-nStart);
	}
}

int CBlockFileSetting::GetTryBaseFlag(BOOL bMustRecved)
{
	int f=GetLastFinishedFlag();
	if (!bMustRecved)
	{
		DWORD ti;
		for(UINT i=f+1;i<m_LastFlag;i++)
		{
			if (m_QueryingFlag.Lookup(i,ti))
				f=i;
			else
				break;
		}
	}
	if (f>m_TryedFlag) m_TryedFlag=f;
	return f;
}

UINT CBlockFileSetting::GetFileSize()
{
	UINT size=0;
	UINT len=sizeof(UINT);
	if (Read(FILESIZEOFFEST,(char *)&size,len)==RESULT_SUCCESS)
		return size;
	return 0;
}

BOOL CBlockFileSetting::SetFileSize(UINT size)
{
	if (Write(FILESIZEOFFEST,(char *)&size,sizeof(UINT))==RESULT_SUCCESS)
	{
		m_FileSize=size;
		UINT filesize=size/FLAGSIZE;
		if (size%FLAGSIZE!=0) filesize++;
		m_LastFlag=filesize;
		m_File.SetLength(filesize+FILEFLAGOFFEST);
		return TRUE;
	}
	return FALSE;
}

char CBlockFileSetting::ReadFlagState(UINT FlagIndex)
{
	char ret=FLAGSTATE_EMPTY;
	UINT len=sizeof(char);
	if (Read(FILEFLAGOFFEST+FlagIndex,&ret,len)==RESULT_SUCCESS)
		return ret;
	return FLAGSTATE_EMPTY;
}

void CBlockFileSetting::SetFinishFlags(UINT StartFlag,UINT Count)
{
	m_cs.Lock();
	for(UINT i=0;i<Count;i++)
	{
		SaveFlagState(StartFlag+i,FLAGSTATE_HAVEDATA);
		m_QueryingFlag.RemoveKey(StartFlag+i);
	}
	m_cs.Unlock();
	//check is need to update bitmap
	if (m_nSegSize!=0)
	{
		//get bitmap offest
		UINT boff=StartFlag/m_nSegSize;
		UINT boff2=(StartFlag+Count)/m_nSegSize;
		if (m_parser.GetBit(boff)==FALSE)
		{
			if (IsFlagsFinish(boff*m_nSegSize,m_nSegSize))
			{
				//need update
				m_parser.SetBit(boff,TRUE);
				if (m_cb) m_cb->OnResult(FileResult_UpdateBitmap,boff,0);
			}
		}
		//later me,ֻ�жϿ�ʼ�ͽ���Ƭ���Ƿ��㹻?�ֽ׶��ǹ��ģ���Ϊһ����ɲ���ܶ�����
		if (boff2!=boff && m_parser.GetBit(boff2)==FALSE)
		{
			if (IsFlagsFinish(boff2*m_nSegSize,m_nSegSize))
			{
				//need update
				m_parser.SetBit(boff2,TRUE);
				if (m_cb) m_cb->OnResult(FileResult_UpdateBitmap,boff2,0);
			}
		}
	}
}

BOOL CBlockFileSetting::IsFlagsFinish(UINT StartFlag,UINT Count)
{
	if (StartFlag>=m_LastFlag) return FALSE;
	if (StartFlag+Count>=m_LastFlag) Count=m_LastFlag-StartFlag;
	//���ڼ���
	if ((int)(StartFlag+Count)<=GetLastFinishedFlag()) return TRUE;//����-1
	//return TRUE;
	char *buf=new char[Count];
	UINT readed=Count;
	BOOL ret=TRUE;
	if (Read(FILEFLAGOFFEST+StartFlag,buf,readed)==RESULT_SUCCESS)
	{
		for(UINT i=0;i<Count;i++)
		{
			if (*(buf+i)!=FLAGSTATE_HAVEDATA)
			{
				ret=FALSE;
				break;
			}
		}
	}
	else
		ret=FALSE;
	delete [] buf;
	return ret;
}

void CBlockFileSetting::FreeRequestingFlag(UINT FlagOffest)
{
	m_QueryingFlag.RemoveKey(FlagOffest);
}

BOOL CBlockFileSetting::GetNeedBuffer(UINT StartFlag,int needindex,UINT *flag,UINT *len)
{
	DWORD ti;
	BOOL bFit;
	m_cs.Lock();
	DWORD myti=GetTickCount();
	UINT EdFlag=(UINT)(GetTryBaseFlag(needindex==0)+1);
	if (StartFlag<EdFlag) StartFlag=EdFlag;
	UINT RetryFlag=0;
	if (StartFlag<m_PriorityFlag)
	{
		RetryFlag=StartFlag;
		StartFlag=m_PriorityFlag;
	}
	for(UINT i=StartFlag;i<m_LastFlag;i++)
	{
		if (ReadFlagState(i)==FLAGSTATE_EMPTY)
		{
			bFit=FALSE;
			if (m_QueryingFlag.Lookup(i,ti))
			{
				//������Ҫ������ʱ�����ж��Ƿ���Ҫ��������֮ǰ��û�е�����
				if (needindex==0)
				{
					if (myti-ti>BUFFERNEEDREQUERY)
					{
						m_QueryingFlag[i]=myti;
						bFit=TRUE;
					}
				}
			}
			else
			{
				//��buffer����
				m_QueryingFlag[i]=myti;
				bFit=TRUE;
			}
			if (bFit)
			{
				for(UINT j=1;i+j<m_LastFlag && j<MAXTRANSFERUNIT;j++)
				{
					if (ReadFlagState(i+j)!=FLAGSTATE_EMPTY)
					{
						//j--;
						break;
					}
					m_QueryingFlag[i+j]=myti;
				}
				*flag=i;
				*len=j;
				m_cs.Unlock();
				return TRUE;
			}
		}
	}
	m_cs.Unlock();
	if (RetryFlag!=0)
	{
		//�������صĵط��Ѿ�ȫ�������꣬���»ص�֮ǰ�ĵط�����
		//PrintLog("The Priority Download Finish,Normalize retry.");
		m_PriorityFlag=0;
		return GetNeedBuffer(RetryFlag,needindex,flag,len);
	}
	//PrintLog("EdFlag is :%d,Can't Found any more not down data",EdFlag);
	return FALSE;
}

void *CBlockFileSetting::GetFileBitmap(UINT nSegSize,UINT *plen)
{
	if (!plen) return NULL;
	*plen=0;
	if (nSegSize==0 || m_FileSize==0) return NULL;
	if (m_nSegSize==0)
	{
		m_nSegSize=nSegSize;
		UINT last=m_LastFlag;
		for(UINT i=0;i<last;i+=m_nSegSize)
		{
			m_parser.SetBit(i/m_nSegSize,IsFlagsFinish(i,min(m_nSegSize,last-i)));
		}
	}
	if (m_nSegSize!=nSegSize) return NULL;
	return m_parser.GetParseDataBuffer(plen);
}

BOOL CFileManager::OnResult(UINT ResultType,UINT p1,UINT p2)
{
	if (m_cb)
		return m_cb->OnResult(ResultType,(UINT)(UINT_PTR)this,p1);
	return FALSE;
}

void CFileManager::CloseLock()
{
	m_cs.Lock();
	delete m_blocksetting;
	m_blocksetting=NULL;
	m_arFileList.RemoveAll();
}

void CFileManager::OpenLock(CString chFilePath)
{
	m_sFilePath=chFilePath;
	CreateDirectory(m_sFilePath,NULL);
	m_blocksetting=new CBlockFileSetting(m_sFilePath,0);
	m_FileSize=m_blocksetting->GetFileSize();
	srand((unsigned int)time(NULL));
	m_cs.Unlock();
}

BOOL CFileManager::IsFinished()
{
	m_cs.Lock();
	BOOL ret=(m_blocksetting->GetLastFinishedFlag()+1==m_blocksetting->GetLastFlag() && m_blocksetting->GetLastFlag()!=0);
	m_cs.Unlock();
	return ret;
}

int CFileManager::Read(UINT uOffest,char *buf,UINT &len,BOOL bForce)
{
	//laterme bForce ΪTrueʱ�����⣬��IsFlagsFinish��ȫ����Чʱ��Ӧ��Ҳ����false����len�п��ܱ���󳤶ȸ���
	m_dwLastHit=GetTickCount();
	m_cs.Lock();
	//check buffer is ready
	UINT nStart=uOffest/FLAGSIZE;
	UINT nEnd=(uOffest+len)/FLAGSIZE;
	if ((uOffest+len)%FLAGSIZE!=0) nEnd++;
	if (bForce==FALSE && !m_blocksetting->IsFlagsFinish(nStart,nEnd-nStart))
	{
		len=0;
		m_cs.Unlock();
		return RESULT_NOMOREDATA;
	}
	//read file
	UINT index=uOffest/m_nBlockSize;
	UINT blockstart=uOffest%m_nBlockSize;
	CBlockFile *file;
	UINT readlen;
	int ret;
	UINT rlen=len;
	len=0;
	while(rlen>0)
	{
		/*			file=m_arFileList[index];
		if (!file)
		{
		file=new CBlockFile(m_sFilePath,index+1);
		m_arFileList[index]=file;
		}*/
		BOOL bpret=m_arFileList.GetAt(index,file);
		if (bpret==FALSE || file==NULL)
		{
			file=new CBlockFile(m_sFilePath,index+1);
			m_arFileList.SetAt(index,file);
		}
		ASSERT(m_nBlockSize>=blockstart);
		readlen=min(blockstart+rlen,m_nBlockSize)-blockstart;
		ret=file->Read(blockstart,buf,readlen);
		len+=readlen;
		rlen-=readlen;
		if (ret==RESULT_FILEERROR)
		{
			len=0;
			m_cs.Unlock();
			return ret;
		}
		else if (ret==RESULT_NOMOREDATA)
		{
			m_cs.Unlock();
			OnResult(FileResult_ReadingData,len,0);
			if (bForce)
				return RESULT_SUCCESS;
			return RESULT_NOMOREDATA;
		}
		buf+=readlen;
		index++;
		blockstart=0;
	}
	m_cs.Unlock();
	OnResult(FileResult_ReadingData,len,0);
	return RESULT_SUCCESS;
}

int CFileManager::Write(UINT uOffest,const char *buf,UINT len)
{
	OnResult(FileResult_WritingData,len,0);
	m_dwLastHit=GetTickCount();
	m_cs.Lock();
	ASSERT(uOffest%FLAGSIZE==0);
	ASSERT(len%FLAGSIZE==0 || (m_FileSize!=0 && uOffest+len>=m_FileSize));
	UINT index=uOffest/m_nBlockSize;
	UINT blockstart=uOffest%m_nBlockSize;
	CBlockFile *file;
	UINT writelen;
	int ret;
	while(len>0)
	{
		/*			file=m_arFileList[index];
		if (!file)
		{
		file=new CBlockFile(m_sFilePath,index+1);
		m_arFileList[index]=file;
		}*/
		BOOL bpret=m_arFileList.GetAt(index,file);
		if (bpret==FALSE || file==NULL)
		{
			file=new CBlockFile(m_sFilePath,index+1);
			m_arFileList.SetAt(index,file);
		}
		writelen=min(blockstart+len,m_nBlockSize)-blockstart;
		ret=file->Write(blockstart,buf,writelen);
		if (ret!=RESULT_SUCCESS)
		{
			m_cs.Unlock();
			OnResult(FileResult_FailData,0,0);
			return ret;
		}
		m_blocksetting->SetFinishFlags((index*m_nBlockSize+blockstart)/FLAGSIZE,writelen/FLAGSIZE+((writelen%FLAGSIZE==0)?0:1));
		len-=writelen;
		buf+=writelen;
		index++;
		blockstart=0;
	}
	m_cs.Unlock();
	return RESULT_SUCCESS;
}

UINT CFileManager::GetUsedFileSize(CString sPath)
{
	UINT totalsize=0;
	if (sPath.IsEmpty()) sPath=m_sFilePath;
	CString path=sPath+_T("\\*.*");
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(path,&FindFileData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		BOOL bRet=TRUE;
		while(bRet)
		{
			if (_tcscmp(FindFileData.cFileName,_T("."))!=0 &&
				_tcscmp(FindFileData.cFileName,_T(".."))!=0)
			{
				if (FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
				{
					totalsize+=GetUsedFileSize(sPath+_T("\\")+FindFileData.cFileName);
				}
				else
				{
					//fixme do not support FindFileData.nFileSizeHigh!=0's file
					totalsize+=FindFileData.nFileSizeLow;
				}
			}
			bRet = FindNextFile(hFind,&FindFileData);
		}
		FindClose(hFind);
	}
	return totalsize;
}

BOOL CFileManager::GetRequestDataOffest(UINT StartOffest,UINT *list,int len)
{
	for(int i=0;i<len;i++)
	{
		*(list+i*2)=0;
		*(list+i*2+1)=0;
	}
	i=0;
	UINT starttag=StartOffest/FLAGSIZE;
	//if (StartOffest%FLAGSIZE!=0) starttag++
	m_cs.Lock();
	while(i<len)
	{
		if (!m_blocksetting->GetNeedBuffer(starttag,i,list+i*2,list+i*2+1))
		{
			m_cs.Unlock();
			return (i!=0);
		}
		i++;
	}
	m_cs.Unlock();
	return (i!=0);
}

CFileManager *CMd5FileManager::FindFileManager(CString Md5)
{
	for(int i=0;i<m_file.GetSize();i++)
	{
		if (m_file[i] && Md5.CompareNoCase(m_file[i]->Md5)==0)
			return m_file[i]->manager;
	}
	return NULL;
}

CFileManager *CMd5FileManager::FindFileManager(PMD5BUFFER Md5)
{
	if (Md5)
	{
		for(int i=0;i<m_file.GetSize();i++)
		{
			if (m_file[i] && memcmp(&m_file[i]->md5buf,Md5,sizeof(MD5BUFFER))==0)
				return m_file[i]->manager;
		}
	}
	return NULL;
}

BOOL CMd5FileManager::IsCompleteFile(CString Md5,UINT nStart,UINT nEnd)
{
	BOOL bRet=FALSE;
	CFileManager *man=FindFileManager(Md5);
	if (man)
	{
		bRet=man->IsFinishedPart(nStart,nEnd);
	}
	else
	{
		man=new CFileManager(m_BasePath+_T("\\")+Md5);
		if (man)
		{
			bRet=man->IsFinishedPart(nStart,nEnd);
			delete man;
		}
	}
	return bRet;
}

BOOL CMd5FileManager::OnResult(UINT ResultType,UINT p1,UINT p2)
{
	if (ResultType==FileResult_UpdateBitmap)
	{
		CFileManager *man=(CFileManager *)(UINT_PTR)p1;
		if (m_notify)
		{
			for(int i=0;i<m_file.GetSize();i++)
			{
				Md5File *tmp=m_file[i];
				if (tmp)
				{
					if (tmp->manager==man)
					{
						m_notify->OnFileManagerUpdateBitmap(tmp->Md5,&tmp->md5buf,p2);
					}
				}
			}
		}
	}
	else if (ResultType==FileResult_WritingData)
	{
		CFileManager *man=(CFileManager *)(UINT_PTR)p1;
		if (m_notify)
		{
			for(int i=0;i<m_file.GetSize();i++)
			{
				Md5File *tmp=m_file[i];
				if (tmp)
				{
					if (tmp->manager==man)
					{
						m_notify->OnFileManagerWriteData(tmp->Md5,&tmp->md5buf,p2);
					}
				}
			}
		}
	}
	else if (ResultType==FileResult_ReadingData)
	{
		CFileManager *man=(CFileManager *)(UINT_PTR)p1;
		if (m_notify)
		{
			for(int i=0;i<m_file.GetSize();i++)
			{
				Md5File *tmp=m_file[i];
				if (tmp)
				{
					if (tmp->manager==man)
					{
						m_notify->OnFileManagerReadData(tmp->Md5,&tmp->md5buf,p2);
					}
				}
			}
		}
	}
	else if (ResultType==FileResult_FailData)
	{
		if (m_notify)
		{
			m_notify->OnFileManagerFailData();
		}
	}
	return TRUE;
}

CFileManager *CMd5FileManager::_CreateOrOpenFileManager(CString md5,PMD5BUFFER buf,BOOL bLimit)
{
	CFileManager *man=FindFileManager(md5);
	if (man) return man;
	MD5BUFFER mdbuf;
	if (!buf)
	{
		if (!CMyMd5::GetMd5Buffer(md5,&mdbuf)) return NULL;
		buf=&mdbuf;
	}
	if (m_limit!=0 && (UINT)m_file.GetSize()>=m_limit && bLimit)
	{
		//PrintLog("File Space Limit,Delete Md5:"+m_file[0]->Md5);
		DeleteOldestFile();
	}
	Md5File *Md5=new Md5File;
	Md5->Md5=md5;
	memcpy(&Md5->md5buf,buf,sizeof(MD5BUFFER));
	Md5->manager=new CFileManager(m_BasePath+_T("\\")+md5);
	if (Md5->manager)
	{
		Md5->manager->SetCbHandler(this);
		m_file.Add(Md5);
	}
	return Md5->manager;
}
