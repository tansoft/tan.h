// HttpServer.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error �ڰ������� PCH �Ĵ��ļ�֮ǰ������stdafx.h��
#endif

#include "resource.h"		// ������

#include <THHttpServer.h>
#include <THWinApp.h>
#include <THThread.h>
// CHttpServerApp:
// �йش����ʵ�֣������ HttpServer.cpp
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
// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CHttpServerApp theApp;
