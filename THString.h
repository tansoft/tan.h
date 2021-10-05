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
* @brief �ַ����и���
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-06-04 �½���
*/
/**<pre>
�÷���
	THStringToken str(_T("qwe 3213 3213"));
	while(str.IsMoreTokens())
	{
		THDebug(_T("Token:%s"),str.GetNextToken());
	}
ע�⣺����Ч�ʲ��ߣ�����GetAmount����
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
* @brief �ַ���ת����
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-06-29 �½���
*/
/**<pre>
�÷���
	THString str=THStringConv::i2s(10);

	THString ret=THStringConv::FormatString(_T("\x11\xdd\x1d\xff\x1a\x00\x01\x41"),8,THStringConv::FormatType_Hex,8,THStringConv::FormatType_Hex,TRUE,TRUE);
	THDebug(ret);
	ret=THStringConv::FormatString(_T("\x11\xdd\x1d\xff\x1a\x00\x01\x41"),8,THStringConv::FormatType_Dec,4,THStringConv::FormatType_Dec,TRUE,FALSE);
	THDebug(ret);
	ret=THStringConv::FormatString(_T("\x11\xdd\x1d\xff\x1a\x00\x01\x41"),8,THStringConv::FormatType_Oct,7,THStringConv::FormatType_None,TRUE,TRUE);
	THDebug(ret);
	ret=THStringConv::FormatString(_T("\x11\xdd\x1d\xff\x1a\x00\x01\x41"),8,THStringConv::FormatType_Bin,8,THStringConv::FormatType_Hex,FALSE);
	THDebug(ret);
	//ԭʼ���ݴ�ӡģʽ
	ret=THStringConv::FormatString(data,len,THStringConv::FormatType_None,100,THStringConv::FormatType_None,FALSE,TRUE);
	THLongDebug(ret);
</pre>*/
class THStringConv
{
public:
	//�����ַ����У��������""�е��ַ����������滻
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

	//ͳ���ַ������ֵĴ���
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
	* @brief ��ʽ���ַ���Ϊһ����ʾ
	* @param str	ԭ�ַ���
	* @return �������ƺ���ַ���
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
	* @brief ����js���õ���\uf5d1\ud2fc����
	* @param sBuf	ԭ�ַ���
	* @return �����ַ���
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
	* @brief ����js�е�\uf5d1\ud2fc����
	* @param sBuf	ԭ�ַ���
	* @return �����ַ���
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
	* @brief ��ȡ�ַ�����С����С��λΪӢ����ĸ��С�������ּ�Ϊ2��(δ��֤����)
	* @return ���ش�С
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
	* @brief ��ȡ�ַ����ַ����������ּ�Ϊ1��(δ��֤����)
	* @return ���ش�С
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
	* @brief �����ַ�����С��֧�������ִ�������ȷ�������ַ���ռ�ô�С������limitsize��Ӣ����ĸ��С
	* @param ret			ԭ�ַ���
	* @param limitsize		���ƴ�С
	* @param bEndEllipsis	�ض�ʱ�Ƿ����������...
	* @return �������ƺ���ַ���
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
		//unicodeģʽ�²���������
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
	* @brief �����ַ���������֧�������ִ�������ȷ�������ַ����ַ�����Ϊlimitsize��һ�������ּ�Ϊ1��
	* @param ret			ԭ�ַ���
	* @param limitsize		���ƴ�С
	* @param bEndEllipsis	�ض�ʱ�Ƿ����������...
	* @return �������ƺ���ַ���
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
		//unicodeģʽ�²���������
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
	* @brief ת��ȫ�����ֺ�Ӣ�ĵ����
	* @param src	ת�����ַ���
	* @return ����ת������ַ���
	*/
	static THString sbc2tbc(THString src)
	{
		const TCHAR chbuf[][3]={
					_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),
					_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),
					_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),
					_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),
					_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),
					_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),
					_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),
					_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),
					_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),
					_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),_T("")
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
	* @brief ת��������ֺ�Ӣ�ĵ�ȫ��
	* @param src	ת�����ַ���
	* @return ����ת������ַ���
	*/
	static THString tbc2sbc(THString src)
	{
		const TCHAR chbuf[][3]={
					_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),
					_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),
					_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),
					_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),
					_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),
					_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),
					_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),
					_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),
					_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),
					_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),_T("")
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
	* @brief ��ȡ�ַ�����ָ��������
	* @param str			ԭ��
	* @param prefix			ƥ���ǰ׺
	* @param postfix		ƥ��ĺ�׺
	* @return ����ƥ������
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
	* @brief �Զ���token�ֽ��ַ�����������
	* @param str			ԭ��
	* @param token			��token�ַ���
	* @param out			�ָ���
	* @param bCase			��Сд����
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
	* @brief �ж��ַ����Ƿ�Ϊ����
	* @return �����Ƿ�
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
	* @brief �ж��ַ����Ƿ�Ϊ���ֺ�С����
	* @return �����Ƿ�
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
	* @brief �ж��ַ����Ƿ�Ϊ����
	* @return �����Ƿ�
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
	* @brief �ж��ַ����Ƿ�ȫΪ��ĸ����ĳ���ļ�д
	* @return �����Ƿ�
	*/
	static BOOL IsStringShengMu(THString str)
	{
		//��ĸ��b p m f d t n l g k h j q x zh ch sh r z c s y w
		//��ͷ��ĸ��a o e ai ei ao ou er an en ang
		//��abcdefgh[i]jklmnopqrst[uv]wxyz
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
	* @brief �ж��ַ����Ƿ�Ϊ����Ascii�ַ�
	* @return �����Ƿ�
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
	* @brief �ж��ַ����Ƿ�Ϊ��ĸ
	* @return �����Ƿ�
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
	* @brief �ж��ַ����Ƿ�Ϊ���ֺ���ĸ
	* @return �����Ƿ�
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
	* @brief ȡ��������С������0
	* @param val		������
	* @param count		������λ��
	* @return ���ؽ��
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
	* @brief ȡ���������ַ���С������0
	* @param str			ԭ��
	* @return ���ؽ��
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
	* @brief ת�����ִ���֧��10���ƺ�16����0x��h�ı��
	* @param str			ԭ��
	* @return ���ؽ��
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
	* @brief ת�����ִ���֧��10���ƺ�16����0x��h�ı��
	* @param str			ԭ��
	* @return ���ؽ��
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
	* @brief ����������ֵ��ַ���
	* @param str			����ԭ��
	* @param ar				����Ҫ���ҵ��ַ���
	* @param bCase			��Сд����
	* @param StartPos		��ʼ������λ��
	* @param spos			�����ҵ����ַ����Ŀ�ʼλ�ã���Ϊ��
	* @param sFitStr		ƥ����ַ�������Ϊ��
	* @return �����Ƿ��ҵ�
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
	* @brief ����ĳ�ַ������ܳ���n�Σ���������������
	* @param orgstr			ԭ�ַ���
	* @param token			�����Ƶ��ַ���
	* @param tokenlimit		���ƴ���
	* @return �������ƺ���ַ���
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
	* @brief ��������ĳ�ַ������ܳ���n�Σ���������������
	* @param orgstr			ԭ�ַ���
	* @param token			�����Ƶ��ַ���
	* @param tokenlimit		���ƴ���
	* @return �������ƺ���ַ���
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

	//nLen ��in out��
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

	//nLen ��in out��
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
		FormatType_Hex,//16������ʾ
		FormatType_Dec,//10������ʾ
		FormatType_Oct,//8������ʾ
		FormatType_Bin,//2������ʾ
		FormatType_None//����ʾ
	}FormatType;
	/**
	* @brief ��ʽ����������
	* @param pData			����ָ��
	* @param nLen			���ݳ���
	* @param ft				������ʾ��ʽ
	* @param nLineCount		ÿ����ʾ������
	* @param ftOffest		ƫ������ʾ��ʽ��FormatType_NoneΪ����ʾ
	* @param bMiddleSep		�м��Ƿ���Ҫ�ָ�
	* @param bHaveRawData	�Ƿ��ӡԭʼ����,'\\' �� '%' �������ӡ
	* @return ���ظ�ʽ���ַ���
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
* @brief ��Xml�ַ���������
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-06-03 �½���
* @2007-06-14 SetParam������Ϊ����THStringָ��
* @2008-02-21 ��SetParam����ֵΪ��ʱ������RemoveParam�������ɽ�ʡ�ڴ�
*/
/**<pre>
ʹ��Sample��
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
	* @brief ��ȡ�����еļ�����
	* @param content	ԭ����
	* @param key		����
	* @param f			��ֵ��ʼ
	* @param e			��ֵ����
	* @return ��������
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
	* @brief ��ȡ�����еļ����ݲ�ɾ����
	* @param content	ԭ����
	* @param key		����
	* @param f			��ֵ��ʼ
	* @param e			��ֵ����
	* @return ��������
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
	* @brief ɾ����
	* @param content	ԭ����
	* @param key		����
	* @param f			��ֵ��ʼ
	* @param e			��ֵ����
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
	* @brief ���¼�ֵ����
	* @param content	�ַ�����
	* @param key		����
	* @param value		��ֵ
	* @param f			��ֵ��ʼ
	* @param e			��ֵ����
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
	* @brief ���ɼ�ֵ����
	* @param key		����
	* @param value		��ֵ
	* @param f			��ֵ��ʼ
	* @param e			��ֵ����
	* @return ��������
	*/
	static THString MakeParam(const THString key,const THString value,const THString f=_T("<"),const THString e=_T(">"))
	{
		return f+key+e+value+f+_T("/")+key+e;
	}
	/**
	* @brief ��xml���ݰ���ʽ�����ַ���
	* @param sContent	xml����
	* @param sStyle		������ʽ����(KEY:VALUE)������KEY��VALUE����ʵ�����滻
	* @param f			��ֵ��ʼ
	* @param e			��ֵ����
	* @return ��������
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
	* @brief ��xml���ݰ���ʽ�����ַ���
	* @param sContent	xml����
	* @param sKey		����Key����
	* @param sValue		����Value����
	* @param f			��ֵ��ʼ
	* @param e			��ֵ����
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
	* @brief ��xml���ݰ���ʽ�����ַ���
	* @param sKey		Key����
	* @param sValue		Value����
	* @param f			��ֵ��ʼ
	* @param e			��ֵ����
	* @return Xml����
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
* @brief ����Xml�ַ���������
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-08-06 �½���
* @2007-08-13 ����һ��bug
* @2007-08-21 ֧�ִ�Сд�޹�
*/
/**<pre>
ʹ��Sample��
</pre>*/
class THTiniXml
{
public:
	THTiniXml(){}
	THTiniXml(THString content,BOOL bCase=FALSE){SetContent(content,bCase);}
	virtual ~THTiniXml(){}

	/**
	* @brief ����Xml����
	* @param content		����
	* @param bCase			�Ƿ����ִ�Сд
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
	* @brief ��ȡXml����
	* @param content		����
	* @return �ַ���
	*/
	THString GetContent(){return m_content;}

	/**
	* @brief ��ȡ��ǰ��λ��Xml����
	* @param content		����
	* @return �ַ���
	*/
	THString GetCurrent(){return m_current;}

	/**
	* @brief �ѵ�ǰ���ݷֽ��������Key�����浽������
	* @param ar			��������
	* @return �Ƿ�ɹ�
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
				//����xml����
				//ȷ��key����
				searchar.RemoveAll();
				searchar.Add(_T("/>"));
				searchar.Add(_T(">"));
				if (THStringConv::MultiFind(tmp,&searchar,m_bCase,pos,&spos,&tmpkey))
				{
					if (tmpkey==_T("/>"))
					{
						//˵����ֵ��ʽ��<key />��
						spos+=2;
					}
					else if (tmpkey==_T(">"))
					{
						//˵���ҵ���ֵ�Ŀ�ͷ����ȷ�ϼ�ֵ����
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
	* @brief ��������
	* @param key			����
	* @param start			ƫ��λ�ã���ʹ��GetKeyValue��pos
	* @return �Ƿ�ɹ�
	*/
	BOOL IntoXml(THString key,int start=0)
	{
		if (!m_bCase) key.MakeLower();
		THString startkey=_T("<")+key+_T(">");
		THString startval=_T("<")+key+_T(" ");
		THString endkey=_T("</")+key+_T(">");
		int pos=m_currentcomp.Find(startkey,start);
		int pos1=m_currentcomp.Find(startval,start);
		//���pos1��ʽ�ı�pos��ʽ�Ļ�ǰ��ʹ��pos1
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
	* @brief ���ص���Ŀ¼
	*/
	void ResetToRoot()
	{
		m_current=m_content;
		m_currentcomp=m_current;
		if (!m_bCase) m_currentcomp.MakeLower();
	}

	/**
	* @brief ��ȡָ������Ӧ��ָ������
	* @param key			����
	* @param attrib			������
	* @return �����ַ���
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
	* @brief ��ȡָ������Ӧ������
	* @param key			����
	* @param retpos			ָ��ָ����ֵΪ��ʼ������λ�ã�ָ��Ϊ��Ϊ��ָ�������ؽ��Ϊ�´ε�λ�ã�����-1Ϊû���ҵ�
	* @return �����ַ���
	*/
	/**<pre>
		��ȷ��ȡ��ʽ:<key>content</key>
		��ȷ��ȡ��ʽ:<key value="">content</key>
		�����ȡ��ʽ:<key value=""/>
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
		//���pos1��ʽ�ı�pos��ʽ�Ļ�ǰ��ʹ��pos1
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
	* @brief ö������ָ������Ӧ������
	* @param key			����
	* @param ar				���ص�����
	* @return �Ƿ�ɹ�
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
* @brief ���ݲ��Ҷ�λ��
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-07-01 �½���
*/
/**<pre>
�÷���
</pre>*/
class THBinSearch
{
public:
	/**
	* @brief ��������������
	* @param buf		ԭ����
	* @param len		ԭ���ݳ���
	* @param search		Ҫ����������
	* @param searchlen	Ҫ�������ݳ���
	* @return �ҵ��ط���ָ�룬�Ҳ�������NULL
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
	* @brief ���������������е��ı�
	* @param buf		ԭ����
	* @param len		ԭ���ݳ���
	* @param search		Ҫ����������
	* @return �ҵ��ط���ָ�룬�Ҳ�������NULL
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
	* @brief ���������������е��ı������Դ�Сд��
	* @param buf		ԭ����
	* @param len		ԭ���ݳ���
	* @param search		Ҫ����������
	* @return �ҵ��ط���ָ�룬�Ҳ�������NULL
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
* @brief ���ʽ���Ҷ�λ��
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-07-01 �½���
*/
/**<pre>
�÷���
</pre>*/
class THMatchSearch
{
public:
	/**
	* @brief ���ַ�����ȡ��ָ����ʽ�ַ�
	* @param str		ԭ���ݣ����ҹ��̻��дstr��ֵ
	* @param match		ƥ����ʽ��%sΪ��ȡ����Ϊ�ַ�����%dΪ��ȡ����Ϊ���֣�%xΪ�ö��������⣬����ȡ��%%Ϊƥ��%�ţ��ո���ʹ��[ ]����ʾƥ��һ�������ո񣬿ɸ�1-9�����޶���ȣ��磺%1d
	* @param ar			���ص�����
	* @return �Ƿ��ҵ�����ֵ
	*/
	/**
		match ƥ����ʽ�﷨
		[%2d-%2d %2d:%2d:%2d.%x][%x]->[%11d]:%s\r\n
	*/
	static BOOL MatchSearch(THString *str,THString match,THStringArray *ar)
	{
		if (!str || !ar) return FALSE;
		//�ֽ�match��
		THStringArray mar;//����ÿ�ε�ƥ��ؼ���
		THStringArray kar;//����ÿ�εĻ�ȡ����
		THStringArray lar;//����ÿ�εĿ��ƥ��
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
						lar.Add(_T("0"));//������
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
							lar.Add(tmpstr);//����
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
				//limit ģʽ
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
	//�����ַ������Զ�ո��ַ����д���
	static int MatchFind(THString str,THString found,int *poslen,int startpos)
	{
		int pos=found.Find(_T("[ ]"),0);
		if (pos!=-1)
		{
			//��Ҫ�����ո�����
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
					//�����Ҫֱ��ƥ��ո����Ȳ��ҿո�
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
							//����ƥ��ո�
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
						//�������Ŀո��С
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
