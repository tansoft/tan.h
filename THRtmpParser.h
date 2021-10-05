#pragma once

#define THSYSTEM_INCLUDE_STL_STRUCT
#include <THSystem.h>
#include <THMemBuf.h>
#include <THFlashBase.h>
#include <THAmfParser.h>

#define THRTMP_PRINTDETAIL
#ifdef THRTMP_PRINTDETAIL
	#include <THDebug.h>
	#define THRtmpDebug			THDebug
#else
	#define THRtmpDebug
#endif

#define THRTMP_DEFCUTSIZE				128
#define THRTMP_HANDSHAKESIZE			1536
#define THRTMP_RTMPVER					0x3

#define THRTMPSTATE_NONE				0	///<初始状态
#define THRTMPSTATE_HANDSHAKE_VER_ACK	1	///<握手版本确认及时间ack
#define THRTMPSTATE_HANDSHAKE_OK		2	///<握手完成
#define THRTMPSTATE_CONNECT				3
#define THRTMPSTATE_CREATESTREAM		4
#define THRTMPSTATE_PLAY				5
#define	THRTMPSTATE_STREAMING			6	///<开始流时状态

#define THRTMPAMFTYPE_CHANGECHUNKSIZE		0x1
#define THRTMPAMFTYPE_KEEPALIVESIZE			0x3
#define THRTMPAMFTYPE_PING					0x4
#define THRTMPAMFTYPE_SERVERBW				0x5
#define THRTMPAMFTYPE_CLIENTBW				0x6
#define THRTMPAMFTYPE_AUDIO					0x8
#define THRTMPAMFTYPE_VIDEO					0x9
#define THRTMPAMFTYPE_AMF3OBJECT			0x11
#define THRTMPAMFTYPE_CALLDONOTEXPECTREPLY	0x12
#define THRTMPAMFTYPE_CALL					0x14
#define THRTMPAMFTYPE_AVPACKET				0x16	//Audio and Video Combin Packet

#define THRTMPPINGTYPE_STREAMCLEAR				0
#define THRTMPPINGTYPE_STREAMPLAYBUFFERCLEAR	1
#define THRTMPPINGTYPE_CLIENTBUFFER				3
#define THRTMPPINGTYPE_STREAMRESET				4
#define THRTMPPINGTYPE_PINGCLIENT				6
#define THRTMPPINGTYPE_PONGSERVER				7

/**
* @brief Rtmp包头内容
*/
typedef struct _THRtmpPartHeader{
	uint32_t timestamp;
	uint32_t pklen;
	uint8_t amftype;
	uint32_t channelid;
	_THRtmpPartHeader()
	{
		timestamp=0;
		pklen=0;
		amftype=0;
		channelid=0;
	}
}THRtmpPartHeader;

typedef map<const uint8_t,THRtmpPartHeader> THRtmpHeaders;

class THRtmpState
{
public:
	THRtmpState()
	{
		m_nState=THRTMPSTATE_NONE;
		m_nCutSize=THRTMP_DEFCUTSIZE;
	}
	virtual ~THRtmpState(){}
	volatile uint32_t m_nState;		///<当前Rtmp状态
	volatile uint32_t m_nCutSize;	///<切割size
	THRtmpHeaders m_header;	///<保存不同streamid对应的头部的默认信息，用于压缩头的还原
};

class THRtmpParser;

class THRtmpPacket
{
public:
	friend class THRtmpParser;
	THRtmpPacket()
	{
		m_nHeaderLen=0;
		m_nStreamId=0;
		m_nAmfType=0;
		m_nTimeStamp=0;
		m_nChannelId=0;
		m_bParsedAmf=FALSE;
	}
	virtual ~THRtmpPacket()
	{
	}

	BOOL IsHeaderComplete(){return m_nHeaderLen==12;}
	uint8_t GetHeaderLen(){return m_nHeaderLen;}

	uint8_t *GetPakcetOrgBuffer(uint32_t &len,const uint32_t cutsize)
	{
		uint32_t pklen;
		uint8_t *amfbuf=(uint8_t *)m_amfout.GetBuf(&pklen);
		uint8_t bit[3];
		uint32_t bitlen=0;
		len=m_nHeaderLen+pklen+GetAddLen(m_nStreamId,cutsize,pklen,bit,&bitlen);
		uint8_t *outbuf=new uint8_t[len];
		ASSERT(outbuf);
		uint8_t *p=outbuf;
		uint8_t header=0;
		if (m_nHeaderLen==1) header=0xC0;
		else if (m_nHeaderLen==4) header=0x80;
		else if (m_nHeaderLen==8) header=0x40;
		header|=m_nStreamId&0x3f;
		*p++=header;
		// 1-3 is timestamp
		if (m_nHeaderLen>=4)
		{
			THFlashBase::_SetU24Length(p,m_nTimeStamp);
			p+=3;
			// 4-6 is amf len
			if (m_nHeaderLen>=8)
			{
				THFlashBase::_SetU24Length(p,pklen);
				p+=3;
				// 7 is amf type
				*p++=m_nAmfType;
				// 8-11 is channel id
				if (m_nHeaderLen>=12)
				{
					uint32_t cid=/*htonl*/(m_nChannelId);
					memcpy(p,&cid,sizeof(uint32_t));
					p+=sizeof(uint32_t);
				}
			}
		}
		if (pklen>0)
		{
			uint32_t edlen=0;
			while(edlen<pklen)
			{
				uint32_t wlen=(pklen-edlen>cutsize)?cutsize:pklen-edlen;
				memcpy(p,amfbuf+edlen,wlen);
				p+=wlen;
				edlen+=wlen;
				if (edlen<pklen && bitlen>0)
				{
					memcpy(p,bit,bitlen);
					p+=bitlen;
				}
			}
		}
		m_amfout.ReleaseGetBufLock();
		return outbuf;
	}
	static void FreePacketOrgBuffer(void *p){delete [] (uint8_t *)p;}

	uint32_t GetStreamId(){return m_nStreamId;}
	uint32_t GetChannelId(){return m_nChannelId;}
	uint8_t GetAmfType(){return m_nAmfType;}
	uint32_t GetTimeStamp(){return m_nTimeStamp;}

	//返回数据不用释放
	uint8_t *GetFullHeader()
	{
		uint8_t *p=m_FullHeaderBuffer;
		*p++=m_nStreamId&0x3f;
		THFlashBase::_SetU24Length(p,m_nTimeStamp);
		p+=3;
		uint32_t amflen=GetRawBufferLen();
		THFlashBase::_SetU24Length(p,amflen);
		p+=3;
		*p++=m_nAmfType;
		uint32_t cid=/*htonl*/(m_nChannelId);
		memcpy(p,&cid,sizeof(uint32_t));
		return m_FullHeaderBuffer;
	}
	inline uint8_t *GetRawBuffer(uint32_t *len){return (uint8_t *)m_amfin.GetBuf(len,FALSE);}
	inline uint32_t GetRawBufferLen() {return m_amfin.GetBufferedSize();}
	uint32_t GetRawBufferU32(uint32_t off=0)
	{
		uint32_t ret=0;
		uint32_t len;
		uint8_t *buf=(uint8_t *)m_amfin.GetBuf(&len);
		if (len>=off+sizeof(uint32_t))
		{
			memcpy(&ret,buf+off,sizeof(uint32_t));
			ret=ntohl(ret);
		}
		m_amfin.ReleaseGetBufLock();
		return ret;
	}
	uint8_t GetRawBufferU8(uint32_t off=0)
	{
		uint8_t ret=0;
		uint32_t len;
		uint8_t *buf=(uint8_t *)m_amfin.GetBuf(&len);
		if (len>=off+sizeof(uint8_t))
			ret=*(buf+off);
		m_amfin.ReleaseGetBufLock();
		return ret;
	}

	THString GetAmfTypeString()
	{
		switch(m_nAmfType)
		{
			case 0:return _T("none");
			case THRTMPAMFTYPE_CHANGECHUNKSIZE:return _T("Change Chunk Size");
			case THRTMPAMFTYPE_KEEPALIVESIZE:return _T("Keep Alive Size");
			case THRTMPAMFTYPE_PING:return _T("Ping");
			case THRTMPAMFTYPE_SERVERBW:return _T("Server BW");
			case THRTMPAMFTYPE_CLIENTBW:return _T("Client BW");
			case THRTMPAMFTYPE_AUDIO:return _T("Audio Packet");
			case THRTMPAMFTYPE_VIDEO:return _T("Video Packet");
			case THRTMPAMFTYPE_CALL:return _T("Remote Call");
			case THRTMPAMFTYPE_CALLDONOTEXPECTREPLY:return _T("Remote Call Donot Expect reply");
			case THRTMPAMFTYPE_AMF3OBJECT:return _T("Amf3Object");
			case THRTMPAMFTYPE_AVPACKET:return _T("AV Packet");
		}
		return _T("unknown");
	}

	static THString GetPingTypeString(uint16_t type)
	{
		switch(type)
		{
			case THRTMPPINGTYPE_STREAMCLEAR:return _T("Stream Clear");
			case THRTMPPINGTYPE_STREAMPLAYBUFFERCLEAR:return _T("Stream PlayBuffer Clear");
			case THRTMPPINGTYPE_CLIENTBUFFER:return _T("Client Buffer");
			case THRTMPPINGTYPE_STREAMRESET:return _T("Stream Reset");
			case THRTMPPINGTYPE_PINGCLIENT:return _T("Ping Client");
			case THRTMPPINGTYPE_PONGSERVER:return _T("Pong Server");
		}
		return _T("unknown");
	}
	THString Print()
	{
		THString s=PrintHeader();
		THAmf0Parser man;
		return s;
	}

	THString PrintHeader()
	{
		THString ret;
		ret.Format(_T("Stream Id:0x%x\r\nTimeStamp:0x%x\r\nAmf Type:0x%x(%s)\r\nChannel Id:0x%x\r\n"),
			m_nStreamId,m_nTimeStamp,m_nAmfType,GetAmfTypeString(),m_nChannelId);
		return ret;
	}
	static BOOL IsAmfType(uint8_t ntype)
	{
		return (ntype==THRTMPAMFTYPE_CALLDONOTEXPECTREPLY || ntype==THRTMPAMFTYPE_CALL);
	}
	BOOL IsAmfType(){return IsAmfType(m_nAmfType);}
protected:
	#define UINT32_1(x1) ((uint32_t)(x1))
	#define UINT32_PART(val,index) ((uint8_t)(UINT32_1(UINT32_1(val)&UINT32_1(0x000000ff<<((3-index)*8)))>>((3-index)*8)))

	//实在恶心！！每128字节会加一个一字节的头，还会动态变化！
	static uint32_t GetAddLen(uint32_t nStreamId,uint32_t cutsize,uint32_t pklen,uint8_t *addheader=NULL,uint32_t *addlen=NULL)
	{
		uint32_t part=0;
		if (nStreamId <= 63)
		{
			if (addheader) addheader[0]=(uint8_t)(0xc0|nStreamId);
			part=1;
		}
		else if (nStreamId < 320)//rtmpprotocolserializer.cpp 为 <= 320,但我觉得是 < 320-64=0x100
		{
			if (addheader)
			{
				addheader[0]=0xc0;
				addheader[1]=(uint8_t)(nStreamId-64);
			}
			part=2;
		}
		else
		{
			if (addheader)
			{
				addheader[0]=0xc1;
				addheader[1]=UINT32_PART(nStreamId-64,3);
				addheader[2]=UINT32_PART(nStreamId-64,2);
			}
			part=3;
		}
		if (addlen) *addlen=part;
		if (pklen==0) return 0;
		return ((uint32_t)(pklen-1)/cutsize)*(part);
	}
	void ParseAmf();
	BOOL m_bParsedAmf;
	uint8_t m_FullHeaderBuffer[12];
	uint8_t m_nHeaderLen;
	uint8_t m_nStreamId;
	uint8_t m_nAmfType;
	uint32_t m_nTimeStamp;
	uint32_t m_nChannelId;
	THMemBuf m_amfin;			///<用于输入包
	THMemBuf m_amfout;			///<用于输出包
};

/**
* @brief Flash RTMP协议封包解释类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2009-09-08 新建类
*/
/**<pre>
  使用Sample：
</pre>*/
class THRtmpParser
{
public:
	THRtmpParser(){}
	virtual ~THRtmpParser(){}

	static uint32_t IsHaveRtmpPacket(const uint8_t *buf,const uint32_t len,THRtmpState &state)
	{
		if (len==0) return 0;
		if (state.m_nState<THRTMPSTATE_HANDSHAKE_OK)
		{
			if (state.m_nState==THRTMPSTATE_NONE)
			{
				if (len>=THRTMP_HANDSHAKESIZE+1) return THRTMP_HANDSHAKESIZE+1;
			}
			else if (state.m_nState==THRTMPSTATE_HANDSHAKE_VER_ACK)
			{
				if (len>=THRTMP_HANDSHAKESIZE) return THRTMP_HANDSHAKESIZE;
			}
			return 0;
		}
		//check header size
		uint8_t header=*buf;
		uint8_t mode=header&0xC0;
		uint8_t id=header&0x3f;
		const THRtmpPartHeader &lastpack=state.m_header[id];
		const uint32_t cutsize=state.m_nCutSize;
		uint8_t headerlen=0;
		if (mode==0x00) headerlen=12;
		else if (mode==0x40) headerlen=8;
		else if (mode==0x80) headerlen=4;
		else headerlen=1;//0xC0
		if (len<headerlen) return 0;
		uint32_t pklen;
		if (headerlen>=8)
			pklen=THFlashBase::_GetU24Length(buf+0x4);
		else
			pklen=lastpack.pklen;
		int addlen=0;
		addlen=THRtmpPacket::GetAddLen(id,cutsize,pklen);
		if (len>=headerlen+pklen+addlen)
		{
			//CMyDebug(_T("h:%x,pk:%x,add:%x"),headerlen,pklen,addlen);
			return headerlen+pklen+addlen;
		}
		return 0;
	}

	//如果出错，返回NULL，成功返回THRtmpPacket对象，在HandShake时，该指针仅仅返回非零值1，通过IsPacketHandShake可判断一个Packet是否有效
	static THRtmpPacket *ParseRtmpPacket(const uint8_t *buf,const uint32_t len,THRtmpState &state)
	{
		if (len==0) return NULL;
		if (state.m_nState<THRTMPSTATE_HANDSHAKE_OK)
		{
			if (state.m_nState==THRTMPSTATE_NONE)
			{
				if (*buf!=THRTMP_RTMPVER)
				{
					THRtmpDebug(_T("Error:Rtmp Parse Fail,Ver:%d do not support"),*buf);
					return NULL;
				}
				uint32_t time=ntohl(*((uint32_t *)(buf+1)));
				THRtmpDebug(_T("Info:HandShake Ver ver:%u ts:%u"),*buf,time);
				if (*((uint32_t *)(buf+5))!=0x0)
				{
					THRtmpDebug(_T("Warning:Rtmp Parse Fail,C1/S1 off 5 is not a empty int32_t,buf:")+THStringConv::BinToHexString(buf,10));
					//return NULL;
				}
				THDebug(_T("HandShake ver:")+THStringConv::BinToHexString(buf,10));
				state.m_nState=THRTMPSTATE_HANDSHAKE_VER_ACK;
				return (THRtmpPacket *)1;
			}
			else if (state.m_nState==THRTMPSTATE_HANDSHAKE_VER_ACK)
			{
				uint32_t time=ntohl(*((uint32_t *)(buf)));
				uint32_t time2=ntohl(*((uint32_t *)(buf+4)));
				THRtmpDebug(_T("Info:HandShake Ack ts:%u other ts2:%u"),time,time2);
				THDebug(_T("HandShake ver ack:")+THStringConv::BinToHexString(buf,10));
				state.m_nState=THRTMPSTATE_HANDSHAKE_OK;
				return (THRtmpPacket *)1;
			}
			return NULL;
		}
		THRtmpPacket *p=new THRtmpPacket;
		ASSERT(p);
		//check header size
		uint8_t header=*buf;
		uint8_t mode=header&0xC0;
		p->m_nStreamId=header&0x3f;
		THRtmpPartHeader &lastpack=state.m_header[p->m_nStreamId];
		const uint32_t cutsize=state.m_nCutSize;
		uint8_t headerlen=0;
		if (mode==0x00) headerlen=12;
		else if (mode==0x40) headerlen=8;
		else if (mode==0x80) headerlen=4;
		else headerlen=1;//0xC0
		p->m_nHeaderLen=headerlen;
		ASSERT(len>=headerlen);
		// 1-3 is timestamp
		if (headerlen>=4)
		{
			p->m_nTimeStamp=THFlashBase::_GetU24Length(buf+0x1);
			lastpack.timestamp=p->m_nTimeStamp;
		}
		else
			p->m_nTimeStamp=lastpack.timestamp;
		// 4-6 is amf len
		uint32_t pklen;
		if (headerlen>=8)
		{
			pklen=THFlashBase::_GetU24Length(buf+0x4);
			lastpack.pklen=pklen;
		}
		else
			pklen=lastpack.pklen;
		// 7 is amf type
		if (headerlen>=8)
		{
			p->m_nAmfType=*(buf+0x7);
			lastpack.amftype=p->m_nAmfType;
		}
		else
			p->m_nAmfType=lastpack.amftype;
		// 8-11 is channel id
		if (headerlen==12)
		{
			memcpy(&p->m_nChannelId,buf+0x8,sizeof(uint32_t));
			//p->m_nChannelId=/*ntohl*/(p->m_nChannelId);
			lastpack.channelid=p->m_nChannelId;
		}
		else
			p->m_nChannelId=lastpack.channelid;
		uint8_t addheader[3];
		uint32_t addheaderlen=0;
		uint32_t addlen=THRtmpPacket::GetAddLen(p->m_nStreamId,cutsize,pklen,addheader,&addheaderlen);
		ASSERT(len>=headerlen+pklen+addlen);
		if (pklen>0)
		{
			if (p->IsAmfType())
			{
				// parse the addlen
				const uint8_t *amfbuf=buf+headerlen;
				uint32_t readed=headerlen;
				while(readed<len)
				{
					uint32_t rlen=(len-readed>cutsize)?cutsize:len-readed;
					p->m_amfin.AddBuf(buf+readed,rlen);
					readed+=rlen+addheaderlen;
					ASSERT(addheaderlen==0 || memcmp(buf+readed-addheaderlen,addheader,addheaderlen)==0 || readed>=len);
				}
			}
			else
			{
				//fixme，这里有问题么，非amf的包，是否也还有每隔cutsize一个标识呢？
				p->m_amfin.AddBuf(buf+headerlen,len-headerlen);
			}
		}
		if (p->m_nAmfType==THRTMPAMFTYPE_CHANGECHUNKSIZE)
		{
			state.m_nCutSize=p->GetRawBufferU32();
			THRtmpDebug(_T("Info:ChunkSize ChangeTo:%u"),state.m_nCutSize);
		}
		return p;
	}

	inline static void FreeRtmpPacket(THRtmpPacket *p) {if (!IsPacketHandShake(p)) delete p;}

	inline static BOOL IsPacketHandShake(THRtmpPacket *p) {return (INT_PTR)p==1;}

	static BOOL GenerateClientHandShake(THMemBuf *buf)
	{
		if (!buf->AddBufU8(THRTMP_RTMPVER)) return FALSE;//RTMP Version
		if (!buf->AddBufU32(GetMsTime())) return FALSE;//local time
		if (!buf->AddBufU32(0)) return FALSE;//must be 0
		uint8_t randBytes[THRTMP_HANDSHAKESIZE-8];
		for(int i=0;i<sizeof(randBytes);i++)
			randBytes[i]=rand();
		return buf->AddBuf(randBytes,sizeof(randBytes));
	}
	static uint32_t GetMsTime(){return (unsigned int)time(NULL);}
protected:

};
