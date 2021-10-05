#pragma once
#include <THSystem.h>
#include <THString.h>
#include <THTime.h>

/**
* @brief Url�ֽ���
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-06-29 �½���
*/
/**<pre>
�÷���
	//֧�ַֽ�ͨ�ø�ʽUrl, type://[user[:pass]@]host[:port][/[file[.ext]][?key=[value][&key2=[value]]][#[tag]]]
	THUrlExplain exp;
	exp.Parse(_T("http://host.com/name"));
</pre>*/
class THUrlExplain
{
public:
	THUrlExplain(){port=0;}

	virtual ~THUrlExplain(){}

	//��ͬ��key����[i]������ʽ����
	static void Param2Map(THString param,map<THString,THString> *pmap){
		param=param.TrimLeft(_T("?"));
		THStringToken p(param,_T("&"));
		THString tmp,value;
		int pos;
		while(p.IsMoreTokens()) {
			tmp=p.GetNextToken();
			pos=tmp.Find('=');
			if (pos!=-1){
				value=tmp.Mid(pos+1);
				tmp=tmp.Left(pos);
				pair< map<THString,THString>::iterator, bool> it=pmap->insert(map<THString,THString>::value_type(tmp,value));
				int i=1;
				while (!it.second) {
					THString tmp1;
					tmp1.Format(_T("[%u]"),i++);
					it=pmap->insert(map<THString,THString>::value_type(tmp+tmp1,value));
				}
			}
		}
	}

	BOOL Parse(THString url)
	{
		//get "type"
		int pos=url.Find(_T("://"),0);
		if (pos==-1) return FALSE;
		fullurl=url;
		type=url.Left(pos);
		url=url.Mid(pos+3);
		//get "host" and "fullpath"
		THString tmphost;
		pos=url.Find(_T("/"),0);
		if (pos==-1)
		{
			tmphost=url;
			fullpath.Empty();
		}
		else
		{
			tmphost=url.Left(pos);
			fullpath=url.Mid(pos);
		}
		//get "user" and "pass"
		pos=tmphost.Find(_T("@"),0);
		if (pos==-1)
		{
			user.Empty();
			pass.Empty();
		}
		else
		{
			user=tmphost.Left(pos);
			tmphost=tmphost.Mid(pos+1);
			pos=user.Find(_T(":"),0);
			if (pos==-1)
				pass.Empty();
			else
			{
				pass=user.Mid(pos+1);
				user=user.Left(pos);
			}
		}
		//get "host" and "port"
		pos=tmphost.Find(_T(":"),0);
		if (pos==-1)
		{
			port=0;
			host=tmphost;
		}
		else
		{
			port=THStringConv::s2u(tmphost.Mid(pos+1));
			host=tmphost.Left(pos);
		}
		//get "requests" and "filepath" and "params" and "tag"
		requests=fullpath;
		pos=fullpath.Find(_T("#"),0);
		if (pos==-1)
			tag.Empty();
		else
		{
			tag=fullpath.Mid(pos+1);
			fullpath=fullpath.Left(pos);
		}
		pos=fullpath.Find(_T("?"),0);
		if (pos==-1)
			params.Empty();
		else
		{
			params=fullpath.Mid(pos+1);
			fullpath=fullpath.Left(pos);
		}
		//get "dir" "file" "ext"
		if (fullpath.IsEmpty() || fullpath.GetAt(fullpath.GetLength()-1)=='/' || (pos=fullpath.ReverseFind('/'))==-1)
		{
			dir=fullpath;
			file.Empty();
			ext.Empty();
		}
		else
		{
			dir=fullpath.Left(pos+1);
			file=fullpath.Mid(pos+1);
			pos=file.ReverseFind('.');
			if (pos==-1)
				ext.Empty();
			else
				ext=file.Mid(pos+1);
		}
		//get params
		parkey.RemoveAll();
		parval.RemoveAll();
		THStringToken token(params,_T("&"));
		while(1)
		{
			THString arg=token.GetNextToken();
			if (arg.IsEmpty()) break;
			pos=arg.Find(_T("="),0);
			if (pos==-1)
			{
				parkey.Add(arg);
				parval.Add(_T(""));
			}
			else
			{
				parkey.Add(arg.Left(pos).Trim());
				parval.Add(arg.Mid(pos+1).Trim());
			}
		}
		return TRUE;
	}

	/**
	* @brief ��ȡ����Url
	* @return ���ػ�ȡ�ַ���
	*/
	THString GetUrl(){return fullurl;}
	/**
	* @brief ��ȡЭ�����ͣ��緵�أ�http
	* @return ���ػ�ȡ�ַ���
	*/
	THString GetType(){return type;}
	/**
	* @brief ��ȡ�û���
	* @return ���ػ�ȡ�ַ���
	*/
	THString GetUser(){return user;}
	/**
	* @brief ��ȡ����
	* @return ���ػ�ȡ�ַ���
	*/
	THString GetPass(){return pass;}
	/**
	* @brief ��ȡ���������緵�أ�subdomain.domain.com
	* @return ���ػ�ȡ�ַ���
	*/
	THString GetHost(){return host;}
	/**
	* @brief ��ȡ�˿ںţ����û��ָ���˿ںţ�����ָ����Ĭ�϶˿ں�
	* @param defport	Ĭ�϶˿ں�
	* @return ���ػ�ȡ�ַ���
	*/
	unsigned int GetPort(unsigned int defport=80){return (port==0)?defport:port;}
	/**
	* @brief ��ȡ������������ַ������緵�أ�/somepath/somefile.ext?tag=val
	* @return ���ػ�ȡ�ַ���
	*/
	THString GetRequests(){return requests;}
	/**
	* @brief ��ȡ�����ļ������緵�أ�/somepath/somefile.ext
	* @return ���ػ�ȡ�ַ���
	*/
	THString GetFullPath(){return fullpath;}
	/**
	* @brief ��ȡ�ļ�����Ŀ¼����������'/'���緵�أ�/somepath/
	* @return ���ػ�ȡ�ַ���
	*/
	THString GetDir(){return dir;}
	/**
	* @brief ��ȡ�ļ������緵�أ�somefile.ext
	* @return ���ػ�ȡ�ַ���
	*/
	THString GetFile(){return file;}
	/**
	* @brief ��ȡ�ļ���׺�����緵�أ�txt
	* @return ���ػ�ȡ�ַ���
	*/
	THString GetExt(){return ext;}
	/**
	* @brief ��ȡê���� text.ext#tag ���أ�tag
	* @return ���ػ�ȡ�ַ���
	*/
	THString GetTag(){return tag;}
	/**
	* @brief ��ȡȫ���������緵�أ�key=val&key2=val2&key3=val3
	* @return ���ػ�ȡ�ַ���
	*/
	THString GetParams(){return params;}
	/**
	* @brief ��ȡָ��������Ӧ��ֵ����ָ��key1���أ�val1
	* @return ���ػ�ȡ�ַ���
	*/
	THString GetKeyVal(THString key)
	{
		for(int i=0;i<parkey.GetSize();i++)
		{
			if (parkey[i]==key)
				return parval[i];
		}
		return _T("");
	}
	/**
	* @brief ��ȡȫ��������Ӧ���ַ�������
	* @param key	����Ӧ������
	* @param val	��ֵ��Ӧ������
	* @return ���ػ�ȡ�ַ���
	*/
	void GetParamsArray(THStringArray **key,THStringArray **val){*key=&parkey;*val=&parval;}

	static char* UrlDecodeA(const char *tmp)
	{
		char *buf=new char[strlen(tmp)+1];
		if (buf)
		{
			char *tmpbuf=buf;
			const char *tmptmp=tmp;
			while(*tmptmp!='\0')
			{
				if (*tmptmp=='%')
				{
					if (*(tmptmp+1)=='%') {
						*tmpbuf='%';
						tmpbuf++;
						tmptmp+=2;
					} else {
						int hi = tolower(*(tmptmp+1));
						int lo = tolower(*(tmptmp+2));
						tmptmp+=3;
						if (hi>='a' && hi<='f') hi=hi-'a'+10;
						else hi=hi-'0';
						if (lo>='a' && lo<='f') lo=lo-'a'+10;
						else lo=lo-'0';
						*tmpbuf++=(char)(uint8_t)((hi<<4) | lo);
					}
				}
				else
				{
					*tmpbuf=*tmptmp;
					tmpbuf++;
					tmptmp++;
				}
			}
			*tmpbuf='\0';
		}
		return buf;
	}

	static FreeBuffer(char *buf)
	{
		delete [] buf;
	}

	/**
	* @brief ���� % ��ת��
	* @return ���ش������ַ���
	*/
	static THString UrlDecode(THString buffer)
	{
		THString ret;
		char *tmp=THCharset::t2a(buffer);
		if (tmp)
		{
			char *reta=UrlDecodeA(tmp);
			if (reta)
			{
				ret=THCharset::a2t(reta);
				FreeBuffer(reta);
			}
			THCharset::free(tmp);
		}
		return ret;
	}

	/**
	* @brief �Ѷ�Ӧ�ַ�תΪ % �ű�ʾ
	* @return ���ش������ַ���
	*/
	static THString UrlEncode(THString buffer)
	{
		THString ret;
		char *tmp=THCharset::t2a(buffer);
		if (tmp)
		{
			char *reta=UrlEncodeA(tmp);
			if (reta)
			{
				ret=THCharset::a2t(reta);
				FreeBuffer(reta);
			}
			THCharset::free(tmp);
		}
		return ret;
	}

	static char *UrlEncodeA(const char *tmp)
	{
		char *buf=new char[(strlen(tmp)+1)*3];
		if (buf)
		{
			char *tmpbuf=buf;
			const unsigned char *tmptmp=(const unsigned char *)tmp;
			while(*tmptmp!='\0')
			{
				if (isalnum(*tmptmp))
				{
					*tmpbuf=*tmptmp;
					tmpbuf++;
					tmptmp++;
				}
				else
				{
					sprintf(tmpbuf,"%%%02x",*tmptmp);
					tmpbuf+=3;
					tmptmp++;
				}
			}
			*tmpbuf='\0';
		}
		return buf;
	}

protected:
	THString fullurl;		///>����url
	THString type;			///>Э������
	THString user;			///>�û���
	THString pass;			///>����
	THString host;			///>������
	unsigned int port;		///>�˿ںţ�0Ϊû��ָ��
	THString requests;		///>������������ַ���
	THString fullpath;		///>����������ļ�·��
	THString dir;			///>Ŀ¼
	THString file;			///>�ļ�
	THString ext;			///>��׺��
	THString params;		///>���в���
	THStringArray parkey;	///>�����������
	THStringArray parval;	///>�����������
	THString tag;			///>ê
};

/**
* @brief http Cookie������
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2011-02-16 �½���
*/
/**<pre>
�÷���
</pre>*/
class THHttpCookieContainer
{
public:
	THHttpCookieContainer(){}
	virtual ~THHttpCookieContainer(){}

	void UpdateCookieValue(THString key,THString value,THString domain,THString path,THTime *expires)
	{
		THCookieContainer ct;
		THString id=domain;
		id.MakeReverse();//�ô���domain��С���ں��棬�ܸ��µ�������ɵ�ֵ
		id+=_T("+")+path+_T("+");
		id+=key;
		ct.key=key;ct.value=value;ct.domain=domain;ct.path=path;
		if (expires)
		{
			ct.expires=&ct.container;
			ct.container=*expires;
		}
		else
			ct.expires=NULL;
		m_cookies[id]=ct;
	}

	THString GetCookieString(THString requestfile,THString host,unsigned short port=80)
	{
		THString ret;
		map<THString,THCookieContainer>::iterator it=m_cookies.begin();
		while(it!=m_cookies.end())
		{
			THCookieContainer &ct=it->second;
			if (!ct.domain.IsEmpty())
			{
				if (host.Find(ct.domain)==-1)
				{
					it++;
					continue;
				}
			}
			if (!ct.path.IsEmpty())
			{
				requestfile.MakeLower();
				THString tmp=ct.path;
				tmp.MakeLower();
				if (requestfile.Find(tmp)!=0)
				{
					it++;
					continue;
				}
			}
			if (ct.expires)
			{
				THTime ti;
				if (*ct.expires < ti)
				{
					it++;
					continue;
				}
			}
			THSimpleXml::SetParam(&ret,ct.key,ct.value);
			it++;
		}
		return THSimpleXml::toString(ret,_T("KEY=VALUE; ")).TrimRight(_T("; "));
	}

	void EmptyCookie(THString key=_T(""),THString domain=_T(""))
	{
		//fixme
	}
protected:
	typedef struct _THCookieContainer{
		THString key;
		THString value;
		THString domain;
		THString path;
		THTime *expires;
		THTime container;
	}THCookieContainer;
	map<THString,THCookieContainer> m_cookies;///<keyֻ�����ж��Ƿ��Ѵ���
};

/**
* @brief httpͷ�����࣬����Request��Response�Ĵ���
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-08-05 �½���
*/
/**<pre>
�÷���
</pre>*/
class THHttpHeaderParser
{
public:
	THHttpHeaderParser(){m_ver=11;m_pcc=NULL;}
	virtual ~THHttpHeaderParser(){}

	BOOL Parse(THString Headers)
	{
		m_ver=0;
		m_headers.RemoveAll();
		m_fh.RemoveAll();
		THStringToken token(Headers,_T("\r\n"));
		int pos=0;
		THString str;
		THString header=token.GetNextToken();
		if (header.IsEmpty()) return FALSE;
		m_headers.Add(header);
		while(1)
		{
			str=token.GetNextToken();
			str.Trim();
			if (str.IsEmpty()) break;
			m_headers.Add(str);
		}
		THStringToken token2(header,_T(" "));
		while(1)
		{
			str=token2.GetNextToken();
			str.Trim();
			if (str.IsEmpty()) break;
			m_fh.Add(str);
		}
		if (header.ReverseFind('/')!=-1)
		{
			m_ver=(int)((_tstof(header.Mid(header.ReverseFind('/')+1)))*10.0);
		}
		if (m_pcc) ParseSetCookie(m_pcc);
		return TRUE;
	}

	/**
	* @brief ��ȡָ��ͷ��Ϣ�е�����
	* @param TagName		����
	* @param idx			��n�γ��ֵļ�ֵ����0��ʼ
	* @return �����ַ���
	*/
	THString GetTag(THString TagName,int idx=0)
	{
		TagName+=_T(":");
		for(int i=0;i<m_headers.GetSize();i++)
		{
			if (_tcsnicmp(m_headers[i],TagName,TagName.GetLength())==0)
			{
				if (idx<=0)
					return m_headers[i].Mid(TagName.GetLength()).Trim();
				idx--;
			}
		}
		return _T("");
	}

	/**
	* @brief ��ȡͷ��Ϣ����
	* @return ͷ��Ϣ����
	*/
	int GetHeaderSize(){return (int)(INT_PTR)m_headers.GetSize();}

	/**
	* @brief ��ȡָ����ŵ�ͷ��Ϣ
	* @param idx		��n��ͷ��Ϣ
	* @param key		���ؼ���
	* @return ��������
	*/
	THString GetHeader(int idx,THString &key)
	{
		THString ret;
		key.Empty();
		if (idx<m_headers.GetSize())
		{
			ret=m_headers[idx];
			int pos=ret.Find(_T(":"),0);
			if (pos!=-1)
			{
				key=ret.Left(pos).Trim();
				ret=ret.Mid(pos+1).Trim();
			}
		}
		return ret;
	}

	/**
	* @brief ��ȡHttp�е�GMTʱ�䴮ת��Ϊʱ��
	* @param str	ʱ�䴮
	* @return ʱ�䣬����ĵط��ɵ�ǰʱ�����
	*/
	static THTime GetGMTTimeToTime(THString str)
	{
		//Thu, 24 Jul 2008 05:15:02 GMT
		//Thu, 24-Jul-2008 05:15:02 GMT
		if (str.Find(_T(","))!=-1)
			str=str.Mid(str.Find(_T(","))+1);
		str.MakeLower();
		str.Replace(_T("gmt"),_T(""));
		str.Replace(_T("jan"),_T("1"));
		str.Replace(_T("feb"),_T("2"));
		str.Replace(_T("mar"),_T("3"));
		str.Replace(_T("apr"),_T("4"));
		str.Replace(_T("may"),_T("5"));
		str.Replace(_T("jun"),_T("6"));
		str.Replace(_T("jul"),_T("7"));
		str.Replace(_T("aug"),_T("8"));
		str.Replace(_T("sep"),_T("9"));
		str.Replace(_T("sept"),_T("9"));
		str.Replace(_T("oct"),_T("10"));
		str.Replace(_T("nov"),_T("11"));
		str.Replace(_T("dec"),_T("12"));
		str.Trim();
		THTime ti;
		if (str.Find(_T("-"))!=-1)
			ti.SetTime(str,_T("%d-%m-%Y %H:%M:%S"));
		else
			ti.SetTime(str,_T("%d %m %Y %H:%M:%S"));
		return ti;
	}

	/**
	* @brief ��ȡCache��Ϣ
	* @param ti				����޸�ʱ��
	* @param nFileLen	�ļ����ȣ�ȡ����Ϊ0
	* @return �Ƿ�ɹ���ȡ
	*/
	BOOL GetIfModifiedSince(THTime &ti,UINT &nFileLen)
	{
		//If-Modified-Since: Thu, 24 Jul 2008 05:15:02 GMT; length=62856
		THString tag=GetTag(_T("If-Modified-Since"));
		if (tag.IsEmpty()) return FALSE;
		THStringToken t(tag,_T(";"));
		THString st=t.GetNextToken();
		t.Init(t.GetNextToken(),_T("="));
		t.GetNextToken();
		THString len=t.GetNextToken();
		if (st.IsEmpty()) return FALSE;
		//check time
		ti=GetGMTTimeToTime(st);
		//check file length if have
		if (!len.IsEmpty())
			nFileLen=THs2u(len);
		else
			nFileLen=0;
		return TRUE;
	}

	/**
	* @brief ��ȡ���е�ͷ��Ϣ
	* @param idx		��n���ո�����Ϣ
	* @return ��������
	*/
	THString GetFirstHeader(int idx){return m_fh.GetSize()>idx?m_fh.GetAt(idx):_T("");}
	int GetVersion(){return m_ver;}
	THString GetHost(){return GetTag(_T("Host"));}
	THString GetAccept(){return GetTag(_T("Accept"));}
	BOOL IsAcceptEncodingGZip(){return (GetTag(_T("Accept-Encoding")).Find(_T("gzip"))!=-1);}
	THString GetUserAgent(){return GetTag(_T("User-Agent"));}
	THString GetConnectionType(){return GetTag(_T("Connection"));}
	THString GetContentLength(){return GetTag(_T("Content-Length"));}
	THString GetContentType(){return GetTag(_T("Content-Type"));}
	THString GetRange(){return GetTag(_T("Range"));}
	THString GetCookie(){return GetTag(_T("Cookie"));}
	void GetCookieToArray(THStringArray &ar)
	{
		THString cookie=GetTag(_T("Cookie"));
		THStringToken t(cookie,_T(";"));
		while(t.IsMoreTokens())
		{
			ar.Add(t.GetNextToken().Trim());
		}
	}
	THString GetCookieToXml()
	{
		THString ret;
		THStringArray ar;
		GetCookieToArray(ar);
		THStringToken t;
		THString key,value;
		for(int i=0;i<ar.GetSize();i++)
		{
			t.Init(ar[i],_T("="));
			key=t.GetNextToken().Trim();
			value=t.GetNextToken().Trim();
			if (!key.IsEmpty())
				ret+=THSimpleXml::MakeParam(key,value);
		}
		return ret;
	}
	BOOL GetRange(THString &unit,int &StartPos,int &EndPos)
	{
		THString range=GetRange();
		if (range.IsEmpty())
		{
			//default value
			StartPos=-1;
			EndPos=-1;
			return TRUE;
		}
		int off=range.Find(_T("="),0);
		if (off==-1) return FALSE;
		int off2=range.Find(_T("-"),off);
		if (off2==-1) return FALSE;
		unit=range.Left(off);
		unit.Trim();
		CString Start=range.Left(off2).Mid(off+1);
		Start.Trim();
		if (Start.IsEmpty())
			StartPos=-1;
		else
			StartPos=_ttoi(Start);
		CString End=range.Mid(off2+1);
		End.Trim();
		if (End.IsEmpty())
			EndPos=-1;
		else
			EndPos=_ttoi(End)+1;//index will +1
		return TRUE;
	}
	THString GetSetCookie(int idx=0){return GetTag(_T("Set-Cookie"),idx);}
	THString GetSetCookieToXml(int idx=0)
	{
		THString ret;
		THString org=GetSetCookie(idx);
		THStringToken t(org,_T(";"));
		while(t.IsMoreTokens())
		{
			org=t.GetNextToken().Trim();
			int pos;
			if ((pos=org.Find(_T("=")))!=-1)
				ret+=THSimpleXml::MakeParam(org.Left(pos),org.Mid(pos+1));
			else
			{
				if (org.CompareNoCase(_T("httponly"))!=-1)
					ret+=THSimpleXml::MakeParam(_T("httponly"),_T("1"));
				else if (org.CompareNoCase(_T("secure"))!=-1)
					ret+=THSimpleXml::MakeParam(_T("secure"),_T("1"));
			}
		}
		return ret;
	}
	void GetSetCookieToArray(THStringArray *value,THStringArray *key=NULL,int idx=0,THTime *expires=NULL,BOOL *useexpires=NULL,THString *path=NULL,THString *domain=NULL,BOOL *bSecure=NULL,BOOL *bHttpOnly=NULL)
	{
		if (!value) return;
		THString ret=GetSetCookieToXml(idx);
		THString sexp=THSimpleXml::GetAndRemoveParam(&ret,_T("expires"));
		THString spath=THSimpleXml::GetAndRemoveParam(&ret,_T("path"));
		THString sdomain=THSimpleXml::GetAndRemoveParam(&ret,_T("domain"));
		BOOL bSec=THSimpleXml::GetAndRemoveParam(&ret,_T("secure"))==_T("1");
		BOOL bOnly=THSimpleXml::GetAndRemoveParam(&ret,_T("httponly"))==_T("1");
		THSimpleXml::toArray(ret,key,value);
		if (useexpires) *useexpires=!sexp.IsEmpty();
		if (expires && !sexp.IsEmpty()) *expires=GetGMTTimeToTime(sexp);
		if (path) *path=spath;
		if (domain) *domain=sdomain;
		if (bSecure) *bSecure=bSec;
		if (bHttpOnly) *bHttpOnly=bOnly;
	}
	void ParseSetCookie(THHttpCookieContainer *ct)
	{
		int idx=0;
		int pos;
		THString domain,path,org,key,val;
		THTime *exp,cur;
		THStringArray arkey,arval;
		while(1)
		{
			org=GetSetCookie(idx);
			if (org.IsEmpty()) return;
			domain.Empty();path.Empty();exp=NULL;
			arkey.RemoveAll();arval.RemoveAll();
			THStringToken t(org,_T(";"));
			while(t.IsMoreTokens())
			{
				org=t.GetNextToken().Trim();
				if ((pos=org.Find(_T("=")))!=-1)
				{
					key=org.Left(pos);
					val=org.Mid(pos+1);
					if (key==_T("domain")) domain=val;
					else if (key==_T("path")) path=val;
					else if (key==_T("expires"))
					{
						exp=&cur;
						cur=GetGMTTimeToTime(val);
					}
					else
					{
						arkey.Add(key);
						arval.Add(val);
					}
				}
			}
			for(int i=0;i<arkey.GetSize();i++)
				ct->UpdateCookieValue(arkey[i],arval[i],domain,path,exp);
			idx++;
		}
	}
	void SetCookieContainer(THHttpCookieContainer *pcc){m_pcc=pcc;}
private:
	int m_ver;
	THStringArray m_headers;
	THStringArray m_fh;
	THHttpCookieContainer *m_pcc;
};

/**
* @brief http��Ӧͷ������
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-09-09 �½���
*/
/**<pre>
�÷���
</pre>*/
class THHttpHeaderResponseMaker
{
public:
	THHttpHeaderResponseMaker(){Reset();}
	virtual ~THHttpHeaderResponseMaker(){}

	void Reset()
	{
		m_TotalLen=0;
		m_StartPos=-1;
		m_EndPos=-1;
		m_RangeUnit=_T("bytes");
		m_Code=200;
		m_CodeText.Empty();
		m_HttpVer=11;
		m_RespType=_T("application/octet-stream");
		m_ServerName=_T("Tansoft TiniServer");
		//GMT time
		m_LastModify.GetCurTime(FALSE);
		m_bUseTime=TRUE;
		m_addheaders.Empty();
		m_addendheaders.Empty();
		m_ConnectionType=_T("Keep-Alive");
		m_ContentEncoding.Empty();
	}

	/**
	* @brief ���ӷ��ص�Cookie
	* @param keyvalue		�����ͼ�ֵ���磺key=value
	* @param expirestime	��ʱʱ�䣬��Ϊû�г�ʱʱ��
	* @param sPath			cookie��Ч��Χ��Ĭ��Ϊ·��/
	* @param sDomain		cookie������Χ���磺.tansoft.net��Ĭ�ϲ�����
	*/
	void AddCookie(THString keyvalue,THTime *expirestime=NULL,THString sPath=_T(""),THString sDomain=_T(""))
	{
		if (sPath.IsEmpty()) sPath=_T("/");
		if (expirestime) keyvalue+=expirestime->Format(_T("; expires=%a, %d %b %Y %H:%M:%S GMT"));
		keyvalue+=_T("; path=")+sPath;
		if (!sDomain.IsEmpty()) keyvalue+=_T("; domain=")+sDomain;
		AddHeaders(_T("Set-Cookie"),keyvalue,FALSE);
	}

	void AddETag(THString tag){AddHeaders(_T("ETag"),tag,TRUE);}
	void AddCacheControl(THString cachectrl=_T("max-age=315360000")){AddHeaders(_T("Cache-Control"),cachectrl,FALSE);}
	void AddExpires(THTime *expirestime)
	{
		THString expires;
		expires=expirestime->Format(_T("%a, %d %b %Y %H:%M:%S GMT;"));
		AddHeaders(_T("Expires"),expires,FALSE);
	}

	/**
	* @brief ���ӷ��ص�ͷ��Ϣ
	* @param key		����
	* @param value		��ֵ
	* @param bHead		�Ƿ�������ǰ��
	*/
	void AddHeaders(THString key,THString value,BOOL bHead=TRUE)
	{
		THString *str=&m_addheaders;
		if (!bHead) str=&m_addendheaders;
		if (str->IsEmpty())
			*str=key+_T(": ")+value;
		else
			str->Append(_T("\r\n")+key+_T(": ")+value);
		/*for(int i=0;i<m_key.GetSize();i++)
		{
			if (m_key[i].CompareNoCase(key)==0)
			{
				m_key[i]=key;
				m_value[i]=value;
				return;
			}
		}
		m_key.Add(key);
		m_value.Add(value);*/
	}

	/**
	* @brief ���÷������ݷ�Χ
	* @param TotalLen		�ܳ���
	* @param StartPos		��ʼλ�ã�-1Ϊ��ָ����-2Ϊ����*������http 416�������ʱEndPosΪ�ļ������ĳ���
	* @param EndPos			����λ�ã�-1Ϊ��ָ����-2Ϊ����*������http 416���
	* @param RangeUnit		���ȵ�λ
	*/
	void SetRange(int TotalLen,int StartPos=-1,int EndPos=-1,THString RangeUnit=_T("bytes"))
	{
		m_TotalLen=TotalLen;
		m_StartPos=StartPos;
		m_EndPos=EndPos;
		m_RangeUnit=RangeUnit;
	}

	/**
	* @brief ���÷��ؽ��
	* @param Code			�������
	* @param CodeText		�������
	* @param HttpVer		Э��汾
	*/
	void SetResult(int Code=200,THString CodeText=_T(""),int HttpVer=11)
	{
		m_Code=Code;
		m_CodeText=CodeText;
		m_HttpVer=HttpVer;
	}

	/**
	* @brief ������������
	* @param ConnectionType	��������
	*/
	void SetConnectionType(THString ConnectionType=_T("Close")){m_ConnectionType=ConnectionType;}

	/**
	* @brief ���÷����ļ���ʽ
	* @param RespType		�ļ���ʽ
	*/
	void SetResponseType(THString RespType){m_RespType=RespType;}

	/**
	* @brief ���ݺ�׺�����÷����ļ���ʽ
	* @param Ext			�ļ���׺��
	* @param DefType		Ĭ�Ϸ��ظ�ʽ
	*/
	void SetResponseTypeByExt(THString Ext,THString DefType=_T("application/octet-stream"))
	{
		TCHAR buf[][2][40]={
			{_T("doc"),_T("application/msword")},
			{_T("ogg"),_T("application/ogg")},
			{_T("pdf"),_T("application/pdf")},
			{_T("xls"),_T("application/vnd.ms-excel")},
			{_T("ppt"),_T("application/vnd.ms-powerpoint")},
			{_T("js"),_T("application/x-javascript")},
			{_T("swf"),_T("application/x-shockwave-flash")},
			{_T("tar"),_T("application/x-tar")},
			{_T("zip"),_T("application/zip")},
			{_T("mid"),_T("audio/midi")},
			{_T("mp3"),_T("audio/mpeg")},
			{_T("aif"),_T("audio/x-aiff")},
			{_T("m3u"),_T("audio/x-mpegurl")},
			{_T("ram"),_T("audio/x-pn-realaudio")},
			{_T("ra"),_T("audio/x-pn-realaudio")},
			{_T("rmf"),_T("audio/x-pn-realaudio")},
			{_T("rm"),_T("application/vnd.rn-realmedia")},
			{_T("rmvb"),_T("application/vnd.rn-realmedia")},
			{_T("wav"),_T("audio/x-wav")},
			{_T("bmp"),_T("image/bmp")},
			{_T("gif"),_T("image/gif")},
			{_T("jpg"),_T("image/jpeg")},
			{_T("png"),_T("image/png")},
			{_T("tif"),_T("image/tiff")},
			{_T("ico"),_T("image/x-icon")},
			{_T("css"),_T("text/css")},
			{_T("html"),_T("text/html")},
			{_T("shtml"),_T("text/html")},
			{_T("htm"),_T("text/html")},
			{_T("tsp"),_T("text/html")},
			{_T("xml"),_T("text/xml")},
			{_T("txt"),_T("text/plain")},
			{_T("rtf"),_T("text/rtf")},
			{_T("mpeg"),_T("video/mpeg")},
			{_T("mpg"),_T("video/mpeg")},
			{_T("mpe"),_T("video/mpeg")},
			{_T("qt"),_T("video/quicktime")},
			{_T("mov"),_T("video/quicktime")},
			{_T("moov"),_T("video/quicktime")},
			{_T("m4u"),_T("video/vnd.mpegurl")},
			{_T("avi"),_T("video/x-msvideo")},
			{_T("asx"),_T("video/x-ms-asf")},
			{_T("asf"),_T("video/x-ms-asf")},
			{_T("flv"),_T("video/x-flv")},
			{_T("pls"),_T("audio/x-scpls")},
			{_T("m3u"),_T("audio/x-mpegurl")},
			{_T("qtl"),_T("application/x-quicktimeplayer")},
			{_T("mpcpl"),_T("application/x-mpc-playlist")},
			{_T("\0"),_T("\0")}
		};
		int i=0;
		while(buf[i][0][0]!='\0')
		{
			if (Ext.CompareNoCase(buf[i][0])==0)
			{
				m_RespType=buf[i][1];
				return;
			}
			i++;
		}
		m_RespType=DefType;
	}

	/**
	* @brief ���÷���������
	* @param ServerName		����������
	*/
	void SetServer(THString ServerName){m_ServerName=ServerName;}

	/**
	* @brief �����޸�ʱ��
	* @param LastModify		�޸�ʱ��
	*/
	void SetModifyTime(const THTime &LastModify){m_LastModify=LastModify;}

	/**
	* @brief �����Ƿ񷵻�ʱ��
	* @param bUseTime		�Ƿ񷵻�ʱ��
	*/
	void SetUseTime(BOOL bUseTime=FALSE){m_bUseTime=bUseTime;}

	/**
	* @brief ���÷��ر����ʽ
	* @param encoding		Ϊ�գ�Ĭ��Ϊdeflate��������Ϊgzip
	*/
	void SetContentEncoding(THString encoding=_T("gzip")){m_ContentEncoding=encoding;}

	/**
	* @brief ����ͷ��Ϣ
	* @return ������������
	*/
	THString MakeHeaders()
	{
		THString ret;
		ret.Format(_T("HTTP/%0.1f %d %s\r\n"),m_HttpVer/10.0,m_Code,m_CodeText.IsEmpty()?GetCodeText(m_Code):m_CodeText);
		if (!m_addheaders.IsEmpty())
			ret+=m_addheaders+_T("\r\n");
		ret.AppendFormat(_T("Content-Type: %s\r\n"),m_RespType);
		//firefox ��Ӧ301ʱ�����û��content-length��һֱ�ȴ�
		//if (m_TotalLen>0)
		{
			if (m_StartPos==-2)
			{
				//http 416
				ret.AppendFormat(_T("Content-Length: %d\r\n"),m_TotalLen);//��ʱΪ������Ϣ�ĳ���
				ret.AppendFormat(_T("Content-Range: %s */%d\r\n"),m_RangeUnit,m_EndPos);//��ʱeΪ�ļ�����
				ret.AppendFormat(_T("Accept-Ranges: %s\r\n"),m_RangeUnit);
			}
			else if (m_StartPos!=-1 || m_EndPos!=-1)
			{
				int s,e;
				if (m_StartPos==-1)
					s=0;
				else
					s=m_StartPos;
				if (m_EndPos==-1)
					e=m_TotalLen;
				else
					e=m_EndPos;
				ret.AppendFormat(_T("Content-Length: %d\r\n"),e-s);
				ret.AppendFormat(_T("Content-Range: %s %d-%d/%d\r\n"),m_RangeUnit,s,e-1,m_TotalLen);//index will -1
				ret.AppendFormat(_T("Accept-Ranges: %s\r\n"),m_RangeUnit);
			}
			else
				ret.AppendFormat(_T("Content-Length: %d\r\n"),m_TotalLen);
		}
		if (!m_ContentEncoding.IsEmpty()) ret.AppendFormat(_T("Content-Encoding: %s\r\n"),m_ContentEncoding);
		ret.AppendFormat(_T("Server: %s\r\n"),m_ServerName);
		if (m_bUseTime) ret+=m_LastModify.Format(_T("Last-Modified: %a, %d %b %Y %H:%M:%S GMT\r\n"));
		ret.AppendFormat(_T("Connection: %s\r\n"),m_ConnectionType);
		if (m_bUseTime)
		{
			THTime now;
			//GMT time
			now.GetCurTime(FALSE);
			ret+=now.Format(_T("Date: %a, %d %b %Y %H:%M:%S GMT\r\n"));
		}
		if (!m_addendheaders.IsEmpty())
			ret+=m_addendheaders+_T("\r\n");
		ret+=_T("\r\n");
		return ret;
	}

	/**
	* @brief ��ȡ��Ӧ״̬�ķ�������
	* @param Code	����
	* @return ������������
	*/
	THString GetCodeText(int Code)
	{
		THString ret;
		switch(Code)
		{
			case 100:ret=_T("Continue");break;
			case 101:ret=_T("Switching Protocols");break;
			case 200:ret=_T("OK");break;
			case 201:ret=_T("Created");break;
			case 202:ret=_T("Accepted");break;
			case 203:ret=_T("Non-Authoritative Information");break;
			case 204:ret=_T("No Content");break;
			case 205:ret=_T("Reset Content");break;
			case 206:ret=_T("Partial Content");break;
			case 300:ret=_T("Multiple Choices");break;
			case 301:ret=_T("Moved Permanently");break;
			case 302:ret=_T("Moved Temporarily");break;
			case 303:ret=_T("See Other");break;
			case 304:ret=_T("Not Modified");break;
			case 305:ret=_T("Use Proxy");break;
			case 307:ret=_T("Temporary Redirect");break;
			case 400:ret=_T("Bad Request");break;
			case 401:ret=_T("Unauthorized");break;
			case 402:ret=_T("Payment Required");break;
			case 403:ret=_T("Forbidden");break;
			case 404:ret=_T("Not Found");break;
			case 405:ret=_T("Method Not Allowed");break;
			case 406:ret=_T("Not Acceptable");break;
			case 407:ret=_T("Proxy Authentication Required");break;
			case 408:ret=_T("Request Timeout");break;
			case 409:ret=_T("Conflict");break;
			case 410:ret=_T("Gone");break;
			case 411:ret=_T("Length Required");break;
			case 412:ret=_T("Precondition Failed");break;
			case 413:ret=_T("Request Entity Too Large");break;
			case 414:ret=_T("Request-URI Too Long");break;
			case 415:ret=_T("Unsupported Media Type");break;
			case 416:ret=_T("Requested Range Not Satisfiable");break;
			case 417:ret=_T("Expectation Failed");break;
			case 500:ret=_T("Internal Server Error");break;
			case 501:ret=_T("Not Implemented");break;
			case 502:ret=_T("Bad Gateway");break;
			case 503:ret=_T("Service Unavailable");break;
			case 504:ret=_T("Gateway Timeout");break;
			case 505:ret=_T("HTTP Version Not Supported");break;
			default:ret=_T("Unknown Response");
		}
		return ret;
	}
	int GetResultCode(){return m_Code;}
	THString GetResultText(){return m_CodeText;}
private:
	int m_TotalLen;
	int m_StartPos;
	int m_EndPos;
	THString m_RangeUnit;
	int m_Code;
	THString m_CodeText;
	int m_HttpVer;
	THString m_RespType;
	THString m_ServerName;
	THTime m_LastModify;
	THString m_ConnectionType;
	THString m_addheaders;
	THString m_addendheaders;
	THString m_ContentEncoding;
	BOOL m_bUseTime;
};

/**
* @brief Http��������
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2011-01-29 �½���
*/
/**<pre>
�÷���
</pre>*/
class THHttpHeaderRequestMaker
{
public:
	THHttpHeaderRequestMaker(){m_pcc=NULL;Reset();}
	virtual ~THHttpHeaderRequestMaker(){}

	//ע�⣬reset������reset CookieContainerָ��
	void Reset()
	{
		m_sMethod=_T("GET");
		m_sFile=_T("/");
		m_nHttpVer=11;
		m_sAccept=_T("image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, application/x-shockwave-flash, */*");
		m_sReferer.Empty();
		m_sAcceptLanguage=_T("zh-cn");
		m_sAcceptEncoding=_T("gzip, deflate");
		m_sUserAgent=_T("Tansoft TiniClient");
		m_sHost.Empty();
		m_sRange.Empty();
		m_sConnection=_T("Keep-Alive");
		m_sCookie.Empty();
		m_sAddHeaders.Empty();
		m_sAddEndHeaders.Empty();
	}

	THString MakeHeaders()
	{
		THString ret;
		ret=m_sMethod+_T(" ");
		ret+=m_sFile;
		ret.AppendFormat(_T(" HTTP/%0.1f\r\n"),m_nHttpVer);
		if (!m_sAddHeaders.IsEmpty()) ret+=m_sAddHeaders+_T("\r\n");
		if (!m_sAccept.IsEmpty()) ret.AppendFormat(_T("Accept: %s\r\n"),m_sAccept);
		if (!m_sReferer.IsEmpty()) ret.AppendFormat(_T("Referer: %s\r\n"),m_sReferer);
		if (!m_sAcceptLanguage.IsEmpty()) ret.AppendFormat(_T("Accept-Language: %s\r\n"),m_sAcceptLanguage);
		if (!m_sAcceptEncoding.IsEmpty()) ret.AppendFormat(_T("Accept-Encoding: %s\r\n"),m_sAcceptEncoding);
		if (!m_sUserAgent.IsEmpty()) ret.AppendFormat(_T("User-Agent: %s\r\n"),m_sUserAgent);
		if (!m_sHost.IsEmpty()) ret.AppendFormat(_T("Host: %s\r\n"),m_sHost);
		if (!m_sRange.IsEmpty()) ret.AppendFormat(_T("Range: %s\r\n"),m_sRange);
		if (!m_sConnection.IsEmpty()) ret.AppendFormat(_T("Connection: %s\r\n"),m_sConnection);
		if (!m_sCookie.IsEmpty()) ret.AppendFormat(_T("Cookie: %s\r\n"),m_sCookie);
		if (!m_sAddEndHeaders.IsEmpty()) ret+=m_sAddEndHeaders+_T("\r\n");
		ret+=_T("\r\n");
		return ret;
	}
	THString BuildRequest(THString url,THString *phost=NULL,unsigned short *pport=NULL)
	{
		THUrlExplain e;
		if (!e.Parse(url)) return _T("");
		if (phost) *phost=e.GetHost();
		if (pport) *pport=e.GetPort();
		return BuildRequest(e.GetRequests(),e.GetHost(),e.GetPort());
	}
	THString BuildRequest(THString requestfile,THString host,unsigned short port=80)
	{
		SetRequestFile(requestfile);
		if (port!=80) host.AppendFormat(_T(":%u"),port);
		SetHost(host);
		if (m_pcc) SetCookie(m_pcc->GetCookieString(requestfile,host,port));
		return MakeHeaders();
	}
	void EmptyAddHeaders(){m_sAddHeaders.Empty();m_sAddEndHeaders.Empty();}
	void AddHeaders(THString header,BOOL bEnd)
	{
		header.Trim(_T("\r\n"));
		if (header.IsEmpty()) return;
		THString *ret;
		if (bEnd)
			ret=&m_sAddEndHeaders;
		else
			ret=&m_sAddHeaders;
		if (!ret->IsEmpty())
			(*ret)+=_T("\r\n");
		(*ret)+=header;
	}
	void SetMethod(THString action=_T("GET")){m_sMethod=action;}
	void SetRequestFile(THString file=_T("/")){m_sFile=file;}
	void SetHttpVersion(int ver=11){m_nHttpVer=ver;}
	void SetAccept(THString accept=_T("*/*")){m_sAccept=accept;}
	void SetReferer(THString referer=_T("")){m_sReferer=referer;}
	void SetAcceptLanguage(THString acceptlang=_T("zh-cn")){m_sAcceptLanguage=acceptlang;}
	void SetAcceptEncoding(THString acceptenc=_T("deflate")){m_sAcceptEncoding=acceptenc;}
	void SetUserAgent(THString useragent=_T("")){m_sUserAgent=useragent;}
	void SetHost(THString host=_T("")){m_sHost=host;}
	void SetRange(THString range=_T("")){m_sRange=range;}
	void SetConnection(THString connection=_T("Keep-Alive")){m_sConnection=connection;}
	void SetCookie(THString cookie=_T("")){m_sCookie=cookie;}
	void SetCookieContainer(THHttpCookieContainer *pcc){m_pcc=pcc;}
private:
	THString m_sMethod;
	THString m_sFile;
	int m_nHttpVer;
	THString m_sAccept;
	THString m_sReferer;
	THString m_sAcceptLanguage;
	THString m_sAcceptEncoding;
	THString m_sUserAgent;
	THString m_sHost;
	THString m_sRange;
	THString m_sConnection;
	THString m_sCookie;
	THString m_sAddHeaders;
	THString m_sAddEndHeaders;
	THHttpCookieContainer *m_pcc;
};

/**
* @brief Http Cookie�Դ�����
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2011-01-29 �½���
*/
/**<pre>
�÷�������server���ص�Cookie��������Cookie��Ӧ������ͷ
</pre>*/
class THHttpHeaderCookieParser
{
public:
	THHttpHeaderCookieParser()
	{
		m_req.SetCookieContainer(&m_cc);
		m_parser.SetCookieContainer(&m_cc);
	}
	virtual ~THHttpHeaderCookieParser(){}
	THHttpHeaderRequestMaker *GetRequestMaker() {return &m_req;}
	THHttpHeaderParser *GetHeaderParser() {return &m_parser;}
	THString BuildRequest(THString url,THString *phost=NULL,unsigned short *pport=NULL) {return m_req.BuildRequest(url,phost,pport);}
	BOOL ParseResponse(THString headers) {return m_parser.Parse(headers);}
	void EmptyCookie(THString key=_T(""),THString domain=_T("")) {m_cc.EmptyCookie(key,domain);}
protected:
	THHttpHeaderRequestMaker m_req;
	THHttpHeaderParser m_parser;
	THHttpCookieContainer m_cc;
};

#define RANDOMINFO_KEY_BROWSERAGENT	_T("browseragent")			///<�����Agent
#define RANDOMINFO_KEY_BROWSERLANG	_T("browserlang")			///<���������
#define RANDOMINFO_KEY_SCREENSCALE	_T("screenscale")			///<��Ļ�߿�
#define RANDOMINFO_KEY_FLASHVER		_T("flashver")				///<Flash�汾 x.x.x.x ��ʽ
#define RANDOMINFO_KEY_FLASHVER2	_T("flashver2")				///<Flash�汾 x.x rx ��ʽ
#define RANDOMINFO_KEY_DAYUSER		_T("dayuser")				///<�û��շ�ֵ���ߣ�������ǰʱ���Ӧ���û�����ռȫ�ձ�ֵ
#define RANDOMINFO_KEY_DAYUSERPER	_T("dayuserper")			///<�û��շ�ֵ���ߣ��ó���ǰʱ���Ӧ���û��ۼ�ռȫ�ձ�ֵ
/**
* @brief �����Ϊ��
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2011-01-29 �½���
*/
/**<pre>
��������������ɣ����ɶ�Ӧ��Ϊ������ֵ
</pre>*/
class THRandomInfo
{
public:
	typedef map<THString,vector<THString> *> THRANDOMINFOMAP;
	THRandomInfo(){FillData(this);}
	virtual ~THRandomInfo(){}
	THString GetRandomInfo(THString key,THString addparam=_T(""))
	{
		THRANDOMINFOMAP::iterator it=m_info.find(key);
		if (it==m_info.end()) return _T("");
		vector<THString> *param=it->second;
		int idx=(int)(rand()%param->size());
		if (key==RANDOMINFO_KEY_DAYUSER || key==RANDOMINFO_KEY_DAYUSERPER)
		{
			THTime ti;
			idx=ti.GetHour();
			if (key==RANDOMINFO_KEY_DAYUSER)
			{
				double limit=THs2f((*param)[idx]);
				double lastlimit;
				if (idx>0) lastlimit=THs2f((*param)[idx-1]);
				else lastlimit=0.0;
				//�ó��÷���Ӧ�ô����limit��
				limit=lastlimit+(limit-lastlimit)*ti.GetMinute()/60;
				return THf2s(limit);
			}
		}
		return (*param)[idx];
	}
protected:
	typedef struct _THRANDOMINFOPAIR
	{
		int times;
		TCHAR info[256];
	}THRANDOMINFOPAIR;
	static void FillData(THRandomInfo *p)
	{
		//default: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; {})
		//IE7:     Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1; Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1) ; .NET CLR 1.1.4322; .NET CLR 2.0.50727; .NET CLR 3.0.04506.648; .NET CLR 3.5.21022; .NET CLR 3.0.4506.2152; .NET CLR 3.5.30729)
		//FireFox 3.0.6: Mozilla/5.0 (Windows; U; Windows NT 5.1; zh-CN; rv:1.9.0.6) Gecko/2009011913 Firefox/3.0.6 (.NET CLR 3.5.30729)
		//FireFox 3.6: Mozilla/5.0 (Windows; U; Windows NT 5.1; zh-CN; rv:1.9.2.12) Gecko/20101026 Firefox/3.6.12 (.NET CLR 3.5.30729)
		//Safari:  Mozilla/5.0 (Windows; U; Windows NT 5.1; zh-CN) AppleWebKit/533.18.1 (KHTML, like Gecko) Version/5.0.2 Safari/533.18.5
		//Chrome 7: Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US) AppleWebKit/534.7 (KHTML, like Gecko) Chrome/7.0.517.44 Safari/534.7
		static THRANDOMINFOPAIR ivagents[]={
			{6,_T("Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1 ; .NET CLR 1.1.4322; .NET CLR 2.0.50727;)")},//ie6
			{9,_T("Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1 ; Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1) ; .NET CLR 1.1.4322; .NET CLR 2.0.50727;)")},//ie7
			{27,_T("Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1 ; Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 6.0) ; .NET CLR 1.1.4322; .NET CLR 2.0.50727; .NET CLR 3.5.21022;)")},//ie8
			{24,_T("Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1 ; Mozilla/5.0 (Windows; U; Windows NT 5.1; zh-CN; rv:1.9.2.12) Gecko/20101026 Firefox/3.6.12 (.NET CLR 3.5.30729);)")},//FireFox 3.6
			{7,_T("Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1 ; Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US) AppleWebKit/534.7 (KHTML, like Gecko) Chrome/6.0.472.53 Safari/533.18.5)")},//Chrome 6
			{5,_T("Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1 ; Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US) AppleWebKit/534.7 (KHTML, like Gecko) Chrome/7.0.517.44 Safari/534.7)")},//Chrome 7
			{5,_T("Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1 ; Mozilla/5.0 (Windows; U; Windows NT 5.1; zh-CN) AppleWebKit/533.18.1 (KHTML, like Gecko) Version/5.0.2 Safari/533.18.5)")}//Safari 5
		};
		static THRANDOMINFOPAIR ivnavlang[]={{70,_T("zh-cn")},{25,_T("en-us")},{4,_T("zh-hk")},{1,_T("zh-tw")}};
		static THRANDOMINFOPAIR ivscreenhw[]={{43,_T("1024*768")},{15,_T("1280*800")},{14,_T("1440*900")},{6,_T("1280*1024")},{5,_T("1366*768")},{3,_T("1152*864")},{3,_T("1280*768")},{2,_T("1680*1050")},{1,_T("800*600")},{1,_T("640*480")}};
		static THRANDOMINFOPAIR ivflashver[]={{23,_T("10.1.102.64")},{17,_T("10.1.85.3")},{11,_T("10.1.82.76")},{13,_T("10.1.53.64")},{17,_T("10.0.45.2")},{8,_T("10.0.32.18")},{4,_T("10.0.42.34")},{3,_T("10.0.22.87")},{2,_T("10.0.12.36")},{1,_T("9.0.124.0")},{1,_T("9.0.115.0")}};
		p->FillData(RANDOMINFO_KEY_BROWSERAGENT,ivagents,sizeof(ivagents)/sizeof(THRANDOMINFOPAIR));
		p->FillData(RANDOMINFO_KEY_BROWSERLANG,ivnavlang,sizeof(ivnavlang)/sizeof(THRANDOMINFOPAIR));
		p->FillData(RANDOMINFO_KEY_SCREENSCALE,ivscreenhw,sizeof(ivscreenhw)/sizeof(THRANDOMINFOPAIR));
		p->FillData(RANDOMINFO_KEY_FLASHVER,ivflashver,sizeof(ivflashver)/sizeof(THRANDOMINFOPAIR));
		p->FillData(RANDOMINFO_KEY_FLASHVER2,ivflashver,sizeof(ivflashver)/sizeof(THRANDOMINFOPAIR));
		static THRANDOMINFOPAIR ivhourlimit[]={{1,_T("3")},{1,_T("5")},{1,_T("6.3")},{1,_T("7.5")},{1,_T("8.4")},{1,_T("9.3")},{1,_T("10.5")},{1,_T("12.2")},{1,_T("14.9")},
												{1,_T("19")},{1,_T("24.3")},{1,_T("30.1")},{1,_T("35.8")},{1,_T("41.9")},{1,_T("48")},{1,_T("54.2")},{1,_T("60.3")},
												{1,_T("66.2")},{1,_T("71.8")},{1,_T("77.1")},{1,_T("82.7")},{1,_T("88.9")},{1,_T("95.2")},{1,_T("100")}};
		static THRANDOMINFOPAIR ivhourlimitper[]={{1,_T("3")},{1,_T("2")},{1,_T("1.3")},{1,_T("1.2")},{1,_T("0.9")},{1,_T("0.9")},{1,_T("1.2")},{1,_T("1.7")},{1,_T("2.7")},
												{1,_T("4.1")},{1,_T("5.3")},{1,_T("5.8")},{1,_T("5.7")},{1,_T("6.1")},{1,_T("6.1")},{1,_T("6.2")},{1,_T("6.1")},
												{1,_T("6.1")},{1,_T("5.6")},{1,_T("5.3")},{1,_T("5.6")},{1,_T("6.2")},{1,_T("6.3")},{1,_T("4.8")}};
		p->FillData(RANDOMINFO_KEY_DAYUSER,ivhourlimit,sizeof(ivhourlimit)/sizeof(THRANDOMINFOPAIR));
		p->FillData(RANDOMINFO_KEY_DAYUSERPER,ivhourlimitper,sizeof(ivhourlimitper)/sizeof(THRANDOMINFOPAIR));
	}
	void FillData(THString key,THRANDOMINFOPAIR *data,int size)
	{
		vector<THString> *v=new vector<THString>;
		if (!v) return;
		for(int i=0;i<size;i++)
		{
			for(int j=0;j<(data+i)->times;j++)
			{
				THString str=(data+i)->info;
				if (key==RANDOMINFO_KEY_FLASHVER2)
				{
					int pos=str.ReverseFind('.');
					if (pos!=-1)
					{
						str=str.Left(pos);
						pos=str.ReverseFind('.');
						if (pos!=-1)
							str=str.Left(pos)+_T(" r")+str.Mid(pos+1);
					}
				}
				v->push_back(str);
			}
			if (key!=RANDOMINFO_KEY_DAYUSER && key!=RANDOMINFO_KEY_DAYUSERPER) random_shuffle(v->begin(),v->end());
		}
		m_info.insert(THRANDOMINFOMAP::value_type(key,v));
	}
	THRANDOMINFOMAP m_info;
};
