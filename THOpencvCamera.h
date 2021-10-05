#pragma once

#define THSYSTEM_INCLUDE_DSHOW
#include <THSystem.h>
#include <THLibOpencv.h>

#define MYFREEMEDIATYPE(mt)	{if ((mt).cbFormat != 0)		\
					{CoTaskMemFree((PVOID)(mt).pbFormat);	\
					(mt).cbFormat = 0;						\
					(mt).pbFormat = NULL;					\
				}											\
				if ((mt).pUnk != NULL)						\
				{											\
					(mt).pUnk->Release();					\
					(mt).pUnk = NULL;						\
				}}

/**
* @brief OpenCV摄像头操作封装类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2010-05-03 新建类
*/
/**<pre>
用法：
</pre>*/
class THOpencvCamera
{
public:
	THOpencvCamera()
	{
		m_bConnected = FALSE;
		m_nWidth = 0;
		m_nHeight = 0;
		m_bLock = FALSE;
		m_bChanged = FALSE;
		m_pFrame = NULL;
		m_nBufferSize = 0;
		m_pNullFilter = NULL;
		m_pMediaEvent = NULL;
		m_pSampleGrabberFilter = NULL;
		m_pGraph = NULL;
		CoInitialize(NULL);
	}
	virtual ~THOpencvCamera()
	{
		CloseCamera();
		CoUninitialize();
	}

	//打开摄像头，nCamID指定打开哪个摄像头，取值可以为0,1,2,...
	//bDisplayProperties指示是否自动弹出摄像头属性页
	//nWidth和nHeight设置的摄像头的宽和高，如果摄像头不支持所设定的宽度和高度，则返回false
	BOOL OpenCamera(int nCamID, BOOL bDisplayProperties=TRUE, int nWidth=320, int nHeight=240)
	{
		HRESULT hr = S_OK;
		CoInitialize(NULL);
		// Create the Filter Graph Manager.
		hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC,IID_IGraphBuilder, (void **)&m_pGraph);
		hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER,IID_IBaseFilter, (LPVOID *)&m_pSampleGrabberFilter);
		hr = m_pGraph->QueryInterface(IID_IMediaControl, (void **) &m_pMediaControl);
		hr = m_pGraph->QueryInterface(IID_IMediaEvent, (void **) &m_pMediaEvent);
		hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER,IID_IBaseFilter, (LPVOID*) &m_pNullFilter);
		hr = m_pGraph->AddFilter(m_pNullFilter, L"NullRenderer");
		hr = m_pSampleGrabberFilter->QueryInterface(IID_ISampleGrabber, (void**)&m_pSampleGrabber);
		AM_MEDIA_TYPE mt;
		ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
		mt.majortype = MEDIATYPE_Video;
		mt.subtype = MEDIASUBTYPE_RGB24;
		mt.formattype = FORMAT_VideoInfo; 
		hr = m_pSampleGrabber->SetMediaType(&mt);
		MYFREEMEDIATYPE(mt);
		m_pGraph->AddFilter(m_pSampleGrabberFilter, L"Grabber");
		// Bind Device Filter.  We know the device because the id was passed in
		BindFilter(nCamID, &m_pDeviceFilter);
		m_pGraph->AddFilter(m_pDeviceFilter, NULL);
		CComPtr<IEnumPins> pEnum;
		m_pDeviceFilter->EnumPins(&pEnum);
		hr = pEnum->Reset();
		hr = pEnum->Next(1, &m_pCameraOutput, NULL); 
		pEnum = NULL; 
		m_pSampleGrabberFilter->EnumPins(&pEnum);
		pEnum->Reset();
		hr = pEnum->Next(1, &m_pGrabberInput, NULL); 
		pEnum = NULL;
		m_pSampleGrabberFilter->EnumPins(&pEnum);
		pEnum->Reset();
		pEnum->Skip(1);
		hr = pEnum->Next(1, &m_pGrabberOutput, NULL); 
		pEnum = NULL;
		m_pNullFilter->EnumPins(&pEnum);
		pEnum->Reset();
		hr = pEnum->Next(1, &m_pNullInputPin, NULL);
		//SetCrossBar();
		if (bDisplayProperties) 
		{
			CComPtr<ISpecifyPropertyPages> pPages;
			HRESULT hr = m_pCameraOutput->QueryInterface(IID_ISpecifyPropertyPages, (void**)&pPages);
			if (SUCCEEDED(hr))
			{
				PIN_INFO PinInfo;
				m_pCameraOutput->QueryPinInfo(&PinInfo);
				CAUUID caGUID;
				pPages->GetPages(&caGUID);
				OleCreatePropertyFrame(NULL, 0, 0,
							L"Property Sheet", 1,
							(IUnknown **)&(m_pCameraOutput.p),
							caGUID.cElems,
							caGUID.pElems,
							0, 0, NULL);
				CoTaskMemFree(caGUID.pElems);
				PinInfo.pFilter->Release();
			}
			pPages = NULL;
		}
		else 
		{
			//由 lWidth和lHeight设置的摄像头的宽和高，默认320*240
			int _Width = nWidth, _Height = nHeight;
			IAMStreamConfig* iconfig;
			iconfig = NULL;
			hr = m_pCameraOutput->QueryInterface(IID_IAMStreamConfig,(void**)&iconfig);
			AM_MEDIA_TYPE* pmt;
			if(iconfig->GetFormat(&pmt) !=S_OK) return FALSE;
			VIDEOINFOHEADER*   phead;
			if ( pmt->formattype == FORMAT_VideoInfo)
			{
				phead=( VIDEOINFOHEADER*)pmt->pbFormat;   
				phead->bmiHeader.biWidth = _Width;   
				phead->bmiHeader.biHeight = _Height;   
				if(( hr=iconfig->SetFormat(pmt)) != S_OK ) return FALSE;
			}
			iconfig->Release();   
			iconfig=NULL;   
			MYFREEMEDIATYPE(*pmt);
		}
		hr = m_pGraph->Connect(m_pCameraOutput, m_pGrabberInput);
		hr = m_pGraph->Connect(m_pGrabberOutput, m_pNullInputPin);
		if (FAILED(hr))
		{
			switch(hr)
			{
				case VFW_S_NOPREVIEWPIN :break;
				case E_FAIL :break;
				case E_INVALIDARG :break;
				case E_POINTER :break;
			}
		}
		m_pSampleGrabber->SetBufferSamples(TRUE);
		m_pSampleGrabber->SetOneShot(TRUE);
		hr = m_pSampleGrabber->GetConnectedMediaType(&mt);
		if(FAILED(hr)) return FALSE;

		VIDEOINFOHEADER *videoHeader;
		videoHeader = reinterpret_cast<VIDEOINFOHEADER*>(mt.pbFormat);
		m_nWidth = videoHeader->bmiHeader.biWidth;
		m_nHeight = videoHeader->bmiHeader.biHeight;
		m_bConnected = TRUE;
		pEnum = NULL;
		return TRUE;
	}

	//关闭摄像头，析构函数会自动调用这个函数
	void CloseCamera()
	{
		if(m_bConnected) m_pMediaControl->Stop();
		m_pGraph = NULL;
		m_pDeviceFilter = NULL;
		m_pMediaControl = NULL;
		m_pSampleGrabberFilter = NULL;
		m_pSampleGrabber = NULL;
		m_pGrabberInput = NULL;
		m_pGrabberOutput = NULL;
		m_pCameraOutput = NULL;
		m_pMediaEvent = NULL;
		m_pNullFilter = NULL;
		m_pNullInputPin = NULL;
		if (m_pFrame) cvReleaseImage(&m_pFrame);
		m_bConnected = FALSE;
		m_nWidth = 0;
		m_nHeight = 0;
		m_bLock = FALSE;
		m_bChanged = FALSE;
		m_nBufferSize = 0;
	}

	//返回摄像头的数目
	static int GetCameraCount()
	{
		int count = 0;
 		CoInitialize(NULL);
		// enumerate all video capture devices
		CComPtr<ICreateDevEnum> pCreateDevEnum;
		HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,IID_ICreateDevEnum, (void**)&pCreateDevEnum);
		CComPtr<IEnumMoniker> pEm;
		hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,&pEm, 0);
		if (hr != NOERROR) return count;
		pEm->Reset();
		ULONG cFetched;
		IMoniker *pM;
		while(hr = pEm->Next(1, &pM, &cFetched), hr==S_OK) count++;
		pCreateDevEnum = NULL;
		pEm = NULL;
		return count;
	}

	//根据摄像头的编号返回摄像头的名字
	//nCamID: 摄像头编号
	//sName: 用于存放摄像头名字的数组
	//nBufferSize: sName的大小
	static CString GetCameraName(int nCamID)
	{
		int count = 0;
		CString name;
		CoInitialize(NULL);
		// enumerate all video capture devices
		CComPtr<ICreateDevEnum> pCreateDevEnum;
		HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,IID_ICreateDevEnum, (void**)&pCreateDevEnum);
		CComPtr<IEnumMoniker> pEm;
		hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,&pEm, 0);
		if (hr != NOERROR) return name;
		pEm->Reset();
		ULONG cFetched;
		IMoniker *pM;
		while(hr = pEm->Next(1, &pM, &cFetched), hr==S_OK)
		{
			if (count == nCamID)
			{
				IPropertyBag *pBag=0;
				hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
				if(SUCCEEDED(hr))
				{
					VARIANT var;
					var.vt = VT_BSTR;
					hr = pBag->Read(L"FriendlyName", &var, NULL); //还有其他属性,像描述信息等等...
					if(hr == NOERROR)
					{
						//获取设备名称
						name=CString(var.bstrVal);
						SysFreeString(var.bstrVal);				
					}
					pBag->Release();
				}
				pM->Release();

				break;
			}
			count++;
		}
		pCreateDevEnum = NULL;
		pEm = NULL;
		return name;
	}

	//返回图像宽度
	int GetWidth(){return m_nWidth;} 

	//返回图像高度
	int GetHeight(){return m_nHeight;}

	//抓取一帧，返回的IplImage不可手动释放！
	//返回图像数据的为RGB模式的Top-down(第一个字节为左上角像素)，即IplImage::origin=0(IPL_ORIGIN_TL)
	IplImage * QueryFrame()
	{
		long evCode;
		long size = 0;
		m_pMediaControl->Run();
		m_pMediaEvent->WaitForCompletion(INFINITE, &evCode);
		m_pSampleGrabber->GetCurrentBuffer(&size, NULL);
		//if the buffer size changed
		if (size != m_nBufferSize)
		{
			if (m_pFrame)
				cvReleaseImage(&m_pFrame);
			m_nBufferSize = size;
			m_pFrame = cvCreateImage(cvSize(m_nWidth, m_nHeight), IPL_DEPTH_8U, 3);
		}
		m_pSampleGrabber->GetCurrentBuffer(&m_nBufferSize, (long*)m_pFrame->imageData);
		cvFlip(m_pFrame);
		return m_pFrame;
	}
private:
	BOOL BindFilter(int nCamID, IBaseFilter **pFilter)
	{
		if (nCamID < 0) return FALSE; 
		// enumerate all video capture devices
		CComPtr<ICreateDevEnum> pCreateDevEnum;
		HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,IID_ICreateDevEnum, (void**)&pCreateDevEnum);
		if (hr != NOERROR) return FALSE;
		CComPtr<IEnumMoniker> pEm;
		hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEm, 0);
		if (hr != NOERROR) return FALSE;
		pEm->Reset();
		ULONG cFetched;
		IMoniker *pM;
		int index = 0;
		while(hr = pEm->Next(1, &pM, &cFetched), hr==S_OK, index <= nCamID)
		{
			IPropertyBag *pBag;
			hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
			if(SUCCEEDED(hr)) 
			{
				VARIANT var;
				var.vt = VT_BSTR;
				hr = pBag->Read(L"FriendlyName", &var, NULL);
				if (hr == NOERROR) 
				{
					if (index == nCamID)
						pM->BindToObject(0, 0, IID_IBaseFilter, (void**)pFilter);
					SysFreeString(var.bstrVal);
				}
				pBag->Release();
			}
			pM->Release();
			index++;
		}
		pCreateDevEnum = NULL;
		return TRUE;
	}
	//将输入crossbar变成PhysConn_Video_Composite
	void SetCrossBar()
	{
		int i;
		IAMCrossbar *pXBar1 = NULL;
		ICaptureGraphBuilder2 *pBuilder = NULL;
		HRESULT hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL,CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void **)&pBuilder);
		if (SUCCEEDED(hr))
			hr = pBuilder->SetFiltergraph(m_pGraph);
		hr = pBuilder->FindInterface(&LOOK_UPSTREAM_ONLY, NULL, m_pDeviceFilter,IID_IAMCrossbar, (void**)&pXBar1);
		if (SUCCEEDED(hr)) 
		{
  			long OutputPinCount;
			long InputPinCount;
			long PinIndexRelated;
			long PhysicalType;
			long inPort = 0;
			long outPort = 0;
			pXBar1->get_PinCounts(&OutputPinCount,&InputPinCount);
			for( i =0;i<InputPinCount;i++)
			{
				pXBar1->get_CrossbarPinInfo(TRUE,i,&PinIndexRelated,&PhysicalType);
				if(PhysConn_Video_Composite==PhysicalType) 
				{
					inPort = i;
					break;
				}
			}
			for( i =0;i<OutputPinCount;i++)
			{
				pXBar1->get_CrossbarPinInfo(FALSE,i,&PinIndexRelated,&PhysicalType);
				if(PhysConn_Video_VideoDecoder==PhysicalType) 
				{
					outPort = i;
					break;
				}
			}
			if(S_OK==pXBar1->CanRoute(outPort,inPort))
				pXBar1->Route(outPort,inPort);
			pXBar1->Release();  
		}
		pBuilder->Release();
	}

	IplImage * m_pFrame;
	BOOL m_bConnected;
	int m_nWidth;
	int m_nHeight;
	BOOL m_bLock;
	BOOL m_bChanged;
	long m_nBufferSize;

	CComPtr<IGraphBuilder> m_pGraph;
	CComPtr<IBaseFilter> m_pDeviceFilter;
	CComPtr<IMediaControl> m_pMediaControl;
	CComPtr<IBaseFilter> m_pSampleGrabberFilter;
	CComPtr<ISampleGrabber> m_pSampleGrabber;
	CComPtr<IPin> m_pGrabberInput;
	CComPtr<IPin> m_pGrabberOutput;
	CComPtr<IPin> m_pCameraOutput;
	CComPtr<IMediaEvent> m_pMediaEvent;
	CComPtr<IBaseFilter> m_pNullFilter;
	CComPtr<IPin> m_pNullInputPin;
};
