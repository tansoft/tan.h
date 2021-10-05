#pragma once

#include <THDBQuery.h>
#include <THBitmap.h>
#include <BaseCtrls\THSortListCtrl.h>
// THDBQueryTreeCtrl

/**
* @brief ���ݼ����ؼ���
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-11-01 �½���
*/
/**<pre>
�÷���
	���ؼ��Բ�ѯ������ݵ�ʹ�ø�ʽ�������£�
	ID��TREEKEY��NAME��IMGID����Ϊ�գ�
	ID�����ݼ��еı�Ǽ�¼��Ψһ��ʶ
	TREEKEY����Ϊ����¼�ĸ�����Ψһ��ʶ��Ϊ0Ϊλ�ڸ�Ŀ¼
	NAME����¼������
	IMGID�������趨ͼ��ʱ����ֵΪͼ������ֵ
</pre>*/
class THDBQueryTreeCtrl : public CTreeCtrl ,public ITHDBQueryEventHandler
{
	DECLARE_DYNAMIC(THDBQueryTreeCtrl)

public:
	THDBQueryTreeCtrl();
	virtual ~THDBQueryTreeCtrl();

	/**
	* @brief ��ʼ�����ؼ�
	* @param qdb	ָ����¼��ָ��
	* @param sRoot	ָ���������ƣ������ָ��ͼ�񣬸��ڵ��ͼ��ID��Ϊ0
	* @param bm		ָ��ͼ������࣬Ϊ��Ϊ��ʹ��ͼ�����в��Լ�������ж���ļ��غ��ͷ�
	* @param crMask	ָ��ͼ������
	*/
	void Init(THDBQuery *qdb,THString sRoot,THBitmap *bm=NULL,COLORREF crMask=RGB(255,0,255));
	void UnInit();

public:
	virtual void OnQueryDataFinish(THDBQuery *cls,int nRow,int nCol);
	virtual void OnSelectionChange(THDBQuery *cls,int nOld,int nNew);
	virtual void OnModifyChange(THDBQuery *cls,BOOL bModify);
	virtual BOOL OnSaveModify(THDBQuery *cls,int idx);
	virtual BOOL OnDeleteRecord(THDBQuery *cls,int idx);

protected:
	DECLARE_MESSAGE_MAP()

	HTREEITEM FindItemByData(int data,HTREEITEM hParent);
	THDBQuery *m_qdb;
	HTREEITEM m_hRoot;
	CImageList m_list;
	BOOL m_bUseImage;
public:
	afx_msg void OnTvnSelchanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDestroy();
};

/**
* @brief ���ݼ������ؼ���
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-11-01 �½���
*/
/**<pre>
�÷���
	�����ؼ�����ö�����м�¼��ͨ��ѡ������λ��ͬ�ļ�¼��֧����ģʽ�Ľṹ
	�����ؼ��Բ�ѯ������ݵ�ʹ�ø�ʽ�������£�
	ID��TREEKEY��NAME		��ָ�����ؼ�ģʽʱ���Զ���������β����ϡ�->��
	ID��NAME				��ָ����ͨģʽʱ
	ID�����ݼ��еı�Ǽ�¼��Ψһ��ʶ
	TREEKEY����Ϊ����¼�ĸ�����Ψһ��ʶ��Ϊ0Ϊλ�ڸ�
	NAME����¼������
</pre>*/
class THDBQueryComboBox : public CComboBox ,public ITHDBQueryEventHandler
{
	DECLARE_DYNAMIC(THDBQueryComboBox)

public:
	THDBQueryComboBox();
	virtual ~THDBQueryComboBox();

	/**
	* @brief ��ʼ�������ؼ�
	* @param qdb		ָ����¼��ָ��
	* @param bTreeMode	ָ���Ƿ�ʹ����ģʽ
	*/
	void Init(THDBQuery *qdb,BOOL bTreeMode=FALSE);
	void UnInit();

public:
	virtual void OnQueryDataFinish(THDBQuery *cls,int nRow,int nCol);

protected:
	DECLARE_MESSAGE_MAP()

	THDBQuery *m_qdb;
	BOOL m_bTreeMode;
	THString FindParentString(int parent);
public:
	afx_msg void OnCbnSelchange();
	afx_msg void OnDestroy();
};

#define WM_CHANGETEXTMSG	WM_USER+1635

/**
* @brief ���ݼ���ѡ�༭�����ؼ���
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-11-09 �½���
*/
/**<pre>
�÷���
	���ݼ���ѡ�༭�����ؼ�ö���������ݼ��и��е�������Ϊ��ѡ�ͨ��ѡ�����޸ĵ�ǰ�е����ݣ�ѡ��ͨ�����ӷ�ʽ���У�ʵ�ֶ�ѡ
	���ݼ���ѡ�༭�����ؼ��Բ�ѯ������ݵ�ʹ��ֱ����Init����ָ��
</pre>*/
class THDBQueryMultiSelectComboBox : public CComboBox ,public ITHDBQueryEventHandler
{
	DECLARE_DYNAMIC(THDBQueryMultiSelectComboBox)

public:
	THDBQueryMultiSelectComboBox();
	virtual ~THDBQueryMultiSelectComboBox();

	/**
	* @brief ��ʼ�������ؼ�
	* @param qdb			ָ����¼��ָ��
	* @param nCol			ָ����ȡ��¼���ĵڼ�������
	* @param bNeedSelect	ָ���޸ĺ��Ƿ����������
	* @param bCanEdit		ָ������༭ģʽʱ�������Ƿ�����༭
	* @param sSpliter		ָ�����ݷ��еķָ���
	*/
	void Init(THDBQuery *qdb,int nCol=0,BOOL bNeedSelect=FALSE,BOOL bCanEdit=TRUE,THString sSpliter=_T(","));
	void UnInit();
	void ReLoadEnum();
public:
	virtual void OnQueryDataFinish(THDBQuery *cls,int nRow,int nCol);
	virtual void OnModifyChange(THDBQuery *cls,BOOL bModify);
	virtual BOOL OnSaveModify(THDBQuery *cls,int idx);
	virtual void OnSelectionChange(THDBQuery *cls,int nOld,int nNew);
	void SetSelectText(THString oldtext,THString addtext);
protected:
	DECLARE_MESSAGE_MAP()

	THDBQuery *m_qdb;
	int m_nCol;
	BOOL m_bNeedSelect;
	BOOL m_bCanEdit;
	THStringArray m_sEnumArray;
	THString m_select;
	THString m_sSpliter;
public:
	afx_msg void OnDestroy();
	afx_msg void OnCbnCloseup();
	afx_msg void OnCbnDropdown();
	afx_msg LRESULT OnChangeTextMessage(WPARAM wParam,LPARAM lParam);
};

/**
* @brief ���ݼ��༭�����ؼ���
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-11-01 �½���
*/
/**<pre>
�÷���
	�����༭�ؼ��Բ�ѯ������ݵ�ʹ��ֱ����Init����ָ��
</pre>*/
class THDBQueryEditComboBox : public CComboBox ,public ITHDBQueryEventHandler
{
	DECLARE_DYNAMIC(THDBQueryEditComboBox)

public:
	THDBQueryEditComboBox();
	virtual ~THDBQueryEditComboBox();

	/**
	* @brief ��ʼ�������ؼ�
	* @param qdb			ָ����¼��ָ��
	* @param nCol			ָ����ȡ��¼���ĵڼ�������
	* @param bNeedSelect	ָ���޸ĺ��Ƿ����ѡ����
	* @param bReDrawTitle	ָ���Ƿ�ÿ�θ��¼�¼��ʱ���ػ����⣬�ػ���ɱ���ԭѡ��ʧ
	* @param bCanEdit		ָ������༭ģʽʱ�������Ƿ�����༭
	* @param bUseDBEnum		ָ���Ƿ�ʹ�����ݼ��Ļ�ȡö��ֵ��������ȡö��ֵ����ʹ�õĻ��Ը�������ֵ��Ϊö��ֵ
	*/
	void Init(THDBQuery *qdb,int nCol,BOOL bNeedSelect=FALSE,BOOL bReDrawTitle=FALSE,BOOL bCanEdit=TRUE,BOOL bUseDBEnum=TRUE);
	void UnInit();
	void ReLoadEnum();
public:
	virtual void OnQueryDataFinish(THDBQuery *cls,int nRow,int nCol);
	virtual void OnModifyChange(THDBQuery *cls,BOOL bModify);
	virtual BOOL OnSaveModify(THDBQuery *cls,int idx);
	virtual void OnSelectionChange(THDBQuery *cls,int nOld,int nNew);
protected:
	DECLARE_MESSAGE_MAP()

	THDBQuery *m_qdb;
	int m_nCol;
	BOOL m_bNeedSelect;
	BOOL m_bReDrawTitle;
	BOOL m_bInit;
	BOOL m_bCanEdit;
	BOOL m_bUseDBEnum;
	THStringArray m_sEnumArray;
public:
	afx_msg void OnDestroy();
};

/**
* @brief ���ݼ��༭�ؼ���
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-11-01 �½���
*/
/**<pre>
�÷���
	�༭�ؼ��Բ�ѯ������ݵ�ʹ��ֱ����Init����ָ��
</pre>*/
class THDBQueryEdit : public CEdit ,public ITHDBQueryEventHandler
{
	DECLARE_DYNAMIC(THDBQueryEdit)

public:
	THDBQueryEdit();
	virtual ~THDBQueryEdit();

	/**
	* @brief ��ʼ���༭�ؼ�
	* @param qdb			ָ����¼��ָ��
	* @param nCol			ָ����ȡ��¼���ĵڼ�������
	* @param bCheckEmpty	ָ���Ƿ���Ҫ�ڱ���ʱ����Ƿ�Ϊ��
	* @param bSelectChange	ָ���Ƿ��ڼ�¼���ı䵱ǰ��ʱ������ʾ��ֵ
	* @param bCanEdit		ָ������༭ģʽʱ�������Ƿ�����༭
	* @param sDefValue		ָ���༭���Ĭ��ֵ
	*/
	void Init(THDBQuery *qdb,int nCol,BOOL bCheckEmpty=TRUE,BOOL bSelectChange=FALSE,BOOL bCanEdit=TRUE,THString sDefValue=_T(""));
	void UnInit();
	void SetCanEdit(BOOL bCanEdit);

public:
	virtual void OnQueryDataFinish(THDBQuery *cls,int nRow,int nCol);
	virtual void OnModifyChange(THDBQuery *cls,BOOL bModify);
	virtual BOOL OnSaveModify(THDBQuery *cls,int idx);
	virtual void OnSelectionChange(THDBQuery *cls,int nOld,int nNew);
protected:
	DECLARE_MESSAGE_MAP()

	THDBQuery *m_qdb;
	int m_col;
	BOOL m_bCheckEmpty;
	BOOL m_bSelectChange;
	BOOL m_bCanEdit;
	THString m_sDefValue;
public:
	afx_msg void OnDestroy();
};

/**
* @brief ���ݼ���ʾ�ؼ���
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-11-01 �½���
*/
/**<pre>
�÷���
	��ʾ�ؼ��Բ�ѯ������ݵ�ʹ��ֱ����Init����ָ��
</pre>*/
class THDBQueryStatic : public CStatic ,public ITHDBQueryEventHandler
{
	DECLARE_DYNAMIC(THDBQueryStatic)

public:
	THDBQueryStatic();
	virtual ~THDBQueryStatic();

	/**
	* @brief ��ʼ���༭�ؼ�
	* @param qdb			ָ����¼��ָ��
	* @param nCol			ָ����ȡ��¼���ĵڼ�������
	* @param sPreString		ָ����ʾ��ǰ׺
	*/
	void Init(THDBQuery *qdb,int nCol,THString sPreString=_T(""));
	void UnInit();

public:
	virtual void OnSelectionChange(THDBQuery *cls,int nOld,int nNew);
	virtual void OnModifyChange(THDBQuery *cls,BOOL bModify);

protected:
	DECLARE_MESSAGE_MAP()

	THDBQuery *m_qdb;
	int m_nCol;
	THString m_sPreString;
public:
	afx_msg void OnDestroy();
};

/**
* @brief ���ݼ���ѡ�ؼ���
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-11-01 �½���
*/
/**<pre>
�÷���
	��ѡ�ؼ��Բ�ѯ������ݵ�ʹ��ֱ����Init����ָ��
</pre>*/
class THDBQueryCheckBox : public CButton ,public ITHDBQueryEventHandler
{
	DECLARE_DYNAMIC(THDBQueryCheckBox)

public:
	THDBQueryCheckBox();
	virtual ~THDBQueryCheckBox();

	/**
	* @brief ��ʼ���༭�ؼ�
	* @param qdb			ָ����¼��ָ��
	* @param nCol			ָ����ȡ��¼���ĵڼ�������
	* @param bSelectChange	ָ���Ƿ��ڼ�¼���ı䵱ǰ��ʱ������ʾ��ֵ
	* @param bCanEdit		ָ������༭ģʽʱ�������Ƿ�����༭
	* @param bDefValue		ָ����ʼĬ��ֵ
	*/
	void Init(THDBQuery *qdb,int nCol,BOOL bSelectChange=FALSE,BOOL bCanEdit=TRUE,BOOL bDefValue=FALSE);
	void UnInit();

public:
	virtual void OnQueryDataFinish(THDBQuery *cls,int nRow,int nCol);
	virtual void OnModifyChange(THDBQuery *cls,BOOL bModify);
	virtual BOOL OnSaveModify(THDBQuery *cls,int idx);
	virtual void OnSelectionChange(THDBQuery *cls,int nOld,int nNew);
protected:
	DECLARE_MESSAGE_MAP()

	THDBQuery *m_qdb;
	int m_col;
	BOOL m_bSelectChange;
	BOOL m_bCanEdit;
	BOOL m_bDefValue;
public:
	afx_msg void OnDestroy();
};

/**
* @brief ���ݼ��б����
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-11-01 �½���
*/
/**<pre>
</pre>*/
class THDBQueryListBox : public CListBox ,public ITHDBQueryEventHandler
{
	DECLARE_DYNAMIC(THDBQueryListBox)
public:
	THDBQueryListBox();
	virtual ~THDBQueryListBox();

	/**
	* @brief ��ʼ���б��ؼ�
	* @param qdb			ָ����¼��ָ��
	* @param nCol			ָ����ȡ��¼���ĵڼ�������
	*/
	void Init(THDBQuery *qdb,int nCol=0);
	void UnInit();

public:
	virtual void OnQueryDataFinish(THDBQuery *cls,int nRow,int nCol);
protected:
	DECLARE_MESSAGE_MAP()

	THDBQuery *m_qdb;
	int m_nCol;
public:
	afx_msg void OnDestroy();
	afx_msg void OnLbnSelchange();
};

/**
* @brief ���ݼ��б�ؼ���
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-11-01 �½���
*/
/**<pre>
�÷���
	�б�ؼ��Բ�ѯ������ݵ�ʹ�ø�ʽ�������£�
	ID��IMGID����Ϊ�գ���XXXX����
	ID�����ݼ��еı�Ǽ�¼��Ψһ��ʶ
	IMGID�������趨ͼ��ʱ����ֵΪͼ������ֵ
	XXXX�����У�����������Init�������¼��ָ��
</pre>*/
class THDBQueryListCtrl : public THSortListCtrl ,public ITHDBQueryEventHandler,public ITHSortListCompareCallBack
{
	DECLARE_DYNAMIC(THDBQueryListCtrl)
public:
	THDBQueryListCtrl();
	virtual ~THDBQueryListCtrl();

	/**
	* @brief ��ʼ���б�ؼ�
	* @param qdb			ָ����¼��ָ��
	* @param sWidth			ָ���б��и��еĿ�ȣ��ɷָ����ֿ����磺20|30|200|100��û��ָ��ʱʹ��Ĭ��ֵ100
	* @param nCol			ָ���б�������Ϊ-1Ϊ��ָ�������ݼ�¼�����ؽ������
	* @param bReDrawTitle	ָ���Ƿ�ÿ�θ��¼�¼��ʱ���ػ����⣬�ػ���ɱ���ļ����ȸı�
	* @param bm				ָ��ͼ������࣬Ϊ��Ϊ��ʹ��ͼ�����в��Լ�������ж���ļ��غ��ͷ�
	* @param crMask			ָ��ͼ������
	*/
	void Init(THDBQuery *qdb,THString sWidth=_T(""),int nCol=-1,BOOL bReDrawTitle=FALSE,THBitmap *bm=NULL,COLORREF crMask=RGB(255,0,255));
	void UnInit();
	int SortListCompare(THSortListCtrl *pCls,LPARAM lParam1,LPARAM lParam2,int nSortIdx,BOOL bAcc);
	BOOL ExportToCsv(THString sPath,BOOL bExportHeader);

public:
	virtual void OnQueryDataFinish(THDBQuery *cls,int nRow,int nCol);
	virtual void OnSelectionChange(THDBQuery *cls,int nOld,int nNew);
	virtual void OnModifyChange(THDBQuery *cls,BOOL bModify);
	virtual BOOL OnSaveModify(THDBQuery *cls,int idx);
	virtual BOOL OnDeleteRecord(THDBQuery *cls,int idx);
protected:
	DECLARE_MESSAGE_MAP()

	THDBQuery *m_qdb;
	CImageList m_list;
	BOOL m_bUseImage;
	BOOL m_bReDrawTitle;
	BOOL m_bInit;
	int m_nCol;
	THList<int> m_Width;
public:
	afx_msg void OnDestroy();
	afx_msg void OnLvnItemchanged(NMHDR *pNMHDR, LRESULT *pResult);
};

#define DBQUERYEDITPANEL_ADD	0x1
#define DBQUERYEDITPANEL_EDIT	0x2
#define DBQUERYEDITPANEL_DELETE 0x4
#define DBQUERYEDITPANEL_ALL	DBQUERYEDITPANEL_ADD|DBQUERYEDITPANEL_EDIT|DBQUERYEDITPANEL_DELETE

/**
* @brief ���ݼ��༭���ؼ���
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-11-01 �½���
*/
/**<pre>
�÷���
	�༭��岻ʹ�ò�ѯ����ľ�������
</pre>*/
class THDBQueryEditPanel : public CStatic ,public ITHDBQueryEventHandler
{
	DECLARE_DYNAMIC(THDBQueryEditPanel)
public:
	THDBQueryEditPanel();
	virtual ~THDBQueryEditPanel();

	/**
	* @brief ��ʼ�����ؼ�
	* @param qdb		ָ����¼��ָ��
	* @param InitType	ָ�������Ƿ���ʹ��
	*/
	void Init(THDBQuery *qdb,int InitType=DBQUERYEDITPANEL_ALL);
	void UnInit();

	void SetButtonState(int StateType);
	void SetDeleteNotifyText(THString text);

	void OnQueryDataFinish(THDBQuery *cls,int nRow,int nCol);
	void OnPreSelectionChange(THDBQuery *cls,int nOld,int nNew);
	void OnSelectionChange(THDBQuery *cls,int nOld,int nNew);
	void OnModifyChange(THDBQuery *cls,BOOL bModify);

protected:
	void OnUpdateButtons();
	THDBQuery *m_pd;
	int m_InitType;
	THString m_deltext;

	DECLARE_MESSAGE_MAP()
public:
	CButton m_btn[5];
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButton5();
	afx_msg void OnDestroy();
};

#include <afxdtctl.h>
/**
* @brief ���ݼ����ڿؼ���
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-11-01 �½���
*/
/**<pre>
�÷���
	�༭�ؼ��Բ�ѯ������ݵ�ʹ��ֱ����Init����ָ��
</pre>*/
class THDBQueryDateTimeCtrl : public CDateTimeCtrl ,public ITHDBQueryEventHandler
{
	DECLARE_DYNAMIC(THDBQueryDateTimeCtrl)
public:
	THDBQueryDateTimeCtrl();
	virtual ~THDBQueryDateTimeCtrl();

	/**
	* @brief ��ʼ���༭�ؼ�
	* @param qdb			ָ����¼��ָ��
	* @param nCol			ָ����ȡ��¼���ĵڼ�������
	* @param sFormat		ָ�������ַ�����ʽ�����������ַ��������ڿؼ�ת������ʽ����THTime::Format��THTime::SetTime(sStr,sFormat)
	* @param bSelectChange	ָ���Ƿ��ڼ�¼���ı䵱ǰ��ʱ������ʾ��ֵ
	* @param bCanEdit		ָ������༭ģʽʱ�������Ƿ�����༭
	*/
	void Init(THDBQuery *qdb,int nCol,THString sFormat=_T("%Y-%m-%d"),BOOL bSelectChange=FALSE,BOOL bCanEdit=TRUE);
	void UnInit();

public:
	virtual void OnQueryDataFinish(THDBQuery *cls,int nRow,int nCol);
	virtual void OnModifyChange(THDBQuery *cls,BOOL bModify);
	virtual BOOL OnSaveModify(THDBQuery *cls,int idx);
	virtual void OnSelectionChange(THDBQuery *cls,int nOld,int nNew);
protected:
	DECLARE_MESSAGE_MAP()

	THDBQuery *m_qdb;
	int m_col;
	THString m_sFormat;
	BOOL m_bSelectChange;
	BOOL m_bCanEdit;
public:
	afx_msg void OnDestroy();
};
