#pragma once

#define THSYSTEM_INCLUDE_WINBASE
#include <THSystem.h>
#include <THThread.h>
#include <THMemBuf.h>

class THCmdProcess;
class ITHCmdProcessEvent;
typedef struct _THProcessInfo
{
	THString sCmd;						///<������
	BOOL bAsyc;							///<�Ƿ��첽
	BOOL bShow;							///<�Ƿ�����
	DWORD nRetCode;						///<������
	DWORD *pnRetCode;					///<���뷵��ֵָ��
	ITHCmdProcessEvent *handler;		///<�¼���Ӧ��
	void *pAddData;						///<����ĸ��Ӳ���
	UINT nSleepTime;					///<��ѯʱ��
	UINT nDebugSleepTime;				///<���ģʽ��ѯʱ��
	DWORD nStartTime;					///<��ʼʱ��
	UINT nLimitTime;					///<�޶�ȡ��ʱ��
	DWORD dwPriority;					///<�߳����ȼ�
	STARTUPINFO startupInfo;			///<����������Ϣ
	PROCESS_INFORMATION processInfo;	///<������Ϣ
	THMemBuf bufout;					///<����ܵ���Ϣ����
	THMemBuf buferr;					///<����ܵ���Ϣ����
	HANDLE hIn;							///<������
}THProcessInfo;

/**
* @brief �����д����¼���Ӧ��
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-06-13 �½���
*/
/**<pre>
�÷���
</pre>*/
class ITHCmdProcessEvent
{
public:
	/**
	* @brief ���̽���ǰ�ص�
	* @param pProcess			����������
	* @param info				������Ϣ������ĸ��Ӳ���������ܵ��ͳ���ܵ���Ϣ
	* @param nThreadId			ȡ�����̵�id
	* @return �Ƿ�ȡ������
	*/
	virtual BOOL OnProcessPreCreate(THCmdProcess *pProcess,THProcessInfo *info,int nThreadId)
	{
		return FALSE;
	}
	/**
	* @brief ���̽����ص�
	* @param pProcess			����������
	* @param info				������Ϣ������ĸ��Ӳ���������ܵ��ͳ���ܵ���Ϣ
	* @param nThreadId			ȡ�����̵�id
	* @return �Ƿ�ȡ������
	*/
	virtual BOOL OnProcessCreate(THCmdProcess *pProcess,THProcessInfo *info,int nThreadId)
	{
		return FALSE;
	}
	/**
	* @brief �����еĻص�
	* @param pProcess			����������
	* @param info				������Ϣ������ĸ��Ӳ���������ܵ��ͳ���ܵ���Ϣ
	* @param nThreadId			ȡ�����̵�id
	* @return �Ƿ�ȡ������
	*/
	virtual BOOL OnProcessingOutput(THCmdProcess *pProcess,THProcessInfo *info,int nThreadId)
	{
		//info->bufout.Empty();
		//info->buferr.Empty();
		return FALSE;
	}

	/**
	* @brief ������ɵĻص�������ܵ�����Ϣ��info->bufout������ܵ�����Ϣ��info->buferr
	* @param pProcess			����������
	* @param info				������Ϣ
	* @param nThreadId			ȡ�����̵�id
	* @param bRunRet			���н��
	* @param nRetCode			���򷵻���
	*/
	virtual void OnProcessFinish(THCmdProcess *pProcess,THProcessInfo *info,int nThreadId,BOOL bRunRet,DWORD nRetCode){}
};

/**
* @brief �����д�����
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-06-13 �½���
*/
/**<pre>
�÷���
</pre>*/
class THCmdProcess : private ITHCmdProcessEvent ,private THThread
{
public:
	THCmdProcess(){}
	virtual ~THCmdProcess(){}

	/**
	* @brief ִ�������г���
	* @param sCmd				������
	* @param bAsyc				�Ƿ��첽��Ĭ��Ϊ�ȴ�������ִ�����
	* @param pnRetCode			���򷵻���
	* @param bShow				�Ƿ���ʾ���̣߳�����showʱ��Ĭ�Ϲҽ��ӽ��������������ʱcfile��Ĭ�ϻ�̳о������ʱ������ӽ��̲��˳����ļ�һֱռ�õ����������������ļ�������ָ��CFile::modeNoInherit����
	* @param nSleepTime			��ѯʱ��
	* @param nLimitTime			�޶�ȡ��ʱ�䣬Ϊ0Ϊ���ǵȴ�������ֵΪ�ȴ�������
	* @param handler			�¼���Ӧ��
	* @param pAddData			���Ӳ���
	* @param dwPriority			�������ȼ�
	* @param bParseException	�Ƿ����д���Exception�����ηǷ������Ի���
	* @return �Ƿ����гɹ�,����0Ϊʧ��,���ط�0Ϊ�߳�id
	*/
	int RunCmd(THString sCmd,BOOL bAsyc=FALSE,DWORD *pnRetCode=NULL,BOOL bShow=FALSE,UINT nSleepTime=500,UINT nLimitTime=0,ITHCmdProcessEvent *handler=NULL,void *pAddData=NULL,DWORD dwPriority=NORMAL_PRIORITY_CLASS,BOOL bParseException=FALSE)
	{
		THProcessInfo *info=new THProcessInfo;
		if (!info) return 0;
		info->bAsyc=bAsyc;
		info->bShow=bShow;
		if (!handler) handler=this;
		info->handler=handler;
		info->nRetCode=0;
		info->pAddData=pAddData;
		if (pnRetCode) *pnRetCode=0;
		info->pnRetCode=pnRetCode;
		info->sCmd=sCmd;
		info->nStartTime=GetTickCount();
		info->nLimitTime=nLimitTime;
		if (bParseException)
		{
			nSleepTime=(nSleepTime+1)/2;
			info->nSleepTime=nSleepTime;
			info->nDebugSleepTime=nSleepTime;
		}
		else
		{
			info->nSleepTime=nSleepTime;
			info->nDebugSleepTime=0;
		}
		info->dwPriority=dwPriority;
		info->hIn=NULL;
		memset(&info->startupInfo, 0, sizeof(STARTUPINFO));
		memset(&info->processInfo, 0, sizeof(PROCESS_INFORMATION));
		info->startupInfo.cb = sizeof(STARTUPINFO);
		if (info->bAsyc)
			return StartThread(0,info);
		return ParseCmd(0,info);
	}

	void CancelCmd(int nThreadId)
	{
		KillThread(nThreadId);
	}

	BOOL WritePipe(HANDLE hIn,const void *data,UINT nLen)
	{
		DWORD lBytesRead;
		while(nLen>0)
		{
			if (!WriteFile(hIn,data,nLen,&lBytesRead,0)) return FALSE;
			nLen-=lBytesRead;
		}
		return TRUE;
	}

	BOOL WritePipe(THProcessInfo *info,const void *data,UINT nLen)
	{
		if (!info || info->hIn==NULL) return FALSE;
		return WritePipe(info->hIn,data,nLen);
	}

	THProcessInfo *GetProcessInfoById(int nThreadId) {return (THProcessInfo *)GetThreadAddData(nThreadId);}
private:
	virtual void ThreadFunc(int threadid,void *adddata)
	{
		ParseCmd(threadid,(THProcessInfo *)adddata);
	}
	BOOL ParseCmd(int nThreadId,THProcessInfo *info)
	{
		if (!info) return FALSE;
		HANDLE hOut=NULL,hIn=NULL,hErr=NULL,hOutRead=NULL,hErrRead=NULL;
		BOOL bInheritHandle=FALSE;
		if (!info->bShow)
		{
			info->startupInfo.dwFlags=STARTF_USESHOWWINDOW;
			info->startupInfo.wShowWindow = SW_HIDE;
			SECURITY_ATTRIBUTES sa;
			sa.nLength=sizeof(SECURITY_ATTRIBUTES);
			sa.lpSecurityDescriptor=0;
			sa.bInheritHandle=TRUE;
			BOOL ret=CreatePipe(&hOutRead,&hOut,&sa,0);
			BOOL ret2=CreatePipe(&hIn,&info->hIn,&sa,0);
			BOOL ret3=CreatePipe(&hErrRead,&hErr,&sa,0);
			if (ret && ret2 && ret3)
			{
				bInheritHandle=TRUE;
				info->startupInfo.dwFlags|=STARTF_USESTDHANDLES;
				info->startupInfo.hStdInput=hIn;
				info->startupInfo.hStdOutput=hOut;
				info->startupInfo.hStdError=hErr;
			}
			else
			{
				if (hIn) CloseHandle(hIn);if (hOut) CloseHandle(hOut);if (hErr) CloseHandle(hErr);
				if (info->hIn) CloseHandle(info->hIn);if (hOutRead) CloseHandle(hOutRead);if (hErrRead) CloseHandle(hErrRead);
				hOutRead=NULL;
				hErrRead=NULL;
				info->hIn=NULL;
			}
		}
		DWORD nFlags=0;
		if (info->nDebugSleepTime!=0)
			nFlags|=DEBUG_PROCESS;
		if (!info->handler->OnProcessPreCreate(this,info,nThreadId))
		if (CreateProcess(NULL,info->sCmd.GetBuffer(), NULL, NULL, bInheritHandle, nFlags, NULL, NULL, &info->startupInfo, &info->processInfo))
		{
			if (info->dwPriority!=NORMAL_PRIORITY_CLASS)
				::SetPriorityClass(info->processInfo.hProcess, info->dwPriority);
			if (info->handler->OnProcessCreate(this,info,nThreadId))
				TerminateProcess(info->processInfo.hProcess, 0);
			else
			{
				DEBUG_EVENT dbe;
				while(1)
				{
					//check if process is call debug event
					if (info->nDebugSleepTime!=0)
					{
						if (WaitForDebugEvent(&dbe,info->nDebugSleepTime))
						{
							DWORD dwDbg=DBG_CONTINUE;
							if (dbe.dwDebugEventCode==EXCEPTION_DEBUG_EVENT)
								dwDbg=DBG_EXCEPTION_NOT_HANDLED;
							ContinueDebugEvent(dbe.dwProcessId, dbe.dwThreadId, dwDbg);
						}
					}
					//check if time over
					if (info->nLimitTime!=0 && GetTickCount()>info->nStartTime+info->nLimitTime)
					{
						TerminateProcess(info->processInfo.hProcess,0);
						break;
					}
					//check if use call CancelCmd to cancel the process
					if (info->bAsyc && WaitSecordWithKillSingal(nThreadId,info->nSleepTime))
					{
						TerminateProcess(info->processInfo.hProcess,0);
						break;
					}
					//get pipe info here
					if (hOutRead) _ReadPipe(hOutRead,&info->bufout);
					if (hErrRead) _ReadPipe(hErrRead,&info->buferr);
					if (WaitForSingleObject(info->processInfo.hProcess,1)==WAIT_TIMEOUT)
					{
						//check if Processing cancel it
						if (info->handler->OnProcessingOutput(this,info,nThreadId))
						{
							TerminateProcess(info->processInfo.hProcess, 0);
							break;
						}
					}
					else
					{
						//process run finish,get exit code
						if (GetExitCodeProcess(info->processInfo.hProcess,&info->nRetCode))
						{
							info->sCmd.ReleaseBuffer();
							if (info->pnRetCode) *info->pnRetCode=info->nRetCode;
							info->handler->OnProcessFinish(this,info,nThreadId,TRUE,info->nRetCode);
							if (hIn) CloseHandle(hIn);if (hOut) CloseHandle(hOut);if (hErr) CloseHandle(hErr);
							if (info->hIn) CloseHandle(info->hIn);if (hOutRead) CloseHandle(hOutRead);if (hErrRead) CloseHandle(hErrRead);
							info->hIn=NULL;
							delete info;
							return TRUE;
						}
					}
				}
			}
		}
		info->sCmd.ReleaseBuffer();
		info->handler->OnProcessFinish(this,info,nThreadId,FALSE,0);
		if (hIn) CloseHandle(hIn);if (hOut) CloseHandle(hOut);if (hErr) CloseHandle(hErr);
		if (info->hIn) CloseHandle(info->hIn);if (hOutRead) CloseHandle(hOutRead);if (hErrRead) CloseHandle(hErrRead);
		info->hIn=NULL;
		delete info;
		return FALSE;
	}

	void _ReadPipe(HANDLE hHandle,THMemBuf *mbuf)
	{
		DWORD lBytesRead=0;
		char buf[1024];
		BOOL ret=PeekNamedPipe(hHandle,buf,1024,&lBytesRead,0,0); 
		while(ret && lBytesRead)
		{
			if (!ReadFile(hHandle,buf,lBytesRead,&lBytesRead,0)) break;
			mbuf->AddBuf(buf,lBytesRead);
			ret=PeekNamedPipe(hHandle,buf,1024,&lBytesRead,0,0);
		}
	}
};
