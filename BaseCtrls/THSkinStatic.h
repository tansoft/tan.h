#pragma once
#include "THBitmap.h"

/**
* @brief 自绘静态控件类，支持自绘图片，闪烁图片，多区域文字
* @author Barry
*/
/**<pre>
  使用Sample：
	m_Static.SetBitmap(THBitmap(IDB_BITMAP1,FALSE),0,0,64,64,TRUE,FALSE,0,FALSE,RGB(255,0,255));
	//增加文字区域
	m_Static.AddTextArea(1,THSkinStatic::CreatePresentFont(_T("宋体")),CRect(32,0,64,32));
	m_Static.AddTextArea(2,THSkinStatic::CreatePresentFont(_T("黑体")),CRect(0,0,32,32));
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
	THList<int> changequeue;
	changequeue.AddHead(1);
	changequeue.AddHead(2);
	changequeue.AddHead(3);
	changequeue.AddHead(2);
	m_Static.SetChangeTimer(1000,changequeue);
	//设置图片的灰度，1.0为正常
	m_Static.SetMask(0.8);
</pre>*/

//fixme,add textarea change queue

class THSkinStatic : public CStatic
{
	DECLARE_DYNAMIC(THSkinStatic)
private:
	static void MapItemFreeCallBack(void *key,void *value,void *adddata);
public:
	THSkinStatic();
	virtual ~THSkinStatic();

	/** 文本排列方式定义 */
	typedef enum{
		StaticTextAlignNone,		///<不显示文本
		StaticTextAlignLeft,		///<文本居左排列
		StaticTextAlignCenter,		///<文本居中排列
		StaticTextAlignRight		///<文本居右排列
	}StaticTextAlign;
protected:
	int			m_nLeft,m_nTop,m_nWidth,m_nHeight;
	HBITMAP		m_hBmp;
	BITMAP		m_bmp;
	COLORREF	m_clrTransColor;
	BOOL		m_bTransparent;
	CRgn		m_rgnTotal;
	int			m_nIndex;
	THList<long>m_rtEmpty;
	UINT		m_nTimer;
	UINT		m_nTimerIndex;
	THList<int>m_displayqueue;
	double		m_mask;
	int GetFilterIndex(CString filter,TCHAR ch);
	typedef struct{
		int type;//0:font,1:bitmap
		struct{
			struct FontStruct{
				HFONT font;
				COLORREF fontcolor;
			}font;
			struct BmpStruct{
				HBITMAP bmp;
				int x;
				int y;
				int w;
				int h;
				BOOL btran;
				COLORREF trancolor;
				CString filtertext;
			}bmp;
		}typeinfo;
		CRect rect;
		StaticTextAlign align;
		CString text;
	}TextInfo;
	THMap<int,TextInfo *>m_textinfo;
	THBitmap *m_pbitmap;
	THList<HWND>m_hWndOver;
public:
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
	* @brief 创建默认的字体
	* @param Name			字体名称
	* @param nHeight		字体高度，为像素值，负值表示向小的匹配，正值表示向大的匹配
	* @param nWeight		字体加粗情度，FW_XXXX
	* @param bItalic		是否倾斜
	* @param bUnderline		是否底划线
	* @return				返回字体句柄
	*/
	static HFONT CreatePresentFont(CString Name,int nHeight=-16,int nWeight=FW_NORMAL,BOOL bItalic=FALSE,BOOL bUnderline=FALSE);
	/**
	* @brief 增加文字区域
	* @param id				索引值，如果有相同索引值的则替换
	* @param font			使用字体的句柄，为空即使用系统默认
	* @param textRect		字体使用的矩形
	* @param fontcolor		字体颜色
	* @param align			字体对齐方式
	* @return				增加区域结果
	*/
	BOOL AddTextArea(int id,HFONT font,CRect textRect,COLORREF fontcolor=RGB(0,0,0),StaticTextAlign align=StaticTextAlignLeft);
	/**
	* @brief 增加图形文字区域
	* @param id				索引值，如果有相同索引值的则替换，与Text的id兼容
	* @param hBmp			使用图片的句柄
	* @param x				数字图片x坐标
	* @param y				数字图片y坐标
	* @param width			数字图片宽
	* @param height			数字图片高
	* @param filtertext		位图与文字的映射关系
	* @param textRect		字体使用的矩形
	* @param align			文字对齐方式
	* @param btran			是否使用透明
	* @param trancolor		透明色
	* @return				增加区域结果
	*/
	BOOL AddBmpTextArea(int id,HBITMAP hBmp,int x,int y,int width,int height,CString filtertext,CRect textRect,StaticTextAlign align=StaticTextAlignLeft,BOOL btran=FALSE,COLORREF trancolor=RGB(255,0,255));
	/**
	* @brief 去除文字区域
	* @param id				索引值
	* @return				去除区域结果
	*/
	BOOL RemoveTextArea(int id);
	/**
	* @brief 设置文字区域中的文字
	* @param id				索引值
	* @param text			设置的文字
	* @return				设置文字结果
	*/
	BOOL SetAreaText(int id,CString text);
	/**
	* @brief 设置显示索引的位图（索引以图片中x轴由左向右计算）
	* @param index			索引值
	*/
	void SetBitmapIndex(int index);
	/**
	* @brief 设置闪烁显示（每一次设置会取消前一次的设置，如果时钟间隔为0或者数组为空，仅取消之前的设置）
	* @param interval		时钟的间隔，以毫秒计算
	* @param displayqueue	循环显示的队列，每个值即为位图的索引
	*/
	void SetChangeTimer(int interval,const THList<int> *displayqueue);
	/**
	* @brief 设置图像灰度
	* @param mask			1.0表示正常，0.X表示增加图像灰度，数值越小图像越黑
	*/
	void SetMask(double mask);
	/**
	* @brief 设置覆盖其之上的控件
	* @param hWnd			控件的句柄
	*/
	void AddOverHwnd(HWND hWnd);
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd( CDC * );
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnTimer(UINT nIDEvent);
	virtual void PrivateDraw(RECT rect,CDC *pDC);
};
