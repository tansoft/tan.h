#pragma once

#include <THString.h>
#include <THINet.h>
#include <THArith.h>
#include <THSysInfo.h>

typedef enum _THAuthState{
	AuthState_None,			//<<<未开始
	AuthState_Processing,	//<<<进行中
	AuthState_Success,		//<<<成功
	AuthState_NetWorkFail,	//<<<网络不能访问
	AuthState_Fail			//<<<失败
}THAuthState;

class THSoftwareAuth;

/**
* @brief 软件授权结果响应类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-05-26 新建类
*/
class ITHSoftwareAuthResult
{
public:
	ITHSoftwareAuthResult(){}
	virtual ~ITHSoftwareAuthResult(){}

	virtual void OnAuthResult(THSoftwareAuth *cls,THAuthState Result,THString strErrMsg,void *adddata)=0;
	//状态有AuthState_Success，AuthState_Fail，AuthState_NetWorkFail三种，成功时strErrMsg为授权码
	virtual void OnDemoRequestResult(THSoftwareAuth *cls,THAuthState Result,THString strErrMsg,void *adddata){}
};

#define ID_CHECKAUTH_REQUEST	1

/**
* @brief 软件授权许可应用类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-05-22 新建类
*/
/**<pre>
使用Sample：
	服务器架构：
		http://auth.tansoft.cn/auth/moduleman.asp		加密模组管理
		http://auth.tansoft.cn/auth/limitman.asp		授权码管理
		http://auth.tansoft.cn/auth/reqestdemo.asp		试用申请
		http://auth.tansoft.cn/auth/requestauth.asp		授权请求
	服务器表结构：
		Auth_Module
			ModuleId	PublicKey	PrivateKey
		Auth_Limit
			Code	sid	Type			Value1				Value2
					1:IsUse				0,1是否使用		原因 & note
					10:HardCode			HardCode			提交时间
					40:HardCodeCount	HardCode			认证内容
					50:LimitUseCount	LimitCount
					20:LimitTime		Time
					30:LimitIpRange		StartIp				EndIp
			(empty)	1:AllowTry			0,1是否允许试用	原因 & note
			(empty)	20:LimitTime		限制时间天数
			(empty)	40:HardCodeCount	HardCode			认证内容
										(empty)				限制个数
			(empty)	50:LimitUseCount	LimitCount
			(empty)	30:LimitIpRange		StartIp				EndIp
			(empty)	10:HardCode			HardCode			提交时间

		Auth_Log
			Code	sid	Result		smap	module	ip	return	LogDate
					0成功
					n失败类型
	1:IsUse
	10:HardCode
	20:LimitTime
	30:LimitIpRange
	40:HardCodeCount
	50:LimitUseCount
	//检验规则时，必须按Type排列，因为需要确保规则都通过时，才减少统计计数等。

	授权以授权码AuthKey方式提供，一个授权码可激活多个产品和授权方式
	AuthKey:
		Hex(0x4,Module) Hex(0x4,PublicKey) Code
	新建流程：
	1.通过rsatest 新建可正确加解密的三个值，录入Module库。
	2.添加新的授权规则时，需要在Module库中选择加密对，在soft库中选择sid，输入任意长度的Code，设置各种授权限制即可。
	验证授权流程：
	1.客户端把AuthKey分离，用Module|PublicKey加密以下数据后请求服务器。
		smap=Encode(Module,PublicKey,Code|sid|cModule|cPublicKey|HardCode)
		授权的内容|软件服务id|客户端模块id|客户端公钥|客户端硬件id
		auth/requestauth.asp?s={smap}&m={Module}
	2.服务器收到后，通过Module获取对应的私钥，对加密内容进行解密，对Code进行二次解密。
	3.通过授权表获取授权内容。

	硬件授权码方式(HardCodeLimit):
	安装客户端数量方式(HardCodeCountLimit):
		通过上报硬件id和code授权，
	次数限制方式(UseCountLimit):
	时间限制方式(TimeLimit):
	Ip限制方式(IpRangeLimit):
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
	* @brief 开始授权请求
	* @param sid		服务id
	* @param sAuthKey	授权码
	* @param ini		保存授权信息的ini，如果sAuthKey为空，从ini中读取，注意ini的密码和crc模式会被更改
	* @return 是否成功开始
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
	* @brief 开始试用申请请求
	* @param sid			服务id
	* @param ini			保存授权信息的ini，注意ini的密码和crc模式会被更改
	* @param nEncodeModule	指定加密模块名称，0为不指定
	* @return 是否成功开始
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
	* @brief 获取授权检查状态
	* @return 授权状态
	*/
	THAuthState GetAuthState(){return m_state;}

	/**
	* @brief 设置授权检查状态
	*/
	void SetAuthState(THAuthState state){m_state=state;}

	/**
	* @brief 如果授权失败，显示信息，如果之前没有开始授权请求，一直等待授权请求完成
	* @param sContent		内容
	* @param sTitle			标题
	* @param nIconType		图标类型
	* @param hWnd			父窗口句柄
	* @param ret			对话框返回码
	* @param bShowIfNetWorkFail	网络无法访问时，是否显示对话框
	* @return 是否授权成功
	*/
	BOOL ShowMessageBoxIfRequire(THString sContent,THString sTitle=_T("授权失败"),UINT nIconType=MB_ICONSTOP,HWND hWnd=NULL,int *ret=NULL,BOOL bShowIfNetWorkFail=TRUE)
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
	* @brief 显示出错信息，并在TimeOut或点击对话框后，关闭程序
	* @param sContent		内容
	* @param sTitle			标题
	* @param hWnd			父窗口句柄
	* @param nIconType		图标类型
	* @param nTimeOut		超时
	*/
	void TimeOutQuitMessageBox(THString sContent,THString sTitle,HWND hWnd=NULL,UINT nIconType=MB_OK|MB_ICONSTOP|MB_TOPMOST,UINT nTimeOut=10000)
	{
		//定时器初始失败，直接退出
		if (m_timer.StartTimer(0,nTimeOut)==0) exit(0);
		MessageBox(hWnd,sContent,sTitle,nIconType);
		exit(0);
	}
	/**
	* @brief 获取本机机器码
	* @return 机器码
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
					ret=_T("无法访问网络");
				}
				else
				{
					unsigned int len=0;
					void *buf=m_crsa.Decode(retxml,&len);
					if (buf==NULL || len==0)
					{
						m_state=AuthState_Fail;
						ret=_T("授权失败");
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
				ret=_T("无法访问网络");
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
					ret=_T("无法访问网络");
				}
				else if (retxml==_T("0"))
				{
					state=AuthState_Fail;
					ret=_T("申请失败");
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
				ret=_T("无法访问网络");
			}
			if (m_handler) m_handler->OnDemoRequestResult(this,state,ret,m_adddata);
		}
	}
	static void TimerCB(int TimerId,THTimerThread *pCls,void *data)
	{
		exit(0);
	}

	THINetHttp m_http;
	//用于客户端数据加解密
	THTiniRsa m_crsa;
	//用于服务器数据的加密
	THTiniRsa m_srsa;
	volatile THAuthState m_state;
	ITHSoftwareAuthResult *m_handler;
	void *m_adddata;
	volatile UINT m_threadid;
	THString m_HardCode;
	THTimerThread m_timer;
};