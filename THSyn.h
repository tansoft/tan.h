
#pragma once

#define THSYSTEM_INCLUDE_WINBASE
#include <THSystem.h>

#ifdef _DEBUG
	#define DEBUG_LOCK_STRING
#endif

/**
* @brief ͬ������
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-05-28 �½���
*/
/**<pre>
  ʹ��Sample��
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
* @brief ��������
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-05-28 �½���
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
* @brief �Ż����࣬�Ż���ȡLock���ɶ����ȡLockͬʱ���У�д��Lock��Ҫ��ס���ж�ȡLock
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-05-16 �½���
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
		//��ȡ�������Ŀ���Ȩ
		while(1)
		{
			//�ȴ�������ͨ���ͷ�
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
	THMutex m_csl;					//<<<�ı���
	THMutex m_csun;					//<<<��ȡ��
	volatile int m_cnt;				//<<<��ȡ���ü���
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
	// ���ΪbManualReset=FALSE����event obj��Ϊ���źţ���һ���ȴ��߳�release��event obj�Զ���Ϊ���ź�
	// �����bManualReset=TRUE����event obj��Ϊ���źţ���release���еȴ��̣߳�Ȼ��event obj�Զ���Ϊ���ź�
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
	// ���ΪbManualReset=FALSE����event obj��Ϊ���źţ���һ���ȴ��߳�release��event obj�Զ���Ϊ���ź�
	// �����bManualReset=TRUE����release���еȴ��̣߳���û�к����Զ�����
	BOOL Set()
	{
		if (m_hObject==NULL) return FALSE;
		return( ::SetEvent( m_hObject ) );
	}
	operator HANDLE()
	{
		return m_hObject;
	}
	//����TRUE��ʾ�ȴ����¼�������FALSE��ʾtimeout
	BOOL WaitForObject(DWORD dwMiSec=INFINITE){return (::WaitForSingleObject(m_hObject,dwMiSec)==WAIT_OBJECT_0);}
protected:
	HANDLE m_hObject;
};
