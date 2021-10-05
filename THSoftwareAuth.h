#pragma once

#include <THString.h>
#include <THINet.h>
#include <THArith.h>
#include <THSysInfo.h>

typedef enum _THAuthState{
	AuthState_None,			//<<<δ��ʼ
	AuthState_Processing,	//<<<������
	AuthState_Success,		//<<<�ɹ�
	AuthState_NetWorkFail,	//<<<���粻�ܷ���
	AuthState_Fail			//<<<ʧ��
}THAuthState;

class THSoftwareAuth;

/**
* @brief �����Ȩ�����Ӧ��
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-05-26 �½���
*/
class ITHSoftwareAuthResult
{
public:
	ITHSoftwareAuthResult(){}
	virtual ~ITHSoftwareAuthResult(){}

	virtual void OnAuthResult(THSoftwareAuth *cls,THAuthState Result,THString strErrMsg,void *adddata)=0;
	//״̬��AuthState_Success��AuthState_Fail��AuthState_NetWorkFail���֣��ɹ�ʱstrErrMsgΪ��Ȩ��
	virtual void OnDemoRequestResult(THSoftwareAuth *cls,THAuthState Result,THString strErrMsg,void *adddata){}
};

#define ID_CHECKAUTH_REQUEST	1

/**
* @brief �����Ȩ���Ӧ����
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-05-22 �½���
*/
/**<pre>
ʹ��Sample��
	�������ܹ���
		http://auth.tansoft.cn/auth/moduleman.asp		����ģ�����
		http://auth.tansoft.cn/auth/limitman.asp		��Ȩ�����
		http://auth.tansoft.cn/auth/reqestdemo.asp		��������
		http://auth.tansoft.cn/auth/requestauth.asp		��Ȩ����
	��������ṹ��
		Auth_Module
			ModuleId	PublicKey	PrivateKey
		Auth_Limit
			Code	sid	Type			Value1				Value2
					1:IsUse				0,1�Ƿ�ʹ��		ԭ�� & note
					10:HardCode			HardCode			�ύʱ��
					40:HardCodeCount	HardCode			��֤����
					50:LimitUseCount	LimitCount
					20:LimitTime		Time
					30:LimitIpRange		StartIp				EndIp
			(empty)	1:AllowTry			0,1�Ƿ���������	ԭ�� & note
			(empty)	20:LimitTime		����ʱ������
			(empty)	40:HardCodeCount	HardCode			��֤����
										(empty)				���Ƹ���
			(empty)	50:LimitUseCount	LimitCount
			(empty)	30:LimitIpRange		StartIp				EndIp
			(empty)	10:HardCode			HardCode			�ύʱ��

		Auth_Log
			Code	sid	Result		smap	module	ip	return	LogDate
					0�ɹ�
					nʧ������
	1:IsUse
	10:HardCode
	20:LimitTime
	30:LimitIpRange
	40:HardCodeCount
	50:LimitUseCount
	//�������ʱ�����밴Type���У���Ϊ��Ҫȷ������ͨ��ʱ���ż���ͳ�Ƽ����ȡ�

	��Ȩ����Ȩ��AuthKey��ʽ�ṩ��һ����Ȩ��ɼ�������Ʒ����Ȩ��ʽ
	AuthKey:
		Hex(0x4,Module) Hex(0x4,PublicKey) Code
	�½����̣�
	1.ͨ��rsatest �½�����ȷ�ӽ��ܵ�����ֵ��¼��Module�⡣
	2.����µ���Ȩ����ʱ����Ҫ��Module����ѡ����ܶԣ���soft����ѡ��sid���������ⳤ�ȵ�Code�����ø�����Ȩ���Ƽ��ɡ�
	��֤��Ȩ���̣�
	1.�ͻ��˰�AuthKey���룬��Module|PublicKey�����������ݺ������������
		smap=Encode(Module,PublicKey,Code|sid|cModule|cPublicKey|HardCode)
		��Ȩ������|�������id|�ͻ���ģ��id|�ͻ��˹�Կ|�ͻ���Ӳ��id
		auth/requestauth.asp?s={smap}&m={Module}
	2.�������յ���ͨ��Module��ȡ��Ӧ��˽Կ���Լ������ݽ��н��ܣ���Code���ж��ν��ܡ�
	3.ͨ����Ȩ���ȡ��Ȩ���ݡ�

	Ӳ����Ȩ�뷽ʽ(HardCodeLimit):
	��װ�ͻ���������ʽ(HardCodeCountLimit):
		ͨ���ϱ�Ӳ��id��code��Ȩ��
	�������Ʒ�ʽ(UseCountLimit):
	ʱ�����Ʒ�ʽ(TimeLimit):
	Ip���Ʒ�ʽ(IpRangeLimit):
</pre>*/
class THSoftwareAuth : private THDownloadEventHandler
{
public:
	THSoftwareAuth()
	{
		m_http.SetHandler(this);
		m_crsa.GenKey();
		m_state=AuthState_None;
		m_handler=NULL;
		m_adddata=NULL;
		m_threadid=0;
		m_timer.SetCallFunc(TimerCB);
	}
	virtual ~THSoftwareAuth()
	{
	}

	void SetEventHandler(ITHSoftwareAuthResult *handler,void *adddata){m_handler=handler;m_adddata=adddata;}

	/**
	* @brief ��ʼ��Ȩ����
	* @param sid		����id
	* @param sAuthKey	��Ȩ��
	* @param ini		������Ȩ��Ϣ��ini�����sAuthKeyΪ�գ���ini�ж�ȡ��ע��ini�������crcģʽ�ᱻ����
	* @return �Ƿ�ɹ���ʼ
	*/
	BOOL StartCheckAuth(UINT sid,THString sAuthKey,ITHIni *ini=NULL,UINT nMustModule=0,UINT nMustPublicKey=0)
	{
		if (ini)
		{
			ini->InitPass(_T("THSoftwareAuth"));
			ini->UseEncodeCrcMode();
		}
		if (sAuthKey.IsEmpty() && ini) sAuthKey=ini->GetString(_T("THSoftwareAuth"),_T("AuthKey"));
		if (sAuthKey.IsEmpty() || sAuthKey.GetLength()<=8) return FALSE;
		UINT nModule=THStringConv::s2h(sAuthKey.Left(4));
		UINT nPublicKey=THStringConv::s2h(sAuthKey.Left(8).Mid(4));
		if (nMustModule!=0 && nMustModule!=nModule) return FALSE;
		if (nMustPublicKey!=0 && nMustPublicKey!=nPublicKey) return FALSE;
		sAuthKey=sAuthKey.Mid(8);
		m_srsa.SetKey(nPublicKey,0,nModule);
		THString str;
		str.Format(_T("%s|%u|%u|%u|%s"),sAuthKey,sid,m_crsa.GetModule(),m_crsa.GetPublicKey(),GetHardCode());
		char *tmp=THCharset::t2a(str);
		if (!tmp) return FALSE;
		str=m_srsa.Encode(tmp,(unsigned int)strlen(tmp));
		THCharset::free(tmp);
		THString url;
		srand((unsigned int)time(NULL));
		url.Format(_T("http://auth.tansoft.cn/auth/requestauth.asp?s=%s&m=%u&t=%u"),str,nModule,rand());
		m_state=AuthState_Processing;
		return ((m_threadid=m_http.StartMemDownload(url,4096,(void *)ID_CHECKAUTH_REQUEST))!=0);
	}

	/**
	* @brief ��ʼ������������
	* @param sid			����id
	* @param ini			������Ȩ��Ϣ��ini��ע��ini�������crcģʽ�ᱻ����
	* @param nEncodeModule	ָ������ģ�����ƣ�0Ϊ��ָ��
	* @return �Ƿ�ɹ���ʼ
	*/
	BOOL StartDemoCodeQuery(UINT sid,ITHIni *ini=NULL,UINT nEncodeModule=0)
	{
		THString url;
		url.Format(_T("http://auth.tansoft.cn/auth/requestdemo.asp?sid=%u"),sid);
		if (nEncodeModule!=0)
			url.AppendFormat(_T("&m=%u"),nEncodeModule);
		return (m_http.StartMemDownload(url,4096,ini)!=0);
	}

	/**
	* @brief ��ȡ��Ȩ���״̬
	* @return ��Ȩ״̬
	*/
	THAuthState GetAuthState(){return m_state;}

	/**
	* @brief ������Ȩ���״̬
	*/
	void SetAuthState(THAuthState state){m_state=state;}

	/**
	* @brief �����Ȩʧ�ܣ���ʾ��Ϣ�����֮ǰû�п�ʼ��Ȩ����һֱ�ȴ���Ȩ�������
	* @param sContent		����
	* @param sTitle			����
	* @param nIconType		ͼ������
	* @param hWnd			�����ھ��
	* @param ret			�Ի��򷵻���
	* @param bShowIfNetWorkFail	�����޷�����ʱ���Ƿ���ʾ�Ի���
	* @return �Ƿ���Ȩ�ɹ�
	*/
	BOOL ShowMessageBoxIfRequire(THString sContent,THString sTitle=_T("��Ȩʧ��"),UINT nIconType=MB_ICONSTOP,HWND hWnd=NULL,int *ret=NULL,BOOL bShowIfNetWorkFail=TRUE)
	{
		while(m_state==AuthState_Processing){Sleep(500);}
		if (m_state==AuthState_Fail
			|| (m_state==AuthState_NetWorkFail && bShowIfNetWorkFail))
		{
			int r=MessageBox(hWnd,sContent,sTitle,nIconType);
			if (ret) *ret=r;
		}
		return m_state==AuthState_Success;
	}

	/**
	* @brief ��ʾ������Ϣ������TimeOut�����Ի���󣬹رճ���
	* @param sContent		����
	* @param sTitle			����
	* @param hWnd			�����ھ��
	* @param nIconType		ͼ������
	* @param nTimeOut		��ʱ
	*/
	void TimeOutQuitMessageBox(THString sContent,THString sTitle,HWND hWnd=NULL,UINT nIconType=MB_OK|MB_ICONSTOP|MB_TOPMOST,UINT nTimeOut=10000)
	{
		//��ʱ����ʼʧ�ܣ�ֱ���˳�
		if (m_timer.StartTimer(0,nTimeOut)==0) exit(0);
		MessageBox(hWnd,sContent,sTitle,nIconType);
		exit(0);
	}
	/**
	* @brief ��ȡ����������
	* @return ������
	*/
	virtual THString GetHardCode()
	{
		if (m_HardCode.IsEmpty()) m_HardCode=THSysInfo::MakeUnqiHardCode();
		return m_HardCode;
	}
	void SetHardCode(THString HardCode){m_HardCode=HardCode;}
private:
	virtual void OnDownloadFinish(const THString url,class THINetHttp *cls,void *adddata,int ErrCode,unsigned int down,const THString filename,const void *data,unsigned int len)
	{
		THString ret;
		if (adddata==(void *)ID_CHECKAUTH_REQUEST)
		{
			m_threadid=0;
			if (ErrCode==0)
			{
				THString retxml=THCharset::a2t((const char *)data,len);
				retxml=THSimpleXml::GetParam(retxml,_T("u"));
				if (retxml.IsEmpty())
				{
					m_state=AuthState_NetWorkFail;
					ret=_T("�޷���������");
				}
				else
				{
					unsigned int len=0;
					void *buf=m_crsa.Decode(retxml,&len);
					if (buf==NULL || len==0)
					{
						m_state=AuthState_Fail;
						ret=_T("��Ȩʧ��");
						if (buf) m_crsa.Free(buf);
					}
					else
					{
						retxml=THCharset::a2t((const char *)buf,len);
						m_crsa.Free(buf);
						THStringToken token(retxml,_T("|"));
						UINT m=THs2u(token.GetNextToken());
						UINT p=THs2u(token.GetNextToken());
						int nret=THs2i(token.GetNextToken());
						ret=THUrlExplain::UrlDecode(token.GetNextToken());
						THString timestr=token.GetNextToken();
						if (m!=m_crsa.GetModule() || p!=m_crsa.GetPublicKey())
						{
							m_state=AuthState_Fail;
						}
						else
						{
							if (nret==0)
							{
								m_state=AuthState_Success;
							}
							else
							{
								m_state=AuthState_Fail;
							}
						}
					}
				}
			}
			else
			{
				m_state=AuthState_NetWorkFail;
				ret=_T("�޷���������");
			}
			if (m_handler) m_handler->OnAuthResult(this,m_state,ret,m_adddata);
		}
		else
		{
			THAuthState state;
			ITHIni *ini=(ITHIni *)adddata;
			if (ErrCode==0)
			{
				THString retxml=THCharset::a2t((const char *)data,len);
				retxml=THSimpleXml::GetParam(retxml,_T("u"));
				if (retxml.IsEmpty())
				{
					state=AuthState_NetWorkFail;
					ret=_T("�޷���������");
				}
				else if (retxml==_T("0"))
				{
					state=AuthState_Fail;
					ret=_T("����ʧ��");
				}
				else
				{
					state=AuthState_Success;
					ret=retxml;
					if (ini)
					{
						ini->InitPass(_T("THSoftwareAuth"));
						ini->UseEncodeCrcMode();
						ini->WriteString(_T("THSoftwareAuth"),_T("AuthKey"),retxml);
					}
				}
			}
			else
			{
				state=AuthState_NetWorkFail;
				ret=_T("�޷���������");
			}
			if (m_handler) m_handler->OnDemoRequestResult(this,state,ret,m_adddata);
		}
	}
	static void TimerCB(int TimerId,THTimerThread *pCls,void *data)
	{
		exit(0);
	}

	THINetHttp m_http;
	//���ڿͻ������ݼӽ���
	THTiniRsa m_crsa;
	//���ڷ��������ݵļ���
	THTiniRsa m_srsa;
	volatile THAuthState m_state;
	ITHSoftwareAuthResult *m_handler;
	void *m_adddata;
	volatile UINT m_threadid;
	THString m_HardCode;
	THTimerThread m_timer;
};