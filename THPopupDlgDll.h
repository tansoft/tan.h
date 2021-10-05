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
* @brief ���������¼������װ��
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-06-12 �½���
*/
/**<pre>
�÷���
	�̳�ʵ�ָ�����ص�����
	����THPopupDlgDll::ShowPopupʱ����̳���ָ��
*/
class THPopupEventHandler
{
public:
	//����bCancel
	virtual BOOL OnNavUrl(const THString url)=0;
	//����bCancel
	virtual BOOL OnNavCmd(const THString cmd)=0;
	virtual void OnNavErr(const THString url,DWORD dwError)=0;
	virtual void OnNavNewWindow(const THString url)=0;
	virtual void OnWindowClose(const THString url)=0;
	virtual void OnWindowStart(const THString url)=0;
	virtual void OnSetTitle(const THString title)=0;
	//����bCancel
	virtual BOOL OnWindowOpen(const THString url)=0;
};

/**
* @brief ��������Dll��װ��
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-06-07 �½���
*/
/**<pre>
�÷���
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
�¼���Ӧ�ص���
	ͨ���̳�THPopupEventHandler��ʵ��
Html��Ӧ��
	popupcmd://				//������Ӧǰ׺
		MoveWindow				//�ƶ�����
		CloseWindow				//�رմ���
		MinWindow				//��С������
		MaxWindow				//��󻯴���
settings �����﷨��
	[height]0[/height]		//0Ϊ���Ĭ�ϴ�С,0ʱ����Ĺ��λ���е�h������
	[width]0[/width]		//0Ϊ���Ĭ�ϴ�С,0ʱ����Ĺ��λ���е�w������
	[dlgtype]1[/dlgtype]	//��������
		DLGTYPE_TITLE = 0x1,	//��Caption
		DLGTYPE_BORDER = 0x2,	//�߿�
		DLGTYPE_RESIZE = 0x4,	//�ɸı��С
		DLGTYPE_MIN = 0x8,		//��С��
		DLGTYPE_MAX = 0x10		//���
	[icon]0[/icon]			//0ΪϵͳĬ��ͼ��(IE),1Ϊ��ǰĿ¼icon.ico�ļ�
	[taskbar]0[/taskbar]	//��������ť 0:�� ,1:��
	[posx]a-w-a/20[/posx]	//����λ��,֧����������
	[posy]b-h-b/20[/posy]	//����λ��,֧����������
		(sx,sy),(ex,ey)			//������������ľ�������
		scx						//��Ļ���
		scy						//��Ļ�߶�
		w						//�����
		h						//���߶�
		th						//�������߶�
		bw						//�߿���
		bh						//�߿�߶�
		rand					//Ϊ���ֵ
		[posx]ex-w-5[/posx]		//�����ޱ߿��񴰿���������
		[posy]ey-h-5[/posy]
		[posx]ex-w-bw*2-5[/posx]//�����б߿��񴰿���������
		[posy]ey-h-bh*2-th-5[/posy]
	[distype]2[/distype]	//���ڳ��ַ�ʽ:0Ϊ���� 1:ֱ�ӵ���2:�� -> ��3:�� -> �� 4:�� -> ��5:�� -> ��
	[closetype]3[/closetype]//���ڹرշ�ʽ:1:ֱ�ӹر�2:�� -> ��3:�� -> �� 4:�� -> ��5:�� -> ��
	[timeout]10[/timeout]	//�����ʾ��ʱ��
		0						//��ʾ��������ʧ�������������ϳ���������������ʧ�����
		-1						//����Ҫ�ر�
	[showtype]-1[/showtype]	//�����ʾ������:-1��ǰ 0:Ϊ��ͨ 1:Ϊ�����ʾ
	[defer]0[/defer]		//�ӳ���ʾ������
	[musthtmltag]0[/musthtmltag]//��ʾ��html�Ƿ���뺬��tag PopupSettings��������ȷ�������html�Ƿ���Ҫ����
</pre>*/
class THPopupDlgDll : public THDllLoader
{
private:
	static BOOL NavUrl(const char *url,THPopupEventHandler *adddata)
	{
		if (adddata) return adddata->OnNavUrl(THCharset::a2t(url));
		return TRUE;
	}
	//����bCancel
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
	//����bCancel
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
	* @brief �����õ�������
	* @param url		��ҳ�ļ���ַ����Ϊ����settings�и���
	* @param settings	��������
	* @return			�Ƿ�ɹ�����
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