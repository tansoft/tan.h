#pragma once

#include <THSystem.h>
#include <THStruct.h>
#include <THSyn.h>
#include <THFile.h>

#define THMEMBUF_ALLOCSIZE	(1024 * 64)//1*1024*1024	//1M

/**
* @brief �����ڴ滺���࣬ע�⣬���������ĳЩʱ��ᷢ�����⣬����ʹ��
* @author Barry
* @2007-08-08 �½���
* @2008-02-02 ����ֱ�ӻ�ȡȫ�����弰����㺯��
*/
/**<pre>
ʹ��Sample��
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
	* @brief �������ݰ�
	* @param buf		����ָ��
	* @param len		���ݳ���
	* @return			�Ƿ�ɹ�
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
	* @brief ��ȡ���ݰ�
	* @param buf		����ָ��
	* @param len		���ݳ���,-1Ϊ�ѻ�������г���
	* @param ret		����ʵ�����ݵĳ��ȣ���Ϊ��
	* @param bPeek		�Ƿ��Ƴ�����
	* @return			�Ƿ�ɹ�
	*/
	BOOL GetBuf(void *buf,int len,unsigned int *ret,BOOL bPeek)
	{
		if (len==-1) len=m_buffedsize;
		if (bPeek==FALSE)
		{
			//��Ϊ���һ��ʼ����bPeek����FALSE��ȡ��������ݲ�����ƫ���ֻᱻ��д��
			if (GetBufferedSize()<(UINT)len)
				return FALSE;
			//BOOL bl=GetBuf(buf,len,ret,TRUE);
			//if (bl==FALSE)
			//	return FALSE;
		}
		//���û�п��Զ�ȡ������
		//OutputDebugString("GetBuf\n");
		if (ret) *ret=0;
		if (!m_curread) return FALSE;
		THSingleLock lock(&m_mutex);
		char *curbuf=(char *)buf;
		MemBufData *pread=m_curread;
		while(len>0)
		{
			//����пɶ�����
			if (pread->readed<pread->wroted)
			{
				//OutputDebugString("GetBuf:Read Cur Data\n");
				//��ȡ��ǰ������
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
				//˵����ǰ���廹û��д��
				if (pread->wroted<pread->len)
				{
					//OutputDebugString("GetBuf:this area not have more data\n");
					return FALSE;
				}
				//�����ǰָ���Ѿ�ѭ����дָ����
				if (pread==m_curwrite)
				{
					//OutputDebugString("GetBuf:no more data\n");
					return FALSE;
				}
				//����һ���ݿ飬�ѵ�ǰ���ݿ���뵽��д���ݿ�β��ʵ���ڴ�����
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
	* @brief ��ջ���
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
	* @brief ��ȡ�������ݳ���
	* @return			���ػ����е����ݳ���
	*/
	UINT GetBufferedSize(){return m_buffedsize;}
	/**
	* @brief ��ȡ���л�������
	* @param pLen		�������ݳ��ȣ�Ϊ��Ϊ����Ҫ����
	* @param bPeek		�Ƿ��Ƴ�����
	* @return			���ػ�������ָ��
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
	* @brief ��ȡ���л������ݣ�������������0����֧��ascii��unicode�ַ�����β
	* @param pLen		�������ݳ��ȣ�Ϊ��Ϊ����Ҫ����
	* @param bPeek		�Ƿ��Ƴ�����
	* @return			���ػ�������ָ��
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
	* @brief �ͷ�����Ļ������ݵ�ָ��
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
* @brief �ڴ滺�����������ע�⣬���������ĳЩʱ��ᷢ�����⣬����ʹ��
* @author Barry
* @2007-08-08 �½���
*/
/**<pre>
ʹ��Sample��
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
* @brief �����ڴ滺����
* @author Barry
* @2008-05-16 �½��࣬ȡ��ԭ�����࣬ԭ���ΪTHMemBufOld
*/
/**<pre>
ʹ��Sample��
	THMemBuf buf;
	buf.AddBuf("1234",4);
	char buf[1024];
	UINT ret=0;
	void *p=buf.GetBuf(&ret);
��ԭ��Ķ���
	BOOL GetBuf(void *buf,int len,unsigned int *ret,BOOL bPeek)
	��Ϊ
	void *GetBuf(UINT *len)
	����
	void ReleaseGetBufLock(UINT nDeleteSize=0)
	void RemoveBuf(UINT nDeleteSize=0)
	�ӿ�
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
	* @brief �������ݰ�
	* @param buf		����ָ��
	* @param len		���ݳ���
	* @return			�Ƿ�ɹ�
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
	* @brief ��ȡ���ݰ���ע���ʱbuffer����ס������ReleaseGetBufLock�ͷţ�ע�����۷����Ƿ�ʧ�ܣ�����Ҫ����ReleaseGetBufLock
	* @param len		���ݳ��ȣ���Ϊ��
	* @param bLock	�Ƿ���ס������
	* @return			����ָ��
	*/
	void *GetBuf(UINT *len,BOOL bLock=TRUE)
	{
		if (bLock) m_wmtx.Lock();
		if (len) *len=m_nBufSize;
		return m_buf;
	}
	/**
	* @brief �ͷŻ�ȡ���ݰ�ʱ��õ�����ָ��
	* @param nDeleteSize	��Ҫ�Ƴ������ݵĴ�С��Ϊ0Ϊ���Ƴ�����
	*/
	void ReleaseGetBufLock(UINT nDeleteSize=0)
	{
		_RemoveBuf(nDeleteSize);
		m_wmtx.Unlock();
	}
	/**
	* @brief �Ƴ�����
	* @param nDeleteSize	��Ҫ�Ƴ������ݵĴ�С��Ϊ0Ϊ���Ƴ�����
	*/
	void RemoveBuf(UINT nDeleteSize=0)
	{
		THSingleLock lock(&m_wmtx);
		_RemoveBuf(nDeleteSize);
	}
	/**
	* @brief ��ջ���
	*/
	void Empty()
	{
		THSingleLock lock(&m_wmtx);
		m_nBufSize=0;
	}
	/**
	* @brief ǿ�ƽض���ʹ�õĴ�С�����������û��newsize��newsize��Ϊ��������С
	* @param nNewSize	ǿ�ƽض̴�С
	*/
	UINT SetSize(UINT nNewSize)
	{
		THSingleLock lock(&m_wmtx);
		if (nNewSize>m_nAllocSize) nNewSize=m_nAllocSize;
		m_nBufSize=nNewSize;
		return m_nBufSize;
	}
	/**
	* @brief ��ȡ�������ݳ���
	* @return			���ػ����е����ݳ���
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
* @brief �ڴ滺���������
* @author Barry
* @2008-05-16 �½��࣬ȡ��ԭ�����࣬ԭ���ΪTHMemBufManagerOld
*/
/**<pre>
ʹ��Sample��
	THMemBufManager<int> buf;
	buf.AddBuf(1,"1234",4);
	buf.SetAddData(1,3);
	char buf[1024];
	UINT ret=0;
	void *p=buf.GetBuf(1,&ret);
��ԭ��Ķ���
	BOOL GetBuf(T id,void *buf,int len,unsigned int *ret,BOOL bPeek)
	��Ϊ��
	void *GetBuf(T id,UINT *len)
	����
	void ReleaseGetBufLock(T id,UINT nDeleteSize=0)
	void RemoveBuf(T id,UINT nDeleteSize=0)
�ӿ�
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
