#pragma once

#include <THSerialPort.h>
#include <THSyn.h>
#include <THStruct.h>

class ITHModemEventHandler
{
public:
	//����Modem���������󣬽��лص������������첽ģʽ
	virtual void OnResponseRequest(THString str,int status)=0;
	//��Modem������֪ͨ���д���ͬ�����첽ģʽ���ɴ���
	virtual void OnComingMessage(THString str,int status)=0;
	//���ڴ����¼�����ͬ�����첽ģʽ���ɴ���
	virtual void OnHandleError(int nErrorType,int nPortNum)=0;
};

#define MODEMSTATUS_NONE	0
#define MODEMSTATUS_OK		1
#define MODEMSTATUS_ERROR	2
#define MODEMSTATUS_TIMEOUT	3

/**
* @brief Modem��װ��
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-08-12 �½���
*/
/**<pre>
�÷���
	//ͬ��ģʽ
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
	//�첽ģʽ
	class MyCls: public ITHModemEventHandler
	{
	public:
		//����Modem���������󣬽��лص������������첽ģʽ
		virtual void OnResponseRequest(THString str,int status)
		{
			if (status==MODEMSTATUS_OK)
				...
		}
		//��Modem������֪ͨ���д���ͬ�����첽ģʽ���ɴ���
		virtual void OnComingMessage(THString str,int status);
		//���ڴ����¼�����ͬ�����첽ģʽ���ɴ���
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
	* @brief ��Modem
	* @param portno			���ں�
	* @param baudrate		�����ʣ����Ϊ 115200
	* @param nTimeout		��ʱʱ�䣬����ͬ��ģʽ��Ϊ0��ʾʹ���첽ģʽ
	* @param paritytype		��żУ��
	* @param databits		����λ
	* @param stopbits		ֹͣλ
	* @return �Ƿ�ɹ�
	*/
	BOOL OpenModem(UINT nPortNo=1,UINT bandrate=115200,UINT nTimeout=1500,char paritytype=SPPARITYTYPE_NONE,UINT databits=8,UINT stopbits=1)
	{
		CloseModem();
		m_nTimeout=nTimeout;
		//������ǿ��ʹ���첽ģʽ��Modem���ͬ�����¼�ʵ��
		if (!m_port.InitPort(nPortNo,bandrate,4096,paritytype,databits,stopbits)) return FALSE;
		m_evResp.Reset();
		return TRUE;
	}

	/**
	* @brief �ر�Modem
	*/
	void CloseModem(){m_port.ClosePort();m_evResp.Set();}

	/**
	* @brief �ж��Ƿ����Modem�豸
	* @return �Ƿ����Modem�豸
	*/
	BOOL IsOpened(){return m_port.IsOpened();}

	/**
	* @brief �ж��Ƿ����Modem�豸
	* @return �Ƿ����Modem�豸
	*/
	BOOL IsReady()
	{
		if (!WriteToPort(_T("AT\r\n"),TRUE,TRUE)) return FALSE;
		if (WaitForResponse(NULL)==MODEMSTATUS_OK) return TRUE;
		return FALSE;
	}

	/**
	* @brief ������modem
	* @return �Ƿ�ɹ�
	*/
	BOOL Reset()
	{
		if (!WriteToPort(_T("ATZ\r\n"),TRUE,TRUE)) return FALSE;
		Sleep(3000);
		if (WaitForResponse(NULL)==MODEMSTATUS_OK) return TRUE;
		return FALSE;
	}

	/**
	* @brief ���û��Կ���/�ر�
	* @return �Ƿ�ɹ�
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
	* @brief �����Ƿ��Զ�ʡ��ģʽ
	* @param bAutoShutDown		�Ƿ������Զ�ʡ��ģʽ��Ĭ��Ϊ��������������ʱ��Ҫ����ÿ��nTimerValʱ���modem���м���
	* @param nTimerVal			����modem�Ķ�ʱʱ��
	* @return �Ƿ�ɹ�
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
		//��ֹmodem����
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
		//�ж��Ƿ�ΪModem������������Ϣ
		unsigned int chkret=OnCheckComingMsg(nLastState,buf);
		if (chkret>0)
		{
			//˵����Ϣ����Modem���������ģ��Ȳ���Ӧԭ�ȴ�
			return chkret;
		}
		if (nLastState!=MODEMSTATUS_NONE)
		{
			if (m_nTimeout==0)
			{
				//�첽ģʽ���ص���Ӧ����
				if (m_handler) m_handler->OnResponseRequest(buf,nLastState);
			}
			else
			{
				//ͬ��ģʽ��֪ͨ�ȴ���
				m_curResp=buf;
				m_nLastState=nLastState;
				m_evResp.Set();
				//ͬ��ģʽ�£������һ�ε���Ӧ��û�д����ȴ��������ȣ�ֱ���ȴ�����ʱ
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
	//�ȴ���Ӧ��ͬ��ģʽ��ʹ�ã������ǰΪ�첽ģʽ�����سɹ�MODEMSTATUS_OK
	int WaitForResponse(THString *ret)
	{
		//�����ǰΪ�첽ģʽ��ֱ�ӷ��سɹ�MODEMSTATUS_OK
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

//��֪ͨTE��������Ĭ���ڴ�λ�ã�AT+CNMI=0,0
#define GSMMODEM_NOTIFY_MODE_NONETE_SAVEDEF		0
//�ȴ������߿�����֪ͨ��������Ĭ���ڴ�λ�ã�AT+CNMI=2,1
#define GSMMODEM_NOTIFY_MODE_IDLETE_SAVEDEF		1
//�ȴ������߿�����֪ͨ��������SIM���򲻱��棬AT+CNMI=2,2
#define GSMMODEM_NOTIFY_MODE_IDLETE_SAVESIMORNOT	2

// �û���Ϣ���뷽ʽ
#define GSMMODEM_ENCODEING_7BIT		0
#define GSMMODEM_ENCODEING_8BIT		4
#define GSMMODEM_ENCODEING_UCS2		8

//�绰��
#define PHONEBOOK_SIM				_T("SM")
#define PHONEBOOK_PHONE				_T("ME")
#define PHONEBOOK_BOTH				_T("MT")
#define PHONEBOOK_FIXDIAL			_T("FD")
#define PHONEBOOK_LASTDIAL			_T("LD")
#define PHONEBOOK_TA				_T("TA")

class ITHGsmModemEventHandler : public ITHModemEventHandler
{
public:
	//������Ȼ�Ѿ�����Ϣ������Ӧ�����ǻ�����֮��Ķ����Żص������������ӦӦ����Post��Ϣ���ⲿ����
	virtual void OnComingSMS(int idx)=0;
};

/**
* @brief �ֻ�Modem��װ��
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-08-11 �½���
*/
/**<pre>
�÷���
	//ͬ��ģʽ
	class MyCls: public ITHGsmModemEventHandler
	{
	public:
		//�첽ģʽ��ʹ��
		virtual void OnResponseRequest(THString str,int status){}
		//��Modem������֪ͨ���д���ͬ�����첽ģʽ���ɴ���
		virtual void OnComingMessage(THString str,int status);
		//���ڴ����¼�����ͬ�����첽ģʽ���ɴ���
		virtual void OnHandleError(int nErrorType,int nPortNum);
		//������Ȼ�Ѿ�����Ϣ������Ӧ�����ǻ�����֮��Ķ����Żص������������ӦӦ����Post��Ϣ���ⲿ����
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
			gsm.SendSMS(_T("13928557836"),_T("���Բ���"));//,_T("13800757500"));
		}
		else
			THDebug(_T("Not Have Modem Here!"));
		//gsm.CloseModem();
	}
	//�첽ģʽ
	class MyCls: public ITHGsmModemEventHandler
	{
	public:
		//����Modem���������󣬽��лص������������첽ģʽ
		virtual void OnResponseRequest(THString str,int status)
		{
			if (status==MODEMSTATUS_OK)
				...
			//�����ﷵ�ػ�ö��ŵ�pdu���룬Ȼ�����THGsmModem::DecodeMsg()�������Ϣ
		}
		//��Modem������֪ͨ���д���ͬ�����첽ģʽ���ɴ���
		virtual void OnComingMessage(THString str,int status);
		//���ڴ����¼�����ͬ�����첽ģʽ���ɴ���
		virtual void OnHandleError(int nErrorType,int nPortNum);
		//�첽ģʽ�£��ú�������ֱ�ӵ��ö��Ŷ�ȡ
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
	* @brief ����ʹ��PDUģʽ��FALSEʹ��TEXT��ʽ
	* @return �Ƿ�ɹ�
	*/
	BOOL SetPduMode(BOOL bPdu=TRUE)
	{
		// PDUģʽ
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
	* @brief �����յ����ŵ�֪ͨ��ʽ
	* @return �Ƿ�ɹ�
	*/
	/**
		������Ϣ�ࣺ
			class 0: ֻ��ʾ��������
			class 1: ������ME�ڴ���
			class 2: ������SIM����
			class 3: ֱ�Ӵ��䵽TE
		AT+CNMI=[<mode>[,<mt>[,<bm>[,<ds>[,<bfr>]]]]]
			mode ֪ͨ��ʽ��
				0- ��֪ͨTE��
				1- ֻ�������߿�������£�֪ͨTE������֪ͨTE��
				2- ֪ͨTE����������ռ������£��Ȼ����������������߿���ʱ��֪ͨ��
				3- ֪ͨTE����������ռ������£�֪ͨ�����������һ���䡣
			mt ��Ϣ���淽ʽ��
				0- ���浽Ĭ���ڴ�λ�ã�����class 3����
				1- ���浽Ĭ���ڴ�λ�ã�������TE����֪ͨ������class 3����
				2- ����class2�����浽SIM������TE����֪ͨ������class��ֱ�ӽ���Ϣת����TE��
				3- ����class3��ֱ�ӽ���Ϣת����TE����������class��ͬmt=1��
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
		int nPduLength;		// PDU������
		unsigned char nSmscLength;	// SMSC������
		char cmd[16];		// ���
		char pdu[512];		// PDU��

		SM_PARAM Src;
		memset(&Src, 0, sizeof(SM_PARAM));
		// ȥ������ǰ��"+"
		if(smsc.GetAt(0)=='+') smsc=smsc.Mid(1);
		if(TargetNo.GetAt(0)=='+') TargetNo = TargetNo.Mid(1);
		// �ں���ǰ��"86"
		if(!smsc.IsEmpty() && smsc.Left(2) != _T("86")) smsc = _T("86") + smsc;
		//Ŀ����벻��ǿ��86������86Ϊ91����ģʽ��û��86Ϊ81����ģʽ
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
		// ������Ϣ�ṹ
		Src.index=0;
		strcpy(Src.SCA,strSmsc);
		strcpy(Src.TPA,strNumber);
		strcpy(Src.TP_UD,strContent);
		Src.TP_PID = 0;
		Src.TP_DCS = GSMMODEM_ENCODEING_UCS2;
		if (strSmsc) THCharset::free(strSmsc);
		if (strNumber) THCharset::free(strNumber);
		if (strContent) THCharset::free(strContent);

		nPduLength = gsmEncodePdu(&Src, pdu);	// ����PDU����������PDU��
		strcat(pdu,"\x1a");		// ��Ctrl-Z����
		gsmString2Bytes(pdu, &nSmscLength, 2);	// ȡPDU���е�SMSC��Ϣ����
		nSmscLength++;		// ���ϳ����ֽڱ���
		// �����еĳ��ȣ�������SMSC��Ϣ���ȣ��������ֽڼ�
		sprintf(cmd, "AT+CMGS=%d\r", nPduLength / 2 - nSmscLength);	// ��������
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
		for(i=0;i<15;i++)//��� 22.5 sec
		{
			ret=WaitForResponse(NULL);
			if (ret==MODEMSTATUS_ERROR) return FALSE;
			else if (ret==MODEMSTATUS_OK)
				break;
		}
		if (ret==MODEMSTATUS_OK) return TRUE;
		//���ܷ��������ˣ�Ϊ�˱�֤modem�������·���һ��"\x1a"
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
	* @brief ͨ�����ŵ�PDUԭ�Ľ������
	* @param buffer		pdu����Ķ���
	* @param sender		���ŷ�����
	* @param msg		��Ϣ����
	* @param ti			����ʱ��
	* @return �Ƿ�ɹ�
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
	* @brief ɾ������Ϣ
	* @param index		����Ϣ���1-255
	* @return �Ƿ�ɹ�
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
		//����Modem�������͹�������Ϣ
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
			//handler�����Լ�
			if (m_handler)
				m_handler->OnComingMessage(str,nStatus);
			return pos2;
		}
		return 0;
	}

	// ����Ϣ�����ṹ������/���빲��
	// ���У��ַ�����'\0'��β
	typedef struct {
		char SCA[16];			// ����Ϣ�������ĺ���(SMSC��ַ)
		char TPA[16];			// Ŀ������ظ�����(TP-DA��TP-RA)
		char TP_PID;			// �û���ϢЭ���ʶ(TP-PID),00:GSM ��Ե�
		char TP_DCS;			// �û���Ϣ���뷽ʽ(TP-DCS),GSMMODEM_ENCODEING_XXX
		char TP_SCTS[16];		// ����ʱ����ַ���(TP_SCTS),����ʱ�õ�07081022105850 = 07-08-10 22:10:58
		char TP_UD[160];		// ԭʼ�û���Ϣ(����ǰ�������TP-UD)
		short index;			// ����Ϣ��ţ��ڶ�ȡʱ�õ�
	} SM_PARAM;

	// �ֽ�����ת��Ϊ�ɴ�ӡ�ַ���
	// �磺{0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01} --> "C8329BFD0E01" 
	// ����: pSrc - Դ����ָ��
	//       nSrcLength - Դ���ݳ���
	// ���: pDst - Ŀ���ַ���ָ��
	// ����: Ŀ���ַ�������
	int gsmBytes2String(const unsigned char* pSrc, char* pDst, int nSrcLength)
	{
		const char tab[]="0123456789ABCDEF";	// 0x0-0xf���ַ����ұ�
		for (int i = 0; i < nSrcLength; i++)
		{
			*pDst++ = tab[*pSrc >> 4];		// �����4λ
			*pDst++ = tab[*pSrc & 0x0f];	// �����4λ
			pSrc++;
		}
		// ����ַ����Ӹ�������
		*pDst = '\0';
		// ����Ŀ���ַ�������
		return (nSrcLength * 2);
	}

	// �ɴ�ӡ�ַ���ת��Ϊ�ֽ�����
	// �磺"C8329BFD0E01" --> {0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01}
	// ����: pSrc - Դ�ַ���ָ��
	//       nSrcLength - Դ�ַ�������
	// ���: pDst - Ŀ������ָ��
	// ����: Ŀ�����ݳ���
	int gsmString2Bytes(const char* pSrc, unsigned char* pDst, int nSrcLength)
	{
		for (int i = 0; i < nSrcLength; i += 2)
		{
			// �����4λ
			if ((*pSrc >= '0') && (*pSrc <= '9'))
				*pDst = (*pSrc - '0') << 4;
			else
				*pDst = (*pSrc - 'A' + 10) << 4;
			pSrc++;
			// �����4λ
			if ((*pSrc>='0') && (*pSrc<='9'))
				*pDst |= *pSrc - '0';
			else
				*pDst |= *pSrc - 'A' + 10;
			pSrc++;
			pDst++;
		}
		// ����Ŀ�����ݳ���
		return (nSrcLength / 2);
	}

	// 7bit����
	// ����: pSrc - Դ�ַ���ָ��
	//       nSrcLength - Դ�ַ�������
	// ���: pDst - Ŀ����봮ָ��
	// ����: Ŀ����봮����
	int gsmEncode7bit(const char* pSrc, unsigned char* pDst, int nSrcLength)
	{
		int nSrc;		// Դ�ַ����ļ���ֵ
		int nDst;		// Ŀ����봮�ļ���ֵ
		int nChar;		// ��ǰ���ڴ���������ַ��ֽڵ���ţ���Χ��0-7
		unsigned char nLeft;	// ��һ�ֽڲ��������

		// ����ֵ��ʼ��
		nSrc = 0;
		nDst = 0;

		// ��Դ��ÿ8���ֽڷ�Ϊһ�飬ѹ����7���ֽ�
		// ѭ���ô�����̣�ֱ��Դ����������
		// ������鲻��8�ֽڣ�Ҳ����ȷ����
		while (nSrc < nSrcLength)
		{
			// ȡԴ�ַ����ļ���ֵ�����3λ
			nChar = nSrc & 7;

			// ����Դ����ÿ���ֽ�
			if(nChar == 0)
			{
				// ���ڵ�һ���ֽڣ�ֻ�Ǳ�����������������һ���ֽ�ʱʹ��
				nLeft = *pSrc;
			}
			else
			{
				// ���������ֽڣ������ұ߲��������������ӣ��õ�һ��Ŀ������ֽ�
				*pDst = (*pSrc << (8-nChar)) | nLeft;

				// �����ֽ�ʣ�µ���߲��֣���Ϊ�������ݱ�������
				nLeft = *pSrc >> nChar;

				// �޸�Ŀ�괮��ָ��ͼ���ֵ
				pDst++;
				nDst++;
			}

			// �޸�Դ����ָ��ͼ���ֵ
			pSrc++;
			nSrc++;
		}

		// ����Ŀ�괮����
		return nDst;
	}

	// 7bit����
	// ����: pSrc - Դ���봮ָ��
	//       nSrcLength - Դ���봮����
	// ���: pDst - Ŀ���ַ���ָ��
	// ����: Ŀ���ַ�������
	int gsmDecode7bit(const unsigned char* pSrc, char* pDst, int nSrcLength)
	{
		int nSrc;		// Դ�ַ����ļ���ֵ
		int nDst;		// Ŀ����봮�ļ���ֵ
		int nByte;		// ��ǰ���ڴ���������ֽڵ���ţ���Χ��0-6
		unsigned char nLeft;	// ��һ�ֽڲ��������

		// ����ֵ��ʼ��
		nSrc = 0;
		nDst = 0;
		
		// �����ֽ���źͲ������ݳ�ʼ��
		nByte = 0;
		nLeft = 0;

		// ��Դ����ÿ7���ֽڷ�Ϊһ�飬��ѹ����8���ֽ�
		// ѭ���ô�����̣�ֱ��Դ���ݱ�������
		// ������鲻��7�ֽڣ�Ҳ����ȷ����
		while(nSrc<nSrcLength)
		{
			// ��Դ�ֽ��ұ߲��������������ӣ�ȥ�����λ���õ�һ��Ŀ������ֽ�
			*pDst = ((*pSrc << nByte) | nLeft) & 0x7f;

			// �����ֽ�ʣ�µ���߲��֣���Ϊ�������ݱ�������
			nLeft = *pSrc >> (7-nByte);

			// �޸�Ŀ�괮��ָ��ͼ���ֵ
			pDst++;
			nDst++;

			// �޸��ֽڼ���ֵ
			nByte++;

			// ����һ������һ���ֽ�
			if(nByte == 7)
			{
				// ����õ�һ��Ŀ������ֽ�
				*pDst = nLeft;

				// �޸�Ŀ�괮��ָ��ͼ���ֵ
				pDst++;
				nDst++;

				// �����ֽ���źͲ������ݳ�ʼ��
				nByte = 0;
				nLeft = 0;
			}

			// �޸�Դ����ָ��ͼ���ֵ
			pSrc++;
			nSrc++;
		}

		// ����ַ����Ӹ�������
		*pDst = '\0';

		// ����Ŀ�괮����
		return nDst;
	}
	// 8bit����
	// ����: pSrc - Դ�ַ���ָ��
	//       nSrcLength - Դ�ַ�������
	// ���: pDst - Ŀ����봮ָ��
	// ����: Ŀ����봮����
	int gsmEncode8bit(const char* pSrc, unsigned char* pDst, int nSrcLength)
	{
		// �򵥸���
		memcpy(pDst, pSrc, nSrcLength);
		return nSrcLength;
	}
	// 8bit����
	// ����: pSrc - Դ���봮ָ��
	//       nSrcLength -  Դ���봮����
	// ���: pDst -  Ŀ���ַ���ָ��
	// ����: Ŀ���ַ�������
	int gsmDecode8bit(const unsigned char* pSrc, char* pDst, int nSrcLength)
	{
		// �򵥸���
		memcpy(pDst, pSrc, nSrcLength);
		// ����ַ����Ӹ�������
		*pDst = '\0';
		return nSrcLength;
	}
	// UCS2����
	// ����: pSrc - Դ�ַ���ָ��
	//       nSrcLength - Դ�ַ�������
	// ���: pDst - Ŀ����봮ָ��
	// ����: Ŀ����봮����
	int gsmEncodeUcs2(const char* pSrc, unsigned char* pDst, int nSrcLength)
	{
		int nDstLength;		// UNICODE���ַ���Ŀ
		WCHAR wchar[128];	// UNICODE��������
		// �ַ���-->UNICODE��
		nDstLength = MultiByteToWideChar(CP_ACP, 0, pSrc, nSrcLength, wchar, 128);
		// �ߵ��ֽڶԵ������
		for(int i=0; i<nDstLength; i++)
		{
			*pDst++ = wchar[i] >> 8;		// �������λ�ֽ�
			*pDst++ = wchar[i] & 0xff;		// �������λ�ֽ�
		}
		// ����Ŀ����봮����
		return nDstLength * 2;
	}
	// UCS2����
	// ����: pSrc - Դ���봮ָ��
	//       nSrcLength -  Դ���봮����
	// ���: pDst -  Ŀ���ַ���ָ��
	// ����: Ŀ���ַ�������
	int gsmDecodeUcs2(const unsigned char* pSrc, char* pDst, int nSrcLength)
	{
		int nDstLength;		// UNICODE���ַ���Ŀ
		WCHAR wchar[128];	// UNICODE��������
		// �ߵ��ֽڶԵ���ƴ��UNICODE
		for(int i=0; i<nSrcLength/2; i++)
		{
			wchar[i] = *pSrc++ << 8;	// �ȸ�λ�ֽ�
			wchar[i] |= *pSrc++;		// ���λ�ֽ�
		}
		// UNICODE��-->�ַ���
		nDstLength = WideCharToMultiByte(CP_ACP, 0, wchar, nSrcLength/2, pDst, 160, NULL, NULL);
		// ����ַ����Ӹ�������
		pDst[nDstLength] = '\0';
		// ����Ŀ���ַ�������
		return nDstLength;
	}
	// ����˳����ַ���ת��Ϊ�����ߵ����ַ�����������Ϊ��������'F'�ճ�ż��
	// �磺"8613851872468" --> "683158812764F8"
	// ����: pSrc - Դ�ַ���ָ��
	//       nSrcLength - Դ�ַ�������
	// ���: pDst - Ŀ���ַ���ָ��
	// ����: Ŀ���ַ�������
	int gsmInvertNumbers(const char* pSrc, char* pDst, int nSrcLength)
	{
		int nDstLength;		// Ŀ���ַ�������
		char ch;			// ���ڱ���һ���ַ�
		// ���ƴ�����
		nDstLength = nSrcLength;
		// �����ߵ�
		for(int i=0; i<nSrcLength;i+=2)
		{
			ch = *pSrc++;		// �����ȳ��ֵ��ַ�
			*pDst++ = *pSrc++;	// ���ƺ���ֵ��ַ�
			*pDst++ = ch;		// �����ȳ��ֵ��ַ�
		}
		// Դ��������������
		if(nSrcLength & 1)
		{
			*(pDst-2) = 'F';	// ��'F'
			nDstLength++;		// Ŀ�괮���ȼ�1
		}
		// ����ַ����Ӹ�������
		*pDst = '\0';
		// ����Ŀ���ַ�������
		return nDstLength;
	}
	// �����ߵ����ַ���ת��Ϊ����˳����ַ���
	// �磺"683158812764F8" --> "8613851872468"
	// ����: pSrc - Դ�ַ���ָ��
	//       nSrcLength - Դ�ַ�������
	// ���: pDst - Ŀ���ַ���ָ��
	// ����: Ŀ���ַ�������
	int gsmSerializeNumbers(const char* pSrc, char* pDst, int nSrcLength)
	{
		int nDstLength;		// Ŀ���ַ�������
		char ch;			// ���ڱ���һ���ַ�

		// ���ƴ�����
		nDstLength = nSrcLength;

		// �����ߵ�
		for(int i=0; i<nSrcLength;i+=2)
		{
			ch = *pSrc++;		// �����ȳ��ֵ��ַ�
			*pDst++ = *pSrc++;	// ���ƺ���ֵ��ַ�
			*pDst++ = ch;		// �����ȳ��ֵ��ַ�
		}

		// �����ַ���'F'��
		if(*(pDst-1) == 'F')
		{
			pDst--;
			nDstLength--;		// Ŀ���ַ������ȼ�1
		}

		// ����ַ����Ӹ�������
		*pDst = '\0';

		// ����Ŀ���ַ�������
		return nDstLength;
	}
	// PDU���룬���ڱ��ơ����Ͷ���Ϣ
	// ����: pSrc - ԴPDU����ָ��
	// ���: pDst - Ŀ��PDU��ָ��
	// ����: Ŀ��PDU������
	int gsmEncodePdu(const SM_PARAM* pSrc, char* pDst)
	{
		int nLength;			// �ڲ��õĴ�����
		int nDstLength;			// Ŀ��PDU������
		unsigned char buf[256];	// �ڲ��õĻ�����
		// SMSC��ַ��Ϣ��
		nLength = (int)strlen(pSrc->SCA);	// SMSC��ַ�ַ����ĳ���
		// ��SMSCģʽ
		if (nLength==0)
		{
			buf[0]=0;
			nDstLength = gsmBytes2String(buf, pDst, 1);		// ת��2���ֽڵ�Ŀ��PDU��
		}
		else
		{
			buf[0] = (char)((nLength & 1) == 0 ? nLength : nLength + 1) / 2 + 1;	// SMSC��ַ��Ϣ����
			buf[1] = 0x91;		// �̶�: �ù��ʸ�ʽ����
			nDstLength = gsmBytes2String(buf, pDst, 2);		// ת��2���ֽڵ�Ŀ��PDU��
			nDstLength += gsmInvertNumbers(pSrc->SCA, &pDst[nDstLength], nLength);	// ת��SMSC���뵽Ŀ��PDU��
		}
		// TPDU�λ���������Ŀ���ַ��
		nLength = (int)strlen(pSrc->TPA);	// TP-DA��ַ�ַ����ĳ���
		buf[0] = 0x11;					// �Ƿ��Ͷ���(TP-MTI=01)��TP-VP����Ը�ʽ(TP-VPF=10)
		buf[1] = 0;						// TP-MR=0
		buf[2] = (char)nLength;			// Ŀ���ַ���ָ���(TP-DA��ַ�ַ�����ʵ����)
		if (strncmp(pSrc->TPA,"86",2)==0)
			buf[3] = 0x91;				//�ù��ʸ�ʽ����
		else
			buf[3] = 0x81;				//�ñ��ظ�ʽ����
		nDstLength += gsmBytes2String(buf, &pDst[nDstLength], 4);		// ת��4���ֽڵ�Ŀ��PDU��
		nDstLength += gsmInvertNumbers(pSrc->TPA, &pDst[nDstLength], nLength);	// ת��TP-DA��Ŀ��PDU��
		// TPDU��Э���ʶ�����뷽ʽ���û���Ϣ��
		nLength = (int)strlen(pSrc->TP_UD);	// �û���Ϣ�ַ����ĳ���
		buf[0] = pSrc->TP_PID;			// Э���ʶ(TP-PID)
		buf[1] = pSrc->TP_DCS;			// �û���Ϣ���뷽ʽ(TP-DCS)
		buf[2] = 0;						// ��Ч��(TP-VP)Ϊ5����
		if(pSrc->TP_DCS == GSMMODEM_ENCODEING_7BIT)	
		{
			// 7-bit���뷽ʽ
			buf[3] = nLength;			// ����ǰ����
			nLength = gsmEncode7bit(pSrc->TP_UD, &buf[4], nLength+1) + 4;	// ת��TP-DA��Ŀ��PDU��
		}
		else if(pSrc->TP_DCS == GSMMODEM_ENCODEING_UCS2)
		{
			// UCS2���뷽ʽ
			buf[3] = gsmEncodeUcs2(pSrc->TP_UD, &buf[4], nLength);	// ת��TP-DA��Ŀ��PDU��
			nLength = buf[3] + 4;		// nLength���ڸö����ݳ���
		}
		else
		{
			// 8-bit���뷽ʽ
			buf[3] = gsmEncode8bit(pSrc->TP_UD, &buf[4], nLength);	// ת��TP-DA��Ŀ��PDU��
			nLength = buf[3] + 4;		// nLength���ڸö����ݳ���
		}
		nDstLength += gsmBytes2String(buf, &pDst[nDstLength], nLength);		// ת���ö����ݵ�Ŀ��PDU��
		// ����Ŀ���ַ�������
		return nDstLength;
	}
	// PDU���룬���ڽ��ա��Ķ�����Ϣ
	// ����: pSrc - ԴPDU��ָ��
	// ���: pDst - Ŀ��PDU����ָ��
	// ����: �û���Ϣ������
	int gsmDecodePdu(const char* pSrc, SM_PARAM* pDst)
	{
		int nDstLength;			// Ŀ��PDU������
		unsigned char tmp;		// �ڲ��õ���ʱ�ֽڱ���
		unsigned char buf[256];	// �ڲ��õĻ�����
		// SMSC��ַ��Ϣ��
		gsmString2Bytes(pSrc, &tmp, 2);	// ȡ����
		tmp = (tmp - 1) * 2;	// SMSC���봮����
		pSrc += 4;			// ָ����ƣ�������SMSC��ַ��ʽ
		gsmSerializeNumbers(pSrc, pDst->SCA, tmp);	// ת��SMSC���뵽Ŀ��PDU��
		pSrc += tmp;		// ָ�����

		// TPDU�λ�������
		gsmString2Bytes(pSrc, &tmp, 2);	// ȡ��������
		pSrc += 2;		// ָ�����

		// ȡ�ظ�����
		gsmString2Bytes(pSrc, &tmp, 2);	// ȡ����
		if(tmp & 1) tmp += 1;	// ������ż��
		pSrc += 4;			// ָ����ƣ������˻ظ���ַ(TP-RA)��ʽ
		gsmSerializeNumbers(pSrc, pDst->TPA, tmp);	// ȡTP-RA����
		pSrc += tmp;		// ָ�����

		// TPDU��Э���ʶ�����뷽ʽ���û���Ϣ��
		gsmString2Bytes(pSrc, (unsigned char*)&pDst->TP_PID, 2);	// ȡЭ���ʶ(TP-PID)
		pSrc += 2;		// ָ�����
		gsmString2Bytes(pSrc, (unsigned char*)&pDst->TP_DCS, 2);	// ȡ���뷽ʽ(TP-DCS)
		pSrc += 2;		// ָ�����
		gsmSerializeNumbers(pSrc, pDst->TP_SCTS, 14);		// ����ʱ����ַ���(TP_SCTS) 
		pSrc += 14;		// ָ�����
		gsmString2Bytes(pSrc, &tmp, 2);	// �û���Ϣ����(TP-UDL)
		pSrc += 2;		// ָ�����
		if(pDst->TP_DCS == GSMMODEM_ENCODEING_7BIT)	
		{
			// 7-bit����
			nDstLength = gsmString2Bytes(pSrc, buf, tmp & 7 ? (int)tmp * 7 / 4 + 2 : (int)tmp * 7 / 4);	// ��ʽת��
			gsmDecode7bit(buf, pDst->TP_UD, nDstLength);	// ת����TP-DU
			nDstLength = tmp;
		}
		else if(pDst->TP_DCS == GSMMODEM_ENCODEING_UCS2)
		{
			// UCS2����
			nDstLength = gsmString2Bytes(pSrc, buf, tmp * 2);			// ��ʽת��
			nDstLength = gsmDecodeUcs2(buf, pDst->TP_UD, nDstLength);	// ת����TP-DU
		}
		else
		{
			// 8-bit����
			nDstLength = gsmString2Bytes(pSrc, buf, tmp * 2);			// ��ʽת��
			nDstLength = gsmDecode8bit(buf, pDst->TP_UD, nDstLength);	// ת����TP-DU
		}

		// ����Ŀ���ַ�������
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
	//��Ӧ�����¼�������ʱ������FALSEΪȡ������
	virtual BOOL OnSMSCannotSendImmediate(const THString sendto,const THString msg,const THTime sti,const int retrycount){return TRUE;}
	//��Ӧ����Ϣ
	virtual void OnSMSComing(const int idx,const THString sender,const THString msg,const THTime ti)=0;
};

/**
* @brief �ֻ�Modem��װ��
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-08-11 �½���
*/
/**<pre>
�÷���
	ʵ�� THGsmModemMachineEventHandler mycls����Ӧ�յ���Ϣ
	m_machine.SetReceiveMessageHandler(mycls);
	if (m_machine.InitModem(1,9600))
	{
		THDebug(_T("machine start now!"));
		m_machine.SendSMS(_T("13925901884"),_T("���ǲ���"));
		m_machine.SendSMS(_T("13925901884"),_T("���Ǻó��Ĳ���3"));
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
	//������Ȼ�Ѿ�����Ϣ������Ӧ�����ǻ�����֮��Ķ����Żص������������ӦӦ����Post��Ϣ���ⲿ����
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
							//�����unlock��ʮ�ֺã�����unlock����Ϊ��OnSMSCannotSendImmediate���õ���Ӧ���п��ܼ����µĶ��ţ���ʱ��ᱻѭ������ס�ˣ�����ɾ��m_ls�Ĳ�����ֻ�б��߳�����������ԱȽϰ�ȫ��
							m_sm.Unlock();
							if (m_handler->OnSMSCannotSendImmediate(list->sender,list->msg,list->ti,count)==FALSE)
							{
								count=0;
								m_ls.RemoveHead();
								bRemove=TRUE;
							}
							m_sm.Lock();
						}
						//�����ȡ������,�ȴ�һ����ٷ�
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
						//֤�����Ż�û�ж�����
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
	//����Modem���������󣬽��лص������������첽ģʽ
	virtual void OnResponseRequest(THString str,int status){}
	//��Modem������֪ͨ���д���ͬ�����첽ģʽ���ɴ���
	virtual void OnComingMessage(THString str,int status){}
	//���ڴ����¼�����ͬ�����첽ģʽ���ɴ���
	virtual void OnHandleError(int nErrorType,int nPortNum){}
};