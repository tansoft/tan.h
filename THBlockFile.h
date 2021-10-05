#pragma once

#include <THStruct.h>
#include <THBitParser.h>

/**
* @brief 文件操作处理结果
*/
#define THFILERESULT_SUCCESS			0		///<成功
#define THFILERESULT_NOMOREDATA		1		///<没有足够数据
#define THFILERESULT_FILEERROR		2		///<文件错误

#define FLAGSIZE				1024	///<文件分片的大小

#define BUFFERNEEDREQUERY		(5*1000)	///<重试请求buffer超时时间
#define MAXTRANSFERUNIT			64		///<每段区域连续尝试的最大偏移量

#define THFileResult_UpdateBitmap	1
#define THFileResult_WritingData	2
#define THFileResult_ReadingData	3
#define THFileResult_FailData		4

/**
* @brief 文件操作回调类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-02-29 新建类
*/
class ITHFileResultCb
{
public:
	virtual BOOL OnResult(UINT ResultType,UINT p1,UINT p2)=0;
};

/**
* @brief 区块文件类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-02-21 新建类
*/
/**<pre>
 每个文件块的处理类，文件块本身没有规定大小，由FileManager本身计算决定每个块的大小
</pre>*/
class THBlockFile
{
public:
	/**
	* @brief 构造，指定文件块属性
	* @param sFilePath	文件路径
	*/
	THBlockFile(THString sFilePath)
	{
		m_sFilePath=sFilePath;
	}

	/**
	* @brief 释构，关闭文件
	*/
	virtual ~THBlockFile()
	{
		if (m_File.IsOpen()) m_File.Close();
	}

	/**
	* @brief 读取文件
	* @param BlockOffest	区块内的偏移量
	* @param buf			数据缓冲
	* @param len			缓冲大小，返回实际读取的大小
	* @return				读取结果值
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
	* @brief 写入文件
	* @param BlockOffest	区块内的偏移量
	* @param buf			数据缓冲
	* @param len			缓冲大小
	* @return				写入结果值
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


//fixme 下面的类未整理


//区块配置文件常量
#define FLAGSTATE_EMPTY			0			///<该标志区块没有使用
#define FLAGSTATE_HAVEDATA		1			///<该标志区块没有数据

#define FILESIZEOFFEST			0			///<文件大小值保存偏移
#define FILEINFOOFFEST			10			///<文件信息保存偏移
#define FILEFLAGOFFEST			100			///<文件区块标志偏移

/**
* @brief 区块配置文件类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-02-21 新建类
*/
/**<pre>
 文件区块信息的保存类
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
	* @brief 设置当前优先读取的地方
	* @param 文件片断位置
	*/
	void SetPriorityRequestFlag(UINT nFlag){m_PriorityFlag=nFlag;}

	/**
	* @brief 读取文件已经完成的Flag索引
	* @return				文件已经完成的Flag索引,返回-1表示没有任何完成片断
	*/
	int GetLastFinishedFlag();

	//nEnd为0为不指定end
	BOOL IsFinishedPart(UINT nStart,UINT nEnd);

	/**
	* @brief 读取文件总索引
	* @return	文件总Flag索引
	*/
	inline int GetLastFlag(){return m_LastFlag;}

	/**
	* @brief 获取到对上一次为止未请求的最小偏移量，用于加速
	* @param bMustRecved	值为true，表示获取的最小偏移量之前必须是全部已经完成的片断
							值为false，表示获取的最小偏移量之前必须是已完成片断或者已请求中的片断
	* @return				偏移量大小
	*/
	int GetTryBaseFlag(BOOL bMustRecved);

	/**
	* @brief 读取文件大小
	* @return				文件大小，未含有大小信息时，返回0
	*/
	UINT GetFileSize();
	/**
	* @brief 写入文件大小
	* @param size			文件大小
	* @return				是否成功
	*/
	BOOL SetFileSize(UINT size);

	/**
	* @brief 保存片断状态
	* @param FlagIndex		片断索引
	* @param State			片断状态
	*/
	inline void SaveFlagState(UINT FlagIndex,char State){Write(FILEFLAGOFFEST+FlagIndex,&State,sizeof(char));}
	/**
	* @brief 读取片断状态
	* @param FlagIndex		片断索引
	* @return				片断状态
	*/
	char ReadFlagState(UINT FlagIndex);
	/**
	* @brief 批量设置片断完成
	* @param StartFlag		开始片断
	* @param Count			连续的片断数
	*/
	void SetFinishFlags(UINT StartFlag,UINT Count);

	/**
	* @brief 片断区域是否都完成
	* @param StartFlag		开始片断
	* @param Count			连续的片断数
	* @return				是否都完成
	*/
	BOOL IsFlagsFinish(UINT StartFlag,UINT Count);

	void FreeRequestingFlag(UINT FlagOffest);

	/**
	* @brief 获取最合适的缓冲
	* @param StartFlag		开始检索的片断
	* @param needindex		该缓冲发送给对方的序列，越前的对方越优先处理
	* @param flag			返回的片断号
	* @param len			片断长度
	* @return				是否有合适片断
	*/
	BOOL GetNeedBuffer(UINT StartFlag,int needindex,UINT *flag,UINT *len);

	//获取文件bitmap，并指定SegSize，指定后不能再更改
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
* @brief 文件缓存类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-02-21 新建类
*/
/**<pre>
 采用多个文件块阵列形式保存数据,这样不用buffer小量数据也new出整个空间
</pre>*/
class CFileManager : public IFileResultCb
{
public:
	/**
	 * @brief 构造函数
	 * @param sFilePath		文件基础路径
	 * @param nBlockSize	每个区块的大小
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
	 * @brief 关闭对文件的占用
	*/
	void CloseLock();

	/**
	 * @brief 重新加载文件
	*/
	void OpenLock(CString chFilePath);

	BOOL IsFinished();

	//nEnd为0为不指定end
	BOOL IsFinishedPart(UINT nStart,UINT nEnd)
	{
		m_cs.Lock();
		BOOL ret=m_blocksetting->IsFinishedPart(nStart,nEnd);
		m_cs.Unlock();
		return ret;
	}

	/**
	 * @brief 读取已完成区块
	 * @return 返回已完成区块号
	*/
	DWORD GetLastFinishedOffest()
	{
		m_cs.Lock();
		int flag=m_blocksetting->GetLastFinishedFlag();
		m_cs.Unlock();
		return (flag+1)*FLAGSIZE;
	}

	/**
	 * @brief 设置现在优先请求的偏移值，即客户想优先请求的地方
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
	 * @brief 读取区块
	 * @param uOffest		读取偏移
	 * @param buf			数据缓冲区
	 * @param len			数据缓冲区长度，返回实际读取长度
	 * @param bForce		是否强制读取，即数据不够的时候是否返回已有数据
	 * @return 是否成功 RESULT_XXX
	*/
	int Read(UINT uOffest,char *buf,UINT &len,BOOL bForce=FALSE);
	/**
	 * @brief 写入区块
	 * @param uOffest		写入偏移，必需为FLAGSIZE的整数倍
	 * @param buf			数据缓冲区
	 * @param len			数据缓冲区长度，正常必需为FLAGSIZE的整数倍，最后的结束包可为非整数倍
	 * @return 是否成功 RESULT_XXX
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

	//返回实际占用磁盘大小
	UINT GetUsedFileSize(CString sPath=_T(""));

	/**
	 * @brief 如果数据段还没有下载完成，清除数据段正在请求的标志
	 * @param StartOffest		开始偏移
	 * @param list				返回偏移队列(包括队列起始点，和连续片断数)
	 * @param len				队列个数
	 * @return 返回是否有需要的数据
	*/
	void FreeRequestingFlag(UINT FlagOffest)
	{
		m_blocksetting->FreeRequestingFlag(FlagOffest);
	}

	/**
	 * @brief 返回最近的还没有的数据段
	 * @param StartOffest		开始偏移
	 * @param list				返回偏移队列(包括队列起始点，和连续片断数)
	 * @param len				队列个数
	 * @return 返回是否有需要的数据
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
	CBlockFileSetting *m_blocksetting;						///<文件总信息保存文件
	UINT m_nBlockSize;										///<区块大小
	UINT m_FileSize;										///<文件大小
	CString m_sFilePath;									///<文件路径
	//CMap<UINT,UINT,CBlockFile *,CBlockFile *>m_arFileList;	///<文件指针
	CMyDynMapPool<UINT,CBlockFile *,20>m_arFileList;			///<动态调整大小的数组
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
* @brief 文件管理器类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-02-21 新建类
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
						//一开始的时候先不进行服务限制，因为不能确定哪个是最冷的视频
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

	//nEnd为0为不指定end
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
		//later me,只判断开始和结束片段是否足够?现阶段是够的，因为一次完成不会很多数据
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
	//用于加速
	if ((int)(StartFlag+Count)<=GetLastFinishedFlag()) return TRUE;//包括-1
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
				//当最重要的请求时，才判断是否需要重新请求之前还没有的数据
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
				//该buffer合适
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
		//优先下载的地方已经全部下载完，重新回到之前的地方下载
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
	//laterme bForce 为True时有问题，当IsFlagsFinish不全部有效时，应该也返回false，但len有可能比最后长度更长
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
