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
* @brief Sqlite数据枚举回调类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-07-03 新建类
*/
class THSqliteDataHandler
{
public:
	/**
	* @brief 枚举数据响应
	* @param titles		数据标题
	* @param args		数据内容
	* @return			返回是否中止枚举
	*/
	virtual BOOL OnEnumData(THStringArray *titles,THStringArray *args)=0;

	/**
	* @brief 查找数据
	* @param key		标题名
	* @param titles		数据标题
	* @param args		数据内容
	* @return			返回内容
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
* @brief Sqlite封装类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-07-03 新建类
* @2007-08-25 增加错误代码返回
* @2008-05-07 增加导出报表
* @2009-11-04 增加获取last_insert_row_id，文本方式报表
*/
/**<pre>
用法：
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
	* @brief 打开数据库
	* @param sFile				数据库文件
	* @param bFailIfNotExist	文件不存在是否返回失败
	* @return 是否成功
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
	* @brief 关闭数据库
	* @return 是否成功
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
	* @brief 格式化并执行Sql语句
	* @param handler			返回结果的回调handler
	* @param lpszSql			Sql语句
	* @param ...				Sql语句参数
	* @return 是否成功
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
	* @brief 执行Sql语句，会改变m_sLastErrMsg值
	* @param sql				Sql语句
	* @param handler			返回结果的回调handler
	* @return 是否成功
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
	* @brief 执行Sql文件中的语句
	* @param sFile				Sql脚本文件，必须以ascii编码
	* @param handler			返回结果的回调handler
	* @return 是否成功
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
	* @brief 开始请求查询
	* @param sql				Sql语句
	* @return 是否成功
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
	* @brief 格式化并开始请求查询
	* @param lpszSql			Sql语句
	* @param ...				Sql语句参数
	* @return 是否成功
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
	* @brief 完成请求查询
	* @param sql				Sql语句
	* @param handler			返回结果的回调handler
	* @return 是否成功
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
	* @brief 获取下一行数据
	* @return 是否成功
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
	* @brief 获取列个数
	* @return 列个数
	*/
	int GetColSize()
	{
		if (InitDll() && sqlcolcount && m_stmt)
			return sqlcolcount(m_stmt);
		return 0;
	}

	/**
	* @brief 获取列名称
	* @param i					第几列
	* @return 列名称
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
	* @brief 获取列值
	* @param col				第几列
	* @return 列值
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
	* @brief 获取列值
	* @param key				列名称
	* @return 列值
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
	* @brief 导出数据，该函数调用不会影响正在调用查询，会改变m_sLastErrMsg值
	* @param sSql				Sql语句
	* @param sSaveFile			导出到文件
	* @param sSepSym			数据分隔符
	* @param sItemSepSym		项目分隔符
	* @param bUseTitle			是否使用标题
	* @return 返回是否成功
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
	* @brief 导出数据，该函数调用不会影响正在调用查询，会改变m_sLastErrMsg值
	* @param sSql				Sql语句
	* @param bUseTitle			是否使用标题
	* @param pColCount			返回列数，可为空
	* @param pItemCount			返回行数，可为空
	* @param sLineS				行前加入内容，默认为<tr>，如需要空，请赋值_T("(empty)")
	* @param sLineE				行后加入内容，默认为</tr>，如需要空，请赋值_T("(empty)")
	* @param sItemS				数据项目前加入内容，默认为<td>，如需要空，请赋值_T("(empty)")
	* @param sItemE				数据项目后加入内容，默认为</td>，如需要空，请赋值_T("(empty)")
	* @param sTitleItemS		标题项目前加入内容，默认为<td><b>，如需要空，请赋值_T("(empty)")
	* @param sTitleItemE		标题项目后加入内容，默认为</b></td>，如需要空，请赋值_T("(empty)")
	* @param sHead				报表前加入的内容，默认为<table>，如需要空，请赋值_T("(empty)")
	* @param sEnd				报表后加入的内容，默认为</table>，如需要空，请赋值_T("(empty)")
	* @param sEmpty				空值替换内容，默认为&nbsp;，如需要空，请赋值_T("(empty)")
	* @return 返回报表字符串
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
	* @brief 导出文本数据，该函数调用不会影响正在调用查询，会改变m_sLastErrMsg值
	* @param sSql				Sql语句
	* @param bUseTitle			是否使用标题
	* @param pColCount			返回行数，可为空
	* @param pItemCount			返回列数，可为空
	* @return 返回报表字符串
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
	* @brief 获取sql语句的首行首列结果，该函数调用不会影响正在调用查询，会改变m_sLastErrMsg值
	* @param sSql				Sql语句
	* @param psRet				返回结果
	* @return 返回结果，如果出错返回空
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
	* @brief 获取sql语句的首行首列结果，该函数调用不会影响正在调用查询，会改变m_sLastErrMsg值
	* @param sSql				Sql语句
	* @return 返回结果，如果出错返回空
	*/
	THString GetFirstResult(THString sSql)
	{
		THString ret;
		GetFirstResult(sSql,&ret);
		return ret;
	}

	/**
	* @brief 格式化并获取sql语句的首行首列结果，该函数调用不会影响正在调用查询，会改变m_sLastErrMsg值
	* @param lpszSql		Sql语句
	* @return 返回结果，如果出错返回空
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
	* @brief 设置查询超时
	* @param nMs	毫秒数
	* @return 返回是否成功
	*/
	BOOL SetTimeOut(int nMs)
	{
		if (!InitDll()) return FALSE;
		if (!sqltimeout) return FALSE;
		if (!m_file) return FALSE;
		return (sqltimeout(m_file,nMs)==0);
	}

	/**
	* @brief 获取最后插入的rowid
	* @return 返回结果
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