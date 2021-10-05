#pragma once

#include <THRawSocketModel.h>

/**
* @brief Whois客户端类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2009-04-09 新建类
*/
/**<pre>
CString ret=m_client.Whois(host,_T("whois.cnnic.cn"),43);
CString status=m_client.GetDomainStatus(ret);
CString reg=m_client.GetRegOrg(ret);
</pre>*/
class THWhoisClient/*:private ITHRawSocketEventHandler*/
{
public:
	THWhoisClient()
	{
		//m_s.SetHandler(this,NULL);
	}
	virtual ~THWhoisClient()
	{
	}
	/*virtual BOOL ReceiveHandler(THRawSocket *cls,RAWSOCKET sock,const void *bufdata,int buflen,struct sockaddr_in *paddr,void *adddata)
	{
		THString result=THCharset::u82t((const char *)bufdata,buflen);
		//MessageBox(NULL,result,_T("d"),0);
		return TRUE;
	}
	virtual void ErrorHandler(THRawSocket *cls,RAWSOCKET sock,DWORD errcode,void *adddata)
	{
	}*/
	THString FindValue(THString data,THString key,int idx=0)
	{
		key+=_T(":");
		data.Replace(_T("\r\n"),_T("\n"));
		THStringToken t(data,_T("\n"));
		THString str;
		while(t.IsMoreTokens())
		{
			str=t.GetNextToken();
			if (str.Find(key)==0)
			{
				if (idx==0)
					return str.Mid(key.GetLength()).Trim();
				idx--;
			}
		}
		return _T("");
	}
	inline THString GetDomainName(THString data){return FindValue(data,_T("Domain Name"));}
	inline THString GetROID(THString data){return FindValue(data,_T("ROID"));}
	inline THString GetDomainStatus(THString data){return FindValue(data,_T("Domain Status"));}
	inline THString GetRegOrg(THString data){return FindValue(data,_T("Registrant Organization"));}
	inline THString GetRegName(THString data){return FindValue(data,_T("Registrant Name"));}
	inline THString GetAdminEmail(THString data){return FindValue(data,_T("Administrative Email"));}
	inline THString GetSponReg(THString data){return FindValue(data,_T("Sponsoring Registrar"));}
	inline THString GetNameServer(THString data,int idx=0){return FindValue(data,_T("Name Server"),idx);}
	inline THString GetRegDate(THString data){return FindValue(data,_T("Registration Date"));}
	inline THString GetExpDate(THString data){return FindValue(data,_T("Expiration Date"));}
	THString Whois(THString hostname,THString whoissvr,U16 sport=IPPORT_WHOIS)
	{
		THString ret;
		RAWSOCKET s=m_s.CreateSocket(THNetWork::GetIpFromHostName(whoissvr,NULL),sport,0,RawProtocol_Tcp,FALSE);
		if (s!=INVALID_SOCKET)
		{
			char *h=THCharset::t2a(hostname+_T("\r\n"));
			if (h)
			{
				if (m_s.Send(s,h,(int)strlen(h)))
				{
					char buf[4096];
					int r=m_s.ReceiveFor(s,buf,4096);
					if (r>0)
						ret=THCharset::u82t((const char *)buf,r);
				}
				THCharset::free(h);
			}
			m_s.FreeSocket(s);
		}
		return ret;
	}
private:
	THRawSocket m_s;
};
