#pragma once

/**
* @brief ����Dll��װ��
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-07-03 �½���
*/
/**<pre>
�÷���
</pre>*/
class THDllLoader
{
public:
	THDllLoader(THString dllname)
	{
		m_module=NULL;
		m_dllname=dllname;
	}
	virtual ~THDllLoader()
	{
		//����ڸ����̳�������������FreeDll
		if (m_module)
		{
			FreeLibrary(m_module);
			m_module=NULL;
		}
	}

	/**
	* @brief ����Dll
	* @return �Ƿ�ɹ�
	*/
	virtual BOOL InitDll()
	{
		if (m_module) return TRUE;//already init
		EmptyDllPointer();
		m_module=LoadLibrary(m_dllname);
		if (!m_module) return FALSE;
		if (!InitDllPoinier())
		{
			FreeDll();
			return FALSE;
		}
		if (!OnInitDll())
		{
			FreeDll();
			return FALSE;
		}
		return TRUE;
	}

	/**
	* @brief ж��Dll
	* @return �Ƿ�ɹ�
	*/
	virtual BOOL FreeDll()
	{
		if (m_module)
		{
			if (!OnFreeDll()) return FALSE;
			FreeLibrary(m_module);
			m_module=NULL;
		}
		EmptyDllPointer();
		return TRUE;
	}

	/**
	* @brief ��ʼ����Dll�ɹ���Ĭ�ϵ��õĺ���
	* @return �Ƿ�ɹ�
	*/
	virtual BOOL OnInitDll(){return TRUE;}

	/**
	* @brief ж��Dllǰ���õĺ���
	* @return �Ƿ�ɹ�
	*/
	virtual BOOL OnFreeDll(){return TRUE;}

	/**
	* @brief ������к���ָ��
	*/
	virtual void EmptyDllPointer()=0;

	/**
	* @brief ��ʼ���к���ָ��
	* @return �Ƿ�ɹ�������ʧ���Զ���ж��Dll
	*/
	virtual BOOL InitDllPoinier()=0;

protected:
	HMODULE m_module;
	THString m_dllname;
};