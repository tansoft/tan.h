#pragma once

#include <THString.h>
#include <THStruct.h>

class ITHDataBase;

/**
* @brief 数据集基础封装类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-10-29 新建类
*/
class ITHRecordSet
{
public:
	ITHRecordSet(){m_pDb=NULL;m_bReadOnly=FALSE;}
	ITHRecordSet(ITHDataBase *pDb,BOOL bReadOnly=FALSE){SetDB(pDb,bReadOnly);}
	virtual ~ITHRecordSet(){}

	virtual ITHRecordSet* NewObject()=0;
	virtual void ReleaseObject(ITHRecordSet*obj){delete obj;}

	virtual void SetDB(ITHDataBase *pDb,BOOL bReadOnly=FALSE){m_pDb=pDb;m_bReadOnly=bReadOnly;}
	virtual BOOL StartQuery(THString sql)=0;
	virtual BOOL ReQuery()=0;
	virtual BOOL EndQuery()=0;

	virtual void *GetIntObj()=0;

	virtual BOOL IsBOF()=0;
	virtual BOOL IsEOF()=0;
	virtual BOOL MoveNext()=0;
	virtual BOOL MovePrev()=0;
	virtual BOOL MoveFirst()=0;
	virtual BOOL MoveLast()=0;
	virtual int GetRecordCount()=0;
	virtual int GetColCount()=0;
	virtual THString GetColName(int idx)=0;
	virtual THString GetColValue(int idx)=0;
	virtual THString GetColValue(THString sKey)=0;
protected:
	ITHDataBase *m_pDb;
	BOOL m_bReadOnly;
};

/**
* @brief 数据库基础封装类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-10-28 新建类
*/
class ITHDataBase
{
public:
	ITHDataBase(){}
	virtual ~ITHDataBase(){}

	virtual ITHDataBase* NewObject()=0;
	virtual void ReleaseObject(ITHDataBase*obj){delete obj;}
	virtual ITHRecordSet* NewRecordSet(BOOL bReadOnly=FALSE)=0;
	virtual void ReleaseRecordSet(ITHRecordSet*obj){obj->ReleaseObject(obj);}

	virtual BOOL OpenDataBase(THString sOpenStr,BOOL bReadOnly=FALSE)=0;
	virtual BOOL OpenDataBase(THString sDriver,THString sHost,int nPort,THString sDataBase,THString sUserName,THString sPassWord,BOOL bReadOnly=FALSE)=0;
	virtual BOOL CloseDataBase()=0;
	virtual BOOL IsOpened()=0;

	virtual void *GetIntObj()=0;

	virtual BOOL ExecuteSql(const TCHAR *lpszSql)=0;
	virtual BOOL ExecuteSqlFormat(const TCHAR *lpszSql,...)
	{
		THString str;
		va_list pArg;
		va_start(pArg,lpszSql);
		str.FormatV(lpszSql,pArg);
		va_end(pArg);
		return ExecuteSql(str);
	}

	virtual ITHRecordSet* StartQuery(BOOL bReadOnly,const TCHAR *lpszSql)
	{
		ITHRecordSet *set=NewRecordSet(bReadOnly);
		if (set)
		{
			if (!set->StartQuery(lpszSql))
			{
				ReleaseRecordSet(set);
				set=NULL;
			}
		}
		return set;
	}

	virtual ITHRecordSet* StartQueryFormat(BOOL bReadOnly,const TCHAR *lpszSql,...)
	{
		THString str;
		va_list pArg;
		va_start(pArg,lpszSql);
		str.FormatV(lpszSql,pArg);
		va_end(pArg);
		return StartQuery(bReadOnly,str);
	}

	virtual BOOL EndQuery(ITHRecordSet *set)
	{
		BOOL bret=FALSE;
		if (set)
		{
			bret=set->EndQuery();
			ReleaseRecordSet(set);
		}
		return bret;
	}

	virtual THString GetProviderName()=0;
};

#define MULDB_PROVIDER		_T("MultiDB Provider")

/**
* @brief 记录集多数据库联合方式封装类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-12-10 新建类
*/
class THRSMulDB : public ITHRecordSet
{
private:
	static void FreeRs(void *key,void *value,void *adddata)
	{
		ITHRecordSet *tmp=(ITHRecordSet *)key;
		if (tmp) delete tmp;
	}
public:
	THRSMulDB():ITHRecordSet()
	{
		m_rslist.SetFreeProc(FreeRs);
	}
	THRSMulDB(ITHDataBase *pDb,BOOL bReadOnly=FALSE):ITHRecordSet(pDb,bReadOnly)
	{
		m_rslist.SetFreeProc(FreeRs);
	}
	virtual ~THRSMulDB(){}

	virtual ITHRecordSet* NewObject(){return new THRSMulDB(m_pDb,m_bReadOnly);}

	virtual void *GetIntObj()
	{
		return (void *)&m_rslist;
	}

	virtual BOOL StartQuery(THString sql)
	{
		if (!m_pDb) return FALSE;
		if (m_pDb->GetProviderName()!=MULDB_PROVIDER) return FALSE;
		if (!EndQuery()) return FALSE;
		THList<ITHDataBase *> *db=(THList<ITHDataBase *> *)m_pDb->GetIntObj();
		THPosition pos=db->GetStartPosition();
		BOOL bret=TRUE;
		ITHDataBase *tmp;
		while(!pos.IsEmpty())
		{
			BOOL bpret=db->GetNextPosition(pos,tmp);
			if (bpret && tmp)
			{
				ITHRecordSet *set=tmp->NewRecordSet(m_bReadOnly);
				if (set)
				{
					m_rslist.AddTail(set);
					if (!set->StartQuery(sql))
						bret=FALSE;
				}
			}
		}
		m_pos=m_rslist.GetStartPosition();
		return bret;
	}
	virtual BOOL ReQuery()
	{
		THPosition pos=m_rslist.GetStartPosition();
		BOOL bret=TRUE;
		ITHRecordSet *tmp;
		while(!pos.IsEmpty())
		{
			BOOL bpret=m_rslist.GetNextPosition(pos,tmp);
			if (bpret && tmp)
				if (!tmp->ReQuery())
					bret=FALSE;
		}
		m_pos=m_rslist.GetStartPosition();
		return bret;
	}
	virtual BOOL EndQuery()
	{
		THPosition pos=m_rslist.GetStartPosition();
		BOOL bret=TRUE;
		ITHRecordSet *tmp;
		while(!pos.IsEmpty())
		{
			BOOL bpret=m_rslist.GetNextPosition(pos,tmp);
			if (bpret && tmp)
				if (!tmp->EndQuery())
					bret=FALSE;
		}
		m_rslist.RemoveAll();
		return bret;
	}
	virtual BOOL IsEOF()
	{
		if (m_rslist.GetSize()<=0) return TRUE;
		ITHRecordSet *tmp;
		BOOL bpret=m_rslist.GetAt(m_pos,tmp,FALSE);
		if (!bpret || tmp==NULL) return TRUE;
		if (!tmp->IsEOF()) return FALSE;
		THPosition pos=m_pos;
		while(!pos.IsEmpty())
		{
			bpret=m_rslist.GetNextPosition(pos,tmp);
			if (!bpret || tmp==NULL) return TRUE;
			if (!tmp->IsEOF()) return FALSE;
			m_pos=pos;
		}
		return TRUE;
	}
	virtual BOOL IsBOF()
	{
		//IsBOF判断方法与IsEOF不同，原因在于list向前查找比较麻烦，但由于初始指针定位在最前，因此很可能就可以
		//IsEOF改成另一种方法，目的是确定一种情况就是第一个记录集没有数据
		if (m_rslist.GetSize()<=0) return TRUE;
		if (m_pos==m_rslist.GetStartPosition())
		{
			ITHRecordSet *tmp;
			BOOL bpret=m_rslist.GetAt(m_pos,tmp,FALSE);
			if (bpret && tmp)
				return tmp->IsBOF();
		}
		return FALSE;
	}
	virtual BOOL MoveNext()
	{
		ITHRecordSet *tmp;
		BOOL bpret=m_rslist.GetAt(m_pos,tmp,FALSE);
		if (bpret && tmp)
		{
			tmp->MoveNext();
			if (tmp->IsEOF())
			{
				bpret=m_rslist.GetNextPosition(m_pos,tmp);
				if (bpret && tmp)
				{
					THPosition pos=m_pos;
					while(!pos.IsEmpty())
					{
						bpret=m_rslist.GetNextPosition(pos,tmp);
						if (!bpret || tmp==NULL) return FALSE;
						if (!tmp->IsEOF())
						{
							return tmp->MoveFirst();
						}
						m_pos=pos;
					}
				}
				return FALSE;
			}
			return TRUE;
		}
		return FALSE;
	}
	virtual BOOL MovePrev()
	{
		ITHRecordSet *tmp;
		BOOL bpret=m_rslist.GetAt(m_pos,tmp,FALSE);
		if (bpret && tmp)
		{
			tmp->MovePrev();
			if (tmp->IsBOF())
			{
				THPosition pos,oldpos;
startcheck:
				pos=m_rslist.GetStartPosition();
				if (pos==m_pos) return FALSE;
				while(pos.IsEmpty())
				{
					oldpos=pos;
					if (m_rslist.GetNextPosition(pos,tmp))
						if (pos==m_pos)
						{
							m_pos=oldpos;
							BOOL bpret=m_rslist.GetNextPosition(oldpos,tmp);
							if (bpret && tmp)
							{
								if (tmp->IsBOF()) goto startcheck;
								return tmp->MoveLast();
							}
						}
				}
				return FALSE;
			}
			return TRUE;
		}
		return FALSE;
	}
	virtual BOOL MoveFirst()
	{
		THPosition pos=m_rslist.GetStartPosition();
		BOOL bret=TRUE;
		ITHRecordSet *tmp;
		while(!pos.IsEmpty())
		{
			BOOL bpret=m_rslist.GetNextPosition(pos,tmp);
			if (bpret && tmp)
				if (!tmp->MoveFirst())
					bret=FALSE;
		}
		m_pos=m_rslist.GetStartPosition();
		return bret;
	}
	virtual BOOL MoveLast()
	{
		THPosition pos=m_rslist.GetStartPosition();
		BOOL bret=TRUE;
		ITHRecordSet *tmp;
		while(!pos.IsEmpty())
		{
			BOOL bpret=m_rslist.GetNextPosition(pos,tmp);
			if (bpret && tmp)
				if (!tmp->MoveLast())
					bret=FALSE;
		}
		if (m_rslist.GetSize()>0)
			m_pos=m_rslist.FindByIdx(m_rslist.GetSize()-1);
		else
			m_pos.Empty();
		return bret;
	}

	virtual int GetRecordCount()
	{
		THPosition pos=m_rslist.GetStartPosition();
		int cnt=0;
		ITHRecordSet *tmp;
		while(!pos.IsEmpty())
		{
			BOOL bpret=m_rslist.GetNextPosition(pos,tmp);
			if (bpret && tmp)
				cnt+=tmp->GetRecordCount();
		}
		return cnt;
	}

	virtual int GetColCount()
	{
		THPosition pos=m_rslist.GetStartPosition();
		int cnt=0,tmpcnt;
		ITHRecordSet *tmp;
		while(!pos.IsEmpty())
		{
			BOOL bpret=m_rslist.GetNextPosition(pos,tmp);
			if (bpret && tmp)
			{
				tmpcnt=tmp->GetColCount();
				if (tmpcnt>cnt)
					cnt=tmpcnt;
			}
		}
		return cnt;
	}

	virtual THString GetColName(int idx)
	{
		if (GetColCount()<=idx) return _T("");
		THPosition pos=m_rslist.GetStartPosition();
		THString ret;
		ITHRecordSet *tmp;
		while(!pos.IsEmpty())
		{
			BOOL bpret=m_rslist.GetNextPosition(pos,tmp);
			if (bpret && tmp)
			{
				ret=tmp->GetColName(idx);
				if (!ret.IsEmpty())
					return ret;
			}
		}
		return ret;
	}

	virtual THString GetColValue(int idx)
	{
		if (GetColCount()<=idx) return _T("");
		ITHRecordSet *tmp;
		BOOL bpret=m_rslist.GetAt(m_pos,tmp,FALSE);
		if (bpret && tmp)
			return tmp->GetColValue(idx);
		return _T("");
	}

	virtual THString GetColValue(THString sKey)
	{
		ITHRecordSet *tmp;
		BOOL bpret=m_rslist.GetAt(m_pos,tmp,FALSE);
		if (bpret && tmp)
			return tmp->GetColValue(sKey);
		return _T("");
	}
protected:
	THPosition m_pos;
	THList<ITHRecordSet *>m_rslist;
};


/**
* @brief 多数据库联合查询封装类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-12-10 新建类
*/
class THMulDataBase :public ITHDataBase
{
private:
	static void FreeDb(void *key,void *value,void *adddata)
	{
		ITHDataBase *tmp=(ITHDataBase *)key;
		if (tmp) tmp->ReleaseObject(tmp);
	}
public:
	THMulDataBase()
	{
		m_dblist.SetFreeProc(FreeDb);
	}
	virtual ~THMulDataBase(){CloseDataBase();}

	virtual ITHDataBase* NewObject(){return new THMulDataBase;}

	virtual ITHRecordSet* NewRecordSet(BOOL bReadOnly=FALSE){return new THRSMulDB(this,bReadOnly);}

	virtual void *GetIntObj()
	{
		return (void *)&m_dblist;
	}

	//注意,指针由类里释放
	virtual BOOL AddDataBase(ITHDataBase *db)
	{
		return m_dblist.AddTail(db);
	}

	virtual BOOL OpenDataBase(THString sOpenStr,BOOL bReadOnly=FALSE)
	{
		return FALSE;
	}

	virtual BOOL OpenDataBase(THString sDriver,THString sHost,int nPort,THString sDataBase,THString sUserName,THString sPassWord,BOOL bReadOnly=FALSE)
	{
		return FALSE;
	}

	virtual BOOL CloseDataBase()
	{
		THPosition pos=m_dblist.GetStartPosition();
		BOOL bret=TRUE;
		ITHDataBase *tmp;
		while(!pos.IsEmpty())
		{
			BOOL bpret=m_dblist.GetNextPosition(pos,tmp);
			if (bpret && tmp)
				if (!tmp->CloseDataBase())
					bret=FALSE;
		}
		return bret;
	}

	virtual BOOL IsOpened()
	{
		ITHDataBase *tmp;
		if (m_dblist.GetAt(0,tmp,FALSE))
			return tmp->IsOpened();
		return FALSE;
	}

	virtual BOOL ExecuteSql(const TCHAR *lpszSql)
	{
		THPosition pos=m_dblist.GetStartPosition();
		BOOL bret=TRUE;
		ITHDataBase *tmp;
		while(!pos.IsEmpty())
		{
			BOOL bpret=m_dblist.GetNextPosition(pos,tmp);
			if (bpret && tmp)
				if (!tmp->ExecuteSql(lpszSql))
					bret=FALSE;
		}
		return bret;
	}

	virtual THString GetProviderName()
	{
		return MULDB_PROVIDER;
	}
protected:
	THList<ITHDataBase *> m_dblist;
};
