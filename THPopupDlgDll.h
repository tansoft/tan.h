#pragma once

#include <THString.h>
#include <THCharset.h>
#include <THDllLoader.h>

typedef BOOL (*PopupDlgShowPopup)(char *settings);
typedef BOOL (*PopupDlgCloseAllPopup)();
typedef BOOL (*PopupDlgFindPopupWindow)(LPSTR szCaption,HWND *hWndTarget);

#define PopupSettingsRightDownPopupTimeoutImage		_T("[dlgtype]0[/dlgtype][posx]ex-w-5[/posx][posy]ey-h-5[/posy][distype]3[/distype][closetype]2[/closetype][timeout]10[/timeout][showtype]-1[/showtype]")
#define PopupSettingsRightDownPopupAlwaysImage		_T("[dlgtype]0[/dlgtype][posx]ex-w-5[/posx][posy]ey-h-5[/posy][distype]3[/distype][closetype]1[/closetype][timeout]-1[/timeout][showtype]-1[/showtype]")
#define PopupSettingsRightDownPopupTimeoutWindow	_T("[dlgtype]3[/dlgtype][posx]ex-w-bw*2-5[/posx][posy]ey-h-bh*2-th-5[/posy][distype]3[/distype][closetype]2[/closetype][timeout]10[/timeout][showtype]-1[/showtype]")
#define PopupSettingsRightDownPopupAlwaysWindow		_T("[dlgtype]3[/dlgtype][posx]ex-w-bw*2-5[/posx][posy]ey-h-bh*2-th-5[/posy][distype]3[/distype][closetype]1[/closetype][timeout]-1[/timeout][showtype]-1[/showtype]")
#define PopupSettingsIe								_T("[dlgtype]31[/dlgtype][taskbar]1[/taskbar][posx]sx+100+rand%100[/posx][posy]sy+100+rand%100[/posy][distype]1[/distype][closetype]1[/closetype][timeout]-1[/timeout][showtype]0[/showtype]")

/**
* @brief 弹出窗口事件处理封装类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-06-12 新建类
*/
/**<pre>
用法：
	继承实现该类各回调函数
	调用THPopupDlgDll::ShowPopup时传入继承类指针
*/
class THPopupEventHandler
{
public:
	//返回bCancel
	virtual BOOL OnNavUrl(const THString url)=0;
	//返回bCancel
	virtual BOOL OnNavCmd(const THString cmd)=0;
	virtual void OnNavErr(const THString url,DWORD dwError)=0;
	virtual void OnNavNewWindow(const THString url)=0;
	virtual void OnWindowClose(const THString url)=0;
	virtual void OnWindowStart(const THString url)=0;
	virtual void OnSetTitle(const THString title)=0;
	//返回bCancel
	virtual BOOL OnWindowOpen(const THString url)=0;
};

/**
* @brief 弹出窗口Dll封装类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-06-07 新建类
*/
/**<pre>
用法：
	THPopupDlgDll m_popupdll;
	m_popupdll.ShowPopup(_T("http://tansoft.cn/tools"),PopupSettingsRightDownPopupAlwaysImage);
	Sleep(1000);
	m_popupdll.ShowPopup(_T("http://tansoft.cn/tools"),PopupSettingsRightDownPopupTimeoutImage);
	Sleep(1000);
	m_popupdll.ShowPopup(_T("http://tansoft.cn/tools"),PopupSettingsRightDownPopupAlwaysWindow);
	Sleep(1000);
	m_popupdll.ShowPopup(_T("http://tansoft.cn/tools"),PopupSettingsRightDownPopupTimeoutWindow);
	Sleep(1000);
	m_popupdll.ShowPopup(_T("http://tansoft.cn/tools"),PopupSettingsIe);
事件响应回调：
	通过继承THPopupEventHandler类实现
Html响应：
	popupcmd://				//命令响应前缀
		MoveWindow				//移动窗口
		CloseWindow				//关闭窗口
		MinWindow				//最小化窗口
		MaxWindow				//最大化窗口
settings 设置语法：
	[height]0[/height]		//0为广告默认大小,0时下面的广告位置中的h不可用
	[width]0[/width]		//0为广告默认大小,0时下面的广告位置中的w不可用
	[dlgtype]1[/dlgtype]	//窗口类型
		DLGTYPE_TITLE = 0x1,	//有Caption
		DLGTYPE_BORDER = 0x2,	//边框
		DLGTYPE_RESIZE = 0x4,	//可改变大小
		DLGTYPE_MIN = 0x8,		//最小化
		DLGTYPE_MAX = 0x10		//最大化
	[icon]0[/icon]			//0为系统默认图标(IE),1为当前目录icon.ico文件
	[taskbar]0[/taskbar]	//任务栏按钮 0:无 ,1:有
	[posx]a-w-a/20[/posx]	//广告的位置,支持四则运算
	[posy]b-h-b/20[/posy]	//广告的位置,支持四则运算
		(sx,sy),(ex,ey)			//桌面除任务栏的矩形坐标
		scx						//屏幕宽度
		scy						//屏幕高度
		w						//广告宽度
		h						//广告高度
		th						//标题栏高度
		bw						//边框宽度
		bh						//边框高度
		rand					//为随机值
		[posx]ex-w-5[/posx]		//常用无边框风格窗口坐标配置
		[posy]ey-h-5[/posy]
		[posx]ex-w-bw*2-5[/posx]//常用有边框风格窗口坐标配置
		[posy]ey-h-bh*2-th-5[/posy]
	[distype]2[/distype]	//窗口出现方式:0为隐藏 1:直接弹出2:上 -> 下3:下 -> 上 4:左 -> 右5:右 -> 左
	[closetype]3[/closetype]//窗口关闭方式:1:直接关闭2:上 -> 下3:下 -> 上 4:左 -> 右5:右 -> 左
	[timeout]10[/timeout]	//广告显示的时间
		0						//显示完立即消失，用于由下至上出现再由上至下消失的情况
		-1						//不需要关闭
	[showtype]-1[/showtype]	//广告显示的类型:-1置前 0:为普通 1:为后端显示
	[defer]0[/defer]		//延迟显示，秒数
	[musthtmltag]0[/musthtmltag]//显示的html是否必须含有tag PopupSettings，可用于确定请求的html是否需要播放
</pre>*/
class THPopupDlgDll : public THDllLoader
{
private:
	static BOOL NavUrl(const char *url,THPopupEventHandler *adddata)
	{
		if (adddata) return adddata->OnNavUrl(THCharset::a2t(url));
		return TRUE;
	}
	//返回bCancel
	static BOOL NavCmd(const char *cmd,THPopupEventHandler *adddata)
	{
		if (adddata) return adddata->OnNavCmd(THCharset::a2t(cmd));
		return TRUE;
	}
	static void NavErr(const char *url,DWORD dwError,THPopupEventHandler *adddata)
	{
		if (adddata) adddata->OnNavErr(THCharset::a2t(url),dwError);
	}
	static void NavNewWindow(const char *url,THPopupEventHandler *adddata)
	{
		if (adddata) adddata->OnNavNewWindow(THCharset::a2t(url));
	}
	static void WindowClose(const char *url,THPopupEventHandler *adddata)
	{
		if (adddata) adddata->OnWindowClose(THCharset::a2t(url));
	}
	static void WindowStart(const char *url,THPopupEventHandler *adddata)
	{
		if (adddata) adddata->OnWindowStart(THCharset::a2t(url));
	}
	static void SetTitle(const char *title,THPopupEventHandler *adddata)
	{
		if (adddata) adddata->OnSetTitle(THCharset::a2t(title));
	}
	//返回bCancel
	static BOOL WindowOpen(const char *url,THPopupEventHandler *adddata)
	{
		if (adddata) return adddata->OnWindowOpen(THCharset::a2t(url));
		return TRUE;
	}
public:
	THPopupDlgDll():THDllLoader(_T("PopupDlg.dll")){EmptyDllPointer();}
	virtual ~THPopupDlgDll(){FreeDll();}

	virtual BOOL OnFreeDll()
	{
		if (m_fnCloseAll) (m_fnCloseAll)();
		return TRUE;
	}

	virtual void EmptyDllPointer()
	{
		m_fnShow=NULL;
		m_fnCloseAll=NULL;
		m_fnFindWindow=NULL;
	}

	virtual BOOL InitDllPoinier()
	{
		m_fnShow=(PopupDlgShowPopup)GetProcAddress(m_module,"ShowPopup");
		m_fnCloseAll=(PopupDlgCloseAllPopup)GetProcAddress(m_module,"CloseAllPopup");
		m_fnFindWindow=(PopupDlgFindPopupWindow)GetProcAddress(m_module,"FindPopupWindow");
		if (!m_fnShow || !m_fnCloseAll || !m_fnFindWindow)
			return FALSE;
		return TRUE;
	}

	/**
	* @brief 按设置弹出窗口
	* @param url		网页文件地址，可为空在settings中给出
	* @param settings	属性设置
	* @return			是否成功加载
	*/
	BOOL ShowPopup(THString url,THString settings=_T(""),THPopupEventHandler *handler=NULL)
	{
		if (!url.IsEmpty())
			settings=THSimpleXml::MakeParam(_T("url"),url,_T("["),_T("]"))+settings;
		if (!InitDll()) return FALSE;
		if (!m_fnShow) return FALSE;
		//add cbfn settings
		if (handler)
		{
			THString cbfnstr,tmpstr;
			#ifdef _UNICODE
				#define SETPOINTER(a)	tmpstr.Format(L"%u",a);cbfnstr+=THSimpleXml::MakeParam(L"fn"L#a,tmpstr,L"(",L")")
			#else
				#define SETPOINTER(a)	tmpstr.Format("%u",a);cbfnstr+=THSimpleXml::MakeParam("fn"#a,tmpstr,"(",")")
			#endif
			SETPOINTER(NavUrl);
			SETPOINTER(NavCmd);
			SETPOINTER(NavErr);
			SETPOINTER(NavNewWindow);
			SETPOINTER(WindowClose);
			SETPOINTER(WindowStart);
			SETPOINTER(SetTitle);
			SETPOINTER(WindowOpen);
			tmpstr.Format(_T("%u"),handler);
			cbfnstr+=THSimpleXml::MakeParam(_T("fnAddData"),tmpstr,_T("("),_T(")"));
			settings=THSimpleXml::MakeParam(_T("cbfn"),cbfnstr,_T("["),_T("]"))+settings;
		}
		char *tmp=THCharset::t2a(settings);
		if (!tmp) return FALSE;
		(m_fnShow)(tmp);
		THCharset::free(tmp);
		return TRUE;
	}

private:
	PopupDlgShowPopup m_fnShow;
	PopupDlgCloseAllPopup m_fnCloseAll;
	PopupDlgFindPopupWindow m_fnFindWindow;
};