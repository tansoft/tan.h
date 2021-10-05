#pragma once

#include <THSystem.h>
#include <THString.h>
#include <THSyn.h>
#include <THFile.h>
#include <THNetWork.h>
#include <THMemBuf.h>
#include <THThread.h>
#include <THINet.h>
#include <THCharset.h>

#ifndef SIO_UDP_CONNRESET
	#define	SIO_UDP_CONNRESET	_WSAIOW(IOC_VENDOR,12)
#endif

#define IsNT()	(GetVersion()<0x80000000)

/** �ײ�Э������*/
typedef enum _RawProtocol{
	RawProtocol_Tcp,
	RawProtocol_Udp,
	RawProtocol_Unknown
}RawProtocol;

#define MAX_PACKET_LEN			65536	//������С

class THRawSocket;

/**
* @brief �ײ�Socket�ص���
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-08-06 �½���
*/
class ITHRawSocketEventHandler
{
public:
	/**
	* @brief ����������Ӧ
	* @param cls		�ײ�socket��ָ��
	* @param sock		�ײ�socket���
	* @param bufdata	����buffer
	* @param buflen		���ݰ���С
	* @param paddr		socket��ַ��TCPΪ�գ�
	* @param adddata	��������
	* @return �Ƿ�����buffer
	*/
	virtual BOOL ReceiveHandler(THRawSocket *cls,RAWSOCKET sock,const void *bufdata,int buflen,struct sockaddr_in *paddr,void *adddata)=0;
	/**
	* @brief ����������Ӧ��TCP����״̬����������״̬���У�
	* @param cls		�ײ�socket��ָ��
	* @param parent		����socket���
	* @param sock		�ײ�socket���
	* @param paddr		socket��ַ
	* @param paddrpeer	�Է�socket��ַ
	* @param adddata	��������
	*/
	virtual void AcceptHandler(THRawSocket *cls,RAWSOCKET parent,RAWSOCKET sock,struct sockaddr_in *paddr,struct sockaddr_in *paddrpeer,void *adddata){}
	/**
	* @brief ������Ӧ
	* @param cls		�ײ�socket��ָ��
	* @param sock		�ײ�socket���
	* @param errcode	�������
	* @param adddata	��������
	*/
	virtual void ErrorHandler(THRawSocket *cls,RAWSOCKET sock,DWORD errcode,void *adddata)=0;
};

/**
* @brief �ײ�Socket����ص���
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-09-07 �½���
*/
/**<pre>
ʹ��Sample��
	//����
	virtual UINT IsBufferReady(THRawSocket *cls,RAWSOCKET sock,const void *bufdata,int buflen,struct sockaddr_in *paddr,void *adddata)=0;
	virtual void BufferedReceiveHandler(THRawSocket *cls,RAWSOCKET sock,const void *bufdata,int buflen,struct sockaddr_in *paddr,void *adddata)=0;
	//ע�⣺�̳�ErrorHandlerʱ�������ITHRawSocketBufferedEventHandler::ErrorHandler�Խ��л������ͷ�
</pre>*/
class ITHRawSocketBufferedEventHandler : public ITHRawSocketEventHandler
{
public:
	/**
	* @brief �������������Ӧ
	* @param cls		�ײ�socket��ָ��
	* @param sock		�ײ�socket���
	* @param bufdata	����buffer
	* @param buflen		���ݰ���С
	* @param paddr		socket��ַ��TCPΪ�գ�
	* @param adddata	��������
	* @return �Ƿ�����buffer
	*/
	virtual BOOL ReceiveHandler(THRawSocket *cls,RAWSOCKET sock,const void *bufdata,int buflen,struct sockaddr_in *paddr,void *adddata)
	{
		if (m_buf.GetBufferedSize(sock)==0)
		{
			//�ȳ���ֱ�ӷ���
			while(buflen>0)
			{
				UINT readylen=IsBufferReady(cls,sock,bufdata,buflen,paddr,adddata);
				if (readylen>0)
					BufferedReceiveHandler(cls,sock,bufdata,readylen,paddr,adddata);
				else
					break;
				bufdata=((const char *)bufdata)+readylen;
				buflen-=readylen;
			}
			if (buflen>0)
			{
				if (!m_buf.AddBuf(sock,bufdata,buflen)) return FALSE;
			}
			return TRUE;
		}
		//�ڴ�������
		if (!m_buf.AddBuf(sock,bufdata,buflen)) return FALSE;
		UINT size=m_buf.GetBufferedSize(sock);
		if (size>0)
		{
			UINT len;
			UINT nDeletelen=0;
			char *buf=(char *)m_buf.GetBuf(sock,&len);
			while(buf && len>0)
			{
				UINT readylen=IsBufferReady(cls,sock,buf,len,paddr,adddata);
				if (readylen>0)
				{
					BufferedReceiveHandler(cls,sock,buf,readylen,paddr,adddata);
					len-=readylen;
					buf+=readylen;
					nDeletelen+=readylen;
				}
				if (readylen==0 || len==0) break;
			}
			m_buf.ReleaseGetBufLock(sock,nDeletelen);
		}
		return TRUE;
/*		//old imp code
		//�ڴ�������
		if (!m_buf.AddBuf(sock,bufdata,buflen)) return FALSE;
		UINT size=m_buf.GetBufferedSize(sock);
		if (size>0)
		{
			char *buf=new char[size];
			if (buf)
			{
				while(m_buf.GetBuf(sock,buf,size,NULL,TRUE))
				{
					UINT readylen=IsBufferReady(cls,sock,buf,size,paddr,adddata);
					if (readylen>0)
					{
						m_buf.GetBuf(sock,buf,readylen,NULL,FALSE);
						BufferedReceiveHandler(cls,sock,buf,readylen,paddr,adddata);
						size-=readylen;
					}
					if (readylen==0 || size==0) break;
				}
				delete [] buf;
			}
		}
		return TRUE;*/
	}

	/**
	* @brief �ж������Ƿ����������Ӧ
	* @param cls		�ײ�socket��ָ��
	* @param sock		�ײ�socket���
	* @param bufdata	����buffer
	* @param buflen		���ݰ���С
	* @param paddr		socket��ַ��TCPΪ�գ�
	* @param adddata	��������
	* @return �Ƿ������������ݣ�����з����������ݵĳ���
	*/
	virtual UINT IsBufferReady(THRawSocket *cls,RAWSOCKET sock,const void *bufdata,int buflen,struct sockaddr_in *paddr,void *adddata)=0;

	/**
	* @brief ��������������Ӧ������ֱ����IsBufferReady�����겻���ú���
	* @param cls		�ײ�socket��ָ��
	* @param sock		�ײ�socket���
	* @param bufdata	����buffer
	* @param buflen		���ݰ���С
	* @param paddr		socket��ַ��TCPΪ�գ�
	* @param adddata	��������
	*/
	virtual void BufferedReceiveHandler(THRawSocket *cls,RAWSOCKET sock,const void *bufdata,int buflen,struct sockaddr_in *paddr,void *adddata){}

	virtual void ErrorHandler(THRawSocket *cls,RAWSOCKET sock,DWORD errcode,void *adddata)
	{
		m_buf.Empty(sock);
	}
protected:
	THMemBufManager<RAWSOCKET> m_buf;
};

#define DYNSERVERURL_SERVERUPDATE	_T("http://dynserver.tansoft.net/dynserver/serverupdate.php?sid=%d&rnd=%d")
#define DYNSERVERURL_CLIENTREQUEST	_T("http://dynserver.tansoft.net/dynserver/clientrequest.php?sid=%d&rnd=%d")
//http://dynserver.tansoft.net/dynserver/clientrequest.php?sid=123 == http://123.dynserver.tansoft.net/dynserver/
#define RAWSOCKET_THREADID_RECEIVE		1

/**
* @brief �ײ�Э���װ��
* @author Barry
* @2007-07-07 �½���
* @2007-09-09 ���붯̬������֧��
* @2008-02-21 ����UDP�Զ�����ѡ��֧��
* @2008-02-25 ����Listenģʽ�¶��Listen����ĳ�ͻ���⣬�Ľ�һЩ����Ч��
*/
/**<pre>
ʹ��Sample��
	class EventHandler : public ITHRawSocketEventHandler
	{
		XXX
	};
	THRawSocket socket;
	EventHandler event;
	socket.SetHandler(&event,this);
	RAWSOCKET sock=socket.CreateSocket(THRawSocket::GetIpFromHostName(_T("127.0.0.1")),1234);
	socket.Send(sock,_T("1234"),4);
	//��̬ģʽ��
	//����CreateSocketʱ��ָ����̬ģʽ
	//����ͨTHRawSocket�÷�һ�����ַ�������ͻ���ģʽ
	//�ڳ�ʼ��ʱ�����������ֱ�ʶ
	//SetServerUpdateTimer ���÷���������ipʱ������Ĭ��5����
	//SetupServerUpdateTimer ��ǿ��������������ʱ����ip����
</pre>*/
class THRawSocket : public THNetWork
{
public:
	/**
	* @brief ���죬������Ϣ��Ӧ��
	* @param handler	��Ϣ��Ӧ��ָ��
	* @param adddata	��������
	*/
	THRawSocket(ITHRawSocketEventHandler *handler=NULL,void *adddata=NULL):THNetWork(SocketVersion22)
	{
		m_handler=handler;
		m_adddata=adddata;
		m_nLastError=0;
		m_mtxFd.Lock();
		FD_ZERO(&m_fdSocketPool);
		m_mtxFd.Unlock();
		m_thread.SetCallFunc(TimerThreadCB);
		m_rawthread.SetCallFunc(MyThreadCallBack);
		m_sListening.SetFreeProc(MyFreeListen,this);
		SetServerUpdateTimer();
		//init thread
		StartListen();
	}
	virtual ~THRawSocket(void)
	{
		//fixme,close socket
		m_mtxFd.Lock();
		FD_ZERO(&m_fdSocketPool);
		m_mtxFd.Unlock();
		StopListen();
		m_http.CancelAllDownload();
		//����TerminateThread�п�����RecvServiceThread��ֱ�ӽ��е��ã�����Լ�����ѭ��������Ӧ��ʹ�ñ������ж��˳�
/*		if (m_hListenThread)
		{
			HANDLE h=m_hListenThread;
			m_hListenThread=NULL;
			while(!TerminateThread(h,0)) Sleep(500);
		}
		if (m_hRecvServiceThread)
		{
			HANDLE h=m_hRecvServiceThread;
			m_hRecvServiceThread=NULL;
			while(!TerminateThread(h,0)) Sleep(500);
		}*/
		//WSACleanup();//parent class called
	}

	virtual CString GetSocketLayerName(){return _T("THRawSocket");}
	/**
	* @brief ����UDPģʽ�£������͵�Ŀ���޷�����ʱ���Ƿ������ر�Socket����NT������Ч
	* @param nTimeVal	�Զ����¼��
	*/
	void SetUdpSocketAutoCloseBehavior(RAWSOCKET s,BOOL bAutoClose=FALSE)
	{
		if (IsNT())
		{
			DWORD dwBytesReturned=0;
			BOOL bNewBehavior=FALSE;
			WSAIoctl(s,SIO_UDP_CONNRESET,&bAutoClose,sizeof(bAutoClose),NULL,0,&dwBytesReturned,NULL,NULL);
		}
	}

	/**
	* @brief ����socketģʽ
	* @param s		socket
	* @param bBlock	����ģʽ
	* @return �Ƿ�ɹ�
	*/
	static BOOL SetSocketMode(RAWSOCKET s,BOOL bBlock=FALSE)
	{
		int iMode = bBlock? 0:1;
		return ioctlsocket(s, FIONBIO, (u_long FAR*) &iMode)==0;
	}

	/**
	* @brief ���ö�̬������ģʽ�£��Զ�����ip�ļ��
	* @param nTimeVal	�Զ����¼��
	*/
	void SetServerUpdateTimer(UINT nTimeVal=60000*5){m_nTimer=nTimeVal;}//5 mins

	/**
	* @brief ���ö�̬������ģʽ�£������Զ����¶�ʱ��
	* @param serviceid	�Զ����µ�serviceid
	*/
	void SetupServerUpdateTimer(int serviceid)
	{
		m_thread.StartTimer(serviceid,m_nTimer,TRUE,this);
	}

	/**
	* @brief ���ö�̬������ģʽ�£������Զ����¶�ʱ��
	* @param serviceid	�Զ����µ�serviceid
	*/
	void StopServerUpdateTimer()
	{
		m_thread.StopAllTimer();
	}

	/**
	* @brief ��ȡ��̬������ip����Ϊ����sHost��Ӧ��ip�����µ�host�ļ���
	* @param sid	serviceid
	* @param sHost	����
	* @param retip	��ȡ����ip
	* @return �Ƿ�ɹ�
	*/
	BOOL UpdateDynServerIpToHostFile(U32 sid,THString sHost,U32 *retip)
	{
		U32 ip=GetDynIpBySid(sid);
		if (ip==0) return FALSE;
		if (retip) *retip=ip;
		THString str;
		str=THSysMisc::GetSystemPath()+_T("\\drivers\\etc\\hosts");
		THStringArray ar;
		if (!THFile::GetContentAToStringArray(str,&ar)) return FALSE;
		BOOL bFound=FALSE;
		for(int i=0;i<ar.GetSize();i++)
		{
			THString stmp=ar[i];
			stmp.MakeLower();
			if (stmp.Find(sHost,0)!=-1)
			{
				//change it
				THString ipstr=THNetWork::GetAddrString(ip);
				if (stmp.Find(ipstr,0)!=-1) return TRUE;
				ar[i]=ipstr+_T(" ")+sHost;
				bFound=TRUE;
				break;
			}
		}
		THString ret;
		for(int i=0;i<ar.GetSize();i++)
			ret+=ar[i]+_T("\r\n");
		if (!bFound) ret+=THNetWork::GetAddrString(ip)+_T(" ")+sHost+_T("\r\n");
		if (!THFile::SaveStringToFileA(str,ret)) return FALSE;
		return TRUE;
	}

	/**
	* @brief ��ȡ��̬������ģʽ�£���������ip
	* @param sid	serviceid
	* @return ��������̬ip������˳�򣬳�����0
	*/
	U32 GetDynIpBySid(U32 sid)
	{
		if (sid==0) return 0;
		U32 ip=0;
		THSysMisc::srand();
		THString tmp;
		tmp.Format(DYNSERVERURL_CLIENTREQUEST,sid,rand());
		//get the realip
		THString filepath=THSysMisc::GenTempFileName();
		if (m_http.StartFileDownload(tmp,filepath,NULL,NULL,FALSE,TRUE)!=0)
		{
			THString ret=THFile::GetContentAToString(filepath);
			THString sip=THSimpleXml::GetParam(ret,_T("lastip"));
			if (!sip.IsEmpty()) ip=GetIpFromHostName(sip);
		}
		DeleteFile(filepath);
		return ip;
	}

	/**
	* @brief ������Ϣ��Ӧ��
	* @param handler	��Ϣ��Ӧ��ָ��
	* @param adddata	��������
	*/
	void SetHandler(ITHRawSocketEventHandler *handler,void *adddata){m_handler=handler;m_adddata=adddata;};

	virtual RAWSOCKET CreateSocketByHost(THString peerip,U16 peerport,U16 localport=0,RawProtocol protocol=RawProtocol_Tcp,BOOL bSelect=TRUE,U32 sid=0,BOOL bCanBindOther=TRUE,U32 localip=0){
		return CreateSocket(THRawSocket::GetIpFromHostNameN(peerip,NULL),peerport,localport,protocol,bSelect,sid,bCanBindOther,localip);
	}

	/**
	* @brief �½�socket
	* @param peerip		�Է�ip��Ϊ��Ϊ����ģʽ��������˳��
	* @param peerport	�Է��˿ڣ�Ϊ��Ϊ����ģʽ��
	* @param localport	�Ƽ��ı��ض˿ڣ�����󶨲��ˣ��Ͱ�����˿ڣ�
	* @param protocol	ʹ��Э��
	* @param bSelect	�Ƿ�ʼSelect������������socket���Լ�����
	* @param sid		�Ƿ�ʹ�ö�̬������ģʽ��sidָ��serviceid��Ϊ0Ϊ��ʹ��
	* @param bCanBindOther	��ʧ��ʱ���Ƿ�����󶨱�Ķ˿�
	* @param localip	ָ���󶨵ı���ip
	* @return			����socket���
	*/
	/**
		tcp���ӣ������peer�˿ڱ�ʾ��Ҫ����connect
		tcp���ӣ������peer�˿ڣ���ʾΪ������ģʽ���󶨶˿ڲ�����
	*/
	virtual RAWSOCKET CreateSocket(U32 peerip,U16 peerport,U16 localport=0,RawProtocol protocol=RawProtocol_Tcp,BOOL bSelect=TRUE,U32 sid=0,BOOL bCanBindOther=TRUE,U32 localip=0)
	{
		//peerport��Ϊ0˵����������socket�����ͳ�����peerip������
		if (peerip==0 && peerport!=0) return INVALID_SOCKET;
		BOOL bListenMode=FALSE;
		//tcp���ӣ������peer�˿ڱ�ʾ��Ҫ����connect
		//tcp���ӣ������peer�˿ڣ���ʾΪ������ģʽ���󶨶˿ڲ�����
		RAWSOCKET sock=INVALID_SOCKET;
		if (sid!=0)
		{
			if (peerport==0)
			{
				//server mode
				UpdateServerIp(sid);
				m_thread.StartTimer(sid,m_nTimer,TRUE,this);
			}
			else
			{
				//client mode
				peerip=GetDynIpBySid(sid);
				if (peerip==0) return INVALID_SOCKET;
			}
		}
		//create socket
		if (protocol==RawProtocol_Tcp)
		{
			sock=socket(AF_INET,SOCK_STREAM,0);
			if (sock==INVALID_SOCKET)
			{
				m_nLastError=WSAGetLastError();
				return INVALID_SOCKET;
			}
			BOOL nodelayval = TRUE;
			setsockopt(sock,IPPROTO_TCP,TCP_NODELAY,(const char *)&nodelayval,sizeof(BOOL));
		}
		else if (protocol==RawProtocol_Udp)
		{
			sock=socket(AF_INET,SOCK_DGRAM,0);
			if (sock==INVALID_SOCKET)
			{
				m_nLastError=WSAGetLastError();
				return INVALID_SOCKET;
			}
		}
		//set buffer size
		int nrcvbuf=81920; //and now the default buffer is 8192
		setsockopt(sock,SOL_SOCKET,SO_SNDBUF,(char *)&nrcvbuf,sizeof(nrcvbuf));
		nrcvbuf=81920;
		setsockopt(sock,SOL_SOCKET,SO_RCVBUF,(char *)&nrcvbuf,sizeof(nrcvbuf));
		//bind local port
		struct sockaddr_in sin;
		memset(&sin,0,sizeof(struct sockaddr_in));
		sin.sin_family=AF_INET;
		sin.sin_addr.s_addr=htonl(localip);
		sin.sin_port=htons(localport);
		if (bind(sock,(const struct sockaddr *)&sin,sizeof(sin))!=0)
		{
			if (!bCanBindOther)
			{
				m_nLastError=WSAGetLastError();
				return INVALID_SOCKET;
			}
			sin.sin_port=0;
			//���԰������˿�
			if (bind(sock,(const struct sockaddr *)&sin,sizeof(sin))!=0)
			{
				m_nLastError=WSAGetLastError();
				return INVALID_SOCKET;
			}
		}
		if (protocol==RawProtocol_Tcp)
		{
			if (peerip!=0 && peerport!=0)
			{
				struct sockaddr_in thataddr;
				memset(&thataddr,0,sizeof(struct sockaddr_in));
				thataddr.sin_family = AF_INET;
				thataddr.sin_addr.s_addr = htonl(peerip);
				thataddr.sin_port = htons(peerport);
				if (connect_route(sock,(LPSOCKADDR)&thataddr,sizeof(thataddr))==SOCKET_ERROR)
				{
					m_nLastError=WSAGetLastError();
					return INVALID_SOCKET;
				}
			}
			else
			{
				if (listen(sock,50)==SOCKET_ERROR)
				{
					m_nLastError=WSAGetLastError();
					return INVALID_SOCKET;
				}
				bListenMode=TRUE;
				m_rawthread.StartThread((int)sock,this,TIMEVAL_NOTDELAY);
				m_sListening.AddHead(sock);
			}
		}
		if (!bListenMode && bSelect)
		{
			m_mtxFd.Lock();
			FD_SET(sock,&m_fdSocketPool);
			m_mtxFd.Unlock();
		}
		return sock;
	}

	virtual void SetSockBufferSize(RAWSOCKET sock,int bz=81920)
	{
		int nrcvbuf=bz; //and now the default buffer is 8192
		setsockopt(sock,SOL_SOCKET,SO_SNDBUF,(char *)&nrcvbuf,sizeof(nrcvbuf));
		nrcvbuf=bz;
		setsockopt(sock,SOL_SOCKET,SO_RCVBUF,(char *)&nrcvbuf,sizeof(nrcvbuf));
	}
	/**
	* @brief �ͷ�socket
	* @param sock		socket���
	* @param errcode	�ص��в����Ĵ����
	*/
	virtual void FreeSocket(RAWSOCKET sock,DWORD errcode=0)
	{
		//fixme free buffer
		m_mtxFd.Lock();
		FD_CLR(sock,&m_fdSocketPool);
		m_mtxFd.Unlock();
		closesocket_route(sock);
		if (m_handler)
			m_handler->ErrorHandler(this,sock,errcode,m_adddata);
	}

	virtual BOOL Send(RAWSOCKET sock,THMemBuf *buf,const struct sockaddr_in *paddr=NULL,int TryTime=-1)
	{
		UINT len;
		void *pbuf=buf->GetBuf(&len,FALSE);
		return Send(sock,pbuf,len,paddr,TryTime);
	}

	virtual BOOL Send(RAWSOCKET sock,THString buf,const struct sockaddr_in *paddr=NULL,int TryTime=-1)
	{
		return Send(sock,(LPCTSTR)buf,buf.GetLength()*sizeof(TCHAR),paddr,TryTime);
	}
	/**
	* @brief ��������
	* @param sock		socket���
	* @param buf		����
	* @param len		����
	* @param paddr		sock��ַָ�루����UDP��ʽ��ʹ�ã�
	* @param TryTime	���Դ��������ڷ�����ģʽ
	* @return			�Ƿ��ͳɹ�
	*/
	virtual BOOL Send(RAWSOCKET sock,const void *buf,int len,const struct sockaddr_in *paddr=NULL,int TryTime=-1)
	{
		if (sock==INVALID_SOCKET) return FALSE;
		m_mtxSendLock.Lock();
		int sent=0;
		int ret;
		while(sent<len)
		{
			if (paddr)
				ret=sendto_route(sock,(const char *)buf+sent,len-sent,0,(const LPSOCKADDR)paddr,sizeof(struct sockaddr_in));
			else
				ret=send_route(sock,(const char *)buf+sent,len-sent,0);
			if (ret<=0)
			{
				int lasterr=WSAGetLastError();
				if (lasterr==WSAEWOULDBLOCK)
				{
					if (TryTime!=-1)
					{
						TryTime--;
						if (TryTime==0)
						{
							//break the timeout socket
							/*if (m_handler)
								m_handler->ErrorHandler(this,sock,m_nLastError,m_adddata);*/
							//FreeSocket(sock);
							//CMyDebug(_T("sock send timeout:%d,%d"),sock,len);
							m_mtxSendLock.Unlock();
							return FALSE;
						}
					}
					Sleep(10);
					continue;
				}
				m_nLastError=lasterr;
				m_mtxSendLock.Unlock();
				/*Send �����socket������return��ʱ����
					if (m_handler)
						m_handler->ErrorHandler(this,sock,m_nLastError,m_adddata);*/
				//FreeSocket(sock);
				//CMyDebug(_T("sock send error:%d,%d"),sock,len);
				return FALSE;
			}
			sent+=ret;
		}
		m_mtxSendLock.Unlock();
		return TRUE;
	}
	virtual BOOL Send(RAWSOCKET sock,THString buf,U32 peerip,U16 peerport,int TryTime)
	{
		return Send(sock,(LPCTSTR)buf,buf.GetLength()*sizeof(TCHAR),peerip,peerport,TryTime);
	}

	/**
	* @brief ��������
	* @param sock		socket���
	* @param buf		����
	* @param len		����
	* @param peerip		�Է�ip������UDP��ʽ��ʹ�ã�
	* @param peerport	�Է��˿ڣ�����UDP��ʽ��ʹ�ã�
	* @return			�Ƿ��ͳɹ�
	*/
	virtual BOOL Send(RAWSOCKET sock,const void *buf,int len,U32 peerip,U16 peerport,int TryTime)
	{
		if (peerip!=0 && peerport!=0)
		{
			struct sockaddr_in addr;
			memset(&addr,0,sizeof(struct sockaddr_in));
			addr.sin_family=AF_INET;
			addr.sin_addr.s_addr=htonl(peerip);
			addr.sin_port=htons(peerport);
			return Send(sock,buf,len,&addr);
		}
		return Send(sock,buf,len,NULL,TryTime);
	}

	/**
	* @brief ��������
	* @param sock		socket���
	* @param buf		����
	* @param len		����
	* @param peerip		�Է�ip������UDP��ʽ��ʹ�ã�
	* @param peerport	�Է��˿ڣ�����UDP��ʽ��ʹ�ã�
	* @return			�Ƿ��ͳɹ�
	*/
	virtual BOOL SendN(RAWSOCKET sock,const void *buf,int len,U32 peerip,U16 peerport,int TryTime)
	{
		if (peerip!=0 && peerport!=0)
		{
			struct sockaddr_in addr;
			memset(&addr,0,sizeof(struct sockaddr_in));
			addr.sin_family=AF_INET;
			addr.sin_addr.s_addr=peerip;
			addr.sin_port=peerport;
			return Send(sock,buf,len,&addr);
		}
		return Send(sock,buf,len,NULL,TryTime);
	}

	/**
	* @brief �������ݣ������ڷ�Selectģʽ
	* @param sock		socket���
	* @param buf		����
	* @param len		����
	* @param peerip		�Է�ip������UDP��ʽ��ʹ�ã�
	* @param peerport	�Է��˿ڣ�����UDP��ʽ��ʹ�ã�
	* @param nWaiting	��ʱʱ�䣬��ֵ����ȷ��Ϊ0Ϊ�������أ���λ����
	* @return			����-1��������0û�����ݣ������������ؽ������ݴ�С
	*/
	virtual int ReceiveFor(RAWSOCKET sock,void *buf,int len,U32 *peerip=NULL,U16 *peerport=NULL,int nWaiting=0)
	{
		struct sockaddr_in from;
		int fromlen=sizeof(from);
		int ret;
		DWORD dw=GetTickCount()+nWaiting;
		do
		{
			ret=Receive(sock,buf,len,MSG_PEEK,peerip,peerport);
			if (ret==-1) return -1;
			if (ret>0)
				return Receive(sock,buf,len,0,peerip,peerport);
			if (nWaiting<=0) return 0;
			Sleep(100);
		}while(GetTickCount()<dw);
		return 0;
	}

	virtual int Receive(RAWSOCKET sock,void *buf,int len,int flag,U32 *peerip,U16 *peerport)
	{
		int ret;
		if (peerip && peerport)
		{
			struct sockaddr_in addr;
			int len=sizeof(sockaddr_in);
			ret=recvfrom_route(sock,(char *)buf,len,flag,(SOCKADDR *)&addr,&len);
			*peerip=ntohl(addr.sin_addr.s_addr);
			*peerport=ntohs(addr.sin_port);
		}
		else
			ret=recv_route(sock,(char *)buf,len,flag);
		return ret;
	}
	virtual int Receive(RAWSOCKET sock,void *buf,int len,int flag=0,struct sockaddr *addr=NULL,int *fromlen=NULL)
	{
		int ret;
		if (addr && fromlen)
			ret=recvfrom_route(sock,(char *)buf,len,flag,addr,fromlen);
		else
			ret=recv_route(sock,(char *)buf,len,flag);
		return ret;
	}

	/**
	* @brief ��ȡ�������
	* @return ������
	*/
	int GetLastError(){return m_nLastError;}

	/**
	* @brief �����Ƿ��socket���м���
	* @param socket		socket���
	* @param bSelect	�Ƿ����
	*/
	virtual void SetSelectSocket(RAWSOCKET socket,BOOL bSelect)
	{
		m_mtxFd.Lock();
		if (bSelect)
		{
			FD_SET(socket,&m_fdSocketPool);
		}
		else
		{
			FD_CLR(socket,&m_fdSocketPool);
		}
		m_mtxFd.Unlock();
	}

	/**
	* @brief ��ȡ���ض˿�
	* @param socket		socket���
	* @return �˿ںţ�����˳��
	*/
	virtual U16 GetLocalPort(RAWSOCKET socket)
	{
		struct sockaddr_in sin;
		int socklen = sizeof(struct sockaddr_in);
		getsockname(socket,(struct sockaddr *)&sin,&socklen);
		return ntohs(sin.sin_port);
	}

	void StopListen()
	{
		StopServerUpdateTimer();
		m_rawthread.KillThread(RAWSOCKET_THREADID_RECEIVE);
		//m_rawthread.KillThread(RAWSOCKET_THREADID_LISTENING);
		m_sListening.RemoveAll();
	}

	void StartListen()
	{
		m_rawthread.StartThread(RAWSOCKET_THREADID_RECEIVE,this,TIMEVAL_NOTDELAY);
	}
protected:
	virtual int connect_route(RAWSOCKET s,const LPSOCKADDR addr,int addrsize){return connect(s,addr,addrsize);}
	virtual RAWSOCKET accept_route(RAWSOCKET s,LPSOCKADDR addr,int *addrlen){return accept(s,addr,addrlen);}
	virtual int recvfrom_route(RAWSOCKET s,char *buf,int len,int flags,LPSOCKADDR addr,int *addrlen){return recvfrom(s,buf,len,flags,addr,addrlen);}
	virtual int recv_route(RAWSOCKET s,char *buf,int len,int flags){return recv(s,buf,len,flags);}
	virtual int sendto_route(RAWSOCKET s,const char *buf,int len,int flags,const LPSOCKADDR to,int tolen){return sendto(s,buf,len,flags,to,tolen);}
	virtual int send_route(RAWSOCKET s,const char *buf,int len,int flags){return send(s,buf,len,flags);}
	virtual int closesocket_route(RAWSOCKET s){return closesocket(s);}

	ITHRawSocketEventHandler *m_handler;
	void *m_adddata;
	THMutex m_mtxSendLock;
	THMutex m_mtxFd;
	THINetHttp m_http;
	THTimerThread m_thread;
	THThread m_rawthread;
	UINT m_nTimer;
	fd_set m_fdSocketPool;
	THList<RAWSOCKET> m_sListening;
	//volatile HANDLE m_hRecvServiceThread;
	//volatile HANDLE m_hListenThread;
	int m_nLastError;

	static void MyFreeListen(void *key,void *value,void *adddata)
	{
		int ThreadId=(int)(INT_PTR)value;
		RAWSOCKET s=(RAWSOCKET)(INT_PTR)value;
		THRawSocket *sock=(THRawSocket *)adddata;
		if (s!=INVALID_SOCKET)
		{
			sock->closesocket_route(s);
			sock->m_rawthread.KillThread(ThreadId);
		}
	}

	void UpdateServerIp(int sid)
	{
		THString tmp;
		THSysMisc::srand();
		tmp.Format(DYNSERVERURL_SERVERUPDATE,sid,rand());
		m_http.StartMemDownload(tmp);//not need parse return
	}

	static void TimerThreadCB(int TimerId,THTimerThread *pCls,void *data)
	{
		((THRawSocket *)data)->UpdateServerIp(TimerId);
	}

	static void MyThreadCallBack(int ThreadId,THThread *pCls,void *data)
	{
		if (ThreadId==RAWSOCKET_THREADID_RECEIVE)
		{
			RecvServiceThread(data);
		}
		else
		{
			ListenServiceThread(ThreadId,data);
		}
	}

	static DWORD RecvServiceThread(LPVOID lpParam)
	{
		THRawSocket *layer=(THRawSocket *)lpParam;
		struct sockaddr_in scaddr;
		int caddsize;
		fd_set set,errset;
		timeval ti;
		int ret;
		//while(1)
		{
			//if (!layer->m_hRecvServiceThread) break;//exit the thread
			char buf[MAX_PACKET_LEN];
			if (layer->m_fdSocketPool.fd_count==0)
			{
				Sleep(100);
				return 0;//continue;
			}
			layer->m_mtxFd.Lock();
			memcpy(&set,&layer->m_fdSocketPool,sizeof(fd_set));
			memcpy(&errset,&layer->m_fdSocketPool,sizeof(fd_set));
			layer->m_mtxFd.Unlock();
			ti.tv_sec=1;
			ti.tv_usec=0;
			ret=select(0,&set,NULL,&errset,&ti);//param 1 is ingore in windows platfrom
			//if (!layer->m_hRecvServiceThread) break;//exit the thread
			if (ret<0)
			{
				//PrintLog("Select Error:ret:%d,Error:%d,the socket closed",ret,WSAGetLastError());
				Sleep(200);
				return 0;//continue;
			}
			else if (ret>0)
			{
				for(UINT i=0;i<errset.fd_count;i++)
				{
					//the socket have some error.
					layer->m_nLastError=WSAGetLastError();
					//PrintLog("ret:%d receive Error:%d,the socket %d closed",ret,errcode,set.fd_array[i]);
					layer->FreeSocket(errset.fd_array[i],layer->m_nLastError);
				}
				for(i=0;i<set.fd_count;i++)
				{
					caddsize=sizeof(scaddr);
					memset(&scaddr,0,sizeof(scaddr));
					ret=layer->recvfrom_route(set.fd_array[i],buf,MAX_PACKET_LEN,MSG_PEEK,(struct sockaddr *)&scaddr,&caddsize);
					if(ret>0)
					{
						BOOL bReceive=TRUE;
						if (layer->m_handler)
							bReceive=layer->m_handler->ReceiveHandler(layer,set.fd_array[i],buf,ret,&scaddr,layer->m_adddata);
						/*if (scaddr.sin_port==0)
							PrintLog("socket %d have %d data receive",(U32)set.fd_array[i],ret);
						else
							PrintLog("socket %d have %d data coming from %s:%d",(U32)set.fd_array[i],ret,inet_ntoa(scaddr.sin_addr),ntohs(scaddr.sin_port));*/
						if (bReceive)
							ret=layer->recvfrom_route(set.fd_array[i],buf,ret,0,(struct sockaddr *)&scaddr,&caddsize);
						/*if (ret<=0)
						{
							int ErrCode=WSAGetLastError();
							if (ErrCode!=WSAEMSGSIZE)
							{
								PrintLog("recvfrom len:%d,error,%d",ret,ErrCode);
								continue;
							}
							else
							{
								//fixme,�����
							}
						}
						else
						{
							if (layer->m_receivehandler)
								(layer->m_receivehandler)(layer,set.fd_array[i],buf,ret,&scaddr,layer->m_adddata);
						}*/
					}
					else
					{
						//the socket have some error.
						int lasterr=WSAGetLastError();
						//PrintLog("ret:%d receive Error:%d,the socket %d closed",ret,errcode,set.fd_array[i]);
						if (lasterr!=WSAEWOULDBLOCK)
						{
							layer->m_nLastError=lasterr;
							//PrintLog("ret:%d receive Error:%d,the socket %d closed",ret,errcode,set.fd_array[i]);
							layer->FreeSocket(set.fd_array[i],layer->m_nLastError);
						}
					}
				}
			}
		}
		return 0;
	}

	static DWORD ListenServiceThread(int ThreadId,LPVOID lpParam)
	{
		THRawSocket *layer=(THRawSocket *)lpParam;
		RAWSOCKET sock=(RAWSOCKET)ThreadId;
		RAWSOCKET sockTemp;
		struct sockaddr_in addr;
		int addrlen;

		//while(1)
		{
			//fixme ,accept function blocked,is it can exit normally?
			//if (!layer->m_hListenThread) break;//exit the thread
			addrlen=sizeof(struct sockaddr_in);
			sockTemp=layer->accept_route(sock,(SOCKADDR*)&addr,&addrlen); 
			if (sockTemp!=INVALID_SOCKET)
			{
				struct sockaddr_in sin;
				int socklen = sizeof(struct sockaddr_in);
				getsockname(sockTemp,(struct sockaddr *)&sin,&socklen);
				//socklen = sizeof(struct sockaddr_in);
				//getpeername(sockTemp,(struct sockaddr *)&sinpeer,&socklen);
				layer->m_mtxFd.Lock();
				FD_SET(sockTemp,&layer->m_fdSocketPool);
				layer->m_mtxFd.Unlock();
				if (layer->m_handler)
					layer->m_handler->AcceptHandler(layer,sock,sockTemp,&sin,&addr,layer->m_adddata);
			}
			else
				Sleep(500);
		}
		return 0;
	}
};

/**
* @brief �ײ�㲥Э���װ��
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2009-04-11 �½���
*/
/**<pre>
�÷���
</pre>*/
class THRawBoardcastSocket: public THRawSocket
{
public:
	THRawBoardcastSocket()
	{
	}
	virtual ~THRawBoardcastSocket()
	{
	}

	/**
	* @brief ���±��ص�ַ��Ϣ
	* @param bForceUpdate	�Ƿ�ǿ�Ƹ���
	*/
	void UpdateAddress(BOOL bForceUpdate=FALSE)
	{
		m_infomtx.Lock();
		if (m_bcips.GetSize()>0 && bForceUpdate==FALSE)
		{
			m_infomtx.Unlock();
			return;
		}
		m_lcips.RemoveAll();
		m_mcips.RemoveAll();
		m_bcips.RemoveAll();
		THAdaptersInfo info;
		for(UINT i=0;i<info.GetAdapterCount();i++)
		{
			THString ip=info.GetIpString(i);
			THString mask=info.GetMaskString(i);
			//not need to try Media disconnected Adapter and internet ip Adapter
			if (ip==_T("0.0.0.0") || mask==_T("255.255.255.255"))
				continue;
			U32 uip=THNetWork::GetIpFromHostName(ip);
			U32 umask=THNetWork::GetIpFromHostName(mask);
			U32 bcip=uip|(~umask);
			m_lcips.AddTail(uip);
			m_mcips.AddTail(umask);
			m_bcips.AddTail(bcip);
		}
		m_infomtx.Unlock();
	}

	inline BOOL IsLocalIpByAdaptersInfo(THString ip) {return IsLocalIpByAdaptersInfoN(THNetWork::GetIpFromHostNameN(ip));}

	inline BOOL IsLocalIpByAdaptersInfoN(U32 ip) {return IsLocalIpByAdaptersInfo(ntohl(ip));}

	BOOL IsLocalIpByAdaptersInfo(U32 ip)
	{
		THSingleLock l(&m_infomtx);
		return m_lcips.IsExist(ip);
	}

	BOOL BoardcastMessage(RAWSOCKET s,const void *buf,int len,U16 port)
	{
		THSingleLock l(&m_infomtx);
		UpdateAddress();
		UINT ipscnt=m_bcips.GetSize();
		for(UINT i=0;i<ipscnt;i++)
			if (!Send(s,buf,len,(U32)m_bcips[i],port,-1)) return FALSE;
		return TRUE;
	}
private:
	THList<U32> m_lcips;			///<����ip�б�
	THList<U32> m_mcips;			///<�����б�
	THList<U32> m_bcips;			///<�㲥��ַ�б�
	THMutex m_infomtx;
};

/**
* @brief IpV4��ַ��װ��
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2009-09-08 �½���
*/
typedef struct _THIpV4{
	_THIpV4()
	{
		ip=0;
		port=0;
	}
	_THIpV4(U32 sip,U16 sport)
	{
		ip=sip;
		port=sport;
	}
	_THIpV4(struct sockaddr_in *paddr)
	{
		if (paddr)
		{
			ip=paddr->sin_addr.s_addr;
			port=paddr->sin_port;
		}
		else
		{
			ip=0;
			port=0;
		}
	}
	U32 ip;
	U16 port;
	bool operator < (const _THIpV4 rip) const
	{
		if (ip==rip.ip)
			return port<rip.port;
		return ip<rip.ip;
	}
}THIpV4;

/**
* @brief �ײ����Ӷ����װ��
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2009-09-08 �½���
*/
typedef struct _THRawSocketObj{
	_THRawSocketObj()
	{
		c=0;s=INVALID_SOCKET;
	}
	_THRawSocketObj(THRawSocket *cls,RAWSOCKET sock,struct sockaddr_in *paddr)
	{
		c=(UINT)(UINT_PTR)cls;
		s=sock;
		ip=THIpV4(paddr);
	}
	UINT c;
	RAWSOCKET s;
	THIpV4 ip;
	bool operator < (const _THRawSocketObj obj) const
	{
		if (c==obj.c)
		{
			if (s==obj.s) return ip<obj.ip;
			return s<obj.s;
		}
		return c<obj.c;
	}
}THRawSocketObj;

/**
* @brief ��������ӽṹӳ����
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2009-04-11 �½���
*/
/**<pre>
�÷���
	���ڷ������������ͻ����������á�
	TCPͨ��RAWSOCKET�������ӣ�UDPͨ��peerip&port��������
	��ͬ���ӷ��ز�ͬ��TCLS����
	limit���ö��������0Ϊ�����ƣ�����ֵΪ���ƶ��������һ����udpģʽ�£�Ӧָ���������
	Ӧע��ʹ��GetClientInfoʱ�Ƚ��������������ʹ������£�

	THRawSocketClients<ClientInfo> m_cltobj;

	BOOL ITHRawSocketEventHandler::ReceiveHandler(THRawSocket *cls,RAWSOCKET sock,const void *bufdata,int buflen,struct sockaddr_in *paddr,void *adddata)
	{
		THSingleLock(m_cltobj.GetMutex());
		ClientInfo &info=m_cltobj.GetClientInfo(THRawSocketObj(cls,sock,paddr));
		return TRUE;
	}

	void ITHRawSocketEventHandler::ErrorHandler(THRawSocket *cls,RAWSOCKET sock,DWORD errcode,void *adddata)
	{
		m_cltobj.Erase(THRawSocketObj(cls,sock,NULL));
	}

</pre>*/
template<typename TCLS,UINT limit=0,typename TKEY=THRawSocketObj>
class THRawSocketClients :private ITHDeleteCb<TKEY>
{
public:
	THRawSocketClients()
	{
		m_mru.SetDeleteCb(this);
		m_mru.SetLimitSize(limit);
	}
	virtual ~THRawSocketClients()
	{
	}
	THMutex* GetMutex(){return &m_mtx;}
	//Ӧע��ʹ��GetClientInfoʱ�Ƚ���������
	TCLS &GetClientInfo(TKEY key)
	{
		m_mru.Update(key);
		typename map<TKEY,TCLS>::iterator it=m_clients.find(key);
		if (it==m_clients.end())
		{
			typename TCLS value;
			m_clients.insert(typename map<TKEY,TCLS>::value_type(key,value));
			it=m_clients.find(key);
		}
		return (it->second);
	}
	void Erase(TKEY key)
	{
		m_mtx.Lock();
		m_mru.Erase(key);
		m_clients.erase(key);
		m_mtx.Unlock();
	}
protected:
	virtual void DeleteNotify(TKEY key)
	{
		m_mtx.Lock();
		m_clients.erase(key);
		m_mtx.Unlock();
	}
	THMutex m_mtx;
	THMRU<TKEY> m_mru;
	map<TKEY,TCLS> m_clients;
};
