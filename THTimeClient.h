#pragma once

#include <THRawSocketModel.h>
#include <THTime.h>

#define __HIGHTIME		21968699 // 21968708 // Jan 1, 1900 FILETIME.highTime
#define __LOWTIME		4259332096 // 1604626432 // Jan 1, 1900 FILETIME.lowtime

/**
* @brief 对时客户端类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2009-04-09 新建类
*/
/**<pre>
	//RFC 868，不支持RFC 2030 SNTP
	THTimeClient client;
	THTime ti;
	if (client.GetTime(&ti))
	{
		//update local datetime
		ti.SetSystemTime();
		THDebug(_T("%s"),ti.Format());
	}
</pre>*/
class THTimeClient/*:private ITHRawSocketBufferedEventHandler*/
{
public:
	THTimeClient()
	{
	}
	virtual ~THTimeClient()
	{
	}
	/*virtual UINT IsBufferReady(THRawSocket *cls,RAWSOCKET sock,const void *bufdata,int buflen,struct sockaddr_in *paddr,void *adddata)
	{
		if (buflen>=4) return TRUE;
		return FALSE;
	}
	virtual BOOL BufferedReceiveHandler(THRawSocket *cls,RAWSOCKET sock,const void *bufdata,int buflen,struct sockaddr_in *paddr,void *adddata)
	{
		U32 ti=ntohl(*(const U32 *)bufdata);
		return TRUE;
	}*/
	BOOL GetTime(THTime *ti,THString server=_T(""),BOOL bAutoTryOther=TRUE)
	{
		if (!ti) return FALSE;
		THString ret;
		THStringArray ar;
		ar.Add(_T("nist1.uccaribe.edu"));//  200.50.25.62  Bayamon, Puerto Rico  Ok  
		ar.Add(_T("nist1-ny.ustiming.org"));//  64.90.182.55  New York City, NY  Recommended for new users  
		ar.Add(_T("time-a.nist.gov"));//  129.6.15.28  NIST, Gaithersburg, Maryland  Ok  
		ar.Add(_T("time-b.nist.gov"));//  129.6.15.29  NIST, Gaithersburg, Maryland  Ok  
		ar.Add(_T("nist1-dc.ustiming.org"));//  206.246.118.250  Alexandria, Virginia  Recommended for new users  
		ar.Add(_T("nist1.aol-va.symmetricom.com"));//  64.236.96.53  Reston, Virginia  Recommended for new users  
		ar.Add(_T("nist1.columbiacountyga.gov"));//  68.216.79.113  Columbia County, Georgia  Recommended for new users  
		ar.Add(_T("nist1-chi.ustiming.org"));//  208.66.175.36  Chicago, Illinois  Recommended for new users  
		ar.Add(_T("nist.expertsmi.com"));//  173.14.47.149  Monroe, Michigan  Recommended for new users  
		ar.Add(_T("nist.netservicesgroup.com"));//  64.113.32.5  Southfield, Michigan  Recommended for new users  
		ar.Add(_T("time-a.timefreq.bldrdoc.gov"));//  132.163.4.101  NIST, Boulder, Colorado  Ok  
		ar.Add(_T("time-b.timefreq.bldrdoc.gov"));//  132.163.4.102  NIST, Boulder, Colorado  Very busy  
		ar.Add(_T("time-c.timefreq.bldrdoc.gov"));//  132.163.4.103  NIST, Boulder, Colorado  ok  
		ar.Add(_T("time.nist.gov"));//  192.43.244.18  NCAR, Boulder, Colorado  Ok  
		ar.Add(_T("utcnist.colorado.edu"));//  128.138.140.44  University of Colorado, Boulder  ok  
		ar.Add(_T("utcnist2.colorado.edu"));//  128.138.188.172  University of Colorado, Boulder  Recommended for new users  
		ar.Add(_T("ntp-nist.ldsbc.edu"));//  198.60.73.8  LDSBC, Salt Lake City, Utah  Recommended for new users  
		ar.Add(_T("time-nw.nist.gov"));//  131.107.13.100  Microsoft, Redmond, Washington  ok  
		ar.Add(_T("nist1.aol-ca.symmetricom.com"));//  207.200.81.113  Mountain View, California  Recommended for new users  
		ar.Add(_T("nist1.symmetricom.com"));//  69.25.96.13  San Jose, California  Recommended for new users  
		ar.Add(_T("nist1-sj.ustiming.org"));//  64.125.78.85  San Jose, California  Recommended for new users  
		ar.Add(_T("nist1-la.ustiming.org"));//  64.147.116.229  Los Angeles, California  Recommended for new users  
		if (!server.IsEmpty()) ar.Add(server);
		int i=(int)ar.GetSize()-1;
		for(;i>=0;i--)
		{
			U16 prt;
			U32 ip=THNetWork::GetIpFromHostName(ar[i],&prt);
			if (ip!=0)
			{
				if (prt==0) prt=IPPORT_TIMESERVER;
				RAWSOCKET s=m_s.CreateSocket(ip,prt,0,RawProtocol_Tcp,FALSE);
				if (s!=INVALID_SOCKET)
				{
					U32 uti;
					int r=m_s.ReceiveFor(s,&uti,4);
					m_s.FreeSocket(s);
					if (r>0)
					{
						uti=ntohl(uti);
						UINT64 uiCurTime, uiBaseTime, uiResult;
						uiBaseTime = ((UINT64) __HIGHTIME << 32) + __LOWTIME;
						uiCurTime = (UINT64)uti * (UINT64)10000000;
						uiResult = uiBaseTime + uiCurTime;
						*ti=*((FILETIME *)&uiResult);
						return TRUE;
					}
				}
			}
			if (!bAutoTryOther) break;
		}
		return FALSE;
	}
private:
	THRawSocket m_s;
};
