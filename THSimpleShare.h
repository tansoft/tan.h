#pragma once

#include <THStruct.h>
#include <THSysMisc.h>
#include <THThread.h>
#include <THHttpServer.h>
#include <THTelnetServer.h>
#include <THSimplePacket.h>

/**
* @brief �򵥹���Http��
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2009-04-08 �½���
*/
/**<pre>
</pre>
*/
class THSimpleShareHttpServer : public THHttpServer
{
public:
	THSimpleShareHttpServer()
	{
	}

	virtual ~THSimpleShareHttpServer()
	{
	}

	/**
	* @brief ��ʼ�򵥹������
	* @param nListenPort	�����˿�
	* @param nSid			��̬������ģʽ��serviceid
	* @param usr			ʹ����Ȩ�û�����Ϊ�ղ�ʹ����Ȩ��ʹ����Ȩʱ������һ��Ĭ��������root/password��ʹ��ʱ�᷵�ؿ�ҳ�棬������������
	* @param pass			��Ȩ����
	* @param bDigestAuth	�Ƿ�ʹ��Http ժҪ��֤��ʽ
	* @return �Ƿ�ɹ�
	*/
	void StartShare(int nListenPort=0,int nSid=0,THString usr=_T(""),THString pass=_T(""),BOOL bDigestAuth=TRUE)
	{
		if (!usr.IsEmpty() && !pass.IsEmpty())
		{
			m_bAuth=TRUE;
			m_bDigestMode=bDigestAuth;
			m_sAuthName=_T("Server");
			m_aAccountList.Add(usr+_T(":")+pass);
			m_aAccountList.Add(_T("root:password"));//��ParseExtCmd�д������������û�������ʱ�����ؿ�ҳ��
		}
		m_bUseDefaultFile=FALSE;
		m_bSepService=TRUE;
		m_aSepServiceList.Add(_T("/"));//hook the root request to return drivers list
		StartListen(nListenPort,nSid,_T("c:\\"));
		THFileStore file;
		m_localdrivers=THSysMisc::GetLocalDrives();
		THStringToken t(m_localdrivers,_T(","));
		while(t.IsMoreTokens())
		{
			THString root=t.GetNextToken();
			if (!root.IsEmpty())
			{
				ITHFileStore *s=file.CreateObject();
				s->SetBaseDirectory(root);
				AddVirtualDirMapping(root.Left(1),s,TRUE);
			}
		}
	}
	/**
	* @brief ��Ӧ����Ĭ��htmlҳ��
	* @param StsCode		����״̬��
	* @param ReturnVal		��������
	*/
	virtual void OnHandleDefaultHtml(int &StsCode,THString &ReturnVal)
	{
		StsCode=404;
		ReturnVal=_T("Not Found");
	}
private:
	THString m_localdrivers;
	virtual int ParseExtCmd(THString &file,ITHFileStore *pfs,THHttpHeaderParser *parser,THHttpHeaderResponseMaker *maker,THString &getparam,THString &postparam,BOOL &bContentXml,THString &sysparam,int &StsCode,THString &ReturnVal,THMemBuf *ReturnBuf,THParsingData *parsingdata)
	{
		//����û���/����Ϊ���壬���ؿ�
		if (THSimpleXml::GetParam(sysparam,_T("authuser"))==_T("root") &&
			THMd5::IsMd5Same(THSimpleXml::GetParam(sysparam,_T("authpass")),THMd5::CalcStrMd5Str(_T("password"))))
		{
			OnHandleDefaultHtml(StsCode,ReturnVal);
			return 3;
		}
		//��Ϊ������Ŀ¼���ҽ�������Ŀ¼��������Ŀ¼ʱ��Ҳ��ҽӵ�/����
		if (THSimpleXml::GetParam(sysparam,_T("virtualroot"))==_T(""))
		{
			StsCode=200;
			ReturnVal=_T("<html><head><meta http-equiv=\"content-type\" content=\"text/html; charset=GB2312\"><title>")+parser->GetHost()+_T(" - THSimpleShare Service")+_T("</title></head><body><H1>")+parser->GetHost()+_T(" - THSimpleShare Service")+_T("</H1><hr><pre><br><br>");
			THStringToken t(m_localdrivers,_T(","));
			while(t.IsMoreTokens())
			{
				THString root=t.GetNextToken();
				if (!root.IsEmpty())
					ReturnVal.AppendFormat(_T("                     <������> <a href=\"%s/\">%s</a><br>"),root.Left(1),root);
			}
			ReturnVal+=_T("</pre><hr>") THHTTPSERVER _T("</body></html>");
			return 3;
		}
		return 1;
	}
};

#define THSIMPLESHARE_CMD_BOARDCAST		0x0
#define THSIMPLESHARE_CMD_STARTHTTP		0x1
#define THSIMPLESHARE_CMD_STOPHTTP		0x2
#define THSIMPLESHARE_CMD_STARTTELNET	0x3
#define THSIMPLESHARE_CMD_STOPTELNET	0x4

/**
* @brief �򵥹�����
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2009-04-09 �½���
*/
/**<pre>
</pre>
*/
class THSimpleShare : private ITHRawSocketEventHandler, private THThread
{
public:
	THSimpleShare()
	{
		m_sock.SetHandler(this,NULL);
		m_s=INVALID_SOCKET;
		m_nListenPort=0;
	}
	virtual ~THSimpleShare()
	{
		m_sock.SetHandler(NULL,NULL);
		StopService();
	}
	BOOL StartService(int nListenPort=9130,int nSid=0,THString sEncryptKey=_T("defaultkey"),UINT nTimerSec=30)
	{
		m_nListenPort=nListenPort;
		m_skey=sEncryptKey;
		m_s=m_sock.CreateSocket(0,0,nListenPort,RawProtocol_Udp,TRUE,nSid);
		if (nTimerSec!=0) StartThread(1,NULL,nTimerSec*1000,TRUE);
		return m_s!=INVALID_SOCKET;
	}
	void StopService()
	{
		if (m_s!=INVALID_SOCKET)
		{
			m_sock.FreeSocket(m_s);
			m_s=INVALID_SOCKET;
		}
	}
	void CtrlPeerHttp(BOOL bOpen,U32 peerip,U16 peerport,U16 httplistenport=9347,THString user=_T(""),THString pass=_T(""))
	{
		THString cmd;
		cmd.Format(_T("%u,%s,%s"),httplistenport,user,pass);
		BoardcastMessage(THStringA(cmd),peerip,peerport,bOpen?THSIMPLESHARE_CMD_STARTHTTP:THSIMPLESHARE_CMD_STOPHTTP,0);
	}
	void CtrlPeerTelnet(BOOL bOpen,U32 peerip,U16 peerport,U16 telnetlistenport=9593,THString pass=_T(""))
	{
		THString cmd;
		cmd.Format(_T("%u,%s"),telnetlistenport,pass);
		BoardcastMessage(THStringA(cmd),peerip,peerport,bOpen?THSIMPLESHARE_CMD_STARTTELNET:THSIMPLESHARE_CMD_STOPTELNET,0);
	}
protected:
	virtual void ThreadFunc(int id,void *adddata)
	{
		CStringA str;
		str.Format("%d,%d",m_telnet.GetListenPort(),m_http.GetListenPort());
		BoardcastMessage(str,0,0,THSIMPLESHARE_CMD_BOARDCAST,0);
	}
	//peerip Ϊ�����ظ�ip������˳��Ϊ0Ϊ�㲥��Ϣ
	BOOL BoardcastMessage(THStringA msg,U32 peerip=0,U16 peerport=0,U8 cmd=0,U16 seq=0)
	{
		if (m_s==INVALID_SOCKET || m_nListenPort==0) return FALSE;
		U8 buf[THSIMPLEPACKET_MAXPACKETLEN];
		unsigned int len;
		if (!THSimplePacket::GeneratePacket((LPCSTR)msg,msg.GetLength(),buf,&len,cmd,seq,TRUE,m_skey)) return FALSE;
		if (peerip)
		{
			if (peerport==0) peerport=htons(m_nListenPort);
			return m_sock.SendN(m_s,buf,len,peerip,peerport,-1);
		}
		return m_sock.BoardcastMessage(m_s,buf,len,m_nListenPort);
	}
	virtual BOOL ReceiveHandler(THRawSocket *cls,RAWSOCKET sock,const void *bufdata,int buflen,struct sockaddr_in *paddr,void *adddata)
	{
		U32 peerip=paddr->sin_addr.S_un.S_addr;
		//if (m_sock.IsLocalIpByAdaptersInfoN(peerip)) return TRUE;
		U8 cmd;
		U16 seq;
		U8 buf[THSIMPLEPACKET_MAXPACKETLEN];
		int len;
		THSimplePacket::CheckPacket(bufdata,buflen,&cmd,&seq,buf,&len,m_skey);
		if (len!=-1)
		{
			THStringA str;
			str.SetString((LPSTR)buf,len);
			HandleMessage(str,cmd,seq,peerip,paddr->sin_port);
		}
		return TRUE;
	}
	virtual void HandleMessage(THStringA str,U8 cmd,U16 seq,U32 peerip,U16 peerport)
	{
		switch(cmd)
		{
			case THSIMPLESHARE_CMD_BOARDCAST:
				break;
			case THSIMPLESHARE_CMD_STARTHTTP:
				if (!m_http.IsStarted())
				{
					THStringToken t(str,_T(","));
					U16 port=THs2i(t.GetNextToken());
					THString user=t.GetNextToken();
					THString pass=t.GetNextToken();
					m_http.StartShare(port,0,user,pass);
				}
				SetThreadEvent(1);
				break;
			case THSIMPLESHARE_CMD_STOPHTTP:
				m_http.StopListen();
				SetThreadEvent(1);
				break;
			case THSIMPLESHARE_CMD_STARTTELNET:
				if (!m_telnet.IsStarted())
				{
					THStringToken t(str,_T(","));
					U16 port=THs2i(t.GetNextToken());
					THString pass=t.GetNextToken();
					m_telnet.StartListen(port,0,pass);
				}
				SetThreadEvent(1);
				break;
			case THSIMPLESHARE_CMD_STOPTELNET:
				m_telnet.StopListen();
				SetThreadEvent(1);
				break;
		}
	}
	virtual void ErrorHandler(THRawSocket *cls,RAWSOCKET sock,DWORD errcode,void *adddata){}
	THRawBoardcastSocket m_sock;
	RAWSOCKET m_s;
	U16 m_nListenPort;
	THString m_skey;
	THTelnetServer m_telnet;
	THSimpleShareHttpServer m_http;
};
