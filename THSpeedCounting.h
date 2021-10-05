#pragma once

#include <THStruct.h>
#include <THString.h>

/**
* @brief �ٶȼ�����
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-04-07 �½���
* @2009-09-28 �����ٶ���ʾ����
*/
class THSpeedCounting : public THThread
{
public:
	THSpeedCounting()
	{
		m_nTotalCount=0;
		m_nCurCount=0;
		m_nPreSec=0;
		m_nCountingRound=1;
		m_nCurrentCounting=0;
		m_bAvgSpeedPreSec=FALSE;
		m_hDisWnd=NULL;
	}
	virtual ~THSpeedCounting(){}
	void SetCountingRound(int nRound=1){m_nCountingRound=nRound;}
	void SetSpeedPreSecMode(BOOL bAvg=TRUE)
	{
		m_bAvgSpeedPreSec=bAvg;
		if (m_bAvgSpeedPreSec)
		{
			m_nCurrentCounting=0;
			m_nTotalCount=0;
			m_nPreSec=0;
			m_nCountingRound=1;
		}
	}
	void AddReceiveData(UINT data){m_nCurCount+=data;m_nTotalCount+=data;}
	void Reset(BOOL bResetSpeed=FALSE)
	{
		m_nCurrentCounting=0;
		m_nCurCount=0;
		m_nTotalCount=0;
		if (bResetSpeed) m_nPreSec=0;
		//not need to reset to m_nPreSec value
		//m_nPreSec=0;
	}
	//ÿ�����һ�ε�TimerPump
	void ResetPreSec()
	{
		m_nCurrentCounting++;
		if (m_bAvgSpeedPreSec)
		{
			if (m_nCurrentCounting!=0)
				m_nPreSec=m_nTotalCount/m_nCurrentCounting;
		}
		else
		{
			if (m_nCountingRound<=m_nCurrentCounting)
			{
				m_nCurrentCounting=0;
				m_nPreSec=m_nCurCount;
				m_nCurCount=0;
			}
		}
		UpdateDisplayText();
	}
	UINT GetSpeed()
	{
		if (m_nCountingRound>1) return m_nPreSec/m_nCountingRound;
		return m_nPreSec;
	}
	UINT GetTotal(){return m_nTotalCount;}
	void StartSelfTimer(){KillAllThread();StartThread(0,NULL,1000);}
	void StopSelfTimer(){KillAllThread();}
	void UpdateDisplayText()
	{
		if (m_hDisWnd && IsWindow(m_hDisWnd))
		{
			THString str;
			str=m_sDisText;
			UINT speed=GetSpeed();
			str.Replace(_T("[UNITSPEED]"),THStringConv::u2sunit(speed));
			str.Replace(_T("[SPEED]"),THu2s(speed));
			speed=GetTotal();
			str.Replace(_T("[UINTTOTAL]"),THStringConv::u2sunit(speed));
			str.Replace(_T("[TOTAL]"),THu2s(speed));
			SetWindowText(m_hDisWnd,str);
		}
	}
	//sTextΪ��ʾ��ʽ
	//[UNITSPEED]�滻Ϊ����λ�ٶ�
	//[SPEED]�滻Ϊ�ٶ�
	//[UINTTOTAL]�滻Ϊ����λ���ֽ���
	//[TOTAL]�滻Ϊ���ֽ���
	void SetDisplayWnd(HWND hWnd,THString sText=_T("[UINTSPEED]/s"))
	{
		m_hDisWnd=hWnd;
		m_sDisText=sText;
	}
private:
	virtual void ThreadFunc(int ThreadId,void *data) {ResetPreSec();}
	volatile UINT m_nTotalCount;
	volatile UINT m_nCurCount;
	volatile UINT m_nPreSec;
	volatile UINT m_nCountingRound;//�������ڣ���Ϊ��Щ���ݷ��ؼ���ǳ���һ��ģ��������ݻ������ʾ���ȶ�
	volatile UINT m_nCurrentCounting;//������������
	BOOL m_bAvgSpeedPreSec;
	HWND m_hDisWnd;
	THString m_sDisText;
};
