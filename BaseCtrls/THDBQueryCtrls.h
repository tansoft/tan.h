#pragma once

#include <THDBQuery.h>
#include <THBitmap.h>
#include <BaseCtrls\THSortListCtrl.h>
// THDBQueryTreeCtrl

/**
* @brief 数据集树控件类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-11-01 新建类
*/
/**<pre>
用法：
	树控件对查询结果数据的使用格式定义如下：
	ID，TREEKEY，NAME，IMGID（可为空）
	ID：数据集中的标记记录的唯一标识
	TREEKEY：作为本记录的父结点的唯一标识，为0为位于根目录
	NAME：记录的名称
	IMGID：当有设定图像时，该值为图像索引值
</pre>*/
class THDBQueryTreeCtrl : public CTreeCtrl ,public ITHDBQueryEventHandler
{
	DECLARE_DYNAMIC(THDBQueryTreeCtrl)

public:
	THDBQueryTreeCtrl();
	virtual ~THDBQueryTreeCtrl();

	/**
	* @brief 初始化树控件
	* @param qdb	指定记录集指针
	* @param sRoot	指定根的名称，如果有指定图像，根节点的图像ID恒为0
	* @param bm		指定图像加载类，为空为不使用图像，类中不对加载类进行额外的加载和释放
	* @param crMask	指定图像掩码
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
* @brief 数据集下拉控件类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-11-01 新建类
*/
/**<pre>
用法：
	下拉控件用于枚举所有记录，通过选择来定位不同的记录，支持树模式的结构
	下拉控件对查询结果数据的使用格式定义如下：
	ID，TREEKEY，NAME		当指定数控件模式时，自动解释树层次并加上“->”
	ID，NAME				当指定普通模式时
	ID：数据集中的标记记录的唯一标识
	TREEKEY：作为本记录的父结点的唯一标识，为0为位于根
	NAME：记录的名称
</pre>*/
class THDBQueryComboBox : public CComboBox ,public ITHDBQueryEventHandler
{
	DECLARE_DYNAMIC(THDBQueryComboBox)

public:
	THDBQueryComboBox();
	virtual ~THDBQueryComboBox();

	/**
	* @brief 初始化下拉控件
	* @param qdb		指定记录集指针
	* @param bTreeMode	指定是否使用树模式
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
* @brief 数据集多选编辑下拉控件类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-11-09 新建类
*/
/**<pre>
用法：
	数据集多选编辑下拉控件枚举所有数据集中该列的数据作为备选项，通过选择来修改当前行的数据，选择通过叠加方式进行，实现多选
	数据集多选编辑下拉控件对查询结果数据的使用直接由Init函数指定
</pre>*/
class THDBQueryMultiSelectComboBox : public CComboBox ,public ITHDBQueryEventHandler
{
	DECLARE_DYNAMIC(THDBQueryMultiSelectComboBox)

public:
	THDBQueryMultiSelectComboBox();
	virtual ~THDBQueryMultiSelectComboBox();

	/**
	* @brief 初始化下拉控件
	* @param qdb			指定记录集指针
	* @param nCol			指定存取记录集的第几列数据
	* @param bNeedSelect	指定修改后是否必需有输入
	* @param bCanEdit		指定进入编辑模式时，该项是否允许编辑
	* @param sSpliter		指定数据分列的分隔符
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
* @brief 数据集编辑下拉控件类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-11-01 新建类
*/
/**<pre>
用法：
	下拉编辑控件对查询结果数据的使用直接由Init函数指定
</pre>*/
class THDBQueryEditComboBox : public CComboBox ,public ITHDBQueryEventHandler
{
	DECLARE_DYNAMIC(THDBQueryEditComboBox)

public:
	THDBQueryEditComboBox();
	virtual ~THDBQueryEditComboBox();

	/**
	* @brief 初始化下拉控件
	* @param qdb			指定记录集指针
	* @param nCol			指定存取记录集的第几列数据
	* @param bNeedSelect	指定修改后是否必需选择项
	* @param bReDrawTitle	指定是否每次更新记录集时都重画标题，重画造成标题原选择丢失
	* @param bCanEdit		指定进入编辑模式时，该项是否允许编辑
	* @param bUseDBEnum		指定是否使用数据集的获取枚举值函数来获取枚举值，不使用的话以该列所有值作为枚举值
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
* @brief 数据集编辑控件类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-11-01 新建类
*/
/**<pre>
用法：
	编辑控件对查询结果数据的使用直接由Init函数指定
</pre>*/
class THDBQueryEdit : public CEdit ,public ITHDBQueryEventHandler
{
	DECLARE_DYNAMIC(THDBQueryEdit)

public:
	THDBQueryEdit();
	virtual ~THDBQueryEdit();

	/**
	* @brief 初始化编辑控件
	* @param qdb			指定记录集指针
	* @param nCol			指定存取记录集的第几列数据
	* @param bCheckEmpty	指定是否需要在保存时检查是否为空
	* @param bSelectChange	指定是否在记录集改变当前项时更新显示的值
	* @param bCanEdit		指定进入编辑模式时，该项是否允许编辑
	* @param sDefValue		指定编辑框的默认值
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
* @brief 数据集显示控件类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-11-01 新建类
*/
/**<pre>
用法：
	显示控件对查询结果数据的使用直接由Init函数指定
</pre>*/
class THDBQueryStatic : public CStatic ,public ITHDBQueryEventHandler
{
	DECLARE_DYNAMIC(THDBQueryStatic)

public:
	THDBQueryStatic();
	virtual ~THDBQueryStatic();

	/**
	* @brief 初始化编辑控件
	* @param qdb			指定记录集指针
	* @param nCol			指定存取记录集的第几列数据
	* @param sPreString		指定显示的前缀
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
* @brief 数据集单选控件类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-11-01 新建类
*/
/**<pre>
用法：
	单选控件对查询结果数据的使用直接由Init函数指定
</pre>*/
class THDBQueryCheckBox : public CButton ,public ITHDBQueryEventHandler
{
	DECLARE_DYNAMIC(THDBQueryCheckBox)

public:
	THDBQueryCheckBox();
	virtual ~THDBQueryCheckBox();

	/**
	* @brief 初始化编辑控件
	* @param qdb			指定记录集指针
	* @param nCol			指定存取记录集的第几列数据
	* @param bSelectChange	指定是否在记录集改变当前项时更新显示的值
	* @param bCanEdit		指定进入编辑模式时，该项是否允许编辑
	* @param bDefValue		指定初始默认值
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
* @brief 数据集列表框类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-11-01 新建类
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
	* @brief 初始化列表框控件
	* @param qdb			指定记录集指针
	* @param nCol			指定存取记录集的第几列数据
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
* @brief 数据集列表控件类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-11-01 新建类
*/
/**<pre>
用法：
	列表控件对查询结果数据的使用格式定义如下：
	ID，IMGID（可为空），XXXX各列
	ID：数据集中的标记记录的唯一标识
	IMGID：当有设定图像时，该值为图像索引值
	XXXX：各列，具体列数由Init函数或记录集指定
</pre>*/
class THDBQueryListCtrl : public THSortListCtrl ,public ITHDBQueryEventHandler,public ITHSortListCompareCallBack
{
	DECLARE_DYNAMIC(THDBQueryListCtrl)
public:
	THDBQueryListCtrl();
	virtual ~THDBQueryListCtrl();

	/**
	* @brief 初始化列表控件
	* @param qdb			指定记录集指针
	* @param sWidth			指定列表中各列的宽度，由分隔符分开，如：20|30|200|100，没有指定时使用默认值100
	* @param nCol			指定列表列数，为-1为不指定，根据记录集返回结果处理
	* @param bReDrawTitle	指定是否每次更新记录集时都重画标题，重画造成标题的记忆宽度改变
	* @param bm				指定图像加载类，为空为不使用图像，类中不对加载类进行额外的加载和释放
	* @param crMask			指定图像掩码
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
* @brief 数据集编辑面板控件类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-11-01 新建类
*/
/**<pre>
用法：
	编辑面板不使用查询结果的具体数据
</pre>*/
class THDBQueryEditPanel : public CStatic ,public ITHDBQueryEventHandler
{
	DECLARE_DYNAMIC(THDBQueryEditPanel)
public:
	THDBQueryEditPanel();
	virtual ~THDBQueryEditPanel();

	/**
	* @brief 初始化树控件
	* @param qdb		指定记录集指针
	* @param InitType	指定按键是否能使用
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
* @brief 数据集日期控件类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-11-01 新建类
*/
/**<pre>
用法：
	编辑控件对查询结果数据的使用直接由Init函数指定
</pre>*/
class THDBQueryDateTimeCtrl : public CDateTimeCtrl ,public ITHDBQueryEventHandler
{
	DECLARE_DYNAMIC(THDBQueryDateTimeCtrl)
public:
	THDBQueryDateTimeCtrl();
	virtual ~THDBQueryDateTimeCtrl();

	/**
	* @brief 初始化编辑控件
	* @param qdb			指定记录集指针
	* @param nCol			指定存取记录集的第几列数据
	* @param sFormat		指定日期字符串格式，用于日期字符串与日期控件转换，格式参照THTime::Format和THTime::SetTime(sStr,sFormat)
	* @param bSelectChange	指定是否在记录集改变当前项时更新显示的值
	* @param bCanEdit		指定进入编辑模式时，该项是否允许编辑
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
