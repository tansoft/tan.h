#pragma once

#include <THDataBase.h>
#include <afxdb.h>
#include <odbcinst.h>

#define ODBC_PROVIDER		_T("ODBC Provider")

/**
* @brief 记录集ODBC方式封装类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-10-29 新建类
*/
class THRSOdbc : public ITHRecordSet
{
public:
	THRSOdbc():ITHRecordSet(){}
	THRSOdbc(ITHDataBase *pDb,BOOL bReadOnly=FALSE):ITHRecordSet(pDb,bReadOnly){}
	virtual ~THRSOdbc(){}

	virtual ITHRecordSet* NewObject(){return new THRSOdbc(m_pDb,m_bReadOnly);}

	virtual void *GetIntObj()
	{
		return (void *)&m_rs;
	}

	virtual BOOL StartQuery(THString sql)
	{
		if (!m_pDb) return FALSE;
		if (m_pDb->GetProviderName()!=ODBC_PROVIDER) return FALSE;
		m_rs.m_pDatabase=(CDatabase *)m_pDb->GetIntObj();
		if (m_rs.IsOpen()) m_rs.Close();
		return m_rs.Open(m_bReadOnly?CRecordset::snapshot:CRecordset::dynaset,sql);
	}
	virtual BOOL ReQuery(){return m_rs.Requery();}
	virtual BOOL EndQuery(){if (m_rs.IsOpen()) m_rs.Close();return TRUE;}
	virtual BOOL IsEOF(){return m_rs.IsEOF();}
	virtual BOOL IsBOF(){return m_rs.IsBOF();}
	virtual BOOL MoveNext(){m_rs.MoveNext();return TRUE;}
	virtual BOOL MovePrev(){m_rs.MovePrev();return TRUE;}
	virtual BOOL MoveFirst(){m_rs.MoveFirst();return TRUE;}
	virtual BOOL MoveLast(){m_rs.MoveLast();return TRUE;}

	virtual int GetRecordCount(){return (int)m_rs.GetRecordCount();}

	virtual int GetColCount(){return (int)m_rs.GetODBCFieldCount();}

	virtual THString GetColName(int idx)
	{
		if (GetColCount()<=idx) return _T("");
		CODBCFieldInfo info;
		m_rs.GetODBCFieldInfo(idx,info);
		return info.m_strName;
	}

	virtual THString GetColValue(int idx)
	{
		if (GetColCount()<=idx) return _T("");
		THString val;
		m_rs.GetFieldValue(idx,val);
		return val;
	}

	virtual THString GetColValue(THString sKey)
	{
		THString val;
		m_rs.GetFieldValue(sKey,val);
		return val;
	}
protected:
	CRecordset m_rs;
};

/**
* @brief 数据库ODBC方式封装类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-10-29 新建类
*/
class THDBOdbc : public ITHDataBase
{
public:
	THDBOdbc(){}
	virtual ~THDBOdbc(){CloseDataBase();}

	virtual ITHDataBase* NewObject(){return new THDBOdbc;}

	virtual ITHRecordSet* NewRecordSet(BOOL bReadOnly=FALSE){return new THRSOdbc(this,bReadOnly);}

	virtual BOOL OpenDataBase(THString sOpenStr,BOOL bReadOnly=FALSE)
	{
		if (!CloseDataBase()) return FALSE;
		DWORD opt=0;
		if (bReadOnly) opt=CDatabase::openReadOnly;
#ifndef _DEBUG
		opt|=CDatabase::noOdbcDialog;
#endif
		BOOL ret=FALSE;
		try{
			ret=m_db.OpenEx(sOpenStr,opt);
		}
		catch(CDBException *)
		{
			ret=FALSE;
		}
		return ret;
	}

	virtual void *GetIntObj()
	{
		return (void *)&m_db;
	}

	//sDriver Simple:SQL Server,
	virtual BOOL OpenDataBase(THString sDriver,THString sHost,int nPort,THString sDataBase,THString sUserName,THString sPassWord,BOOL bReadOnly=FALSE)
	{
		THString str;
		str.Format(_T("Driver={%s};Server=%s,%d;Database=%s;Uid=%s;Pwd=%s"),sDriver,sHost,nPort,sDataBase,sUserName,sPassWord);
		return OpenDataBase(str,bReadOnly);
	}

	virtual BOOL CloseDataBase()
	{
		if (m_db.IsOpen()) m_db.Close();
		return TRUE;
	}

	virtual BOOL IsOpened()
	{
		return m_db.IsOpen();
	}

	virtual BOOL ExecuteSql(const TCHAR *lpszSql)
	{
		m_db.ExecuteSQL(lpszSql);
		return TRUE;
	}

	virtual THString GetProviderName()
	{
		return ODBC_PROVIDER;
	}
protected:
	CDatabase m_db;
};
