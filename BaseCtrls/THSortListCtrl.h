#pragma once

class THSortListCtrl;

/**
* @brief �����б���ȽϺ����ص�
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-11-01 �½���
*/
class ITHSortListCompareCallBack
{
public:
	/**
	* @brief �����������������������������ʱ��Ӧ
	* @param pCls		�ؼ�ָ��
	* @param idx	����������
	* @param idx	����������
	* @param sTitle	��������
	*/
	virtual void OnSetItemText(THSortListCtrl *pCls,int idx,int subidx,THString sTitle){}
	/**
	* @brief ����ȽϺ���
	* @param pCls		�ؼ�ָ��
	* @param lParam1	������1
	* @param lParam2	������2
	* @param nSortIdx	�������Ŀ
	* @param bAcc		�Ƿ���������
	* @return ���رȽϽ��
	*/
	virtual int SortListCompare(THSortListCtrl *pCls,LPARAM lParam1,LPARAM lParam2,int nSortIdx,BOOL bAcc)=0;
};

/**
* @brief �����б�ͷ�Ի���
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-11-01 �½���
*/
class THSortHeaderCtrl : public CHeaderCtrl
{
public:
	THSortHeaderCtrl();
	virtual ~THSortHeaderCtrl();

	void SetBkColor(COLORREF cr);
	void SetSortArrow( const int iColumn, const BOOL bAscending );
	int m_iSortColumn;
	BOOL m_bSortAscending;
protected:	
	void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );
	COLORREF m_crBk;
	DECLARE_MESSAGE_MAP()

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

//����Ĭ��������
class THSortListCompareCallBack : public ITHSortListCompareCallBack
{
public:
	THSortListCompareCallBack();
	virtual ~THSortListCompareCallBack();

	virtual void OnSetItemText(THSortListCtrl *pCls,int idx,int subidx,THString sTitle);
	virtual int SortListCompare(THSortListCtrl *pCls,LPARAM lParam1,LPARAM lParam2,int nSortIdx,BOOL bAcc);
protected:
	THStringArray m_sSorting;
};

/**
* @brief �����б���
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-11-01 �½���
*/
/**<pre>
�÷���
	ʵ��ITHSortListCompareCallBack�ಢʹ�ú���SetCompareCallBack���ûص���Ӧ
	������ʱ������SetLparam�����������Ӧ��LPARAM����LPARAM�ڻص�ʱ����
	ע��SetItemData����ʹ�ã�LPARAM��ΪItemData
</pre>
*/
class THSortListCtrl : public CListCtrl
{
public:
	THSortListCtrl();
	virtual ~THSortListCtrl();

	int InsertColumn(int nCol, const LVCOLUMN* pColumn);
	int InsertColumn(int nCol, LPCTSTR lpszColumnHeading,int nFormat = LVCFMT_LEFT, int nWidth = -1, int nSubItem = -1);

	int InsertItem(const LVITEM* pItem);
	int InsertItem(int nItem, LPCTSTR lpszItem);
	int InsertItem(int nItem, LPCTSTR lpszItem, int nImage);
	int InsertItem(UINT nMask, int nItem, LPCTSTR lpszItem, UINT nState, UINT nStateMask, int nImage, LPARAM lParam);

	BOOL SetItem(int nItem, int nSubItem, UINT nMask, LPCTSTR lpszItem, int nImage, UINT nState, UINT nStateMask, LPARAM lParam);
	BOOL SetItem(const LVITEM* pItem);
	BOOL SetItem(int nItem, int nSubItem, UINT nMask, LPCTSTR lpszItem,	int nImage, UINT nState, UINT nStateMask, LPARAM lParam, int nIndent);
	BOOL SetItemText(int nItem, int nSubItem, LPCTSTR lpszText);

	/**
	* @brief ����LPARAM����������Ƚ�
	* @param idx	��ǰ������
	* @param lParam	�������ڱȽϵ�����
	*/
	void SetLparam(int idx,LPARAM lParam);
	/**
	* @brief ��ȡLPARAM����������Ƚ�
	* @param idx	��ǰ������
	* @return		�������ڱȽϵ�����
	*/
	LPARAM GetLparam(int idx);
	/**
	* @brief ����ָ�����򷽷�
	* @param iColumn	ָ�������������
	* @param bAscending	ָ���Ƿ�����
	*/
	void Sort(int iColumn,BOOL bAscending);
	/**
	* @brief ���ñ�������ɫ
	* @param cr			��ɫ
	*/
	void SetHeaderColor(COLORREF cr);
	/**
	* @brief ��������ص���Ӧ������������Ĭ��ʹ���ַ������бȽ�
	* @param pCB		�ص�����ʵ����
	*/
	void SetCompareCallBack(ITHSortListCompareCallBack *pCB);
protected:
	void UseSortHeader();
	THSortHeaderCtrl m_header;
	static int CALLBACK CompareFunction(LPARAM lParam1,LPARAM lParam2,LPARAM lParamData);
	int m_iSortColumn;
	BOOL m_bSortAscending;
	BOOL m_bInit;
	THSortListCompareCallBack m_defaultcb;
	ITHSortListCompareCallBack *m_pCB;
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnLvnColumnclick(NMHDR *pNMHDR, LRESULT *pResult);
};