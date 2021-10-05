#pragma once

#include <THRawSocketModel.h>
//include ITHProxyEvent
#include <THSocketProxyServer.h>

/**
* @brief Http�����������
* @author Barry
* @2013-03-17 �½���
*/
/**<pre>
ʹ��Sample��
	m_proxyrs,m_socketrs �����ڽӲ�ͬ��Э��ջ�ģ�Ĭ����THRawSocket������ָ�벻�ܸ���ͬ��ʵ��
	proxyrs������httpproxy��׼Э����õ�socket�㣬socketrs����Ҫ����ʵ������ʱʹ�õ�socket�㣬�������԰�socket�����׽ӵ�����Э��ջ��
</pre>*/
class THHttpProxyServer : public ITHRawSocketBufferedEventHandler
{
public:
	THHttpProxyServer(){
		m_proxyrs=&m_rsp;
		m_socketrs=&m_rss;
		m_proxyrs->SetHandler(this,NULL);
		m_socketrs->SetHandler(this,NULL);
		m_s=INVALID_SOCKET;
		m_event=NULL;
	}
	virtual ~THHttpProxyServer(){
		StopProxy();
	}

	BOOL StartProxy(unsigned short port){
		StopProxy();
		m_s=m_proxyrs->CreateSocket(0,0,port,RawProtocol_Tcp,TRUE,0,FALSE);
		return m_s!=INVALID_SOCKET;
	}
	void StopProxy(){
		if (m_s!=INVALID_SOCKET) {
			m_proxyrs->FreeSocket(m_s);
			m_s=INVALID_SOCKET;
		}
	}
	void SetEventCb(ITHProxyEvent *event){m_event=event;}
	void SetSocketLayer(THRawSocket *proxy,THRawSocket *socket){
		if (proxy==socket && proxy!=NULL) return;
		m_proxyrs=(proxy==NULL)?&m_rsp:proxy;
		m_socketrs=(socket==NULL)?&m_rss:socket;
		m_proxyrs->SetHandler(this,NULL);
		m_socketrs->SetHandler(this,NULL);
	}
protected:
	virtual UINT IsBufferReady(THRawSocket *cls,RAWSOCKET sock,const void *bufdata,int buflen,struct sockaddr_in *paddr,void *adddata)
	{
		const unsigned char *buf=(const unsigned char *)bufdata;
		if (cls==m_proxyrs) {
			//map<RAWSOCKET,RAWSOCKET>::iterator it=m_proxyclients.find(sock);
			//if (it==m_proxyclients.end()){
				const char *hbuf=(const char *)THBinSearch::BinSearch(bufdata,buflen,"Host:",5);
				if (hbuf) {
					hbuf+=5;
					const char *end=(const char *)THBinSearch::BinSearch((const void *)hbuf,
						(unsigned int)(((const char *)bufdata)+buflen-hbuf),"\n",1);
					if (end) {
						CStringA host;
						host.SetString(hbuf,(int)(end-hbuf));
						host=host.Trim(" \r\n");
						unsigned short port=0;
						U32 ip=THRawSocket::GetIpFromHostName(THCharset::a2t(host),&port);
						if (port==0) port=80;
						//m_proxyclients[sock]=INVALID_SOCKET;
						RAWSOCKET consock=m_socketrs->CreateSocket(ip,port);
						if (consock==INVALID_SOCKET || !m_socketrs->Send(consock,bufdata,buflen)){
							//SendRep(cls,sock,THSPREP_TTL_TIMEOUT,m_proxyrs->GetLocalIpN(),0);
							cls->FreeSocket(sock);
							return 0;
						}
						if (m_event) m_event->OnConnect(sock,consock);
						//it->second=consock;
						m_sockclients[consock]=sock;
						return buflen;
					}
				}
				return 0;
			/*} else {
				if (m_event) m_event->OnData(sock,it->second,(const char *)bufdata,buflen,0);
				if (!m_socketrs->Send(it->second,bufdata,buflen)) cls->FreeSocket(sock);
				return buflen;
			}*/
		} else {
			map<RAWSOCKET,RAWSOCKET>::iterator it=m_sockclients.find(sock);
			if (it!=m_sockclients.end()) {
				if (m_event) m_event->OnData(it->second,sock,(const char *)bufdata,buflen,1);
				if (!m_proxyrs->Send(it->second,bufdata,buflen)) cls->FreeSocket(sock);
			} else
				cls->FreeSocket(sock);
			return buflen;
		}
		return 0;
	}
	virtual void AcceptHandler(THRawSocket *cls,RAWSOCKET parent,RAWSOCKET sock,
		struct sockaddr_in *paddr,struct sockaddr_in *paddrpeer,void *adddata){
	}
	virtual void ErrorHandler(THRawSocket *cls,RAWSOCKET sock,DWORD errcode,void *adddata){
		ITHRawSocketBufferedEventHandler::ErrorHandler(cls,sock,errcode,adddata);
		if (cls==m_proxyrs) {
			if (sock!=m_s) {
				if (m_event) m_event->OnClose(sock,INVALID_SOCKET/*m_proxyclients[sock]*/,0);
				//m_proxyclients.erase(sock);
			}
		}else if (cls==m_socketrs) {
			if (m_event) m_event->OnClose(m_sockclients[sock],sock,1);
			m_sockclients.erase(sock);
		}
	}
	BOOL SendRep(THRawSocket *cls,RAWSOCKET sock,unsigned char Rep,unsigned long ipmark,int port){
		char data[10];
		unsigned long ip;
		data[0]=THSP_VERSION5;
		data[1]=Rep;
		data[2]=0x00;
		data[3]=THSPATYP_IPV4;
		port=htons(port);
		memcpy((void *)&data[8],(const void *)&port,2);
		ip=ipmark;
		memcpy((void *)&data[4],(const void *)&ip,4);
		return cls->Send(sock,data,sizeof(data));
	}
	RAWSOCKET m_s;
	THRawSocket *m_proxyrs;		///>ͨ����������socket��
	THRawSocket *m_socketrs;	///>���������socket�㣬�ֿ���������ڰѲ�ͬ�׽ӻ���������
	THRawSocket m_rsp;
	THRawSocket m_rss;
	//map<RAWSOCKET,RAWSOCKET> m_proxyclients;
	map<RAWSOCKET,RAWSOCKET> m_sockclients;
	ITHProxyEvent *m_event;
};
