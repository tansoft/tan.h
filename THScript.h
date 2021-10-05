#pragma once

#include <THSystem.h>
#include <THString.h>
#include <THFileStore.h>
#include <THIni.h>
#include <THLogicCalc.h>
#include <THTime.h>
#include <THSqliteDll.h>
#include <THStruct.h>
#include <THThread.h>
#include <THSysCallRapper.h>

//#ifdef _DEBUG
//#define THSCRIPT_DEBUG
//#endif

#ifdef THSCRIPT_DEBUG
#include <THDebug.h>
#endif

class ITHScriptDebugger
{
public:
	//����TRUE��ʾ���Ըô���
	virtual BOOL OnScriptErrorHandler(int nErrCode,THString sErrText){return FALSE;}
	//����ʼ���ú���ʱ������nRecursion��ʾ�ݹ������ע����Timer,Schedule���ⲿ��������ʱ���ݹ鶼��0��ʼ����
	//����FALSE��ʾ�˳��ú���ִ��
	virtual BOOL OnEnterFunction(THString sCurrentFn,int nRecursion,int localidx,THString *tempsetting){return TRUE;}
	//���������ú���ʱ������sRet��ʾ��������ֵ
	virtual void OnLeaveFunction(THString sCurrentFn,int nRecursion,THString *sRet,int localidx){}
	//����FALSE��ʾ��ִ�иþ�ű�
	virtual BOOL OnRunScript(THString script,int localidx,THString *tempsetting){return TRUE;}
	//����tag
	virtual void OnEnterTag(THString fn,THString tag,THString *script){}
};

class THScript;
typedef BOOL (*THParseExtCmdProc)(THScript *script,THString cmd,THStringToken *tokenvar,THString *tempsetting,int localidx,const void *adddata);

/**
* @brief �ű�����������
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-06-13 �½���
* @2007-06-18 ���ӽ������汾�жϣ�ʹ���ĵ�׫д
* @2007-07-04 ����Sqlite���ݿ�֧�֣�����ϵͳĿ¼·��������ʹ���ĵ�����
* @2007-07-10 ���ӳ�ʼ�����ú���InitCall���͹����ú���FreeCall�����Ӷ����滻�꣬ʹ���ĵ�����
* @2007-07-24 ���Ӷ����õ���֧�֣�ʹ���ĵ�����
* @2007-07-31 ���Ӷ�ʱ��֧�֣�ʹ���ĵ�����
* @2007-08-01 �����ⲿ������ͽӿڣ�֧��include���ؽű��Ľ��ͽӿڴ��ݣ�ʹ���ĵ�����
* @2007-08-20 ����sqlite������openex��֧�ֲ����ڽű��ļ�ʱ���н���ʹ���ĵ�����
* @2007-08-22 ���ӵ���ģʽ��������н��
* @2007-08-31 ����THScript-UltraEdit.txt��֧��THScript��UltraEdit�и�����ʾ
* @2007-09-09 �����ļ��洢��֧�֣�ע��sqlite���ݿ������֧�ִ������ʹ洢��
* @2007-09-12 ���Ӹ���time32�����ʹ���ĵ�����
* @2007-09-17 ��������resetglobalsetting��ʹ���ĵ�����
* @2007-11-03 THScript-UltraEdit.txt�ļ�����
* @2007-11-18 ���Ӹ���time32�����ʹ���ĵ�����
* @2007-11-22 ����֧�ּƻ���������file�����ʹ���ĵ���THScript-UltraEdit.txt����
* @2008-05-07 ����setintu�������sqlite������exporttable������str������formatnum��ʹ���ĵ���THScript-UltraEdit.txt����
* @2009-09-27 ����syscall������ڵ���ϵͳAPI��ʹ���ĵ���THScript-UltraEdit.txt���£����İ汾180
* @2009-09-28 ���ӵ���ģʽ֧��
*/
/**<pre>
ʹ�÷�����
	THScript script;
	THString varsettings;
	script.Init("c:\\a.txt");
	varsettings+=THSimpleXml::MakeParam(_T("arg1"),FetionNum);
	varsettings+=THSimpleXml::MakeParam(_T("arg2"),PhoneNum);
	varsettings+=THSimpleXml::MakeParam(_T("arg3"),UserName);
	varsettings+=THSimpleXml::MakeParam(_T("arg4"),Msg);
	THString ret=m_script.RunScript(_T("NewInstance"),1234,varsettings);
	ret=m_script.GetLocalSettings(1234,_T("l_retstr"));
�﷨��
	�ļ��ṹ���ű��ļ��ṹʹ��Windows��׼ini�ṹ��ɡ�
		�������£�
			//����TagName
			[TagName]
			start=XXX;

			//����
			[TagData]
			data1=XXX;
		������[]�е����ֳ�Ϊ������ÿ�������ж������ݣ��磺start���data1�start��data1��Ϊ������XXX��Ϊ��ֵ��//���ڳ���ע��
		�ļ��к��������ݵĶ��嶼��һ���ģ������Ķ��б�����start����п�ʼ��
	�������κνű����ɶ�������κ����ݶ���ɣ����������ݶ���ѭini�ļ��﷨������д����û��ʲô����
		���ýű�ͨ�����þ��庯�������У����û��ָ����Ҫ���õĺ�����Ĭ�ϵ����ļ��еĺ���ScriptMain��
		����ĳ����ʵ���������к����е�start�����еĽű���ע��ϵͳֻ��ִ������нű���������˳��ִ�������ű�����˶��֧���߼���ͨ���ڽű��жԲ�ͬ������в�ͬ�Ĵ���������ò�ͬ�����Ľű���ִ�С�
			�������£�
				start=goto:tag1;
				tag1=XXX;
			��start����ת��tag1������ִ�С�
		�����ķ���ֵ��ָ��end���������ָ��end��
		��������ʼ��ʱ�������Լ�����InitCall�������г�ʼ���������������͹�ʱ����FreeCall���������ͷš����ļ�����ʱ�ֱ����ļ���include:init��include:freeʱ����InitCall��FreeCall�����includeָ�
	��䣺����﷨��ָ��ȷ�����ִ�е�����,��:��,���ݲ���,��;��ʾ����,ͬһ���ڿ�ִ�ж��ű���
			ָ��[:����1[,����2,[����n...]]][;][ָ��[:����1[,����2,[����n...]]][;]]
		�������£�
			end		//���һ��ָ���Ҫ�ӷֺ�Ҳ��
			end;
			end:1
			end:1;
			if:1,=,2,tag1,tag2;end
	��ת��������������ת����������ʱ���������﷨�������£�
			����1[~������ü���]
		����ת������1���������1�����ڣ����ó�����ü�����������ü�������ʡ�ԡ�
		ע�⣺�������_continue��ʾ�������и��е���һ��ָ���������ת��
		������ʹ�ñ���ȷ����
		�������£�
			goto:tagnext;
			goto:tagnext~tagerror;
			goto:_continue;
			goto:tagnext~_continue;
			goto:$state$;
			//��nowtype����ָ����ת��tag
			goto tagstate$nowtype$;
	������������$��ɣ���������ʹ��һ��[]��ʾ��Ҫ�Ƚ���[]�еı������������£�
			$������[���ȱ�����]$
			�������£�
				//����mystateֵΪ1����ô����������mystate ��ֵΪ��1
				mystate ��ֵΪ��$mystate$
				//����������$tag1$�������������
				$tag[mystate]$
			����$$������ʵ$���š�
	�ַ������ַ�����������ֱ����д��
			�ַ�������Ҫ�������ֱ����д$������$����Ҫ$��������$$��
			����\Ϊת���ַ������\������\\����������궨�����£�
				\r	�س���
				\n	���з�
				\t	Tab
				\s	�ո񣨻�ֱ������ո�
				\q  ,
				\f	;
				\m	:
				\e	���滻�κ����ݣ����ڰ�$$�ֿ����磺$var$\e$var1$
	���������ڣ���Ч��Χ������������Ϊ4������
		1.ȫ�ֱ���
			��g_��ͷ����������±��������й��ã��ű���ʼ����ʱ�����Զ���ȡ�ű��ļ��е�GlobalVal�Σ��������г�ʼ�����Զ�����
			�������£�
				ScriptFile.txt���ݣ�
				[GlobalVal]
				g_TitleCount=4;
				g_MyTest=����һ������
			��ô�ű���ʼ��ʱ�ͻ������Щ������
			�ڽű���ֱ�ӶԸ�ֵ��ȡ���ɡ�
			�ڽ�������ͨ������Get/SetGlobalSettingsʵ��ȫ�ֱ�����ȡ��
		2.�û�����
			��l_��ͷ��ͬһ�û�ִ��ʱ�������ã��û������ڽ���������ִ�нű�ʱ��������ͬ�û�����ʹ����ͬ�ı����������ͱ����ʺ���״̬����
			�ڽű���ֱ�ӶԸ�ֵ��ȡ���ɡ�
			�ڽ�������ͨ������Get/SetLocalSettingsʵ��ȫ�ֱ�����ȡ��
		3.ϵͳ����
			��s_��ͷ��ϵͳ�еĳ�ʼ����ı��������ܶ�ϵͳ�������и�ֵ��
			s_LastError��ϵͳ������ĳ����룬ע�⣺s_LastError������ִ����ȷʱ�����¸�ֵ����������Ҫ�ж�ĳһ�����Ƿ�ɹ����ڵ���֮ǰ���ȵ���emptylasterror������s_LastError�������û�������
			s_Rand��Ϊ���ֵ��ÿ�ζ�̬����
			s_TickCount��ʵʱTickCountֵ������ϵͳ�������ĺ��������������ڼ��㳬ʱ
			s_Time32��32λ����ʱ��
			s_EngineVer���ű���������汾��
			s_CurPath���ű��ļ�����Ŀ¼
			s_CurScript���ű��ļ�·��
			s_WinPath��WindowsĿ¼
			s_SysPath��SystemĿ¼
			s_TempPath����ʱĿ¼
			�ڽű���ֱ�ӶԸ�ֵ��ȡ���ɡ�
		4.��ʱ����
			�ɷ����ϱ�����ɣ�ÿ�ε���ʱ��ʹ�ã�����ִ����ɺ��ͷš�
			���ú���ʱ����������ͨ����ʱ������ֵ���ݣ�����������arg1-arg20���
			�ڽű���ֱ�ӶԸ�ֵ��ȡ���ɡ�
			�ڽ�������ͨ������ʱ�Ӳ������롣
	ָ�
		���ָ���޷����ͣ�����Error_Cmd_NotRecognise��
		���ָ���������������Error_Cmd_ParameterNotFound��
		����޷�����ת���'\\'����˼������Error_Cmd_CanNotParseSepSym��
		���������������ɵ�ǰ�ű��󣬲�֪��������������������Error_Function_UnknownHowToContinue��
		���������������ʱ���ִ�������Error_LogicCalc_Error��
		�ⲿָ����ͣ�
			������֧��������ⲿ�ű����ͣ�ʵ���м��ַ�����
				1.�̳���THScript��������OnParseExtCmd������
				2.����һ�ⲿ������THParseExtCmdProc��ͨ��SetParseExtCmdProc���ùҽӡ�
				3.���THScript����Ҫ�õ�ͬ���Ľ��ͺ�������ͨ��SetParseExtCmdClass���ùҽӵ�������н��н��͡�
			ͨ��include���ؽ��еĽű������������Զ��̳��ⲿ�������á�
			����������TRUE��ʾ��ȷ���ͽű�������FALSEΪ�޷���ȷ��������ű�����������Error_Cmd_NotRecognise���󣬶������غ�����ʽ��ʵ�֣����Է���TRUE�������ñ�Ĵ�����롣
		goto[:tag[~errtag]]
			ת��ָ����tag��ֵ���С����ת�����ת��errtag��ֵ���С����ת�������û��errtag����������Error_Function_TagNotFound��
		getini:$savevar$,section,key[,defval[,file]]
			�ӽű��ļ��л�ȡ����
			savevar���������ݺ󱣴浽�ñ���
			section���������ڶ���
			key���������ڼ���
			defval��Ĭ��ֵ
			file����ָ���ӵ�ǰ�ļ���ȡ��ָ����ָ���ļ���ȡ
		setini:section,key,value[,file]
			�ӽű��ļ��б�������
			section���������ڶ���
			key���������ڼ���
			value����ֵ
			file����ָ���ӵ�ǰ�ļ���ȡ��ָ����ָ���ļ���ȡ
		if:val1,=,val2,tag1,tag2
			�߼��жϣ����val1=val2����ת��tag1��������ת��tag2
			va1�����ʽ1��֧����������
			=�����˵�ֵ�Ƚϣ�����>��<��>=��<=��<>
			var2�����ʽ2��֧����������
			������޷������ش���Error_Oper_NotRecognise
		resetglobalsetting
			����(���¶�ȡ)ȫ�ֱ�������������include�����Ľű��ļ���
		emptyglobalsetting
			���ȫ�ֱ�������������include�����Ľű��ļ���
		emptylocalsetting
			��ձ��ر�������������include�����Ľű��ļ���
			ע������ϵͳ����s_LastError���ñ��ر����Ĵ洢������б��棬�����ձ��ر��������LastError��ʹ������ע�⡣
		emptytempsetting
			�����ʱ������
		emptylasterror
			���s_LastErrorֵ��
			ע�⣺����s_LastError������ִ����ȷʱ�����¸�ֵ����������Ҫ�ж�ĳһ�����Ƿ�ɹ����ڵ���֮ǰ���ȵ���emptylasterror��
			ע������ϵͳ����s_LastError���ñ��ر����Ĵ洢������б��棬�����ձ��ر��������LastError��ʹ������ע�⡣
			ע��include�������ļ���lasterror�Ǳ����ڶ�Ӧ�ű��ļ��еġ�
		str:subcmd,$val$[,$val1$[,$val2$]]
			�ַ�����������������subcmdָ�������subcmd���ʹ��󣬷��ش���Error_SubOper_NotRecognise
			subcmd�������£�
				i2s,$val$
					�Բ���ȡ������
				lower,$val$
					�Բ���val����Сдת��
				upper,$val$
					�Բ���val���д�дת��
				replace,$val$,ȫ��,all
					�Ѳ���val�е�'ȫ��'����'all'
				append,$val$,AAA
					�ڲ���val������ַ���'AAA'
				trim,$val$[,\r\t]
					���ַ���������β�հײü����հ׷�ΧĬ��Ϊ�ո�Ҳ���Ժ���Ĳ���ָ��
				set,$val$[,value]
					�����ַ���ֵvalΪvalue��valueΪ��Ϊ��ո�ֵ
				compare,$val$,text,tag1,tag2
					�Ƚϱ���val��text�ַ����Ľ������ͬת��tag1������ת��tag2
				leftcompare,$val$,text,tag1,tag2
					����text�Ƚϴӿ�ʼ��$val$���ȵ��ַ�������ͬת��tag1������ת��tag2
				left,$val$,$str$,cnt
					��ض��ַ���
				mid,$val$,$str$,cnt
					�ҽض��ַ���
				find,$ret$,$val$,AAA,startpos
					��startpos�������ַ��������ַ���AAA���ֵ�λ�ã��Ҳ�������-1
				len,$ret$,$val$
					��ȡ�ַ�������
				formatnum,$ret$,$val$
					������ת����TGMK�ȵ�λ��ֱ����ֵ��
		set:$val$,(1+2)%4,2
			���ʽ��ֵ����(1+2)%4��������ֵ��val�У�֧���������㣬�ַ�����ֵ��ʹ��str:set������2��ʾ����С�����2λ����Ϊ�ա�
		setint:$val$,(1+2)%4
			���ʽ��ֵ����(1+2)%4������4��5��Ϊ��������ֵ��val�У�֧���������㣬�ַ�����ֵ��ʹ��str:set��
		setintu:$val$,(1+2)%4
			���ʽ��ֵ����(1+2)%4������4��5��Ϊ�޷�����������ֵ��val�У�֧���������㣬�ַ�����ֵ��ʹ��str:set��
		function:GenQuestion[,$ret$,0,1,5]
			������ص����ݳ������ش���Error_Function_Return_NotAvailable��
			���ú���GenQuestion������ֵ���浽����ret�У������������ᱣ�浽���ú�������ʱ����arg1-arg20�С�
			��Ҫ���ļ�����ʱ��!������,��: 1!GenQuestion������!֮ǰ����������֧���������㣬����������������ϸ���ܣ���μ�include����֡�
			ע�⣺���ú�������ʱ�����뵱ǰִ�к�����ʱ�����ǻ�������ġ�
		time32:subcmd,$val$[,$val$]
			32λʱ�䴦������������subcmdָ�������subcmd���ʹ��󣬷��ش���Error_SubOper_NotRecognise
			subcmd�������£�
				year,$val$,$s_Time32$
					��ȡ��ǰʱ��s_Time32����ݣ����浽val��
				month,$val$,$s_Time32$
					��ȡ��ǰʱ��s_Time32���·ݣ����浽val��
				day,$val$,$s_Time32$
					��ȡ��ǰʱ��s_Time32�����ڣ����浽val��
				hour,$val$,$s_Time32$
					��ȡ��ǰʱ��s_Time32��Сʱ�������浽val��
				minute,$val$,$s_Time32$
					��ȡ��ǰʱ��s_Time32�ķ����������浽val��
				second,$val$,$s_Time32$
					��ȡ��ǰʱ��s_Time32�����������浽val��
				tickcount,$val$
					��ȡcpuʱ�䣬���浽val��
				maketime,$val$,$year$,$month$,$day$,$hour$,$minute$,$secord$
					����ָ��ʱ�䣬ʱ������ʡ�ԡ�
				maketimestr,$val$,$timestr$
					��ʱ���ַ�������ָ��ʱ�䡣
				maketimestrex,$val$,$timestr$,%Y-%m-%d
					��ʱ���ַ�����ָ����ʽ����ʱ�䡣
				timespan,$val$,$timeval$,$days$,$hour$,$minute$,$secord$
					��$timeval$ʱ��ƫ��$days$,$hour$,$minute$,$secord$������ʱ��$val$��ƫ��ʱ��������ʡ�ԡ�
				timespanstr,$val$,$timeval$,%y-%m-%d,$days$,$hour$,$minute$,$secord$
					��%y-%m-%d��ʽָ����$timeval$ʱ���ַ���ƫ��$days$,$hour$,$minute$,$secord$������ʱ���ַ���$val$��ƫ��ʱ��������ʡ�ԡ�
				format,$val$,$timeval$,%Y-%m-%d
					��%y-%d-%mָ����ʽ��ʱ��$timeval$����ʽ�ַ�������Ϊ�գ����ص�$val$�С�
				localtogmt,$val$,$timeval$
					��$timeval$ָ���ı���ʱ��ת��Ϊ$val$��GMTʱ�䡣
				gmttolocal,$val$,$timeval$
					��$timeval$ָ����GMTʱ��ת��Ϊ$val$�ı���ʱ�䡣
				localtogmtstr,$val$,$timeval$,%y-%m-%d
					��$timeval$ָ���ı���ʱ���ַ���ת��Ϊ$val$��GMTʱ���ַ�������������ַ�����ʽ��%y-%m-%dָ����
				gmttolocalstr,$val$,$timeval$
					��$timeval$ָ����GMTʱ���ַ���ת��Ϊ$val$�ı���ʱ���ַ�������������ַ�����ʽ��%y-%m-%dָ����
		end[:3]
			���к�����ɡ����������Ϊ����ֵ���ص����ú����ߡ�
		sqlite:subcmd[,value1[,value2]]
			Sqlite���ݿ⴦������������subcmdָ�������subcmd���ʹ��󣬷��ش���Error_SubOper_NotRecognise��
			subcmd�������£�
				open,$val$
					�����ݿ⣬����򿪳�������Error_Sqlite_OpenDataBaseError
				openex,$val$,init.sql
					�����ݿ⣬����򿪳�������Error_Sqlite_OpenDataBaseError�������û�����ݿ��ļ����ȵ��ý���Ľű�init.sql�����ݿ�ű��ļ�������ascii����
				sql,insert into testtable(id\qi) values(1\q2)
					ִ��sql��䣬sql����еĶ�����\q�滻�����ִ�г�������Error_Sqlite_ExecSqlError
				getret,$var1$,select id from testable where id=1
					ִ��sql��䲢��ȡ��һ����¼�ĵ�һ����ֵ��sql����еĶ�����\q�滻�����ִ�г�������ֵΪ��
				beginquery,select id from testable
					��ʼ�������ݣ����ִ�г�������Error_Sqlite_BeginQueryError
				ifhavenext,readtag,endtag
					������������ݣ�ת��readtag������ת��endtag
				getval,$var1$,0
					��ȡָ���е����ݣ�ָ��������ɽ�����������
				getvalbykey,$var1$,id
					��ȡָ���е����ݣ�ָ�������ַ�������
				getcolcount,$var1$
					��ȡ��Ŀ����
				getcolname,$var1$,id
					��ȡ��Ŀ����
				endquery
					�����������
				close
					�ر����ݿ�
				export,select * from a,$filename$,\q,\r\n,1
					�������ݽ����ִ��Sql��䣬�����ݱ��浽�ļ��У����ݼ���\q�ָ�����Ŀ����\r\n�ָ���1Ϊ��Ҫ���⣬���ִ�г�������Error_Sqlite_ExportError
				exporttable,$var$,select * from a,1,$lcnt$,$icnt$,ls,le,is,ie,tis,tie,head,end,emptyvalue
					�����ݽ�����ɱ��ִ��Sql��䣬�����ݷ��ص��ַ���var�У�1Ϊ��Ҫ���⣨��ֵ���Ժ�Ĳ������ɺ��ԣ���
					ÿ��ǰ����ÿ��ǰ���󣬱�����ǰ����ֱ�Ϊls,le,is,ie,tis,tie�����ʼ����head���ݣ�������end���ݣ�������Ϊ��ʱ����emptyvalue���棬���ִ�г�������Error_Sqlite_ExportError
		msgbox:$ret$,content,title,type
			�����Ի�������Ϊcontent������Ϊtitle����ʾ��ʽ����ֵtypeָ��������ֵ���浽$ret$�С�
		include:subcmd[,value1[,value2]]
			������֧�ֿ��ļ����ã����ļ�����ʹ�������������ֲ�ͬ�ļ���װ�ؽű��ļ�ʱ����Խű��ļ����г�ʼ����
			���ļ��������÷�����μ�function���
			������õ����������ڣ�����Error_Include_NoExistIndex
			subcmd�������£�
				init,number,Script1.txt
					��Script1.txt���г�ʼ������ӳ�䵽����number��������֧���������㣬���ؽű��ļ��󣬽�������������Զ������ͷţ���ȻҲ�����ֶ������ͷš���������ԭ����ӳ��Ľű�ʱ��ԭ���Ľű����Զ��ͷš�
				free,number
					���Ѽ��ؽű������ͷš�
				getlocal,number,$ret$,l_retstr
					��ȡ�ӽű���local����l_retstr,����l_retstr֧�ֱ�������
				savelocal,number,l_retstr,value
					�����ӽű���local����l_retstrֵΪvalue
				getglobal,number,$ret$,g_tmpstr
					��ȡ�ӽű���ȫ�ֱ���g_tmpstr,����g_tmpstr֧�ֱ�������
				saveglobal,number,g_tmpstr,value
					�����ӽű���ȫ�ֱ���g_tmpstrֵΪvalue
		timer:subcmd[,value1[,value2]]
			��ʱ�����ã����ö�ʱ���󣬾�����ʱʱ����������ö�ʱ��ָ���ĺ������е��ã����в���һΪ��ʱ��id��������Ϊ����ı������ݡ�
			subcmd�������£�
				init,$timerid$,100,RetCall,1,Ret Value
					�½�һ����ʱ������ʱ���ŷ��ص�timerid�У��������0��ʾʧ�ܣ�����Error_InitTimer_Error����100Ϊʱ��������λ���룬֧���������㣻RetCallΪʱ�䵽ʱ���õĽű���������1Ϊ�Ƿ�ѭ����ʱ��֧���������㣻Ret ValueΪ����Ĳ������ڵ��õ�ʱ��Ӧ����ʱ������
				free,$timerid$
					�ͷ�һ����ʱ����timeridΪ��ʱ���ţ�֧���������㡣
				reset,$timerid$,$timeval$
					���ö�ʱ��������������ǰ��ʱʱ�䣬timeridΪ��ʱ���ţ�֧���������㣬���Ϊ0����ʾ���㵽��ʼֵ���¼��������Ϊ��ֵ����ʾ�ӳ����ٶ�ʱʱ�䣻��ֵ��ʾ���ٶ��ٶ�ʱʱ�䣬֧���������㡣
		schedule:subcmd
			�ƻ�������������󣬵�ָ����ʱʱ�䣬��������ָ���Ļص��������е��ã����в���һΪ�ƻ�����id��������Ϊ����ı������ݡ�
			subcmd�������£�
				new,$schid$,2007-01-01 18:00:00,H,60000,1,RetCall,Ret Value
					�½�һ���ƻ���������ŷ��ص�$schid$�У�ʱ��Ϊ������Ӧʱ�䣬hΪ��Сʱ����Ӧʱ��ƥ�䣨����md w HMS����60000Ϊ�����ʱ�䣬ע���ֵ̫��������������û��Ӧ��1Ϊ�Ƿ�ѭ��
					RetCall������Ӧ�ص�������Ret ValueΪ����Ĳ������ڵ��õ�ʱ��Ӧ����ʱ������
				free,$schid$
					ȡ����ʱ��
		file:subcmd
			�ļ���������
			subcmd�������£�
				copyfile,srcfile,dictfile,1
					�����ļ���1Ϊ������ʧ��
				movefile,srcfile,dictfile
					�ƶ��ļ�
				isfileexist,$ret$,filepath
					�ж��ļ��Ƿ����
		syscall:fn,type,ret,arg,arg2,arg3...
			ϵͳAPI���ã�������ó�������Error_SysCall_Failed����
			fnΪ���ú����������ָ��dllʹ��!���磺GDI32.DLL!Bitblt�����ڳ���Api���ɲ�ָ��Dll����ϸ�����THSysCallRapper��ʵ��
			typeΪ���ؽ����ʽ��Ĭ��Ϊ0��
				0Ϊ����Ҫ���ء�
				1Ϊ������������
				2Ϊ���طǸ�����������ָ�롣
				3Ϊ�����ַ�����
			retΪ���ڷ��ر���ı��������typeָ��Ϊ0ʱ���ò���û��
			arg..����APIʱ�Ĳ����б�ע���������Ϊ���֣����ת��Ϊ���ֽ���API����
</pre>*/
class THScript : public THTimerThread ,public THScheduleThread
{
private:
	static void FreeMap(void *key,void *value,void *adddata)
	{
		delete (THString *)value;
	}
	static void FreeIncMap(void *key,void *value,void *adddata)
	{
		delete (THScript *)value;
	}
	static void FreeTimerMap(void *key,void *value,void *adddata)
	{
		int TimerId=(int)(INT_PTR)key;
		THTimerMap *p=(THTimerMap *)value;
		if (p && adddata)
		{
			((THScript *)adddata)->StopTimer(TimerId);
			delete p;
		}
	}
public:
	static const int THScriptEngineVersion=180;	///>�������汾
	enum enErrCode{
		Error_None,								///>û�д���
		Error_Cmd_NotRecognise,					///>�޷�ʶ�������
		Error_Function_Return_NotAvailable,		///>�������ؽ���޷�����
		Error_Cmd_ParameterNotFound,			///>��������
		Error_Cmd_CanNotParseSepSym,			///>���ܴ��������ַ�'\\'
		Error_Function_UnknownHowToContinue,	///>û�м��������ָ���������֪����������
		Error_Function_TagNotFound,				///>û���ҵ���Ӧ�ı��
		Error_LogicCalc_Error,					///>�����������
		Error_Oper_NotRecognise,				///>�������޷�����
		Error_SubOper_NotRecognise,				///>������Ӳ����޷�����
		Error_Sqlite_ExecSqlError,				///>Sqliteִ��Sql������
		Error_Sqlite_OpenDataBaseError,			///>Sqliteִ�д����ݿ����
		Error_Sqlite_BeginQueryError,			///>Sqliteִ�������ѯ����
		Error_Sqlite_ExportError,				///>Sqlite�������ݴ���
		Error_Include_NoExistIndex,				///>������һ����Ч���ⲿ�ű�����
		Error_InitTimer_Error,					///>��ʱ����������
		Error_SysCall_Failed					///>ϵͳ���ô���
	};
	THScript()
	{
		m_cb=NULL;
		m_adddata=NULL;
		m_extcls=NULL;
		m_local.SetFreeProc(FreeMap);
		m_include.SetFreeProc(FreeIncMap);
		m_TimerMap.SetFreeProc(FreeTimerMap,this);
		//Ĭ�ϲ�����ͨ�ļ�ϵͳ
		m_pStore=&m_Store;
		m_ini.InitStore(m_pStore,FALSE);
		m_pDebugger=NULL;
	}
	THScript(THString sPath)
	{
		m_cb=NULL;
		m_adddata=NULL;
		m_extcls=NULL;
		m_local.SetFreeProc(FreeMap);
		m_include.SetFreeProc(FreeIncMap);
		m_TimerMap.SetFreeProc(FreeTimerMap,this);
		Init(sPath);
	}
	virtual ~THScript()
	{
		FreeScript();
	}

	void FreeScript()
	{
		if (m_ini.IsInited())
		{
			RunScript(_T("FreeCall"));
			m_ini.Init();
		}
		m_include.RemoveAll();
		m_local.RemoveAll();
		m_TimerMap.RemoveAll();
	}
	void SetDebugger(ITHScriptDebugger *debugger){m_pDebugger=debugger;}
	static THString GetErrorText(enErrCode ErrCode)
	{
		switch(ErrCode)
		{
			case Error_None:return _T("û�д���");
			case Error_Cmd_NotRecognise:return _T("�޷�ʶ�������");
			case Error_Function_Return_NotAvailable:return _T("�������ؽ���޷�����");
			case Error_Cmd_ParameterNotFound:return _T("��������");
			case Error_Cmd_CanNotParseSepSym:return _T("���ܴ��������ַ�'\\'");
			case Error_Function_UnknownHowToContinue:return _T("û�м��������ָ���������֪����������");
			case Error_Function_TagNotFound:return _T("û���ҵ���Ӧ�ı��");
			case Error_LogicCalc_Error:return _T("�����������");
			case Error_Oper_NotRecognise:return _T("�������޷�����");
			case Error_SubOper_NotRecognise:return _T("������Ӳ����޷�����");
			case Error_Sqlite_ExecSqlError:return _T("Sqliteִ��Sql������");
			case Error_Sqlite_OpenDataBaseError:return _T("Sqliteִ�д����ݿ����");
			case Error_Sqlite_BeginQueryError:return _T("Sqliteִ�������ѯ����");
			case Error_Sqlite_ExportError:return _T("Sqlite�������ݴ���");
			case Error_Include_NoExistIndex:return _T("������һ����Ч���ⲿ�ű�����");
			case Error_InitTimer_Error:return _T("��ʱ����������");
			case Error_SysCall_Failed:return _T("ϵͳ���ô���");
		}
		return _T("δ֪����");
	}
//localsetting parse
#define OnScriptError(a,b) {SetLocalSettings(localidx,_T("s_LastError"),THStringConv::i2s(a));OnErrorHandler(a,b);}

	virtual OnErrorHandler(enErrCode ErrCode,THString ErrText=_T(""))
	{
		//fixme,parse script error handle
		if (m_pDebugger==NULL || m_pDebugger->OnScriptErrorHandler((int)ErrCode,ErrText)==FALSE)
		{
#ifdef THSCRIPT_DEBUG
			THDebug(_T("THScript:OnErrorHandler:ErrCode:%d,%s,ErrText:%s"),ErrCode,GetErrorText(ErrCode),ErrText);
#endif
			ASSERT(FALSE);
		}
	}

	/**
	* @brief ��ʼ��·��
	* @param sPath		�ű��ļ�·��
	* @param pStore		ָ��ʹ�õ��ļ�ϵͳ
	* @param bAutoFree	�Ƿ��Զ��ͷ��ļ�ϵͳָ��
	*/
	void Init(THString sPath,ITHFileStore *pStore=NULL,BOOL bAutoFree=FALSE)
	{
		FreeScript();
		if (pStore)
			m_pStore=pStore;
		else
		{
			m_Store.SetBaseDirectory(THSysMisc::GetFilePath(sPath));
			m_pStore=&m_Store;
			bAutoFree=FALSE;
		}
#ifdef THSCRIPT_DEBUG
		THDebug(_T("THScript:�ű���ʼ��,�ļ�:%s,�ļ�ϵͳ:%s"),sPath,m_pStore->GetStoreName());
#endif
		m_ini.InitStore(m_pStore,bAutoFree,sPath);
		THStringArray ar,arval;
		if (m_ini.EnumStruct(_T("GlobalVal"),&ar,&arval))
		{
			//init the global val
			for(int i=0;i<ar.GetSize();i++)
				SetGlobalSettings(ar[i],ParseVar(arval[i],NULL,NULL));//ʹ��ParseVar����ת���
		}
		RunScript(_T("InitCall"));
	}

	/**
	* @brief ���ؽű��ļ���·��
	* @return �ű��ļ�·��
	*/
	THString GetScriptFilePath(){return m_ini.GetIniPath();}

	/**
	* @brief ���нű�
	* @param fnName			�ű��������ƣ�Ϊ����Ϊ���ú���ScriptMain
	* @param localidx		�ṩ���û���������
	* @param tempsettings	���ڴ��ݺ���������������Ϊarg1-arg20
	* @return �������н��
	*/
	THString RunScript(THString fnName,int localidx=0,THString tempsettings=_T(""))
	{
#ifdef THSCRIPT_DEBUG
		THDebug(_T("THScript:���к���:%s,��������:%s"),fnName,tempsettings);
#endif
		//setup local settings
		m_localsettinglock.Lock();
		THString *localsettings=GetLocalSettings(localidx);
#ifdef THSCRIPT_DEBUG
		THDebug(_T("THScript:�û���������:%d,��������:%s"),localidx,(localsettings)?(*localsettings):_T(""));
#endif
		m_localsettinglock.Unlock();
		THString ret=RunFunction(fnName,localidx,tempsettings,0);
#ifdef THSCRIPT_DEBUG
		THDebug(_T("THScript:����:%s ���ؽ��:%s"),fnName,ret);
#endif
		return ret;
	}

	void EmptyLocalSettings(int localidx)
	{
#ifdef THSCRIPT_DEBUG
		THDebug(_T("THScript:����û�����,����:%d"),localidx);
#endif
/*		THString *tmp;
		THString ret;
		if (m_local.GetAt(localidx,tmp))
			tmp->Empty();*/
		//localsetting parse
		m_localsettinglock.Lock();
		m_local.RemoveAt(localidx);
		m_localsettinglock.Unlock();
		THPosition pos=m_include.GetStartPosition();
		int idx;
		THScript *script;
		while(!pos.IsEmpty())
		{
			if (m_include.GetNextPosition(pos,idx,script))
			{
				if (script) script->EmptyLocalSettings(localidx);
			}
		}
	}

	THString GetLocalSettings(int localidx,THString key)
	{
		//localsetting parse
		THSingleLock lock(&m_localsettinglock);
		THString *tmp;
		THString ret;
		BOOL bpret=m_local.GetAt(localidx,tmp);
		if (bpret && tmp)
		{
			if (key.IsEmpty())
				ret=*tmp;
			else
				ret=THSimpleXml::GetParam(*tmp,key);
		}
#ifdef THSCRIPT_DEBUG
		THDebug(_T("THScript:��ȡ�û�����,����:%d,����:%s,��ֵ:%s"),localidx,key,ret);
#endif
		return ret;
	}

	void SetLocalSettings(int localidx,THString key,THString value)
	{
		//localsetting parse
		if (key.Left(2)!=_T("l_") && key!=_T("s_LastError")) return;
#ifdef THSCRIPT_DEBUG
		THDebug(_T("THScript:�����û�����,����:%d,����:%s,��ֵ:%s"),localidx,key,value);
#endif
		THSingleLock lock(&m_localsettinglock);
		THString *tmp;
		if (!m_local.GetAt(localidx,tmp) || tmp==NULL)
		{
			tmp=new THString;
			if (!tmp) return;
			m_local.SetAt(localidx,tmp);
		}
		THSimpleXml::SetParam(tmp,key,value);
	}

	void EmptyGlobalSettings()
	{
#ifdef THSCRIPT_DEBUG
		THDebug(_T("THScript:���ȫ�ֻ���"));
#endif
		m_global.Empty();
		THPosition pos=m_include.GetStartPosition();
		int idx;
		THScript *script;
		while(!pos.IsEmpty())
		{
			if (m_include.GetNextPosition(pos,idx,script))
			{
				if (script) script->EmptyGlobalSettings();
			}
		}
	}

	THString GetGlobalSettings(THString key)
	{
		THString ret;
		if (key.IsEmpty())
			ret=m_global;
		else
			ret=THSimpleXml::GetParam(m_global,key);
#ifdef THSCRIPT_DEBUG
		THDebug(_T("THScript:��ȡȫ�ֻ���,����:%s,��ֵ:%s"),key,ret);
#endif
		return ret;
	}

	void SetGlobalSettings(THString key,THString value)
	{
		if (key.Left(2)!=_T("g_")) return;
#ifdef THSCRIPT_DEBUG
		THDebug(_T("THScript:  ����ȫ�ֱ���,����:%s,��ֵ:%s"),key,value);
#endif
		THSimpleXml::SetParam(&m_global,key,value);
	}

	void ResetGlobalSettings()
	{
#ifdef THSCRIPT_DEBUG
		THDebug(_T("THScript:�ض�ȫ�ֻ���"));
#endif
		EmptyGlobalSettings();
		THStringArray ar,arval;
		if (m_ini.EnumStruct(_T("GlobalVal"),&ar,&arval))
		{
			//init the global val
			for(int i=0;i<ar.GetSize();i++)
				SetGlobalSettings(ar[i],ParseVar(arval[i],NULL,NULL));//ʹ��ParseVar����ת���
		}
		THPosition pos=m_include.GetStartPosition();
		int idx;
		THScript *script;
		while(!pos.IsEmpty())
		{
			if (m_include.GetNextPosition(pos,idx,script))
			{
				if (script) script->ResetGlobalSettings();
			}
		}
	}

	/**
	* @brief ��ʼ��ʱ��
	* @param localidx		�ṩ���û���������
	* @param timeval		��ʱ����ʱ���
	* @param fnName			�ű���������
	* @param retval			���ص��õĺ���
	* @param bLoop			�Ƿ�ѭ������
	* @return ���ض�ʱ����,����0��ʾʧ��
	*/
	int SetTimer(int localidx,int timeval,THString fnName,THString retval,BOOL bLoop)
	{
		THTimerMap *map=new THTimerMap;
		if (!map) return 0;
		int TimerId=StartTimer(0,timeval,bLoop,NULL);
		if (TimerId==0)
		{
			delete map;
			return 0;
		}
		map->localidx=localidx;
		map->fnName=fnName;
		map->retval=retval;
		map->bLoop=bLoop;
		m_TimerMap.SetAt(TimerId,map);
#ifdef THSCRIPT_DEBUG
		THDebug(_T("THScript:���ö�ʱ��,�û���������:%d,��ʱ�����:%d,�ص�����:%s,�ص�����:%s,�Ƿ��ظ�:%d"),localidx,timeval,fnName,retval,bLoop);
#endif
		return TimerId;
	}

	/**
	* @brief ȡ����ʱ��
	* @param timerid		��ʱ����
	*/
	void UnSetTimer(int timerid)
	{
#ifdef THSCRIPT_DEBUG
		THDebug(_T("THScript:ȡ����ʱ��:%d"),timerid);
#endif
		THTimerMap *map;
		BOOL bpret=m_TimerMap.GetAt(timerid,map);
		if (bpret && map)
		{
			//StopTimer in FreeProc
			m_TimerMap.RemoveAt(timerid);
		}
	}
	//void ResetTimer(int timerid,int timeval);

	/**
	* @brief ����ʹ���ⲿ���������ⲿ����
	* @param cb				������
	* @param adddata		���Ӳ���
	*/
	void SetParseExtCmdProc(THParseExtCmdProc cb,const void *adddata){m_cb=cb;m_adddata=adddata;}

	/**
	* @brief ����ʹ���ⲿ�ദ���ⲿ����
	* @param cls			�ⲿ��ָ��
	*/
	void SetParseExtCmdClass(THScript *cls){m_extcls=cls;}

	/**
	* @brief ���洦��ı���
	* @param var			�����ֵ��
	* @param value			��������
	* @param tempsetting	��ʱ����ָ��
	* @param localidx		���ر�������
	*/
	void SaveVar(THString var,THString value,THString *tempsetting,int localidx)
	{
		if (var.GetAt(0)!='$' && var.GetAt(var.GetLength()-1)!='$') return;
#ifdef THSCRIPT_DEBUG
		THString myvar=var;
#endif
		var=var.Left(var.GetLength()-1).Mid(1);
		while(1)
		{
			int pos=var.Find(_T("["),0);
			int pos2=-1;
			if (pos!=-1)
				pos2=var.Find(_T("]"),pos);
			if (pos!=-1 && pos2!=-1)
			{
				//˵����Ҫ����Ƕ�ױ���
				THString subvarname=var.Left(pos2).Mid(pos+1);
				THString change=GetVar(subvarname,tempsetting,localidx);
				var.Replace(_T("[")+subvarname+_T("]"),change);
				//value=GetVar(var,tempsetting,localidx);
			}
			else
				break;
		}
#ifdef THSCRIPT_DEBUG
		THDebug(_T("THScript:  ������� %s[%s]=%s"),myvar,var,value);
#endif
		THString type=var.Left(2);
		THString tmpsave;
		THString *save=NULL;
		if (type==_T("g_"))
			save=&m_global;
		else if (type==_T("l_"))
		{
			m_localsettinglock.Lock();
			save=GetLocalSettings(localidx);
			if (save==NULL)
				save=&tmpsave;
		}
		else
			save=tempsetting;
		THSimpleXml::SetParam(save,var,value);
		if (type==_T("l_"))
			m_localsettinglock.Unlock();
	}

	/**
	* @brief �������
	* @param var			��������
	* @param tempsetting	��ʱ����ָ��
	* @param localidx		���ر�������
	* @return ���ݴ�����
	*/
	THString ParseVar(THString var,THString *tempsetting,int localidx)
	{
#ifdef THSCRIPT_DEBUG
		THString myvar=var;
#endif
		int pos,pos2;
		THString varname,varvalue;
		var=ParseSepSym(var);
		while(1)
		{
			pos=var.Find(_T("$"),0);
			//�ų���double$$���֣�$$��ʾ����ʵ$��
			while(pos!=-1 && var.GetLength()>pos+1 && var.GetAt(pos+1)=='$')
			{
				pos=var.Find(_T("$"),pos+2);
			}
			if (pos!=-1)
			{
				pos2=var.Find(_T("$"),pos+1);
				//�ų���double$$���֣�$$��ʾ����ʵ$��
				//pos2��Ӧ���жϣ���Ϊ��$var$$var2$�������
				/*while(pos2!=-1 && var.GetLength()>pos2+1 && var.GetAt(pos2+1)=='$')
				{
					pos2=var.Find(_T("$"),pos2+2);
				}*/
			}
			else
				pos2=-1;
			if (pos==-1 || pos2==-1) break;
			varname=var.Left(pos2).Mid(pos+1);
			BOOL parsemarco=FALSE;
			THString tmpvarname=varname;
			while(1)
			{
				pos=tmpvarname.Find(_T("["),0);
				if (pos!=-1)
					pos2=tmpvarname.Find(_T("]"),pos);
				else
					pos2=-1;
				if (pos!=-1 && pos2!=-1)
				{
					//˵����Ҫ����Ƕ�ױ���
					THString subvarname=tmpvarname.Left(pos2).Mid(pos+1);
					THString change=GetVar(subvarname,tempsetting,localidx);
					tmpvarname.Replace(_T("[")+subvarname+_T("]"),change);
					parsemarco=TRUE;
				}
				else
					break;
			}
			if (parsemarco)
				varvalue=GetVar(tmpvarname,tempsetting,localidx);
			else
				varvalue=GetVar(varname,tempsetting,localidx);
			var.Replace(_T("$")+varname+_T("$"),varvalue);
		}
#ifdef THSCRIPT_DEBUG
		if(myvar!=var) THDebug(_T("THScript:  ��ȡ���� %s=%s"),myvar,var);
#endif
		return var;
	}
protected:
	THString *GetLocalSettings(int localidx)
	{
		THString *str=NULL;
		if (!m_local.GetAt(localidx,str) || str==NULL)
		{
			str=new THString;
			if (str)
				m_local.SetAt(localidx,str);
		}
		return str;
	}

	/**
	* @brief �ڲ����нű�
	* @param fnName			�ű���������
	* @param localidx		�ṩ���û���������
	* @param tempsetting	��ʱ����,����ں����ڲ�ʹ��,�������ֵ���ݷ�ʽ��ֵ,���ڴ�����ú����Ĳ���
	* @return �������н��
	*/
	THString RunFunction(THString fnName,int localidx,THString tempsetting,int nRecursion)
	{
		if (fnName.IsEmpty())
			fnName=_T("ScriptMain");
		if (m_pDebugger && m_pDebugger->OnEnterFunction(fnName,nRecursion,localidx,&tempsetting)==FALSE) return _T("");
		THString theret;
		THString script=m_ini.GetString(fnName,_T("start"));
		if (m_pDebugger) m_pDebugger->OnEnterTag(fnName,_T("start"),&script);
#ifdef THSCRIPT_DEBUG
		THDebug(_T("THScript:���нű�:[%s] start=%s"),fnName,script);
#endif
		THString next=ParseScript(script,localidx,&tempsetting,nRecursion);
		while(!next.IsEmpty())
		{
			if (next.Left(3)==_T("tag"))
			{
				THString tag=next.Mid(4);
				THString errtag;
				int pos=tag.Find(_T("~"),0);
				if (pos!=-1)
				{
					errtag=tag.Mid(pos+1);
					tag=tag.Left(pos);
				}
				script=m_ini.GetString(fnName,tag);
				if (script.IsEmpty())
				{
					script=m_ini.GetString(fnName,errtag);
					if (script.IsEmpty())
					{
						OnScriptError(Error_Function_TagNotFound,script);
						break;
					}
					else
					{
						if (m_pDebugger) m_pDebugger->OnEnterTag(fnName,errtag,&script);
#ifdef THSCRIPT_DEBUG
						THDebug(_T("THScript:��������ת��:[%s] %s=%s"),fnName,errtag,script);
#endif
					}
				}
				else
				{
					if (m_pDebugger) m_pDebugger->OnEnterTag(fnName,tag,&script);
#ifdef THSCRIPT_DEBUG
					THDebug(_T("THScript:��ת��:[%s] %s=%s"),fnName,tag,script);
#endif
				}
				next=ParseScript(script,localidx,&tempsetting,nRecursion);
			}
			else if (next.Left(3)==_T("ret"))
			{
				theret=next.Mid(4);
				break;
			}
			else
			{
				OnScriptError(Error_Function_Return_NotAvailable,next);
				break;
			}
		}
		if (next.IsEmpty())
			OnScriptError(Error_Function_UnknownHowToContinue,script);
		if (m_pDebugger) m_pDebugger->OnLeaveFunction(fnName,nRecursion,&theret,localidx);
		return theret;
	}
	/**
	* @brief �ڲ�����ű�
	* @param script			��ǰҪ���еĽű�
	* @param localidx		�ṩ���û���������
	* @param tempsetting	��ʱ����
	* @param
	* @return �������н��������tag:tagName,��ʾ��λ��tag,����ret:retValue,��ʾ�������صĽ��
	*/
	THString ParseScript(THString script,int localidx,THString *tempsetting,int nRecursion)
	{
		if (script.IsEmpty()) return _T("ret:");
		THStringToken token;
		token.Init(script,_T(";"));
		while(token.IsMoreTokens())
		{
			THString script=token.GetNextToken();
			if (m_pDebugger && m_pDebugger->OnRunScript(script,localidx,tempsetting)==FALSE) continue;
			THString cmd;
			THStringToken tokenvar;
			int pos=script.Find(_T(":"),0);
			if (pos==-1)
			{
				cmd=script;
#ifdef THSCRIPT_DEBUG
				THDebug(_T("THScript:ִ�нű�:%s"),cmd);
#endif
			}
			else
			{
				cmd=script.Left(pos);
				THString tmp=script.Mid(pos+1);
				tokenvar.Init(tmp,_T(","));
#ifdef THSCRIPT_DEBUG
				THDebug(_T("THScript:ִ�нű�:%s,����:%s"),cmd,tmp);
#endif
			}
#define ISCMD(a) cmd.Compare(_T(#a))==0
			if (ISCMD(goto))
			{
				//goto:tag//goto:tag~err//ת��tag�����tag�����ڣ�ת��err
				if (!tokenvar.IsMoreTokens())
					OnScriptError(Error_Cmd_ParameterNotFound,cmd)
				else
				{
					THString ret=_T("tag:")+ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
					//continueָ���ʾ��˳�����ִ����һ��
					if (ret!=_T("tag:_continue"))
					//if (tokenvar.IsMoreTokens())
					//	ret+=_T(",")+tokenvar.GetNextToken();
						return ret;
				}
			}
			else if (ISCMD(getini))
			{
				//getini:saveto,section,key
				if (tokenvar.GetRemainTokensCount()<3)
					OnScriptError(Error_Cmd_ParameterNotFound,cmd)
				else
				{
					THString savevar=tokenvar.GetNextToken();
					THString tmpvar=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
					THString tmpvar2=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
					THString defval=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
					THString file=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
					THString ret;
					if (file.IsEmpty())
						ret=m_ini.GetString(tmpvar,tmpvar2,defval);
					else
					{
						ITHIni ini;
						if (ini.InitStore(m_pStore,FALSE,file))
							ret=ini.GetString(tmpvar,tmpvar2,defval);
					}
#ifdef THSCRIPT_DEBUG
					THDebug(_T("THScript:��ȡ�����ļ�[%s] %s=%s,Ĭ��ֵ:%s,�ļ�:%s"),tmpvar,tmpvar2,ret,defval,file);
#endif
					SaveVar(savevar,ret,tempsetting,localidx);
				}
			}
			else if (ISCMD(setini))
			{
				//setini:userlog,s$usernum$,$v5$
				if (tokenvar.GetRemainTokensCount()<3)
					OnScriptError(Error_Cmd_ParameterNotFound,cmd)
				else
				{
					THString var1=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
					THString var2=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
					THString var3=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
					THString file=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
#ifdef THSCRIPT_DEBUG
					THDebug(_T("THScript:д�������ļ�[%s] %s=%s,�ļ�:%s"),var1,var2,var3,file);
#endif
					if (file.IsEmpty())
                        m_ini.WriteString(var1,var2,var3);
					else
					{
						ITHIni ini;
						ini.InitStore(m_pStore,FALSE,file);
						ini.WriteString(var1,var2,var3);
					}
				}
			}
			else if (ISCMD(if))
			{
				//isequ:$v7$+1,<=,$v8$,tagstart,tagtimelimit
				if (tokenvar.GetRemainTokensCount()<5)
					OnScriptError(Error_Cmd_ParameterNotFound,cmd)
				else
				{
					THString var=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
					THString sym=tokenvar.GetNextToken();
					THString var2=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
					double ret=0.0;
					if (!m_calc.Run(var,&ret))
						OnScriptError(Error_LogicCalc_Error,cmd)
					else
					{
						double ret2=0.0;
						if (!m_calc.Run(var2,&ret2))
							OnScriptError(Error_LogicCalc_Error,cmd)
						else
						{
							BOOL bRet=FALSE;
							if (sym==_T(">"))
							{
								if (ret>ret2) bRet=TRUE;
							}
							else if (sym==_T("<"))
							{
								if (ret<ret2) bRet=TRUE;
							}
							else if (sym==_T("<=") || sym==_T("=<"))
							{
								if (ret<=ret2) bRet=TRUE;
							}
							else if (sym==_T(">=") || sym==_T("=>"))
							{
								if (ret>=ret2) bRet=TRUE;
							}
							else if (sym==_T("<>") || sym==_T("!="))
							{
								if (ret!=ret2) bRet=TRUE;
							}
							else if (sym==_T("=") || sym==_T("=="))
							{
								if (ret==ret2) bRet=TRUE;
							}
							else
								OnScriptError(Error_Oper_NotRecognise,cmd)
							THString tagret=tokenvar.GetNextToken();
							if (bRet==FALSE)
								tagret=tokenvar.GetNextToken();
#ifdef THSCRIPT_DEBUG
							THDebug(_T("THScript:��ת�ж�(%s=%.3f) %s (%s=%.3f) = %s,��ת:%s"),var,ret,sym,var2,ret2,bRet?_T("��"):_T("��"),tagret);
#endif
							tagret=ParseVar(tagret,tempsetting,localidx);
							//continueָ���ʾ��˳�����ִ����һ��
							if (tagret!=_T("_continue"))
								return (_T("tag:")+tagret);
						}
					}
				}
			}
			else if (ISCMD(resetglobalsetting))
			{
				ResetGlobalSettings();
			}
			else if (ISCMD(emptyglobalsetting))
			{
				EmptyGlobalSettings();
			}
			else if (ISCMD(emptylocalsetting))
			{
				EmptyLocalSettings(localidx);
			}
			else if (ISCMD(emptytempsetting))
			{
				tempsetting->Empty();
			}
			else if (ISCMD(str))
			{
				//str:lower,$val$ //str:replace,$val$,2,3 //str:upper,$val$ //str:append,$val$,$s1$
				if (tokenvar.GetRemainTokensCount()<2)
					OnScriptError(Error_Cmd_ParameterNotFound,cmd)
				else
				{
					THString oper=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
					THString val=tokenvar.GetNextToken();
					THString valvalue=ParseVar(val,tempsetting,localidx);
					if (oper==_T("i2s"))
					{
#ifdef THSCRIPT_DEBUG
						THString orgval=valvalue;
#endif
						valvalue=THStringConv::i2s(THSysMisc::cuttail(_tstof(valvalue)));
#ifdef THSCRIPT_DEBUG
						THDebug(_T("THScript:�ַ���i2s %s->%s"),orgval,valvalue);
#endif
					}
					else if (oper==_T("lower"))
					{
						valvalue.MakeLower();
#ifdef THSCRIPT_DEBUG
						THDebug(_T("THScript:תСд->%s"),valvalue);
#endif
					}
					else if (oper==_T("upper"))
					{
						valvalue.MakeUpper();
#ifdef THSCRIPT_DEBUG
						THDebug(_T("THScript:ת��д->%s"),valvalue);
#endif
					}
					else if (oper==_T("replace"))
					{
						THString tmpvar=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
						THString orgrep=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
#ifdef THSCRIPT_DEBUG
						THString orgval=valvalue;
#endif
						valvalue.Replace(tmpvar,orgrep);
#ifdef THSCRIPT_DEBUG
						THDebug(_T("THScript:�ַ����滻 �� %s �е� %s �滻Ϊ %s ���:%s"),orgval,tmpvar,orgrep,valvalue);
#endif
					}
					else if (oper==_T("append"))
					{
#ifdef THSCRIPT_DEBUG
						THString orgval=valvalue;
#endif
						valvalue+=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
#ifdef THSCRIPT_DEBUG
						THDebug(_T("THScript:�ַ���׷��%s->%s"),orgval,valvalue);
#endif
					}
					else if (oper==_T("trim"))
					{
#ifdef THSCRIPT_DEBUG
						THString orgval=valvalue;
#endif
						THString trimvar=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
						if (trimvar.IsEmpty())
							valvalue.Trim();
						else
							valvalue.Trim(trimvar);
#ifdef THSCRIPT_DEBUG
						THDebug(_T("THScript:�ַ�������%s->%s,�����ַ�%s"),orgval,valvalue,trimvar);
#endif
					}
					else if (oper==_T("set"))
						valvalue=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
					else if (oper==_T("compare") || oper==_T("leftcompare"))
					{
						BOOL bSame=FALSE;
						if (oper==_T("leftcompare"))
						{
							if (valvalue.Compare(ParseVar(tokenvar.GetNextToken(),tempsetting,localidx).Left(valvalue.GetLength()))==0)
								bSame=TRUE;
						}
						else
						{
							if (valvalue==ParseVar(tokenvar.GetNextToken(),tempsetting,localidx))
								bSame=TRUE;
						}
#ifdef THSCRIPT_DEBUG
						THDebug(_T("THScript:�ַ���%s�Ƚ�,���:%s"),oper,bSame?_T("��ͬ"):_T("����ͬ"));
#endif
						THString ret=tokenvar.GetNextToken();
						if (!bSame)
							ret=tokenvar.GetNextToken();
						if (ret.IsEmpty())
							OnScriptError(Error_Cmd_ParameterNotFound,cmd)
						if (ret!=_T("_continue"))
							return _T("tag:")+ParseVar(ret,tempsetting,localidx);
					}
					else if (oper==_T("left"))
					{
						THString tmp=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
						THString tmpcalc=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
						double ret2=0.0;
						if (!m_calc.Run(tmpcalc,&ret2))
							OnScriptError(Error_LogicCalc_Error,cmd)
						else
						{
							valvalue=tmp.Left(THSysMisc::cuttail(ret2));
#ifdef THSCRIPT_DEBUG
							THDebug(_T("THScript:�ַ���ȡ��(%s = %.3f)���ַ�:%s->%s"),tmpcalc,ret2,tmp,valvalue);
#endif
						}
					}
					else if (oper==_T("mid"))
					{
						THString tmp=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
						THString tmpcalc=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
						double ret2=0.0;
						if (!m_calc.Run(tmpcalc,&ret2))
							OnScriptError(Error_LogicCalc_Error,cmd)
						else
						{
							valvalue=tmp.Mid(THSysMisc::cuttail(ret2));
#ifdef THSCRIPT_DEBUG
							THDebug(_T("THScript:�ַ���ȡ��(%s = %.3f)���ַ�:%s->%s"),tmpcalc,ret2,tmp,valvalue);
#endif
						}
					}
					else if (oper==_T("find"))
					{
						THString tmp=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
						THString tmpstr=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
						THString tmpcalc=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
						double ret2=0.0;
						if (!m_calc.Run(tmpcalc,&ret2))
							OnScriptError(Error_LogicCalc_Error,cmd)
						else
						{
							valvalue=THStringConv::i2s(tmp.Find(tmpstr,THSysMisc::cuttail(ret2)));
#ifdef THSCRIPT_DEBUG
							THDebug(_T("THScript:�ַ�������,���ַ��� %s �в��� %s,��λ�� (%s = %.3f) ��ʼ����,�ҵ���λ��:%s"),tmp,tmpstr,tmpcalc,ret2,valvalue);
#endif
						}
					}
					else if (oper==_T("len"))
					{
						THString tmpstr=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
						valvalue=THStringConv::i2s(tmpstr.GetLength());
#ifdef THSCRIPT_DEBUG
						THDebug(_T("THScript:�ַ������ȼ���,�ַ��� %s,����:%s"),tmpstr,valvalue);
#endif
					}
					else if (oper==_T("formatnum"))
					{
						THString tmpstr=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
						tmpstr.Trim();
						int len=tmpstr.GetLength();
						if (len<4)
							valvalue=tmpstr;
						else if (len<7)
							valvalue=tmpstr.Left(len-3)+_T("K");
						else if (len<10)
							valvalue=tmpstr.Left(len-6)+_T("M");
						else if (len<13)
							valvalue=tmpstr.Left(len-9)+_T("G");
						else
							valvalue=tmpstr.Left(len-12)+_T("T");
#ifdef THSCRIPT_DEBUG
						THDebug(_T("THScript:�ַ�����������,�ַ��� %s,����:%s"),tmpstr,valvalue);
#endif
					}
					else
						OnScriptError(Error_SubOper_NotRecognise,cmd)
					SaveVar(val,valvalue,tempsetting,localidx);
				}
			}
			else if (ISCMD(set) || ISCMD(setint) || ISCMD(setintu))
			{
				//set:$next$,123 //set:$next$
				if (tokenvar.GetRemainTokensCount()<1)
					OnScriptError(Error_Cmd_ParameterNotFound,cmd)
				else
				{
					THString var=tokenvar.GetNextToken();
					THString calc=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
					THString limitcount=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
					double ret=0.0;
					if (!m_calc.Run(calc,&ret))
						OnScriptError(Error_LogicCalc_Error,cmd)
					else
					{
						if (ISCMD(setint))
						{
							SaveVar(var,THStringConv::i2s(THSysMisc::cuttail(ret)),tempsetting,localidx);
#ifdef THSCRIPT_DEBUG
							THDebug(_T("THScript:�������� %d -> %s"),THSysMisc::cuttail(ret),var);
#endif
						}
						else if (ISCMD(setintu))
						{
							SaveVar(var,THStringConv::u2s(THSysMisc::cuttail(ret)),tempsetting,localidx);
#ifdef THSCRIPT_DEBUG
							THDebug(_T("THScript:�����޷������� %u -> %s"),THSysMisc::cuttail(ret),var);
#endif
						}
						else
						{
							THString retval;
							if (!limitcount.IsEmpty())
							{
								retval=THStringConv::f2sc(ret,THs2i(limitcount));
							}
							else
								retval=THStringConv::f2s(ret);
							SaveVar(var,retval,tempsetting,localidx);
#ifdef THSCRIPT_DEBUG
							THDebug(_T("THScript:������ֵ %.3f -> %s"),ret,var);
#endif
						}
					}
				}
			}
			else if (ISCMD(function))
			{
				//function:genquestion,$l_qno1$,0,0,0
				if (tokenvar.GetRemainTokensCount()<1)
					OnScriptError(Error_Cmd_ParameterNotFound,cmd)
				else
				{
					THString fnName=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
					//parse fnName
					THScript *inc=NULL;
					int idx=0;
					if (fnName.Find(_T("!"),0)!=-1)
					{
						int pos=fnName.Find(_T("!"),0);
						THString calc=ParseVar(fnName.Left(pos),tempsetting,localidx);
						fnName=fnName.Mid(pos+1);
						double ret=0.0;
						if (!m_calc.Run(calc,&ret))
							OnScriptError(Error_LogicCalc_Error,cmd)
						else
						{
							idx=THSysMisc::cuttail(ret);
							if (!m_include.GetAt(idx,inc))
								OnScriptError(Error_Include_NoExistIndex,cmd)
						}
					}
					//ret value setting
					THString retval=tokenvar.GetNextToken();
					//pass the args
					THString tmp;
					THString fntempsetting;
					int index=1;
					while(tokenvar.IsMoreTokens() && index<=20)
					{
						tmp.Format(_T("arg%d"),index++);
						THSimpleXml::SetParam(&fntempsetting,tmp,ParseVar(tokenvar.GetNextToken(),tempsetting,localidx));
					}
					if (inc)
					{
						tmp=inc->RunScript(fnName,localidx,fntempsetting);
#ifdef THSCRIPT_DEBUG
						THDebug(_T("THScript:�ⲿ��������,����:%s,����:%s,����:%s->%s,�ļ�:%d,%s"),fnName,fntempsetting,tmp,retval,idx,inc->GetScriptFilePath());
#endif
					}
					else
					{
						tmp=RunFunction(fnName,localidx,fntempsetting,nRecursion+1);
#ifdef THSCRIPT_DEBUG
						THDebug(_T("THScript:��������,����:%s,����:%s,����:%s->%s"),fnName,fntempsetting,tmp,retval);
#endif
					}
					if (!retval.IsEmpty())
						SaveVar(retval,tmp,tempsetting,localidx);
				}
			}
			else if (ISCMD(time32))
			{
				//time32:month,$retmon$,$s_Time32$ time32:tickcount,$lasttime$
				if (tokenvar.GetRemainTokensCount()<1)
					OnScriptError(Error_Cmd_ParameterNotFound,cmd)
				else
				{
					THString oper=tokenvar.GetNextToken();
					if (oper==_T("year") || oper==_T("month") || oper==_T("day") ||
						oper==_T("hour") || oper==_T("minute") || oper==_T("second"))
					{
						THString var=tokenvar.GetNextToken();
						THTime ti((__time64_t)THStringConv::s2i(ParseVar(tokenvar.GetNextToken(),tempsetting,localidx)));
						if (var.IsEmpty())
							OnScriptError(Error_Cmd_ParameterNotFound,cmd)
						else if (oper==_T("year"))
							SaveVar(var,THStringConv::i2s(ti.GetYear()),tempsetting,localidx);
						else if (oper==_T("month"))
							SaveVar(var,THStringConv::i2s(ti.GetMonth()),tempsetting,localidx);
						else if (oper==_T("day"))
							SaveVar(var,THStringConv::i2s(ti.GetDay()),tempsetting,localidx);
						else if (oper==_T("hour"))
							SaveVar(var,THStringConv::i2s(ti.GetHour()),tempsetting,localidx);
						else if (oper==_T("minute"))
							SaveVar(var,THStringConv::i2s(ti.GetMinute()),tempsetting,localidx);
						else if (oper==_T("second"))
							SaveVar(var,THStringConv::i2s(ti.GetSecond()),tempsetting,localidx);
					}
					else if (oper==_T("tickcount"))
					{
						THString var=tokenvar.GetNextToken();
						if (var.IsEmpty())
							OnScriptError(Error_Cmd_ParameterNotFound,cmd)
						else
							SaveVar(var,THStringConv::u2s(GetTickCount()),tempsetting,localidx);
					}
					else if (oper==_T("maketime"))
					{
						THString var=tokenvar.GetNextToken();
						int ret[6];
						for(int i=0;i<6;i++)
						{
							double dret=0.0;
							if (!m_calc.Run(ParseVar(tokenvar.GetNextToken(),tempsetting,localidx),&dret))
							{
								OnScriptError(Error_LogicCalc_Error,cmd)
								ret[i]=0;
							}
							else
								ret[i]=THSysMisc::cuttail(dret);
						}
						THTime ti;
						ti.SetTime(ret[0],ret[1],ret[2],ret[3],ret[4],ret[5]);
						SaveVar(var,THStringConv::i2s((int)ti.GetTimeAsTime64()),tempsetting,localidx);
					}
					else if (oper==_T("maketimestr") || oper==_T("maketimestrex"))
					{
						if (tokenvar.GetRemainTokensCount()<2)
							OnScriptError(Error_Cmd_ParameterNotFound,cmd)
						else
						{
							THString var=tokenvar.GetNextToken();
							THString timestr=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
							THTime ti;
							if (oper==_T("maketimestrex"))
							{
								if (tokenvar.GetRemainTokensCount()<1)
									OnScriptError(Error_Cmd_ParameterNotFound,cmd)
								else
								{
									ti.SetTime(timestr,ParseVar(tokenvar.GetNextToken(),tempsetting,localidx));
								}
							}
							else
								ti.SetTime(timestr);
							SaveVar(var,THStringConv::i2s((int)ti.GetTimeAsTime64()),tempsetting,localidx);
						}
					}
					else if (oper==_T("timespan") || oper==_T("timespanstr"))
					{
						if (tokenvar.GetRemainTokensCount()<2)
							OnScriptError(Error_Cmd_ParameterNotFound,cmd)
						else
						{
							THString var=tokenvar.GetNextToken();
							THTime ti;
							THString timefmt;
							if (oper==_T("timespanstr"))
							{
								THString timestr=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
								timefmt=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
								if (timefmt.IsEmpty())
									OnScriptError(Error_Cmd_ParameterNotFound,cmd)
								else
									ti.SetTime(timestr,timefmt);
							}
							else
								ti.SetTime((__time64_t)THStringConv::s2i(ParseVar(tokenvar.GetNextToken(),tempsetting,localidx)));
							int ret[4];
							for(int i=0;i<4;i++)
							{
								double dret=0.0;
								if (!m_calc.Run(ParseVar(tokenvar.GetNextToken(),tempsetting,localidx),&dret))
								{
									OnScriptError(Error_LogicCalc_Error,cmd)
									ret[i]=0;
								}
								else
									ret[i]=THSysMisc::cuttail(dret);
							}
							THTimeSpan span;
							span.SetDateTimeSpan(ret[0],ret[1],ret[2],ret[3]);
							ti+=span;
							if (oper==_T("timespanstr"))
								SaveVar(var,ti.Format(timefmt),tempsetting,localidx);
							else
								SaveVar(var,THStringConv::i2s((int)ti.GetTimeAsTime64()),tempsetting,localidx);
						}
					}
					else if (oper==_T("format"))
					{
						THString var=tokenvar.GetNextToken();
						THTime ti;
						ti.SetTime((__time64_t)THStringConv::s2i(ParseVar(tokenvar.GetNextToken(),tempsetting,localidx)));
						SaveVar(var,ti.Format(ParseVar(tokenvar.GetNextToken(),tempsetting,localidx)),tempsetting,localidx);
					}
					else if (oper==_T("localtogmt") || oper==_T("localtogmtstr"))
					{
						if (tokenvar.GetRemainTokensCount()<2)
							OnScriptError(Error_Cmd_ParameterNotFound,cmd)
						else
						{
							THString savevar=tokenvar.GetNextToken();
							THString timevar=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
							THTime ti;
							THString timeformat;
							if (oper==_T("localtogmtstr"))
							{
								timeformat=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
								if (timeformat.IsEmpty())
									OnScriptError(Error_Cmd_ParameterNotFound,cmd)
								else
									ti.SetTime(timevar,timeformat);
							}
							else
								ti.SetTime((__time64_t)THs2i(timevar));
							ti=ti.LocalTimeToSystemTime();
							if (oper==_T("localtogmtstr"))
								SaveVar(savevar,ti.Format(timeformat),tempsetting,localidx);
							else
								SaveVar(savevar,THStringConv::i2s((int)ti.GetTimeAsTime64()),tempsetting,localidx);
						}
					}
					else if (oper==_T("gmttolocal") || oper==_T("gmttolocalstr"))
					{
						if (tokenvar.GetRemainTokensCount()<2)
							OnScriptError(Error_Cmd_ParameterNotFound,cmd)
						else
						{
							THString savevar=tokenvar.GetNextToken();
							THString timevar=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
							THTime ti;
							THString timeformat;
							if (oper==_T("gmttolocalstr"))
							{
								timeformat=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
								if (timeformat.IsEmpty())
									OnScriptError(Error_Cmd_ParameterNotFound,cmd)
								else
									ti.SetTime(timevar,timeformat);
							}
							else
								ti.SetTime((__time64_t)THs2i(timevar));
							ti=ti.SystemTimeToLocalTime();
							if (oper==_T("gmttolocalstr"))
								SaveVar(savevar,ti.Format(timeformat),tempsetting,localidx);
							else
								SaveVar(savevar,THStringConv::i2s((int)ti.GetTimeAsTime64()),tempsetting,localidx);
						}
					}
					else
						OnScriptError(Error_SubOper_NotRecognise,cmd)
				}
			}
			else if (ISCMD(end))
			{
				//end //end:$returnval$
				//if (tokenvar.IsMoreTokens())
				return _T("ret:")+ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
			}
			else if (ISCMD(emptylasterror))
			{
				//localsetting parse
				SetLocalSettings(localidx,_T("s_LastError"),_T(""));
			}
			else if (ISCMD(sqlite))
			{
				//sqlite:open,abc.db
				//sqlite:sql,insert into testtable(id,i) values(1,2)
				//sqlite:getret,$var1$,select id from testable where id=1
				//sqlite:beginquery,select id from testable
				//sqlite:ifhavenext,readtag,endtag
				//sqlite:getval,$var1$,0
				//sqlite:getvalbykey,$var1$,id
				//sqlite:endquery
				//sqlite:close
				if (tokenvar.GetRemainTokensCount()<1)
					OnScriptError(Error_Cmd_ParameterNotFound,cmd)
				else
				{
					THString oper=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
					if (oper==_T("open"))
					{
						THString path=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
						if (path.IsEmpty())
							OnScriptError(Error_Cmd_ParameterNotFound,cmd)
						else
						{
							if (!m_sqlite.OpenDataBase(path))
								OnScriptError(Error_Sqlite_OpenDataBaseError,cmd)
#ifdef THSCRIPT_DEBUG
							THDebug(_T("THScript:Sql�����ݿ�:%s"),path);
#endif
							//����Sqlite��Ĭ������Timeout,�ű��ж���ط�ͬʱ����sqlite����������
							m_sqlite.SetTimeOut(10000);
						}
					}
					else if (oper==_T("openex"))
					{
						THString path=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
						THString initpath=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
						if (path.IsEmpty())
							OnScriptError(Error_Cmd_ParameterNotFound,cmd)
						else
						{
							BOOL bInit=FALSE;
							if (!::PathFileExists(path)) bInit=TRUE;
							if (!m_sqlite.OpenDataBase(path))
								OnScriptError(Error_Sqlite_OpenDataBaseError,cmd)
#ifdef THSCRIPT_DEBUG
							THDebug(_T("THScript:Sql�߼������ݿ�:%s,��ʼ����:%s"),path,bInit?initpath:_T("��"));
#endif
							if (bInit)
							{
								if (initpath.IsEmpty())
									OnScriptError(Error_Cmd_ParameterNotFound,cmd)
								else
									m_sqlite.ExecSqlFromFile(initpath);
							}
							//����Sqlite��Ĭ������Timeout,�ű��ж���ط�ͬʱ����sqlite����������
							m_sqlite.SetTimeOut(10000);
						}
					}
					else if (oper==_T("sql"))
					{
						THString sql=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
						if (sql.IsEmpty())
							OnScriptError(Error_Cmd_ParameterNotFound,cmd)
						else
						{
#ifdef THSCRIPT_DEBUG
							THDebug(_T("THScript:Sqlִ��sql���:%s"),sql);
#endif
							if (!m_sqlite.ExecSql(sql))
							{
								OnScriptError(Error_Sqlite_ExecSqlError,m_sqlite.GetLastErrMessage())
							}
						}
					}
					else if (oper==_T("getret"))
					{
						THString savevar=tokenvar.GetNextToken();
						THString sql=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
						if (savevar.IsEmpty() || sql.IsEmpty())
							OnScriptError(Error_Cmd_ParameterNotFound,cmd)
						else
						{
							THString ret;
							if (!m_sqlite.GetFirstResult(sql,&ret))
							{
								OnScriptError(Error_Sqlite_ExecSqlError,m_sqlite.GetLastErrMessage())
							}
#ifdef THSCRIPT_DEBUG
							THDebug(_T("THScript:Sqlִ��sql���:%s,���ؽ��:%s->%s"),sql,ret,savevar);
#endif
							SaveVar(savevar,ret,tempsetting,localidx);
						}
					}
					else if (oper==_T("beginquery"))
					{
						THString sql=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
						if (sql.IsEmpty())
							OnScriptError(Error_Cmd_ParameterNotFound,cmd)
						else
						{
							if (!m_sqlite.StartQuery(sql))
								OnScriptError(Error_Sqlite_BeginQueryError,cmd)
#ifdef THSCRIPT_DEBUG
							THDebug(_T("THScript:Sql��ʼ��ѯ����sql���:%s"),sql);
#endif
						}
					}
					else if (oper==_T("endquery"))
					{
						m_sqlite.EndQuery();
#ifdef THSCRIPT_DEBUG
						THDebug(_T("THScript:Sql������ѯ����"));
#endif
					}
					else if (oper==_T("ifhavenext"))
					{
						if (tokenvar.GetRemainTokensCount()<2)
							OnScriptError(Error_Cmd_ParameterNotFound,cmd)
						THString oktag=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
						THString errtag=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
						BOOL bNext=m_sqlite.GetNext();
						THString jumptag;
						if (bNext)
							jumptag=oktag;
						else
							jumptag=errtag;
#ifdef THSCRIPT_DEBUG
						THDebug(_T("THScript:Sql�Ƿ�������:%s,��ת:%s"),bNext?_T("��"):_T("��"),jumptag);
#endif
						return _T("tag:")+jumptag;
					}
					else if (oper==_T("getval"))
					{
						if (tokenvar.GetRemainTokensCount()<2)
							OnScriptError(Error_Cmd_ParameterNotFound,cmd)
						THString savevar=tokenvar.GetNextToken();
						THString key=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
						double ret=0.0;
						if (!m_calc.Run(key,&ret))
							OnScriptError(Error_LogicCalc_Error,cmd)
						else
						{
							THString strret=m_sqlite.GetValue(THSysMisc::cuttail(ret));
#ifdef THSCRIPT_DEBUG
							THDebug(_T("THScript:Sql��ȡ�� (%s = %d) ֵ:%s->%s"),key,THSysMisc::cuttail(ret),strret,savevar);
#endif
							SaveVar(savevar,strret,tempsetting,localidx);
						}
					}
					else if (oper==_T("getvalbykey"))
					{
						if (tokenvar.GetRemainTokensCount()<2)
							OnScriptError(Error_Cmd_ParameterNotFound,cmd)
						else
						{
							THString savevar=tokenvar.GetNextToken();
							THString key=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
							THString strret=m_sqlite.GetValueByKey(key);
#ifdef THSCRIPT_DEBUG
							THDebug(_T("THScript:Sql��ȡ�� %s ֵ:%s->%s"),key,strret,savevar);
#endif
							SaveVar(savevar,strret,tempsetting,localidx);
						}
					}
					else if (oper==_T("getcolcount"))
					{
						if (tokenvar.GetRemainTokensCount()<1)
							OnScriptError(Error_Cmd_ParameterNotFound,cmd)
						else
						{
							THString savevar=tokenvar.GetNextToken();
							THString strret=THStringConv::i2s(m_sqlite.GetColSize());
#ifdef THSCRIPT_DEBUG
							THDebug(_T("THScript:Sql��ȡ����:%s->%s"),strret,savevar);
#endif
							SaveVar(savevar,strret,tempsetting,localidx);
						}
					}
					else if (oper==_T("getcolname"))
					{
						if (tokenvar.GetRemainTokensCount()<2)
							OnScriptError(Error_Cmd_ParameterNotFound,cmd)
						else
						{
							THString savevar=tokenvar.GetNextToken();
							THString key=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
							double ret=0.0;
							if (!m_calc.Run(key,&ret))
								OnScriptError(Error_LogicCalc_Error,cmd)
							else
							{
								THString strret=m_sqlite.GetColName(THSysMisc::cuttail(ret));
	#ifdef THSCRIPT_DEBUG
								THDebug(_T("THScript:Sql��ȡ�� (%s = %d) ������:%s->%s"),key,THSysMisc::cuttail(ret),strret,savevar);
	#endif
								SaveVar(savevar,strret,tempsetting,localidx);
							}
						}
					}
					else if (oper==_T("close"))
					{
						m_sqlite.CloseDataBase();
#ifdef THSCRIPT_DEBUG
						THDebug(_T("THScript:Sql�ر����ݿ�"));
#endif
					}
					else if (oper==_T("export"))
					{
						THString sql=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
						THString file=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
						THString sep1=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
						THString sep2=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
						BOOL bUseTitle=THStringConv::s2i(ParseVar(tokenvar.GetNextToken(),tempsetting,localidx));
#ifdef THSCRIPT_DEBUG
						THDebug(_T("THScript:Sql��������,sql���:%s,�ļ�:%s,�зָ���:%s,�зָ���:%s,�������:%s"),sql,file,sep1,sep2,bUseTitle?_T("��"):_T("��"));
#endif
						if (!m_sqlite.ExportData(sql,file,sep1,sep2,bUseTitle))
							OnScriptError(Error_Sqlite_ExportError,cmd)
					}
					else if (oper==_T("exporttable"))
					{
						if (tokenvar.GetRemainTokensCount()<2)
							OnScriptError(Error_Cmd_ParameterNotFound,cmd)
						else
						{
							THString savevar=tokenvar.GetNextToken();
							THString sql=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
							BOOL bUseTitle=THStringConv::s2i(ParseVar(tokenvar.GetNextToken(),tempsetting,localidx));
							THString saveline=tokenvar.GetNextToken();
							THString savecnt=tokenvar.GetNextToken();
							THString ls=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
							THString le=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
							THString is=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
							THString ie=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
							THString tis=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
							THString tie=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
							THString head=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
							THString end=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
							THString emptyvalue=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
#ifdef THSCRIPT_DEBUG
							THDebug(_T("THScript:Sql�������ݱ�,sql���:%s,�������:%s,��:%s,%s,��Ŀ:%s,%s,������:%s,%s,ͷ:%s,β:%s,��ֵ:%s"),sql,bUseTitle?_T("��"):_T("��"),ls,le,is,ie,tis,tie,head,end,emptyvalue);
#endif
							UINT lc,ic;
							THString ret=m_sqlite.ExportDataToTable(sql,bUseTitle,&lc,&ic,ls,le,is,ie,tis,tie,head,end,emptyvalue);
							if (ret.IsEmpty())
								OnScriptError(Error_Sqlite_ExportError,cmd)
							else
							{
								SaveVar(savevar,ret,tempsetting,localidx);
								if (!saveline.IsEmpty()) SaveVar(saveline,THu2s(lc),tempsetting,localidx);
								if (!savecnt.IsEmpty()) SaveVar(savecnt,THu2s(ic),tempsetting,localidx);
							}
						}
					}
					else
						OnScriptError(Error_SubOper_NotRecognise,cmd)
				}
			}
			else if (ISCMD(msgbox))
			{
				THString saveval=tokenvar.GetNextToken();
				THString text=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
				THString title=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
				THString mode=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
				THString ret;
				ret.Format(_T("%d"),::MessageBox(NULL,text,title,THStringConv::s2i(mode)));
#ifdef THSCRIPT_DEBUG
				THDebug(_T("THScript:�����Ի���,����:%s,����:%s,��ʽ:%s,����ֵ:%s->%s"),title,text,mode,ret,saveval);
#endif
				if (!saveval.IsEmpty())
					SaveVar(saveval,ret,tempsetting,localidx);
			}
			else if (ISCMD(include))
			{
				if (tokenvar.GetRemainTokensCount()<2)
					OnScriptError(Error_Cmd_ParameterNotFound,cmd)
				else
				{
					THString oper=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
					THString calc=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
					double ret=0.0;
					if (!m_calc.Run(calc,&ret))
						OnScriptError(Error_LogicCalc_Error,cmd)
					{
						int idx=THSysMisc::cuttail(ret);
						if (oper==_T("init"))
						{
							if (tokenvar.GetRemainTokensCount()<1)
								OnScriptError(Error_Cmd_ParameterNotFound,cmd)
							else
							{
								THString path=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
#ifdef THSCRIPT_DEBUG
								THDebug(_T("THScript:�ⲿ���ýű���ʼ��:%d,�ļ�:%s"),idx,path);
#endif
								THScript *scrcls=new THScript();
								//����ext�����������
								scrcls->SetParseExtCmdClass(m_extcls);
								scrcls->SetParseExtCmdProc(m_cb,m_adddata);
								ITHFileStore *store=m_pStore->CreateObject();
								store->SetBaseDirectory(THSysMisc::GetFilePath(path));
								scrcls->Init(path,store,TRUE);
								m_include.SetAt(idx,scrcls);
							}
						}
						else if (oper==_T("free"))
						{
#ifdef THSCRIPT_DEBUG
							THDebug(_T("THScript:�ⲿ���ýű��ͷ�:%d"),idx);
#endif
							m_include.RemoveAt(idx);
						}
						else if (oper==_T("getlocal") || oper==_T("savelocal") || oper==_T("getglobal") || oper==_T("saveglobal"))
						{
							if (tokenvar.GetRemainTokensCount()<2)
								OnScriptError(Error_Cmd_ParameterNotFound,cmd)
							else
							{
								THString par1=tokenvar.GetNextToken();
								THString par2=tokenvar.GetNextToken();
								THScript *inc=NULL;
								THString val;
								if (!m_include.GetAt(idx,inc) || inc==NULL)
									OnScriptError(Error_Include_NoExistIndex,cmd)
								else
								{
#ifdef THSCRIPT_DEBUG
									THDebug(_T("THScript:�ⲿ���ýű���������:����:%s,����:%d,����1:%s,����2:%s"),oper,idx,par1,par2);
#endif
									if (oper==_T("getlocal"))
									{
										val=inc->GetLocalSettings(localidx,ParseVar(par2,tempsetting,localidx));
										SaveVar(par1,val,tempsetting,localidx);
									}
									else if (oper==_T("getglobal"))
									{
										val=inc->GetGlobalSettings(ParseVar(par2,tempsetting,localidx));
										SaveVar(par1,val,tempsetting,localidx);
									}
									else if (oper==_T("savelocal"))
									{
										inc->SetLocalSettings(localidx,ParseVar(par1,tempsetting,localidx),ParseVar(par2,tempsetting,localidx));
									}
									else if (oper==_T("saveglobal"))
									{
										inc->SetGlobalSettings(ParseVar(par1,tempsetting,localidx),ParseVar(par2,tempsetting,localidx));
									}
								}
							}
						}
						else
							OnScriptError(Error_SubOper_NotRecognise,cmd)
					}
				}
			}
			else if (ISCMD(timer))
			{
				if (tokenvar.GetRemainTokensCount()<2)
					OnScriptError(Error_Cmd_ParameterNotFound,cmd)
				else
				{
					THString oper=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
					THString Timerid=tokenvar.GetNextToken();
					THString Timeridret=ParseVar(Timerid,tempsetting,localidx);
					if (oper==_T("init"))
					{
						if (tokenvar.GetRemainTokensCount()<2)
							OnScriptError(Error_Cmd_ParameterNotFound,cmd)
						else
						{
							THString timeval=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
							THString retfn=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
							THString loop=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
							THString adddata=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
							double ret=0.0,ret1=0.0;
							if (!m_calc.Run(timeval,&ret) || !m_calc.Run(loop,&ret1))
								OnScriptError(Error_LogicCalc_Error,cmd)
							{
								int timev=THSysMisc::cuttail(ret);
								int bloop=THSysMisc::cuttail(ret1);
								int timerid=SetTimer(localidx,timev,retfn,adddata,bloop);
								if (timerid==0)
									OnScriptError(Error_InitTimer_Error,cmd)
#ifdef THSCRIPT_DEBUG
									THDebug(_T("THScript:��ʱ����ʼ��:��ʱ����%d->%s,��ʱ���:(%s = %d),�ص�����:%s,���Ӳ���:%s,�Ƿ�ѭ��:(%s = %s)"),timerid,Timerid,timeval,timev,retfn,adddata,loop,bloop?_T("��"):_T("��"));
#endif
								SaveVar(Timerid,THStringConv::i2s(timerid),tempsetting,localidx);
							}
						}
					}
					else if (oper==_T("free"))
					{
						double ret=0.0;
						if (!m_calc.Run(Timeridret,&ret))
							OnScriptError(Error_LogicCalc_Error,cmd)
						{
							int timerid=THSysMisc::cuttail(ret);
#ifdef THSCRIPT_DEBUG
							THDebug(_T("THScript:��ʱ���ͷ�:��ʱ����(%s = %d)"),Timeridret,timerid);
#endif
							UnSetTimer(timerid);
						}
					}
					else if (oper==_T("reset"))
					{
						THString valret=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
						double ret=0.0,ret1=0.0;
						if (!m_calc.Run(Timeridret,&ret) || !m_calc.Run(valret,&ret1))
							OnScriptError(Error_LogicCalc_Error,cmd)
						{
							int timerid=THSysMisc::cuttail(ret);
							int timerval=THSysMisc::cuttail(ret1);
#ifdef THSCRIPT_DEBUG
							THDebug(_T("THScript:��ʱ������:��ʱ����:(%s = %d),����ʱ��:(%s = %d)"),Timeridret,timerid,valret,timerval);
#endif
							ResetTimer(timerid,timerval);
						}
					}
					else
						OnScriptError(Error_SubOper_NotRecognise,cmd)
				}
			}
			else if (ISCMD(schedule))
			{
				THString oper=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
				THString scid=tokenvar.GetNextToken();
				THString scidret=ParseVar(scid,tempsetting,localidx);
				if (oper==_T("new"))
				{
					if (tokenvar.GetRemainTokensCount()<5)
						OnScriptError(Error_Cmd_ParameterNotFound,cmd)
					else
					{
						THString tformat=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
						THString mode=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
						THString defval=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
						THString loop=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
						THString retcall=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
						THString retvalue=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
						THTime ba;
						ba.SetTime(tformat);
						double ret=0.0,ret1=0.0;
						if (!m_calc.Run(defval,&ret) || !m_calc.Run(loop,&ret1))
							OnScriptError(Error_LogicCalc_Error,cmd)
						else
						{
							int nmode=0;
							if (mode.Find(_T("m"))!=-1) nmode|=SCHEDULEMODE_MONTH;
							if (mode.Find(_T("d"))!=-1) nmode|=SCHEDULEMODE_DAY;
							if (mode.Find(_T("w"))!=-1) nmode|=SCHEDULEMODE_WEEK;
							if (mode.Find(_T("H"))!=-1) nmode|=SCHEDULEMODE_HOUR;
							if (mode.Find(_T("M"))!=-1) nmode|=SCHEDULEMODE_MINUTE;
							if (mode.Find(_T("S"))!=-1) nmode|=SCHEDULEMODE_SECORD;
							int nscid=NewSchedule(0,ba,nmode,(int)ret,(int)ret1,(void *)m_scar.Add(THi2s(localidx)+_T(":")+retcall+_T(":")+retvalue));
#ifdef THSCRIPT_DEBUG
							THDebug(_T("THScript:�ƻ�����:�½��ƻ�:�ƻ���:%d,ʱ��:%s,ģʽ:%d,��ʱ���:%d,ѭ��:%d,�ص�:%s,����:%s"),nscid,ba.Format(),nmode,(int)ret,(int)ret1,retcall,retvalue);
#endif
							if (nscid==0)
								OnScriptError(Error_LogicCalc_Error,cmd)
							SaveVar(scid,THi2s(nscid),tempsetting,localidx);
						}
					}
				}
				else if (oper==_T("free"))
				{
					int nscid=THs2i(scidret);
					if (nscid!=0)
					{
#ifdef THSCRIPT_DEBUG
						THDebug(_T("THScript:�ƻ�����:ֹͣ����::%d"),nscid);
#endif
						StopSchedule(nscid);
					}
				}
				else
					OnScriptError(Error_SubOper_NotRecognise,cmd)
			}
			else if (ISCMD(file))
			{
				THString oper=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
				if (oper==_T("copyfile"))
				{
					if (tokenvar.GetRemainTokensCount()<2)
						OnScriptError(Error_Cmd_ParameterNotFound,cmd)
					else
					{
						THString src=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
						THString dist=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
						THString ismust=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
						BOOL bMust;
						if (ismust.IsEmpty())
							bMust=TRUE;
						else
						{
							double ret=0.0;
							if (!m_calc.Run(ismust,&ret))
								OnScriptError(Error_LogicCalc_Error,cmd)
							else
								bMust=(int)ret;
						}
						BOOL ret=CopyFile(src,dist,bMust);
#ifdef THSCRIPT_DEBUG
						THDebug(_T("THScript:�����ļ�:���:%d ������ʧ��:%d %s -> %s"),ret,bMust,src,dist);
#endif
					}
				}
				else if (oper==_T("movefile"))
				{
					if (tokenvar.GetRemainTokensCount()<2)
						OnScriptError(Error_Cmd_ParameterNotFound,cmd)
					else
					{
						THString src=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
						THString dist=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
						BOOL ret=MoveFile(src,dist);
#ifdef THSCRIPT_DEBUG
						THDebug(_T("THScript:�ƶ��ļ�:���:%d %s -> %s"),ret,src,dist);
#endif
					}
				}
				else if (oper==_T("isfileexist"))
				{
					if (tokenvar.GetRemainTokensCount()<2)
						OnScriptError(Error_Cmd_ParameterNotFound,cmd)
					else
					{
						THString sret=tokenvar.GetNextToken();
						THString src=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
						BOOL ret=::PathFileExists(src);
#ifdef THSCRIPT_DEBUG
						THDebug(_T("THScript:�ļ��Ƿ����:���:%d %s"),ret,src);
#endif
						SaveVar(sret,THi2s(ret),tempsetting,localidx);
					}
				}
				else
					OnScriptError(Error_SubOper_NotRecognise,cmd)
			}
			else if (ISCMD(syscall))
			{
				//syscall:CopyFileA,1,$ret$,c\m\\RtmpSDK.log,c\m\\RtmpSDK111.log,1;
				if (tokenvar.GetRemainTokensCount()<1)
					OnScriptError(Error_Cmd_ParameterNotFound,cmd)
				else
				{
					THString fn=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
					int type=THs2i(ParseVar(tokenvar.GetNextToken(),tempsetting,localidx));
					THString savevar;
					if (type!=0) savevar=tokenvar.GetNextToken();
					THString ret;
					//pass the args
					THString arg;
					int index=1;
					while(tokenvar.IsMoreTokens() && index<=20)
					{
						THSimpleXml::SetParam(&arg,_T("arg")+THi2s(index++),ParseVar(tokenvar.GetNextToken(),tempsetting,localidx));
					}
					BOOL bret=THSysCallRapper::GetInstance()->SysCall(fn,arg,type,&ret);
#ifdef THSCRIPT_DEBUG
					THDebug(_T("THScript:ϵͳAPI����:���%d:����:%s,ģʽ:%d,����ֵ:%s,����:%s"),bret,fn,type,ret,arg);
#endif
					if (bret)
					{
						if (!savevar.IsEmpty()) SaveVar(savevar,ret,tempsetting,localidx);
					}
					else
						OnScriptError(Error_SysCall_Failed,cmd)
				}
			}
			else
			{
#ifdef THSCRIPT_DEBUG
				THDebug(_T("THScript:�ⲿ��������:%s"),cmd);
#endif
				if (!OnParseExtCmd(cmd,&tokenvar,tempsetting,localidx))
					OnScriptError(Error_Cmd_NotRecognise,cmd)
			}
		}
		return _T("");
	}
	//�����Ƿ�֧�ָ��������
	virtual BOOL OnParseExtCmd(THString cmd,THStringToken *tokenvar,THString *tempsetting,int localidx)
	{
		if (m_cb)
			return m_cb(this,cmd,tokenvar,tempsetting,localidx,m_adddata);
		if (m_extcls)
			return m_extcls->OnParseExtCmd(cmd,tokenvar,tempsetting,localidx);
		return FALSE;
	}

	THString ParseSepSym(THString var)
	{
		var.Replace(_T("$$"),_T("$"));
		THString ret;
		for(int i=0;i<var.GetLength();i++)
		{
			if (var.GetAt(i)=='\\')
			{
				TCHAR ch=var.GetAt(i+1);
				if (ch=='r') ret+=_T("\r");
				else if (ch=='n') ret+=_T("\n");
				else if (ch=='t') ret+=_T("\t");
				else if (ch=='s') ret+=_T(" ");
				else if (ch=='q') ret+=_T(",");
				else if (ch=='f') ret+=_T(";");
				else if (ch=='m') ret+=_T(":");
				else if (ch=='\\') ret+=_T("\\");
				// \e for empty
				else if (ch!='e')
					OnErrorHandler(Error_Cmd_CanNotParseSepSym,var);
				i++;
			}
			else
				ret+=var.Mid(i,1);
		}
		return ret;
	}

	THString GetVar(THString name,THString *tempsetting,int localidx)
	{
		THString value;
		if (name.Left(2)==_T("g_"))
			value=THSimpleXml::GetParam(m_global,name);
		if (value.IsEmpty())
		{
			//localsetting parse
			if (name.Left(2)==_T("l_"))
				value=GetLocalSettings(localidx,name);
			if (value.IsEmpty())
				if (tempsetting) value=THSimpleXml::GetParam(*tempsetting,name);
		}
		if (name==_T("s_Rand"))
		{
			THSysMisc::srand();
			value.Format(_T("%u"),rand());
		}
		else if (name==_T("s_TickCount"))
		{
			value.Format(_T("%u"),GetTickCount());
		}
		else if (name==_T("s_Time32"))
		{
			THTime time;
			value=THStringConv::i2s((int)time.GetTimeAsTime64());
		}
		else if (name==_T("s_LastError"))
		{
			value=GetLocalSettings(localidx,_T("s_LastError"));
		}
		else if (name==_T("s_EngineVer"))
		{
			value=THStringConv::i2s(THScriptEngineVersion);
		}
		else if (name==_T("s_CurPath"))
		{
			value=THSysMisc::GetFilePath(GetScriptFilePath());
		}
		else if (name==_T("s_CurScript"))
		{
			value=GetScriptFilePath();
		}
		else if (name==_T("s_WinPath"))
		{
			value=THSysMisc::GetWindowsPath();
		}
		else if (name==_T("s_SysPath"))
		{
			value=THSysMisc::GetSystemPath();
		}
		else if (name==_T("s_TempPath"))
		{
			value=THSysMisc::GetTempPath();
		}
		return value;
	}

	virtual void ThreadFunc(int timerid,void *adddata)
	{
		THTimerMap *map;
		BOOL bRelease=FALSE;
		BOOL bpret=m_TimerMap.GetAt(timerid,map);
		if (bpret && map)
		{
			THString varsettings;
			varsettings+=THSimpleXml::MakeParam(_T("arg1"),THStringConv::i2s(timerid));
			varsettings+=THSimpleXml::MakeParam(_T("arg2"),map->retval);
			RunScript(map->fnName,map->localidx,varsettings);
			if (!map->bLoop) bRelease=TRUE;
		}
		else
			bRelease=TRUE;
		if (bRelease)
			UnSetTimer(timerid);
	}

	virtual void ScheduleFunc(int scheduleid,void *adddata)
	{
		int scid=(int)(INT_PTR)adddata;
		if (m_scar.GetSize()>scid && scid>=0)
		{
			THStringToken token(m_scar[scid],_T(":"));
			int localidx=THs2i(token.GetNextToken());
			THString ret=token.GetNextToken();
			THString adddata=token.GetNextToken();
			THString varsettings;
			varsettings+=THSimpleXml::MakeParam(_T("arg1"),THStringConv::i2s(scheduleid));
			varsettings+=THSimpleXml::MakeParam(_T("arg2"),adddata);
			if (!ret.IsEmpty())
				RunScript(ret,localidx,varsettings);
		}
	}

	typedef struct _THTimerMap{
		int localidx;
		THString fnName;
		THString retval;
		BOOL bLoop;
	}THTimerMap;
	THMap<int,THTimerMap *> m_TimerMap;
	ITHIni m_ini;
	THString m_global;
	THMap<int,THString *> m_local;
	THMap<int,THScript *> m_include;
	THParseExtCmdProc m_cb;
	const void *m_adddata;
	THScript *m_extcls;
	ITHFileStore *m_pStore;
	THFileStore m_Store;
	THMutex m_localsettinglock;
	ITHScriptDebugger *m_pDebugger;
public:
	//��ExtCmdProc����ʱ�ÿ�����Ҫ�õ�
	THLogicCalc m_calc;
	THSqliteDll m_sqlite;
	THStringArray m_scar;
};
