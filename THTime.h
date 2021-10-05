#pragma once

#define THSYSTEM_INCLUDE_MATH
#define THSYSTEM_INCLUDE_TIME
#include <THSystem.h>
#include <THStruct.h>
#include <THString.h>

#define THDATETIME_HALFSECOND (1.0/(2.0*(60.0*60.0*24.0)))

#define RANGETYPE_MONTH		0
#define RANGETYPE_QUARTER	1
#define RANGETYPE_YEAR		2

const long _maxDaysInSpan  =	3615897L;

/**
* @brief ʱ��������
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-07-16 �½���
*/
/**<pre>
�÷���
</pre>*/
class THTimeSpan
{
public:
	THTimeSpan()
	{
		m_span=0;
		m_status=valid;
	}

	THTimeSpan(double dblSpanSrc)
	{
		m_span=dblSpanSrc;
		m_status=valid;
		CheckRange();
	}

	THTimeSpan(LONG lDays, int nHours, int nMins, int nSecs)
	{
		SetDateTimeSpan(lDays, nHours, nMins, nSecs);
	}
private:
	enum DateTimeSpanStatus
	{
		valid = 0,
		invalid = 1,    // Invalid span (out of range, etc.)
		null = 2,       // Literally has no value
	};
	double m_span;
	DateTimeSpanStatus m_status;
public:
	void SetStatus(DateTimeSpanStatus status){m_status = status;}

	DateTimeSpanStatus GetStatus() const{return m_status;}

	double GetTotalDays() const
	{
		if (GetStatus() == valid)
		{
			return LONG(m_span + (m_span < 0 ?
				-THDATETIME_HALFSECOND : THDATETIME_HALFSECOND));
		}
		return 0.0;
	}

	double GetTotalHours() const
	{
		if (GetStatus() == valid)
		{
			return LONG((m_span + (m_span < 0 ? 
				-THDATETIME_HALFSECOND : THDATETIME_HALFSECOND)) * 24);
		}
		return 0.0;
	}

	double GetTotalMinutes() const
	{
		if (GetStatus() == valid)
		{
			return LONG((m_span + (m_span < 0 ?
				-THDATETIME_HALFSECOND : THDATETIME_HALFSECOND)) * (24 * 60));
		}
		return 0.0;
	}

	double GetTotalSeconds() const
	{
		if (GetStatus() == valid)
		{
			return LONG((m_span + (m_span < 0 ?
				-THDATETIME_HALFSECOND : THDATETIME_HALFSECOND)) * (24 * 60 * 60));
		}
		return 0.0;
	}

	LONG GetDays() const
	{
		if (GetStatus() == valid)
			return LONG(m_span);
		return 0;
	}

	LONG GetHours() const
	{
		return LONG(GetTotalHours()) % 24;
	}

	LONG GetMinutes() const
	{
		return LONG(GetTotalMinutes()) % 60;
	}

	LONG GetSeconds() const
	{
		return LONG(GetTotalSeconds()) % 60;
	}

	THTimeSpan& operator=(double dblSpanSrc)
	{
		m_span = dblSpanSrc;
		m_status = valid;
		CheckRange();
		return *this;
	}

	bool operator==(const THTimeSpan& dateSpan) const
	{
		if(GetStatus() == dateSpan.GetStatus())
		{
			if(GetStatus() == valid)
				return (m_span == dateSpan.m_span);			
			
			return (GetStatus() == null);
		}

		return false;
	}

	bool operator!=(const THTimeSpan& dateSpan) const
	{
		return !operator==(dateSpan);
	}

	bool operator<(const THTimeSpan& dateSpan) const
	{
		if (GetStatus() == valid && dateSpan.GetStatus() == GetStatus())
			return m_span < dateSpan.m_span;
		return false;
	}

	bool operator>(const THTimeSpan& dateSpan) const
	{
		if( (GetStatus() == valid) && (GetStatus() == dateSpan.GetStatus()) )
			return m_span > dateSpan.m_span ;
		return false;
	}

	bool operator<=(const THTimeSpan& dateSpan) const
	{
		return operator<(dateSpan) || operator==(dateSpan);
	}

	bool operator>=(const THTimeSpan& dateSpan) const
	{
		return operator>(dateSpan) || operator==(dateSpan);
	}

	THTimeSpan operator+(const THTimeSpan& dateSpan) const
	{
		THTimeSpan dateSpanTemp;

		// If either operand Null, result Null
		if (GetStatus() == null || dateSpan.GetStatus() == null)
		{
			dateSpanTemp.SetStatus(null);
			return dateSpanTemp;
		}

		// If either operand Invalid, result Invalid
		if (GetStatus() == invalid || dateSpan.GetStatus() == invalid)
		{
			dateSpanTemp.SetStatus(invalid);
			return dateSpanTemp;
		}

		// Add spans and validate within legal range
		dateSpanTemp.m_span = m_span + dateSpan.m_span;
		dateSpanTemp.CheckRange();

		return dateSpanTemp;
	}

	THTimeSpan operator-(const THTimeSpan& dateSpan) const
	{
		THTimeSpan dateSpanTemp;

		// If either operand Null, result Null
		if (GetStatus() == null || dateSpan.GetStatus() == null)
		{
			dateSpanTemp.SetStatus(null);
			return dateSpanTemp;
		}

		// If either operand Invalid, result Invalid
		if (GetStatus() == invalid || dateSpan.GetStatus() == invalid)
		{
			dateSpanTemp.SetStatus(invalid);
			return dateSpanTemp;
		}

		// Subtract spans and validate within legal range
		dateSpanTemp.m_span = m_span - dateSpan.m_span;
		dateSpanTemp.CheckRange();

		return dateSpanTemp;
	}

	THTimeSpan& operator+=(const THTimeSpan dateSpan)
	{
		ATLASSERT(GetStatus() == valid);
		ATLASSERT(dateSpan.GetStatus() == valid);
		*this = *this + dateSpan;
		CheckRange();
		return *this;
	}

	THTimeSpan& operator-=(const THTimeSpan dateSpan)
	{
		ATLASSERT(GetStatus() == valid);
		ATLASSERT(dateSpan.GetStatus() == valid);
		*this = *this - dateSpan;
		CheckRange();
		return *this;
	}

	THTimeSpan operator-() const
	{
		return -this->m_span;
	}

	operator double() const
	{
		return m_span;
	}

	void SetDateTimeSpan(LONG lDays, int nHours, int nMins, int nSecs)
	{
		// Set date span by breaking into fractional days (all input ranges valid)
		m_span = lDays + ((double)nHours)/24 + ((double)nMins)/(24*60) +
			((double)nSecs)/(24*60*60);
		m_status = valid;
		CheckRange();
	}

	void CheckRange()
	{
		if(m_span < -_maxDaysInSpan || m_span > _maxDaysInSpan)
			m_status = invalid;
	}
};

#ifndef __time32_t
	#define __time32_t time_t
	#define _localtime32 localtime
	#define _gmtime32 gmtime
#endif
/**
* @brief ʱ�����
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-06-15 �½���
* @2007-07-06 ���Ӷ��ָ�ʽת��
*/
/**<pre>
�÷���
</pre>*/
class THTime
{
public:
	SYSTEMTIME m_st;

	THTime(BOOL bLocal=TRUE){InitCom();GetCurTime(bLocal);}
	THTime(SYSTEMTIME st){InitCom();SetTime(st);}
	THTime(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec){InitCom();SetTime(nYear, nMonth, nDay, nHour, nMin, nSec);}
	THTime(DATE dt){InitCom();SetTime(dt);}
	THTime(const FILETIME& filetimeSrc){InitCom();SetTime(filetimeSrc);}
	THTime(WORD wDosDate, WORD wDosTime){InitCom();SetTime(wDosDate,wDosTime);}
	THTime(__time64_t ti,BOOL bLocal=TRUE){InitCom();SetTime(ti,bLocal);}
	THTime(__time32_t ti,BOOL bLocal=TRUE){InitCom();SetTime(ti,bLocal);}
	virtual ~THTime()
	{
		UninitCom();
	}

	void InitCom() const
	{
		::CoInitialize(NULL);
	}

	void UninitCom() const
	{
		::CoUninitialize();
	}

	void GetCurTime(BOOL bLocal=TRUE)
	{
		if (bLocal)
			GetLocalTime(&m_st);
		else
			GetSystemTime(&m_st);
	}
	//same as double
	BOOL SetTime(DATE dt)
	{
		//InitCom();
		return VariantTimeToSystemTime(dt,&m_st);
	}

	THTime LocalTimeToSystemTime()
	{
		TIME_ZONE_INFORMATION tzinfo;
		GetTimeZoneInformation(&tzinfo);
		SYSTEMTIME out;
		TzSpecificLocalTimeToSystemTime(&tzinfo,&m_st,&out);
		return THTime(out);
	}

	THTime SystemTimeToLocalTime()
	{
		TIME_ZONE_INFORMATION tzinfo;
		GetTimeZoneInformation(&tzinfo);
		SYSTEMTIME out;
		SystemTimeToTzSpecificLocalTime(&tzinfo,&m_st,&out);
		return THTime(out);
	}

	BOOL SetTime(__time64_t ti,BOOL bLocal=TRUE)
	{
		struct tm* ptm;
		if (bLocal)
			ptm=_localtime64(&ti);
		else
			ptm=_gmtime64(&ti);
		if (ptm)
		{
			m_st.wYear = (WORD) (1900 + ptm->tm_year);
			m_st.wMonth = (WORD) (1 + ptm->tm_mon);
			m_st.wDayOfWeek = (WORD) ptm->tm_wday;
			m_st.wDay = (WORD) ptm->tm_mday;
			m_st.wHour = (WORD) ptm->tm_hour;
			m_st.wMinute = (WORD) ptm->tm_min;
			m_st.wSecond = (WORD) ptm->tm_sec;
			m_st.wMilliseconds = 0;
		}
		else
		{
			memset(&m_st,0,sizeof(m_st));
		}
		return TRUE;
	}

	BOOL SetTime(__time32_t ti,BOOL bLocal=TRUE)
	{
		struct tm* ptm;
		if (bLocal)
			ptm=_localtime32(&ti);
		else
			ptm=_gmtime32(&ti);
		if (ptm)
		{
			m_st.wYear = (WORD) (1900 + ptm->tm_year);
			m_st.wMonth = (WORD) (1 + ptm->tm_mon);
			m_st.wDayOfWeek = (WORD) ptm->tm_wday;
			m_st.wDay = (WORD) ptm->tm_mday;
			m_st.wHour = (WORD) ptm->tm_hour;
			m_st.wMinute = (WORD) ptm->tm_min;
			m_st.wSecond = (WORD) ptm->tm_sec;
			m_st.wMilliseconds = 0;
		}
		else
		{
			memset(&m_st,0,sizeof(m_st));
		}
		return TRUE;
	}

	BOOL SetTime(SYSTEMTIME st)
	{
		m_st=st;
		return TRUE;
	}
	BOOL SetTime(const FILETIME& filetimeSrc)
	{
		return ::FileTimeToSystemTime(&filetimeSrc, &m_st);
	}
	BOOL SetTime(WORD wDosDate, WORD wDosTime)
	{
		//InitCom();
		DOUBLE dt;
		if (::DosDateTimeToVariantTime(wDosDate, wDosTime, &dt))
			return VariantTimeToSystemTime(dt,&m_st);
		return FALSE;
	}
	//format like Y-m-d h:m:s
	BOOL SetTime(THString sTimeString)
	{
		int nYear,nMonth,nDay,nHour,nMin,nSec;
		nYear=nMonth=nDay=nHour=nMin=nSec=0;
		//year
		int posy=sTimeString.Find(_T("-"),0);
		if (posy==-1 || posy>6) return FALSE;
		nYear=THStringConv::s2i(sTimeString.Left(posy));
		sTimeString=sTimeString.Mid(posy+1);
		posy=sTimeString.Find(_T("-"),0);
		if (posy==-1 || posy>3) return FALSE;
		//month
		nMonth=THStringConv::s2i(sTimeString.Left(posy));
		sTimeString=sTimeString.Mid(posy+1);
		//day
		posy=sTimeString.Find(_T(" "),0);
		if (posy!=-1 && posy>3) return FALSE;
		if (posy!=-1)
			nDay=THStringConv::s2i(sTimeString.Left(posy));
		else
		{
			nDay=THStringConv::s2i(sTimeString);
			return SetTime(nYear,nMonth,nDay,nHour,nMin,nSec);
		}
		sTimeString=sTimeString.Mid(posy+1);
		//hour
		posy=sTimeString.Find(_T(":"),0);
		if (posy==-1 || posy>3) return FALSE;
		nHour=THStringConv::s2i(sTimeString.Left(posy));
		sTimeString=sTimeString.Mid(posy+1);
		//min
		posy=sTimeString.Find(_T(":"),0);
		if (posy==-1 || posy>3) return FALSE;
		nMin=THStringConv::s2i(sTimeString.Left(posy));
		sTimeString=sTimeString.Mid(posy+1);
		//secord
		nSec=THStringConv::s2i(sTimeString);
		return SetTime(nYear,nMonth,nDay,nHour,nMin,nSec);
	}
	/**
	* @brief �����ַ�������ʱ�䣬û�����õ�ʱ��ֵ���᷵�ص�ǰʱ��Ķ�Ӧֵ
	* @param sTimeString	ʱ���ַ���
	* @param sTimeFormat	�ַ�����ʽ��ֻ֧��%d,%2d(��)%H,%2H(ʱ)%m,%2m(��)%M,%2M(��)%S,%2S(��)%y,%2y(��������),%Y,%4Y(��������)��֧��MatchSearch��ʽ�����THMatchSearch::MatchSearch
	* @return �Ƿ�ɹ�
	*/
	BOOL SetTime(THString sTimeString,THString sTimeFormat)
	{
		if (sTimeString.IsEmpty() || sTimeFormat.IsEmpty()) return FALSE;
		//d H m M S y Y
		const int PosCnt=14;
		const TCHAR PosKey[PosCnt][5]={	_T("%d"),_T("%2d"),_T("%H"),_T("%2H"),_T("%m"),_T("%2m"),_T("%M"),_T("%2M"),_T("%S"),_T("%2S"),_T("%y"),_T("%2y"),_T("%Y"),_T("%4Y")};
		const TCHAR PosRep[PosCnt][5]={	_T("%d"),_T("%2d"),_T("%d"),_T("%2d"),_T("%d"),_T("%2d"),_T("%d"),_T("%2d"),_T("%d"),_T("%2d"),_T("%2d"),_T("%2d"),_T("%4d"),_T("%4d")};
		int PosData[PosCnt];
		int SortPosData[PosCnt];
		int i;
		for(i=0;i<PosCnt;i++)
			PosData[i]=sTimeFormat.Find(PosKey[i]);
		for(i=0;i<PosCnt;i++)
			sTimeFormat.Replace(PosKey[i],PosRep[i]);
		THStringArray ar;
		if (!THMatchSearch::MatchSearch(&sTimeString,sTimeFormat,&ar)) return FALSE;
		if (!THSort<int,int>::SortPos(PosData,SortPosData,PosCnt)) return FALSE;
		THTime ti;
		int year=ti.GetYear();
		int mon=ti.GetMonth();
		int day=ti.GetDay();
		int hour=ti.GetHour();
		int min=ti.GetMinute();
		int sec=ti.GetSecond();
		int z=0;
		for(int i=0;i<PosCnt;i++)
		{
			if (PosData[SortPosData[i]]!=-1)
			{
				int val=THs2i(ar[z++]);
				switch(SortPosData[i])
				{
					case 0:case 1:day=val;break;
					case 2:case 3:hour=val;break;
					case 4:case 5:mon=val;break;
					case 6:case 7:min=val;break;
					case 8:case 9:sec=val;break;
					case 10:case 11:year=((int)(year/100))*100+val;break;
					case 12:case 13:year=val;break;
				}
			}
		}
		return SetTime(year,mon,day,hour,min,sec);
	}
	BOOL SetTime(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec)
	{
		//InitCom();
		if (nYear<0 || nMonth<1 || nMonth>12 || nDay<1 || nDay>31 || nHour<0 || nHour>24 || nMin<0 || nMin>60 || nSec<0 || nSec>60) return FALSE;
		m_st.wYear = (WORD) nYear;
		m_st.wMonth = (WORD) nMonth;
		m_st.wDay = (WORD) nDay;
		m_st.wHour = (WORD) nHour;
		m_st.wMinute = (WORD) nMin;
		m_st.wSecond = (WORD) nSec;
		m_st.wMilliseconds = 0;
		DOUBLE dl;
		//fill the m_st.wDayOfWeek field;
		if (SystemTimeToVariantTime(&m_st,&dl))
			return VariantTimeToSystemTime(dl,&m_st);
		return FALSE;
	}

	/**
	* @brief �ж��Ƿ���ʱ�����
	* @param sRange		ʱ��Σ���09:30-11:30
	* @return �Ƿ����
	*/
	BOOL IsInHourRange(THString sRange)
	{
		THStringToken t,t1;
		t.Init(sRange,_T("-"));
		t1.Init(t.GetNextToken(),_T(":"));
		int hour1,hour2;
		hour1=THs2i(t1.GetNextToken())*100;
		hour1+=THs2i(t1.GetNextToken());
		t1.Init(t.GetNextToken(),_T(":"));
		hour2=THs2i(t1.GetNextToken())*100;
		hour2+=THs2i(t1.GetNextToken());
		int now=((int)m_st.wHour)*100+m_st.wMinute;
		return (hour1<=now && now<=hour2);
	}

	/**
	* @brief ��ȡʱ��Ŀ�ʼ���Ӻͽ�������
	* @param starttime		��ʼʱ��
	* @param endtime		����ʱ��
	* @return �Ƿ�ɹ�
	*/
	BOOL GetMinuteRange(THTime *starttime,THTime *endtime)
	{
		starttime->SetTime(m_st.wYear,m_st.wMonth,m_st.wDay,m_st.wHour,m_st.wMinute,0);
		endtime->SetTime(m_st.wYear,m_st.wMonth,m_st.wDay,m_st.wHour,m_st.wMinute,59);
		return TRUE;
	}

	/**
	* @brief ��ȡʱ��Ŀ�ʼСʱ�ͽ���Сʱ
	* @param starttime		��ʼʱ��
	* @param endtime		����ʱ��
	* @return �Ƿ�ɹ�
	*/
	BOOL GetHourRange(THTime *starttime,THTime *endtime)
	{
		starttime->SetTime(m_st.wYear,m_st.wMonth,m_st.wDay,m_st.wHour,0,0);
		endtime->SetTime(m_st.wYear,m_st.wMonth,m_st.wDay,m_st.wHour,59,59);
		return TRUE;
	}

	/**
	* @brief ��ȡ���ڵĿ�ʼʱ��ͽ���ʱ��
	* @param starttime		��ʼʱ��
	* @param endtime		����ʱ��
	* @return �Ƿ�ɹ�
	*/
	BOOL GetDayRange(THTime *starttime,THTime *endtime)
	{
		starttime->SetTime(m_st.wYear,m_st.wMonth,m_st.wDay,0,0,0);
		endtime->SetTime(m_st.wYear,m_st.wMonth,m_st.wDay,23,59,59);
		return TRUE;
	}

	/**
	* @brief ��ȡ�������ڵ����ڿ�ʼ�պͽ�����
	* @param starttime		��ʼ��ʱ��
	* @param endtime		������ʱ��
	* @param bStartSunday	�Ƿ�����������ʼ˳�򣬷�Ϊ����һΪ��ʼ˳��
	* @return �Ƿ�ɹ�
	*/
	BOOL GetWeekRange(THTime *starttime,THTime *endtime,BOOL bStartSunday)
	{
		int wd=(int)GetDayOfWeek();
		int startoff,endoff;
		if (bStartSunday)
		{
			startoff=wd;
			endoff=6-startoff;
		}
		else
		{
			startoff=(wd+6)%7;
			endoff=6-startoff;
		}
		THTimeSpan span(startoff,0,0,0);
		THTimeSpan span1(endoff,0,0,0);
		THTime now(m_st.wYear,m_st.wMonth,m_st.wDay,0,0,0);
		(*starttime)=now-span;
		now=now+span1;
		endtime->SetTime(now.GetYear(),now.GetMonth(),now.GetDay(),23,59,59);
		return TRUE;
	}

	/**
	* @brief ��ȡ�������ڵ��·ݿ�ʼ�պͽ�����
	* @param starttime		��ʼ��ʱ��
	* @param endtime		������ʱ��
	* @return �Ƿ�ɹ�
	*/
	BOOL GetMonthRange(THTime *starttime,THTime *endtime)
	{
		starttime->SetTime(m_st.wYear,m_st.wMonth,1,0,0,0);
		THTime end;
		if (m_st.wMonth==12)
			end.SetTime(m_st.wYear+1,1,1,0,0,0);
		else
			end.SetTime(m_st.wYear,m_st.wMonth+1,1,0,0,0);
		//the next month's frist day - 1 sec
		THTimeSpan span(0,0,0,1);
		(*endtime)=end-span;
		return TRUE;
	}

	/**
	* @brief ��ȡ�������ڵ��꿪ʼ�պͽ�����
	* @param starttime		��ʼ��ʱ��
	* @param endtime		������ʱ��
	* @return �Ƿ�ɹ�
	*/
	BOOL GetYearRange(THTime *starttime,THTime *endtime)
	{
		starttime->SetTime(m_st.wYear,1,1,0,0,0);
		THTime end;
		end.SetTime(m_st.wYear+1,1,1,0,0,0);
		//the next year's frist day - 1 sec
		THTimeSpan span(0,0,0,1);
		(*endtime)=end-span;
		return TRUE;
	}

	time_t GetTimeAsTime() const
	{
		if (m_st.wYear < 1900) return 0;
		struct tm atm;
		atm.tm_sec = m_st.wSecond;
		atm.tm_min = m_st.wMinute;
		atm.tm_hour = m_st.wHour;
		atm.tm_mday = m_st.wDay;
		atm.tm_mon = m_st.wMonth - 1;        // tm_mon is 0 based
		atm.tm_year = m_st.wYear - 1900;     // tm_year is 1900 based
		atm.tm_isdst = 0;
		time_t ttime=mktime(&atm);
		if (ttime == -1) return 0;
		return ttime;
	}

	__time64_t GetTimeAsTime64() const
	{
		if (m_st.wYear < 1900) return 0;
		struct tm atm;
		atm.tm_sec = m_st.wSecond;
		atm.tm_min = m_st.wMinute;
		atm.tm_hour = m_st.wHour;
		atm.tm_mday = m_st.wDay;
		atm.tm_mon = m_st.wMonth - 1;        // tm_mon is 0 based
		atm.tm_year = m_st.wYear - 1900;     // tm_year is 1900 based
		atm.tm_isdst = 0;
		__time64_t ttime = _mktime64(&atm);
		if(ttime == -1)
			return 0;
		return ttime;
	}
	DATE GetTimeAsDate()
	{
		//InitCom();
		DOUBLE dt;
		if (SystemTimeToVariantTime(&m_st,&dt)) return dt;
		return 0.0;
	}
	SYSTEMTIME GetTimeAsSystemTime()
	{
		return m_st;
	}
	BOOL GetTimeAsFileTime(LPFILETIME ft)
	{
		return ::SystemTimeToFileTime(&m_st,ft);
	}
	BOOL GetTimeAsDosTime(WORD *wDosDate, WORD *wDosTime)
	{
		//InitCom();
		DOUBLE dt;
		if (::SystemTimeToVariantTime(&m_st,&dt))
			return ::VariantTimeToDosDateTime(dt,wDosDate,wDosTime);
		return FALSE;
	}
	BOOL GetTimeAsUdate(UDATE &udate)
	{
		//InitCom();
		DOUBLE dt;
		if (::SystemTimeToVariantTime(&m_st,&dt))
			return SUCCEEDED(::VarUdateFromDate(dt, 0, &udate));
		return FALSE;
	}

	/**
	* @brief �����ַ���ָ��������
	* @param sWeekStr		�����ַ�����֧�����ģ�Ӣ�ģ�Ӣ�ļ�д������
	* @return ���ڣ�0Ϊ����
	*/
	static int GetWeekFromStr(THString sWeekStr)
	{
		const TCHAR buf[5][7][20]={
			{_T("����һ"),_T("���ڶ�"),_T("������"),_T("������"),_T("������"),_T("������"),_T("������")},
			{_T("һ"),_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),_T("��")},
			{_T("monday"),_T("tuesday"),_T("wenseday"),_T("thursday"),_T("friday"),_T("saturday"),_T("sunday")},
			{_T("mon"),_T("tus"),_T("wen"),_T("thu"),_T("fri"),_T("sat"),_T("sun")},
			{_T("1"),_T("2"),_T("3"),_T("4"),_T("5"),_T("6"),_T("7")}};
		sWeekStr.MakeLower();
		if (sWeekStr.Find(_T("������"))!=-1 || sWeekStr.Find(_T("������"))!=-1) return 7;
		//����ƥ��ǰ���
		for(int j=0;j<5;j++)
		{
			for(int i=0;i<7;i++)
			{
				if (sWeekStr.Find(buf[j][i])!=-1)
					return i+1;
			}
		}
		return 0;
	}

	/**
	* @brief �����ַ���ָ�����·�
	* @param sMonStr		�·��ַ�����֧�����ģ�Ӣ�ģ�Ӣ�ļ�д���·�
	* @return �·ݣ�0Ϊ����
	*/
	static int GetMonthFromStr(THString sMonStr)
	{
		const TCHAR buf[4][12][20]={
			{_T("һ"),_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),_T("ʮ"),_T("ʮһ"),_T("ʮ��")},
			{_T("january"),_T("february"),_T("march"),_T("april"),_T("may"),_T("june"),_T("july"),_T("august"),_T("september"),_T("october"),_T("november"),_T("december")},
			{_T("jan"),_T("feb"),_T("mar"),_T("apr"),_T("may"),_T("jun"),_T("jul"),_T("aug"),_T("sep"),_T("oct"),_T("nov"),_T("dec")},
			{_T("1"),_T("2"),_T("3"),_T("4"),_T("5"),_T("6"),_T("7"),_T("8"),_T("9"),_T("10"),_T("11"),_T("12")}};
		sMonStr.MakeLower();
		//����ƥ��ǰ���
		for(int j=0;j<4;j++)
		{
			//��������Ϊʮһ��һ�ظ���ʮ���Ͷ��ظ��ȵ�
			for(int i=11;i>=0;i--)
			{
				if (sMonStr.Find(buf[j][i])!=-1)
					return i+1;
			}
		}
		return 0;
	}

	/**
	* @brief ��ʽ�����
	* @param sFormat		��ʽ���ַ���������vc��ʽ�����ڶ�����֧�ֲ��ã�����֧�ֺ��滻��Щ�ַ���
							[year] -> ��	[month] -> ��	[day] -> ��
							[hour] -> ʱ	[min] -> ��		[sec] -> ��		[msec] -> ����
	* @return ��ʽ���ַ���
	*/
	THString Format(const THString sFormat=_T("%Y-%m-%d %H:%M:%S")) const
	{
		//InitCom();
		UDATE ud;
		DOUBLE dt;
		SYSTEMTIME tmp=m_st;
		if (!SystemTimeToVariantTime(&tmp,&dt)) return _T("");
		if (S_OK != VarUdateFromDate(dt, 0, &ud)) return _T("");
		struct tm tmTemp;
		tmTemp.tm_sec	= ud.st.wSecond;
		tmTemp.tm_min	= ud.st.wMinute;
		tmTemp.tm_hour	= ud.st.wHour;
		tmTemp.tm_mday	= ud.st.wDay;
		tmTemp.tm_mon	= ud.st.wMonth - 1;
		tmTemp.tm_year	= ud.st.wYear - 1900;
		tmTemp.tm_wday	= ud.st.wDayOfWeek;
		tmTemp.tm_yday	= ud.wDayOfYear - 1;
		tmTemp.tm_isdst	= 0;

		THString strDate;
		LPTSTR lpszTemp = strDate.GetBufferSetLength(1024);
		_tcsftime(lpszTemp, 1024, sFormat, &tmTemp);
		strDate.ReleaseBuffer();
		strDate.Replace(_T("[year]"),_T("��"));
		strDate.Replace(_T("[month]"),_T("��"));
		strDate.Replace(_T("[day]"),_T("��"));
		strDate.Replace(_T("[hour]"),_T("ʱ"));
		strDate.Replace(_T("[min]"),_T("��"));
		strDate.Replace(_T("[sec]"),_T("��"));
		strDate.Replace(_T("[msec]"),_T("����"));
		return strDate;
	}
	THStringA FormatA(const THStringA sFormat="%Y-%m-%d %H:%M:%S") const
	{
		//InitCom();
		UDATE ud;
		DOUBLE dt;
		SYSTEMTIME tmp=m_st;
		if (!SystemTimeToVariantTime(&tmp,&dt)) return "";
		if (S_OK != VarUdateFromDate(dt, 0, &ud)) return "";
		struct tm tmTemp;
		tmTemp.tm_sec	= ud.st.wSecond;
		tmTemp.tm_min	= ud.st.wMinute;
		tmTemp.tm_hour	= ud.st.wHour;
		tmTemp.tm_mday	= ud.st.wDay;
		tmTemp.tm_mon	= ud.st.wMonth - 1;
		tmTemp.tm_year	= ud.st.wYear - 1900;
		tmTemp.tm_wday	= ud.st.wDayOfWeek;
		tmTemp.tm_yday	= ud.wDayOfYear - 1;
		tmTemp.tm_isdst	= 0;

		THStringA strDate;
		LPSTR lpszTemp = strDate.GetBufferSetLength(1024);
		strftime(lpszTemp, 1024, sFormat, &tmTemp);
		strDate.ReleaseBuffer();
		strDate.Replace("[year]","��");
		strDate.Replace("[month]","��");
		strDate.Replace("[day]","��");
		strDate.Replace("[hour]","ʱ");
		strDate.Replace("[min]","��");
		strDate.Replace("[sec]","��");
		strDate.Replace("[msec]","����");
		return strDate;
	}
	/**
	* @brief ��ȡ��Χ�ַ���
	* @param nType			��Χ����
	* @param nPreCount		��ǰ��ȡn����Χ
	* @param nAfterCount	����ȡn����Χ
	* @param ar				�����ַ�������
	*/
	void GetRangeString(int nType,int nPreCount,int nAfterCount,THStringArray &ar)
	{
		THString str;
		if (nType==RANGETYPE_MONTH)
		{
			int nYear=GetYear();
			int nMon=GetMonth();
			int tmpy=nYear;
			int tmpm=nMon;
			THStringArray tmpar;
			while(nPreCount>0)
			{
				tmpm--;
				if (tmpm<=0)
				{
					tmpm=12;
					tmpy--;
				}
				str.Format(_T("%d��%d��"),tmpy,tmpm);
				tmpar.Add(str);
				nPreCount--;
			}
			for(int i=0;i<tmpar.GetSize();i++)
				ar.Add(tmpar[tmpar.GetSize()-1-i]);
			str.Format(_T("%d��%d��"),nYear,nMon);
			ar.Add(str);
			tmpy=nYear;
			tmpm=nMon;
			while(nAfterCount>0)
			{
				tmpm++;
				if (tmpm>=13)
				{
					tmpm=1;
					tmpy++;
				}
				str.Format(_T("%d��%d��"),tmpy,tmpm);
				ar.Add(str);
				nAfterCount--;
			}
		}
		else if (nType==RANGETYPE_QUARTER)
		{
			int nYear=GetYear();
			int nMon=GetMonth();
			int q[13]={0,1,1,1,2,2,2,3,3,3,4,4,4};
			int quarter=q[nMon];
			TCHAR qch[][5]={_T(""),_T("����"),_T("�ļ�"),_T("�＾"),_T("����")};
			int tmpy=nYear;
			int tmpm=quarter;
			THStringArray tmpar;
			while(nPreCount>0)
			{
				tmpm--;
				if (tmpm<=0)
				{
					tmpm=4;
					tmpy--;
				}
				str.Format(_T("%d��%s"),tmpy,qch[tmpm]);
				tmpar.Add(str);
				nPreCount--;
			}
			for(int i=0;i<tmpar.GetSize();i++)
				ar.Add(tmpar[tmpar.GetSize()-1-i]);
			str.Format(_T("%d��%s"),nYear,qch[quarter]);
			ar.Add(str);
			tmpy=nYear;
			tmpm=quarter;
			while(nAfterCount>0)
			{
				tmpm++;
				if (tmpm>=5)
				{
					tmpm=1;
					tmpy++;
				}
				str.Format(_T("%d��%s"),tmpy,qch[tmpm]);
				ar.Add(str);
				nAfterCount--;
			}
		}
		else if (nType==RANGETYPE_YEAR)
		{
			int nYear=GetYear();
			int tmpy=nYear;
			THStringArray tmpar;
			while(nPreCount>0)
			{
				tmpy--;
				str.Format(_T("%d��"),tmpy);
				tmpar.Add(str);
				nPreCount--;
			}
			for(int i=0;i<tmpar.GetSize();i++)
				ar.Add(tmpar[tmpar.GetSize()-1-i]);
			str.Format(_T("%d��"),nYear);
			ar.Add(str);
			tmpy=nYear;
			while(nAfterCount>0)
			{
				tmpy++;
				str.Format(_T("%d��"),tmpy);
				ar.Add(str);
				nAfterCount--;
			}
		}
	}

	/**
	* @brief ��ȡ��Χ�ַ�����Ӧ��ʱ��
	* @param nType			��Χ����
	* @param str			ʱ���ַ���
	* @param st				��ʼʱ��
	* @param et				����ʱ��
	* @return				�����Ƿ���ȷ
	*/
	static BOOL GetRangeTimeByString(int nType,THString str,THTime *st,THTime *et)
	{
		if (nType==RANGETYPE_MONTH)
		{
			THTime ti;
			if (!ti.SetTime(str,_T("%Y��%m��"))) return FALSE;
			int year=ti.GetYear();
			int mon=ti.GetMonth();
			int nyear=year;
			int nmon=mon+1;
			if (nmon>=13)
			{
				nmon=1;
				nyear++;
			}
			if (st) st->SetTime(year,mon,1,0,0,0);
			if (et)
			{
				et->SetTime(nyear,nmon,1,0,0,0);
				THTimeSpan sec(0,0,0,1);
				*et-=sec;
			}
		}
		else if (nType==RANGETYPE_QUARTER)
		{
			THTime ti;
			if (!ti.SetTime(str,_T("%Y��"))) return FALSE;
			int year=ti.GetYear();
			TCHAR qch[][5]={_T(""),_T("����"),_T("�ļ�"),_T("�＾"),_T("����")};
			int nqch=0;
			for(int i=1;i<=4;i++)
			{
				if (str.Find(qch[i],0)!=-1)
				{
					nqch=i;
					break;
				}
			}
			if (nqch==0) return FALSE;
			int nyear=year;
			int nnqch=nqch+1;
			if (nnqch>=5)
			{
				nnqch=1;
				nyear++;
			}
			nqch=(nqch-1)*3+1;
			nnqch=(nnqch-1)*3+1;
			if (nnqch>=13)
			{
				nnqch=1;
				nyear++;
			}
			if (st) st->SetTime(year,nqch,1,0,0,0);
			if (et)
			{
				et->SetTime(nyear,nnqch,1,0,0,0);
				THTimeSpan sec(0,0,0,1);
				*et-=sec;
			}
		}
		else if (nType==RANGETYPE_YEAR)
		{
			THTime ti;
			if (!ti.SetTime(str,_T("%Y��"))) return FALSE;
			int year=ti.GetYear();
			int nyear=year+1;
			if (st) st->SetTime(year,1,1,0,0,0);
			if (et)
			{
				et->SetTime(nyear,1,1,0,0,0);
				THTimeSpan sec(0,0,0,1);
				*et-=sec;
			}
		}
		return TRUE;
	}

	int GetYear(){return m_st.wYear;}
	int GetMonth(){return m_st.wMonth;}
	int GetDay(){return m_st.wDay;}
	int GetDayOfYear()
	{
		UDATE udate;
		if (GetTimeAsUdate(udate))
			return udate.wDayOfYear;
		return 0;
	}
	int GetWeekOfYear()
	{
		return (GetDayOfYear()+6)/7;//��6Ϊ�˴���intȡ����С����ʧ
	}

	typedef enum _THWeekDay
	{
		Sunday=0,
		Monday,
		Tuesday,
		Wednesday,
		Thursday,
		Friday,
		Saturday
	}THWeekDay;

	inline THWeekDay GetDayOfWeek(){return (THWeekDay)m_st.wDayOfWeek;}
	inline int GetHour(){return m_st.wHour;}
	inline int GetMinute(){return m_st.wMinute;}
	inline int GetSecond(){return m_st.wSecond;}
	inline int GetMilliseconds(){return m_st.wMilliseconds;}

	double DoubleFromDate( DATE date )
	{
		double fTemp;
		// No problem if positive
		if( date >= 0 ) return( date );
		// If negative, must convert since negative dates not continuous
		// (examples: -1.25 to -.75, -1.50 to -.50, -1.75 to -.25)
		fTemp = ceil( date );
		return( fTemp-(date-fTemp) );
	}

	DATE DateFromDouble( double f )
	{
		double fTemp;
		// No problem if positive
		if( f >= 0 ) return( f );
		// If negative, must convert since negative dates not continuous
		// (examples: -.75 to -1.25, -.50 to -1.50, -.25 to -1.75)
		fTemp = floor( f ); // fTemp is now whole part
		return( fTemp+(fTemp-f) );
	}

	void Seek(LONG lDays,int nHours,int nMinutes,int nSecords)
	{
		THTimeSpan span(lDays,nHours,nMinutes,nSecords);
		(*this)+=span;
	}

	//��λminutes
	static LONG GetLocalTimeZone()
	{
		TIME_ZONE_INFORMATION tzinfo;
		GetTimeZoneInformation(&tzinfo);
		return tzinfo.Bias;
	}

	THTime& operator=(const VARIANT& varSrc)
	{
		//InitCom();
		DOUBLE dt;
		if (varSrc.vt != VT_DATE)
		{
			VARIANT varDest;
			varDest.vt = VT_EMPTY;
			if(SUCCEEDED(::VariantChangeType(&varDest, const_cast<VARIANT *>(&varSrc), 0, VT_DATE)))
				dt = varDest.date;
		}
		else
			dt = varSrc.date;
		VariantTimeToSystemTime(dt,&m_st);
		return *this;
	}

	THTime& operator=(DATE dtSrc)
	{
		//InitCom();
		VariantTimeToSystemTime(dtSrc,&m_st);
		return *this;
	}

	THTime& operator=(const __time64_t& timeSrc)
	{
		SetTime(timeSrc,TRUE);
		return *this;
	}

	THTime &operator=(const SYSTEMTIME &systimeSrc)
	{
		m_st=systimeSrc;
		return *this;
	}

	THTime &operator=(const FILETIME &filetimeSrc)
	{
		::FileTimeToSystemTime(&filetimeSrc, &m_st);
		return *this;
	}

	THTime &operator=(const UDATE &udate)
	{
		//InitCom();
		DOUBLE dt;
		VarDateFromUdate((UDATE*)&udate, 0, &dt);
		VariantTimeToSystemTime(dt,&m_st);
		return *this;
	}

	bool operator==(THTime& date )
	{
		//InitCom();
		DOUBLE dt,dt2;
		SystemTimeToVariantTime(&m_st,&dt);
		SystemTimeToVariantTime(&date.m_st,&dt2);
		return( dt == dt2 );
	}

	bool operator!=(THTime& date )
	{
		return !operator==(date);
	}

	bool operator<(THTime& date )
	{
		//InitCom();
		DOUBLE dt,dt2;
		SystemTimeToVariantTime(&m_st,&dt);
		SystemTimeToVariantTime(&date.m_st,&dt2);
		return( DoubleFromDate( dt ) < DoubleFromDate( dt2 ) );
	}

	bool operator>(THTime& date )
	{
		//InitCom();
		DOUBLE dt,dt2;
		SystemTimeToVariantTime(&m_st,&dt);
		SystemTimeToVariantTime(&date.m_st,&dt2);
		return( DoubleFromDate( dt ) > DoubleFromDate( dt2 ) );
	}

	bool operator<=(THTime& date )
	{
		return operator<(date) || operator==(date);
	}

	bool operator>=(THTime& date )
	{
		return operator>(date) || operator==(date);
	}

	THTime operator+( THTimeSpan dateSpan )
	{
		//InitCom();
		DOUBLE dt;
		SystemTimeToVariantTime(&m_st,&dt);
		return( THTime( DateFromDouble( DoubleFromDate( dt )+(double)dateSpan ) ) );
	}

	THTime operator-( THTimeSpan dateSpan )
	{
		//InitCom();
		DOUBLE dt;
		SystemTimeToVariantTime(&m_st,&dt);
		return( THTime( DateFromDouble( DoubleFromDate( dt )-(double)dateSpan ) ) );
	}

	THTime& operator+=( THTimeSpan dateSpan )
	{
		//InitCom();
		DOUBLE dt;
		SystemTimeToVariantTime(&m_st,&dt);
		dt = DateFromDouble( DoubleFromDate( dt )+(double)dateSpan );
		VariantTimeToSystemTime(dt,&m_st);
		return( *this );
	}

	THTime& operator-=( THTimeSpan dateSpan )
	{
		//InitCom();
		DOUBLE dt;
		SystemTimeToVariantTime(&m_st,&dt);
		dt = DateFromDouble( DoubleFromDate( dt )-(double)dateSpan );
		VariantTimeToSystemTime(dt,&m_st);
		return( *this );
	}

	THTimeSpan operator-(THTime& date)
	{
		//InitCom();
		DOUBLE dt,dt2;
		SystemTimeToVariantTime(&m_st,&dt);
		SystemTimeToVariantTime(&date.m_st,&dt2);
		return DoubleFromDate(dt) - DoubleFromDate(dt2);
	}

	void SetSystemTime()
	{
		::SetSystemTime(&m_st);
	}

	operator DATE()
	{
		//InitCom();
		DOUBLE dt;
		SystemTimeToVariantTime(&m_st,&dt);
		return(dt);
	}
};