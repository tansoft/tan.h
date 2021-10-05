#pragma once

#include <THStruct.h>
#include <THThread.h>

#define FILE_CHANGE_ADDED		1
#define FILE_CHANGE_REMOVED		2
#define FILE_CHANGE_MODIFIED	3
#define FILE_CHANGE_RENAMED		4

class ITHFileChangeNotifyHandler
{
public:
	virtual void OnChangeNotify(int nCode,THString str1,THString str2,void *adddata)=0;
};

#define	ALL_FILE_CHANGE_FILTER		FILE_NOTIFY_CHANGE_FILE_NAME|\
									FILE_NOTIFY_CHANGE_DIR_NAME|\
									FILE_NOTIFY_CHANGE_ATTRIBUTES|\
									FILE_NOTIFY_CHANGE_SIZE|\
									FILE_NOTIFY_CHANGE_LAST_WRITE|\
									FILE_NOTIFY_CHANGE_LAST_ACCESS|\
									FILE_NOTIFY_CHANGE_CREATION|\
									FILE_NOTIFY_CHANGE_SECURITY

/**
* @brief 文件目录改变监视类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-08-18 新建类
*/
/**<pre>
</pre>*/
class THFileChangeWatcher : private THThread
{
public:
	THFileChangeWatcher()
	{
		m_handler=NULL;
		m_adddata=NULL;
		m_data.SetFreeProc(FreeWatcher,this);
	}

	virtual ~THFileChangeWatcher()
	{
		m_data.RemoveAll();
	}

	void SetHandler(ITHFileChangeNotifyHandler *handler,void *adddata)
	{
		m_handler=handler;
		m_adddata=adddata;
	}

	int AddWatcher(THString sWatchPath,DWORD dwWatchFilter=ALL_FILE_CHANGE_FILTER,BOOL bSubDir=TRUE)
	{
		THWatchData *data=new THWatchData;
		if (!data) return 0;
		data->sWatchPath=sWatchPath;
		data->dwWatchFilter=dwWatchFilter;
		data->bSubDir=bSubDir;
		data->hPath=INVALID_HANDLE_VALUE;
		data->nThreadId=StartThread(0,data,0);
		if (data->nThreadId==0)
		{
			delete data;
			return 0;
		}
		m_data.SetAt(data->nThreadId,data);
		return data->nThreadId;
	}

	BOOL RemoveWatcher(int nWatcherId)
	{
		return m_data.RemoveAt(nWatcherId);
	}

private:
	typedef struct _THWatchData{
		int nThreadId;
		THString sWatchPath;
		DWORD dwWatchFilter;
		BOOL bSubDir;
		HANDLE hPath;
	}THWatchData;
	THMap<int,THWatchData *> m_data;
	ITHFileChangeNotifyHandler *m_handler;
	void *m_adddata;
	static void FreeWatcher(void *key,void *value,void *adddata)
	{
		THWatchData *data=(THWatchData *)value;
		if (data)
		{
			THFileChangeWatcher *p=(THFileChangeWatcher *)adddata;
			p->KillThread(data->nThreadId,0);
			if (data->hPath!=INVALID_HANDLE_VALUE)
				CloseHandle(data->hPath);
			delete data;
		}
	}
	virtual void ThreadFunc(int threadid,void *adddata)
	{
		THWatchData *data=(THWatchData *)adddata;
		if (data->hPath==INVALID_HANDLE_VALUE)
		{
			data->hPath=CreateFile(data->sWatchPath,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
				NULL,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS,NULL);
		}
        char buf[ 2*(sizeof(FILE_NOTIFY_INFORMATION)+MAX_PATH) ];
        FILE_NOTIFY_INFORMATION* pNotify=(FILE_NOTIFY_INFORMATION *)buf;
        DWORD BytesReturned;
        while(1)
        {
            if( ReadDirectoryChangesW(data->hPath,pNotify,sizeof(buf),data->bSubDir,
				data->dwWatchFilter,&BytesReturned,NULL,NULL))
            {
				THString str1=THCharset::w2t(pNotify->FileName,pNotify->FileNameLength/sizeof(wchar_t));
				THString str2;
                if( 0 != pNotify->NextEntryOffset )
                {
                    PFILE_NOTIFY_INFORMATION p = (PFILE_NOTIFY_INFORMATION)((char*)pNotify+pNotify->NextEntryOffset);
					str2=THCharset::w2t(p->FileName,p->FileNameLength/sizeof(wchar_t));
                }
                if (m_handler)
					m_handler->OnChangeNotify(pNotify->Action,str1,str2,m_adddata);
            }
            else
            {
                break;
            }
        }
    }
};
