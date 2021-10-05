
#pragma once

#include <THStruct.h>
#include <THSyn.h>
#include <THString.h>
#include <THCmdProcess.h>

/**
* @brief Telnet服务器类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2009-04-09 新建类
*/
/**<pre>
继承
	GetSpceialModeText
	OnParseOtherMode
	OnParseCmd
进行命令扩展
</pre>*/
class THTelnetServer : private ITHRawSocketEventHandler, private ITHCmdProcessEvent
{
public:
	THTelnetServer()
	{
		m_s=INVALID_SOCKET;
		m_sock.SetHandler(this,NULL);
		m_client.SetFreeProc(FreeClientInfo,this);
	}
	virtual ~THTelnetServer()
	{
		m_sock.SetHandler(NULL,NULL);
		StopListen();
	}
	BOOL IsStarted() {return m_s!=INVALID_SOCKET;}
	U16 GetListenPort()
	{
		if (m_s==INVALID_SOCKET) return 0;
		return m_sock.GetLocalPort(m_s);
	}
	BOOL StartListen(UINT nListenPort=23,UINT nSid=0,THString sDefPass=_T(""))
	{
		StopListen();
		m_defaultpass=sDefPass;
		m_s=m_sock.CreateSocket(0,0,nListenPort,RawProtocol_Tcp,TRUE,nSid);
		return m_s!=INVALID_SOCKET;
	}
	void StopListen()
	{
		if (m_s!=INVALID_SOCKET)
		{
			m_sock.FreeSocket(m_s);
			m_s=INVALID_SOCKET;
		}
		m_mtx.Lock();
		m_client.RemoveAll();
		m_mtx.Unlock();
	}
private:
	typedef struct _ClientInfo{
		BOOL bInit;
		int nThreadId;
		HANDLE hIn;
		THStringA curcmd;
		int nMode;
	}ClientInfo;
	virtual BOOL OnProcessPreCreate(THCmdProcess *pProcess,THProcessInfo *info,int nThreadId)
	{
		RAWSOCKET s=(RAWSOCKET)info->pAddData;
		ClientInfo *client;
		m_mtx.Lock();
		if (m_client.GetAt(s,client))
		{
			client->hIn=info->hIn;
			m_mtx.Unlock();
			return FALSE;
		}
		m_mtx.Unlock();
		return TRUE;
	}
	virtual BOOL OnProcessingOutput(THCmdProcess *pProcess,THProcessInfo *info,int nThreadId)
	{
		RAWSOCKET s=(RAWSOCKET)info->pAddData;
		ClientInfo *client;
		m_mtx.Lock();
		if (m_client.GetAt(s,client))
		{
			if (client->nMode==0)
			{
				//if in dos mode,send the dos buffer
				UINT len;
				void *buf=info->bufout.GetBuf(&len);
				if (len>0) m_sock.Send(s,buf,len);
				info->bufout.ReleaseGetBufLock(len);
				buf=info->buferr.GetBuf(&len);
				if (len>0) m_sock.Send(s,buf,len);
				info->buferr.ReleaseGetBufLock(len);
			}
			m_mtx.Unlock();
			return FALSE;
		}
		m_mtx.Unlock();
		return TRUE;
	}

	virtual void OnProcessFinish(THCmdProcess *pProcess,THProcessInfo *info,int nThreadId,BOOL bRunRet,DWORD nRetCode)
	{
		RAWSOCKET s=(RAWSOCKET)info->pAddData;
		ClientInfo *client;
		m_mtx.Lock();
		if (m_client.GetAt(s,client))
			client->nThreadId=0;
		m_mtx.Unlock();
	}

	static void FreeClientInfo(void *key,void *value,void *adddata)
	{
		ClientInfo *client=(ClientInfo *)value;
		THTelnetServer *server=(THTelnetServer *)adddata;
		if (client->nThreadId!=0)
			server->m_cmd.CancelCmd(client->nThreadId);
		delete client;
	}

	virtual void AcceptHandler(THRawSocket *cls,RAWSOCKET parent,RAWSOCKET sock,struct sockaddr_in *paddr,struct sockaddr_in *paddrpeer,void *adddata)
	{
		THStringA welcome="Welcome to use Tansoft Telnet Server 1.0\r\nPlease Input the Password:";
		cls->Send(sock,(LPCSTR)welcome,welcome.GetLength());
	}

/*	virtual UINT IsBufferReady(THRawSocket *cls,RAWSOCKET sock,const void *bufdata,int buflen,struct sockaddr_in *paddr,void *adddata)
	{
		return buflen;
		const char crbf[]="\r\n";
		const void *tmpcrbf=THBinSearch::BinSearch(bufdata,buflen,crbf,2);
		if (!tmpcrbf) return 0;
		return (UINT)(UINT_PTR)((const char *)tmpcrbf-(const char *)bufdata+2);
	}
*/
	virtual BOOL ReceiveHandler(THRawSocket *cls,RAWSOCKET sock,const void *bufdata,int buflen,struct sockaddr_in *paddr,void *adddata)
	//virtual void BufferedReceiveHandler(THRawSocket *cls,RAWSOCKET sock,const void *bufdata,int buflen,struct sockaddr_in *paddr,void *adddata)
	{
		ClientInfo *client;
		m_mtx.Lock();
		if (!m_client.GetAt(sock,client))
		{
			client=new ClientInfo;
			client->bInit=FALSE;
			client->nThreadId=0;
			client->hIn=NULL;
			client->nMode=0;
			m_client.SetAt(sock,client);
		}
		if (!client->bInit)
		{
			THStringA str;
			str.SetString((const char *)bufdata,buflen);
			client->curcmd+=str;
			if (client->curcmd.Find("\r\n",0)!=-1)
			{
				if (client->curcmd==m_defaultpass+_T("\r\n"))
				{
					THStringA msg="Login Success!\r\nPress ESC to into spceial mode\r\n";
					cls->Send(sock,(LPCSTR)msg,msg.GetLength());
					client->curcmd.Empty();
					client->bInit=TRUE;
				}
				else
				{
					client->curcmd.Empty();
					THStringA retry="Unauthorized\r\nPlease Input the Password:";
					cls->Send(sock,(LPCSTR)retry,retry.GetLength());
				}
				buflen=0;
			}
		}
		if (client->bInit)
		{
			if (client->nThreadId==0)
			{
				client->nThreadId=m_cmd.RunCmd(_T("cmd.exe"),TRUE,NULL,FALSE,50,0,this,(void *)sock);
			}
			if (client->nMode==0)
			{
				//dos mode
				if (buflen>0 && client->hIn)
				{
					if (buflen==1 && *(const char *)bufdata==0x1b)
					{
						THStringA msg=GetSpceialModeText();
						cls->Send(sock,(LPCSTR)msg,msg.GetLength());
						client->nMode=1;
						client->curcmd.Empty();
					}
					else
						m_cmd.WritePipe(client->hIn,bufdata,buflen);
				}
			}
			else if (client->nMode==1)
			{
				//special mode
				THStringA str;
				str.SetString((const char *)bufdata,buflen);
				client->curcmd+=str;
				if (client->curcmd.Find("\r\n",0)!=-1)
				{
					THString cmd=client->curcmd.Left(client->curcmd.GetLength()-2);
					client->curcmd.Empty();
					THStringToken t(cmd,_T(" "));
					cmd=t.GetNextToken();
					OnParseCmd(cmd,t,client);
					if (cmd==_T("exit"))
					{
						THStringA msg="return to telnet mode now.\r\n";
						cls->Send(sock,(LPCSTR)msg,msg.GetLength());
						client->nMode=0;
					}
				}
			}
			else
			{
				OnParseOtherMode(client);
			}
		}
		m_mtx.Unlock();
		return TRUE;
	}

	virtual THStringA GetSpceialModeText()
	{
		return " \r\nspceial mode:\r\n  exit for return to telnet mode\r\n";
	}

	virtual void OnParseOtherMode(ClientInfo *client)
	{
		//e.g.data transfer mode
		//if (client->nMode==2)
	}

	virtual void OnParseCmd(THString &cmd,THStringToken &t,ClientInfo *client)
	{
		//e.g.other cmd translateor
		/*if (cmd==_T("rz"))
		{
		}
		else if (cmd==_T("sz"))
		{
		}*/
	}
	virtual void ErrorHandler(THRawSocket *cls,RAWSOCKET sock,DWORD errcode,void *adddata)
	{
		m_mtx.Lock();
		m_client.RemoveAt(sock);
		m_mtx.Unlock();
		//ITHRawSocketBufferedEventHandler::ErrorHandler(cls,sock,errcode,adddata);
	}
	THMap<RAWSOCKET,ClientInfo *> m_client;
	THStringA m_defaultpass;
	THRawSocket m_sock;
	RAWSOCKET m_s;
	THCmdProcess m_cmd;
	THMutex m_mtx;
};
