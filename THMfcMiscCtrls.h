#pragma once
#include <THLibMfcCtrls.h>
#include <THBitmap.h>
#include <BaseCtrls\THSkinButton.h>
#include <BaseCtrls\THSkinStatic.h>
#include <BaseCtrls\THSkinButtonStatic.h>
#include <BaseCtrls\THDBQueryCtrls.h>
#include <BaseCtrls\THSortListCtrl.h>

/**
* @brief 加强ComboBox
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-06-04 新建类
*/
/**<pre>
使用Sample：
	THComboBox m_combo;
	m_combo.ReadSettings(THString settings,THString defset);
</pre>*/
class THComboBox : public CComboBox
{
public:
	void SetSettings(THString settings,THString defset)
	{
		ResetContent();
		THString resToken;
		int curPos= 0;
		while (curPos!=-1)
		{
			resToken=settings.Tokenize(_T(";"),curPos);
			if (!resToken.IsEmpty())
				AddString(resToken);
		}
		for(int i=0;i<GetCount();i++)
		{
			GetLBText(i,resToken);
			if (resToken==defset)
			{
				SetCurSel(i);
				return;
			}
		}
		SetWindowText(defset);
	}
	THString GetSettings()
	{
		THString str;
		GetWindowText(str);
		BOOL bFound=FALSE;
		THString out;
		THString tmp;
		for(int i=0;i<GetCount();i++)
		{
			GetLBText(i,tmp);
			if (tmp==str)
				bFound=TRUE;
			out+=tmp+_T(";");
		}
		if (!bFound)
		{
			out+=str+_T(";");
		}
		return out;
	}
};

/**
* @brief 加强TreeCtrl
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-06-04 新建类
*/
/**<pre>
使用Sample：
</pre>*/
class THTreeCtrl : public CTreeCtrl
{
public:
	/**
	* @brief 设置图片列表
	* @param bitmap			图片对象
	* @param crMask			图片掩码色
	* @param nListType		图片列表类型:TVSIL_NORMAL,TVSIL_STATE
	* @return 是否成功
	*/
	BOOL SetBitmapList(THBitmap bitmap,COLORREF crMask,int nListType=TVSIL_NORMAL)
	{
		if (bitmap.IsEmpty()) return FALSE;
		int x=0,y=0;
		bitmap.GetBmpSize(x,y);
		if (x==0 || y==0) return FALSE;
		m_list.DeleteImageList();
		if (!m_list.Create(y,y,ILC_COLOR32|ILC_MASK,x/y,5)) return FALSE;
		//不使用ImageList的Add，不用做一个CBitmap出来
		ImageList_AddMasked(m_list,bitmap,crMask);
		SetImageList(&m_list,nListType);
		return TRUE;
	}
private:
	CImageList m_list;
};