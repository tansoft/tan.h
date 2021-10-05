#pragma once

#include <THSyn.h>
#include <THStruct.h>

/**
* @brief 对象池管理类
* @author Barry
* @2008-04-08 新建类
*/
/**<pre>
使用Sample：
THObjectBuffer<ParseBuffer> buf;
注意没有释放的对象将会引起内存泄露。
</pre>*/
template <typename TCLS>
class THObjectBuffer
{
public:
	THObjectBuffer()
	{
	}
	virtual ~THObjectBuffer()
	{
		m_cs.Lock();
		TCLS *cls;
		THPosition pos;
		pos=m_listfree.GetStartPosition();
		while(!pos.IsEmpty())
		{
			if (m_listfree.GetNextPosition(pos,cls))
			{
				if (cls) delete cls;
			}
		}
		m_listfree.RemoveAll();
		m_cs.Unlock();
	}
	virtual TCLS *NewObj()
	{
		THSingleLock lock(&m_cs);
		TCLS *ret;
		if (m_listfree.GetSize()<=0)
		{
			ret=new TCLS;
			if (!ret) return NULL;
			return ret;
		}
		else
		{
			if (m_listfree.GetAt(0,ret,TRUE)) return ret;
		}
		return NULL;
	}
	virtual void ReleaseObj(TCLS *obj)
	{
		if (obj)
		{
			THSingleLock lock(&m_cs);
			m_listfree.AddHead(obj);
		}
	}
private:
	THList<TCLS *> m_listfree;
	THMutex m_cs;
};

/**
* @brief 限制个数对象池管理类
* @author Barry
* @2008-08-10 新建类
*/
/**<pre>
使用Sample：
注意：使用THLimitObjectBuffer生成的对象使用前必须调用IsObjValid判断对象是否还有效
THLimitObjectBuffer<ParseBuffer> buf;
buf.SetLimitCount(50);
ParseBuffer *p=buf.NewObj();
...
if (buf.IsObjValid(p))
{
	//use the p
	...
	buf.UpdateLastUseTime(p);
}
...
buf.ReleaseObj(p);
注意没有释放的对象将会引起内存泄露。
注意由于对象指针是可重用的，因此有可能造成的情况是：
obj *p1=newobj,obj in addr 1;
relase p1
obj *p2=newobj,obj in addr 1;
isobjvalid p1==true,so it is wrong
因此需要使用objid来判断
</pre>*/
template <typename TCLS>
class THLimitObjectBuffer : public THObjectBuffer<TCLS>
{
public:
	THLimitObjectBuffer()
	{
		m_nCnt=10;
	}

	virtual ~THLimitObjectBuffer()
	{
	}

	//当nCnt==0时为不限制
	void SetLimitCount(UINT nCnt){m_nCnt=nCnt;CheckLimitState();}

	//如果TCLS指针对象会重用，因此判断对象是否还有效时，除了判断TCLS指针是否还有效外，还需要在每次NewObj时，指定一与其他Object不同的值，用于区分obj
	virtual TCLS *NewObj(DWORD objid=0)
	{
		THSingleLock l(&m_cs);
		TCLS *cls=THObjectBuffer<TCLS>::NewObj();
		if (!cls) return NULL;
		UINT np=(UINT)(UINT_PTR)cls;
		m_map.SetAt(np,GetTickCount());
		m_objid.SetAt(np,objid);
		CheckLimitState();
		return cls;
	}
	//如果objid对不上号，则可认为释放的是之前的对象，不作处理，objid==0时，强制释放
	virtual void ReleaseObj(TCLS *obj,DWORD objid=0)
	{
		THSingleLock l(&m_cs);
		UINT np=(UINT)(UINT_PTR)obj;
		//it is only want to delete the prev obj,skip it
		if (objid!=0 && m_objid[np]!=objid) return;
		//check if the obj already valid
		if (m_map[np]!=0)
		{
			m_map.RemoveAt(np);
			THObjectBuffer<TCLS>::ReleaseObj(obj);
		}
		m_objid.RemoveAt(np);
	}
	//检查限制的状态，当限制值改变或新建对象时，都会主动调用
	virtual void CheckLimitState()
	{
		if (m_nCnt==0) return;
		THSingleLock l(&m_cs);
		while(m_map.GetSize()>m_nCnt)
		{
			THPosition pos=m_map.GetStartPosition();
			UINT ncnt;
			DWORD ti,nti=GetTickCount();
			UINT nMin=0;
			while(!pos.IsEmpty())
			{
				if (m_map.GetNextPosition(pos,ncnt,ti))
				{
					if (ti<=nti)
					{
						nti=ti;
						nMin=ncnt;
					}
				}
			}
			if (nMin!=0)
				ReleaseObj((TCLS *)((UINT_PTR)nMin));
		}
	}

	//更新对象的使用时间，以使用时间作为清理旧数据的标准
	virtual void UpdateLastUseTime(TCLS *obj,DWORD objid=0)
	{
		THSingleLock l(&m_cs);
		UINT np=(UINT)(UINT_PTR)obj;
		//it is only want to update the prev obj,skip it
		if (objid!=0 && m_objid[np]!=objid) return;
		DWORD dw;
		if (m_map.GetAt(np,dw))
			m_map.SetAt(np,GetTickCount());
	}

	//判断对象是否还有效，数据有可能已经被释放
	virtual BOOL IsObjValid(TCLS *obj,DWORD objid=0)
	{
		THSingleLock l(&m_cs);
		UINT np=(UINT)(UINT_PTR)obj;
		//it is only want to check the prev obj,skip it
		if (objid!=0 && m_objid[np]!=objid) return FALSE;
		return (m_map[np]!=0);
	}
private:
	UINT m_nCnt;
	THMap<UINT,DWORD> m_map;	///<用于选择超时对象
	THMap<UINT,DWORD> m_objid;	///<用于检测对象是否还有效
	THMutex m_cs;
};