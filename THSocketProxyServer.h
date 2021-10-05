#pragma once

#include <THRawSocketModel.h>

class ITHProxyEvent{
public:
	//only tcp
	virtual void OnConnect(SOCKET local,SOCKET peer)=0;
	virtual void OnData(SOCKET local,SOCKET peer,const char* buf,unsigned int len,int direction)=0;
	virtual void OnClose(SOCKET local,SOCKET peer,int direction)=0;
};

#define THSP_VERSION5 0x05
#define THSPAUTH_NOAUTHLOGIN 0x00
#define THSPAUTH_GSSAPI 0x01
#define THSPAUTH_AUTHLOGIN 0x02
#define THSPAUTH_NOMETHOD 0xff
#define THSPCMD_CONNECT 0x01
#define THSPCMD_BIND 0x02
#define THSPCMD_UDP 0x03

#define THSPATYP_IPV4 0x01
#define THSPATYP_DOMAIN 0x03
#define THSPATYP_IPV6 0x04

#define THSPREP_SUCCESS 0x00
#define THSPREP_FAILED 0x01
#define THSPREP_CANNOT_CONNECT 0x02
#define THSPREP_NET_CANNOTTO 0x03
#define THSPREP_PC_CANNOTTO 0x04
#define THSPREP_CONNECT_DENIED 0x05
#define THSPREP_TTL_TIMEOUT 0x06
#define THSPREP_CMD_WRONG 0x07
#define THSPREP_ATYP_WRONG 0x08

/**
* @brief Socket代理服务器类
* @author Barry
* @2013-03-07 新建类
*/
/**<pre>
使用Sample：
	m_proxyrs,m_socketrs 是用于接不同的协议栈的，默认是THRawSocket，两个指针不能给相同的实例
	proxyrs是用于socket5proxy标准协议层用的socket层，socketrs是需要访问实际数据时使用的socket层，这样可以把socket代理套接到任意协议栈内
</pre>*/
class THSocketProxyServer : public ITHRawSocketBufferedEventHandler
{
public:
	THSocketProxyServer(){
		m_proxyrs=&m_rsp;
		m_socketrs=&m_rss;
		m_proxyrs->SetHandler(this,NULL);
		m_socketrs->SetHandler(this,NULL);
		m_s=INVALID_SOCKET;
		m_event=NULL;
	}
	virtual ~THSocketProxyServer(){
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
			map<RAWSOCKET,RAWSOCKET>::iterator it=m_proxyclients.find(sock);
			if (it==m_proxyclients.end()){
				//init
				if (buflen>2) {
					if (buf[0]!=THSP_VERSION5) {
						cls->FreeSocket(sock);
						return 0;
					} else {
						int methodlen=buf[1];
						for(int i=2;i<min(methodlen+2,buflen);i++){
							if (buf[i]==THSPAUTH_NOAUTHLOGIN) {
								unsigned char sdata[2];
								sdata[0]=buf[0];
								sdata[1]=buf[i];
								if (!cls->Send(sock,sdata,2)) {
									cls->FreeSocket(sock);
									buflen=0;//force return 0
								}
								break;
							}
						}
						m_proxyclients[sock]=INVALID_SOCKET;
						return min(methodlen+2,buflen);
					}
				}
				return 0;
			} else if (it->second==INVALID_SOCKET) {
				//cmd
				unsigned short port;char ip[250];int index;
				if (buflen>=10) {
					switch(buf[3]){
						case THSPATYP_IPV4:
							memcpy(&port,&buf[8],2);
							sprintf(ip,"%u.%u.%u.%u",buf[4],buf[5],buf[6],buf[7]);
							break;
						case THSPATYP_DOMAIN:
							index=buf[4];
							memcpy(&port,&buf[index+5],2);
							memcpy((void *)ip,(const void *)&buf[5],index);
							ip[index]='\0';
							break;
						case THSPATYP_IPV6:
						default:
							cls->FreeSocket(sock);
							return 0;
							break;
					}
					if (buf[1]==THSPCMD_CONNECT) {
						port=ntohs(port);
						RAWSOCKET consock=m_socketrs->CreateSocketByHost(THCharset::a2t(ip),port);
						if (consock==INVALID_SOCKET){
							SendRep(cls,sock,THSPREP_TTL_TIMEOUT,m_proxyrs->GetLocalIpN(),0);
							cls->FreeSocket(sock);
							return 0;
						}
						//finish connect
						if (!SendRep(cls,sock,THSPREP_SUCCESS,m_proxyrs->GetLocalIpN(),port)){
							cls->FreeSocket(sock);
							return 0;
						}
						if (m_event) m_event->OnConnect(sock,consock);
						it->second=consock;
						m_sockclients[consock]=sock;
						return buflen;
					} else if (buf[1]==THSPCMD_BIND){
						//fixme
						cls->FreeSocket(sock);
						return 0;
					} else if (buf[1]==THSPCMD_UDP) {
						//fixme
						cls->FreeSocket(sock);
						return 0;
					}
				}
				return 0;
			} else {
				if (m_event) m_event->OnData(sock,it->second,(const char *)bufdata,buflen,0);
				if (!m_socketrs->Send(it->second,bufdata,buflen)) cls->FreeSocket(sock);
				return buflen;
			}
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
				if (m_event) m_event->OnClose(sock,m_proxyclients[sock],0);
				m_proxyclients.erase(sock);
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
	THRawSocket *m_proxyrs;		///>通过代理服务的socket层
	THRawSocket *m_socketrs;	///>向外请求的socket层，分开两个层便于把不同套接环境接起来
	THRawSocket m_rsp;
	THRawSocket m_rss;
	map<RAWSOCKET,RAWSOCKET> m_proxyclients;
	map<RAWSOCKET,RAWSOCKET> m_sockclients;
	ITHProxyEvent *m_event;
};
