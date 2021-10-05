// HttpServer.h : PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error 在包含用于 PCH 的此文件之前包含“stdafx.h”
#endif

#include "resource.h"		// 主符号

#include <THHttpServer.h>
#include <THWinApp.h>
#include <THThread.h>
// CHttpServerApp:
// 有关此类的实现，请参阅 HttpServer.cpp
//

class CHttpServerApp : public THWinApp ,public THScheduleThread
{
public:
	CHttpServerApp();

	virtual void ScheduleFunc(int scheduleid,void *adddata)
	{
		THString filepath=m_ini.GetString(_T("THHttpServerSettings"),_T("sRecordPath"),m_sExePath+_T("\\HttpServer.log"));
		THTime ti;
		filepath=THSysMisc::AddTailFileName(filepath,ti.Format(_T("%y%m%d")));
		m_server.SetWebLogMode(filepath,m_server.GetWebLogMode());
	}

	void StartLogSchedule()
	{
		StopAllSchedule();
		if (m_ini.GetInt(_T("THHttpServerSettings"),_T("nRecordType"),0)==1)
		{
			THTime ti;
			ti.SetTime(ti.GetYear(),ti.GetMonth(),ti.GetDay(),0,0,0);
			NewSchedule(0,ti,SCHEDULEMODE_DAY);
			//Frist Call
			ScheduleFunc(1,NULL);
		}
	}

	THHttpServer m_server;
// 重写
	public:
	virtual BOOL InitInstance();

// 实现

	DECLARE_MESSAGE_MAP()
};

extern CHttpServerApp theApp;
