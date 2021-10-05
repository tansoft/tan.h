#pragma once

#include <THSerialPort.h>
#include <THSyn.h>
#include <THStruct.h>

class ITHModemEventHandler
{
public:
	//对向Modem发出的请求，进行回调处理，仅用在异步模式
	virtual void OnResponseRequest(THString str,int status)=0;
	//对Modem发出的通知进行处理，同步，异步模式都可处理
	virtual void OnComingMessage(THString str,int status)=0;
	//串口错误事件处理，同步，异步模式都可处理
	virtual void OnHandleError(int nErrorType,int nPortNum)=0;
};

#define MODEMSTATUS_NONE	0
#define MODEMSTATUS_OK		1
#define MODEMSTATUS_ERROR	2
#define MODEMSTATUS_TIMEOUT	3

/**
* @brief Modem封装类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-08-12 新建类
*/
/**<pre>
用法：
	//同步模式
	THModem m;
	if (m.OpenModem(1))
	{
		if (m.IsReady())
		{
			THDebug(_T("Modem Ready!"));
			m.SetEcho();
		}
		else
			THDebug(_T("Not Have Modem Here!"));
		m.CloseModem();
	}
	//异步模式
	class MyCls: public ITHModemEventHandler
	{
	public:
		//对向Modem发出的请求，进行回调处理，仅用在异步模式
		virtual void OnResponseRequest(THString str,int status)
		{
			if (status==MODEMSTATUS_OK)
				...
		}
		//对Modem发出的通知进行处理，同步，异步模式都可处理
		virtual void OnComingMessage(THString str,int status);
		//串口错误事件处理，同步，异步模式都可处理
		virtual void OnHandleError(int nErrorType,int nPortNum);
	};
	THModem m;
	if (m.OpenModem(1,115200,0))
	{
		m.SetHandler(MyCls);
		m.IsReady();
		m.SetEcho();
		m.CloseModem();
	}
</pre>*/
class THModem : public ITHSerialPortBufferedEventHandler
{
public:
	THModem()
	{
		m_handler=NULL;
		m_nTimeout=1500;
		m_nLastState=MODEMSTATUS_NONE;
		m_timer.SetCallFunc(OnTimerCallBack);
		m_port.SetHandler(this);
		m_evResp.Create(NULL,TRUE,FALSE,NULL);
		m_ResultKey.Add(_T("OK\r\n"));
		m_ResultKey.Add(_T("ERROR\r\n"));
		m_ResultKey.Add(_T("OK\r"));
		m_ResultKey.Add(_T("ERROR\r"));
		m_ResultKey.Add(_T(">"));
	}
	virtual ~THModem()
	{
		CloseModem();
	}

	/**
	* @brief 打开Modem
	* @param portno			串口号
	* @param baudrate		波特率，最大为 115200
	* @param nTimeout		超时时间，用于同步模式，为0表示使用异步模式
	* @param paritytype		奇偶校验
	* @param databits		数据位
	* @param stopbits		停止位
	* @return 是否成功
	*/
	BOOL OpenModem(UINT nPortNo=1,UINT bandrate=115200,UINT nTimeout=1500,char paritytype=SPPARITYTYPE_NONE,UINT databits=8,UINT stopbits=1)
	{
		CloseModem();
		m_nTimeout=nTimeout;
		//串口类强制使用异步模式，Modem类的同步由事件实现
		if (!m_port.InitPort(nPortNo,bandrate,4096,paritytype,databits,stopbits)) return FALSE;
		m_evResp.Reset();
		return TRUE;
	}

	/**
	* @brief 关闭Modem
	*/
	void CloseModem(){m_port.ClosePort();m_evResp.Set();}

	/**
	* @brief 判断是否打开了Modem设备
	* @return 是否存在Modem设备
	*/
	BOOL IsOpened(){return m_port.IsOpened();}

	/**
	* @brief 判断是否存在Modem设备
	* @return 是否存在Modem设备
	*/
	BOOL IsReady()
	{
		if (!WriteToPort(_T("AT\r\n"),TRUE,TRUE)) return FALSE;
		if (WaitForResponse(NULL)==MODEMSTATUS_OK) return TRUE;
		return FALSE;
	}

	/**
	* @brief 软重启modem
	* @return 是否成功
	*/
	BOOL Reset()
	{
		if (!WriteToPort(_T("ATZ\r\n"),TRUE,TRUE)) return FALSE;
		Sleep(3000);
		if (WaitForResponse(NULL)==MODEMSTATUS_OK) return TRUE;
		return FALSE;
	}

	/**
	* @brief 设置回显开启/关闭
	* @return 是否成功
	*/
	BOOL SetEcho(BOOL bClose=TRUE)
	{
		THString cmd;
		if (bClose)
			cmd=_T("ATE0\r\n");
		else
			cmd=_T("ATE1\r\n");
		if (!WriteToPort(cmd,TRUE,TRUE)) return FALSE;
		if (WaitForResponse(NULL)==MODEMSTATUS_OK) return TRUE;
		return FALSE;
	}

	THString GetModemFactory()
	{
		THString cmd;
		THString ret;
		cmd=_T("AT+CGMI\r\n");
		if (WriteToPort(cmd,TRUE,TRUE))
			WaitForResponse(&ret);
		return ret;
	}

	THString GetModemModel()
	{
		THString cmd;
		THString ret;
		cmd=_T("AT+CGMM\r\n");
		if (WriteToPort(cmd,TRUE,TRUE))
			WaitForResponse(&ret);
		return ret;
	}

	THString GetModemFactorySerial()
	{
		THString cmd;
		THString ret;
		cmd=_T("AT+GSN\r\n");
		if (WriteToPort(cmd,TRUE,TRUE))
			WaitForResponse(&ret);
		return ret;
	}

	THString GetModemFirmwareVersion()
	{
		THString cmd;
		THString ret;
		cmd=_T("AT+CGMR\r\n");
		if (WriteToPort(cmd,TRUE,TRUE))
			WaitForResponse(&ret);
		return ret;
	}

	void SetHandler(ITHModemEventHandler *handler){m_handler=handler;}

	/**
	* @brief 设置是否自动省电模式
	* @param bAutoShutDown		是否启用自动省电模式，默认为不启动，不启动时需要程序每隔nTimerVal时间对modem进行激活
	* @param nTimerVal			激活modem的定时时间
	* @return 是否成功
	*/
	BOOL SetAutoPowerShutDown(BOOL bAutoShutDown=FALSE,int nTimerVal=60000)
	{
		if (bAutoShutDown)
		{
			if (!m_timer.IsThreadRunning(1)) return TRUE;
			return m_timer.StopTimer(1);
		}
		else
		{
			if (m_timer.IsThreadRunning(1))
			{
				m_timer.StopTimer(1);
				Sleep(500);
			}
			return m_timer.StartTimer(1,nTimerVal,TRUE,this);
		}
		return TRUE;
	}

	static void OnTimerCallBack(int TimerId,THTimerThread *pCls,void *data)
	{
		//防止modem休眠
		((THModem *)data)->IsReady();
	}
protected:
	BOOL WriteToPort(LPCTSTR string,BOOL bAscii=TRUE,BOOL bWaitSend=FALSE)
	{
		if (m_timer.IsThreadRunning(1)) m_timer.ResetTimer(1);
		return m_port.WriteToPort(string,bAscii,bWaitSend);
	}

	BOOL WriteToPortBin(const void *buf,unsigned int n,BOOL bWaitSend=FALSE)
	{
		if (m_timer.IsThreadRunning(1)) m_timer.ResetTimer(1);
		return m_port.WriteToPortBin(buf,n,bWaitSend);
	}

	virtual void OnHandleError(int nErrorType,int nPortNum)
	{
		if (m_handler) m_handler->OnHandleError(nErrorType,nPortNum);
	}

	virtual void OnHandleFlagEvent(DWORD dwCommEvent,int nPortNum){/*not need imp*/}

	virtual unsigned int OnHandleBufferCharEvent(const void *pData,unsigned int nLen,int nPortNum)
	{
		THString buf=m_port.BufferToString(pData,nLen);
		int nLastState=MODEMSTATUS_NONE;
		int ret=0;
		int spos;
		THString key;
		if (THStringConv::MultiFind(buf,&m_ResultKey,FALSE,0,&spos,&key))
		{
			if (key.Find(_T("OK"),0)!=-1 || key.Find(_T(">"),0)!=-1)
				nLastState=MODEMSTATUS_OK;
			else if (key.Find(_T("ERROR"),0)!=-1)
				nLastState=MODEMSTATUS_ERROR;
			ret=spos+key.GetLength();
			if (ret!=0)
				buf=buf.Left(ret);
		}
		//判断是否为Modem主动发出的消息
		unsigned int chkret=OnCheckComingMsg(nLastState,buf);
		if (chkret>0)
		{
			//说明消息是由Modem主动发出的，先不响应原等待
			return chkret;
		}
		if (nLastState!=MODEMSTATUS_NONE)
		{
			if (m_nTimeout==0)
			{
				//异步模式，回调响应内容
				if (m_handler) m_handler->OnResponseRequest(buf,nLastState);
			}
			else
			{
				//同步模式，通知等待端
				m_curResp=buf;
				m_nLastState=nLastState;
				m_evResp.Set();
				//同步模式下，如果上一次的响应还没有处理，等待处理完先，直到等待到超时
				int wait=m_nTimeout;
				while(m_nLastState!=MODEMSTATUS_NONE && wait>0)
				{
					Sleep(100);
					wait-=100;
				}
				m_nLastState=MODEMSTATUS_NONE;
			}
		}
		return ret;
	}

	virtual unsigned int OnCheckComingMsg(int nLast,THString str)
	{
		//not need imp
		return 0;
	}
	//等待响应，同步模式下使用，如果当前为异步模式，返回成功MODEMSTATUS_OK
	int WaitForResponse(THString *ret)
	{
		//如果当前为异步模式，直接返回成功MODEMSTATUS_OK
		if (m_nTimeout==0) return MODEMSTATUS_OK;
		if (m_evResp.WaitForObject(m_nTimeout))
		{
			m_evResp.Reset();
			if (ret) *ret=m_curResp;
			int nLastState=m_nLastState;
			m_nLastState=MODEMSTATUS_NONE;
			return nLastState;
		}
		if (ret) ret->Empty();
		return MODEMSTATUS_TIMEOUT;
	}

	int m_nTimeout;
	volatile int m_nLastState;
	THSerialPort m_port;
	THEvent m_evResp;
	THString m_curResp;
	ITHModemEventHandler *m_handler;
	THStringArray m_ResultKey;
	THTimerThread m_timer;
};

//不通知TE，储存在默认内存位置，AT+CNMI=0,0
#define GSMMODEM_NOTIFY_MODE_NONETE_SAVEDEF		0
//等待数据线空闲再通知，储存在默认内存位置，AT+CNMI=2,1
#define GSMMODEM_NOTIFY_MODE_IDLETE_SAVEDEF		1
//等待数据线空闲再通知，储存在SIM卡或不保存，AT+CNMI=2,2
#define GSMMODEM_NOTIFY_MODE_IDLETE_SAVESIMORNOT	2

// 用户信息编码方式
#define GSMMODEM_ENCODEING_7BIT		0
#define GSMMODEM_ENCODEING_8BIT		4
#define GSMMODEM_ENCODEING_UCS2		8

//电话簿
#define PHONEBOOK_SIM				_T("SM")
#define PHONEBOOK_PHONE				_T("ME")
#define PHONEBOOK_BOTH				_T("MT")
#define PHONEBOOK_FIXDIAL			_T("FD")
#define PHONEBOOK_LASTDIAL			_T("LD")
#define PHONEBOOK_TA				_T("TA")

class ITHGsmModemEventHandler : public ITHModemEventHandler
{
public:
	//这里虽然已经在消息泵中响应，但是会阻塞之后的读短信回调，因此这里响应应该在Post消息到外部处理
	virtual void OnComingSMS(int idx)=0;
};

/**
* @brief 手机Modem封装类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-08-11 新建类
*/
/**<pre>
用法：
	//同步模式
	class MyCls: public ITHGsmModemEventHandler
	{
	public:
		//异步模式才使用
		virtual void OnResponseRequest(THString str,int status){}
		//对Modem发出的通知进行处理，同步，异步模式都可处理
		virtual void OnComingMessage(THString str,int status);
		//串口错误事件处理，同步，异步模式都可处理
		virtual void OnHandleError(int nErrorType,int nPortNum);
		//这里虽然已经在消息泵中响应，但是会阻塞之后的读短信回调，因此这里响应应该在Post消息到外部处理
		virtual void OnComingSMS(int idx)
		{::PostMessage(m_hWnd,WM_COMINGSMS,idx,0);}
	};
	LRESULT SomeCls::OnComingSMSMsg(WPARAM wParam,LPARAM lParam)
	{
		int idx=(int)wParam;
		THString sender,msg;
		THTime time;
		if (gsm.ReadSMS(idx,&sender,&msg,&time))
		{
			THDebug(_T("Receive Msg %d: sender:%s msg:%s time:%s"),idx,sender,msg,time.Format());
			gsm.DeleteSMS(idx);
		}
		return 0;
	}
	THGsmModem gsm;
	if (gsm.OpenModem(1))
	{
		if (gsm.IsReady())
		{
			THDebug(_T("Modem Ready!"));
			gsm.SetEcho();
			gsm.SetPduMode();
			gsm.SetGsmHandler(this);
			gsm.SetMsgComingNotifyMode();
			THString sender,msg;
			THTime ti;
			int idx=2;
			if (gsm.ReadSMS(idx,&sender,&msg,&ti))
			{
				THDebug(_T("Receive Msg %d: sender:%s msg:%s time:%s"),idx,sender,msg,ti.Format());
			}
			gsm.SendSMS(_T("13928557836"),_T("测试测试"));//,_T("13800757500"));
		}
		else
			THDebug(_T("Not Have Modem Here!"));
		//gsm.CloseModem();
	}
	//异步模式
	class MyCls: public ITHGsmModemEventHandler
	{
	public:
		//对向Modem发出的请求，进行回调处理，仅用在异步模式
		virtual void OnResponseRequest(THString str,int status)
		{
			if (status==MODEMSTATUS_OK)
				...
			//从这里返回获得短信的pdu编码，然后调用THGsmModem::DecodeMsg()解出短消息
		}
		//对Modem发出的通知进行处理，同步，异步模式都可处理
		virtual void OnComingMessage(THString str,int status);
		//串口错误事件处理，同步，异步模式都可处理
		virtual void OnHandleError(int nErrorType,int nPortNum);
		//异步模式下，该函数可以直接调用短信读取
		virtual void OnComingSMS(int idx)
		{
			THString sender,msg;
			THTime time;
			if (gsm.ReadSMS(idx,NULL,NULL,NULL))
			{
				gsm.DeleteSMS(idx);
			}
		}
	};
	THGsmModem gsm;
	if (gsm.OpenModem(1,115200,0))
	{
		gsm.SetGsmHandler(MyCls);
		m.IsReady();
		m.SetEcho();
	}
</pre>*/
class THGsmModem : public THModem ,public ITHModemEventHandler
{
public:
	THGsmModem()
	{
		m_gsmhandler=NULL;
		SetHandler(this);
	}
	virtual ~THGsmModem(){}

	void SetGsmHandler(ITHGsmModemEventHandler *handler){m_gsmhandler=handler;}
	/**
	* @brief 设置使用PDU模式，FALSE使用TEXT方式
	* @return 是否成功
	*/
	BOOL SetPduMode(BOOL bPdu=TRUE)
	{
		// PDU模式
		THString cmd;
		if (bPdu)
			cmd=_T("AT+CMGF=0\r\n");
		else
			cmd=_T("AT+CMGF=1\r\n");
		if (!WriteToPort(cmd,TRUE,TRUE)) return FALSE;
		if (WaitForResponse(NULL)==MODEMSTATUS_OK) return TRUE;
		return FALSE;
	}

	/**
	* @brief 设置收到短信的通知方式
	* @return 是否成功
	*/
	/**
		短信消息类：
			class 0: 只显示，不储存
			class 1: 储存在ME内存中
			class 2: 储存在SIM卡中
			class 3: 直接传输到TE
		AT+CNMI=[<mode>[,<mt>[,<bm>[,<ds>[,<bfr>]]]]]
			mode 通知方式：
				0- 不通知TE。
				1- 只在数据线空闲情况下，通知TE；否则不通知TE。
				2- 通知TE。在数据线占用情况下，先缓冲起来，待数据线空闲时再通知。
				3- 通知TE。在数据线占用情况下，通知混合在数据中一起传输。
			mt 消息储存方式：
				0- 储存到默认内存位置（包括class 3）。
				1- 储存到默认内存位置，并且向TE发出通知（包括class 3）。
				2- 对于class2，储存到SIM卡，向TE发出通知；其他class，直接将消息转发到TE。
				3- 对于class3，直接将消息转发到TE，对于其他class，同mt=1。
		GSMMODEM_NOTIFY_MODE_IDLETE_SAVEDEF			AT+CNMI=2,1
		GSMMODEM_NOTIFY_MODE_IDLETE_SAVESIMORNOT	AT+CNMI=2,2
	*/
	BOOL SetMsgComingNotifyMode(int NotifyMode=GSMMODEM_NOTIFY_MODE_IDLETE_SAVEDEF)
	{
		THString cmd;
		if (NotifyMode==GSMMODEM_NOTIFY_MODE_IDLETE_SAVEDEF)
			cmd=_T("AT+CNMI=2,1\r\n");
		else if (NotifyMode==GSMMODEM_NOTIFY_MODE_IDLETE_SAVESIMORNOT)
			cmd=_T("AT+CNMI=2,2\r\n");
		else if (NotifyMode==GSMMODEM_NOTIFY_MODE_NONETE_SAVEDEF)
			cmd=_T("AT+CNMI=0,0\r\n");
		if (cmd.IsEmpty()) return FALSE;
		if (!WriteToPort(cmd,TRUE,TRUE)) return FALSE;
		if (WaitForResponse(NULL)==MODEMSTATUS_OK) return TRUE;
		return FALSE;
	}

	BOOL SendSMS(THString TargetNo,THString msg,THString smsc=_T(""))
	{
		if (TargetNo.IsEmpty() || msg.IsEmpty()) return FALSE;
		msg=THStringConv::LimitStringCount(msg,70);
		int nPduLength;		// PDU串长度
		unsigned char nSmscLength;	// SMSC串长度
		char cmd[16];		// 命令串
		char pdu[512];		// PDU串

		SM_PARAM Src;
		memset(&Src, 0, sizeof(SM_PARAM));
		// 去掉号码前的"+"
		if(smsc.GetAt(0)=='+') smsc=smsc.Mid(1);
		if(TargetNo.GetAt(0)=='+') TargetNo = TargetNo.Mid(1);
		// 在号码前加"86"
		if(!smsc.IsEmpty() && smsc.Left(2) != _T("86")) smsc = _T("86") + smsc;
		//目标号码不作强制86处理，有86为91国际模式，没有86为81本地模式
		//if(TargetNo.Left(2) != _T("86")) TargetNo = _T("86") + TargetNo;

		char *strSmsc=THCharset::t2a(smsc);
		char *strNumber=THCharset::t2a(TargetNo);
		char *strContent=THCharset::t2a(msg);
		if (!strSmsc || !strNumber || !strContent)
		{
			if (strSmsc) THCharset::free(strSmsc);
			if (strNumber) THCharset::free(strNumber);
			if (strContent) THCharset::free(strContent);
			return FALSE;
		}
		// 填充短消息结构
		Src.index=0;
		strcpy(Src.SCA,strSmsc);
		strcpy(Src.TPA,strNumber);
		strcpy(Src.TP_UD,strContent);
		Src.TP_PID = 0;
		Src.TP_DCS = GSMMODEM_ENCODEING_UCS2;
		if (strSmsc) THCharset::free(strSmsc);
		if (strNumber) THCharset::free(strNumber);
		if (strContent) THCharset::free(strContent);

		nPduLength = gsmEncodePdu(&Src, pdu);	// 根据PDU参数，编码PDU串
		strcat(pdu,"\x1a");		// 以Ctrl-Z结束
		gsmString2Bytes(pdu, &nSmscLength, 2);	// 取PDU串中的SMSC信息长度
		nSmscLength++;		// 加上长度字节本身
		// 命令中的长度，不包括SMSC信息长度，以数据字节计
		sprintf(cmd, "AT+CMGS=%d\r", nPduLength / 2 - nSmscLength);	// 生成命令
		if (!WriteToPortBin((void *)cmd,(unsigned int)strlen(cmd),TRUE)) return FALSE;
		int ret;
		for(int i=0;i<3;i++)
		{
			ret=WaitForResponse(NULL);
			if (ret==MODEMSTATUS_ERROR) return FALSE;
			else if (ret==MODEMSTATUS_OK)
				break;
		}
		if (ret!=MODEMSTATUS_OK) return FALSE;
		if (!WriteToPortBin((void *)pdu,(unsigned int)strlen(pdu),TRUE)) return FALSE;
		for(i=0;i<15;i++)//最多 22.5 sec
		{
			ret=WaitForResponse(NULL);
			if (ret==MODEMSTATUS_ERROR) return FALSE;
			else if (ret==MODEMSTATUS_OK)
				break;
		}
		if (ret==MODEMSTATUS_OK) return TRUE;
		//可能发生问题了，为了保证modem正常重新发送一个"\x1a"
		WriteToPortBin((void *)"\x1a",1,TRUE);
		return FALSE;
	}

	BOOL ReadSMS(int idx,THString *sender,THString *msg,THTime *ti)
	{
		THString cmd;
		cmd.Format(_T("AT+CMGR=%d\r\n"),idx);
		if (!WriteToPort(cmd,TRUE,TRUE)) return FALSE;
		THString ret;
		if (WaitForResponse(&ret)==MODEMSTATUS_OK)
			return DecodeMsg(ret,sender,msg,ti);
		return FALSE;
	}

	/**
	* @brief 通过短信的PDU原文解出短信
	* @param buffer		pdu编码的短信
	* @param sender		短信发送人
	* @param msg		信息内容
	* @param ti			发送时间
	* @return 是否成功
	*/
	BOOL DecodeMsg(THString buffer,THString *sender,THString *msg,THTime *ti)
	{
		//+CMGR: 8,27
		int spos=buffer.Find(_T("+CMGR:"),0);
		if (spos==-1) return FALSE;
		int pos=buffer.Find(_T("\r\n"),spos);
		if (pos==-1) return FALSE;
		buffer=buffer.Mid(pos+2);
		pos=buffer.Find(_T("\r\n"),0);
		if (pos!=-1)
			buffer=buffer.Left(pos);
		SM_PARAM smsg;
		memset(&smsg,0,sizeof(smsg));
		char *tmp=THCharset::t2a(buffer);
		if (!tmp) return FALSE;
		int len=gsmDecodePdu(tmp,&smsg);
		THCharset::free(tmp);
		if (len<=0) return FALSE;
		if (sender)
		{
			*sender=THCharset::a2t(smsg.TPA);
			if (sender->Left(1)==_T("+")) *sender=sender->Mid(1);
			if (sender->Left(2)==_T("86")) *sender=sender->Mid(2);
		}
		if (msg) *msg=THCharset::a2t(smsg.TP_UD);
		if (ti)
		{
			THString TimeStr=THCharset::a2t(smsg.TP_SCTS);
			ti->SetTime(2000+THStringConv::s2u(TimeStr.Left(2)),//y
				THStringConv::s2u(TimeStr.Mid(2).Left(2)),//m
				THStringConv::s2u(TimeStr.Mid(4).Left(2)),//d
				THStringConv::s2u(TimeStr.Mid(6).Left(2)),//h
				THStringConv::s2u(TimeStr.Mid(8).Left(2)),//m
				THStringConv::s2u(TimeStr.Mid(10).Left(2)));//s
		}
		return TRUE;
	}

	/**
	* @brief 删除短信息
	* @param index		短消息序号1-255
	* @return 是否成功
	*/
	BOOL DeleteSMS(int index)
	{
		THString cmd;
		cmd.Format(_T("AT+CMGD=%d\r\n"),index);
		if (!WriteToPort(cmd,TRUE,TRUE)) return FALSE;
		if (WaitForResponse(NULL)==MODEMSTATUS_OK) return TRUE;
		return FALSE;
	}

	THString GetMobileIMEI()
	{
		THString cmd;
		THString ret;
		cmd=_T("AT+CGSN\r\n");
		if (WriteToPort(cmd,TRUE,TRUE))
			WaitForResponse(&ret);
		return ret;
	}

	BOOL GetMobilePhoneBookSize(int *used,int *total,THString phonebooktype=PHONEBOOK_SIM)
	{
		THString cmd;
		cmd.Format(_T("AT+CPBS=\"%s\"\r\n"),phonebooktype);
		if (!WriteToPort(cmd,TRUE,TRUE)) return FALSE;
		if (WaitForResponse(NULL)!=MODEMSTATUS_OK) return FALSE;
		cmd=_T("AT+CPBS?\r\n");
		if (!WriteToPort(cmd,TRUE,TRUE)) return FALSE;
		THString ret;
		if (WaitForResponse(&ret)!=MODEMSTATUS_OK) return FALSE;
		int pos=ret.Find(_T(","),0);
		if (pos==-1) return FALSE;
		int pos2=ret.Find(_T(","),pos+1);
		if (pos2==-1) return FALSE;
		if (used) *used=THStringConv::s2u(ret.Mid(pos+1));
		if (total) *total=THStringConv::s2u(ret.Mid(pos2+1));
		return TRUE;
	}
protected:
	ITHGsmModemEventHandler *m_gsmhandler;

	virtual unsigned int OnCheckComingMsg(int nStatus,THString str)
	{
		//处理Modem主动发送过来的信息
		int pos=str.Find(_T("+CMTI"),0);
		if (pos!=-1)
		{
			int pos2=str.Find(_T("\r"),pos);
			if (pos2!=-1)
			{
				if (str.GetAt(pos2+1)=='\n') pos2++;
				str=str.Left(pos2);
			}
			else
				pos2=str.GetLength();
			//handler就是自己
			if (m_handler)
				m_handler->OnComingMessage(str,nStatus);
			return pos2;
		}
		return 0;
	}

	// 短消息参数结构，编码/解码共用
	// 其中，字符串以'\0'结尾
	typedef struct {
		char SCA[16];			// 短消息服务中心号码(SMSC地址)
		char TPA[16];			// 目标号码或回复号码(TP-DA或TP-RA)
		char TP_PID;			// 用户信息协议标识(TP-PID),00:GSM 点对点
		char TP_DCS;			// 用户信息编码方式(TP-DCS),GSMMODEM_ENCODEING_XXX
		char TP_SCTS[16];		// 服务时间戳字符串(TP_SCTS),接收时用到07081022105850 = 07-08-10 22:10:58
		char TP_UD[160];		// 原始用户信息(编码前或解码后的TP-UD)
		short index;			// 短消息序号，在读取时用到
	} SM_PARAM;

	// 字节数据转换为可打印字符串
	// 如：{0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01} --> "C8329BFD0E01" 
	// 输入: pSrc - 源数据指针
	//       nSrcLength - 源数据长度
	// 输出: pDst - 目标字符串指针
	// 返回: 目标字符串长度
	int gsmBytes2String(const unsigned char* pSrc, char* pDst, int nSrcLength)
	{
		const char tab[]="0123456789ABCDEF";	// 0x0-0xf的字符查找表
		for (int i = 0; i < nSrcLength; i++)
		{
			*pDst++ = tab[*pSrc >> 4];		// 输出高4位
			*pDst++ = tab[*pSrc & 0x0f];	// 输出低4位
			pSrc++;
		}
		// 输出字符串加个结束符
		*pDst = '\0';
		// 返回目标字符串长度
		return (nSrcLength * 2);
	}

	// 可打印字符串转换为字节数据
	// 如："C8329BFD0E01" --> {0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01}
	// 输入: pSrc - 源字符串指针
	//       nSrcLength - 源字符串长度
	// 输出: pDst - 目标数据指针
	// 返回: 目标数据长度
	int gsmString2Bytes(const char* pSrc, unsigned char* pDst, int nSrcLength)
	{
		for (int i = 0; i < nSrcLength; i += 2)
		{
			// 输出高4位
			if ((*pSrc >= '0') && (*pSrc <= '9'))
				*pDst = (*pSrc - '0') << 4;
			else
				*pDst = (*pSrc - 'A' + 10) << 4;
			pSrc++;
			// 输出低4位
			if ((*pSrc>='0') && (*pSrc<='9'))
				*pDst |= *pSrc - '0';
			else
				*pDst |= *pSrc - 'A' + 10;
			pSrc++;
			pDst++;
		}
		// 返回目标数据长度
		return (nSrcLength / 2);
	}

	// 7bit编码
	// 输入: pSrc - 源字符串指针
	//       nSrcLength - 源字符串长度
	// 输出: pDst - 目标编码串指针
	// 返回: 目标编码串长度
	int gsmEncode7bit(const char* pSrc, unsigned char* pDst, int nSrcLength)
	{
		int nSrc;		// 源字符串的计数值
		int nDst;		// 目标编码串的计数值
		int nChar;		// 当前正在处理的组内字符字节的序号，范围是0-7
		unsigned char nLeft;	// 上一字节残余的数据

		// 计数值初始化
		nSrc = 0;
		nDst = 0;

		// 将源串每8个字节分为一组，压缩成7个字节
		// 循环该处理过程，直至源串被处理完
		// 如果分组不到8字节，也能正确处理
		while (nSrc < nSrcLength)
		{
			// 取源字符串的计数值的最低3位
			nChar = nSrc & 7;

			// 处理源串的每个字节
			if(nChar == 0)
			{
				// 组内第一个字节，只是保存起来，待处理下一个字节时使用
				nLeft = *pSrc;
			}
			else
			{
				// 组内其它字节，将其右边部分与残余数据相加，得到一个目标编码字节
				*pDst = (*pSrc << (8-nChar)) | nLeft;

				// 将该字节剩下的左边部分，作为残余数据保存起来
				nLeft = *pSrc >> nChar;

				// 修改目标串的指针和计数值
				pDst++;
				nDst++;
			}

			// 修改源串的指针和计数值
			pSrc++;
			nSrc++;
		}

		// 返回目标串长度
		return nDst;
	}

	// 7bit解码
	// 输入: pSrc - 源编码串指针
	//       nSrcLength - 源编码串长度
	// 输出: pDst - 目标字符串指针
	// 返回: 目标字符串长度
	int gsmDecode7bit(const unsigned char* pSrc, char* pDst, int nSrcLength)
	{
		int nSrc;		// 源字符串的计数值
		int nDst;		// 目标解码串的计数值
		int nByte;		// 当前正在处理的组内字节的序号，范围是0-6
		unsigned char nLeft;	// 上一字节残余的数据

		// 计数值初始化
		nSrc = 0;
		nDst = 0;
		
		// 组内字节序号和残余数据初始化
		nByte = 0;
		nLeft = 0;

		// 将源数据每7个字节分为一组，解压缩成8个字节
		// 循环该处理过程，直至源数据被处理完
		// 如果分组不到7字节，也能正确处理
		while(nSrc<nSrcLength)
		{
			// 将源字节右边部分与残余数据相加，去掉最高位，得到一个目标解码字节
			*pDst = ((*pSrc << nByte) | nLeft) & 0x7f;

			// 将该字节剩下的左边部分，作为残余数据保存起来
			nLeft = *pSrc >> (7-nByte);

			// 修改目标串的指针和计数值
			pDst++;
			nDst++;

			// 修改字节计数值
			nByte++;

			// 到了一组的最后一个字节
			if(nByte == 7)
			{
				// 额外得到一个目标解码字节
				*pDst = nLeft;

				// 修改目标串的指针和计数值
				pDst++;
				nDst++;

				// 组内字节序号和残余数据初始化
				nByte = 0;
				nLeft = 0;
			}

			// 修改源串的指针和计数值
			pSrc++;
			nSrc++;
		}

		// 输出字符串加个结束符
		*pDst = '\0';

		// 返回目标串长度
		return nDst;
	}
	// 8bit编码
	// 输入: pSrc - 源字符串指针
	//       nSrcLength - 源字符串长度
	// 输出: pDst - 目标编码串指针
	// 返回: 目标编码串长度
	int gsmEncode8bit(const char* pSrc, unsigned char* pDst, int nSrcLength)
	{
		// 简单复制
		memcpy(pDst, pSrc, nSrcLength);
		return nSrcLength;
	}
	// 8bit解码
	// 输入: pSrc - 源编码串指针
	//       nSrcLength -  源编码串长度
	// 输出: pDst -  目标字符串指针
	// 返回: 目标字符串长度
	int gsmDecode8bit(const unsigned char* pSrc, char* pDst, int nSrcLength)
	{
		// 简单复制
		memcpy(pDst, pSrc, nSrcLength);
		// 输出字符串加个结束符
		*pDst = '\0';
		return nSrcLength;
	}
	// UCS2编码
	// 输入: pSrc - 源字符串指针
	//       nSrcLength - 源字符串长度
	// 输出: pDst - 目标编码串指针
	// 返回: 目标编码串长度
	int gsmEncodeUcs2(const char* pSrc, unsigned char* pDst, int nSrcLength)
	{
		int nDstLength;		// UNICODE宽字符数目
		WCHAR wchar[128];	// UNICODE串缓冲区
		// 字符串-->UNICODE串
		nDstLength = MultiByteToWideChar(CP_ACP, 0, pSrc, nSrcLength, wchar, 128);
		// 高低字节对调，输出
		for(int i=0; i<nDstLength; i++)
		{
			*pDst++ = wchar[i] >> 8;		// 先输出高位字节
			*pDst++ = wchar[i] & 0xff;		// 后输出低位字节
		}
		// 返回目标编码串长度
		return nDstLength * 2;
	}
	// UCS2解码
	// 输入: pSrc - 源编码串指针
	//       nSrcLength -  源编码串长度
	// 输出: pDst -  目标字符串指针
	// 返回: 目标字符串长度
	int gsmDecodeUcs2(const unsigned char* pSrc, char* pDst, int nSrcLength)
	{
		int nDstLength;		// UNICODE宽字符数目
		WCHAR wchar[128];	// UNICODE串缓冲区
		// 高低字节对调，拼成UNICODE
		for(int i=0; i<nSrcLength/2; i++)
		{
			wchar[i] = *pSrc++ << 8;	// 先高位字节
			wchar[i] |= *pSrc++;		// 后低位字节
		}
		// UNICODE串-->字符串
		nDstLength = WideCharToMultiByte(CP_ACP, 0, wchar, nSrcLength/2, pDst, 160, NULL, NULL);
		// 输出字符串加个结束符
		pDst[nDstLength] = '\0';
		// 返回目标字符串长度
		return nDstLength;
	}
	// 正常顺序的字符串转换为两两颠倒的字符串，若长度为奇数，补'F'凑成偶数
	// 如："8613851872468" --> "683158812764F8"
	// 输入: pSrc - 源字符串指针
	//       nSrcLength - 源字符串长度
	// 输出: pDst - 目标字符串指针
	// 返回: 目标字符串长度
	int gsmInvertNumbers(const char* pSrc, char* pDst, int nSrcLength)
	{
		int nDstLength;		// 目标字符串长度
		char ch;			// 用于保存一个字符
		// 复制串长度
		nDstLength = nSrcLength;
		// 两两颠倒
		for(int i=0; i<nSrcLength;i+=2)
		{
			ch = *pSrc++;		// 保存先出现的字符
			*pDst++ = *pSrc++;	// 复制后出现的字符
			*pDst++ = ch;		// 复制先出现的字符
		}
		// 源串长度是奇数吗？
		if(nSrcLength & 1)
		{
			*(pDst-2) = 'F';	// 补'F'
			nDstLength++;		// 目标串长度加1
		}
		// 输出字符串加个结束符
		*pDst = '\0';
		// 返回目标字符串长度
		return nDstLength;
	}
	// 两两颠倒的字符串转换为正常顺序的字符串
	// 如："683158812764F8" --> "8613851872468"
	// 输入: pSrc - 源字符串指针
	//       nSrcLength - 源字符串长度
	// 输出: pDst - 目标字符串指针
	// 返回: 目标字符串长度
	int gsmSerializeNumbers(const char* pSrc, char* pDst, int nSrcLength)
	{
		int nDstLength;		// 目标字符串长度
		char ch;			// 用于保存一个字符

		// 复制串长度
		nDstLength = nSrcLength;

		// 两两颠倒
		for(int i=0; i<nSrcLength;i+=2)
		{
			ch = *pSrc++;		// 保存先出现的字符
			*pDst++ = *pSrc++;	// 复制后出现的字符
			*pDst++ = ch;		// 复制先出现的字符
		}

		// 最后的字符是'F'吗？
		if(*(pDst-1) == 'F')
		{
			pDst--;
			nDstLength--;		// 目标字符串长度减1
		}

		// 输出字符串加个结束符
		*pDst = '\0';

		// 返回目标字符串长度
		return nDstLength;
	}
	// PDU编码，用于编制、发送短消息
	// 输入: pSrc - 源PDU参数指针
	// 输出: pDst - 目标PDU串指针
	// 返回: 目标PDU串长度
	int gsmEncodePdu(const SM_PARAM* pSrc, char* pDst)
	{
		int nLength;			// 内部用的串长度
		int nDstLength;			// 目标PDU串长度
		unsigned char buf[256];	// 内部用的缓冲区
		// SMSC地址信息段
		nLength = (int)strlen(pSrc->SCA);	// SMSC地址字符串的长度
		// 无SMSC模式
		if (nLength==0)
		{
			buf[0]=0;
			nDstLength = gsmBytes2String(buf, pDst, 1);		// 转换2个字节到目标PDU串
		}
		else
		{
			buf[0] = (char)((nLength & 1) == 0 ? nLength : nLength + 1) / 2 + 1;	// SMSC地址信息长度
			buf[1] = 0x91;		// 固定: 用国际格式号码
			nDstLength = gsmBytes2String(buf, pDst, 2);		// 转换2个字节到目标PDU串
			nDstLength += gsmInvertNumbers(pSrc->SCA, &pDst[nDstLength], nLength);	// 转换SMSC号码到目标PDU串
		}
		// TPDU段基本参数、目标地址等
		nLength = (int)strlen(pSrc->TPA);	// TP-DA地址字符串的长度
		buf[0] = 0x11;					// 是发送短信(TP-MTI=01)，TP-VP用相对格式(TP-VPF=10)
		buf[1] = 0;						// TP-MR=0
		buf[2] = (char)nLength;			// 目标地址数字个数(TP-DA地址字符串真实长度)
		if (strncmp(pSrc->TPA,"86",2)==0)
			buf[3] = 0x91;				//用国际格式号码
		else
			buf[3] = 0x81;				//用本地格式号码
		nDstLength += gsmBytes2String(buf, &pDst[nDstLength], 4);		// 转换4个字节到目标PDU串
		nDstLength += gsmInvertNumbers(pSrc->TPA, &pDst[nDstLength], nLength);	// 转换TP-DA到目标PDU串
		// TPDU段协议标识、编码方式、用户信息等
		nLength = (int)strlen(pSrc->TP_UD);	// 用户信息字符串的长度
		buf[0] = pSrc->TP_PID;			// 协议标识(TP-PID)
		buf[1] = pSrc->TP_DCS;			// 用户信息编码方式(TP-DCS)
		buf[2] = 0;						// 有效期(TP-VP)为5分钟
		if(pSrc->TP_DCS == GSMMODEM_ENCODEING_7BIT)	
		{
			// 7-bit编码方式
			buf[3] = nLength;			// 编码前长度
			nLength = gsmEncode7bit(pSrc->TP_UD, &buf[4], nLength+1) + 4;	// 转换TP-DA到目标PDU串
		}
		else if(pSrc->TP_DCS == GSMMODEM_ENCODEING_UCS2)
		{
			// UCS2编码方式
			buf[3] = gsmEncodeUcs2(pSrc->TP_UD, &buf[4], nLength);	// 转换TP-DA到目标PDU串
			nLength = buf[3] + 4;		// nLength等于该段数据长度
		}
		else
		{
			// 8-bit编码方式
			buf[3] = gsmEncode8bit(pSrc->TP_UD, &buf[4], nLength);	// 转换TP-DA到目标PDU串
			nLength = buf[3] + 4;		// nLength等于该段数据长度
		}
		nDstLength += gsmBytes2String(buf, &pDst[nDstLength], nLength);		// 转换该段数据到目标PDU串
		// 返回目标字符串长度
		return nDstLength;
	}
	// PDU解码，用于接收、阅读短消息
	// 输入: pSrc - 源PDU串指针
	// 输出: pDst - 目标PDU参数指针
	// 返回: 用户信息串长度
	int gsmDecodePdu(const char* pSrc, SM_PARAM* pDst)
	{
		int nDstLength;			// 目标PDU串长度
		unsigned char tmp;		// 内部用的临时字节变量
		unsigned char buf[256];	// 内部用的缓冲区
		// SMSC地址信息段
		gsmString2Bytes(pSrc, &tmp, 2);	// 取长度
		tmp = (tmp - 1) * 2;	// SMSC号码串长度
		pSrc += 4;			// 指针后移，忽略了SMSC地址格式
		gsmSerializeNumbers(pSrc, pDst->SCA, tmp);	// 转换SMSC号码到目标PDU串
		pSrc += tmp;		// 指针后移

		// TPDU段基本参数
		gsmString2Bytes(pSrc, &tmp, 2);	// 取基本参数
		pSrc += 2;		// 指针后移

		// 取回复号码
		gsmString2Bytes(pSrc, &tmp, 2);	// 取长度
		if(tmp & 1) tmp += 1;	// 调整奇偶性
		pSrc += 4;			// 指针后移，忽略了回复地址(TP-RA)格式
		gsmSerializeNumbers(pSrc, pDst->TPA, tmp);	// 取TP-RA号码
		pSrc += tmp;		// 指针后移

		// TPDU段协议标识、编码方式、用户信息等
		gsmString2Bytes(pSrc, (unsigned char*)&pDst->TP_PID, 2);	// 取协议标识(TP-PID)
		pSrc += 2;		// 指针后移
		gsmString2Bytes(pSrc, (unsigned char*)&pDst->TP_DCS, 2);	// 取编码方式(TP-DCS)
		pSrc += 2;		// 指针后移
		gsmSerializeNumbers(pSrc, pDst->TP_SCTS, 14);		// 服务时间戳字符串(TP_SCTS) 
		pSrc += 14;		// 指针后移
		gsmString2Bytes(pSrc, &tmp, 2);	// 用户信息长度(TP-UDL)
		pSrc += 2;		// 指针后移
		if(pDst->TP_DCS == GSMMODEM_ENCODEING_7BIT)	
		{
			// 7-bit解码
			nDstLength = gsmString2Bytes(pSrc, buf, tmp & 7 ? (int)tmp * 7 / 4 + 2 : (int)tmp * 7 / 4);	// 格式转换
			gsmDecode7bit(buf, pDst->TP_UD, nDstLength);	// 转换到TP-DU
			nDstLength = tmp;
		}
		else if(pDst->TP_DCS == GSMMODEM_ENCODEING_UCS2)
		{
			// UCS2解码
			nDstLength = gsmString2Bytes(pSrc, buf, tmp * 2);			// 格式转换
			nDstLength = gsmDecodeUcs2(buf, pDst->TP_UD, nDstLength);	// 转换到TP-DU
		}
		else
		{
			// 8-bit解码
			nDstLength = gsmString2Bytes(pSrc, buf, tmp * 2);			// 格式转换
			nDstLength = gsmDecode8bit(buf, pDst->TP_UD, nDstLength);	// 转换到TP-DU
		}

		// 返回目标字符串长度
		return nDstLength;
	}
	virtual void OnHandleError(int nErrorType,int nPortNum)
	{
		if (m_gsmhandler) m_gsmhandler->OnHandleError(nErrorType,nPortNum);
	}
	virtual void OnResponseRequest(THString str,int status)
	{
		if (m_gsmhandler) m_gsmhandler->OnResponseRequest(str,status);
	}
	virtual void OnComingMessage(THString str,int status)
	{
		if (m_gsmhandler)
		{
			m_gsmhandler->OnComingMessage(str,status);
			int pos=str.Find(_T("+CMTI"),0);
			if (pos!=-1)
			{
				pos=str.Find(_T(","),pos);
				if (pos!=-1)
				{
					m_gsmhandler->OnComingSMS(THStringConv::s2u(str.Mid(pos+1)));
				}
			}
		}
	}
};

class THGsmModemMachineEventHandler
{
public:
	virtual void OnSmsSendSuccess(const THString sendto,const THString msg,const THTime sti){}
	//响应错误事件发送延时，返回FALSE为取消发送
	virtual BOOL OnSMSCannotSendImmediate(const THString sendto,const THString msg,const THTime sti,const int retrycount){return TRUE;}
	//响应来信息
	virtual void OnSMSComing(const int idx,const THString sender,const THString msg,const THTime ti)=0;
};

/**
* @brief 手机Modem封装类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-08-11 新建类
*/
/**<pre>
用法：
	实现 THGsmModemMachineEventHandler mycls类响应收到消息
	m_machine.SetReceiveMessageHandler(mycls);
	if (m_machine.InitModem(1,9600))
	{
		THDebug(_T("machine start now!"));
		m_machine.SendSMS(_T("13925901884"),_T("这是测试"));
		m_machine.SendSMS(_T("13925901884"),_T("这是好长的测试3"));
	}
</pre>*/
class THGsmModemMachine : public ITHGsmModemEventHandler ,public THThread
{
public:
	THGsmModemMachine()
	{
		m_handler=NULL;
		m_lr.SetFreeProc(FreeCallBack);
		m_ls.SetFreeProc(FreeCallBack);
	}
	virtual ~THGsmModemMachine()
	{
		CloseModem();
	}

	void SetReceiveMessageHandler(THGsmModemMachineEventHandler *handler){m_handler=handler;}

	BOOL InitModem(UINT nPortNo,UINT bandrate=115200)
	{
		if (!m_modem.OpenModem(nPortNo,bandrate)) return FALSE;
		if (!m_modem.IsReady())
		{
			m_modem.Reset();
			Sleep(3000);
			if (!m_modem.IsReady()) return FALSE;
		}
		if (!IsThreadRunning(1))
			if (StartThread(1,NULL,5000,TRUE)==0) return FALSE;
		if (!IsThreadRunning(2))
			if (StartThread(2,NULL,5000,TRUE)==0) return FALSE;
		m_modem.SetEcho();
		m_modem.SetPduMode();
		m_modem.SetGsmHandler(this);
		THString sender,msg;
		THTime ti;
		BOOL bReaded=FALSE;
		for(int idx=1;idx<20;idx++)
		{
			if (m_modem.ReadSMS(idx,&sender,&msg,&ti))
			{
				bReaded=TRUE;
				AddReceiveMessage(idx,&sender,&msg,&ti);
				m_modem.DeleteSMS(idx);
			}
			else
				break;
		}
		if (bReaded) SetThreadEvent(2);
		m_modem.SetMsgComingNotifyMode();
		m_modem.SetAutoPowerShutDown();
		return TRUE;
	}

	void CloseModem()
	{
		if (m_modem.IsOpened())
		{
			m_modem.SetMsgComingNotifyMode(GSMMODEM_NOTIFY_MODE_NONETE_SAVEDEF);
			m_modem.SetGsmHandler(NULL);
		}
		KillThread(1);
		KillThread(2);
		if (m_modem.IsOpened())
		{
			m_modem.SetAutoPowerShutDown(TRUE);
			m_modem.SetEcho(FALSE);
		}
		m_modem.CloseModem();
	}

	BOOL IsModemOpened(){return m_modem.IsOpened();}

	BOOL SendSMS(THString sendto,THString msg)
	{
		if (!IsModemOpened()) return FALSE;
		THSingleLock lock(&m_sm);
		SMSList *tmp=new SMSList;
		if (!tmp) return FALSE;
		tmp->idx=0;
		tmp->msg=msg;
		tmp->sender=sendto;
		//not need to parse it,it is current time,tmp->ti;
		if (!m_ls.AddTail(tmp)) return FALSE;
		return SetThreadEvent(1);
	}
	UINT GetWaitingQueue(){return m_ls.GetSize();}
	THGsmModem m_modem;
protected:
	THGsmModemMachineEventHandler *m_handler;
	THMutex m_sm;
	THMutex m_rm;
	typedef struct _SMSList
	{
		int idx;
		THString sender;
		THString msg;
		THTime ti;
	}SMSList;
	THList<SMSList *>m_ls,m_lr;
	static void FreeCallBack(void *key,void *value,void *adddata)
	{
		delete (SMSList *)value;
	}
	//这里虽然已经在消息泵中响应，但是会阻塞之后的读短信回调，因此这里响应应该在Post消息到外部处理
	virtual void OnComingSMS(int idx)
	{
		AddReceiveMessage(idx);
		SetThreadEvent(2);
	}

	void ThreadFunc(int threadid,void *adddata)
	{
		if (threadid==1)
		{
			int count=0;
			while(m_ls.GetSize()>0)
			{
				//send thread
				THSingleLock lock(&m_sm);
				SMSList *list;
				if (m_ls.GetHead(list,FALSE))
				{
					if (m_modem.SendSMS(list->sender,list->msg))
					{
						THString sender=list->sender;
						THString msg=list->msg;
						THTime ti=list->ti;
						m_ls.RemoveHead();
						m_sm.Unlock();
						if (m_handler)
							m_handler->OnSmsSendSuccess(sender,msg,ti);
						Sleep(1000);
						m_sm.Lock();
						count=0;
					}
					else
					{
						count++;
						BOOL bRemove=FALSE;
						if (m_handler)
						{
							//这里的unlock不十分好，这里unlock是因为，OnSMSCannotSendImmediate调用的响应，有可能加入新的短信，这时候会被循环地锁住了，不过删除m_ls的操作都只有本线程做，还是相对比较安全的
							m_sm.Unlock();
							if (m_handler->OnSMSCannotSendImmediate(list->sender,list->msg,list->ti,count)==FALSE)
							{
								count=0;
								m_ls.RemoveHead();
								bRemove=TRUE;
							}
							m_sm.Lock();
						}
						//如果不取消发送,等待一会儿再发
						if (bRemove==FALSE)
						{
							Sleep(3000);
							if (!m_modem.IsReady())
								if (!m_modem.IsReady())
									m_modem.Reset();
						}
					}
				}
			}
		}
		else if (threadid==2)
		{
			//receive thread
			if (m_handler)
			{
				while(m_lr.GetSize()>0)
				{
					m_rm.Lock();
					SMSList *list;
					if (m_lr.GetHead(list,FALSE))
					{
						BOOL bRead=TRUE;
						//证明短信还没有读进来
						if (list->sender.IsEmpty())
						{
							if (!m_modem.ReadSMS(list->idx,&list->sender,&list->msg,&list->ti))
								bRead=FALSE;
							else
								m_modem.DeleteSMS(list->idx);
						}
						if (bRead)
						{
							int idx=list->idx;
							THString sender=list->sender;
							THString msg=list->msg;
							THTime ti=list->ti;
							m_lr.RemoveHead();
							m_rm.Unlock();
							m_handler->OnSMSComing(idx,sender,msg,ti);
							m_rm.Lock();
						}
						else
							m_lr.RemoveHead();
					}
					m_rm.Unlock();
				}
			}
		}
	}

	void AddReceiveMessage(int idx,THString *sender=NULL,THString *msg=NULL,THTime *ti=NULL)
	{
		THSingleLock lock(&m_rm);
		SMSList *tmp=new SMSList;
		if (!tmp) return;
		tmp->idx=idx;
		if (msg) tmp->msg=*msg;
		if (sender) tmp->sender=*sender;
		if (ti) tmp->ti=*ti;
		m_lr.AddTail(tmp);
	}
	//对向Modem发出的请求，进行回调处理，仅用在异步模式
	virtual void OnResponseRequest(THString str,int status){}
	//对Modem发出的通知进行处理，同步，异步模式都可处理
	virtual void OnComingMessage(THString str,int status){}
	//串口错误事件处理，同步，异步模式都可处理
	virtual void OnHandleError(int nErrorType,int nPortNum){}
};