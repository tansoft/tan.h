#pragma once

#include "THBitmap.h"

/**
* @brief 自绘按钮类，支持自绘图片，非规则矩形
* @author Barry
*/
/**<pre>
  使用Sample：
	THSkinButton m_btn1;
	THBitmap bmp("c:\\a.bmp");
	THBitmap bmp1(IDB_BITMAP1);
	m_btn1.SetBitmap(bmp,0,0,16,16,FALSE);
	m_btn1.SetBitmap(bmp1,0,0,16,16,FALSE,TRUE,RGB(192,192,192));
	m_btn1.SetBitmap(hBitmap,0,0,16,16,TRUE,FALSE,RGB(192,192,192),FALSE,RGB(0,0,255));
	m_btn1.SetBtnIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME));
	m_btn1.SetBtnCursor(AfxGetApp()->LoadCursor(IDC_CURSOR1));
	m_btn1.SetShowText(THSkinButton::ButtonTextAlignCenter);
</pre>*/
class THSkinButton : public CButton
{
	DECLARE_DYNAMIC(THSkinButton)
public:
	THSkinButton();
	virtual ~THSkinButton();
	/** 文本排列方式定义 */
	typedef enum{
		ButtonTextAlignNone,		///<不显示文本
		ButtonTextAlignLeft,		///<文本居左排列
		ButtonTextAlignCenter,		///<文本居中排列
		ButtonTextAlignRight		///<文本居右排列
	}ButtonTextAlign;
protected:
	UINT		m_nTimer;
	HICON		m_hIcon;
	HCURSOR		m_hCursor;
	HBITMAP		m_hBmp;
	BITMAP		m_bmp;
	COLORREF	m_clrTransColor;
	BOOL		m_bTransparent;
	int			m_nLeft,m_nTop,m_nWidth,m_nHeight;
	BOOL		m_bMouseIn;
	BOOL		m_bMouseDown;
	CPoint		m_prePos;
	int			m_nTypeRadio;
	COLORREF	m_crText;
	ButtonTextAlign	m_TextAlign;
	CRgn		m_rgnTotal;
	THList<long>m_rtEmpty;
	THBitmap	*m_pbitmap;
	double		m_mask;
	HFONT		m_hFont;
public:
	/**
	* @brief 设置按钮模式为可维持按下模式
	* @param bPress 设置默认按下状态
	*/
	void SetRadioMode(BOOL bPress=FALSE);
	/** @brief 设置按钮模式为正常模式，按钮默认模式为正常模式*/
	void SetNormalMode(){m_nTypeRadio=0;Invalidate();}
	/**
	* @brief 设置按钮使用的背景图片，图片以y轴方式依次为 正常，鼠标经过，鼠标按下，禁止状态（如果没有禁止可不提供禁止图片）
	* @param hBitmap		图片位图句柄，可以使用图片处理类THBitmap来获得
	* @param left			按钮使用背景在图片中的起始x
	* @param top			按钮使用背景在图片中的起始y
	* @param width			按钮使用背景宽
	* @param height			按钮使用背景高
	* @param bAutoExpand	是否根据上面的宽高设定按钮控件的大小，如果否，绘图将会拉伸绘画
	* @param bTrans			是否使用透明色
	* @param crTran			透明色
	* @param bNormalRect	是否使用不规则矩形（不规则矩形计算是以正常模式图片计算，并且不支持拉伸模式）
	* @param crNormal		背景图片中不规则矩形需要透明的颜色
	*/
	void SetBitmap(HBITMAP hBitmap,int left,int top,int width,int height,BOOL bAutoExpand=TRUE,BOOL bTrans=FALSE,COLORREF crTran=RGB(255,0,255),BOOL bNormalRect=TRUE,COLORREF crNormal=RGB(255,0,255));
	/**
	* @brief 设置按钮使用的图标
	* @param hIcon			图标默认在靠左方位置，如果显示文本，文本会相应进行移动，图标规格为16*16
	*/
	void SetBtnIcon(HICON hIcon){m_hIcon=hIcon;Invalidate();};
	/**
	* @brief 设置按钮使用的光标
	* @param hCur			使用的光标
	*/
	void SetBtnCursor(HCURSOR hCur){m_hCursor=hCur;};
	/**
	* @brief 设置按钮是否显示文字（文字由SetWindowText设置）
	* @param align			文字对齐方式，为ButtonTextAlignNone表示不显示文字
	* @param crText			文字的颜色
	* @param hFont			使用字体，为空使用系统默认字体
	*/
	void SetShowText(THSkinButton::ButtonTextAlign align=ButtonTextAlignNone,COLORREF crText=RGB(0,0,0),HFONT hFont=NULL);
	/**
	* @brief 设置图像灰度
	* @param mask			1.0表示正常，0.X表示增加图像灰度，数值越小图像越黑
	*/
	void SetMask(double mask);
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd( CDC * cdc);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
	virtual void PreSubclassWindow();
};
