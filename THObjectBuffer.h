#pragma once

#include <THSyn.h>
#include <THStruct.h>

/**
* @brief ����ع�����
* @author Barry
* @2008-04-08 �½���
*/
/**<pre>
ʹ��Sample��
THObjectBuffer<ParseBuffer> buf;
ע��û���ͷŵĶ��󽫻������ڴ�й¶��
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
* @brief ���Ƹ�������ع�����
* @author Barry
* @2008-08-10 �½���
*/
/**<pre>
ʹ��Sample��
ע�⣺ʹ��THLimitObjectBuffer���ɵĶ���ʹ��ǰ�������IsObjValid�ж϶����Ƿ���Ч
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
ע��û���ͷŵĶ��󽫻������ڴ�й¶��
ע�����ڶ���ָ���ǿ����õģ�����п�����ɵ�����ǣ�
obj *p1=newobj,obj in addr 1;
relase p1
obj *p2=newobj,obj in addr 1;
isobjvalid p1==true,so it is wrong
�����Ҫʹ��objid���ж�
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

	//��nCnt==0ʱΪ������
	void SetLimitCount(UINT nCnt){m_nCnt=nCnt;CheckLimitState();}

	//���TCLSָ���������ã�����ж϶����Ƿ���Чʱ�������ж�TCLSָ���Ƿ���Ч�⣬����Ҫ��ÿ��NewObjʱ��ָ��һ������Object��ͬ��ֵ����������obj
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
	//���objid�Բ��Ϻţ������Ϊ�ͷŵ���֮ǰ�Ķ��󣬲�������objid==0ʱ��ǿ���ͷ�
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
	//������Ƶ�״̬��������ֵ�ı���½�����ʱ��������������
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

	//���¶����ʹ��ʱ�䣬��ʹ��ʱ����Ϊ��������ݵı�׼
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

	//�ж϶����Ƿ���Ч�������п����Ѿ����ͷ�
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
	THMap<UINT,DWORD> m_map;	///<����ѡ��ʱ����
	THMap<UINT,DWORD> m_objid;	///<���ڼ������Ƿ���Ч
	THMutex m_cs;
};