#pragma once

#include <THSyn.h>
#include <THStruct.h>

/**
* @brief �ڴ����ù�����
* @author Barry
* @2008-04-03 �½���
*/
/**<pre>
ʹ��Sample��
TVALUEΪ����ָ�������,TUNITSIZEΪ�ڴ�ƬÿƬ�Ĵ�С��������ڴ��С��Ϊ��ֵ����
THMemManager<char,1024> buf;
</pre>*/
template <typename TVALUE,UINT TUNITSIZE>
class THMenManager
{
	THMenManager()
	{
		m_allocpool.SetFreeProc(FreePool);
	}
	virtual ~THMenManager()
	{
		m_mtx.Lock();
		m_freemap.RemoveAll();
		m_mtx.Unlock();
		m_allocpool.RemoveAll();
	}

	TVALUE *new(UINT allocsize)
	{
		if (allocsize==0) return NULL;
		UINT segsize=(allocsize+TUNITSIZE-1)/TUNITSIZE;
		//check free if have buffer
		PtrList *list;
		TVALUE *tmp;
		if (m_freemap.GetAt(segsize,list))
		{
			THSingleLock lock(&m_mtx);
			if (list && list->GetHead(tmp,TRUE)) return tmp;
		}
		tmp=new TVALUE[segsize*TUNITSIZE];
		if (!tmp) return NULL;
		m_allocpool.SetAt(tmp,segsize);
		return tmp;
	}

	void delete(TVALUE *val)
	{
		if (val==NULL) return;
		UINT segsize;
		if (!m_allocpool.GetAt(val,segsize))
		{
			ASSERT(FALSE);
			return;
		}
		PtrList *list;
		THSingleLock lock(&m_mtx);
		if (!m_freemap.GetAt(segsize,list))
		{
			list=new PtrList;
			if (!list) return;
			m_freemap.SetAt(segsize,list);
		}
		list->AddHead(val);
	}
private:
	static void FreePool(void *key,void *value,void *adddata)
	{
		delete [] (TVALUE *)key;
	}
	typedef THList<TVALUE *> PtrList;
	THMap<UINT,PtrList *> m_freemap;
	THMap<TVALUE *,UINT> m_allocpool;
	THMutex m_mtx;
};
