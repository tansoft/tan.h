#pragma once

#define THSINGLEPROCESS_MESSAGEID		2512

/**
* @brief 单一进程类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-12-25 新建类
*/
class THSingleProcess
{
public:
	THSingleProcess()
	{
		m_hSem=NULL;
		m_FristhWnd=NULL;
	}
	virtual ~THSingleProcess()
	{
		if (m_hSem!=NULL)
		{
			CloseHandle(m_hSem);
			m_hSem=NULL;
		}
	}

	/**
	* @brief 判断是否是第一个进程
	* @param sProcessStr	进程的唯一名称
	* @return 是否是第一个进程
	*/
	BOOL IsFristProcess(THString sProcessStr)
	{
		if (m_hSem!=NULL) return TRUE;
		m_hSem=CreateSemaphore(NULL,1,1,sProcessStr);
		if (GetLastError()==ERROR_ALREADY_EXISTS)
		{
			m_FristhWnd=FindWindow(NULL,sProcessStr);
			CloseHandle(m_hSem);
			m_hSem=NULL;
			return FALSE;
		}
		return TRUE;
	}

	/**
	* @brief 发送信息到第一个进程
	* @param sData	进程的唯一名称
	* @param hLocalWnd	本进程用于接收信息的句柄
	*/
	void SendStringDataToFristProcess(THString sData,HWND hLocalWnd)
	{
		if (m_FristhWnd)
		{
			COPYDATASTRUCT cds;
			cds.dwData=THSINGLEPROCESS_MESSAGEID;
			cds.cbData=sData.GetLength();
			cds.lpData=(void *)(LPCTSTR)sData;
			SendMessage(m_FristhWnd,WM_COPYDATA,(WPARAM)hLocalWnd,(LPARAM)&cds);
		}
	}

	/**
	* @brief 响应copydata消息，返回内容
	* @param cds	copydata结构
	* @return copydata的消息
	*/
	THString GetStringDataFromOtherProcess(COPYDATASTRUCT *cds)
	{
		THString ret;
		if (cds->dwData==THSINGLEPROCESS_MESSAGEID)
			ret.SetString((LPCTSTR)cds->lpData,cds->cbData);
		return ret;
	}
protected:
	HANDLE m_hSem;
	HWND m_FristhWnd;
};
