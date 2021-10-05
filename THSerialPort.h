#pragma once

#include <THThread.h>
#include <THMemBuf.h>
#include <THSyn.h>

#define SPPARITYTYPE_NONE				'N'
#define SPPARITYTYPE_ODD				'O'
#define SPPARITYTYPE_EVEN				'E'

#define SPERRORTYPE_SETCOMMSTATE		1
#define SPERRORTYPE_BUILDCOMMDCB		2
#define SPERRORTYPE_GETCOMMSTATE		3
#define SPERRORTYPE_SETCOMMMASK			4
#define SPERRORTYPE_SETCOMMTIMEOUT		5
#define SPERRORTYPE_WAITCOMMEVENT		6
#define SPERRORTYPE_READFILE			7
#define SPERRORTYPE_READFILEOVERLAP		8
#define SPERRORTYPE_WRITEFILE			9
/**
* @brief �����¼���Ӧ������
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-08-11 �½���
*/
/**<pre>
�÷���
	�̳�ʵ��
	virtual void OnHandleError(int nErrorType,int nPortNum);
	virtual void OnHandleFlagEvent(DWORD dwCommEvent,int nPortNum);
	virtual void OnHandleCharEvent(unsigned char chBuffer,int nPortNum);
</pre>*/
class ITHSerialPortEventHandler
{
public:
	/**
	* @brief ��Ӧ�����¼�
	* @param dwCommEvent	�����¼�
	* @param nPortNum		���ں�
	*/
	/**<pre>
	�¼���Ӧdefine��
		EV_BREAK	A break was detected on input.
		EV_CTS		The CTS (clear-to-send) signal changed state. 
		EV_DSR		The DSR (data-set-ready) signal changed state. 
		EV_ERR		A line-status error occurred. Line-status errors are CE_FRAME, CE_OVERRUN, and CE_RXPARITY. 
		EV_RING		A ring indicator was detected. 
		EV_RLSD		The RLSD (receive-line-signal-detect) signal changed state. 
		EV_RXFLAG	The event character was received and placed in the input buffer.  
	</pre>*/
	virtual void OnHandleFlagEvent(DWORD dwCommEvent,int nPortNum)=0;
	/**
	* @brief ��Ӧ���������¼�
	* @param chBuffer		��������
	* @param nPortNum		���ں�
	*/
	virtual void OnHandleCharEvent(unsigned char chBuffer,int nPortNum)=0;
	/**
	* @brief ��Ӧ���ڳ����¼�
	* @param nErrorType		�����룬defineֵSPERRORTYPE_
	* @param nPortNum		���ں�
	*/
	virtual void OnHandleError(int nErrorType,int nPortNum)=0;
};

/**
* @brief �����崮���¼���Ӧ������
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-08-11 �½���
*/
/**<pre>
�÷���
	�̳�ʵ��
	virtual void OnHandleError(int nErrorType,int nPortNum);
	virtual void OnHandleFlagEvent(DWORD dwCommEvent,int nPortNum);
	virtual unsigned int OnHandleBufferCharEvent(const void *pData,unsigned int nLen,int nPortNum);
</pre>*/
class ITHSerialPortBufferedEventHandler : public ITHSerialPortEventHandler
{
public:
	ITHSerialPortBufferedEventHandler()
	{
		m_nPortno=0;
		m_nLastNotNeedSize=0;
		m_nTimerVal=500;
		SetMonitoringInterval();
		m_timer.SetCallFunc(OnHandlerTimerCB);
		//�ڽ��յ��ַ�ʱ���ÿ�ʼ������
		//StartMonitoring();
	}
	virtual ~ITHSerialPortBufferedEventHandler()
	{
		StopMonitoring();
		THSingleLock lock(&m_mutex);
		m_nLastNotNeedSize=0;
		m_buf.Empty();
	}
	/**
	* @brief ��ʼ�������ݻ�����ӣ��ú���������ʽ����
	*/
	void StartMonitoring(){m_timer.StartTimer(1,m_nTimerVal,TRUE,this);}
	/**
	* @brief ���ü��ӵļ��ʱ��
	* @param TimerVal		���峬ʱ����ֵ�������յ��������TimerVal����󣬶�û��������ʱ������OnHandleBufferCharEvent
	*/
	void SetMonitoringInterval(int InterVal=500)
	{
		BOOL bPause=FALSE;
		if (m_timer.IsThreadRunning(1)) bPause=TRUE;
		if (bPause) StopMonitoring();
		m_nTimerVal=InterVal;
		if (bPause) StartMonitoring();
	}
	/**
	* @brief ֹͣ�������ݻ�����ӣ��ú���������ʽ����
	*/
	void StopMonitoring(){m_timer.StopTimer(1);}
	static void OnHandlerTimerCB(int TimerId,THTimerThread *pCls,void *data)
	{
		ITHSerialPortBufferedEventHandler *cls=(ITHSerialPortBufferedEventHandler *)data;
		if (cls) cls->TimerPump();
	}
	virtual void OnHandleCharEvent(unsigned char buffer,int portno)
	{
		if (!m_timer.IsThreadRunning(1)) StartMonitoring();
		THSingleLock lock(&m_mutex);
		m_nPortno=portno;
		m_buf.AddBuf(&buffer,sizeof(unsigned char));
		m_timer.ResetTimer(1);
	}
	/**
	* @brief �����崮���¼���Ӧʵ��
	* @param pData			����
	* @param nLen			����
	* @param nPortNum		���ں�
	* @return ����0��ʾ��û���㹻��Ҫ�����ݣ�������ֵ��ʾ�Ѵ�������ֽ����ݣ�������ɾ����Щ����
	*/
	virtual unsigned int OnHandleBufferCharEvent(const void *pData,unsigned int nLen,int nPortNum)=0;
protected:
	void TimerPump()
	{
		UINT size=m_buf.GetBufferedSize();
		if (size>m_nLastNotNeedSize)
		{
			//wait to check the new buffer
			UINT ret;
			void *tmpbuf=m_buf.GetBuf(&ret);
			if (tmpbuf && ret>0)
			{
				unsigned int rmvsize=OnHandleBufferCharEvent(tmpbuf,ret,m_nPortno);
				if (rmvsize>ret) rmvsize=ret;
				//THSingleLock lock(&m_mutex);
				m_buf.ReleaseGetBufLock(rmvsize);
				if (rmvsize>0)
				{
					m_nLastNotNeedSize=0;
				}
				else
				{
					m_nLastNotNeedSize=ret;
					m_timer.ResetTimer(1,60000);//delay 1 min����Ϊ��ʱ�����ж��ˣ�����cpuռ��
				}
			}
			else
				m_buf.ReleaseGetBufLock();
			//old imp code
/*			//wait to check the new buffer
			char *tmpbuf=new char[size+1];
			if (tmpbuf)
			{
				unsigned int ret=0;
				if (m_buf.GetBuf(tmpbuf,size,&ret,TRUE))
				{
					unsigned int rmvsize=OnHandleBufferCharEvent(tmpbuf,ret,m_nPortno);
					if (rmvsize>0)
					{
						if (rmvsize>ret) rmvsize=ret;
						THSingleLock lock(&m_mutex);
						m_buf.GetBuf(tmpbuf,rmvsize,&ret,FALSE);
						m_nLastNotNeedSize=0;
					}
					else
					{
						m_nLastNotNeedSize=ret;
						m_timer.ResetTimer(1,60000);//delay 1 min����Ϊ��ʱ�����ж��ˣ�����cpuռ��
					}
				}
				delete [] tmpbuf;
			}*/
		}
		else
			m_timer.ResetTimer(1,60000);//delay 1 min����Ϊ��ʱ�����ж��ˣ�����cpuռ��
	}
	THMemBuf m_buf;
	THTimerThread m_timer;
	THMutex m_mutex;
	volatile UINT m_nLastNotNeedSize;
	volatile int m_nPortno;
	volatile int m_nTimerVal;
};

/**
* @brief ���ڴ�����
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-08-10 �½���
*/
/**<pre>
�÷���
	//�첽ģʽ���첽ģʽ��Ӧ��ʹ�������࣬ITHSerialPortBufferedEventHandler��ITHSerialPortEventHandler
	//Buffered��������OnHandleCharEvent�ӿڣ�ͨ����������棬������OnHandleBufferCharEvent���ã���ϸ�÷���μ�ITHSerialPortBufferedEventHandler��ITHSerialPortEventHandler�ࡣ
	class MyClass:public ITHSerialPortBufferedEventHandler
	{
		virtual void OnHandleError(int nErrorType,int nPortNum)
		{
			THDebug(_T("Seiral Port:%d Error %d"),nPortNum,nErrorType);
		}
		virtual void OnHandleFlagEvent(DWORD dwCommEvent,int nPortNum)
		{
			THDebug(_T("Seiral Port:%d Event:%d"),nPortNum,dwCommEvent);
		}
		virtual unsigned int OnHandleBufferCharEvent(const void *pData,unsigned int nLen,int nPortNum)
		{
			THDebug(_T("Seiral Port:%d Data:%s"),nPortNum,THStringConv::FormatString(pData,nLen));
			return nLen;
		}
	};
	THSerialPort m_port;
	m_port.SetHandler(MyClass);
	if (m_port.InitPort(1,115200))
	{
		if (m_port.IsCanWrite())
			m_port.WriteToPort(_T("AT+CMGR=4\r\n"));
		//or
		m_port.WriteToPort(_T("AT+CMGR=3\r\n"),TRUE,TRUE);
	}
	//when release
	m_port.ClosePort();
	//ͬ��ģʽ
	THSerialPort m_port;
	if (m_port.InitPort(1,115200,0))
	{
		m_port.WriteToPort(_T("AT+CMGR=3\r\n"));
		THDebug(_T("%s"),m_port.ReadFromPort(1024));
	}
	m_port.ClosePort();
</pre>*/
class THSerialPort : public THThread
{
public:
	// contruction and destruction
	THSerialPort()
	{
		m_handler = NULL;
		m_hComm = INVALID_HANDLE_VALUE;
		// initialize overlapped structure members to zero
		m_ov.Offset = 0;
		m_ov.OffsetHigh = 0;
		m_ov.hEvent = NULL;
		// create events
		m_ov.hEvent = NULL;
		m_hWriteEvent = NULL;
		m_hShutdownEvent = NULL;
		m_szWriteBuffer = NULL;
		m_nWriteSize=0;
		m_bThreadAlive = FALSE;
		memset(m_hEventArray,0,sizeof(m_hEventArray));
		memset(&m_CommTimeouts,0,sizeof(m_CommTimeouts));
		memset(&m_dcb,0,sizeof(m_dcb));
		m_nPortNr=0;
		m_nWriteBufferSize=0;
		m_bOpened=FALSE;
	}

	virtual	~THSerialPort()
	{
		ClosePort();
		if (m_hShutdownEvent)
		{
			do
			{
				SetEvent(m_hShutdownEvent);
			}while (m_bThreadAlive);
		}
		// if the port is still opened: close it 
		if (m_hComm != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_hComm);
			m_hComm = INVALID_HANDLE_VALUE;
		}
		// Close Handles  
		if(m_hShutdownEvent)
		{
			CloseHandle( m_hShutdownEvent); 
			m_hShutdownEvent=NULL;
		}
		if(m_ov.hEvent)
		{
			CloseHandle( m_ov.hEvent );
			m_ov.hEvent=NULL;
		}
		if(m_hWriteEvent)
		{
			CloseHandle( m_hWriteEvent );
			m_hWriteEvent=NULL;
		}
		if (m_szWriteBuffer)
		{
			delete [] m_szWriteBuffer;
			m_szWriteBuffer=NULL;
		}
	}

	void SetHandler(ITHSerialPortEventHandler *handler){m_handler=handler;}

	/**
	* @brief �򿪴���
	* @param portno			���ں�
	* @param baudrate		�����ʣ����Ϊ 115200
	* @param writebufsize	д�����С�������첽ģʽ�����ʹ��ͬ��ģʽ����ֵΪ0
	* @param paritytype		��żУ��
	* @param databits		����λ
	* @param stopbits		ֹͣλ
	* @param dwCommEvents	��Ҫ��Ӧ��Comm�¼��������첽ģʽ��define EV_
	* @return �Ƿ�ɹ�
	*/
	BOOL InitPort(UINT portno=1,UINT baudrate=19200,UINT writebufsize=1024,char paritytype=SPPARITYTYPE_NONE,UINT databits=8,UINT stopbits=1,DWORD dwCommEvents=EV_RXCHAR)
	{
		ClosePort();
		if (writebufsize!=0)
		{
			// create events
			if (m_ov.hEvent != NULL)
				ResetEvent(m_ov.hEvent);
			else
				m_ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

			if (m_hWriteEvent != NULL)
				ResetEvent(m_hWriteEvent);
			else
				m_hWriteEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			
			if (m_hShutdownEvent != NULL)
				ResetEvent(m_hShutdownEvent);
			else
				m_hShutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

			// initialize the event objects
			m_hEventArray[0] = m_hShutdownEvent;	// highest priority
			m_hEventArray[1] = m_ov.hEvent;
			m_hEventArray[2] = m_hWriteEvent;
		}
		else
		{
			if(m_hShutdownEvent)
			{
				CloseHandle(m_hShutdownEvent);
				m_hShutdownEvent=NULL;
			}
			if(m_ov.hEvent)
			{
				CloseHandle(m_ov.hEvent);
				m_ov.hEvent=NULL;
			}
			if(m_hWriteEvent)
			{
				CloseHandle(m_hWriteEvent);
				m_hWriteEvent=NULL;
			}
			m_hEventArray[0] = NULL;
			m_hEventArray[1] = NULL;
			m_hEventArray[2] = NULL;
		}

		if (m_szWriteBuffer != NULL)
		{
			delete [] m_szWriteBuffer;
			m_szWriteBuffer=NULL;
		}
		if (writebufsize!=0)
			m_szWriteBuffer=new char[writebufsize];

		m_nPortNr = portno;

		m_nWriteBufferSize = writebufsize;

		BOOL bResult = FALSE;
		// now it critical!
		m_mutex.Lock();
		// if the port is already opened: close it
		if (m_hComm != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_hComm);
			m_hComm = INVALID_HANDLE_VALUE;
		}
		// prepare port strings
		THString szPort,szBaud;
		szPort.Format(_T("COM%d"),portno);
		szBaud.Format(_T("baud=%d parity=%c data=%d stop=%d"),baudrate,paritytype,databits,stopbits);

		// get a handle to the port
		m_hComm = CreateFile(szPort,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,writebufsize==0?0:FILE_FLAG_OVERLAPPED,0);
		if (m_hComm == INVALID_HANDLE_VALUE)
		{
			m_mutex.Unlock();
			return FALSE;
		}

		// set the timeout values
		m_CommTimeouts.ReadIntervalTimeout = 1000;
		m_CommTimeouts.ReadTotalTimeoutMultiplier = 1000;
		m_CommTimeouts.ReadTotalTimeoutConstant = 1000;
		m_CommTimeouts.WriteTotalTimeoutMultiplier = 1000;
		m_CommTimeouts.WriteTotalTimeoutConstant = 1000;

		// configure
		if (SetCommTimeouts(m_hComm, &m_CommTimeouts))
		{						   
			if (SetCommMask(m_hComm, dwCommEvents))
			{
				if (GetCommState(m_hComm, &m_dcb))
				{
					m_dcb.EvtChar = 'q';
					m_dcb.fRtsControl = RTS_CONTROL_ENABLE;		// set RTS bit high!
					if (BuildCommDCB(szBaud, &m_dcb))
					{
						if (SetCommState(m_hComm, &m_dcb))
							; // normal operation... continue
						else
							ProcessErrorMessage(SPERRORTYPE_SETCOMMSTATE);
					}
					else
						ProcessErrorMessage(SPERRORTYPE_BUILDCOMMDCB);
				}
				else
					ProcessErrorMessage(SPERRORTYPE_GETCOMMSTATE);
			}
			else
				ProcessErrorMessage(SPERRORTYPE_SETCOMMMASK);
		}
		else
			ProcessErrorMessage(SPERRORTYPE_SETCOMMTIMEOUT);
		// flush the port
		PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);
		// release critical section
		m_mutex.Unlock();
		//�����첽ģʽ�����߳�
		if (writebufsize!=0)
			StartThread(1);
		m_bOpened=TRUE;
		return TRUE;
	}

	/**
	* @brief �رմ���
	*/
	void ClosePort()
	{
		// if the thread is alive: Kill
		if (m_hShutdownEvent)
		{
			do
			{
				SetEvent(m_hShutdownEvent);
			} while (m_bThreadAlive);
		}
		if (m_nWriteBufferSize!=0)
			SetEvent(m_hShutdownEvent);
		else
		{
			if (m_hComm!=INVALID_HANDLE_VALUE)
			{
				CloseHandle(m_hComm);
				m_hComm=INVALID_HANDLE_VALUE;
			}
		}
		KillThread(1);
		m_bOpened=FALSE;
	}

	/**
	* @brief ��ȡд�뻺������С�����첽ģʽ��ʹ��
	*/
	DWORD GetWriteBufferSize(){return m_nWriteBufferSize;}

	/**
	* @brief ��ȡDCB����
	*/
	DCB GetDCB(){return m_dcb;}

	/**
	* @brief �ж��Ƿ�����豸
	* @return �Ƿ�����豸
	*/
	BOOL IsOpened(){return m_bOpened;}

	/**
	* @brief �Ƿ��������д�룬���첽ģʽ��ʹ��
	*/
	BOOL IsCanWrite(){return m_nWriteSize==0;}

	/**
	* @brief ��˿�д���ݣ��첽ģʽ�´�С���ܳ���InitPortʱָ���Ļ����С
	* @param buf		����ָ��
	* @param n			���ݴ�С���첽ģʽ�У�������͵��ֽ�������ָ���Ļ����С��������ѭ�����ͣ�ֱ��ȫ�������������л���еȴ�
	* @param bWaitSend	��������ڷ������ݣ��Ƿ���еȴ����ͣ����첽ģʽ��ʹ��
	*/
	BOOL WriteToPortBin(const void* buf,unsigned int n,BOOL bWaitSend=FALSE)
	{
		if (m_hComm==INVALID_HANDLE_VALUE) return FALSE;
		THSingleLock lock(&m_sendmutex);
		DWORD sended=0;
		const char *tmpbuf=(const char *)buf;
		//�ж��Ƿ���ʹ���첽ģʽ
		if (m_nWriteBufferSize!=0)
		{
			if (m_nWriteSize!=0)
			{
				if (!bWaitSend)
					return FALSE;
				else
				{
					while(1)
					{
						if (m_nWriteSize==0) break;
						Sleep(100);
					}
				}
			}
			unsigned int len;
			while(sended<n)
			{
				//memset(m_szWriteBuffer,0,m_nWriteBufferSize);
				len=n>m_nWriteBufferSize?m_nWriteBufferSize:n;
				memcpy(m_szWriteBuffer,tmpbuf+sended,len);
				sended+=len;
				m_nWriteSize=len;
				SetEvent(m_hWriteEvent);
				//������͵��ֽ�������ָ���Ļ����С��������ѭ�����ͣ�ֱ��ȫ�������������л���еȴ�
				if (sended<n)
				{
					while(1)
					{
						if (m_nWriteSize==0) break;
						Sleep(100);
					}
				}
			}
		}
		else
		{
			DWORD dwNumWrite;
			while(sended<n)
			{
				if (!WriteFile(m_hComm,tmpbuf+sended,n-sended,&dwNumWrite,NULL))
					return FALSE;
				sended+=dwNumWrite;
			}
		}
		return TRUE;
	}

	/**
	* @brief ��˿�д�ַ������ݣ���С���ܳ���InitPortʱָ���Ļ����С
	* @param string		�ַ�������
	* @param bAscii		�Ƿ���Ҫ���ַ���ת����ascii��ʽ�ٷ���
	* @param bWaitSend	��������ڷ������ݣ��Ƿ���еȴ����ͣ����첽ģʽ��ʹ��
	*/
	BOOL WriteToPort(LPCTSTR string,BOOL bAscii=TRUE,BOOL bWaitSend=FALSE)
	{
		if (!bAscii)
			return WriteToPortBin((const void *)string,(unsigned int)_tcslen(string)*sizeof(TCHAR),bWaitSend);
		char *tmp=THCharset::t2a(string);
		if (!tmp) return FALSE;
		BOOL ret=WriteToPortBin(tmp,(unsigned int)_tcslen(string),bWaitSend);
		THCharset::free(tmp);
		return ret;
	}

	/**
	* @brief �Ӷ˿ڶ����ݣ�����ͬ��ģʽ��ʹ��
	* @param pData		����ָ��
	* @param nLength	���峤��
	* @return ����ʵ�ʶ������ݳ���
	*/
	int ReadFromPort(void* pData,int nLength)
	{
		if (m_hComm==INVALID_HANDLE_VALUE) return FALSE;
		//�ж��Ƿ���ʹ���첽ģʽ
		if (m_nWriteBufferSize!=0) return 0;
		DWORD dwNumRead;
		ReadFile(m_hComm, pData, (DWORD)nLength, &dwNumRead, NULL);
		return (int)dwNumRead;
	}

	/**
	* @brief �Ӷ˿ڶ����ݣ�����ͬ��ģʽ��ʹ��
	* @param nMax		����ȡ��С
	* @param bAscii		�Ƿ���Ҫ���յ����ݵ���ascii��ʽ����
	* @return ����ʵ�ʶ������ݳ���
	*/
	THString ReadFromPort(int nMax,BOOL bAscii=TRUE)
	{
		THString ret;
		char *buf=new char[nMax+1];
		if (buf)
		{
			int read=ReadFromPort(buf,nMax);
			*(buf+read)='\0';
			ret=THCharset::a2t(buf);
			delete [] buf;
		}
		return ret;
	}

	/**
	* @brief ���ַ�������ת����T��ʽ
	* @param buf		����ָ��
	* @param len		��С
	* @param bAscii		�Ƿ���Ҫ���յ����ݵ���ascii��ʽ����
	* @return ����ʵ�ʶ������ݳ���
	*/
	static THString BufferToString(const void *buf,unsigned int len,BOOL bAscii=TRUE)
	{
		THString ret;
		if (bAscii==FALSE)
			ret.SetString((LPCTSTR)buf,len);
		else
		{
			char *tmpbuf=new char[len+1];
			if (tmpbuf)
			{
				memcpy(tmpbuf,buf,len);
				*(tmpbuf+len)='\0';
				ret=THCharset::a2t(tmpbuf);
				delete [] tmpbuf;
			}
		}
		return ret;
	}
protected:
	// protected memberfunctions
	void ProcessErrorMessage(int nErrorType)
	{
		if (m_handler) m_handler->OnHandleError(nErrorType,m_nPortNr);
	}

	virtual void ThreadFunc(int threadid,void *adddata)
	{
		// Set the status variable in the dialog class to
		// TRUE to indicate the thread is running.
		m_bThreadAlive = TRUE;
		// Misc. variables
		DWORD BytesTransfered = 0; 
		DWORD Event = 0;
		DWORD CommEvent = 0;
		DWORD dwError = 0;
		COMSTAT comstat;
		BOOL  bResult = TRUE;

		memset(&comstat,0,sizeof(COMSTAT));

		// Clear comm buffers at startup
		if (m_hComm!=INVALID_HANDLE_VALUE) PurgeComm(m_hComm,PURGE_RXCLEAR|PURGE_TXCLEAR|PURGE_RXABORT|PURGE_TXABORT);

		// begin forever loop.  This loop will run as long as the thread is alive.
		while(m_hComm!=INVALID_HANDLE_VALUE)
		{
			// Make a call to WaitCommEvent().  This call will return immediatly
			// because our port was created as an async port (FILE_FLAG_OVERLAPPED
			// and an m_OverlappedStructerlapped structure specified).  This call will cause the 
			// m_OverlappedStructerlapped element m_OverlappedStruct.hEvent, which is part of the m_hEventArray to 
			// be placed in a non-signeled state if there are no bytes available to be read,
			// or to a signeled state if there are bytes available.  If this event handle 
			// is set to the non-signeled state, it will be set to signeled when a 
			// character arrives at the port.

			// we do this for each port!

			bResult = WaitCommEvent(m_hComm, &Event, &m_ov);

			if (!bResult)  
			{ 
				// If WaitCommEvent() returns FALSE, process the last error to determin
				// the reason..
				switch (dwError = GetLastError()) 
				{ 
				case ERROR_IO_PENDING: 	
					{ 
						// This is a normal return value if there are no bytes
						// to read at the port.
						// Do nothing and continue
						break;
					}
				case 87:
					{
						// Under Windows NT, this value is returned for some reason.
						// I have not investigated why, but it is also a valid reply
						// Also do nothing and continue.
						break;
					}
				default:
					{
						// All other error codes indicate a serious error has
						// occured.  Process this error.
						ProcessErrorMessage(SPERRORTYPE_WAITCOMMEVENT);
						break;
					}
				}
			}
			else
			{
				// If WaitCommEvent() returns TRUE, check to be sure there are
				// actually bytes in the buffer to read.  
				//
				// If you are reading more than one byte at a time from the buffer 
				// (which this program does not do) you will have the situation occur 
				// where the first byte to arrive will cause the WaitForMultipleObjects() 
				// function to stop waiting.  The WaitForMultipleObjects() function 
				// resets the event handle in m_OverlappedStruct.hEvent to the non-signelead state
				// as it returns.  
				//
				// If in the time between the reset of this event and the call to 
				// ReadFile() more bytes arrive, the m_OverlappedStruct.hEvent handle will be set again
				// to the signeled state. When the call to ReadFile() occurs, it will 
				// read all of the bytes from the buffer, and the program will
				// loop back around to WaitCommEvent().
				// 
				// At this point you will be in the situation where m_OverlappedStruct.hEvent is set,
				// but there are no bytes available to read.  If you proceed and call
				// ReadFile(), it will return immediatly due to the async port setup, but
				// GetOverlappedResults() will not return until the next character arrives.
				//
				// It is not desirable for the GetOverlappedResults() function to be in 
				// this state.  The thread shutdown event (event 0) and the WriteFile()
				// event (Event2) will not work if the thread is blocked by GetOverlappedResults().
				//
				// The solution to this is to check the buffer with a call to ClearCommError().
				// This call will reset the event handle, and if there are no bytes to read
				// we can loop back through WaitCommEvent() again, then proceed.
				// If there are really bytes to read, do nothing and proceed.
			
				bResult = ClearCommError(m_hComm, &dwError, &comstat);

				if (comstat.cbInQue == 0)
					continue;
			}	// end if bResult

			// Main wait function.  This function will normally block the thread
			// until one of nine events occur that require action.
			Event = WaitForMultipleObjects(3, m_hEventArray, FALSE, INFINITE);

			switch (Event)
			{
			case 0:
				{
					// Shutdown event.  This is event zero so it will be
					// the higest priority and be serviced first.
					CloseHandle(m_hComm);
					m_hComm=INVALID_HANDLE_VALUE;
					m_bThreadAlive=FALSE;
					return;
					break;
				}
			case 1:	// read event
				{
					GetCommMask(m_hComm, &CommEvent);
					if (CommEvent & EV_RXCHAR) ReceiveChar(comstat);
					if (CommEvent != EV_RXCHAR && m_handler) m_handler->OnHandleFlagEvent(CommEvent,m_nPortNr);
					break;
				}  
			case 2: // write event
				{
					// Write character event from port
					WriteChar();
					break;
				}

			} // end switch

		} // close forever loop
	}

	void ReceiveChar(COMSTAT comstat)
	{
		BOOL  bRead = TRUE; 
		BOOL  bResult = TRUE;
		DWORD dwError = 0;
		DWORD BytesRead = 0;
		unsigned char RXBuff;
		while(1)
		{ 
			// Gain ownership of the comm port critical section.
			// This process guarantees no other part of this program 
			// is using the port object. 
			m_mutex.Lock();
			// ClearCommError() will update the COMSTAT structure and
			// clear any other errors.
			bResult = ClearCommError(m_hComm, &dwError, &comstat);
			m_mutex.Unlock();
			// start forever loop.  I use this type of loop because I
			// do not know at runtime how many loops this will have to
			// run. My solution is to start a forever loop and to
			// break out of it when I have processed all of the
			// data available.  Be careful with this approach and
			// be sure your loop will exit.
			// My reasons for this are not as clear in this sample 
			// as it is in my production code, but I have found this 
			// solutiion to be the most efficient way to do this.
			if (comstat.cbInQue == 0)
			{
				// break out when all bytes have been read
				break;
			}
			m_mutex.Lock();
			if (bRead)
			{
				bResult = ReadFile(m_hComm,&RXBuff,1,&BytesRead,&m_ov);
				// deal with the error code 
				if (!bResult)  
				{ 
					switch (dwError = GetLastError()) 
					{ 
						case ERROR_IO_PENDING: 	
							{ 
								// asynchronous i/o is still in progress 
								// Proceed on to GetOverlappedResults();
								bRead = FALSE;
								break;
							}
						default:
							{
								// Another error has occured.  Process this error.
								ProcessErrorMessage(SPERRORTYPE_READFILE);
								break;
							} 
					}
				}
				else
				{
					// ReadFile() returned complete. It is not necessary to call GetOverlappedResults()
					bRead = TRUE;
				}
			}  // close if (bRead)
			if (!bRead)
			{
				bRead = TRUE;
				bResult = GetOverlappedResult(m_hComm,&m_ov,&BytesRead,TRUE);

				// deal with the error code 
				if (!bResult)  
					ProcessErrorMessage(SPERRORTYPE_READFILEOVERLAP);
			}  // close if (!bRead)
			m_mutex.Unlock();
			// notify parent that a byte was received
			if (m_handler) m_handler->OnHandleCharEvent(RXBuff,m_nPortNr);
		} // end forever loop
	}
	void WriteChar()
	{
		if (m_nWriteSize==0) return;
		BOOL bWrite = TRUE;
		BOOL bResult = TRUE;

		DWORD BytesSent = 0;

		ResetEvent(m_hWriteEvent);

		// Gain ownership of the critical section
		m_mutex.Lock();

		if (bWrite)
		{
			// Initailize variables
			m_ov.Offset = 0;
			m_ov.OffsetHigh = 0;

			// Clear buffer
			PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

			bResult = WriteFile(m_hComm,m_szWriteBuffer,m_nWriteSize,&BytesSent,&m_ov);

			// deal with any error codes
			if (!bResult)  
			{
				DWORD dwError = GetLastError();
				switch (dwError)
				{
					case ERROR_IO_PENDING:
						{
							// continue to GetOverlappedResults()
							BytesSent = 0;
							bWrite = FALSE;
							break;
						}
					default:
						{
							// all other error codes
							ProcessErrorMessage(SPERRORTYPE_WRITEFILE);
						}
				}
			} 
		}
		if (!bWrite)
		{
			bWrite = TRUE;
			bResult = GetOverlappedResult(m_hComm,&m_ov,&BytesSent,TRUE);
			// deal with the error code 
	//		if (!bResult)  
			{
	//			port->ProcessErrorMessage("GetOverlappedResults() in WriteFile()");
			}	
		}
		m_mutex.Unlock();
		m_nWriteSize=0;
		//Verify that the data size send equals what we tried to send
		//if (BytesSent != port->m_nWriteSize){}	// Length of message to send)
	//	::SendMessage((port->m_pOwner)->m_hWnd, SPEVTTYPE_TXEMPTY_DETECTED, (WPARAM) RXBuff, (LPARAM) port->m_nPortNr);
	//	::SendMessage((port->m_pOwner)->m_hWnd, SPEVTTYPE_TXEMPTY_DETECTED,0,(LPARAM) port->m_nPortNr);
	}

	volatile int m_nWriteSize;
	HANDLE m_hComm;

	// synchronisation objects
	THMutex m_mutex;
	volatile BOOL m_bThreadAlive;

	// handles
	HANDLE m_hWriteEvent;
	HANDLE m_hShutdownEvent;

	// Event array. 
	// One element is used for each event. There are two event handles for each port.
	// A Write event and a receive character event which is located in the overlapped structure (m_ov.hEvent).
	// There is a general shutdown when the port is closed. 
	HANDLE m_hEventArray[3];

	// structures
	OVERLAPPED m_ov;
	COMMTIMEOUTS m_CommTimeouts;
	DCB m_dcb;

	// misc
	UINT m_nPortNr;
	char* m_szWriteBuffer;
	DWORD m_nWriteBufferSize;
	ITHSerialPortEventHandler *m_handler;
	THMutex m_sendmutex;
	BOOL m_bOpened;
};
