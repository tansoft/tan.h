#pragma once

#include <THSystem.h>
#include <ResizableLib\ResizableDialog.h>
#include <THLibMfcCtrls.h>

/**
* @brief 自适应大小对话框 基类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-05-30 新建类
*/
/**<pre>
用法：
	//继承Dlg自THResizeDlg
	AddAnchor(IDOK,TOP_LEFT,TOP_RIGHT);//NOANCHOR,TOP_LEFT,TOP_CENTER,TOP_RIGHT,MIDDLE_LEFT,MIDDLE_CENTER,MIDDLE_RIGHT,BOTTOM_LEFT,BOTTOM_CENTER,BOTTOM_RIGHT
	EnableSaveRestore(FALSE);
	void SetMaximizedRect(const CRect& rc);		// set window rect when maximized
	void ResetMaximizedRect();					// reset to default maximized rect
	void SetMinTrackSize(const CSize& size);	// set minimum tracking size
	void ResetMinTrackSize();					// reset to default minimum tracking size
	void SetMaxTrackSize(const CSize& size);	// set maximum tracking size
	void ResetMaxTrackSize();					// reset to default maximum
	BOOL IsSizeGripVisible();	// TRUE if grip is set to be visible
	SetSizeGripVisibility(FALSE);
	SetSizeGripBkMode(TRANSPARENT);
	SetSizeGripShape(TRUE);
</pre>*/
typedef CResizableDialog THResizeDlg;
/*class THResizeDlg : public CResizableDialog
{
public:
	THResizeDlg():CResizableDialog(){}
	THResizeDlg(UINT nIDTemplate, CWnd* pParentWnd = NULL):CResizableDialog(nIDTemplate,pParentWnd){}
	THResizeDlg(LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL):CResizableDialog(lpszTemplateName,pParentWnd){}
	virtual ~THResizeDlg(){}
};*/