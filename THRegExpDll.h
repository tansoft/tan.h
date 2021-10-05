#pragma once

#include <THString.h>
#include <THDllLoader.h>

typedef BOOL (*fnCheckRegExp)(const char* szStr,const char* szExpr);

#define RegExpEmail	_T("^([w-.]+)@(([[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}.)|(([w-]+.)+))([a-zA-Z]{2,4}|[0-9]{1,3})(]?)$") ///<ƥ��Email��\\w+([-+.]\\w+)*@\\w+([-.]\\w+)*\\.\\w+([-.]\\w+)*
#define RegExpAlphabetAndNumber _T("^[A-Za-z0-9]+$")								///<ƥ�����ֺ�Ӣ��
#define RegExpAlphabet _T("^[A-Za-z]+$")											///<ƥ��Ӣ����
#define RegExpAlphabetUp _T("^[A-Z]+$")												///<ƥ���дӢ����
#define RegExpAlphabetLow _T("^[a-z]+$")											///<ƥ��СдӢ����
#define RegExpChineseCharset _T("[\\u4e00-\\u9fa5]")								///<ƥ��������
#define RegExpDoubleChar _T("[^\\x00-\\xff]")										///<ƥ��˫�ֽ�����
#define RegExpEmptyRow _T("\\n\\s*\\r")												///<ƥ��հ��У�\\n[\\s| ]*\\r
#define RegExpHtmlTag _T("<(\\S*?)[^>]*>.*?</\\1>|<.*? />")							///<ƥ��xml/html��ֵ<key></key>��/<(.*)>.*<\\/\\1>|<(.*) \/>/
#define RegExpTrim _T("^\\s*|\\s*$")												///<ƥ����ǰ�к�Ŀո��Ʊ������ҳ��
//RegExpUrl valid ok
#define RegExpUrl _T("({[^:/?#]+}:)?(//{[^/?#]*})?{[^?#]*}(?{[^#]*})?(#{.*})?")		///<ƥ��Url��^[a-zA-z]+://(\\w+(-\\w+)*)(\\.(\\w+(-\\w+)*))*(\\?\\S*)?$ �� [a-zA-z]+://[^\\s]*
#define RegExpHttp _T("^http:\\/\\/[A-Za-z0-9]+\\.[A-Za-z0-9]+[\\/=\\?%\\-&_~`@[\\]\\':+!]*([^<>\"\"])*$g")	///<ƥ��http
#define RegExpUserName _T("^[a-zA-Z][a-zA-Z0-9_]{2,15}$")							///<ƥ�䳣���û���(��ĸ��ͷ������3-16�ֽڣ�������ĸ�����»���)
#define RegExpPhoneNumber _T("\\d{3}-\\d{8}|\\d{4}-\\d{7}")							///<ƥ�����ź͵绰����
#define RegExpPostalCode _T("[1-9]\\d{5}(?!\\d)")									///<ƥ���й�6λ��������
#define RegExpIpAddress _T("^(d{1,2}|1dd|2[0-4]d|25[0-5]).(d{1,2}|1dd|2[0-4]d|25[0-5]).(d{1,2}|1dd|2[0-4]d|25[0-5]).(d{1,2}|1dd|2[0-4]d|25[0-5])$")	///<ƥ��Ip��ַ��\\d+\\.\\d+\\.\\d+\\.\\d+
#define RegExpIntegerPositive _T("^[1-9]\\d*$")										///<ƥ��������
#define RegExpIntegerNegative _T("^-[1-9]\\d*$")									///<ƥ�为����
#define RegExpInteger _T("^-?[1-9]\\d*$")											///<ƥ������
#define RegExpIntegerNotNegative _T("^[1-9]\\d*|0$")								///<ƥ��Ǹ������������� �� 0��
#define RegExpIntegerNotPositive _T("^-[1-9]\\d*|0$")								///<ƥ����������������� �� 0��
#define RegExpFloat _T("^-?([1-9]\\d*\\.\\d*|0\\.\\d*[1-9]\\d*|0?\\.0+|0)$")		///<ƥ�両����
#define RegExpDateYMD _T("^(d{2}|d{4})-((0([1-9]{1}))|(1[1|2]))-(([0-2]([1-9]{1}))|(3[0|1]))$")	///<ƥ����-��-��
#define RegExpDateMDY _T("^((0([1-9]{1}))|(1[1|2]))/(([0-2]([1-9]{1}))|(3[0|1]))/(d{2}|d{4})$")	///<ƥ����/��/��

/**
* @brief ������ʽDll��װ��
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-06-07 �½���
*/
/**<pre>
�÷���
	THRegExpDll m_regexp;
	THString str1=_T("barrytan@21cn.com");
	if (m_regexp.CheckRegExp(str1,THRegExpDll::RegExpEmail))
		THDebug(_T("%s match with email RegExp"),str1);
������ʽ�﷨��
	\ ����һ���ַ����Ϊһ�������ַ�����һ��ԭ���ַ�����һ���������á���һ���˽���ת�����
	^ ƥ�������ַ����Ŀ�ʼλ�á���������� RegExp �����Multiline ���ԣ�^ Ҳƥ�� ��\n�� �� ��\r�� ֮���λ�á� 
	$ ƥ�������ַ����Ľ���λ�á���������� RegExp �����Multiline ���ԣ�$ Ҳƥ�� ��\n�� �� ��\r�� ֮ǰ��λ�á� 
	* ƥ��ǰ����ӱ��ʽ��λ��Ρ� 
	+ ƥ��ǰ����ӱ��ʽһ�λ��Ρ�+ �ȼ��� {1,}�� 
	? ƥ��ǰ����ӱ��ʽ��λ�һ�Ρ�? �ȼ��� {0,1}�� 
	{n} n ��һ���Ǹ�������ƥ��ȷ����n �Ρ�
	{n,} n ��һ���Ǹ�����������ƥ��n �Ρ� 
	{n,m} m �� n ��Ϊ�Ǹ�����������n <= m������ƥ�� n �������ƥ�� m �Ρ��ڶ��ź�������֮�䲻���пո�
	? �����ַ��������κ�һ���������Ʒ� (*, +, ?, {n}, {n,}, {n,m}) ����ʱ��ƥ��ģʽ�Ƿ�̰���ġ���̰��ģʽ�������ٵ�ƥ�����������ַ�������Ĭ�ϵ�̰��ģʽ�򾡿��ܶ��ƥ�����������ַ����� 
	. ƥ��� "\n" ֮����κε����ַ���Ҫƥ����� ��\n�� ���ڵ��κ��ַ�����ʹ���� ��[.\n]�� ��ģʽ�� 
	(pattern) ƥ��pattern ����ȡ��һƥ�䡣 
	(?:pattern) ƥ��pattern ������ȡƥ������Ҳ����˵����һ���ǻ�ȡƥ�䣬�����д洢���Ժ�ʹ�á� 
	(?=pattern) ����Ԥ�飬���κ�ƥ�� pattern ���ַ�����ʼ��ƥ������ַ���������һ���ǻ�ȡƥ�䣬Ҳ����˵����ƥ�䲻��Ҫ��ȡ���Ժ�ʹ�á� 
	(?!pattern) ����Ԥ�飬��(?=pattern)�����෴ 
	x|y ƥ�� x �� y�� 
	[xyz] �ַ����ϡ� 
	[^xyz] ��ֵ�ַ����ϡ� 
	[a-z] �ַ���Χ��ƥ��ָ����Χ�ڵ������ַ��� 
	[^a-z] ��ֵ�ַ���Χ��ƥ���κβ���ָ����Χ�ڵ������ַ��� 
	\b ƥ��һ�����ʱ߽磬Ҳ����ָ���ʺͿո���λ�á�
	\B ƥ��ǵ��ʱ߽硣 
	\cx ƥ����xָ���Ŀ����ַ��� 
	\d ƥ��һ�������ַ����ȼ��� [0-9]�� 
	\D ƥ��һ���������ַ����ȼ��� [^0-9]�� 
	\f ƥ��һ����ҳ�����ȼ��� \x0c �� \cL�� 
	\n ƥ��һ�����з����ȼ��� \x0a �� \cJ�� 
	\r ƥ��һ���س������ȼ��� \x0d �� \cM�� 
	\s ƥ���κοհ��ַ��������ո��Ʊ������ҳ���ȵȡ��ȼ���[ \f\n\r\t\v]�� 
	\S ƥ���κηǿհ��ַ����ȼ��� [^ \f\n\r\t\v]�� 
	\t ƥ��һ���Ʊ�����ȼ��� \x09 �� \cI�� 
	\v ƥ��һ����ֱ�Ʊ�����ȼ��� \x0b �� \cK�� 
	\w ƥ������»��ߵ��κε����ַ����ȼ��ڡ�[A-Za-z0-9_]���� 
	\W ƥ���κηǵ����ַ����ȼ��� ��[^A-Za-z0-9_]���� 
	\xn ƥ�� n������ n Ϊʮ������ת��ֵ��ʮ������ת��ֵ����Ϊȷ�����������ֳ���
	\num ƥ�� num������num��һ����������������ȡ��ƥ������á� 
	\n ��ʶһ���˽���ת��ֵ��һ���������á���� \n ֮ǰ���� n ����ȡ���ӱ��ʽ���� n Ϊ�������á�������� n Ϊ�˽������� (0-7)���� n Ϊһ���˽���ת��ֵ�� 
	\nm ��ʶһ���˽���ת��ֵ��һ���������á���� \nm ֮ǰ������is preceded by at least nm ����ȡ���ӱ��ʽ���� nm Ϊ�������á���� \nm ֮ǰ������ n ����ȡ���� n Ϊһ��������� m �ĺ������á����ǰ��������������㣬�� n �� m ��Ϊ�˽������� (0-7)���� \nm ��ƥ��˽���ת��ֵ nm�� 
	\nml ��� n Ϊ�˽������� (0-3)���� m �� l ��Ϊ�˽������� (0-7)����ƥ��˽���ת��ֵ nml�� 
	\un ƥ�� n������ n ��һ�����ĸ�ʮ���������ֱ�ʾ��Unicode�ַ���

	*			��������ַ����ַ���
	+			���ơ�*����������Ҫһ���ַ�
	.			ƥ���κ��ַ�
	[-]			ƥ�䷶Χ����[a-zA-Z_0-9]ƥ����ĸ�����ֻ��»���
	\w			����
	\w+			ƥ��������һ���ַ��ĵ����ַ�����
	^			��ʼ
	$			��β
	\d			ƥ������
	{n}			ƥ���ظ�n��
	^5\d{15}$	ƥ��5��ͷ��16λ����
	{}			���ô������Ա��ʽ���з���
	{0,1}		�ظ�0�λ�1�Σ�������д���ʺ� ?��
	^\d{5}(-\d{4}){0,1}$	������ֺ��ظ�0�λ�1�Σ�����5�����ֻ�5�����ּ����ۺź���ĸ����֣�
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