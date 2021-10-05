#pragma once

#include <THString.h>
#include <THCharset.h>
#include <THDllLoader.h>
#include <THSystem.h>
#include <THFile.h>

typedef int (*sqlite3_callback)(void*,int,char**,char**);
typedef int (*sqlite3_open)(const char *filename,void **ppDb);
typedef int (*sqlite3_exec)(void *db,const char *sql,sqlite3_callback cb,void *pArg,char **errmsg);
typedef void (*sqlite3_free)(void *);
typedef int (*sqlite3_prepare_v2)(void *db,const void *zSql,int nBytes,void **ppStmt,const void **pzTail);
typedef int (*sqlite3_step)(void *pStmt);
typedef const char * (*sqlite3_column_text)(void *pStmt,int iCol);
typedef const char * (*sqlite3_column_name)(void *pStmt,int iCol);
typedef int (*sqlite3_column_count)(void *pStmt);
typedef int (*sqlite3_finalize)(void *pStmt);
typedef int (*sqlite3_close)(void *db);
typedef int (*sqlite3_get_table)(void *db,const wchar_t *sql,char ***result,int *nrow,int *ncolumn,char **errmsg);
typedef void (*sqlite3_free_table)(char **result);
typedef int (*sqlite3_busy_timeout)(void*db,int ms);
typedef __int64 (*sqlite3_last_insert_rowid)(void*);

/**
* @brief Sqlite����ö�ٻص���
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-07-03 �½���
*/
class THSqliteDataHandler
{
public:
	/**
	* @brief ö��������Ӧ
	* @param titles		���ݱ���
	* @param args		��������
	* @return			�����Ƿ���ֹö��
	*/
	virtual BOOL OnEnumData(THStringArray *titles,THStringArray *args)=0;

	/**
	* @brief ��������
	* @param key		������
	* @param titles		���ݱ���
	* @param args		��������
	* @return			��������
	*/
	THString FindValues(THString key,THStringArray *titles,THStringArray *args)
	{
		for(int i=0;i<titles->GetSize();i++)
		{
			if (key==(*titles)[i])
				return (*args)[i];
		}
		return _T("");
	}
};

/**
* @brief Sqlite��װ��
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-07-03 �½���
* @2007-08-25 ���Ӵ�����뷵��
* @2008-05-07 ���ӵ�������
* @2009-11-04 ���ӻ�ȡlast_insert_row_id���ı���ʽ����
*/
/**<pre>
�÷���
	class MyData : public THSqliteDataHandler
	{
		BOOL CAllTestDlg::OnEnumData(THStringArray *titles,THStringArray *args)
		{
			THDebug(FindValues(_T("first_name"),titles,args));
			return FALSE;
		}
	}

	THSqliteDll m_sqldll;
	MyData *tmp=new MyData;
	if (m_sqldll.OpenDataBase(_T("c:\\aaa.db")))
	{
		m_sqldll.ExecSql(_T("Create table contacts (id INTEGER PRIMARY KEY,first_name CHAR,last_name CHAR,email CHAR);"));
		m_sqldll.ExecSql(_T("insert into contacts(first_name,last_name,email) values('dwq','dwqf','wqfqw@21cn.com')"));
		m_sqldll.ExecSql(_T("SELECT * FROM contacts"),tmp);
		if (m_sqldll.StartQuery(_T("SELECT * FROM contacts")))
		{
			while(m_sqldll.GetNext())
			{
				THDebug(_T("%s,%s"),m_sqldll.GetValue(0),m_sqldll.GetValueByKey(_T("email")));
			}
			m_sqldll.EndQuery();
		}
	}
</pre>*/
class THSqliteDll : public THDllLoader
{
public:
	static int OnDataCallBack(void *pArg, int argc, char **argv, char **columnNames)
	{
		THSqliteDataHandler *handler=(THSqliteDataHandler *)pArg;
		if (handler)
		{
			THStringArray key;
			THStringArray value;
			for(int i=0; i<argc; i++)
			{
				key.Add(THCharset::u82t(columnNames[i]));
				value.Add(THCharset::u82t(argv[i]));
			}
			return handler->OnEnumData(&key,&value);
		}
		return(0);
	}

	THSqliteDll():THDllLoader(_T("Sqlite340.dll"))
	{
		EmptyDllPointer();
		m_file=NULL;
		m_stmt=NULL;
	}

	virtual ~THSqliteDll(){FreeDll();}

	virtual BOOL OnFreeDll()
	{
		CloseDataBase();
		return THDllLoader::OnFreeDll();
	}

	virtual void EmptyDllPointer()
	{
		sqlopen=NULL;
		sqlexec=NULL;
		sqlfree=NULL;
		sqlprepare=NULL;
		sqlstep=NULL;
		sqlcoltext=NULL;
		sqlcolname=NULL;
		sqlcolcount=NULL;
		sqlfinalize=NULL;
		sqlclose=NULL;
		sqlgettable=NULL;
		sqlfreetable=NULL;
		sqltimeout=NULL;
	}

	virtual BOOL InitDllPoinier()
	{
		sqlopen=(sqlite3_open)GetProcAddress(m_module,"sqlite3_open");
		sqlexec=(sqlite3_exec)GetProcAddress(m_module,"sqlite3_exec");
		sqlfree=(sqlite3_free)GetProcAddress(m_module,"sqlite3_free");
		sqlprepare=(sqlite3_prepare_v2)GetProcAddress(m_module,"sqlite3_prepare_v2");
		sqlstep=(sqlite3_step)GetProcAddress(m_module,"sqlite3_step");
		sqlcoltext=(sqlite3_column_text)GetProcAddress(m_module,"sqlite3_column_text");
		sqlcolname=(sqlite3_column_name)GetProcAddress(m_module,"sqlite3_column_name");
		sqlcolcount=(sqlite3_column_count)GetProcAddress(m_module,"sqlite3_column_count");
		sqlfinalize=(sqlite3_finalize)GetProcAddress(m_module,"sqlite3_finalize");
		sqlclose=(sqlite3_close)GetProcAddress(m_module,"sqlite3_close");
		sqlgettable=(sqlite3_get_table)GetProcAddress(m_module,"sqlite3_get_table");
		sqlfreetable=(sqlite3_free_table)GetProcAddress(m_module,"sqlite3_free_table");
		sqltimeout=(sqlite3_busy_timeout)GetProcAddress(m_module,"sqlite3_busy_timeout");
		sqllastinsertrowid=(sqlite3_last_insert_rowid)GetProcAddress(m_module,"sqlite3_last_insert_rowid");
		if (!sqlopen || !sqlexec || !sqlfree || !sqlprepare ||
			!sqlstep || !sqlcoltext || !sqlcolname || !sqlcolcount || !sqlfinalize || !sqlclose || !sqlgettable || !sqlfreetable ||
			!sqltimeout || !sqllastinsertrowid)
			return FALSE;
		return TRUE;
	}

	/**
	* @brief �����ݿ�
	* @param sFile				���ݿ��ļ�
	* @param bFailIfNotExist	�ļ��������Ƿ񷵻�ʧ��
	* @return �Ƿ�ɹ�
	*/
	BOOL OpenDataBase(const THString sFile,BOOL bFailIfNotExist=FALSE)
	{
		if (sFile.IsEmpty()) return FALSE;
		CloseDataBase();
		if (!sqlopen) return FALSE;
		if (bFailIfNotExist && ::PathFileExists(sFile)==FALSE) return FALSE;
		char *pFile=THCharset::t2u8(sFile);
		if (!pFile) return FALSE;
		int ret=sqlopen(pFile,&m_file);
		THCharset::free(pFile);
		return (ret==0);
	}

	/**
	* @brief �ر����ݿ�
	* @return �Ƿ�ɹ�
	*/
	BOOL CloseDataBase()
	{
		if (!InitDll()) return FALSE;
		if (!sqlclose) return FALSE;
		if (m_file)
		{
			if (sqlclose(m_file)!=0)
			{
				m_file=NULL;
				return FALSE;
			}
			m_file=NULL;
		}
		return TRUE;
	}

	/**
	* @brief ��ʽ����ִ��Sql���
	* @param handler			���ؽ���Ļص�handler
	* @param lpszSql			Sql���
	* @param ...				Sql������
	* @return �Ƿ�ɹ�
	*/
	BOOL ExecSqlFormat(THSqliteDataHandler *handler,const TCHAR *lpszSql,...)
	{
		THString str;
		va_list pArg;
		va_start(pArg,lpszSql);
		str.FormatV(lpszSql,pArg);
		va_end(pArg);
		return ExecSql(str,handler);
	}

	/**
	* @brief ִ��Sql��䣬��ı�m_sLastErrMsgֵ
	* @param sql				Sql���
	* @param handler			���ؽ���Ļص�handler
	* @return �Ƿ�ɹ�
	*/
	BOOL ExecSql(const THString sql,THSqliteDataHandler *handler=NULL)
	{
		if (!InitDll()) return FALSE;
		if (!sqlexec || !sqlfree) return FALSE;
		if (!m_file) return FALSE;
		char *pSql=THCharset::t2u8(sql);
		if (!pSql) return FALSE;
		char *errmsg=NULL;
		int ret=sqlexec(m_file,pSql,OnDataCallBack,handler,&errmsg);
		THCharset::free(pSql);
		if (errmsg)
		{
			m_sLastErrMsg=THCharset::u82t(errmsg);
			sqlfree(errmsg);
		}
		else
			m_sLastErrMsg.Empty();
		return (ret==0);
	}

	/**
	* @brief ִ��Sql�ļ��е����
	* @param sFile				Sql�ű��ļ���������ascii����
	* @param handler			���ؽ���Ļص�handler
	* @return �Ƿ�ɹ�
	*/
	BOOL ExecSqlFromFile(const THString sFile,THSqliteDataHandler *handler=NULL)
	{
		if (!InitDll()) return FALSE;
		if (!sqlexec || !sqlfree) return FALSE;
		if (!m_file) return FALSE;
		THFile file;
		char *tmp=NULL;
		if (!file.Open(sFile,THFile::modeRead)) return FALSE;
		DWORD size=(DWORD)file.GetLength();
		if (size<=0) return FALSE;
		tmp=new char[size+1];
		if (!tmp)
		{
			file.Close();
			return FALSE;
		}
		if (file.Read(tmp,size)!=size)
		{
			file.Close();
			delete [] tmp;
			return FALSE;
		}
		tmp[size]='\0';
		file.Close();
		THString sret=THCharset::a2t(tmp);
		delete [] tmp;
		char *pSql=THCharset::t2u8(sret);
		if (!pSql) return FALSE;
		char *errmsg=NULL;
		int ret=sqlexec(m_file,pSql,OnDataCallBack,handler,&errmsg);
		THCharset::free(pSql);
		if (errmsg)
		{
			m_sLastErrMsg=THCharset::u82t(errmsg);
			sqlfree(errmsg);
		}
		else
			m_sLastErrMsg.Empty();
		return (ret==0);
	}

	/**
	* @brief ��ʼ�����ѯ
	* @param sql				Sql���
	* @return �Ƿ�ɹ�
	*/
	BOOL StartQuery(const THString sql)
	{
		if (!EndQuery()) return FALSE;
		if (!InitDll()) return FALSE;
		if (!m_file) return FALSE;
		if (!sqlprepare) return FALSE;
		char *pSql=THCharset::t2u8(sql);
		if (!pSql) return FALSE;
		int ret=sqlprepare(m_file,pSql,-1,&m_stmt,NULL);//NULL is Pointer to unused portion of pSql
		THCharset::free(pSql);
		return (ret==0);
	}

	/**
	* @brief ��ʽ������ʼ�����ѯ
	* @param lpszSql			Sql���
	* @param ...				Sql������
	* @return �Ƿ�ɹ�
	*/
	BOOL StartQueryFormat(const TCHAR *lpszSql,...)
	{
		THString str;
		va_list pArg;
		va_start(pArg,lpszSql);
		str.FormatV(lpszSql,pArg);
		va_end(pArg);
		return StartQuery(str);
	}

	/**
	* @brief ��������ѯ
	* @param sql				Sql���
	* @param handler			���ؽ���Ļص�handler
	* @return �Ƿ�ɹ�
	*/
	BOOL EndQuery()
	{
		if (!InitDll()) return FALSE;
		if (!sqlfinalize) return FALSE;
		if (!m_file) return FALSE;
		if (!m_stmt) return TRUE;
		int ret=sqlfinalize(m_stmt);
		m_stmt=NULL;
		return (ret==0);
	}

	/**
	* @brief ��ȡ��һ������
	* @return �Ƿ�ɹ�
	*/
	BOOL GetNext()
	{
		if (!InitDll()) return FALSE;
		if (!sqlstep) return FALSE;
		if (!m_file) return FALSE;
		if (!m_stmt) return FALSE;
		return (sqlstep(m_stmt)==100);//SQLITE_ROW
	}

	/**
	* @brief ��ȡ�и���
	* @return �и���
	*/
	int GetColSize()
	{
		if (InitDll() && sqlcolcount && m_stmt)
			return sqlcolcount(m_stmt);
		return 0;
	}

	/**
	* @brief ��ȡ������
	* @param i					�ڼ���
	* @return ������
	*/
	THString GetColName(int i)
	{
		THString str;
		if (InitDll() && sqlcolname && m_stmt)
		{
			const char *tmp=sqlcolname(m_stmt,i);
			if (tmp) str=THCharset::u82t(tmp);
		}
		return str;
	}

	/**
	* @brief ��ȡ��ֵ
	* @param col				�ڼ���
	* @return ��ֵ
	*/
	THString GetValue(int col,THString defval=_T(""))
	{
		THString ret;
		if (InitDll() && sqlcoltext && m_stmt)
		{
			const char *tmp=sqlcoltext(m_stmt,col);
			if (tmp)
				ret=THCharset::u82t(tmp);
		}
		if (ret.IsEmpty())
			ret=defval;
		return ret;
	}

	/**
	* @brief ��ȡ��ֵ
	* @param key				������
	* @return ��ֵ
	*/
	THString GetValueByKey(THString key,THString defval=_T(""))
	{
		if (InitDll() && sqlcoltext && m_stmt)
		{
			int colsize=GetColSize();
			THString str;
			for(int i=0;i<colsize;i++)
			{
				str=GetColName(i);
				if (str==key)
					return GetValue(i,defval);
			}
		}
		return defval;
	}

	/**
	* @brief �������ݣ��ú������ò���Ӱ�����ڵ��ò�ѯ����ı�m_sLastErrMsgֵ
	* @param sSql				Sql���
	* @param sSaveFile			�������ļ�
	* @param sSepSym			���ݷָ���
	* @param sItemSepSym		��Ŀ�ָ���
	* @param bUseTitle			�Ƿ�ʹ�ñ���
	* @return �����Ƿ�ɹ�
	*/
	BOOL ExportData(THString sSql,THString sSaveFile,THString sSepSym,THString sItemSepSym,BOOL bUseTitle)
	{
		THFile file;
		m_sLastErrMsg=_T("can not create file");
		if (!file.Open(sSaveFile,THFile::modeCreate|THFile::modeWrite)) return FALSE;
		m_sLastErrMsg=_T("sqlite not prepare");
		if (!InitDll()) return FALSE;
		if (!m_file) return FALSE;
		if (!sqlprepare || !sqlstep || !sqlcoltext || !sqlfinalize || !sqlcolcount || !sqlcolname) return FALSE;
		char *pSql=THCharset::t2u8(sSql);
		m_sLastErrMsg=_T("memory error");
		if (!pSql) return FALSE;
		void *stmt;
		int nret=sqlprepare(m_file,pSql,-1,&stmt,NULL);//NULL is Pointer to unused portion of pSql
		THCharset::free(pSql);
		m_sLastErrMsg=_T("sqlprepare error");
		if (nret!=0) return FALSE;
		if (!stmt) return FALSE;
		int ColCount=sqlcolcount(stmt);
		THString ret;
		if (bUseTitle)
		{
			for(int i=0;i<ColCount;i++)
			{
				if (i!=0) ret+=sSepSym;
				const char *tmp=sqlcolname(stmt,i);
				if (tmp) ret+=THCharset::u82t(tmp);
			}
			ret+=sItemSepSym;
		}
		while(sqlstep(stmt)==100)//SQLITE_ROW
		{
			for(int i=0;i<ColCount;i++)
			{
				if (i!=0) ret+=sSepSym;
				const char *tmp=sqlcoltext(stmt,i);
				if (tmp) ret+=THCharset::u82t(tmp);
			}
			ret+=sItemSepSym;
		}
		nret=sqlfinalize(stmt);
		file.Write((LPCTSTR)ret,ret.GetLength());
		file.Close();
		return TRUE;
	}

#define CHECKTABLE(a,b) if (a.IsEmpty()) a=b;a.Replace(_T("(empty)"),_T(""))

	/**
	* @brief �������ݣ��ú������ò���Ӱ�����ڵ��ò�ѯ����ı�m_sLastErrMsgֵ
	* @param sSql				Sql���
	* @param bUseTitle			�Ƿ�ʹ�ñ���
	* @param pColCount			������������Ϊ��
	* @param pItemCount			������������Ϊ��
	* @param sLineS				��ǰ�������ݣ�Ĭ��Ϊ<tr>������Ҫ�գ��븳ֵ_T("(empty)")
	* @param sLineE				�к�������ݣ�Ĭ��Ϊ</tr>������Ҫ�գ��븳ֵ_T("(empty)")
	* @param sItemS				������Ŀǰ�������ݣ�Ĭ��Ϊ<td>������Ҫ�գ��븳ֵ_T("(empty)")
	* @param sItemE				������Ŀ��������ݣ�Ĭ��Ϊ</td>������Ҫ�գ��븳ֵ_T("(empty)")
	* @param sTitleItemS		������Ŀǰ�������ݣ�Ĭ��Ϊ<td><b>������Ҫ�գ��븳ֵ_T("(empty)")
	* @param sTitleItemE		������Ŀ��������ݣ�Ĭ��Ϊ</b></td>������Ҫ�գ��븳ֵ_T("(empty)")
	* @param sHead				����ǰ��������ݣ�Ĭ��Ϊ<table>������Ҫ�գ��븳ֵ_T("(empty)")
	* @param sEnd				������������ݣ�Ĭ��Ϊ</table>������Ҫ�գ��븳ֵ_T("(empty)")
	* @param sEmpty				��ֵ�滻���ݣ�Ĭ��Ϊ&nbsp;������Ҫ�գ��븳ֵ_T("(empty)")
	* @return ���ر����ַ���
	*/
	THString ExportDataToTable(THString sSql,BOOL bUseTitle=FALSE,UINT *pColCount=NULL,UINT *pItemCount=NULL,
		THString sLineS=_T(""),THString sLineE=_T(""),THString sItemS=_T(""),THString sItemE=_T(""),THString sTitleItemS=_T(""),THString sTitleItemE=_T(""),
		THString sHead=_T(""),THString sEnd=_T(""),THString sEmpty=_T(""))
	{
		THString ret;
		if (pColCount) *pColCount=0;
		if (pItemCount) *pItemCount=0;
		CHECKTABLE(sLineS,_T("<tr>"));
		CHECKTABLE(sLineE,_T("</tr>"));
		CHECKTABLE(sItemS,_T("<td>"));
		CHECKTABLE(sItemE,_T("</td>"));
		CHECKTABLE(sTitleItemS,_T("<td><b>"));
		CHECKTABLE(sTitleItemE,_T("</b></td>"));
		CHECKTABLE(sHead,_T("<table>"));
		CHECKTABLE(sEnd,_T("</table>"));
		CHECKTABLE(sEmpty,_T("&nbsp;"));
		m_sLastErrMsg=_T("sqlite not prepare");
		if (!InitDll()) return ret;
		if (!m_file) return ret;
		if (!sqlprepare || !sqlstep || !sqlcoltext || !sqlfinalize || !sqlcolcount || !sqlcolname) return ret;
		char *pSql=THCharset::t2u8(sSql);
		m_sLastErrMsg=_T("memory error");
		if (!pSql) return ret;
		void *stmt;
		int nret=sqlprepare(m_file,pSql,-1,&stmt,NULL);//NULL is Pointer to unused portion of pSql
		THCharset::free(pSql);
		m_sLastErrMsg=_T("sqlprepare error");
		if (nret!=0) return ret;
		if (!stmt) return ret;
		int ColCount=sqlcolcount(stmt);
		if (ColCount==0) return ret;
		if (pColCount) *pColCount=ColCount;
		THString sTemp;
		ret+=sHead;
		if (bUseTitle)
		{
			ret+=sLineS;
			for(int i=0;i<ColCount;i++)
			{
				ret+=sTitleItemS;
				const char *tmp=sqlcolname(stmt,i);
				if (tmp)
					sTemp=THCharset::u82t(tmp);
				else
					sTemp.Empty();
				if (sTemp.IsEmpty())
					ret+=sEmpty;
				else
					ret+=sTemp;
				ret+=sTitleItemE;
			}
			ret+=sLineE;
		}
		while(sqlstep(stmt)==100)//SQLITE_ROW
		{
			ret+=sLineS;
			for(int i=0;i<ColCount;i++)
			{
				ret+=sItemS;
				const char *tmp=sqlcoltext(stmt,i);
				if (tmp)
					sTemp=THCharset::u82t(tmp);
				else
					sTemp.Empty();
				if (sTemp.IsEmpty())
					ret+=sEmpty;
				else
					ret+=sTemp;
				ret+=sItemE;
			}
			if (pItemCount) *pItemCount=(*pItemCount)+1;
			ret+=sLineE;
		}
		ret+=sEnd;
		nret=sqlfinalize(stmt);
		return ret;
	}

	/**
	* @brief �����ı����ݣ��ú������ò���Ӱ�����ڵ��ò�ѯ����ı�m_sLastErrMsgֵ
	* @param sSql				Sql���
	* @param bUseTitle			�Ƿ�ʹ�ñ���
	* @param pColCount			������������Ϊ��
	* @param pItemCount			������������Ϊ��
	* @return ���ر����ַ���
	*/
	THString ExportDataToTextTable(THString sSql,BOOL bUseTitle=TRUE,UINT *pColCount=NULL,UINT *pItemCount=NULL)
	{
		THString ret;
		if (pColCount) *pColCount=0;
		if (pItemCount) *pItemCount=0;
		m_sLastErrMsg=_T("sqlite not prepare");
		if (!InitDll()) return ret;
		if (!m_file) return ret;
		if (!sqlprepare || !sqlstep || !sqlcoltext || !sqlfinalize || !sqlcolcount || !sqlcolname) return ret;
		char *pSql=THCharset::t2u8(sSql);
		m_sLastErrMsg=_T("memory error");
		if (!pSql) return ret;
		void *stmt;
		int nret=sqlprepare(m_file,pSql,-1,&stmt,NULL);//NULL is Pointer to unused portion of pSql
		THCharset::free(pSql);
		m_sLastErrMsg=_T("sqlprepare error");
		if (nret!=0) return ret;
		if (!stmt) return ret;
		int ColCount=sqlcolcount(stmt);
		if (ColCount==0) return ret;
		if (pColCount) *pColCount=ColCount;
		vector<int> colw;
		ret=_T("[COLLINE]\r\n");
		for(int i=0;i<ColCount;i++)
			colw.push_back(0);
		THString sTemp;
		THString colkey;
		int off;
		if (bUseTitle)
		{
			THString title=_T("[COLLINE]\r\n|");
			for(int i=0;i<ColCount;i++)
			{
				colkey.Format(_T("[COL%d]"),i);
				const char *tmp=sqlcolname(stmt,i);
				if (tmp)
					sTemp=THCharset::u82t(tmp);
				else
					sTemp.Empty();
				off=sTemp.GetLength();
				if (colw[i]<off)
				{
					off=off-colw[i];
					colw[i]+=off;
					ret.Replace(colkey,THStrRepeat(off)+colkey);
				}
				title+=sTemp+colkey+_T("|");
			}
			ret=title+_T("\r\n")+ret;
		}
		ret=_T("\r\n")+ret;
		while(sqlstep(stmt)==100)//SQLITE_ROW
		{
			ret+=_T("|");
			for(int i=0;i<ColCount;i++)
			{
				colkey.Format(_T("[COL%d]"),i);
				const char *tmp=sqlcoltext(stmt,i);
				if (tmp)
					sTemp=THCharset::u82t(tmp);
				else
					sTemp.Empty();
				off=sTemp.GetLength();
				if (colw[i]<off)
				{
					off=off-colw[i];
					colw[i]+=off;
					ret.Replace(colkey,THStrRepeat(off)+colkey);
					ret+=colkey+sTemp+_T("|");
				}
				else
					ret+=colkey+THStrRepeat(colw[i]-off)+sTemp+_T("|");
			}
			ret+=_T("\r\n");
			if (pItemCount) *pItemCount=(*pItemCount)+1;
		}
		ret+=_T("[COLLINE]\r\n");
		THString colstr=_T("+");
		for(int i=0;i<ColCount;i++)
		{
			colkey.Format(_T("[COL%d]"),i);
			ret.Replace(colkey,_T(""));
			colstr+=THStrRepeat(colw[i],_T("-"))+_T("+");
		}
		ret.Replace(_T("[COLLINE]"),colstr);
		nret=sqlfinalize(stmt);
		return ret;
	}

	/**
	* @brief ��ȡsql�����������н�����ú������ò���Ӱ�����ڵ��ò�ѯ����ı�m_sLastErrMsgֵ
	* @param sSql				Sql���
	* @param psRet				���ؽ��
	* @return ���ؽ������������ؿ�
	*/
	BOOL GetFirstResult(THString sSql,THString *psRet)
	{
		m_sLastErrMsg=_T("args is wrong");
		if (!psRet) return FALSE;
		m_sLastErrMsg=_T("sqlite not prepare");
		if (!InitDll()) return FALSE;
		if (!m_file) return FALSE;
		if (!sqlprepare || !sqlstep || !sqlcoltext || !sqlfinalize) return FALSE;
		char *pSql=THCharset::t2u8(sSql);
		m_sLastErrMsg=_T("memory error");
		if (!pSql) return FALSE;
		void *stmt;
		int nret=sqlprepare(m_file,pSql,-1,&stmt,NULL);//NULL is Pointer to unused portion of pSql
		THCharset::free(pSql);
		m_sLastErrMsg=_T("sqlprepare error");
		if (nret!=0) return FALSE;
		if (!stmt) return FALSE;
		if (sqlstep(stmt)==100)//SQLITE_ROW
		{
			const char *tmp=sqlcoltext(stmt,0);
			if (tmp)
				*psRet=THCharset::u82t(tmp);
		}
		nret=sqlfinalize(stmt);
		return TRUE;
	}

	/**
	* @brief ��ȡsql�����������н�����ú������ò���Ӱ�����ڵ��ò�ѯ����ı�m_sLastErrMsgֵ
	* @param sSql				Sql���
	* @return ���ؽ������������ؿ�
	*/
	THString GetFirstResult(THString sSql)
	{
		THString ret;
		GetFirstResult(sSql,&ret);
		return ret;
	}

	/**
	* @brief ��ʽ������ȡsql�����������н�����ú������ò���Ӱ�����ڵ��ò�ѯ����ı�m_sLastErrMsgֵ
	* @param lpszSql		Sql���
	* @return ���ؽ������������ؿ�
	*/
	THString GetFirstResultFormat(const TCHAR *lpszSql,...)
	{
		THString str;
		va_list pArg;
		va_start(pArg,lpszSql);
		str.FormatV(lpszSql,pArg);
		va_end(pArg);
		THString ret;
		GetFirstResult(str,&ret);
		return ret;
	}

	/**
	* @brief ���ò�ѯ��ʱ
	* @param nMs	������
	* @return �����Ƿ�ɹ�
	*/
	BOOL SetTimeOut(int nMs)
	{
		if (!InitDll()) return FALSE;
		if (!sqltimeout) return FALSE;
		if (!m_file) return FALSE;
		return (sqltimeout(m_file,nMs)==0);
	}

	/**
	* @brief ��ȡ�������rowid
	* @return ���ؽ��
	*/
	__int64 GetLastInsertRowId()
	{
		if (!InitDll()) return 0;
		if (!sqltimeout) return 0;
		if (!m_file) return 0;
		return (sqllastinsertrowid(m_file));
	}
	THString GetLastErrMessage(){return m_sLastErrMsg;}
private:
	sqlite3_open sqlopen;
	sqlite3_exec sqlexec;
	sqlite3_free sqlfree;
	sqlite3_prepare_v2 sqlprepare;
	sqlite3_step sqlstep;
	sqlite3_column_text sqlcoltext;
	sqlite3_column_name sqlcolname;
	sqlite3_column_count sqlcolcount;
	sqlite3_finalize sqlfinalize;
	sqlite3_close sqlclose;
	sqlite3_get_table sqlgettable;
	sqlite3_free_table sqlfreetable;
	sqlite3_busy_timeout sqltimeout;
	sqlite3_last_insert_rowid sqllastinsertrowid;
	void *m_file;
	void *m_stmt;
	THString m_sLastErrMsg;
};