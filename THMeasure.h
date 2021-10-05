#pragma once

#define THTwip2Pixel(twip)	THMeasure::Twip2Pixel(twip)
#define THPixel2Twip(pixel)	THMeasure::Pixel2Twip(twip)

/**
* @brief 度量单位转换类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-07-17 新建类
*/
/**<pre>
使用Sample：
</pre>*/
class THMeasure
{
public:
	static inline int Twip2Pixel(int nTwip){return nTwip/20;}
	static inline int Pixel2Twip(int nPixel){return nPixel*20;}
};

typedef CRect THRect;
typedef CPoint THPoint;
class THSize : public CSize
{
public:
	THString Print(){return THFormat(_T("%d*%d"),cx,cy);}
};