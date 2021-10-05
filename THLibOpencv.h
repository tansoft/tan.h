#pragma once

/**
* @brief OpenCV封装
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2010-05-03 新建类
*/

#include <opencvlib\include\cv.h>
#include <opencvlib\include\cxcore.h>
#include <opencvlib\include\highgui.h>
#ifdef _DEBUG
	#pragma message("link opencv debug lib,dont forget opencv*d.dll and vc90 redist")
	#pragma comment(lib,"highgui200d.lib")
	#pragma comment(lib,"cxcore200d.lib")
	#pragma comment(lib,"cv200d.lib")
	#pragma comment(lib,"cvaux200d.lib")
	#pragma comment(lib,"cxts200d.lib")
	#pragma comment(lib,"ml200d.lib")
#else
	#pragma message("link opencv lib,dont forget opencv*.dll and vc90 redist")
	#pragma comment(lib,"highgui200.lib")
	#pragma comment(lib,"cxcore200.lib")
	#pragma comment(lib,"cv200.lib")
	#pragma comment(lib,"cvaux200.lib")
	#pragma comment(lib,"cxts200.lib")
	#pragma comment(lib,"ml200.lib")
#endif
