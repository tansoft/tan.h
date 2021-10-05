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

//�ڲ�֪��KillThreadʱ�Ƿ��Լ�����ThreadFunc��ʱ����ʹ�øú�
#define SAVEKILLTHREADOREXIT(tid)		{if (IsThreadInCurrentThread(tid))\
											{SelfKillThread(tid);return;}\
										else\
											KillThread(tid);\
										}

/**
* @brief �����߳���
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-05-28 �½���
* @2007-06-30 StartThread��Ϊ����idΪ0ʱ��ʾ�����½�������ֵΪ�߳�id��
* @2007-08-03 StartThread��������ͬ��������������ָ�����
* @2007-09-20 ��������ģʽ���˳�δ�ͷ������������
*/
/**<pre>
ʹ��Sample��
	//ģʽһ��ͨ���̳�THThread�࣬ʵ��ThreadFunc����ʵ��
	class XXX: public THThread
	{
		virtual void ThreadFunc(int threadid,void *adddata);
	}
	XXX a;
	//ģʽ����ͨ���ص�����ʵ��
	void MyThreadCallBack(int ThreadId,THThread *pCls,void *data)
	{
	}
	THThread a;
	a.SetCallFunc(MyThreadCallBack);
	//����ʹ�÷���
	a.StartThread(1,NULL);
	a.StartThread(2,NULL,500);
	a.PauseThread(1);
	a.ResumeThread(1);
	a.SetThreadPriority(1);
	a.KillThread(1);
	a.SetThreadEvent(2);
	ע��㣺
	����KillThreadʱ�������̷߳����źţ���ThreadFunc��Ӧ�����ܿ췵�أ���Ҫ����Sleep������500������в�������StartThreadʱָ�����500��ThreadFunc�����꼴���ء�
	���������ThreadFunc��Sleep�������THThread::WaitSecordWithKillSingal����Sleep����������ֵΪTrueʱ�������ж��˳���
	���ThreadFunc�й�������������ʱ����ܳ���5��ʱ����ȷ��ThreadFunc�н��г�ʱ�����ǰ��������Lock���ͷţ���ΪKillThreadʱ�ȴ�5���̲߳��˳��Ļ�������ǿ�ƽ����̣߳����ܻ����֮���Unlock����û�е��á�
	ThreadFunc���Ϊ�����ģ���������ʹ��CString���࣬��Ϊ��TerimThreadʱ���������Щ����͹�û�е�����
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
		//��FreeListʱ����KillThread
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
	* @brief �ص�ʵ�ֺ�������timevalΪ0ʱ���ú���ֻ����һ�Σ���Ϊ��0ʱ���ú���wantforobject timeval����յ��¼�����ã���ʹ����̳з�ʽʱ�����ظú���
	* @param ThreadId	�̱߳�ʶ
	* @param data		��������
	*/
	virtual void ThreadFunc(int ThreadId,void *data)
	{
		if (m_cb) (m_cb)(ThreadId,this,data);
	}

	/**
	* @brief ���ûص�����������ʹ��ʵ�ֻص�����ģʽʱʹ��
	* @param cb			�ص�����
	*/
	void SetCallFunc(ThreadCallBack cb){m_cb=cb;}

	/**
	* @brief ��ȡ��ָ��id��ʼ�Ŀ���id��������ThreadFunc����ThreadId��Χ���ֲ�ͬ�����߳�
	* @param StartId		ָ��id
	* @return ����id
	*/
	int GetAvailableId(int StartId)
	{
		THSingleLock lock(&m_mutex);
		ThreadData *info1=NULL;
		while(m_ThreadMap.GetAt(StartId,info1)) StartId++;
		return StartId;
	}

	/**
	* @brief �����߳�
	* @param ThreadId	�̱߳�ʶ���ظ���ʶ������ʧ�ܣ����Ϊ0����ʾ�����¿�
	* @param adddata	��������
	* @param timeval	ʱ��������ֵΪ0ʱ��THThread��ʹ�����õ�WaitForObject��Event�����ã���ʱThreadFuncֻ����һ�Σ������ֵ��0��THThread WaitforObject���ҿ���ͨ��event��������ΪTIMEVAL_NOTDELAY=1������WaitForObject��ֱ�ӵ���
	* @param bDelay		������ʱ����ʱ����ֵ��ʾ�Ƿ����һ��ʱ�����ŵ���ThreadFunc
	* @param nStackSize	ջ��С��Ĭ��Ϊ0
	* @return			�����̺߳ţ�����0��ʾ����
	*/
	inline int StartThread(int ThreadId=0,void *adddata=NULL,DWORD timeval=0,BOOL bDelay=FALSE,int nStackSize=0)
	{
		if (timeval==TIMEVAL_NOTDELAY)
			return StartThread2(ThreadId,adddata,0,bDelay?timeval:0,TRUE,nStackSize);
		return StartThread2(ThreadId,adddata,timeval,bDelay?timeval:0,timeval?TRUE:FALSE,nStackSize);
	}

	/**
	* @brief �����߳̽ӿ�2
	* @param ThreadId	�̱߳�ʶ���ظ���ʶ������ʧ�ܣ����Ϊ0����ʾ�����¿�
	* @param adddata	��������
	* @param timeval	ʱ��������ֵΪ0ʱ��THThread��ʹ�����õ�WaitForObject��Event�����ã������ֵ��0��THThread WaitforObject���ҿ���ͨ��event����
	* @param nDelay		������ʱ����ʱ����ֵ��ʾ�Ƿ����һ��ʱ�����ŵ���ThreadFunc
	* @param bLoop		�Ƿ�ѭ��
	* @param nStackSize	ջ��С��Ĭ��Ϊ0
	* @return			�����̺߳ţ�����0��ʾ����
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
		//����beginthread,CreateThread�п�������ڴ�й¶
		//If the thread generated by _beginthread exits quickly, the handle returned to the caller of _beginthread might be invalid or, worse, point to another thread.
		info->hThread=(HANDLE)_beginthreadex(NULL,nStackSize,MyCreateThread,(LPVOID)info,0,NULL);
		//info->hThread=(HANDLE)_beginthread(MyCreateThread,0,(LPVOID)info);
		if (info->hThread==NULL/* || info->hThread==1L*/)//beginthread return 1L for fail,beginthreadex return 0L
		//if (!info->hThread)
			KillThread(ThreadId);
		return (info->hThread)?ThreadId:0;
	}
	/**
	* @brief ��ͣ�߳�
	* @param ThreadId	�̱߳�ʶ
	* @return			�Ƿ�ɹ�
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
	* @brief �ָ��߳�
	* @param ThreadId	�̱߳�ʶ
	* @return			�Ƿ�ɹ�
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
	* @brief �����̣߳����߳���ʱ������KillThread���ᱻ��������ʱӦ�õ���SelfKillThread
	* @param ThreadId	�̱߳�ʶ
	* @param nWaitForExitSec	�ȴ������߳�ʱ��
	* @return			�Ƿ�ɹ�
	*/
	BOOL KillThread(int ThreadId,UINT nWaitForExitSec=5000)
	{
		//THSingleLock lock(&m_mutex);
		ThreadData *info;
		BOOL bpret=m_ThreadMap.GetAt(ThreadId,info);
		if (!bpret || !info) return FALSE;
		if (info->tid==GetCurrentThreadId())
		{
			//�������assert��˵����KillThread���������ΪThreadFunc������ȻKillThread��Ȼ������Ч��������ʹ��TerminateThread�����ģ���ʱ����ʹThread�е���Ȳ��ܵ����͹������������
			//��׼����ӦΪ���ж��Ƿ���ThreadFunc�У���ʹ�ú���IsThreadInCurrentThread�жϣ�����Ǳ��߳��е�KillThread��ʹ��SelfKillThread�ý���λ��������return
			//������鷳������ʹ��SAVEKILLTHREADOREXIT��
			ASSERT(FALSE);
			info->nWaitForExitSec=0;
		}
		else
			info->nWaitForExitSec=nWaitForExitSec;
		//THDebug(_T("THThread:kill thread:%d"),ThreadId);
		//��ListFreeʱKillThread
		//������Ҫ��������map������ͷŵ��ã���˲���lock���������lock
		return m_ThreadMap.RemoveAt(ThreadId);
		//remove function will call it
		//::TerminateThread(info->hThread,0);
		//Sleep(500);
		//delete info;
		//return TRUE;
	}

	/*
	* @brief ���߳���ʱ������KillThread���ᱻ��������ʱӦ�õ���SelfKillThread���������Ӧ��return�˳��߳���
	* @param ThreadId	�̱߳�ʶ
	* @return			�Ƿ�ɹ�
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
	* @brief ���������߳�
	* @return			�Ƿ�ɹ�
	*/
	void KillAllThread()
	{
		m_ThreadMap.RemoveAll();
	}

	/**
	* @brief ��ȡ��ǰ�߳���
	* @return			��ǰ�߳���
	*/
	UINT GetThreadCount() {return m_ThreadMap.GetCount();}

	/**
	* @brief �����߳��ź�
	* @param ThreadId	�̱߳�ʶ
	* @return			�Ƿ�ɹ�
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
	* @brief �����̳߳��źţ����ҿ��е��̣߳������䷢���ź�
	* @param nStartThread	ָ�����ĸ�Thread��ʼ��0Ϊȫ������ʼ�����߳�ΪnStartThread+1
	* @return			��Ӧ���߳�id������0Ϊ���Ͳ��ɹ�
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
						//������ҪSleep��������Լ��Ȱ��ź���������
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
	* @brief �����߳����ȼ�
	* @param ThreadId	�̱߳�ʶ
	* @param Priority	�߳����ȼ���THREAD_PRIORITY_XXX
	* @return			�Ƿ�ɹ�
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
	* @brief �ж�Thread�Ƿ���������
	* @param ThreadId	�̱߳�ʶ
	* @return			�Ƿ���������
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
	* @brief ��ȡ�̵߳ĸ�����Ϣ
	* @param ThreadId	�̱߳�ʶ
	* @return			�Ƿ���������
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
	* @brief �ȴ��̲߳����������߳��ź�,һ�����߳��е���
	* @param ThreadId	�̱߳�ʶ
	* @return			�Ƿ��߳���Ҫ�˳�
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
* @brief ��ʱ���߳���
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-07-30 �½���
* @2007-08-29 ��Ϊ��GetTickCount��ʱ��ԭ���ļ�ʱ���ϸߣ���ʱ���ȷֲ�ͬ����
*/
/**<pre>
ʹ��Sample��
	//ģʽһ��ͨ���̳�THTimerThread�࣬ʵ��ThreadFunc����ʵ��
	class XXX: public THTimerThread
	{
		virtual void ThreadFunc(int timerid,void *adddata);
	}
	XXX a;
	//ģʽ����ͨ���ص�����ʵ��
	THTimerThread a;
	a.SetCallFunc(MyThreadCallBack);
	//����ʹ�÷���
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
	* @brief �ص�ʵ�ֺ�����ʹ����̳з�ʽʱ�����ظú���
	* @param TimerId	��ʱ����ʶ
	* @param data		��������
	*/
	virtual void ThreadFunc(int TimerId,void *data)
	{
		if (m_cb) (m_cb)(TimerId,this,data);
	}

	/**
	* @brief ���ûص�����������ʹ��ʵ�ֻص�����ģʽʱʹ��
	* @param cb			�ص�����
	*/
	void SetCallFunc(TimerThreadCallBack cb){m_cb=cb;}

	/**
	* @brief ��ʼһ���µĶ�ʱ��
	* @param TimerId	��ʱ���ţ�������ж�Ӧ��ʱ���ţ�����ʧ�ܣ����Ϊ0����ʾ����Ҫָ��
	* @param TimerVal	��ʱ���������λ���룬��10000��10�룩�ڶ�ʱ����С����Ϊ50���룬30����Ϊ500���룬����Ϊ2��
	* @param bLoop		�Ƿ�ѭ����ʱ
	* @param adddata	��������
	* @return			��ʱ����
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
	* @brief ֹͣ��ʱ��
	* @param TimerId	��ʱ����
	* @return			�Ƿ�ɹ�
	*/
	BOOL StopTimer(int TimerId)
	{
		ThreadData *info;
		BOOL bpret=m_ThreadMap.GetAt(TimerId,info);
		if (!bpret || !info) return FALSE;
		//��ListFreeʱKillThread
		return m_ThreadMap.RemoveAt(TimerId);
	}

	/**
	* @brief ֹͣ���ж�ʱ��
	*/
	void StopAllTimer()
	{
		m_ThreadMap.RemoveAll();
	}

	/**
	* @brief ���㶨ʱ������
	* @param TimerId	��ʱ����
	* @param TimerVal	���Ϊ0����ʾ���㵽��ʼֵ���¼��������Ϊ��ֵ����ʾ�ӳ����ٶ�ʱʱ�䣻��ֵ��ʾ���ٶ��ٶ�ʱʱ�䡣��λΪ����
	* @return			�Ƿ�ɹ�
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
	* @brief �ж�Thread�Ƿ���������
	* @param ThreadId	�̱߳�ʶ
	* @return			�Ƿ���������
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
* @brief �ƻ�������
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-11-22 �½���
*/
/**<pre>
ʹ��Sample��
	//ģʽһ��ͨ���̳�THScheduleThread�࣬ʵ��ThreadFunc����ʵ��
	class XXX: public THScheduleThread
	{
		virtual void ScheduleFunc(int scheduleid,void *adddata);
	}
	XXX a;
	//ģʽ����ͨ���ص�����ʵ��
	THScheduleThread a;
	a.SetCallFunc(MyThreadCallBack);
	//����ʹ�÷���
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
	* @brief �ص�ʵ�ֺ�����ʹ����̳з�ʽʱ�����ظú���
	* @param TimerId	��ʱ����ʶ
	* @param data		��������
	*/
	virtual void ScheduleFunc(int TimerId,void *data)
	{
		if (m_cb) (m_cb)(TimerId,this,data);
	}

	/**
	* @brief ���ûص�����������ʹ��ʵ�ֻص�����ģʽʱʹ��
	* @param cb			�ص�����
	*/
	void SetCallFunc(ScheduleThreadCallBack cb){m_cb=cb;}

	/**
	* @brief ��ʼһ���µļƻ�����
	* @param ScheduleId	�ƻ�����ţ�������ж�Ӧ�ţ�����ʧ�ܣ����Ϊ0����ʾ����Ҫָ��
	* @param tBase		�ƻ�����ʱ��
	* @param nScheduleMode �ƻ�������ʽ��Ĭ��ΪСʱƥ�䣬��ÿ��
	* @param nDefVal	��ʱ��ȷ�ȣ�Ĭ��1���ӣ�ע�⾫��̫С���ܻ�����ʧ��ĳ����Ӧ
	* @param bLoop		�Ƿ�ѭ��
	* @param adddata	��������
	* @return			�ƻ�����ţ�0Ϊʧ��
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
		info->nLast=-1;//����ȷ��������
		return ScheduleId;
	}

	/**
	* @brief ֹͣ�ƻ�����
	* @param ScheduleId	�ƻ������
	* @return			�Ƿ�ɹ�
	*/
	BOOL StopSchedule(int ScheduleId)
	{
		ThreadData *info;
		BOOL bpret=m_ThreadMap.GetAt(ScheduleId,info);
		if (!bpret || !info) return FALSE;
		//��ListFreeʱKillThread
		return m_ThreadMap.RemoveAt(ScheduleId);
	}

	/**
	* @brief ֹͣ���мƻ�����
	*/
	void StopAllSchedule()
	{
		m_ThreadMap.RemoveAll();
	}

	/**
	* @brief �жϼƻ������Ƿ���������
	* @param ScheuleId	�ƻ������
	* @return �Ƿ���������
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
* @brief �첽����������
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-09-27 �½���
*/
/**<pre>
ʹ��Sample��
	//ͨ���̳�THAsynCall��ʵ��
	class XXX: public THAsynCall<XXX>
	{
		virtual void ThreadFunc(int timerid,void *adddata);
		void Test();
		void Test1(LPVOID p);
	}
	//����ʹ�÷���
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
