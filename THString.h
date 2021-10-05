#pragma once

#if defined(_ATL_VER)
	#include <atlstr.h>
	#pragma message("CString mapping to ATL")
	#define USE_THSTRING_IMPL 0
	#define USE_THSTRINGARRAY_IMPL 1
#elif defined(_MFC_VER)
	#include <afxstr.h>
	#include <afxcoll.h>
	#pragma message("CString mapping to MFC")
	#define USE_THSTRING_IMPL 0
	#define USE_THSTRINGARRAY_IMPL 0
#else
	#ifndef ATLASSERT
		#define	ATLASSERT
	#endif
	#ifndef ASSERT
		#define ASSERT
	#endif
	#ifndef VERIFY
		#define VERIFY
	#endif
	#include <THStringImpl.h>
	#include <THStringArrayImpl.h>
	#pragma message("CString mapping to OwnImp")
	#define USE_THSTRING_IMPL 1
	#define USE_THSTRINGARRAY_IMPL 1
#endif

typedef CString THString;
typedef CStringA THStringA;
typedef CStringW THStringW;

#define THSYSTEM_INCLUDE_WINSOCK
#include <THSystem.h>
/*
class THString : public CString
{
};

class THStringA : public CStringA
{
};

class THStringW : public CStringW
{
};*/

//fixme
class THStringArray : public CStringArray
{
public:
	THString Print(THString split=_T("[key]:[value]\r\n"))
	{
		THString ret;
		THString tmp,tmp1;
		for(INT_PTR i=0;i<GetSize();i++)
		{
			tmp=split;
			tmp1.Format(_T("%d"),i);
			tmp.Replace(_T("[key]"),tmp1);
			tmp.Replace(_T("[value]"),GetAt(i));
			ret+=tmp;
		}
		return ret;
	}
	THString toString(THString token=_T("\r\n"))
	{
		THString str;
		for(INT_PTR i=0;i<GetSize();i++)
			str+=GetAt(i)+token;
		return str;
	}
};

/**
* @brief 字符串切割类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-06-04 新建类
*/
/**<pre>
用法：
	THStringToken str(_T("qwe 3213 3213"));
	while(str.IsMoreTokens())
	{
		THDebug(_T("Token:%s"),str.GetNextToken());
	}
注意：该类效率不高，尤其GetAmount函数
</pre>*/
class THStringToken
{
public:
	THStringToken()
	{
		m_nAmount = 0;
		m_nCurrent = 0;
	}
	THStringToken(THString str,THString token=_T(" "))
	{
		Init(str,token);
	}
	virtual ~THStringToken(){}
	static void FillStringArray(THStringArray &ar,THString str,THString token=_T(" "))
	{
		THStringToken t(str,token);
		t.FillStringArray(ar);
	}
	void FillStringArray(THStringArray &ar)
	{
		while(IsMoreTokens())
		{
			THString ret=GetNextToken();
			if (!ret.IsEmpty()) ar.Add(ret);
		}
	}
	void Init(THString str,THString token=_T(" "))
	{
		str.Trim();
		m_strToToken = str;
		m_strLeft = str;
		m_strToken = token;
		m_nAmount = GetAmount(str);
		m_nCurrent = 0;
	}
	virtual THString GetNextToken()
	{
		if(!IsMoreTokens()) return _T("");
		THString ret;
		int pos, len;
		len = m_strLeft.GetLength();
		pos = m_strLeft.Find(m_strToken,0);
		if (pos != -1)
		{
			ret = m_strLeft.Left(pos);
			pos++;
			m_strLeft = m_strLeft.Right(len - pos - m_strToken.GetLength() +1);
			//m_strLeft.TrimLeft();
		}
		else ret = m_strLeft;
		m_nCurrent++;
		return ret;
	}
	virtual BOOL IsMoreTokens()
	{
		//return (m_strLeft.IsEmpty()==FALSE);}
		if (m_nCurrent < m_nAmount)
			return TRUE;
		else return FALSE;
	}
	virtual int GetRemainTokensCount()
	{
		return m_nAmount-m_nCurrent;
	}
	virtual THString GetContent(){return m_strToToken;}
	virtual THString GetRemainContent(){return m_strLeft;}
	int m_nAmount;
	int m_nCurrent;
protected:
	THString m_strToToken;
	THString m_strLeft;
	THString m_strToken;
private: 
	int GetAmount(THString str)
	{
		int count = 0;
		int pos,len;
		len = str.GetLength();
		pos = str.Find(m_strToken,0);
		while (pos != -1)
		{
			count++;
			pos++;
			str = str.Right(len - pos);
			//str.TrimLeft();
			len = str.GetLength();
			pos = str.Find(m_strToken,0);
		}
		if (!str.IsEmpty()) count++;
		return count;
	}
};

#include <THCharset.h>

#define THFormat	THStringConv::Format
#define THi2s		THStringConv::i2s
#define THs2i		THStringConv::s2i
#define THu2s		THStringConv::u2s
#define THs2u		THStringConv::s2u
#define THs2f		THStringConv::s2f
#define THf2s		THStringConv::f2s
#define THf2sc		THStringConv::f2sc
#define THfsc		THStringConv::fsc
#define THStrOrNull(str)	((str).IsEmpty()?NULL:(LPCTSTR)(str))
#define THStrRepeat	THStringConv::StrRepeat

/**
* @brief 字符串转换类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-06-29 新建类
*/
/**<pre>
用法：
	THString str=THStringConv::i2s(10);

	THString ret=THStringConv::FormatString(_T("\x11\xdd\x1d\xff\x1a\x00\x01\x41"),8,THStringConv::FormatType_Hex,8,THStringConv::FormatType_Hex,TRUE,TRUE);
	THDebug(ret);
	ret=THStringConv::FormatString(_T("\x11\xdd\x1d\xff\x1a\x00\x01\x41"),8,THStringConv::FormatType_Dec,4,THStringConv::FormatType_Dec,TRUE,FALSE);
	THDebug(ret);
	ret=THStringConv::FormatString(_T("\x11\xdd\x1d\xff\x1a\x00\x01\x41"),8,THStringConv::FormatType_Oct,7,THStringConv::FormatType_None,TRUE,TRUE);
	THDebug(ret);
	ret=THStringConv::FormatString(_T("\x11\xdd\x1d\xff\x1a\x00\x01\x41"),8,THStringConv::FormatType_Bin,8,THStringConv::FormatType_Hex,FALSE);
	THDebug(ret);
	//原始数据打印模式
	ret=THStringConv::FormatString(data,len,THStringConv::FormatType_None,100,THStringConv::FormatType_None,FALSE,TRUE);
	THLongDebug(ret);
</pre>*/
class THStringConv
{
public:
	//查找字符串中，出现配对""中的字符串，进行替换
	static THString ChangeStringValue(THString orgstr,THString findstr,THString replacestr)
	{
		THString ret;
		while(!orgstr.IsEmpty())
		{
			int pos=orgstr.Find(_T("\""));
			if (pos==-1)
			{
				ret+=orgstr;
				orgstr.Empty();
			}
			else
			{
				int pos2=orgstr.Find(_T("\""),pos+1);
				if (pos2==-1)
				{
					ret+=orgstr;
					orgstr.Empty();
				}
				else
				{
					ret+=orgstr.Left(pos+1);
					THString tmp;
					tmp=orgstr.Left(pos2).Mid(pos+1);
					tmp.Replace(findstr,replacestr);
					ret+=tmp;
					ret+=_T("\"");
					orgstr=orgstr.Mid(pos2+1);
				}
			}
		}
		return ret;
	}

	//统计字符串出现的次数
	static UINT FindStringCount(THString orgstr,THString findstr,BOOL bCase=TRUE,int iStart=0)
	{
		if (bCase==FALSE)
		{
			orgstr.MakeLower();
			findstr.MakeLower();
		}
		int i=0;
		int len=findstr.GetLength();
		while((iStart=orgstr.Find(findstr,iStart))!=-1)
		{
			i++;
			iStart+=len;
		}
		return i;
	}

	static THString u2sunit(UINT size)
	{
		THString str;
		if (size<1000)
			str.Format(_T("%d B"),size);
		else if (size<1000*1000)
			str.Format(_T("%0.2f KB"),size/1000.0);
		else if (size<1000*1000*1000)
			str.Format(_T("%0.2f MB"),size/1000.0/1000.0);
		else
			str.Format(_T("%0.2f GB"),size/1000.0/1000.0/1000.0);
		str.Replace(_T(".00"),_T(""));
		return str;
	}

	/**
	* @brief 格式化字符串为一行显示
	* @param str	原字符串
	* @return 返回限制后的字符串
	*/
	static THString FormatStringToSingleLine(THString str)
	{
		str.Replace(_T("\r"),_T("\n"));
		str.Replace(_T("\n\n"),_T("\n"));
		str.Replace(_T("\n"),_T(" "));
		str.Replace(_T("\t"),_T(" "));
		str.Trim();
		return str;
	}

	/**
	* @brief 解码js中用到的\uf5d1\ud2fc编码
	* @param sBuf	原字符串
	* @return 返回字符串
	*/
	static THString DecodeUString(THString sBuf)
	{
		int sp=0;
		int pos;
		int add;
		U16 buf[2];
		THString hbuf;
		unsigned int len;
		while((pos=sBuf.Find(_T("\\"),sp))!=-1)
		{
			TCHAR ch=sBuf.GetAt(pos+1);
			if (ch=='u' || ch=='U')
			{
				hbuf=sBuf.Mid(pos+2).Left(4);
				len=4;
				THStringConv::HexStringToBin(hbuf,buf,&len);
				buf[0]=ntohs(buf[0]);
				THString text=THCharset::w2t((const wchar_t *)buf,2);
				add=0;
				sBuf=sBuf.Left(pos)+text+sBuf.Mid(pos+6);
			}
			else add=1;
			sp=pos+add;
		}
		sBuf.Replace(_T("\\\\"),_T("\\"));
		return sBuf;
	}

	/**
	* @brief 编码js中的\uf5d1\ud2fc编码
	* @param sBuf	原字符串
	* @return 返回字符串
	*/
	static THString EncodeUString(THString sBuf)
	{
		sBuf.Replace(_T("\\"),_T("\\\\"));
		THString str;
		wchar_t *outbuf=THCharset::t2w(sBuf);
		if (!outbuf) return _T("");
		UINT len=(UINT)wcslen(outbuf);
		for(UINT i=0;i<len;i++)
		{
			wchar_t ch=*(outbuf+i);
			if (!((ch>='0' && ch<='9') ||
				(ch>='A' && ch<='Z') ||
				(ch>='a' && ch<='z')))
				str.AppendFormat(_T("\\u%04x"),ch);
			else
				str.AppendFormat(_T("%c"),ch);
		}
		THCharset::free(outbuf);
		return str;
	}

	/**
	* @brief 获取字符串大小，大小单位为英文字母大小，中文字计为2个(未验证程序)
	* @return 返回大小
	*/
	static UINT GetStringSize(THString ret)
	{
#ifndef _UNICODE
		return ret.GetLength();
#endif
		UINT cnt=0;
		for(int i=0;i<ret.GetLength();i++)
		{
			int val=ret.GetAt(i);
			if (val>0)
				cnt++;
			else
				cnt+=2;
		}
		return cnt;
	}

	/**
	* @brief 获取字符串字符数，中文字计为1个(未验证程序)
	* @return 返回大小
	*/
	static UINT GetStringCount(THString ret)
	{
#ifdef _UNICODE
		return ret.GetLength();
#endif
		UINT cnt=0;
		for(int i=0;i<ret.GetLength();i++)
		{
			int val=ret.GetAt(i);
			if (val>0)
				cnt++;
			else
			{
				cnt++;
				i++;
			}
		}
		return cnt;
	}

	/**
	* @brief 限制字符串大小，支持中文字处理，返回确保整个字符串占用大小不大于limitsize个英文字母大小
	* @param ret			原字符串
	* @param limitsize		限制大小
	* @param bEndEllipsis	截短时是否在最后增加...
	* @return 返回限制后的字符串
	*/
	static THString LimitStringSize(THString ret,int limitsize,BOOL bEndEllipsis=FALSE)
	{
		if (bEndEllipsis)
		{
			THString tmp=LimitStringSize(ret,limitsize,FALSE);
			if (ret==tmp) return ret;
			//must add the end ellipsis
			if (limitsize<=3) return _T("");
			tmp=LimitStringSize(ret,limitsize-3,FALSE)+_T("...");
			return tmp;
		}
#ifdef _UNICODE
		//unicode模式下不存在问题
		return ret.Left(limitsize);
#endif
		if (ret.GetLength()<limitsize) return ret;
		int ch=ret.GetAt(limitsize);
		if (ch>0) return ret.Left(limitsize);
		int i=limitsize-1;
		while(ch<0 && i>=0)
		{
			ch=ret.GetAt(i);
			if (ch>0 || i==0)
			{
				if (i!=0) i--;
				if ((limitsize-i)%2)
					return ret.Left(limitsize-1);
				else
					return ret.Left(limitsize);
			}
			i--;
		}
		return _T("");
	}

	/**
	* @brief 限制字符串字数，支持中文字处理，返回确保整个字符串字符个数为limitsize，一个中文字计为1个
	* @param ret			原字符串
	* @param limitsize		限制大小
	* @param bEndEllipsis	截短时是否在最后增加...
	* @return 返回限制后的字符串
	*/
	static THString LimitStringCount(THString ret,int limitsize,BOOL bEndEllipsis=FALSE)
	{
		if (bEndEllipsis)
		{
			THString tmp=LimitStringCount(ret,limitsize,FALSE);
			if (ret==tmp) return ret;
			//must add the end ellipsis
			if (limitsize<=3) return _T("");
			tmp=LimitStringCount(ret,limitsize-3,FALSE)+_T("...");
			return tmp;
		}
#ifdef _UNICODE
		//unicode模式下不存在问题
		return ret.Left(limitsize);
#endif
		limitsize*=2;
		int i;
		for(i=0;i<ret.GetLength() && limitsize>0;i++)
		{
			int val=ret.GetAt(i);
			if (val>0)
				limitsize-=2;
			else
				limitsize-=1;
		}
		return ret.Left(i);
	}

	/**
	* @brief 转换全角数字和英文到半角
	* @param src	转换的字符串
	* @return 返回转换后的字符串
	*/
	static THString sbc2tbc(THString src)
	{
		const TCHAR chbuf[][3]={
					_T("０"),_T("１"),_T("２"),_T("３"),_T("４"),
					_T("５"),_T("６"),_T("７"),_T("８"),_T("９"),
					_T("ａ"),_T("ｂ"),_T("ｃ"),_T("ｄ"),_T("ｅ"),_T("ｆ"),_T("ｇ"),
					_T("ｈ"),_T("ｉ"),_T("ｊ"),_T("ｋ"),_T("ｌ"),_T("ｍ"),_T("ｎ"),
					_T("ｏ"),_T("ｐ"),_T("ｑ"),_T("ｒ"),_T("ｓ"),_T("ｔ"),
					_T("ｕ"),_T("ｖ"),_T("ｗ"),_T("ｘ"),_T("ｙ"),_T("ｚ"),
					_T("Ａ"),_T("Ｂ"),_T("Ｃ"),_T("Ｄ"),_T("Ｅ"),_T("Ｆ"),_T("Ｇ"),
					_T("Ｈ"),_T("Ｉ"),_T("Ｊ"),_T("Ｋ"),_T("ｌ"),_T("Ｍ"),_T("Ｎ"),
					_T("Ｏ"),_T("Ｐ"),_T("Ｑ"),_T("Ｒ"),_T("Ｓ"),_T("Ｔ"),
					_T("Ｕ"),_T("Ｖ"),_T("Ｗ"),_T("Ｘ"),_T("Ｙ"),_T("Ｚ"),_T("")
					};
		const TCHAR chbuf1[][3]={
					_T("0"),_T("1"),_T("2"),_T("3"),_T("4"),
					_T("5"),_T("6"),_T("7"),_T("8"),_T("9"),
					_T("a"),_T("b"),_T("c"),_T("d"),_T("e"),_T("f"),_T("g"),
					_T("h"),_T("i"),_T("j"),_T("k"),_T("l"),_T("m"),_T("n"),
					_T("o"),_T("p"),_T("q"),_T("r"),_T("s"),_T("t"),
					_T("u"),_T("v"),_T("w"),_T("x"),_T("y"),_T("z"),
					_T("A"),_T("B"),_T("C"),_T("D"),_T("E"),_T("F"),_T("G"),
					_T("H"),_T("I"),_T("J"),_T("K"),_T("L"),_T("M"),_T("N"),
					_T("O"),_T("P"),_T("Q"),_T("R"),_T("S"),_T("T"),
					_T("U"),_T("V"),_T("W"),_T("X"),_T("Y"),_T("Z"),_T("")
					};
		int i=0;
		while(chbuf[i][0]!='\0')
		{
			src.Replace(chbuf[i],chbuf1[i]);
			i++;
		}
		return src;
	}

	/**
	* @brief 转换半角数字和英文到全角
	* @param src	转换的字符串
	* @return 返回转换后的字符串
	*/
	static THString tbc2sbc(THString src)
	{
		const TCHAR chbuf[][3]={
					_T("０"),_T("１"),_T("２"),_T("３"),_T("４"),
					_T("５"),_T("６"),_T("７"),_T("８"),_T("９"),
					_T("ａ"),_T("ｂ"),_T("ｃ"),_T("ｄ"),_T("ｅ"),_T("ｆ"),_T("ｇ"),
					_T("ｈ"),_T("ｉ"),_T("ｊ"),_T("ｋ"),_T("ｌ"),_T("ｍ"),_T("ｎ"),
					_T("ｏ"),_T("ｐ"),_T("ｑ"),_T("ｒ"),_T("ｓ"),_T("ｔ"),
					_T("ｕ"),_T("ｖ"),_T("ｗ"),_T("ｘ"),_T("ｙ"),_T("ｚ"),
					_T("Ａ"),_T("Ｂ"),_T("Ｃ"),_T("Ｄ"),_T("Ｅ"),_T("Ｆ"),_T("Ｇ"),
					_T("Ｈ"),_T("Ｉ"),_T("Ｊ"),_T("Ｋ"),_T("ｌ"),_T("Ｍ"),_T("Ｎ"),
					_T("Ｏ"),_T("Ｐ"),_T("Ｑ"),_T("Ｒ"),_T("Ｓ"),_T("Ｔ"),
					_T("Ｕ"),_T("Ｖ"),_T("Ｗ"),_T("Ｘ"),_T("Ｙ"),_T("Ｚ"),_T("")
					};
		const TCHAR chbuf1[][3]={
					_T("0"),_T("1"),_T("2"),_T("3"),_T("4"),
					_T("5"),_T("6"),_T("7"),_T("8"),_T("9"),
					_T("a"),_T("b"),_T("c"),_T("d"),_T("e"),_T("f"),_T("g"),
					_T("h"),_T("i"),_T("j"),_T("k"),_T("l"),_T("m"),_T("n"),
					_T("o"),_T("p"),_T("q"),_T("r"),_T("s"),_T("t"),
					_T("u"),_T("v"),_T("w"),_T("x"),_T("y"),_T("z"),
					_T("A"),_T("B"),_T("C"),_T("D"),_T("E"),_T("F"),_T("G"),
					_T("H"),_T("I"),_T("J"),_T("K"),_T("L"),_T("M"),_T("N"),
					_T("O"),_T("P"),_T("Q"),_T("R"),_T("S"),_T("T"),
					_T("U"),_T("V"),_T("W"),_T("X"),_T("Y"),_T("Z"),_T("")
					};
		int i=0;
		while(chbuf[i][0]!='\0')
		{
			src.Replace(chbuf1[i],chbuf[i]);
			i++;
		}
		return src;
	}

	/**
	* @brief 获取字符串中指定的内容
	* @param str			原文
	* @param prefix			匹配的前缀
	* @param postfix		匹配的后缀
	* @return 返回匹配内容
	*/
	static THString GetFixValue(THString str,THString prefix,THString postfix)
	{
		int pos;
		if (prefix.IsEmpty())
			pos=0;
		else
			pos=str.Find(prefix);
		if (pos==-1) return _T("");
		pos+=prefix.GetLength();
		int pos2;
		if (postfix.IsEmpty())
			pos2=str.GetLength();
		else
			pos2=str.Find(postfix,pos);
		if (pos2==-1) return _T("");
		return str.Left(pos2).Mid(pos);
	}

	/**
	* @brief 以多重token分解字符串到数组中
	* @param str			原文
	* @param token			各token字符串
	* @param out			分割结果
	* @param bCase			大小写敏感
	* @return
	*/
	static void MultiTokenToArray(THString str,THStringArray *token,THStringArray *out,BOOL bCase=TRUE)
	{
		if (!token || !out) return;
		int spos=0,mspos=0;
		while(MultiFind(str,token,bCase,spos,&mspos))
		{
			if (mspos>=spos)
				out->Add(str.Left(mspos).Mid(spos));
			spos=mspos+1;
		}
	}

	static THString StrRepeat(int cnt,THString str=_T(" "))
	{
		THString ret;
		for(int i=0;i<cnt;i++)
			ret+=str;
		return ret;
	}
	static THString Format(const TCHAR* lpszMsg,...)
	{
		THString buf;
		va_list pArg;
		va_start(pArg,lpszMsg);
		buf.FormatV(lpszMsg,pArg);
		va_end(pArg);
		return buf;
	}

	/**
	* @brief 判断字符串是否为数字
	* @return 返回是否
	*/
	static BOOL IsStringNumber(THString str)
	{
		TCHAR ch;
		for(int i=0;i<str.GetLength();i++)
		{
			ch=str.GetAt(i);
			if (ch<'0' || ch>'9') return FALSE;
		}
		return TRUE;
	}

	/**
	* @brief 判断字符串是否为数字和小数点
	* @return 返回是否
	*/
	static BOOL IsStringNumberAndDot(THString str)
	{
		TCHAR ch;
		for(int i=0;i<str.GetLength();i++)
		{
			ch=str.GetAt(i);
			if ((ch<'0' || ch>'9') && ch!='.')
				return FALSE;
		}
		return TRUE;
	}

	/**
	* @brief 判断字符串是否为数字
	* @return 返回是否
	*/
	static BOOL IsStringNumberAndOperator(THString str)
	{
		TCHAR ch;
		for(int i=0;i<str.GetLength();i++)
		{
			ch=str.GetAt(i);
			if ((ch<'0' || ch>'9') && ch!='.' && ch!='-' && ch!='+')
				return FALSE;
		}
		return TRUE;
	}

	/**
	* @brief 判断字符串是否全为声母，即某中文简写
	* @return 返回是否
	*/
	static BOOL IsStringShengMu(THString str)
	{
		//声母：b p m f d t n l g k h j q x zh ch sh r z c s y w
		//起头韵母：a o e ai ei ao ou er an en ang
		//即abcdefgh[i]jklmnopqrst[uv]wxyz
		TCHAR ch;
		for(int i=0;i<str.GetLength();i++)
		{
			ch=str.GetAt(i);
			if (!((ch>='a' && ch<='z') || (ch>='A' && ch<='Z')) ||
				ch=='i' || ch=='u' || ch=='v') return FALSE;
		}
		return TRUE;
	}

	/**
	* @brief 判断字符串是否为可视Ascii字符
	* @return 返回是否
	*/
	static BOOL IsStringVisiableAscii(THString str)
	{
		TCHAR ch;
		for(int i=0;i<str.GetLength();i++)
		{
			ch=str.GetAt(i);
			if (!((ch>=32 && ch<=0x7a))) return FALSE;
		}
		return TRUE;
	}

	/**
	* @brief 判断字符串是否为字母
	* @return 返回是否
	*/
	static BOOL IsStringLetter(THString str)
	{
		TCHAR ch;
		for(int i=0;i<str.GetLength();i++)
		{
			ch=str.GetAt(i);
			if (!((ch>='a' && ch<='z') || (ch>='A' && ch<='Z'))) return FALSE;
		}
		return TRUE;
	}

	/**
	* @brief 判断字符串是否为数字和字母
	* @return 返回是否
	*/
	static BOOL IsStringLetterAndNumber(THString str)
	{
		TCHAR ch;
		for(int i=0;i<str.GetLength();i++)
		{
			ch=str.GetAt(i);
			if (!((ch>='a' && ch<='z') || (ch>='A' && ch<='Z') || (ch>='0' && ch<='9'))) return FALSE;
		}
		return TRUE;
	}

	static THString i2s(int val)
	{
		THString str;
		str.Format(_T("%d"),val);
		return str;
	}

	static THString f2s(double val,int count=6)
	{
		THString str,str1;
		str1.Format(_T("%%.%df"),count);
		str.Format(str1,val);
		return str;
	}

	/**
	* @brief 取整浮点数小数点后的0
	* @param val		浮点数
	* @param count		浮点数位数
	* @return 返回结果
	*/
	static THString f2sc(double val,int count=2)
	{
		THString out=f2s(val,count);
		if (out.Find(_T("."))!=-1)
			out.TrimRight(_T("0"));
		out.TrimRight(_T("."));
		if (out.IsEmpty()) out=_T("0");
		return out;
	}

	/**
	* @brief 取整浮点数字符串小数点后的0
	* @param str			原文
	* @return 返回结果
	*/
	static THString fsc(THString val)
	{
		if (val.Find(_T("."))!=-1)
			val.TrimRight(_T("0"));
		val.TrimRight(_T("."));
		if (val.IsEmpty()) val=_T("0");
		return val;
	}

	static THString u2s(unsigned int val)
	{
		THString str;
		str.Format(_T("%u"),val);
		return str;
	}

	static int s2i(THString str){return _ttoi(str);}

	static double s2f(THString str){return _tstof(str);}

	static inline unsigned int s2u(THString str){return (unsigned int)_ttoi(str);}

	static unsigned int s2h(THString str){return _tcstol(str,'\0',16);}

	/**
	* @brief 转换数字串，支持10进制和16进制0x，h的辨别
	* @param str			原文
	* @return 返回结果
	*/
	static unsigned int s2num_u(THString str)
	{
		str.Trim();
		str.MakeLower();
		BOOL bHex=FALSE;
		if (str.GetLength()>=2 && str.Left(2)==_T("0x"))
		{
			str=str.Mid(2);
			bHex=TRUE;
		}
		else if (str.GetLength()>=1 && str.GetAt(str.GetLength()-1)=='h')
		{
			str=str.Left(str.GetLength()-1);
			bHex=TRUE;
		}
		if (bHex)
			return s2h(str);
		return s2u(str);
	}

	/**
	* @brief 转换数字串，支持10进制和16进制0x，h的辨别
	* @param str			原文
	* @return 返回结果
	*/
	static int s2num_i(THString str)
	{
		str.Trim();
		str.MakeLower();
		BOOL bHex=FALSE;
		if (str.Left(2)==_T("0x"))
		{
			str=str.Mid(2);
			bHex=TRUE;
		}
		else if (str.GetAt(str.GetLength()-1)=='h')
		{
			str=str.Left(str.GetLength()-1);
			bHex=TRUE;
		}
		if (bHex)
			return (int)s2h(str);
		return s2i(str);
	}

	/**
	* @brief 查找最早出现的字符串
	* @param str			查找原文
	* @param ar				各需要查找的字符串
	* @param bCase			大小写敏感
	* @param StartPos		开始搜索的位置
	* @param spos			返回找到的字符串的开始位置，可为空
	* @param sFitStr		匹配的字符串，可为空
	* @return 返回是否找到
	*/
	static BOOL MultiFind(THString str,THStringArray *ar,BOOL bCase=TRUE,int StartPos=0,int *spos=NULL,THString *sFitStr=NULL)
	{
		if (!ar) return FALSE;
		if (bCase==FALSE) str.MakeLower();
		int ret=-1,tmp;
		THString retstr;
		for(int i=0;i<ar->GetSize();i++)
		{
			THString stmp=ar->GetAt(i);
			if (bCase==FALSE) stmp.MakeLower();
			tmp=str.Find(stmp,StartPos);
			if (tmp!=-1)
			{
				if (ret==-1 || tmp<ret)
				{
					ret=tmp;
					retstr=ar->GetAt(i);
				}
			}
		}
		if (spos) *spos=ret;
		if (sFitStr) *sFitStr=retstr;
		return (ret!=-1);
	}

	/**
	* @brief 限制某字符串可能出现n次，如用于限制行数
	* @param orgstr			原字符串
	* @param token			需限制的字符串
	* @param tokenlimit		限制次数
	* @return 返回限制后的字符串
	*/
	static THString LimitStringByToken(THString orgstr,THString token,int tokenlimit)
	{
		if (token.IsEmpty()) return orgstr;
		int pos=0;
		int len=token.GetLength();
		while(tokenlimit>0)
		{
			pos=orgstr.Find(token,pos);
			tokenlimit--;
			if (pos==-1) break;
			pos+=len;
		}
		if (pos!=-1)
			return orgstr.Left(pos);
		return orgstr;
	}

	/**
	* @brief 反向限制某字符串可能出现n次，如用于限制行数
	* @param orgstr			原字符串
	* @param token			需限制的字符串
	* @param tokenlimit		限制次数
	* @return 返回限制后的字符串
	*/
	static THString LimitStringByTokenInv(THString orgstr,THString token,int tokenlimit)
	{
		if (token.IsEmpty()) return orgstr;
		THString tmpstr=orgstr;
		int pos=0;
		int len=token.GetLength();
		while(tokenlimit>0)
		{
			pos=tmpstr.ReverseFind(token.GetAt(0));
			if (pos==-1) break;
			if (tmpstr.Mid(pos,len)==token)
				tokenlimit--;
			tmpstr=tmpstr.Left(pos);
		}
		if (pos!=-1)
			return orgstr.Mid(pos+len);
		return orgstr;
	}

	static THStringA LimitStringByTokenInvA(THStringA orgstr,THStringA token,int tokenlimit)
	{
		if (token.IsEmpty()) return orgstr;
		THStringA tmpstr=orgstr;
		int pos=0;
		int len=token.GetLength();
		while(tokenlimit>0)
		{
			pos=tmpstr.ReverseFind(token.GetAt(0));
			if (pos==-1) break;
			if (tmpstr.Mid(pos,len)==token)
				tokenlimit--;
			tmpstr=tmpstr.Left(pos);
		}
		if (pos!=-1)
			return orgstr.Mid(pos+len);
		return orgstr;
	}

	static THString BinToHexString(const void *pData,unsigned int nLen)
	{
		THString ret;
		const unsigned char*tmp=(const unsigned char*)pData;
		for(unsigned int i=0;i<nLen;i++)
			ret.AppendFormat(_T("%02x"),*(tmp+i));
		return ret;
	}

	//nLen 是in out的
	static BOOL HexStringToBin(THString sInput,void *pRetData,unsigned int *nLen)
	{
		if (sInput.GetLength()%2!=0 || !pRetData || !nLen) return FALSE;
		memset(pRetData,0,*nLen);
		unsigned char *tmp=(unsigned char *)pRetData;
		unsigned int i;
		for(i=0;i<(unsigned int)sInput.GetLength() && i/2<(*nLen);i+=2)
		{
			*(tmp+i/2)=(unsigned char)s2h(sInput.Mid(i).Left(2));
		}
		if (nLen) *nLen=i/2;
		if (i==(unsigned int)sInput.GetLength()) return TRUE;
		return FALSE;
	}

	//nLen 是in out的
	static void *HexStringToBin(THString sInput,unsigned int *nLen)
	{
		if (sInput.GetLength()%2!=0) return NULL;
		unsigned int size=(sInput.GetLength()/2+1);
		void *tmp=new char[size];
		if (!tmp) return NULL;
		if (!HexStringToBin(sInput,tmp,&size))
		{
			Free(tmp);
			return NULL;
		}
		if (nLen) *nLen=size;
		return tmp;
	}

	static void Free(void *data)
	{
		if (data) delete [] (char *)data;
	}

	typedef enum _FormatType
	{
		FormatType_Hex,//16进制显示
		FormatType_Dec,//10进制显示
		FormatType_Oct,//8进制显示
		FormatType_Bin,//2进制显示
		FormatType_None//不显示
	}FormatType;
	/**
	* @brief 格式化内容数据
	* @param pData			数据指针
	* @param nLen			数据长度
	* @param ft				数据显示方式
	* @param nLineCount		每行显示数据量
	* @param ftOffest		偏移量显示方式，FormatType_None为不显示
	* @param bMiddleSep		中间是否需要分隔
	* @param bHaveRawData	是否打印原始数据,'\\' 和 '%' 将不会打印
	* @return 返回格式化字符串
	*/
	static THString FormatString(const void *pData,unsigned int nLen,FormatType ft=FormatType_Hex,int nLineCount=16,FormatType ftOffest=FormatType_Hex,BOOL bMiddleSep=TRUE,BOOL bHaveRawData=TRUE)
	{
		THString ret;
		if (pData && nLen>0)
		{
			THString sym;
			if (ft==FormatType_Hex)
				sym=_T("%02X ");
			else if (ft==FormatType_Oct)
				sym=_T("%3o ");
			/*else if (ft==FormatType_Bin)
				sym=_T("%8s ");*/
			else if (ft==FormatType_Dec)
				sym=_T("%3u ");
			THString symoff;
			if (ftOffest==FormatType_Hex)
				symoff=_T("%08XH ");
			else if (ftOffest==FormatType_Dec)
				symoff=_T("%08u ");
			const unsigned char *pch=(const unsigned char *)pData;
			THString linestr;
			int idx;
			int mididx=nLineCount/2;
			unsigned char ch;
			UINT i;
			for(i=0;i<nLen;i++)
			{
				idx=i%nLineCount;
				ch=*(pch+i);
				if (idx==0)
				{
					//linestr.Empty();
					if (!symoff.IsEmpty())
						ret.AppendFormat(symoff,i);
				}
				if (bMiddleSep && idx==mididx)
					ret+=_T("- ");
				if (!sym.IsEmpty())
					ret.AppendFormat(sym,*(pch+i));
				else if (ft==FormatType_Bin)
					ret.AppendFormat(_T("%8s "),FormatBin(ch));
				if (bHaveRawData)
				{
					//trim the '\\' and '%'
					if (ch > 0x1f && ch!='\\' && ch!='%')
						linestr.AppendFormat(_T("%c"),ch);
					else
						linestr+=_T(".");
				}
				if (idx==nLineCount-1)
				{
					ret+=linestr;
					if (i!=nLen-1)
						ret+=_T("\r\n");
					linestr.Empty();
				}
			}
			if (!linestr.IsEmpty())
			{
				//the last raw data is not print
				THString tmpret;
				for(idx=(i-1)%nLineCount;idx<nLineCount-1;idx++)
				{
					if (bMiddleSep && idx==mididx)
						tmpret+=_T("  ");//the middle line space
					if (ft==FormatType_Hex)
						tmpret+=_T("   ");
					else if (ft==FormatType_Oct)
						tmpret+=_T("    ");
					else if (ft==FormatType_Bin)
						tmpret+=_T("         ");
					else if (ft==FormatType_Dec)
						tmpret+=_T("    ");
				}
				ret+=tmpret+linestr;
			}
		}
		return ret;
	}

	static THString FormatBin(TCHAR ch)
	{
		unsigned int uch=(unsigned int)ch;
		THString ret;
		for(int i=7;i>=0;i--)
			ret.AppendFormat(_T("%d"),((uch>>i)%2));
		return ret;
	}
};

/**
* @brief 简单Xml字符串处理类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-06-03 新建类
* @2007-06-14 SetParam函数改为传入THString指针
* @2008-02-21 当SetParam函数值为空时，调用RemoveParam，这样可节省内存
*/
/**<pre>
使用Sample：
	THString str=_T("<item1>23</item1><item2></item2><aaa><bbb>dert</bbb><ccc>gf</ccc></aaa>");
	THDebug(str);
	THSimpleXml::SetParam(str,_T("bbb"),_T("test1412"));
	THDebug(str);
	THDebug(THSimpleXml::GetParam(str,_T("ccc")));
	THDebug(THSimpleXml::GetAndRemoveParam(&str,_T("aaa")));
	THDebug(str);
	THSimpleXml::RemoveParam(&str,_T("item2"));
	THDebug(str);
</pre>*/
class THSimpleXml
{
public:
	/**
	* @brief 获取内容中的键内容
	* @param content	原内容
	* @param key		键名
	* @param f			键值开始
	* @param e			键值结束
	* @return 键的内容
	*/
	static THString GetParam(const THString content,const THString key,const THString f=_T("<"),const THString e=_T(">"))
	{
		THString skey;
		THString ekey;
		skey=f+key+e;
		ekey=f+_T("/")+key+e;
		int pos=content.Find(skey,0);
		if (pos==-1) return _T("");
		pos+=skey.GetLength();
		int pos2=content.Find(ekey,pos);
		if (pos2==-1) return _T("");
		return content.Mid(pos,pos2-pos);
	}
	/**
	* @brief 获取内容中的键内容并删除键
	* @param content	原内容
	* @param key		键名
	* @param f			键值开始
	* @param e			键值结束
	* @return 键的内容
	*/
	static THString GetAndRemoveParam(THString *content,const THString key,const THString f=_T("<"),const THString e=_T(">"))
	{
		THString skey;
		THString ekey;
		skey=f+key+e;
		ekey=f+_T("/")+key+e;
		int pos=content->Find(skey,0);
		if (pos==-1) return _T("");
		pos+=skey.GetLength();
		int pos2=content->Find(ekey,pos);
		if (pos2==-1) return _T("");
		THString ret=content->Mid(pos,pos2-pos);
		content->SetString(content->Left(pos-skey.GetLength())+content->Mid(pos2+ekey.GetLength()));
		return ret;
	}
	/**
	* @brief 删除键
	* @param content	原内容
	* @param key		键名
	* @param f			键值开始
	* @param e			键值结束
	*/
	static void RemoveParam(THString *content,const THString key,const THString f=_T("<"),const THString e=_T(">"))
	{
		THString skey;
		THString ekey;
		skey=f+key+e;
		ekey=f+_T("/")+key+e;
		int pos=content->Find(skey,0);
		if (pos==-1) return;
		int pos2=content->Find(ekey,pos);
		if (pos2==-1) return;
		content->SetString(content->Left(pos)+content->Mid(pos2+ekey.GetLength()));
	}
	/**
	* @brief 更新键值内容
	* @param content	字符数据
	* @param key		键名
	* @param value		键值
	* @param f			键值开始
	* @param e			键值结束
	*/
	static void SetParam(THString *content,const THString key,const THString value,const THString f=_T("<"),const THString e=_T(">"))
	{
		if (value.IsEmpty())
		{
			RemoveParam(content,key,f,e);
			return;
		}
		THString skey;
		THString ekey;
		skey=f+key+e;
		ekey=f+_T("/")+key+e;
		int pos=content->Find(skey,0);
		int pos2=-1;
		if (pos!=-1) pos2=content->Find(ekey,pos);
		if (pos2==-1)
			content->Append(MakeParam(key,value));
		else
			content->SetString(content->Left(pos+skey.GetLength())+value+content->Mid(pos2));
	}
	/**
	* @brief 生成键值内容
	* @param key		键名
	* @param value		键值
	* @param f			键值开始
	* @param e			键值结束
	* @return 键的内容
	*/
	static THString MakeParam(const THString key,const THString value,const THString f=_T("<"),const THString e=_T(">"))
	{
		return f+key+e+value+f+_T("/")+key+e;
	}
	/**
	* @brief 把xml内容按样式生成字符串
	* @param sContent	xml内容
	* @param sStyle		生成样式，如(KEY:VALUE)，其中KEY和VALUE以真实数据替换
	* @param f			键值开始
	* @param e			键值结束
	* @return 键的内容
	*/
	static THString toString(const THString sXml,const THString sStyle,const THString f=_T("<"),const THString e=_T(">"))
	{
		THString sContent=sXml;
		THString ret;
		while(1)
		{
			int pos=sContent.Find(f);
			if (pos==-1) break;
			int pos2=sContent.Find(e,pos);
			if (pos2==-1) break;
			THString key=sContent.Left(pos2).Mid(pos+1);
			key.Replace(_T("/"),_T(""));
			if (key.IsEmpty())
			{
				//may be error key "<>",skip it
				sContent=sContent.Mid(pos2+1);
			}
			else
			{
				THString tmp=sStyle;
				tmp.Replace(_T("KEY"),key);
				tmp.Replace(_T("VALUE"),GetAndRemoveParam(&sContent,key,f,e));
				ret+=tmp;
			}
		}
		return ret;
	}
	/**
	* @brief 把xml内容按样式生成字符串
	* @param sContent	xml内容
	* @param sKey		返回Key数组
	* @param sValue		返回Value数组
	* @param f			键值开始
	* @param e			键值结束
	*/
	static void toArray(const THString sXml,THStringArray *sKey,THStringArray *sValue,const THString f=_T("<"),const THString e=_T(">"))
	{
		THString sContent=sXml;
		THString ret;
		while(1)
		{
			int pos=sContent.Find(f);
			if (pos==-1) break;
			int pos2=sContent.Find(e,pos);
			if (pos2==-1) break;
			THString key=sContent.Left(pos2).Mid(pos+1);
			key.Replace(_T("/"),_T(""));
			if (key.IsEmpty())
			{
				//may be error key "<>",skip it
				sContent=sContent.Mid(pos2+1);
			}
			else
			{
				ret=GetAndRemoveParam(&sContent,key,f,e);
				if (sKey) sKey->Add(key);
				if (sValue) sValue->Add(ret);
			}
		}
	}
	/**
	* @brief 把xml内容按样式生成字符串
	* @param sKey		Key数组
	* @param sValue		Value数组
	* @param f			键值开始
	* @param e			键值结束
	* @return Xml内容
	*/
	static THString MakeFromArray(const THStringArray sKey,const THStringArray sValue,const THString f=_T("<"),const THString e=_T(">"))
	{
		THString ret;
		int size=min((int)sKey.GetSize(),(int)sValue.GetSize());
		for(int i=0;i<size;i++)
			ret+=MakeParam(sKey[i],sValue[i],f,e);
		return ret;
	}
};

/**
* @brief 轻型Xml字符串处理类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-08-06 新建类
* @2007-08-13 修正一个bug
* @2007-08-21 支持大小写无关
*/
/**<pre>
使用Sample：
</pre>*/
class THTiniXml
{
public:
	THTiniXml(){}
	THTiniXml(THString content,BOOL bCase=FALSE){SetContent(content,bCase);}
	virtual ~THTiniXml(){}

	/**
	* @brief 设置Xml内容
	* @param content		内容
	* @param bCase			是否区分大小写
	*/
	void SetContent(THString content,BOOL bCase=FALSE)
	{
		m_content=content;
		m_current=m_content;
		m_currentcomp=m_current;
		m_bCase=bCase;
		if (!m_bCase)
			m_currentcomp.MakeLower();
	}

	/**
	* @brief 获取Xml内容
	* @param content		内容
	* @return 字符串
	*/
	THString GetContent(){return m_content;}

	/**
	* @brief 获取当前定位的Xml内容
	* @param content		内容
	* @return 字符串
	*/
	THString GetCurrent(){return m_current;}

	/**
	* @brief 把当前内容分解出各个子Key，保存到数组中
	* @param ar			保存数组
	* @return 是否成功
	*/
	BOOL SplitAllKeys(THStringArray *ar)
	{
		if (!ar) return FALSE;
		THStringArray searchar;
		ar->RemoveAll();
		THString tmp=m_current;
		int pos;
		int spos;
		THString tmpkey;
		while(!tmp.IsEmpty())
		{
			pos=tmp.Find(_T("<"),0);
			if (pos!=-1)
			{
				//还有xml内容
				//确定key名称
				searchar.RemoveAll();
				searchar.Add(_T("/>"));
				searchar.Add(_T(">"));
				if (THStringConv::MultiFind(tmp,&searchar,m_bCase,pos,&spos,&tmpkey))
				{
					if (tmpkey==_T("/>"))
					{
						//说明键值形式是<key />的
						spos+=2;
					}
					else if (tmpkey==_T(">"))
					{
						//说明找到键值的开头，先确认键值名称
						searchar.RemoveAll();
						searchar.Add(_T(" "));
						searchar.Add(_T(">"));
						if (THStringConv::MultiFind(tmp,&searchar,m_bCase,pos,&spos,&tmpkey))
						{
							THString key=_T("</")+tmp.Left(spos).Mid(pos+1)+_T(">");
							if (!m_bCase)
							{
								key.MakeLower();
								THString stmp=tmp;
								stmp.MakeLower();
								spos=stmp.Find(key,pos);
							}
							else
							{
								spos=tmp.Find(key,pos);
							}
							if (spos!=-1)
								spos+=key.GetLength();
						}
						else
							spos=-1;
					}
					else
						spos=-1;
				}
				else
					spos=-1;
				if (spos!=-1)
				{
					THString ret;
					ret=tmp.Left(spos).Mid(pos);
					tmp=tmp.Mid(spos);
					ar->Add(ret);
				}
				else
				{
					tmp.Empty();
				}
			}
			else
			{
				tmp.Empty();
			}
		}
		return TRUE;
	}

	/**
	* @brief 进入子项
	* @param key			键名
	* @param start			偏移位置，可使用GetKeyValue的pos
	* @return 是否成功
	*/
	BOOL IntoXml(THString key,int start=0)
	{
		if (!m_bCase) key.MakeLower();
		THString startkey=_T("<")+key+_T(">");
		THString startval=_T("<")+key+_T(" ");
		THString endkey=_T("</")+key+_T(">");
		int pos=m_currentcomp.Find(startkey,start);
		int pos1=m_currentcomp.Find(startval,start);
		//如果pos1格式的比pos格式的还前，使用pos1
		if (pos==-1) pos=pos1;
		if (pos1!=-1 && pos1<pos) pos=pos1;
		if (pos==-1) return FALSE;
		int startpos=m_currentcomp.Find(_T(">"),pos);
		if (startpos==-1) return FALSE;
		int pos2=m_currentcomp.Find(endkey,startpos);
		if (pos2==-1) return FALSE;
		m_current=m_current.Left(pos2).Mid(startpos+1);
		m_currentcomp=m_current;
		if (!m_bCase)
			m_currentcomp.MakeLower();
		return TRUE;
	}

	/**
	* @brief 返回到根目录
	*/
	void ResetToRoot()
	{
		m_current=m_content;
		m_currentcomp=m_current;
		if (!m_bCase) m_currentcomp.MakeLower();
	}

	/**
	* @brief 获取指定键对应的指定属性
	* @param key			键名
	* @param attrib			属性名
	* @return 内容字符串
	*/
	THString GetAttribute(THString key,THString attrib)
	{
		if (!m_bCase) key.MakeLower();
		THString startval=_T("<")+key+_T(" ");
		int pos=m_currentcomp.Find(startval,0);
		if (pos==-1) return _T("");
		pos=m_currentcomp.Find(attrib+_T("="),pos+startval.GetLength());
		if (pos==-1) return _T("");
		pos+=attrib.GetLength()+1;
		int pos2;
		if (m_currentcomp.GetAt(pos)=='\"')
		{
			pos++;
			//find end "
			pos2=m_currentcomp.Find(_T("\""),pos);
			if (pos2==-1) return _T("");
		}
		else
		{
			//find end space
			pos2=m_currentcomp.Find(_T(" "),pos);
			if (pos2==-1) return _T("");
		}
		return m_current.Left(pos2).Mid(pos);
	}

	/**
	* @brief 获取指定键对应的内容
	* @param key			键名
	* @param retpos			指针指向数值为开始搜索的位置，指针为空为不指定，返回结果为下次的位置，返回-1为没有找到
	* @return 内容字符串
	*/
	/**<pre>
		正确读取方式:<key>content</key>
		正确读取方式:<key value="">content</key>
		错误读取方式:<key value=""/>
	</pre>
	*/
	THString GetKeyValue(THString key,int *retpos=NULL)
	{
		if (!m_bCase) key.MakeLower();
		THString startkey=_T("<")+key+_T(">");
		THString startval=_T("<")+key+_T(" ");
		THString endkey=_T("</")+key+_T(">");
		int start=0;
		if (retpos)
		{
			start=*retpos;
			*retpos=-1;
		}
		int pos=m_currentcomp.Find(startkey,start);
		int pos1=m_currentcomp.Find(startval,start);
		//如果pos1格式的比pos格式的还前，使用pos1
		if (pos==-1) pos=pos1;
		if (pos1!=-1 && pos1<pos) pos=pos1;
		if (pos==-1) return _T("");
		int startpos=m_currentcomp.Find(_T(">"),pos);
		if (startpos==-1) return _T("");
		int pos2=m_currentcomp.Find(endkey,startpos);
		if (pos2==-1) return _T("");
		if (retpos) *retpos=pos2+endkey.GetLength();
		return m_current.Left(pos2).Mid(startpos+1);
	}

	/**
	* @brief 枚举所有指定键对应的内容
	* @param key			键名
	* @param ar				返回的内容
	* @return 是否成功
	*/
	BOOL GetAllKeyValue(THString key,THStringArray *ar)
	{
		if (!ar) return FALSE;
		ar->RemoveAll();
		int pos=0;
		BOOL bFound=FALSE;
		THString ret;
		while(pos!=-1)
		{
			ret=GetKeyValue(key,&pos);
			if (pos!=-1)
			{
				bFound=TRUE;
				ar->Add(ret);
			}
		}
		return bFound;
	}
protected:
	THString m_content;
	THString m_current;
	THString m_currentcomp;
	BOOL m_bCase;
};

/**
* @brief 数据查找定位类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-07-01 新建类
*/
/**<pre>
用法：
</pre>*/
class THBinSearch
{
public:
	/**
	* @brief 搜索二进制内容
	* @param buf		原内容
	* @param len		原内容长度
	* @param search		要搜索的内容
	* @param searchlen	要搜索内容长度
	* @return 找到地方的指针，找不到返回NULL
	*/
	static const void *BinSearch(const void *buf,unsigned int len,const void *search,unsigned int searchlen)
	{
		if (searchlen>len) return NULL;
		const char *tbuf=(const char *)buf;
		const char *sbuf=(const char *)search;
		for(unsigned int i=0;i<=len-searchlen;i++)
		{
			if (tbuf[i]==sbuf[0])
			{
				if (memcmp(tbuf+i,sbuf,searchlen)==0)
					return (const void *)(tbuf+i);
			}
		}
		return NULL;
	}

	/**
	* @brief 搜索二进制内容中的文本
	* @param buf		原内容
	* @param len		原内容长度
	* @param search		要搜索的内容
	* @return 找到地方的指针，找不到返回NULL
	*/
	static const void *StrSearch(const void *buf,unsigned int len,const TCHAR *search)
	{
		unsigned int size=(unsigned int)_tcslen(search);
		if (size>len) return NULL;
		while(len>=0)
		{
			const void *ret;
#ifdef _UNICODE
			ret=wmemchr((const wchar_t *)buf,*search,len);
#else
			ret=memchr(buf,*search,len);
#endif
			if (!ret) return NULL;
			len-=(unsigned int)(((char *)ret)-((char *)buf));
			if (ret && _tcsncmp(search,(const TCHAR *)ret,min(size,len))==0)
				return ret;
			if (len>0) len--;
			buf=((char *)ret)+1;
		}
		return NULL;
	}

	/**
	* @brief 搜索二进制内容中的文本（忽略大小写）
	* @param buf		原内容
	* @param len		原内容长度
	* @param search		要搜索的内容
	* @return 找到地方的指针，找不到返回NULL
	*/
	static const void *StrSearchNoCase(const void *buf,unsigned int len,const TCHAR *search)
	{
		unsigned int size=(unsigned int)_tcslen(search);
		if (size>len) return NULL;
		while(len>=0)
		{
			const void *ret;
#ifdef _UNICODE
			ret=wmemchr((const wchar_t *)buf,*search,len);
#else
			ret=memchr(buf,*search,len);
#endif
			if (!ret) return NULL;
			len-=(unsigned int)(((char *)ret)-((char *)buf));
			if (ret && _tcsnicmp(search,(const TCHAR *)ret,min(size,len))==0)
				return ret;
			buf=((char *)ret)+1;
		}
		return NULL;
	}
};

/**
* @brief 表达式查找定位类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-07-01 新建类
*/
/**<pre>
用法：
</pre>*/
class THMatchSearch
{
public:
	/**
	* @brief 从字符串中取出指定格式字符
	* @param str		原内容，查找过程会改写str的值
	* @param match		匹配表达式，%s为提取内容为字符串，%d为提取内容为数字，%x为该段内容任意，不提取，%%为匹配%号，空格如使用[ ]，表示匹配一个或多个空格，可跟1-9数字限定宽度，如：%1d
	* @param ar			返回的内容
	* @return 是否找到合适值
	*/
	/**
		match 匹配表达式语法
		[%2d-%2d %2d:%2d:%2d.%x][%x]->[%11d]:%s\r\n
	*/
	static BOOL MatchSearch(THString *str,THString match,THStringArray *ar)
	{
		if (!str || !ar) return FALSE;
		//分解match段
		THStringArray mar;//保存每段的匹配关键字
		THStringArray kar;//保存每段的获取类型
		THStringArray lar;//保存每段的宽度匹配
		THString prematch;
		while(!match.IsEmpty())
		{
			int keypos=match.Find(_T("%"),0);
			if (keypos==-1)
			{
				mar.Add(prematch+match);
				prematch.Empty();
				match.Empty();
			}
			else
			{
				if (keypos+1>=match.GetLength()) return FALSE;
				TCHAR ch=match.GetAt(keypos+1);
				if (ch=='%')
				{
					prematch+=match.Left(keypos+1);
					match=match.Mid(keypos+2);
				}
				else
				{
					mar.Add(prematch+match.Left(keypos));
					prematch.Empty();
					if (ch=='d' || ch=='s' || ch=='x')
					{
						match=match.Mid(keypos+2);
						THString tmpstr;
						tmpstr.Format(_T("%c"),ch);
						kar.Add(tmpstr);
						lar.Add(_T("0"));//不限制
					}
					else if (ch>='0' && ch<='9')
					{
						TCHAR ch2=match.GetAt(keypos+2);
						if (ch2=='d' || ch2=='s' || ch2=='x')
						{
							match=match.Mid(keypos+3);
							THString tmpstr;
							tmpstr.Format(_T("%c"),ch2);
							kar.Add(tmpstr);
							tmpstr.Format(_T("%c"),ch);
							lar.Add(tmpstr);//限制
						}
						else
							return FALSE;
					}
					else
						return FALSE;
				}
			}
		}
		int len,pos;
		int i;
		for(i=0;i<mar.GetSize();i++)
		{
			len=0;
			if (i!=0 && lar[i-1]!=_T("0"))
			{
				//limit 模式
				len=THStringConv::s2i(lar[i-1]);
			}
			int sizelen;
			pos=MatchFind(*str,mar[i],&sizelen,len);
			//pos=str->Find(mar[i],len);
			if (pos==-1) return FALSE;
			if (i!=0)
			{
				THString ret;
				if (len!=0 && pos!=len) return FALSE;
				ret=str->Left(pos);
				//*str=str->Mid(pos);
				if (kar[i-1]=='d')
				{
					ar->Add(THStringConv::i2s(THStringConv::s2i(ret)));
				}
				else if (kar[i-1]=='s')
				{
					ar->Add(ret);
				}
			}
			*str=str->Mid(pos+sizelen);
		}
		if (kar.GetSize()>i-1)
		{
			int lenlimit=THStringConv::s2i(lar[i-1]);
			if (lenlimit!=0) *str=str->Left(lenlimit);
			if (kar[i-1]=='d')
			{
				ar->Add(THStringConv::i2s(THStringConv::s2i(*str)));
			}
			else if (kar[i-1]=='s')
			{
				ar->Add(*str);
			}
		}
		return TRUE;
	}
protected:
	//查找字符串，对多空格字符进行处理
	static int MatchFind(THString str,THString found,int *poslen,int startpos)
	{
		int pos=found.Find(_T("[ ]"),0);
		if (pos!=-1)
		{
			//需要处理多空格问题
			THStringArray searcharray;
			while(pos!=-1)
			{
				searcharray.Add(found.Left(pos));
				found=found.Mid(pos+3);
				pos=found.Find(_T("[ ]"),0);
			}
			if (!found.IsEmpty())
				searcharray.Add(found);
			if (searcharray.GetSize()>0)
			{
				while(1)
				{
					//如果需要直接匹配空格，则先查找空格
					if (searcharray[0].IsEmpty())
						pos=str.Find(_T(" "),startpos);
					else
						pos=str.Find(searcharray[0],startpos);
					if (pos==-1) break;
					pos+=searcharray[0].GetLength();
					*poslen=searcharray[0].GetLength();
					BOOL bMatch=TRUE;
					for(int i=1;i<searcharray.GetSize();i++)
					{
						THString tmp=str.Mid(pos);
						tmp.TrimLeft();
						int spacesize=str.Mid(pos).GetLength()-tmp.GetLength();
						if (tmp.Find(searcharray[i],0)!=0)
						{
							//不能匹配空格
							if (i!=searcharray.GetSize()-1 && spacesize<=0)
							{
								//unmatch
								bMatch=FALSE;
								break;
							}
						}
						else
						{
							pos+=searcharray[i].GetLength()+spacesize;
							*poslen=*poslen+searcharray[i].GetLength()+spacesize;
						}
					}
					if (bMatch)
					{
						//计算最后的空格大小
						THString tmp=str.Mid(pos);
						tmp.TrimLeft();
						int spacesize=str.Mid(pos).GetLength()-tmp.GetLength();
						*poslen=*poslen+spacesize;
						if (searcharray[0].IsEmpty())
							return str.Find(_T(" "),startpos);
						return str.Find(searcharray[0],startpos);
					}
					startpos=pos;
				}
			}
			*poslen=0;
			return -1;
		}
		*poslen=found.GetLength();
		return str.Find(found,startpos);
	}
};
