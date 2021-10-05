#pragma once

#include <THSystem.h>
#include <THStruct.h>
#include <THSyn.h>
#include <THTime.h>
//#include <THDebug.h>

class THThread;
class THTimerThread;
class THScheduleThread;

typedef void (*ThreadCallBack)(int ThreadId,THThread *pCls,void *data);
typedef void (*TimerThreadCallBack)(int TimerId,THTimerThread *pCls,void *data);
typedef void (*ScheduleThreadCallBack)(int ScheduleId,THScheduleThread *pCls,void *data);

#define TIMEVAL_NOTDELAY	1

//在不知道KillThread时是否自己就在ThreadFunc中时，可使用该宏
#define SAVEKILLTHREADOREXIT(tid)		{if (IsThreadInCurrentThread(tid))\
											{SelfKillThread(tid);return;}\
										else\
											KillThread(tid);\
										}

/**
* @brief 基础线程类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-05-28 新建类
* @2007-06-30 StartThread改为传入id为0时表示总是新建，返回值为线程id号
* @2007-08-03 StartThread函数加入同步锁，改正重用指针错误
* @2007-09-20 修正监听模式下退出未释放引起出错问题
*/
/**<pre>
使用Sample：
	//模式一：通过继承THThread类，实现ThreadFunc函数实现
	class XXX: public THThread
	{
		virtual void ThreadFunc(int threadid,void *adddata);
	}
	XXX a;
	//模式二：通过回调函数实现
	void MyThreadCallBack(int ThreadId,THThread *pCls,void *data)
	{
	}
	THThread a;
	a.SetCallFunc(MyThreadCallBack);
	//函数使用方法
	a.StartThread(1,NULL);
	a.StartThread(2,NULL,500);
	a.PauseThread(1);
	a.ResumeThread(1);
	a.SetThreadPriority(1);
	a.KillThread(1);
	a.SetThreadEvent(2);
	注意点：
	由于KillThread时会先向线程发送信号，在ThreadFunc中应尽可能快返回，不要进行Sleep，如间隔500毫秒进行操作可在StartThread时指定间隔500，ThreadFunc处理完即返回。
	如果必需在ThreadFunc中Sleep，请调用THThread::WaitSecordWithKillSingal进行Sleep，并处理返回值为True时，立即中断退出。
	如果ThreadFunc中过程阻塞，持续时间可能超过5秒时，请确认ThreadFunc中进行长时间操作前，把所有Lock先释放，因为KillThread时等待5秒线程不退出的话，将会强制结束线程，可能会造成之后的Unlock操作没有调用。
	ThreadFunc如果为阻塞的，尽量不能使用CString等类，因为在TerimThread时，会造成这些类的释构没有调用上
</pre>*/
class THThread
{
private:
	static void FreeCallBack(void *key,void *value,void *adddata)
	{
		ThreadData *info=(ThreadData *)value;
		THThread *p=(THThread *)adddata;
		if (info)
		{
			//THDebug(_T("THThread:TerminateThread:%x,%d,%x"),info,info->nThreadId,info->hThread);
			UINT nWaitForExitSec=info->nWaitForExitSec;
			info->nExit=1;
			DWORD exitcode;
			UINT timeout=0;
			while(info->hThread)
			{
				//THDebug(_T("THThread:Waitingfor:%x,%d,%x"),info,info->nThreadId,info->hThread);
				if (!GetExitCodeThread(info->hThread,&exitcode)) break;
				if (exitcode!=STILL_ACTIVE) break;
				if (info->hEvent) SetEvent(info->hEvent);
				Sleep(100);
				timeout+=100;
				if (timeout>=nWaitForExitSec)
				{
					//5 sec waiting,kill thread
					//THDebug(_T("Waiting Thread timeout 5 sec,TerminateThread:%x,%d,%x"),info,info->nThreadId,info->hThread);
					TerminateThread(info->hThread,0);
				}
			}
			//THDebug(_T("CMyThread:TerminateThreadSuccess:%x,%x,%d"),info,info->hThread,info->nThreadId);
			info->hThread=NULL;
			info->nThreadId=0;
			if (info->hEvent)
				CloseHandle(info->hEvent);
			/*if (info->hThread)
			{
				while(!::TerminateThread(info->hThread,0))
				SetEvent(info->hEvent);
			}*/
			delete info;//p->m_pool.Delete(info);
		}
	}
public:
	THThread(void)
	{
		m_cb=NULL;
		m_ThreadMap.SetFreeProc(FreeCallBack,this);
	}
	virtual ~THThread(void)
	{
		KillAllThread();
		//在FreeList时调用KillThread
/*		THPosition pos=m_ThreadMap.GetStartPosition();
		int key;
		
		while(!pos.IsEmpty())
		{
			m_ThreadMap.GetNextPosition(pos,key,info);
			if (info)
			{
				::TerminateThread(info->hThread,0);
				delete info;
			}
		}
		m_ThreadMap.RemoveAll();*/
	}

	/**
	* @brief 回调实现函数（当timeval为0时，该函数只调用一次，当为非0时，该函数wantforobject timeval后或收到事件后调用），使用类继承方式时，重载该函数
	* @param ThreadId	线程标识
	* @param data		附加数据
	*/
	virtual void ThreadFunc(int ThreadId,void *data)
	{
		if (m_cb) (m_cb)(ThreadId,this,data);
	}

	/**
	* @brief 设置回调函数，当不使用实现回调函数模式时使用
	* @param cb			回调函数
	*/
	void SetCallFunc(ThreadCallBack cb){m_cb=cb;}

	/**
	* @brief 获取从指定id开始的空闲id，用于在ThreadFunc中用ThreadId范围区分不同功能线程
	* @param StartId		指定id
	* @return 空闲id
	*/
	int GetAvailableId(int StartId)
	{
		THSingleLock lock(&m_mutex);
		ThreadData *info1=NULL;
		while(m_ThreadMap.GetAt(StartId,info1)) StartId++;
		return StartId;
	}

	/**
	* @brief 启动线程
	* @param ThreadId	线程标识，重复标识会启动失败，如果为0，表示总是新开
	* @param adddata	附加数据
	* @param timeval	时间间隔，该值为0时，THThread不使用内置的WaitForObject和Event来调用，这时ThreadFunc只调用一次，如果该值非0，THThread WaitforObject并且可以通用event触发，如为TIMEVAL_NOTDELAY=1不进行WaitForObject，直接调用
	* @param bDelay		当含有时间间隔时，该值表示是否进行一次时间间隔才调用ThreadFunc
	* @param nStackSize	栈大小，默认为0
	* @return			返回线程号，返回0表示出错
	*/
	inline int StartThread(int ThreadId=0,void *adddata=NULL,DWORD timeval=0,BOOL bDelay=FALSE,int nStackSize=0)
	{
		if (timeval==TIMEVAL_NOTDELAY)
			return StartThread2(ThreadId,adddata,0,bDelay?timeval:0,TRUE,nStackSize);
		return StartThread2(ThreadId,adddata,timeval,bDelay?timeval:0,timeval?TRUE:FALSE,nStackSize);
	}

	/**
	* @brief 启动线程接口2
	* @param ThreadId	线程标识，重复标识会启动失败，如果为0，表示总是新开
	* @param adddata	附加数据
	* @param timeval	时间间隔，该值为0时，THThread不使用内置的WaitForObject和Event来调用，如果该值非0，THThread WaitforObject并且可以通用event触发
	* @param nDelay		当含有时间间隔时，该值表示是否进行一次时间间隔才调用ThreadFunc
	* @param bLoop		是否循环
	* @param nStackSize	栈大小，默认为0
	* @return			返回线程号，返回0表示出错
	*/
	int StartThread2(int ThreadId=0,void *adddata=NULL,DWORD timeval=0,DWORD nDelay=0,BOOL bLoop=TRUE,int nStackSize=0)
	{
		THSingleLock lock(&m_mutex);
		//already exist
		ThreadData *info;
		if (ThreadId!=0 && m_ThreadMap.GetAt(ThreadId,info)) return 0;
		info=new ThreadData;//m_pool.New();
		if (!info) return 0;
		ThreadData *info1=NULL;
		if (ThreadId==0)
		{
			ThreadId=GetThreadCount()+1;
			while(m_ThreadMap.GetAt(ThreadId,info1)) ThreadId++;
		}
		//THDebug(_T("THThread:start thread:%d"),ThreadId);
		m_ThreadMap.SetAt(ThreadId,info);
		info->nThreadId=ThreadId;
		info->tid=0;
		info->data=adddata;
		info->pthread=this;
		info->timeval=timeval;
		info->nDelay=nDelay;
		info->bLoop=bLoop;
		info->nExit=0;
		info->nWaitForExitSec=5000;
		info->bProcessing=FALSE;
		if (timeval==0 && nDelay==0)
			info->hEvent=NULL;
		else
			info->hEvent=CreateEvent(NULL,FALSE,FALSE,NULL);
		//改用beginthread,CreateThread有可能造成内存泄露
		//If the thread generated by _beginthread exits quickly, the handle returned to the caller of _beginthread might be invalid or, worse, point to another thread.
		info->hThread=(HANDLE)_beginthreadex(NULL,nStackSize,MyCreateThread,(LPVOID)info,0,NULL);
		//info->hThread=(HANDLE)_beginthread(MyCreateThread,0,(LPVOID)info);
		if (info->hThread==NULL/* || info->hThread==1L*/)//beginthread return 1L for fail,beginthreadex return 0L
		//if (!info->hThread)
			KillThread(ThreadId);
		return (info->hThread)?ThreadId:0;
	}
	/**
	* @brief 暂停线程
	* @param ThreadId	线程标识
	* @return			是否成功
	*/
	BOOL PauseThread(int ThreadId)
	{
		THSingleLock lock(&m_mutex);
		ThreadData *info;
		BOOL bpret=m_ThreadMap.GetAt(ThreadId,info);
		if (!bpret || !info) return FALSE;
		::SuspendThread(info->hThread);
		return TRUE;
	}
	/**
	* @brief 恢复线程
	* @param ThreadId	线程标识
	* @return			是否成功
	*/
	BOOL ResumeThread(int ThreadId)
	{
		THSingleLock lock(&m_mutex);
		ThreadData *info;
		BOOL bpret=m_ThreadMap.GetAt(ThreadId,info);
		if (!bpret || !info) return FALSE;
		::ResumeThread(info->hThread);
		return TRUE;
	}

	/**
	* @brief 结束线程，在线程中时，调用KillThread将会被阻塞，这时应该调用SelfKillThread
	* @param ThreadId	线程标识
	* @param nWaitForExitSec	等待结束线程时间
	* @return			是否成功
	*/
	BOOL KillThread(int ThreadId,UINT nWaitForExitSec=5000)
	{
		//THSingleLock lock(&m_mutex);
		ThreadData *info;
		BOOL bpret=m_ThreadMap.GetAt(ThreadId,info);
		if (!bpret || !info) return FALSE;
		if (info->tid==GetCurrentThreadId())
		{
			//如果这里assert了说明该KillThread代码调用者为ThreadFunc本身，虽然KillThread依然可以生效，但是是使用TerminateThread结束的，这时容易使Thread中的类等不能调用释构函数引起错误
			//标准做法应为先判断是否在ThreadFunc中，可使用函数IsThreadInCurrentThread判断，如果是本线程中的KillThread，使用SelfKillThread置结束位，并立即return
			//如果怕麻烦，可以使用SAVEKILLTHREADOREXIT宏
			ASSERT(FALSE);
			info->nWaitForExitSec=0;
		}
		else
			info->nWaitForExitSec=nWaitForExitSec;
		//THDebug(_T("THThread:kill thread:%d"),ThreadId);
		//在ListFree时KillThread
		//现在主要问题在于map类的自释放调用，因此不能lock，否则可能lock
		return m_ThreadMap.RemoveAt(ThreadId);
		//remove function will call it
		//::TerminateThread(info->hThread,0);
		//Sleep(500);
		//delete info;
		//return TRUE;
	}

	/*
	* @brief 在线程中时，调用KillThread将会被阻塞，这时应该调用SelfKillThread，调用完后，应该return退出线程体
	* @param ThreadId	线程标识
	* @return			是否成功
	*/
	BOOL SelfKillThread(int ThreadId)
	{
		THSingleLock lock(&m_mutex);
		ThreadData *info;
		BOOL bpret=m_ThreadMap.GetAt(ThreadId,info);
		if (!bpret || !info) return FALSE;
		info->nExit=2;
		return TRUE;
	}

	BOOL IsThreadInCurrentThread(int ThreadId)
	{
		THSingleLock lock(&m_mutex);
		ThreadData *info;
		BOOL bpret=m_ThreadMap.GetAt(ThreadId,info);
		if (!bpret || !info) return FALSE;
		return info->tid==GetCurrentThreadId();
	}

	/**
	* @brief 结束所有线程
	* @return			是否成功
	*/
	void KillAllThread()
	{
		m_ThreadMap.RemoveAll();
	}

	/**
	* @brief 获取当前线程数
	* @return			当前线程数
	*/
	UINT GetThreadCount() {return m_ThreadMap.GetCount();}

	/**
	* @brief 发送线程信号
	* @param ThreadId	线程标识
	* @return			是否成功
	*/
	BOOL SetThreadEvent(int ThreadId)
	{
		//THSingleLock lock(&m_mutex);
		ThreadData *info;
		BOOL bpret=m_ThreadMap.GetAt(ThreadId,info);
		if (!bpret || !info) return FALSE;
		return SetEvent(info->hEvent);
	}

	/**
	* @brief 发送线程池信号，查找空闲的线程，并向其发送信号
	* @param nStartThread	指定从哪个Thread开始，0为全部，开始查找线程为nStartThread+1
	* @return			响应的线程id，返回0为发送不成功
	*/
	int SetThreadPoolEvent(int nStartThread=0)
	{
		THSingleLock lock(&m_mutex);
		THPosition pos=m_ThreadMap.GetStartPosition();
		ThreadData *info;
		int nThreadId;
		while(!pos.IsEmpty())
		{
			if (m_ThreadMap.GetNextPosition(pos,nThreadId,info))
			{
				if (nThreadId>nStartThread && info && info->hEvent && info->bProcessing==FALSE)
				{
					if (SetEvent(info->hEvent))
					{
						//这里需要Sleep，否则会自己先把信号量接上了
						Sleep(50);
						//test the event will pass to myself?
						//it means the thread is processing,busy now
						DWORD dw=WaitForSingleObject(info->hEvent,0);
						if (dw!=WAIT_OBJECT_0 && dw!=WAIT_FAILED)
							return nThreadId;
					}
				}
			}
		}
		return 0;
	}

	/**
	* @brief 设置线程优先级
	* @param ThreadId	线程标识
	* @param Priority	线程优先级，THREAD_PRIORITY_XXX
	* @return			是否成功
	*/
	BOOL SetThreadPriority(int ThreadId,int Priority)
	{
		THSingleLock lock(&m_mutex);
		ThreadData *info;
		BOOL bpret=m_ThreadMap.GetAt(ThreadId,info);
		if (!bpret || !info) return FALSE;
		return ::SetThreadPriority(info->hThread,Priority);
	}

	/**
	* @brief 判断Thread是否正在运行
	* @param ThreadId	线程标识
	* @return			是否正在运行
	*/
	BOOL IsThreadRunning(int ThreadId)
	{
		THSingleLock lock(&m_mutex);
		ThreadData *info;
		BOOL bpret=m_ThreadMap.GetAt(ThreadId,info);
		if (!bpret || !info) return FALSE;
		return TRUE;
	}

	/**
	* @brief 获取线程的附加信息
	* @param ThreadId	线程标识
	* @return			是否正在运行
	*/
	void *GetThreadAddData(int ThreadId)
	{
		THSingleLock lock(&m_mutex);
		ThreadData *info;
		BOOL bpret=m_ThreadMap.GetAt(ThreadId,info);
		if (!bpret || !info) return FALSE;
		return info->data;
	}

	/**
	* @brief 等待线程并监听结束线程信号,一般在线程中调用
	* @param ThreadId	线程标识
	* @return			是否线程需要退出
	*/
	BOOL WaitSecordWithKillSingal(int ThreadId,UINT nWaitSec)
	{
		m_mutex.Lock();
		ThreadData *info;
		BOOL bpret=m_ThreadMap.GetAt(ThreadId,info);
		if (!bpret || !info)
		{
			m_mutex.Unlock();
			return TRUE;
		}
		if (info->nExit)
		{
			m_mutex.Unlock();
			return TRUE;
		}
		HANDLE event=info->hEvent;
		m_mutex.Unlock();
		if (event)
			::WaitForSingleObject(event,nWaitSec);
		else
			//the Thread Not Have Singal,only use Sleep to sleep
			Sleep(nWaitSec);
		m_mutex.Lock();
		bpret=m_ThreadMap.GetAt(ThreadId,info);
		if (!bpret || !info)
		{
			m_mutex.Unlock();
			return TRUE;
		}
		bpret=info->nExit;
		m_mutex.Unlock();
		return bpret;
	}

	int GetFristThreadId()
	{
		THPosition pos=m_ThreadMap.GetStartPosition();
		if (pos.IsEmpty()) return 0;
		int ThreadId;
		ThreadData *data;
		if (!m_ThreadMap.GetNextPosition(pos,ThreadId,data)) return 0;
		return ThreadId;
	}
private:
	typedef struct _ThreadData{
		volatile HANDLE hThread;
		void *data;
		THThread *pthread;
		int nThreadId;
		HANDLE hEvent;
		DWORD timeval;
		DWORD tid;
		int nDelay;
		BOOL bLoop;
		volatile int nExit;
		volatile UINT nWaitForExitSec;
		volatile BOOL bProcessing;
	}ThreadData;
	THMap<int,ThreadData *> m_ThreadMap;
	//THMemPool<ThreadData> m_pool;
	ThreadCallBack m_cb;
	THMutex m_mutex;

	static unsigned __stdcall MyCreateThread(LPVOID lpParam)
	{
		ThreadData *data=(ThreadData *)lpParam;
		data->tid=GetCurrentThreadId();
		int threadid=data->nThreadId;
		srand((unsigned int)time(NULL)+threadid);
		//THDebug(_T("THThread:thread route:%x,%d,%x"),data,data->nThreadId,data->hThread);
		if (data->nDelay)
			::WaitForSingleObject(data->hEvent,data->nDelay);
		if (data->bLoop==FALSE)
		{
			if (data->nExit==0)
			{
				data->bProcessing=TRUE;
				data->pthread->ThreadFunc(data->nThreadId,data->data);
				data->bProcessing=FALSE;
			}
		}
		else
		{
			while(1)
			{
				if (data->nExit) break;
				data->bProcessing=TRUE;
				data->pthread->ThreadFunc(data->nThreadId,data->data);
				data->bProcessing=FALSE;
				if (data->nExit) break;
				if (data->timeval)
					::WaitForSingleObject(data->hEvent,data->timeval);
			}
		}
		if (data->nThreadId==threadid && data->hThread)
		{
			//not free the item yet
			//THDebug(_T("CMyThread:thread route kill:%x,%d"),data,threadid);
			//when finish,reset the state
			data->hThread=NULL;
			if (data->nExit==0 || data->nExit==2)
				data->pthread->m_ThreadMap.RemoveAt(threadid);
		}
		_endthreadex(0);
		return 0;
		//data->pthread->m_pool.Delete(data);
	}
};

/**
* @brief 计时器线程类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-07-30 新建类
* @2007-08-29 改为由GetTickCount计时，原来的计时误差较高，计时精度分不同级别
*/
/**<pre>
使用Sample：
	//模式一：通过继承THTimerThread类，实现ThreadFunc函数实现
	class XXX: public THTimerThread
	{
		virtual void ThreadFunc(int timerid,void *adddata);
	}
	XXX a;
	//模式二：通过回调函数实现
	THTimerThread a;
	a.SetCallFunc(MyThreadCallBack);
	//函数使用方法
	a.StartTimer(1,100);
	a.ResetTimer(1);
</pre>*/
class THTimerThread
{
private:
	static void MyThreadCallBack(int ThreadId,THThread *pCls,void *data)
	{
		THTimerThread *p=(THTimerThread *)data;
		if (p)
		{
			ThreadData *info;
			BOOL bpret=p->m_ThreadMap.GetAt(ThreadId,info);
			if (bpret && info)
			{
				DWORD dwTickCount=::GetTickCount();
				//info->count++;
				if (info->count<=dwTickCount)
				{
					//finish call back
					p->ThreadFunc(ThreadId,info->adddata);
					if (info->bloop)
						info->count=dwTickCount+info->timeval;
					else
						p->m_ThreadMap.RemoveAt(ThreadId);
				}
			}
			else
				p->m_thread.KillThread(ThreadId);
		}
	}

	static void FreeCallBack(void *key,void *value,void *adddata)
	{
		ThreadData *info=(ThreadData *)value;
		THTimerThread *p=(THTimerThread *)adddata;
		if (info)
		{
			p->m_thread.KillThread(info->TimerId);
			delete info;//p->m_pool.Delete(info);
		}
	}
public:
	THTimerThread()
	{
		m_cb=NULL;
		m_thread.SetCallFunc(MyThreadCallBack);
		m_ThreadMap.SetFreeProc(FreeCallBack,this);
	}
	virtual ~THTimerThread()
	{
		StopAllTimer();
	}

	/**
	* @brief 回调实现函数，使用类继承方式时，重载该函数
	* @param TimerId	定时器标识
	* @param data		附加数据
	*/
	virtual void ThreadFunc(int TimerId,void *data)
	{
		if (m_cb) (m_cb)(TimerId,this,data);
	}

	/**
	* @brief 设置回调函数，当不使用实现回调函数模式时使用
	* @param cb			回调函数
	*/
	void SetCallFunc(TimerThreadCallBack cb){m_cb=cb;}

	/**
	* @brief 开始一个新的定时器
	* @param TimerId	定时器号，如果已有对应定时器号，返回失败；如果为0，表示不需要指定
	* @param TimerVal	定时器间隔，单位毫秒，在10000（10秒）内定时器最小精度为50毫秒，30秒内为500豪秒，否则为2秒
	* @param bLoop		是否循环定时
	* @param adddata	附加数据
	* @return			定时器号
	*/
	int StartTimer(int TimerId,int TimerVal,BOOL bLoop=FALSE,void *adddata=NULL)
	{
		if (TimerVal<=10000)
			TimerId=m_thread.StartThread(TimerId,this,50,TRUE);
		else if (TimerVal<=30000)
			TimerId=m_thread.StartThread(TimerId,this,500,TRUE);
		else
			TimerId=m_thread.StartThread(TimerId,this,2000,TRUE);
		if (TimerId==0) return TimerId;
		ThreadData *info;
		if (!m_ThreadMap.GetAt(TimerId,info))
		{
			info=new ThreadData;//m_pool.New();
			if (!info)
			{
				m_thread.KillThread(TimerId);
				return 0;
			}
			m_ThreadMap.SetAt(TimerId,info);
		}
		info->bloop=bLoop;
		info->timeval=TimerVal;
		info->count=GetTickCount()+TimerVal;
		info->TimerId=TimerId;
		info->adddata=adddata;
		return TimerId;
	}

	/**
	* @brief 停止定时器
	* @param TimerId	定时器号
	* @return			是否成功
	*/
	BOOL StopTimer(int TimerId)
	{
		ThreadData *info;
		BOOL bpret=m_ThreadMap.GetAt(TimerId,info);
		if (!bpret || !info) return FALSE;
		//在ListFree时KillThread
		return m_ThreadMap.RemoveAt(TimerId);
	}

	/**
	* @brief 停止所有定时器
	*/
	void StopAllTimer()
	{
		m_ThreadMap.RemoveAll();
	}

	/**
	* @brief 清零定时器计数
	* @param TimerId	定时器号
	* @param TimerVal	如果为0，表示清零到初始值重新计数；如果为正值，表示延长多少定时时间；负值表示减少多少定时时间。单位为毫秒
	* @return			是否成功
	*/
	BOOL ResetTimer(int TimerId,int TimerVal=0)
	{
		ThreadData *info;
		BOOL bpret=m_ThreadMap.GetAt(TimerId,info);
		if (!bpret || !info) return FALSE;
		if (TimerVal==0)
			info->count=GetTickCount()+info->timeval;
		else
			info->count+=TimerVal;
		return TRUE;
	}

	/**
	* @brief 判断Thread是否正在运行
	* @param ThreadId	线程标识
	* @return			是否正在运行
	*/
	BOOL IsThreadRunning(int ThreadId)
	{
		return m_thread.IsThreadRunning(ThreadId);
	}
private:
	typedef struct _TimerData{
		int TimerId;
		int timeval;
		volatile DWORD count;
		BOOL bloop;
		void *adddata;
	}ThreadData;
	TimerThreadCallBack m_cb;
	THMap<int,ThreadData *> m_ThreadMap;
	//THMemPool<ThreadData> m_pool;
	THThread m_thread;
};

#define SCHEDULEMODE_SECORD	0x1
#define SCHEDULEMODE_MINUTE	0x2
#define SCHEDULEMODE_HOUR	0x4
#define SCHEDULEMODE_DAY	0x8
#define SCHEDULEMODE_WEEK	0x10
#define SCHEDULEMODE_MONTH	0x20

/**
* @brief 计划任务类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-11-22 新建类
*/
/**<pre>
使用Sample：
	//模式一：通过继承THScheduleThread类，实现ThreadFunc函数实现
	class XXX: public THScheduleThread
	{
		virtual void ScheduleFunc(int scheduleid,void *adddata);
	}
	XXX a;
	//模式二：通过回调函数实现
	THScheduleThread a;
	a.SetCallFunc(MyThreadCallBack);
	//函数使用方法
	a.NewSchedule();
</pre>*/
class THScheduleThread
{
private:
	static void MyThreadCallBack(int ThreadId,THThread *pCls,void *data)
	{
		THScheduleThread *p=(THScheduleThread *)data;
		if (p)
		{
			ThreadData *info;
			BOOL bpret=p->m_ThreadMap.GetAt(ThreadId,info);
			if (bpret && info)
			{
				BOOL bOk=FALSE;
				THTime ti;
				if (info->nSchedule&SCHEDULEMODE_SECORD)
				{
					if (ti.GetSecond()==info->tBase.GetSecond() && ti.GetMinute()!=info->nLast)
					{
						info->nLast=ti.GetMinute();
						bOk=TRUE;
					}
				}
				else if (info->nSchedule&SCHEDULEMODE_MINUTE)
				{
					if (ti.GetMinute()==info->tBase.GetMinute() && ti.GetHour()!=info->nLast)
					{
						info->nLast=ti.GetHour();
						bOk=TRUE;
					}
				}
				else if (info->nSchedule&SCHEDULEMODE_HOUR)
				{
					if (ti.GetHour()==info->tBase.GetHour() && ti.GetDay()!=info->nLast)
					{
						info->nLast=ti.GetDay();
						bOk=TRUE;
					}
				}
				else if (info->nSchedule&SCHEDULEMODE_DAY)
				{
					if (ti.GetDay()==info->tBase.GetDay() && ti.GetMonth()!=info->nLast)
					{
						info->nLast=ti.GetMonth();
						bOk=TRUE;
					}
				}
				else if (info->nSchedule&SCHEDULEMODE_WEEK)
				{
					if (ti.GetDayOfWeek()==info->tBase.GetDayOfWeek() && ti.GetMonth()!=info->nLast)
					{
						info->nLast=ti.GetMonth();
						bOk=TRUE;
					}
				}
				else if (info->nSchedule&SCHEDULEMODE_MONTH)
				{
					if (ti.GetMonth()==info->tBase.GetMonth() && ti.GetYear()!=info->nLast)
					{
						info->nLast=ti.GetYear();
						bOk=TRUE;
					}
				}
				if (bOk)
				{
					//finish call back
					p->ScheduleFunc(ThreadId,info->adddata);
					if (!info->bloop)
						p->m_ThreadMap.RemoveAt(ThreadId);
				}
			}
			else
				p->m_thread.KillThread(ThreadId);
		}
	}

	static void FreeCallBack(void *key,void *value,void *adddata)
	{
		ThreadData *info=(ThreadData *)value;
		THScheduleThread *p=(THScheduleThread *)adddata;
		if (info)
		{
			p->m_thread.KillThread(info->ScheduleId);
			delete info;//p->m_pool.Delete(info);
		}
	}
public:
	THScheduleThread()
	{
		m_cb=NULL;
		m_thread.SetCallFunc(MyThreadCallBack);
		m_ThreadMap.SetFreeProc(FreeCallBack,this);
	}
	virtual ~THScheduleThread()
	{
		StopAllSchedule();
	}

	/**
	* @brief 回调实现函数，使用类继承方式时，重载该函数
	* @param TimerId	定时器标识
	* @param data		附加数据
	*/
	virtual void ScheduleFunc(int TimerId,void *data)
	{
		if (m_cb) (m_cb)(TimerId,this,data);
	}

	/**
	* @brief 设置回调函数，当不使用实现回调函数模式时使用
	* @param cb			回调函数
	*/
	void SetCallFunc(ScheduleThreadCallBack cb){m_cb=cb;}

	/**
	* @brief 开始一个新的计划任务
	* @param ScheduleId	计划任务号，如果已有对应号，返回失败；如果为0，表示不需要指定
	* @param tBase		计划任务时间
	* @param nScheduleMode 计划任务形式，默认为小时匹配，即每天
	* @param nDefVal	定时精确度，默认1分钟，注意精度太小可能会引起丢失掉某次响应
	* @param bLoop		是否循环
	* @param adddata	附加数据
	* @return			计划任务号，0为失败
	*/
	int NewSchedule(int ScheduleId,THTime tBase,int nScheduleMode=SCHEDULEMODE_HOUR,int nDefVal=60000,BOOL bLoop=TRUE,void *adddata=NULL)
	{
		ScheduleId=m_thread.StartThread(ScheduleId,this,nDefVal,TRUE);
		if (ScheduleId==0) return ScheduleId;
		ThreadData *info;
		if (!m_ThreadMap.GetAt(ScheduleId,info))
		{
			info=new ThreadData;//m_pool.New();
			if (!info)
			{
				m_thread.KillThread(ScheduleId);
				return 0;
			}
			m_ThreadMap.SetAt(ScheduleId,info);
		}
		info->bloop=bLoop;
		info->tBase=tBase;
		info->nSchedule=nScheduleMode;
		info->ScheduleId=ScheduleId;
		info->adddata=adddata;
		info->nLast=-1;//用于确保不重入
		return ScheduleId;
	}

	/**
	* @brief 停止计划任务
	* @param ScheduleId	计划任务号
	* @return			是否成功
	*/
	BOOL StopSchedule(int ScheduleId)
	{
		ThreadData *info;
		BOOL bpret=m_ThreadMap.GetAt(ScheduleId,info);
		if (!bpret || !info) return FALSE;
		//在ListFree时KillThread
		return m_ThreadMap.RemoveAt(ScheduleId);
	}

	/**
	* @brief 停止所有计划任务
	*/
	void StopAllSchedule()
	{
		m_ThreadMap.RemoveAll();
	}

	/**
	* @brief 判断计划任务是否正在运行
	* @param ScheuleId	计划任务号
	* @return 是否正在运行
	*/
	BOOL IsScheuleRunning(int ScheuleId)
	{
		return m_thread.IsThreadRunning(ScheuleId);
	}
private:
	typedef struct _ScheduleData{
		int ScheduleId;
		int nSchedule;
		int nLast;
		THTime tBase;
		BOOL bloop;
		void *adddata;
	}ThreadData;
	ScheduleThreadCallBack m_cb;
	THMap<int,ThreadData *> m_ThreadMap;
	//THMemPool<ThreadData> m_pool;
	THThread m_thread;
};

/**
* @brief 异步函数调用类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-09-27 新建类
*/
/**<pre>
使用Sample：
	//通过继承THAsynCall类实现
	class XXX: public THAsynCall<XXX>
	{
		virtual void ThreadFunc(int timerid,void *adddata);
		void Test();
		void Test1(LPVOID p);
	}
	//函数使用方法
	StartAsynCall(&XXX::Test);
	StartAsynCall1(&XXX::Test1,p);
</pre>*/
template<typename TKEY>
class THAsynCall
{
public:
	typedef void (TKEY::*THAsynCallFnVoid)(void);
	typedef void (TKEY::*THAsynCallFnParam)(LPVOID adddata);
	typedef void (TKEY::*THAsynCallFnParam2)(LPVOID adddata,LPVOID adddata2);
	typedef void (TKEY::*THAsynCallFnParam3)(LPVOID adddata,LPVOID adddata2,LPVOID adddata3);
	typedef void (TKEY::*THAsynCallFnParam4)(LPVOID adddata,LPVOID adddata2,LPVOID adddata3,LPVOID adddata4);
	typedef void (TKEY::*THAsynCallFnParam5)(LPVOID adddata,LPVOID adddata2,LPVOID adddata3,LPVOID adddata4,LPVOID adddata5);
	typedef void (TKEY::*THAsynCallFnParam6)(LPVOID adddata,LPVOID adddata2,LPVOID adddata3,LPVOID adddata4,LPVOID adddata5,LPVOID adddata6);

	BOOL StartAsynCall(THAsynCallFnVoid fn)
	{
		AsynCallInfo *tmp=new AsynCallInfo;
		if (!tmp) return FALSE;
		tmp->p=this;
		tmp->fn=fn;
		return _beginthreadex(NULL,0,MyCreateThread,(LPVOID)tmp,0,NULL)!=NULL;
	}

	BOOL StartAsynCall1(THAsynCallFnParam fn,LPVOID adddata)
	{
		AsynCallInfo *tmp=new AsynCallInfo;
		if (!tmp) return FALSE;
		tmp->p=this;
		tmp->fn=NULL;
		tmp->fn1=fn;
		tmp->adddata=adddata;
		return _beginthreadex(NULL,0,MyCreateThread,(LPVOID)tmp,0,NULL)!=NULL;
	}

	BOOL StartAsynCall2(THAsynCallFnParam2 fn,LPVOID adddata,LPVOID adddata2)
	{
		AsynCallInfo *tmp=new AsynCallInfo;
		if (!tmp) return FALSE;
		tmp->p=this;
		tmp->fn=NULL;
		tmp->fn1=NULL;
		tmp->fn2=fn;
		tmp->adddata=adddata;
		tmp->adddata2=adddata2;
		return _beginthreadex(NULL,0,MyCreateThread,(LPVOID)tmp,0,NULL)!=NULL;
	}

	BOOL StartAsynCall3(THAsynCallFnParam3 fn,LPVOID adddata,LPVOID adddata2,LPVOID adddata3)
	{
		AsynCallInfo *tmp=new AsynCallInfo;
		if (!tmp) return FALSE;
		tmp->p=this;
		tmp->fn=NULL;
		tmp->fn1=NULL;
		tmp->fn2=NULL;
		tmp->fn3=fn;
		tmp->adddata=adddata;
		tmp->adddata2=adddata2;
		tmp->adddata3=adddata3;
		return _beginthreadex(NULL,0,MyCreateThread,(LPVOID)tmp,0,NULL)!=NULL;
	}

	BOOL StartAsynCall4(THAsynCallFnParam4 fn,LPVOID adddata,LPVOID adddata2,LPVOID adddata3,LPVOID adddata4)
	{
		AsynCallInfo *tmp=new AsynCallInfo;
		if (!tmp) return FALSE;
		tmp->p=this;
		tmp->fn=NULL;
		tmp->fn1=NULL;
		tmp->fn2=NULL;
		tmp->fn3=NULL;
		tmp->fn4=fn;
		tmp->adddata=adddata;
		tmp->adddata2=adddata2;
		tmp->adddata3=adddata3;
		tmp->adddata4=adddata4;
		DWORD tid=0;
		return _beginthreadex(NULL,0,MyCreateThread,(LPVOID)tmp,0,NULL)!=NULL;
	}

	BOOL StartAsynCall5(THAsynCallFnParam5 fn,LPVOID adddata,LPVOID adddata2,LPVOID adddata3,LPVOID adddata4,LPVOID adddata5)
	{
		AsynCallInfo *tmp=new AsynCallInfo;
		if (!tmp) return FALSE;
		tmp->p=this;
		tmp->fn=NULL;
		tmp->fn1=NULL;
		tmp->fn2=NULL;
		tmp->fn3=NULL;
		tmp->fn4=NULL;
		tmp->fn5=fn;
		tmp->adddata=adddata;
		tmp->adddata2=adddata2;
		tmp->adddata3=adddata3;
		tmp->adddata4=adddata4;
		tmp->adddata5=adddata5;
		DWORD tid=0;
		return _beginthreadex(NULL,0,MyCreateThread,(LPVOID)tmp,0,NULL)!=NULL;
	}

	BOOL StartAsynCall6(THAsynCallFnParam6 fn,LPVOID adddata,LPVOID adddata2,LPVOID adddata3,LPVOID adddata4,LPVOID adddata5,LPVOID adddata6)
	{
		AsynCallInfo *tmp=new AsynCallInfo;
		if (!tmp) return FALSE;
		tmp->p=this;
		tmp->fn=NULL;
		tmp->fn1=NULL;
		tmp->fn2=NULL;
		tmp->fn3=NULL;
		tmp->fn4=NULL;
		tmp->fn5=NULL;
		tmp->fn6=fn;
		tmp->adddata=adddata;
		tmp->adddata2=adddata2;
		tmp->adddata3=adddata3;
		tmp->adddata4=adddata4;
		tmp->adddata5=adddata5;
		tmp->adddata6=adddata6;
		DWORD tid=0;
		return _beginthreadex(NULL,0,MyCreateThread,(LPVOID)tmp,0,NULL)!=NULL;
	}
private:
	static unsigned int __stdcall MyCreateThread(LPVOID lpParam)
	{
		AsynCallInfo *tmp=(AsynCallInfo *)lpParam;
		if (tmp)
		{
			TKEY *t=(TKEY*)tmp->p;
			if (tmp->fn)
			{
				THAsynCallFnVoid fn=tmp->fn;
				delete tmp;
				(t->*fn)();
			}
			else if (tmp->fn1)
			{
				THAsynCallFnParam fn=tmp->fn1;
				LPVOID adddata=tmp->adddata;
				delete tmp;
				(t->*fn)(adddata);
			}
			else if (tmp->fn2)
			{
				THAsynCallFnParam2 fn=tmp->fn2;
				LPVOID adddata=tmp->adddata;
				LPVOID adddata2=tmp->adddata2;
				delete tmp;
				(t->*fn)(adddata,adddata2);
			}
			else if (tmp->fn3)
			{
				THAsynCallFnParam3 fn=tmp->fn3;
				LPVOID adddata=tmp->adddata;
				LPVOID adddata2=tmp->adddata2;
				LPVOID adddata3=tmp->adddata3;
				delete tmp;
				(t->*fn)(adddata,adddata2,adddata3);
			}
			else if (tmp->fn4)
			{
				THAsynCallFnParam4 fn=tmp->fn4;
				LPVOID adddata=tmp->adddata;
				LPVOID adddata2=tmp->adddata2;
				LPVOID adddata3=tmp->adddata3;
				LPVOID adddata4=tmp->adddata4;
				delete tmp;
				(t->*fn)(adddata,adddata2,adddata3,adddata4);
			}
			else if (tmp->fn5)
			{
				THAsynCallFnParam5 fn=tmp->fn5;
				LPVOID adddata=tmp->adddata;
				LPVOID adddata2=tmp->adddata2;
				LPVOID adddata3=tmp->adddata3;
				LPVOID adddata4=tmp->adddata4;
				LPVOID adddata5=tmp->adddata5;
				delete tmp;
				(t->*fn)(adddata,adddata2,adddata3,adddata4,adddata5);
			}
			else
			{
				THAsynCallFnParam6 fn=tmp->fn6;
				LPVOID adddata=tmp->adddata;
				LPVOID adddata2=tmp->adddata2;
				LPVOID adddata3=tmp->adddata3;
				LPVOID adddata4=tmp->adddata4;
				LPVOID adddata5=tmp->adddata5;
				LPVOID adddata6=tmp->adddata6;
				delete tmp;
				(t->*fn)(adddata,adddata2,adddata3,adddata4,adddata5,adddata6);
			}
		}
		_endthreadex(0);
		return 0;
	}
	typedef struct _AsynCallInfo{
		THAsynCall *p;
		THAsynCallFnVoid fn;
		THAsynCallFnParam fn1;
		THAsynCallFnParam2 fn2;
		THAsynCallFnParam3 fn3;
		THAsynCallFnParam4 fn4;
		THAsynCallFnParam5 fn5;
		THAsynCallFnParam6 fn6;
		LPVOID adddata;
		LPVOID adddata2;
		LPVOID adddata3;
		LPVOID adddata4;
		LPVOID adddata5;
		LPVOID adddata6;
	}AsynCallInfo;
};
