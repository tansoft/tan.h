#pragma once

#define THAMF0_NUMBER			0x0
#define THAMF0_BOOLEAN			0x1
#define THAMF0_STRING			0x2
#define THAMF0_OBJECT			0x3
#define THAMF0_MOVIECLIP		0x4		///<The Movieclip and Recordset types are not supported for serialization; their markers are retained with a reserved status for future use.
#define THAMF0_NULL				0x5
#define THAMF0_UNDEFINE			0x6
#define THAMF0_REFERENCE		0x7
#define THAMF0_ECMAARRAY		0x8		///<数组
#define THAMF0_OBJECTEND		0x9		///<用于结束，The object-end-type should only appear to mark the end of a set of properties of an object-type or typed-object-type or to signal the end of an associative section of an ECMA Array.
#define THAMF0_STRICTARRAY		0xA		///<严格数组
#define THAMF0_DATE				0xB
#define THAMF0_LONGSTRING		0xC
#define THAMF0_UNSUPPORT		0xD
#define THAMF0_RECORDSET		0xE		///<The Movieclip and Recordset types are not supported for serialization; their markers are retained with a reserved status for future use.
#define THAMF0_XMLDOC			0xF
#define THAMF0_TYPED_OBJECT		0x10
#define THAMF0_AMF3				0x11	///<NOTE: With the introduction of AMF 3 in Flash Player 9, a special type marker was added to AMF 0 to signal a switch to AMF 3 serialization. This allows NetConnection requests to start out in AMF 0 and switch to AMF 3 on the first complex type to take advantage of the more the efficient encoding of AMF 3.

/**
* @brief Flash AMF协议解释类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2009-09-08 新建类
*/
/**<pre>
  使用Sample：
  根据协议版本编写：
	http://opensource.adobe.com/wiki/download/attachments/1114283/amf0_spec_121207.pdf?version=1
	http://opensource.adobe.com/wiki/download/attachments/1114283/amf3_spec_05_05_08.pdf?version=1
</pre>*/
class THAmfParser
{
public:
	THAmfParser(){}
	virtual ~THAmfParser(){}
protected:
};

class THAmf0Parser;

//AMF0结构
//U16 nKeyLength
//CHAR *pKey
//U8 nValueType
//void *pValueData

/**
* @brief Amf0对象描述类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2009-09-08 新建类
*/
/**<pre>
  使用Sample：
</pre>*/
class THAmf0
{
public:
	THAmf0()
	{
		u8Val=0;
		u16Val=0;
		u32Val=0;
		dVal=0;
		pVal=NULL;
		pValLen=0;
		tzVal=0;
		nType=THAMF0_UNDEFINE;
	}

	virtual ~THAmf0()
	{
		//pVal new info not delete in here
	}

	BOOL SafeGetNumber(double &ret)
	{
		if (nType!=THAMF0_NUMBER) return FALSE;
		ret=dVal;
		return TRUE;
	}

	BOOL SafeGetBool(BOOL &ret)
	{
		if (nType!=THAMF0_BOOLEAN) return FALSE;
		ret=u8Val;
		return TRUE;
	}

	BOOL SafeGetString(THString &ret)
	{
		if (nType!=THAMF0_STRING && nType!=THAMF0_LONGSTRING) return FALSE;
		ret=sVal;
		return TRUE;
	}

	BOOL SafeGetObject(void *pret,UINT &len)
	{
		if ((nType!=THAMF0_OBJECT && nType!=THAMF0_TYPED_OBJECT) || pVal==NULL || pValLen==0) return FALSE;
		pret=pVal;
		len=pValLen;
		return TRUE;
	}

	BOOL SafeGetRecordSet(THString &ret)
	{
		if (nType!=THAMF0_RECORDSET) return FALSE;
		ret=sVal;
		return TRUE;
	}

	BOOL SafeGetMovieClip(THString &ret)
	{
		if (nType!=THAMF0_MOVIECLIP) return FALSE;
		ret=sVal;
		return TRUE;
	}

	BOOL IsNullValue(){return nType==THAMF0_NULL;}
	BOOL IsAmf3(){return nType==THAMF0_AMF3;}

	BOOL SafeGetReference(U16 &ret)
	{
		if (nType!=THAMF0_REFERENCE) return FALSE;
		ret=u16Val;
		return TRUE;
	}

	BOOL SafeGetArray(void **pret,UINT &len)
	{
		if ((nType!=THAMF0_ECMAARRAY && nType!=THAMF0_STRICTARRAY) || !pret || !(*pret) || pValLen==0) return FALSE;
		*pret=pVal;
		len=pValLen;
		return TRUE;
	}

	//local time
	BOOL SafeGetDate(THTime &ti)
	{
		if (nType!=THAMF0_DATE) return FALSE;
		ti=tiVal;
		return TRUE;
	}

	//单位分钟
	BOOL SafeGetDateTimeZone(LONG &tz)
	{
		if (nType!=THAMF0_DATE) return FALSE;
		tz=tzVal;
		return TRUE;
	}

	BOOL SafeGetXmlDoc(THString &xml)
	{
		if (nType!=THAMF0_XMLDOC) return FALSE;
		xml=sVal;
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
			case THAMF0_NUMBER:return _T("数字");
			case THAMF0_BOOLEAN:return _T("布尔值");
			case THAMF0_STRING:return _T("字符串");
			case THAMF0_OBJECT:return _T("对象");
			case THAMF0_MOVIECLIP:return _T("剪辑路径");
			case THAMF0_NULL:return _T("空");
			case THAMF0_UNDEFINE:return _T("未定义");
			case THAMF0_REFERENCE:return _T("引用");
			case THAMF0_OBJECTEND:return _T("结束");
			case THAMF0_ECMAARRAY:return _T("数组");
			case THAMF0_STRICTARRAY:return _T("严格数组");
			case THAMF0_DATE:return _T("日期");
			case THAMF0_LONGSTRING:return _T("长字符串");
			case THAMF0_UNSUPPORT:return _T("不支持");
			case THAMF0_RECORDSET:return _T("记录集");
			case THAMF0_XMLDOC:return _T("Xml");
			case THAMF0_TYPED_OBJECT:return _T("自定义类型对象");
			case THAMF0_AMF3:return _T("Amf3协议");
		}
		return _T("未知类型");
	}

	THString GetValueString()
	{
		switch(nType)
		{
			case THAMF0_NUMBER:
				return THf2sc(dVal,6);
			case THAMF0_BOOLEAN:
				return (u8Val)?_T("TRUE"):_T("FALSE");
			case THAMF0_STRING:
			case THAMF0_MOVIECLIP:
			case THAMF0_LONGSTRING:
			case THAMF0_XMLDOC:
			case THAMF0_RECORDSET:
				return sVal;
			case THAMF0_OBJECT:
			case THAMF0_ECMAARRAY:
			case THAMF0_STRICTARRAY:
			case THAMF0_TYPED_OBJECT:
				return _T("(对象)");
			case THAMF0_NULL:
			case THAMF0_UNDEFINE:
			case THAMF0_OBJECTEND:
				return _T("");
			case THAMF0_REFERENCE:
				return THFormat(_T("%u"),u16Val);
			case THAMF0_DATE:
				return tiVal.Format()+_T("(")+THf2sc((tzVal/60.0),2)+_T(")");
			case THAMF0_AMF3:
				return _T("AMF3协议");
		}
		return _T("未知类型");
	}

	BOOL IsObject(){return (nType==THAMF0_OBJECT || nType==THAMF0_ECMAARRAY || nType==THAMF0_STRICTARRAY || nType==THAMF0_TYPED_OBJECT);}

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
	THAmf0Parser man;
	man.AppendAmf0Buffer(buf,nSize);
	THDebug(man.GetDetail());
	UINT nRetLen;
	void *amf0buf=man.MakeAmf0Buffer(&nRetLen);
	int ret=memcmp(buf,amf0buf,nSize);
	if (amf0buf) man.FreeAmf0Buffer(amf0buf);
	int ret=memcmp(buf,amf0buf,nSize);
	if (amf0buf) man.FreeAmfBuffer(amf0buf);
</pre>*/
class THAmf0Parser
{
public:
	THAmf0Parser()
	{
		m_arObj.SetFreeProc(FreeAmf0);
	}
	virtual ~THAmf0Parser()
	{
		Empty();
	}

	void Empty(){m_arObj.RemoveAll();}

	BOOL ParseAmf0Buffer(const void *pBuf,UINT nLen,UINT *nRefLen=NULL,UINT size=0,BOOL bCheckKeyType=TRUE,BOOL bCheckKey=TRUE)
	{
		Empty();
		return AppendAmf0Buffer(pBuf,nLen,nRefLen,size,bCheckKeyType,bCheckKey);
	}

	//nRefLen为剩余没处理的数据大小,size为只处理多少个对象
	BOOL AppendAmf0Buffer(const void *pBuf,UINT nLen,UINT *nRefLen=NULL,UINT size=0,BOOL bCheckKeyType=TRUE,BOOL bCheckKey=TRUE)
	{
		if (!pBuf) return FALSE;
		const char *pchBuf=(const char *)pBuf;
		THAmf0 *tmp,*tmp1;
		UINT i=0;
		U8 CheckType;
		while(nLen>0 && (size==0 || i<size))
		{
			THString key;
			if (bCheckKey)
			{
				if (bCheckKeyType)
					CheckType=THAMF0_UNDEFINE;
				else
					CheckType=THAMF0_STRING;
				tmp=_GetBufObject(&pchBuf,nLen,TRUE,CheckType);
				if (!tmp)
				{
					ASSERT(FALSE);
					return FALSE;
				}
				if (tmp->nType==THAMF0_OBJECTEND)
				{
					delete tmp;
					break;
				}
				if (!tmp->SafeGetString(key))
				{
					ASSERT(FALSE);
					delete tmp;
					return FALSE;
				}
				if (key.IsEmpty())
				{
					ASSERT(FALSE);
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
			if (!tmp1)
			{
				ASSERT(FALSE);
				return FALSE;
			}
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
	void *MakeAmf0Buffer(UINT *nLen,BOOL bAddEnd=FALSE,U32 *nSize=NULL,BOOL bCheckKeyType=TRUE,BOOL bCheckKey=TRUE)
	{
		if (nLen) *nLen=0;
		if (nSize) *nSize=0;
		m_buf.Empty();
		THAmf0 *tmp;
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
						case THAMF0_NUMBER:
						case THAMF0_DATE:
						{
							char ctmp[8],ctmpo[8];
							double dVal;
							if (tmp->nType==THAMF0_NUMBER)
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
							if (tmp->nType==THAMF0_DATE)
							{
								I16 timezoneoffest=(I16)tmp->tzVal;
								timezoneoffest=htons(timezoneoffest);
								m_buf.AddBuf(&timezoneoffest,2);
							}
							break;
						}
						case THAMF0_BOOLEAN:
						{
							m_buf.AddBuf(&tmp->nType,1);
							m_buf.AddBuf(&tmp->u8Val,1);
							break;
						}
						case THAMF0_STRING:
						case THAMF0_MOVIECLIP:
						case THAMF0_LONGSTRING:
						{
							if (tmp->nType==THAMF0_MOVIECLIP)
								_AddStringValueToBuffer(tmp->sVal,TRUE);
							else
								_AddStringValueToBuffer(tmp->sVal);
							break;
						}
						case THAMF0_OBJECT:
						case THAMF0_ECMAARRAY:
						case THAMF0_STRICTARRAY:
						{
							BOOL bCheckKey;
							BOOL bAddEnd;
							if (tmp->nType==THAMF0_STRICTARRAY)
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
							THAmf0Parser *man=(THAmf0Parser *)(tmp->pVal);
							UINT nRet;
							U32 nSize;
							void *buf=man->MakeAmf0Buffer(&nRet,bAddEnd,&nSize,FALSE,bCheckKey);//也是有end的
							if ((buf==NULL || nRet==0) && nSize!=0)
							{
								if (buf) man->FreeAmf0Buffer(buf);
								return NULL;
							}
							//nSize==0 is allow
							if (tmp->nType!=THAMF0_OBJECT)
							{
								nSize=htonl(nSize);
								m_buf.AddBuf(&nSize,4);
							}
							if (buf!=NULL && nRet>0) m_buf.AddBuf(buf,nRet);
							if (buf) man->FreeAmf0Buffer(buf);
							break;
						}
						case THAMF0_NULL:
						case THAMF0_AMF3:
						{
							m_buf.AddBuf(&tmp->nType,1);
						}
							break;
						case THAMF0_UNDEFINE:
						{
							//error cause
							return NULL;
							break;
						}
						case THAMF0_REFERENCE:
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

	void FreeAmf0Buffer(void *buf)
	{
		m_buf.ReleaseGetBufLock();
	}
	BOOL AddNumberObject(THString sKey,double dVal)
	{
		THAmf0 *tmp=new THAmf0;
		if (!tmp) return FALSE;
		tmp->dVal=dVal;
		tmp->nType=THAMF0_NUMBER;
		tmp->sKey=sKey;
		return m_arObj.AddTail(tmp);
	}

	BOOL AddBoolObject(THString sKey,BOOL bVal)
	{
		THAmf0 *tmp=new THAmf0;
		if (!tmp) return FALSE;
		tmp->u8Val=bVal;
		tmp->nType=THAMF0_BOOLEAN;
		tmp->sKey=sKey;
		return m_arObj.AddTail(tmp);
	}

	//String or LongString is Automatic to check
	BOOL AddStringObject(THString sKey,THString sValue)
	{
		THAmf0 *tmp=new THAmf0;
		if (!tmp) return FALSE;
		tmp->sVal=sValue;
		if (THStringConv::GetStringSize(sValue)>0xFFFF)
			tmp->nType=THAMF0_LONGSTRING;
		else
			tmp->nType=THAMF0_STRING;
		tmp->sKey=sKey;
		return m_arObj.AddTail(tmp);
	}

	BOOL AddObjectObject(THString sKey,THAmf0Parser *man,BOOL bTypedObject=FALSE)
	{
		THAmf0 *tmp=new THAmf0;
		if (!tmp) return FALSE;
		tmp->pVal=man;
		tmp->pValLen=4;
		tmp->nType=bTypedObject?THAMF0_TYPED_OBJECT:THAMF0_OBJECT;
		tmp->sKey=sKey;
		return m_arObj.AddTail(tmp);
	}

	BOOL AddMovieClipObject(THString sKey,THString sMoveClipPath)
	{
		THAmf0 *tmp=new THAmf0;
		if (!tmp) return FALSE;
		tmp->sVal=sMoveClipPath;
		tmp->nType=THAMF0_MOVIECLIP;
		tmp->sKey=sKey;
		return m_arObj.AddTail(tmp);
	}

	BOOL AddRecordSetObject(THString sKey,THString sRecordSet)
	{
		THAmf0 *tmp=new THAmf0;
		if (!tmp) return FALSE;
		tmp->sVal=sRecordSet;
		tmp->nType=THAMF0_RECORDSET;
		tmp->sKey=sKey;
		return m_arObj.AddTail(tmp);
	}

	BOOL AddXmlDocObject(THString sKey,THString sXml)
	{
		THAmf0 *tmp=new THAmf0;
		if (!tmp) return FALSE;
		tmp->sVal=sXml;
		tmp->nType=THAMF0_XMLDOC;
		tmp->sKey=sKey;
		return m_arObj.AddTail(tmp);
	}

	BOOL AddNullObject(THString sKey)
	{
		THAmf0 *tmp=new THAmf0;
		if (!tmp) return FALSE;
		tmp->nType=THAMF0_NULL;
		tmp->sKey=sKey;
		return m_arObj.AddTail(tmp);
	}

	BOOL AddReferenceObject(THString sKey,U16 nVal)
	{
		THAmf0 *tmp=new THAmf0;
		if (!tmp) return FALSE;
		tmp->u16Val=nVal;
		tmp->nType=THAMF0_REFERENCE;
		tmp->sKey=sKey;
		return m_arObj.AddTail(tmp);
	}

	//bECMA=FALSE:Strict array
	BOOL AddArrayObject(THString sKey,THAmf0Parser *man,BOOL bECMA=TRUE)
	{
		THAmf0 *tmp=new THAmf0;
		if (!tmp) return FALSE;
		tmp->pVal=man;
		tmp->pValLen=4;
		if (bECMA)
			tmp->nType=THAMF0_ECMAARRAY;
		else
			tmp->nType=THAMF0_STRICTARRAY;
		tmp->sKey=sKey;
		return m_arObj.AddTail(tmp);
	}

	BOOL AddDateObject(THString sKey,THTime ti)
	{
		THAmf0 *tmp=new THAmf0;
		if (!tmp) return FALSE;
		tmp->tiVal=ti;
		tmp->tzVal=THTime::GetLocalTimeZone();
		tmp->nType=THAMF0_DATE;
		tmp->sKey=sKey;
		return m_arObj.AddTail(tmp);
	}

	BOOL AddAmf3Object(THString sKey)
	{
		THAmf0 *tmp=new THAmf0;
		if (!tmp) return FALSE;
		tmp->nType=THAMF0_AMF3;
		tmp->sKey=sKey;
		return m_arObj.AddTail(tmp);
	}

	BOOL RemoveObject(THString sKey)
	{
		THAmf0 *tmp;
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

	THAmf0 *GetObject(THString sKey)
	{
		THAmf0 *tmp;
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
		THAmf0 *tmp=GetObject(sKey);
		if (tmp)
		{
			double Val;
			if (tmp->SafeGetNumber(Val)) return Val;
		}
		return 0.0;
	}

	BOOL GetBoolObject(THString sKey)
	{
		THAmf0 *tmp=GetObject(sKey);
		if (tmp)
		{
			BOOL Val;
			if (tmp->SafeGetBool(Val)) return Val;
		}
		return FALSE;
	}

	THString GetStringObject(THString sKey)
	{
		THAmf0 *tmp=GetObject(sKey);
		if (tmp)
		{
			THString Val;
			if (tmp->SafeGetString(Val)) return Val;
		}
		return _T("");
	}

	THString GetMoveClipObject(THString sKey)
	{
		THAmf0 *tmp=GetObject(sKey);
		if (tmp)
		{
			THString Val;
			if (tmp->SafeGetMovieClip(Val)) return Val;
		}
		return _T("");
	}

	THString GetRecordSetObject(THString sKey)
	{
		THAmf0 *tmp=GetObject(sKey);
		if (tmp)
		{
			THString Val;
			if (tmp->SafeGetRecordSet(Val)) return Val;
		}
		return _T("");
	}

	THString GetXmlDocObject(THString sKey)
	{
		THAmf0 *tmp=GetObject(sKey);
		if (tmp)
		{
			THString Val;
			if (tmp->SafeGetXmlDoc(Val)) return Val;
		}
		return _T("");
	}

	U16 GetReferenceObject(THString sKey)
	{
		THAmf0 *tmp=GetObject(sKey);
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
		THAmf0 *tmp=GetObject(sKey);
		if (tmp) tmp->SafeGetDate(Val);
		return Val;
	}

	LONG GetDateObjectTimeZone(THString sKey)
	{
		THAmf0 *tmp=GetObject(sKey);
		if (tmp)
		{
			LONG Val=0;
			if (tmp->SafeGetDateTimeZone(Val)) return Val;
		}
		return 0;
	}

	THAmf0Parser *GetObjectObject(THString sKey)
	{
		THAmf0 *tmp=GetObject(sKey);
		if (tmp)
		{
			void *Val;
			UINT nLen;
			if (tmp->SafeGetObject(Val,nLen))
			{
				return (THAmf0Parser *)Val;
			}
		}
		return NULL;
	}

	THAmf0Parser *GetArrayObject(THString sKey)
	{
		THAmf0 *tmp=GetObject(sKey);
		if (tmp)
		{
			void *Val;
			UINT nLen;
			if (tmp->SafeGetArray(&Val,nLen))
			{
				return (THAmf0Parser *)Val;
			}
		}
		return NULL;
	}

	//是否存在
	BOOL GetNullObject(THString sKey)
	{
		THAmf0 *tmp=GetObject(sKey);
		if (tmp) return tmp->IsNullValue();
		return FALSE;
	}

	//fixme get codeTHAMF0_AVMPLUS

	//获取全部对象列表
	//Format 格式：[key] 替换键名 [type] 替换键值类型 [value] 替换键值内容 [step] 替换递归级数 支持\r \n \t
	THString GetDetail(THString sFormat=_T("[step][key]([type]):[value]\r\n"),THString sStep=_T(" "))
	{
		THString ret;
		THAmf0 *tmp;
		THPosition pos=m_arObj.GetStartPosition();
		while(!pos.IsEmpty())
		{
			if (m_arObj.GetNextPosition(pos,tmp))
			{
				if (tmp)
				{
					if (tmp->IsObject())
					{
						THAmf0Parser *man=(THAmf0Parser *)tmp->pVal;
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
	THAmf0 *_GetBufObject(const char **retpch,UINT &nLen,BOOL bCheckEnd=FALSE,U8 nUseKeyType=THAMF0_UNDEFINE)
	{
		if (!retpch || !(*retpch) || nLen==0) return NULL;
		const char *pch=*retpch;
		U8 type;
		if (nUseKeyType==THAMF0_UNDEFINE)
		{
			type=*pch;
			pch++;nLen--;
		}
		else
			type=nUseKeyType;
		THAmf0 *tmp=new THAmf0;
		if (!tmp) return NULL;
		if (bCheckEnd)
		{
			if (nUseKeyType==THAMF0_UNDEFINE)
			{
				if (type==0 && (*pch)==0 && (*(pch+1))==9)
				{
					tmp->nType=THAMF0_OBJECTEND;
					pch+=2;nLen-=2;
					return tmp;
				}
			}
			else
			{
				if ((*pch)==0 && (*(pch+1))==0 && (*(pch+2))==9)
				{
					tmp->nType=THAMF0_OBJECTEND;
					pch+=3;nLen-=3;
					return tmp;
				}
			}
		}
		tmp->nType=type;
		switch(type)
		{
			case THAMF0_NUMBER:
			case THAMF0_DATE:
			{
				char ctmp[8];
				for(int i=0;i<8;i++)
					ctmp[i]=*(pch+7-i);
				double dVal;
				memcpy(&dVal,ctmp,8);
				pch+=8;nLen-=8;
				if (type==THAMF0_NUMBER)
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
			case THAMF0_BOOLEAN:
			{
				tmp->u8Val=*pch;
				pch++;nLen--;
				break;
			}
			case THAMF0_STRING:
			case THAMF0_MOVIECLIP:
			case THAMF0_RECORDSET:
			case THAMF0_XMLDOC:
			case THAMF0_LONGSTRING:
			{
				U32 strlen;
				if (type==THAMF0_LONGSTRING || type==THAMF0_XMLDOC)
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
			case THAMF0_OBJECT:
			case THAMF0_TYPED_OBJECT:
			case THAMF0_ECMAARRAY:
			case THAMF0_STRICTARRAY:
			{
				U32 arsize=0;
				if (type!=THAMF0_OBJECT)
				{
					//array size
					arsize=ntohl(*(const U32 *)pch);
					pch+=4;nLen-=4;
					//because of ecma must check the end 00 00 09,check it when arsize set to 0
					if (type==THAMF0_ECMAARRAY)
						arsize=0;
				}
				//object n,end of 00 00 09
				THAmf0Parser *man=new THAmf0Parser;
				if (!man)
				{
					delete tmp;
					return NULL;
				}
				UINT nRefLen=0;
				BOOL bCheckKey;
				if (type==THAMF0_STRICTARRAY)
					bCheckKey=FALSE;
				else
					bCheckKey=TRUE;
				if (!man->AppendAmf0Buffer(pch,nLen,&nRefLen,arsize,FALSE,bCheckKey))//本来最后参数传入arsize可限制数组只读取多少，但是有结束符，就可以忽略了
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
			case THAMF0_NULL:
			case THAMF0_AMF3:
				break;
			case THAMF0_UNDEFINE:
			{
				delete tmp;
				return NULL;
				break;
			}
			case THAMF0_REFERENCE:
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
				type=THAMF0_LONGSTRING;
			else
			{
				if (bMoveClip)
					type=THAMF0_MOVIECLIP;
				else
					type=THAMF0_STRING;
			}
			if (bAddKeyType) m_buf.AddBuf(&type,1);
			if (type==THAMF0_LONGSTRING)
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

	static void FreeAmf0(void *key,void *value,void *adddata)
	{
		THAmf0 *tmp=(THAmf0 *)value;
		if (tmp)
		{
			if (tmp->pVal && tmp->pValLen)
			{
				if (tmp->nType==THAMF0_OBJECT || tmp->nType==THAMF0_ECMAARRAY || tmp->nType==THAMF0_STRICTARRAY)
					delete (THAmf0Parser *)tmp->pVal;
				else
					//not use now
					delete [] tmp->pVal;
			}
			delete tmp;
		}
	}
	THList<THAmf0 *> m_arObj;
	THMemBuf m_buf;
};
