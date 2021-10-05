#pragma once

#define THSYSTEM_INCLUDE_WININET
#include <THSystem.h>
#include <THString.h>
#include <THStringParser.h>
#include <THThread.h>
#include <THFile.h>
#include <THMemBuf.h>

#define DownloadError_Ok			0		//�������
#define DownloadError_UserCancel	1		//�û�ȡ��
#define DownloadError_FileError		2		//�ļ�����
#define DownloadError_NetWorkFail	3		//�������
#define DownloadError_UrlExplain	4		//�޷�����Url

class THINetHttp;
/**
* @brief ������Ӧ�ص���װ
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-08-02 �½���
*/
/**<pre>
�÷���
	�̳�ʵ�ָ�����ص�����
	ע��ʹ��blockģʽʱ��������Ӧ�����У����ܵ�����һ�����صĿ�ʼ���ᱻlock��ס
</pre>
*/
class THDownloadEventHandler
{
public:
	/**
	* @brief ����ʼʱ�������¼�
	* @param url		���ص�ַ
	* @param adddata	��������
	* @return ����TRUE��ʾȡ���ô����أ�FALSE��ʾ��ʼ
	*/
	virtual BOOL OnStartDownload(const THString url,class THINetHttp *cls,void *adddata){return FALSE;}
	/**
	* @brief �������ʱ�������¼�
	* @param url		���ص�ַ
	* @param adddata	��������
	* @param current	��ǰ���ֽ���
	* @param down		�������ֽ���
	* @param total		���ֽ���������ʱΪ0
	* @return ����TRUE��ʾȡ���ô����أ�FALSE��ʾ��ʼ
	*/
	virtual BOOL OnDownloading(const THString url,class THINetHttp *cls,void *adddata,unsigned int current,unsigned int down,unsigned int total){return FALSE;}
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
	virtual void OnDownloadFinish(const THString url,class THINetHttp *cls,void *adddata,int ErrCode,unsigned int down,const THString filename,const void *data,unsigned int len){}
};

/**
* @brief Post��ʽ��ָ���Ķ�����������
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-08-06 �½���
*/
/**<pre>
�÷���
	������ݺ���THINetHttp���е���
*/
class THHttpPostData
{
public:
	friend class THINetHttp;
	THHttpPostData()
	{
		m_bUsePostMethod=FALSE;
		m_bPostFile=FALSE;
		m_bDelFile=FALSE;
	}
	virtual ~THHttpPostData(){}

	/**
	* @brief ����headers
	* @param header		����Ҫ��\r\n��β�����header����ʱ��headers֮����\r\n����
	*/
	void AddHeaders(THString header=_T("Content-Type: application/x-www-form-urlencoded"))
	{
		if (header.IsEmpty()) return;
		if (!m_sAddHeaders.IsEmpty())
			m_sAddHeaders+=_T("\r\n");
		m_sAddHeaders+=header;
	}

	void SetRefer(THString refer){m_sRefer=refer;}

	void SetPostData(THString data)
	{
		m_bUsePostMethod=TRUE;
		m_bPostFile=FALSE;
		m_sPostData=data;
	}

	void SetPostFile(THString filepath,BOOL bDelAfter=FALSE)
	{
		m_bUsePostMethod=TRUE;
		m_bPostFile=TRUE;
		m_sPostData=filepath;
		m_bDelFile=bDelAfter;
	}
protected:
	BOOL m_bUsePostMethod;		///>�Ƿ�ʹ��post��ʽ
	THString m_sAddHeaders;		///>�������ӵ�headers���������ü�\r\n
	THString m_sRefer;			///>reference
	BOOL m_bPostFile;			///>�Ƿ�Ϊ�ϴ��ļ�ģʽ
	THString m_sPostData;		///>�ϴ��ļ�ģʽ����ֵΪ�ļ������ϴ�����ģʽ����ֵΪ��������
	BOOL m_bDelFile;			///>�ϴ��ļ�ģʽʱ���ϴ�����Ƿ�ɾ���ļ�
};

/**
* @brief Inet��ʽHttp��װ
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-08-02 �½���
* @2007-09-05 ��������ģʽ����
* @2008-01-28 ����FileDownload֧��httpsЭ�飬����httpsЭ��Ĭ�϶˿�
*/
/**<pre>
�÷���
	THINetHttp m_http;
	m_http.StartFileDownload(_T("http://127.0.0.1/a.rar"),_T("c:\\a.rar"));
	//����Ҫ��Ӧ��Ϣ���̳в�ʵ����THDownloadEventHandler
</pre>*/
class THINetHttp : private THThread
{
public:
	THINetHttp()
	{
		m_sAgent=_T("THINetHttp");
		m_handler=NULL;
		m_BlockDowned=0;
		m_BlockDownRet=0;
		SetPerServerConnection();
	}
	virtual ~THINetHttp(){CancelAllDownload();}

	void SetAgent(THString agent){m_sAgent=agent;}
	void SetHandler(THDownloadEventHandler *handler){m_handler=handler;}
	void CancelAllDownload(){KillAllThread();}
	static void SetPerServerConnection(int nMaxConnect=10)
	{
		//����inet��ÿһ��������վ�㣬ͬʱ�򿪵���������Ĭ��ֻ��2��4
		InternetSetOption(NULL,INTERNET_OPTION_MAX_CONNS_PER_SERVER,&nMaxConnect,sizeof(nMaxConnect));
	}

	/**
	* @brief ��ʼ�ļ�����
	* @param url		���ص�ַ
	* @param filename	�ļ�·�������û�и�������ʾ�Զ�����ʱ�ļ�Ŀ¼�½��ļ�
	* @param adddata	��������
	* @param postdata	���Ϊpostģʽ������postģʽ�����ã�getģʽ��ֵΪ��
	* @param bContinueBreak �Ƿ�ʹ�öϵ���������
	* @param bBlock		�Ƿ�ʹ������ģʽ
	* @param dwFlags	Interopen flags
	* @return ��������������,Ϊ0��ʾʧ��,����ģʽʱ�ɹ����Ƿ���-1
	*/
	int StartFileDownload(THString url,THString &filepath,void *adddata=NULL,THHttpPostData *postdata=NULL,BOOL bContinueBreak=FALSE,BOOL bBlock=FALSE,DWORD dwFlags=INTERNET_FLAG_RELOAD)
	{
		if (_tcsnicmp(url,_T("http://"),7)!=0 &&
			_tcsnicmp(url,_T("https://"),7)!=0) return 0;
		DownloadData *data=new DownloadData;
		if (!data) return 0;
		data->url=url;
		data->cls=this;
		if (filepath.IsEmpty()) filepath=THSysMisc::GenTempFileName();
		data->filepath=filepath;
		data->membuf=NULL;
		data->memsize=0;
		data->adddata=adddata;
		data->flags=dwFlags;
		data->bContinueBreak=bContinueBreak;
		data->bBlock=bBlock;
		if (postdata) data->postdata=*postdata;
		if (bBlock)
		{
			THSingleLock lock(&m_BlockMutex);
			m_BlockDowned=0;
			m_BlockDownRet=0;
			ThreadFunc(-1,data);
			if (m_BlockDownRet==DownloadError_Ok) return -1;
			return 0;
		}
		int threadid=StartThread(0,data);
		if (threadid==0)
		{
			delete data;
			return 0;
		}
		return threadid;
	}

	/**
	* @brief ��ʼ�ڴ����أ��ڴ�ģʽ��֧����������
	* @param url		���ص�ַ
	* @param maxsize	ָ�����ص�����С
	* @param adddata	��������
	* @param postdata	���Ϊpostģʽ������postģʽ�����ã�getģʽ��ֵΪ��
	* @param dwFlags	Interopen flags
	* @return ��������������,Ϊ0��ʾʧ��
	*/
	int StartMemDownload(THString url,unsigned int maxsize=4096,void *adddata=NULL,THHttpPostData *postdata=NULL,DWORD dwFlags=INTERNET_FLAG_RELOAD)
	{
		if (_tcsnicmp(url,_T("http://"),7)!=0 &&
			_tcsnicmp(url,_T("https://"),7)!=0) return 0;
		DownloadData *data=new DownloadData;
		if (!data || maxsize==0) return 0;
		data->url=url;
		data->cls=this;
		data->membuf=new char[maxsize];
		//�ڴ�ģʽ��֧�ֶϵ�����ģʽ,��֧������ģʽ
		data->bContinueBreak=FALSE;
		data->bBlock=FALSE;
		if (!data->membuf)
		{
			delete data;
			return 0;
		}
		data->memsize=maxsize;
		data->adddata=adddata;
		data->flags=dwFlags;
		if (postdata) data->postdata=*postdata;
		int threadid=StartThread(0,data);
		if (threadid==0)
		{
			delete [] data->membuf;
			delete data;
			return 0;
		}
		return threadid;
	}

	BOOL CancelDownload(int threadid){return KillThread(threadid);}

	BOOL PauseDownload(int threadid){return PauseThread(threadid);}

	BOOL ResumeDownload(int threadid){return ResumeThread(threadid);}
protected:
	typedef struct _DownloadData{
		THINetHttp *cls;
		THString url;
		THString filepath;
		char *membuf;
		unsigned int memsize;
		void *adddata;
		DWORD flags;
		THHttpPostData postdata;
		BOOL bContinueBreak;
		BOOL bBlock;
	}DownloadData;
	void HandleFinish(DownloadData *tmp,unsigned int down,int ErrCode)
	{
		if (tmp)
		{
			if (tmp->bBlock)
			{
				m_BlockDowned=down;
				m_BlockDownRet=ErrCode;
			}
			else
			{
				if (m_handler)
					m_handler->OnDownloadFinish(tmp->url,this,tmp->adddata,ErrCode,down,tmp->filepath,tmp->membuf,down);
			}
		}
	}

	virtual void ThreadFunc(int threadid,void *adddata)
	{
		DWORD nDownloadedSize=0;
		DownloadData *tmp=(DownloadData *)adddata;
		if (tmp->cls->m_handler)
		{
			if (tmp->cls->m_handler->OnStartDownload(tmp->url,this,tmp->adddata))
			{
				HandleFinish(tmp,0,DownloadError_UserCancel);
				if (tmp->membuf) delete [] tmp->membuf;
				if (tmp) delete tmp;
				return;
			}
		}

		THFile file;
		if (!tmp->filepath.IsEmpty())
		{
			if (tmp->bContinueBreak)
			{
				if (!file.Open(tmp->filepath,THFile::modeNoTruncate|THFile::modeCreate|THFile::modeWrite))
				{
					HandleFinish(tmp,0,DownloadError_FileError);
					if (tmp->membuf) delete [] tmp->membuf;
					if (tmp) delete tmp;
					return;
				}
				nDownloadedSize=(DWORD)file.SeekToEnd();
			}
			else
			{
				if (!file.Open(tmp->filepath,THFile::modeCreate|THFile::modeWrite))
				{
					HandleFinish(tmp,0,DownloadError_FileError);
					if (tmp->membuf) delete [] tmp->membuf;
					if (tmp) delete tmp;
					return;
				}
			}
		}

		HINTERNET hInetSession=InternetOpen(tmp->cls->m_sAgent,INTERNET_OPEN_TYPE_PRECONFIG,NULL,NULL,tmp->flags);
		if (!hInetSession)
		{
			HandleFinish(tmp,0,DownloadError_NetWorkFail);
			if (!tmp->filepath.IsEmpty()) file.Close();
			if (tmp->membuf) delete [] tmp->membuf;
			if (tmp) delete tmp;
			return;
		}

		HINTERNET hUrl=NULL;
		HINTERNET hConnect=NULL;
		if (tmp->postdata.m_bUsePostMethod)
		{
			THUrlExplain urlexp;
			if (!urlexp.Parse(tmp->url))
			{
				HandleFinish(tmp,0,DownloadError_UrlExplain);
				if (!tmp->filepath.IsEmpty()) file.Close();
				if (hInetSession) InternetCloseHandle(hInetSession);
				if (tmp->membuf) delete [] tmp->membuf;
				if (tmp) delete tmp;
				return;
			}

			void *buf=NULL;
			unsigned int len;
			//read file
			if (tmp->postdata.m_bPostFile)
			{
				THFile upfile;
				if (!upfile.Open(tmp->postdata.m_sPostData,THFile::modeRead))
				{
					HandleFinish(tmp,0,DownloadError_FileError);
					if (!tmp->filepath.IsEmpty()) file.Close();
					if (hInetSession) InternetCloseHandle(hInetSession);
					if (tmp->membuf) delete [] tmp->membuf;
					if (tmp) delete tmp;
					return;
				}
				len=(unsigned int)upfile.GetLength();
				buf=new char[len];
				if (!buf)
				{
					HandleFinish(tmp,0,DownloadError_FileError);
					if (!tmp->filepath.IsEmpty()) file.Close();
					if (hInetSession) InternetCloseHandle(hInetSession);
					if (tmp->membuf) delete [] tmp->membuf;
					if (tmp) delete tmp;
					return;
				}
				if (upfile.Read(buf,len)!=len)
				{
					HandleFinish(tmp,0,DownloadError_FileError);
					if (buf) delete [] buf;
					if (!tmp->filepath.IsEmpty()) file.Close();
					if (hInetSession) InternetCloseHandle(hInetSession);
					if (tmp->membuf) delete [] tmp->membuf;
					if (tmp) delete tmp;
					return;
				}
				upfile.Close();
				if (tmp->postdata.m_bDelFile)
				{
					DeleteFile(tmp->postdata.m_sPostData);
				}
			}

			//Explain Url
			THString username=urlexp.GetUser();
			THString password=urlexp.GetPass();
			if (urlexp.GetType()==_T("https"))
				hConnect=InternetConnect(hInetSession,urlexp.GetHost(),urlexp.GetPort(INTERNET_DEFAULT_HTTPS_PORT),username,password,INTERNET_SERVICE_HTTP,tmp->flags,1);
			else
				hConnect=InternetConnect(hInetSession,urlexp.GetHost(),urlexp.GetPort(INTERNET_DEFAULT_HTTP_PORT),username,password,INTERNET_SERVICE_HTTP,tmp->flags,1);
			if (!hConnect)
			{
				HandleFinish(tmp,0,DownloadError_NetWorkFail);
				if (buf) delete [] buf;
				if (!tmp->filepath.IsEmpty()) file.Close();
				if (hInetSession) InternetCloseHandle(hInetSession);
				if (tmp->membuf) delete [] tmp->membuf;
				if (tmp) delete tmp;
				return;
			}
			//fixme accept types
			//int accepttypes=tmp->postdata.m_accepttypes.GetSize();
			//if (accepttypes>0)
			hUrl=HttpOpenRequest(hConnect,_T("POST"),urlexp.GetRequests(),NULL,
						tmp->postdata.m_sRefer.IsEmpty()?NULL:tmp->postdata.m_sRefer,NULL,tmp->flags,1);
			if (!hUrl)
			{
				HandleFinish(tmp,0,DownloadError_NetWorkFail);
				if (buf) delete [] buf;
				if (!tmp->filepath.IsEmpty()) file.Close();
				if (hInetSession) InternetCloseHandle(hInetSession);
				if (tmp->membuf) delete [] tmp->membuf;
				if (tmp) delete tmp;
				return;
			}
			if (!HttpSendRequest(hUrl,tmp->postdata.m_sAddHeaders.IsEmpty()?NULL:tmp->postdata.m_sAddHeaders,
								tmp->postdata.m_sAddHeaders.GetLength(),
								buf?buf:(LPVOID)(LPCSTR)THStringA(tmp->postdata.m_sPostData),
								buf?len:(tmp->postdata.m_sPostData.GetLength())))
			{
				HandleFinish(tmp,0,DownloadError_NetWorkFail);
				if (buf) delete [] buf;
				if (!tmp->filepath.IsEmpty()) file.Close();
				if (hInetSession) InternetCloseHandle(hInetSession);
				if (tmp->membuf) delete [] tmp->membuf;
				if (tmp) delete tmp;
				return;
			}
			if (buf) delete [] buf;
		}
		else
		{
			hUrl=InternetOpenUrl(hInetSession,tmp->url,NULL,0,tmp->flags,1);
			if (!hUrl)
			{
				HandleFinish(tmp,0,DownloadError_NetWorkFail);
				if (!tmp->filepath.IsEmpty()) file.Close();
				if (hInetSession) InternetCloseHandle(hInetSession);
				if (tmp->membuf) delete [] tmp->membuf;
				if (tmp) delete tmp;
				return;
			}
		}
/*		//fixme , the raw buffer return
		char lpvSomeBuffer[4096];
		DWORD dwSize=4096;
		BOOL bRet = HttpQueryInfo(hUrl, HTTP_QUERY_RAW_HEADERS_CRLF,
                   lpvSomeBuffer, &dwSize, NULL);*/
		unsigned int total=0;
		DWORD nLen;
		DWORD dwSize = sizeof(DWORD);
		if(HttpQueryInfo(hUrl,HTTP_QUERY_FLAG_NUMBER|HTTP_QUERY_CONTENT_LENGTH,&nLen,&dwSize,NULL))
			total=nLen;

		if (nDownloadedSize)
		{
			//��˵Wininet��setfilepointerֻ����Cache��״̬����Ч�����ǿ��Ƶ�cachefile��pointer�����ǿ�����AddRequest�ķ��������еײ�Ŀ��Ʒ��� range��head��ʵ��seek.
			//�ϵ�����ģʽ����������λ��
			DWORD ret=InternetSetFilePointer(hUrl,nDownloadedSize,NULL,FILE_BEGIN,0);
			if(ret == -1)
			{
				//վ�㲻֧�ֶϵ�����
				file.SeekToBegin();
				nDownloadedSize=0;
			}
			else
			{
				if (ret!=nDownloadedSize)
				{
					//ͬ���ļ���λ��
					file.Seek(ret,SEEK_SET);
				}
				nDownloadedSize=ret;
			}
		}
		unsigned int downed=nDownloadedSize;

		BOOL bRead;
		DWORD nbytesRetr;
		char buf[4096];
		BOOL bOk=FALSE;
		while(1)
		{
			bRead=InternetReadFile(hUrl,buf,sizeof(buf),&nbytesRetr);
			if(bRead && nbytesRetr!=0)
			{
				if (!tmp->filepath.IsEmpty())
				{
					file.Write(buf,nbytesRetr);
				}
				if (tmp->membuf)
				{
					if (tmp->memsize>(unsigned int)(downed+nbytesRetr))
					{
						memcpy(((char *)tmp->membuf)+downed,buf,nbytesRetr);
					}
					else
					{
						if (tmp->memsize>(unsigned int)downed)
						{
							nbytesRetr=tmp->memsize-downed;
							memcpy(((char *)tmp->membuf)+downed,buf,nbytesRetr);
							downed+=nbytesRetr;
							if (total<downed) total=downed;
						}
						HandleFinish(tmp,downed,DownloadError_Ok);
						break;
					}
				}
				downed+=nbytesRetr;
				if (total<downed) total=downed;
				if (tmp->cls->m_handler)
					if (tmp->cls->m_handler->OnDownloading(tmp->url,this,tmp->adddata,nbytesRetr,downed,total))
					{
						HandleFinish(tmp,downed,DownloadError_UserCancel);
						break;
					}
			}
			else
			{
				bOk=TRUE;
				break;
			}
		}
		if (!tmp->filepath.IsEmpty()) file.Close();
		if (hUrl) InternetCloseHandle(hUrl);
		if (hConnect) InternetCloseHandle(hConnect);
		if (hInetSession) InternetCloseHandle(hInetSession);
		if (bOk) HandleFinish(tmp,downed,DownloadError_Ok);
		if (tmp->membuf) delete [] tmp->membuf;
		if (tmp) delete tmp;
	}

	THString m_sAgent;
	THDownloadEventHandler *m_handler;
	THMutex m_BlockMutex;
	UINT m_BlockDowned;
	int m_BlockDownRet;
};

#define THDEFAULT_AGENT		_T("Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; THINetUrl)")
#define THUrlDownloadToString		THINetUrlDownload::UrlDownloadToString
#define THUrlDownloadCallBack		THINetUrlDownload::UrlDownloadCallBack
#define THUrlDownloadToFile			THINetUrlDownload::UrlDownloadToFile
#define THUrlDownloadToCacheFile	THINetUrlDownload::UrlDownloadToCacheFile
#define THURLDownloadToFile			THINetUrlDownload::URLDownloadToFile
#define THURLDownloadToCacheFile	THINetUrlDownload::URLDownloadToCacheFile

/**
* @brief Inet��ʽ��UrlDownloadToXXϵ�к�����װ
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2009-11-03 �½���
*/
/**<pre>
�÷���
	THINetQuickHttp::
</pre>*/
class THINetUrlDownload
{
public:
	/**
	* @brief ��ʼ�ڴ����أ������ַ���
	* @param url		���ص�ַ
	* @param nFlags		InternetOpen flags����INTERNET_FLAG_RELOAD|INTERNET_FLAG_NO_CACHE_WRITE
	* @param sPostData	Post���ݣ�Ϊ��ΪGet
	* @param sAgent		Ĭ��Agent
	* @param sAddHeader	����Httpͷ
	* @return ������������
	*/
	static THStringA UrlDownloadToString(THString url,DWORD nFlags=0,THStringA sPostData="",THString sAgent=THDEFAULT_AGENT,THString sAddHeader=_T(""))
	{
		THStringA ret;
		DWORD nDownloadedSize=0;
		//ppva�ػ�������������⣬
		HINTERNET hInetSession=InternetOpen(sAgent,INTERNET_OPEN_TYPE_PRECONFIG,NULL,NULL,0);
		if (!hInetSession) return "";
		DWORD ctx=rand()+GetTickCount();
		HINTERNET hUrl;
		HINTERNET hConnect=NULL;
		LPCTSTR pHeader;
		DWORD nHeaderLen;
		if (sAddHeader.IsEmpty())
		{
			pHeader=NULL;
			nHeaderLen=0;
		}
		else
		{
			pHeader=(LPCTSTR)sAddHeader;
			nHeaderLen=sAddHeader.GetLength();
		}
		if (sPostData.IsEmpty())
		{
			hUrl=InternetOpenUrl(hInetSession,url,pHeader,nHeaderLen,nFlags,ctx);
			if (!hUrl)
			{
				if (hInetSession) InternetCloseHandle(hInetSession);
				return "";
			}
		}
		else
		{
			THUrlExplain e;
			if (!e.Parse(url)) return "";
			hConnect=InternetConnect(hInetSession,e.GetHost(),e.GetPort(INTERNET_DEFAULT_HTTP_PORT),THStrOrNull(e.GetUser()),THStrOrNull(e.GetPass()),INTERNET_SERVICE_HTTP,0,ctx);
			if (!hConnect)
			{
				if (hInetSession) InternetCloseHandle(hInetSession);
				return "";
			}
			hUrl=HttpOpenRequest(hConnect,_T("POST"),e.GetRequests(),NULL,NULL,NULL,nFlags,ctx);
			if (!hUrl)
			{
				if (hConnect) InternetCloseHandle(hConnect);
				if (hInetSession) InternetCloseHandle(hInetSession);
				return "";
			}
			if (!HttpSendRequest(hUrl,pHeader,nHeaderLen,(LPVOID)(LPCSTR)sPostData,sPostData.GetLength()))
			{
				if (hUrl) InternetCloseHandle(hUrl);
				if (hConnect) InternetCloseHandle(hConnect);
				if (hInetSession) InternetCloseHandle(hInetSession);
				return "";
			}
		}
		BOOL bRead;
		DWORD nbytesRetr;
		char buf[4096];
		BOOL bOk=FALSE;
		THStringA res;
		while(1)
		{
			bRead=InternetReadFile(hUrl,buf,sizeof(buf),&nbytesRetr);
			if (!bRead) break;
			if(nbytesRetr!=0)
			{
				res.SetString(buf,nbytesRetr);
				ret+=res;
			}
			else
			{
				bOk=TRUE;
				break;
			}
		}
		if (hUrl) InternetCloseHandle(hUrl);
		if (hConnect) InternetCloseHandle(hConnect);
		if (hInetSession) InternetCloseHandle(hInetSession);
		return ret;
	}

	/**
	* @brief ��ʼ�ڴ����أ���ͨ���ص���������
	* @param url		���ص�ַ
	* @param cb			��URLDownloadToFile���ݵ�cb��Ӧ
	* @param nFlags		InternetOpen flags
	* @param postbuf	Post���ݣ�Ϊ��ΪGet
	* @param sAgent		Ĭ��Agent
	* @param sAddHeader	����Httpͷ
	* @return �������ؽ��
	*/
	static HRESULT UrlDownloadCallBack(THString url,IBindStatusCallback *cb,DWORD nFlags=0,THMemBuf *postbuf=NULL,THString sAgent=THDEFAULT_AGENT,THString sAddHeader=_T(""))
	{
		DWORD nDownloadedSize=0;
		HINTERNET hInetSession=InternetOpen(sAgent,INTERNET_OPEN_TYPE_PRECONFIG,NULL,NULL,0);
		if (!hInetSession) return E_FAIL;
		DWORD ctx=rand()+GetTickCount();
		HINTERNET hUrl;
		HINTERNET hConnect=NULL;
		LPCTSTR pHeader;
		DWORD nHeaderLen;
		if (sAddHeader.IsEmpty())
		{
			pHeader=NULL;
			nHeaderLen=0;
		}
		else
		{
			pHeader=(LPCTSTR)sAddHeader;
			nHeaderLen=sAddHeader.GetLength();
		}
		if (postbuf==NULL)
		{
			hUrl=InternetOpenUrl(hInetSession,url,pHeader,nHeaderLen,nFlags,ctx);
			if (!hUrl)
			{
				if (hInetSession) InternetCloseHandle(hInetSession);
				return E_FAIL;
			}
		}
		else
		{
			THUrlExplain e;
			if (!e.Parse(url)) return E_FAIL;
			hConnect=InternetConnect(hInetSession,e.GetHost(),e.GetPort(INTERNET_DEFAULT_HTTP_PORT),THStrOrNull(e.GetUser()),THStrOrNull(e.GetPass()),INTERNET_SERVICE_HTTP,0,ctx);
			if (!hConnect)
			{
				if (hInetSession) InternetCloseHandle(hInetSession);
				return E_FAIL;
			}
			hUrl=HttpOpenRequest(hConnect,_T("POST"),e.GetRequests(),NULL,NULL,NULL,nFlags,ctx);
			if (!hUrl)
			{
				if (hConnect) InternetCloseHandle(hConnect);
				if (hInetSession) InternetCloseHandle(hInetSession);
				return E_FAIL;
			}
			UINT len;
			void *buf=postbuf->GetBuf(&len);
			if (!HttpSendRequest(hUrl,pHeader,nHeaderLen,buf,len))
			{
				postbuf->ReleaseGetBufLock();
				if (hUrl) InternetCloseHandle(hUrl);
				if (hConnect) InternetCloseHandle(hConnect);
				if (hInetSession) InternetCloseHandle(hInetSession);
				return E_FAIL;
			}
			postbuf->ReleaseGetBufLock();
		}
		unsigned int total=0;
		unsigned int downed=0;
		DWORD nLen;
		DWORD dwSize = sizeof(DWORD);
		if(HttpQueryInfo(hUrl,HTTP_QUERY_FLAG_NUMBER|HTTP_QUERY_CONTENT_LENGTH,&nLen,&dwSize,NULL))
			total=nLen;
		BOOL bRead;
		DWORD nbytesRetr;
		char buf[4096];
		BOOL bOk=FALSE;
		while(1)
		{
			bRead=InternetReadFile(hUrl,buf,sizeof(buf),&nbytesRetr);
			if (!bRead) break;
			if(nbytesRetr!=0)
			{
				downed+=nbytesRetr;
				if (total<downed) total=downed;
				if (cb)
				{
					cb->OnDataAvailable((DWORD)(DWORD_PTR)buf,nbytesRetr,NULL,NULL);
					if (cb->OnProgress(downed,total,0,L"")==E_ABORT)
						break;
				}
			}
			else
			{
				bOk=TRUE;
				break;
			}
		}
		if (hUrl) InternetCloseHandle(hUrl);
		if (hConnect) InternetCloseHandle(hConnect);
		if (hInetSession) InternetCloseHandle(hInetSession);
		return bOk?S_OK:E_FAIL;
	}

	/**
	* @brief ��ʼ�ļ����أ���API URLDownloadToFile��������
	* @param e1			���ݲ���������ʹ��
	* @param url		���ص�ַ
	* @param filepath	������ļ�
	* @param e2			���ݲ���������ʹ��
	* @param cb			��URLDownloadToFile���ݵ�cb��Ӧ
	* @return ������������
	*/
	static HRESULT URLDownloadToFile(void *e1,CString url,CString filepath,int e2,IBindStatusCallback *cb)
	{
		return UrlDownloadToFile(url,filepath,0,cb);
	}

	/**
	* @brief ��ʼ�ļ�����
	* @param url		���ص�ַ
	* @param filepath	������ļ�
	* @param nFlags		InternetOpen flags
	* @param cb			��URLDownloadToFile���ݵ�cb��Ӧ
	* @param postbuf	Post���ݣ�Ϊ��ΪGet
	* @param sAgent		Ĭ��Agent
	* @param sAddHeader	����Httpͷ
	* @return �������ؽ��
	*/
	static HRESULT UrlDownloadToFile(THString url,THString filepath,DWORD nFlags=0,IBindStatusCallback *cb=NULL,THMemBuf *postbuf=NULL,THString sAgent=THDEFAULT_AGENT,THString sAddHeader=_T(""))
	{
		DWORD nDownloadedSize=0;
		THFile file;
		if (!file.Open(filepath,THFile::modeCreate|THFile::modeWrite)) return E_FAIL;
		HINTERNET hInetSession=InternetOpen(sAgent,INTERNET_OPEN_TYPE_PRECONFIG,NULL,NULL,0);
		if (!hInetSession) return E_FAIL;
		DWORD ctx=rand()+GetTickCount();
		HINTERNET hUrl;
		HINTERNET hConnect=NULL;
		LPCTSTR pHeader;
		DWORD nHeaderLen;
		if (sAddHeader.IsEmpty())
		{
			pHeader=NULL;
			nHeaderLen=0;
		}
		else
		{
			pHeader=(LPCTSTR)sAddHeader;
			nHeaderLen=sAddHeader.GetLength();
		}
		if (postbuf==NULL)
		{
			hUrl=InternetOpenUrl(hInetSession,url,pHeader,nHeaderLen,nFlags,ctx);
			if (!hUrl)
			{
				if (hInetSession) InternetCloseHandle(hInetSession);
				return E_FAIL;
			}
		}
		else
		{
			THUrlExplain e;
			if (!e.Parse(url)) return E_FAIL;
			hConnect=InternetConnect(hInetSession,e.GetHost(),e.GetPort(INTERNET_DEFAULT_HTTP_PORT),THStrOrNull(e.GetUser()),THStrOrNull(e.GetPass()),INTERNET_SERVICE_HTTP,0,ctx);
			if (!hConnect)
			{
				if (hInetSession) InternetCloseHandle(hInetSession);
				return E_FAIL;
			}
			hUrl=HttpOpenRequest(hConnect,_T("POST"),e.GetRequests(),NULL,NULL,NULL,nFlags,ctx);
			if (!hUrl)
			{
				if (hConnect) InternetCloseHandle(hConnect);
				if (hInetSession) InternetCloseHandle(hInetSession);
				return E_FAIL;
			}
			UINT len;
			void *buf=postbuf->GetBuf(&len);
			if (!HttpSendRequest(hUrl,pHeader,nHeaderLen,buf,len))
			{
				postbuf->ReleaseGetBufLock();
				if (hUrl) InternetCloseHandle(hUrl);
				if (hConnect) InternetCloseHandle(hConnect);
				if (hInetSession) InternetCloseHandle(hInetSession);
				return E_FAIL;
			}
			postbuf->ReleaseGetBufLock();
		}
		unsigned int total=0;
		unsigned int downed=0;
		DWORD nLen;
		DWORD dwSize = sizeof(DWORD);
		if(HttpQueryInfo(hUrl,HTTP_QUERY_FLAG_NUMBER|HTTP_QUERY_CONTENT_LENGTH,&nLen,&dwSize,NULL))
			total=nLen;
		BOOL bRead;
		DWORD nbytesRetr;
		char buf[4096];
		BOOL bOk=FALSE;
		while(1)
		{
			bRead=InternetReadFile(hUrl,buf,sizeof(buf),&nbytesRetr);
			if (!bRead) break;
			if(nbytesRetr!=0)
			{
				try{
					file.Write(buf,nbytesRetr);
				}
				catch(...)
				{
					if (cb) cb->OnStopBinding(STG_E_WRITEFAULT,L"");
					break;
				}
				downed+=nbytesRetr;
				if (total<downed) total=downed;
				if (cb)
					if (cb->OnProgress(downed,total,0,L"")==E_ABORT)
						break;
			}
			else
			{
				bOk=TRUE;
				break;
			}
		}
		file.Close();
		if (hUrl) InternetCloseHandle(hUrl);
		if (hInetSession) InternetCloseHandle(hInetSession);
		return bOk?S_OK:E_FAIL;
	}

	/**
	* @brief ��ʼ��ʱ�ļ����أ���API URLDownloadToCacheFile��������
	* @param e1			���ݲ���������ʹ��
	* @param url		���ص�ַ
	* @param retbuf		�����ļ�·���Ļ���
	* @param buflen		���峤��
	* @param e2			���ݲ���������ʹ��
	* @param cb			��URLDownloadToFile���ݵ�cb��Ӧ
	* @return �������ؽ��
	*/
	static HRESULT URLDownloadToCacheFile(void *e1,CString url,TCHAR *retbuf,DWORD buflen,int e2,IBindStatusCallback *cb)
	{
		if (!retbuf) return E_FAIL;
		THString str=THSysMisc::GenTempFileName();
		if (buflen<(DWORD)str.GetLength()) return E_FAIL;
		_tcscpy(retbuf,str);
		return UrlDownloadToFile(url,str,0,cb);
	}

	/**
	* @brief ��ʼ��ʱ�ļ�����
	* @param url		���ص�ַ
	* @param nFlags		InternetOpen flags
	* @param cb			��URLDownloadToFile���ݵ�cb��Ӧ
	* @param postbuf	Post���ݣ�Ϊ��ΪGet
	* @param sAgent		Ĭ��Agent
	* @param sAddHeader	����Httpͷ
	* @return ���ر������ݵ��ļ���
	*/
	static THString UrlDownloadToCacheFile(THString url,DWORD nFlags=0,IBindStatusCallback *cb=NULL,THMemBuf *postbuf=NULL,THString sAgent=THDEFAULT_AGENT,THString sAddHeader=_T(""))
	{
		THString str=THSysMisc::GenTempFileName();
		HRESULT hr=UrlDownloadToFile(url,str,nFlags,cb,postbuf,sAgent,sAddHeader);
		if (SUCCEEDED(hr)) return str;
		::DeleteFile(str);
		return _T("");
	}
};

#define FTPCOMMAND_QUIT			_T("quit")
#define FTPCOMMAND_CHANGEDIR	_T("chdir")
#define FTPCOMMAND_MAKEDIR		_T("mddir")
#define FTPCOMMAND_DELETEDIR	_T("rddir")
#define FTPCOMMAND_CURRENTDIR	_T("getcurdir")
#define FTPCOMMAND_LISTDIR		_T("dir")
#define FTPCOMMAND_DOWNLOADFILE	_T("downfile")
#define FTPCOMMAND_DOWNLOADJOB	_T("downjob")
#define FTPCOMMAND_UPLOADFILE	_T("upfile")
#define FTPCOMMAND_UPLOADJOB	_T("upjob")
#define FTPCOMMAND_UPLOADJOB1	_T("upjob1")	//���ڲ�ʹ��
#define FTPCOMMAND_DELETEFILE	_T("delfile")
#define FTPCOMMAND_RENAMEFILE	_T("renfile")

class THINetFtp;
/**
* @brief ������Ӧ�ص���װ
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-01-28 �½���
*/
/**<pre>
�÷���
	�̳�ʵ�ָ�����ص�����
*/
class THFtpEventHandler
{
public:
	/**
	* @brief ��ʼ����Ftp������ʱ�������¼�
	* @param threadid	�߳�id
	* @param host		Ftp������
	* @param cls		��ָ��
	* @param adddata	��������
	* @return ����TRUE��ʾȡ���ô����أ�FALSE��ʾ��ʼ
	*/
	virtual BOOL OnStartConnect(int threadid,const THString host,class THINetFtp *cls,void *adddata){return FALSE;}
	/**
	* @brief ����Ftp�������ɹ�ʱ�������¼�
	* @param threadid	�߳�id
	* @param host		Ftp������
	* @param cls		��ָ��
	* @param adddata	��������
	* @return ����TRUE��ʾȡ���ô����أ�FALSE��ʾ��ʼ
	*/
	virtual BOOL OnConnected(int threadid,const THString host,class THINetFtp *cls,void *adddata){return FALSE;}
	/**
	* @brief �������ʱ�������¼�
	* @param threadid	�߳�id
	* @param host		Ftp������
	* @param cls		��ָ��
	* @param adddata	��������
	* @param cmd		����
	* @param ret		�����ֵ
	* @param val1		�������1
	* @param val2		�������2
	* @param cmdadddata	��������
	*/
	/*
		�������ֵ���£�
		����						����ֵ
		FTPCOMMAND_QUIT				������
		FTPCOMMAND_CHANGEDIR		0ʧ�� 1�ɹ�
		FTPCOMMAND_MAKEDIR			0ʧ�� 1�ɹ�
		FTPCOMMAND_DELETEDIR		0ʧ�� 1�ɹ�
		FTPCOMMAND_CURRENTDIR		��ǰĿ¼
		FTPCOMMAND_LISTDIR			����Ŀ¼�ṹxml
									<result>1</result><dirs><item>...</item>...</dirs><files><item>...</item>...</files>
									result:0ʧ�� 1�ɹ�
									dirs:��������Ŀ¼����
									files:���������ļ�����
									dirs��files�а������item��һ��item����һ���ļ�����Ϣ������THINetFtp::DirEmunItemToFileFindDataת���õ�WIN32_FIND_DATA
		FTPCOMMAND_DOWNLOADFILE		����
		FTPCOMMAND_UPLOADFILE		����
		FTPCOMMAND_DELETEFILE		0ʧ�� 1�ɹ�
		FTPCOMMAND_RENAMEFILE		0ʧ�� 1�ɹ�
	*/
	virtual void OnCommandFinish(int threadid,const THString host,class THINetFtp *cls,void *adddata,THString cmd,THString ret,THString val1,THString val2,int cmdadddata){}
	/**
	* @brief �������ʱ�������¼�
	* @param threadid	�߳�id
	* @param host		���ص�ַ
	* @param cls		��ָ��
	* @param adddata	��������
	* @param current	��ǰ���ֽ���
	* @param down		�������ֽ���
	* @param total		���ֽ���������ʱΪ0
	* @param cmd		����ϴ�ʱΪupfile������ʱΪdownfile
	* @param val1		�������1
	* @param val2		�������2
	* @param cmdadddata	��������
	* @return ����TRUE��ʾȡ���ô����أ�FALSE��ʾ��ʼ
	*/
	virtual BOOL OnProcessing(int threadid,const THString host,class THINetFtp *cls,void *adddata,unsigned int current,unsigned int down,unsigned int total,THString cmd,THString val1,THString val2,int cmdadddata){return FALSE;}
	/**
	* @brief �������ʱ�������¼�
	* @param threadid	�߳�id
	* @param host		Ftp��������ַ
	* @param cls		��ָ��
	* @param adddata	��������
	* @param ErrCode	������룬0Ϊû�д���
	*/
	virtual void OnFinish(int threadid,const THString host,class THINetFtp *cls,void *adddata,int ErrCode){}
};

/**
* @brief Inet��ʽFtp��װ
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-01-28 �½���
*/
/**<pre>
�÷���
</pre>*/
class THINetFtp : private THThread
{
public:
	THINetFtp()
	{
		m_sAgent=_T("THINetFtp");
		m_handler=NULL;
		SetPerServerConnection();
	}
	virtual ~THINetFtp(){CancelAllDownload();}

	void SetAgent(THString agent){m_sAgent=agent;}
	void SetHandler(THFtpEventHandler *handler){m_handler=handler;}
	void CancelAllDownload(){KillAllThread();}
	void SetPerServerConnection(int nMaxConnect=10)
	{
		//����inet��ÿһ��������վ�㣬ͬʱ�򿪵���������Ĭ��ֻ��2��4
		InternetSetOption(NULL,INTERNET_OPTION_MAX_CONNS_PER_SERVER,&nMaxConnect,sizeof(nMaxConnect));
	}

	/**
	* @brief ����Ftp������
	* @param url		��������ַ
	* @param adddata	���Ӳ���
	* @param dwFlag		Interopen flags
	* @return ��������������,Ϊ0��ʾʧ��,����ģʽʱ�ɹ����Ƿ���-1
	*/
	int ConnectToFtp(THString url,void *adddata=NULL,DWORD dwFlag=INTERNET_FLAG_PASSIVE)
	{
		THUrlExplain urlexp;
		if (!urlexp.Parse(url))
			return 0;
		if (!urlexp.GetType().IsEmpty() && urlexp.GetType()!=_T("ftp")) return 0;
		return ConnectToFtp(urlexp.GetHost(),urlexp.GetPort(INTERNET_DEFAULT_FTP_PORT),urlexp.GetUser(),urlexp.GetPass(),urlexp.GetDir(),adddata,dwFlag);
	}

	/**
	* @brief ����Ftp������
	* @param host		��������ַ
	* @param nPort		�������˿�
	* @param user		�û���
	* @param pass		����
	* @param path		Զ��·��
	* @param adddata	���Ӳ���
	* @param dwFlag		Interopen flags
	* @return ��������������,Ϊ0��ʾʧ��,����ģʽʱ�ɹ����Ƿ���-1
	*/
	int ConnectToFtp(THString host,unsigned int nPort,THString user=_T("Anonymous"),THString pass=_T("email@somewhere.com"),THString path=_T("/"),void *adddata=NULL,DWORD dwFlag=INTERNET_FLAG_PASSIVE)
	{
		FtpData *data=new FtpData;
		if (!data) return 0;
		data->cls=this;
		data->host=host;
		data->nPort=nPort;
		data->user=user;
		data->pass=pass;
		data->path=path;
		data->adddata=adddata;
		data->flags=dwFlag;
		data->hInetSession=NULL;
		data->hUrl=NULL;
		int threadid=StartThread(0,data,30000,FALSE);
		if (threadid==0)
		{
			delete data;
			return 0;
		}
		return threadid;
	}

	inline BOOL CloseConnection(int threadid){return RunCommand(threadid,FTPCOMMAND_QUIT);}
	inline BOOL ChangeDirectory(int threadid,THString dir,int adddata=0){return RunCommand(threadid,FTPCOMMAND_CHANGEDIR,dir,_T(""),adddata);}
	inline BOOL MakeDirectory(int threadid,THString dir,int adddata=0){return RunCommand(threadid,FTPCOMMAND_MAKEDIR,dir,_T(""),adddata);}
	inline BOOL DeleteDirectory(int threadid,THString dir,int adddata=0){return RunCommand(threadid,FTPCOMMAND_DELETEDIR,dir,_T(""),adddata);}
	//inline BOOL RenameDirectory(int threadid,THString dir,THString newdir,int adddata=0){return RunCommand(threadid,_T("rendir"),dir,newdir,adddata);}
	inline BOOL GetCurrentDirectory(int threadid,int adddata=0){return RunCommand(threadid,FTPCOMMAND_CURRENTDIR,_T(""),_T(""),adddata);}
	//����rfc��http://www.w3.org/Protocols/��Ҫ��REST(restart)�������RETR,��FtpCommand FtpReadFile��Ҫ��������� Type to I PASV ��һ��� Ȼ���RETR������ʹREST�����ˣ��ϵ������ܶ�ʱ����Ч
	inline BOOL ListDirectory(int threadid,int adddata=0){return RunCommand(threadid,FTPCOMMAND_LISTDIR,_T(""),_T(""),adddata);}

	/**
	* @brief ���������ļ��������в���Ӧ֪ͨ����֧�ֶϵ����������������ӦΪFTPCOMMAND_DOWNLOADFILE
	* @param threadid		�̺߳�
	* @param filename		�����ص��ļ���
	* @param savefile		�����ļ���
	* @param adddata		���Ӳ���
	* @return �Ƿ�ɹ�
	*/
	inline BOOL DownloadWholeFile(int threadid,THString filename,THString savefile,int adddata=0){return RunCommand(threadid,FTPCOMMAND_DOWNLOADFILE,filename,savefile,adddata);}

	/**
	* @brief �����ļ�����������Ӧ֪ͨ��֧�ֶϵ����������������ӦΪFTPCOMMAND_DOWNLOADJOB
	* @param threadid		�̺߳�
	* @param filename		�����ص��ļ���
	* @param savefile		�����ļ���
	* @param bContinueDown	�ļ������Ƿ�ϵ�����
	* @param adddata		���Ӳ���
	* @return �Ƿ�ɹ�
	*/
	BOOL DownloadFile(int threadid,THString filename,THString savefile,BOOL bContinueDown=TRUE,int adddata=0)
	{
		if (!bContinueDown) ::DeleteFile(savefile);
		return RunCommand(threadid,FTPCOMMAND_DOWNLOADJOB,filename,savefile,adddata);
	}

	/**
	* @brief �ϴ������ļ��������в���Ӧ֪ͨ����֧�ֶϵ����������������ӦΪFTPCOMMAND_UPLOADFILE
	* @param threadid		�̺߳�
	* @param filename		�����ļ���
	* @param upfile			�����ļ���
	* @param adddata		���Ӳ���
	* @return �Ƿ�ɹ�
	*/
	inline BOOL UploadWholeFile(int threadid,THString filename,THString upfile,int adddata=0){return RunCommand(threadid,FTPCOMMAND_UPLOADFILE,filename,upfile,adddata);}
	/**
	* @brief �ϴ��ļ�����������Ӧ֪ͨ��֧�ֶϵ����������������ӦΪFTPCOMMAND_UPLOADJOB
	* @param threadid		�̺߳�
	* @param filename		�����ļ���
	* @param upfile			�����ļ���
	* @param bContinueDown	�ļ������Ƿ�ϵ�����
	* @param adddata		���Ӳ���
	* @return �Ƿ�ɹ�
	*/
	BOOL UploadFile(int threadid,THString filename,THString upfile,BOOL bContinueDown=TRUE,int adddata=0)
	{
		if (bContinueDown)
			return RunCommand(threadid,FTPCOMMAND_UPLOADJOB1,filename,upfile,adddata);
		return RunCommand(threadid,FTPCOMMAND_UPLOADJOB,filename,upfile,adddata);
	}
	inline BOOL DeleteFile(int threadid,THString filename,int adddata=0){return RunCommand(threadid,FTPCOMMAND_DELETEFILE,filename,_T(""),adddata);}
	/**
	* @brief �����ļ�����Ŀ¼��
	* @param threadid	�̺߳�
	* @param file		�ļ���
	* @param newfile	���ļ���
	* @param adddata	���Ӳ���
	* @return �Ƿ�ɹ�
	*/
	inline BOOL RenameFile(int threadid,THString file,THString newfile,int adddata=0){return RunCommand(threadid,FTPCOMMAND_RENAMEFILE,file,newfile,adddata);}

	BOOL CancelConnection(int threadid){return KillThread(threadid);}

	BOOL PauseConnection(int threadid){return PauseThread(threadid);}

	BOOL ResumeConnection(int threadid){return ResumeThread(threadid);}

	static void DirEmunItemToFileFindData(THString item,WIN32_FIND_DATA *data)
	{
		if (data)
		{
			_tcscpy(data->cFileName,THSimpleXml::GetParam(item,_T("filename")));
			_tcscpy(data->cAlternateFileName,THSimpleXml::GetParam(item,_T("afilename")));
			data->dwFileAttributes=THs2u(THSimpleXml::GetParam(item,_T("fileattr")));
			data->ftCreationTime.dwLowDateTime=THs2u(THSimpleXml::GetParam(item,_T("filectl")));
			data->ftCreationTime.dwHighDateTime=THs2u(THSimpleXml::GetParam(item,_T("filecth")));
			data->ftLastWriteTime.dwLowDateTime=THs2u(THSimpleXml::GetParam(item,_T("filewtl")));
			data->ftLastWriteTime.dwHighDateTime=THs2u(THSimpleXml::GetParam(item,_T("filewth")));
			data->ftLastAccessTime.dwLowDateTime=THs2u(THSimpleXml::GetParam(item,_T("fileatl")));
			data->ftLastAccessTime.dwHighDateTime=THs2u(THSimpleXml::GetParam(item,_T("fileath")));
			data->nFileSizeLow=THs2u(THSimpleXml::GetParam(item,_T("filesizel")));
			data->nFileSizeHigh=THs2u(THSimpleXml::GetParam(item,_T("filesizeh")));
			data->dwReserved0=0;
			data->dwReserved1=0;
		}
	}
protected:
	typedef struct _FtpData{
		THINetFtp *cls;
		THString host;
		unsigned int nPort;
		THString user;
		THString pass;
		THString path;
		void *adddata;
		HINTERNET hInetSession;
		HINTERNET hUrl;
		DWORD flags;
	}FtpData;
	void HandleFinish(int threadid,FtpData *tmp,int ErrCode)
	{
		if (tmp)
		{
			if (m_handler)
				m_handler->OnFinish(threadid,tmp->host,this,tmp->adddata,ErrCode);
		}
	}
	void HandleCommand(int threadid,FtpData *tmp,THString cmd,THString ret,THString val1,THString val2,THString cmdadddata)
	{
		if (tmp)
		{
			if (m_handler)
				m_handler->OnCommandFinish(threadid,tmp->host,this,tmp->adddata,cmd,ret,val1,val2,THs2i(cmdadddata));
		}
	}
	void QuitThings(int threadid,FtpData *tmp)
	{
		if (tmp)
		{
			if (tmp->hUrl) InternetCloseHandle(tmp->hUrl);
			if (tmp->hInetSession) InternetCloseHandle(tmp->hInetSession);
			delete tmp;
		}
		SAVEKILLTHREADOREXIT(threadid);
	}
	DWORD MyFtpSetFilePointer(int threadid,HINTERNET hFtp,DWORD off)
	{
		DWORD ret=(DWORD)-1;
		if (hFtp)
		{
			HINTERNET hResponse;
			THString str;
			str.Format(_T("REST %lu"),off);
			BOOL bRet=::FtpCommand(hFtp,TRUE,FTP_TRANSFER_TYPE_BINARY,str,0,&hResponse);
			//if (bRet)
			{
				DWORD dwError;
				TCHAR tpszBuff[2*1024];
				DWORD dwBuffLen = 2*1024;
				bRet=::InternetGetLastResponseInfo(&dwError,tpszBuff,&dwBuffLen);
				if (bRet && _tcsstr(tpszBuff,_T("350"))) ret=off;
			}
		}
		return ret;
	}

	BOOL RunCommand(int threadid,THString cmd,THString val1=_T(""),THString val2=_T(""),int adddata=0)
	{
		if (!IsThreadRunning(threadid)) return FALSE;
		if (cmd.IsEmpty()) return FALSE;
		THString key;
		key.Format(_T("key%d"),threadid);
		THString value;
		value+=THSimpleXml::MakeParam(_T("cmd"),cmd);
		value+=THSimpleXml::MakeParam(_T("val1"),val1);
		value+=THSimpleXml::MakeParam(_T("val2"),val2);
		value+=THSimpleXml::MakeParam(_T("adddata"),THi2s(adddata));
		m_CmdMutex.Lock();
		m_CmdXml+=THSimpleXml::MakeParam(key,value);
		m_CmdMutex.Unlock();
		return SetThreadEvent(threadid);
	}
	virtual void ThreadFunc(int threadid,void *adddata)
	{
		FtpData *tmp=(FtpData *)adddata;
		THINetFtp *pFtp=tmp->cls;
		if (!tmp->hInetSession)
		{
			if (m_handler)
			{
				if (m_handler->OnStartConnect(threadid,tmp->host,this,tmp->adddata))
				{
					HandleFinish(threadid,tmp,DownloadError_UserCancel);
					QuitThings(threadid,tmp);
					return;
				}
			}
			tmp->hInetSession=InternetOpen(tmp->cls->m_sAgent,INTERNET_OPEN_TYPE_PRECONFIG,NULL,NULL,tmp->flags);
			if (!tmp->hInetSession)
			{
				HandleFinish(threadid,tmp,DownloadError_NetWorkFail);
				QuitThings(threadid,tmp);
				return;
			}
		}

		if (!tmp->hUrl)
		{
			tmp->hUrl=InternetConnect(tmp->hInetSession,tmp->host,tmp->nPort,tmp->user,tmp->pass,INTERNET_SERVICE_FTP,tmp->flags,threadid);
			if (!tmp->hUrl)
			{
				HandleFinish(threadid,tmp,DownloadError_NetWorkFail);
				QuitThings(threadid,tmp);
				return;
			}
			if (m_handler)
			{
				if (m_handler->OnConnected(threadid,tmp->host,this,tmp->adddata))
				{
					HandleFinish(threadid,tmp,DownloadError_UserCancel);
					QuitThings(threadid,tmp);
					return;
				}
			}
		}

		THString prefixkey;
		prefixkey.Format(_T("key%d"),threadid);
		THString cmd;
		THString val1,val2;
		THString cmdadddata;
		while(1)
		{
			m_CmdMutex.Lock();
			THString key=THSimpleXml::GetAndRemoveParam(&tmp->cls->m_CmdXml,prefixkey);
			m_CmdMutex.Unlock();
			//wait for next cmd
			if (key.IsEmpty()) return;
			cmd=THSimpleXml::GetParam(key,_T("cmd"));
			val1=THSimpleXml::GetParam(key,_T("val1"));
			val2=THSimpleXml::GetParam(key,_T("val2"));
			cmdadddata=THSimpleXml::GetParam(key,_T("adddata"));
			if (cmd==FTPCOMMAND_QUIT) break;
			else if (cmd==FTPCOMMAND_CHANGEDIR)
			{
				BOOL ret=::FtpSetCurrentDirectory(tmp->hUrl,val1);
				HandleCommand(threadid,tmp,cmd,THi2s(ret),val1,val2,cmdadddata);
			}
			else if (cmd==FTPCOMMAND_DELETEDIR)
			{
				BOOL ret=::FtpRemoveDirectory(tmp->hUrl,val1);
				HandleCommand(threadid,tmp,cmd,THi2s(ret),val1,val2,cmdadddata);
			}
			else if (cmd==FTPCOMMAND_MAKEDIR)
			{
				BOOL ret=::FtpCreateDirectory(tmp->hUrl,val1);
				HandleCommand(threadid,tmp,cmd,THi2s(ret),val1,val2,cmdadddata);
			}
			else if (cmd==FTPCOMMAND_CURRENTDIR)
			{
				THString str;
				DWORD dwSize=MAX_PATH;
				BOOL ret=::FtpGetCurrentDirectory(tmp->hUrl,str.GetBuffer(MAX_PATH),&dwSize);
				str.ReleaseBuffer();
				if (!ret) str.Empty();
				HandleCommand(threadid,tmp,cmd,str,val1,val2,cmdadddata);
			}
			else if (cmd==FTPCOMMAND_LISTDIR)
			{
				WIN32_FIND_DATA dirInfo;
				THString ret;
				DWORD dwError;
				HINTERNET hFind=FtpFindFirstFile(tmp->hUrl,_T("*.*"),&dirInfo,INTERNET_FLAG_RELOAD,threadid);
				if (!hFind)
				{
					dwError=GetLastError();
					if(dwError==ERROR_NO_MORE_FILES)
						ret=THSimpleXml::MakeParam(_T("result"),_T("1"));
					else
						ret=THSimpleXml::MakeParam(_T("result"),_T("0"));
					HandleCommand(threadid,tmp,cmd,ret,val1,val2,cmdadddata);
					continue;
				}
				THString dirs;
				THString files;
				do
				{
					if (_tcscmp(dirInfo.cFileName,_T("."))!=0 && _tcscmp(dirInfo.cFileName,_T(".."))!=0)
					{
						THString val;
						val+=THSimpleXml::MakeParam(_T("filename"),dirInfo.cFileName);
						val+=THSimpleXml::MakeParam(_T("afilename"),dirInfo.cAlternateFileName);
						val+=THSimpleXml::MakeParam(_T("fileattr"),THu2s(dirInfo.dwFileAttributes));
						val+=THSimpleXml::MakeParam(_T("filectl"),THu2s(dirInfo.ftCreationTime.dwLowDateTime));
						val+=THSimpleXml::MakeParam(_T("filecth"),THu2s(dirInfo.ftCreationTime.dwHighDateTime));
						val+=THSimpleXml::MakeParam(_T("filewtl"),THu2s(dirInfo.ftLastWriteTime.dwLowDateTime));
						val+=THSimpleXml::MakeParam(_T("filewth"),THu2s(dirInfo.ftLastWriteTime.dwHighDateTime));
						val+=THSimpleXml::MakeParam(_T("fileatl"),THu2s(dirInfo.ftLastAccessTime.dwLowDateTime));
						val+=THSimpleXml::MakeParam(_T("fileath"),THu2s(dirInfo.ftLastAccessTime.dwHighDateTime));
						val+=THSimpleXml::MakeParam(_T("filesizel"),THu2s(dirInfo.nFileSizeLow));
						val+=THSimpleXml::MakeParam(_T("filesizeh"),THu2s(dirInfo.nFileSizeHigh));
						if (dirInfo.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
							dirs+=THSimpleXml::MakeParam(_T("item"),val);
						else
							files+=THSimpleXml::MakeParam(_T("item"),val);
					}
				}while(InternetFindNextFile(hFind,(LPVOID)&dirInfo));
				if((dwError=GetLastError())==ERROR_NO_MORE_FILES)
					ret=THSimpleXml::MakeParam(_T("result"),_T("1"));
				else
					ret=THSimpleXml::MakeParam(_T("result"),_T("0"));
				ret+=THSimpleXml::MakeParam(_T("dirs"),dirs);
				ret+=THSimpleXml::MakeParam(_T("files"),files);
				InternetCloseHandle(hFind);
				HandleCommand(threadid,tmp,cmd,ret,val1,val2,cmdadddata);
			}
			else if (cmd==FTPCOMMAND_DOWNLOADFILE)
			{
				//whole file download do not support Pause & Resume
				BOOL ret=::FtpGetFile(tmp->hUrl,val1,val2,FALSE,FILE_ATTRIBUTE_NORMAL,0,threadid);
				HandleCommand(threadid,tmp,cmd,THi2s(ret),val1,val2,cmdadddata);
			}
			else if (cmd==FTPCOMMAND_DOWNLOADJOB)
			{
				WIN32_FIND_DATA dirInfo;
				DWORD total=0;
				DWORD downed=0;
				HINTERNET hFind=FtpFindFirstFile(tmp->hUrl,val1,&dirInfo,INTERNET_FLAG_RELOAD,threadid);
				if (hFind)
				{
					InternetCloseHandle(hFind);
					total=dirInfo.nFileSizeLow;
				}
				THString ret=_T("0");
				DWORD ss=0;
				THFile file;
				if (file.Open(val2,THFile::modeRead))
				{
					ss=(DWORD)file.GetLength();
					file.Close();
				}
				BOOL bCancel=FALSE;
				if (file.Open(val2,THFile::modeWrite|THFile::modeNoTruncate))
				{
					BOOL bSeek=TRUE;
					if (ss!=0)
					{
						bSeek=FALSE;
						//Pause & Resume mode
						DWORD seekpos=MyFtpSetFilePointer(threadid,tmp->hUrl,ss);
						//DWORD seekpos=InternetSetFilePointer(hFile,ss,0,FILE_BEGIN,threadid);
						//the server not support Pause & Resume mode
						if (seekpos==(DWORD)-1)
						{
							bSeek=TRUE;
							ss=0;
						}
						else
						{
							if (seekpos==ss)
								if (file.Seek(0,SEEK_END)==ss)
									bSeek=TRUE;
						}
					}
					if (bSeek)
					{
						HINTERNET hFile=::FtpOpenFile(tmp->hUrl,val1,GENERIC_READ,0,threadid);
						if (hFile)
						{
							char buf[4096];
							DWORD readed=0;
							downed=ss;
							while(1)
							{
								if (!InternetReadFile(hFile,buf,4096,&readed)) break;
								if (readed==0)
								{
									ret=_T("1");
									break;
								}
								downed+=readed;
								if (m_handler)
									if (m_handler->OnProcessing(threadid,tmp->host,this,tmp->adddata,readed,downed,total,cmd,val1,val2,THs2i(cmdadddata)))
									{
										bCancel=TRUE;
										break;
									}
								file.Write(buf,readed);
							}
							InternetCloseHandle(hFile);
						}
					}
					file.Close();
				}
				HandleCommand(threadid,tmp,cmd,ret,val1,val2,cmdadddata);
				if (bCancel)
				{
					HandleFinish(threadid,tmp,DownloadError_UserCancel);
					QuitThings(threadid,tmp);
				}
			}
			else if (cmd==FTPCOMMAND_UPLOADFILE)
			{
				//whole file download do not support Pause & Resume
				BOOL ret=::FtpPutFile(tmp->hUrl,val1,val2,0,threadid);
				HandleCommand(threadid,tmp,cmd,THi2s(ret),val1,val2,cmdadddata);
			}
			else if (cmd==FTPCOMMAND_UPLOADJOB || cmd==FTPCOMMAND_UPLOADJOB1)
			{
				THString ret=_T("0");
				DWORD ss=0;
				DWORD total=0;
				DWORD downed=0;
				//do not Pause & Resume,make sure delete the file
				if (cmd==FTPCOMMAND_UPLOADJOB)
					::FtpDeleteFile(tmp->hUrl,val2);
				else
				{
					cmd=FTPCOMMAND_UPLOADJOB;
					//use Pause & Resume mode,get file size first
					WIN32_FIND_DATA dirInfo;
					HINTERNET hFind=FtpFindFirstFile(tmp->hUrl,val2,&dirInfo,INTERNET_FLAG_RELOAD,threadid);
					if (hFind)
					{
						InternetCloseHandle(hFind);
						if (dirInfo.nFileSizeHigh!=0)
						{
							//fixme do not support too big file now,change to note Pause & Resume mode
							//HandleCommand(threadid,tmp,cmd,ret,val1,val2,cmdadddata);
							//return;
						}
						else
							ss=dirInfo.nFileSizeLow;
					}
				}
				THFile file;
				BOOL bCancel=FALSE;
				if (file.Open(val1,THFile::modeRead))
				{
					total=(DWORD)file.GetLength();
					HINTERNET hFile=::FtpOpenFile(tmp->hUrl,val2,GENERIC_WRITE,0,threadid);
					if (hFile)
					{
						BOOL bSeek=TRUE;
						if (ss!=0)
						{
							bSeek=FALSE;
							//Pause & Resume mode
							DWORD seekpos=MyFtpSetFilePointer(threadid,tmp->hUrl,ss);
							//DWORD seekpos=InternetSetFilePointer(hFile,ss,0,FILE_BEGIN,threadid);
							//the server not support Pause & Resume mode
							if (seekpos==(DWORD)-1)
							{
								bSeek=TRUE;
								ss=0;
							}
							else
							{
								if (seekpos==ss)
									if (file.Seek(ss,SEEK_SET)==ss)
										bSeek=TRUE;
							}
						}
						if (bSeek)
						{
							char buf[4096];
							DWORD readed=0;
							downed=ss;
							while(1)
							{
								UINT read=file.Read(buf,4096);
								if (read==0)
								{
									ret=_T("1");
									break;
								}
								if (!InternetWriteFile(hFile,buf,read,&readed)) break;
								if (read!=readed) break;
								downed+=readed;
								if (m_handler)
									if (m_handler->OnProcessing(threadid,tmp->host,this,tmp->adddata,readed,downed,total,cmd,val1,val2,THs2i(cmdadddata)))
									{
										bCancel=TRUE;
										break;
									}
							}
						}
						InternetCloseHandle(hFile);
					}
					file.Close();
				}
				HandleCommand(threadid,tmp,cmd,ret,val1,val2,cmdadddata);
				if (bCancel)
				{
					HandleFinish(threadid,tmp,DownloadError_UserCancel);
					QuitThings(threadid,tmp);
				}
			}
			else if (cmd==FTPCOMMAND_DELETEFILE)
			{
				BOOL ret=::FtpDeleteFile(tmp->hUrl,val1);
				HandleCommand(threadid,tmp,cmd,THi2s(ret),val1,val2,cmdadddata);
			}
			else if (cmd==FTPCOMMAND_RENAMEFILE)
			{
				BOOL ret=::FtpRenameFile(tmp->hUrl,val1,val2);
				HandleCommand(threadid,tmp,cmd,THi2s(ret),val1,val2,cmdadddata);
			}
		}
		HandleFinish(threadid,tmp,DownloadError_Ok);
		QuitThings(threadid,tmp);
	}

	THString m_sAgent;
	THFtpEventHandler *m_handler;
	THMutex m_CmdMutex;
	THString m_CmdXml;
};