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
	//返回TRUE表示忽略该错误
	virtual BOOL OnScriptErrorHandler(int nErrCode,THString sErrText){return FALSE;}
	//当开始调用函数时触发，nRecursion表示递归次数，注意在Timer,Schedule及外部函数调用时，递归都由0开始计数
	//返回FALSE表示退出该函数执行
	virtual BOOL OnEnterFunction(THString sCurrentFn,int nRecursion,int localidx,THString *tempsetting){return TRUE;}
	//当结束调用函数时触发，sRet表示函数返回值
	virtual void OnLeaveFunction(THString sCurrentFn,int nRecursion,THString *sRet,int localidx){}
	//返回FALSE表示不执行该句脚本
	virtual BOOL OnRunScript(THString script,int localidx,THString *tempsetting){return TRUE;}
	//进入tag
	virtual void OnEnterTag(THString fn,THString tag,THString *script){}
};

class THScript;
typedef BOOL (*THParseExtCmdProc)(THScript *script,THString cmd,THStringToken *tokenvar,THString *tempsetting,int localidx,const void *adddata);

/**
* @brief 脚本解释运行类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-06-13 新建类
* @2007-06-18 增加解释器版本判断，使用文档撰写
* @2007-07-04 增加Sqlite数据库支持，增加系统目录路径变量，使用文档更新
* @2007-07-10 增加初始化调用函数InitCall和释构调用函数FreeCall，增加逗号替换宏，使用文档更新
* @2007-07-24 增加对引用调用支持，使用文档更新
* @2007-07-31 增加定时器支持，使用文档更新
* @2007-08-01 增加外部命令解释接口，支持include加载脚本的解释接口传递，使用文档更新
* @2007-08-20 增加sqlite子命令openex，支持不存在脚本文件时进行建表，使用文档更新
* @2007-08-22 增加调试模式，输出运行结果
* @2007-08-31 增加THScript-UltraEdit.txt，支持THScript在UltraEdit中高亮显示
* @2007-09-09 增加文件存储器支持，注意sqlite数据库操作仅支持磁盘类型存储器
* @2007-09-12 增加更多time32子命令，使用文档更新
* @2007-09-17 增加命令resetglobalsetting，使用文档更新
* @2007-11-03 THScript-UltraEdit.txt文件更新
* @2007-11-18 增加更多time32子命令，使用文档更新
* @2007-11-22 增加支持计划任务，增加file子命令，使用文档，THScript-UltraEdit.txt更新
* @2008-05-07 增加setintu命令，增加sqlite子命令exporttable，增加str子命令formatnum，使用文档，THScript-UltraEdit.txt更新
* @2009-09-27 增加syscall命令，用于调用系统API，使用文档，THScript-UltraEdit.txt更新，更改版本180
* @2009-09-28 增加调试模式支持
*/
/**<pre>
使用方法：
	THScript script;
	THString varsettings;
	script.Init("c:\\a.txt");
	varsettings+=THSimpleXml::MakeParam(_T("arg1"),FetionNum);
	varsettings+=THSimpleXml::MakeParam(_T("arg2"),PhoneNum);
	varsettings+=THSimpleXml::MakeParam(_T("arg3"),UserName);
	varsettings+=THSimpleXml::MakeParam(_T("arg4"),Msg);
	THString ret=m_script.RunScript(_T("NewInstance"),1234,varsettings);
	ret=m_script.GetLocalSettings(1234,_T("l_retstr"));
语法：
	文件结构：脚本文件结构使用Windows标准ini结构组成。
		举例如下：
			//函数TagName
			[TagName]
			start=XXX;

			//数据
			[TagData]
			data1=XXX;
		中括号[]中的名字称为段名，每个段内有多项数据，如：start项和data1项。start和data1称为键名，XXX称为键值。//用于程序注释
		文件中函数和数据的定义都是一样的，函数的段中必须由start项进行开始。
	函数：任何脚本都由多个函数段和数据段组成，函数和数据都遵循ini文件语法，两者写法上没有什么区别。
		调用脚本通过调用具体函数来运行，如果没有指定需要调用的函数，默认调用文件中的函数ScriptMain。
		调用某函数实质上是运行函数中的start键名中的脚本，注意系统只会执行完该行脚本，而不会顺序执行其它脚本，因此多分支的逻辑是通过在脚本中对不同情况进行不同的处理，例如调用不同键名的脚本来执行。
			举例如下：
				start=goto:tag1;
				tag1=XXX;
			在start中跳转到tag1键名上执行。
		函数的返回值由指令end给出，详见指令end。
		解释器初始化时，将会自己调用InitCall函数进行初始化操作，解释器释构时调用FreeCall函数进行释放。跨文件引用时分别在文件的include:init和include:free时调用InitCall和FreeCall，详见include指令。
	语句：语句语法由指令确定语句执行的命令,由:和,传递参数,由;表示结束,同一行内可执行多句脚本：
			指令[:参数1[,参数2,[参数n...]]][;][指令[:参数1[,参数2,[参数n...]]][;]]
		举例如下：
			end		//最后一句指令不需要加分号也行
			end;
			end:1
			end:1;
			if:1,=,2,tag1,tag2;end
	跳转键名：当程序跳转到其它键名时，键名的语法定义如下：
			键名1[~出错调用键名]
		即跳转到键名1，如果键名1不存在，调用出错调用键名，出错调用键名可以省略。
		注意：特殊键名_continue表示继续运行该行的下一条指令，不进行跳转。
		键名可使用变量确定。
		举例如下：
			goto:tagnext;
			goto:tagnext~tagerror;
			goto:_continue;
			goto:tagnext~_continue;
			goto:$state$;
			//由nowtype变量指定跳转的tag
			goto tagstate$nowtype$;
	变量：由两个$组成，变量可以使用一对[]表示需要先解释[]中的变量，定义如下：
			$变量名[优先变量名]$
			举例如下：
				//例如mystate值为1，那么下面语句就是mystate 的值为：1
				mystate 的值为：$mystate$
				//下面语句就是$tag1$这个变量的内容
				$tag[mystate]$
			连续$$代表真实$符号。
	字符串：字符串不用区分直接书写。
			字符串中需要插入变量直接书写$变量名$，如要$号请输入$$。
			另外\为转义字符，输出\请输入\\。输出其他宏定义如下：
				\r	回车符
				\n	换行符
				\t	Tab
				\s	空格（或直接输入空格）
				\q  ,
				\f	;
				\m	:
				\e	不替换任何数据，用于把$$分开，如：$var$\e$var1$
	变量生存期（有效范围）：变量共分为4种类型
		1.全局变量
			由g_开头，所有情况下变量都进行共用，脚本开始运行时，会自动读取脚本文件中的GlobalVal段，并把所有初始变量自动读入
			举例如下：
				ScriptFile.txt内容：
				[GlobalVal]
				g_TitleCount=4;
				g_MyTest=这是一个测试
			那么脚本初始化时就会读入这些变量。
			在脚本中直接对该值存取即可。
			在解释器中通过调用Get/SetGlobalSettings实现全局变量存取。
		2.用户变量
			由l_开头，同一用户执行时变量共用，用户索引在解释器调用执行脚本时给出，相同用户索引使用相同的变量，该类型变量适合做状态机。
			在脚本中直接对该值存取即可。
			在解释器中通过调用Get/SetLocalSettings实现全局变量存取。
		3.系统变量
			由s_开头，系统中的初始定义的变量，不能对系统变量进行赋值。
			s_LastError：系统最后错误的出错码，注意：s_LastError在命令执行正确时不更新赋值，因此如果需要判断某一操作是否成功，在调用之前请先调用emptylasterror，另外s_LastError保存在用户变量中
			s_Rand：为随机值，每次动态生成
			s_TickCount：实时TickCount值，操作系统已启动的毫秒数，多数用于计算超时
			s_Time32：32位整数时间
			s_EngineVer：脚本解释引擎版本号
			s_CurPath：脚本文件所在目录
			s_CurScript：脚本文件路径
			s_WinPath：Windows目录
			s_SysPath：System目录
			s_TempPath：临时目录
			在脚本中直接对该值读取即可。
		4.临时变量
			由非以上变量组成，每次调用时才使用，函数执行完成后释放。
			调用函数时，函数参数通过临时变量赋值传递，函数参数以arg1-arg20组成
			在脚本中直接对该值存取即可。
			在解释器中通过调用时从参数传入。
	指令：
		如果指令无法解释，引发Error_Cmd_NotRecognise。
		如果指令参数不够，引发Error_Cmd_ParameterNotFound。
		如果无法处理转义符'\\'的意思，引发Error_Cmd_CanNotParseSepSym。
		如果解释器解释完成当前脚本后，不知道怎样继续操作，引发Error_Function_UnknownHowToContinue。
		如果进行四则运算时出现错误，引发Error_LogicCalc_Error。
		外部指令解释：
			解释器支持扩充的外部脚本解释，实现有几种方法：
				1.继承类THScript，并重载OnParseExtCmd函数。
				2.定义一外部处理函数THParseExtCmdProc，通过SetParseExtCmdProc设置挂接。
				3.多个THScript类需要用到同样的解释函数，可通过SetParseExtCmdClass设置挂接到别的类中进行解释。
			通过include加载进行的脚本解释器，会自动继承外部调用设置。
			处理函数返回TRUE表示正确解释脚本，返回FALSE为无法正确解释命令，脚本解释器引发Error_Cmd_NotRecognise错误，对于重载函数方式的实现，可以返回TRUE，并设置别的错误代码。
		goto[:tag[~errtag]]
			转向到指定的tag键值运行。如果转向错误转向errtag键值运行。如果转向出错且没有errtag，引发错误Error_Function_TagNotFound。
		getini:$savevar$,section,key[,defval[,file]]
			从脚本文件中获取数据
			savevar：读出数据后保存到该变量
			section：数据所在段名
			key：数据所在键名
			defval：默认值
			file：不指定从当前文件读取，指定从指定文件读取
		setini:section,key,value[,file]
			从脚本文件中保存数据
			section：数据所在段名
			key：数据所在键名
			value：键值
			file：不指定从当前文件读取，指定从指定文件读取
		if:val1,=,val2,tag1,tag2
			逻辑判断，如果val1=val2，跳转到tag1，否则跳转到tag2
			va1：表达式1，支持四则运算
			=：除了等值比较，还有>，<，>=，<=，<>
			var2：表达式2，支持四则运算
			运算符无法处理返回错误Error_Oper_NotRecognise
		resetglobalsetting
			重置(重新读取)全局变量。包括所有include进来的脚本文件。
		emptyglobalsetting
			清空全局变量。包括所有include进来的脚本文件。
		emptylocalsetting
			清空本地变量。包括所有include进来的脚本文件。
			注意由于系统变量s_LastError借用本地变量的存储区域进行保存，因此清空本地变量会清空LastError，使用中请注意。
		emptytempsetting
			清空临时变量。
		emptylasterror
			清空s_LastError值。
			注意：由于s_LastError在命令执行正确时不更新赋值，因此如果需要判断某一操作是否成功，在调用之前请先调用emptylasterror。
			注意由于系统变量s_LastError借用本地变量的存储区域进行保存，因此清空本地变量会清空LastError，使用中请注意。
			注意include进来的文件的lasterror是保存在对应脚本文件中的。
		str:subcmd,$val$[,$val1$[,$val2$]]
			字符串处理函数，功能由subcmd指定，如果subcmd解释错误，返回错误Error_SubOper_NotRecognise
			subcmd命令如下：
				i2s,$val$
					对参数取整操作
				lower,$val$
					对参数val进行小写转换
				upper,$val$
					对参数val进行大写转换
				replace,$val$,全部,all
					把参数val中的'全部'换成'all'
				append,$val$,AAA
					在参数val后加入字符串'AAA'
				trim,$val$[,\r\t]
					把字符串进行首尾空白裁减，空白范围默认为空格，也可以后面的参数指定
				set,$val$[,value]
					设置字符串值val为value，value为空为清空该值
				compare,$val$,text,tag1,tag2
					比较变量val和text字符串的结果，相同转向tag1，否则转向tag2
				leftcompare,$val$,text,tag1,tag2
					仅与text比较从开始到$val$长度的字符串，相同转向tag1，否则转向tag2
				left,$val$,$str$,cnt
					左截断字符串
				mid,$val$,$str$,cnt
					右截断字符串
				find,$ret$,$val$,AAA,startpos
					从startpos处查找字符串中子字符串AAA出现的位置，找不到返回-1
				len,$ret$,$val$
					获取字符串长度
				formatnum,$ret$,$val$
					把数字转换成TGMK等单位。直接舍值。
		set:$val$,(1+2)%4,2
			表达式赋值，把(1+2)%4计算结果赋值到val中，支持四则运算，字符串赋值请使用str:set，最后的2表示保留小数点后2位，可为空。
		setint:$val$,(1+2)%4
			表达式赋值，把(1+2)%4计算结果4舍5入为整数，赋值到val中，支持四则运算，字符串赋值请使用str:set。
		setintu:$val$,(1+2)%4
			表达式赋值，把(1+2)%4计算结果4舍5入为无符号整数，赋值到val中，支持四则运算，字符串赋值请使用str:set。
		function:GenQuestion[,$ret$,0,1,5]
			如果返回的数据出错，返回错误Error_Function_Return_NotAvailable。
			调用函数GenQuestion，返回值保存到变量ret中，后跟多个参数会保存到调用函数的临时变量arg1-arg20中。
			需要跨文件调用时由!号区分,如: 1!GenQuestion，其中!之前的引用索引支持四则运算，关于引用索引的详细介绍，请参见include命令部分。
			注意：调用函数的临时变量与当前执行函数临时变量是互相独立的。
		time32:subcmd,$val$[,$val$]
			32位时间处理函数，功能由subcmd指定，如果subcmd解释错误，返回错误Error_SubOper_NotRecognise
			subcmd命令如下：
				year,$val$,$s_Time32$
					获取当前时间s_Time32的年份，保存到val中
				month,$val$,$s_Time32$
					获取当前时间s_Time32的月份，保存到val中
				day,$val$,$s_Time32$
					获取当前时间s_Time32的日期，保存到val中
				hour,$val$,$s_Time32$
					获取当前时间s_Time32的小时数，保存到val中
				minute,$val$,$s_Time32$
					获取当前时间s_Time32的分钟数，保存到val中
				second,$val$,$s_Time32$
					获取当前时间s_Time32的秒数，保存到val中
				tickcount,$val$
					获取cpu时间，保存到val中
				maketime,$val$,$year$,$month$,$day$,$hour$,$minute$,$secord$
					生成指定时间，时间量可省略。
				maketimestr,$val$,$timestr$
					用时间字符串生成指定时间。
				maketimestrex,$val$,$timestr$,%Y-%m-%d
					用时间字符串以指定格式生成时间。
				timespan,$val$,$timeval$,$days$,$hour$,$minute$,$secord$
					由$timeval$时间偏移$days$,$hour$,$minute$,$secord$后生成时间$val$，偏移时间量可以省略。
				timespanstr,$val$,$timeval$,%y-%m-%d,$days$,$hour$,$minute$,$secord$
					由%y-%m-%d格式指定的$timeval$时间字符串偏移$days$,$hour$,$minute$,$secord$后生成时间字符串$val$，偏移时间量可以省略。
				format,$val$,$timeval$,%Y-%m-%d
					由%y-%d-%m指定格式化时间$timeval$，格式字符串可以为空，返回到$val$中。
				localtogmt,$val$,$timeval$
					由$timeval$指定的本地时间转换为$val$的GMT时间。
				gmttolocal,$val$,$timeval$
					由$timeval$指定的GMT时间转换为$val$的本地时间。
				localtogmtstr,$val$,$timeval$,%y-%m-%d
					由$timeval$指定的本地时间字符串转换为$val$的GMT时间字符串，输入输出字符串格式由%y-%m-%d指定。
				gmttolocalstr,$val$,$timeval$
					由$timeval$指定的GMT时间字符串转换为$val$的本地时间字符串，输入输出字符串格式由%y-%m-%d指定。
		end[:3]
			运行函数完成。后跟参数作为返回值返回到调用函数者。
		sqlite:subcmd[,value1[,value2]]
			Sqlite数据库处理函数，功能由subcmd指定，如果subcmd解释错误，返回错误Error_SubOper_NotRecognise。
			subcmd命令如下：
				open,$val$
					打开数据库，如果打开出错，引发Error_Sqlite_OpenDataBaseError
				openex,$val$,init.sql
					打开数据库，如果打开出错，引发Error_Sqlite_OpenDataBaseError，如果尚没有数据库文件，先调用建表的脚本init.sql，数据库脚本文件必须以ascii编码
				sql,insert into testtable(id\qi) values(1\q2)
					执行sql语句，sql语句中的逗号以\q替换，如果执行出错，引发Error_Sqlite_ExecSqlError
				getret,$var1$,select id from testable where id=1
					执行sql语句并获取第一条记录的第一列数值，sql语句中的逗号以\q替换，如果执行出错，返回值为空
				beginquery,select id from testable
					开始请求数据，如果执行出错，引发Error_Sqlite_BeginQueryError
				ifhavenext,readtag,endtag
					如果继续有数据，转到readtag，否则转到endtag
				getval,$var1$,0
					获取指定列的数据，指定列区域可进行四则运算
				getvalbykey,$var1$,id
					获取指定列的数据，指定列由字符串给出
				getcolcount,$var1$
					获取栏目列数
				getcolname,$var1$,id
					获取栏目名字
				endquery
					完成请求数据
				close
					关闭数据库
				export,select * from a,$filename$,\q,\r\n,1
					导出数据结果，执行Sql语句，把数据保存到文件中，数据间以\q分隔，项目间以\r\n分隔，1为需要标题，如果执行出错，引发Error_Sqlite_ExportError
				exporttable,$var$,select * from a,1,$lcnt$,$icnt$,ls,le,is,ie,tis,tie,head,end,emptyvalue
					把数据结果生成表格，执行Sql语句，把数据返回到字符串var中，1为需要标题（该值及以后的参数都可忽略），
					每行前，后，每项前，后，标题项前，后分别为ls,le,is,ie,tis,tie，在最开始加入head内容，最后加入end内容，当数据为空时，以emptyvalue代替，如果执行出错，引发Error_Sqlite_ExportError
		msgbox:$ret$,content,title,type
			弹出对话框，内容为content，标题为title，显示方式由数值type指定，返回值保存到$ret$中。
		include:subcmd[,value1[,value2]]
			解释器支持跨文件调用，跨文件调用使用索引数字区分不同文件，装载脚本文件时，会对脚本文件进行初始化。
			跨文件函数调用方法请参见function命令。
			如果调用的索引不存在，引发Error_Include_NoExistIndex
			subcmd命令如下：
				init,number,Script1.txt
					对Script1.txt进行初始化，并映射到数字number处，数字支持四则运算，加载脚本文件后，解释器会在最后自动进行释放，当然也可以手动进行释放。当该数字原来有映射的脚本时，原来的脚本被自动释放。
				free,number
					对已加载脚本进行释放。
				getlocal,number,$ret$,l_retstr
					获取子脚本的local变量l_retstr,名字l_retstr支持变量处理
				savelocal,number,l_retstr,value
					保存子脚本的local变量l_retstr值为value
				getglobal,number,$ret$,g_tmpstr
					获取子脚本的全局变量g_tmpstr,名字g_tmpstr支持变量处理
				saveglobal,number,g_tmpstr,value
					保存子脚本的全局变量g_tmpstr值为value
		timer:subcmd[,value1[,value2]]
			定时器设置，设置定时器后，经过定时时间后，引擎会调用定时器指定的函数进行调用，其中参数一为定时器id，参数二为传入的变量内容。
			subcmd命令如下：
				init,$timerid$,100,RetCall,1,Ret Value
					新建一个定时器，定时器号返回到timerid中，如果返回0表示失败，引发Error_InitTimer_Error错误；100为时间间隔，单位毫秒，支持四则运算；RetCall为时间到时调用的脚本函数名；1为是否循环定时，支持四则运算；Ret Value为传入的参数，在调用到时响应函数时给出；
				free,$timerid$
					释放一个定时器，timerid为定时器号，支持四则运算。
				reset,$timerid$,$timeval$
					重置定时器参数，调整当前定时时间，timerid为定时器号，支持四则运算，如果为0，表示清零到初始值重新计数；如果为正值，表示延长多少定时时间；负值表示减少多少定时时间，支持四则运算。
		schedule:subcmd
			计划任务，设置任务后，到指定定时时间，引擎会调用指定的回调函数进行调用，其中参数一为计划任务id，参数二为传入的变量内容。
			subcmd命令如下：
				new,$schid$,2007-01-01 18:00:00,H,60000,1,RetCall,Ret Value
					新建一个计划任务，任务号返回到$schid$中，时间为任务响应时间，h为以小时作响应时间匹配（另还有md w HMS），60000为检查间隔时间，注意该值太大会可能引起任务没响应；1为是否循环
					RetCall任务响应回调函数；Ret Value为传入的参数，在调用到时响应函数时给出；
				free,$schid$
					取消定时器
		file:subcmd
			文件处理函数。
			subcmd命令如下：
				copyfile,srcfile,dictfile,1
					复制文件，1为存在则失败
				movefile,srcfile,dictfile
					移动文件
				isfileexist,$ret$,filepath
					判断文件是否存在
		syscall:fn,type,ret,arg,arg2,arg3...
			系统API调用，如果调用出错，引发Error_SysCall_Failed错误
			fn为调用函数名，如果指定dll使用!，如：GDI32.DLL!Bitblt，对于常用Api，可不指定Dll，详细请参阅THSysCallRapper中实现
			type为返回结果形式，默认为0：
				0为不需要返回。
				1为返回整形数。
				2为返回非负整形数，如指针。
				3为返回字符串。
			ret为用于返回保存的变量，如果type指定为0时，该参数没有
			arg..调用API时的参数列表，注意如果参数为数字，则会转换为数字进行API调用
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
	static const int THScriptEngineVersion=180;	///>解释器版本
	enum enErrCode{
		Error_None,								///>没有错误
		Error_Cmd_NotRecognise,					///>无法识别的命令
		Error_Function_Return_NotAvailable,		///>函数返回结果无法处理
		Error_Cmd_ParameterNotFound,			///>参数不齐
		Error_Cmd_CanNotParseSepSym,			///>不能处理特殊字符'\\'
		Error_Function_UnknownHowToContinue,	///>没有继续处理的指令，解释器不知道怎样继续
		Error_Function_TagNotFound,				///>没有找到对应的标号
		Error_LogicCalc_Error,					///>四则运算错误
		Error_Oper_NotRecognise,				///>操作符无法辨认
		Error_SubOper_NotRecognise,				///>命令的子操作无法辨认
		Error_Sqlite_ExecSqlError,				///>Sqlite执行Sql语句出错
		Error_Sqlite_OpenDataBaseError,			///>Sqlite执行打开数据库出错
		Error_Sqlite_BeginQueryError,			///>Sqlite执行请求查询出错
		Error_Sqlite_ExportError,				///>Sqlite导出数据错误
		Error_Include_NoExistIndex,				///>调用了一个无效的外部脚本索引
		Error_InitTimer_Error,					///>定时器启动错误
		Error_SysCall_Failed					///>系统调用错误
	};
	THScript()
	{
		m_cb=NULL;
		m_adddata=NULL;
		m_extcls=NULL;
		m_local.SetFreeProc(FreeMap);
		m_include.SetFreeProc(FreeIncMap);
		m_TimerMap.SetFreeProc(FreeTimerMap,this);
		//默认采用普通文件系统
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
			case Error_None:return _T("没有错误");
			case Error_Cmd_NotRecognise:return _T("无法识别的命令");
			case Error_Function_Return_NotAvailable:return _T("函数返回结果无法处理");
			case Error_Cmd_ParameterNotFound:return _T("参数不齐");
			case Error_Cmd_CanNotParseSepSym:return _T("不能处理特殊字符'\\'");
			case Error_Function_UnknownHowToContinue:return _T("没有继续处理的指令，解释器不知道怎样继续");
			case Error_Function_TagNotFound:return _T("没有找到对应的标号");
			case Error_LogicCalc_Error:return _T("四则运算错误");
			case Error_Oper_NotRecognise:return _T("操作符无法辨认");
			case Error_SubOper_NotRecognise:return _T("命令的子操作无法辨认");
			case Error_Sqlite_ExecSqlError:return _T("Sqlite执行Sql语句出错");
			case Error_Sqlite_OpenDataBaseError:return _T("Sqlite执行打开数据库出错");
			case Error_Sqlite_BeginQueryError:return _T("Sqlite执行请求查询出错");
			case Error_Sqlite_ExportError:return _T("Sqlite导出数据错误");
			case Error_Include_NoExistIndex:return _T("调用了一个无效的外部脚本索引");
			case Error_InitTimer_Error:return _T("定时器启动错误");
			case Error_SysCall_Failed:return _T("系统调用错误");
		}
		return _T("未知错误");
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
	* @brief 初始化路径
	* @param sPath		脚本文件路径
	* @param pStore		指定使用的文件系统
	* @param bAutoFree	是否自动释放文件系统指针
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
		THDebug(_T("THScript:脚本初始化,文件:%s,文件系统:%s"),sPath,m_pStore->GetStoreName());
#endif
		m_ini.InitStore(m_pStore,bAutoFree,sPath);
		THStringArray ar,arval;
		if (m_ini.EnumStruct(_T("GlobalVal"),&ar,&arval))
		{
			//init the global val
			for(int i=0;i<ar.GetSize();i++)
				SetGlobalSettings(ar[i],ParseVar(arval[i],NULL,NULL));//使用ParseVar处理转义符
		}
		RunScript(_T("InitCall"));
	}

	/**
	* @brief 返回脚本文件的路径
	* @return 脚本文件路径
	*/
	THString GetScriptFilePath(){return m_ini.GetIniPath();}

	/**
	* @brief 运行脚本
	* @param fnName			脚本函数名称，为空则为调用函数ScriptMain
	* @param localidx		提供的用户变量索引
	* @param tempsettings	用于传递函数参数，参数名为arg1-arg20
	* @return 返回运行结果
	*/
	THString RunScript(THString fnName,int localidx=0,THString tempsettings=_T(""))
	{
#ifdef THSCRIPT_DEBUG
		THDebug(_T("THScript:运行函数:%s,函数参数:%s"),fnName,tempsettings);
#endif
		//setup local settings
		m_localsettinglock.Lock();
		THString *localsettings=GetLocalSettings(localidx);
#ifdef THSCRIPT_DEBUG
		THDebug(_T("THScript:用户环境索引:%d,环境参数:%s"),localidx,(localsettings)?(*localsettings):_T(""));
#endif
		m_localsettinglock.Unlock();
		THString ret=RunFunction(fnName,localidx,tempsettings,0);
#ifdef THSCRIPT_DEBUG
		THDebug(_T("THScript:函数:%s 返回结果:%s"),fnName,ret);
#endif
		return ret;
	}

	void EmptyLocalSettings(int localidx)
	{
#ifdef THSCRIPT_DEBUG
		THDebug(_T("THScript:清空用户环境,索引:%d"),localidx);
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
		THDebug(_T("THScript:读取用户环境,索引:%d,键名:%s,键值:%s"),localidx,key,ret);
#endif
		return ret;
	}

	void SetLocalSettings(int localidx,THString key,THString value)
	{
		//localsetting parse
		if (key.Left(2)!=_T("l_") && key!=_T("s_LastError")) return;
#ifdef THSCRIPT_DEBUG
		THDebug(_T("THScript:保存用户环境,索引:%d,键名:%s,键值:%s"),localidx,key,value);
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
		THDebug(_T("THScript:清空全局环境"));
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
		THDebug(_T("THScript:读取全局环境,键名:%s,键值:%s"),key,ret);
#endif
		return ret;
	}

	void SetGlobalSettings(THString key,THString value)
	{
		if (key.Left(2)!=_T("g_")) return;
#ifdef THSCRIPT_DEBUG
		THDebug(_T("THScript:  设置全局变量,键名:%s,键值:%s"),key,value);
#endif
		THSimpleXml::SetParam(&m_global,key,value);
	}

	void ResetGlobalSettings()
	{
#ifdef THSCRIPT_DEBUG
		THDebug(_T("THScript:重读全局环境"));
#endif
		EmptyGlobalSettings();
		THStringArray ar,arval;
		if (m_ini.EnumStruct(_T("GlobalVal"),&ar,&arval))
		{
			//init the global val
			for(int i=0;i<ar.GetSize();i++)
				SetGlobalSettings(ar[i],ParseVar(arval[i],NULL,NULL));//使用ParseVar处理转义符
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
	* @brief 开始定时器
	* @param localidx		提供的用户变量索引
	* @param timeval		定时器定时间隔
	* @param fnName			脚本函数名称
	* @param retval			返回调用的函数
	* @param bLoop			是否循环调用
	* @return 返回定时器号,返回0表示失败
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
		THDebug(_T("THScript:设置定时器,用户变量索引:%d,定时器间隔:%d,回调函数:%s,回调变量:%s,是否重复:%d"),localidx,timeval,fnName,retval,bLoop);
#endif
		return TimerId;
	}

	/**
	* @brief 取消定时器
	* @param timerid		定时器号
	*/
	void UnSetTimer(int timerid)
	{
#ifdef THSCRIPT_DEBUG
		THDebug(_T("THScript:取消定时器:%d"),timerid);
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
	* @brief 设置使用外部函数处理外部命令
	* @param cb				处理函数
	* @param adddata		附加参数
	*/
	void SetParseExtCmdProc(THParseExtCmdProc cb,const void *adddata){m_cb=cb;m_adddata=adddata;}

	/**
	* @brief 设置使用外部类处理外部命令
	* @param cls			外部类指针
	*/
	void SetParseExtCmdClass(THScript *cls){m_extcls=cls;}

	/**
	* @brief 保存处理的变量
	* @param var			保存的值名
	* @param value			保存内容
	* @param tempsetting	临时变量指针
	* @param localidx		本地变量索引
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
				//说明需要处理嵌套变量
				THString subvarname=var.Left(pos2).Mid(pos+1);
				THString change=GetVar(subvarname,tempsetting,localidx);
				var.Replace(_T("[")+subvarname+_T("]"),change);
				//value=GetVar(var,tempsetting,localidx);
			}
			else
				break;
		}
#ifdef THSCRIPT_DEBUG
		THDebug(_T("THScript:  保存变量 %s[%s]=%s"),myvar,var,value);
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
	* @brief 处理变量
	* @param var			处理内容
	* @param tempsetting	临时变量指针
	* @param localidx		本地变量索引
	* @return 内容处理结果
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
			//排除是double$$出现，$$表示是真实$号
			while(pos!=-1 && var.GetLength()>pos+1 && var.GetAt(pos+1)=='$')
			{
				pos=var.Find(_T("$"),pos+2);
			}
			if (pos!=-1)
			{
				pos2=var.Find(_T("$"),pos+1);
				//排除是double$$出现，$$表示是真实$号
				//pos2不应该判断，因为有$var$$var2$这种情况
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
					//说明需要处理嵌套变量
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
		if(myvar!=var) THDebug(_T("THScript:  读取变量 %s=%s"),myvar,var);
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
	* @brief 内部运行脚本
	* @param fnName			脚本函数名称
	* @param localidx		提供的用户变量索引
	* @param tempsetting	临时变量,因仅在函数内部使用,这里采用值传递方式传值,用于传入调用函数的参数
	* @return 返回运行结果
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
		THDebug(_T("THScript:运行脚本:[%s] start=%s"),fnName,script);
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
						THDebug(_T("THScript:遇错误跳转到:[%s] %s=%s"),fnName,errtag,script);
#endif
					}
				}
				else
				{
					if (m_pDebugger) m_pDebugger->OnEnterTag(fnName,tag,&script);
#ifdef THSCRIPT_DEBUG
					THDebug(_T("THScript:跳转到:[%s] %s=%s"),fnName,tag,script);
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
	* @brief 内部处理脚本
	* @param script			当前要运行的脚本
	* @param localidx		提供的用户变量索引
	* @param tempsetting	临时变量
	* @param
	* @return 返回运行结果，返回tag:tagName,表示定位到tag,返回ret:retValue,表示函数返回的结果
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
				THDebug(_T("THScript:执行脚本:%s"),cmd);
#endif
			}
			else
			{
				cmd=script.Left(pos);
				THString tmp=script.Mid(pos+1);
				tokenvar.Init(tmp,_T(","));
#ifdef THSCRIPT_DEBUG
				THDebug(_T("THScript:执行脚本:%s,参数:%s"),cmd,tmp);
#endif
			}
#define ISCMD(a) cmd.Compare(_T(#a))==0
			if (ISCMD(goto))
			{
				//goto:tag//goto:tag~err//转到tag，如果tag不存在，转到err
				if (!tokenvar.IsMoreTokens())
					OnScriptError(Error_Cmd_ParameterNotFound,cmd)
				else
				{
					THString ret=_T("tag:")+ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
					//continue指令表示按顺序继续执行下一条
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
					THDebug(_T("THScript:读取配置文件[%s] %s=%s,默认值:%s,文件:%s"),tmpvar,tmpvar2,ret,defval,file);
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
					THDebug(_T("THScript:写入配置文件[%s] %s=%s,文件:%s"),var1,var2,var3,file);
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
							THDebug(_T("THScript:跳转判断(%s=%.3f) %s (%s=%.3f) = %s,跳转:%s"),var,ret,sym,var2,ret2,bRet?_T("真"):_T("假"),tagret);
#endif
							tagret=ParseVar(tagret,tempsetting,localidx);
							//continue指令表示按顺序继续执行下一条
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
						THDebug(_T("THScript:字符串i2s %s->%s"),orgval,valvalue);
#endif
					}
					else if (oper==_T("lower"))
					{
						valvalue.MakeLower();
#ifdef THSCRIPT_DEBUG
						THDebug(_T("THScript:转小写->%s"),valvalue);
#endif
					}
					else if (oper==_T("upper"))
					{
						valvalue.MakeUpper();
#ifdef THSCRIPT_DEBUG
						THDebug(_T("THScript:转大写->%s"),valvalue);
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
						THDebug(_T("THScript:字符串替换 把 %s 中的 %s 替换为 %s 结果:%s"),orgval,tmpvar,orgrep,valvalue);
#endif
					}
					else if (oper==_T("append"))
					{
#ifdef THSCRIPT_DEBUG
						THString orgval=valvalue;
#endif
						valvalue+=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
#ifdef THSCRIPT_DEBUG
						THDebug(_T("THScript:字符串追加%s->%s"),orgval,valvalue);
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
						THDebug(_T("THScript:字符串裁切%s->%s,裁切字符%s"),orgval,valvalue,trimvar);
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
						THDebug(_T("THScript:字符串%s比较,结果:%s"),oper,bSame?_T("相同"):_T("不相同"));
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
							THDebug(_T("THScript:字符串取左(%s = %.3f)个字符:%s->%s"),tmpcalc,ret2,tmp,valvalue);
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
							THDebug(_T("THScript:字符串取右(%s = %.3f)个字符:%s->%s"),tmpcalc,ret2,tmp,valvalue);
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
							THDebug(_T("THScript:字符串查找,在字符串 %s 中查找 %s,在位置 (%s = %.3f) 开始查找,找到在位置:%s"),tmp,tmpstr,tmpcalc,ret2,valvalue);
#endif
						}
					}
					else if (oper==_T("len"))
					{
						THString tmpstr=ParseVar(tokenvar.GetNextToken(),tempsetting,localidx);
						valvalue=THStringConv::i2s(tmpstr.GetLength());
#ifdef THSCRIPT_DEBUG
						THDebug(_T("THScript:字符串长度计算,字符串 %s,长度:%s"),tmpstr,valvalue);
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
						THDebug(_T("THScript:字符串数字整形,字符串 %s,长度:%s"),tmpstr,valvalue);
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
							THDebug(_T("THScript:设置整数 %d -> %s"),THSysMisc::cuttail(ret),var);
#endif
						}
						else if (ISCMD(setintu))
						{
							SaveVar(var,THStringConv::u2s(THSysMisc::cuttail(ret)),tempsetting,localidx);
#ifdef THSCRIPT_DEBUG
							THDebug(_T("THScript:设置无符号整数 %u -> %s"),THSysMisc::cuttail(ret),var);
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
							THDebug(_T("THScript:设置数值 %.3f -> %s"),ret,var);
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
						THDebug(_T("THScript:外部函数调用,函数:%s,参数:%s,返回:%s->%s,文件:%d,%s"),fnName,fntempsetting,tmp,retval,idx,inc->GetScriptFilePath());
#endif
					}
					else
					{
						tmp=RunFunction(fnName,localidx,fntempsetting,nRecursion+1);
#ifdef THSCRIPT_DEBUG
						THDebug(_T("THScript:函数调用,函数:%s,参数:%s,返回:%s->%s"),fnName,fntempsetting,tmp,retval);
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
							THDebug(_T("THScript:Sql打开数据库:%s"),path);
#endif
							//设置Sqlite的默认重试Timeout,脚本中多个地方同时调用sqlite容易有问题
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
							THDebug(_T("THScript:Sql高级打开数据库:%s,初始建表:%s"),path,bInit?initpath:_T("否"));
#endif
							if (bInit)
							{
								if (initpath.IsEmpty())
									OnScriptError(Error_Cmd_ParameterNotFound,cmd)
								else
									m_sqlite.ExecSqlFromFile(initpath);
							}
							//设置Sqlite的默认重试Timeout,脚本中多个地方同时调用sqlite容易有问题
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
							THDebug(_T("THScript:Sql执行sql语句:%s"),sql);
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
							THDebug(_T("THScript:Sql执行sql语句:%s,返回结果:%s->%s"),sql,ret,savevar);
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
							THDebug(_T("THScript:Sql开始查询事务sql语句:%s"),sql);
#endif
						}
					}
					else if (oper==_T("endquery"))
					{
						m_sqlite.EndQuery();
#ifdef THSCRIPT_DEBUG
						THDebug(_T("THScript:Sql结束查询事务"));
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
						THDebug(_T("THScript:Sql是否还有数据:%s,跳转:%s"),bNext?_T("有"):_T("无"),jumptag);
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
							THDebug(_T("THScript:Sql获取列 (%s = %d) 值:%s->%s"),key,THSysMisc::cuttail(ret),strret,savevar);
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
							THDebug(_T("THScript:Sql获取列 %s 值:%s->%s"),key,strret,savevar);
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
							THDebug(_T("THScript:Sql获取列数:%s->%s"),strret,savevar);
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
								THDebug(_T("THScript:Sql获取第 (%s = %d) 列列名:%s->%s"),key,THSysMisc::cuttail(ret),strret,savevar);
	#endif
								SaveVar(savevar,strret,tempsetting,localidx);
							}
						}
					}
					else if (oper==_T("close"))
					{
						m_sqlite.CloseDataBase();
#ifdef THSCRIPT_DEBUG
						THDebug(_T("THScript:Sql关闭数据库"));
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
						THDebug(_T("THScript:Sql导出数据,sql语句:%s,文件:%s,行分隔符:%s,列分隔符:%s,输出标题:%s"),sql,file,sep1,sep2,bUseTitle?_T("是"):_T("否"));
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
							THDebug(_T("THScript:Sql导出数据表,sql语句:%s,输出标题:%s,行:%s,%s,项目:%s,%s,标题项:%s,%s,头:%s,尾:%s,空值:%s"),sql,bUseTitle?_T("是"):_T("否"),ls,le,is,ie,tis,tie,head,end,emptyvalue);
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
				THDebug(_T("THScript:弹出对话框,标题:%s,内容:%s,形式:%s,返回值:%s->%s"),title,text,mode,ret,saveval);
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
								THDebug(_T("THScript:外部引用脚本初始化:%d,文件:%s"),idx,path);
#endif
								THScript *scrcls=new THScript();
								//传递ext命令解释设置
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
							THDebug(_T("THScript:外部引用脚本释放:%d"),idx);
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
									THDebug(_T("THScript:外部引用脚本变量设置:操作:%s,索引:%d,参数1:%s,参数2:%s"),oper,idx,par1,par2);
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
									THDebug(_T("THScript:定时器初始化:定时器号%d->%s,定时间隔:(%s = %d),回调函数:%s,附加参数:%s,是否循环:(%s = %s)"),timerid,Timerid,timeval,timev,retfn,adddata,loop,bloop?_T("是"):_T("否"));
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
							THDebug(_T("THScript:定时器释放:定时器号(%s = %d)"),Timeridret,timerid);
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
							THDebug(_T("THScript:定时器重置:定时器号:(%s = %d),重置时间:(%s = %d)"),Timeridret,timerid,valret,timerval);
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
							THDebug(_T("THScript:计划任务:新建计划:计划号:%d,时间:%s,模式:%d,定时间隔:%d,循环:%d,回调:%s,参数:%s"),nscid,ba.Format(),nmode,(int)ret,(int)ret1,retcall,retvalue);
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
						THDebug(_T("THScript:计划任务:停止任务::%d"),nscid);
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
						THDebug(_T("THScript:复制文件:结果:%d 存在则失败:%d %s -> %s"),ret,bMust,src,dist);
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
						THDebug(_T("THScript:移动文件:结果:%d %s -> %s"),ret,src,dist);
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
						THDebug(_T("THScript:文件是否存在:结果:%d %s"),ret,src);
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
					THDebug(_T("THScript:系统API调用:结果%d:函数:%s,模式:%d,返回值:%s,参数:%s"),bret,fn,type,ret,arg);
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
				THDebug(_T("THScript:外部解释命令:%s"),cmd);
#endif
				if (!OnParseExtCmd(cmd,&tokenvar,tempsetting,localidx))
					OnScriptError(Error_Cmd_NotRecognise,cmd)
			}
		}
		return _T("");
	}
	//返回是否支持该命令解析
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
	//在ExtCmdProc调用时好可能需要用到
	THLogicCalc m_calc;
	THSqliteDll m_sqlite;
	THStringArray m_scar;
};
