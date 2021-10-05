#pragma once

/**
* @brief Flash 基本函数封装类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2009-09-08 新建类
*/
/**<pre>
  使用Sample：
</pre>*/
class THFlashBase
{
public:
	THFlashBase(){}
	virtual ~THFlashBase(){}

	inline static U32 _GetU24Length(const U8 *buf)
	{
		return (U32)(buf[0]*0x10000+buf[1]*0x100+buf[2]);
	}

	inline static void _SetU24Length(U8 *size,U32 nLen)
	{
		*size=(U8)((nLen/0x10000)%0x100);size++;
		*size=(U8)((nLen/0x100)%0x100);size++;
		*size=(U8)(nLen%0x100);
	}

	inline static U32 _GetU32Length(const U8 *buf)
	{
		return (U32)(buf[3]*0x1000000+buf[0]*0x10000+buf[1]*0x100+buf[2]);
	}

	inline static void _SetU32Length(U8 *size,U32 nLen)
	{
		*size=(U8)((nLen/0x10000)%0x100);size++;
		*size=(U8)((nLen/0x100)%0x100);size++;
		*size=(U8)(nLen%0x100);size++;
		*size=(U8)((nLen/0x1000000)%0x100);
	}
};
