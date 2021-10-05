#pragma once

#define THSYSTEM_INCLUDE_MSHTML
#define THSYSTEM_INCLUDE_OLEACC
#define THSYSTEM_INCLUDE_STL_STRUCT
#include <THSystem.h>
#include <THString.h>
#include <THDebug.h>

/**
* @brief Ie跨进程操作类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2009-08-22 新建类
*/
/**<pre>
</pre>*/
class THRemoteIeAccess
{
public:
	THRemoteIeAccess()
	{
		CoInitialize(NULL);
		//显式装载 Microsoft Active Accessibility (MSAA)
		m_hInst=::LoadLibrary(_T("OLEACC.DLL"));
		if (m_hInst)
			m_pfn=(LPFNOBJECTFROMLRESULT)::GetProcAddress(m_hInst,"ObjectFromLresult");
		else
			m_pfn=NULL;
	}
	virtual ~THRemoteIeAccess()
	{
		m_pfn=NULL;
		::FreeLibrary(m_hInst);
		m_hInst=NULL;
		CoUninitialize();
	}
	BOOL GetHtmlDocFromHwnd(HWND hWnd,IHTMLDocument2 **pDoc)
	{
		if (m_hInst && m_pfn)
		{
			HWND hWndChild=NULL;
			::EnumChildWindows(hWnd,_EnumChildProc,(LPARAM)&hWndChild);
			if (hWndChild)
			{
				CComPtr<IHTMLDocument2> pHTMLDoc;
				LRESULT lRes;
				UINT nMsg=::RegisterWindowMessage(_T("WM_HTML_GETOBJECT"));
				if (::SendMessageTimeout(hWndChild,nMsg,0L,0L,SMTO_ABORTIFHUNG,1000,(DWORD*)&lRes))
				{
					HRESULT hr=(*m_pfn)(lRes,IID_IHTMLDocument2,0,(void**)pDoc);
					return (SUCCEEDED(hr));
				}
			}
		}
		return FALSE;
	}
private:
	HINSTANCE m_hInst;
	LPFNOBJECTFROMLRESULT m_pfn;
	static BOOL CALLBACK _EnumChildProc(HWND hwnd,LPARAM lParam)
	{
		TCHAR buf[1024];
		GetClassName(hwnd,buf,1024);
		if (strcmp(_T("Internet Explorer_Server"),buf)==0)
		{
			HWND *ret=(HWND *)lParam;
			*ret=hwnd;
			return FALSE;
		}
		return TRUE;
	}
};

/**
* @brief Ie操作封装类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2009-08-22 新建类
*/
/**<pre>
</pre>*/
class THIeHandler
{
public:
	//当使用HtmlWindows无法获得doc时，是因为跨域问题，这时先从window获取browser，再用browser获取doc即可
	// Converts a IHTMLWindow2 object to a IHTMLDocument2. Returns NULL in case of failure.
	// It takes into account accessing the DOM across frames loaded from different domains.
	static CComQIPtr<IHTMLDocument2> IHtmlWindow2IHtmlDocument(CComQIPtr<IHTMLWindow2> spWindow)
	{
		if (spWindow==NULL) return CComQIPtr<IHTMLDocument2>();
		CComQIPtr<IHTMLDocument2> spDocument;
		HRESULT hRes = spWindow->get_document(&spDocument);
		// The html document was properly retrieved.
		if ((S_OK == hRes) && (spDocument != NULL)) return spDocument;
		// hRes could be E_ACCESSDENIED that means a security restriction that
		// prevents scripting across frames that loads documents from different internet domains.
		CComQIPtr<IWebBrowser2> spBrws=IHtmlWindow2IWebBrowser(spWindow);
		if (spBrws == NULL) return CComQIPtr<IHTMLDocument2>();
		// Get the document object from the IWebBrowser2 object.
		CComQIPtr<IDispatch> spDisp;
		hRes = spBrws->get_Document(&spDisp);
		spDocument=spDisp;
		return spDocument;
	}

	// Converts a IHTMLWindow2 object to a IWebBrowser2. Returns NULL in case of failure.
	static CComQIPtr<IWebBrowser2> IHtmlWindow2IWebBrowser(CComQIPtr<IHTMLWindow2> spWindow)
	{
		if (spWindow==NULL) return CComQIPtr<IWebBrowser2>();
		CComQIPtr<IServiceProvider>  spServiceProvider=spWindow;
		if (spServiceProvider == NULL) return CComQIPtr<IWebBrowser2>();
		CComQIPtr<IWebBrowser2> spWebBrws;
		HRESULT hRes = spServiceProvider->QueryService(IID_IWebBrowserApp, IID_IWebBrowser2,(void**)&spWebBrws);
		if (hRes != S_OK) return CComQIPtr<IWebBrowser2>();
		return spWebBrws;
	}

	static CComQIPtr<IWebBrowser2> IHTMLDocument2IWebBrowser(CComQIPtr<IHTMLDocument2> spDoc)
	{
		if (spDoc==NULL) return CComQIPtr<IWebBrowser2>();
		CComQIPtr<IServiceProvider>  spServiceProvider=spDoc;
		if (spServiceProvider == NULL) return CComQIPtr<IWebBrowser2>();
		CComQIPtr<IWebBrowser2> spWebBrws;
		HRESULT hRes = spServiceProvider->QueryService(IID_IWebBrowserApp, IID_IWebBrowser2,(void**)&spWebBrws);
		if (hRes != S_OK) return CComQIPtr<IWebBrowser2>();
		return spWebBrws;
	}

	//直接用IWebBrowser::get_HWND在ie7里，会返回上一级窗口的hwnd，如果需要获取具体ie窗口的句柄，且兼容ie6，使用以下函数
	static HWND GetHwndFromIWebBrowser(IWebBrowser *pWebBrowser2)
	{
		HWND hwndBrowser = NULL;
		IServiceProvider* pServiceProvider = NULL;
		if (SUCCEEDED(pWebBrowser2->QueryInterface(IID_IServiceProvider,(void**)&pServiceProvider)))
		{
			IOleWindow* pWindow = NULL;
			if (SUCCEEDED(pServiceProvider->QueryService(SID_SShellBrowser,IID_IOleWindow,(void**)&pWindow)))
			{
				// hwndBrowser is the handle of TabWindowClass
				pWindow->GetWindow(&hwndBrowser);
				pWindow->Release();
			}
			pServiceProvider->Release();
		}
		return hwndBrowser;
	}
};

/**
* @brief Ie元素操作封装类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2009-08-22 新建类
*/
/**<pre>
支持选择器说明，兼容css和jQuery选择器：
	*						所有
	elm						指定类型elm，如a，input
	.class					指定class
	#id						指定id
	elm.class				指定类型且指定class
	elm1,elm2.class,elm3	分别多个
	elm a					elm下的所有a
	elm > a					elm第一层子元素a
	//elm + a					elm下一个元素a，=next
	//elm ~ a					elm后所有同辈元素a，=nextAll
	elm:first
	elm:last				第一/最后一个elm元素
	elm:odd
	elm:even				奇数/偶数elm元素
	elm:eq(1)
	elm:gt(1)
	elm:lt(1)				索引等于/大于/小于1的元素，n从0开始
	elm:contains(text)		包含内容text的elm
查找伪代码：
	空或*直接返回全部
	用,分隔，逐个叠加调用
	查找:号，存在则先找前面的元素，再调用后面的过滤器过滤
	查找空格,>,+,~，存在则分别作对应判断处理
	查找是否存在非开始的.和#号，如果存在先找前部分元素，再作后部分处理
	直接处理最后的元素
</pre>*/
class THIeHtmlElementParser
{
public:
	THIeHtmlElementParser(){}
	virtual ~THIeHtmlElementParser(){}

	static RECT GetElementArea(IHTMLElement *elem){
		CComQIPtr<IHTMLElement2> pElem = elem;
		RECT rect;
		memset(&rect,0,sizeof(RECT));
		if (pElem!=NULL) {
			IHTMLRect *prect;
			if (SUCCEEDED(pElem->getBoundingClientRect(&prect))) {
				prect->get_top(&rect.top);
				prect->get_left(&rect.left);
				prect->get_right(&rect.right);
				prect->get_bottom(&rect.bottom);
				prect->Release();
			}
		}
		return rect;
	}

	//滚动视图，确保元素在可视范围内
	static BOOL ScrollElementToVisible(IHTMLElement *elem,IHTMLDocument2 *pDoc,long vieww,long viewh)
	{
		IHTMLElement *body = NULL;
		HRESULT hr = pDoc->get_body(&body);
		if (FAILED(hr)) return FALSE;
		CComQIPtr<IHTMLElement2> pElem = elem;
		CComQIPtr<IHTMLElement2> pBody = body;
		body->Release();
		if (pElem==NULL || pBody==NULL) return FALSE;
		IHTMLRect *rect;
		if (FAILED(pElem->getBoundingClientRect(&rect))) return FALSE;
		long t,l,r,b;
		if (FAILED(rect->get_top(&t)) || FAILED(rect->get_left(&l)) ||
			FAILED(rect->get_right(&r)) || FAILED(rect->get_bottom(&b)) ){
			rect->Release();
			return FALSE;
		}
		rect->Release();
		IHTMLWindow2 *pWin;
		if (FAILED(pDoc->get_parentWindow(&pWin))) return FALSE;
		long st=t+(b-t)/2-viewh/2;
		long sl=l+(r-l)/2-vieww/2;
		//由于BoundingClientRect由当前scroll位置计算出，因此这里用scrollby
		pWin->scrollBy(sl,st);
		pWin->Release();
		return TRUE;
	}

	//引起事件，如：onmousedown onmouseup onclick
	static BOOL FireEvent(THString event,IHTMLDocument2 *spDoc2,IHTMLElement *pElem)
	{
		CComQIPtr<IHTMLDocument4> spDoc4 = spDoc2;
		if(spDoc4 == NULL) return FALSE;
		CComPtr<IHTMLEventObj> pEvent;
		spDoc4->createEventObject(NULL, &pEvent);
		CComQIPtr<IHTMLEventObj2> pEvent2(pEvent);
		CComBSTR eventName = event;
		pEvent2->put_type(eventName);
		CComQIPtr<IHTMLElement3, &IID_IHTMLElement3> pElem3(pElem);
		VARIANT_BOOL result;
		CComVariant vEvent = pEvent;
		return SUCCEEDED(pElem3->fireEvent(eventName, &vEvent, &result));
	}

	/**
	* @brief 查找页面中匹配的元素，只返回第一个匹配的
	* @param selector		元素选择器，详见类说明
	* @param pDoc			文档指针
	* @return 返回元素指针，找不到返回NULL，返回有效时需要在使用完之后调用Release
	*/
	static IHTMLElement *FindElement(THString selector,IHTMLDocument2 *pDoc){
		list<IHTMLElement *> items;
		FindElements(selector,pDoc,&items,1);
		ASSERT(items.size()==0 || items.size()==1);
		if (items.size()>0) return *items.begin();
		return NULL;
	}

	/**
	* @brief 查找页面中匹配的元素集
	* @param selector		元素选择器，详见类说明
	* @param pDoc			文档指针
	* @param items			返回结果数组，需要循环调用Release进行释放
	* @param limitcount		是否限制查找的个数，-1为不限制
	* @return 返回是否成功
	*/
	static BOOL FindElements(THString selector,IHTMLDocument2 *pDoc,list<IHTMLElement *> *items,int limitcount=-1){
		if (!pDoc || !items) return FALSE;
		if (limitcount==0) return TRUE;
		//空或*直接返回全部
		if (selector.IsEmpty() || selector==_T("*")) return AllElements(pDoc,items,limitcount);
		//用,分隔，逐个叠加调用
		if (selector.Find(_T(","))!=-1) {
			THStringToken t(selector,_T(","));
			while(t.IsMoreTokens())
				if (!FindElements(t.GetNextToken(),pDoc,items,limitcount)) return FALSE;
			return TRUE;
		}
		//查找:号，存在则先找前面的元素，再调用后面的过滤器过滤
		list<IHTMLElement *> tempitems;
		int pos=selector.Find(_T(":"));
		if (pos!=-1) {
			if (!FindSingleElements(selector.Left(pos),pDoc,&tempitems,-1)) return FALSE;
			return PositionFilter(selector.Mid(pos+1),&tempitems,items,limitcount);
		}
		//查找空格,>,+,~，存在则分别作对应判断处理
		selector.Replace(_T(" >"),_T(">"));
		selector.Replace(_T("> "),_T(">"));
		selector.Replace(_T(" +"),_T("+"));
		selector.Replace(_T("+ "),_T("+"));
		selector.Replace(_T(" ~"),_T("~"));
		selector.Replace(_T("~ "),_T("~"));
		pos=selector.Find(_T(" "));
		int pos2=selector.Find(_T(">"));
		if (pos2!=-1) pos=pos2;
		if (pos!=-1) {
			if (!FindSingleElements(selector.Left(pos),pDoc,&tempitems,-1)) return FALSE;
			return SubElements(selector.Mid(pos+1),&tempitems,items,limitcount,pos2==-1);
		}
		//fixme + ~
		//查找是否存在非开始的.和#号，如果存在先找前部分元素，再作后部分处理
		pos=selector.Find(_T("."),1);
		if (pos==-1) pos=selector.Find(_T("#"),1);
		if (pos!=-1) {
			if (!FindSingleElements(selector.Left(pos),pDoc,&tempitems,-1)) return FALSE;
			return FindSingleElements(selector.Mid(pos),&tempitems,items,limitcount);
		}
		//直接处理最后的元素
		return FindSingleElements(selector,pDoc,items,limitcount);
	}
	static void FreeElementsList(list<IHTMLElement *> *items,IHTMLElement *except=NULL) {
		list<IHTMLElement *>::iterator it=items->begin();
		IHTMLElement *elm;
		while(it!=items->end()) {
			elm=*it;
			if (elm==except) it++;
			else {
				elm->Release();
				items->erase(it++);
			}
		}
	}
	static void TestCase(IHTMLDocument2 *pDoc){
		#define TESTCASE(selector)	{ASSERT(FindElements(selector,pDoc,&items)); \
			THDebug(_T("FindElements %s result:%d"),selector,items.size()); \
			FreeElementsList(&items);ASSERT(items.size()==0);}
		list<IHTMLElement *> items;
		TESTCASE(_T(""));
		TESTCASE(_T("*"));
		TESTCASE(_T("a"));
		TESTCASE(_T(".a2"));
		TESTCASE(_T("#a3"));
		TESTCASE(_T("a.a2"));
		TESTCASE(_T("a#a3"));
		TESTCASE(_T("input,a.a2,div"));
		TESTCASE(_T("#click a"));
		TESTCASE(_T("#click > a"));
		//TESTCASE(_T("#id + a"));
		//TESTCASE(_T("#id ~ a"));
		TESTCASE(_T("#click:first"));
		TESTCASE(_T("#click:last"));
		TESTCASE(_T(".a2:odd"));
		TESTCASE(_T(".a2:even"));
		TESTCASE(_T(".a2:eq(1)"));
		TESTCASE(_T(".a2:gt(1)"));
		TESTCASE(_T(".a2:lt(1)"));
		TESTCASE(_T("button:contains('OK')"));
	}
private:
	static BOOL SubElement(THString selector,IHTMLElement *input,list<IHTMLElement *> *output,int limitcount,BOOL recursion){
		IDispatch *pDispatch;
		HRESULT hr=input->get_children(&pDispatch);
		if(SUCCEEDED(hr) && pDispatch) {
			IHTMLElementCollection* pColl;
			hr=pDispatch->QueryInterface(IID_IHTMLElementCollection,(void**)&pColl);
			if (SUCCEEDED(hr) && pColl) {
				if (!FindSingleElements(selector,pColl,output,limitcount)) return FALSE;
				if (recursion && (limitcount==-1 || output->size()<(size_t)limitcount)) {
					list<IHTMLElement *> tempitems;
					if (!FillCollectionToList(pColl,&tempitems,-1)) return FALSE;
					list<IHTMLElement *>::iterator it=tempitems.begin();
					while(it!=tempitems.end()) {
						SubElement(selector,*it,output,limitcount,recursion);
						(*it)->Release();
						it++;
					}
				}
				pColl->Release();
			}
		}
		return TRUE;
	}
	static BOOL SubElements(THString selector,list<IHTMLElement *> *input,list<IHTMLElement *> *output,int limitcount,BOOL recursion){
		list<IHTMLElement *>::iterator it=input->begin();
		BOOL ret=TRUE;
		while(it!=input->end()) {
			if (!SubElement(selector,*it,output,-1,recursion)) ret=FALSE;
			(*it)->Release();
			it++;
		}
		input->clear();
		return ret;
	}
	static BOOL PositionFilter(THString filter,list<IHTMLElement *> *input,list<IHTMLElement *> *output,int limitcount=-1){
		#define NORMALPARSE(cond) {int i=0;it=input->begin();while(it!=input->end()){\
							if (cond) output->push_back(*it); else (*it)->Release();\
							it++;i++;}input->clear();}
		if (limitcount==0) return TRUE;
		list<IHTMLElement *>::iterator it;
		if (filter==_T("first") || filter==_T("last")) {
			it=filter==_T("first")?input->begin():(input->size()>0?--input->end():input->end());
			if (it!=input->end()) output->push_back(*it);
			FreeElementsList(input,(it!=input->end())?*it:NULL);
		} else if (filter==_T("odd") || filter==_T("even")) {
			int fit=filter==_T("odd")?1:0;
			NORMALPARSE(i%2==fit);
		} else if (filter.Left(8)==_T("contains")){
			filter=filter.Mid(8).Trim(_T("()'\""));
			int i=0;
			it=input->begin();
			BSTR text;
			CString ctext;
			HRESULT hr;
			while(it!=input->end()) {
				ctext.Empty();
				hr=(*it)->get_innerText(&text);
				if (SUCCEEDED(hr)) ctext=text;
				if (ctext.Find(filter)!=-1)
					output->push_back(*it);
				else
					(*it)->Release();
				it++;
				i++;
			}
			input->clear();
		} else {
			THString fmode=filter.Left(2);
			if (fmode==_T("eq") || fmode==_T("gt") || fmode==_T("lt")) {
				filter=filter.Mid(2);
				filter.Replace(_T("("),_T(""));
				filter.Replace(_T(")"),_T(""));
				int index=_ttoi(filter);
				it=input->begin();
				int i=0;
				if (fmode==_T("eq")) NORMALPARSE(i==index)
				else if (fmode==_T("gt")) NORMALPARSE(i>index)
				else if (fmode==_T("lt")) NORMALPARSE(i<index)
			} else
				return FALSE;
		}
		return TRUE;
	}
	static BOOL FindSingleElements(THString selector,IHTMLDocument2 *pDoc,list<IHTMLElement *> *items,int limitcount=-1){
		IHTMLElementCollection* pColl = NULL;
		HRESULT hr = pDoc->get_all(&pColl);
		if( SUCCEEDED(hr) && pColl){
			BOOL ret=FindSingleElements(selector,pColl,items,limitcount);
			pColl->Release();
			return ret;
		}
		return FALSE;
	}
	static BOOL FindSingleElements(THString selector,list<IHTMLElement *> *in,list<IHTMLElement *> *items,int limitcount=-1) {
		list<IHTMLElement *>::iterator it=in->begin();
		BSTR str;CString cstr;
		HRESULT hr;
		if (selector.Left(1)==_T("#")) {
			selector=selector.Mid(1);
			while(it!=in->end()) {
				cstr.Empty();
				hr=(*it)->get_id(&str);
				if (SUCCEEDED(hr)) cstr=str;
				if (cstr==selector && (limitcount==-1 || items->size()<(size_t)limitcount))
					items->push_back(*it);
				else
					(*it)->Release();
				it++;
			}
			in->clear();
			return TRUE;
		} else if (selector.Left(1)==_T(".")) {
			selector=selector.Mid(1);
			while(it!=in->end()) {
				cstr.Empty();
				hr=(*it)->get_className(&str);
				if (SUCCEEDED(hr)) cstr=str;
				if (cstr==selector && (limitcount==-1 || items->size()<(size_t)limitcount))
					items->push_back(*it);
				else
					(*it)->Release();
				it++;
			}
			in->clear();
			return TRUE;
		} else {
			IID iid;
			if (!FindIidByType(selector,iid)) return FALSE;
			BOOL bOk=FALSE;
			IDispatch *pDisp;
			while(it!=in->end()) {
				hr=(*it)->QueryInterface(iid,(void **)&pDisp);
				if (SUCCEEDED(hr) && pDisp) {
					bOk=TRUE;
					pDisp->Release();
				} else
					bOk=FALSE;
				if (bOk && (limitcount==-1 || items->size()<(size_t)limitcount))
					items->push_back(*it);
				else
					(*it)->Release();
				it++;
			}
			in->clear();
			return TRUE;
		}
		return FALSE;
	}
	static BOOL FindSingleElements(THString selector,IHTMLElementCollection* pColl,list<IHTMLElement *> *items,int limitcount=-1){
		if (selector.Left(1)==_T("#")) {
			long len = 0;
			if (!SUCCEEDED(pColl->get_length(&len))) return FALSE;
			COleVariant Elementname(selector.Mid(1));
			HRESULT hr;
			for (int i=0; i<len && items->size()!=limitcount; i++) {
				VARIANT Elementindex; 
				Elementindex.vt = VT_I4;
				Elementindex.intVal = i;
				IDispatch *disp = NULL;
				hr = pColl->item(Elementname, Elementindex,&disp);
				if (!SUCCEEDED(hr) || disp == NULL) return TRUE;
				if(disp) {
					IHTMLElement* pElem = NULL;
					hr = disp->QueryInterface(&pElem);
					if(SUCCEEDED(hr) && pElem) items->push_back(pElem);
					disp->Release();
				}
			}
			return TRUE;
		} else if (selector.Left(1)==_T(".")) {
			selector=selector.Mid(1);
			list<IHTMLElement *> tempitems;
			if (!FillCollectionToList(pColl,&tempitems,-1)) return FALSE;
			list<IHTMLElement *>::iterator it=tempitems.begin();
			HRESULT hr;
			BSTR clsName;
			CString cclsName;
			while(it!=tempitems.end()) {
				IHTMLElement *tmp=(*it);
				cclsName.Empty();
				hr=tmp->get_className(&clsName);
				if (SUCCEEDED(hr)) cclsName=clsName;
				if (cclsName==selector && (limitcount==-1 || items->size()<(size_t)limitcount))
					items->push_back(tmp);
				else
					tmp->Release();
				it++;
			}
			return TRUE;
		} else {
			IID iid;
			if (!FindIidByType(selector,iid)) return FALSE;
			long len = 0;
			if (!SUCCEEDED(pColl->get_length(&len))) return FALSE;
			HRESULT hr;
			for (int i=0; i<len && items->size()!=limitcount; i++) {
				VARIANT Elementindex; 
				Elementindex.vt = VT_I4;
				Elementindex.intVal = i;
				VARIANT Elementsubindex;
				Elementsubindex.vt = VT_I4;
				Elementsubindex.intVal = 0;
				IDispatch *disp = NULL;
				hr = pColl->item(Elementindex, Elementsubindex,&disp);
				if (!SUCCEEDED(hr) || disp == NULL) return TRUE;//finish
				if(disp) {
					IDispatch *pDisp;
					hr = disp->QueryInterface(iid,(void **)&pDisp);
					if(SUCCEEDED(hr) && pDisp) {
						IHTMLElement *pElem;
						hr = disp->QueryInterface(&pElem);
						if(SUCCEEDED(hr) && pElem)
							items->push_back(pElem);
						pDisp->Release();
					}
				}
			}
			return TRUE;
		}
		return FALSE;
	}
	static BOOL FindIidByType(THString selector,IID &iid){
		if (selector==_T("a")) iid=__uuidof(IHTMLAnchorElement);
		else if (selector==_T("area")) iid=__uuidof(IHTMLAreaElement);
		//else if (selector==_T("audio")) iid=__uuidof(IHTMLAudioElement);
		else if (selector==_T("body")) iid=__uuidof(IHTMLBodyElement);
		else if (selector==_T("button")) iid=__uuidof(IHTMLButtonElement);
		//else if (selector==_T("canvas")) iid=__uuidof(IHTMLCanvasElement);
		else if (selector==_T("comment")) iid=__uuidof(IHTMLCommentElement);
		else if (selector==_T("control")) iid=__uuidof(IHTMLControlElement);
		else if (selector==_T("dd")) iid=__uuidof(IHTMLDDElement);
		else if (selector==_T("dialog")) iid=__uuidof(IHTMLDialog);
		else if (selector==_T("div")) iid=__uuidof(IHTMLDivElement);
		else if (selector==_T("embed")) iid=__uuidof(IHTMLEmbedElement);
		else if (selector==_T("fieldset")) iid=__uuidof(IHTMLFieldSetElement);
		else if (selector==_T("font")) iid=__uuidof(IHTMLFontElement);
		else if (selector==_T("form")) iid=__uuidof(IHTMLFormElement);
		else if (selector==_T("frame")) iid=__uuidof(IHTMLFrameElement);
		else if (selector==_T("frameset")) iid=__uuidof(IHTMLFrameSetElement);
		else if (selector==_T("head")) iid=__uuidof(IHTMLHeadElement);
		else if (selector==_T("header")) iid=__uuidof(IHTMLHeaderElement);
		else if (selector==_T("hr")) iid=__uuidof(IHTMLHRElement);
		else if (selector==_T("html")) iid=__uuidof(IHTMLHtmlElement);
		else if (selector==_T("iframe")) iid=__uuidof(IHTMLIFrameElement);
		else if (selector==_T("img")) iid=__uuidof(IHTMLImgElement);
		else if (selector==_T("inputbutton")) iid=__uuidof(IHTMLInputButtonElement);
		else if (selector==_T("input")) iid=__uuidof(IHTMLInputElement);
		else if (selector==_T("inputfile")) iid=__uuidof(IHTMLInputFileElement);
		else if (selector==_T("inputhidden")) iid=__uuidof(IHTMLInputHiddenElement);
		//else if (selector==_T("inputrange")) iid=__uuidof(IHTMLInputRangeElement);
		else if (selector==_T("inputtext")) iid=__uuidof(IHTMLInputTextElement);
		else if (selector==_T("label")) iid=__uuidof(IHTMLLabelElement);
		else if (selector==_T("li")) iid=__uuidof(IHTMLLIElement);
		else if (selector==_T("link")) iid=__uuidof(IHTMLLinkElement);
		else if (selector==_T("list")) iid=__uuidof(IHTMLListElement);
		else if (selector==_T("map")) iid=__uuidof(IHTMLMapElement);
		else if (selector==_T("marquee")) iid=__uuidof(IHTMLMarqueeElement);
		//else if (selector==_T("media")) iid=__uuidof(IHTMLMediaElement);
		else if (selector==_T("meta")) iid=__uuidof(IHTMLMetaElement);
		else if (selector==_T("object")) iid=__uuidof(IHTMLObjectElement);
		else if (selector==_T("optionbutton")) iid=__uuidof(IHTMLOptionButtonElement);
		else if (selector==_T("option")) iid=__uuidof(IHTMLOptionElement);
		else if (selector==_T("p")) iid=__uuidof(IHTMLParaElement);
		else if (selector==_T("param")) iid=__uuidof(IHTMLParamElement);
		else if (selector==_T("script")) iid=__uuidof(IHTMLScriptElement);
		else if (selector==_T("select")) iid=__uuidof(IHTMLSelectElement);
		//else if (selector==_T("source")) iid=__uuidof(IHTMLSourceElement);
		else if (selector==_T("span")) iid=__uuidof(IHTMLSpanElement);
		else if (selector==_T("style")) iid=__uuidof(IHTMLStyleElement);
		else if (selector==_T("table")) iid=__uuidof(IHTMLTable);
		else if (selector==_T("tablecaption")) iid=__uuidof(IHTMLTableCaption);
		else if (selector==_T("td")) iid=__uuidof(IHTMLTableCell);
		else if (selector==_T("col")) iid=__uuidof(IHTMLTableCol);
		else if (selector==_T("tr")) iid=__uuidof(IHTMLTableRow);
		else if (selector==_T("textarea")) iid=__uuidof(IHTMLTextAreaElement);
		else if (selector==_T("text")) iid=__uuidof(IHTMLTextElement);
		else if (selector==_T("title")) iid=__uuidof(IHTMLTitleElement);
		//else if (selector==_T("video")) iid=__uuidof(IHTMLVideoElement);
		else return FALSE;
		return TRUE;
	}
	static BOOL AllElements(IHTMLDocument2 *pDoc,list<IHTMLElement *> *items,int limitcount=-1){
		IHTMLElementCollection* pColl = NULL; 
		HRESULT hr = pDoc->get_all(&pColl); 
		if( SUCCEEDED(hr) && pColl) {
			BOOL ret=FillCollectionToList(pColl,items,limitcount);
			pColl->Release();
			return ret;
		}
		return FALSE;
	}
	static BOOL FillCollectionToList(IHTMLElementCollection *pColl,list<IHTMLElement *> *items,int limitcount=-1) {
		long len = 0;
		if (!SUCCEEDED(pColl->get_length(&len))) return FALSE;
		HRESULT hr;
		for (int i=0; i<len && items->size()!=limitcount; i++) {
			VARIANT Elementindex; 
			Elementindex.vt = VT_I4;
			Elementindex.intVal = i;
			VARIANT Elementsubindex;
			Elementsubindex.vt = VT_I4;
			Elementsubindex.intVal = 0;
			IDispatch *disp = NULL;
			hr = pColl->item(Elementindex, Elementsubindex,&disp);
			if (!SUCCEEDED(hr) || disp == NULL) return TRUE;//finish
			if(disp) {
				IHTMLElement *pElem;
				hr = disp->QueryInterface(&pElem);
				if(SUCCEEDED(hr) && pElem)
					items->push_back(pElem);
			}
		}
		return TRUE;
	}
};
