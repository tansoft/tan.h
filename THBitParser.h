#pragma once

/**
* @brief 位处理类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-02-23 新建类
* @2008-11-30 增加网络字节序转换参数，增加RollBuffer操作等函数
*/
/**<pre>
使用Sample：
</pre>*/
class THBitParser
{
public:
	THBitParser()
	{
		m_pBuffer=NULL;
		m_nBufferLen=0;
		m_nBitLen=0;
		m_nCachedFristNoSetBit=0;
	}
	virtual ~THBitParser()
	{
		if (m_pBuffer)
		{
			delete [] m_pBuffer;
			m_pBuffer=NULL;
		}
		m_nBufferLen=0;
		m_nBitLen=0;
		m_nCachedFristNoSetBit=0;
	}

	BOOL SetParseDataBuffer(const void *buffer,UINT nBitLen)
	{
		if (!m_pBuffer || nBitLen > m_nBufferLen*8)
		{
			if (m_pBuffer)
			{
				delete [] m_pBuffer;
				m_nBufferLen=0;
				m_nBitLen=0;
			}
			UINT size=max(nBitLen/4,1024);
			m_pBuffer=new unsigned char[size];//init twice length of setbuffer
			if (!m_pBuffer) return FALSE;
			memset(m_pBuffer,0,size);
			m_nBufferLen=size;
		}
		if (nBitLen>0)
		{
			UINT copy=(nBitLen+7)/8;
			//may be copy to it self,memmove is safey
			memmove(m_pBuffer,buffer,copy);
			if (m_nBufferLen>copy) memset(m_pBuffer+copy,0,m_nBufferLen-copy);
		}
		else if (m_nBufferLen>0)
			memset(m_pBuffer,0,m_nBufferLen);
		m_nBitLen=nBitLen;
		m_nCachedFristNoSetBit=0;//reflush it
		return TRUE;
	}

	//返回的大小是字节数大小，因为函数返回的内容如果进行更改，将会可能使m_nCachedFristNoSetBit失效
	const void *GetParseDataBuffer(UINT *pBufferLen) const
	{
		if (pBufferLen)
			*pBufferLen=(m_nBitLen+7)/8;
		return (const U8 *)m_pBuffer;
	}

	//bH为本地顺序还是网络顺序
	inline BOOL GetBit(UINT nBitOffest,BOOL bH=TRUE) const
	{
		const unsigned char maskmap[]={0x1,0x2,0x4,0x8,0x10,0x20,0x40,0x80};
		const unsigned char maskmapn[]={0x80,0x40,0x20,0x10,0x8,0x4,0x2,0x1};
		if (nBitOffest>=m_nBitLen) return FALSE;
		UINT off=nBitOffest/8;
		unsigned char mask;
		if (bH)
			mask=maskmap[nBitOffest%8];
		else
			mask=maskmapn[nBitOffest%8];
		return ((*(m_pBuffer+off))&mask)!=0;
	}

	//获取nBitOffest开始nCount位组成的二进制值,最多支持获取sizeof(U32)*8位,bH为本地顺序还是网络顺序
	U32 GetBit2U32(UINT nBitOffest,UINT nCount,BOOL bH=TRUE)
	{
		if (nCount>sizeof(U32)*8) return 0;
		U32 ret=0;
		THString str;
		for(UINT i=0;i<nCount;i++)
		{
			if (GetBit(nBitOffest+i,bH))
			{
				if (bH)
					ret|=(1<<i);
				else
					ret|=(1<<(nCount-1-i));
				str+=_T("1");
			}
			else
				str+=_T("0");
		}
		//if (!bH) ret=ntohl(ret);
		return ret;
	}

	inline BOOL SetBit(BOOL bBitValue)
	{
		return SetBit(m_nBitLen,bBitValue);
	}

	BOOL SetBit(UINT nBitOffest,BOOL bBitValue,BOOL bH=TRUE)
	{
		const unsigned char maskmap[]={0x1,0x2,0x4,0x8,0x10,0x20,0x40,0x80};
		const unsigned char maskmapn[]={0x80,0x40,0x20,0x10,0x8,0x4,0x2,0x1};
		if (nBitOffest>=m_nBufferLen*8)
		{
			//Realloc buffer
			UINT size=max(nBitOffest/4,1024);
			unsigned char *tmp=new unsigned char[size];
			if (!tmp) return FALSE;
			memset(tmp,0,size);
			memcpy(tmp,m_pBuffer,m_nBufferLen);
			delete [] m_pBuffer;
			m_pBuffer=tmp;
			m_nBufferLen=size;
		}
		UINT off=nBitOffest/8;
		unsigned char mask;
		if (bH)
			mask=maskmap[nBitOffest%8];
		else
			mask=maskmapn[nBitOffest%8];
		if (bBitValue)
			*(m_pBuffer+off)=(*(m_pBuffer+off))|mask;
		else
			*(m_pBuffer+off)=(*(m_pBuffer+off))&(~mask);
		if (m_nBitLen<nBitOffest+1) m_nBitLen=nBitOffest+1;
		m_nCachedFristNoSetBit=0;//reflush it
		return TRUE;
	}

	inline BOOL SetBits(UINT nBitOffest,BOOL *bBitValue,UINT nCount)
	{
		for(UINT i=0;i<nCount;i++)
			if (!SetBit(nBitOffest+i,*(bBitValue+i))) return FALSE;
		return TRUE;
	}

	//滚动bm，取消前面的n个字节数据，使用字节对齐是为了加速
	void RollBuffer(UINT nBytes)
	{
		if (m_nBitLen>0)
		{
			if (nBytes*8>=m_nBitLen)
			{
				Empty();
			}
			else
			{
				m_nBitLen-=nBytes*8;
				UINT copyoff=(m_nBitLen+7)/8;
				memmove(m_pBuffer,m_pBuffer+nBytes,copyoff);
				if (m_nBufferLen>copyoff) memset(m_pBuffer+copyoff,0,m_nBufferLen-copyoff);
			}
		}
	}

	THString Print() const
	{
		THString ret;
		UINT plen=(m_nBitLen+7)/8;
		for(UINT i=0;i<plen;i++)
			ret.AppendFormat(_T("%02x"),*(m_pBuffer+i));
		return ret;
	}

	inline void Empty()
	{
		m_nBitLen=0;
		if (m_nBufferLen>0) memset(m_pBuffer,0,m_nBufferLen);
		m_nCachedFristNoSetBit=0;
	}

	//把bitset传到某buffer中，以buflen为准，缓冲不够只返回前面的，多出的地方memset为0
	BOOL CopyToBuffer(void *outbuf,UINT buflen)
	{
		UINT copybuf=(m_nBitLen+7)/8;
		//如果buffer过小，则只返回前面的
		if (copybuf>buflen) copybuf=buflen;
		memcpy(outbuf,m_pBuffer,copybuf);
		if (buflen>copybuf) memset((char *)outbuf+copybuf,0,buflen-copybuf);
		return TRUE;
	}

	inline UINT GetBitLen(){return m_nBitLen;}

	BOOL IsAllBitSet(UINT nBitLen) const
	{
		UINT fastcheck=nBitLen/8;
		UINT nReMain=nBitLen%8;
		UINT i;
		if (m_nCachedFristNoSetBit!=0)
			i=(m_nCachedFristNoSetBit-1)/8;//在m_nCachedFristNoSetBit之前的肯定是已完成的
		for(i=0;i<fastcheck;i++)
			if (*(m_pBuffer+i)!=0xff) return FALSE;
		fastcheck*=8;
		for(i=0;i<nReMain;i++)
			if (!GetBit(fastcheck+i)) return FALSE;
		return TRUE;
	}

	BOOL SetMaxBit(UINT nBitLen)
	{
		//ASSERT(nBitLen<=256);
		if (m_nBitLen!=nBitLen)
		{
			if (m_nBitLen<nBitLen)
			{
				if (nBitLen<m_nBufferLen*8)
					//buffer is enough,just set m_nBitLen
					m_nBitLen=nBitLen;
				else
				{
					//inc the buffer
					for(UINT i=m_nBitLen;i<nBitLen;i++) SetBit(i,0);
				}
			}
			else
			{
				//reduce the buffer,just reset the bytes which may be reuse
				for(UINT i=nBitLen;i<nBitLen+8 && i<m_nBitLen;i++) SetBit(i,0);
				UINT copy=(nBitLen+7)/8;
				if (m_nBufferLen>copy) memset(m_pBuffer+copy,0,m_nBufferLen-copy);
				m_nBitLen=nBitLen;
			}
			return TRUE;
		}
		return FALSE;
	}

	UINT GetFirstNoSetBit(UINT nStartBit) const
	{
		if (m_nCachedFristNoSetBit!=0 && nStartBit<m_nCachedFristNoSetBit) return m_nCachedFristNoSetBit;
		UINT nUpBit=nStartBit;
		while(nUpBit<m_nBitLen)
		{
			if (nUpBit%8==0)
			{
				//if at the bytes start
				if (*(m_pBuffer+nUpBit/8)==0xff)
					nUpBit+=8;
				else
				{
					if (!GetBit(nUpBit)) break;
					nUpBit++;
				}
			}
			else
			{
				if (!GetBit(nUpBit)) break;
				nUpBit++;
			}
		}
		if (nStartBit==0) m_nCachedFristNoSetBit=nUpBit;
		return nUpBit;
	}
private:
	unsigned char *m_pBuffer;
	UINT m_nBitLen;
	UINT m_nBufferLen;
	mutable volatile UINT m_nCachedFristNoSetBit;		///<用于加速获取最后没有设置位
};
