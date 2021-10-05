#pragma once

#include <THSystem.h>
#include <THStruct.h>
#include <THSyn.h>
#include <THFile.h>

#define THMEMBUF_ALLOCSIZE	(1024 * 64)//1*1024*1024	//1M

/**
* @brief 基础内存缓冲类，注意，该类好像有某些时候会发生问题，不再使用
* @author Barry
* @2007-08-08 新建类
* @2008-02-02 增加直接获取全部缓冲及最后补零函数
*/
/**<pre>
使用Sample：
	THMemBuf buf;
	buf.AddBuf("1234",4);
	char buf[1024];
	int ret=0;
	buf.GetBuf(buf,6,&ret,FLASE);
</pre>*/
class THMemBufOld
{
public:
	THMemBufOld(void){m_curread=NULL;m_curwrite=NULL;m_buffedsize=0;}
	virtual ~THMemBufOld(void)
	{
		THSingleLock lock(&m_mutex);
		if (m_curread)
		{
			MemBufData *cur=m_curread;
			MemBufData *tmp;
			m_curread=NULL;
			while(cur)
			{
				delete [] cur->buf;
				tmp=cur->next;
				delete cur;
				cur=tmp;
			}
		}
	}

	/**
	* @brief 加入数据包
	* @param buf		数据指针
	* @param len		数据长度
	* @return			是否成功
	*/
	BOOL AddBuf(const void *buf,int len)
	{
		if (len==0) return TRUE;
		THSingleLock lock(&m_mutex);
		if (!m_curwrite)
		{
			//init the first buffer
			m_curwrite=NewBuf();
			if (!m_curwrite) return FALSE;
			m_curread=m_curwrite;
		}
		const char *curbuf=(const char *)buf;
		while(len>0)
		{
			if (m_curwrite->wroted<m_curwrite->len)
			{
				//OutputDebugString("AddBuf:Copy Cur Data\n");
				int copylen=m_curwrite->len-m_curwrite->wroted;
				if (copylen>len) copylen=len;
				memcpy(m_curwrite->buf+m_curwrite->wroted,curbuf,copylen);
				m_curwrite->wroted+=copylen;
				curbuf+=copylen;
				len-=copylen;
				m_buffedsize+=copylen;
			}
			else
			{
				if (!m_curwrite->next)
				{
					//init next buffer
					//OutputDebugString("AddBuf:New Next Buf\n");
					m_curwrite->next=NewBuf();
					if (!m_curwrite->next) return FALSE;
				}
				//OutputDebugString("AddBug:Locate next buf\n");
				m_curwrite=m_curwrite->next;
			}
		}
		return TRUE;
	}
	/**
	* @brief 获取数据包
	* @param buf		数据指针
	* @param len		数据长度,-1为已缓冲的所有长度
	* @param ret		返回实际数据的长度，可为空
	* @param bPeek		是否移除数据
	* @return			是否成功
	*/
	BOOL GetBuf(void *buf,int len,unsigned int *ret,BOOL bPeek)
	{
		if (len==-1) len=m_buffedsize;
		if (bPeek==FALSE)
		{
			//因为如果一开始就用bPeek等于FALSE读取，如果数据不够，偏移又会被改写的
			if (GetBufferedSize()<(UINT)len)
				return FALSE;
			//BOOL bl=GetBuf(buf,len,ret,TRUE);
			//if (bl==FALSE)
			//	return FALSE;
		}
		//如果没有可以读取的数据
		//OutputDebugString("GetBuf\n");
		if (ret) *ret=0;
		if (!m_curread) return FALSE;
		THSingleLock lock(&m_mutex);
		char *curbuf=(char *)buf;
		MemBufData *pread=m_curread;
		while(len>0)
		{
			//如果有可读数据
			if (pread->readed<pread->wroted)
			{
				//OutputDebugString("GetBuf:Read Cur Data\n");
				//读取当前的数据
				int copylen=pread->wroted-pread->readed;
				if (copylen>len) copylen=len;
				memcpy(curbuf,pread->buf+pread->readed,copylen);
				if (!bPeek)
				{
					pread->readed+=copylen;
					m_buffedsize-=copylen;
				}
				if (ret) *ret+=copylen;
				curbuf+=copylen;
				len-=copylen;
			}
			if (len>0)
			{
				//说明当前缓冲还没有写满
				if (pread->wroted<pread->len)
				{
					//OutputDebugString("GetBuf:this area not have more data\n");
					return FALSE;
				}
				//如果当前指针已经循环到写指针了
				if (pread==m_curwrite)
				{
					//OutputDebugString("GetBuf:no more data\n");
					return FALSE;
				}
				//读下一数据块，把当前数据块加入到可写数据块尾，实现内存重用
				MemBufData *tmp=pread;
				pread=pread->next;
				if (!pread)
				{
					//OutputDebugString("GetBuf:locate next,not more data\n");
					if (!bPeek) m_curread=tmp;
					return FALSE;
				}
				//OutputDebugString("GetBuf:locate next,free buf join to write queue\n");
				if (!bPeek)
				{
					m_curread=pread;
					tmp->next=NULL;
					tmp->readed=0;
					tmp->wroted=0;
					MemBufData *tmp1=m_curwrite;
					if (!m_curwrite)
						m_curwrite=tmp;
					else
					{
						while(tmp1->next)
							tmp1=tmp1->next;
						tmp1->next=tmp;
					}
				}
			}
		}
		return TRUE;
	}
	/**
	* @brief 清空缓冲
	*/
	void Empty()
	{
		THSingleLock lock(&m_mutex);
		if (m_curread)
		{
			MemBufData *cur=m_curread;
			while(cur)
			{
				cur->readed=0;
				cur->wroted=0;
				cur=cur->next;
			}
		}
		m_buffedsize=0;
	}
	/**
	* @brief 获取缓冲数据长度
	* @return			返回缓冲中的数据长度
	*/
	UINT GetBufferedSize(){return m_buffedsize;}
	/**
	* @brief 获取所有缓冲数据
	* @param pLen		返回数据长度，为空为不需要长度
	* @param bPeek		是否移除数据
	* @return			返回缓冲数据指针
	*/
	void *GetAllBufferedData(UINT *pLen,BOOL bPeek=TRUE)
	{
		UINT len=GetBufferedSize();
		void *buf=NULL;
		if (len!=0)
		{
			buf=new char[len];
			if (buf)
			{
				unsigned int ulen;
				BOOL bret=GetBuf(buf,len,&ulen,bPeek);
				if (!bret || len!=ulen)
				{
					delete [] (char *)buf;
					buf=NULL;
					len=0;
				}
			}
		}
		if (pLen) *pLen=len;
		return buf;
	}
	/**
	* @brief 获取所有缓冲数据，并在最后加两个0，即支持ascii和unicode字符串结尾
	* @param pLen		返回数据长度，为空为不需要长度
	* @param bPeek		是否移除数据
	* @return			返回缓冲数据指针
	*/
	void *GetAllBufferedDataWithEnd0(UINT *pLen=NULL,BOOL bPeek=TRUE)
	{
		UINT len=GetBufferedSize();
		char *buf=NULL;
		if (len!=0)
		{
			buf=new char[len+2];
			if (buf)
			{
				unsigned int ulen;
				BOOL bret=GetBuf(buf,len,&ulen,bPeek);
				if (!bret || len!=ulen)
				{
					delete [] (char *)buf;
					buf=NULL;
					len=0;
				}
				buf[len]='\0';
				buf[len+1]='\0';
			}
		}
		if (pLen) *pLen=len;
		return (void *)buf;
	}

	/**
	* @brief 释放请求的缓冲数据的指针
	*/
	inline void FreeBufferedPointer(void *p){if (p) delete [] (char *)p;}
private:
	typedef struct _MemBufData{
		char *buf;
		int len;
		int wroted;
		int readed;
		struct _MemBufData *next;
	}MemBufData;
	//m_curread is the start
	//m_curwrite is the end
	MemBufData *m_curread,*m_curwrite;
	volatile UINT m_buffedsize;
	MemBufData *NewBuf()
	{
		MemBufData *newbuf=new MemBufData;
		if (!newbuf) return NULL;
		newbuf->buf=new char[THMEMBUF_ALLOCSIZE];
		if (!newbuf->buf)
		{
			delete newbuf;
			return NULL;
		}
		newbuf->len=THMEMBUF_ALLOCSIZE-1;
		newbuf->next=NULL;
		newbuf->readed=0;
		newbuf->wroted=0;
		return newbuf;
	}
	THMutex m_mutex;
};

/**
* @brief 内存缓冲类管理器，注意，该类好像有某些时候会发生问题，不再使用
* @author Barry
* @2007-08-08 新建类
*/
/**<pre>
使用Sample：
	THMemBufManager<int> buf;
	buf.AddBuf(1,"1234",4);
	buf.SetAddData(1,3);
	char buf[1024];
	int ret=0;
	buf.GetBuf(1,buf,6,&ret,FLASE);
</pre>*/
template <typename T>
class THMemBufManagerOld
{
private:
	static void FreeMapCallBack(void *key,void *value,void *adddata)
	{
		delete (THMemBuf *)value;
	}
public:
	THMemBufManagerOld(void)
	{
		m_mem.SetFreeProc(FreeMapCallBack);
	}
	virtual ~THMemBufManagerOld(void)
	{
		m_mem.RemoveAll();
	}

	BOOL IsHaveData(T id)
	{
		THMemBuf *buf;
		return m_mem.GetAt(id,buf);
	}

	BOOL AddBuf(T id,const void *buf,int len)
	{
		THMemBuf *m;
		if (!m_mem.GetAt(id,m))
		{
			m=new THMemBuf;
			if (!m) return FALSE;
			m_mem.SetAt(id,m);
		}
		return m->AddBuf(buf,len);
	}

	BOOL GetBuf(T id,void *buf,int len,unsigned int *ret,BOOL bPeek)
	{
		THMemBuf *m;
		if (!m_mem.GetAt(id,m))
			return FALSE;
		return m->GetBuf(buf,len,ret,bPeek);
	}

	void Empty(T id)
	{
		THMemBuf *m;
		if (!m_mem.GetAt(id,m)) return;
		m->Empty();
	}

	UINT GetBufferedSize(T id)
	{
		THMemBuf *m;
		if (!m_mem.GetAt(id,m)) return 0;
		return m->GetBufferedSize();
	}

	void SetAddData(T id,int data)
	{
		m_adddata.SetAt(id,data);
	}
	int GetAddData(T id)
	{
		return m_adddata.GetAt(id);
	}
private:
	THMap<T,THMemBufOld *>m_mem;
	THMap<int,int>m_adddata;
};

/**
* @brief 基础内存缓冲类
* @author Barry
* @2008-05-16 新建类，取代原来的类，原类改为THMemBufOld
*/
/**<pre>
使用Sample：
	THMemBuf buf;
	buf.AddBuf("1234",4);
	char buf[1024];
	UINT ret=0;
	void *p=buf.GetBuf(&ret);
与原类改动：
	BOOL GetBuf(void *buf,int len,unsigned int *ret,BOOL bPeek)
	改为
	void *GetBuf(UINT *len)
	增加
	void ReleaseGetBufLock(UINT nDeleteSize=0)
	void RemoveBuf(UINT nDeleteSize=0)
	接口
</pre>*/
class THMemBuf
{
public:
	THMemBuf()
	{
		m_buf=NULL;
		m_nBufSize=0;
		m_nAllocSize=0;
	}

	virtual ~THMemBuf()
	{
		THSingleLock lock(&m_wmtx);
		if (m_buf) delete [] m_buf;
		m_buf=NULL;
		m_nBufSize=0;
		m_nAllocSize=0;
	}

	inline BOOL AddBufU8(U8 key){return AddBuf(&key,1);}
	inline BOOL AddBufU16(U16 key){return AddBuf(&key,2);}
	inline BOOL AddBufU16N(U16 key){key=htons(key);return AddBuf(&key,2);}
	inline BOOL AddBufU32(U32 key){return AddBuf(&key,4);}
	inline BOOL AddBufU32N(U32 key){key=htonl(key);return AddBuf(&key,4);}

	/**
	* @brief 加入数据包
	* @param buf		数据指针
	* @param len		数据长度
	* @return			是否成功
	*/
	BOOL AddBuf(const void *buf,UINT len)
	{
		THSingleLock lock(&m_wmtx);
		int malloccnt=0;
		if (!m_buf)
		{
			malloccnt=(len+THMEMBUF_ALLOCSIZE-1)/THMEMBUF_ALLOCSIZE;
		}
		else if (m_nAllocSize-m_nBufSize<len)
		{
			malloccnt=(len+m_nBufSize*2+THMEMBUF_ALLOCSIZE-1)/THMEMBUF_ALLOCSIZE;
		}
		if (malloccnt!=0)
		{
			char *tmp=new char[malloccnt*THMEMBUF_ALLOCSIZE];
			if (!tmp)
			{
				if (m_buf) delete [] m_buf;
				m_buf=NULL;
				m_nBufSize=0;
				m_nAllocSize=0;
				return FALSE;
			}
			if (m_buf)
			{
				if (m_nBufSize>0) memcpy(tmp,m_buf,m_nBufSize);
				delete [] m_buf;
			}
			m_nAllocSize=malloccnt*THMEMBUF_ALLOCSIZE;
			m_buf=tmp;
		}
		memcpy(m_buf+m_nBufSize,buf,len);
		m_nBufSize+=len;
		return TRUE;
	}
	/**
	* @brief 获取数据包，注意此时buffer被锁住，调用ReleaseGetBufLock释放，注意无论返回是否失败，都需要调用ReleaseGetBufLock
	* @param len		数据长度，可为空
	* @param bLock	是否锁住缓存区
	* @return			数据指针
	*/
	void *GetBuf(UINT *len,BOOL bLock=TRUE)
	{
		if (bLock) m_wmtx.Lock();
		if (len) *len=m_nBufSize;
		return m_buf;
	}
	/**
	* @brief 释放获取数据包时获得的数据指针
	* @param nDeleteSize	需要移除的数据的大小，为0为不移除数据
	*/
	void ReleaseGetBufLock(UINT nDeleteSize=0)
	{
		_RemoveBuf(nDeleteSize);
		m_wmtx.Unlock();
	}
	/**
	* @brief 移除数据
	* @param nDeleteSize	需要移除的数据的大小，为0为不移除数据
	*/
	void RemoveBuf(UINT nDeleteSize=0)
	{
		THSingleLock lock(&m_wmtx);
		_RemoveBuf(nDeleteSize);
	}
	/**
	* @brief 清空缓冲
	*/
	void Empty()
	{
		THSingleLock lock(&m_wmtx);
		m_nBufSize=0;
	}
	/**
	* @brief 强制截短已使用的大小，如果缓冲区没有newsize大，newsize变为缓冲区大小
	* @param nNewSize	强制截短大小
	*/
	UINT SetSize(UINT nNewSize)
	{
		THSingleLock lock(&m_wmtx);
		if (nNewSize>m_nAllocSize) nNewSize=m_nAllocSize;
		m_nBufSize=nNewSize;
		return m_nBufSize;
	}
	/**
	* @brief 获取缓冲数据长度
	* @return			返回缓冲中的数据长度
	*/
	UINT GetBufferedSize(){return m_nBufSize;}
	THString BinToHexString()
	{
		THSingleLock lock(&m_wmtx);
		return THStringConv::BinToHexString(m_buf,m_nBufSize);
	}
	THString FormatString(THStringConv::FormatType ft=THStringConv::FormatType_Hex,int nLineCount=16,THStringConv::FormatType ftOffest=THStringConv::FormatType_Hex,BOOL bMiddleSep=TRUE,BOOL bHaveRawData=TRUE)
	{
		THSingleLock lock(&m_wmtx);
		return THStringConv::FormatString(m_buf,m_nBufSize);
	}
	BOOL AddBuf(THMemBuf *other)
	{
		BOOL bRet=FALSE;
		if (other)
		{
			UINT len;
			void *buf=other->GetBuf(&len);
			bRet=AddBuf(buf,len);
			other->ReleaseGetBufLock();
		}
		return bRet;
	}
	BOOL SaveToFile(THString file) {return THFile::SaveFile(file,GetBuf(NULL,FALSE),GetBufferedSize());}
private:
	void _RemoveBuf(UINT nDeleteSize=0)
	{
		if (nDeleteSize>0)
		{
			if (nDeleteSize>m_nBufSize)
			{
				ASSERT(FALSE);
				nDeleteSize=m_nBufSize;
			}
			if (nDeleteSize==m_nBufSize)
			{
				m_nBufSize=0;
			}
			else
			{
				m_nBufSize-=nDeleteSize;
				memmove(m_buf,m_buf+nDeleteSize,m_nBufSize);
			}
		}
	}
	char *m_buf;
	//THMutex m_rmtx;
	THMutex m_wmtx;
	volatile UINT m_nBufSize;
	volatile UINT m_nAllocSize;
};

/**
* @brief 内存缓冲类管理器
* @author Barry
* @2008-05-16 新建类，取代原来的类，原类改为THMemBufManagerOld
*/
/**<pre>
使用Sample：
	THMemBufManager<int> buf;
	buf.AddBuf(1,"1234",4);
	buf.SetAddData(1,3);
	char buf[1024];
	UINT ret=0;
	void *p=buf.GetBuf(1,&ret);
与原类改动：
	BOOL GetBuf(T id,void *buf,int len,unsigned int *ret,BOOL bPeek)
	改为：
	void *GetBuf(T id,UINT *len)
	增加
	void ReleaseGetBufLock(T id,UINT nDeleteSize=0)
	void RemoveBuf(T id,UINT nDeleteSize=0)
接口
</pre>*/
template <typename T>
class THMemBufManager
{
private:
	static void FreeMapCallBack(void *key,void *value,void *adddata)
	{
		delete (THMemBuf *)value;
	}
public:
	THMemBufManager(void)
	{
		m_mem.SetFreeProc(FreeMapCallBack);
	}
	virtual ~THMemBufManager(void)
	{
		RemoveAll();
	}

	inline void RemoveAll(){return m_mem.RemoveAll();}

	BOOL IsHaveData(T id)
	{
		THMemBuf *buf;
		return m_mem.GetAt(id,buf);
	}

	BOOL AddBuf(T id,const void *buf,int len)
	{
		THMemBuf *m;
		if (!m_mem.GetAt(id,m))
		{
			m=new THMemBuf;
			if (!m) return FALSE;
			m_mem.SetAt(id,m);
		}
		return m->AddBuf(buf,len);
	}

	void *GetBuf(T id,UINT *len)
	{
		THMemBuf *m;
		if (!m_mem.GetAt(id,m))
		{
			if (len) *len=0;
			return NULL;
		}
		return m->GetBuf(len);
	}

	void ReleaseGetBufLock(T id,UINT nDeleteSize=0)
	{
		THMemBuf *m;
		if (!m_mem.GetAt(id,m)) return;
		m->ReleaseGetBufLock(nDeleteSize);
	}

	void RemoveBuf(T id,UINT nDeleteSize=0)
	{
		THMemBuf *m;
		if (!m_mem.GetAt(id,m)) return;
		m->RemoveBuf(nDeleteSize);
	}

	void Empty(T id)
	{
		THMemBuf *m;
		if (!m_mem.GetAt(id,m)) return;
		m->Empty();
	}

	UINT GetBufferedSize(T id)
	{
		THMemBuf *m;
		if (!m_mem.GetAt(id,m)) return 0;
		return m->GetBufferedSize();
	}

	void SetAddData(T id,int data)
	{
		m_adddata.SetAt(id,data);
	}
	int GetAddData(T id)
	{
		return m_adddata.GetAt(id);
	}
private:
	THMap<T,THMemBuf *>m_mem;
	THMap<T,int>m_adddata;
};
