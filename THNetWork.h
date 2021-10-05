#pragma once

#define THSYSTEM_INCLUDE_SOCKET2
#define THSYSTEM_INCLUDE_IPTYPES
#define THSYSTEM_INCLUDE_IPHELPAPI
#define THSYSTEM_INCLUDE_TCPIP
#include <THSystem.h>
#include <THThread.h>
#include <THString.h>
#include <THCharset.h>
#include <THStruct.h>
#include <THSyn.h>
#include <THDebug.h>

#define SocketVersion11		MAKEWORD(1,1)
#define SocketVersion22		MAKEWORD(2,2)

typedef SOCKET RAWSOCKET;
#pragma pack(push,1)
/* The IP header */
typedef struct _THIpHeader{
#if __BYTE_ORDER == __LITTLE_ENDIAN 
		unsigned char header_len:4;               //little-endian IP头长度(单位为32位)4位 
		unsigned char version:4;                //版本号4 IP4用4 
#else
		unsigned char version:4;                // version 
		unsigned char header_len:4;               // header length 
#endif
        unsigned char  tos;            // type of service
        unsigned short total_len;      // length of the packet
        unsigned short ident;          // unique identifier
        unsigned short flags;          
        unsigned char  ttl;            
        unsigned char  proto;          // protocol ( IP , TCP, UDP etc)
        unsigned short checksum;       
        unsigned int   source_ip;
        unsigned int   dest_ip;
}THIpHeader;
/* The Tcp header */
typedef struct _THTcpHeader{
        unsigned short source_port;
        unsigned short dest_port;
        unsigned int   seq_num;
		unsigned int   ack_num;
		/*union{
			unsigned short whole_headerflags;
			struct {
				unsigned char  header_len:4;
				unsigned char  hflags:4;
				unsigned char  lflags;
			}header_wholeflags;*/
			struct {
#if __BYTE_ORDER == __LITTLE_ENDIAN 
				unsigned char  nonce:1;
				unsigned char  reserved:3;
				unsigned char  header_len:4;
				unsigned char  fin:1;
				unsigned char  syn:1;
				unsigned char  reset:1;
				unsigned char  push:1;
				unsigned char  ack:1;
				unsigned char  urgent:1;
				unsigned char  ecn_echo:1;
				unsigned char  cwr:1;
#else
				unsigned char  header_len:4;
				unsigned char  reserved:3;
				unsigned char  nonce:1;
				unsigned char  cwr:1;
				unsigned char  ecn_echo:1;
				unsigned char  urgent:1;
				unsigned char  ack:1;
				unsigned char  push:1;
				unsigned char  reset:1;
				unsigned char  syn:1;
				unsigned char  fin:1;
#endif
			}header_flags;
		//}flags;
		unsigned short window_size;
		unsigned short checksum;
        unsigned short reserved;
}THTcpHeader;


typedef struct _THUdpHeader
{
        unsigned short source_port;
        unsigned short dest_port;
        unsigned short len;
		unsigned short checksum;
}THUdpHeader;
#pragma pack(pop)

/**
* @brief 网络处理基类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-06-30 新建类
* @2008-01-30 GetLocalIp 函数改bug
*/
/**<pre>
用法：
</pre>*/
class THNetWork
{
public:
	THNetWork(WORD wVersionRequested=SocketVersion22)
	{
		InitSocket(wVersionRequested);
		localip=NULL;
		localipsize=0;
	}
	virtual ~THNetWork()
	{
		UnInitSocket();
		if (localip)
		{
			delete [] localip;
			localip=NULL;
		}
	}

	BOOL InitSocket(WORD wVersionRequested)
	{
		WSADATA wsaData;
		int nResult=WSAStartup(wVersionRequested,&wsaData);
		if (nResult!=0) return FALSE;
		if (wsaData.wVersion!=wVersionRequested)
		{
			WSACleanup();
			return FALSE;
		}
		return TRUE;
	}

	BOOL UnInitSocket()
	{
		return (WSACleanup()==0);
	}

	/**
	* @brief 解释域名或ip为域名和端口号分开
	* @param sHostName			域名或ip字符串
	* @param pnPort				返回端口号，空为不返回，如没有指定端口号，返回0
	* @return 返回域名或ip字符串
	*/
	static THString GetHostFromHostName(THString sHostName,U16 *pnPort=NULL)
	{
		U16 port=0;
		int pos=sHostName.ReverseFind(':');
		if (pos!=-1)
		{
			port=THs2u(sHostName.Mid(pos+1));
			sHostName=sHostName.Left(pos);
		}
		if (pnPort) *pnPort=port;
		return sHostName;
	}

	/**
	* @brief 解释域名或ip为整数ip，处理端口号
	* @param sHostName			域名或ip字符串
	* @param pnPort				返回端口号，空为不返回，如没有指定端口号，返回0
	* @return 返回32位整数ip，本机顺序
	*/
	static U32 GetIpFromHostName(THString sHostName,U16 *pnPort=NULL)
	{
		U32 ret=ntohl(GetIpFromHostNameN(sHostName,pnPort));
		if (pnPort) *pnPort=ntohs(*pnPort);
		return ret;
	}

	/**
	* @brief 解释域名或ip为整数ip，处理端口号
	* @param sHostName			域名或ip字符串
	* @param pnPort				返回端口号，空为不返回，如没有指定端口号，返回0
	* @return 返回32位整数ip，网络顺序
	*/
	static U32 GetIpFromHostNameN(THString sHostName,U16 *pnPort=NULL)
	{
		sHostName=GetHostFromHostName(sHostName,pnPort);
		if (pnPort) *pnPort=htons(*pnPort);
		U32 ret=0;
		hostent* remoteHost=NULL;
		char *host=THCharset::t2a(sHostName);
		if (host)
		{
			if (_istalpha(sHostName.GetAt(0)))
			{
				remoteHost=gethostbyname(host);
				if (remoteHost)
					ret=(U32)((LPIN_ADDR)remoteHost->h_addr)->s_addr;
			}
			else
				ret=inet_addr(host);
			THCharset::free(host);
		}
		return ret;
	}

	THString GetHostName()
	{
		char name[256];
		name[0]='\0';
		gethostname(name,255);
		return THCharset::a2t(name);
	}

	/**
	* @brief 获取本地ip
	* @param idx			ip的索引，0开始，-1表示获取本地共有几个ip，127.0.0.1的ip忽略
	* @return 返回32位整数ip，本机顺序
	*/
	inline U32 GetLocalIp(int idx=0)
	{
		return ntohl(GetLocalIpN(idx));
	}

	/**
	* @brief 获取本地ip
	* @param idx			ip的索引，0开始，-1表示获取本地共有几个ip，127.0.0.1的ip忽略
	* @return 返回32位整数ip，本机顺序
	*/
	U32 GetLocalIpN(int idx=0)
	{
		struct hostent *he;
		char name[256];
		U32 ip=0;
		int i=0,j=0;
		int count=0;
		gethostname(name,255);
		name[255] = 0;
		he = gethostbyname(name);
		if (he == NULL) return 0;
		while (1)
		{
			if (he->h_addr_list[i] == NULL) break;
			if (idx==-1 || idx<=j)
			{
				if (he->h_addr_list[i][0]!=127 && he->h_addr_list[i][0]!=0)
				{
					if (idx==-1)
						count++;
					else
					{
						ip=*((U32 *)he->h_addr_list[i]);
						break;
					}
				}
				else
					j++;
			}
			else
				j++;
			i++;
		}
		if (idx==-1)
			return count;
		return ip;
	}

	/**
	* @brief 获取ip字符串
	* @param hostip			ip地址，本机顺序
	* @return 返回字符串
	*/
	static THString GetAddrString(U32 hostip)
	{
		hostip=htonl(hostip);
		return GetAddrStringN(hostip);
	}

	/**
	* @brief 获取ip字符串
	* @param hostip			ip地址，网络顺序
	* @return 返回字符串
	*/
	static inline THString GetAddrStringN(U32 hostip)
	{
		return THCharset::a2t(inet_ntoa((struct in_addr &)hostip));
	}

	/**
	* @brief 获取对方端口信息
	* @param s		socket
	* @param pport	对方端口，本机顺序，可为空
	* @return 返回对方ip，本机顺序
	*/
	static U32 inline GetPeerInfo(RAWSOCKET s,U16 *pport)
	{
		U32 ret=GetPeerInfoN(s,pport);
		if (pport) *pport=ntohs(*pport);
		return ntohl(ret);
	}

	/**
	* @brief 获取对方端口信息
	* @param s		socket
	* @param pport	对方端口，网络顺序，可为空
	* @return 返回对方ip，网络顺序
	*/
	static U32 GetPeerInfoN(RAWSOCKET s,U16 *pport)
	{
		struct sockaddr_in sinpeer;
		int socklen = sizeof(struct sockaddr_in);
		if (getpeername(s,(struct sockaddr *)&sinpeer,&socklen)!=0)
		{
			if (pport) *pport=0;
			return 0;
		}
		if (pport) *pport=sinpeer.sin_port;
		return sinpeer.sin_addr.s_addr;
	}

	/**
	* @brief 判断ip是否为本地ip
	* @param hostip			ip地址，本机顺序
	* @param reload			是否重新获取本地ip列表
	* @return 返回字符串
	*/
	inline BOOL IsLocalIp(U32 hostip,BOOL reload=FALSE)
	{
		return IsLocalIpN(htonl(hostip),reload);
	}

	/**
	* @brief 判断ip是否为本地ip
	* @param hostip			ip地址，网络顺序
	* @param reload			是否重新获取本地ip列表
	* @return 返回字符串
	*/
	BOOL IsLocalIpN(U32 hostip,BOOL reload=FALSE)
	{
		if (reload)
		{
			if (localip)
			{
				delete [] localip;
				localip=NULL;
			}
		}
		if (!localip)
		{
			//buffered the local addr
			U32 count=GetLocalIpN(-1);
			//can't check the local addr
			if (count==0) return FALSE;
			localipsize=count*2*sizeof(U32);
			localip=new char[localipsize];//double size for empty space
			if (!localip) return FALSE;
			memset(localip,0,localipsize);
			U32 ip;
			for(U32 i=0;i<count;i++)
			{
				ip=GetLocalIpN(i);
				memcpy(localip+i*2*sizeof(U32),&ip,sizeof(U32));
			}
		}
		return THBinSearch::BinSearch(localip,localipsize,&hostip,sizeof(hostip))!=NULL;
	}

	/**
	* @brief 判断ip是否为内网ip
	* @param ip				ip地址，网络顺序
	* @return 返回是否
	*/
	static BOOL IsNatIpN(U32 ip) {return IsNatIp(ntohl(ip));}

	/**
	* @brief 判断ip是否为内网ip
	* @param ip				ip地址，本地顺序
	* @return 返回是否
	*/
	static BOOL IsNatIp(U32 ip)
	{
		//10.0.0.0~10.255.255.255
		if ((ip&0xFF000000)==0x0A000000) return TRUE;
		//172.16.0.0~172.31.255.255
		if ((ip&0xFFF00000)==0xAC100000) return TRUE;
		//192.168.0.0~192.168.255.255
		if ((ip&0xFFFF0000)==0xC0A80000) return TRUE;
		//169.254.0.0~169.254.255.255
		if ((ip&0xFFFF0000)==0xA9FE0000) return TRUE;
		return FALSE;
	}

	/**
	* @brief 获取Socket信息
	* @param s				socket
	* @param localip		socket本地ip
	* @param localport		socket本地端口，本机顺序，为空为不获取
	* @param peerip			socket远程ip
	* @param peerport		socket远程端口，本机顺序，为空为不获取
	* @return 是否成功
	*/
	BOOL GetSocketInfo(RAWSOCKET s,THString *localip,U16 *localport,THString *peerip=NULL,U16 *peerport=NULL)
	{
		U32 ip,ip1;
		U32 *pip=NULL,*pip1=NULL;
		if (localip) pip=&ip;
		if (peerip) pip1=&ip1;
		if (!GetSocketInfo(s,pip,localport,pip1,peerport)) return FALSE;
		if (localip) *localip=GetAddrString(*pip);
		if (peerip) *peerip=GetAddrString(*pip1);
		return TRUE;
	}
	/**
	* @brief 获取Socket信息
	* @param s				socket
	* @param localip		socket本地ip，本机顺序，为空为不获取
	* @param localport		socket本地端口，本机顺序，为空为不获取
	* @param peerip			socket远程ip，本机顺序，为空为不获取
	* @param peerport		socket远程端口，本机顺序，为空为不获取
	* @return 是否成功
	*/
	BOOL GetSocketInfo(RAWSOCKET s,U32 *localip,U16 *localport,U32 *peerip=NULL,U16 *peerport=NULL)
	{
		if (s==INVALID_SOCKET) return FALSE;
		if (localip || localport)
		{
			struct sockaddr_in sin;
			int socklen = sizeof(struct sockaddr_in);
			if (getsockname(s,(struct sockaddr *)&sin,&socklen)!=0) return FALSE;
			if (localip) *localip=ntohl(sin.sin_addr.s_addr);
			if (localport) *localport=ntohs(sin.sin_port);
		}
		if (peerip || peerport)
		{
			struct sockaddr_in sinpeer;
			int socklen = sizeof(struct sockaddr_in);
			if (getpeername(s,(struct sockaddr *)&sinpeer,&socklen)!=0) return FALSE;
			if (peerip) *peerip=ntohl(sinpeer.sin_addr.s_addr);
			if (peerport) *peerport=ntohs(sinpeer.sin_port);
		}
		return TRUE;
	}
	BOOL GetSocketInfoN(RAWSOCKET s,U32 *localip,U16 *localport,U32 *peerip=NULL,U16 *peerport=NULL)
	{
		if (s==INVALID_SOCKET) return FALSE;
		if (localip || localport)
		{
			struct sockaddr_in sin;
			int socklen = sizeof(struct sockaddr_in);
			if (getsockname(s,(struct sockaddr *)&sin,&socklen)!=0) return FALSE;
			if (localip) *localip=sin.sin_addr.s_addr;
			if (localport) *localport=sin.sin_port;
		}
		if (peerip || peerport)
		{
			struct sockaddr_in sinpeer;
			int socklen = sizeof(struct sockaddr_in);
			if (getpeername(s,(struct sockaddr *)&sinpeer,&socklen)!=0) return FALSE;
			if (peerip) *peerip=sinpeer.sin_addr.s_addr;
			if (peerport) *peerport=sinpeer.sin_port;
		}
		return TRUE;
	}
private:
	char *localip;
	int localipsize;
};

#define ErrorCode_InitError			0
#define ErrorCode_SetOptionError	1
#define ErrorCode_BindError			2
#define ErrorCode_IoctlError		3
#define MAX_SNIFFERSIZE				10

class THNetSniffer;

/**
* @brief 网络抓包类响应类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-06-30 新建类
*/
class THNetSnifferDataHandler
{
public:
	THNetSnifferDataHandler()
	{
		m_sSourceAddr=0;
		memset(m_sDictAddr,0,sizeof(m_sDictAddr));
		for(int i=0;i<MAX_SNIFFERSIZE;i++)
			m_iProtocol[i]=-1;
	}
	virtual void OnError(int nErrCode)=0;

	/**
	* @brief 抓包数据回调函数
	* @param protocol		协议类型,IPPROTO_XXX
	* @param saddr			源地址,网络顺序
	* @param daddr			目标地址,网络顺序
	* @param data			数据指针
	* @param len			数据长度
	*/
	virtual void OnData(U8 protocol,U32 saddr,U32 daddr,const void *data,unsigned int len)=0;

	/**
	* @brief 设置监听源地址，对应监听网卡的ip地址
	* @param sAddrIp		32位整数ip，本机顺序
	*/
	void SetSnifferAddr(U32 sAddrIp){m_sSourceAddr=htonl(sAddrIp);}

	/**
	* @brief 增加过滤地址，即需要监听的ip地址
	* @param sAddrIp		32位整数ip，本机顺序
	*/
	void AddFilterAddr(U32 sAddrIp)
	{
		for(int i=0;i<MAX_SNIFFERSIZE;i++)
		{
			if (m_sDictAddr[i]==0)
				break;
		}
		if (i==MAX_SNIFFERSIZE) i=MAX_SNIFFERSIZE-1;
		m_sDictAddr[i]=htonl(sAddrIp);
	}

	/**
	* @brief 清空过滤地址列表
	*/
	void EmptyFilterAddr(){memset(m_sDictAddr,0,sizeof(m_sDictAddr));}

	/**
	* @brief 增加过滤协议，即需要监听的协议
	* @param proto		协议名称,IPPROTO_XXX
	*/
	void AddFilterProtocol(U8 proto)
	{
		for(int i=0;i<MAX_SNIFFERSIZE;i++)
		{
			if (m_iProtocol[i]==-1)
				break;
		}
		if (i==MAX_SNIFFERSIZE) i=MAX_SNIFFERSIZE-1;
		m_iProtocol[i]=(I32)(U32)proto;
	}

	/**
	* @brief 清空过滤协议
	*/
	void EmptyFilterProtocol()
	{
		for(int i=0;i<MAX_SNIFFERSIZE;i++)
			m_iProtocol[i]=-1;
	}

	friend class THNetSniffer;
protected:
	U32 m_sSourceAddr;///<监听源地址，对应监听网卡的ip地址，该值不可为0，可由THNetWork::GetLocalIp获得，网络顺序。
	U32 m_sDictAddr[MAX_SNIFFERSIZE];///<监听目标地址，如需要监听所有数据，该值全为0，否则为各个允许的ip，包括源和目标地址，网络顺序。
	I32 m_iProtocol[MAX_SNIFFERSIZE];///<协议类型过滤，-1为不过滤，其他为协议类型，IPPROTO_XXX,IP:0,ICMP:1,IGMP:2,TCP:6,UDP:17,RAW:255
};

/**
* @brief 网络抓包分析类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2013-02-21 新建类
*/
class THNetSnifferDataParser
{
public:
	THNetSnifferDataParser()
	{
	}
	virtual ~THNetSnifferDataParser()
	{
	}
	typedef struct _TCPPacket{
		U32 saddr;
		U16 sport;
		U32 daddr;
		U16 dport;
		U8 *data;
		unsigned int len;
	}TCPPacket;
	static THNetSnifferDataParser::TCPPacket *ParsePacket(U8 protocol,U32 saddr,U32 daddr,const void *data,unsigned int len) {
		if (protocol!=IPPROTO_TCP) return NULL;
		TCPPacket *p=new TCPPacket;
		if (!p) return NULL;
		const unsigned char *buf=(const unsigned char *)data;
		buf+=sizeof(THIpHeader);
		const THTcpHeader *tcp=(const THTcpHeader *)buf;
		//push signal for tcp data
		if (!tcp->header_flags.push) return NULL;
//		tcp->whole_headerflags=ntohs(tcp->whole_headerflags);
		p->saddr=saddr;
		p->daddr=daddr;
		p->sport=tcp->source_port;
		p->dport=tcp->dest_port;
		THDebug(_T("len:%x*4 r:%x nonce:%x cwr:%x echo:%x urgent:%x ack:%x push:%x reset:%x syn:%x fin:%x"),
			tcp->header_flags.header_len,tcp->header_flags.reserved,
			tcp->header_flags.nonce,tcp->header_flags.cwr,
			tcp->header_flags.ecn_echo,tcp->header_flags.urgent,
			tcp->header_flags.ack,tcp->header_flags.push,
			tcp->header_flags.reset,tcp->header_flags.syn,
			tcp->header_flags.fin);
		len-=sizeof(THIpHeader)+sizeof(THTcpHeader);
		if (len>0) {
			p->data=new unsigned char[len+1];
			memcpy(p->data, buf+sizeof(THTcpHeader),len);
			p->data[len]='\0';
		} else p->data=NULL;
		p->len=len;
		return p;
	}

	static void FreePacket(THNetSnifferDataParser::TCPPacket *packet) {
		if (packet) {
			if (packet->data) delete [] packet->data;
			delete packet;
		}
	}

	/**
	* @brief 抓包数据回调函数
	* @param protocol		协议类型,IPPROTO_XXX
	* @param saddr			源地址,网络顺序
	* @param daddr			目标地址,网络顺序
	* @param data			数据指针
	* @param len			数据长度
	*/
	virtual void OnData(U8 protocol,U32 saddr,U32 daddr,const void *data,unsigned int len)=0;
};

/**
* @brief 网络抓包类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-06-30 新建类
*/
/**<pre>
用法：
	class MyData : public THNetSnifferDataHandler
	{
	public:
		MyData():THNetSnifferDataHandler(){}
		virtual void OnError(int nErrCode)
		{
			THDebug(_T("OnError:%d,LastError:%d"),nErrCode,WSAGetLastError());
		}
		virtual void OnData(U8 protocol,U32 saddr,U32 daddr,const void *data,unsigned int len)
		{
			THDebug(_T("Protocol:%d,Len:%d\r\n%s"),protocol,len,THStringConv::FormatString(data,len));
		}
	};

	THNetSniffer sniffer;
	MyData data;
	data.SetSnifferAddr(sniffer.GetLocalIp(1));
	data.AddFilterAddr(sniffer.GetIpFromHostName(_T("www.google.com")));
	data.AddFilterProtocol(IPPROTO_TCP);
	data.AddFilterProtocol(IPPROTO_UDP);
	sniffer.StartSniffer(data);
</pre>*/
class THNetSniffer : public THNetWork,public THThread
{
public:
	THNetSniffer(WORD wVersionRequested=SocketVersion22):THNetWork(wVersionRequested){}
	virtual ~THNetSniffer(){}

	/**
	* @brief 开始一个监听
	* @param THSnifferDataHandler		数据回调指针
	* @return 返回线程号，用于停止抓包，返回0为出错
	*/
	inline int StartSniffer(THNetSnifferDataHandler *handler){return StartThread(0,handler);}

	/**
	* @brief 暂停监听
	* @param threadid		线程id
	* @return 是否成功
	*/
	inline BOOL PauseSniffer(int threadid){return PauseThread(threadid);}

	/**
	* @brief 恢复监听
	* @param threadid		线程id
	* @return 是否成功
	*/
	inline BOOL ResumeSniffer(int threadid){return ResumeThread(threadid);}

	/**
	* @brief 停止监听
	* @param threadid		线程id
	* @return 是否成功
	*/
	inline BOOL StopSniffer(int threadid){return KillThread(threadid);}

	/**
	* @brief 设置监听优先级
	* @param threadid		线程id
	* @param priority		线程优先级
	* @return 是否成功
	*/
	inline BOOL SetSnifferPriority(int threadid,int priority){return SetThreadPriority(threadid,priority);}

	virtual void ThreadFunc(int ThreadId,void *data)
	{
		THNetSnifferDataHandler *handler=(THNetSnifferDataHandler *)data;
		if (handler)
		{
			SOCKET s=socket(AF_INET,SOCK_RAW,IPPROTO_IP);
			if(INVALID_SOCKET==s)
			{
				handler->OnError(ErrorCode_InitError);
				return;
			}
			int rcvtimeo=5000;
			if(setsockopt(s,SOL_SOCKET,SO_RCVTIMEO,(const char *)&rcvtimeo,sizeof(rcvtimeo))==SOCKET_ERROR)
			{
				handler->OnError(ErrorCode_SetOptionError);
				closesocket(s);
				return;
			}
			SOCKADDR_IN sa;
			sa.sin_family=AF_INET;
 			sa.sin_port=htons(7000);
			sa.sin_addr.s_addr=handler->m_sSourceAddr;
			if (bind(s,(PSOCKADDR)&sa,sizeof(sa))==SOCKET_ERROR)
			{
				handler->OnError(ErrorCode_BindError);
				closesocket(s);
				return;
			}
    		DWORD dwBufferLen[10];
			DWORD dwBufferInLen=1;
			DWORD dwBytesReturned=0;
			if(WSAIoctl(s,SIO_RCVALL,&dwBufferInLen,sizeof(dwBufferInLen),&dwBufferLen,sizeof(dwBufferLen),&dwBytesReturned,NULL,NULL)==SOCKET_ERROR)
			{
				handler->OnError(ErrorCode_IoctlError);
				closesocket(s);
				return;
			}
			char buf[1024];
			char *bufwork;
			int iRet;
			THIpHeader *pIpHeader;
			char *pLastBuf=NULL;
			while(1)
			{
				memset(buf,0,sizeof(buf));
				iRet=recv(s,buf,sizeof(buf),0);
				if(iRet==SOCKET_ERROR)
				{
					Sleep(100);
					continue;//is this have pro
				}
				else if (iRet==0) return;//end socket
        		pIpHeader=(THIpHeader *)buf;
				bufwork=buf;
	            WORD iLen=ntohs(pIpHeader->total_len);
				while(1)
				{
					if(iLen<=iRet)
					{
						//have whole packet
 					 	if(IsFitPacket(pIpHeader,handler))
						{
							handler->OnData(pIpHeader->proto,pIpHeader->source_ip,pIpHeader->dest_ip,bufwork,iLen);
						}
						if(iLen<iRet)
						{
							iRet-=iLen;
							bufwork+=iLen;
        					pIpHeader=(THIpHeader *)bufwork;
							iLen=ntohs(pIpHeader->total_len);
						}
						else
							break;// pIpHeader->total_len == iRet and go out
					}
					else
					{
						// read last part of buf. I wrote it , but always recv() read exactly
						// the lenght of the packet
						int iLast=iLen-iRet;
						pLastBuf=new char[iLen];
						int iReaden=iRet ;
						memcpy(pLastBuf,bufwork ,iReaden);
						iRet=recv(s,pLastBuf+iReaden,iLast,0);
						if(iRet==SOCKET_ERROR)
							break ;
						else
						{
							bufwork = pLastBuf;	
       						pIpHeader = (THIpHeader *)bufwork;
							if( iRet == iLast )
								iRet = iLen ;
							else
							{
								// read all last data
								iReaden+=iRet;
								iLast-=iRet;
								while(1)
								{
									iRet = recv(s , pLastBuf +iReaden , iLast , 0 ) ;
									if( iRet == SOCKET_ERROR )
										break ;
									else
									{
								        iReaden += iRet ;
								        iLast -= iRet ;
									    if( iLast <= 0 ) 
										    break ;
									}	
								}// while
							}
						}
					}
				}// while
				if(pLastBuf)
				{
					delete [] pLastBuf;
					pLastBuf=NULL;
				}
				Sleep(100);//Polling each 100 millisecond
			}
		}
	}
private:
	BOOL IsFitPacket(THIpHeader *header,THNetSnifferDataHandler *handler)
	{
		//check the ip rule
		BOOL bFound=FALSE;
		for(int i=0;i<MAX_SNIFFERSIZE;i++)
		{
			if (handler->m_sDictAddr[i]==0)
			{
				if (bFound==FALSE)
					break;
				else
					return FALSE;
			}
			else
				bFound=TRUE;
			if (handler->m_sDictAddr[i]==header->dest_ip ||
				handler->m_sDictAddr[i]==header->source_ip)
				break;
		}
		//check the protocol
		bFound=FALSE;
		for(int i=0;i<MAX_SNIFFERSIZE;i++)
		{
			if (handler->m_iProtocol[i]==-1)
			{
				if (bFound==FALSE)
					break;
				else
					return FALSE;
			}
			else
				bFound=TRUE;
			if (handler->m_iProtocol[i]==((I32)(U32)header->proto))
				break;
		}
		return TRUE;
	}
};

/**
* @brief 网卡信息获取类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-03-05 新建类
*/
/**<pre>
用法：
</pre>*/
class THAdaptersInfo
{
public:
	THAdaptersInfo()
	{
		m_nCount=0;
		DWORD dwRetVal = 0;

		m_pAdapterInfo = (IP_ADAPTER_INFO *) new char[sizeof(IP_ADAPTER_INFO)];
		ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);

		if (m_pAdapterInfo)
		{
			// Make an initial call to GetAdaptersInfo to get
			// the necessary size into the ulOutBufLen variable
			if (GetAdaptersInfo( m_pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
				delete [] (char *)m_pAdapterInfo;
				m_pAdapterInfo = (IP_ADAPTER_INFO *) new char[ulOutBufLen]; 
			}
		}

		if (m_pAdapterInfo)
		{
			if ((dwRetVal = GetAdaptersInfo( m_pAdapterInfo, &ulOutBufLen)) != NO_ERROR)
			{
				delete [] (char *)m_pAdapterInfo;
				m_pAdapterInfo=NULL;
			}
			else
			{
				PIP_ADAPTER_INFO pAdapter = m_pAdapterInfo;
				while (pAdapter)
				{
					pAdapter=pAdapter->Next;
					m_nCount++;
				}
			}
		}
	}

	virtual ~THAdaptersInfo()
	{
		if (m_pAdapterInfo)
			delete [] (char*)m_pAdapterInfo;
	}

	UINT GetAdapterCount()
	{
		return m_nCount;
	}

	inline PIP_ADAPTER_INFO GetAdapterInfo(int idx) 
	{
		PIP_ADAPTER_INFO pAdapter = m_pAdapterInfo;
		while (pAdapter && idx>0)
		{
			pAdapter=pAdapter->Next;
			idx--;
		}
		return pAdapter;
	}

	THString GetAdapterName(int idx)
	{
		PIP_ADAPTER_INFO pAdapter = GetAdapterInfo(idx);
		if (pAdapter)
			return THCharset::a2t(pAdapter->AdapterName);
		return _T("");
	}

	THString GetDescription(int idx)
	{
		PIP_ADAPTER_INFO pAdapter = GetAdapterInfo(idx);
		if (pAdapter)
			return THCharset::a2t(pAdapter->Description);
		return _T("");
	}

	//mac
	void* GetAddress(int idx)
	{
		PIP_ADAPTER_INFO pAdapter = GetAdapterInfo(idx);
		if (pAdapter)
			return pAdapter->Address;
		return NULL;
	}

	//mac
	THString GetAddressString(int idx)
	{
		void *buf=GetAddress(idx);
		if (buf)
			return THStringConv::BinToHexString(buf,MAX_ADAPTER_ADDRESS_LENGTH);
		return _T("");
	}

	//mac
	THString GetAddressStringTrim(int idx)
	{
		return GetAddressString(idx).Left(12);
	}

	U32 GetIp(int idx)
	{
		return THNetWork::GetIpFromHostName(GetIpString(idx));
	}

	THString GetIpString(int idx)
	{
		PIP_ADAPTER_INFO pAdapter = GetAdapterInfo(idx);
		if (pAdapter)
			return THCharset::a2t(pAdapter->IpAddressList.IpAddress.String);
		return _T("");
	}

	U32 GetMask(int idx)
	{
		return THNetWork::GetIpFromHostName(GetMaskString(idx));
	}

	THString GetMaskString(int idx)
	{
		PIP_ADAPTER_INFO pAdapter = GetAdapterInfo(idx);
		if (pAdapter)
			return THCharset::a2t(pAdapter->IpAddressList.IpMask.String);
		return _T("");
	}

	U32 GetGateway(int idx)
	{
		return THNetWork::GetIpFromHostName(GetGatewayString(idx));
	}

	THString GetGatewayString(int idx)
	{
		PIP_ADAPTER_INFO pAdapter = GetAdapterInfo(idx);
		if (pAdapter)
			return THCharset::a2t(pAdapter->GatewayList.IpAddress.String);
		return _T("");
	}

	BOOL IsDhcpEnable(int idx)
	{
		PIP_ADAPTER_INFO pAdapter = GetAdapterInfo(idx);
		if (pAdapter)
			return pAdapter->DhcpEnabled;
		return FALSE;
	}

	/*MIB_IF_TYPE_OTHER 
	MIB_IF_TYPE_ETHERNET 
	MIB_IF_TYPE_TOKENRING 
	MIB_IF_TYPE_FDDI 
	MIB_IF_TYPE_PPP 
	MIB_IF_TYPE_LOOPBACK 
	MIB_IF_TYPE_SLIP*/
	UINT GetAdapterType(int idx)
	{
		PIP_ADAPTER_INFO pAdapter = GetAdapterInfo(idx);
		if (pAdapter)
			return pAdapter->Type;
		return 0;
	}

	THString GetDhcpString(int idx)
	{
		PIP_ADAPTER_INFO pAdapter = GetAdapterInfo(idx);
		if (pAdapter && pAdapter->DhcpEnabled)
			return THCharset::a2t(pAdapter->DhcpServer.IpAddress.String);
		return _T("");
	}

	time_t GetDhcpLeaseObtained(int idx)
	{
		PIP_ADAPTER_INFO pAdapter = GetAdapterInfo(idx);
		if (pAdapter && pAdapter->DhcpEnabled)
			return pAdapter->LeaseObtained;
		return 0;
	}

	time_t GetDhcpLeaseExpires(int idx)
	{
		PIP_ADAPTER_INFO pAdapter = GetAdapterInfo(idx);
		if (pAdapter && pAdapter->DhcpEnabled)
			return pAdapter->LeaseExpires;
		return 0;
	}

	BOOL IsHaveWins(int idx)
	{
		PIP_ADAPTER_INFO pAdapter = GetAdapterInfo(idx);
		if (pAdapter)
			return pAdapter->HaveWins;
		return FALSE;
	}

	THString GetPrimaryWinsServer(int idx)
	{
		PIP_ADAPTER_INFO pAdapter = GetAdapterInfo(idx);
		if (pAdapter && pAdapter->HaveWins)
			return THCharset::a2t(pAdapter->PrimaryWinsServer.IpAddress.String);
		return FALSE;
	}

	THString GetSecondaryWinsServer(int idx)
	{
		PIP_ADAPTER_INFO pAdapter = GetAdapterInfo(idx);
		if (pAdapter && pAdapter->HaveWins)
			return THCharset::a2t(pAdapter->SecondaryWinsServer.IpAddress.String);
		return FALSE;
	}

	THString GetBestIntIpStr()
	{
		THString str;
		PIP_ADAPTER_INFO pAdapter = m_pAdapterInfo;
		while(pAdapter)
		{
			if (THNetWork::GetIpFromHostName(THCharset::a2t(pAdapter->GatewayList.IpAddress.String))!=0)
			{
				//the out going adapter
				str=THCharset::a2t(pAdapter->IpAddressList.IpAddress.String);
				if (str!=_T("0.0.0.0")) break;
			}
			else
			{
				if (pAdapter->Type==MIB_IF_TYPE_ETHERNET)
				{
					//other ethernet adapter,temp to use
					THString tmpintip=THCharset::a2t(pAdapter->IpAddressList.IpAddress.String);
					if (tmpintip!=_T("0.0.0.0")) str=tmpintip;
				}
			}
			pAdapter=pAdapter->Next;
		}
		return str;
	}

	//返回最合适的内网网卡ip
	inline U32 GetBestIntIp()
	{
		return THNetWork::GetIpFromHostName(GetBestIntIpStr());
	}

	inline U32 GetBestIntIpN()
	{
		return THNetWork::GetIpFromHostNameN(GetBestIntIpStr());
	}

	//返回最合适的内网网卡索引
	int GetBestIntAdapter()
	{
		int idx=0;
		int i=0;
		PIP_ADAPTER_INFO pAdapter = m_pAdapterInfo;
		while (pAdapter)
		{
			if (THNetWork::GetIpFromHostName(THCharset::a2t(pAdapter->GatewayList.IpAddress.String))!=0)
			{
				//the out going adapter
				idx=i;
				break;
			}
			else
			{
				if (pAdapter->Type==MIB_IF_TYPE_ETHERNET)
				{
					//other ethernet adapter,temp to use
					if (idx==0) idx=i;
				}
			}
			pAdapter=pAdapter->Next;
			i++;
		}
		return idx;
	}
private:
	PIP_ADAPTER_INFO m_pAdapterInfo;
	UINT m_nCount;
};

/**
* @brief 网卡信息获取类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-03-05 新建类
*/
/**<pre>
用法：
</pre>*/
class THIpRange
{
public:
	THIpRange()
	{
		m_ip=NULL;
		m_size=0;
		m_allocsize=0;
	}

	virtual ~THIpRange()
	{
		if (m_ip) delete [] m_ip;
	}

	//192.168.0.1:255.255.255.0|192.168.1.50:255.255.255.255
	THString SaveToString()
	{
		THString str;
		for(U32 i=0;i<m_size;i++)
			str.AppendFormat(_T("%s:%s|"),THNetWork::GetAddrStringN(m_ip[i*2]),THNetWork::GetAddrStringN(m_ip[i*2+1]));
		return str;
	}

	//192.168.0.1:255.255.255.0|192.168.1.50:255.255.255.255,mask 默认为255.255.255.255
	void LoadFromString(THString str)
	{
		m_mtx.ChangeLock();
		m_size=0;
		m_mtx.ChangeUnlock();
		THStringToken t(str,_T("|")),t1;
		THString ip,mask;
		while(t.IsMoreTokens())
		{
			t1.Init(t.GetNextToken(),_T(":"));
			ip=t1.GetNextToken();
			mask=t1.GetNextToken();
			if (!ip.IsEmpty())
			{
				if (mask.IsEmpty()) mask=_T("255.255.255.255");
				AddIpRange(ip,mask);
			}
		}
	}

	//注意：ip 为网络顺序，性能更高
	//只要有任一bNot=0的Range成立即退出
	BOOL IsIpInRangeN(U32 ip)
	{
		m_mtx.ReadOnlyLock();
		BOOL bRet=FALSE;
		for(U32 i=0;i<m_size;i++)
		{
			int off=i*2;
			if ((m_ip[off+1]&ip) == m_ip[off])
			{
				bRet=TRUE;
				break;
			}
		}
		m_mtx.ReadOnlyUnlock();
		return bRet;
	}

	//ip,mask为本机顺序
	void AddIpRange(U32 ip,U32 mask)
	{
		m_mtx.ChangeLock();
		if (m_allocsize<=m_size)
		{
			U32 *newip=new U32[(m_allocsize+10)*2];
			if (!newip)
			{
				m_mtx.ChangeUnlock();
				return;
			}
			if (m_size>0)
			{
				memcpy(newip,m_ip,m_allocsize*2);
				delete [] m_ip;
			}
			m_ip=newip;
			m_allocsize+=10;
		}
		m_ip[m_size*2]=htonl(ip&mask);
		m_ip[m_size*2+1]=htonl(mask);
		m_size++;
		m_mtx.ChangeUnlock();
	}

	void AddIpRange(LPCTSTR ip,LPCTSTR mask)
	{
		if (!ip || !mask) return;
		AddIpRange(THNetWork::GetIpFromHostName(ip),THNetWork::GetIpFromHostName(mask));
	}
protected:
	THMutexEx m_mtx;
	U32 *m_ip;
	U32 m_size;
	U32 m_allocsize;
};
