#pragma once

#include <THString.h>

#define PRINTMODE_ASCII99	0x0					///<9*9����
#define PRINTMODE_ASCII79	0x1					///<7*9����
#define PRINTMODE_BOLD		0x8					///<�Ӵ�
#define PRINTMODE_DOUBLEH	0x10				///<����ģʽ
#define PRINTMODE_DOUBLEW	0x20				///<����ģʽ
#define PRINTMODE_4SIZE		PRINTMODE_DOUBLEH|PRINTMODE_DOUBLEW	///<4����С
#define PRINTMODE_UNDERLINE	0x80				///<�»���
#define PRINTMODE_NORMAL	PRINTMODE_ASCII79	///<Ĭ��ģʽ
#define PRINTMODE_TITLE		PRINTMODE_4SIZE|PRINTMODE_BOLD		///<����ģʽ

#define ALIGNMODE_LEFT		0x0
#define ALIGNMODE_CENTER	0x1
#define ALIGNMODE_RIGHT		0x2
/**
* @brief GP76ϵ�д�ӡ����װ��
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-11-22 �½���
*/
/**<pre>
</pre>*/
class THGP76Printer
{
public:
	THGP76Printer()
	{
	}

	virtual ~THGP76Printer(){CloseDevice();}

	BOOL OpenDevice(int nPort)
	{
		//ͬ��ģʽport
		if (!m_port.InitPort(nPort,19200,0)) return FALSE;
		if (!InitPrinter()) return FALSE;
		if (!SelectPrintMode()) return FALSE;
		return TRUE;
	}

	BOOL CloseDevice()
	{
		if (!m_port.IsOpened()) return TRUE;
		m_port.ClosePort();
		return TRUE;
	}

	BOOL SelectPrintMode(unsigned char mode=PRINTMODE_NORMAL)
	{
		if (!m_port.IsOpened()) return FALSE;
		unsigned char buf[3];
		//����Ӣ�Ĵ�ӡģʽ
		buf[0]=0x1B;
		buf[1]=0x21;
		buf[2]=mode;
		if (!m_port.WriteToPortBin(buf,3)) return FALSE;
		//�������Ĵ�ӡģʽ
		int cmode=0;
		if (mode&PRINTMODE_DOUBLEH) cmode|=0x08;
		if (mode&PRINTMODE_DOUBLEW) cmode|=0x04;
		if (mode&PRINTMODE_UNDERLINE) cmode|=PRINTMODE_UNDERLINE;
		buf[0]=0x1C;
		buf[2]=cmode;
		return m_port.WriteToPortBin(buf,3);
	}

	/**
	* @brief �����м��
	* @param nSpace		�м��߶ȣ�(nSpace*0.176mm(1/144inch))
	* @return �Ƿ�ɹ�
	*/
	BOOL SetupHSpace(unsigned char nSpace=24)
	{
		if (!m_port.IsOpened()) return FALSE;
		unsigned char buf[3];
		buf[0]=0x1B;
		buf[1]=0x33;
		buf[2]=nSpace;
		return m_port.WriteToPortBin(buf,3);
	}

	BOOL InitPrinter()
	{
		if (!m_port.IsOpened()) return FALSE;
		unsigned char buf[2];
		buf[0]=0x1B;
		buf[1]=0x40;
		return m_port.WriteToPortBin(buf,2);
	}

	BOOL SetAlign(unsigned char mode=ALIGNMODE_LEFT)
	{
		if (!m_port.IsOpened()) return FALSE;
		unsigned char buf[3];
		buf[0]=0x1B;
		buf[1]=0x61;
		buf[2]=mode;
		return m_port.WriteToPortBin(buf,3);
	}

	BOOL StartPrint()
	{
		if (!m_port.IsOpened()) return FALSE;
		unsigned char buf[2];
		buf[0]=0x1B;
		buf[1]=0xE9;
		if (!m_port.WriteToPortBin(buf,2)) return FALSE;
		if (m_port.ReadFromPort(buf,2)<2) return FALSE;
		if (buf[1]==0xE8) return FALSE;
		return TRUE;
	}

	BOOL EndPrint()
	{
		if (!m_port.IsOpened()) return FALSE;
		unsigned char buf[2];
		buf[0]=0x1B;
		buf[1]=0x7D;
		if (!m_port.WriteToPortBin(buf,2)) return FALSE;
		if (m_port.ReadFromPort(buf,2)<2) return FALSE;
		if (buf[1]==0x78) return FALSE;
		return TRUE;
	}

	BOOL CreatePulse(unsigned char m,unsigned char p1,unsigned char p2)
	{
		if (!m_port.IsOpened()) return FALSE;
		unsigned char buf[5];
		buf[0]=0x1B;
		buf[1]=0x70;
		buf[2]=m;
		buf[3]=p1;
		buf[4]=p2;
		return m_port.WriteToPortBin(buf,5);
	}

	BOOL OpenCashBox()
	{
		return CreatePulse(0,25,255);
	}

	BOOL PrintText(THString text)
	{
		return m_port.WriteToPort(text);
	}

	/**
	* @brief ��ӡ����ֽ
	* @param nRol		��ֽ������(nRol*0.176mm(1/144inch))
	* @return �Ƿ�ɹ�
	*/
	BOOL Roll(unsigned char nRol)
	{
		if (!m_port.IsOpened()) return FALSE;
		unsigned char buf[3];
		buf[0]=0x1B;
		buf[1]=0x4A;
		buf[2]=nRol;
		return m_port.WriteToPortBin(buf,3);
	}

	BOOL SimplePrint(THString title,THStringArray &headers,THStringArray &contents,THString end)
	{
		SetAlign(ALIGNMODE_CENTER);
		SelectPrintMode(PRINTMODE_DOUBLEW);
		PrintText(title+_T("\r\n\r\n"));
		SelectPrintMode(PRINTMODE_NORMAL);
		for(int i=0;i<headers.GetSize();i++)
		{
			if (i%2==0)
				SetAlign(ALIGNMODE_LEFT);
			else
				SetAlign(ALIGNMODE_RIGHT);
			PrintText(headers[i]);
			if (i%2==0)
				PrintText(_T("\r"));
			else
				PrintText(_T("\r\n"));
		}
		if (headers.GetSize()%2==1)
			PrintText(_T("\r\n"));
		PrintText(_T("========================================\r\n"));
		for(i=0;i<contents.GetSize();i++)
			PrintText(contents[i]+_T("\r\n"));
		PrintText(_T("========================================\r\n"));
		PrintText(end);
		Roll(250);
		return TRUE;
	}
protected:
	THSerialPort m_port;
};
