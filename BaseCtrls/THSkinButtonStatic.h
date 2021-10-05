#pragma once
#include <THBitmap.h>
#include "THSkinStatic.h"

/**
* @brief �Ի澲̬�ؼ�����ť�֧࣬���Ի澲̬�ؼ�����������İ�ť
* @author Barry
*/
/**<pre>
  ʹ��Sample��
	m_Static.SetBitmap(THBitmap(IDB_BITMAP1),0,0,64,64,TRUE,FALSE,0,FALSE,RGB(255,0,255));
	//���ð�ť��Ӧ��Ϣ
	m_Static.SetButtonMsgHandler(m_hWnd,14001);
	//����ͼƬ��ť����
	m_Static.AddBmpButtonArea(3,THBitmap(IDB_MAIN),0,0,20,20,CRect(0,0,20,20),StaticTextAlignLeft,TRUE,RGB(255,0,255),CBmpStatic::CreatePresentFont(_T("����")),RGB(0,0,0));
	//����ͼƬ��ť�����ı�
	m_Static.SetAreaText(3,_T("Hello!"));
	//������������
	m_Static.AddTextArea(1,CBmpStatic::CreatePresentFont(_T("����")),CRect(32,0,64,32));
	m_Static.AddTextArea(2,CBmpStatic::CreatePresentFont(_T("����")),CRect(0,0,32,32));
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
	CArray<int,int> changequeue;
	changequeue.Add(1);
	changequeue.Add(2);
	changequeue.Add(3);
	changequeue.Add(2);
	m_Static.SetChangeTimer(1000,changequeue);
	//����ͼƬ�ĻҶȣ�1.0Ϊ����
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
	* @brief ����ͼ�ΰ�ť����
	* @param id				����ֵ���������ͬ����ֵ�����滻����Text��BmpText��id����
	* @param hBmp			ʹ��ͼƬ�ľ��
	* @param x				ͼƬx����
	* @param y				ͼƬy����
	* @param width			ͼƬ��
	* @param height			ͼƬ��
	* @param btnRect		��ťʹ�õľ���
	* @param align			���ֶ��뷽ʽ
	* @param btran			�Ƿ�ʹ��͸��
	* @param trancolor		͸��ɫ
	* @return				��������ť���
	*/
	BOOL AddBmpButtonArea(int id,HBITMAP hBmp,int x,int y,int width,int height,CRect btnRect,StaticTextAlign align=StaticTextAlignLeft,BOOL btran=FALSE,COLORREF trancolor=RGB(255,0,255),HFONT font=NULL,COLORREF fontcolor=RGB(0,0,0));
	/**
	* @brief ���ð�ť����Ӧ
	* @param hwnd			������Ϣ�ľ��
	* @param fristCmd		���͵�WM_COMMAND��Ϣ��idֵ���������˰�ťidΪ1��fristCmdΪ14000����ť1����ʱ��������ϢΪ14001
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
