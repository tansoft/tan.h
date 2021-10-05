#pragma once
#include <THBitmap.h>
#include "THSkinStatic.h"

/**
* @brief 自绘静态控件及按钮类，支持自绘静态控件，及其上面的按钮
* @author Barry
*/
/**<pre>
  使用Sample：
	m_Static.SetBitmap(THBitmap(IDB_BITMAP1),0,0,64,64,TRUE,FALSE,0,FALSE,RGB(255,0,255));
	//设置按钮响应消息
	m_Static.SetButtonMsgHandler(m_hWnd,14001);
	//增加图片按钮区域
	m_Static.AddBmpButtonArea(3,THBitmap(IDB_MAIN),0,0,20,20,CRect(0,0,20,20),StaticTextAlignLeft,TRUE,RGB(255,0,255),CBmpStatic::CreatePresentFont(_T("宋体")),RGB(0,0,0));
	//设置图片按钮区域文本
	m_Static.SetAreaText(3,_T("Hello!"));
	//增加文字区域
	m_Static.AddTextArea(1,CBmpStatic::CreatePresentFont(_T("宋体")),CRect(32,0,64,32));
	m_Static.AddTextArea(2,CBmpStatic::CreatePresentFont(_T("黑体")),CRect(0,0,32,32));
	//设置文字区域的文本
	m_Static.SetAreaText(1,_T("哈哈"));
	m_Static.SetAreaText(2,_T("呵呵"));
	//增加图片区域
	m_Static.AddBmpTextArea(1,THBitmap(IDB_MAIN),0,122,10,20,_T("0123456789ABCDEF"),CRect(100,100,160,120));
	//设置图片区域文本
	m_Static.SetAreaText(1,_T("123AB"));
	//设置当前显示索引1的图片
	m_Static.SetBitmapIndex(1);
	//设置用于闪烁的队列
	CArray<int,int> changequeue;
	changequeue.Add(1);
	changequeue.Add(2);
	changequeue.Add(3);
	changequeue.Add(2);
	m_Static.SetChangeTimer(1000,changequeue);
	//设置图片的灰度，1.0为正常
	m_Static.SetMask(0.8);
</pre>*/

class THSkinButtonStatic : public THSkinStatic
{
	DECLARE_DYNAMIC(THSkinButtonStatic)
public:
	THSkinButtonStatic();
	virtual ~THSkinButtonStatic();

public:
	/**
	* @brief 增加图形按钮区域
	* @param id				索引值，如果有相同索引值的则替换，与Text，BmpText的id兼容
	* @param hBmp			使用图片的句柄
	* @param x				图片x坐标
	* @param y				图片y坐标
	* @param width			图片宽
	* @param height			图片高
	* @param btnRect		按钮使用的矩形
	* @param align			文字对齐方式
	* @param btran			是否使用透明
	* @param trancolor		透明色
	* @return				增加区域按钮结果
	*/
	BOOL AddBmpButtonArea(int id,HBITMAP hBmp,int x,int y,int width,int height,CRect btnRect,StaticTextAlign align=StaticTextAlignLeft,BOOL btran=FALSE,COLORREF trancolor=RGB(255,0,255),HFONT font=NULL,COLORREF fontcolor=RGB(0,0,0));
	/**
	* @brief 设置按钮的响应
	* @param hwnd			接收消息的句柄
	* @param fristCmd		发送的WM_COMMAND消息的id值，如设置了按钮id为1，fristCmd为14000，则按钮1按下时，发送消息为14001
	*/
	void SetButtonMsgHandler(HWND hwnd,UINT fristCmd);
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT nIDEvent);
	virtual void PrivateDraw(RECT rect,CDC *pDC);
	BOOL		m_bMouseDown;
	int			m_nLastHitButton;
	HWND		m_hWnd;
	UINT		m_nFristCmd;
public:
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
};
