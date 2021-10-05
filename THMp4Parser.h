#pragma once
//mpeg4 and iso format desc: ISO/IEC 14496-12,2005
/*
Sample Description, Sample Size, Sample To Chunk, and Chunk Offset

The Sync Sample Box is optional.
If the Sync Sample Box is not present, all samples are sync samples.

composition times (CT) 组合时间标签,指示在哪个时间要进入到 Compositor进行组合
decoding times (DT) 解码时间标签,指示在哪个时间进入到解码器进行解码

Decoding Time to Sample Box : stts
Composition Time to Sample Box : ctts
Sample Description Box : stsd
Sample Size Boxes : stsz,stz2
Sample To Chunk Box : stsc
Chunk Offset Box : stco,co64
Sync Sample Box : stss
*/

#define THSYSTEM_INCLUDE_SOCKET2
#include <THSystem.h>
#include <THDebug.h>
#include <THFile.h>
#include <THMemBuf.h>

#pragma pack(push,1)

typedef struct _THMp4Box{
	U32 nSize;
	U32 nBoxType;
	void ntoh()
	{
		nSize=ntohl(nSize);
		//nBoxType=ntohl(nBoxType);
	}
	void hton()
	{
		nSize=htonl(nSize);
		//nBoxType=htonl(nBoxType);
	}
}THMp4Box;

///<the THMp4Box.nSize must be 1
typedef struct _THMp4LargeBox{
	U64 nLargeSize;
	void ntoh()
	{
		U8 buf[8];
		U8 *tmp=(U8 *)&nLargeSize;
		for(int i=0;i<7;i++)
			buf[i]=*(tmp+7-i);
		memcpy(&nLargeSize,buf,8);
	}
	void hton()
	{
		U8 buf[8];
		U8 *tmp=(U8 *)&nLargeSize;
		for(int i=0;i<7;i++)
			buf[i]=*(tmp+7-i);
		memcpy(&nLargeSize,buf,8);
	}
}THMp4LargeBox;

typedef struct _THMp4ExtBox{
	U8 nExtType[16];
}THMp4ExtBox;

typedef struct _THMp4FullBox{
	U8 nVersion;
	U8 nFlags[3];
}THMp4FullBox;

typedef struct _THMp4FileTypeBox{
	U32 nMajorBrand;
	U32 nMinorVersion;
	U32	pnCompatibleBrands;
	void ntoh()
	{
		nMinorVersion=ntohl(nMinorVersion);
	}
	void hton()
	{
		nMinorVersion=htonl(nMinorVersion);
	}
}THMp4FileTypeBox;

typedef enum _THMp4BoxType{
	THMp4BoxType_None,
	THMp4BoxType_Box,
	THMp4BoxType_LargeBox,
	THMp4BoxType_ExtBox,
	THMp4BoxType_FullBox
}THMp4BoxType;

#pragma pack(pop)

#define CHECKBOXTYPE(a) (strcmp(m_nid,a)==0)
#define CHECKBOXTYPEP(a,b) (strcmp((const char *)a,b)==0)

/**
* @brief Mp4Box描述类(NetStream object)
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-06-06 新建类
*/
class THMp4BoxContainer
{
public:
	THMp4BoxContainer()
	{
		memset(&m_nid,0,sizeof(m_nid));
		m_nSize=0;
		m_nFileOff=0;
		m_intobj.SetFreeProc(FreeIntObj);
	}
	virtual ~THMp4BoxContainer()
	{
		m_intobj.RemoveAll();
	}

	//返回处理的偏移量，-1为错误
	I64 ParseBox(THFile *f,THString sParentSettings=_T(""))
	{
		if (!f) return -1;
		memset(&m_nid,0,sizeof(m_nid));
		m_nSize=0;
		m_nFileOff=0;
		m_buf.Empty();
		m_sParseData.Empty();
		THMp4Box box;
		U32 nSize=0;
		if (!_ReadFile(f,&box,sizeof(THMp4Box))) return -1;
		m_buf.AddBuf(&box,sizeof(THMp4Box));
		nSize+=sizeof(THMp4Box);
		box.ntoh();
		memcpy(&m_nid,&box.nBoxType,4);
		m_sParseData+=THSimpleXml::MakeParam(_T("BoxId"),GetIdDisplayName(m_nid));
		if (box.nSize==0)
		{
			ASSERT(FALSE);
			return 0;
		}
		else if (box.nSize==1)
		{
			//LargeBox
			THMp4LargeBox lbox;
			if (!_ReadFile(f,&lbox,sizeof(THMp4LargeBox))) return -1;
			m_buf.AddBuf(&lbox,sizeof(THMp4LargeBox));
			nSize+=sizeof(THMp4LargeBox);
			lbox.ntoh();
			m_nSize=lbox.nLargeSize;
		}
		else
		{
			m_nSize=box.nSize;
		}
		if (CHECKBOXTYPEP(&box.nBoxType,"uuid"))
		{
			//ExtBox
			THMp4ExtBox ebox;
			if (!_ReadFile(f,&ebox,sizeof(THMp4ExtBox))) return -1;
			m_buf.AddBuf(&ebox,sizeof(THMp4ExtBox));
			nSize+=sizeof(THMp4ExtBox);
			m_sParseData+=THSimpleXml::MakeParam(_T("BoxExtId"),THStringConv::BinToHexString(ebox.nExtType,16));
		}
		//ensure the remain size
		m_nSize-=nSize;
		m_nFileOff=f->GetPosition();
		//need get all buf?
		if (_IsNeedGetAllBuf())
		{
			UINT sz=(UINT)m_nSize;
			char *buf=new char[sz];
			if (!buf) return -1;
			if (!_ReadFile(f,buf,sz))
			{
				delete [] buf;
				return -1;
			}
			if (!_ParseContent(buf,sz,sParentSettings))
			{
				delete [] buf;
				return -1;
			}
			nSize+=(UINT)m_nSize;
			m_nSize=0;
			m_nFileOff=0;
			m_buf.AddBuf(buf,sz);
			delete [] buf;
		}
		else if (_IsNeedParse())
		{
			if (!_ParseContentFile(f,m_nSize,sParentSettings)) return -1;
		}
		else
		{
			if (!_Seek(f,m_nSize,SEEK_CUR)) return -1;
		}
		return m_nSize+nSize;
	}

	THString PrintDetail(THString sPre=_T(" "),THString sLevel=_T(""))
	{
		THString ret;
		if (m_nSize!=0)
			ret.Format(_T("%sBox:Type:%02x,%s remain data: off:%lu size:%ld\r\n"),sLevel,*(U32 *)m_nid,m_sParseData,(U32)m_nFileOff,(U32)m_nSize);
		else
			ret.Format(_T("%sBox:Type:%02x,%s\r\n"),sLevel,*(U32 *)m_nid,m_sParseData);
		THPosition pos=m_intobj.GetStartPosition();
		THMp4BoxContainer *container;
		while(!pos.IsEmpty())
		{
			if (m_intobj.GetNextPosition(pos,container))
			{
				if (container) ret+=container->PrintDetail(sPre,sPre+sLevel);
			}
		}
		return ret;
	}
	THMp4BoxContainer* FindIntObjByType(THString type)
	{
		THPosition pos=m_intobj.GetStartPosition();
		THMp4BoxContainer *container;
		while(!pos.IsEmpty())
		{
			if (m_intobj.GetNextPosition(pos,container))
			{
				if (container)
				{
					if (container->GetType()==type)
					{
						return container;
					}
					else
					{
						THMp4BoxContainer *tmp=container->FindIntObjByType(type);
						if (tmp) return tmp;
					}
				}
			}
		}
		return NULL;
	}

	//强制刷新容器的所有子容器，然后生成整段的buffer返回,多个sRemoveID以,分隔
	void GetModifyBuf(THMemBuf *buf,THFile &in,THString sRemoveID=_T(""))
	{
		UINT stoff=buf->GetBufferedSize();
		buf->AddBuf(&m_buf);
		THPosition pos=m_intobj.GetStartPosition();
		THMp4BoxContainer *container;
		while(!pos.IsEmpty())
		{
			if (m_intobj.GetNextPosition(pos,container))
			{
				if (container)
				{
					if (sRemoveID.Find(container->GetType())==-1)
						container->GetModifyBuf(buf,in,sRemoveID);
					else
						THDebug(_T("Skip RemoveID %s,%s"),container->GetType(),sRemoveID);
				}
			}
		}
		if (m_intobj.GetCount()==0)
		{
			if (m_nSize!=0)
			{
				char *b=new char[(size_t)m_nSize];
				in.Seek(m_nFileOff,SEEK_SET);
				buf->AddBuf(b,in.Read(b,(UINT)m_nSize));
				delete [] b;
			}
		}
		UINT total;
		char *orgbuf=(char *)buf->GetBuf(&total);
		if (orgbuf)
		{
			THMp4Box *box=(THMp4Box *)(orgbuf+stoff);
			box->nSize=total-stoff;
			box->hton();
		}
		buf->ReleaseGetBufLock();
	}
	THString GetType()
	{
		return THSimpleXml::GetParam(m_sParseData,_T("BoxId"));
	}
	//本容器保存的
	THMemBuf* GetIntBuf(){return &m_buf;}
	U64 GetRemainDataOff(){return m_nFileOff;}
	I64 GetRemainDataSize(){return m_nSize;}
	UINT GetIntObjCount(){return m_intobj.GetCount();}
private:
	char m_nid[5];///<本容器的类型
	I64 m_nSize;///<本容器未解释大小
	U64 m_nFileOff;///<本容器未解释在原文件的偏移
	THMemBuf m_buf;///<本容器已解释原始数据
	THString m_sParseData;///<容器数据解释结果
	THList<THMp4BoxContainer *> m_intobj;///<容器的子容器
	/*
	容器类型     子容器类型    子容器类型
	 |  容器信息  | 子容器信息 | 子容器信息
	 |    |       |    |       |   |
	xxxxyyyyyyyy[xxxxyyyyyy][xxxxyyyyyy]...
	|           |                        ||
	|           +---------m_nSize--------+|
	|           |                         |
	|      m_nFileOff                     |
	|           |                         |
	+--m_buf----+                         |
	+-----------------容器长度------------+
	*/

	BOOL _ReadBufData(void **p,UINT &nSize,void *pRead,UINT nReadLen)
	{
		if (nReadLen>nSize)
		{
			ASSERT(FALSE);
			return FALSE;
		}
		memcpy(pRead,*p,nReadLen);
		*p=(void *)(((char *)(*p))+nReadLen);
		nSize-=nReadLen;
		return TRUE;
	}

#define ReadU8(a)	if (!_ReadBufData(&buf,nSize,&buf8,sizeof(U8))) return FALSE;\
					m_sParseData+=THSimpleXml::MakeParam(a,THFormat(_T("%u"),buf8))

#define ReadU16(a)	if (!_ReadBufData(&buf,nSize,&buf16,sizeof(U16))) return FALSE;\
					buf16=ntohs(buf16);\
					m_sParseData+=THSimpleXml::MakeParam(a,THFormat(_T("%u"),buf16))

#define ReadU32(a)	if (!_ReadBufData(&buf,nSize,&buf32,sizeof(U32))) return FALSE;\
					buf32=ntohl(buf32);\
					m_sParseData+=THSimpleXml::MakeParam(a,THFormat(_T("%u"),buf32))

#define ReadU64(a)	if (!_ReadBufData(&buf,nSize,&buf64,sizeof(U64))) return FALSE;\
					buf64=ntoh64(buf64);\
					m_sParseData+=THSimpleXml::MakeParam(a,THFormat(_T("%I64u"),buf64))

	BOOL _IsNeedGetAllBuf()
	{
		if (CHECKBOXTYPE("ftyp")) return TRUE;
		if (CHECKBOXTYPE("mvhd")) return TRUE;//moov->
		if (CHECKBOXTYPE("tkhd")) return TRUE;//moov->trak->
		if (CHECKBOXTYPE("mdhd")) return TRUE;//moov->trak->mdia->
		if (CHECKBOXTYPE("hdlr")) return TRUE;//moov->trak->mdia->
		if (CHECKBOXTYPE("vmhd")) return TRUE;//moov->trak->mdia->minf->
		if (CHECKBOXTYPE("smhd")) return TRUE;//moov->trak->mdia->minf->
		if (CHECKBOXTYPE("hmhd")) return TRUE;//moov->trak->mdia->minf->
		if (CHECKBOXTYPE("nmhd")) return TRUE;//moov->trak->mdia->minf->
		if (CHECKBOXTYPE("stts")) return TRUE;//moov->trak->mdia->minf->stbl->
		if (CHECKBOXTYPE("ctts")) return TRUE;//moov->trak->mdia->minf->stbl->
		if (CHECKBOXTYPE("stsz")) return TRUE;//moov->trak->mdia->minf->stbl->
		if (CHECKBOXTYPE("stz2")) return TRUE;//moov->trak->mdia->minf->stbl->
		if (CHECKBOXTYPE("stsc")) return TRUE;//moov->trak->mdia->minf->stbl->
		if (CHECKBOXTYPE("stco")) return TRUE;//moov->trak->mdia->minf->stbl->
		if (CHECKBOXTYPE("co64")) return TRUE;//moov->trak->mdia->minf->stbl->
		if (CHECKBOXTYPE("stss")) return TRUE;//moov->trak->mdia->minf->stbl->
		if (CHECKBOXTYPE("stsh")) return TRUE;//moov->trak->mdia->minf->stbl->
		if (CHECKBOXTYPE("stdp")) return TRUE;//moov->trak->mdia->minf->stbl->
		if (CHECKBOXTYPE("free")) return TRUE;
		return FALSE;
	}

	BOOL _ParseContent(void *buf,UINT nSize,THString sParentSettings)
	{
		if (CHECKBOXTYPE("ftyp"))
		{
			THMp4FileTypeBox *tbox=(THMp4FileTypeBox *)buf;
			//if (!CHECKBOXTYPEP(&tbox->nMajorBrand,"isom")) return FALSE;
			tbox->ntoh();
			m_sParseData+=THSimpleXml::MakeParam(_T("BoxFileType"),GetIdDisplayName(&tbox->nMajorBrand));
			m_sParseData+=THSimpleXml::MakeParam(_T("BoxFileVersion"),THFormat(_T("%u"),tbox->nMinorVersion));
			tbox->hton();//revert the org buffer
			THString idlist;
			for(UINT i=0;i<nSize/sizeof(U32)-2;i++)
			{
				if (idlist.IsEmpty())
					idlist=GetIdDisplayName(&tbox->pnCompatibleBrands+i);
				else
					idlist+=_T(",")+GetIdDisplayName(&tbox->pnCompatibleBrands+i);
			}
			m_sParseData+=THSimpleXml::MakeParam(_T("BoxContentFormatList"),idlist);
		}
		else if (CHECKBOXTYPE("mvhd"))
		{
			THMp4FullBox fbox;
			if (!_ReadBufData(&buf,nSize,&fbox,sizeof(THMp4FullBox))) return FALSE;
			m_sParseData+=THSimpleXml::MakeParam(_T("MvBoxVersion"),THFormat(_T("%u"),fbox.nVersion));
			m_sParseData+=THSimpleXml::MakeParam(_T("MvBoxFlags"),THFormat(_T("%u,%u,%u"),fbox.nFlags[0],fbox.nFlags[1],fbox.nFlags[2]));
			U16 buf16;
			U32 buf32;
			U64 buf64;
			if (fbox.nVersion==0)
			{
				ReadU32(_T("MvCreateTime"));
				ReadU32(_T("MvModifyTime"));
				ReadU32(_T("MvTimeScale"));
				ReadU32(_T("MvDuration"));
			}
			else if (fbox.nVersion==1)
			{
				ReadU64(_T("MvCreateTime"));
				ReadU64(_T("MvModifyTime"));
				ReadU32(_T("MvTimeScale"));
				ReadU64(_T("MvDuration"));
			}
			ReadU32(_T("MvRate"));
			ReadU16(_T("MvVolume"));
			//Reserved
			ReadU16(_T("MvReserved"));
			ReadU32(_T("MvReserved2"));
			ReadU32(_T("MvReserved3"));
			//Unqi Matrix
			ReadU32(_T("MvMatrix0"));
			ReadU32(_T("MvMatrix1"));
			ReadU32(_T("MvMatrix2"));
			ReadU32(_T("MvMatrix3"));
			ReadU32(_T("MvMatrix4"));
			ReadU32(_T("MvMatrix5"));
			ReadU32(_T("MvMatrix6"));
			ReadU32(_T("MvMatrix7"));
			ReadU32(_T("MvMatrix8"));
			//PreDefine
			ReadU32(_T("MvPreDefine0"));
			ReadU32(_T("MvPreDefine1"));
			ReadU32(_T("MvPreDefine2"));
			ReadU32(_T("MvPreDefine3"));
			ReadU32(_T("MvPreDefine4"));
			ReadU32(_T("MvPreDefine5"));

			ReadU32(_T("MvNextTrack"));
		}
		else if (CHECKBOXTYPE("tkhd"))
		{
			THMp4FullBox fbox;
			if (!_ReadBufData(&buf,nSize,&fbox,sizeof(THMp4FullBox))) return FALSE;
			m_sParseData+=THSimpleXml::MakeParam(_T("TkBoxVersion"),THFormat(_T("%u"),fbox.nVersion));
			m_sParseData+=THSimpleXml::MakeParam(_T("TkBoxFlags"),THFormat(_T("%u,%u,%u"),fbox.nFlags[0],fbox.nFlags[1],fbox.nFlags[2]));
			U16 buf16;
			U32 buf32;
			U64 buf64;
			if (fbox.nVersion==0)
			{
				ReadU32(_T("TkCreateTime"));
				ReadU32(_T("TkModifyTime"));
				ReadU32(_T("TkTrackId"));
				ReadU32(_T("TkReserved"));
				ReadU32(_T("TkDuration"));
			}
			else if (fbox.nVersion==1)
			{
				ReadU64(_T("TkCreateTime"));
				ReadU64(_T("TkModifyTime"));
				ReadU32(_T("TkTrackId"));
				ReadU32(_T("TkReserved"));
				ReadU64(_T("TkDuration"));
			}
			//Reserved
			ReadU32(_T("TkReserved1"));
			ReadU32(_T("TkReserved2"));

			ReadU16(_T("TkLayer"));
			ReadU16(_T("TkAlternateGroup"));
			ReadU16(_T("TkVolume"));
			ReadU16(_T("TkReserved"));
			//Unqi Matrix
			ReadU32(_T("TkMatrix0"));
			ReadU32(_T("TkMatrix1"));
			ReadU32(_T("TkMatrix2"));
			ReadU32(_T("TkMatrix3"));
			ReadU32(_T("TkMatrix4"));
			ReadU32(_T("TkMatrix5"));
			ReadU32(_T("TkMatrix6"));
			ReadU32(_T("TkMatrix7"));
			ReadU32(_T("TkMatrix8"));
			ReadU16(_T("TkWidth"));
			ReadU16(_T("TkWidthH"));
			ReadU16(_T("TkHeight"));
			ReadU16(_T("TkHeightH"));
		}
		else if (CHECKBOXTYPE("mdhd"))
		{
			THMp4FullBox fbox;
			if (!_ReadBufData(&buf,nSize,&fbox,sizeof(THMp4FullBox))) return FALSE;
			m_sParseData+=THSimpleXml::MakeParam(_T("MdBoxVersion"),THFormat(_T("%u"),fbox.nVersion));
			m_sParseData+=THSimpleXml::MakeParam(_T("MdBoxFlags"),THFormat(_T("%u,%u,%u"),fbox.nFlags[0],fbox.nFlags[1],fbox.nFlags[2]));
			U16 buf16;
			U32 buf32;
			U64 buf64;
			if (fbox.nVersion==0)
			{
				ReadU32(_T("MdCreateTime"));
				ReadU32(_T("MdModifyTime"));
				ReadU32(_T("MdTimeScale"));
				ReadU32(_T("MdDuration"));
			}
			else if (fbox.nVersion==1)
			{
				ReadU64(_T("MdCreateTime"));
				ReadU64(_T("MdModifyTime"));
				ReadU32(_T("MdTimeScale"));
				ReadU64(_T("MdDuration"));
			}
			ReadU16(_T("MdLanguage"));
			ReadU16(_T("MdPreDefine"));
		}
		else if (CHECKBOXTYPE("hdlr"))
		{
			THMp4FullBox fbox;
			if (!_ReadBufData(&buf,nSize,&fbox,sizeof(THMp4FullBox))) return FALSE;
			m_sParseData+=THSimpleXml::MakeParam(_T("HdBoxVersion"),THFormat(_T("%u"),fbox.nVersion));
			m_sParseData+=THSimpleXml::MakeParam(_T("HdBoxFlags"),THFormat(_T("%u,%u,%u"),fbox.nFlags[0],fbox.nFlags[1],fbox.nFlags[2]));
			U32 buf32;
			ReadU32(_T("HdPreDefine"));
			if (!_ReadBufData(&buf,nSize,&buf32,sizeof(U32))) return FALSE;
			m_sParseData+=THSimpleXml::MakeParam(_T("HdHandlerType"),GetIdDisplayName(&buf32));
			ReadU32(_T("HdReserved1"));
			ReadU32(_T("HdReserved2"));
			ReadU32(_T("HdReserved3"));
			m_sParseData+=THSimpleXml::MakeParam(_T("HdName"),THCharset::u82t((const char *)buf));
		}
		else if (CHECKBOXTYPE("vmhd"))
		{
			THMp4FullBox fbox;
			if (!_ReadBufData(&buf,nSize,&fbox,sizeof(THMp4FullBox))) return FALSE;
			m_sParseData+=THSimpleXml::MakeParam(_T("VmBoxVersion"),THFormat(_T("%u"),fbox.nVersion));
			m_sParseData+=THSimpleXml::MakeParam(_T("VmBoxFlags"),THFormat(_T("%u,%u,%u"),fbox.nFlags[0],fbox.nFlags[1],fbox.nFlags[2]));
			U16 buf16;
			ReadU16(_T("VmGraphicsMode"));
			ReadU16(_T("VmOpcolor1"));
			ReadU16(_T("VmOpcolor2"));
			ReadU16(_T("VmOpcolor3"));
		}
		else if (CHECKBOXTYPE("smhd"))
		{
			THMp4FullBox fbox;
			if (!_ReadBufData(&buf,nSize,&fbox,sizeof(THMp4FullBox))) return FALSE;
			m_sParseData+=THSimpleXml::MakeParam(_T("SmBoxVersion"),THFormat(_T("%u"),fbox.nVersion));
			m_sParseData+=THSimpleXml::MakeParam(_T("SmBoxFlags"),THFormat(_T("%u,%u,%u"),fbox.nFlags[0],fbox.nFlags[1],fbox.nFlags[2]));
			U16 buf16;
			ReadU16(_T("SmBalance"));
			ReadU16(_T("SmReserved"));
		}
		else if (CHECKBOXTYPE("hmhd"))
		{
			THMp4FullBox fbox;
			if (!_ReadBufData(&buf,nSize,&fbox,sizeof(THMp4FullBox))) return FALSE;
			m_sParseData+=THSimpleXml::MakeParam(_T("HmBoxVersion"),THFormat(_T("%u"),fbox.nVersion));
			m_sParseData+=THSimpleXml::MakeParam(_T("HmBoxFlags"),THFormat(_T("%u,%u,%u"),fbox.nFlags[0],fbox.nFlags[1],fbox.nFlags[2]));
			U16 buf16;
			U32 buf32;
			ReadU16(_T("HmMaxPDUsize"));
			ReadU16(_T("HmAvgPDUsize"));
			ReadU32(_T("HmMaxBitrate"));
			ReadU32(_T("HmAvgBitrate"));
			ReadU32(_T("HmReserved"));
		}
		else if (CHECKBOXTYPE("nmhd"))
		{
			THMp4FullBox fbox;
			if (!_ReadBufData(&buf,nSize,&fbox,sizeof(THMp4FullBox))) return FALSE;
			m_sParseData+=THSimpleXml::MakeParam(_T("NmBoxVersion"),THFormat(_T("%u"),fbox.nVersion));
			m_sParseData+=THSimpleXml::MakeParam(_T("NmBoxFlags"),THFormat(_T("%u,%u,%u"),fbox.nFlags[0],fbox.nFlags[1],fbox.nFlags[2]));
			//null media header box
		}
		else if (CHECKBOXTYPE("stts"))
		{
			THMp4FullBox fbox;
			if (!_ReadBufData(&buf,nSize,&fbox,sizeof(THMp4FullBox))) return FALSE;
			m_sParseData+=THSimpleXml::MakeParam(_T("StBoxVersion"),THFormat(_T("%u"),fbox.nVersion));
			m_sParseData+=THSimpleXml::MakeParam(_T("StBoxFlags"),THFormat(_T("%u,%u,%u"),fbox.nFlags[0],fbox.nFlags[1],fbox.nFlags[2]));
			U32 buf32,cnt;
			if (!_ReadBufData(&buf,nSize,&cnt,sizeof(U32))) return FALSE;
			cnt=ntohl(cnt);
			THString ret;
			for(UINT i=0;i<cnt;i++)
			{
				if (!_ReadBufData(&buf,nSize,&buf32,sizeof(U32))) return FALSE;
				buf32=ntohl(buf32);
				ret.AppendFormat(_T("%u:"),buf32);
				if (!_ReadBufData(&buf,nSize,&buf32,sizeof(U32))) return FALSE;
				buf32=ntohl(buf32);
				ret.AppendFormat(_T("%u,"),buf32);
			}
			m_sParseData+=THSimpleXml::MakeParam(_T("StSampleCountAndDelta"),ret);
		}
		else if (CHECKBOXTYPE("ctts"))
		{
			THMp4FullBox fbox;
			if (!_ReadBufData(&buf,nSize,&fbox,sizeof(THMp4FullBox))) return FALSE;
			m_sParseData+=THSimpleXml::MakeParam(_T("CtBoxVersion"),THFormat(_T("%u"),fbox.nVersion));
			m_sParseData+=THSimpleXml::MakeParam(_T("CtBoxFlags"),THFormat(_T("%u,%u,%u"),fbox.nFlags[0],fbox.nFlags[1],fbox.nFlags[2]));
			U32 buf32,cnt;
			if (!_ReadBufData(&buf,nSize,&cnt,sizeof(U32))) return FALSE;
			cnt=ntohl(cnt);
			THString ret;
			for(UINT i=0;i<cnt;i++)
			{
				if (!_ReadBufData(&buf,nSize,&buf32,sizeof(U32))) return FALSE;
				buf32=ntohl(buf32);
				ret.AppendFormat(_T("%u:"),buf32);
				if (!_ReadBufData(&buf,nSize,&buf32,sizeof(U32))) return FALSE;
				buf32=ntohl(buf32);
				ret.AppendFormat(_T("%u,"),buf32);
			}
			m_sParseData+=THSimpleXml::MakeParam(_T("CtSampleCountAndOffest"),ret);
		}
		else if (CHECKBOXTYPE("stsz"))
		{
			THMp4FullBox fbox;
			if (!_ReadBufData(&buf,nSize,&fbox,sizeof(THMp4FullBox))) return FALSE;
			m_sParseData+=THSimpleXml::MakeParam(_T("SzBoxVersion"),THFormat(_T("%u"),fbox.nVersion));
			m_sParseData+=THSimpleXml::MakeParam(_T("SzBoxFlags"),THFormat(_T("%u,%u,%u"),fbox.nFlags[0],fbox.nFlags[1],fbox.nFlags[2]));
			U32 buf32,cnt,size;
			if (!_ReadBufData(&buf,nSize,&size,sizeof(U32))) return FALSE;
			if (!_ReadBufData(&buf,nSize,&cnt,sizeof(U32))) return FALSE;
			size=ntohl(size);
			cnt=ntohl(cnt);
			m_sParseData+=THSimpleXml::MakeParam(_T("SzSampleSize"),THFormat(_T("%u"),size));
			m_sParseData+=THSimpleXml::MakeParam(_T("SzSampleCount"),THFormat(_T("%u"),cnt));
			if (size==0)
			{
				THString ret;
				for(UINT i=0;i<cnt;i++)
				{
					if (!_ReadBufData(&buf,nSize,&buf32,sizeof(U32))) return FALSE;
					buf32=ntohl(buf32);
					ret.AppendFormat(_T("%u,"),buf32);
				}
				m_sParseData+=THSimpleXml::MakeParam(_T("SzSampleEntry"),ret);
			}
		}
		else if (CHECKBOXTYPE("stz2"))
		{
			THMp4FullBox fbox;
			if (!_ReadBufData(&buf,nSize,&fbox,sizeof(THMp4FullBox))) return FALSE;
			m_sParseData+=THSimpleXml::MakeParam(_T("S2BoxVersion"),THFormat(_T("%u"),fbox.nVersion));
			m_sParseData+=THSimpleXml::MakeParam(_T("S2BoxFlags"),THFormat(_T("%u,%u,%u"),fbox.nFlags[0],fbox.nFlags[1],fbox.nFlags[2]));
			U8 buf8;
			ReadU8(_T("S2Reserved0"));
			ReadU8(_T("S2Reserved1"));
			ReadU8(_T("S2Reserved2"));
			U8 fieldsize;
			if (!_ReadBufData(&buf,nSize,&fieldsize,sizeof(U8))) return FALSE;
			m_sParseData+=THSimpleXml::MakeParam(_T("S2FieldSize"),THFormat(_T("%u"),fieldsize));
			U32 buf32,cnt;
			if (!_ReadBufData(&buf,nSize,&cnt,sizeof(U32))) return FALSE;
			cnt=ntohl(cnt);
			m_sParseData+=THSimpleXml::MakeParam(_T("S2SampleCount"),THFormat(_T("%u"),cnt));
			//fieldsize is 4 bits 8 bits or 16 bits
			if (fieldsize!=4 || fieldsize!=8 || fieldsize!=16) return FALSE;
			if (fieldsize==4)
				cnt=(cnt+1)/2;
			else
				cnt=cnt*fieldsize/8;
			THString ret;
			for(UINT i=0;i<cnt;i++)
			{
				if (!_ReadBufData(&buf,nSize,&buf32,sizeof(U32))) return FALSE;
				ret.AppendFormat(_T("{%u},"),buf32);
			}
			m_sParseData+=THSimpleXml::MakeParam(_T("S2SampleEntry"),ret);
		}
		else if (CHECKBOXTYPE("stsc"))
		{
			THMp4FullBox fbox;
			if (!_ReadBufData(&buf,nSize,&fbox,sizeof(THMp4FullBox))) return FALSE;
			m_sParseData+=THSimpleXml::MakeParam(_T("ScBoxVersion"),THFormat(_T("%u"),fbox.nVersion));
			m_sParseData+=THSimpleXml::MakeParam(_T("ScBoxFlags"),THFormat(_T("%u,%u,%u"),fbox.nFlags[0],fbox.nFlags[1],fbox.nFlags[2]));
			U32 buf32,cnt;
			if (!_ReadBufData(&buf,nSize,&cnt,sizeof(U32))) return FALSE;
			cnt=ntohl(cnt);
			m_sParseData+=THSimpleXml::MakeParam(_T("ScSampleCount"),THFormat(_T("%u"),cnt));
			THString ret;
			for(UINT i=0;i<cnt;i++)
			{
				if (!_ReadBufData(&buf,nSize,&buf32,sizeof(U32))) return FALSE;
				buf32=ntohl(buf32);
				ret.AppendFormat(_T("%u:"),buf32);
				if (!_ReadBufData(&buf,nSize,&buf32,sizeof(U32))) return FALSE;
				buf32=ntohl(buf32);
				ret.AppendFormat(_T("%u:"),buf32);
				if (!_ReadBufData(&buf,nSize,&buf32,sizeof(U32))) return FALSE;
				buf32=ntohl(buf32);
				ret.AppendFormat(_T("%u,"),buf32);
			}
			m_sParseData+=THSimpleXml::MakeParam(_T("ScSampleEntryFirstAndPerChunkAndDescIndex"),ret);
		}
		else if (CHECKBOXTYPE("stco"))
		{
			THMp4FullBox fbox;
			if (!_ReadBufData(&buf,nSize,&fbox,sizeof(THMp4FullBox))) return FALSE;
			m_sParseData+=THSimpleXml::MakeParam(_T("CoBoxVersion"),THFormat(_T("%u"),fbox.nVersion));
			m_sParseData+=THSimpleXml::MakeParam(_T("CoBoxFlags"),THFormat(_T("%u,%u,%u"),fbox.nFlags[0],fbox.nFlags[1],fbox.nFlags[2]));
			U32 buf32,cnt;
			if (!_ReadBufData(&buf,nSize,&cnt,sizeof(U32))) return FALSE;
			cnt=ntohl(cnt);
			m_sParseData+=THSimpleXml::MakeParam(_T("CoOffestCount"),THFormat(_T("%u"),cnt));
			THString ret;
			for(UINT i=0;i<cnt;i++)
			{
				if (!_ReadBufData(&buf,nSize,&buf32,sizeof(U32))) return FALSE;
				buf32=ntohl(buf32);
				ret.AppendFormat(_T("%u,"),buf32);
			}
			m_sParseData+=THSimpleXml::MakeParam(_T("CoOffest"),ret);
		}
		else if (CHECKBOXTYPE("co64"))
		{
			THMp4FullBox fbox;
			if (!_ReadBufData(&buf,nSize,&fbox,sizeof(THMp4FullBox))) return FALSE;
			m_sParseData+=THSimpleXml::MakeParam(_T("Co64BoxVersion"),THFormat(_T("%u"),fbox.nVersion));
			m_sParseData+=THSimpleXml::MakeParam(_T("Co64BoxFlags"),THFormat(_T("%u,%u,%u"),fbox.nFlags[0],fbox.nFlags[1],fbox.nFlags[2]));
			U32 cnt;
			U64 buf64;
			if (!_ReadBufData(&buf,nSize,&cnt,sizeof(U32))) return FALSE;
			cnt=ntohl(cnt);
			m_sParseData+=THSimpleXml::MakeParam(_T("Co64OffestCount"),THFormat(_T("%u"),cnt));
			THString ret;
			for(UINT i=0;i<cnt;i++)
			{
				if (!_ReadBufData(&buf,nSize,&buf64,sizeof(U64))) return FALSE;
				buf64=ntoh64(buf64);
				ret.AppendFormat(_T("%I64u,"),buf64);
			}
			m_sParseData+=THSimpleXml::MakeParam(_T("Co64Offest"),ret);
		}
		else if (CHECKBOXTYPE("stss"))
		{
			THMp4FullBox fbox;
			if (!_ReadBufData(&buf,nSize,&fbox,sizeof(THMp4FullBox))) return FALSE;
			m_sParseData+=THSimpleXml::MakeParam(_T("SsBoxVersion"),THFormat(_T("%u"),fbox.nVersion));
			m_sParseData+=THSimpleXml::MakeParam(_T("SsBoxFlags"),THFormat(_T("%u,%u,%u"),fbox.nFlags[0],fbox.nFlags[1],fbox.nFlags[2]));
			U32 buf32,cnt;
			if (!_ReadBufData(&buf,nSize,&cnt,sizeof(U32))) return FALSE;
			cnt=ntohl(cnt);
			m_sParseData+=THSimpleXml::MakeParam(_T("SsNumberCount"),THFormat(_T("%u"),cnt));
			THString ret;
			for(UINT i=0;i<cnt;i++)
			{
				if (!_ReadBufData(&buf,nSize,&buf32,sizeof(U32))) return FALSE;
				buf32=ntohl(buf32);
				ret.AppendFormat(_T("%u,"),buf32);
			}
			m_sParseData+=THSimpleXml::MakeParam(_T("SsNumber"),ret);
		}
		else if (CHECKBOXTYPE("stsh"))
		{
			THMp4FullBox fbox;
			if (!_ReadBufData(&buf,nSize,&fbox,sizeof(THMp4FullBox))) return FALSE;
			m_sParseData+=THSimpleXml::MakeParam(_T("ShBoxVersion"),THFormat(_T("%u"),fbox.nVersion));
			m_sParseData+=THSimpleXml::MakeParam(_T("ShBoxFlags"),THFormat(_T("%u,%u,%u"),fbox.nFlags[0],fbox.nFlags[1],fbox.nFlags[2]));
			U32 buf32,cnt;
			if (!_ReadBufData(&buf,nSize,&cnt,sizeof(U32))) return FALSE;
			cnt=ntohl(cnt);
			m_sParseData+=THSimpleXml::MakeParam(_T("ShNumberCount"),THFormat(_T("%u"),cnt));
			THString ret;
			for(UINT i=0;i<cnt;i++)
			{
				if (!_ReadBufData(&buf,nSize,&buf32,sizeof(U32))) return FALSE;
				buf32=ntohl(buf32);
				ret.AppendFormat(_T("%u:"),buf32);
				if (!_ReadBufData(&buf,nSize,&buf32,sizeof(U32))) return FALSE;
				buf32=ntohl(buf32);
				ret.AppendFormat(_T("%u,"),buf32);
			}
			m_sParseData+=THSimpleXml::MakeParam(_T("ShShadowAndSyncSample"),ret);
		}
		else if (CHECKBOXTYPE("stdp"))
		{
			//fixme , need stsz's sample_count
			/*THMp4FullBox fbox;
			if (!_ReadBufData(&buf,nSize,&fbox,sizeof(THMp4FullBox))) return FALSE;
			m_sParseData+=THSimpleXml::MakeParam(_T("DpBoxVersion"),THFormat(_T("%u"),fbox.nVersion));
			m_sParseData+=THSimpleXml::MakeParam(_T("DpBoxFlags"),THFormat(_T("%u,%u,%u"),fbox.nFlags[0],fbox.nFlags[1],fbox.nFlags[2]));
			U32 buf32,cnt;
			if (!_ReadBufData(&buf,nSize,&cnt,sizeof(U32))) return FALSE;
			cnt=ntohl(cnt);
			m_sParseData+=THSimpleXml::MakeParam(_T("DpNumberCount"),THFormat(_T("%u"),cnt));
			for(int i=0;i<cnt;i++)
			{
				ReadU16(THFormat(_T("DpPriority%d"),i));
			}*/
		}
		else if (CHECKBOXTYPE("free"))
		{
			m_sParseData+=THSimpleXml::MakeParam(_T("Free"),THCharset::u82t((const char *)buf));
		}
		return TRUE;
	}

	BOOL _IsNeedParse()
	{
		if (CHECKBOXTYPE("moov")) return TRUE;
		if (CHECKBOXTYPE("moof")) return TRUE;
		if (CHECKBOXTYPE("mfra")) return TRUE;
		if (CHECKBOXTYPE("meta")) return TRUE;
		if (CHECKBOXTYPE("trak")) return TRUE;//moov->
		if (CHECKBOXTYPE("mvex")) return TRUE;//moov->
		if (CHECKBOXTYPE("traf")) return TRUE;//moof->
		if (CHECKBOXTYPE("edts")) return TRUE;//moov->trak->
		if (CHECKBOXTYPE("mdia")) return TRUE;//moov->trak->
		if (CHECKBOXTYPE("minf")) return TRUE;//moov->trak->mdia->
		if (CHECKBOXTYPE("dinf")) return TRUE;//moov->trak->mdia->minf->
											//meta->
		if (CHECKBOXTYPE("stbl")) return TRUE;//moov->trak->mdia->minf->
		if (CHECKBOXTYPE("ipro")) return TRUE;//meta->
		if (CHECKBOXTYPE("sinf")) return TRUE;//meta->ipro->
		return FALSE;
	}

	BOOL _ParseContentFile(THFile *f,I64 nSize,THString sParentSettings)
	{
		while(nSize>0)
		{
			THMp4BoxContainer *container=new THMp4BoxContainer;
			if (!container) return NULL;
			I64 ret=container->ParseBox(f,sParentSettings+m_sParseData);
			if (ret==-1)
			{
				delete container;
				return FALSE;
			}
			nSize-=ret;
			m_intobj.AddTail(container);
		}
		return TRUE;
	}

	static BOOL _Seek(THFile *file,LONGLONG nOff,UINT nFrom)
	{
		ASSERT(file);
		UINT nOffRet=(UINT)file->Seek(nOff,nFrom);
		if (nFrom==SEEK_SET && nOffRet!=nOff) return FALSE;
		return TRUE;
	}

	static BOOL _ReadFile(THFile *file,void *buf,UINT nLen)
	{
		ASSERT(file && buf);
		if (file->Read(buf,nLen)!=nLen) return FALSE;
		return TRUE;
	}
	static BOOL _WriteFile(THFile *file,const void *buf,UINT nLen)
	{
		ASSERT(file);
		if (file->Write(buf,nLen)!=nLen) return FALSE;
		return TRUE;
	}
	static THString GetIdDisplayName(const void *buf)
	{
		const U8 *tmp=(const U8 *)buf;
		THString ret;
		for(int i=0;i<4;i++)
		{
			if ((*tmp)==0) break;
			else if ((*tmp)>0x1f && (*tmp)<='z')
				ret.AppendFormat(_T("%c"),*tmp);
			else
				ret.AppendFormat(_T("0x%02x"),*tmp);
			tmp++;
		}
		return ret;
	}

	static void FreeIntObj(void *key,void *value,void *adddata)
	{
		delete (THMp4BoxContainer *)value;
	}
};

class THMp4Parser
{
public:
	enum THMp4ParserError{
		THMp4ParserError_None=0,
		THMp4ParserError_FileError,		///<文件打不开
	};

	THMp4Parser()
	{
	}

	virtual ~THMp4Parser()
	{
	}

	static BOOL ParseFile(THString sFilePath)
	{
		THFile infile;
		if (infile.Open(sFilePath,THFile::modeRead))
		{
			BOOL bOk=FALSE;
			THMp4BoxContainer *container;
			while(1)
			{
				container=GetBox(&infile);
				if (!container) break;
				bOk=TRUE;
				THDebug(container->PrintDetail());
				delete container;
			}
			infile.Close();
			if (bOk) return THMp4ParserError_None;
		}
		return THMp4ParserError_FileError;
	}

	static vector<THMp4BoxContainer *> GetFileBoxs(THFile &file)
	{
		vector<THMp4BoxContainer *> dat;
		THMp4BoxContainer *container;
		while(1)
		{
			container=GetBox(&file);
			if (!container) break;
			THDebug(container->PrintDetail());
			dat.push_back(container);
		}
		return dat;
	}

	static vector<THMp4BoxContainer *> GetFileBoxs(THString sFilePath)
	{
		vector<THMp4BoxContainer *> dat;
		THFile infile;
		if (infile.Open(sFilePath,THFile::modeRead))
		{
			dat=GetFileBoxs(infile);
			infile.Close();
		}
		return dat;
	}

	static void FreeFileBoxs(vector<THMp4BoxContainer *> &dat)
	{
		vector<THMp4BoxContainer *>::iterator it=dat.begin();
		while(it!=dat.end())
		{
			delete (*it);
			it++;
		}
		dat.clear();
	}

	static unsigned long FindBoxOffest(THFile &file,THString tag)
	{
		file.SeekToBegin();
		THMp4BoxContainer *container;
		unsigned long off;
		THString typetag;
		while(1)
		{
			off=(unsigned long)file.GetPosition();
			container=GetBox(&file);
			if (!container) break;
			typetag=container->GetType();
			delete container;
			if (typetag==tag) break;
		}
		return off;
	}

	static BOOL RemoveKeyFrameInfo(THString sInFile,THString sOutFile,THString deletetag=_T(""))
	{
		THFile infile;
		if (!infile.Open(sInFile,THFile::modeRead)) return FALSE;
		vector<THMp4BoxContainer *> dat=GetFileBoxs(infile);
		if (dat.empty())
		{
			infile.Close();
			return FALSE;
		}
		THFile outfile;
		BOOL bRet=FALSE;
		if (outfile.Open(sOutFile,THFile::modeCreate|THFile::modeReadWrite))
		{
			THMp4BoxContainer *moovct=NULL;
			vector<THMp4BoxContainer *>::iterator it=dat.begin();
			while(it!=dat.end())
			{
				THMp4BoxContainer *container=*it;
				if (container->GetType()==_T("moov"))
				{
					moovct=container;
					break;
				}
				it++;
			}
			if (moovct)
			{
				//get inner box
				//RemoveInfo(moovct,_T("stts")); 删掉后只能播几秒
				//RemoveInfo(moovct,_T("stsc")); 删掉不能播放
				//RemoveInfo(moovct,_T("stsz")); 删掉后只能播几秒
				//RemoveInfo(moovct,_T("stco")); 删掉后只能播几秒
				RemoveInfo(moovct,_T("stss"));
				//save file
				THMemBuf buf;
				it=dat.begin();
				while(it!=dat.end())
				{
					buf.Empty();
					(*it)->GetModifyBuf(&buf,infile,deletetag);
					outfile.Write(buf.GetBuf(NULL,FALSE),buf.GetBufferedSize());
					it++;
				}
			}
			outfile.Close();
		}
		infile.Close();
		return bRet;
	}

	static BOOL SaveBoxsToFile(THString sInFile,THString sOutFile,vector<THMp4BoxContainer *> &dat,THString deletetag=_T(""))
	{
		THFile infile;
		if (!infile.Open(sInFile,THFile::modeRead)) return FALSE;
		THFile outfile;
		BOOL bRet=FALSE;
		if (outfile.Open(sOutFile,THFile::modeCreate|THFile::modeReadWrite))
		{
			unsigned long orgmdatoff=FindBoxOffest(infile,_T("mdat"));
			//calc off
			unsigned long mdatoff=0;
			unsigned long moovoff=0;
			unsigned long curoff=0;
			THMp4BoxContainer *moovct=NULL;
			vector<THMp4BoxContainer *>::iterator it=dat.begin();
			THMemBuf buf;
			while(it!=dat.end())
			{
				buf.Empty();
				THMp4BoxContainer *container=*it;
				THString type=container->GetType();
				container->GetModifyBuf(&buf,infile,deletetag);
				if (type==_T("mdat")) mdatoff=curoff;
				else if (type==_T("moov"))
				{
					moovct=container;
					moovoff=curoff;
				}
				curoff+=buf.GetBufferedSize();
				it++;
			}
			//recalc stco
			if (orgmdatoff!=0 && moovoff!=0 && mdatoff!=0)
			{
				THMp4BoxContainer *stco=moovct->FindIntObjByType(_T("stco"));
				if (stco)
				{
					THMemBuf *intbuf=stco->GetIntBuf();
					int addoff=mdatoff-orgmdatoff;
					unsigned char *stcobuf=(unsigned char *)intbuf->GetBuf(NULL,FALSE);
					unsigned int size=ntohl(*(unsigned int *)(stcobuf+sizeof(THMp4Box)+sizeof(THMp4FullBox)));
					unsigned int *nstco=(unsigned int *)(stcobuf+sizeof(THMp4Box)+sizeof(THMp4FullBox)+sizeof(unsigned int));
					for(unsigned int i=0;i<size;i++)
					{
						*nstco=htonl(((int)ntohl(*nstco))+addoff);
						nstco++;
					}
					it=dat.begin();
					while(it!=dat.end())
					{
						buf.Empty();
						(*it)->GetModifyBuf(&buf,infile,deletetag);
						outfile.Write(buf.GetBuf(NULL,FALSE),buf.GetBufferedSize());
						it++;
					}
					bRet=TRUE;
				}
			}
			outfile.Close();
		}
		infile.Close();
		return bRet;
	}

	static BOOL TakeMoovBeforeMdat(vector<THMp4BoxContainer *> &dat)
	{
		THMp4BoxContainer *moov=NULL;
		THMp4BoxContainer *mdat=NULL;
		size_t nmoov=-1;
		size_t nmdat=-1;
		for(size_t i=0;i<dat.size();i++)
		{
			if (dat[i]->GetType()==_T("moov")) {nmoov=i;moov=dat[i];}
			if (dat[i]->GetType()==_T("mdat")) {nmdat=i;mdat=dat[i];}
		}
		if (nmoov!=-1 && nmdat!=-1)
		{
			dat[nmoov]=mdat;
			dat[nmdat]=moov;
			return TRUE;
		}
		return FALSE;
	}

	static BOOL TakeMoovBeforeMdat(THString sInFile,THString sOutFile,THString deletetag=_T(""))
	{
		vector<THMp4BoxContainer *> dat=GetFileBoxs(_T("d:\\onlyvideo.mp4"));
		if (dat.empty()) return FALSE;
		if (!TakeMoovBeforeMdat(dat))
		{
			FreeFileBoxs(dat);
			return FALSE;
		}
		BOOL bRet=SaveBoxsToFile(sInFile,sOutFile,dat,deletetag);
		FreeFileBoxs(dat);
		return bRet;
	}

	static BOOL ReplaceFile(THString sInFile,THString sOutFile,THString deletetag=_T(""))//e.g.: udta,dinf
	{
		vector<THMp4BoxContainer *> dat;
		dat=GetFileBoxs(sInFile);
		if (dat.empty()) return THMp4ParserError_FileError;
		BOOL bRet=SaveBoxsToFile(sInFile,sOutFile,dat,deletetag);
		FreeFileBoxs(dat);
		return bRet;
	}
protected:
	static void RemoveInfo(THMp4BoxContainer *moov,THString tag)
	{
		THMp4BoxContainer *obj=moov->FindIntObjByType(tag);
		if (obj)
		{
			THMemBuf *buf=obj->GetIntBuf();
			unsigned char *objheader=(unsigned char *)buf->GetBuf(NULL,FALSE);
			int addoff=0;
			if (tag==_T("stsz")) addoff=sizeof(unsigned int);
			*(unsigned int *)(objheader+sizeof(THMp4Box)+sizeof(THMp4FullBox)+addoff)=htonl(1);
			addoff=sizeof(THMp4Box)+sizeof(THMp4FullBox)+addoff+sizeof(unsigned int)*2;
			*(unsigned int *)(objheader)=htonl(addoff-sizeof(unsigned int));
			buf->SetSize(addoff);
		}
	}

	static THMp4BoxContainer* GetBox(THFile *file)
	{
		THMp4BoxContainer *container=new THMp4BoxContainer;
		if (!container) return NULL;
		if (container->ParseBox(file)!=-1)
			return container;
		delete container;
		return NULL;
	}
	static THMp4ParserError _Seek(THFile *file,UINT nOff,UINT nFrom)
	{
		ASSERT(file);
		UINT nOffRet=(UINT)file->Seek(nOff,nFrom);
		if (nFrom==SEEK_SET && nOffRet!=nOff) return THMp4ParserError_FileError;
		return THMp4ParserError_None;
	}

	static THMp4ParserError _ReadFile(THFile *file,void *buf,UINT nLen)
	{
		ASSERT(file && buf);
		if (file->Read(buf,nLen)!=nLen) return THMp4ParserError_FileError;
		return THMp4ParserError_None;
	}
	static THMp4ParserError _WriteFile(THFile *file,const void *buf,UINT nLen)
	{
		ASSERT(file);
		try{
			file->Write(buf,nLen);
		}
		catch(...)
		{
			return THMp4ParserError_FileError;
		}
		return THMp4ParserError_None;
	}
};
