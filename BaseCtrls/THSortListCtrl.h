#pragma once

class THSortListCtrl;

/**
* @brief 排序列表类比较函数回调
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-11-01 新建类
*/
class ITHSortListCompareCallBack
{
public:
	/**
	* @brief 当插入新数据项或设置数据项内容时响应
	* @param pCls		控件指针
	* @param idx	数据项索引
	* @param idx	数据列索引
	* @param sTitle	标题内容
	*/
	virtual void OnSetItemText(THSortListCtrl *pCls,int idx,int subidx,THString sTitle){}
	/**
	* @brief 排序比较函数
	* @param pCls		控件指针
	* @param lParam1	数据项1
	* @param lParam2	数据项2
	* @param nSortIdx	排序的项目
	* @param bAcc		是否升序排列
	* @return 返回比较结果
	*/
	virtual int SortListCompare(THSortListCtrl *pCls,LPARAM lParam1,LPARAM lParam2,int nSortIdx,BOOL bAcc)=0;
};

/**
* @brief 排序列表头自绘类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-11-01 新建类
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

//内置默认排序类
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
* @brief 排序列表类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-11-01 新建类
*/
/**<pre>
用法：
	实现ITHSortListCompareCallBack类并使用函数SetCompareCallBack设置回调响应
	插入项时，调用SetLparam函数设置项对应的LPARAM，该LPARAM在回调时传回
	注意SetItemData不能使用，LPARAM即为ItemData
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
	* @brief 设置LPARAM，用于排序比较
	* @param idx	当前项索引
	* @param lParam	传入用于比较的数据
	*/
	void SetLparam(int idx,LPARAM lParam);
	/**
	* @brief 获取LPARAM，用于排序比较
	* @param idx	当前项索引
	* @return		返回用于比较的数据
	*/
	LPARAM GetLparam(int idx);
	/**
	* @brief 设置指定排序方法
	* @param iColumn	指定用于排序的列
	* @param bAscending	指定是否升序
	*/
	void Sort(int iColumn,BOOL bAscending);
	/**
	* @brief 设置标题栏底色
	* @param cr			颜色
	*/
	void SetHeaderColor(COLORREF cr);
	/**
	* @brief 设置排序回调响应函数，不设置默认使用字符串进行比较
	* @param pCB		回调函数实现类
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