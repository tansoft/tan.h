
#pragma once

#pragma pack(push,1)

typedef enum _THDWMNCRENDERINGPOLICY {
    THDWMNCRP_USEWINDOWSTYLE,
    THDWMNCRP_DISABLED,
    THDWMNCRP_ENABLED,
    THDWMNCRP_LAST
} THDWMNCRENDERINGPOLICY;

enum THDWMWINDOWATTRIBUTE
{
    THDWMWA_NCRENDERING_ENABLED = 1,      // [get] Is non-client rendering enabled/disabled
    THDWMWA_NCRENDERING_POLICY,           // [set] Non-client rendering policy
    THDWMWA_TRANSITIONS_FORCEDISABLED,    // [set] Potentially enable/forcibly disable transitions
    THDWMWA_ALLOW_NCPAINT,                // [set] Allow contents rendered in the non-client area to be visible on the DWM-drawn frame.
    THDWMWA_CAPTION_BUTTON_BOUNDS,        // [get] Bounds of the caption button area in window-relative space.
    THDWMWA_NONCLIENT_RTL_LAYOUT,         // [set] Is non-client content RTL mirrored
    THDWMWA_FORCE_ICONIC_REPRESENTATION,  // [set] Force this window to display iconic thumbnails.
    THDWMWA_FLIP3D_POLICY,                // [set] Designates how Flip3D will treat the window.
    THDWMWA_EXTENDED_FRAME_BOUNDS,        // [get] Gets the extended frame bounds rectangle in screen space
    THDWMWA_LAST
};
#pragma pack(pop)

typedef HRESULT (__stdcall *FnDwmSetWindowAttribute)(HWND hWnd,DWORD dwAttribute,LPCVOID pvAttribute,DWORD cbAttribute);

/**
* @brief Window Vista Dwm 函数封装类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-10-09 新建类
*/
class THVistaDwmFnHandler
{
public:
	THVistaDwmFnHandler()
	{
		m_mod=NULL;
		EmptyPointer();
		LoadLibrary();
	}

	virtual ~THVistaDwmFnHandler()
	{
		EmptyPointer();
		FreeLibrary();
	}

	FnDwmSetWindowAttribute m_swa;
private:
	void EmptyPointer()
	{
		m_swa=NULL;
	}
	void LoadLibrary()
	{
		m_mod=::LoadLibrary(_T("dwmapi.dll"));
		if (m_mod)
		{
			m_swa=(FnDwmSetWindowAttribute)::GetProcAddress(m_mod,"DwmSetWindowAttribute");
		}
	}
	void FreeLibrary()
	{
		if (m_mod)
		{
			::FreeLibrary(m_mod);
			m_mod=NULL;
		}
	}
	HMODULE m_mod;
};
/**
* @brief Window Vista Dwm 封装类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-10-09 新建类
*/
class THVistaDwm
{
public:
	static THVistaDwmFnHandler& GetInstance()
	{
		static THVistaDwmFnHandler g_handler;
		return g_handler;
	}
	static HRESULT DwmSetWindowAttribute(HWND hWnd,DWORD dwAttribute,LPCVOID pvAttribute,DWORD cbAttribute)
	{
		if (!GetInstance().m_swa) return E_NOINTERFACE;
		return (GetInstance().m_swa)(hWnd,dwAttribute,pvAttribute,cbAttribute);
	}
	static HRESULT SetNcRenderingPolicy(HWND hWnd,THDWMNCRENDERINGPOLICY policy)
	{
		return DwmSetWindowAttribute(hWnd,THDWMWA_NCRENDERING_POLICY,&policy,sizeof(policy));
	}
};