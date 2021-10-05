#pragma once

#include "THBitmap.h"

/**
* @brief �Ի水ť�֧࣬���Ի�ͼƬ���ǹ������
* @author Barry
*/
/**<pre>
  ʹ��Sample��
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
	/** �ı����з�ʽ���� */
	typedef enum{
		ButtonTextAlignNone,		///<����ʾ�ı�
		ButtonTextAlignLeft,		///<�ı���������
		ButtonTextAlignCenter,		///<�ı���������
		ButtonTextAlignRight		///<�ı���������
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
	* @brief ���ð�ťģʽΪ��ά�ְ���ģʽ
	* @param bPress ����Ĭ�ϰ���״̬
	*/
	void SetRadioMode(BOOL bPress=FALSE);
	/** @brief ���ð�ťģʽΪ����ģʽ����ťĬ��ģʽΪ����ģʽ*/
	void SetNormalMode(){m_nTypeRadio=0;Invalidate();}
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
	* @brief ���ð�ťʹ�õ�ͼ��
	* @param hIcon			ͼ��Ĭ���ڿ���λ�ã������ʾ�ı����ı�����Ӧ�����ƶ���ͼ����Ϊ16*16
	*/
	void SetBtnIcon(HICON hIcon){m_hIcon=hIcon;Invalidate();};
	/**
	* @brief ���ð�ťʹ�õĹ��
	* @param hCur			ʹ�õĹ��
	*/
	void SetBtnCursor(HCURSOR hCur){m_hCursor=hCur;};
	/**
	* @brief ���ð�ť�Ƿ���ʾ���֣�������SetWindowText���ã�
	* @param align			���ֶ��뷽ʽ��ΪButtonTextAlignNone��ʾ����ʾ����
	* @param crText			���ֵ���ɫ
	* @param hFont			ʹ�����壬Ϊ��ʹ��ϵͳĬ������
	*/
	void SetShowText(THSkinButton::ButtonTextAlign align=ButtonTextAlignNone,COLORREF crText=RGB(0,0,0),HFONT hFont=NULL);
	/**
	* @brief ����ͼ��Ҷ�
	* @param mask			1.0��ʾ������0.X��ʾ����ͼ��Ҷȣ���ֵԽСͼ��Խ��
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
