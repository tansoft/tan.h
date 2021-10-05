#pragma once

#include <THString.h>
#include <THDllLoader.h>

typedef BOOL (*fnCheckRegExp)(const char* szStr,const char* szExpr);

#define RegExpEmail	_T("^([w-.]+)@(([[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}.)|(([w-]+.)+))([a-zA-Z]{2,4}|[0-9]{1,3})(]?)$") ///<匹配Email，\\w+([-+.]\\w+)*@\\w+([-.]\\w+)*\\.\\w+([-.]\\w+)*
#define RegExpAlphabetAndNumber _T("^[A-Za-z0-9]+$")								///<匹配数字和英文
#define RegExpAlphabet _T("^[A-Za-z]+$")											///<匹配英文字
#define RegExpAlphabetUp _T("^[A-Z]+$")												///<匹配大写英文字
#define RegExpAlphabetLow _T("^[a-z]+$")											///<匹配小写英文字
#define RegExpChineseCharset _T("[\\u4e00-\\u9fa5]")								///<匹配中文字
#define RegExpDoubleChar _T("[^\\x00-\\xff]")										///<匹配双字节内容
#define RegExpEmptyRow _T("\\n\\s*\\r")												///<匹配空白行，\\n[\\s| ]*\\r
#define RegExpHtmlTag _T("<(\\S*?)[^>]*>.*?</\\1>|<.*? />")							///<匹配xml/html键值<key></key>，/<(.*)>.*<\\/\\1>|<(.*) \/>/
#define RegExpTrim _T("^\\s*|\\s*$")												///<匹配行前行后的空格、制表符、换页符
//RegExpUrl valid ok
#define RegExpUrl _T("({[^:/?#]+}:)?(//{[^/?#]*})?{[^?#]*}(?{[^#]*})?(#{.*})?")		///<匹配Url，^[a-zA-z]+://(\\w+(-\\w+)*)(\\.(\\w+(-\\w+)*))*(\\?\\S*)?$ ， [a-zA-z]+://[^\\s]*
#define RegExpHttp _T("^http:\\/\\/[A-Za-z0-9]+\\.[A-Za-z0-9]+[\\/=\\?%\\-&_~`@[\\]\\':+!]*([^<>\"\"])*$g")	///<匹配http
#define RegExpUserName _T("^[a-zA-Z][a-zA-Z0-9_]{2,15}$")							///<匹配常规用户名(字母开头，允许3-16字节，允许字母数字下划线)
#define RegExpPhoneNumber _T("\\d{3}-\\d{8}|\\d{4}-\\d{7}")							///<匹配区号和电话号码
#define RegExpPostalCode _T("[1-9]\\d{5}(?!\\d)")									///<匹配中国6位邮政编码
#define RegExpIpAddress _T("^(d{1,2}|1dd|2[0-4]d|25[0-5]).(d{1,2}|1dd|2[0-4]d|25[0-5]).(d{1,2}|1dd|2[0-4]d|25[0-5]).(d{1,2}|1dd|2[0-4]d|25[0-5])$")	///<匹配Ip地址，\\d+\\.\\d+\\.\\d+\\.\\d+
#define RegExpIntegerPositive _T("^[1-9]\\d*$")										///<匹配正整数
#define RegExpIntegerNegative _T("^-[1-9]\\d*$")									///<匹配负整数
#define RegExpInteger _T("^-?[1-9]\\d*$")											///<匹配整数
#define RegExpIntegerNotNegative _T("^[1-9]\\d*|0$")								///<匹配非负整数（正整数 和 0）
#define RegExpIntegerNotPositive _T("^-[1-9]\\d*|0$")								///<匹配非正整数（负整数 和 0）
#define RegExpFloat _T("^-?([1-9]\\d*\\.\\d*|0\\.\\d*[1-9]\\d*|0?\\.0+|0)$")		///<匹配浮点数
#define RegExpDateYMD _T("^(d{2}|d{4})-((0([1-9]{1}))|(1[1|2]))-(([0-2]([1-9]{1}))|(3[0|1]))$")	///<匹配年-月-日
#define RegExpDateMDY _T("^((0([1-9]{1}))|(1[1|2]))/(([0-2]([1-9]{1}))|(3[0|1]))/(d{2}|d{4})$")	///<匹配月/日/年

/**
* @brief 正则表达式Dll封装类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-06-07 新建类
*/
/**<pre>
用法：
	THRegExpDll m_regexp;
	THString str1=_T("barrytan@21cn.com");
	if (m_regexp.CheckRegExp(str1,THRegExpDll::RegExpEmail))
		THDebug(_T("%s match with email RegExp"),str1);
正则表达式语法：
	\ 将下一个字符标记为一个特殊字符、或一个原义字符、或一个后向引用、或一个八进制转义符。
	^ 匹配输入字符串的开始位置。如果设置了 RegExp 对象的Multiline 属性，^ 也匹配 ’\n’ 或 ’\r’ 之后的位置。 
	$ 匹配输入字符串的结束位置。如果设置了 RegExp 对象的Multiline 属性，$ 也匹配 ’\n’ 或 ’\r’ 之前的位置。 
	* 匹配前面的子表达式零次或多次。 
	+ 匹配前面的子表达式一次或多次。+ 等价于 {1,}。 
	? 匹配前面的子表达式零次或一次。? 等价于 {0,1}。 
	{n} n 是一个非负整数，匹配确定的n 次。
	{n,} n 是一个非负整数，至少匹配n 次。 
	{n,m} m 和 n 均为非负整数，其中n <= m。最少匹配 n 次且最多匹配 m 次。在逗号和两个数之间不能有空格。
	? 当该字符紧跟在任何一个其他限制符 (*, +, ?, {n}, {n,}, {n,m}) 后面时，匹配模式是非贪婪的。非贪婪模式尽可能少的匹配所搜索的字符串，而默认的贪婪模式则尽可能多的匹配所搜索的字符串。 
	. 匹配除 "\n" 之外的任何单个字符。要匹配包括 ’\n’ 在内的任何字符，请使用象 ’[.\n]’ 的模式。 
	(pattern) 匹配pattern 并获取这一匹配。 
	(?:pattern) 匹配pattern 但不获取匹配结果，也就是说这是一个非获取匹配，不进行存储供以后使用。 
	(?=pattern) 正向预查，在任何匹配 pattern 的字符串开始处匹配查找字符串。这是一个非获取匹配，也就是说，该匹配不需要获取供以后使用。 
	(?!pattern) 负向预查，与(?=pattern)作用相反 
	x|y 匹配 x 或 y。 
	[xyz] 字符集合。 
	[^xyz] 负值字符集合。 
	[a-z] 字符范围，匹配指定范围内的任意字符。 
	[^a-z] 负值字符范围，匹配任何不在指定范围内的任意字符。 
	\b 匹配一个单词边界，也就是指单词和空格间的位置。
	\B 匹配非单词边界。 
	\cx 匹配由x指明的控制字符。 
	\d 匹配一个数字字符。等价于 [0-9]。 
	\D 匹配一个非数字字符。等价于 [^0-9]。 
	\f 匹配一个换页符。等价于 \x0c 和 \cL。 
	\n 匹配一个换行符。等价于 \x0a 和 \cJ。 
	\r 匹配一个回车符。等价于 \x0d 和 \cM。 
	\s 匹配任何空白字符，包括空格、制表符、换页符等等。等价于[ \f\n\r\t\v]。 
	\S 匹配任何非空白字符。等价于 [^ \f\n\r\t\v]。 
	\t 匹配一个制表符。等价于 \x09 和 \cI。 
	\v 匹配一个垂直制表符。等价于 \x0b 和 \cK。 
	\w 匹配包括下划线的任何单词字符。等价于’[A-Za-z0-9_]’。 
	\W 匹配任何非单词字符。等价于 ’[^A-Za-z0-9_]’。 
	\xn 匹配 n，其中 n 为十六进制转义值。十六进制转义值必须为确定的两个数字长。
	\num 匹配 num，其中num是一个正整数。对所获取的匹配的引用。 
	\n 标识一个八进制转义值或一个后向引用。如果 \n 之前至少 n 个获取的子表达式，则 n 为后向引用。否则，如果 n 为八进制数字 (0-7)，则 n 为一个八进制转义值。 
	\nm 标识一个八进制转义值或一个后向引用。如果 \nm 之前至少有is preceded by at least nm 个获取得子表达式，则 nm 为后向引用。如果 \nm 之前至少有 n 个获取，则 n 为一个后跟文字 m 的后向引用。如果前面的条件都不满足，若 n 和 m 均为八进制数字 (0-7)，则 \nm 将匹配八进制转义值 nm。 
	\nml 如果 n 为八进制数字 (0-3)，且 m 和 l 均为八进制数字 (0-7)，则匹配八进制转义值 nml。 
	\un 匹配 n，其中 n 是一个用四个十六进制数字表示的Unicode字符。

	*			零个或多个字符的字符串
	+			类似“*”，但至少要一个字符
	.			匹配任何字符
	[-]			匹配范围，如[a-zA-Z_0-9]匹配字母、数字或下划线
	\w			单词
	\w+			匹配至少有一个字符的单词字符序列
	^			开始
	$			结尾
	\d			匹配数字
	{n}			匹配重复n次
	^5\d{15}$	匹配5开头的16位数字
	{}			可用大括弧对表达式进行分组
	{0,1}		重复0次或1次（可以缩写成问号 ?）
	^\d{5}(-\d{4}){0,1}$	五个数字后重复0次或1次（即纯5个数字或5个数字加破折号后跟四个数字）
</pre>*/
class THRegExpDll : public THDllLoader
{
public:
	THRegExpDll():THDllLoader(_T("RegExp.dll")){EmptyDllPointer();}
	virtual ~THRegExpDll(){FreeDll();}

	virtual void EmptyDllPointer()
	{
		m_fnCheckRegExp=NULL;
	}

	virtual BOOL InitDllPoinier()
	{
		m_fnCheckRegExp=(fnCheckRegExp)GetProcAddress(m_module,"CheckRegExp");
		if (!m_fnCheckRegExp) return FALSE;
		return TRUE;
	}

	BOOL CheckRegExp(const THString sContent,const THString sRegExp)
	{
		if (!InitDll()) return FALSE;
		if (!m_fnCheckRegExp) return FALSE;
		if (sContent.IsEmpty() || sRegExp.IsEmpty()) return FALSE;
		char *tmpbuf1=new char[sContent.GetLength()+1];
		char *tmpbuf2=new char[sRegExp.GetLength()+1];
		THCharset::t2a(sContent,tmpbuf1,sContent.GetLength()+1);
		THCharset::t2a(sRegExp,tmpbuf2,sRegExp.GetLength()+1);
		BOOL ret=(m_fnCheckRegExp)(tmpbuf1,tmpbuf2);
		if (tmpbuf1) delete tmpbuf1;
		if (tmpbuf2) delete tmpbuf2;
		return ret;
	}
private:
	HMODULE m_module;
	fnCheckRegExp m_fnCheckRegExp;
};