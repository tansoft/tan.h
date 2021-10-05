#pragma once

#define LIVEUPDATE_NOTNEEDUPDATE	0
#define LIVEUPDATE_NETWORKERROR		1
#define LIVEUPDATE_FILEERROR		2
#define LIVEUPDATE_USERCANCEL		3
#define LIVEUPDATE_FINISH			4

#include <THINet.h>
#include <THIni.h>
#include <THArith.h>
#include <THStruct.h>
#include <THThread.h>
#include <THSystem.h>

class THLiveUpdate;

/**
* @brief 自动更新响应封装类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-08-20 新建类
*/
class ITHLiveUpdateEventHandler
{
public:
	/**
	* @brief 开始进行自动更新
	* @param pUpdate			THLiveUpdate类实例指针
	* @return 是否取消
	*/
	virtual BOOL OnStartLiveUpdate(THLiveUpdate *pUpdate)
	{
		return FALSE;
	}
	
	/**
	* @brief 完成自动更新，无论成功失败都引发调用该接口
	* @param pUpdate			THLiveUpdate类实例指针
	* @param nReason			下载结果码，LIVEUPDATE_XXX
	*/
	virtual void OnFinishLiveUpdate(THLiveUpdate *pUpdate,int nReason)
	{
	}
	/**
	* @brief 更新状态信息
	* @param pUpdate			THLiveUpdate类实例指针
	* @param sMsg				更新状态信息
	*/
	virtual void OnProcessStateMessage(THLiveUpdate *pUpdate,THString sMsg)
	{
	}
	/**
	* @brief 更新进度信息
	* @param pUpdate			THLiveUpdate类实例指针
	* @param nPosAll			总进度百分比，100为完成
	* @param nPosCurrent		当前任务百分比，100为完成
	* @param nTotal				总需要下载大小
	* @param nFinished			已完成大小
	* @param nSpeedPreSecond	当前下载速度
	*/
	virtual void OnProcessPercent(THLiveUpdate *pUpdate,UINT nPosAll,UINT nPosCurrent,UINT nTotal,UINT nFinished,UINT nSpeedPreSecond)
	{
	}
	/**
	* @brief 找到可用更新
	* @param pUpdate			THLiveUpdate类实例指针
	* @param nMax				可用更新总个数，这时可调用THLiveUpate中的GetUpdateInfo完成信息的读取
	* @return 是否取消
	*/
	virtual BOOL OnFoundedUpdate(THLiveUpdate *update,int nMax)
	{
		return FALSE;
	}
	/**
	* @brief 开始进行安装
	* @param pUpdate			THLiveUpdate类实例指针
	* @return 是否取消
	*/
	virtual BOOL OnStartInstall(THLiveUpdate *pUpdate)
	{
		return FALSE;
	}
};

#define	LIVEUPDATESTATE_IDLE		0
#define LIVEUPDATESTATE_QUERYING	1
#define LIVEUPDATESTATE_DOWNING		2

/**
* @brief 自动更新底层类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-08-20 新建类
*/
/**<pre>
自动更新包含功能：
	1.定时更新
	2.断点续传
	3.失败自动重试
配置文件格式：
	[天塑通讯录关键更新1203]										//更新模块标题名
	desc=天塑通讯录关键更新，针对XXX问题作出了修正					//更新内容描述
	size=2412523													//更新程序大小
	url=http://liveupdate.tansoft.cn/tanbook/tanbookcore1203.exe	//更新程序下载地址
	md5=CD24CDA213BDD224326EF										//更新程序md5
	type=1															//更新形式，1：为关键更新，必须安装，0：为选装更新（默认不选中）
自动更新流程：
	1.开始liveupdate（直接开始或Timeout后开始）
	2.回调OnStartLiveUpdate开始自动更新
	3.请求sUrl的配置文件
	4.
用法：
</pre>*/
class THLiveUpdate : public THDownloadEventHandler
{
public:
	THLiveUpdate()
	{
		m_list.SetFreeProc(FreeCallBack);
		m_nReTryMin=0;
		m_nFailedReTryMin=0;
		m_nFailReTryCount=2;
		m_nState=LIVEUPDATESTATE_IDLE;
		m_handler=NULL;
		m_nTrying=0;
		m_nDownloading=0;
		m_bSuccessed=FALSE;
		m_SleepTime=0;
		m_nLastUpdateTime=0;
		m_nTotalDown=0;
		m_nFinished=0;
		m_timer.SetCallFunc(MyTimerCallBack);
		m_http.SetHandler(this);
	}
	virtual ~THLiveUpdate()
	{
		CancelLiveUpdate();
	}

	void CancelLiveUpdate()
	{
		m_http.CancelAllDownload();
		m_timer.StopAllTimer();
		m_list.RemoveAll();
		m_nState=LIVEUPDATESTATE_IDLE;
		m_arInstall.RemoveAll();
		m_arMd5.RemoveAll();
	}

	void SetHandler(ITHLiveUpdateEventHandler *handler){m_handler=handler;}

	/**
	* @brief 初始化自动更新
	* @param sPath				自动更新使用的目录
	* @param sTitle				自动更新模块名称，如：天塑通讯录
	* @param sUrl				自动更新配置文件url
	* @param nReTryMin			自动更新定时更新间隔，单位为分钟，0为不进行定时更新
	* @param nFailedReTryMin	自动更新失败定时更新间隔，单位为分钟，0为不进行定时更新
	* @param nFailReTryCount	更新过程失败后重试次数
	* @return 是否成功
	*/
	BOOL InitLiveUpdate(THString sPath,THString sTitle,THString sUrl,UINT nReTryMin=0,UINT nFailedReTryMin=0,UINT nFailReTryCount=2)
	{
		if (m_nState!=LIVEUPDATESTATE_IDLE) return FALSE;
		m_ini.Init(sPath+_T("\\LiveUpdated.ini"));
		m_sPath=sPath;
		m_sTitle=sTitle;
		m_sUrl=sUrl;
		m_nReTryMin=nReTryMin;
		m_nFailedReTryMin=nFailedReTryMin;
		m_nFailReTryCount=nFailReTryCount;
		m_nLastUpdateTime=0;
		m_nTotalDown=0;
		m_nFinished=0;
		m_timer.StartTimer(1,60000,TRUE,this);
		return _StartLiveUpdate();
	}

	/**
	* @brief 获取每个安装的信息
	* @param idx				安装索引
	* @param key				取值的键名
	* @return 键值
	*/
	THString GetUpdateInfo(int idx,const THString key)
	{
		LiveUpdateInfo *info;
		if (m_list.GetAt(idx,info,FALSE))
			return THSimpleXml::GetParam(info->sKey,key);
		return _T("");
	}

	/**
	* @brief 设置是否安装更新
	* @param idx				安装索引
	* @param bInstall			是否安装
	*/
	void SetInstallUpdate(int idx,BOOL bInstall)
	{
		LiveUpdateInfo *info;
		if (m_list.GetAt(idx,info,FALSE))
		{
			info->bInstall=bInstall;
		}
	}

	virtual BOOL OnStartDownload(const THString url,class THINetHttp *cls,void *adddata)
	{
		if (m_handler)
		{
			if (m_nState==LIVEUPDATESTATE_QUERYING)
			{
				m_handler->OnProcessStateMessage(this,_T("正在查询可用的更新列表..."));
			}
			else if (m_nState==LIVEUPDATESTATE_DOWNING)
			{
				THString msg;
				msg.Format(_T("正在下载第 %d 个更新..."),m_nDownloading);
				m_handler->OnProcessStateMessage(this,msg);
			}
		}
		return FALSE;
	}
	virtual BOOL OnDownloading(const THString url,class THINetHttp *cls,void *adddata,unsigned int current,unsigned int down,unsigned int total)
	{
		if (m_nState==LIVEUPDATESTATE_DOWNING)
		{
			DWORD curtime=GetTickCount();
			m_nFinished+=current;
			UINT speed=0;
			if (m_nLastUpdateTime!=0 && curtime>m_nLastUpdateTime)
				speed=(UINT)(current*1000.0/(curtime-m_nLastUpdateTime));
			UINT post=0,posc=0;
			if (m_nTotalDown!=0) post=(UINT)(m_nFinished*100.0/m_nTotalDown);
			if (total!=0) posc=(UINT)(down*100.0/total);
			if (m_handler) m_handler->OnProcessPercent(this,post,posc,m_nTotalDown,m_nFinished,speed);
		}
		return FALSE;
	}
	/**
	* @brief 任务结束时引发该事件
	* @param url		下载地址
	* @param adddata	附加数据
	* @param ErrCode	错误代码，0为没有错误
	* @param down		已下载内容长度
	* @param filename	文件下载模式时，该值为文件名
	* @param data		内存下载模式时，该值为下载内容，回调调用完毕后，将会释放该内存
	* @param len		内存下载模式时，该值为下载内容长度
	*/
	virtual void OnDownloadFinish(const THString url,class THINetHttp *cls,void *adddata,int ErrCode,unsigned int down,const THString filename,const void *data,unsigned int len)
	{
		BOOL bPassState=FALSE;
		if (m_nState==LIVEUPDATESTATE_QUERYING)
		{
			if (ErrCode!=DownloadError_Ok)
			{
				//处理下载错误
				if (ErrCode==DownloadError_FileError)
				{
					m_nState=LIVEUPDATESTATE_IDLE;
					if (m_handler) m_handler->OnFinishLiveUpdate(this,LIVEUPDATE_FILEERROR);
				}
				else
				{
					if (m_nTrying<m_nFailReTryCount)
					{
						if (m_handler) m_handler->OnProcessStateMessage(this,_T("无法访问网络，正在重试..."));
						m_nTrying++;
						if (m_http.StartFileDownload(m_sUrl,m_sPath+_T("\\LiveUpdate.ini"))==0)
						{
							m_nState=LIVEUPDATESTATE_IDLE;
							if (m_handler) m_handler->OnFinishLiveUpdate(this,LIVEUPDATE_NETWORKERROR);
						}
					}
					else
					{
						if (m_handler) m_handler->OnProcessStateMessage(this,_T("无法访问网络。"));
						m_nState=LIVEUPDATESTATE_IDLE;
						if (m_handler) m_handler->OnFinishLiveUpdate(this,LIVEUPDATE_NETWORKERROR);
					}
				}
			}
			else
			{
				m_nTrying=0;
				m_nState=LIVEUPDATESTATE_DOWNING;
				//parse the result
				THIni ini;
				ini.Init(m_sPath+_T("\\LiveUpdate.ini"));
				THStringArray ar;
				if (!ini.EnumAllStruct(&ar))
				{
					//下载的文件有错
					m_nState=LIVEUPDATESTATE_IDLE;
					if (m_handler) m_handler->OnFinishLiveUpdate(this,LIVEUPDATE_NETWORKERROR);
				}
				else
				{
					m_list.RemoveAll();
					for(int i=0;i<ar.GetSize();i++)
					{
						THString key=ar[i];
						THString url=ini.GetString(key,_T("url"));
						THString md5=ini.GetString(key,_T("md5"));
						//fixme check the md5 is already installed
						if (!url.IsEmpty() && !md5.IsEmpty())
						{
							if (m_ini.GetString(_T("Installed"),md5).IsEmpty())
							{
								LiveUpdateInfo *tmp=new LiveUpdateInfo;
								if (tmp)
								{
									THSimpleXml::SetParam(&tmp->sKey,_T("md5"),md5);
									THSimpleXml::SetParam(&tmp->sKey,_T("url"),url);
									THSimpleXml::SetParam(&tmp->sKey,_T("size"),ini.GetString(key,_T("size")));
									THSimpleXml::SetParam(&tmp->sKey,_T("desc"),ini.GetString(key,_T("desc")));
									tmp->bInstall=ini.GetBool(key,_T("type"),FALSE);
									m_list.AddHead(tmp);
								}
							}
						}
					}
					if (m_list.GetSize()==0)
					{
						m_nState=LIVEUPDATESTATE_IDLE;
						m_bSuccessed=TRUE;
						if (m_handler) m_handler->OnFinishLiveUpdate(this,LIVEUPDATE_NOTNEEDUPDATE);
						return;
					}
					if (m_handler)
					{
						if (m_handler->OnFoundedUpdate(this,m_list.GetSize()))
						{
							m_nState=LIVEUPDATESTATE_IDLE;
							m_handler->OnFinishLiveUpdate(this,LIVEUPDATE_USERCANCEL);
							return;
						}
					}
					m_nDownloading=0;
					m_nState=LIVEUPDATESTATE_DOWNING;
					bPassState=TRUE;
					m_nLastUpdateTime=0;
					m_nFinished=0;
					m_nTotalDown=0;
					THPosition pos=m_list.GetStartPosition();
					LiveUpdateInfo *tmp;
					while(!pos.IsEmpty())
					{
						if (m_list.GetNextPosition(pos,tmp))
						{
							if (tmp->bInstall)
							{
								m_nTotalDown+=THStringConv::s2u(THSimpleXml::GetParam(tmp->sKey,_T("size")));
							}
						}
					}
				}
			}
		}
		if (m_nState==LIVEUPDATESTATE_DOWNING)//这里不能用else if，上面状态改变后直接调用
		{
			if (ErrCode!=DownloadError_Ok)
			{
				//处理下载错误
				if (ErrCode==DownloadError_FileError)
				{
					m_nState=LIVEUPDATESTATE_IDLE;
					if (m_handler) m_handler->OnFinishLiveUpdate(this,LIVEUPDATE_FILEERROR);
				}
				else
				{
					if (m_nTrying<m_nFailReTryCount)
					{
						m_nTrying++;
						THString url=GetUpdateInfo(0,_T("url"));
						THString md5=GetUpdateInfo(0,_T("md5"));
						if (!url.IsEmpty() && !md5.IsEmpty())
						{
							if (m_handler) m_handler->OnProcessStateMessage(this,_T("无法访问网络，正在重试..."));
							if (m_http.StartFileDownload(url,m_sPath+_T("\\")+md5+_T(".exe"))==0)
							{
								m_nState=LIVEUPDATESTATE_IDLE;
								if (m_handler) m_handler->OnFinishLiveUpdate(this,LIVEUPDATE_NETWORKERROR);
							}
						}
						else
						{
							if (m_handler) m_handler->OnProcessStateMessage(this,_T("无法访问网络。"));
							m_nState=LIVEUPDATESTATE_IDLE;
							if (m_handler) m_handler->OnFinishLiveUpdate(this,LIVEUPDATE_USERCANCEL);
						}
					}
					else
					{
						m_nState=LIVEUPDATESTATE_IDLE;
						if (m_handler) m_handler->OnFinishLiveUpdate(this,LIVEUPDATE_NETWORKERROR);
					}
				}
			}
			else
			{
				if (!bPassState)
				{
					//fixme check md5
					//if wrong trying
					THString url=GetUpdateInfo(0,_T("url"));
					THString md5=GetUpdateInfo(0,_T("md5"));
					if (url.IsEmpty() || md5.IsEmpty())
					{
						m_nState=LIVEUPDATESTATE_IDLE;
						if (m_handler) m_handler->OnFinishLiveUpdate(this,LIVEUPDATE_USERCANCEL);
						return;
					}
					else
					{
						if (m_handler) m_handler->OnProcessStateMessage(this,_T("文件下载完成。正在检查完整性..."));
						if (THMd5::IsMd5Same(THMd5::CalcFileMd5Str(filename),md5))
						{
							m_nTrying=0;
							m_arInstall.Add(m_sPath+_T("\\")+md5+_T(".exe"));
							m_arMd5.Add(md5);
							m_list.RemoveHead();
						}
						else
						{
							if (m_nTrying<m_nFailReTryCount)
							{
								if (m_handler) m_handler->OnProcessStateMessage(this,_T("文件校验错误，正在重试..."));
								DeleteFile(m_sPath+_T("\\")+md5+_T(".exe"));
								m_nTrying++;
								if (m_http.StartFileDownload(url,m_sPath+_T("\\")+md5+_T(".exe"))==0)
								{
									m_nState=LIVEUPDATESTATE_IDLE;
									if (m_handler) m_handler->OnFinishLiveUpdate(this,LIVEUPDATE_NETWORKERROR);
								}
							}
							else
							{
								if (m_handler) m_handler->OnProcessStateMessage(this,_T("文件校验错误。"));
								m_nState=LIVEUPDATESTATE_IDLE;
								if (m_handler) m_handler->OnFinishLiveUpdate(this,LIVEUPDATE_NETWORKERROR);
							}
							return;
						}
					}
				}
				//get next job
				while(m_list.GetSize()>0)
				{
					LiveUpdateInfo *tmp;
					if (m_list.GetHead(tmp,FALSE))
					{
						if (!tmp->bInstall)
						{
							m_list.RemoveHead();
						}
						else
						{
							if (m_handler)
							{
								if (m_handler->OnStartLiveUpdate(this))
								{
									m_nState=LIVEUPDATESTATE_IDLE;
									m_handler->OnFinishLiveUpdate(this,LIVEUPDATE_USERCANCEL);
									return;
								}
							}
							//start install
							m_nDownloading++;
							THString url=THSimpleXml::GetParam(tmp->sKey,_T("url"));
							THString md5=THSimpleXml::GetParam(tmp->sKey,_T("md5"));
							if (url.IsEmpty() || md5.IsEmpty())
							{
								m_nState=LIVEUPDATESTATE_IDLE;
								if (m_handler) m_handler->OnFinishLiveUpdate(this,LIVEUPDATE_USERCANCEL);
								return;
							}
							//断点续传模式
							if (m_http.StartFileDownload(url,m_sPath+_T("\\")+md5+_T(".exe"),NULL,NULL,TRUE)==0)
							{
								m_nState=LIVEUPDATESTATE_IDLE;
								if (m_handler) m_handler->OnFinishLiveUpdate(this,LIVEUPDATE_NETWORKERROR);
							}
							return;
						}
					}
				}
				m_nState=LIVEUPDATESTATE_IDLE;
				m_bSuccessed=TRUE;
				//finish all job
				if (!m_arInstall.IsEmpty())
				{
					if (m_handler)
					{
						if (m_handler->OnStartInstall(this))
							m_handler->OnFinishLiveUpdate(this,LIVEUPDATE_USERCANCEL);
					}
					for(int i=0;i<m_arInstall.GetSize();i++)
					{
						ShellExecute(NULL,_T("open"),m_arInstall[i],NULL,NULL,SW_SHOW);
						m_ini.WriteString(_T("Installed"),m_arMd5[i],_T("1"));
					}
					if (m_handler) m_handler->OnFinishLiveUpdate(this,LIVEUPDATE_FINISH);
					m_arInstall.RemoveAll();
					m_arMd5.RemoveAll();
				}
				else
				{
					if (m_handler) m_handler->OnFinishLiveUpdate(this,LIVEUPDATE_NOTNEEDUPDATE);
				}
			}
		}
	}

	static void MyTimerCallBack(int TimerId,THTimerThread *pCls,void *adddata)
	{
		THLiveUpdate *update=(THLiveUpdate *)adddata;
		update->m_SleepTime++;
		UINT nTotal;
		if (update->m_bSuccessed)
			nTotal=update->m_nReTryMin;
		else
			nTotal=update->m_nFailedReTryMin;
		if (nTotal!=0 && update->m_SleepTime>=nTotal)
			update->_StartLiveUpdate();
	}
protected:
	BOOL _StartLiveUpdate()
	{
		if (m_nState!=LIVEUPDATESTATE_IDLE) return FALSE;
		m_SleepTime=0;
		m_bSuccessed=FALSE;
		m_nState=LIVEUPDATESTATE_QUERYING;
		m_nTrying=0;
		m_nDownloading=0;
		m_arInstall.RemoveAll();
		m_arMd5.RemoveAll();
		if (m_http.StartFileDownload(m_sUrl,m_sPath+_T("\\LiveUpdate.ini"))==0)
		{
			m_nState=LIVEUPDATESTATE_IDLE;
			return FALSE;
		}
		return TRUE;
	}
	static void FreeCallBack(void *key,void *value,void *adddata)
	{
		delete (LiveUpdateInfo*)value;
	}
	typedef struct
	{
		BOOL bInstall;
		THString sKey;
	}LiveUpdateInfo;
	THList<LiveUpdateInfo *>m_list;
	ITHLiveUpdateEventHandler *m_handler;
	THINetHttp m_http;
	THString m_sPath;
	THString m_sTitle;
	THString m_sUrl;
	THIni m_ini;
	UINT m_nReTryMin;
	UINT m_nFailedReTryMin;
	UINT m_nFailReTryCount;
	DWORD m_nLastUpdateTime;
	UINT m_nTotalDown;
	UINT m_nFinished;
	int m_nState;
	volatile UINT m_nTrying;
	volatile UINT m_nDownloading;
	THStringArray m_arInstall;
	THStringArray m_arMd5;
	volatile BOOL m_bSuccessed;
	THTimerThread m_timer;
	volatile UINT m_SleepTime;
};

#define LIVEUPDATE_THREADID				1

/**
* @brief 简单自动更新类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-03-07 新建类
*/
/**<pre>
自动更新包含功能：
	1.定时更新
配置文件格式：
	[LiveUpdate]
	desc=天塑通讯录关键更新，\t针对XXX问题作出了修正\r\n		//更新内容描述，如果没有不显示对话框
	url=http://liveupdate.tansoft.cn/tanbook/tanbookcore1203.exe	//更新程序下载地址
	md5=CD24CDA213BDD224326EF										//更新程序md5，如果没有不进行MD5比较
	type=1															//更新形式，1：为关键更新，必须安装，弹出的对话框只有确定按钮，0：为选装更新，弹出的对话框有选择按钮
	version=10														//更新版本
</pre>*/
class THSimpleLiveUpdate : private THThread
{
public:
	THSimpleLiveUpdate()
	{
		m_hWnd=NULL;
		m_nMsg=0;
		m_nCurrentVersion=0;
	}
	virtual ~THSimpleLiveUpdate()
	{
		StopLiveUpdate();
	}

	/**
	* @brief 开始自动更新流程
	* @param nCurVer	当前版本
	* @param sUrl		下载地址
	* @param sParam		下载地址的参数，如：a=2&f=3，必需使用该参数传入，程序会每次访问加上动态参数
	* @param sExePath	程序所在目录
	* @param nTimerVal	检查间隔，单位分钟
	* @param bDelay		是否延迟
	* @param hWnd		主窗口句柄，如指定为弹出提示对话框时的父窗口句柄
	* @param nMsg		更新时的退出通知消息，如指定发该消息，如不指定调用exit(0)
	*/
	BOOL StartLiveUpdate(UINT nCurVer,THString sUrl,THString sParam,THString sExePath,UINT nTimerVal=24*60,BOOL bDelay=FALSE,HWND hWnd=NULL,UINT nMsg=0)
	{
		StopLiveUpdate();
		m_nCurrentVersion=nCurVer;
		m_sUrl=sUrl;
		m_sParam=sParam;
		m_sExePath=sExePath;
		m_hWnd=hWnd;
		m_nMsg=nMsg;
		DeleteFile(m_sExePath+_T("\\LiveUpdate.ini"));
		DeleteFile(m_sExePath+_T("\\LiveUpdate.exe"));
		return (StartThread(LIVEUPDATE_THREADID,NULL,nTimerVal*60000,bDelay)!=0);
	}

	void StopLiveUpdate()
	{
		KillThread(LIVEUPDATE_THREADID);
	}
protected:
	virtual void ThreadFunc(int threadid,void *adddata)
	{
		if (threadid==LIVEUPDATE_THREADID)
		{
			//liveupdate thread
			::CoInitialize(NULL);
			THString url=m_sUrl;
			if (m_sParam.IsEmpty())
				url.AppendFormat(_T("?rand=%u&version=%u"),GetTickCount(),m_nCurrentVersion);
			else
				url.AppendFormat(_T("?%s&rand=%u&version=%u"),m_sParam,GetTickCount(),m_nCurrentVersion);
			if (::URLDownloadToFile(NULL,url,m_sExePath+_T("\\LiveUpdate.ini"),0,NULL)==S_OK)
			{
				THIni ini;
				if (ini.Init(m_sExePath+_T("\\LiveUpdate.ini")))
				{
					url=ini.GetString(_T("LiveUpdate"),_T("url"));
					THString tips=ini.GetString(_T("LiveUpdate"),_T("desc"));
					THString md5=ini.GetString(_T("LiveUpdate"),_T("md5"));
					THString type=ini.GetString(_T("LiveUpdate"),_T("type"));
					tips.Replace(_T("\\r\\n"),_T("\r\n"));
					tips.Replace(_T("\\t"),_T("\t"));
					UINT version=(UINT)ini.GetInt(_T("LiveUpdate"),_T("version"));
					if (version>m_nCurrentVersion && !url.IsEmpty())
					{
						THString ExePath=m_sExePath+_T("\\LiveUpdate.exe");
						if (::URLDownloadToFile(NULL,url,ExePath,0,NULL)==S_OK)
						{
							if (!md5.IsEmpty() && THMd5::IsMd5Same(THMd5::CalcFileMd5Str(ExePath),md5))
							{
								UINT nType=MB_ICONQUESTION|MB_TOPMOST;
								if (type!=_T("1")) nType|=MB_YESNO;
								if (!tips.IsEmpty() && ::MessageBox(m_hWnd,tips,_T("发现新版本更新"),nType)==IDNO) return;
								if (tips.IsEmpty())
									ShellExecute(NULL,_T("open"),ExePath,_T("/S"),m_sExePath,SW_SHOW);
								else
									ShellExecute(NULL,_T("open"),ExePath,NULL,m_sExePath,SW_SHOW);
								if (m_hWnd && m_nMsg)
									SendMessage(m_hWnd,m_nMsg,0,0);
								else
									exit(0);
							}
							else
								DeleteFile(ExePath);
						}
					}
				}
			}
			DeleteFile(m_sExePath+_T("\\LiveUpdate.ini"));
			::CoUninitialize();
		}
	}

	THString m_sUrl;
	THString m_sParam;
	THString m_sExePath;
	UINT m_nMsg;
	UINT m_nCurrentVersion;
	HWND m_hWnd;
};
