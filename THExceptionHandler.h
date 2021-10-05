#pragma once

#define THSYSTEM_INCLUDE_WINNT
#include <THSystem.h>
#include <THString.h>
#include <THTime.h>
#include <THSysInfo.h>

/**
* @brief 程序出错处理类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-06-20 新建类
*/
/**<pre>
</pre>*/
class THExceptionHandler
{
public:
	THExceptionHandler(){}
	virtual ~THExceptionHandler(){}

	static THString GetExceptionCode(DWORD dwCode)
	{
		switch(dwCode)
		{
			case EXCEPTION_ACCESS_VIOLATION :return _T("EXCEPTION_ACCESS_VIOLATION");
			case EXCEPTION_ARRAY_BOUNDS_EXCEEDED :return _T("EXCEPTION_ARRAY_BOUNDS_EXCEEDED");
			case EXCEPTION_BREAKPOINT :return _T("EXCEPTION_BREAKPOINT");
			case EXCEPTION_DATATYPE_MISALIGNMENT :return _T("EXCEPTION_DATATYPE_MISALIGNMENT");
			case EXCEPTION_FLT_DENORMAL_OPERAND :return _T("EXCEPTION_FLT_DENORMAL_OPERAND");
			case EXCEPTION_FLT_DIVIDE_BY_ZERO :return _T("EXCEPTION_FLT_DIVIDE_BY_ZERO");
			case EXCEPTION_FLT_INEXACT_RESULT :return _T("EXCEPTION_FLT_INEXACT_RESULT");
			case EXCEPTION_FLT_INVALID_OPERATION :return _T("EXCEPTION_FLT_INVALID_OPERATION");
			case EXCEPTION_FLT_OVERFLOW :return _T("EXCEPTION_FLT_OVERFLOW");
			case EXCEPTION_FLT_STACK_CHECK :return _T("EXCEPTION_FLT_STACK_CHECK");
			case EXCEPTION_FLT_UNDERFLOW :return _T("EXCEPTION_FLT_UNDERFLOW");
			case EXCEPTION_ILLEGAL_INSTRUCTION :return _T("EXCEPTION_ILLEGAL_INSTRUCTION");
			case EXCEPTION_IN_PAGE_ERROR :return _T("EXCEPTION_IN_PAGE_ERROR");
			case EXCEPTION_INT_DIVIDE_BY_ZERO :return _T("EXCEPTION_INT_DIVIDE_BY_ZERO");
			case EXCEPTION_INT_OVERFLOW :return _T("EXCEPTION_INT_OVERFLOW");
			case EXCEPTION_INVALID_DISPOSITION :return _T("EXCEPTION_INVALID_DISPOSITION");
			case EXCEPTION_NONCONTINUABLE_EXCEPTION :return _T("EXCEPTION_NONCONTINUABLE_EXCEPTION");
			case EXCEPTION_PRIV_INSTRUCTION :return _T("EXCEPTION_PRIV_INSTRUCTION");
			case EXCEPTION_SINGLE_STEP :return _T("EXCEPTION_SINGLE_STEP");
			case EXCEPTION_STACK_OVERFLOW :return _T("EXCEPTION_STACK_OVERFLOW");
		}
		THString str;
		str.Format(_T("EXCEPTION_UNKNOWN_0x%X"),dwCode);
		return str;
	}

	static THString MakeExceptionReport(PEXCEPTION_POINTERS exp,THString sCurstomStr=_T(""))
	{
		THString ret;
		THTime ti;
		//parse base info
		ret.Format(_T("------------------------------------------------------\r\n"));
		ret+=ti.Format(_T("Date: %Y-%m-%d %H:%M:%S\r\n"));
		ret.AppendFormat(_T("Os:%s\r\n%s\r\n"),THSysInfo::GetOSVersion(),sCurstomStr);
		//parse exception info
		//not parse the exp->ExceptionRecord->ExceptionRecord
		ret.AppendFormat(_T("Exception Type:%s,	Can be Continue:%s\r\nException Address:%u(0x%08X)\r\n"),
			GetExceptionCode(exp->ExceptionRecord->ExceptionCode),
			(exp->ExceptionRecord->ExceptionFlags&EXCEPTION_NONCONTINUABLE)?_T("FALSE"):_T("TRUE"),
			exp->ExceptionRecord->ExceptionAddress,exp->ExceptionRecord->ExceptionAddress);
		for(DWORD i=0;i<exp->ExceptionRecord->NumberParameters;i++)
			ret.AppendFormat(_T("  info[%d]=%u(0x%08X)\r\n"),i,exp->ExceptionRecord->ExceptionInformation[i],exp->ExceptionRecord->ExceptionInformation[i]);
		if (exp->ExceptionRecord->ExceptionCode==EXCEPTION_ACCESS_VIOLATION)
			ret.AppendFormat(_T("Access Violation at %s Address %u(0x%08X)\r\n"),(exp->ExceptionRecord->ExceptionInformation[0]==0)?_T("Reading"):_T("Writeing"),exp->ExceptionRecord->ExceptionInformation[1],exp->ExceptionRecord->ExceptionInformation[1]);
		//parse context info
		ret.AppendFormat(_T("Info: ContextFlags:%08X\r\n"),
			exp->ContextRecord->ContextFlags);
		ret.AppendFormat(_T("Registers: AX = %08X BX = %08X CX = %08X DX = %08X\r\n DI = %08X IP = %08X SP = %08X SI = %08X BP = %08X\r\n"),
			exp->ContextRecord->Eax,
			exp->ContextRecord->Ebx,
			exp->ContextRecord->Ecx,
			exp->ContextRecord->Edx,
		    exp->ContextRecord->Edi,
			exp->ContextRecord->Eip,
			exp->ContextRecord->Esp,
			exp->ContextRecord->Esi,
			exp->ContextRecord->Ebp);
		return ret;
	}

	static void DisableExceptionDialog(){SetErrorMode(SEM_NOGPFAULTERRORBOX);}
};