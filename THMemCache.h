#pragma once

#include <THStruct.h>
#include <THObjectBuffer.h>

/**
* @brief 内存缓存响应类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-08-16 新建类
*/
class ITHMemCacheNotify
{
public:
	/**
	* @brief 当缓存中没有对应数据，需要请求真实对象时，响应该函数
	* @param bufidx		读取对象的索引号
	* @param off		读取数据的偏移量
	* @param buf		返回数据缓冲
	* @param len		需要读取大小，返回为实际读取大小
	* @param adddata	全局的附加参数
	* @param curadddata	引发本次读取事件时，传入的附加参数
	* @return 读取是否成功
	*/
	virtual BOOL BufferNeedRead(int bufidx,UINT off,void *buf,UINT &len,void *adddata,void *curadddata)=0;
	/**
	* @brief 当需要写入更新数据时，响应该函数
	* @param bufidx		写入对象的索引号
	* @param off		写入数据的偏移量
	* @param buf		写入数据缓冲
	* @param len		写入数据大小
	* @param adddata	全局的附加参数
	* @param curadddata	引发本次写入事件时，传入的附加参数
	* @return 读取是否成功
	*/
	virtual BOOL BufferNeedWrite(int bufidx,UINT off,const void *buf,UINT len,void *adddata,void *curadddata)=0;
};

/**
* @brief 内存缓存类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-08-16 新建类
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
	* @brief 读取数据
	* @param bufidx		读取对象的索引号
	* @param off		读取数据的偏移量
	* @param buf		返回数据缓冲
	* @param len		需要读取大小，返回为实际读取大小
	* @param curadddata	引发本次读取事件时，传入的附加参数
	* @param bUpdateBuffer		如果缓存中没有数据，请求真实数据后，是否把数据缓存到缓存中；如果缓存已有数据，是否更新缓存数据的最后访问时间
	* @return 读取是否成功
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
			//索引位置
			UINT toff=off-partlen;
			m_mapmtx.ReadOnlyLock();
			THMemFileBuf *fbuf=m_map[bufidx];
			if (fbuf)
			{
				MemCache *cache=(*fbuf)[toff];
				if (cache && m_memman.IsObjValid(cache,toff+1))
				{
					//这里应该cache->nLen-partlen和tlen比较，但是这样怕cache->nLen比partlen小
					int rlen=min(cache->nLen,tlen+partlen);
					//已经没有缓存数据
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
					//这里应该cache->nLen-partlen和tlen比较，但是这样怕cache->nLen比partlen小
					int rlen=min(cache->nLen,tlen+partlen);
					//已经没有缓存数据
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
	* @brief 写入数据
	* @param bufidx		写入对象的索引号
	* @param off		写入数据的偏移量
	* @param buf		写入的数据缓冲
	* @param len		需要写入大小
	* @param curadddata	引发本次写入事件时，传入的附加参数
	* @param bUpdateBuffer		如果缓存中没有数据，仅写入真实数据；如果缓存已有数据，是否更新缓存数据的最后访问时间
	* @return 写入是否成功
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
	* @brief 清空缓存数据
	* @param bufidx		清空的对象的索引号
	*/
	void EmptyBuffer(int bufidx)
	{
		m_mapmtx.ChangeLock();
		m_map.RemoveAt(bufidx);
		m_mapmtx.ChangeUnlock();
	}

	/**
	* @brief 清空所有缓存数据
	*/
	void EmptyAllBuffer()
	{
		m_mapmtx.ChangeLock();
		m_map.RemoveAll();
		m_mapmtx.ChangeUnlock();
	}

	/**
	* @brief 设置缓存最大允许数
	* @param cnt		即改变CACHECOUNT的值，cnt*CACHESIZE=实际可缓存数据大小，0为不限制
	*/
	void SetLimitCount(int cnt)
	{
		m_LimitCnt=cnt;
		m_memman.SetLimitCount(cnt);
	}

	/**
	* @brief 获取缓存击中比例
	* @param nHit		击中缓存字节数
	* @param nNoHit		没有击中缓存字节数
	*/
	void GetCacheHit(__int64 &nHit,__int64 &nNoHit)
	{
		nHit=m_nCacheHitSize;
		nNoHit=m_nCacheNotHitSize;
	}

	/**
	* @brief 重置缓存击中计数
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
	//每个文件中保存的数据索引
	//int=offest,MemCache=缓存数据
	typedef THMap<int,MemCache *> THMemFileBuf;
	//文件储存索引
	//int=bufidx,THMemFileBuf=每个文件的缓存数据
	THMap<int,THMemFileBuf *> m_map;
	//文件缓存的限制机制
	THLimitObjectBuffer<MemCache> m_memman;
	THMutexEx m_mapmtx;
	__int64 m_nCacheHitSize;
	__int64 m_nCacheNotHitSize;
};
