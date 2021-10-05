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
* @brief �Զ�������Ӧ��װ��
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-08-20 �½���
*/
class ITHLiveUpdateEventHandler
{
public:
	/**
	* @brief ��ʼ�����Զ�����
	* @param pUpdate			THLiveUpdate��ʵ��ָ��
	* @return �Ƿ�ȡ��
	*/
	virtual BOOL OnStartLiveUpdate(THLiveUpdate *pUpdate)
	{
		return FALSE;
	}
	
	/**
	* @brief ����Զ����£����۳ɹ�ʧ�ܶ��������øýӿ�
	* @param pUpdate			THLiveUpdate��ʵ��ָ��
	* @param nReason			���ؽ���룬LIVEUPDATE_XXX
	*/
	virtual void OnFinishLiveUpdate(THLiveUpdate *pUpdate,int nReason)
	{
	}
	/**
	* @brief ����״̬��Ϣ
	* @param pUpdate			THLiveUpdate��ʵ��ָ��
	* @param sMsg				����״̬��Ϣ
	*/
	virtual void OnProcessStateMessage(THLiveUpdate *pUpdate,THString sMsg)
	{
	}
	/**
	* @brief ���½�����Ϣ
	* @param pUpdate			THLiveUpdate��ʵ��ָ��
	* @param nPosAll			�ܽ��Ȱٷֱȣ�100Ϊ���
	* @param nPosCurrent		��ǰ����ٷֱȣ�100Ϊ���
	* @param nTotal				����Ҫ���ش�С
	* @param nFinished			����ɴ�С
	* @param nSpeedPreSecond	��ǰ�����ٶ�
	*/
	virtual void OnProcessPercent(THLiveUpdate *pUpdate,UINT nPosAll,UINT nPosCurrent,UINT nTotal,UINT nFinished,UINT nSpeedPreSecond)
	{
	}
	/**
	* @brief �ҵ����ø���
	* @param pUpdate			THLiveUpdate��ʵ��ָ��
	* @param nMax				���ø����ܸ�������ʱ�ɵ���THLiveUpate�е�GetUpdateInfo�����Ϣ�Ķ�ȡ
	* @return �Ƿ�ȡ��
	*/
	virtual BOOL OnFoundedUpdate(THLiveUpdate *update,int nMax)
	{
		return FALSE;
	}
	/**
	* @brief ��ʼ���а�װ
	* @param pUpdate			THLiveUpdate��ʵ��ָ��
	* @return �Ƿ�ȡ��
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
* @brief �Զ����µײ���
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-08-20 �½���
*/
/**<pre>
�Զ����°������ܣ�
	1.��ʱ����
	2.�ϵ�����
	3.ʧ���Զ�����
�����ļ���ʽ��
	[����ͨѶ¼�ؼ�����1203]										//����ģ�������
	desc=����ͨѶ¼�ؼ����£����XXX��������������					//������������
	size=2412523													//���³����С
	url=http://liveupdate.tansoft.cn/tanbook/tanbookcore1203.exe	//���³������ص�ַ
	md5=CD24CDA213BDD224326EF										//���³���md5
	type=1															//������ʽ��1��Ϊ�ؼ����£����밲װ��0��Ϊѡװ���£�Ĭ�ϲ�ѡ�У�
�Զ��������̣�
	1.��ʼliveupdate��ֱ�ӿ�ʼ��Timeout��ʼ��
	2.�ص�OnStartLiveUpdate��ʼ�Զ�����
	3.����sUrl�������ļ�
	4.
�÷���
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
	* @brief ��ʼ���Զ�����
	* @param sPath				�Զ�����ʹ�õ�Ŀ¼
	* @param sTitle				�Զ�����ģ�����ƣ��磺����ͨѶ¼
	* @param sUrl				�Զ����������ļ�url
	* @param nReTryMin			�Զ����¶�ʱ���¼������λΪ���ӣ�0Ϊ�����ж�ʱ����
	* @param nFailedReTryMin	�Զ�����ʧ�ܶ�ʱ���¼������λΪ���ӣ�0Ϊ�����ж�ʱ����
	* @param nFailReTryCount	���¹���ʧ�ܺ����Դ���
	* @return �Ƿ�ɹ�
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
	* @brief ��ȡÿ����װ����Ϣ
	* @param idx				��װ����
	* @param key				ȡֵ�ļ���
	* @return ��ֵ
	*/
	THString GetUpdateInfo(int idx,const THString key)
	{
		LiveUpdateInfo *info;
		if (m_list.GetAt(idx,info,FALSE))
			return THSimpleXml::GetParam(info->sKey,key);
		return _T("");
	}

	/**
	* @brief �����Ƿ�װ����
	* @param idx				��װ����
	* @param bInstall			�Ƿ�װ
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
				m_handler->OnProcessStateMessage(this,_T("���ڲ�ѯ���õĸ����б�..."));
			}
			else if (m_nState==LIVEUPDATESTATE_DOWNING)
			{
				THString msg;
				msg.Format(_T("�������ص� %d ������..."),m_nDownloading);
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
	* @brief �������ʱ�������¼�
	* @param url		���ص�ַ
	* @param adddata	��������
	* @param ErrCode	������룬0Ϊû�д���
	* @param down		���������ݳ���
	* @param filename	�ļ�����ģʽʱ����ֵΪ�ļ���
	* @param data		�ڴ�����ģʽʱ����ֵΪ�������ݣ��ص�������Ϻ󣬽����ͷŸ��ڴ�
	* @param len		�ڴ�����ģʽʱ����ֵΪ�������ݳ���
	*/
	virtual void OnDownloadFinish(const THString url,class THINetHttp *cls,void *adddata,int ErrCode,unsigned int down,const THString filename,const void *data,unsigned int len)
	{
		BOOL bPassState=FALSE;
		if (m_nState==LIVEUPDATESTATE_QUERYING)
		{
			if (ErrCode!=DownloadError_Ok)
			{
				//�������ش���
				if (ErrCode==DownloadError_FileError)
				{
					m_nState=LIVEUPDATESTATE_IDLE;
					if (m_handler) m_handler->OnFinishLiveUpdate(this,LIVEUPDATE_FILEERROR);
				}
				else
				{
					if (m_nTrying<m_nFailReTryCount)
					{
						if (m_handler) m_handler->OnProcessStateMessage(this,_T("�޷��������磬��������..."));
						m_nTrying++;
						if (m_http.StartFileDownload(m_sUrl,m_sPath+_T("\\LiveUpdate.ini"))==0)
						{
							m_nState=LIVEUPDATESTATE_IDLE;
							if (m_handler) m_handler->OnFinishLiveUpdate(this,LIVEUPDATE_NETWORKERROR);
						}
					}
					else
					{
						if (m_handler) m_handler->OnProcessStateMessage(this,_T("�޷��������硣"));
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
					//���ص��ļ��д�
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
		if (m_nState==LIVEUPDATESTATE_DOWNING)//���ﲻ����else if������״̬�ı��ֱ�ӵ���
		{
			if (ErrCode!=DownloadError_Ok)
			{
				//�������ش���
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
							if (m_handler) m_handler->OnProcessStateMessage(this,_T("�޷��������磬��������..."));
							if (m_http.StartFileDownload(url,m_sPath+_T("\\")+md5+_T(".exe"))==0)
							{
								m_nState=LIVEUPDATESTATE_IDLE;
								if (m_handler) m_handler->OnFinishLiveUpdate(this,LIVEUPDATE_NETWORKERROR);
							}
						}
						else
						{
							if (m_handler) m_handler->OnProcessStateMessage(this,_T("�޷��������硣"));
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
						if (m_handler) m_handler->OnProcessStateMessage(this,_T("�ļ�������ɡ����ڼ��������..."));
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
								if (m_handler) m_handler->OnProcessStateMessage(this,_T("�ļ�У�������������..."));
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
								if (m_handler) m_handler->OnProcessStateMessage(this,_T("�ļ�У�����"));
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
							//�ϵ�����ģʽ
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
* @brief ���Զ�������
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-03-07 �½���
*/
/**<pre>
�Զ����°������ܣ�
	1.��ʱ����
�����ļ���ʽ��
	[LiveUpdate]
	desc=����ͨѶ¼�ؼ����£�\t���XXX��������������\r\n		//�����������������û�в���ʾ�Ի���
	url=http://liveupdate.tansoft.cn/tanbook/tanbookcore1203.exe	//���³������ص�ַ
	md5=CD24CDA213BDD224326EF										//���³���md5�����û�в�����MD5�Ƚ�
	type=1															//������ʽ��1��Ϊ�ؼ����£����밲װ�������ĶԻ���ֻ��ȷ����ť��0��Ϊѡװ���£������ĶԻ�����ѡ��ť
	version=10														//���°汾
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
	* @brief ��ʼ�Զ���������
	* @param nCurVer	��ǰ�汾
	* @param sUrl		���ص�ַ
	* @param sParam		���ص�ַ�Ĳ������磺a=2&f=3������ʹ�øò������룬�����ÿ�η��ʼ��϶�̬����
	* @param sExePath	��������Ŀ¼
	* @param nTimerVal	���������λ����
	* @param bDelay		�Ƿ��ӳ�
	* @param hWnd		�����ھ������ָ��Ϊ������ʾ�Ի���ʱ�ĸ����ھ��
	* @param nMsg		����ʱ���˳�֪ͨ��Ϣ����ָ��������Ϣ���粻ָ������exit(0)
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
								if (!tips.IsEmpty() && ::MessageBox(m_hWnd,tips,_T("�����°汾����"),nType)==IDNO) return;
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
