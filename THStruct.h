#pragma once

#define THSYSTEM_INCLUDE_STL_STRUCT
#include <THSystem.h>
#include <THString.h>
#include <THSyn.h>

template <typename TVALUE> class THList;
template <typename TKEY,typename TVALUE> class THMap;
template <typename TKEY,typename TKEY2,typename TVALUE> class THDoubleKeyMap;
template <typename TKEY,typename TVALUE,unsigned int TMAXPOOL> class THDynMapPool;

/**
* @brief ���ݽṹλ��������
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-06-01 �½���
*/
/**<pre>
ʹ��Sample��
</pre>*/
class THPosition
{
public:
	THPosition(){m_pItem=m_pPreItem=m_pNextItem=NULL;}
	virtual ~THPosition(){}
	template <typename TVALUE> friend class THList;
	template <typename TKEY,typename TVALUE> friend class THMap;
	template <typename TKEY,typename TKEY2,typename TVALUE> friend class THDoubleKeyMap;
	template <typename TKEY,typename TVALUE,unsigned int TMAXPOOL> friend class THDynMapPool;

	THPosition& operator=(const THPosition& other)
	{
		m_pItem=other.m_pItem;
		m_pPreItem=other.m_pPreItem;
		m_pNextItem=other.m_pNextItem;
		return *this;
	}
	BOOL operator ==(THPosition pos) const
	{
		return (m_pItem==pos.GetCurrentItem() && m_pPreItem==pos.GetPreItem() && m_pNextItem==pos.GetNextItem());
	}
	BOOL IsEmpty() const{return (m_pItem==NULL);}
	void Empty(){m_pItem=m_pPreItem=m_pNextItem=NULL;}
private:
	void SetPositionData(const void *pItem,const void *pPreItem=NULL,const void *pNextItem=NULL)
	{
		m_pItem=pItem;
		m_pPreItem=pPreItem;
		m_pNextItem=pNextItem;
	}
	const void *GetCurrentItem() const{return m_pItem;}
	const void *GetPreItem() const{return m_pPreItem;}
	const void *GetNextItem() const{return m_pNextItem;}

	const void *m_pItem;				///>������ǰitem��ָ��λ��
	const void *m_pPreItem;				///>������ǰitemǰһ��item��ָ��λ�ã����ڼ��٣���һ��ʹ��
	const void *m_pNextItem;			///>������ǰitem��һ��item��ָ��λ�ã����ڼ��٣���һ��ʹ��
};

//not finish
/**
* @brief �ڴ����������
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-06-02 �½���
*/
/**<pre>
ʹ��Sample��
</pre>*/
template <typename TVALUE>
class THMemPool
{
public:
	THMemPool(int nIncreaseSize=16){m_nInc=nIncreaseSize;}
	virtual ~THMemPool()
	{
	}

	TVALUE *New()
	{
		return new TVALUE;
	}

	void Delete(TVALUE *pVal)
	{
		delete pVal;
	}

private:
	int m_nInc;
	//
};

typedef void (*StructCallBack)(void *key,void *value,void *adddata);

/**
* @brief ������ֵ�ڴ��ͷ���
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-06-01 �½���
*/
/**<pre>
ʹ��Sample��
  ���ڸ������ݽṹ�ĸ���
  ͨ��SetFreeProc����cb������ֱ�Ӽ̳���д����FreeProcʵ�ֺ󣬵����ݽṹ����Ҫ�ͷż�ֵʱ������cb���������FreeProc�������������ͷš�
</pre>*/
template <typename TKEY,typename TVALUE>
class THMemFreeProc
{
public:
	THMemFreeProc(){m_cb=NULL;m_adddata=NULL;}
	virtual ~THMemFreeProc(){}

	void SetFreeProc(StructCallBack cb,void *pAddData=NULL)
	{
		m_cb=cb;
		m_adddata=pAddData;
	}
protected:
	virtual void FreeProc(TKEY key,TVALUE value)
	{
		if (m_cb) m_cb((void *)(INT_PTR)key,(void *)(INT_PTR)value,m_adddata);
	}

	StructCallBack m_cb;
	void *m_adddata;
};

/**
* @brief ����List��
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-06-01 �½���
* @2007-07-09 ����Copy����
*/
/**<pre>
ʹ��Sample��
	//��Header�����ȶ�Tail��������Ҫ��Щ
	void FreeCallBack(void *key,void *value,void *adddata)
	{
		THDebug(_T("Free:%d"),value);
		delete [] value;
	}

	THList<char *> m_list;
	m_list.SetFreeProc(FreeCallBack);
	m_list.AddHead(new char[123]);
	m_list.AddTail(new char[123]);
	m_list.AddHead(new char[123]);
	m_list.AddHead(new char[123]);
	THDebug(_T("size:%d"),m_list.GetSize());
	//�������ʹ��bRemove=FALSE,��Ϊ����������ָ�룬��������FreeProc
	char *tmp=m_list.GetHead(FALSE);
	//use pointer tmp
	m_list.RemoveHead();
	//ʹ��THPosition��ʽö�ټ�ֵ,Ч�ʽϸ�
	THPosition pos=m_list.GetStartPosition();
	while(!pos.IsEmpty())
	{
		if (m_list.GetNextPosition(pos,tmp))
		{
			m_list.SetAt(pos,char[123]);//����ĳ����ֵ����
			m_list.GetAt(pos,tmp,FALSE);
			THDebug(_T("item:%d"),tmp);
		}
	}
	//ʹ��idx��ʽö�ټ�ֵ,Ч�ʽϵ�
	for(UINT i=0;i<m_list.GetSize();i++)
	{
		THDebug(_T("item[%d]:%d"),i,m_list[i]);
	}
	m_list.RemoveAll();
</pre>*/
template <typename TVALUE>
class THList : public THMemFreeProc<TVALUE,TVALUE>
{
public:
	THList(){m_pList=NULL;m_nBufCnt=0;}
	virtual ~THList(void){RemoveAll();}
	BOOL AddHead(TVALUE val)
	{
		pListItem item=new ListItem;//m_pool.New();
		if (!item) return FALSE;
		if (m_pList)
			item->next=m_pList;
		else
			item->next=NULL;
		m_pList=item;
		item->item=val;
		m_nBufCnt++;
		return TRUE;
	}
	BOOL AddTail(TVALUE val)
	{
		pListItem item=new ListItem;//m_pool.New();
		if (!item) return FALSE;
		if (m_pList)
		{
			pListItem tmp=m_pList;
			while(tmp->next) tmp=tmp->next;
			tmp->next=item;
		}
		else
			m_pList=item;
		item->next=NULL;
		item->item=val;
		m_nBufCnt++;
		return TRUE;
	}
	//����ע�⣬�����ָ����ֵ����������FreeProc���������ͷ��˷���ֵ
	BOOL GetHead(TVALUE &val,BOOL bRemove)
	{
		if (!m_pList) return FALSE;
		val=m_pList->item;
		if (bRemove)
			RemoveHead();
		return TRUE;
	}
	//����ע�⣬�����ָ����ֵ����������FreeProc���������ͷ��˷���ֵ
	TVALUE GetHead(BOOL bRemove)
	{
		TVALUE val=NULL;
		GetHead(val,bRemove);
		return val;
	}
	//����ע�⣬�����ָ����ֵ����������FreeProc���������ͷ��˷���ֵ
	BOOL GetTail(TVALUE val,BOOL bRemove)
	{
		if (!m_pList) return FALSE;
		//����ֱ�ӵ�����Ϊ�˼��٣���Ӧ����RemoveHead��
		pListItem tmp=m_pList;
		pListItem lasttmp=NULL;
		while(tmp->next)
		{
			lasttmp=tmp;
			tmp=tmp->next;
		}
		val=tmp->item;
		if (bRemove)
		{
			if (lasttmp)
				lasttmp->next=NULL;
			else
				m_pList=NULL;
			FreeProc(tmp->item,tmp->item);
			delete tmp;//m_pool.Delete(tmp);
		}
		return TRUE;
	}
	//����ע�⣬�����ָ����ֵ����������FreeProc���������ͷ��˷���ֵ
	TVALUE GetTail(BOOL bRemove)
	{
		TVALUE val=NULL;
		GetTail(val,bRemove);
		return val;
	}
	inline UINT GetCount() const{return GetSize();}
	UINT GetSize() const
	{
		return m_nBufCnt;
/*		UINT count=0;
		pListItem tmp=m_pList;
		while(tmp)
		{
			count++;
			tmp=tmp->next;
		}
		return count;*/
	}
	void RemoveAll()
	{
		pListItem tmp;
		pListItem tmp2;
		tmp=m_pList;
		//quick set the header to NULL,so can avoid recall
		m_pList=NULL;
		m_nBufCnt=0;
		while(tmp)
		{
			tmp2=tmp->next;
			FreeProc(tmp->item,tmp->item);
			delete tmp;//m_pool.Delete(tmp);
			tmp=tmp2;
		}
	}
	BOOL RemoveHead()
	{
		if (!m_pList) return FALSE;
		pListItem item=m_pList;
		m_pList=m_pList->next;
		FreeProc(item->item,item->item);
		delete item;//m_pool.Delete(item);
		m_nBufCnt--;
		return TRUE;
	}
	BOOL RemoveTail()
	{
		if (!m_pList) return FALSE;
		pListItem tmp=m_pList;
		pListItem lasttmp=NULL;
		while(tmp->next)
		{
			lasttmp=tmp;
			tmp=tmp->next;
		}
		if (lasttmp)
			lasttmp->next=NULL;
		else
			m_pList=NULL;
		FreeProc(tmp->item,tmp->item);
		delete tmp;//m_pool.Delete(tmp);
		m_nBufCnt--;
		return TRUE;
	}
	BOOL Remove(TVALUE val)
	{
		if (!m_pList) return FALSE;
		pListItem tmp=m_pList;
		pListItem lasttmp=NULL;
		while(tmp)
		{
			if (tmp->item==val)
			{
				if (lasttmp)
					lasttmp->next=tmp->next;
				else
					m_pList=tmp->next;
				FreeProc(tmp->item,tmp->item);
				delete tmp;//m_pool.Delete(tmp);
				m_nBufCnt--;
				return TRUE;
			}
			lasttmp=tmp;
			tmp=tmp->next;
		}
		return FALSE;
	}
	BOOL IsExist(TVALUE val) const
	{
		if (!m_pList) return FALSE;
		pListItem tmp=m_pList;
		while(tmp)
		{
			if (tmp->item==val) return TRUE;
		}
		return FALSE;
	}
	THPosition Find(TVALUE val) const
	{
		THPosition pos;
		if (m_pList)
		{
			pListItem tmp=m_pList;
			pListItem lasttmp=NULL;
			while(tmp)
			{
				if (tmp->item==val)
				{
					pos.SetPositionData(tmp,lasttmp,tmp->next);
					break;
				}
				lasttmp=tmp;
				tmp=tmp->next;
			}
		}
		return pos;
	}
	THPosition FindByIdx(int idx) const
	{
		THPosition ret;
		int count=0;
		TVALUE tmpval;
		THPosition lastpos;
		THPosition pos=GetStartPosition();
		while(count<=idx && !pos.IsEmpty())
		{
			lastpos=pos;
			GetNextPosition(pos,tmpval);
			count++;
		}
		if (count>idx)
		{
			//���ҳɹ�
			ret=lastpos;
		}
		return ret;
	}
	THPosition GetStartPosition() const
	{
		//pos pointer ��ǰ��ֵָ��,��һ��ֵָ��,��һ��ֵָ��
		THPosition pos;
		if (!m_pList) return pos;
		pos.SetPositionData(m_pList,NULL,m_pList->next);
		return pos;
	}
	BOOL GetNextPosition(THPosition &pos,TVALUE &val) const
	{
		if (pos.IsEmpty()) return FALSE;
		const pListItem item=(const pListItem)pos.GetCurrentItem();
		val=item->item;
		pos.SetPositionData(item->next,item,(item->next)?item->next->next:NULL);
		return TRUE;
	}
	BOOL AddAfter(const THPosition &pos,TVALUE val)
	{
		if (pos.IsEmpty()) return AddTail(val);
		pListItem item=new ListItem;//m_pool.New();
		if (!item) return FALSE;
		pListItem tmp=(pListItem)(void *)pos.GetCurrentItem();
		item->item=val;
		item->next=tmp->next;
		tmp->next=item;
		m_nBufCnt++;
		return TRUE;
	}
	BOOL AddBefore(const THPosition &pos,TVALUE val)
	{
		if (pos.IsEmpty()) return AddHead(val);
		pListItem item=new ListItem;//m_pool.New();
		if (!item) return FALSE;
		pListItem tmp=(pListItem)(void *)pos.GetCurrentItem();
		pListItem pre=(pListItem)(void *)pos.GetPreItem();
		item->item=val;
		if (pre)
			pre->next=item;
		else
			m_pList=item;
		item->next=tmp;
		m_nBufCnt++;
		return TRUE;
	}
	BOOL RemoveAt(const THPosition &pos)
	{
		if (pos.IsEmpty()) return FALSE;
		pListItem tmp=(pListItem)(void *)pos.GetCurrentItem();
		pListItem pre=(pListItem)(void *)pos.GetPreItem();
		if (pre)
			pre->next=tmp->next;
		else
			m_pList=tmp->next;
		m_nBufCnt--;
		FreeProc(tmp->item,tmp->item);
		delete tmp;//m_pool.Delete(tmp);
		return TRUE;
	}

	//Low��ͷ�ļ����������ڰ�Set��Remove����ĵ���FreeProc��Lock���֣��Ա���Lockס
	TVALUE LowRemoveAt(const THPosition &pos)
	{
		if (pos.IsEmpty()) return NULL;
		pListItem tmp=(pListItem)(void *)pos.GetCurrentItem();
		pListItem pre=(pListItem)(void *)pos.GetPreItem();
		if (pre)
			pre->next=tmp->next;
		else
			m_pList=tmp->next;
		m_nBufCnt--;
		TVALUE val=tmp->item;
		delete tmp;
		return val;
	}

	TVALUE LowSetAt(const THPosition &pos,TVALUE newval)
	{
		if (pos.IsEmpty()) return NULL;
		TVALUE val;
		const pListItem item=(const pListItem)pos.GetCurrentItem();
		if (item->item!=newval)
		{
			val=item->item;
			item->item=newval;
		}
		else
			val=NULL;
		return val;
	}

	void LowFree(TVALUE item)
	{
		if (item) FreeProc(item,item);
	}

	//����ע�⣬�����ָ����ֵ����������FreeProc���������ͷ��˷���ֵ
	BOOL GetAt(THPosition &pos,TVALUE &val,BOOL bRemove)
	{
		val=NULL;
		if (pos.IsEmpty()) return FALSE;
		pListItem tmp=(pListItem)(void *)pos.GetCurrentItem();
		val=tmp->item;
		if (bRemove)
			return RemoveAt(pos);
		return TRUE;
	}
	//����ע�⣬�����ָ����ֵ����������FreeProc���������ͷ��˷���ֵ
	TVALUE GetAt(THPosition &pos,BOOL bRemove)
	{
		TVALUE val=NULL;
		GetAt(pos,val,bRemove);
		return val;
	}
	//ע�⣬ʹ��idx���ܻ��THPosition�ͺܶ�
	//����ע�⣬�����ָ����ֵ����������FreeProc���������ͷ��˷���ֵ
	BOOL GetAt(int idx,TVALUE &val,BOOL bRemove)
	{
		return GetAt(FindByIdx(idx),val,bRemove);
	}
	//ע�⣬ʹ��idx���ܻ��THPosition�ͺܶ�
	//����ע�⣬�����ָ����ֵ����������FreeProc���������ͷ��˷���ֵ
	TVALUE GetAt(int idx,BOOL bRemove)
	{
		TVALUE val=NULL;
		GetAt(idx,val,bRemove);
		return val;
	}
	BOOL SetAt(const THPosition &pos,TVALUE newval)
	{
		if (pos.IsEmpty()) return FALSE;
		const pListItem item=(const pListItem)pos.GetCurrentItem();
		if (item->item!=newval)
		{
			FreeProc(item->item,item->item);
			item->item=newval;
		}
		return TRUE;
	}
	BOOL SetAt(int idx,TVALUE newval)
	{
		SetAt(FindByIdx(idx),newval);
	}
	//ע�⣬ʹ��idx���ܻ��THPosition�ͺܶ�
	TVALUE operator[](int nIndex)
	{
		return GetAt(nIndex,FALSE);
	}
	BOOL Copy(const THList<typename TVALUE> *src)
	{
		//�Ƿ���Ҫ�ҽ�FreeProc
		if (!src) return FALSE;
		TVALUE tmp;
		THPosition pos=src->GetStartPosition();
		while(!pos.IsEmpty())
		{
			if (src->GetNextPosition(pos,tmp))
				AddHead(tmp);
		}
		return TRUE;
	}
private:
	typedef struct _ListItem{
		TVALUE item;
		struct _ListItem *next;
	}ListItem,*pListItem;			///<�ڵ����
	pListItem m_pList;				///<���ڵ�
	//THMemPool<ListItem> m_pool;		///<�ڴ�����
	volatile UINT m_nBufCnt;
};

/**
* @brief ����Map��
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-06-01 �½���
*/
/**<pre>
ʹ��Sample��
	void FreeMapCallBack(void *key,void *value,void *adddata)
	{
		THDebug(_T("Free key:%d 's value %d"),(int)(INT_PTR)key,value);
		delete [] value;
	}
	THMap<int,char *> m_map;
	m_map.SetFreeProc(FreeMapCallBack);
	m_map.SetAt(10,new char[123]);
	m_map.RemoveAt(10);
	m_map.SetAt(11,new char[124]);
	m_map.SetAt(12,new char[124]);
	m_map.SetAt(13,new char[124]);
	m_map.SetAt(21,new char[124]);
	m_map.SetAt(101,new char[124]);
	pos=m_map.GetStartPosition();
	int key;
	char *val;
	while(!pos.IsEmpty())
	{
		if (m_map.GetNextPosition(pos,key,val))
		{
			THDebug(_T("map member key:%d,val:%d"),key,val);
		}
	}
	m_map.RemoveAll();
</pre>*/
template <typename TKEY,typename TVALUE>
class THMap : public THMemFreeProc<TKEY,TVALUE>
{
private:
	typedef struct _MapItem{
		TVALUE val;
		TKEY key;
	}MapItem,*pMapItem;			///<�ڵ����
	static void MapItemFreeCallBack(void *key,void *value,void *adddata)
	{
		pMapItem tmp=(pMapItem)value;
		THMap *p=(THMap *)adddata;
		//Map's Free Proc
		if (p)
		{
			p->FreeProc(tmp->key,tmp->val);
			delete tmp;//p->m_pool.Delete(tmp);
		}
	}
public:
	THMap(UINT nHashSize=16)
	{
		ASSERT(nHashSize>1);
		m_nHash=nHashSize;
		m_list=new THList<pMapItem>[m_nHash];
		for(UINT i=0;i<m_nHash;i++)
			m_list[i].SetFreeProc(MapItemFreeCallBack,this);
		m_nBufUpdate=0;
	}
	virtual ~THMap(void)
	{
		RemoveAll();
		if (m_list)
		{
			delete [] m_list;
			m_list=NULL;
		}
	}
	BOOL SetAt(TKEY key,TVALUE val)
	{
		if (!m_list) return FALSE;
		pMapItem tmp=new MapItem;//m_pool.New();
		if (!tmp) return FALSE;
		tmp->key=key;
		tmp->val=val;
		UINT idx=HashKey(key);
		//���������Ӧ��ס����Ȼ����ɶ�ε���ʱ֮ǰ��THPositionʧЧ��
		m_mtx.Lock(30);
		THPosition pos=FindKeyPos(idx,key);
		if (pos.IsEmpty())
		{
			BOOL bRet=m_list[idx].AddTail(tmp);
			m_mtx.Unlock(30);
			if (bRet) m_nBufUpdate++;
			return bRet;
		}
		//BOOL bRet=m_list[idx].SetAt(pos,tmp);
		pMapItem old=m_list[idx].LowSetAt(pos,tmp);
		m_mtx.Unlock(30);
		BOOL bRet=(old!=NULL);
		m_list[idx].LowFree(old);
		return bRet;
	}
	BOOL RemoveAt(TKEY key)
	{
		if (!m_list) return FALSE;
		UINT idx=HashKey(key);
		//���������Ӧ��ס����Ȼ����ɶ�ε���ʱ֮ǰ��THPositionʧЧ��
		m_mtx.Lock(31);
		THPosition pos=FindKeyPos(idx,key);
		if (pos.IsEmpty())
		{
			m_mtx.Unlock(31);
			return FALSE;
		}
		//BOOL bRet=m_list[idx].RemoveAt(pos);
		pMapItem tmp=m_list[idx].LowRemoveAt(pos);
		BOOL bRet=(tmp!=NULL);
		if (bRet) m_nBufUpdate--;
		m_mtx.Unlock(31);
		m_list[idx].LowFree(tmp);
		return bRet;
	}
	void RemoveAll()
	{
		if (!m_list) return;
		m_nBufUpdate=0;
		for(UINT i=0;i<m_nHash;i++)
			m_list[i].RemoveAll();
	}
	THPosition GetStartPosition() const
	{
		//pos pointer ��ǰ��ֵָ��,��һ��Hash idx,��list�е�idx
		THPosition pos;
		if (m_list)
		{
			for(UINT idx=0;idx<m_nHash;idx++)
			{
				if (m_list[idx].GetSize()>0)
				{
					UINT subidx=0;
					pos.SetPositionData(m_list[idx][0],(void *)(UINT_PTR)idx,(void *)(UINT_PTR)subidx);
					break;
				}
			}
		}
		return pos;
	}
	BOOL GetNextPosition(THPosition &pos,TKEY &key,TVALUE &val) const
	{
		if (!m_list) return FALSE;
		if (pos.IsEmpty()) return FALSE;
		const pMapItem tmp=(const pMapItem)pos.GetCurrentItem();
		UINT idx=(UINT)(UINT_PTR)(void *)pos.GetPreItem();
		UINT subidx=(UINT)(UINT_PTR)(void *)pos.GetNextItem();
		key=tmp->key;
		val=tmp->val;
		pos.SetPositionData(NULL);
		if (subidx+1<m_list[idx].GetSize())
		{
			subidx++;
			pos.SetPositionData(m_list[idx][subidx],(void *)(UINT_PTR)idx,(void *)(UINT_PTR)subidx);
		}
		else
		{
			//the same list not have any more data,so next list
			idx++;
			subidx=0;
			for(;idx<m_nHash;idx++)
			{
				if (m_list[idx].GetSize()>0)
				{
					pos.SetPositionData(m_list[idx][0],(void *)(UINT_PTR)idx,(void *)(UINT_PTR)subidx);
					break;
				}
			}
		}
		return TRUE;
	}
	//��ȡ��ӦMap��ֵ��pos
	THPosition GetKeyPos(TKEY key) const
	{
		THPosition ret;
		if (m_list)
		{
			UINT idx=HashKey(key);
			UINT subidx=0;
			pMapItem tmpval;
			THPosition pos=m_list[idx].GetStartPosition();
			while(!pos.IsEmpty())
			{
				if (m_list[idx].GetNextPosition(pos,tmpval))
				{
					if (tmpval->key==key)
					{
						ret.SetPositionData(m_list[idx][subidx],(void *)(UINT_PTR)idx,(void *)(UINT_PTR)subidx);
					}
					subidx++;
				}
			}
		}
		return ret;
	}
	inline UINT GetSize()
	{
		return m_nBufUpdate;
	}
	inline UINT GetCount()
	{
		return m_nBufUpdate;
		//if (m_nBufUpdate!=0) return m_nBufUpdate;
		/*UINT cnt=0;
		CMyPosition pos=GetStartPosition();
		TKEY key;
		TVALUE val;
		while(!pos.IsEmpty())
		{
			GetNextPosition(pos,key,val);
			cnt++;
		}
		m_nBufUpdate=cnt;
		return cnt;*/
	}
	BOOL GetAt(TKEY key,TVALUE &val) const
	{
		if (!m_list) return FALSE;
		//Get����ֵ��������ס
		//m_mtx.Lock();
		UINT idx=HashKey(key);
		THPosition pos=FindKeyPos(idx,key);
		if (pos.IsEmpty())
		{
			//m_mtx.Unlock();
			return FALSE;
		}
		pMapItem tmp;
		if (m_list[idx].GetAt(pos,tmp,FALSE))
		{
			//m_mtx.Unlock();
			val=tmp->val;
			return TRUE;
		}
		//m_mtx.Unlock();
		return FALSE;
	}
	TVALUE GetAt(TKEY key) const
	{
		TVALUE val=NULL;
		GetAt(key,val);
		return val;
	}
	TVALUE operator[](TKEY key)
	{
		return GetAt(key);
	}
protected:
	UINT HashKey(TKEY key) const{return ((DWORD)(((DWORD_PTR)key)>>4))%m_nHash;}
	//��ȡ��ӦList��ֵ��pos
	THPosition FindKeyPos(UINT idx,TKEY key) const
	{
		THPosition pos;
		THPosition lastpos;
		if (m_list)
		{
			pos=m_list[idx].GetStartPosition();
			pMapItem tmp;
			while(!pos.IsEmpty())
			{
				lastpos=pos;
				if (m_list[idx].GetNextPosition(pos,tmp))
					if (tmp->key==key)
						return lastpos;
			}
			lastpos.SetPositionData(NULL);
		}
		return lastpos;
	}
	UINT m_nHash;
	THList<pMapItem> *m_list;
	//THMemPool<MapItem> m_pool;		///<�ڴ�����
	volatile UINT m_nBufUpdate;					///<���ڼ���
	THMutex m_mtx;
};

/**
* @brief ˫��ֵMapģ����
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-06-02 �½���
*/
/**<pre>
ʹ��Sample��
	FreeCallBack ��valueΪNULLʱ��keyΪTKEY������keyΪTKEY2��valueΪTVALUE
	void FreeDMapCallBack(void *key,void *value,void *adddata)
	{
		if (value==NULL)
		{
			THDebug(_T("Free DMap Key1:%d"),key);
		}
		else
		{
			THDebug(_T("Free DMap Key2:%d and Value:%d"),key,value);
			delete [] value;
		}
	}
	THDoubleKeyMap<int,short,char *> m_dmap;
	m_dmap.SetFreeProc(FreeDMapCallBack);
	m_dmap.SetAt(2,4,new char[124]);
	m_dmap.SetAt(3,4,new char[124]);
	m_dmap.SetAt(2,4,new char[124]);
	THDebug(_T("double map member 2,4 is:%d"),m_dmap[2][4]);
	m_dmap.RemoveAt(2,4);
	THDebug(_T("double map member 2,4 is now:%d"),m_dmap.GetAt(2,4));
	pos=m_dmap.GetStartPosition();
	short key2;
	while(!pos.IsEmpty())
	{
		if (m_dmap.GetNextPosition(pos,key,key2,val))
		{
			THDebug(_T("double map member key:%d,%d val:%d"),key,key2,val);
		}
	}
	m_dmap.RemoveAll();
</pre>*/
template <typename TKEY,typename TKEY2,typename TVALUE>
class THDoubleKeyMap : public THMemFreeProc<TKEY,TVALUE>
{
private:
	static void MapSubItemFreeCallBack(void *key,void *value,void *adddata)
	{
		THDoubleKeyMap *p=(THDoubleKeyMap *)adddata;
		if (p)
		{
			p->FreeProc((TKEY)(INT_PTR)key,(TVALUE)(INT_PTR)value);//Free TKEY2 & TVALUE
		}
	}
	static void MapItemFreeCallBack(void *key,void *value,void *adddata)
	{
		THDoubleKey *tmp=(THDoubleKey *)value;
		THDoubleKeyMap *p=(THDoubleKeyMap *)adddata;
		if (tmp && p)
		{
			tmp->RemoveAll();
			p->FreeProc((TKEY)(INT_PTR)key,(TVALUE)(INT_PTR)NULL);//Free TKEY
			delete tmp;//p->m_pool.Delete(tmp);
		}
	}
	typedef THMap<TKEY2,TVALUE> THDoubleKey;
public:
	THDoubleKeyMap()
	{
		m_map.SetFreeProc(MapItemFreeCallBack,this);
	}
	virtual ~THDoubleKeyMap()
	{
		RemoveAll();
	}

	BOOL SetAt(TKEY key,TKEY2 key2,TVALUE value)
	{
		THDoubleKey *subkey=m_map[key];
		if (!subkey)
		{
			subkey=new THDoubleKey;//m_pool.New();
			if (!subkey) return FALSE;
			subkey->SetFreeProc(MapSubItemFreeCallBack,this);
			if (!m_map.SetAt(key,subkey)) return FALSE;
		}
		return subkey->SetAt(key2,value);
	}
	BOOL RemoveAt(TKEY key,TKEY2 key2)
	{
		THDoubleKey *subkey=m_map[key];
		if (!subkey) return FALSE;
		return subkey->RemoveAt(key2);
	}
	void RemoveAll()
	{
		m_map.RemoveAll();
	}
	THPosition GetStartPosition() const
	{
		//pos pointer ��ǰTHDoublaKeyָ��,TKEY,TKEY2
		THPosition pos;
		THPosition mp=m_map.GetStartPosition();
		while(!mp.IsEmpty())
		{
			TKEY key;
			THDoubleKey *tmp;
			if (m_map.GetNextPosition(mp,key,tmp))
			{
				THPosition mp2=tmp->GetStartPosition();
				TKEY2 key2;
				TVALUE val;
				while(!mp2.IsEmpty())
				{
					if (tmp->GetNextPosition(mp2,key2,val))
					{
						pos.SetPositionData(tmp,(void *)(INT_PTR)key,(void *)(INT_PTR)key2);
						//escape 2 while loop
						return pos;
					}
				}
			}
		}
		return pos;
	}
	BOOL GetNextPosition(THPosition &pos,TKEY &key,TKEY2 &key2,TVALUE &val) const
	{
		if (pos.IsEmpty()) return FALSE;
		THDoubleKey *tmp=(THDoubleKey *)pos.GetCurrentItem();
		key=(TKEY)(INT_PTR)pos.GetPreItem();
		key2=(TKEY2)(INT_PTR)pos.GetNextItem();
		val=tmp->GetAt(key2);
		TKEY tkey;
		TKEY2 tkey2;
		TVALUE tval;
		THPosition mp=m_map.GetKeyPos(key);
		BOOL bFristTime=TRUE;
		while(!mp.IsEmpty())
		{
			if (m_map.GetNextPosition(mp,tkey,tmp))
			{
				THPosition mp2;
				if (bFristTime)
				{
					bFristTime=FALSE;
					mp2=tmp->GetKeyPos(key2);
					tmp->GetNextPosition(mp2,tkey2,tval);//next item
				}
				else
				{
					mp2=tmp->GetStartPosition();
				}
				while(!mp2.IsEmpty())
				{
					if (tmp->GetNextPosition(mp2,tkey2,tval))
					{
						pos.SetPositionData(tmp,(void *)(INT_PTR)tkey,(void *)(INT_PTR)tkey2);
						//escape 2 while loop
						return TRUE;
					}
				}
			}
		}
		pos.SetPositionData(NULL);
		return TRUE;
	}
	UINT GetCount()
	{
		UINT cnt=0;
		THPosition mp=m_map.GetStartPosition();
		while(!mp.IsEmpty())
		{
			TKEY key;
			THDoubleKey *tmp;
			if (m_map.GetNextPosition(mp,key,tmp))
			{
				cnt+=tmp->GetCount();
			}
		}
		return cnt;
	}
	BOOL GetAt(TKEY key,TKEY2 key2,TVALUE &val)
	{
		THDoubleKey *subkey=m_map[key];
		if (!subkey) return FALSE;
		return subkey->GetAt(key2,val);
	}
	TVALUE GetAt(TKEY key,TKEY2 key2)
	{
		THDoubleKey *subkey=m_map[key];
		if (!subkey) return FALSE;
		return subkey->GetAt(key2);
	}
	THDoubleKey &operator[](TKEY key)
	{
		return *m_map[key];
	}
private:
	THMap<TKEY,THDoubleKey *>m_map;
	//THMemPool<THDoubleKey> m_pool;			///<�ڴ�����
};

/**
* @brief �ַ�����ֵMapģ����
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-09-28 �½���
*/
/**<pre>
ʹ��Sample��
</pre>*/
template <typename TVALUE>
class THStringMap
{
public:
	THStringMap()
	{
	}
	virtual ~THStringMap()
	{
		RemoveAll();
	}

	//ֻ��value����
	void SetFreeProc(StructCallBack cb,void *pAddData=NULL)
	{
		m_map.SetFreeProc(cb,pAddData);
	}

	BOOL SetAt(THString key,TVALUE val)
	{
		int idx=FindKeyPos(key);
		if (idx==-1)
			idx=(int)m_ar.Add(key);
		return m_map.SetAt(idx,val);
	}

	BOOL RemoveAt(THString key)
	{
		int idx=FindKeyPos(key);
		if (idx==-1) return FALSE;
		return m_map.RemoveAt(idx);
	}
	void RemoveAll()
	{
		m_ar.RemoveAll();
		return m_map.RemoveAll();
	}
	THPosition GetStartPosition() const
	{
		return m_map.GetStartPosition();
	}
	BOOL GetNextPosition(THPosition &pos,THString &key,TVALUE &val) const
	{
		int idx;//=FindKeyPos(key);
		if (!m_map.GetNextPosition(pos,idx,val)) return FALSE;
		if (m_ar.GetSize()<=idx || idx<0) return FALSE;
		key=m_ar[idx];
		return TRUE;
	}
	THPosition GetKeyPos(THString key) const
	{
		THPosition pos;
		int idx=FindKeyPos(key);
		if (idx==-1) return pos;
		return m_map.GetKeyPos(idx);
	}
	BOOL GetAt(THString key,TVALUE &val)
	{
		int idx=FindKeyPos(key);
		if (idx==-1) return FALSE;
		return m_map.GetAt(idx,val);
	}
	TVALUE GetAt(THString key)
	{
		TVALUE val=NULL;
		GetAt(key,val);
		return val;
	}
	UINT GetCount()
	{
		return m_map.GetCount();
		//if (m_nBufUpdate!=0) return m_nBufUpdate;
		/*UINT cnt=0;
		CMyPosition pos=GetStartPosition();
		TKEY key;
		TVALUE val;
		while(!pos.IsEmpty())
		{
			GetNextPosition(pos,key,val);
			cnt++;
		}
		m_nBufUpdate=cnt;
		return cnt;*/
	}
private:
	int FindKeyPos(THString key) const
	{
		for(int i=0;i<m_ar.GetSize();i++)
			if (key==m_ar[i]) return i;
		return -1;
	}
	THStringArray m_ar;
	THMap<int,TVALUE> m_map;
};

/**
* @brief ˫�ַ�����ֵMapģ����
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-09-28 �½���
*/
/**<pre>
ʹ��Sample��
	ͨ��SetFreeProc����CallBack��CallBackû���ر��÷�,ֻ�ṩvalue��
</pre>*/
template <typename TVALUE>
class THDoubleStringMap
{
private:
	static void NormalCallBack(void *key,void *value,void *adddata)
	{
		THDoubleStringMap<TVALUE> *p=(THDoubleStringMap<TVALUE> *)adddata;
		if (adddata)
		{
			//if value is empty,is just key1 is available,skip it
			if (value && p->m_cb)
			{
				p->m_cb(NULL,value,p->m_adddata);
			}
		}
	}
public:
	THDoubleStringMap()
	{
		RemoveAll();
		m_cb=NULL;
		m_adddata=NULL;
	}
	virtual ~THDoubleStringMap()
	{
		RemoveAll();
	}

	void SetFreeProc(StructCallBack cb,void *pAddData=NULL)
	{
		m_cb=cb;
		m_adddata=pAddData;
		//ͨ��NormalCallBack����key1�Ļص���������key2�ص�ʱ��value
		m_map.SetFreeProc(NormalCallBack,this);
	}

	BOOL SetAt(THString key,THString key2,TVALUE val)
	{
		int idx=FindKeyPos(key);
		if (idx==-1)
			idx=(int)m_ar.Add(key);
		int idx2=FindKeyPos(key2);
		if (idx2==-1)
			idx2=(int)m_ar.Add(key2);
		return m_map.SetAt(idx,idx2,val);
	}

	BOOL RemoveAt(THString key,THString key2)
	{
		int idx=FindKeyPos(key);
		if (idx==-1) return FALSE;
		int idx2=FindKeyPos(key2);
		if (idx2==-1) return FALSE;
		return m_map.RemoveAt(idx,idx2);
	}
	void RemoveAll()
	{
		m_ar.RemoveAll();
		m_ar.Add(_T("Index0ForDoubleStringMapIsNotAvail,thisisjustavoid0"));
		return m_map.RemoveAll();
	}
	THPosition GetStartPosition() const
	{
		return m_map.GetStartPosition();
	}
	BOOL GetNextPosition(THPosition &pos,THString &key,THString &key2,TVALUE &val) const
	{
		int idx,idx2;//=FindKeyPos(key);
		if (!m_map.GetNextPosition(pos,idx,idx2,val)) return FALSE;
		if (m_ar.GetSize()<=idx || idx<0 ||
			m_ar.GetSize()<=idx2 || idx2<0) return FALSE;
		key=m_ar[idx];
		key2=m_ar[idx2];
		return TRUE;
	}
	THPosition GetKeyPos(THString key,THString key2) const
	{
		THPosition pos;
		int idx=FindKeyPos(key);
		if (idx==-1) return pos;
		int idx2=FindKeyPos(key2);
		if (idx2==-1) return pos;
		return m_map.GetKeyPos(idx,idx2);
	}
	BOOL GetAt(THString key,THString key2,TVALUE &val)
	{
		int idx=FindKeyPos(key);
		if (idx==-1) return FALSE;
		int idx2=FindKeyPos(key2);
		if (idx2==-1) return FALSE;
		return m_map.GetAt(idx,idx2,val);
	}
	TVALUE GetAt(THString key,THString key2)
	{
		TVALUE val=NULL;
		GetAt(key,key2,val);
		return val;
	}
	UINT GetCount()
	{
		return m_map.GetCount();
		//if (m_nBufUpdate!=0) return m_nBufUpdate;
		/*UINT cnt=0;
		CMyPosition pos=GetStartPosition();
		TKEY key;
		TVALUE val;
		while(!pos.IsEmpty())
		{
			GetNextPosition(pos,key,val);
			cnt++;
		}
		m_nBufUpdate=cnt;
		return cnt;*/
	}
private:
	int FindKeyPos(THString key) const
	{
		for(int i=0;i<m_ar.GetSize();i++)
			if (key==m_ar[i]) return i;
		return -1;
	}
	THStringArray m_ar;
	THDoubleKeyMap<int,int,TVALUE> m_map;
	StructCallBack m_cb;
	void *m_adddata;
};

/**
* @brief ��̬ģ��Map�࣬ά�ּ�ֵ������TMAXPOOL��
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-06-02 �½���
*/
/**<pre>
ʹ��Sample��
	void FreeDynMapCallBack(void *key,void *value,void *adddata)
	{
		THDebug(_T("Free dyn map key:%d 's value %d"),(int)(INT_PTR)key,value);
		delete [] value;
	}
	THDynMapPool<int,char *,3> m_dynmap;
	m_dynmap.SetFreeProc(FreeDynMapCallBack);
	m_dynmap.SetAt(1,new char[124]);
	m_dynmap.SetAt(3,new char[124]);
	m_dynmap.SetAt(4,new char[124]);
	m_dynmap.SetAt(5,new char[124]);
	m_dynmap.SetAt(7,new char[124]);
	THDebug(_T("dyn map member key:%d val:%d"),7,m_dynmap[7]);
	m_dynmap.RemoveAt(5);
	pos=m_dynmap.GetStartPosition();
	while(!pos.IsEmpty())
	{
		if (m_dynmap.GetNextPosition(pos,key,val))
		{
			THDebug(_T("dyn map member key:%d,val:%d"),key,val);
		}
	}
	m_dynmap.RemoveAll();
</pre>*/
template <typename TKEY,typename TVALUE,unsigned int TMAXPOOL>
class THDynMapPool : public THMemFreeProc<TKEY,TVALUE>
{
public:
	THDynMapPool()
	{
		for(int i=0;i<TMAXPOOL;i++)
			m_Content[i].bUse=FALSE;
	}

	virtual ~THDynMapPool()
	{
		RemoveAll();
	}

	void RemoveAll()
	{
		for(int i=0;i<TMAXPOOL;i++)
			if (m_Content[i].bUse)
			{
				FreeProc(m_Content[i].key,m_Content[i].value);
				m_Content[i].bUse=FALSE;
			}
	}

	void SetAt(TKEY key,TVALUE value)
	{
		int nLast=-1;
		for(int i=0;i<TMAXPOOL;i++)
		{
			if (m_Content[i].bUse && m_Content[i].key==key)
			{
				nLast=i;
				break;
			}
		}
		if (nLast==-1)
		{
			DWORD nLastti=GetTickCount();
			for(int i=0;i<TMAXPOOL;i++)
			{
				if (!m_Content[i].bUse)
				{
					nLast=i;
					break;
				}
				else if (m_Content[i].dwLast<=nLastti)
				{
					nLast=i;
					nLastti=m_Content[i].dwLast;
				}
			}
		}
		ASSERT(nLast!=-1);
		if (nLast!=-1)
		{
			if (m_Content[nLast].bUse)
			{
				if (value!=m_Content[nLast].value)
					FreeProc(m_Content[nLast].key,m_Content[nLast].value);
			}
			m_Content[nLast].bUse=TRUE;
			m_Content[nLast].dwLast=GetTickCount();
			m_Content[nLast].key=key;
			m_Content[nLast].value=value;
		}
	}

	TVALUE GetAt(TKEY key)
	{
		TVALUE ret=NULL;
		GetAt(key,ret);
		return ret;
	}

	BOOL GetAt(TKEY key,TVALUE &val)
	{
		for(int i=0;i<TMAXPOOL;i++)
		{
			if (m_Content[i].bUse && m_Content[i].key==key)
			{
				m_Content[i].dwLast=GetTickCount();
                val=m_Content[i].value;
				return TRUE;
			}
		}
		return FALSE;
	}

	BOOL RemoveAt(TKEY key)
	{
		for(int i=0;i<TMAXPOOL;i++)
		{
			if (m_Content[i].bUse && m_Content[i].key==key)
			{
				m_Content[i].bUse=FALSE;
				FreeProc(m_Content[i].key,m_Content[i].value);
				return TRUE;
			}
		}
		return FALSE;
	}

	THPosition GetStartPosition() const
	{
		//pos pointer 1,��ǰ��ֵ����,��
		THPosition pos;
		for(int i=0;i<TMAXPOOL;i++)
		{
			if (m_Content[i].bUse)
			{
				pos.SetPositionData((void *)(INT_PTR)1,(void *)(INT_PTR)i,NULL);
				break;
			}
		}
		return pos;
	}

	BOOL GetNextPosition(THPosition &pos,TKEY &key,TVALUE &val) const
	{
		if (pos.IsEmpty()) return FALSE;
		int idx=(int)(INT_PTR)pos.GetPreItem();
		if (!m_Content[idx].bUse) return FALSE;
		key=m_Content[idx].key;
		val=m_Content[idx].value;
		pos.SetPositionData(NULL);
		for(int i=idx+1;i<TMAXPOOL;i++)
		{
			if (m_Content[i].bUse)
			{
				pos.SetPositionData((void *)(INT_PTR)1,(void *)(INT_PTR)i,NULL);
				break;
			}
		}
		return TRUE;
	}

	TVALUE operator[](TKEY key)
	{
		return GetAt(key);
	}
private:
	struct _MapContent{
		TKEY key;
		TVALUE value;
		BOOL bUse;
		DWORD dwLast;
	}m_Content[TMAXPOOL];
};

/**
* @brief ջ��
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-06-07 �½���
*/
/**<pre>
�÷���
</pre>*/
template <typename TVALUE>
class THStack :public THList<TVALUE>
{
public:
	THStack(){}
	virtual ~THStack(){}

	//ѹջ
	inline BOOL Push(TVALUE val){return m_list.AddHead(val);}

	//��ջ
	inline BOOL Pop(TVALUE &val){return m_list.GetHead(val,TRUE);}

	inline BOOL GetTop(TVALUE &val){return m_list.GetHead(val,FALSE);}

	//ջ�Ƿ�Ϊ��
	inline BOOL IsEmpty() const{return (m_list.GetSize()==0);}

	//���ջ
	inline void Empty(){m_list.RemoveAll();}
private:
	THList<TVALUE> m_list;
};

/**
* @brief ������
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-11-04 �½���
*/
/**<pre>
	DWORD data[10];
	int datapos[10];
	THSort<DWORD,int>::SortPos(data,datapos,10,TRUE);
</pre>*/
template <typename TDATATYPE,typename TPOSTYPE>
class THSort
{
public:
	/**
	* @brief ��������
	* @param pData		��������������
	* @param nCnt		���ݳ���
	* @param bAscending	�Ƿ���������
	* @return �Ƿ�ִ�����
	*/
	static BOOL Sort(TDATATYPE *pData,int nCnt,BOOL bAscending=TRUE)
	{
		if (!pData) return FALSE;
		int tmp;
		TDATATYPE min;
		for(int i=0;i<nCnt;i++)
		{
			tmp=i;
			min=*(pData+i);
			for(int j=i+1;j<nCnt;j++)
			{
				if ((min>(*(pData+j)) && bAscending)||
					(min<(*(pData+j)) && !bAscending))
				{
					tmp=j;
					min=*(pData+j);
				}
			}
			if (tmp!=i)
			{
				*(pData+tmp)=*(pData+i);
				*(pData+i)=min;
			}
		}
		return TRUE;
	}

	/**
	* @brief ����λ�����򣬷���λ������
	* @param pData		��������
	* @param pPos		��������������
	* @param nCnt		���ݳ���
	* @param bAscending	�Ƿ���������
	* @return �Ƿ�ִ�����
	*/
	static BOOL SortPos(TDATATYPE *pData,TPOSTYPE *pPos,int nCnt,BOOL bAscending=TRUE)
	{
		if (!pData || !pPos) return FALSE;
		if (nCnt<=0) return FALSE;
		TDATATYPE *tmp=new TDATATYPE[nCnt];
		if (!tmp) return FALSE;
		memcpy(tmp,pData,sizeof(TDATATYPE)*nCnt);
		if (!Sort(tmp,nCnt,bAscending))
		{
			delete [] tmp;
			return FALSE;
		}
		int *tmp2=new int[nCnt];
		if (!tmp2)
		{
			delete [] tmp;
			return FALSE;
		}
		memset(tmp2,0,sizeof(int)*nCnt);
		int k=0;
		for(int i=0;i<nCnt;i++)
		{
			for(int j=0;j<nCnt;j++)
			{
				if (tmp[i]==*(pData+j) && *(tmp2+j)==0)
				{
					*(pPos+k)=j;
					*(tmp2+j)=1;
					k++;
					break;
				}
			}
		}
		delete [] tmp;
		delete [] tmp2;
		return TRUE;
	}
};

/**
* @brief MRU LRU���ͷ�֪ͨ
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2009-02-16 �½���
*/
template<typename TKEY>
class ITHDeleteCb
{
public:
	virtual void DeleteNotify(TKEY key)=0;
};

/**
* @brief ����ʹ���� Most recently used
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2009-02-13 �½���
*/
/**<pre>
</pre>*/
template<typename TKEY>
class THMRU
{
public:
	typedef list<TKEY> MRUL;
	typedef typename MRUL::iterator mruit,*pmruit;
	typedef map<TKEY,mruit> MRUM;

	THMRU()
	{
		m_cb=NULL;
		m_limitsize=0;
	}

	virtual ~THMRU()
	{
		Empty();
	}

	/**
	* @brief ����key���б��е�λ��,���limitsizeΪ0���򲻽�����ӣ���Ϊ����Ҫ������
	* @return �����Ƿ�
	*/
	void Update(TKEY key)
	{
		m_mrumtx.Lock();
		if (m_limitsize!=0)
		{
			typename MRUM::iterator it;
			if ((it=m_mrumap.find(key))!=m_mrumap.end())
			{
				m_mrulist.erase(it->second);
				it->second=m_mrulist.insert(m_mrulist.end(),key);
			}
			else
			{
				m_mrumap.insert(typename MRUM::value_type(key,m_mrulist.insert(m_mrulist.end(),key)));
			}
		}
		m_mrumtx.Unlock();
		DoLimit();
	}

	void Erase(TKEY key)
	{
		m_mrumtx.Lock();
		typename MRUM::iterator it;
		if ((it=m_mrumap.find(key))!=m_mrumap.end())
		{
			//����cb�кܿ����ֻ����Erase����Ԫ��ɾ�����������ͷŵ�Ԫ�أ�ȷ�����ᱻ�ظ�ɾ��
			TKEY key=*(it->second);
			m_mrulist.erase(it->second);
			m_mrumap.erase(it);
			//list�е�it���ɿ�һЩ����Ϊ�п��ܶ�����һЩ�ṹ���ȽϺ���ֻ�ǱȽ�����һ��ֵ��������ֵû�иı䣬���ֽṹ�µ�Updateֻ����list�е�ָ�룬û�и���map�е�ָ��
			if (m_cb) m_cb->DeleteNotify(key);
		}
		m_mrumtx.Unlock();
	}

	unsigned int GetCount()
	{
		m_mrumtx.Lock();
		unsigned int size=(unsigned int)m_mrulist.size();
		m_mrumtx.Unlock();
		return size;
	}

	void Empty()
	{
		m_mrumtx.Lock();
		//�Ȱ�map�����ȷ���ص����������Erase���ᷢ������ɾ�������
		m_mrumap.clear();
		if (m_cb)
		{
			typename MRUL::iterator it=m_mrulist.begin();
			for(;it!=m_mrulist.end();it++) m_cb->DeleteNotify(*it);
		}
		m_mrulist.clear();
		m_mrumtx.Unlock();
	}

	BOOL IsHave(TKEY key)
	{
		return (m_mrumap.find(key)!=m_mrumap.end());
	}

	inline BOOL IsIteratorEnd(typename MRUM::iterator it)
	{
		return (it==m_mrumap.end());
	}

	typename MRUM::iterator GetIterator(TKEY key)
	{
		return m_mrumap.find(key);
	}
	inline BOOL IsEmpty() {return GetCount()==0;}

	/**
	* @brief ����size���Ѷ������������,���limitsizeΪ0���򲻽�������
	* @return �����Ƿ�
	*/
	void SetLimitSize(unsigned int limitsize)
	{
		m_limitsize=limitsize;
		DoLimit();
	}

	/**
	* @brief ���б��в���������������
	* @return ��������
	*/
	TKEY RandomAccessFrist(unsigned int item)
	{
		TKEY key=(TKEY)0;
		m_mrumtx.Lock();
		typename MRUL::reverse_iterator it=m_mrulist.rbegin();
		while(item>0 && it!=m_mrulist.rend())
		{
			it++;
			item--;
		}
		if (it!=m_mrulist.rend())
			key=*it;
		m_mrumtx.Unlock();
		return key;
	}

	/**
	* @brief ���б��в���������������
	* @param items ���������б�
	* @param size ��Ҫ���ظ���
	* @return ����ʵ�ʸ���
	*/
	unsigned int RandomAccessFrists(TKEY *items,unsigned int size)
	{
		if (!items || size==0) return 0;
		unsigned int i=0;
		TKEY key=(TKEY)0;
		m_mrumtx.Lock();
		typename MRUL::reverse_iterator it;
		it=m_mrulist.rbegin();
		while(size>0 && it!=m_mrulist.rend())
		{
			*items=*it;
			items++;
			it++;
			size--;
			i++;
		}
		if (size!=0)
			memset(items,0,sizeof(TKEY)*size);
		m_mrumtx.Unlock();
		return i;
	}

	/**
	* @brief �������ͷ�ʱ����
	* @return �����Ƿ�
	*/
	void SetDeleteCb(class ITHDeleteCb<TKEY> *cb)
	{
		m_mrumtx.Lock();
		m_cb=cb;
		m_mrumtx.Unlock();
	}
protected:
	void DoLimit()
	{
		if (m_limitsize!=0)
		{
			m_mrumtx.Lock();
			unsigned int size=(unsigned int)m_mrulist.size();
			if (m_limitsize!=0 && size>m_limitsize)
			{
				//do limit
				typename MRUL::iterator it=m_mrulist.begin();
				for(unsigned int i=0;i<size-m_limitsize && it!=m_mrulist.end();i++)
				{
					//����cb�кܿ����ֻ����Erase����Ԫ��ɾ�����������ͷŵ�Ԫ�أ�ȷ�����ᱻ�ظ�ɾ��
					TKEY key=*it;
					m_mrumap.erase(key);
					m_mrulist.erase(it++);
					//list�е�it���ɿ�һЩ����Ϊ�п��ܶ�����һЩ�ṹ���ȽϺ���ֻ�ǱȽ�����һ��ֵ��������ֵû�иı䣬���ֽṹ�µ�Updateֻ����list�е�ָ�룬û�и���map�е�ָ��
					if (m_cb) m_cb->DeleteNotify(key);
				}
				//m_mrulist.erase(m_mrulist.begin(),m_mrulist.begin()+size-m_limitsize);
			}
			m_mrumtx.Unlock();
		}
	}
	ITHDeleteCb<TKEY> *m_cb;
	MRUL m_mrulist;					//��������
	MRUM m_mrumap;					//��������list�е��Ѵ��ڵ���Ŀ
	THMutex m_mrumtx;
	unsigned int m_limitsize;		//���ƴ�С��0Ϊ������
};

/**
* @brief ����˳������״̬����������ϲ��ռ�
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2009-10-23 �½���
*/
/**<pre>
	�����ʾ����Ϊ-5~-10,-2,-8~-15,1,2,3,5,8,9,11,13,14,15,16��״̬��ͨ�������鴦��Ϊ
	-15~-5,-2,1~3,5,8,9,11,13~16
</pre>*/
class THIndexArray
{
public:
	THIndexArray()
	{
	}
	virtual ~THIndexArray()
	{
	}
	/**
	* @brief ���������ַ��������ʽ-1,3~10,14,16
	* @return �����ַ���
	*/
	THString toString(THString formatJoin=_T("~"),THString formatSpilt=_T(",")) const
	{
		THString str;
		m_mtx.Lock();
		map<int,int>::const_iterator it=m_havemap.begin();
		for(;it!=m_havemap.end();it++)
		{
			if (!str.IsEmpty()) str+=formatSpilt;
			if (it->first==it->second)
				str.AppendFormat(_T("%d"),it->first);
			else
				str.AppendFormat(_T("%d%s%d"),it->first,formatJoin,it->second);
		}
		m_mtx.Unlock();
		return str;
	}
	/**
	* @brief ͨ���ַ����������ݣ�ԭ������գ����ʽ1,3~10,14,16
	* @param str ���õ��ַ���
	*/
	inline void fromString(THString str,THString formatJoin=_T("~"),THString formatSpilt=_T(","))
	{
		m_mtx.Lock();
		m_havemap.clear();
		m_mtx.Unlock();
		IndexSetsByString(str,TRUE,formatJoin,formatSpilt);
	}
	BOOL IsIndexSeted(int idx) const
	{
		BOOL bSeted=FALSE;
		m_mtx.Lock();
		map<int,int>::const_iterator it=map_lower_key_c< map<int,int> >(m_havemap,idx);
		if (it!=m_havemap.end())
			if (it->first<=idx && it->second>=idx) bSeted=TRUE;
		m_mtx.Unlock();
		return bSeted;
	}
	inline void IndexSet(int idx,BOOL bSet=TRUE){IndexSets(idx,idx,bSet);}
	/**
	* @brief ͨ���ַ����������ݣ����ʽ1,3~10,14,16
	* @param str ���õ��ַ���
	* @param bSet ָ�������û�������
	*/
	void IndexSetsByString(THString str,BOOL bSet=TRUE,THString formatJoin=_T("~"),THString formatSpilt=_T(","))
	{
		THStringToken t(str,formatSpilt),t1;
		while(t.IsMoreTokens())
		{
			str=t.GetNextToken();
			if (!str.IsEmpty())
			{
				t1.Init(str,formatJoin);
				int idx=THs2i(t1.GetNextToken());
				str=t1.GetNextToken();
				if (str.IsEmpty())
					IndexSet(idx,bSet);
				else
					IndexSets(idx,THs2i(str),bSet);
			}
		}
	}
	void IndexSets(int idx,int endidx,BOOL bSet=TRUE)
	{
		if (endidx<idx) {int tmp=endidx;endidx=idx;idx=tmp;}
		m_mtx.Lock();
		if (bSet)
		{
			//����
			//         C1-----C2 --- ---- --- D1-----D2
			//            A---------------------B
			/*�ҺͿ�ʼλ��A�ص���C��
				����У����C2�Ƿ���ڻ����A��
					���������C2�Ƿ���ڻ����B��
						�������˳���
						������������������C2-B֮�����ϣ���ɾ������������ֽ���λ�ñ�B��ģ�����B��ֵ�����C2=B���˳�
					��������������C��
				���û�У�����A������������A-B֮�����ϣ���ɾ������������ֽ���λ�ñ�B��ģ�����B��ֵ���˳�*/
			map<int,int>::iterator oit,it;
			oit=map_lower_key< map<int,int> >(m_havemap,idx);
			BOOL bNeedCheck;
			if (oit==m_havemap.end() || oit->second+1<idx)
			{
				pair< map<int,int>::iterator, bool> pr=m_havemap.insert(map<int,int>::value_type(idx,idx));
				oit=pr.first;
				bNeedCheck=TRUE;
			}
			else
				bNeedCheck=(oit->second<endidx);
			if (bNeedCheck)
			{
				it=oit;it++;
				while(it!=m_havemap.end())
				{
					if (it->first>endidx+1) break;
					if (it->second>endidx) endidx=it->second;
					m_havemap.erase(it++);
				}
				oit->second=endidx;
			}
		}
		else
		{
			//����
			//         C1-----C2 --- ---- --- D1-----D2
			//            A---------------------B
			/*�ҺͿ�ʼλ��A�ص���C��
				����У����C2�Ƿ���ڻ����A��
					���������C2�Ƿ���ڻ����B��
						������ֱ���޸�C���ɣ�
						�����������޸�C���֡�
					��������������C��
				���û�У�
				����A������������A-B֮�����ϣ���ɾ������������ֽ���λ�ñ�B��ģ�����B��ֵ���˳�*/
			map<int,int>::iterator it;
			it=map_lower_key< map<int,int> >(m_havemap,idx);
			if (it==m_havemap.end())
				it=m_havemap.upper_bound(idx);
			while(it!=m_havemap.end())
			{
				if (it->first>endidx) break;
				if (it->second<idx)
					it++;
				else
				{
					if (it->second>endidx)
						m_havemap.insert(map<int,int>::value_type(endidx+1,it->second));
					if (it->first>=idx)
						m_havemap.erase(it++);
					else
					{
						it->second=idx-1;
						it++;
					}
				}
			}
		}
		m_mtx.Unlock();
	}
	inline void Copy(const THIndexArray &ar) {fromString(ar.toString());}
	inline void Join(const THIndexArray &ar) {IndexSetsByString(ar.toString(),TRUE);}
private:
	map<int,int> m_havemap;
	THMutex m_mtx;
};
