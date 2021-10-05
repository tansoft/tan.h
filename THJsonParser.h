#pragma once
#include <THStruct.h>
#include <THString.h>
#include <THFile.h>

/**
* @brief Json������
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-06-30 �½���
* @2008-12-23 ���ӿɶ༶��ȡ����Ľӿ�
* @2009-03-03 ���Ӻ��Դ���û�õ�Json��ֵ�Ĳ���
* @2009-03-12 �Ż�����������
*/
/**<pre>
  ʹ��Sample��
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
	* @brief �������Ƿ�Ϊ�ַ����ݣ��� Name:1234 �����ϼ���THJsonObject�пɲ�keyΪName��object��object����Ϊ�ַ�����1234
	* @return �Ƿ�Ϊ�ַ�����
	*/
	inline BOOL IsValue(){return (m_nObjType==0);}

	/**
	* @brief �������Ƿ�Ϊ������ Name:{key1:123,key2:124} �����ϼ���THJsonObject�пɲ�keyΪName��object��object����Ϊ����
	* @return �Ƿ�Ϊ�ַ�����
	*/
	inline BOOL IsObject(){return (m_nObjType==1);}

	/**
	* @brief �������Ƿ�Ϊ���飬�� Name:[{key1:123,key2:124} �����ϼ���THJsonObject�пɲ�keyΪName��object��object����Ϊ����
	* @return �Ƿ�Ϊ�ַ�����
	*/
	inline BOOL IsArray(){return (m_nObjType==2);}

	/**
	* @brief ���ض�������
	* @return ����Ϊ�ַ����ݷ����ַ���������Ϊobject��������ʱ���ر�ʾ���ַ���
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
	* @brief ���ض�������
	* @return ����Ϊ�ַ����ݷ����ַ���������Ϊobject��������ʱ���ر�ʾ���ַ���
	*/
	operator THString()
	{
		return GetContent();
	}

	/**
	* @brief �Ż��Ĳ�����
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
	* @brief �Զ��ز��������ַ����ж�
	*/
	BOOL MultiFindProxy(THString str,int StartPos,int *spos,THString *sFitStr)
	{
		str=str.Mid(StartPos);
		THString ret;
		int retpos=0;
		BOOL bRet=MultiFind(str,StartPos,&retpos,&ret);
		if (bRet==FALSE) return FALSE;
		int nret=str.FindOneOf(_T("\"'"));
		//֤���ַ����ȹؼ����ȳ���
		if (nret!=-1 && nret+StartPos<retpos)
		{
			//�����ַ�����β
			THString hit;
			hit.Format(_T("%c"),str.GetAt(nret));
			//�ȳ���"���ַ������ȴ�����ַ���
			while(1)
			{
				nret=str.Find(hit,nret+1);
				//û�취����
				if (nret==-1) return FALSE;
				//�����ת�������������
				if (str.GetAt(nret-1)!='\\') break;//found it
			}
			//֤���ؼ������ַ����У���Ҫ�����ַ����ٲ�����һ���ؼ���
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
	* @brief ���ض༶���������
	* @param multikey �༶������ _T("data.title.list[0].key")
	* @return �������ݣ������ؿ�
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
	* @brief ���ض�������
	* @param sKey	�������
	* @return ���ض���sKey��objectָ��
	*/
	THJsonObject *GetObject(THString sKey)
	{
		ASSERT(IsObject());
		return m_intobj.GetAt(sKey);
	}

	/**
	* @brief ���ض�������
	* @param sKey	�������
	* @return �������sKey���ַ�����
	*/
	THString GetObjectContent(THString sKey)
	{
		THJsonObject *obj=GetObject(sKey);
		if (obj) return obj->GetContent();
		return _T("");
	}

	/**
	* @brief ���ص�ǰ���������Ԫ������
	* @param nIdx	�����±�
	* @return �������sKey���ַ�����
	*/
	THJsonObject *GetAt(UINT nIdx)
	{
		ASSERT(IsArray());
		return m_intobj.GetAt(THFormat(_T("%u"),nIdx));
	}

	/**
	* @brief ���ص�ǰ���������Ԫ������
	* @param nIdx	�����±�
	* @return �������sKey���ַ�����
	*/
	THJsonObject * operator [](UINT nIdx)
	{
		ASSERT(IsArray());
		return m_intobj.GetAt(THFormat(_T("%u"),nIdx));
	}

	/**
	* @brief �����ַ�����ת��Ϊobject
	* @param sBuf	�账����ַ��������贫��һ��{}�е����ݣ�����ֱ�ӵ��ַ�����
	* @return �Ѵ����ֽ�,����-1Ϊ���ͳ���
	*/
	int ParseJsonBuffer(THString sBuf,THString sNotIngoreList=_T(""),BOOL bIngoreing=FALSE)
	{
		m_sLastError=_T("�ڴ����");
		m_nObjType=0;
		m_intobj.RemoveAll();
		THJsonObject *obj=NULL;
		int retpos;
		int lastpos=0;
		THString fs;
		THString lastkey;
		BOOL bStart1=FALSE;		//��ʼ������
		BOOL bStart2=FALSE;		//��ʼ������
		if (MultiFindProxy(sBuf,lastpos,&retpos,&fs)==FALSE)
		{
			//only the string
			if (!bIngoreing)
				m_sContent=_ParseValue(sBuf);
			m_sLastError=_T("�ɹ�");
			return sBuf.GetLength();
		}
		if (fs==_T(",") || fs==_T("]") || fs==_T("}"))
		{
			if (THStringConv::IsStringVisiableAscii(sBuf.Left(retpos)))
			{
				//just the content
				if (!bIngoreing)
					m_sContent=_ParseValue(sBuf.Left(retpos));
				m_sLastError=_T("�ɹ�");
				return retpos;
			}
			m_sLastError=_T("��������ƥ��");
			return -1;
		}
		while(MultiFindProxy(sBuf,lastpos,&retpos,&fs))
		{
			if (fs==_T("{"))
			{
				if (bStart1==FALSE && bStart2==FALSE)
				{
					//��ʼ����{}����
					bStart1=TRUE;
					lastpos=retpos+1;
				}
				else if (bStart2==TRUE)
				{
					//��ʼ����Ԫ�ش���
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
					m_sLastError=_T("��Ӧ��������{{");
					return -1;
				}
			}
			else if (fs==_T("}"))
			{
				if (bStart1==TRUE && bStart2==FALSE)
				{
					//finish
					bStart1=FALSE;
					m_sLastError=_T("�ɹ�");
					m_nObjType=1;
					return retpos+1;
				}
				m_sLastError=_T("���ֲ�ƥ���}");
				return -1;
			}
			else if (fs==_T("["))
			{
				if (bStart1!=FALSE)
				{
					m_sLastError=_T("[��ʼǰ��Ӧ����{");
					return -1;
				}
				if (bStart2!=FALSE)
				{
					m_sLastError=_T("��Ӧ��������[[");
					return -1;
				}
				bStart2=TRUE;
				lastpos=retpos+1;
			}
			else if (fs==_T("]"))
			{
				if (bStart1!=FALSE)
				{
					m_sLastError=_T("������δ����}�ͽ���");
					return -1;
				}
				//finish
				bStart2=FALSE;
				m_nObjType=2;
				m_sLastError=_T("�ɹ�");
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
		m_sLastError=_T("ȱ�ٽ�����");
		return -1;
	}

	/**
	* @brief ���ɶ�����ַ���
	* @return ������ַ���
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
	* @brief ��ӡ��������
	* @return ���������ַ���
	*/
	THString PrintDetail(THString sPre=_T(" "),THString sLevel=_T(""))
	{
		THString ret;
		//ret.Format(_T("%s\r\n"),(m_nObjType==0)?m_sContent:((m_nObjType==1)?_T("����"):_T("����"))));
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
	* @brief ���ؽ��ʹ�����
	* @return ���ʹ�����
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
	int m_nObjType;	//0Ϊ�ַ�����1Ϊobject��2Ϊ����
	THString m_sContent;
	THString m_sToken;
	THString m_sLastError;
};

/**
* @brief Json������
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-06-30 �½���
*/
/**<pre>
  ʹ��Sample��
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