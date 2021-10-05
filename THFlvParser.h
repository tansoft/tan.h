#pragma once

#include <THSystem.h>
#include <THFile.h>
#include <THFlashBase.h>
#include <THSwfParser.h>

//使用swfparser类解释及构造onMeta内容
#define USE_SWFPARSER

#ifdef USE_SWFPARSER
	#define PRINT_FLVINFO
#endif

#define THFLVCNTTYPE_VIDEO		0x1
#define THFLVCNTTYPE_AUDIO		0x4

//官方文档没提及有该位
//http://www.adobe.com/devnet/flv/pdf/video_file_format_spec_v9.pdf
//http://www.adobe.com/devnet/swf/pdf/swf_file_format_spec_v9.pdf
//TypeFlagsReserved UB[5] Must be 0
//TypeFlagsAudio UB[1] Audio tags are present
//TypeFlagsReserved UB[1] Must be 0
//TypeFlagsVideo UB[1] Video tags are present
#define THFLVCNTTYPE_UNKNOWN	0x8

#define THFLVCNTTYPE_SUPPORTNOW	(THFLVCNTTYPE_VIDEO|THFLVCNTTYPE_AUDIO)

#define THFLVTAGTYPE_AUDIO		0x08
#define THFLVTAGTYPE_VIDEO		0x09
#define THFLVTAGTYPE_META		0x12

#define THFLVCURRENTVERSION		0x1

#pragma pack(push,1)

typedef struct _THFlvHeader{
	U8 sHeader[3];	//always FLV
	U8 nVersion;	//always THFLVCURRENTVERSION now
	U8 nCntType;	//THFLVCNTTYPE_XXX
	U32 nOffest;	//Offest of Stream Start
}THFlvHeader;

typedef struct _THFlvStream{
	U32 nPreviousTagSize;	//Total size of previous tag, or 0 for first tag
}THFlvStream;

typedef struct _THFlvTag{
	U8 nType;			//THFLVTAGTYPE_XXX
	U8 nBodyLength[3];	//TotalLength - sizeof(THFlvTag)
	struct _THFlvTagTimeStamp{
		U8 nTimeStamp[3];
		U8 nTimeStampHigh;
	}TimeStamp;
	U8 nStreamId[3];
}THFlvTag;

//THFlvMeta
//Script Data Object Buffer(THSwfSdoManager)

#pragma pack(pop)

#define THFLVANALYSEMODE_AUDIO 0x1
#define THFLVANALYSEMODE_VIDEO 0x2
#define THFLVANALYSEMODE_ALL THFLVANALYSEMODE_AUDIO|THFLVANALYSEMODE_VIDEO

typedef void (*THFLVANALYSECB)(const THFlvTag &tag,unsigned char *data,unsigned int len,unsigned int fileoff,const TCHAR *info);

/**
* @brief Flv文件处理类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-06-04 新建类
*/
/**<pre>
  使用Sample：
</pre>*/
class THFlvParser
{
public:
	THFlvParser()
	{
	}

	virtual ~THFlvParser()
	{
	}

	enum THFlvParserError{
		THFlvParserError_None=0,
		THFlvParserError_FileError,		///<文件打不开
		THFlvParserError_InvalidParam,	///<参数错误
		THFlvParserError_NotFlvFile,	///<不是Flv文件
		THFlvParserError_VerNotSupport,	///<版本不支持
		THFlvParserError_FlvStreamError,///<Flash中有流错误
		THFlvParserError_MemoryError,	///<内存不够
	};
	/**
	* @brief 判断是否为Flv文件
	* @param sSrcFile			输入文件
	* @return 成功返回0，失败返回错误码THFlvParserError_XXX
	*/
	static THFlvParserError IsFlvFile(THString sSrcFile)
	{
		if (sSrcFile.IsEmpty()) return THFlvParserError_InvalidParam;
		THFile infile;
		if (!infile.Open(sSrcFile,THFile::modeRead|THFile::modeNoInherit)) return THFlvParserError_FileError;
		THFlvParserError err;
		THFlvHeader header;
		err=_ReadFile(&infile,&header,sizeof(THFlvHeader));
		if (err!=THFlvParserError_None) return err;
		err=_IsHeaderOk(&header);
		return err;
	}
	/**
	* @brief 合并Flv文件
	* @param arSrcFileArray		输入的文件列表
	* @param sOutFile			输出文件列表
	* @return 成功返回0，失败返回错误码THFlvParserError_XXX
	*/
	static THFlvParserError CombinFlvFile(THStringArray *arSrcFileArray,THString sOutFile)
	{
		if (!arSrcFileArray) return THFlvParserError_InvalidParam;
		if (arSrcFileArray->GetSize()==0 || sOutFile.IsEmpty()) return THFlvParserError_InvalidParam;
		THFile infile;
		THFile outfile;
		if (!outfile.Open(sOutFile,THFile::modeWrite|THFile::modeCreate))
		{
			return THFlvParserError_FileError;
		}
		THFlvParserError err;
		err=_MakeFlvHeader(&outfile);
		if (err!=THFlvParserError_None) return err;
		UINT nMetaPos=(UINT)outfile.GetPosition();
		err=_MakeFlvMeta(&outfile);
		if (err!=THFlvParserError_None) return err;
		U32 ts=0;
		double duration=0.0;
		for(int i=0;i<arSrcFileArray->GetSize();i++)
		{
			if (!infile.Open(arSrcFileArray->GetAt(i),THFile::modeRead))
			{
				outfile.Close();
				return THFlvParserError_FileError;
			}
			err=_AppendFlv(&infile,&outfile,&ts,&duration);
			infile.Close();
			if (err!=THFlvParserError_None)
			{
				outfile.Close();
				return err;
			}
		}
		err=_Seek(&outfile,nMetaPos,SEEK_SET);
		if (err!=THFlvParserError_None) return err;
		//re write meta info,update duration of the file
		err=_MakeFlvMeta(&outfile,duration);
		if (err!=THFlvParserError_None) return err;
		outfile.Close();
		return THFlvParserError_None;
	}
	static THString AnalyseFlv(THString flv,int filtermode=THFLVANALYSEMODE_ALL,int maxprintbuf=180,THFLVANALYSECB cb=NULL)
	{
		THString ret;
		THFile infile;
		unsigned int fileoff=0;
		if (infile.Open(flv,THFile::modeRead))
		{
			THFlvParserError err;
			THFlvHeader header;
			err=_ReadFile(&infile,&header,sizeof(THFlvHeader));
			if (err!=THFlvParserError_None) return ret;
			err=_IsHeaderOk(&header);
			if (err!=THFlvParserError_None) return ret;
			fileoff=ntohl(header.nOffest);
			err=_Seek(&infile,fileoff,SEEK_SET);
			if (err!=THFlvParserError_None) return ret;
			THFlvStream stream;
			err=_ReadFile(&infile,&stream,sizeof(THFlvStream));
			if (err!=THFlvParserError_None) return ret;
			fileoff+=sizeof(THFlvStream);
			THFlvTag tag;
			U32 lastts[2]={0,0};
			THString tmpstr;
			while(1)
			{
				err=_ReadFile(&infile,&tag,sizeof(THFlvTag));
				if (err!=THFlvParserError_None) break;//the end
				fileoff+=sizeof(THFlvTag);
#ifdef USE_SWFPARSER
				if (tag.nType==THFLVTAGTYPE_META)
				{
					//read it
					UINT nSize=THFlashBase::_GetU24Length(tag.nBodyLength);
					if (nSize>0)
					{
						char *buf=new char[nSize];
						if (buf)
						{
							err=_ReadFile(&infile,buf,nSize);
							if (err==THFlvParserError_None)
							{
								THSwfSdoManager man;
								man.AppendSdoBuffer(buf,nSize);
								ret+=man.GetDetail()+_T("\r\n");
								fileoff+=nSize;
							}
							delete [] buf;
						}
					}
				}
				else
#endif
				{
					if (tag.nType==THFLVTAGTYPE_AUDIO || tag.nType==THFLVTAGTYPE_VIDEO)
					{
						if ((tag.nType-THFLVTAGTYPE_AUDIO+1)&filtermode)
						{
							unsigned char *headerbuf;
							U32 ts=THFlashBase::_GetU32Length((U8 *)&tag.TimeStamp);
							U32 bodylen=THFlashBase::_GetU24Length(tag.nBodyLength);
							U32 listlen=maxprintbuf;
							if (bodylen<listlen) listlen=bodylen;
							headerbuf=new unsigned char[listlen];
							if (_ReadFile(&infile,headerbuf,listlen)==THFlvParserError_None) _Seek(&infile,-(int)listlen,SEEK_CUR);
							tmpstr.Format(_T("%s ts:%-12s size:%-12s off:%-12u header buf:%s"),(tag.nType==THFLVTAGTYPE_AUDIO)?"A":"V",
									THFormat(_T("%u(-%u)"),ts,ts-lastts[tag.nType-THFLVTAGTYPE_AUDIO]),
									THFormat(_T("%u(0x%02x)"),bodylen,bodylen),
									fileoff,
									THStringConv::FormatString(headerbuf,listlen,THStringConv::FormatType_Hex,listlen,THStringConv::FormatType_None,FALSE,FALSE));
							if (cb) (cb)(tag,headerbuf,listlen,fileoff,tmpstr);
							ret+=tmpstr+_T("\r\n");
							lastts[tag.nType-THFLVTAGTYPE_AUDIO]=ts;
							delete [] headerbuf;
						}
					}
					else
						//skip it
						ret.AppendFormat(_T("skip tag type:%u size:%u\r\n"),tag.nType,THFlashBase::_GetU24Length(tag.nBodyLength));
					err=_Seek(&infile,THFlashBase::_GetU24Length(tag.nBodyLength),SEEK_CUR);
					fileoff+=THFlashBase::_GetU24Length(tag.nBodyLength);
					if (err!=THFlvParserError_None) return ret;
				}
				//read the end stream tag
				err=_ReadFile(&infile,&stream,sizeof(THFlvStream));
				if (err!=THFlvParserError_None) return ret;
				fileoff+=sizeof(THFlvStream);
				if (ntohl(stream.nPreviousTagSize)!=THFlashBase::_GetU24Length(tag.nBodyLength)+sizeof(THFlvTag))
				{
					ret.AppendFormat(_T("tag end size %u not equ to body:%u + tagsize\r\n"),ntohl(stream.nPreviousTagSize),THFlashBase::_GetU24Length(tag.nBodyLength));
				}
			}
			infile.Close();
		}
		return ret;
	}
protected:
	static THFlvParserError _AppendFlv(THFile *infile,THFile *outfile,U32 *ts,double *duration)
	{
		ASSERT(infile && outfile && ts && duration);
		double d=0.0;
		THFlvParserError err;
		THFlvHeader header;
		err=_ReadFile(infile,&header,sizeof(THFlvHeader));
		if (err!=THFlvParserError_None) return err;
		err=_IsHeaderOk(&header);
		if (err!=THFlvParserError_None) return err;
		err=_Seek(infile,ntohl(header.nOffest),SEEK_SET);
		if (err!=THFlvParserError_None) return err;
		THFlvStream stream;
		err=_ReadFile(infile,&stream,sizeof(THFlvStream));
		if (err!=THFlvParserError_None) return err;
		//try read tag
		U32 lastts=0;
		U32 tmp;
		U8 *tmpu8;
		THFlvTag tag;
		while(1)
		{
			err=_ReadFile(infile,&tag,sizeof(THFlvTag));
			if (err!=THFlvParserError_None) break;//the end
			if (tag.nType==THFLVTAGTYPE_AUDIO || tag.nType==THFLVTAGTYPE_VIDEO)
			{
				lastts=THFlashBase::_GetU24Length(tag.TimeStamp.nTimeStamp)+tag.TimeStamp.nTimeStampHigh*0x1000000;
				if ((*ts)!=0)
				{
					//update to timestamp
					tmp=htonl(lastts+(*ts));
					tmpu8=(U8 *)&tmp;
					tag.TimeStamp.nTimeStampHigh=*tmpu8;
					memcpy(tag.TimeStamp.nTimeStamp,tmpu8+1,3);
				}
				//rewrite to outfile
				err=_WriteFile(outfile,&tag,sizeof(THFlvTag));
				if (err!=THFlvParserError_None) return err;
				err=_TranData(infile,outfile,THFlashBase::_GetU24Length(tag.nBodyLength));
				if (err!=THFlvParserError_None) return err;
				//read the end stream tag
				err=_ReadFile(infile,&stream,sizeof(THFlvStream));
				if (err!=THFlvParserError_None) return err;
				if (ntohl(stream.nPreviousTagSize)!=THFlashBase::_GetU24Length(tag.nBodyLength)+sizeof(THFlvTag)) return THFlvParserError_FlvStreamError;
				err=_WriteFile(outfile,&stream,sizeof(THFlvStream));
				if (err!=THFlvParserError_None) return err;
			}
			else if (tag.nType==THFLVTAGTYPE_META)
			{
#ifdef USE_SWFPARSER
				//read it
				UINT nSize=THFlashBase::_GetU24Length(tag.nBodyLength);
				if (nSize>0)
				{
					char *buf=new char[nSize];
					if (!buf) return THFlvParserError_MemoryError;
					err=_ReadFile(infile,buf,nSize);
					if (err!=THFlvParserError_None)
					{
						delete [] buf;
						return err;
					}
					THSwfSdoManager man;
					man.AppendSdoBuffer(buf,nSize);
					delete [] buf;
					THSwfSdoManager *subman=man.GetArrayObject(_T("onMetaData"));
					if (subman)
						d=subman->GetNumberObject(_T("duration"));
#ifdef PRINT_FLVINFO
					THDebug(man.GetDetail());
#endif
				}
#else
				//skip it
				err=_Seek(infile,THFlashBase::_GetU24Length(tag.nBodyLength),SEEK_CUR);
				if (err!=THFlvParserError_None) return err;
#endif
				//read the end stream tag
				err=_ReadFile(infile,&stream,sizeof(THFlvStream));
				if (err!=THFlvParserError_None) return err;
				if (ntohl(stream.nPreviousTagSize)!=THFlashBase::_GetU24Length(tag.nBodyLength)+sizeof(THFlvTag)) return THFlvParserError_FlvStreamError;
			}
			else
			{
				ASSERT(FALSE);
			}
		}
		//add infile's timestamp
		*ts=lastts+(*ts);
		if (d<=0) d=(double)(lastts/1000.0);
		*duration=d+(*duration);
		return THFlvParserError_None;
	}

	static THFlvParserError _IsHeaderOk(THFlvHeader *header)
	{
		ASSERT(header);
		if (memcmp(header->sHeader,"FLV",3)!=0) return THFlvParserError_NotFlvFile;
		if (header->nVersion>THFLVCURRENTVERSION) return THFlvParserError_VerNotSupport;
		//not need parser header->nCntType and header->nOffest
		return THFlvParserError_None;
	}

	static THFlvParserError _MakeFlvHeader(THFile *file)
	{
		THFlvHeader header;
		memcpy(header.sHeader,"FLV",3);
		header.nVersion=THFLVCURRENTVERSION;
		header.nCntType=THFLVCNTTYPE_SUPPORTNOW;
		header.nOffest=htonl(sizeof(THFlvHeader));
		THFlvParserError err;
		err=_WriteFile(file,&header,sizeof(THFlvHeader));
		if (err!=THFlvParserError_None) return err;
		THFlvStream stream;
		stream.nPreviousTagSize=0;
		return _WriteFile(file,&stream,sizeof(THFlvStream));
	}

	static THFlvParserError _MakeFlvMeta(THFile *file,double duration=0.0)
	{
		THFlvParserError err;
		THFlvTag tag;
		THFlvStream stream;
#ifdef USE_SWFPARSER
		THSwfSdoManager man1;
		//will free in man1::~THSwfSdoManager()
		THSwfSdoManager *man=new THSwfSdoManager;
		if (!man) return THFlvParserError_MemoryError;
		man->AddNumberObject(_T("duration"),duration);
		man->AddStringObject(_T("metadatacreator"),_T("Tansoft FlvParser"));
		man1.AddArrayObject(_T("onMetaData"),man);
		UINT nRetLen;
		void *sdobuf=man1.MakeSdoBuffer(&nRetLen);
#else
		U8 sdobuf[]={0x02,0x00,0x0a,0x6f,0x6e,0x4d,0x65,0x74,0x61,0x44,0x61,0x74,0x61,0x08,0x00,0x00,0x00,0x02,0x00,0x08,0x64,0x75,0x72,0x61,0x74,0x69,0x6f,0x6e,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0f,0x6d,0x65,0x74,0x61,0x64,0x61,0x74,0x61,0x63,0x72,0x65,0x61,0x74,0x6f,0x72,0x02,0x00,0x11,0x54,0x61,0x6e,0x73,0x6f,0x66,0x74,0x20,0x46,0x6c,0x76,0x50,0x61,0x72,0x73,0x65,0x72,0x00,0x00,0x09};
		UINT nRetLen=sizeof(sdobuf);
		//update duration
		const UINT nduroff=29;
		U8 *tmpdur=(U8 *)&duration;
		for(int i=0;i<8;i++)
			sdobuf[nduroff+i]=*(tmpdur+7-i);
#endif
		//THFlashBase::_SetU24Length(tag.nStreamId,0);
		//tag.TimeStamp.nTimeStamp;
		memset(&tag,0,sizeof(THFlvTag));
		tag.nType=THFLVTAGTYPE_META;
		THFlashBase::_SetU24Length(tag.nBodyLength,nRetLen);
		//write info to outfile
		//write tag
		err=_WriteFile(file,&tag,sizeof(THFlvTag));
		if (err!=THFlvParserError_None)
		{
#ifdef USE_SWFPARSER
			if (sdobuf) man1.FreeSdoBuffer(sdobuf);
#endif
			return err;
		}
		//write info
		err=_WriteFile(file,sdobuf,nRetLen);
#ifdef USE_SWFPARSER
		if (sdobuf) man1.FreeSdoBuffer(sdobuf);
#endif
		if (err!=THFlvParserError_None) return err;
		//write end
		stream.nPreviousTagSize=nRetLen+sizeof(THFlvTag);
		stream.nPreviousTagSize=htonl(stream.nPreviousTagSize);
		err=_WriteFile(file,&stream,sizeof(THFlvStream));
		if (err!=THFlvParserError_None) return err;
		return THFlvParserError_None;
	}

	static THFlvParserError _Seek(THFile *file,int nOff,UINT nFrom)
	{
		ASSERT(file);
		UINT nOffRet=(UINT)file->Seek(nOff,nFrom);
		if (nFrom==SEEK_SET && nOffRet!=(UINT)nOff) return THFlvParserError_FileError;
		return THFlvParserError_None;
	}

	static THFlvParserError _ReadFile(THFile *file,void *buf,UINT nLen)
	{
		ASSERT(file && buf);
		if (file->Read(buf,nLen)!=nLen) return THFlvParserError_FileError;
		return THFlvParserError_None;
	}
	static THFlvParserError _WriteFile(THFile *file,const void *buf,UINT nLen)
	{
		ASSERT(file);
		if (file->Write(buf,nLen)!=nLen) return THFlvParserError_FileError;
		return THFlvParserError_None;
	}

	static THFlvParserError _TranData(THFile *in,THFile *out,UINT nLen)
	{
		ASSERT(in && out);
		THFlvParserError err;
		UINT nPart;
		char buf[4096];
		while(nLen>0)
		{
			nPart=(nLen>4096)?4096:nLen;
			err=_ReadFile(in,buf,nPart);
			if (err!=THFlvParserError_None) return err;
			err=_WriteFile(out,buf,nPart);
			if (err!=THFlvParserError_None) return err;
			nLen-=nPart;
		}
		return THFlvParserError_None;
	}
};