#pragma once

#define THSYSTEM_INCLUDE_WINBASE
#include <THSystem.h>
#include <THThread.h>
#include <THMemBuf.h>

class THCmdProcess;
class ITHCmdProcessEvent;
typedef struct _THProcessInfo
{
	THString sCmd;						///<命令行
	BOOL bAsyc;							///<是否异步
	BOOL bShow;							///<是否隐藏
	DWORD nRetCode;						///<返回码
	DWORD *pnRetCode;					///<传入返回值指针
	ITHCmdProcessEvent *handler;		///<事件响应类
	void *pAddData;						///<传入的附加参数
	UINT nSleepTime;					///<轮询时间
	UINT nDebugSleepTime;				///<查错模式轮询时间
	DWORD nStartTime;					///<开始时间
	UINT nLimitTime;					///<限定取消时间
	DWORD dwPriority;					///<线程优先级
	STARTUPINFO startupInfo;			///<进程启动信息
	PROCESS_INFORMATION processInfo;	///<进程信息
	THMemBuf bufout;					///<输出管道信息缓冲
	THMemBuf buferr;					///<错误管道信息缓冲
	HANDLE hIn;							///<输入句柄
}THProcessInfo;

/**
* @brief 命令行处理事件响应类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-06-13 新建类
*/
/**<pre>
用法：
</pre>*/
class ITHCmdProcessEvent
{
public:
	/**
	* @brief 进程建立前回调
	* @param pProcess			处理器对象
	* @param info				进程信息，传入的附加参数，输出管道和出错管道信息
	* @param nThreadId			取消进程的id
	* @return 是否取消进程
	*/
	virtual BOOL OnProcessPreCreate(THCmdProcess *pProcess,THProcessInfo *info,int nThreadId)
	{
		return FALSE;
	}
	/**
	* @brief 进程建立回调
	* @param pProcess			处理器对象
	* @param info				进程信息，传入的附加参数，输出管道和出错管道信息
	* @param nThreadId			取消进程的id
	* @return 是否取消进程
	*/
	virtual BOOL OnProcessCreate(THCmdProcess *pProcess,THProcessInfo *info,int nThreadId)
	{
		return FALSE;
	}
	/**
	* @brief 处理中的回调
	* @param pProcess			处理器对象
	* @param info				进程信息，传入的附加参数，输出管道和出错管道信息
	* @param nThreadId			取消进程的id
	* @return 是否取消进程
	*/
	virtual BOOL OnProcessingOutput(THCmdProcess *pProcess,THProcessInfo *info,int nThreadId)
	{
		//info->bufout.Empty();
		//info->buferr.Empty();
		return FALSE;
	}

	/**
	* @brief 处理完成的回调，输出管道的信息在info->bufout，出错管道的信息在info->buferr
	* @param pProcess			处理器对象
	* @param info				进程信息
	* @param nThreadId			取消进程的id
	* @param bRunRet			运行结果
	* @param nRetCode			程序返回码
	*/
	virtual void OnProcessFinish(THCmdProcess *pProcess,THProcessInfo *info,int nThreadId,BOOL bRunRet,DWORD nRetCode){}
};

/**
* @brief 命令行处理类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-06-13 新建类
*/
/**<pre>
用法：
</pre>*/
class THCmdProcess : private ITHCmdProcessEvent ,private THThread
{
public:
	THCmdProcess(){}
	virtual ~THCmdProcess(){}

	/**
	* @brief 执行命令行程序
	* @param sCmd				命令行
	* @param bAsyc				是否异步，默认为等待到程序执行完成
	* @param pnRetCode			程序返回码
	* @param bShow				是否显示子线程，当不show时，默认挂接子进程输入输出，这时cfile类默认会继承句柄，这时会造成子进程不退出，文件一直占用的情况，因此其他的文件操作请指明CFile::modeNoInherit属性
	* @param nSleepTime			轮询时间
	* @param nLimitTime			限定取消时间，为0为总是等待，其他值为等待毫秒数
	* @param handler			事件响应器
	* @param pAddData			附加参数
	* @param dwPriority			进程优先级
	* @param bParseException	是否自行处理Exception，屏蔽非法操作对话框
	* @return 是否运行成功,返回0为失败,返回非0为线程id
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
