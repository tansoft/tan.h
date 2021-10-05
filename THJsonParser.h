#pragma once
#include <THStruct.h>
#include <THString.h>
#include <THFile.h>

/**
* @brief Json对象类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-06-30 新建类
* @2008-12-23 增加可多级存取对象的接口
* @2009-03-03 增加忽略处理没用到Json键值的参数
* @2009-03-12 优化查找器性能
*/
/**<pre>
  使用Sample：
</pre>*/
class THJsonObject
{
public:
	THJsonObject()
	{
		m_intobj.SetFreeProc(FreeObj);
		m_nObjType=0;
		m_sToken=_T("{}[]:,");	//object array value nextkey
	}

	virtual ~THJsonObject()
	{
		m_intobj.RemoveAll();
	}

	/**
	* @brief 对象中是否为字符内容，如 Name:1234 ，在上级的THJsonObject中可查key为Name的object，object内容为字符内容1234
	* @return 是否为字符内容
	*/
	inline BOOL IsValue(){return (m_nObjType==0);}

	/**
	* @brief 对象中是否为对象，如 Name:{key1:123,key2:124} ，在上级的THJsonObject中可查key为Name的object，object内容为对象
	* @return 是否为字符内容
	*/
	inline BOOL IsObject(){return (m_nObjType==1);}

	/**
	* @brief 对象中是否为数组，如 Name:[{key1:123,key2:124} ，在上级的THJsonObject中可查key为Name的object，object内容为数组
	* @return 是否为字符内容
	*/
	inline BOOL IsArray(){return (m_nObjType==2);}

	/**
	* @brief 返回对象内容
	* @return 对象为字符内容返回字符串，对象为object或者数组时返回表示的字符串
	*/
	THString GetContent()
	{
		if (IsValue())
		{
			THString str;
			str=m_sContent;
			str.Trim(_T("\""));
			return str;
		}
		return MakeJsonBuffer();
	}

	/**
	* @brief 返回对象内容
	* @return 对象为字符内容返回字符串，对象为object或者数组时返回表示的字符串
	*/
	operator THString()
	{
		return GetContent();
	}

	/**
	* @brief 优化的查找器
	*/
	BOOL MultiFind(THString str,int StartPos,int *retpos,THString *sFitStr)
	{
		int ret;
		if ((ret=str.FindOneOf(m_sToken))==-1) return FALSE;
		if (retpos) *retpos=ret+StartPos;
		if (sFitStr) sFitStr->Format(_T("%c"),str.GetAt(ret));
		return TRUE;
	}

	/**
	* @brief 对多重查找增加字符串判断
	*/
	BOOL MultiFindProxy(THString str,int StartPos,int *spos,THString *sFitStr)
	{
		str=str.Mid(StartPos);
		THString ret;
		int retpos=0;
		BOOL bRet=MultiFind(str,StartPos,&retpos,&ret);
		if (bRet==FALSE) return FALSE;
		int nret=str.FindOneOf(_T("\"'"));
		//证明字符串比关键字先出现
		if (nret!=-1 && nret+StartPos<retpos)
		{
			//查找字符串结尾
			THString hit;
			hit.Format(_T("%c"),str.GetAt(nret));
			//先出现"的字符串，先处理掉字符串
			while(1)
			{
				nret=str.Find(hit,nret+1);
				//没办法处理
				if (nret==-1) return FALSE;
				//如果是转义符，继续处理
				if (str.GetAt(nret-1)!='\\') break;//found it
			}
			//证明关键字在字符串中，需要跳过字符串再查找下一个关键字
			if (nret+StartPos>retpos)
			{
				str=str.Mid(nret);
				StartPos+=nret;
				return MultiFind(str,StartPos,spos,sFitStr);
			}
		}
		*spos=retpos;
		*sFitStr=ret;
		return TRUE;
	}

	/**
	* @brief 返回多级对象的内容
	* @param multikey 多级对象，如 _T("data.title.list[0].key")
	* @return 返回内容，出错返回空
	*/
	THString GetMultiObjectContent(THString multikey)
	{
		THJsonObject *obj=this;
		THStringToken t(multikey,_T("."));
		THString next,next1;
		next=t.GetNextToken();
		while(t.IsMoreTokens() && obj)
		{
			next1=t.GetNextToken();
			if (next1.IsEmpty()) return _T("");
			int pos=next.Find(_T("["),0);
			int pos2;
			if (pos!=-1 && (pos2=next.Find(_T("]"),pos))!=-1)
			{
				obj=obj->GetObject(next.Left(pos));
				if (obj)
					obj=obj->GetAt(THs2i(next.Left(pos2).Mid(pos+1)));
			}
			else
				obj=obj->GetObject(next);
			next=next1;
		}
		if (next.IsEmpty() || obj==NULL) return _T("");
		return obj->GetObjectContent(next);
	}

	/**
	* @brief 返回对象内容
	* @param sKey	对象键名
	* @return 返回对象sKey的object指针
	*/
	THJsonObject *GetObject(THString sKey)
	{
		ASSERT(IsObject());
		return m_intobj.GetAt(sKey);
	}

	/**
	* @brief 返回对象内容
	* @param sKey	对象键名
	* @return 对象对象sKey的字符内容
	*/
	THString GetObjectContent(THString sKey)
	{
		THJsonObject *obj=GetObject(sKey);
		if (obj) return obj->GetContent();
		return _T("");
	}

	/**
	* @brief 返回当前对象数组的元素内容
	* @param nIdx	数组下标
	* @return 对象对象sKey的字符内容
	*/
	THJsonObject *GetAt(UINT nIdx)
	{
		ASSERT(IsArray());
		return m_intobj.GetAt(THFormat(_T("%u"),nIdx));
	}

	/**
	* @brief 返回当前对象数组的元素内容
	* @param nIdx	数组下标
	* @return 对象对象sKey的字符内容
	*/
	THJsonObject * operator [](UINT nIdx)
	{
		ASSERT(IsArray());
		return m_intobj.GetAt(THFormat(_T("%u"),nIdx));
	}

	/**
	* @brief 处理字符串，转化为object
	* @param sBuf	需处理的字符串，必需传入一对{}中的内容，或者直接的字符类型
	* @return 已处理字节,返回-1为解释出错
	*/
	int ParseJsonBuffer(THString sBuf,THString sNotIngoreList=_T(""),BOOL bIngoreing=FALSE)
	{
		m_sLastError=_T("内存错误");
		m_nObjType=0;
		m_intobj.RemoveAll();
		THJsonObject *obj=NULL;
		int retpos;
		int lastpos=0;
		THString fs;
		THString lastkey;
		BOOL bStart1=FALSE;		//开始大括号
		BOOL bStart2=FALSE;		//开始中括号
		if (MultiFindProxy(sBuf,lastpos,&retpos,&fs)==FALSE)
		{
			//only the string
			if (!bIngoreing)
				m_sContent=_ParseValue(sBuf);
			m_sLastError=_T("成功");
			return sBuf.GetLength();
		}
		if (fs==_T(",") || fs==_T("]") || fs==_T("}"))
		{
			if (THStringConv::IsStringVisiableAscii(sBuf.Left(retpos)))
			{
				//just the content
				if (!bIngoreing)
					m_sContent=_ParseValue(sBuf.Left(retpos));
				m_sLastError=_T("成功");
				return retpos;
			}
			m_sLastError=_T("结束符不匹配");
			return -1;
		}
		while(MultiFindProxy(sBuf,lastpos,&retpos,&fs))
		{
			if (fs==_T("{"))
			{
				if (bStart1==FALSE && bStart2==FALSE)
				{
					//开始自身{}处理
					bStart1=TRUE;
					lastpos=retpos+1;
				}
				else if (bStart2==TRUE)
				{
					//开始数组元素处理
					obj=new THJsonObject;
					if (!obj) return -1;
					int ret=obj->ParseJsonBuffer(sBuf.Mid(retpos),sNotIngoreList,bIngoreing);
					if (ret==-1)
					{
						m_sLastError=obj->GetLastError();
						delete obj;
						return -1;
					}
					if (bIngoreing)
						delete obj;
					else
						m_intobj.SetAt(THFormat(_T("%u"),m_intobj.GetCount()),obj);
					lastpos=retpos+ret;
				}
				else
				{
					m_sLastError=_T("不应有连续的{{");
					return -1;
				}
			}
			else if (fs==_T("}"))
			{
				if (bStart1==TRUE && bStart2==FALSE)
				{
					//finish
					bStart1=FALSE;
					m_sLastError=_T("成功");
					m_nObjType=1;
					return retpos+1;
				}
				m_sLastError=_T("出现不匹配的}");
				return -1;
			}
			else if (fs==_T("["))
			{
				if (bStart1!=FALSE)
				{
					m_sLastError=_T("[开始前不应该有{");
					return -1;
				}
				if (bStart2!=FALSE)
				{
					m_sLastError=_T("不应有连续的[[");
					return -1;
				}
				bStart2=TRUE;
				lastpos=retpos+1;
			}
			else if (fs==_T("]"))
			{
				if (bStart1!=FALSE)
				{
					m_sLastError=_T("数组内未出现}就结束");
					return -1;
				}
				//finish
				bStart2=FALSE;
				m_nObjType=2;
				m_sLastError=_T("成功");
				return retpos+1;
			}
			else if (fs==_T(":"))
			{
				//get key
				THString key=_ParseValue(sBuf.Left(retpos).Mid(lastpos),TRUE);
				BOOL bSubKeyIngore=bIngoreing;
				if (bSubKeyIngore==FALSE && sNotIngoreList.IsEmpty()==FALSE)
				{
					if (sNotIngoreList.Find(_T("[")+key+_T("]"))==-1)
						bSubKeyIngore=TRUE;
				}
				obj=new THJsonObject;
				if (!obj) return -1;
				int ret=obj->ParseJsonBuffer(sBuf.Mid(retpos+1),sNotIngoreList,bSubKeyIngore);
				if (ret==-1)
				{
					m_sLastError=obj->GetLastError();
					delete obj;
					return -1;
				}
				if (bSubKeyIngore)
					delete obj;
				else
					m_intobj.SetAt(key,obj);
				key.Empty();
				lastpos=retpos+1+ret;
			}
			else if (fs==_T(","))
			{
				//not thing to do
				lastpos=retpos+1;
			}
		}
		m_sLastError=_T("缺少结束符");
		return -1;
	}

	/**
	* @brief 生成对象的字符串
	* @return 对象的字符串
	*/
	THString MakeJsonBuffer()
	{
		if (m_nObjType==0)
		{
			THString out;
			out=m_sContent;
			if (out.Left(1)==_T("\""))
			{
				out.Trim(_T("\""));
				out=_T("\"")+THStringConv::EncodeUString(out)+_T("\"");
			}
			return out;
		}
		THString ret;
		//object
		THString key;
		THJsonObject *obj;
		THPosition pos=m_intobj.GetStartPosition();
		while(!pos.IsEmpty())
		{
			if (m_intobj.GetNextPosition(pos,key,obj))
			{
				if (obj)
				{
					if (!ret.IsEmpty()) ret+=_T(",");
					if (m_nObjType==1)
						ret.AppendFormat(_T("\"%s\":%s"),key,obj->MakeJsonBuffer());
					else if (m_nObjType==2)
						ret.AppendFormat(_T("%s"),obj->MakeJsonBuffer());
				}
			}
		}
		if (m_nObjType==1)
			return _T("{")+ret+_T("}");
		else if (m_nObjType==2)
			return _T("[")+ret+_T("]");
		return ret;
	}

	/**
	* @brief 打印对象内容
	* @return 对象内容字符串
	*/
	THString PrintDetail(THString sPre=_T(" "),THString sLevel=_T(""))
	{
		THString ret;
		//ret.Format(_T("%s\r\n"),(m_nObjType==0)?m_sContent:((m_nObjType==1)?_T("对象"):_T("数组"))));
		if (m_nObjType==0)
		{
			THString str=m_sContent;
			str.Trim(_T("\""));
			ret.Format(_T("%s\r\n"),str);
		}
		else
		{
			THPosition pos=m_intobj.GetStartPosition();
			THJsonObject *obj;
			THString key;
			THString sFormat;
			ret+=_T("{\r\n");
			if (IsArray())
				sFormat=_T("%s[%s]:%s");
			else
				sFormat=_T("%s%s=%s");
			while(!pos.IsEmpty())
			{
				if (m_intobj.GetNextPosition(pos,key,obj))
				{
					if (obj)
					{
						ret.AppendFormat(sFormat,sPre+sLevel,key,obj->PrintDetail(sPre,sPre+sLevel));
					}
				}
			}
			ret+=sLevel+_T("}\r\n");
		}
		return ret;
	}

	/**
	* @brief 返回解释错误码
	* @return 解释错误码
	*/
	THString GetLastError() {return m_sLastError;}
protected:
	static void FreeObj(void *key,void *value,void *adddata)
	{
		delete (THJsonObject *)value;
	}
	static THString _ParseValue(THString sBuf,BOOL bTrim=FALSE)
	{
		if (bTrim) sBuf.Trim(_T("\""));
		sBuf=THStringConv::DecodeUString(sBuf);
		return sBuf;
	}
	THStringMap<THJsonObject *> m_intobj;
	int m_nObjType;	//0为字符串，1为object，2为数组
	THString m_sContent;
	THString m_sToken;
	THString m_sLastError;
};

/**
* @brief Json解释类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-06-30 新建类
*/
/**<pre>
  使用Sample：
</pre>*/
class THJsonParser
{
public:
	THJsonParser()
	{
	}

	virtual ~THJsonParser()
	{
	}

	THJsonObject *ParseJsonBuffer(THString sBuf,CString sNotIngoreList=_T(""))
	{
		THJsonObject *obj=new THJsonObject;
		if (obj)
		{
			int ret=obj->ParseJsonBuffer(sBuf,sNotIngoreList,FALSE);
			if (ret==-1)
			{
				m_sLastError=obj->GetLastError();
				delete obj;
				obj=NULL;
			}
		}
		return obj;
	}

	void FreeObject(THJsonObject *obj){delete obj;}
	THString GetLastError(){return m_sLastError;}
protected:
	THString m_sLastError;
};