
#pragma once

#define THSYSTEM_INCLUDE_WINBASE
#include <THSystem.h>

#ifdef _DEBUG
	#define DEBUG_LOCK_STRING
#endif

/**
* @brief 同步锁类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-05-28 新建类
*/
/**<pre>
  使用Sample：
  THMutex mu;
  mu.Lock();
  mu.Unlock();

  THMutex m_mu;
  void fn1()
  {
	THSingleLock(&m_mu);
	//...
  }
</pre>*/
class THMutex
{
public:
	THMutex(){InitializeCriticalSection(&m_cs);}
	virtual ~THMutex(){DeleteCriticalSection(&m_cs);}
	inline void Lock(int num=0) const
	{
		EnterCriticalSection(&m_cs);
#ifdef DEBUG_LOCK_STRING
		if (num!=0) m_str.AppendFormat(_T("[%d]"),num);
#endif
	}
	inline void Unlock(int num=0) const
	{
#ifdef DEBUG_LOCK_STRING
		if (num!=0)
		{
			THString str;
			str.Format(_T("[%d]"),num);
			int pos=m_str.Find(str,0);
			if (pos==-1)
			{
				m_str.AppendFormat(_T("[U:%d]"),num);
				ASSERT(FALSE);
			}
			else
			{
				m_str=m_str.Left(pos)+m_str.Mid(pos+str.GetLength());
			}
		}
#endif
		LeaveCriticalSection(&m_cs);
	}
#if (_WIN32_WINNT >= 0x0400)

	inline BOOL TryLock(int num=0) const
	{
		BOOL ret=TryEnterCriticalSection(&m_cs);
#ifdef DEBUG_LOCK_STRING
		if (ret && num!=0)
			m_str.AppendFormat(_T("[%d]"),num);
#endif
		return ret;
	}
	inline BOOL TryLockSleep(int WaitCnt,int num=0) const
	{
		BOOL bLockOk=FALSE;
		for(int i=0;i<WaitCnt;i++)
		{
			if (TryEnterCriticalSection(&m_cs))
			{
				bLockOk=TRUE;
				break;
			}
			else
				Sleep(10);
		}
#ifdef DEBUG_LOCK_STRING
		if (bLockOk && num!=0)
			m_str.AppendFormat(_T("[%d]"),num);
#endif
		return bLockOk;
	}
#endif
private:
	mutable CRITICAL_SECTION m_cs;
#ifdef DEBUG_LOCK_STRING
public:
	mutable THString m_str;
#endif
};

/**
* @brief 快速锁类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-05-28 新建类
*/
class THSingleLock
{
public:
	THSingleLock(THMutex *mutex,int num=0)
	{
		m_pMutex=mutex;
		m_num=num;
		if (m_pMutex) m_pMutex->Lock(num);
	}
	virtual ~THSingleLock()
	{
		if (m_pMutex) m_pMutex->Unlock(m_num);
	}
private:
	THMutex *m_pMutex;
	int m_num;
};

/**
* @brief 优化锁类，优化读取Lock，可多个读取Lock同时进行，写入Lock需要锁住所有读取Lock
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-05-16 新建类
*/
class THMutexEx
{
public:
	THMutexEx()
	{
		m_cnt=0;
	}
	virtual ~THMutexEx()
	{
	}

	void ReadOnlyLock(int num=0)
	{
		m_csl.Lock();
		m_csun.Lock();
		m_cnt++;
#ifdef DEBUG_LOCK_STRING
		if (num!=0) m_str.AppendFormat(_T("[%d]"),num);
#endif
		m_csun.Unlock();
		m_csl.Unlock();
	}
	void ReadOnlyUnlock(int num=0)
	{
		m_csun.Lock();
#ifdef DEBUG_LOCK_STRING
		if (num!=0)
		{
			THString str;
			str.Format(_T("[%d]"),num);
			int pos=m_str.Find(str,0);
			if (pos==-1)
			{
				m_str.AppendFormat(_T("[U:%d]"),num);
				ASSERT(FALSE);
			}
			else
			{
				m_str=m_str.Left(pos)+m_str.Mid(pos+str.GetLength());
			}
		}
#endif
		ASSERT(m_cnt>0);
		if (m_cnt>0) m_cnt--;
		m_csun.Unlock();
	}

	void ChangeLock(int num=0)
	{
		m_csl.Lock(num);
		//获取进入锁的控制权
		while(1)
		{
			//等待所有普通锁释放
			m_csun.Lock();
			if (m_cnt<=0)
			{
				m_csun.Unlock();
				return;
			}
			m_csun.Unlock();
			Sleep(50);
		}
	}

	void ChangeUnlock(int num=0)
	{
		m_csl.Unlock(num);
	}
private:
	THMutex m_csl;					//<<<改变锁
	THMutex m_csun;					//<<<读取锁
	volatile int m_cnt;				//<<<读取引用计数
#ifdef DEBUG_LOCK_STRING
	THString m_str;
#endif
};

class THEvent
{
public:
	THEvent(){m_hObject=NULL;}
	THEvent( THEvent& h ){m_hObject=h.m_hObject;}
	THEvent( BOOL bManualReset, BOOL bInitialState ){Create( NULL, bManualReset, bInitialState, NULL );}
	THEvent( BOOL bInitiallyOwn, BOOL bManualReset, LPCTSTR pstrName, LPSECURITY_ATTRIBUTES lpsaAttribute )
	{m_hObject = ::CreateEvent(lpsaAttribute, bManualReset, bInitiallyOwn, pstrName);}
	THEvent( HANDLE h ){m_hObject=h;}
	virtual ~THEvent()
	{
		if (m_hObject)
		{
			CloseHandle(m_hObject);
			m_hObject=NULL;
		}
	}

	// Create a new event
	BOOL Create( LPSECURITY_ATTRIBUTES pSecurity=NULL, BOOL bManualReset=TRUE, BOOL bInitialState=FALSE, LPCTSTR pszName=NULL)
	{
		if (m_hObject!=NULL) return FALSE;
		m_hObject = ::CreateEvent( pSecurity, bManualReset, bInitialState, pszName );
		return( m_hObject != NULL );
	}
	// Open an existing named event
	BOOL Open( DWORD dwAccess, BOOL bInheritHandle, LPCTSTR pszName )
	{
		if (m_hObject!=NULL) return FALSE;
		m_hObject = ::OpenEvent( dwAccess, bInheritHandle, pszName );
		return( m_hObject != NULL );
	}
	// Pulse the event (signals waiting objects, then resets)
	// 如果为bManualReset=FALSE，则将event obj设为有信号，当一个等待线程release后，event obj自动设为无信号
	// 如果是bManualReset=TRUE，将event obj设为有信号，且release所有等待线程，然后将event obj自动设为无信号
	BOOL Pulse()
	{
		if (m_hObject==NULL) return FALSE;
		return( ::PulseEvent( m_hObject ) );
	}
	// Set the event to the non-signaled state
	BOOL Reset()
	{
		if (m_hObject==NULL) return FALSE;
		return( ::ResetEvent( m_hObject ) );
	}
	// Set the event to the signaled state
	// 如果为bManualReset=FALSE，则将event obj设为有信号，当一个等待线程release后，event obj自动设为无信号
	// 如果是bManualReset=TRUE，则release所有等待线程，且没有后面自动重设
	BOOL Set()
	{
		if (m_hObject==NULL) return FALSE;
		return( ::SetEvent( m_hObject ) );
	}
	operator HANDLE()
	{
		return m_hObject;
	}
	//返回TRUE表示等待到事件，返回FALSE表示timeout
	BOOL WaitForObject(DWORD dwMiSec=INFINITE){return (::WaitForSingleObject(m_hObject,dwMiSec)==WAIT_OBJECT_0);}
protected:
	HANDLE m_hObject;
};
