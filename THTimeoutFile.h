#pragma once

#include <THFile.h>
#include <THStruct.h>
#include <THSyn.h>
#include <THThread.h>

#define TIMERID_TIMEOUTFILE		1
/**
* @brief 自动关闭文件处理基类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-03-13 新建类
*/
/**<pre>
  用途：文件不使用超过等待时间即关闭文件，重新操作时自动重打开文件并定位
</pre>*/
class THTimeoutFile : private THTimerThread
{
public:
// Flag values
	enum OpenFlags {
		modeRead =         (int) 0x00000,
		modeWrite =        (int) 0x00001,
		modeReadWrite =    (int) 0x00002,
		shareCompat =      (int) 0x00000,
		shareExclusive =   (int) 0x00010,
		shareDenyWrite =   (int) 0x00020,
		shareDenyRead =    (int) 0x00030,
		shareDenyNone =    (int) 0x00040,
		modeNoInherit =    (int) 0x00080,
		modeCreate =       (int) 0x01000,
		modeNoTruncate =   (int) 0x02000,
		typeText =         (int) 0x04000, // typeText and typeBinary are
		typeBinary =       (int) 0x08000, // used in derived classes only
		osNoBuffer =       (int) 0x10000,
		osWriteThrough =   (int) 0x20000,
		osRandomAccess =   (int) 0x40000,
		osSequentialScan = (int) 0x80000,
		};

	enum Attribute {
		normal =    0x00,
		readOnly =  0x01,
		hidden =    0x02,
		system =    0x04,
		volume =    0x08,
		directory = 0x10,
		archive =   0x20
		};

	enum SeekPosition { begin = 0x0, current = 0x1, end = 0x2 };

	THTimeoutFile()
	{
		m_bTimeOutClose=FALSE;
		m_nOpenFlags=0;
		m_nTimeVal=5000;
		m_nLastSeek=0;
		m_nFileLength=0;
	}

	THTimeoutFile(UINT nTimeVal)
	{
		m_bTimeOutClose=FALSE;
		m_nOpenFlags=0;
		m_nTimeVal=nTimeVal;
		m_nLastSeek=0;
	}

	void SetTimeVal(UINT TimeVal){m_nTimeVal=TimeVal;}

	virtual ~THTimeoutFile()
	{
		THSingleLock lock(&m_mutex);
		StopTimer(TIMERID_TIMEOUTFILE);
	}

	operator HANDLE()
	{
		THSingleLock lock(&m_mutex);
		OpenTimeOutedFile();
		return m_file.m_hFile;
	}

	virtual ULONGLONG GetPosition()
	{
		THSingleLock lock(&m_mutex);
		OpenTimeOutedFile();
		return m_file.GetPosition();
	}

	//bDelayOpen 为TRUE时，文件不保持打开，到需要使用的时候才打开
	virtual BOOL Open(THString sFileName, UINT nOpenFlags, BOOL bDelayOpen=FALSE)
	{
		THSingleLock lock(&m_mutex);
		m_sFile=sFileName;
		m_nOpenFlags=nOpenFlags;
		if (!m_file.Open(sFileName,nOpenFlags)) return FALSE;
		if (bDelayOpen)
		{
			m_nLastSeek=0;
			m_file.Close();
			m_bTimeOutClose=TRUE;
		}
		else
		{
			m_bTimeOutClose=FALSE;
			SetupTimer();
		}
		return TRUE;
	}

	virtual void Close()
	{
		THSingleLock lock(&m_mutex);
		StopTimer(TIMERID_TIMEOUTFILE);
		m_bTimeOutClose=FALSE;
		m_nLastSeek=0;
		m_nFileLength=0;
	}

	virtual ULONGLONG Seek(LONGLONG lOff, UINT nFrom)
	{
		if (!(nFrom==SEEK_SET && m_nLastSeek==lOff && lOff!=0))
		{
			THSingleLock lock(&m_mutex);
			OpenTimeOutedFile();
			m_nLastSeek=m_file.Seek(lOff,nFrom);
		}
		return m_nLastSeek;
	}

	inline virtual THString GetFileName() const{return m_sFile;}

	virtual ULONGLONG GetLength()
	{
		if (m_nFileLength==0)
		{
			THSingleLock lock(&m_mutex);
			OpenTimeOutedFile();
			m_nFileLength=m_file.GetLength();
		}
		return m_nFileLength;
	}

	virtual UINT Read(void* lpBuf,UINT nCount)
	{
		THSingleLock lock(&m_mutex);
		OpenTimeOutedFile();
		UINT readed=m_file.Read(lpBuf,nCount);
		m_nLastSeek+=readed;
		return readed;
	}

	virtual BOOL IsOpen()
	{
		THSingleLock lock(&m_mutex);
		OpenTimeOutedFile();
		return m_file.IsOpen();
	}
	virtual void Write(const void* lpBuf, UINT nCount)
	{
		THSingleLock lock(&m_mutex);
		OpenTimeOutedFile();
		m_nFileLength=0;
		m_nLastSeek=0;
		m_file.Write(lpBuf,nCount);
	}
private:
	UINT m_nTimeVal;
	BOOL m_bTimeOutClose;
	THFile m_file;
	THMutex m_mutex;
	THString m_sFile;
	UINT m_nOpenFlags;
	ULONGLONG m_nLastSeek;
	ULONGLONG m_nFileLength;

	void SetupTimer()
	{
		if (IsThreadRunning(TIMERID_TIMEOUTFILE))
		{
			ResetTimer(TIMERID_TIMEOUTFILE);
		}
		else
			StartTimer(TIMERID_TIMEOUTFILE,m_nTimeVal);
	}

	BOOL OpenTimeOutedFile()
	{
		if (m_bTimeOutClose)
		{
			m_bTimeOutClose=FALSE;
			if (!m_file.Open(m_sFile,m_nOpenFlags)) return FALSE;
			m_nLastSeek=m_file.Seek(m_nLastSeek,SEEK_SET);
		}
		SetupTimer();
		return TRUE;
	}

	virtual void ThreadFunc(int timerid,void *adddata)
	{
		if (timerid==TIMERID_TIMEOUTFILE)
		{
			THSingleLock lock(&m_mutex);
			if (m_file.IsOpen())
			{
				m_nLastSeek=m_file.GetPosition();
				m_file.Close();
				m_bTimeOutClose=TRUE;
				m_nFileLength=0;
				OutputDebugString(_T("TimeoutFile\r\n"));
			}
		}
	}
};