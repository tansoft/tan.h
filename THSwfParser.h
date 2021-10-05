#pragma once

#include <THStruct.h>
#include <THMemBuf.h>
#include <THArith.h>
#include <THMeasure.h>
#include <THBitParser.h>
#include <THMemBuf.h>

#define THSWFSDOTYPE_NUMBER			0
#define THSWFSDOTYPE_BOOLEAN		1
#define THSWFSDOTYPE_STRING			2
#define THSWFSDOTYPE_OBJECT			3
#define THSWFSDOTYPE_MOVIECLIP		4
#define THSWFSDOTYPE_NULL			5
#define THSWFSDOTYPE_UNDEFINE		6
#define THSWFSDOTYPE_REFERENCE		7
#define THSWFSDOTYPE_END			9	///<用于结束
#define THSWFSDOTYPE_ECMAARRAY		8	///<数组
#define THSWFSDOTYPE_STRICTARRAY	10	///<严格数组
#define THSWFSDOTYPE_DATE			11
#define THSWFSDOTYPE_LONGSTRING		12

class THSwfSdoManager;

//Script Data Object Buffer(sdo结构)
//U16 nKeyLength
//CHAR *pKey
//U8 nValueType
//void *pValueData

/**
* @brief Flash Script Data Object描述类(NetStream object)
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-06-04 新建类
*/
/**<pre>
  使用Sample：
</pre>*/
class THSwfSdo
{
public:
	THSwfSdo()
	{
		u8Val=0;
		u16Val=0;
		u32Val=0;
		dVal=0;
		pVal=NULL;
		pValLen=0;
		tzVal=0;
		nType=THSWFSDOTYPE_UNDEFINE;
	}

	virtual ~THSwfSdo()
	{
		//pVal new info not delete in here
	}

	BOOL SafeGetNumber(double &ret)
	{
		if (nType!=THSWFSDOTYPE_NUMBER) return FALSE;
		ret=dVal;
		return TRUE;
	}

	BOOL SafeGetBool(BOOL &ret)
	{
		if (nType!=THSWFSDOTYPE_BOOLEAN) return FALSE;
		ret=u8Val;
		return TRUE;
	}

	BOOL SafeGetString(THString &ret)
	{
		if (nType!=THSWFSDOTYPE_STRING && nType!=THSWFSDOTYPE_LONGSTRING) return FALSE;
		ret=sVal;
		return TRUE;
	}

	BOOL SafeGetObject(void *pret,UINT &len)
	{
		if (nType!=THSWFSDOTYPE_OBJECT || pVal==NULL || pValLen==0) return FALSE;
		pret=pVal;
		len=pValLen;
		return TRUE;
	}

	BOOL SafeGetMovieClip(THString &ret)
	{
		if (nType!=THSWFSDOTYPE_MOVIECLIP) return FALSE;
		ret=sVal;
		return TRUE;
	}

	BOOL IsNullValue(){return nType==THSWFSDOTYPE_NULL;}

	BOOL SafeGetReference(U16 &ret)
	{
		if (nType!=THSWFSDOTYPE_REFERENCE) return FALSE;
		ret=u16Val;
		return TRUE;
	}

	BOOL SafeGetArray(void **pret,UINT &len)
	{
		if ((nType!=THSWFSDOTYPE_ECMAARRAY && nType!=THSWFSDOTYPE_STRICTARRAY) || !pret || !(*pret) || pValLen==0) return FALSE;
		*pret=pVal;
		len=pValLen;
		return TRUE;
	}

	//local time
	BOOL SafeGetDate(THTime &ti)
	{
		if (nType!=THSWFSDOTYPE_DATE) return FALSE;
		ti=tiVal;
		return TRUE;
	}

	//单位分钟
	BOOL SafeGetDateTimeZone(LONG &tz)
	{
		if (nType!=THSWFSDOTYPE_DATE) return FALSE;
		tz=tzVal;
		return TRUE;
	}

	//Format 格式：[key] 替换键名 [type] 替换键值类型 [value] 替换键值内容 [step] 替换递归级数
	THString GetDetail(THString sFormat=_T("[step][key]:[type]:[value]\r\n"),THString sStep=_T(" "))
	{
		sFormat.Replace(_T("[key]"),sKey);
		sFormat.Replace(_T("[type]"),GetTypeName());
		sFormat.Replace(_T("[value]"),GetValueString());
		sFormat.Replace(_T("[step]"),sStep);
		return sFormat;
	}

	THString GetTypeName()
	{
		switch(nType)
		{
			case THSWFSDOTYPE_NUMBER:return _T("数字");
			case THSWFSDOTYPE_BOOLEAN:return _T("布尔值");
			case THSWFSDOTYPE_STRING:return _T("字符串");
			case THSWFSDOTYPE_OBJECT:return _T("对象");
			case THSWFSDOTYPE_MOVIECLIP:return _T("剪辑路径");
			case THSWFSDOTYPE_NULL:return _T("空");
			case THSWFSDOTYPE_UNDEFINE:return _T("未定义");
			case THSWFSDOTYPE_REFERENCE:return _T("引用");
			case THSWFSDOTYPE_END:return _T("结束");
			case THSWFSDOTYPE_ECMAARRAY:return _T("数组");
			case THSWFSDOTYPE_STRICTARRAY:return _T("严格数组");
			case THSWFSDOTYPE_DATE:return _T("日期");
			case THSWFSDOTYPE_LONGSTRING:return _T("长字符串");
		}
		return _T("未知类型");
	}

	THString GetValueString()
	{
		switch(nType)
		{
			case THSWFSDOTYPE_NUMBER:
				return THf2sc(dVal,6);
			case THSWFSDOTYPE_BOOLEAN:
				return (u8Val)?_T("TRUE"):_T("FALSE");
			case THSWFSDOTYPE_STRING:
			case THSWFSDOTYPE_MOVIECLIP:
			case THSWFSDOTYPE_LONGSTRING:
				return sVal;
			case THSWFSDOTYPE_OBJECT:
			case THSWFSDOTYPE_ECMAARRAY:
			case THSWFSDOTYPE_STRICTARRAY:
				return _T("(对象)");
			case THSWFSDOTYPE_NULL:
			case THSWFSDOTYPE_UNDEFINE:
			case THSWFSDOTYPE_END:
				return _T("");
			case THSWFSDOTYPE_REFERENCE:
				return THFormat(_T("%u"),u16Val);
			case THSWFSDOTYPE_DATE:
				return tiVal.Format()+_T("(")+THf2sc((tzVal/60.0),2)+_T(")");
		}
		return _T("未知类型");
	}

	BOOL IsObject(){return (nType==THSWFSDOTYPE_OBJECT || nType==THSWFSDOTYPE_ECMAARRAY || nType==THSWFSDOTYPE_STRICTARRAY);}

	THString sVal;
	U8 u8Val;
	U16 u16Val;
	U32 u32Val;
	double dVal;
	THTime tiVal;
	LONG tzVal;
	void *pVal;
	U32 pValLen;
	U8 nType;
	THString sKey;
};

/**
* @brief Flash Script Data Object处理类(NetStream object)
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-06-04 新建类
*/
/**<pre>
  使用Sample：
	THSwfSdoManager man;
	man.AppendSdoBuffer(buf,nSize);
	THDebug(man.GetDetail());
	UINT nRetLen;
	void *sdobuf=man.MakeSdoBuffer(&nRetLen);
	int ret=memcmp(buf,sdobuf,nSize);
	if (sdobuf) man.FreeSdoBuffer(sdobuf);
	int ret=memcmp(buf,sdobuf,nSize);
	if (sdobuf) man.FreeSdoBuffer(sdobuf);
</pre>*/
class THSwfSdoManager
{
public:
	THSwfSdoManager()
	{
		m_arObj.SetFreeProc(FreeSdo);
	}
	virtual ~THSwfSdoManager()
	{
		Empty();
	}

	void Empty(){m_arObj.RemoveAll();}

	BOOL ParseSdoBuffer(const void *pBuf,UINT nLen,UINT *nRefLen=NULL,UINT size=0,BOOL bCheckKeyType=TRUE,BOOL bCheckKey=TRUE)
	{
		Empty();
		return AppendSdoBuffer(pBuf,nLen,nRefLen,size,bCheckKeyType,bCheckKey);
	}

	//nRefLen为剩余没处理的数据大小,size为只处理多少个对象
	BOOL AppendSdoBuffer(const void *pBuf,UINT nLen,UINT *nRefLen=NULL,UINT size=0,BOOL bCheckKeyType=TRUE,BOOL bCheckKey=TRUE)
	{
		if (!pBuf) return FALSE;
		const char *pchBuf=(const char *)pBuf;
		THSwfSdo *tmp,*tmp1;
		UINT i=0;
		U8 CheckType;
		while(nLen>0 && (size==0 || i<size))
		{
			THString key;
			if (bCheckKey)
			{
				if (bCheckKeyType)
					CheckType=THSWFSDOTYPE_UNDEFINE;
				else
					CheckType=THSWFSDOTYPE_STRING;
				tmp=_GetBufObject(&pchBuf,nLen,TRUE,CheckType);
				if (!tmp) return FALSE;
				if (tmp->nType==THSWFSDOTYPE_END)
				{
					delete tmp;
					break;
				}
				if (!tmp->SafeGetString(key))
				{
					delete tmp;
					return FALSE;
				}
				if (key.IsEmpty())
				{
					delete tmp;
					return FALSE;
				}
				delete tmp;
			}
			else
			{
				//array not have key,make identity key instead
				key.Format(_T("[%d]"),i);
			}
			tmp1=_GetBufObject(&pchBuf,nLen);
			if (!tmp1) return FALSE;
			tmp1->sKey=key;
			if (!m_arObj.AddTail(tmp1))
			{
				delete tmp1;
				return FALSE;
			}
			i++;
		}
		if (nRefLen) *nRefLen=nLen;
		return TRUE;
	}

	//根据内容生成buffer,nSize返回对象个数,bAddEnd为是否加上结束符00 00 09
	void *MakeSdoBuffer(UINT *nLen,BOOL bAddEnd=FALSE,U32 *nSize=NULL,BOOL bCheckKeyType=TRUE,BOOL bCheckKey=TRUE)
	{
		if (nLen) *nLen=0;
		if (nSize) *nSize=0;
		m_buf.Empty();
		THSwfSdo *tmp;
		THPosition pos=m_arObj.GetStartPosition();
		while(!pos.IsEmpty())
		{
			if (m_arObj.GetNextPosition(pos,tmp))
			{
				if (tmp)
				{
					if (bCheckKey)
						_AddStringValueToBuffer(tmp->sKey,FALSE,bCheckKeyType);
					switch(tmp->nType)
					{
						case THSWFSDOTYPE_NUMBER:
						case THSWFSDOTYPE_DATE:
						{
							char ctmp[8],ctmpo[8];
							double dVal;
							if (tmp->nType==THSWFSDOTYPE_NUMBER)
								dVal=tmp->dVal;
							else
							{
								THTime ti;
								dVal=((double)tmp->tiVal.GetTimeAsTime64())*1000.0;
								dVal+=tmp->tiVal.m_st.wMilliseconds;
							}
							memcpy(ctmp,&dVal,8);
							for(int i=0;i<8;i++)
								ctmpo[i]=ctmp[7-i];
							m_buf.AddBuf(&tmp->nType,1);
							m_buf.AddBuf(ctmpo,8);
							if (tmp->nType==THSWFSDOTYPE_DATE)
							{
								I16 timezoneoffest=(I16)tmp->tzVal;
								timezoneoffest=htons(timezoneoffest);
								m_buf.AddBuf(&timezoneoffest,2);
							}
							break;
						}
						case THSWFSDOTYPE_BOOLEAN:
						{
							m_buf.AddBuf(&tmp->nType,1);
							m_buf.AddBuf(&tmp->u8Val,1);
							break;
						}
						case THSWFSDOTYPE_STRING:
						case THSWFSDOTYPE_MOVIECLIP:
						case THSWFSDOTYPE_LONGSTRING:
						{
							if (tmp->nType==THSWFSDOTYPE_MOVIECLIP)
								_AddStringValueToBuffer(tmp->sVal,TRUE);
							else
								_AddStringValueToBuffer(tmp->sVal);
							break;
						}
						case THSWFSDOTYPE_OBJECT:
						case THSWFSDOTYPE_ECMAARRAY:
						case THSWFSDOTYPE_STRICTARRAY:
						{
							BOOL bCheckKey;
							BOOL bAddEnd;
							if (tmp->nType==THSWFSDOTYPE_STRICTARRAY)
							{
								bAddEnd=FALSE;
								bCheckKey=FALSE;
							}
							else
							{
								bCheckKey=TRUE;
								bAddEnd=TRUE;
							}
							m_buf.AddBuf(&tmp->nType,1);
							//error cause
							if (tmp->pVal==NULL || tmp->pValLen==0) return NULL;
							THSwfSdoManager *man=(THSwfSdoManager *)(tmp->pVal);
							UINT nRet;
							U32 nSize;
							void *buf=man->MakeSdoBuffer(&nRet,bAddEnd,&nSize,FALSE,bCheckKey);//也是有end的
							if ((buf==NULL || nRet==0) && nSize!=0)
							{
								if (buf) man->FreeSdoBuffer(buf);
								return NULL;
							}
							//nSize==0 is allow
							if (tmp->nType!=THSWFSDOTYPE_OBJECT)
							{
								nSize=htonl(nSize);
								m_buf.AddBuf(&nSize,4);
							}
							if (buf!=NULL && nRet>0) m_buf.AddBuf(buf,nRet);
							if (buf) man->FreeSdoBuffer(buf);
							break;
						}
						case THSWFSDOTYPE_NULL:
						{
							m_buf.AddBuf(&tmp->nType,1);
						}
							break;
						case THSWFSDOTYPE_UNDEFINE:
						{
							//error cause
							return NULL;
							break;
						}
						case THSWFSDOTYPE_REFERENCE:
						{
							m_buf.AddBuf(&tmp->nType,1);
							U16 val=htons(tmp->u16Val);
							m_buf.AddBuf(&val,2);
							break;
						}
						default:
						{
							//error cause
							return NULL;
						}
					}
					if (nSize) *nSize=(*nSize)+1;
				}
			}
		}
		if (bAddEnd)
		{
			char tmp[3];
			tmp[0]=0;tmp[1]=0;tmp[2]=9;
			m_buf.AddBuf(tmp,3);
		}
		return m_buf.GetBuf(nLen);
	}

	void FreeSdoBuffer(void *buf)
	{
		m_buf.ReleaseGetBufLock();
	}
	BOOL AddNumberObject(THString sKey,double dVal)
	{
		THSwfSdo *tmp=new THSwfSdo;
		if (!tmp) return FALSE;
		tmp->dVal=dVal;
		tmp->nType=THSWFSDOTYPE_NUMBER;
		tmp->sKey=sKey;
		return m_arObj.AddTail(tmp);
	}

	BOOL AddBoolObject(THString sKey,BOOL bVal)
	{
		THSwfSdo *tmp=new THSwfSdo;
		if (!tmp) return FALSE;
		tmp->u8Val=bVal;
		tmp->nType=THSWFSDOTYPE_BOOLEAN;
		tmp->sKey=sKey;
		return m_arObj.AddTail(tmp);
	}

	//String or LongString is Automatic to check
	BOOL AddStringObject(THString sKey,THString sValue)
	{
		THSwfSdo *tmp=new THSwfSdo;
		if (!tmp) return FALSE;
		tmp->sVal=sValue;
		if (THStringConv::GetStringSize(sValue)>0xFFFF)
			tmp->nType=THSWFSDOTYPE_LONGSTRING;
		else
			tmp->nType=THSWFSDOTYPE_STRING;
		tmp->sKey=sKey;
		return m_arObj.AddTail(tmp);
	}

	BOOL AddObjectObject(THString sKey,THSwfSdoManager *man)
	{
		THSwfSdo *tmp=new THSwfSdo;
		if (!tmp) return FALSE;
		tmp->pVal=man;
		tmp->pValLen=4;
		tmp->nType=THSWFSDOTYPE_OBJECT;
		tmp->sKey=sKey;
		return m_arObj.AddTail(tmp);
	}

	BOOL AddMovieClipObject(THString sKey,THString sMoveClipPath)
	{
		THSwfSdo *tmp=new THSwfSdo;
		if (!tmp) return FALSE;
		tmp->sVal=sMoveClipPath;
		tmp->nType=THSWFSDOTYPE_MOVIECLIP;
		tmp->sKey=sKey;
		return m_arObj.AddTail(tmp);
	}

	BOOL AddNullObject(THString sKey)
	{
		THSwfSdo *tmp=new THSwfSdo;
		if (!tmp) return FALSE;
		tmp->nType=THSWFSDOTYPE_NULL;
		tmp->sKey=sKey;
		return m_arObj.AddTail(tmp);
	}

	BOOL AddReferenceObject(THString sKey,U16 nVal)
	{
		THSwfSdo *tmp=new THSwfSdo;
		if (!tmp) return FALSE;
		tmp->u16Val=nVal;
		tmp->nType=THSWFSDOTYPE_REFERENCE;
		tmp->sKey=sKey;
		return m_arObj.AddTail(tmp);
	}

	//bECMA=FALSE:Strict array
	BOOL AddArrayObject(THString sKey,THSwfSdoManager *man,BOOL bECMA=TRUE)
	{
		THSwfSdo *tmp=new THSwfSdo;
		if (!tmp) return FALSE;
		tmp->pVal=man;
		tmp->pValLen=4;
		if (bECMA)
			tmp->nType=THSWFSDOTYPE_ECMAARRAY;
		else
			tmp->nType=THSWFSDOTYPE_STRICTARRAY;
		tmp->sKey=sKey;
		return m_arObj.AddTail(tmp);
	}

	BOOL AddDateObject(THString sKey,THTime ti)
	{
		THSwfSdo *tmp=new THSwfSdo;
		if (!tmp) return FALSE;
		tmp->tiVal=ti;
		tmp->tzVal=THTime::GetLocalTimeZone();
		tmp->nType=THSWFSDOTYPE_DATE;
		tmp->sKey=sKey;
		return m_arObj.AddTail(tmp);
	}

	BOOL RemoveObject(THString sKey)
	{
		THSwfSdo *tmp;
		THPosition pos=m_arObj.GetStartPosition();
		while(!pos.IsEmpty())
		{
			if (m_arObj.GetNextPosition(pos,tmp))
			{
				if (tmp && tmp->sKey==sKey)
				{
					return m_arObj.Remove(tmp);
				}
			}
		}
		return FALSE;
	}

	THSwfSdo *GetObject(THString sKey)
	{
		THSwfSdo *tmp;
		THPosition pos=m_arObj.GetStartPosition();
		while(!pos.IsEmpty())
		{
			if (m_arObj.GetNextPosition(pos,tmp))
			{
				if (tmp && tmp->sKey==sKey) return tmp;
			}
		}
		return NULL;
	}

	double GetNumberObject(THString sKey)
	{
		THSwfSdo *tmp=GetObject(sKey);
		if (tmp)
		{
			double Val;
			if (tmp->SafeGetNumber(Val)) return Val;
		}
		return 0.0;
	}

	BOOL GetBoolObject(THString sKey)
	{
		THSwfSdo *tmp=GetObject(sKey);
		if (tmp)
		{
			BOOL Val;
			if (tmp->SafeGetBool(Val)) return Val;
		}
		return FALSE;
	}

	THString GetStringObject(THString sKey)
	{
		THSwfSdo *tmp=GetObject(sKey);
		if (tmp)
		{
			THString Val;
			if (tmp->SafeGetString(Val)) return Val;
		}
		return _T("");
	}

	THString GetMoveClipObject(THString sKey)
	{
		THSwfSdo *tmp=GetObject(sKey);
		if (tmp)
		{
			THString Val;
			if (tmp->SafeGetMovieClip(Val)) return Val;
		}
		return _T("");
	}

	U16 GetReferenceObject(THString sKey)
	{
		THSwfSdo *tmp=GetObject(sKey);
		if (tmp)
		{
			U16 Val;
			if (tmp->SafeGetReference(Val)) return Val;
		}
		return 0;
	}

	//local time
	THTime GetDateObject(THString sKey)
	{
		THTime Val;
		THSwfSdo *tmp=GetObject(sKey);
		if (tmp) tmp->SafeGetDate(Val);
		return Val;
	}

	LONG GetDateObjectTimeZone(THString sKey)
	{
		THSwfSdo *tmp=GetObject(sKey);
		if (tmp)
		{
			LONG Val=0;
			if (tmp->SafeGetDateTimeZone(Val)) return Val;
		}
		return 0;
	}

	THSwfSdoManager *GetObjectObject(THString sKey)
	{
		THSwfSdo *tmp=GetObject(sKey);
		if (tmp)
		{
			void *Val;
			UINT nLen;
			if (tmp->SafeGetObject(Val,nLen))
			{
				return (THSwfSdoManager *)Val;
			}
		}
		return NULL;
	}

	THSwfSdoManager *GetArrayObject(THString sKey)
	{
		THSwfSdo *tmp=GetObject(sKey);
		if (tmp)
		{
			void *Val;
			UINT nLen;
			if (tmp->SafeGetArray(&Val,nLen))
			{
				return (THSwfSdoManager *)Val;
			}
		}
		return NULL;
	}

	//是否存在
	BOOL GetNullObject(THString sKey)
	{
		THSwfSdo *tmp=GetObject(sKey);
		if (tmp) return tmp->IsNullValue();
		return FALSE;
	}

	//获取全部对象列表
	//Format 格式：[key] 替换键名 [type] 替换键值类型 [value] 替换键值内容 [step] 替换递归级数 支持\r \n \t
	THString GetDetail(THString sFormat=_T("[step][key]([type]):[value]\r\n"),THString sStep=_T(" "))
	{
		THString ret;
		THSwfSdo *tmp;
		THPosition pos=m_arObj.GetStartPosition();
		while(!pos.IsEmpty())
		{
			if (m_arObj.GetNextPosition(pos,tmp))
			{
				if (tmp)
				{
					if (tmp->IsObject())
					{
						THSwfSdoManager *man=(THSwfSdoManager *)tmp->pVal;
						THString stmp=sFormat;
						stmp.Replace(_T("[key]"),tmp->sKey);
						stmp.Replace(_T("[type]"),tmp->GetTypeName());
						stmp.Replace(_T("[value]"),_T(""));
						stmp.Replace(_T("[step]"),sStep);
						ret+=stmp;
						ret+=man->GetDetail(sFormat,sStep+sStep);
					}
					else
						ret+=tmp->GetDetail(sFormat,sStep);
				}
			}
		}
		return ret;
	}

private:
	THSwfSdo *_GetBufObject(const char **retpch,UINT &nLen,BOOL bCheckEnd=FALSE,U8 nUseKeyType=THSWFSDOTYPE_UNDEFINE)
	{
		if (!retpch || !(*retpch) || nLen==0) return NULL;
		const char *pch=*retpch;
		U8 type;
		if (nUseKeyType==THSWFSDOTYPE_UNDEFINE)
		{
			type=*pch;
			pch++;nLen--;
		}
		else
			type=nUseKeyType;
		THSwfSdo *tmp=new THSwfSdo;
		if (!tmp) return NULL;
		if (bCheckEnd)
		{
			if (nUseKeyType==THSWFSDOTYPE_UNDEFINE)
			{
				if (type==0 && (*pch)==0 && (*(pch+1))==9)
				{
					tmp->nType=THSWFSDOTYPE_END;
					pch+=2;nLen-=2;
					return tmp;
				}
			}
			else
			{
				if ((*pch)==0 && (*(pch+1))==0 && (*(pch+2))==9)
				{
					tmp->nType=THSWFSDOTYPE_END;
					pch+=3;nLen-=3;
					return tmp;
				}
			}
		}
		tmp->nType=type;
		switch(type)
		{
			case THSWFSDOTYPE_NUMBER:
			case THSWFSDOTYPE_DATE:
			{
				char ctmp[8];
				for(int i=0;i<8;i++)
					ctmp[i]=*(pch+7-i);
				double dVal;
				memcpy(&dVal,ctmp,8);
				pch+=8;nLen-=8;
				if (type==THSWFSDOTYPE_NUMBER)
				{
					tmp->dVal=dVal;
				}
				else
				{
					//Local time offset in minutes from UTC.
					//For time zones located west of Greenwich, UK,this value is a negative number.
					//Time zones east of Greenwich, UK, are positive.
					I16 timezoneoffest=ntohs(*(const I16 *)pch);
					pch+=2;nLen-=2;
					tmp->tiVal.SetTime((__time64_t)(dVal/1000),TRUE);//DATE parser
					tmp->tiVal.m_st.wMilliseconds=(WORD)(((__int64)dVal)%1000);
					tmp->tzVal=timezoneoffest;
				}
				break;
			}
			case THSWFSDOTYPE_BOOLEAN:
			{
				tmp->u8Val=*pch;
				pch++;nLen--;
				break;
			}
			case THSWFSDOTYPE_STRING:
			case THSWFSDOTYPE_MOVIECLIP:
			case THSWFSDOTYPE_LONGSTRING:
			{
				U32 strlen;
				if (type==THSWFSDOTYPE_LONGSTRING)
				{
					strlen=ntohl(*(const U32 *)pch);
					pch+=4;nLen-=4;
				}
				else
				{
					strlen=ntohs(*(const U16 *)pch);
					pch+=2;nLen-=2;
				}
				if (strlen>0)
				{
					tmp->sVal=THCharset::a2t(pch,strlen);
					pch+=strlen;nLen-=strlen;
				}
				break;
			}
			case THSWFSDOTYPE_OBJECT:
			case THSWFSDOTYPE_ECMAARRAY:
			case THSWFSDOTYPE_STRICTARRAY:
			{
				U32 arsize=0;
				if (type!=THSWFSDOTYPE_OBJECT)
				{
					//array size
					arsize=ntohl(*(const U32 *)pch);
					pch+=4;nLen-=4;
					//because of ecma must check the end 00 00 09,check it when arsize set to 0
					if (type==THSWFSDOTYPE_ECMAARRAY)
						arsize=0;
				}
				//object n,end of 00 00 09
				THSwfSdoManager *man=new THSwfSdoManager;
				if (!man)
				{
					delete tmp;
					return NULL;
				}
				UINT nRefLen=0;
				BOOL bCheckKey;
				if (type==THSWFSDOTYPE_STRICTARRAY)
					bCheckKey=FALSE;
				else
					bCheckKey=TRUE;
				if (!man->AppendSdoBuffer(pch,nLen,&nRefLen,arsize,FALSE,bCheckKey))//本来最后参数传入arsize可限制数组只读取多少，但是有结束符，就可以忽略了
				{
					delete man;
					delete tmp;
					return NULL;
				}
				tmp->pVal=man;
				tmp->pValLen=4;
				pch+=(nLen-nRefLen);
				nLen-=(nLen-nRefLen);
				break;
			}
			case THSWFSDOTYPE_NULL:
				break;
			case THSWFSDOTYPE_UNDEFINE:
			{
				delete tmp;
				return NULL;
				break;
			}
			case THSWFSDOTYPE_REFERENCE:
			{
				tmp->u16Val=ntohs(*(const U16 *)pch);
				pch+=2;nLen-=2;
				break;
			}
			default:
			{
				//fail to parse
				delete tmp;
				return NULL;
			}
		}
		*retpch=pch;
		return tmp;
	}

	void _AddStringValueToBuffer(THString sVal,BOOL bMoveClip=FALSE,BOOL bAddKeyType=TRUE)
	{
		U8 type;
		char *tmp=THCharset::t2a(sVal);
		if (tmp)
		{
			UINT len=(UINT)strlen(tmp);
			if (len>0xFFFF)
				type=THSWFSDOTYPE_LONGSTRING;
			else
			{
				if (bMoveClip)
					type=THSWFSDOTYPE_MOVIECLIP;
				else
					type=THSWFSDOTYPE_STRING;
			}
			if (bAddKeyType) m_buf.AddBuf(&type,1);
			if (type==THSWFSDOTYPE_LONGSTRING)
			{
				U32 utmp=len;
				utmp=htonl(utmp);
				m_buf.AddBuf(&utmp,4);
			}
			else
			{
				U16 utmp=len;
				utmp=htons(utmp);
				m_buf.AddBuf(&utmp,2);
			}
			m_buf.AddBuf(tmp,len);
			THCharset::free(tmp);
		}
	}

	static void FreeSdo(void *key,void *value,void *adddata)
	{
		THSwfSdo *tmp=(THSwfSdo *)value;
		if (tmp)
		{
			if (tmp->pVal && tmp->pValLen)
			{
				if (tmp->nType==THSWFSDOTYPE_OBJECT || tmp->nType==THSWFSDOTYPE_ECMAARRAY || tmp->nType==THSWFSDOTYPE_STRICTARRAY)
					delete (THSwfSdoManager *)tmp->pVal;
				else
					//not use now
					delete [] tmp->pVal;
			}
			delete tmp;
		}
	}
	THList<THSwfSdo *> m_arObj;
	THMemBuf m_buf;
};

enum THSwfObjectType{
	THSwfObj_End=0,
	THSwfObj_ShowFrame=1,
	THSwfObj_DefineShape=2,
	THSwfObj_PlaceObject=4,
	THSwfObj_RemoveObject=5,
	THSwfObj_DefineBits=6,
	THSwfObj_DefineButton=7,
	THSwfObj_JPEGTables=8,
	THSwfObj_SetBackgroundColor=9,
	THSwfObj_DefineFont=10,
	THSwfObj_DefineText=11,
	THSwfObj_DoAction=12,
	THSwfObj_DefineFontInfo=13,
	THSwfObj_DefineSound=14,
	THSwfObj_StartSound=15,
	THSwfObj_DefineButtonSound=17,
	THSwfObj_SoundStreamHead=18,
	THSwfObj_SoundStreamBlock=19,
	THSwfObj_DefineBitsLossless=20,
	THSwfObj_DefineBitsJPEG2=21,
	THSwfObj_DefineShape2=22,
	THSwfObj_DefineButtonCxform=23,
	THSwfObj_Protect=24,
	THSwfObj_PlaceObject2=26,
	THSwfObj_RemoveObject2=28,
	THSwfObj_DefineShape3=32,
	THSwfObj_DefineText2=33,
	THSwfObj_DefineButton2=34,
	THSwfObj_DefineBitsJPEG3=35,
	THSwfObj_DefineBitsLossless2=36,
	THSwfObj_DefineEditText=37,
	THSwfObj_DefineSprite=39,
	THSwfObj_FrameLabel=43,
	THSwfObj_SoundStreamHead2=45,
	THSwfObj_DefineMorphShape=46,
	THSwfObj_DefineFont2=48,
	THSwfObj_ExportAssets=56,
	THSwfObj_ImportAssets=57,
	THSwfObj_EnableDebugger=58,
	THSwfObj_DoInitAction=59,
	THSwfObj_DefineVideoStream=60,
	THSwfObj_VideoFrame=61,
	THSwfObj_DefineFontInfo2=62,
	THSwfObj_EnableDebugger2=64,
	THSwfObj_ScriptLimits=65,
	THSwfObj_SetTabIndex=66,
	THSwfObj_FileAttributes=69,
	THSwfObj_PlaceObject3=70,
	THSwfObj_ImportAssets2=71,
	THSwfObj_DefineFontAlignZones=73,
	THSwfObj_CSMTextSettings=74,
	THSwfObj_DefineFont3=75,
	THSwfObj_SymbolClass=76,
	THSwfObj_Metadata=77,
	THSwfObj_DefineScalingGrid=78,
	THSwfObj_DoABC=82,
	THSwfObj_DefineShape4=83,
	THSwfObj_DefineMorphShape2=84,
	THSwfObj_DefineSceneAndFrameLabelData=86,
	THSwfObj_DefineBinaryData=87,
	THSwfObj_DefineFontName=88,
	THSwfObj_StartSound2=89,
};

enum THSwfParserError{
	THSwfParserError_None=0,
	THSwfParserError_FileError,		///<文件打不开
	THSwfParserError_InvalidParam,	///<参数错误
	THSwfParserError_NotSwfFile,	///<不是Swf文件
	THSwfParserError_VerNotSupport,	///<版本不支持
	THSwfParserError_SwfStreamError,///<Flash中有流错误
	THSwfParserError_MemoryError,	///<内存不够
};

#pragma pack(push,1)

typedef struct _THSwfRect{
	U8	nCnt;
	U32 GetStructSize()
	{
		THBitParser parser;
		parser.SetParseDataBuffer(&nCnt,8);
		U32 Nbits=parser.GetBit2U32(0,5,FALSE);
		return ((Nbits*4+5)+7)/8;
	}
	THSize GetSize()
	{
		THSize size;
		THBitParser parser;
		parser.SetParseDataBuffer(&nCnt,GetStructSize()*8);
		U32 Nbits=parser.GetBit2U32(0,5,FALSE);
		U32 tmp=parser.GetBit2U32(5,Nbits,FALSE);
		size.cx=(U32)(parser.GetBit2U32(5+Nbits,Nbits,FALSE)-tmp);
		tmp=parser.GetBit2U32(5+Nbits*2,Nbits,FALSE);
		size.cy=(U32)(parser.GetBit2U32(5+Nbits*3,Nbits,FALSE)-tmp);
		return size;
	}
}THSwfRect;

typedef struct _THSwfHeader{
	U8 sHeader[3];	//FWS or CWS(compressed,SWF 6 and later only)
	U8 nVersion;	//0x9 for Flash 9
	U32 nFileLen;//len of file
	THSwfRect rFrameSize;	//Offest of Stream Start
	U32 GetFileLen(){return nFileLen;}
	THSwfParserError CheckHeader()
	{
		if (memcmp(sHeader,"FWS",3)!=0 && memcmp(sHeader,"CWS",3)!=0) return THSwfParserError_NotSwfFile;
		if (rFrameSize.GetStructSize()==0) return THSwfParserError_SwfStreamError;
		if (nVersion>0xa) return THSwfParserError_VerNotSupport;
		return THSwfParserError_None;
	}
	THSize GetSize() 
	{
		THSize size;
		size=rFrameSize.GetSize();
		size.cx=THTwip2Pixel(size.cx);
		size.cy=THTwip2Pixel(size.cy);
		return size;
	}
	U32 GetStructSize() {return rFrameSize.GetStructSize()+8;}
	BOOL IsCompressed() {return memcmp(sHeader,"CWS",3)==0;}
}THSwfHeader;

typedef struct _THSwfHeader2{
	U16 nFrameRate;//8.8 format
	U16 nFrameCount;
	U16 GetFrameRate(){return ntohs(nFrameRate);}
	U16 GetFrameCount(){return nFrameCount;}
}THSwfHeader2;

typedef struct _THSwfRecordTag{
	U16 nTag;
	U16 GetTagType()
	{
		U16 tag=ntohs(nTag);
		THBitParser parser;
		parser.SetParseDataBuffer(&tag,16);
		return parser.GetBit2U32(0,10,FALSE);
	}
	U16 GetTagLength()
	{
		U16 tag=ntohs(nTag);
		THBitParser parser;
		parser.SetParseDataBuffer(&tag,16);
		return parser.GetBit2U32(10,6,FALSE);
	}
	BOOL IsLongHeader()
	{
		return (nTag&0x3F)==0x3F;
	}
}THSwfRecordTag;
#pragma pack(pop)

class THSwfObject
{
public:
	THSwfObject()
	{
		m_buf=NULL;
		m_buflen=NULL;
		m_type=THSwfObj_End;
		m_bRefBuf=TRUE;
	}

	virtual ~THSwfObject()
	{
		if (!m_bRefBuf && m_buf)
			delete [] m_buf;
	}

	virtual void SetupBuf(THSwfObjectType type,const U8 *buf,unsigned int buflen)
	{
		if (!m_bRefBuf && m_buf) delete [] m_buf;
		m_type=type;
		m_buf=buf;
		m_buflen=buflen;
		m_bRefBuf=TRUE;
		m_sObjXml.Empty();
		ParseBuffer();
	}

	THString &GetObjectDestXml(){return m_sObjXml;}

	virtual void ParseBuffer()
	{
	}

	virtual void MakeBuffer(THMemBuf *buf)
	{
		buf->AddBuf(m_buf,m_buflen);
	}

	THString GetObjectTypeString() {return GetObjectTypeString(m_type);}
	THSwfObjectType GetObjectType() {return m_type;}

	static THString GetObjectTypeString(THSwfObjectType type)
	{
		switch(type)
		{
			case 0:return _T("End");
			case 1:return _T("ShowFrame");
			case 2:return _T("DefineShape");
			case 4:return _T("PlaceObject");
			case 5:return _T("RemoveObject");
			case 6:return _T("DefineBits");
			case 7:return _T("DefineButton");
			case 8:return _T("JPEGTables");
			case 9:return _T("SetBackgroundColor");
			case 10:return _T("DefineFont");
			case 11:return _T("DefineText");
			case 12:return _T("DoAction");
			case 13:return _T("DefineFontInfo");
			case 14:return _T("DefineSound");
			case 15:return _T("StartSound");
			case 17:return _T("DefineButtonSound");
			case 18:return _T("SoundStreamHead");
			case 19:return _T("SoundStreamBlock");
			case 20:return _T("DefineBitsLossless");
			case 21:return _T("DefineBitsJPEG2");
			case 22:return _T("DefineShape2");
			case 23:return _T("DefineButtonCxform");
			case 24:return _T("Protect");
			case 26:return _T("PlaceObject2");
			case 28:return _T("RemoveObject2");
			case 32:return _T("DefineShape3");
			case 33:return _T("DefineText2");
			case 34:return _T("DefineButton2");
			case 35:return _T("DefineBitsJPEG3");
			case 36:return _T("DefineBitsLossless2");
			case 37:return _T("DefineEditText");
			case 39:return _T("DefineSprite");
			case 43:return _T("FrameLabel");
			case 45:return _T("SoundStreamHead2");
			case 46:return _T("DefineMorphShape");
			case 48:return _T("DefineFont2");
			case 56:return _T("ExportAssets");
			case 57:return _T("ImportAssets");
			case 58:return _T("EnableDebugger");
			case 59:return _T("DoInitAction");
			case 60:return _T("DefineVideoStream");
			case 61:return _T("VideoFrame");
			case 62:return _T("DefineFontInfo2");
			case 64:return _T("EnableDebugger2");
			case 65:return _T("ScriptLimits");
			case 66:return _T("SetTabIndex");
			case 69:return _T("FileAttributes");
			case 70:return _T("PlaceObject3");
			case 71:return _T("ImportAssets2");
			case 73:return _T("DefineFontAlignZones");
			case 74:return _T("CSMTextSettings");
			case 75:return _T("DefineFont3");
			case 76:return _T("SymbolClass");
			case 77:return _T("Metadata");
			case 78:return _T("DefineScalingGrid");
			case 82:return _T("DoABC");
			case 83:return _T("DefineShape4");
			case 84:return _T("DefineMorphShape2");
			case 86:return _T("DefineSceneAndFrameLabelData");
			case 87:return _T("DefineBinaryData");
			case 88:return _T("DefineFontName");
			case 89:return _T("StartSound2");
		}
		return THFormat(_T("unknown:0x%x(%d)"),type,type);
	}

	virtual THString PrintDetail()
	{
		THString str;
		str.Format(_T("Object:%s,Len:0x%x\r\n%s\r\n"),GetObjectTypeString(),m_buflen,m_sObjXml.IsEmpty()?THStringConv::FormatString(m_buf,m_buflen,THStringConv::FormatType_Hex,32,THStringConv::FormatType_Hex,TRUE,FALSE):m_sObjXml);
		return str;
	}
protected:
	THSwfObjectType m_type;
	const U8 *m_buf;
	unsigned int m_buflen;
	THString m_sObjXml;
	BOOL m_bRefBuf;
};

class THSwfObjectFont: public THSwfObject
{
public:
	virtual void ParseBuffer()
	{
		if (m_buf)
		{
			//(U16 *)m_buf;
		}
	}
};

class THSwfObjectText: public THSwfObject
{
public:
	virtual void ParseBuffer()
	{
		if (m_buf)
		{
			m_sObjXml+=THSimpleXml::MakeParam(_T("id"),THu2s(*(U16 *)m_buf));
		}
	}
};

/**
* @brief Swf文件处理类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-06-04 新建类
*/
/**<pre>
  使用Sample：
</pre>*/
class THSwfParser
{
public:
	THSwfParser()
	{
		m_bInit=FALSE;
		m_header=NULL;
		m_header2=NULL;
		m_objlist.SetFreeProc(FreeObj);
	}
	virtual ~THSwfParser()
	{
		CloseSwfFile();
	}

	//返回 THSwfParserError_SwfStreamError 也表示成功，其中有部分流没有解析成功
	THSwfParserError OpenSwfFile(THString sFile)
	{
		CloseSwfFile();
		m_sFile=sFile;
		THSwfParserError err=THSwfParserError_InvalidParam;
		if (!sFile.IsEmpty())
		{
			err=THSwfParserError_FileError;
			unsigned int len;
			U8 *buf=(U8 *)THFile::GetContentToBuffer(sFile,&len);
			if (buf)
			{
				err=((THSwfHeader *)buf)->CheckHeader();
				if (err==THSwfParserError_None)
				{
					if (((THSwfHeader *)buf)->IsCompressed())
					{
						//translate CWS to FWS
						m_buf.AddBuf("F",1);
						m_buf.AddBuf(buf+1,7);
						//try expand data
						if (!THGZip::ZipDecode2((const void *)(buf+8),len-8,&m_buf,THGZIP_ZLIB))
						{
							m_buf.Empty();
						}
					}
					else
					{
						m_buf.AddBuf(buf,len);
					}
					THFile::FreeContentBuffer(buf);
					U8 *cur=(U8 *)m_buf.GetBuf(&len);
					//make sure that do not change when use the pointer
					m_buf.ReleaseGetBufLock();
					if (!cur || len==0)
					{
						err=THSwfParserError_NotSwfFile;
						return err;
					}
					m_header=(THSwfHeader *)cur;
					err=m_header->CheckHeader();
					if (err==THSwfParserError_None)
					{
						//try fix the len
						if (len>m_header->GetFileLen()) len=m_header->GetFileLen();
#define MoveCurrentPointer(off)	{U32 ioff=off;if (len<ioff){return THSwfParserError_SwfStreamError;}cur+=ioff;len-=ioff;}
						MoveCurrentPointer(m_header->GetStructSize());
						//Frame and Count
						m_header2=(THSwfHeader2 *)cur;
						MoveCurrentPointer(4);
						//other
						m_bInit=TRUE;
						//tag
						while(len>0)
						{
							THSwfRecordTag *tag=(THSwfRecordTag *)cur;
							MoveCurrentPointer(2);
							U16 ttype=tag->GetTagType();
							U32 tlen;
							if (tag->IsLongHeader())
							{
								tlen=(*(U32 *)cur);
								MoveCurrentPointer(4);
							}
							else
								tlen=tag->GetTagLength();
							THSwfObject *obj=NewSwfObject((THSwfObjectType)ttype);
							if (obj)
							{
								obj->SetupBuf((THSwfObjectType)ttype,cur,tlen);
								m_objlist.AddTail(obj);
							}
							//THDebug(_T("%d,len:%d"),ttype,tlen);
							MoveCurrentPointer(tlen);
						}
						err=THSwfParserError_None;
					}
				}
				else
				{
					err=THSwfParserError_NotSwfFile;
					THFile::FreeContentBuffer(buf);
				}
			}
		}
		return err;
	}

	static THSwfObject *NewSwfObject(THSwfObjectType t)
	{
		switch(t)
		{
			case THSwfObj_DefineFont:
			case THSwfObj_DefineFont2:
				return new THSwfObjectFont;
			case THSwfObj_DefineText:
				return new THSwfObjectText;
		}
		return new THSwfObject;
	}

	static void FreeSwfObject(THSwfObject *obj)
	{
		if (obj)
		{
			switch(obj->GetObjectType())
			{
				case THSwfObj_DefineFont:
				case THSwfObj_DefineFont2:
					delete (THSwfObjectFont *)obj;break;
				case THSwfObj_DefineText:
					delete (THSwfObjectText *)obj;break;
				default:
					delete obj;
			}
		}
	}

	THMemBuf* GetBuf(){return &m_buf;}
	//保存swf文件
	BOOL DumpSwf(THString sSave)
	{
		BOOL ret=FALSE;
		UINT len;
		void *cur=m_buf.GetBuf(&len);
		if(cur && len)
			ret=THFile::SaveFile(sSave,cur,len);
		m_buf.ReleaseGetBufLock();
		return ret;
	}

	void CloseSwfFile()
	{
		if (m_bInit)
		{
			m_objlist.RemoveAll();
			m_buf.Empty();
			m_bInit=FALSE;
			m_header=NULL;
			m_header2=NULL;
		}
	}

	THString PrintDetail()
	{
		THString ret;
		if (m_bInit)
		{
			ret.Format(_T("File Path:%s\r\nFile Header: IsCompressed:%s Version:%d FileSize:%u Size:%s Frame:%dbps*%d\r\n"),
				m_sFile,m_header->IsCompressed()?_T("TRUE"):_T("FALSE"),m_header->nVersion,m_header->GetFileLen(),
				m_header->GetSize().Print(),m_header2->GetFrameRate(),m_header2->GetFrameCount());
			THSwfObject *obj;
			THPosition pos=m_objlist.GetStartPosition();
			while(!pos.IsEmpty())
			{
				if (m_objlist.GetNextPosition(pos,obj))
				{
					ret+=obj->PrintDetail();
				}
			}
		}
		return ret;
	}

	//把一个文件中可能包含的swf都保存下来
	static BOOL DumpSwfFromFile(THString sFile,U8 nMinVer=3,U8 nMaxVer=10)
	{
		if (sFile.IsEmpty()) return FALSE;
		int outcount=0;
		unsigned int len;
		U8 *buf=(U8 *)THFile::GetContentToBuffer(sFile,&len);
		if (!buf) return FALSE;
		unsigned int curoff=0;
		while(curoff<len)
		{
			U8 *f=(U8 *)THBinSearch::BinSearch(buf+curoff,len-curoff,"WS",2);
			if (!f) break;
			curoff=(unsigned int)(f-buf)+2;
			if (*(f-1)!='C' && *(f-1)!='F') continue;
			if (((*(f+2)>nMaxVer) || (*(f+2)<nMinVer))) continue;
			//CWF 的plen是压缩前的len，该数字只是用于尽量减少swf的大小，输出的swf大小不是原swf的大小
			U32 plen=*((U32 *)(f+3));
			if (plen==0) continue;
			THFile::SaveFile(sFile+_T("_out")+THi2s(outcount++)+_T(".swf"),f-1,min(len-(f-1-buf),plen+8));
		}
		return outcount!=0;
	}
protected:
	THSwfHeader *m_header;
	THSwfHeader2 *m_header2;
	THString m_sFile;
	THMemBuf m_buf;
	BOOL m_bInit;
	THList<THSwfObject *> m_objlist;
	static void FreeObj(void *key,void *value,void *adddata)
	{
		FreeSwfObject((THSwfObject *)value);
	}
};