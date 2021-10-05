#pragma once

#include <THSystem.h>
#include <THString.h>

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

/**
* @brief 字符集转换类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-05-30 新建类
* @2007-07-04 增加utf8支持
*/
/**<pre>
  使用Sample：
	char tmpbuffer[MAX_PATH];
	THCharset::t2a(FilePath,tmpbuffer,MAX_PATH);
*/
class THCharset
{
public:
	THCharset(){}
	virtual ~THCharset(){}

	/**
	* @brief 确定字符串是否为UTF-8编码
	* @param src		源字符串
	* @return 是否为UTF8
	*/
	/**
		因为UTF-8编码有个规则:
			UNICODE编码表中编码小于0X7F的UTF-8编码为其本身(占一个字节),
			>=0x80的其高位连续的1的位数表示本字符占用的字节数,其他字节均以10开头,
			因此其他字符均编码为1110xxxx   10xxxxxx(二进制)的形式。
			按照这个规则遍历文本,大概可以判断是否UTF-8编码了,但显然不能十分确定
	*/
	static BOOL IsCharsetMayBeUTF8(const char *src)
	{
		int len=(int)strlen(src);
		const unsigned char *s=(const unsigned char *)src;
		for(int i=0;i<len;i++)
		{
			unsigned char ch=*(s+i);
			if (ch&0x80)
			{
				if ((ch&0xF0)!=0xE0 && (ch&0xC0)!=0x80)
					return FALSE;
			}
		}
		return TRUE;
	}

	/**
	* @brief 转换A类型为W类型
	* @param src		源字符串
	* @param dist		目标字符串
	* @param distsize	目标字符串缓冲大小，单位为字符个数
	* @return 是否成功
	*/
	static BOOL a2w(const char *src,wchar_t *dist,int distsize)
	{
		*dist='\0';
		return MultiByteToWideChar(CP_OEMCP,NULL,src,-1,dist,distsize);
	}

	/**
	* @brief 转换A类型为W类型，限制源长度
	* @param src		源字符串
	* @param srcsize	源字符串大小
	* @param dist		目标字符串
	* @param distsize	目标字符串缓冲大小，单位为字符个数
	* @return 是否成功
	*/
	static BOOL a2w(const char *src,unsigned int srcsize,wchar_t *dist,unsigned int distsize)
	{
		*dist='\0';
		int ret=MultiByteToWideChar(CP_OEMCP,NULL,src,srcsize,dist,distsize);
		dist[ret]='\0';
		return ret;
	}

	/**
	* @brief 转换A类型为Utf8类型
	* @param src		源字符串
	* @param dist		目标字符串
	* @param distsize	目标字符串缓冲大小，单位为字符个数
	* @return 是否成功
	*/
	static BOOL a2u8(const char *src,char *dist,int distsize)
	{
		*dist='\0';
		wchar_t *tmp=new wchar_t[(distsize+1)*3];
		if (!a2w(src,tmp,(distsize+1)*3))
		{
			delete [] tmp;
			return FALSE;
		}
		BOOL ret=w2u8(tmp,dist,distsize);
		delete [] tmp;
		return ret;
	}

	/**
	* @brief 转换Utf8类型为A类型
	* @param src		源字符串
	* @param dist		目标字符串
	* @param distsize	目标字符串缓冲大小，单位为字符个数
	* @return 是否成功
	*/
	static BOOL u82a(const char *src,char *dist,int distsize)
	{
		*dist='\0';
		wchar_t *tmp=new wchar_t[(distsize+1)*3];
		if (!u82w(src,tmp,(distsize+1)*3))
		{
			delete [] tmp;
			return FALSE;
		}
		BOOL ret=w2a(tmp,dist,distsize);
		delete [] tmp;
		return ret;
	}

	/**
	* @brief 转换Utf8类型为A类型
	* @param src		源字符串
	* @param srcsize	源字符串大小
	* @param dist		目标字符串
	* @param distsize	目标字符串缓冲大小，单位为字符个数
	* @return 是否成功
	*/
	static BOOL u82a(const char *src,unsigned int srcsize,char *dist,int distsize)
	{
		*dist='\0';
		wchar_t *tmp=new wchar_t[(distsize+1)*3];
		if (!u82w(src,srcsize,tmp,(distsize+1)*3))
		{
			delete [] tmp;
			return FALSE;
		}
		BOOL ret=w2a(tmp,dist,distsize);
		delete [] tmp;
		return ret;
	}

	/**
	* @brief 转换Utf8类型为W类型
	* @param src		源字符串
	* @param dist		目标字符串
	* @param distsize	目标字符串缓冲大小，单位为字符个数
	* @return 是否成功
	*/
	static BOOL u82w(const char *src,wchar_t *dist,int distsize)
	{
		*dist='\0';
		return MultiByteToWideChar(CP_UTF8,0,src,-1,dist,distsize);
	}

	/**
	* @brief 转换Utf8类型为W类型
	* @param src		源字符串
	* @param srclen		源字符串大小
	* @param dist		目标字符串
	* @param distsize	目标字符串缓冲大小，单位为字符个数
	* @return 是否成功
	*/
	static BOOL u82w(const char *src,unsigned int srclen,wchar_t *dist,int distsize)
	{
		*dist='\0';
		int ret=MultiByteToWideChar(CP_UTF8,NULL,src,srclen,dist,distsize);
		dist[ret]='\0';
		return ret;
	}

	/**
	* @brief 转换W类型为Utf8类型
	* @param src		源字符串
	* @param dist		目标字符串
	* @param distsize	目标字符串缓冲大小，单位为字符个数
	* @return 是否成功
	*/
	static BOOL w2u8(const wchar_t *src,char *dist,int distsize)
	{
		*dist='\0';
		return WideCharToMultiByte(CP_UTF8,0,src,-1,dist,distsize,NULL,NULL);
	}

	/**
	* @brief 转换W类型为A类型
	* @param src		源字符串
	* @param dist		目标字符串
	* @param distsize	目标字符串缓冲大小，单位为字符个数
	* @return 是否成功
	*/
	static BOOL w2a(const wchar_t *src,char *dist,int distsize)
	{
		*dist='\0';
		return WideCharToMultiByte(CP_ACP,0,src,-1,dist,distsize,NULL,NULL);
	}

	/**
	* @brief 转换W类型为A类型
	* @param src		源字符串
	* @param dist		目标字符串
	* @param distsize	目标字符串缓冲大小，单位为字符个数
	* @return 是否成功
	*/
	static BOOL w2a(const wchar_t *src,int srcsize,char *dist,int distsize)
	{
		*dist='\0';
		int ret=WideCharToMultiByte(CP_ACP,0,src,srcsize,dist,distsize,NULL,NULL);
		dist[ret]='\0';
		return ret;
	}

	/**
	* @brief 转换A类型为T类型
	* @param src		源字符串
	* @param dist		目标字符串
	* @param distsize	目标字符串缓冲大小，单位为字符个数
	* @return 是否成功
	*/
	static BOOL a2t(const char *src,TCHAR *dist,unsigned int distsize)
	{
		if (!src || !dist) return FALSE;
#ifdef _UNICODE
		return a2w(src,dist,distsize);
#else
		strncpy(dist,src,distsize);
		dist[distsize-1]='\0';
		return TRUE;
#endif
	}

	/**
	* @brief 转换A类型为T类型
	* @param src		源字符串
	* @param srcsize	源字符串大小
	* @param dist		目标字符串
	* @param distsize	目标字符串缓冲大小，单位为字符个数
	* @return 是否成功
	*/
	static BOOL a2t(const char *src,unsigned int srcsize,TCHAR *dist,unsigned int distsize)
	{
		if (!src || !dist) return FALSE;
#ifdef _UNICODE
		return a2w(src,srcsize,dist,distsize);
#else
		strncpy(dist,src,min(srcsize,distsize-1));
		dist[min(srcsize,distsize-1)]='\0';
		return TRUE;
#endif
	}

	/**
	* @brief 转换A类型为T类型
	* @param src		源字符串
	* @return 目标字符串
	*/
	static THString a2t(const char *src)
	{
		THString ret;
		if (src)
		{
			int len=(int)(strlen(src)+1)*2;
			a2t(src,ret.GetBuffer(len),len);
			ret.ReleaseBuffer();
		}
		return ret;
	}

	/**
	* @brief 转换A类型为T类型，指定长度
	* @param src		源字符串
	* @param len		源字符串长度
	* @return 目标字符串
	*/
	static THString a2t(const char *src,unsigned int len)
	{
		THString ret;
		if (src)
		{
			char *srcout=new char[len+1];
			if (srcout)
			{
				memcpy(srcout,src,len);
				srcout[len]='\0';
				ret=a2t(srcout);
				delete [] srcout;
			}
		}
		return ret;
	}

	/**
	* @brief 转换T类型为A类型
	* @param src		源字符串
	* @param dist		目标字符串
	* @param distsize	目标字符串缓冲大小，单位为字符个数
	* @return 是否成功
	*/
	static BOOL t2a(const TCHAR *src,char *dist,int distsize)
	{
		if (!src || !dist) return FALSE;
#ifdef _UNICODE
		return w2a(src,dist,distsize);
#else
		strncpy(dist,src,distsize);
		dist[distsize-1]='\0';
		return TRUE;
#endif
	}

	/**
	* @brief 转换T类型为A类型
	* @param src		源字符串
	* @return 目标字符串
	*/
	static char *t2a(const TCHAR *src)
	{
		if (!src) return NULL;
		size_t len=(_tcslen(src)+1)*2;//because of the len of chinese is only 1
		char *tmp=new char[len];
		if (!tmp) return NULL;
		if (!t2a(src,tmp,(int)len))
		{
			delete [] tmp;
			return NULL;
		}
		return tmp;
	}

	/**
	* @brief 转换T类型为A类型
	* @param src		源字符串
	* @return 目标字符串,不需要释放
	*/
	static THStringA t2astr(const TCHAR *src)
	{
		THStringA ret;
		char *buf=t2a(src);
		if (buf) {
			ret=buf;
			free(buf);
		}
		return ret;
	}

	/**
	* @brief 转换W类型为T类型
	* @param src		源字符串
	* @param dist		目标字符串
	* @param distsize	目标字符串缓冲大小，单位为字符个数
	* @return 是否成功
	*/
	static BOOL w2t(const wchar_t *src,int srcsize,TCHAR *dist,int distsize)
	{
		if (!src || !dist) return FALSE;
#ifdef _UNICODE
		wcsncpy(dist,src,min(distsize,srcsize));
		dist[distsize-1]='\0';
		return TRUE;
#else
		return w2a(src,srcsize,dist,distsize);
#endif
	}

	/**
	* @brief 转换W类型为T类型
	* @param src		源字符串
	* @param dist		目标字符串
	* @param distsize	目标字符串缓冲大小，单位为字符个数
	* @return 是否成功
	*/
	static BOOL w2t(const wchar_t *src,TCHAR *dist,int distsize)
	{
		if (!src || !dist) return FALSE;
#ifdef _UNICODE
		wcsncpy(dist,src,distsize);
		dist[distsize-1]='\0';
		return TRUE;
#else
		return w2a(src,dist,distsize);
#endif
	}

	/**
	* @brief 转换W类型为T类型
	* @param src		源字符串
	* @return 目标字符串
	*/
	static THString w2t(const wchar_t *src)
	{
		THString ret;
		if (src)
		{
			int len=(int)(wcslen(src)+1)*2;
			w2t(src,ret.GetBuffer(len),len);
			ret.ReleaseBuffer();
		}
		return ret;
	}

	/**
	* @brief 转换W类型为T类型
	* @param src		源字符串
	* @return 目标字符串
	*/
	static THString w2t(const wchar_t *src,unsigned len)
	{
		THString ret;
		if (src)
		{
			wchar_t *srcout=new wchar_t[len+1];
			if (srcout)
			{
				wcsncpy(srcout,src,len);
				srcout[len]='\0';
				w2t(srcout,ret.GetBuffer((len+1)*2),(len+1)*2);
				ret.ReleaseBuffer();
				delete [] srcout;
			}
		}
		return ret;
	}


	/**
	* @brief 转换T类型为W类型
	* @param src		源字符串
	* @param dist		目标字符串
	* @param distsize	目标字符串缓冲大小，单位为字符个数
	* @return 是否成功
	*/
	static BOOL t2w(const TCHAR *src,int srcsize,wchar_t *dist,int distsize)
	{
		if (!src || !dist) return FALSE;
#ifdef _UNICODE
		wcsncpy(dist,src,min(distsize,srcsize));
		dist[distsize-1]='\0';
		return TRUE;
#else
		*dist='\0';
		int ret=MultiByteToWideChar(CP_OEMCP,NULL,src,srcsize,dist,distsize);
		dist[ret]='\0';
		return ret;
#endif
	}

	/**
	* @brief 转换T类型为W类型
	* @param src		源字符串
	* @param dist		目标字符串
	* @param distsize	目标字符串缓冲大小，单位为字符个数
	* @return 是否成功
	*/
	static BOOL t2w(const TCHAR *src,wchar_t *dist,int distsize)
	{
		if (!src || !dist) return FALSE;
#ifdef _UNICODE
		wcsncpy(dist,src,distsize);
		dist[distsize-1]='\0';
		return TRUE;
#else
		*dist='\0';
		int ret=MultiByteToWideChar(CP_OEMCP,NULL,src,-1,dist,distsize);
		dist[ret]='\0';
		return ret;
#endif
	}

	/**
	* @brief 转换T类型为W类型
	* @param src		源字符串
	* @return 目标字符串
	*/
	static wchar_t *t2w(const TCHAR *src)
	{
		if (!src) return NULL;
		size_t len=(_tcslen(src)+1)*2;//because of the len of chinese is only 1
		wchar_t *tmp=new wchar_t[len];
		if (!tmp) return NULL;
		if (!t2w(src,tmp,(int)len))
		{
			delete [] tmp;
			return NULL;
		}
		return tmp;
	}

	/**
	* @brief 转换u8类型为T类型
	* @param src		源字符串
	* @param dist		目标字符串
	* @param distsize	目标字符串缓冲大小，单位为字符个数
	* @return 是否成功
	*/
	static BOOL u82t(const char *src,TCHAR *dist,int distsize)
	{
		if (!src || !dist) return FALSE;
#ifdef _UNICODE
		return u82w(src,dist,distsize);
#else
		return u82a(src,dist,distsize);
#endif
	}

	/**
	* @brief 转换u8类型为T类型
	* @param src		源字符串
	* @param len		字符串长度
	* @return 目标字符串
	*/
	static THString u82t(const char *src,unsigned int len)
	{
		THString ret;
		if (src)
		{
			DWORD dwNum=len*3;
#ifdef _UNICODE
			u82w(src,len,ret.GetBuffer(dwNum),dwNum);
#else
			u82a(src,len,ret.GetBuffer(dwNum),dwNum);
#endif
			ret.ReleaseBuffer();
		}
		return ret;
	}

	/**
	* @brief 转换u8类型为T类型
	* @param src		源字符串
	* @return 目标字符串
	*/
	static THString u82t(const char *src)
	{
		THString ret;
		if (src)
		{
			DWORD dwNum=MultiByteToWideChar(CP_UTF8,NULL,src,-1,NULL,0)*2;
#ifdef _UNICODE
			u82w(src,ret.GetBuffer(dwNum),dwNum);
#else
			u82a(src,ret.GetBuffer(dwNum),dwNum);
#endif
			ret.ReleaseBuffer();
		}
		return ret;
	}

	/**
	* @brief 转换T类型为Utf8类型
	* @param src		源字符串
	* @param dist		目标字符串
	* @param distsize	目标字符串缓冲大小，单位为字符个数
	* @return 是否成功
	*/
	static BOOL t2u8(const TCHAR *src,char *dist,int distsize)
	{
		if (!src || !dist) return FALSE;
#ifdef _UNICODE
		return w2u8(src,dist,distsize);
#else
		return a2u8(src,dist,distsize);
#endif
	}

	/**
	* @brief 转换T类型为Utf8类型
	* @param src		源字符串
	* @return 目标字符串
	*/
	static char *t2u8(const TCHAR *src)
	{
		if (!src) return NULL;
		size_t len=(_tcslen(src)+1)*3;//because of the len of chinese is only 1
		char *tmp=new char[len];
		if (!tmp) return NULL;
		if (!t2u8(src,tmp,(int)len))
		{
			delete [] tmp;
			return NULL;
		}
		return tmp;
	}

	static void free(void *p)
	{
		if (p) delete [] p;
	}
};

/**
* @brief 字符集转换缓存类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-09-08 新建类
*/
/**<pre>
  使用Sample：
	THCharsetBuf buf(str);
	char *buf=buf.t2a();
	wchar_t *wbuf=buf.t2w();
*/
class THCharsetBuf
{
public:
	THCharsetBuf()
	{
		m_buf=NULL;
		m_type=0;
	}
	THCharsetBuf(THString str)
	{
		m_buf=NULL;
		m_type=0;
		SetString(str);
	}
	virtual ~THCharsetBuf() {free();}
	void SetString(THString str) {m_str=str;m_type=0;}
	char *t2a()
	{
		if (m_type!=1)
		{
			free();
			m_type=1;
			m_buf=THCharset::t2a(m_str);
		}
		return (char *)m_buf;
	}
	wchar_t *t2w()
	{
		if (m_type!=2)
		{
			free();
			m_type=2;
			m_buf=THCharset::t2w(m_str);
		}
		return (wchar_t *)m_buf;
	}
	char *t2u8()
	{
		if (m_type!=3)
		{
			free();
			m_type=3;
			m_buf=THCharset::t2u8(m_str);
		}
		return (char *)m_buf;
	}
	void free() {if (m_buf!=NULL){THCharset::free(m_buf);m_buf=NULL;m_type=0;}}
protected:
	void *m_buf;
	THString m_str;
	int m_type;
};