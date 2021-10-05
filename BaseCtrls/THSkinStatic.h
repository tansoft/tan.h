#pragma once
#include "THBitmap.h"

/**
* @brief �Ի澲̬�ؼ��֧࣬���Ի�ͼƬ����˸ͼƬ������������
* @author Barry
*/
/**<pre>
  ʹ��Sample��
	m_Static.SetBitmap(THBitmap(IDB_BITMAP1,FALSE),0,0,64,64,TRUE,FALSE,0,FALSE,RGB(255,0,255));
	//������������
	m_Static.AddTextArea(1,THSkinStatic::CreatePresentFont(_T("����")),CRect(32,0,64,32));
	m_Static.AddTextArea(2,THSkinStatic::CreatePresentFont(_T("����")),CRect(0,0,32,32));
	//��������������ı�
	m_Static.SetAreaText(1,_T("����"));
	m_Static.SetAreaText(2,_T("�Ǻ�"));
	//����ͼƬ����
	m_Static.AddBmpTextArea(1,THBitmap(IDB_MAIN),0,122,10,20,_T("0123456789ABCDEF"),CRect(100,100,160,120));
	//����ͼƬ�����ı�
	m_Static.SetAreaText(1,_T("123AB"));
	//���õ�ǰ��ʾ����1��ͼƬ
	m_Static.SetBitmapIndex(1);
	//����������˸�Ķ���
	THList<int> changequeue;
	changequeue.AddHead(1);
	changequeue.AddHead(2);
	changequeue.AddHead(3);
	changequeue.AddHead(2);
	m_Static.SetChangeTimer(1000,changequeue);
	//����ͼƬ�ĻҶȣ�1.0Ϊ����
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

	/** �ı����з�ʽ���� */
	typedef enum{
		StaticTextAlignNone,		///<����ʾ�ı�
		StaticTextAlignLeft,		///<�ı���������
		StaticTextAlignCenter,		///<�ı���������
		StaticTextAlignRight		///<�ı���������
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
	* @brief ���ð�ťʹ�õı���ͼƬ��ͼƬ��y�᷽ʽ����Ϊ ��������꾭������갴�£���ֹ״̬�����û�н�ֹ�ɲ��ṩ��ֹͼƬ��
	* @param hBitmap		ͼƬλͼ���������ʹ��ͼƬ������THBitmap�����
	* @param left			��ťʹ�ñ�����ͼƬ�е���ʼx
	* @param top			��ťʹ�ñ�����ͼƬ�е���ʼy
	* @param width			��ťʹ�ñ�����
	* @param height			��ťʹ�ñ�����
	* @param bAutoExpand	�Ƿ��������Ŀ���趨��ť�ؼ��Ĵ�С������񣬻�ͼ��������滭
	* @param bTrans			�Ƿ�ʹ��͸��ɫ
	* @param crTran			͸��ɫ
	* @param bNormalRect	�Ƿ�ʹ�ò�������Σ���������μ�����������ģʽͼƬ���㣬���Ҳ�֧������ģʽ��
	* @param crNormal		����ͼƬ�в����������Ҫ͸������ɫ
	*/
	void SetBitmap(HBITMAP hBitmap,int left,int top,int width,int height,BOOL bAutoExpand=TRUE,BOOL bTrans=FALSE,COLORREF crTran=RGB(255,0,255),BOOL bNormalRect=TRUE,COLORREF crNormal=RGB(255,0,255));
	/**
	* @brief ����Ĭ�ϵ�����
	* @param Name			��������
	* @param nHeight		����߶ȣ�Ϊ����ֵ����ֵ��ʾ��С��ƥ�䣬��ֵ��ʾ����ƥ��
	* @param nWeight		����Ӵ���ȣ�FW_XXXX
	* @param bItalic		�Ƿ���б
	* @param bUnderline		�Ƿ�׻���
	* @return				����������
	*/
	static HFONT CreatePresentFont(CString Name,int nHeight=-16,int nWeight=FW_NORMAL,BOOL bItalic=FALSE,BOOL bUnderline=FALSE);
	/**
	* @brief ������������
	* @param id				����ֵ���������ͬ����ֵ�����滻
	* @param font			ʹ������ľ����Ϊ�ռ�ʹ��ϵͳĬ��
	* @param textRect		����ʹ�õľ���
	* @param fontcolor		������ɫ
	* @param align			������뷽ʽ
	* @return				����������
	*/
	BOOL AddTextArea(int id,HFONT font,CRect textRect,COLORREF fontcolor=RGB(0,0,0),StaticTextAlign align=StaticTextAlignLeft);
	/**
	* @brief ����ͼ����������
	* @param id				����ֵ���������ͬ����ֵ�����滻����Text��id����
	* @param hBmp			ʹ��ͼƬ�ľ��
	* @param x				����ͼƬx����
	* @param y				����ͼƬy����
	* @param width			����ͼƬ��
	* @param height			����ͼƬ��
	* @param filtertext		λͼ�����ֵ�ӳ���ϵ
	* @param textRect		����ʹ�õľ���
	* @param align			���ֶ��뷽ʽ
	* @param btran			�Ƿ�ʹ��͸��
	* @param trancolor		͸��ɫ
	* @return				����������
	*/
	BOOL AddBmpTextArea(int id,HBITMAP hBmp,int x,int y,int width,int height,CString filtertext,CRect textRect,StaticTextAlign align=StaticTextAlignLeft,BOOL btran=FALSE,COLORREF trancolor=RGB(255,0,255));
	/**
	* @brief ȥ����������
	* @param id				����ֵ
	* @return				ȥ��������
	*/
	BOOL RemoveTextArea(int id);
	/**
	* @brief �������������е�����
	* @param id				����ֵ
	* @param text			���õ�����
	* @return				�������ֽ��
	*/
	BOOL SetAreaText(int id,CString text);
	/**
	* @brief ������ʾ������λͼ��������ͼƬ��x���������Ҽ��㣩
	* @param index			����ֵ
	*/
	void SetBitmapIndex(int index);
	/**
	* @brief ������˸��ʾ��ÿһ�����û�ȡ��ǰһ�ε����ã����ʱ�Ӽ��Ϊ0��������Ϊ�գ���ȡ��֮ǰ�����ã�
	* @param interval		ʱ�ӵļ�����Ժ������
	* @param displayqueue	ѭ����ʾ�Ķ��У�ÿ��ֵ��Ϊλͼ������
	*/
	void SetChangeTimer(int interval,const THList<int> *displayqueue);
	/**
	* @brief ����ͼ��Ҷ�
	* @param mask			1.0��ʾ������0.X��ʾ����ͼ��Ҷȣ���ֵԽСͼ��Խ��
	*/
	void SetMask(double mask);
	/**
	* @brief ���ø�����֮�ϵĿؼ�
	* @param hWnd			�ؼ��ľ��
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
