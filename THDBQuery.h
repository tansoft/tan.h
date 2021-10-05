#pragma once

#include <THString.h>
#include <THStruct.h>
#include <THSyn.h>
#include <THDataBase.h>

class THDBQuery;

/**
* @brief ���ݿⱨ��ͨ�ô������ⲿ��������Ӧ��
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-10-28 �½���
*/
/**
,public ITHDBQueryExtServiceHandler
public:
	virtual BOOL OnAddEvent(THDBQuery *pCls,THString sql,int nSelection);
	virtual BOOL OnUpdateEvent(THDBQuery *pCls,THString sql,int nSelection);
	virtual BOOL OnDeleteEvent(THDBQuery *pCls,THString sql,int nSelection);
	virtual THString ConvertData(THDBQuery *pCls,int nRow,int nCol,THString sValue,BOOL bRead);
	virtual void GetColEnumState(THDBQuery *pCls,int nCol,THStringArray *ar);
*/
class ITHDBQueryExtServiceHandler
{
public:
	/**
	* @brief �������ݼ���Ӳ���
	* @param pCls		���ݼ�ָ��
	* @param sql		ִ�����
	* @param nSelection	��ǰѡ����
	* @return �����Ƿ�����
	*/
	virtual BOOL OnAddEvent(THDBQuery *pCls,THString sql,int nSelection){return TRUE;}
	/**
	* @brief �������ݼ����²���
	* @param pCls		���ݼ�ָ��
	* @param sql		ִ�����
	* @param nSelection	��ǰѡ����
	* @return �����Ƿ�����
	*/
	virtual BOOL OnUpdateEvent(THDBQuery *pCls,THString sql,int nSelection){return TRUE;}
	/**
	* @brief �������ݼ�ɾ������
	* @param pCls		���ݼ�ָ��
	* @param sql		ִ�����
	* @param nSelection	��ǰѡ����
	* @return �����Ƿ�����
	*/
	virtual BOOL OnDeleteEvent(THDBQuery *pCls,THString sql,int nSelection){return TRUE;}
	/**
	* @brief �������ݼ������ݵ�ת��
	* @param pCls		���ݼ�ָ��
	* @param nRow		ָ��ת����
	* @param nCol		ָ��ת����
	* @param sValue		ԭ����
	* @param bRead		ָ��д�����ݻ��Ƕ�������
	* @return ����ת�����ֵ
	*/
	virtual THString ConvertData(THDBQuery *pCls,int nRow,int nCol,THString sValue,BOOL bRead){return sValue;}
	/**
	* @brief ��ѯ���ݼ���ָ���е�ö�ٷ�Χ
	* @param pCls		���ݼ�ָ��
	* @param nCol		ָ����
	* @param ar			ö����Ϣ��������Ϊ��ָ��
	*/
	virtual void GetColEnumState(THDBQuery *pCls,int nCol,THStringArray *ar){}
};

/**
* @brief ���ݿⱨ��ͨ�ô�����Ӧ��
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-10-28 �½���
*/
class ITHDBQueryEventHandler
{
public:
	virtual void OnQueryDataFinish(THDBQuery *cls,int nRow,int nCol){}
	virtual void OnSelectionChange(THDBQuery *cls,int nOld,int nNew){}
	virtual void OnModifyChange(THDBQuery *cls,BOOL bModify){}
	//�Ƿ�������ĺͽ��иĶ����ύ
	virtual BOOL OnSaveModify(THDBQuery *cls,int idx){return TRUE;}
	//�Ƿ�����ɾ��
	virtual BOOL OnDeleteRecord(THDBQuery *cls,int idx){return TRUE;}
	//�ı�֮ǰ��֪ͨ
	virtual void OnPreSelectionChange(THDBQuery *cls,int nOld,int nNew){}
};

/**
* @brief ���ݿⱨ��ͨ�ô�����
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-10-28 �½���
*/
class THDBQuery
{
public:
	THDBQuery()
	{
		m_pDataBase=NULL;
		m_pRS=NULL;
		m_bReadOnly=FALSE;
		m_bModify=FALSE;
		m_nSelection=-1;
		m_contents=NULL;
		m_handler=NULL;
	}
	virtual ~THDBQuery()
	{
		m_notify.RemoveAll();
		EndQuery();
	}

	void SetDataBase(ITHDataBase *pDataBase){m_pDataBase=pDataBase;}
	void AddNotifyCb(ITHDBQueryEventHandler *handler){m_notifylock.Lock();m_notify.AddTail(handler);m_notifylock.Unlock();}
	void RemoveNotifyCb(ITHDBQueryEventHandler *handler){m_notifylock.Lock();m_notify.Remove(handler);m_notifylock.Unlock();}

	/**
	* @brief ����ʹ���ⲿ��������������
	* @param handler			������ָ��
	*/
	void SetHandler(ITHDBQueryExtServiceHandler *handler){m_handler=handler;}

	/**
	* @brief ��ʼ�µ����ݼ�����
	* @param sSql				ִ�е�Sql���
	* @param sDataBaseSettings	ָ�����ݿ�ĸ������ԣ�Ϊ��Ϊֻ��ģʽ���������������Զ�ת�������¡��޸ġ�ɾ���ȣ����磺�����ע��
	*/
	/**<pre>
		databasename|colname1,colname2,colname3|attrib1,attrib2,attrib3
		databasename��ָ�����ݿ�����
		��colname��Ϊ���ݼ���Ӧ�е����ݿ��ֶ���
		��attrib��Ϊ���ݼ���Ӧ�е����ͣ�dΪ���֣�sΪ�ַ���
	</pre>*/
	BOOL StartQuery(THString sSql,THString sDataBaseSettings=_T(""))
	{
		EndQuery();
		if (!m_pDataBase) return FALSE;
		BOOL bReadOnly=sDataBaseSettings.IsEmpty();
		m_bReadOnly=bReadOnly;
		m_pRS=m_pDataBase->NewRecordSet(bReadOnly);
		if (!m_pRS) return FALSE;
		if (!m_pRS->StartQuery(sSql))
		{
			EndQuery();
			return FALSE;
		}
		int colcnt=m_pRS->GetColCount();
		if (colcnt<=0)
		{
			EndQuery();
			return FALSE;
		}
		m_contents=new THStringArray[colcnt];
		if (!m_contents)
		{
			EndQuery();
			return FALSE;
		}
		for(int i=0;i<colcnt;i++)
			m_headers.Add(m_pRS->GetColName(i));
		while(!m_pRS->IsEOF())
		{
			for(int i=0;i<colcnt;i++)
				m_contents[i].Add(m_pRS->GetColValue(i));
			m_pRS->MoveNext();
		}
		if (!bReadOnly)
		{
			THStringToken t(sDataBaseSettings,_T("|"));
			m_DataBase=t.GetNextToken();
			THStringToken t1(t.GetNextToken(),_T(","));
			THStringToken t2(t.GetNextToken(),_T(","));
			while(t1.IsMoreTokens()) m_ColKeys.Add(t1.GetNextToken());
			while(t2.IsMoreTokens()) m_ColKeyTypes.Add(t2.GetNextToken());
		}
		OnNotify(1,(int)m_contents[0].GetSize(),colcnt);
		ChangeSelection(-1);
		return TRUE;
	}

	BOOL ReQuery()
	{
		if (!m_pRS) return FALSE;
		m_pRS->ReQuery();
		int colcnt=m_pRS->GetColCount();
		if (colcnt!=m_headers.GetSize()) return FALSE;
		for(int i=0;i<colcnt;i++)
			m_contents[i].RemoveAll();
		while(!m_pRS->IsEOF())
		{
			for(int i=0;i<colcnt;i++)
				m_contents[i].Add(m_pRS->GetColValue(i));
			m_pRS->MoveNext();
		}
		OnNotify(1,(int)m_contents[0].GetSize(),colcnt);
		ChangeSelection(-1);
		return TRUE;
	}

	BOOL EndQuery()
	{
		if (m_pRS)
		{
			m_pRS->EndQuery();
			m_pRS->ReleaseObject(m_pRS);
			m_pRS=NULL;
		}
		if (m_contents)
		{
			delete [] m_contents;
			m_contents=NULL;
		}
		m_headers.RemoveAll();
		m_SaveOper.Empty();
		m_SaveOper1.Empty();
		m_ColKeys.RemoveAll();
		m_ColKeyTypes.RemoveAll();
		m_DataBase.Empty();
		SetModifyFlag(FALSE);
		return TRUE;
	}

	BOOL ExecSql(THString sql)
	{
		if (m_pDataBase && !sql.IsEmpty())
			return m_pDataBase->ExecuteSql(sql);
		return FALSE;
	}

	//-1 δû��ѡ��
	void ChangeSelection(int idx)
	{
		OnNotify(6,m_nSelection,idx);
		int nOld=m_nSelection;
		m_nSelection=idx;
		OnNotify(2,nOld,m_nSelection);
	}
	int GetSelection(){return m_nSelection;}

	//void SetReadOnly(BOOL bReadOnly){m_bReadOnly=bReadOnly;}
	void SetModifyFlag(BOOL bModify,BOOL bAdd=FALSE)
	{
		if (m_bModify==bModify) return;
		//���������ӣ���ı�m_nSelectionΪ��ѡ��״̬�����Ҳ�����֪ͨ
		if (bModify && bAdd) m_nSelection=-1;
		m_bModify=bModify;
		OnNotify(3,m_bModify,0);
	}

	BOOL GetReadOnly(){return m_bReadOnly;}
	BOOL GetModifyFlag(){return m_bModify;}

	THString GetDataValue(THString val,THString type)
	{
		if (type==_T("s") || type==_T("t"))
			return _T("'")+val+_T("'");
		if (type==_T("d"))
			return THi2s(THs2i(val));
		if (type==_T("f"))
			return THf2s(THs2f(val));
		return val;
	}

	THString GetColValueType(int nCol)
	{
		THString ret;
		if (m_ColKeyTypes.GetSize()>nCol)
			ret=m_ColKeyTypes[nCol];
		if (ret.IsEmpty()) ret=_T("s");
		return ret;
	}

	virtual void SetContentChange(int nCol,THString str)
	{
		if (m_handler) str=m_handler->ConvertData(this,GetSelection(),nCol,str,FALSE);
		if (GetSelection()==-1)
		{
			//add
			if (m_ColKeys.GetSize()>nCol && m_ColKeyTypes.GetSize()>nCol)
			{
				THString tmp,tmp1;
				tmp=m_ColKeys[nCol];
				tmp1=GetDataValue(str,m_ColKeyTypes[nCol]);
				if (m_SaveOper.IsEmpty())
					m_SaveOper=tmp;
				else
					m_SaveOper+=_T(",")+tmp;
				if (m_SaveOper1.IsEmpty())
					m_SaveOper1=tmp1;
				else
					m_SaveOper1+=_T(",")+tmp1;
			}
		}
		else
		{
			//edit
			if (m_ColKeys.GetSize()>nCol && m_ColKeyTypes.GetSize()>nCol)
			{
				THString tmp;
				tmp.Format(_T("%s=%s"),m_ColKeys[nCol],GetDataValue(str,m_ColKeyTypes[nCol]));
				if (m_SaveOper.IsEmpty())
					m_SaveOper=tmp;
				else
					m_SaveOper+=_T(",")+tmp;
			}
			//SetContent(GetSelection(),ncol,str);
		}
	}
	virtual void SaveModify()
	{
		m_SaveOper.Empty();
		m_SaveOper1.Empty();
		//�����ѸĶ���ֵ�ռ�
		if (OnNotify(4,m_nSelection,0))
		{
			THString sql;
			if (GetSelection()==-1)
			{
				//add
				sql.Format(_T("insert into %s(%s) values(%s);"),m_DataBase,m_SaveOper,m_SaveOper1);
				if (m_handler && m_handler->OnAddEvent(this,sql,m_nSelection)==FALSE)
					sql.Empty();
			}
			else
			{
				//edit
				if (m_ColKeys.GetSize()>0 && m_ColKeyTypes.GetSize()>0)
				{
					sql.Format(_T("update %s set %s where %s=%s"),m_DataBase,m_SaveOper,m_ColKeys[0],GetDataValue(GetContent(GetSelection(),0),m_ColKeyTypes[0]));
					if (m_handler && m_handler->OnUpdateEvent(this,sql,m_nSelection)==FALSE)
						sql.Empty();
				}
			}
			if (!sql.IsEmpty() && m_pDataBase)
				m_pDataBase->ExecuteSql(sql);
			SetModifyFlag(FALSE);
			ReQuery();
			return;
		}
		//SetModifyFlag(FALSE);
	}
	virtual void DelRecord()
	{
		if (OnNotify(5,m_nSelection,0))
		{
			if (m_ColKeys.GetSize()>0 && m_ColKeyTypes.GetSize()>0 && m_pDataBase)
			{
				THString sql;
				sql.Format(_T("delete from %s where %s=%s"),m_DataBase,m_ColKeys[0],GetDataValue(GetContent(GetSelection(),0),m_ColKeyTypes[0]));
				if (m_handler && m_handler->OnDeleteEvent(this,sql,m_nSelection)==FALSE)
					sql.Empty();
				if (!sql.IsEmpty())
					m_pDataBase->ExecuteSql(sql);
			}
			SetModifyFlag(FALSE);
			ReQuery();
		}
	}
	virtual THString GetHeader(int idx)
	{
		if (idx>=m_headers.GetSize()) return _T("");
		return m_headers[idx];
	}
	virtual THString GetContent(int row,int col)
	{
		if (col>=m_headers.GetSize()) return _T("");
		if (row>=m_contents[0].GetSize()) return _T("");
		if (m_handler) return m_handler->ConvertData(this,row,col,m_contents[col][row],TRUE);
		return m_contents[col][row];
	}
	virtual THString GetContentByKey(int row,THString sTitle)
	{
		for(int i=0;i<m_headers.GetSize();i++)
			if (m_headers[i]==sTitle)
				return GetContent(row,i);
		ASSERT(FALSE);
		return _T("");
	}
	virtual THString GetValueByKey(int nCol,THString sValue,int nRetCol)
	{
		for(int i=0;i<GetRecordCount();i++)
		{
			if (GetContent(i,nCol)==sValue)
				return GetContent(i,nRetCol);
		}
		return _T("");
	}
	//ǿ���޸���Ŀ����
	virtual void SetContent(int row,int col,THString val)
	{
		if (col>=m_headers.GetSize()) return;
		if (row>=m_contents[0].GetSize()) return;
		m_contents[col][row]=val;
	}
	/**
	* @brief ��ȡ���������ö�����ݣ���������ѡ���
	* @param nCol		���ȡ����
	* @param ar			����ö�����ݵ����飬��û�У�������
	*/
	virtual void GetColEnumState(int nCol,THStringArray *ar)
	{
		if (m_handler) m_handler->GetColEnumState(this,nCol,ar);
	}
	/**
	* @brief ��ȡĳ���е��������ݣ�����һ�ַ���
	* @param nCol			���ȡ����
	* @param sSpliter		���ݼ�ķָ���
	* @param bFilterEmpty	�Ƿ���˵�Ϊ�յ�����
	* @return �����ַ���ֵ
	*/
	virtual THString GetAllColumnData(int nCol,THString sSpliter=_T(","),BOOL bFilterEmpty=TRUE)
	{
		THString ret;
		THString tmp;
		BOOL bFrist=TRUE;
		for(int i=0;i<GetRecordCount();i++)
		{
			tmp=GetContent(i,nCol);
			if (bFilterEmpty==FALSE || tmp.IsEmpty()==FALSE)
			{
				if (bFrist)
				{
					bFrist=FALSE;
					ret=tmp;
				}
				else
					ret+=_T(",")+tmp;
			}
		}
		return ret;
	}
	virtual int GetHeadersCount(){return (int)m_headers.GetSize();}
	virtual int GetRecordCount()
	{
		if (m_headers.GetSize()>0)
			return (int)m_contents[0].GetSize();
		return 0;
	}
protected:
	BOOL OnNotify(int notify,int val1,int val2)
	{
		BOOL ret=TRUE;
		ITHDBQueryEventHandler *handler;
		m_notifylock.Lock();
		THPosition pos=m_notify.GetStartPosition();
		while(!pos.IsEmpty())
		{
			if (m_notify.GetNextPosition(pos,handler) && handler)
			{
				if (notify==1)
					handler->OnQueryDataFinish(this,val1,val2);
				else if (notify==2)
					handler->OnSelectionChange(this,val1,val2);
				else if (notify==3)
					handler->OnModifyChange(this,val1);
				else if (notify==4)
				{
					if (!handler->OnSaveModify(this,val1))
					{
						ret=FALSE;
						break;
					}
				}
				else if (notify==5)
				{
					if (!handler->OnDeleteRecord(this,val1))
					{
						ret=FALSE;
						break;
					}
				}
				else if (notify==6)
					handler->OnPreSelectionChange(this,val1,val2);
			}
		}
		m_notifylock.Unlock();
		return ret;
	}
	ITHDataBase *m_pDataBase;
	ITHRecordSet *m_pRS;
	THStringArray m_headers;
	THStringArray *m_contents;
	ITHDBQueryExtServiceHandler *m_handler;
	BOOL m_bReadOnly,m_bModify;
	int m_nSelection;
	THList<ITHDBQueryEventHandler *> m_notify;
	THMutex m_notifylock;
	THString m_SaveOper,m_SaveOper1;
	THStringArray m_ColKeys;
	THStringArray m_ColKeyTypes;
	THString m_DataBase;
};
