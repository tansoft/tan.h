#pragma once

#include <THStruct.h>
#include <THObjectBuffer.h>

/**
* @brief �ڴ滺����Ӧ��
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-08-16 �½���
*/
class ITHMemCacheNotify
{
public:
	/**
	* @brief ��������û�ж�Ӧ���ݣ���Ҫ������ʵ����ʱ����Ӧ�ú���
	* @param bufidx		��ȡ�����������
	* @param off		��ȡ���ݵ�ƫ����
	* @param buf		�������ݻ���
	* @param len		��Ҫ��ȡ��С������Ϊʵ�ʶ�ȡ��С
	* @param adddata	ȫ�ֵĸ��Ӳ���
	* @param curadddata	�������ζ�ȡ�¼�ʱ������ĸ��Ӳ���
	* @return ��ȡ�Ƿ�ɹ�
	*/
	virtual BOOL BufferNeedRead(int bufidx,UINT off,void *buf,UINT &len,void *adddata,void *curadddata)=0;
	/**
	* @brief ����Ҫд���������ʱ����Ӧ�ú���
	* @param bufidx		д������������
	* @param off		д�����ݵ�ƫ����
	* @param buf		д�����ݻ���
	* @param len		д�����ݴ�С
	* @param adddata	ȫ�ֵĸ��Ӳ���
	* @param curadddata	��������д���¼�ʱ������ĸ��Ӳ���
	* @return ��ȡ�Ƿ�ɹ�
	*/
	virtual BOOL BufferNeedWrite(int bufidx,UINT off,const void *buf,UINT len,void *adddata,void *curadddata)=0;
};

/**
* @brief �ڴ滺����
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-08-16 �½���
*/
template<UINT CACHESIZE,UINT CACHECOUNT>
class THMemCache
{
public:
	THMemCache()
	{
		m_notify=NULL;
		m_adddata=NULL;
		SetLimitCount(CACHECOUNT);
		m_map.SetFreeProc(FreeMap,this);
		m_nCacheHitSize=0;
		m_nCacheNotHitSize=0;
	}

	virtual ~THMemCache()
	{
		EmptyAllBuffer();
	}

	void SetBufferObject(ITHMemCacheNotify *notify,void *adddata)
	{
		m_notify=notify;
		m_adddata=adddata;
	}

	/**
	* @brief ��ȡ����
	* @param bufidx		��ȡ�����������
	* @param off		��ȡ���ݵ�ƫ����
	* @param buf		�������ݻ���
	* @param len		��Ҫ��ȡ��С������Ϊʵ�ʶ�ȡ��С
	* @param curadddata	�������ζ�ȡ�¼�ʱ������ĸ��Ӳ���
	* @param bUpdateBuffer		���������û�����ݣ�������ʵ���ݺ��Ƿ�����ݻ��浽�����У���������������ݣ��Ƿ���»������ݵ�������ʱ��
	* @return ��ȡ�Ƿ�ɹ�
	*/
	BOOL BufferedRead(int bufidx,UINT off,void *buf,UINT &len,void *curadddata=NULL,BOOL bUpdateBuffer=TRUE)
	{
		char *cbuf=(char *)buf;
		UINT tlen=len;
		len=0;
		BOOL bCache;
		while(tlen>0)
		{
			bCache=FALSE;
			int partlen=off%CACHESIZE;
			//����λ��
			UINT toff=off-partlen;
			m_mapmtx.ReadOnlyLock();
			THMemFileBuf *fbuf=m_map[bufidx];
			if (fbuf)
			{
				MemCache *cache=(*fbuf)[toff];
				if (cache && m_memman.IsObjValid(cache,toff+1))
				{
					//����Ӧ��cache->nLen-partlen��tlen�Ƚϣ�����������cache->nLen��partlenС
					int rlen=min(cache->nLen,tlen+partlen);
					//�Ѿ�û�л�������
					if (rlen<=partlen)
					{
						m_mapmtx.ReadOnlyUnlock();
						return TRUE;
					}
					rlen-=partlen;
					memcpy(cbuf,cache->buf+partlen,rlen);
					m_nCacheHitSize+=rlen;
					cbuf+=rlen;
					off+=rlen;
	 				tlen-=rlen;
					len+=rlen;
					bCache=TRUE;
					if (bUpdateBuffer)
						m_memman.UpdateLastUseTime(cache,toff+1);
				}
			}
			m_mapmtx.ReadOnlyUnlock();
			if (bCache==FALSE)
			{
				if (bUpdateBuffer)
				{
					//read from real buffer and cached
					if (!m_notify) return FALSE;
					MemCache *cache=m_memman.NewObj(toff+1);
					if (!cache) return FALSE;
					UINT needread=CACHESIZE;
					if (!m_notify->BufferNeedRead(bufidx,toff,cache->buf,needread,m_adddata,curadddata))
					{
						m_memman.ReleaseObj(cache,toff+1);
						return FALSE;
					}
					cache->nLen=needread;
					m_mapmtx.ChangeLock();
					THMemFileBuf *fbuf=m_map[bufidx];
					if (!fbuf)
					{
						fbuf=new THMemFileBuf;
						m_map.SetAt(bufidx,fbuf);
					}
					fbuf->SetAt(toff,cache);
					m_mapmtx.ChangeUnlock();
					//����Ӧ��cache->nLen-partlen��tlen�Ƚϣ�����������cache->nLen��partlenС
					int rlen=min(cache->nLen,tlen+partlen);
					//�Ѿ�û�л�������
					if (rlen<=partlen)
						return TRUE;
					rlen-=partlen;
					memcpy(cbuf,cache->buf+partlen,rlen);
					m_nCacheNotHitSize+=rlen;
					cbuf+=rlen;
					off+=rlen;
	 				tlen-=rlen;
					len+=rlen;
				}
				else
				{
					//read from real buffer
					if (!m_notify) return FALSE;
					UINT needread=min(CACHESIZE-partlen,tlen);
					UINT needread1=needread;
					if (!m_notify->BufferNeedRead(bufidx,off,cbuf,needread,m_adddata,curadddata)) return FALSE;
					m_nCacheNotHitSize+=needread;
					cbuf+=needread;
					off+=needread;
	 				tlen-=needread;
					len+=needread;
					//it means it is end of the file
					if (needread1!=needread)
						return TRUE;
				}
			}
		}
		return TRUE;
	}

	/**
	* @brief д������
	* @param bufidx		д������������
	* @param off		д�����ݵ�ƫ����
	* @param buf		д������ݻ���
	* @param len		��Ҫд���С
	* @param curadddata	��������д���¼�ʱ������ĸ��Ӳ���
	* @param bUpdateBuffer		���������û�����ݣ���д����ʵ���ݣ���������������ݣ��Ƿ���»������ݵ�������ʱ��
	* @return д���Ƿ�ɹ�
	*/
	BOOL BufferedWrite(int bufidx,UINT off,const void *buf,UINT len,void *curadddata,BOOL bUpdateBuffer=TRUE)
	{
		char *cbuf=(char *)buf;
		if (!m_notify) return FALSE;
		if (!m_notify->BufferNeedWrite(bufidx,off,buf,len,m_adddata,curadddata)) return FALSE;
		//update the cache
		m_mapmtx.ChangeLock();
		THMemFileBuf *fbuf=m_map[bufidx];
		if (fbuf)
		{
			while(len>0)
			{
				int partlen=off%CACHESIZE;
				UINT toff=off-partlen;
				UINT wlen=min(len,CACHESIZE-partlen);
				MemCache *cache=(*fbuf)[toff];
				//update the cache
				if (cache && m_memman.IsObjValid(cache,toff+1))
				{
					memcpy(cache->buf+partlen,cbuf,wlen);
					if (bUpdateBuffer)
						m_memman.UpdateLastUseTime(cache,toff+1);
				}
				len-=wlen;
				cbuf+=wlen;
				off+=wlen;
			}
		}
		m_mapmtx.ChangeUnlock();
		return TRUE;
	}

	/**
	* @brief ��ջ�������
	* @param bufidx		��յĶ����������
	*/
	void EmptyBuffer(int bufidx)
	{
		m_mapmtx.ChangeLock();
		m_map.RemoveAt(bufidx);
		m_mapmtx.ChangeUnlock();
	}

	/**
	* @brief ������л�������
	*/
	void EmptyAllBuffer()
	{
		m_mapmtx.ChangeLock();
		m_map.RemoveAll();
		m_mapmtx.ChangeUnlock();
	}

	/**
	* @brief ���û������������
	* @param cnt		���ı�CACHECOUNT��ֵ��cnt*CACHESIZE=ʵ�ʿɻ������ݴ�С��0Ϊ������
	*/
	void SetLimitCount(int cnt)
	{
		m_LimitCnt=cnt;
		m_memman.SetLimitCount(cnt);
	}

	/**
	* @brief ��ȡ������б���
	* @param nHit		���л����ֽ���
	* @param nNoHit		û�л��л����ֽ���
	*/
	void GetCacheHit(__int64 &nHit,__int64 &nNoHit)
	{
		nHit=m_nCacheHitSize;
		nNoHit=m_nCacheNotHitSize;
	}

	/**
	* @brief ���û�����м���
	*/
	void ClearCacheHit()
	{
		m_nCacheHitSize=m_nCacheNotHitSize=0;
	}
protected:
	static void FreeMap(void *key,void *value,void *adddata)
	{
		THMemCache *cls=(THMemCache *)adddata;
		THMemFileBuf *buf=(THMemFileBuf *)value;
		if (buf)
		{
			THPosition pos=buf->GetStartPosition();
			int idx;
			MemCache *cache;
			while(!pos.IsEmpty())
			{
				if (buf->GetNextPosition(pos,idx,cache))
				{
					cls->m_memman.ReleaseObj(cache);
				}
			}
		}
	}
	typedef struct _MemCache{
 		UINT nLen;
		char buf[CACHESIZE];
		DWORD dwCacheId;
	}MemCache;
	UINT m_LimitCnt;
	ITHMemCacheNotify *m_notify;
	void *m_adddata;
	//ÿ���ļ��б������������
	//int=offest,MemCache=��������
	typedef THMap<int,MemCache *> THMemFileBuf;
	//�ļ���������
	//int=bufidx,THMemFileBuf=ÿ���ļ��Ļ�������
	THMap<int,THMemFileBuf *> m_map;
	//�ļ���������ƻ���
	THLimitObjectBuffer<MemCache> m_memman;
	THMutexEx m_mapmtx;
	__int64 m_nCacheHitSize;
	__int64 m_nCacheNotHitSize;
};
