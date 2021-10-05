
#pragma once
#include <THString.h>
#include <THStlEx.h>
#include <THLibOpencv.h>

/**
* @brief OpenCV常用操作封装类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2010-05-03 新建类
*/
/**<pre>
用法：
	IplImage *img=cvLoadImage(src);
	IplImage *out=THOpencvMisc::GetCannyImg(img);
	cvNamedWindow("Src");
	cvShowImage("Src",img);
	cvNamedWindow("Result");
	cvShowImage("Result",out);
	cvReleaseImage(&out);
	cvReleaseImage(&img);
</pre>*/
class THOpencvMisc
{
public:
	//缩放图像
	static IplImage* GetScaleImg(IplImage* src,int w,int h)
	{
		if (!src) return NULL;
		CvSize dst_cvsize;
		float scale = 2;
		dst_cvsize.width = w;
		dst_cvsize.height = h;
		IplImage *dst=cvCreateImage(dst_cvsize, src->depth, src->nChannels);
		cvResize(src, dst, CV_INTER_LINEAR);
		return dst;
	}
	//缩放图像
	static IplImage* GetScaleImg(IplImage* src,float scale)
	{
		if (!src) return NULL;
		return GetScaleImg(src,int(src->width * scale),int(src->height * scale));
	}
	//旋转图像
	static IplImage* GetAngleImg(IplImage* src,int angle)
	{
		if (!src) return NULL;
		float m[6];
		CvMat M = cvMat (2, 3, CV_32F, m);
		m[0] = (float) (cos(-angle * CV_PI / 180.));
		m[1] = (float) (sin(-angle * CV_PI / 180.));
		m[3] = -m[1];
		m[4] = m[0];
		//旋转中心 图像中间
		m[2] = src->width * 0.5f;
		m[5] = src->height * 0.5f;
		IplImage* dst = cvCloneImage (src);
		cvGetQuadrangleSubPix (src, dst, &M);
		return dst;
	}
	//灰度图
	static IplImage* GetGrayImg(IplImage* src)
	{
		if (!src) return NULL;
		IplImage* gray;
		gray=cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
		cvCvtColor(src, gray, CV_BGR2GRAY);
		return gray;
	}
	//边缘检测
	static IplImage* GetCannyImg(IplImage *src)
	{
		if (!src) return NULL;
		IplImage *gray=GetGrayImg(src);
		IplImage* canny = cvCreateImage(cvGetSize(gray),IPL_DEPTH_8U,1);
		cvCanny(gray, canny, 50, 150, 3);
		cvReleaseImage(&gray);
		return canny;
	}
	//轮廓检测
	static IplImage *GetContourImg(IplImage *src,CvScalar extclr=CV_RGB(0,0,255),CvScalar holeclr=CV_RGB(255, 0, 0))
	{
		if (!src) return NULL;
		CvMemStorage * storage = cvCreateMemStorage(0);
		IplImage *gray=GetGrayImg(src);
		IplImage *out=cvCreateImage(cvGetSize(gray),IPL_DEPTH_8U,3);
		cvCvtColor(gray,out,CV_GRAY2BGR);
		CvSeq *seq;
		int mode = CV_RETR_TREE;//CV_RETR_TREE:，CV_RETR_CCOMP:全检测，CV_RETR_EXTERNAL:外检测
		cvFindContours( gray, storage, &seq, sizeof(CvContour), mode, CV_CHAIN_APPROX_SIMPLE);
		//序列转换为图
		cvDrawContours(out, seq, extclr, holeclr, 2, 2, 8);
		cvReleaseImage(&gray);
		cvReleaseMemStorage(&storage);
		return out;
	}
	//线段检测
	static IplImage *GetHoughImg(IplImage *src,BOOL bFullLine=FALSE)
	{
		if (!src) return NULL;
		CvMemStorage * storage = cvCreateMemStorage(0);
		IplImage *gray=GetGrayImg(src);
		IplImage *out=cvCreateImage(cvGetSize(gray),IPL_DEPTH_8U,1);
		IplImage *color_out=cvCreateImage(cvGetSize(gray),IPL_DEPTH_8U,3);
		cvCanny( gray, out, 50, 200, 3);
		cvCvtColor( out, color_out, CV_GRAY2BGR );
		CvSeq *seq;
		//CV_HOUGH_STANDARD:基本，CV_HOUGH_PROBABILISTIC:
		if (bFullLine)
		{
			seq = cvHoughLines2( out, storage, CV_HOUGH_STANDARD, 1, CV_PI/180, 100, 0, 0 );
			int max=seq->total;
			if (max>100) max=100;
			for(int i = 0; i < max; i++ )
			{
				float* line = (float*)cvGetSeqElem(seq,i);
				float rho = line[0];
				float theta = line[1];
				CvPoint pt1, pt2;
				double a = cos(theta), b = sin(theta);
				double x0 = a*rho, y0 = b*rho;
				pt1.x = cvRound(x0 + 1000*(-b));
				pt1.y = cvRound(y0 + 1000*(a));
				pt2.x = cvRound(x0 - 1000*(-b));
				pt2.y = cvRound(y0 - 1000*(a));
				cvLine( color_out, pt1, pt2, CV_RGB(255,0,0), 3, CV_AA, 0 );
			}
		}
		else
		{
			seq = cvHoughLines2( out, storage, CV_HOUGH_PROBABILISTIC, 1, CV_PI/180, 50, 50, 10 );
			for(int i = 0; i < seq->total; i++ )
			{
				CvPoint* line = (CvPoint*)cvGetSeqElem(seq,i);
				cvLine( color_out, line[0], line[1], CV_RGB(255,0,0), 3, CV_AA, 0 );
			}
		}
		cvReleaseImage(&out);
		cvReleaseImage(&gray);
		cvReleaseMemStorage(&storage);
		return color_out;
	}
	//角点检测
	static IplImage* GetCornerHarrisImg(IplImage* src)
	{
		if (!src) return NULL;
		IplImage *gray=GetGrayImg(src);
		IplImage *harris=cvCreateImage(cvGetSize(gray),IPL_DEPTH_32F, 1);
		uchar *data_harris=(uchar*)harris->imageData;
		memset(data_harris,255,harris->imageSize);
		cvCornerHarris(gray ,harris, 3, 3, 0.04);
		cvReleaseImage(&gray);
		return harris;
	}
	static void FreeImg(IplImage **img){if (*img) cvReleaseImage(img);}
};

/**
* @brief OpenCV画图类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2010-05-03 新建类
*/
/**<pre>
用法：
</pre>*/
class THOpencvDraw
{
public:
	THOpencvDraw(){}
	virtual ~THOpencvDraw(){}

	//设置线段的标题，由于Opencv问题只支持英文，网上有用FreeType实现的中文支持，比较麻烦尚未实现
	BOOL SetName(int lines,THString name=_T(""))
	{
		map<int,LineInfo>::iterator it;
		it=m_lines.find(lines);
		if (it!=m_lines.end())
		{
			it->second.name=name;
			return TRUE;
		}
		return FALSE;
	}

	//增加一个点，lines为线段id，x坐标重复的话会覆盖原来添加的值
	void AddPoints(double x,double y,int lines=0)
	{
		map<int,LineInfo>::iterator it;
		it=m_lines.find(lines);
		if (it==m_lines.end())
		{
			LineInfo info;
			m_lines.insert(map<int,LineInfo>::value_type(lines,info));
			it=m_lines.find(lines);
			it->second.name.Format(_T("Line%d"),lines+1);
		}
		LineInfo &info=it->second;
		if (info.minx>x) info.minx=x;
		if (info.miny>y) info.miny=y;
		if (info.maxx<x) info.maxx=x;
		if (info.maxy<y) info.maxy=y;
		info.points[x]=y;
	}
	//如果指定高宽，生成指定大小图片，否则根据
	IplImage* GetImg(int w=500,int h=500)
	{
		/*
			0,0
			 10,0
			500,0
		*/
#define BASELINE	0
		CvSize size;
		size.width=w;
		size.height=h;
		IplImage *out=cvCreateImage(size,IPL_DEPTH_32F, 3);
		map<int,LineInfo>::iterator it;
		map<double,double>::iterator dit,lit;
		int lines;
		for(it=m_lines.begin();it!=m_lines.end();it++)
		{
			lines=it->first;
			LineInfo &info=it->second;
			map<double,double> &dmap=info.points;
			double xscale=w/(info.maxx-info.minx);
			double yscale=h/(info.maxy-info.miny);
			lit=dmap.end();
			for(dit=dmap.begin();dit!=dmap.end();dit++)
			{
				if (lit!=dmap.end())
				{
					//THDebug(_T("%f,%f,%d,%d,%d,%d"),lit->first,m_minx,(int)((lit->first-m_minx)*xscale),(int)((lit->second-m_miny)*yscale),(int)((dit->first-m_minx)*xscale),(int)((dit->second-m_miny)*yscale));
					cvLine(out,cvPoint((int)((lit->first-info.minx)*xscale),h-(int)((lit->second-info.miny)*yscale)),cvPoint((int)((dit->first-info.minx)*xscale),h-(int)((dit->second-info.miny)*yscale)),GetColor(lines));
					//cvLine(out,cvPoint((int)((m_maxx-lit->first)*xscale),(int)((m_maxy-lit->second)*yscale)),cvPoint((int)((m_maxx-dit->first)*xscale),(int)((m_maxy-dit->second)*yscale)),CV_RGB(255,0,255));
				}
				lit=dit;
			}
			CvFont font;
			cvInitFont(&font,CV_FONT_HERSHEY_PLAIN,1.0,1.0,0,1);
			cvPutText(out,(LPCSTR)THStringA(THf2sc(info.minx)+_T(" ")+info.name),cvPoint(0,h/2-50+(lines*20)),&font,GetColor(lines));
			cvPutText(out,(LPCSTR)THStringA(THf2sc(info.maxx)),cvPoint(w-100,h/2-50+(lines*20)),&font,GetColor(lines));
			cvPutText(out,(LPCSTR)THStringA(THf2sc(info.maxy)),cvPoint(0,20+(lines*20)),&font,GetColor(lines));
			cvPutText(out,(LPCSTR)THStringA(THf2sc(info.miny)),cvPoint(0,h-10-(lines*20)),&font,GetColor(lines));
		}
		return out;
	}
	void ShowImgWindow(int w=500,int h=500)
	{
		cvNamedWindow("THOpencvDraw");
		IplImage *img=GetImg(w,h);
		cvShowImage("THOpencvDraw",img);
		cvReleaseImage(&img);
	}
	void SaveFile(THString file,int w=500,int h=500)
	{
		IplImage *img=GetImg(w,h);
		cvSaveImage((LPCSTR)THStringA(file),img);
		cvReleaseImage(&img);
	}
	CvScalar GetColor(int lines)
	{
		switch(lines%10)
		{
			case 1:return CV_RGB(255,0,0);
			case 2:return CV_RGB(0,255,0);
			case 3:return CV_RGB(0,0,255);
			case 4:return CV_RGB(255,255,0);
			case 5:return CV_RGB(255,0,255);
			case 6:return CV_RGB(0,255,255);
		}
		return CV_RGB(255,255,255);
	}
protected:
	typedef struct _LineInfo{
		double minx;
		double miny;
		double maxx;
		double maxy;
		map<double,double> points;
		THString name;
		_LineInfo()
		{
			minx=DBL_MAX;
			miny=DBL_MAX;
			maxx=DBL_MIN;
			maxy=DBL_MIN;
		}
	}LineInfo;
	//lines,x,y
	map<int,LineInfo> m_lines;
};
