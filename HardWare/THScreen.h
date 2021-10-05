#pragma once

#include <THString.h>
#include <THCharset.h>
#include <THSerialPort.h>

#define LIGHTTYPE_PRECOUNT		0x31	//小计
#define LIGHTTYPE_TOTALCOUNT	0x32	//总计
#define LIGHTTYPE_PAY			0x33	//收款
#define LIGHTTYPE_CHANGE		0x34	//找零
#define LIGHTTYPE_NONE			0x30	//全灭

/**
* @brief CX-D8A收银显示屏封装类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-11-22 新建类
*/
/**<pre>
</pre>*/
class THCXD8AScreen
{
public:
	THCXD8AScreen()
	{
	}

	virtual ~THCXD8AScreen(){CloseDevice();}

	BOOL OpenDevice(int nPort)
	{
		if (!m_port.InitPort(nPort,2400)) return FALSE;
		unsigned char buf[2];
		buf[0]=0x1B;
		buf[1]=0x40;
		return m_port.WriteToPortBin(buf,2);
	}

	BOOL CloseDevice()
	{
		if (!m_port.IsOpened()) return TRUE;
		m_port.ClosePort();
		return TRUE;
	}

	BOOL ClearScreen()
	{
		if (!m_port.IsOpened()) return FALSE;
		unsigned char buf;
		buf=0xc;
		return m_port.WriteToPortBin(&buf,1);
	}

	BOOL DisplayNumber(THString sNumber)
	{
		if (!m_port.IsOpened()) return FALSE;
		sNumber=THfsc(sNumber);
		if (!THStringConv::IsStringNumberAndDot(sNumber)) return FALSE;
		char *tmp=THCharset::t2a(sNumber);
		if (!tmp) return FALSE;
		int size=(int)strlen(tmp);
		if (size>15) size=15;
		unsigned char buf[20];
		buf[0]=0x1B;
		buf[1]=0x51;
		buf[2]=0x41;
		strncpy((char *)&buf[3],tmp,size);
		THCharset::free(tmp);
		buf[3+size]=0x0D; 
		return m_port.WriteToPortBin(buf,4+size);
	}

	BOOL DisplayNumber(double dVal,int nCount=2)
	{
		return DisplayNumber(THf2s(dVal,nCount));
	}

	BOOL LightControl(unsigned char nLightType)
	{
		if (!m_port.IsOpened()) return FALSE;
		char buf[3];
		buf[0]=0x1B;
		buf[1]=0x73;
		buf[2]=nLightType;
		return m_port.WriteToPortBin(buf,3);
	}

	BOOL Display(THString sNumber,unsigned char nLightType)
	{
		if (!LightControl(nLightType)) return FALSE;
		Sleep(100);
		if (!DisplayNumber(sNumber)) return FALSE;
		return TRUE;
	}

	BOOL Display(double dNumber,unsigned char nLightType)
	{
		if (!LightControl(nLightType)) return FALSE;
		Sleep(100);
		if (!DisplayNumber(dNumber)) return FALSE;
		return TRUE;
	}
protected:
	THSerialPort m_port;
};
