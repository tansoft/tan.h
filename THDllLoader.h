#pragma once

/**
* @brief 加载Dll封装类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-07-03 新建类
*/
/**<pre>
用法：
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
		//最好在各个继承类中主动调用FreeDll
		if (m_module)
		{
			FreeLibrary(m_module);
			m_module=NULL;
		}
	}

	/**
	* @brief 加载Dll
	* @return 是否成功
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
	* @brief 卸载Dll
	* @return 是否成功
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
	* @brief 初始加载Dll成功后默认调用的函数
	* @return 是否成功
	*/
	virtual BOOL OnInitDll(){return TRUE;}

	/**
	* @brief 卸载Dll前调用的函数
	* @return 是否成功
	*/
	virtual BOOL OnFreeDll(){return TRUE;}

	/**
	* @brief 清空所有函数指针
	*/
	virtual void EmptyDllPointer()=0;

	/**
	* @brief 初始所有函数指针
	* @return 是否成功，返回失败自动调卸载Dll
	*/
	virtual BOOL InitDllPoinier()=0;

protected:
	HMODULE m_module;
	THString m_dllname;
};