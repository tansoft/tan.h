#pragma once

#define THSINGLEPROCESS_MESSAGEID		2512

/**
* @brief ��һ������
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-12-25 �½���
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
	* @brief �ж��Ƿ��ǵ�һ������
	* @param sProcessStr	���̵�Ψһ����
	* @return �Ƿ��ǵ�һ������
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
	* @brief ������Ϣ����һ������
	* @param sData	���̵�Ψһ����
	* @param hLocalWnd	���������ڽ�����Ϣ�ľ��
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
	* @brief ��Ӧcopydata��Ϣ����������
	* @param cds	copydata�ṹ
	* @return copydata����Ϣ
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
