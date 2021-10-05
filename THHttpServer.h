#pragma once
/*
//fixme

后缀解释支持
flv支持
mp3支持
mp4支持
asp支持
php支持
cgi支持
fcgi支持

检查keepalive 相关代码,keepalive 加入超时关闭连接功能

通用插件类，插件访问ts获取认证信息结果类

脚本后缀解释支持
脚本ts扩展命令插件支持
插件特别服务(虚拟目录)扩展支持
插件文件系统扩展支持

内置组件单独为img包或单独插件

*/

#include <THStruct.h>
#include <THString.h>
#include <THStringParser.h>
#include <THTime.h>
#include <THRawSocketModel.h>
#include <THFileStore.h>
#include <THScript.h>
#include <THMemBuf.h>
#include <THObjectBuffer.h>
#include <THSysInfo.h>
#include <THPlugin.h>

#define THHTTPSERVER		_T("Tansoft Tini-Server/1.3")
#define THHTTPSERVERVER		13

#define PLUGINTYPE_SOCKETLAYERREPLACER		1	///<替换Socket层插件

/**
* @brief Socket层接口替换插件类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2010-12-03 新建类
*/
class ITHHttpServerPluginTypeSocketLayerReplacer
{
public:
	virtual THRawSocket *GetSocketLayer()=0;
	virtual void ReleaseSocketLayer(THRawSocket *layer)=0;
};

#define LOG_NORMAL	0x1
#define LOG_ERROR	0x2
#define LOG_DETAIL	0x4

#define CHARSET_AUTOFIX	0	///<Url编码自动适应Utf8还是gb2312，不十分准
#define CHARSET_UTF8	1	///<Url编码使用utf8编码，ie默认
#define CHARSET_GB2312	2	///<Url编码使用gb2312

/**
* @brief http服务器类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-09-09 新建类
* @2007-11-17 增加WebLog功能
* @2008-05-08 增加系统变量功能
* @2009-04-09 修正虚拟目录相对路径问题
* @2011-01-03 修正列目录时，最后不带/请求时，浏览器相对目录的debug(301到/)
              传递ResponseMaker给外部处理函数，用于对返回头进行控制
			  改正外部处理完成时，如果是返回200且内容为空时，返回状态出错问题
* @2011-02-16 增加Cache Age支持，增加GZip返回支持
*/
/**<pre>
用法：
	Log:
		SetWebLogMode
		GetWebLogMode
		GetWebLogFile

	Auth:
		BOOL m_bAuth;									//<<<是否使用认证
		BOOL m_bDigestMode;							//<<<是否使用摘要模式
		THString m_sAuthName;						//<<<授权名称
		THStringArray m_aAccountList;				//<<<账号列表，username:password

	IpLimit:
		BOOL m_bLimitIp;									//<<<是否限制ip
		THIpRange m_allowrange;						//<<<允许范围
		THIpRange m_blockrange;					//<<<禁止范围

	Script:
		BOOL m_bUseScript;						//<<<是否使用脚本，默认使用
		THString m_sScriptExtList;				//<<<脚本文件后缀.tsp.头尾必需含有.
		UINT m_nRunLimitTime;						//<<<限制线程的运行时间，0为不限制

	Session:
		BOOL m_bUseSession;							//<<<是否使用session机制
		BOOL m_bSessionCookielessMode;				//<<<是否使用CookieLess模式
		UINT m_nSessionLimitTime;					//<<<session生存时间

	ThreadPool:
		UINT m_nParseThread;											//<<<并发处理线程的数量，在StartListen前指定
		UINT m_nMaxParseThread;									//<<<当后台进程处理不及时，允许新开线程的上限值，0为不指定上限

	FileCache:
		UINT m_nFileCacheSize;						//<<<文件缓存大小
		UINT m_nFileCacheMode;						//<<<文件缓存方式，0为不使用，1为使用固定值，10-80 为使用空余内存的比例
		UINT m_nFileAge;							//<<<浏览器文件缓存时间，0为不使用，秒数，设置后会在文件请求时返回 Cache-Control: Max-age=sec
		BOOL m_bGZip;								//<<<是否支持GZip压缩

	Speical Service:
		virtual ParseExtCmd function				//<<<继承处理函数
		BOOL m_bSepService;							//<<<是否使用特别处理函数
		THStringArray m_aSepServiceList;			//<<<特别处理列表，如果/将会挂接/根请求，如m_aSepServiceList.Add(_T("/request"));
		THStringArray m_aSepServiceDirList;			//<<<特别处理目录列表，如果/将会挂接所有请求，如m_aSepServiceDirList.Add(_T("/request/"));

	m_server.m_sAcceptExtList=_T(".exe.");
	m_server.m_bUseAcceptExt=TRUE;
	m_server.m_bListDirectory=FALSE;
	m_server.StartListen(80,0,basepath+_T("\\Web"));

  Html脚本语法：
  <!--[ts:start]
    <function file="file.ts" name="RunScript">
	  <param name="arg1" value="123" method="const" />	//const或空 表示为常数
	  <param name="arg2" value="q" method="get" />		//get 为变量q从get参数堆取
	  <param name="arg3" value="a2" method="post" />	//post 为变量a2从post参数堆取
	  <param name="arg4" value="a3" method="request" />	//reqeust 为变量a3先从post取，没有从get取
	  <param name="arg5" value="a4" method="cookie" />	//cookie 为变量a4从cookie取
	  <param name="arg6" value="a5" method="session" />	//session 为变量a5从session区取
	  <param name="arg7" value="remoteip" method="sysvalue" /> //sysvalue 为系统变量，value指定内容为变量内容，可用内容如下：
		remoteip: 客户ip数值，本机字节序
		remoteipstr: 客户ip字符串
		remoteport: 客户port，本机字节序
		requestpath: 请求的文件web路径
		requestfile: 请求的文件真实路径
		requestfileext: 请求文件后缀名
		host: 请求主机名
		useragent: 请求Agent
		authuser: 授权的用户名
		authpass: 授权的密码，md5加密
		virtualroot: 虚拟主目录
	</function>
  [ts:end]-->

  关于session：
	session是区分不同用户的唯一标识，该值需要服务器在session支持模式时才会生成。
	服务器为session cookie模式时，从cookie取值TsSessionID作为session的存储键值；
	session cookieless模式时，从路径中取TsSessionID，也保存到cookieparam中。

  关于脚本返回：
	脚本只能返回字符串。
	脚本返回的字符串直接替换到函数所在的html片段中。
	当返回中含有<ReDirectUrl>/redirpath/file</ReDirectUrl>时，返回为302，跳转到对应的页面中。
	设置cookie通过返回<Cookie>key=value<Expires>413242433</Expires><Path>/</Path></Cookie>完成，可返回多个<Cookie>。
		key=value 为设置到客户端cookie的值。
		Expires 为可选值，表示将在该时间失效，没有表示不过期，时间为脚本中的32位时间，详见THScript的time32系列函数。
		Path 为可选值，表示cookie的作用域，空时为默认的根目录/。
	设置session通过返回<Session>key=value</Session>完成。
	返回<NoCache>1</NoCache>表示该页不使用缓存，每次强制刷新。

  关于文件上传支持：
	文件相关的信息会保存到getparam中，保存为xml格式，其中以文件对象的名称作key，以下为具体内容：
		当系统设置 m_bUploadSaveToFile 为 TRUE时，保存为文件
			<key-savetype>file</key-savetype>
			<key>文件保存到的全路径</key>
		否则，文件内容保存到xml中：
			<key-savetype>xml</key-savetype>
			//如果文件中带有字符'\0'，字符串不能表示的时候，encoding为base64
			<key-encoding>base64</key-encoding>
			//否则为plain
			<key-encoding>plain</key-encoding>
		其他保存的值分别为：
			<key-filename>上传文件名称</key-filename>
			<key-filebasename>上传文件名称，不带路径</key-filebasename>
			<key-fileext>上传文件后缀名</key-fileext>
			<key-cnttype>上传文件的类型</key-cnttype>

  外部接口实现：
	脚本文件的外部脚本处理接口可通过SetScriptParseExtCmd完成
	Http请求外部处理接口可通过重载ParseExtCmd实现，该函数返回值定义如下：
		//如果返回0，表示外部处理已经全部处理完成，可直接返回
		//如果需要HttpServer本身处理流程进行返回，返回3，ReturnVal为返回内容
		//如果对请求进行处理完成后，想再继续原来的普通目录，文件，脚本调用，返回其他值
		//ExtCmd对file，getparam，postparam，bContentXml，sysparam，StsCode，ReturnVal的更改都生效
	注意Http请求外部处理接口处理完成后，还可以继续进入普通目录，文件，脚本调用过程

  关于中文路径及文件名：
	可通过设置m_nCharsetMode确定使用utf8解码还是使用普通gb2312编码解码，还是自适应，默认为自适应，但是会影响性能，ie浏览器默认使用utf8编码。
	注意swf文件ie会先以utf8编码请求，后swf文件自身会使用gb2312请求。

  关于Https服务：
    通过UsePluginSocketLayer设置使用SshSocket的插件代替标准Socket层实现。
</pre>*/
class THHttpServer : protected ITHRawSocketBufferedEventHandler ,protected THThread ,protected THPlugin
{
protected:
	typedef struct _THParsingData{
		THRawSocket *cls;			///<socket类厂
		RAWSOCKET sock;				///<回复socket
		THMemBuf buf;				///<请求头
		struct sockaddr_in addr;	///<对端地址
		DWORD nSt;					///<开始响应时间
		int nThreadId;				///<处理线程
	}THParsingData;

	typedef struct _SessionInfo{
		THString sVal;
		DWORD nLastTime;
	}SessionInfo;
public:
	THHttpServer()
	{
		m_s=INVALID_SOCKET;
		m_pStore=&m_Store;
		//http server settings;
		m_bListDirectory=TRUE;
		m_bUseDefaultFile=TRUE;
		//Default File List
		m_aDefaultFileList.Add(_T("default.tsp"));
		m_aDefaultFileList.Add(_T("default.htm"));
		m_aDefaultFileList.Add(_T("default.html"));
		m_aDefaultFileList.Add(_T("index.tsp"));
		m_aDefaultFileList.Add(_T("index.htm"));
		m_aDefaultFileList.Add(_T("index.html"));
		//Default Accept List
		m_sAcceptExtList=_T(".tsp.htm.html.js.css.jpg.gif.png.bmp.ico.rar.zip.7z.gz.txt.xml.");
		m_sScriptExtList=_T(".tsp.");
		m_bSepService=FALSE;
		m_bUseScript=TRUE;
		m_bUseAcceptExt=FALSE;
		m_bUploadSaveToFile=FALSE;
		m_bUploadAutoRename=TRUE;
		m_scriptcls=NULL;
		m_scriptcb=NULL;
		m_scriptdata=NULL;
		m_nLogMode=0;
		m_nParseThread=10;
		m_nMaxParseThread=0;
		m_nRunLimitTime=90000;//90 sec limit
		m_nCharsetMode=CHARSET_AUTOFIX;
		m_bUseHostName=FALSE;
		m_bUseVirtualDir=FALSE;
		m_bUseSession=FALSE;
		m_bSessionCookielessMode=FALSE;
		m_nSessionLimitTime=5*60*1000;
		m_dirmapping.SetFreeProc(FreeFileStore,this);
		m_hostmapping.SetFreeProc(FreeFileStore,this);
		m_sessionmapping.SetFreeProc(FreeSession,this);
		m_bAuth=FALSE;
		m_bDigestMode=TRUE;
		m_sAuthName=_T("TansoftServer");
		m_nFileCacheSize=0;
		m_nFileCacheMode=0;
		m_nFileCacheLast=0;
		m_bLimitIp=FALSE;
		m_bUsePlugin=FALSE;
		m_pSLReplacer=NULL;
		m_nFileAge=0;
		m_bGZip=TRUE;
	}
	virtual ~THHttpServer()
	{
		StopListen();
		if (m_pStore)
		{
			m_pStore->DeleteObjectByFreeor(this);
			m_pStore=NULL;
		}
		ReleaseSockFactory();
	}

	virtual void InitSocketFactory()
	{
		if (!m_psock)
		{
			if (m_pSLReplacer)
				m_psock=m_pSLReplacer->GetSocketLayer();
			else
				m_psock=new THRawSocket;
			m_psock->SetHandler(this,NULL);
		}
	}

	virtual void ReleaseSockFactory()
	{
		if (m_psock)
		{
			if (m_pSLReplacer)
				m_pSLReplacer->ReleaseSocketLayer(m_psock);
			else
			{
				m_psock->StopListen();
				delete m_psock;
			}
			m_psock=NULL;
		}
	}

	inline UINT GetThreadCount(){return THThread::GetThreadCount();}
	UINT GetWaitingParseCount() {return m_ParseList.GetCount();}

	BOOL SaveSettings(ITHIni *ini,THString sSession=_T("THHttpServerSettings"))
	{
		if (!ini) return FALSE;
		THIniAutoWBool(ini,sSession,bListDirectory,TRUE);
		THIniAutoWInt(ini,sSession,nCharsetMode,CHARSET_AUTOFIX);
		THIniAutoWBool(ini,sSession,bUseDefaultFile,TRUE);
		THIniAutoWStrAr(ini,sSession,aDefaultFileList);
		THIniAutoWBool(ini,sSession,bUseAcceptExt,FALSE);
		THIniAutoWString(ini,sSession,sAcceptExtList,_T(""));
		THIniAutoWBool(ini,sSession,bUseScript,TRUE);
		THIniAutoWString(ini,sSession,sScriptExtList,_T(""));
		THIniAutoWBool(ini,sSession,bSepService,FALSE);
		THIniAutoWStrAr(ini,sSession,aSepServiceList);
		THIniAutoWStrAr(ini,sSession,aSepServiceDirList);

		THIniAutoWBool(ini,sSession,bUploadSaveToFile,FALSE);
		THIniAutoWBool(ini,sSession,bUploadAutoRename,TRUE);
		THIniAutoWString(ini,sSession,sUploadSaveBasePath,_T(""));

		THIniAutoWInt(ini,sSession,nLogMode,0);
		THIniAutoWString(ini,sSession,sLogFile,_T(""));

		THIniAutoWInt(ini,sSession,nParseThread,10);
		THIniAutoWInt(ini,sSession,nMaxParseThread,0);
		THIniAutoWInt(ini,sSession,nRunLimitTime,90000);

		THIniAutoWBool(ini,sSession,bUseSession,FALSE);
		THIniAutoWBool(ini,sSession,bSessionCookielessMode,FALSE);
		THIniAutoWInt(ini,sSession,nSessionLimitTime,5*60*1000);

		THIniAutoWBool(ini,sSession,bAuth,FALSE);
		THIniAutoWBool(ini,sSession,bDigestMode,TRUE);
		THIniAutoWString(ini,sSession,sAuthName,_T("TansoftServer"));
		THIniAutoWStrAr(ini,sSession,aAccountList);

		THIniWString(ini,sSession,_T("FileStore"),THFileStore2String::FileStore2String(m_pStore),_T(""));
		THIniAutoWBool(ini,sSession,bUseHostName,FALSE);
		THIniAutoWBool(ini,sSession,bUseVirtualDir,FALSE);

		THIniAutoWInt(ini,sSession,nFileCacheSize,0);
		THIniAutoWInt(ini,sSession,nFileCacheMode,0);

		THIniAutoWInt(ini,sSession,nFileAge,0);
		THIniAutoWBool(ini,sSession,bGZip,TRUE);

		THIniAutoWBool(ini,sSession,bLimitIp,FALSE);
		THIniWString(ini,sSession,_T("AllowIpRange"),m_allowrange.SaveToString(),_T(""));
		THIniWString(ini,sSession,_T("BlockIpRange"),m_blockrange.SaveToString(),_T(""));

		THIniAutoWBool(ini,sSession,bUsePlugin,FALSE);

		THString ret;
		THPosition pos;
		THString tmp;
		ITHFileStore *tmpfs;
		pos=m_hostmapping.GetStartPosition();
		while(!pos.IsEmpty())
		{
			if (m_hostmapping.GetNextPosition(pos,tmp,tmpfs))
			{
				if (tmpfs)
				{
					ret+=tmp+_T("||")+THFileStore2String::FileStore2String(tmpfs)+_T("||");
				}
			}
		}
		THIniWString(ini,sSession,_T("HostMapping"),ret,_T(""));

		pos=m_dirmapping.GetStartPosition();
		while(!pos.IsEmpty())
		{
			if (m_dirmapping.GetNextPosition(pos,tmp,tmpfs))
			{
				if (tmpfs)
				{
					ret+=tmp+_T("||")+THFileStore2String::FileStore2String(tmpfs)+_T("||");
				}
			}
		}
		THIniWString(ini,sSession,_T("DirMapping"),ret,_T(""));
		THPlugin::SaveSettings(ini,sSession);
		return TRUE;
	}

	BOOL LoadSettings(ITHIni *ini,THString sSession=_T("THHttpServerSettings"))
	{
		if (!ini) return FALSE;
		THIniAutoRBool(ini,sSession,bListDirectory,TRUE);
		THIniAutoRBool(ini,sSession,nCharsetMode,CHARSET_AUTOFIX);
		THIniAutoRBool(ini,sSession,bUseDefaultFile,TRUE);
		THIniAutoRStrAr(ini,sSession,aDefaultFileList);
		THIniAutoRBool(ini,sSession,bUseAcceptExt,FALSE);
		THIniAutoRString(ini,sSession,sAcceptExtList,_T(".tsp.htm.html.js.css.jpg.gif.png.bmp.ico.rar.zip.7z.gz.txt.xml."));
		THIniAutoRBool(ini,sSession,bUseScript,TRUE);
		THIniAutoRString(ini,sSession,sScriptExtList,_T(".tsp."));
		THIniAutoRBool(ini,sSession,bSepService,FALSE);
		THIniAutoRStrAr(ini,sSession,aSepServiceList);
		THIniAutoRStrAr(ini,sSession,aSepServiceDirList);

		THIniAutoRBool(ini,sSession,bUploadSaveToFile,FALSE);
		THIniAutoRBool(ini,sSession,bUploadAutoRename,TRUE);
		THIniAutoRString(ini,sSession,sUploadSaveBasePath,_T(""));

		THIniAutoRInt(ini,sSession,nLogMode,0);
		THIniAutoRString(ini,sSession,sLogFile,_T(""));

		THIniAutoRInt(ini,sSession,nParseThread,10);
		THIniAutoRInt(ini,sSession,nMaxParseThread,0);
		THIniAutoRInt(ini,sSession,nRunLimitTime,90000);

		THIniAutoRBool(ini,sSession,bUseSession,FALSE);
		THIniAutoRBool(ini,sSession,bSessionCookielessMode,FALSE);
		THIniAutoRInt(ini,sSession,nSessionLimitTime,5*60*1000);

		THIniAutoRBool(ini,sSession,bAuth,FALSE);
		THIniAutoRBool(ini,sSession,bDigestMode,TRUE);
		THIniAutoRString(ini,sSession,sAuthName,_T("TansoftServer"));
		THIniAutoRStrAr(ini,sSession,aAccountList);

		THIniAutoRInt(ini,sSession,nFileCacheSize,0);
		THIniAutoRInt(ini,sSession,nFileCacheMode,0);

		THIniAutoRInt(ini,sSession,nFileAge,0);
		THIniAutoRInt(ini,sSession,bGZip,TRUE);

		THIniAutoRBool(ini,sSession,bLimitIp,FALSE);
		THString setting;
		THIniRString(ini,sSession,_T("AllowIpRange"),setting,_T(""));
		if (setting.IsEmpty()) setting=_T("0.0.0.0:0.0.0.0|");
		m_allowrange.LoadFromString(setting);
		THIniRString(ini,sSession,_T("BlockIpRange"),setting,_T(""));
		m_blockrange.LoadFromString(setting);

		THIniAutoRBool(ini,sSession,bUsePlugin,FALSE);

		THString sfs;
		THIniRString(ini,sSession,_T("FileStore"),sfs,_T(""));
		if (sfs.IsEmpty())
		{
			m_pStore->DeleteObjectByFreeor(this);
			m_pStore=NULL;
		}
		else
		{
			m_pStore=THFileStore2String::String2FileStore(sfs);
			if (m_pStore) m_pStore->SetFreeor(this);
		}
		if (!m_pStore) m_pStore=&m_Store;

		THIniAutoRBool(ini,sSession,bUseHostName,FALSE);
		THIniAutoRBool(ini,sSession,bUseVirtualDir,FALSE);

		THString ret;
		THString tmp,tmp1;
		ITHFileStore *tmpfs;

		m_hostmapping.RemoveAll();
		THIniRString(ini,sSession,_T("HostMapping"),ret,_T(""));
		THStringToken t(ret,_T("||"));
		while(t.IsMoreTokens())
		{
			tmp=t.GetNextToken();
			tmp1=t.GetNextToken();
			if (!tmp.IsEmpty() && !tmp1.IsEmpty())
			{
				tmpfs=THFileStore2String::String2FileStore(tmp1);
				if (tmpfs)
				{
					tmpfs->SetFreeor(this);
					tmp.MakeLower();
					m_hostmapping.SetAt(tmp,tmpfs);
				}
			}
		}

		m_dirmapping.RemoveAll();
		THIniRString(ini,sSession,_T("DirMapping"),ret,_T(""));
		t.Init(ret,_T("||"));
		while(t.IsMoreTokens())
		{
			tmp=t.GetNextToken();
			tmp1=t.GetNextToken();
			if (!tmp.IsEmpty() && !tmp1.IsEmpty())
			{
				tmpfs=THFileStore2String::String2FileStore(tmp1);
				if (tmpfs)
				{
					tmpfs->SetFreeor(this);
					tmp.MakeLower();
					m_dirmapping.SetAt(tmp,tmpfs);
				}
			}
		}
		THPlugin::LoadSettings(ini,sSession);
		return TRUE;
	}

	/**
	* @brief 初始化插件系统
	* @param sBasePath		插件目录
	*/
	void InitPlugin(THString sBasePath)
	{
		//Init Plugin System
		Init(sBasePath,TRUE,_T("plg"),THHTTPSERVERVER);
		if (m_bUsePlugin) LoadAllPlugin();
	}

	/**
	* @brief 开始http监听服务
	* @param nListenPort	监听端口
	* @param nSid			动态服务器模式的serviceid
	* @param sBasePath		传统文件模式下的服务器根目录
	* @return 是否成功
	*/
	BOOL StartListen(int nListenPort,int nSid,THString sBasePath)
	{
		m_Store.SetBaseDirectory(sBasePath);
		return StartListen(nListenPort,nSid,NULL);
	}

	/**
	* @brief 开始http监听服务
	* @param nListenPort	监听端口
	* @param nSid			动态服务器模式的serviceid
	* @param pStore			文件系统指针，传入时应该已调用SetBaseDirectory设置好主目录
	* @param bAutoFree		文件系统是否自动释放，注意如果自动释放，当StartListen失败时，就会释放文件系统了
	* @return 是否成功
	*/
	BOOL StartListen(int nListenPort,int nSid,ITHFileStore *pStore,BOOL bAutoFree=FALSE)
	{
		StopListen();
		InitSocketFactory();
		if (pStore)
		{
			m_pStore=pStore;
			if (bAutoFree) pStore->SetFreeor(this);
		}
		else
		{
			m_pStore=&m_Store;
		}
		//start time out check thread
		StartThread(1,(void *)1,1000);
		for(UINT i=0;i<m_nParseThread;i++)
			StartThread(0,0,30000);
		m_s=m_psock->CreateSocket(0,0,nListenPort,RawProtocol_Tcp,TRUE,nSid,FALSE);
		if (m_s==INVALID_SOCKET)
		{
			AddLog(LOG_ERROR,_T("无法启动网络服务，端口可能被占用。"));
			KillAllThread();
			if (m_pStore)
				m_pStore->DeleteObjectByFreeor(this);
			m_pStore=&m_Store;
			ReleaseSockFactory();
			return FALSE;
		}
		AddLog(LOG_NORMAL,_T("Http服务已经启动。"));
		AddLog(LOG_DETAIL,_T("监听端口:%d,动态服务器服务id:%d,基础路径:%s,Socket套接字:%d,文件系统:%s"),nListenPort,nSid,m_pStore->GetBaseDirectory(),m_s,m_pStore->GetStoreName());
		return TRUE;
	}

	BOOL StopListen()
	{
		if (m_s!=INVALID_SOCKET)
		{
			if (m_psock) m_psock->FreeSocket(m_s);
			m_s=INVALID_SOCKET;
			AddLog(LOG_NORMAL,_T("Http服务已经停止。"));
		}
		KillAllThread();
		if (m_pStore)
			m_pStore->DeleteObjectByFreeor(this);
		m_pStore=&m_Store;
		ReleaseSockFactory();
		return TRUE;
	}

	inline BOOL IsStarted(){return (m_s!=INVALID_SOCKET);}
	U16 GetListenPort()
	{
		if (m_s==INVALID_SOCKET || !m_psock) return 0;
		return m_psock->GetLocalPort(m_s);
	}

	BOOL AddHostMapping(THString hostname,ITHFileStore *store,BOOL bFree)
	{
		if (!store) return FALSE;
		hostname.MakeLower();
		if (bFree) store->SetFreeor(this);
		m_bUseHostName=TRUE;
		return m_hostmapping.SetAt(hostname,store);
	}

	BOOL RemoveHostMapping(THString hostname)
	{
		hostname.MakeLower();
		BOOL bRet=m_hostmapping.RemoveAt(hostname);
		if (m_hostmapping.GetCount()==0) m_bUseHostName=FALSE;
		return bRet;
	}

	BOOL AddVirtualDirMapping(THString path,ITHFileStore *store,BOOL bFree)
	{
		if (!store) return FALSE;
		path.MakeLower();
		if (bFree) store->SetFreeor(this);
		m_bUseVirtualDir=TRUE;
		return m_dirmapping.SetAt(path,store);
	}

	BOOL RemoveVirtualDirMapping(THString hostname)
	{
		hostname.MakeLower();
		BOOL bRet=m_dirmapping.RemoveAt(hostname);
		if (m_dirmapping.GetCount()==0) m_bUseVirtualDir=FALSE;
		return bRet;
	}

	void SetScriptParseExtCmd(THScript *cls,THParseExtCmdProc cb,const void* data)
	{
		m_scriptcls=cls;
		m_scriptcb=cb;
		m_scriptdata=data;
	}

	void SetWebLogMode(THString sLogFile,int LogMode=LOG_NORMAL|LOG_ERROR)
	{
		m_nLogMode=LogMode;
		m_sLogFile=sLogFile;
	}

	int GetWebLogMode(){return m_nLogMode;}
	THString GetWebLogFile(){return m_sLogFile;}

	void EmptyCache(){m_pStore->EmptyCache();}

	BOOL SetupHttpsServer(THString pem,THString key)
	{
		char *ppem=THCharset::t2a(pem);
		if (ppem)
		{
			char *pkey=THCharset::t2a(key);
			if (pkey)
			{
				CauseEvent("SslSocketPemAndKey",ppem,pkey);
				THCharset::free(pkey);
			}
			THCharset::free(ppem);
		}
		return UsePluginSocketLayer(_T("{2CCEEB3D-5E31-4e2c-B772-C34BDCEF543C}"));
	}

	BOOL UsePluginSocketLayer(THString guid=_T("{2CCEEB3D-5E31-4e2c-B772-C34BDCEF543C}"))
	{
		StopListen();
		m_pSLReplacer=(ITHHttpServerPluginTypeSocketLayerReplacer *)QueryInterfaceT(guid,_T("ITHHttpServerPluginTypeSocketLayerReplacer"));
		return m_pSLReplacer!=NULL;
	}
protected:
	virtual UINT IsBufferReady(THRawSocket *cls,RAWSOCKET sock,const void *bufdata,int buflen,struct sockaddr_in *paddr,void *adddata)
	{
		const char crbf[]="\r\n\r\n";
		static int crbflen=(int)strlen(crbf);
		const void *tmpcrbf=THBinSearch::BinSearch(bufdata,buflen,crbf,crbflen);
		if (!tmpcrbf) return 0;
		int headerlen=(int)(INT_PTR)((const char *)tmpcrbf-(const char *)bufdata);
		const char *contentorg=((const char *)tmpcrbf)+crbflen;
		int contentlen=buflen-headerlen-crbflen;
/*		char *mytmp=new char[headerlen+1];
		if (!mytmp)
		{
			AddLog(LOG_ERROR,_T("申请内存失败。"));
			return 0;
		}
		strncpy(mytmp,(const char *)bufdata,headerlen);
		*(mytmp+headerlen)='\0';
		THString header=THCharset::a2t(mytmp);
		delete [] mytmp;*/
		THString header=THCharset::a2t((const char *)bufdata,headerlen);
		if (header.IsEmpty()) return 0;
		THHttpHeaderParser parser;
		if (!parser.Parse(header))
		{
			AddLog(LOG_ERROR,_T("文件头解释错误。文件头：%s"),header);
			return 0;
		}
		int cntlen=THStringConv::s2i(parser.GetContentLength());
		if (buflen>=headerlen+cntlen+crbflen)
			return headerlen+cntlen+crbflen;
		return 0;
	}

	virtual void BufferedReceiveHandler(THRawSocket *cls,RAWSOCKET sock,const void *bufdata,int buflen,struct sockaddr_in *paddr,void *adddata)
	{
		//adddata not use
		adddata;
		THParsingData *data=m_objbuf.NewObj();
		if (data)
		{
			memset(&data->addr,0,sizeof(struct sockaddr_in));
			data->addr.sin_addr.S_un.S_addr=THNetWork::GetPeerInfoN(sock,&data->addr.sin_port);
			//memcpy(&data->addr,paddr,sizeof(struct sockaddr_in));
			data->cls=cls;
			data->sock=sock;
			data->buf.Empty();
			data->buf.AddBuf(bufdata,buflen);
			data->nSt=GetTickCount();
			data->nThreadId=0;
			m_ParseLock.Lock();
			m_ParseList.AddTail(data);
			m_ParseLock.Unlock();
			int nThreadId=SetThreadPoolEvent(1);
			if (nThreadId==0 && (m_nMaxParseThread==0 || m_nMaxParseThread>GetThreadCount()))
			{
				//Thread All Fulling,Try Start Thread
				StartThread(0,0,30000);
				AddLog(LOG_DETAIL,_T("新开线程，线程数:%d"),GetThreadCount());
			}
			//AddLog(LOG_DETAIL,_T("线程 %d 响应中..."),nThreadId);
		}
	}

	virtual void _BufferedReceiveHandler(THParsingData *data,const void *bufdata,int buflen)
	{
		THRawSocket *cls=data->cls;
		RAWSOCKET sock=data->sock;
		struct sockaddr_in *paddr=&data->addr;
		const char crbf[]="\r\n\r\n";
		static int crbflen=(int)strlen(crbf);
		const void *tmpcrbf=THBinSearch::BinSearch(bufdata,buflen,crbf,crbflen);
		if (!tmpcrbf) return;
		int headerlen=(int)(INT_PTR)((const char *)tmpcrbf-(const char *)bufdata);
		const char *contentorg=((const char *)tmpcrbf)+crbflen;
		int contentlen=buflen-headerlen-crbflen;
/*		char *tmp=new char[buflen+1];
		if (!tmp)
		{
			AddLog(LOG_ERROR,_T("错误:申请内存失败。"));
			return;
		}
		memcpy(tmp,bufdata,buflen);
		*(tmp+buflen)='\0';
		THString header=THCharset::a2t(tmp);
		delete [] tmp;*/
		THString header=THCharset::a2t((const char *)bufdata,headerlen);
		THString content=THCharset::a2t((const char *)contentorg,contentlen);
		BOOL bContentXml=FALSE;
		int pos;
/*		int pos=header.Find(_T("\r\n\r\n"),0);
		if (pos!=-1)
		{
			if (header.GetLength()>pos+4)
				content=header.Mid(pos+4);
			header=header.Left(pos);
		}*/
		THHttpHeaderParser parser;
		if (!parser.Parse(header))
		{
			AddLog(LOG_ERROR,_T("文件头解释错误。文件头：%s"),header);
			return;
		}
		THString unit;
		int StartPos,EndPos;
		if (!parser.GetRange(unit,StartPos,EndPos))
		{
			AddLog(LOG_ERROR,_T("获取范围错误。文件头：%s"),header);
			return;
		}
		THString sysparam;
		//check the cookie
		THString cookieparam=parser.GetCookieToXml();
		THString sessionid;
		//file
		THString file,orgfile;
		orgfile=parser.GetFirstHeader(1);
		THHttpHeaderResponseMaker maker;
		maker.SetServer(THHTTPSERVER);
		THString ReturnVal;
		THStringA ReturnValA;
		THMemBuf ReturnBuf;
		int cntlen=0;
		int fileidx=-1;
		int mode=0;
		int StsCode=0;
		U16 rport;
		U32 rip=THNetWork::GetPeerInfo(sock,&rport);
		BOOL bOk=TRUE;
		BOOL bUseGZip=FALSE;
		void *zipbuf=NULL;
		//check ip range settings
		if (m_bLimitIp)
		{
			if (!m_allowrange.IsIpInRangeN(paddr->sin_addr.S_un.S_addr))
			{
				bOk=FALSE;
				StsCode=403;
				ReturnVal=_T("403 Forbidden");
			}
			else
			{
				if (m_blockrange.IsIpInRangeN(paddr->sin_addr.S_un.S_addr))
				{
					bOk=FALSE;
					StsCode=403;
					ReturnVal=_T("403 Forbidden");
				}
			}
		}
		//check session settings
		if (m_bUseSession && bOk)
		{
			if (m_bSessionCookielessMode)
			{
				//check the path
				GetSessionId(orgfile,sessionid);
				if (sessionid.IsEmpty())
				{
					//if in session with cookieless mode,and path is not have sessions
					StsCode=302;
					sessionid=MakeSessionId(rip);
					maker.AddHeaders(_T("Location"),_T("/")+sessionid+orgfile,FALSE);
					bOk=FALSE;
				}
				else
				{
					//session Ok,save to cookieparam maybe script will use
					cookieparam+=THSimpleXml::MakeParam(_T("TsSession"),sessionid);
				}
			}
			else
			{
				//check cookie
				sessionid=THSimpleXml::GetParam(cookieparam,_T("TsSession"));
				if (sessionid.IsEmpty())
				{
					sessionid=MakeSessionId(rip);
					maker.AddCookie(_T("TsSession=")+sessionid,NULL);
					cookieparam+=THSimpleXml::MakeParam(_T("TsSession"),sessionid);
				}
			}
		}
		//check auth
		if (m_bAuth && bOk)
		{
			bOk=FALSE;
			if (m_bDigestMode)
			{
				THString auth=parser.GetTag(_T("Authorization")).Trim();
				if (auth.Left(6).CompareNoCase(_T("Digest"))==0)
				{
					auth=auth.Mid(7);
					//change key value to xml format
					auth=THStringConv::ChangeStringValue(auth,_T(","),_T("&#44;"));
					auth=THStringConv::ChangeStringValue(auth,_T("="),_T("&#61;"));
					THStringToken t(auth,_T(","));
					THStringToken tt;
					THString xml;
					while(t.IsMoreTokens())
					{
						tt.Init(t.GetNextToken(),_T("="));
						THString key=tt.GetNextToken();
						THString value=tt.GetNextToken().Trim(_T("\" "));
						value.Replace(_T("&#44;"),_T(","));
						value.Replace(_T("&#61;"),_T("="));
						if (!key.IsEmpty())
							xml+=THSimpleXml::MakeParam(key,value);
					}
					THString username=THSimpleXml::GetParam(xml,_T("username"));
					THString uri=THSimpleXml::GetParam(xml,_T("uri"));
					THString nonce=THSimpleXml::GetParam(xml,_T("nonce"));
					THString response=THSimpleXml::GetParam(xml,_T("response"));
					//check if uri is same
					THString ur1=THUrlExplain::UrlDecode(parser.GetFirstHeader(1));
					THString ur2=THUrlExplain::UrlDecode(uri);
					if (ur1==ur2 && !username.IsEmpty() && !nonce.IsEmpty() && !response.IsEmpty())
					{
						int length=username.GetLength()+1;
						THString match=username+_T(":");
						THString pass;
						//find user's password
						for(int i=0;i<m_aAccountList.GetSize();i++)
						{
							if (m_aAccountList[i].Left(length)==match)
							{
								pass=m_aAccountList[i].Mid(length);
								if (!pass.IsEmpty())
								{
									//可能存在多个用户名一样但密码不同的账号，因此应该全部循环
									THString resp=THMd5::CalcStrMd5Str(THMd5::CalcStrMd5Str(username+_T(":")+m_sAuthName+_T(":")+pass,FALSE)+_T(":")+nonce+_T(":")+THMd5::CalcStrMd5Str(parser.GetFirstHeader(0)+_T(":")+uri,FALSE),FALSE);
									if (THMd5::IsMd5Same(resp,response))
									{
										sysparam+=THSimpleXml::MakeParam(_T("authuser"),username);
										sysparam+=THSimpleXml::MakeParam(_T("authpass"),THMd5::CalcStrMd5Str(pass));
										bOk=TRUE;
										break;
									}
								}
							}
						}
/*							MD5BUFFER ha1,ha2;
							if (THMd5::CalcStrMd5(username+_T(":")+m_sAuthName+_T(":")+pass,&ha1))
							{
								if (THMd5::CalcStrMd5(parser.GetFirstHeader(0)+_T(":")+uri,&ha2))
								{
									char *tmp=THCharset::t2a(nonce);
									if (tmp)
									{
										THMemBuf buf;
										buf.AddBuf(&ha1,sizeof(MD5BUFFER));
										buf.AddBuf(":",1);
										buf.AddBuf(tmp,strlen(tmp));
										buf.AddBuf(":",1);
										buf.AddBuf(&ha2,sizeof(MD5BUFFER));
										THCharset::free(tmp);
										UINT len;
										void *pb=buf.GetBuf(&len,FALSE);
										if (pb)
										{
											THString resp=THMd5::CalcMd5Str(pb,len);
											if (THMd5::IsMd5Same(resp,response))
												bOk=TRUE;
										}
									}
								}
							}*/
							//THString resp=THMd5::CalcStrMd5Str(+_T(":")+nonce+_T(":")+);
							//if (THMd5::IsMd5Same(resp,response))
							//	bOk=TRUE;
					}
				}
				if (bOk==FALSE)
				{
					//request auth
					THString value;
					value.Format(_T("Digest algorithm=\"md5\",realm=\"%s\",nonce=\"%s\",opaque=\"%s\""),m_sAuthName,MakeNonce(rip),MakeOpaque());
					maker.AddHeaders(_T("WWW-Authenticate"),value);
					StsCode=401;
					ReturnVal=_T("Unauthorized");
				}
			}
			else
			{
				THString auth=parser.GetTag(_T("Authorization")).Trim();
				if (auth.Left(5).CompareNoCase(_T("Basic"))==0)
				{
					THString str=THBase64::DecodeStr(auth.Mid(6));
					for(int i=0;i<m_aAccountList.GetSize();i++)
					{
						if (m_aAccountList[i]==str)
						{
							if (str.Find(_T(":"),0)!=-1)
							{
								sysparam+=THSimpleXml::MakeParam(_T("authuser"),str.Left(str.Find(_T(":"),0)));
								sysparam+=THSimpleXml::MakeParam(_T("authpass"),THMd5::CalcStrMd5Str(str.Mid(str.Find(_T(":"),0)+1)));
							}
							bOk=TRUE;
							break;
						}
					}
				}
				if (bOk==FALSE)
				{
					//request auth
					THString value;
					value.Format(_T("Basic realm=\"%s\""),m_sAuthName);
					maker.AddHeaders(_T("WWW-Authenticate"),value);
					StsCode=401;
					ReturnVal=_T("Unauthorized");
				}
			}
		}
		file=orgfile;
		ITHFileStore *pfs=m_pStore;
		if (bOk)
		{
			THString param;
			pos=file.Find(_T("?"),0);
			if (pos!=-1)
			{
				param=file.Mid(pos+1);
				file=file.Left(pos);
			}
			if (file.Find(_T("%"))!=-1)
			{
				//decode url
				if (m_nCharsetMode==CHARSET_GB2312)
				{
					file=THUrlExplain::UrlDecode(file);
				}
				else
				{
					char *filea=THCharset::t2a(file);
					if (filea)
					{
						char *deca=THUrlExplain::UrlDecodeA(filea);
						if (deca)
						{
							//	CHARSET_UTF8 MODE				CHARSET_AUTOFIX MODE
							if (m_nCharsetMode==CHARSET_UTF8 || THCharset::IsCharsetMayBeUTF8(deca))
								file=THCharset::u82t(deca);
							else
								file=THCharset::a2t(deca);
							THUrlExplain::FreeBuffer(deca);
						}
						THCharset::free(filea);
					}
				}
			}
			sysparam+=THSimpleXml::MakeParam(_T("requestpath"),file);
			//check host and virtual directory mapping
			if (m_bUseHostName)
			{
				THString host=parser.GetHost();
				host.MakeLower();
				ITHFileStore *fs;
				if (m_hostmapping.GetAt(host,fs))
				{
					if (fs) pfs=fs;
				}
			}
			if (m_bUseVirtualDir)
			{
				THPosition pos=m_dirmapping.GetStartPosition();
				THString host;
				THString tmpfile=file;
				int basecnt=0;
				if (tmpfile.GetAt(0)=='/')
				{
					basecnt=1;
					tmpfile=tmpfile.Mid(1);
				}
				tmpfile.MakeLower();
				ITHFileStore *fs;
				while(!pos.IsEmpty())
				{
					if (m_dirmapping.GetNextPosition(pos,host,fs))
					{
						if (fs && tmpfile.Find(host)==0)
						{
							sysparam+=THSimpleXml::MakeParam(_T("virtualroot"),host);
							file=file.Mid(host.GetLength()+basecnt);
							if (file.IsEmpty())
							{
								file=_T("/");
								//由于该请求是目录，而列目录的时候，会导致相对目录定位错误，这时应该301到目录+/下
								StsCode=301;
								maker.AddHeaders(_T("Location"),orgfile+_T("/"),FALSE);
							}
							pfs=fs;
							break;
						}
					}
				}
			}
			ITHFileStore *uppfs=pfs;
			THString ext;
			ext=pfs->GetFileExt(file);
			sysparam+=THSimpleXml::MakeParam(_T("requestfileext"),ext);
			//处理file post data
			THString conntype=parser.GetContentType();
			if (conntype.Find(_T("multipart/form-data"))!=-1)
			{
				//Content-Type: multipart/form-data; boundary=---------------------------7d85dc260f8c
				if (conntype.Find(_T("boundary="))==-1)
				{
					AddLog(LOG_ERROR,_T("类型multipart，但没有boundary。%s,%s"),header,content);
					return;
				}
				THString bound=_T("--")+conntype.Mid(conntype.Find(_T("boundary="))+9);
				bound.Trim();
				content.Empty();
				bContentXml=TRUE;
				char *tmpbound=THCharset::t2a(bound);
				if (tmpbound)
				{
					int tmpboundlen=(int)strlen(tmpbound);
					const char key1[]="Content-Disposition";
					static int key1len=(int)strlen(key1);
					const char *foundpart,*secordpart;
					int partlen;
					foundpart=(const char *)THBinSearch::BinSearch(contentorg,contentlen,tmpbound,tmpboundlen);
					while(foundpart!=NULL)
					{
						foundpart+=tmpboundlen;
						secordpart=(const char *)THBinSearch::BinSearch(foundpart,contentlen-(unsigned int)(UINT_PTR)(foundpart-contentorg),tmpbound,tmpboundlen);
						if (secordpart==NULL)
							partlen=contentlen-(unsigned int)(UINT_PTR)(foundpart-contentorg);
						else
							partlen=(unsigned int)(UINT_PTR)(secordpart-foundpart);
						//foundpart,partlen
						const char *header=(const char *)THBinSearch::BinSearch(foundpart,partlen,"\r\n\r\n",4);
						const char *pcnt;
						int cntlen;
						CString str;
						if (header)
						{
							*(char *)header='\0';
							str=THCharset::a2t(foundpart);
							pcnt=header+4;
							cntlen=partlen-(unsigned int)(UINT_PTR)(pcnt-foundpart);
						}
						else
						{
							str=THCharset::a2t(foundpart);
							pcnt=NULL;
							cntlen=0;
						}
						THStringToken t0(str,_T("\r\n"));
						THString skey,sfile,scntt;
						while(t0.IsMoreTokens())
						{
							THString str1=t0.GetNextToken();
							if (str1.Find(_T("Content-Disposition"))!=-1)
							{
								THStringToken t1(str1,_T(";"));
								while(t1.IsMoreTokens())
								{
									THString key=t1.GetNextToken();
									if (key.Find(_T("="))!=-1)
									{
										THString sk=key.Left(key.Find(_T("=")));
										sk.Trim();
										THString sv=key.Mid(key.Find(_T("="))+1);
										sv.Trim(_T(" \""));
										if (sk==_T("name"))
											skey=sv;
										else if (sk==_T("filename"))
											sfile=sv;
									}
								}
							}
							else if (str1.Find(_T("Content-Type"))!=-1)
							{
								str1=str1.Mid(str1.Find(_T("Content-Type"))+12);
								if (str1.Find(_T(":"))!=-1)
									str1=str1.Mid(str1.Find(_T(":"))+1);
								scntt=str1.Trim();
							}
						}
						if (!skey.IsEmpty() && sfile.IsEmpty())
							THSimpleXml::SetParam(&content,skey,THCharset::a2t(pcnt,cntlen-2));
						if (!sfile.IsEmpty())
						{
							THString sbase=uppfs->GetFileBaseName(sfile);
							if (m_bUploadSaveToFile)
							{
								THString curpath;
								if (m_sUploadSaveBasePath.IsEmpty())
								{
									curpath=uppfs->GetFullPath(file);
									curpath=uppfs->GetCurPath(curpath);
									curpath+=_T("\\")+sbase;
								}
								else
								{
									curpath=m_sUploadSaveBasePath+_T("\\")+sbase;
									//must get with pStore
									uppfs=m_pStore;
									curpath=uppfs->GetFullPath(curpath);
								}
								if (m_bUploadAutoRename)
								{
									THString tmp=curpath;
									int i=0;
									while(uppfs->IsPathFile(tmp))
									{
										tmp.Format(_T("%s_%d"),curpath,i++);
									}
									curpath=tmp;
								}
								else
								{
									if (uppfs->IsPathFile(curpath)) uppfs->DeleteFile(curpath);
								}
								int fidx=uppfs->OpenFile(curpath,FILEMODE_ALWAYSCREATE);
								if (fidx!=-1)
								{
									uppfs->WriteFile(fidx,pcnt,cntlen-2);
									uppfs->CloseFile(fidx);
								}
								THSimpleXml::SetParam(&content,skey+_T("-savetype"),_T("file"));
								THSimpleXml::SetParam(&content,skey,curpath);
							}
							else
							{
								THSimpleXml::SetParam(&content,skey+_T("-savetype"),_T("xml"));
								if (THBinSearch::BinSearch(pcnt,cntlen,"\0",1)!=NULL)
								{
									THSimpleXml::SetParam(&content,skey,THBase64::Encode(pcnt,cntlen-2));
									THSimpleXml::SetParam(&content,skey+_T("-encoding"),_T("base64"));
								}
								else
								{
									THSimpleXml::SetParam(&content,skey,THCharset::a2t(pcnt,cntlen-2));
									THSimpleXml::SetParam(&content,skey+_T("-encoding"),_T("plain"));
								}
							}
							THSimpleXml::SetParam(&content,skey+_T("-filename"),sfile);
							THSimpleXml::SetParam(&content,skey+_T("-filebasename"),sbase);
							THSimpleXml::SetParam(&content,skey+_T("-fileext"),uppfs->GetFileExt(sfile));
						}
						if (!scntt.IsEmpty())
							THSimpleXml::SetParam(&content,skey+_T("-cnttype"),scntt);

						contentlen=contentlen-(unsigned int)(UINT_PTR)(foundpart-contentorg);
						contentorg=foundpart;
						foundpart=secordpart;
					}
					THCharset::free(tmpbound);
				}
			}
			sysparam+=THSimpleXml::MakeParam(_T("remoteip"),THu2s(rip));
			sysparam+=THSimpleXml::MakeParam(_T("remoteipstr"),THNetWork::GetAddrString(rip));
			sysparam+=THSimpleXml::MakeParam(_T("remoteport"),THu2s(rport));
			sysparam+=THSimpleXml::MakeParam(_T("host"),parser.GetHost());
			sysparam+=THSimpleXml::MakeParam(_T("useragent"),parser.GetUserAgent());
			if (!unit.IsEmpty() && unit.CompareNoCase(_T("bytes"))!=0)
			{
				//range not support
				StsCode=400;
				//maker.SetResult(400,_T("Range Not Support"),parser.GetVersion());
				ReturnVal=_T("400 Range Not Support");
				AddLog(LOG_ERROR,_T("范围类型“%s”不支持。"),unit);
			}
			else
			{
				//check gzip is support
				if (m_bGZip && parser.IsAcceptEncodingGZip()) bUseGZip=TRUE;
				if (m_bSepService)
				{
					int i;
					for(i=0;i<m_aSepServiceList.GetSize();i++)
					{
						if (file.CompareNoCase(m_aSepServiceList[i])==0)
						{
							mode=3;
							break;
						}
					}
					if (mode!=3)
					{
						for(i=0;i<m_aSepServiceDirList.GetSize();i++)
						{
							if (_tcsnicmp(file,m_aSepServiceDirList[i],m_aSepServiceDirList[i].GetLength())==0)
							{
								mode=3;
								break;
							}
						}
					}
				}
				//通过外部调用确定处理模式
				if (mode==3)
				{
					mode=ParseExtCmd(file,pfs,&parser,&maker,param,content,bContentXml,sysparam,StsCode,ReturnVal,&ReturnBuf,data);
					if (mode==0)
					{
						AddLog(LOG_NORMAL,_T("请求文件:%s 返回:外部处理完成。"),parser.GetFirstHeader(1));
						AddLog(LOG_DETAIL,_T("系统变量:%s\r\n请求:\r\n%s\r\n\r\n%s\r\n\r\n返回:外部处理完成。"),sysparam,header,content);
						return;
					}
				}
				BOOL bEndSlash=file.GetAt(file.GetLength()-1)=='/';
				file=pfs->GetFullPath(file);
				if (mode!=3)
				{
					//判断请求的内容属于什么，1：目录，2：文件，3：特殊处理，4：脚本
					if (pfs->IsPathDir(file))
					{
						mode=1;
						if (!bEndSlash)
						{
							//由于该请求是目录，而列目录的时候，会导致相对目录定位错误，这时应该301到目录+/下
							StsCode=301;
							maker.AddHeaders(_T("Location"),orgfile+_T("/"),FALSE);
						}
						else if (m_bUseDefaultFile)
						{
							for(int i=0;i<m_aDefaultFileList.GetSize();i++)
							{
								THString tmpfile;
								tmpfile=file+_T("\\")+m_aDefaultFileList[i];
								if (pfs->IsPathFile(tmpfile))
								{
									file=tmpfile;
									ext=pfs->GetFileExt(file);
									mode=2;
									break;
								}
							}
						}
					}
					else if (pfs->IsPathFile(file))
					{
						mode=2;
					}
					else
					{
						//404 not found
						mode=0;
					}
				}
				sysparam+=THSimpleXml::MakeParam(_T("requestfile"),file);
				if (mode==2)
				{
					if (m_bUseScript && m_sScriptExtList.Find(_T(".")+ext+_T("."),0)!=-1)
						mode=4;
				}
				if (mode==0)
				{
					StsCode=404;
					ReturnVal=_T("404 Not Found");
				}
				else if (mode==1)
				{
					if (StsCode!=301)
					{
						if (m_bListDirectory)
						{
							StsCode=200;
							ReturnVal=GetDirList(file,&parser,pfs,sysparam);
						}
						else
						{
							StsCode=403;
							ReturnVal=_T("403 Forbidden");
						}
					}
				}
				else if (mode==2)
				{
					BOOL bRet=TRUE;
					if (m_bUseAcceptExt)
					{
						FALSE;
						THString tmpext=ext;
						tmpext.MakeLower();
						bRet=(m_sAcceptExtList.Find(_T(".")+tmpext+_T("."))!=-1);
					}
					if (bRet==FALSE)
					{
						StsCode=403;
						ReturnVal=_T("403 Forbidden");
					}
					else
					{
						THTime ti;
						StsCode=500;
						ReturnVal=_T("500 Internal Server Error");
						if (pfs->GetFileAttrib(file,NULL,NULL,&ti))
							maker.SetModifyTime(ti);
						maker.SetResponseTypeByExt(ext);
						fileidx=pfs->OpenFile(file,FILEMODE_READ);
						if (fileidx!=-1)
						{
							cntlen=pfs->GetFileSize(fileidx);
							if (/*cntlen<512 || */cntlen>512*1024) bUseGZip=FALSE;//少于512字节或大于512K的都不进行GZip
							StsCode=200;
							ReturnVal.Empty();
							//check header tag If-Modified-Since
							UINT nFileLen;
							THTime tim;
							if (parser.GetIfModifiedSince(tim,nFileLen))
							{
								if (ti==tim && (nFileLen==0 || nFileLen==cntlen))
								{
									StsCode=304;
									pfs->CloseFile(fileidx);
									fileidx=-1;
								}
							}
							if (StsCode==200 && bUseGZip)
							{
								//GZip Support
								UINT ziplen;
								zipbuf=pfs->ReadWholeGZipFile(fileidx,&ziplen);
								if (zipbuf)
								{
									//maker.AddHeaders(_T("gziplen"),THFormat(_T("%u"),ziplen),FALSE);
									//maker.AddHeaders(_T("normallen"),THFormat(_T("%u"),cntlen),FALSE);
									if (ziplen<(UINT)cntlen)
									{
										//注意，GZip模式在这里已经关闭了fileidx，这样文件能尽快空出来，但是fileidx不赋-1，使下面的判断正常
										pfs->CloseFile(fileidx);
										cntlen=ziplen;
										maker.SetContentEncoding();
									}
									else
										bUseGZip=FALSE;
								}
								else
									bUseGZip=FALSE;
							}
							if (StsCode==200 && bUseGZip==FALSE)
							{
								//由于计算GZip可能发生了Seek，确保重定位正确
								if (StartPos!=-1)
									pfs->SeekFile(fileidx,SEEK_SET,StartPos);
								else
									pfs->SeekFile(fileidx,SEEK_SET,0);
							}
						}
					}
				}
				else if (mode==4)
				{
					THString sessionparam;
					if (m_bUseSession && !sessionid.IsEmpty())
					{
						SessionInfo *info;
						m_SessionLock.Lock();
						if (m_sessionmapping.GetAt(sessionid,info))
						{
							if (info)
							{
								sessionparam=info->sVal;
								info->nLastTime=GetTickCount();
							}
						}
						m_SessionLock.Unlock();
					}
					ReturnVal=ParseScript(file,pfs,&parser,param,content,bContentXml,sysparam,cookieparam,sessionparam,StsCode,maker);
					//update the session values
					if (m_bUseSession && !sessionid.IsEmpty())
					{
						SessionInfo *info;
						m_SessionLock.Lock();
						if (m_sessionmapping.GetAt(sessionid,info))
						{
							if (info)
							{
								info->sVal=sessionparam;
								info->nLastTime=GetTickCount();
							}
						}
						m_SessionLock.Unlock();
					}
				}
			}
		}
		if (mode!=2)
		{
			if (ReturnVal!=_T("ReturnBufInstead"))
				maker.SetResponseType(_T("text/html"));
			THTime ti;
			ti.GetCurTime(FALSE);
			maker.SetModifyTime(ti);
		}
		else if (StsCode==200 && m_nFileAge!=0)
			maker.AddCacheControl(THFormat(_T("max-age=%u"),m_nFileAge));
		if (StsCode>=400) GetCustomCodePage(StsCode,ReturnVal);
		if (StsCode==200 && (StartPos!=-1 || EndPos!=-1)) StsCode=206;
		//开始范围比结束范围大，range 范围不对
		if (EndPos!=-1 && StartPos>=EndPos)
		{
			StsCode=416;
			StartPos=-2;
			EndPos=cntlen;
			cntlen=0;
			ReturnVal=_T("Requested Range Not Satisfiable");
		}
		if (StsCode==206)
		{
			//修正结束大小
			if (EndPos!=-1 && EndPos>cntlen)
				EndPos=cntlen;
			//开始范围超出文件大小
			if (StartPos!=-1 && StartPos>=cntlen)
			{
				StsCode=416;
				StartPos=-2;
				EndPos=cntlen;
				cntlen=0;
				ReturnVal=_T("Requested Range Not Satisfiable");
			}
		}
		//处理ReturnVal编码
		THString cnttypecs=parser.GetContentType();
		cnttypecs.MakeLower();
		if (cnttypecs.Find(_T("utf-8"))!=-1 && ReturnVal!=_T("ReturnBufInstead"))
		{
			maker.SetResponseType(_T("text/html; charset=UTF-8"));
			char *tmp=THCharset::t2u8(ReturnVal);
			if (tmp)
			{
				ReturnValA=tmp;
				THCharset::free(tmp);
			}
		}
		else
		{
			char *tmp=THCharset::t2a(ReturnVal);
			if (tmp)
			{
				ReturnValA=tmp;
				THCharset::free(tmp);
			}
		}
		if (cntlen==0)
		{
			if (ReturnVal==_T("ReturnBufInstead"))
			{
				cntlen=ReturnBuf.GetBufferedSize();
				if (bUseGZip)
				{
					bUseGZip=FALSE;
					if (cntlen>0)
					{
						unsigned long gsize;
						if (THGZip::ZipEncode(ReturnBuf.GetBuf(NULL,FALSE),cntlen,&zipbuf,&gsize,THGZIP_GZIP))
						{
							//maker.AddHeaders(_T("gziplen"),THFormat(_T("%u"),gsize),FALSE);
							//maker.AddHeaders(_T("normallen"),THFormat(_T("%u"),cntlen),FALSE);
							if (gsize<(unsigned long)cntlen)
							{
								cntlen=gsize;
								bUseGZip=TRUE;
								maker.SetContentEncoding();
							}
							else
								THGZip::Free(zipbuf);
						}
					}
				}
			}
			else
			{
				cntlen=ReturnValA.GetLength();
				if (bUseGZip)
				{
					bUseGZip=FALSE;
					if (ReturnValA.GetLength()>0)
					{
						unsigned long gsize;
						if (THGZip::ZipEncode((LPCSTR)ReturnValA,cntlen,&zipbuf,&gsize,THGZIP_GZIP))
						{
							//maker.AddHeaders(_T("gziplen"),THFormat(_T("%u"),gsize),FALSE);
							//maker.AddHeaders(_T("normallen"),THFormat(_T("%u"),cntlen),FALSE);
							if (gsize<(unsigned long)cntlen)
							{
								cntlen=gsize;
								bUseGZip=TRUE;
								maker.SetContentEncoding();
							}
							else
								THGZip::Free(zipbuf);
						}
					}
				}
			}
		}
		if (StsCode==200 && cntlen==0) StsCode=204;
		maker.SetRange(cntlen,StartPos,EndPos);
		maker.SetResult(StsCode,_T(""),parser.GetVersion());
		THString outhead=maker.MakeHeaders();
		AddLog(LOG_NORMAL,_T("请求文件:%s 返回:%d %s"),parser.GetFirstHeader(1),maker.GetResultCode(),maker.GetResultText());
		THString logval=ReturnVal;
		if (mode==2 && ReturnVal.IsEmpty())
			logval=_T("文件：")+file;
		AddLog(LOG_DETAIL,_T("系统变量:%s\r\n响应时间:%d 毫秒\r\n请求:\r\n%s\r\n\r\n%s\r\n\r\n返回:\r\n%s%s\r\n"),sysparam,GetTickCount()-data->nSt,header,content,outhead,logval);
		m_TimeoutLock.Lock();
		m_TimeoutList.Remove(data);
		m_TimeoutLock.Unlock();

		char *tmp=THCharset::t2a(outhead);
		if (tmp)
		{
			m_psock->Send(sock,tmp,(int)strlen(tmp));
			THCharset::free(tmp);
		}
		if (mode==2 && fileidx!=-1 && ReturnValA.IsEmpty())
		{
			UINT sendlen;
			if (StartPos==-1) StartPos=0;
			if (EndPos==-1)
				sendlen=cntlen-StartPos;
			else
				sendlen=EndPos-StartPos;
			char buf[65535];
			if (bUseGZip)
			{
				unsigned char *rbuf=(unsigned char *)zipbuf;
				rbuf+=StartPos;
				while(1)
				{
					if (sendlen==0) break;
					UINT len;
					len=(sendlen>65535)?65535:sendlen;
					if (!m_psock->Send(sock,rbuf,len)) break;
					sendlen-=len;
					rbuf+=len;
				}
				pfs->FreeBuf(zipbuf);
			}
			else
			{
				while(1)
				{
					UINT read=pfs->ReadFile(fileidx,buf,65535);
					if (read==0 || sendlen==0) break;
					UINT len;
					if (sendlen>read)
						len=read;
					else
						len=sendlen;
						//if (len<read) len=read;
					sendlen-=len;
					if (!m_psock->Send(sock,buf,len)) break;
				}
				pfs->CloseFile(fileidx);
			}
		}
		else
		{
			if (bUseGZip && zipbuf)
			{
				m_psock->Send(sock,zipbuf,cntlen);
				THGZip::Free(zipbuf);
			}
			else if (cntlen>0)
			{
				if (ReturnVal==_T("ReturnBufInstead"))
					m_psock->Send(sock,ReturnBuf.GetBuf(NULL,FALSE),cntlen);
				else
					m_psock->Send(sock,(LPCSTR)ReturnValA,cntlen);
			}
		}
	}

	virtual void AcceptHandler(THRawSocket *cls,RAWSOCKET parent,RAWSOCKET sock,struct sockaddr_in *paddr,struct sockaddr_in *paddrpeer,void *adddata)
	{
	}

	virtual void ErrorHandler(THRawSocket *cls,RAWSOCKET sock,DWORD errcode,void *adddata)
	{
		ITHRawSocketBufferedEventHandler::ErrorHandler(cls,sock,errcode,adddata);
		AddLog(LOG_ERROR,_T("Socket %d 错误,代码:%d"),sock,errcode);
		//kill the processing thread
		m_TimeoutLock.Lock();
		THPosition pos=m_TimeoutList.GetStartPosition();
		THParsingData *data;
		while(!pos.IsEmpty())
		{
			if (m_TimeoutList.GetNextPosition(pos,data))
			{
				if (data && data->sock==sock)
				{
					//Time out Running
					KillThread(data->nThreadId,0);
					m_TimeoutList.Remove(data);
					//ReStart Thread
					StartThread(0,0,30000);
					//Record the Timeout Oper
					UINT len;
					char *buf=(char *)data->buf.GetBuf(&len,FALSE);
					AddLog(LOG_DETAIL,_T("客户取消连接: 执行时间: %d 毫秒\r\nHttp头:\r\n%s\r\n"),GetTickCount()-data->nSt,THCharset::a2t(buf,len));
					m_objbuf.ReleaseObj(data);
					break;
				}
			}
		}
		m_TimeoutLock.Unlock();
	}

	virtual void ThreadFunc(int threadid,void *adddata)
	{
		if (adddata==(void *)1)
		{
			if (m_nRunLimitTime!=0)
			{
				//time out check thread
				m_TimeoutLock.Lock();
				DWORD dw=GetTickCount();
				THPosition pos=m_TimeoutList.GetStartPosition();
				THParsingData *data;
				while(!pos.IsEmpty())
				{
					if (m_TimeoutList.GetNextPosition(pos,data))
					{
						if (data && dw-data->nSt>m_nRunLimitTime)
						{
							//Time out Running
							KillThread(data->nThreadId,0);
							m_TimeoutList.Remove(data);
							m_TimeoutLock.Unlock();
							//Reply to client
							THHttpHeaderResponseMaker maker;
							THString str=_T("Request Timeout");
							maker.SetServer(THHTTPSERVER);
							maker.SetResult(408);
							maker.SetResponseType(_T("text/html"));
							maker.SetRange(str.GetLength());
							UINT len;
							char *buf=(char *)data->buf.GetBuf(&len,FALSE);
							AddLog(LOG_ERROR,_T("请求超时: 执行时间: %d 毫秒 返回:%d %s\r\nHttp头:\r\n%s\r\n"),dw-data->nSt,maker.GetResultCode(),maker.GetResultText(),THCharset::a2t(buf,len));
							THString headers=maker.MakeHeaders();
							headers+=str;
							char *tmp=THCharset::t2a(headers);
							if (tmp)
							{
								m_psock->Send(data->sock,tmp,(int)strlen(tmp));
								THCharset::free(tmp);
							}
							m_objbuf.ReleaseObj(data);
							//ReStart Thread
							StartThread(0,0,30000);
							m_TimeoutLock.Lock();
							pos=m_TimeoutList.GetStartPosition();
						}
					}
				}
				m_TimeoutLock.Unlock();
			}
			//check session timeout
			if (m_bUseSession)
			{
				m_SessionLock.Lock();
				THPosition pos=m_sessionmapping.GetStartPosition();
				THString key;
				SessionInfo *info;
				DWORD dw=GetTickCount();
				while(!pos.IsEmpty())
				{
					if (m_sessionmapping.GetNextPosition(pos,key,info))
					{
						if (info)
						{
							if (info->nLastTime+m_nSessionLimitTime<dw)
							{
								m_sessionmapping.RemoveAt(key);
								pos=m_sessionmapping.GetStartPosition();
							}
						}
					}
				}
				m_SessionLock.Unlock();
			}
			//check filecache
			if (m_nFileCacheMode>=10)
			{
				//check free memory state
				MEMORYSTATUS st;
				THSysInfo::GetMemoryStatus(st);
				m_nFileCacheSize=(UINT)st.dwAvailPhys;
			}
			if (m_nFileCacheLast!=m_nFileCacheSize)
			{
				m_pStore->SetMemCacheSettings(m_nFileCacheSize);
				m_nFileCacheLast=m_nFileCacheSize;
			}
			return;
		}
		m_ParseLock.Lock();
		THParsingData *data=m_ParseList.GetHead(TRUE);
		m_ParseLock.Unlock();
		if (data)
		{
			data->nSt=GetTickCount();
			data->nThreadId=threadid;
			//AddLog(LOG_DETAIL,_T("线程 %d 处理中..."),threadid);
			//remove in _BufferedReceiveHandler Run Finish
			m_TimeoutLock.Lock();
			m_TimeoutList.AddHead(data);
			m_TimeoutLock.Unlock();
			UINT len;
			void *buf=data->buf.GetBuf(&len,FALSE);
			_BufferedReceiveHandler(data,buf,len);
			m_objbuf.ReleaseObj(data);
		}
		if (GetThreadCount()>m_nParseThread)
		{
			AddLog(LOG_DETAIL,_T("结束线程%d，线程数:%d"),threadid,GetThreadCount()-1);
			SelfKillThread(threadid);
		}
	}

	void AddLog(int nLogMode,const TCHAR* lpszMsg,...)
	{
		if (nLogMode & m_nLogMode)
		{
			va_list pArg;
			va_start(pArg,lpszMsg);
			THTime ti;
			THString str;
			str=ti.Format(_T("[%m-%d %H:%M:%S]:"));
			if (nLogMode==LOG_NORMAL)
				str+=_T("信息:");
			else if (nLogMode==LOG_ERROR)
				str+=_T("错误:");
			else if (nLogMode==LOG_DETAIL)
				str+=_T("详情:");
			str.AppendFormatV(lpszMsg,pArg);
			va_end(pArg);
			str+=_T("\r\n");
			THFile file;
			if (file.Open(m_sLogFile,THFile::modeWrite|THFile::modeNoTruncate|THFile::modeCreate))
			{
				file.SeekToEnd();
				file.Write((LPCTSTR)str,str.GetLength()*sizeof(TCHAR));
				file.Close();
			}
		}
	}
	THString GetDirList(THString filepath,THHttpHeaderParser *parser,ITHFileStore *pfs,THString sysparam)
	{
		if (filepath.GetLength()>1 && filepath.GetAt(filepath.GetLength()-1)!='\\') filepath+=_T("\\");
		THString rootps=THSimpleXml::GetParam(sysparam,_T("virtualroot"));
		//如果有virtualroot，更新相对根目录
		if (!rootps.IsEmpty()) rootps=_T("/")+rootps;
		THString relpath=_T("/")+pfs->GetRelPath(filepath,TRUE);
		THString parentdir=_T("/")+pfs->GetRelPath(pfs->GetParentPath(filepath),TRUE);
		if (parentdir!=_T("/")) parentdir+=_T("/");
		//如果已经没有上一级，把virtualroot退掉
		if (relpath!=_T("/"))
			parentdir=rootps+parentdir;
		THString ret;
		ret=_T("<html><head><meta http-equiv=\"content-type\" content=\"text/html; charset=GB2312\"><title>")+parser->GetHost()+_T(" - ")+rootps+relpath+_T("</title></head><body><H1>")+parser->GetHost()+_T(" - ")+rootps+relpath+_T("</H1><hr><pre><A HREF=\"")+parentdir+_T("\">[上级目录]</A><br><br>");
		THStringArray dir,file;
		if (pfs->FindFiles(filepath,&dir,&file))
		{
			THTime mt;
			UINT size;
			for(int i=0;i<dir.GetSize();i++)
			{
				if (pfs->GetFileAttrib(filepath+dir[i],NULL,NULL,&mt))
					ret.AppendFormat(_T("%40s%15s <a href=\"%s/\">%s</a><br>"),mt.Format(),_T("<目录>"),dir[i],dir[i]);
			}
			for(i=0;i<file.GetSize();i++)
			{
				if (pfs->GetFileAttrib(filepath+file[i],&size,NULL,&mt))
					ret.AppendFormat(_T("%40s%15u <a href=\"%s\">%s</a><br>"),mt.Format(),size,file[i],file[i]);
				else
					ret.AppendFormat(_T("%40s ---文件过大--- %s<br>"),mt.Format(),file[i]);
			}
		}
		ret+=_T("</pre><hr>") THHTTPSERVER _T("</body></html>");
		return ret;
	}

	/**
	* @brief 外部处理命令重载函数
	* @param file			请求的文件名
	* @param pfs			文件系统
	* @param parser			头部处理器
	* @param maker			返回头制造器，需要额外增加header在这里设置
	* @param getparam		get参数列表，改动对返回生效，get参数可使用GetValue函数获取
	* @param postparam		post参数列表，改动对返回生效
	* @param bContentXml	post参数是否为xml格式
	* @param sysparam		sys参数列表，改动对返回生效
	* @param StsCode		返回代码
	* @param ReturnVal		返回内容
	* @param ReturnBuf		返回二进制内容，这时ReturnVal应为_T("ReturnBufInstead")
	* @param parsingdata	对应请求的信息，可以得到socket类厂，回复socket，请求buffer，对端地址，请求开始时间，线程id等
	* @return 返回0，表示外部处理已经全部处理完成，可直接返回，这时通过socket等直接向客户端返回数据；返回3，标识需要HttpServer返回数据，ReturnVal为返回内容，StsCode为返回代码；如果对请求进行处理完成后，想再继续原来的普通目录，文件，脚本调用，返回其他值
	*/
	virtual int ParseExtCmd(THString &file,ITHFileStore *pfs,THHttpHeaderParser *parser,THHttpHeaderResponseMaker *maker,THString &getparam,THString &postparam,BOOL &bContentXml,THString &sysparam,int &StsCode,THString &ReturnVal,THMemBuf *ReturnBuf,THParsingData *parsingdata)
	{
		StsCode=404;
		ReturnVal=_T("404 Not Found");
		return 3;
	}

	//获取get方式下的value
	THString GetValue(THString key,THString list)
	{
		THStringToken token(list,_T("&"));
		key=key+_T("=");
		THString val;
		while(token.IsMoreTokens())
		{
			val=token.GetNextToken();
			val.Trim();
			if (_tcsncmp(key,val,key.GetLength())==0)
			{
				val=val.Mid(key.GetLength());
				return THUrlExplain::UrlDecode(val);
			}
		}
		return _T("");
	}

	THString ParseScript(THString file,ITHFileStore *pfs,THHttpHeaderParser *parser,THString getparam,THString postparam,BOOL bContentXml,THString sysparam,THString cookieparam,THString &sessionparam,int &StsCode,THHttpHeaderResponseMaker &maker)
	{
		int idx=pfs->OpenFile(file,FILEMODE_READ);
		if (idx==-1)
		{
			StsCode=500;
			return _T("500 Internal Server Error");
		}
		UINT len=0;
		void *buf=pfs->ReadWholeFile(idx,&len);
		THString strscript;
		strscript=THCharset::a2t((char *)buf,len);
		//strscript.SetString((LPCTSTR)buf,len);
		pfs->FreeBuf(buf);
		pfs->CloseFile(idx);
		THString start=_T("<!--[ts:start]");
		THString end=_T("[ts:end]-->");
		THTiniXml xml;
		StsCode=200;
		while(1)
		{
			int pos=strscript.Find(start,0);
			if (pos==-1) break;
			int pos2=strscript.Find(end,pos+start.GetLength());
			if (pos2==-1) break;
			THString ret;
			THString strxml=strscript.Left(pos2).Mid(pos+start.GetLength());
			xml.SetContent(strxml);
			THString scrfile=xml.GetAttribute(_T("function"),_T("file"));
			THString fnname=xml.GetAttribute(_T("function"),_T("name"));
			if (!scrfile.IsEmpty() && !fnname.IsEmpty())
			{
				THString param;
				xml.IntoXml(_T("function"));
				THStringArray ar;
				if (xml.SplitAllKeys(&ar))
				{
					for(int i=0;i<ar.GetSize();i++)
					{
						xml.SetContent(ar[i]);
						THString pname=xml.GetAttribute(_T("param"),_T("name"));
						THString pval=xml.GetAttribute(_T("param"),_T("value"));
						THString pmethod=xml.GetAttribute(_T("param"),_T("method"));
						if (!pname.IsEmpty() && !pval.IsEmpty())
						{
							if (pmethod.CompareNoCase(_T("const"))==0 || pmethod.IsEmpty())
								param+=THSimpleXml::MakeParam(pname,pval);
							else if (pmethod.CompareNoCase(_T("get"))==0)
								param+=THSimpleXml::MakeParam(pname,GetValue(pval,getparam));
							else if (pmethod.CompareNoCase(_T("sysvalue"))==0)
								param+=THSimpleXml::MakeParam(pname,THSimpleXml::GetParam(sysparam,pval));
							else if (pmethod.CompareNoCase(_T("cookie"))==0)
								param+=THSimpleXml::MakeParam(pname,THSimpleXml::GetParam(cookieparam,pval));
							else if (pmethod.CompareNoCase(_T("session"))==0)
								param+=THSimpleXml::MakeParam(pname,THSimpleXml::GetParam(sessionparam,pval));
							else if (pmethod.CompareNoCase(_T("post"))==0)
							{
								if (bContentXml)
									param+=THSimpleXml::MakeParam(pname,THSimpleXml::GetParam(postparam,pval));
								else
									param+=THSimpleXml::MakeParam(pname,GetValue(pval,postparam));
							}
							else if (pmethod.CompareNoCase(_T("request"))==0)
							{
								THString tmp;
								if (bContentXml)
									tmp=THSimpleXml::GetParam(postparam,pval);
								else
									tmp=GetValue(pval,postparam);
								if (tmp.IsEmpty())
									tmp=GetValue(pval,getparam);
								param+=THSimpleXml::MakeParam(pname,tmp);
							}
						}
					}
				}
				//call function
				scrfile=pfs->GetFullPath(scrfile,pfs->GetCurPath(file));
				if (!pfs->IsPathFile(scrfile))
				{
					StsCode=500;
					return _T("500 Internal Server Error");
				}
				THScript script;
				script.SetParseExtCmdProc(m_scriptcb,m_scriptdata);
				script.SetParseExtCmdClass(m_scriptcls);
				script.Init(scrfile,pfs,FALSE);
				ret=script.RunScript(fnname,0,param);
				THString redirect=THSimpleXml::GetAndRemoveParam(&ret,_T("ReDirectUrl"));
				if (!redirect.IsEmpty())
				{
					StsCode=302;
					maker.AddHeaders(_T("Location"),redirect,FALSE);
				}
				while(1)
				{
					THString cookies=THSimpleXml::GetAndRemoveParam(&ret,_T("Cookie"));
					if (cookies.IsEmpty()) break;
					UINT tv=THs2u(THSimpleXml::GetAndRemoveParam(&cookies,_T("Expires")));
					THString path=THSimpleXml::GetAndRemoveParam(&cookies,_T("Path"));
					THTime *pti=NULL;
					if (tv!=0)
					{
						THTime ti;
						ti.SetTime((__time64_t)tv,FALSE);
						pti=&ti;
					}
					maker.AddCookie(cookies,pti,path);
				}
				THStringToken t;
				while(1)
				{
					THString session=THSimpleXml::GetAndRemoveParam(&ret,_T("Session"));
					if (session.IsEmpty()) break;
					t.Init(session,_T("="));
					THString key=t.GetNextToken().Trim();
					THString value=t.GetNextToken().Trim();
					THSimpleXml::SetParam(&sessionparam,key,value);
				}
				if (THs2u(THSimpleXml::GetAndRemoveParam(&ret,_T("NoCache"))))
				{
					maker.AddCacheControl(_T("no-cache"));
					maker.AddHeaders(_T("Pragma"),_T("no-cache"));
				}
			}
			strscript=strscript.Left(pos)+ret+strscript.Mid(pos2+end.GetLength());
		}
		return strscript;
	}

	THString MakeSessionId(U32 ip)
	{
		srand(ip+GetTickCount());
		THString ret;
		ret=_T("ts");
		for(int i=0;i<16;i++)
		{
			ret.AppendFormat(_T("%c"),'a'+rand()%26);
		}
		ret+=_T("se");
		return ret;
	}

	void GetSessionId(THString &sOrgFile,THString &sessionid)
	{
		sessionid.Empty();
		THString sTmp=sOrgFile;
		if (sTmp.GetAt(0)=='/') sTmp=sTmp.Mid(1);
		if (sTmp.Left(2)==_T("ts") && sTmp.Mid(18).Left(2)==_T("se"))
		{
			if (THStringConv::IsStringLetter(sTmp.Left(20)))
			{
				sessionid=sTmp.Left(20);
				sOrgFile=sTmp.Mid(20);
				if (sOrgFile.IsEmpty()) sOrgFile=_T("/");
			}
		}
	}

	void SetCustomCodePageMappingFile(int StsCode,THString sFileStoreValue,THString sFile)
	{
		//fixme
		//m_custommap.SetAt(StsCode,sFileStoreValue);
	}

	void SetCustomCodePageMappingString(int StsCode,THString sValue)
	{
		//fixme
	}

	void GetCustomCodePage(int &StsCode,THString &ReturnVal)
	{
		//fixme custom page mapping
		//400
		//401
		//404
		//403
		//500
	}

	inline THString MakeOpaque()
	{
		return MakeNonce(0);
	}

	THString MakeNonce(UINT nip)
	{
		THString str;
		srand(nip+GetTickCount());
		str.Format(_T("%d%d%d%d%d"),rand(),rand(),rand(),rand(),rand());
		return THMd5::CalcStrMd5Str(str,FALSE);
	}

	virtual void *QuerySystemInterface(THString fnName)
	{
		return NULL;
	}

	static void FreeFileStore(void *key,void *value,void *adddata)
	{
		ITHFileStore *fs=(ITHFileStore *)value;
		if (fs) fs->DeleteObjectByFreeor(adddata);
	}

	static void FreeSession(void *key,void *value,void *adddata)
	{
		delete (SessionInfo *)value;
	}

	THRawSocket *m_psock;					//<<< socket factory
	RAWSOCKET m_s;
	ITHFileStore *m_pStore;
	THFileStore m_Store;
	THParseExtCmdProc m_scriptcb;
	const void* m_scriptdata;
	THScript *m_scriptcls;
	int m_nLogMode;
	THString m_sLogFile;
	THList<THParsingData *> m_ParseList;					//<<<处理列表
	THObjectBuffer<THParsingData> m_objbuf;			//<<<并发处理对象池
	THMutex m_ParseLock;											//<<<并发对象锁
	THList<THParsingData *> m_TimeoutList;			//<<<正在处理中的线程链表
	THMutex m_TimeoutLock;						//<<<正在处理中的线程链表锁
	THStringMap<SessionInfo *>m_sessionmapping;	//<<<session变量存储区
	THMutex m_SessionLock;						//<<<session锁
	UINT m_nFileCacheLast;						//<<<最后一次修改的缓存大小值
	ITHHttpServerPluginTypeSocketLayerReplacer *m_pSLReplacer;	//<<<Socket层替换器
public:
	//http server settings;
	BOOL m_bListDirectory;					//<<<是否允许列目录，默认允许
	BOOL m_bUseDefaultFile;					//<<<是否使用默认文件，默认使用
	THStringArray m_aDefaultFileList;		//<<<默认文件列表
	BOOL m_bSepService;						//<<<是否使用特别服务，默认不使用
	THStringArray m_aSepServiceList;		//<<<特别服务列表，如m_aSepServiceList.Add(_T("/request"));
	THStringArray m_aSepServiceDirList;		//<<<特别服务目录列表，如m_aSepServiceDirList.Add(_T("/request/"));
	BOOL m_bUseScript;						//<<<是否使用脚本，默认使用
	THString m_sScriptExtList;				//<<<脚本文件后缀.tsp.头尾必需含有.
	BOOL m_bUseAcceptExt;					//<<<是否使用只允许下载已知后缀名文件模式，默认不使用
	THString m_sAcceptExtList;				//<<<已知后缀名列表
	BOOL m_bUploadSaveToFile;				//<<<Post的文件数据是否保存为文件
	THString m_sUploadSaveBasePath;			//<<<指定上传的文件保存的路径，为空为当前目录，绝对路径
	BOOL m_bUploadAutoRename;				//<<<如果文件已存在则自动改名
	UINT m_nParseThread;											//<<<并发处理线程的数量，在StartListen前指定
	UINT m_nMaxParseThread;									//<<<当后台进程处理不及时，允许新开线程的上限值，0为不指定上限
	UINT m_nRunLimitTime;						//<<<限制线程的运行时间，0为不限制，主要是脚本
	UINT m_nCharsetMode;						//<<<Url中文编码方式
	BOOL m_bUseHostName;						//<<<是否支持多域名
	THStringMap<ITHFileStore *>m_hostmapping;	//<<<多域名存储系统映射
	BOOL m_bUseVirtualDir;						//<<<是否支持虚拟目录
	THStringMap<ITHFileStore *>m_dirmapping;	//<<<虚拟目录映射

	BOOL m_bUseSession;							//<<<是否使用session机制
	BOOL m_bSessionCookielessMode;				//<<<是否使用CookieLess模式
	UINT m_nSessionLimitTime;					//<<<session生存时间

	BOOL m_bAuth;								//<<<是否使用认证
	BOOL m_bDigestMode;							//<<<是否使用摘要模式
	THString m_sAuthName;						//<<<授权名称
	THStringArray m_aAccountList;				//<<<账号列表，username:password

	UINT m_nFileCacheSize;						//<<<文件缓存大小
	UINT m_nFileCacheMode;						//<<<文件缓存方式，0为不使用，1为使用固定值，10-80 为使用空余内存的比例
	UINT m_nFileAge;							//<<<浏览器文件缓存时间，0为不使用，秒数，设置后会在文件请求时返回 Cache-Control: Max-age=sec
	BOOL m_bGZip;								//<<<是否支持GZip压缩

	BOOL m_bLimitIp;							//<<<是否限制ip
	THIpRange m_allowrange;						//<<<允许范围
	THIpRange m_blockrange;						//<<<禁止范围

	BOOL m_bUsePlugin;							//<<<是否使用插件
};

#define	HTMLDATA_MOOTOOLS				0x1
#define HTMLDATA_CALENDAR				0x2

/**
* @brief 集成常用Html服务器类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2009-05-23 新建类
*/
/**<pre>
	实现逻辑是通过标准httpserver中挂接对应文件的特殊服务实现
</pre>
*/
class THHtmlServer:public THHttpServer
{
public:
	THHtmlServer()
	{
		m_bSepService=TRUE;
		//m_aSepServiceList.Add(_T("/html/mootools.js"));//ver1.2.1
		//m_aSepServiceList.Add(_T("/html/caledar.js"));
		m_nDataMode=HTMLDATA_MOOTOOLS|HTMLDATA_CALENDAR;
	}
	virtual ~THHtmlServer()
	{
	}
	void SetHtmlData(UINT DataMode){m_nDataMode=DataMode;}
	virtual int ParseExtCmd(THString &file,ITHFileStore *pfs,THHttpHeaderParser *parser,THHttpHeaderResponseMaker *maker,THString &getparam,THString &postparam,BOOL &bContentXml,THString &sysparam,int &StsCode,THString &ReturnVal,THMemBuf *ReturnBuf,THParsingData *parsingdata)
	{
		if (file==_T("/html/mootools.js"))
		{
			ReturnVal=_T("data");
		}
		else if (file==_T("/html/caledar.js"))
		{
			ReturnVal=_T("data");
		}
		else
			return THHttpServer::ParseExtCmd(file,pfs,parser,maker,getparam,postparam,bContentXml,sysparam,StsCode,ReturnVal,ReturnBuf,parsingdata);
		StsCode=200;
		return 3;
	}
private:
	UINT m_nDataMode;
};

/**
* @brief C++数据Html展现服务器类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2009-05-23 新建类
*/
/**<pre>
	用于C中数据在Html展现
	实现逻辑是通过标准httpserver中挂接一个特殊服务，html/dyndata 实现数据请求
	Html加载完成后，用js不停加载
	Json返回格式：
	{"list":[
		{
			"oper":"set",
			"item":"test1",
			"value":"data1"
		},
		{
			"oper":"set",
			"item":"test1",
			"value":"data1"
		}
	]}
	操作列表：
		set,item,value设置内容
		add,item,value设置内容，如果不存在则添加
		append,item,value追加内容
		table_addrow,item,{value|value|value}	增加一行
		set,item,{x|y|value}		维护table中的内容
		table_delrow,item,y			删除table中的内容
		setid,id					设置更新时使用的id
	显示出错信息
	AddJob(ReturnVal,_T("set"),_T("html4cInfo"),_T("请求失败，实例id不能为空。"));
</pre>
*/
class THHtml4CServer:public THHtmlServer
{
public:
	THHtml4CServer()
	{
		m_aSepServiceList.Add(_T("/html/html4cdyndata.js"));
	}
	virtual ~THHtml4CServer()
	{
	}
	void UpdateHtmlData(THString item,THString value)
	{
		m_parsemtx.Lock();
		AllDataMap::iterator it=m_alldata.find(item);
		if (it==m_alldata.end())
			m_alldata.insert(AllDataMap::value_type(item,value));
		else
			it->second=value;
		THString ret;
		AddJob(ret,_T("set"),item,value);
		DWORD ti=GetTickCount()-10000;
		UpdateDataMap::iterator it1=m_updatedata.begin();
		for(;it1!=m_updatedata.end();it1++)
		{
			if (it1->second.lti<ti)
				m_updatedata.erase(it1++);
			else
			{
				if (!it1->second.info.IsEmpty()) it1->second.info+=_T(",");
				it1->second.info+=ret;
			}
		}
		m_parsemtx.Unlock();
	}
	void AppendHtmlData(THString item,THString value)
	{
		m_parsemtx.Lock();
		AllDataMap::iterator it=m_alldata.find(item);
		if (it==m_alldata.end())
			m_alldata.insert(AllDataMap::value_type(item,value));
		else
			it->second+=value;
		THString ret;
		AddJob(ret,_T("append"),item,value);
		DWORD ti=GetTickCount()-10000;
		UpdateDataMap::iterator it1=m_updatedata.begin();
		for(;it1!=m_updatedata.end();it1++)
		{
			if (it1->second.lti<ti)
				m_updatedata.erase(it1++);
			else
			{
				if (!it1->second.info.IsEmpty()) it1->second.info+=_T(",");
				it1->second.info+=ret;
			}
		}
		m_parsemtx.Unlock();
	}
	//fixme,how to save it to alldata
	void UpdateHtmlTableData(THString tblname,int x,int y,THString value)
	{
		CString str;
		value.Replace(_T("|"),_T("HTML4CREPLACEA"));
		str.Format(_T("%d|%d|%s|"),x,y,value);
		AppendHtmlData(tblname,str);
	}
	void AddJob(THString &ret,THString oper,THString item,THString value)
	{
		if (!ret.IsEmpty()) ret+=_T(",");
		//json对"处理好像都不一样，这里忽略这个符号，其他符号{}[]:,'貌似都不用处理
		value.Replace(_T("\""),_T("HTML4CREPLACEB"));
		value=THStringConv::EncodeUString(value);
		ret.AppendFormat(_T("{\"oper\":\"%s\",\"item\":\"%s\",\"value\":\"%s\"}"),oper,item,value);
	}
	virtual int ParseExtCmd(THString &file,ITHFileStore *pfs,THHttpHeaderParser *parser,THHttpHeaderResponseMaker *maker,THString &getparam,THString &postparam,BOOL &bContentXml,THString &sysparam,int &StsCode,THString &ReturnVal,THMemBuf *ReturnBuf,THParsingData *parsingdata)
	{
		if (file==_T("/html/html4cdyndata.js"))
		{
			m_parsemtx.Lock();
			int id=_ttoi(GetValue(_T("id"),getparam));
			UpdateDataMap::iterator it=m_updatedata.find(id);
			if (it!=m_updatedata.end())
			{
				//have data
				it->second.lti=GetTickCount();
				ReturnVal=it->second.info;
				it->second.info.Empty();
			}
			else
			{
				UpdateInfo info;
				info.lti=GetTickCount();
				m_updatedata.insert(UpdateDataMap::value_type(id,info));
				//put in all data
				AllDataMap::iterator it1=m_alldata.begin();
				for(;it1!=m_alldata.end();it1++)
					AddJob(ReturnVal,_T("set"),it1->first,it1->second);
			}
			m_parsemtx.Unlock();
			ReturnVal=_T("{\"list\":[")+ReturnVal+_T("]}");
		}
		else
			return THHtmlServer::ParseExtCmd(file,pfs,parser,maker,getparam,postparam,bContentXml,sysparam,StsCode,ReturnVal,ReturnBuf,parsingdata);
		StsCode=200;
		return 3;
	}
protected:
	typedef struct _UpdateInfo
	{
		CString info;
		DWORD lti;
	}UpdateInfo;
	typedef map<CString,CString> AllDataMap;
	typedef map<int,UpdateInfo> UpdateDataMap;
	AllDataMap m_alldata;
	UpdateDataMap m_updatedata;
	THMutex m_parsemtx;
};
