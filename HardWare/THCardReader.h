#pragma once

#include <THString.h>
#include <THDllLoader.h>
#include <THTime.h>
#include <THSystem.h>
#include <THThread.h>

extern "C"
{
typedef HANDLE (__stdcall *OpenCommPort)(int nPort,int nBaudRate);
typedef BOOL (__stdcall *ClosePortHandle)(HANDLE hPort);
typedef BOOL (__stdcall *CallClock)(HANDLE hPort,int nClockId);
typedef BOOL (__stdcall *UnCallClock)(HANDLE hPort);
typedef BOOL (__stdcall *ReadICCard)(HANDLE hPort,char* pCardNo,char *pCardName,int* nMoney,int* nTimes,int *nVer);
typedef BOOL (__stdcall *WriteICCard)(HANDLE hPort,char* pCardNo,char *pCardName,int nMoney,int nTimes,int nVer);
typedef BOOL (__stdcall *ReadICCardSerialNo)(HANDLE hPort,__int64 *buf,BOOL bLong);
}

#define CARDSTYLE_UNKNOWN	0
#define CARDSTYLE_IDCARD	1
#define CARDSTYLE_STDCARD	830
#define CARDSTYLE_NEWCARD	880
#define CARDSTYLE_690CARD	690
/**
* @brief EastRiver��������װ��
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-11-18 �½���
*/
/**<pre>
�÷���
	OpenPort/ClosePort ���ڴ�/�رմ���
	OpenClock/CloseClock ���ڴ�/�ر��豸,ע���豸�ڴ򿪺�,����ˢ���豸û����Ӧ,���ÿ�ζ���ҪOpenClock,��ȡ��ɺ��CloseClock
</pre>*/
class THERIcCardDll : public THDllLoader
{
public:
	THERIcCardDll():THDllLoader(_T("EastRiver.dll"))
	{
		m_hPort=INVALID_HANDLE_VALUE;
		m_bCallClock=FALSE;
		m_nClockid=0;
		EmptyDllPointer();
	}

	virtual ~THERIcCardDll(){FreeDll();}

	virtual BOOL OnFreeDll()
	{
		ClosePort();
		return THDllLoader::OnFreeDll();
	}

	virtual void EmptyDllPointer()
	{
		openport=NULL;
		closeport=NULL;
		callclock=NULL;
		uncallclock=NULL;
		readcard=NULL;
		writecard=NULL;
		cardserialno=NULL;
	}

	virtual BOOL InitDllPoinier()
	{
		openport=(OpenCommPort)GetProcAddress(m_module,"OpenCommPort");
		closeport=(ClosePortHandle)GetProcAddress(m_module,"ClosePortHandle");
		callclock=(CallClock)GetProcAddress(m_module,"CallClock");
		uncallclock=(UnCallClock)GetProcAddress(m_module,"UnCallClock");
		readcard=(ReadICCard)GetProcAddress(m_module,"ReadICCard");
		writecard=(WriteICCard)GetProcAddress(m_module,"WriteICCard");
		cardserialno=(ReadICCardSerialNo)GetProcAddress(m_module,"ReadICCardSerialNo");
		if (!openport || !closeport || !callclock || !uncallclock ||
			!readcard || !writecard || !cardserialno)
			return FALSE;
		return TRUE;
	}

	BOOL OpenPort(int nInt,int nClockid)
	{
		m_nClockid=nClockid;
		if (!ClosePort()) return FALSE;
		if (!openport) return FALSE;
		m_hPort=openport(nInt,9600);
		if (m_hPort==(HANDLE)0 || m_hPort==(HANDLE)-1)
		{
			m_hPort=INVALID_HANDLE_VALUE;
			return FALSE;
		}
		return TRUE;
	}

	BOOL OpenClock()
	{
		if (!CloseClock()) return FALSE;
		if (!callclock) return FALSE;
		m_bCallClock=TRUE;
		return callclock(m_hPort,m_nClockid);
	}

	BOOL CloseClock()
	{
		if (!InitDll()) return FALSE;
		if (m_hPort==INVALID_HANDLE_VALUE) return FALSE;
		if (!m_bCallClock) return TRUE;
		if (!uncallclock) return FALSE;
		m_bCallClock=FALSE;
		return uncallclock(m_hPort);
	}

	BOOL ClosePort()
	{
		if (!InitDll()) return FALSE;
		if (m_hPort==INVALID_HANDLE_VALUE) return TRUE;
		if (!CloseClock()) return FALSE;
		if (!closeport) return FALSE;
		if (!closeport(m_hPort)) return FALSE;
		m_hPort=INVALID_HANDLE_VALUE;
		return TRUE;
	}

	BOOL IsPortOpened(){return m_hPort!=INVALID_HANDLE_VALUE;}
	BOOL IsPortCalled(){return m_bCallClock;}

	BOOL ReadCard(THString *sCardNo,THString *sCardName=NULL,int* nMoney=NULL,int *nTimes=NULL,int *nVer=NULL)
	{
		if (!InitDll()) return FALSE;
		if (m_hPort==INVALID_HANDLE_VALUE) return FALSE;
		if (!readcard) return FALSE;
		if (!OpenClock()) return FALSE;
		char buf[1024];
		char buf1[1024];
		memset(buf,0,1024);
		memset(buf1,0,1024);
		int tmpMoney=0;
		int tmpTimes=0;
		int tmpVer=0;
		BOOL ret=readcard(m_hPort,buf,buf1,&tmpMoney,&tmpTimes,&tmpVer);
		if (!ret)
		{
			CloseClock();
			return FALSE;
		}
		if (sCardNo) *sCardNo=THCharset::a2t(buf);
		if (sCardName) *sCardName=THCharset::a2t(buf1);
		if (nMoney) *nMoney=tmpMoney;
		if (nTimes) *nTimes=tmpTimes;
		if (nVer) *nVer=tmpVer;
		if (!CloseClock()) return FALSE;
		return TRUE;
	}

	BOOL WriteCard(THString sCardNo,THString sCardName=_T(""),int nMoney=0,int nTimes=0,int nVer=CARDSTYLE_690CARD)
	{
		if (!InitDll()) return FALSE;
		if (m_hPort==INVALID_HANDLE_VALUE) return FALSE;
		if (!writecard) return FALSE;
		if (!OpenClock()) return FALSE;
		char *buf=THCharset::t2a(sCardNo);
		char *buf1=THCharset::t2a(sCardName);
		BOOL ret=FALSE;
		if (buf && buf1)
			ret=writecard(m_hPort,buf,buf1,nMoney,nTimes,nVer);
		if (buf) THCharset::free(buf);
		if (buf1) THCharset::free(buf1);
		if (!CloseClock()) return FALSE;
		return ret;
	}

	THString ReadCardSerialNo()
	{
		THString ret;
		if (!InitDll()) return ret;
		if (m_hPort==INVALID_HANDLE_VALUE) return ret;
		if (!cardserialno) return ret;
		if (!OpenClock()) return FALSE;
		__int64 buf;
		if (!cardserialno(m_hPort,&buf,TRUE))
		{
			CloseClock();
			return ret;
		}
		ret.Format(_T("%I64d"),buf);
		if (!CloseClock()) return FALSE;
		return ret;
	}
private:
	OpenCommPort openport;
	ClosePortHandle closeport;
	CallClock callclock;
	UnCallClock uncallclock;
	ReadICCard readcard;
	WriteICCard writecard;
	ReadICCardSerialNo cardserialno;
	HANDLE m_hPort;
	int m_nClockid;
	BOOL m_bCallClock;
};

extern "C"
{
	typedef int (__stdcall *MCOpenPort)(char *PortName,int *PortHandle);
	typedef int (__stdcall *MCSelectPort)(int PortHandle);
	typedef int (__stdcall *MCClosePort)();
	typedef int (__stdcall *MCReaderVersion)();
	typedef int (__stdcall *MCPowerOn)();
	typedef int (__stdcall *MCPowerOff)();
	typedef int (__stdcall *MCCardPresent)();
	typedef int (__stdcall *MCCardChanged)();
	typedef int (__stdcall *MCGetCardType)(char *ATRBuffer);
	//typedef char* (__stdcall *MCGetCardName)(int CardType);
	//typedef void (__stdcall *MCDes64)(unsigned char *input,unsigned char *output,unsigned char *key,int Encrypt);
	typedef int (__stdcall *MCOpenReader)(char *ComStr,HWND NotifyWindow,UINT NotifyMsg);
	typedef int (__stdcall *MCCloseReader)();
	//	SLE4442 
	typedef int (__stdcall *MCRead_4442_NO_PB)(int StartPos,int NOB,unsigned char *Bfr);
	typedef int (__stdcall *MCWrite_4442_Array)(int StartPos,int NOB,const unsigned char *Bfr,const unsigned char *PBSetFlag);
	typedef int (__stdcall *MCVerify_4442_PSC)(char PSC1,char PSC2,char PSC3);
	typedef int (__stdcall *MCWrite_4442_SM)(int SMAddress,char SMByte);
}

/**
* @brief MC900��������װ��
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-11-22 �½���
*/
/**<pre>
	SLE4442��Ƭ���
		256�ֽ�EEPROM
		32λ�����洢��
		3�ֽ��û����룬������������3�Ρ�
		����10��β�д
		����10�����ݱ�����
		����˶���ȷǰ��ȫ�����ݾ��ɶ����������Ҫ���ɶ����ݽ����ʵ����ܡ��˶�������ȷ���д����޸����ݡ�
		���ֽڵ��û����룬У��ͨ���������µ�һֱ��Ч��У��ͨ�������뱾��ɸ��ġ�ע�⣺�����������ַ������0��256�ֽ��ڡ�
		����������������ַ��1021��
		����������������ʼֵΪ3������˶Գ���1�Σ����1����������ֵΪ0�����Զ�����������ֻ�ɶ����������ٽ��и���Ҳ�޷��ٽ�������˶ԣ�����Ϊ��ʱ����һ������˶���ȷ���ɻָ�����ʼֵ��
		д��������ǰ32���ֽڣ���ÿһ�ֽڿɵ�������д����������д���������ݲ����ٸ��ģ����̻����ݣ���
		�ֽڵ�ַ0x15��0x1AΪ�û����룬SIEMENS��˾��Ϊ���û��ڴ�ר��һ��ȫ��Ψһ���룬�Ա��û�ͳһ����ʹ�ã����еĿ������󲻿����޸ģ���
		�ֽڵ�ַ0��7����ǰ���ɳ���д�ã����ɸ��ġ�
		��Ƭ�洢���ڵ���ʼ��ַ��0x20
		����������0xFFFFFF
		���Ȳ����Ƿ���SLE4442�������д�������У��˶����벢��ȡ���������Ȼ���д���벢�ض����Ƚ����β��������Ƿ���ͬ������ͬ�����ز�����ȷ��
</pre>*/
class THMC900IcCardReader : public THDllLoader,public THThread
{
public:
	THMC900IcCardReader():THDllLoader(_T("lx900.dll"))
	{
		m_buf[0]='\0';
		m_hComm=INVALID_HANDLE_VALUE;
		m_hWnd=NULL;
		m_Msg=0;
		EmptyDllPointer();
	}

	virtual ~THMC900IcCardReader(){FreeDll();}

	virtual BOOL OnFreeDll()
	{
		ClosePort();
		return THDllLoader::OnFreeDll();
	}

	virtual void EmptyDllPointer()
	{
		openport=NULL;
		selectport=NULL;
		closeport=NULL;
		poweron=NULL;
		poweroff=NULL;
		cardpresent=NULL;
		cardchanged=NULL;
		cardtype=NULL;
		openreader=NULL;
		closereader=NULL;
		readdata4442=NULL;
		writedata4442=NULL;
		verifyps4442=NULL;
		writesm4442=NULL;
		readerversion=NULL;
	}

	virtual BOOL InitDllPoinier()
	{
		openport=(MCOpenPort)GetProcAddress(m_module,"OpenPort");
		selectport=(MCSelectPort)GetProcAddress(m_module,"SelectPort");
		closeport=(MCClosePort)GetProcAddress(m_module,"ClosePort");
		poweron=(MCPowerOn)GetProcAddress(m_module,"PowerOn");
		poweroff=(MCPowerOff)GetProcAddress(m_module,"PowerOff");
		cardpresent=(MCCardPresent)GetProcAddress(m_module,"CardPresent");
		cardchanged=(MCCardChanged)GetProcAddress(m_module,"CardChanged");
		cardtype=(MCGetCardType)GetProcAddress(m_module,"GetCardType");
		openreader=(MCOpenReader)GetProcAddress(m_module,"OpenReader");
		closereader=(MCCloseReader)GetProcAddress(m_module,"CloseReader");
		readerversion=(MCReaderVersion)GetProcAddress(m_module,"ReaderVersion");
		readdata4442=(MCRead_4442_NO_PB)GetProcAddress(m_module,"Read_4442_NO_PB");
		writedata4442=(MCWrite_4442_Array)GetProcAddress(m_module,"Write_4442_Array");
		verifyps4442=(MCVerify_4442_PSC)GetProcAddress(m_module,"Verify_4442_PSC");
		writesm4442=(MCWrite_4442_SM)GetProcAddress(m_module,"Write_4442_SM");
		if (!openport || !selectport || !closeport || !poweron || !poweroff || !cardpresent ||
			!cardchanged || !cardtype || !openreader || !closereader || !readerversion || !readdata4442 || !writedata4442 ||
            !verifyps4442 || !writesm4442)
			return FALSE;
		return TRUE;
	}

	virtual void ThreadFunc(int ThreadId,void *data)
	{
		if (IsCardChanged())
		{
			if (m_hWnd && m_Msg)
				SendMessage(m_hWnd,m_Msg,IsCardPresent(),0);
		}
	}

	BOOL OpenPort(int nInt)
	{
		if (!ClosePort()) return FALSE;
		if (!openport) return FALSE;
		sprintf(m_buf,"COM%d",nInt);
		int ret;
		int fret=openport(m_buf,&ret);
		if (fret!=ERR_SUCCESS) return FALSE;
		m_hComm=(HANDLE)(INT_PTR)ret;
		if (m_hComm==(HANDLE)0 || m_hComm==(HANDLE)-1)
		{
			m_hComm=INVALID_HANDLE_VALUE;
			return FALSE;
		}
		if (!SetPower(TRUE)) return FALSE;
		return TRUE;
	}

	BOOL ClosePort()
	{
		if (!InitDll()) return FALSE;
		if (m_hComm==INVALID_HANDLE_VALUE) return TRUE;
		if (!SetNotify(FALSE)) return FALSE;
		if (!SetPower(FALSE)) return FALSE;
		if (!closeport) return FALSE;
		if (closeport()!=ERR_SUCCESS)
		{
			m_hComm=INVALID_HANDLE_VALUE;
			return FALSE;
		}
		m_hComm=INVALID_HANDLE_VALUE;
		return TRUE;
	}

	//�κβ���֮ǰ��Ӧʹ��SelectPort˵�����ڶԸö˿ڲ���
	BOOL SelectPort()
	{
		if (!InitDll()) return FALSE;
		if (!selectport) return FALSE;
		if (m_hComm==INVALID_HANDLE_VALUE) return FALSE;
		selectport((int)(INT_PTR)m_hComm);
		return TRUE;
	}

	BOOL SetPower(BOOL bOpen=TRUE)
	{
		if (!InitDll()) return FALSE;
		if (!poweroff || !poweron) return FALSE;
		if (bOpen)
			poweron();
		else
			poweroff();
		return TRUE;
	}

	BOOL IsPortOpened(){return m_hComm!=INVALID_HANDLE_VALUE;}

	BOOL IsCardPresent()
	{
		if (!InitDll()) return FALSE;
		if (!cardpresent) return FALSE;
		return cardpresent();
	}

	BOOL IsCardChanged()
	{
		if (!InitDll()) return FALSE;
		if (!cardchanged) return FALSE;
		return cardchanged();
	}

	int GetCardType()
	{
		if (!InitDll()) return CT_UNKNOWN;
		if (!cardtype) return CT_UNKNOWN;
		char buf[5];
		return cardtype(buf);
	}

	//�����Ƿ���Ӵ���
	BOOL SetNotify(BOOL bNotify,HWND hNotifyWindow=NULL,UINT nMsg=0)
	{
		if (!InitDll()) return FALSE;
		if (m_hComm==INVALID_HANDLE_VALUE) return FALSE;
		m_hWnd=hNotifyWindow;
		m_Msg=nMsg;
		if (bNotify)
			StartThread(1,NULL,500,TRUE);
		else
			KillThread(1);
		return TRUE;
	}

	int ReaderVersion()
	{
		if (!InitDll()) return 0;
		if (!readerversion) return 0;
		return readerversion();
	}

	//��Ƭһ�ζ�д�������ݳ����Ͻ�����16�ֽ�
	BOOL ReadData4442(int nStartPos,int nLen,unsigned char *buf)
	{
		if (!InitDll()) return FALSE;
		if (!readdata4442) return FALSE;
		return (readdata4442(nStartPos,nLen,buf)==ERR_SUCCESS);
	}

	//��Ƭһ�ζ�д�������ݳ����Ͻ�����16�ֽ�
	BOOL WriteData4442(int nStartPos,int nLen,const unsigned char *buf,const unsigned char *protect)
	{
		if (!InitDll()) return FALSE;
		if (!writedata4442) return FALSE;
		return (writedata4442(nStartPos,nLen,buf,protect)==ERR_SUCCESS);
	}

	BOOL VerifyPs4442(unsigned char psc1,unsigned char psc2,unsigned char psc3)
	{
		if (!InitDll()) return FALSE;
		if (!verifyps4442) return FALSE;
		return (verifyps4442(psc1,psc2,psc3)==ERR_SUCCESS);
	}

	BOOL WriteSm4442(int nAddr,unsigned char val)
	{
		if (!InitDll()) return FALSE;
		if (!writesm4442) return FALSE;
		return (writesm4442(nAddr,val)==ERR_SUCCESS);
	}

	/**
	* @brief ��ȡ��Ƭ���кţ���4442����û�����кŵģ���˺���ʵ�ַ���д�����кţ����̲��վ���˵��
	* @return �������кţ�û�п�����󷵻ؿ�
	*/
	/**<pre>
	</pre>*/
	THString ReadCardSerialNo4442()
	{
		THString ret;
		if (SelectPort() && IsCardPresent())
		{
			if (GetCardType()==CT_SLE4442)
			{
				unsigned char buf[16];
				BOOL bRead=FALSE;
				if (ReadData4442(0x20,16,buf))
				{
					if (memcmp(buf,"THMC",4)==0)
					{
						//�Ѿ��п���
						bRead=TRUE;
					}
					else
					{
						//�¿�
						if (VerifyPs4442(0xff,0xff,0xff))
						{
							//write password
							WriteSm4442(1,0xec);
							WriteSm4442(2,0xda);
							WriteSm4442(3,0xc1);
							memcpy(buf,"THMC",4);
							THTime ti;
							int nti=(int)ti.GetTimeAsTime64();
							memcpy(&buf[8],&nti,4);
							THSysMisc::srand();
							nti=rand()*rand();memcpy(&buf[4],&nti,4);
							nti=rand()*rand();memcpy(&buf[12],&nti,4);
							//write data
							if (WriteData4442(0x20,16,buf,(const unsigned char *)"0000000000000000"))
								bRead=TRUE;
						}
					}
					if (bRead)
					{
						unsigned int *tmp=(unsigned int *)&buf[4];
						ret.Format(_T("%04x%04x%04x"),*tmp,*(tmp+1),*(tmp+2));
					}
				}
			}
		}
		return ret;
	}

	enum myCardType
	{
		CT_UNKNOWN,
		CT_SLE4432,CT_SLE4442,
		CT_SLE4418,CT_SLE4428,
		CT_SLE4406,CT_SLE4436,
		CT_AT88SC101,CT_AT88SC102,
		CT_SLE44C42S,CT_SLE44C80,
		CT_X76F640,
		CT_AT24C01A,CT_AT24C02,CT_AT24C04,CT_AT24C08,CT_AT24C16,
		CT_AT24C32,CT_AT24C64,CT_AT24C128,CT_AT24C256,CT_AT24C512,
		CT_AT88SC1604,CT_AT88SC1608,CT_AT88SC153,
		CT_AT45D041,
		CT_NO_CARD
	};

	enum myErrorCode
	{
		ERR_SUCCESS = 0,
		ERR_NO_CARD,ERR_TIMEOUT,ERR_COUNTER,
		ERR_PSC1,ERR_PSC2,ERR_LENGTH,
		ERR_NOT_ENOUGH_MEMORY,
		ERR_CREATE_PORT_FILE,
		ERR_GET_COMM_STATE,ERR_SET_COMM_STATE,
		ERR_SET_COMM_MASK,ERR_SET_COMM_TIMEOUTS,
		ERR_CREATE_THREAD,ERR_NULL_PORT_HANDLE,
		RET_COMP_EQUAL = 200,RET_COMP_UNEQUAL
	};
private:
	MCOpenPort openport;
	MCSelectPort selectport;
	MCClosePort closeport;
	MCPowerOn poweron;
	MCPowerOff poweroff;
	MCCardPresent cardpresent;
	MCCardChanged cardchanged;
	MCGetCardType cardtype;
	MCOpenReader openreader;
	MCCloseReader closereader;
	MCReaderVersion readerversion;
	MCRead_4442_NO_PB readdata4442;
	MCWrite_4442_Array writedata4442;
	MCVerify_4442_PSC verifyps4442;
	MCWrite_4442_SM writesm4442;
	HANDLE m_hComm;
	char m_buf[10];
	HWND m_hWnd;
	UINT m_Msg;
};
