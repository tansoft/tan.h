#pragma once

#include <THSystem.h>
#include <THString.h>
#include <THTime.h>
#include <THFile.h>
#include <THArith.h>
#include <THSyn.h>

#define PRINTLOG_MODE_TIME		0x1							///<输出格式带时间
#define PRINTLOG_MODE_THREADID	0x2							///<输出格式带线程号
#define PRINTLOG_MODE_RETURN	0x4							///<输出格式末尾带回车
#define PRINTLOG_MODE_SHORTTIME	0x8							///<输出格式带短时间

#ifdef _DEBUG
#define PRINTLOG
#endif

#ifndef PRINTLOG_MODE
#define PRINTLOG_MODE		PRINTLOG_MODE_TIME|PRINTLOG_MODE_THREADID|PRINTLOG_MODE_RETURN
#endif

#ifdef PRINTLOG
	#define THDebug					THDebugLog::DebugLog			///<调试输出
	#define THDebugA				THDebugLog::DebugLogA			///<调试输出
	#define THLongDebug				THDebugLog::DebugLog			///<兼容旧有接口
	#define THOrgDebug				THDebugLog::OrgDebugLog			///<直接输出内容，可支持%等不转义
	#define THOrgDebugA				THDebugLog::OrgDebugLogA		///<直接输出内容，可支持%等不转义
	#define THDebugFile				THDebugLog::DebugLogAndFile		///<带文件记录的调试输出
	#define THDebugFile2			THDebugLog::DebugLogAndFile2	///<带文件记录的调试输出，可指定文件名
	#define THOrgDebugFile			THDebugLog::OrgDebugLogAndFile	///<直接输出内容，可支持%等不转义
	#define THEncDebugFile			THDebugLog::DebugLogAndEncFile	///<带加密文件记录的调试输出
	#define THEncDebugFile2			THDebugLog::DebugLogAndEncFile2	///<带加密文件记录的调试输出，可指定文件名
	#define THOrgEncDebugFile		THDebugLog::OrgDebugLogAndEncFile	///<直接输出内容，可支持%等不转义
	#define THFn					THFnPrint fn(__FUNCTION__)		///<输出函数名
	#define	THLine					THDebug(_T("Line %d in File %s"),__LINE__,__FILE__)	///<输出当前行
	#define THDebugSetupKeyWord		THDebugLog::SetupKeyWord		///<设置过滤关键字
	#define THDebugSetupReceiveCtrl	THDebugLog::SetupReceiveCtrl	///<设置打印输出控件句柄
	#define THDebugSetupPrintStyle	THDebugLog::SetupPrintStyle		///<设置打印格式
	#define THDebugDeleteLogFile	THDebugLog::DeleteLogFile		///<清空输出文件
#else
	#define THDebug
	#define THDebugA
	#define THLongDebug
	#define THOrgDebug
	#define THDebugFile
	#define THDebugFile2
	#define THOrgDebugFile
	#define THEncDebugFile
	#define THEncDebugFile2
	#define THOrgEncDebugFile
	#define THFn
	#define	THLine
	#define THOrgDebug
	#define THDebugSetupKeyWord
	#define THDebugSetupReceiveCtrl
	#define THDebugSetupPrintStyle
#endif

/**
* @brief Debug信息输出类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-05-28 新建类
* @2007-08-16 使用THString类作字符串处理
* @2009-08-31 加入打印过滤器；加入输出句柄，把信息打印到输出控件中；加入运行中改变log输出样式
*/
/**<pre>
  使用Sample：
  //在需要使用的工程中添加 define 值 PRINTLOG 或使用 _DEBUG 模式
  THDebug(_T("Log Detail:%d"),1);
</pre>*/
class THDebugLog
{
public:
	/**
	* @brief 输出Debug信息，可直接输出带%数据
	* @param lpszMsg		参数内容
	*/
	static void OrgDebugLog(const TCHAR* lpszMsg)
	{
		THString ret=_MakeString(lpszMsg,NULL);
		if (!ret.IsEmpty())
			OutputDebug(ret);
	}

	static void OrgDebugLogA(const char* lpszMsg)
	{
		THStringA ret=_MakeStringA(lpszMsg,NULL);
		if (!ret.IsEmpty())
			OutputDebugA(ret);
	}

	/**
	* @brief 输出Debug信息，在vc调试环境下，debuglog一次输出的最大值为36,181左右；如果需要打印带%的信息，请在调用前先把%替换成%%
	* @param lpszMsg		参数内容
	*/
	static void DebugLog(const TCHAR* lpszMsg,...)
	{
		THString buf;
		va_list pArg;
		va_start(pArg,lpszMsg);
		THString ret=_MakeString(lpszMsg,pArg);
		if (!ret.IsEmpty())
			OutputDebug(ret);
		va_end(pArg);
	}

	static void DebugLogA(const char* lpszMsg,...)
	{
		THStringA buf;
		va_list pArg;
		va_start(pArg,lpszMsg);
		THStringA ret=_MakeStringA(lpszMsg,pArg);
		if (!ret.IsEmpty())
			OutputDebugA(ret);
		va_end(pArg);
	}

	/**
	* @brief 输出文件Debug信息；可直接输出带%数据
	* @param lpszMsg		参数内容
	*/
	static void OrgDebugLogAndFile(const TCHAR* lpszMsg)
	{
		_DebugLogAndFile2(NULL,lpszMsg,NULL);
	}

	/**
	* @brief 输出文件Debug信息；如果需要打印带%的信息，请在调用前先把%替换成%%
	* @param lpszMsg		参数内容
	*/
	static void DebugLogAndFile(const TCHAR* lpszMsg,...)
	{
		va_list pArg;
		va_start(pArg,lpszMsg);
		_DebugLogAndFile2(NULL,lpszMsg,pArg);
		va_end(pArg);
	}

	/**
	* @brief 输出加密的文件Debug信息；可直接输出带%数据
	* @param lpszMsg		参数内容
	*/
	static void OrgDebugLogAndEncFile(const TCHAR* lpszMsg)
	{
		_DebugLogAndEncFile2(NULL,lpszMsg,NULL);
	}

	/**
	* @brief 输出加密的文件Debug信息；如果需要打印带%的信息，请在调用前先把%替换成%%
	* @param lpszMsg		参数内容
	*/
	static void DebugLogAndEncFile(const TCHAR* lpszMsg,...)
	{
		va_list pArg;
		va_start(pArg,lpszMsg);
		_DebugLogAndEncFile2(NULL,lpszMsg,pArg);
		va_end(pArg);
	}

	/**
	* @brief 输出文件Debug信息，并可指定输出的文件名；如果需要打印带%的信息，请在调用前先把%替换成%%
	* @param lpszFile		指定输出的文件名
	* @param lpszMsg		参数内容
	*/
	static void DebugLogAndFile2(const TCHAR* lpszFile,const TCHAR* lpszMsg,...)
	{
		va_list pArg;
		va_start(pArg,lpszMsg);
		_DebugLogAndFile2(lpszFile,lpszMsg,pArg);
		va_end(pArg);
	}

	/**
	* @brief 输出加密的文件Debug信息，并可指定输出的文件名；如果需要打印带%的信息，请在调用前先把%替换成%%
	* @param lpszFile		指定输出的文件名
	* @param lpszMsg		参数内容
	*/
	static void DebugLogAndEncFile2(const TCHAR* lpszFile,const TCHAR* lpszMsg,...)
	{
		va_list pArg;
		va_start(pArg,lpszMsg);
		_DebugLogAndEncFile2(lpszFile,lpszMsg,pArg);
		va_end(pArg);
	}

	static void DecodeLogFile(THString filepath)
	{
		THAes aes;
		THString out=THSysMisc::RenameFileExt(filepath,_T(".txt"));
		char *buf;
		char orgbuf[4096];
		THFile fi,fo;
		if (fi.Open(filepath,THFile::modeRead))
		{
			if (fo.Open(out,THFile::modeCreate|THFile::modeWrite))
			{
				int size;
				while(fi.Read(&size,sizeof(int))==sizeof(int))
				{
					if (size<4096)
						buf=orgbuf;
					else
						buf=new char[size];
					if (!buf) break;
					if (fi.Read(buf,size)==(UINT)size)
					{
						void *tmp=aes.DecryptBuffer(buf,size,&size,GetPassString());
						if (tmp)
						{
							fo.Write(tmp,size);
							aes.Free(tmp);
						}
					}
					if (buf!=orgbuf) delete [] buf;
				}
				fo.Close();
			}
			fi.Close();
		}
	}

	static void SetupKeyWord(THString include=_T(""),THString exclude=_T(""))
	{
		GetIncludeKey()=include;
		GetExcludeKey()=exclude;
		GetIncludeKeyA()=THCharset::t2astr(include);
		GetExcludeKeyA()=THCharset::t2astr(exclude);
	}

	static void SetupReceiveCtrl(HWND hWnd,UINT LimitTextLine=0)
	{
		GetHwnd()=hWnd;
		GetHwndLimitLine()=LimitTextLine;
	}

	static void SetupPrintStyle(int style=PRINTLOG_MODE_TIME|PRINTLOG_MODE_THREADID|PRINTLOG_MODE_RETURN)
	{
		GetStyle()=style;
	}

	static void DeleteLogFile()
	{
		::DeleteFile(GetLogFileName());
	}
protected:
	static void OutputDebug(THString &str)
	{
		OutputDebugString(str);
		HWND hWnd=GetHwnd();
		if (hWnd)
		{
			if (IsWindow(hWnd))
			{
				THString orgtext;
				int len=GetWindowTextLength(hWnd);
				if (len>0)
				{
					GetWindowText(hWnd,orgtext.GetBuffer(len+1),len+1);
					orgtext.ReleaseBuffer();
				}
				UINT limit=GetHwndLimitLine();
				if (limit!=0)
					orgtext=THStringConv::LimitStringByTokenInv(orgtext,_T("\r\n"),limit);
				orgtext+=str;
				SetWindowText(hWnd,orgtext);
			}
			else
				GetHwnd()=NULL;
		}
	}

	static void OutputDebugA(THStringA &str)
	{
		OutputDebugStringA(str);
		HWND hWnd=GetHwnd();
		if (hWnd)
		{
			if (IsWindow(hWnd))
			{
				THStringA orgtext;
				int len=GetWindowTextLength(hWnd);
				if (len>0)
				{
					GetWindowTextA(hWnd,orgtext.GetBuffer(len+1),len+1);
					orgtext.ReleaseBuffer();
				}
				UINT limit=GetHwndLimitLine();
				if (limit!=0)
					orgtext=THStringConv::LimitStringByTokenInvA(orgtext,"\r\n",limit);
				orgtext+=str;
				SetWindowTextA(hWnd,orgtext);
			}
			else
				GetHwnd()=NULL;
		}
	}

	static THString GetPassString()
	{
		static THString sPass;
		if (sPass.IsEmpty())
		{
			unsigned char buf[15];
			buf[0]=(unsigned char)0xdc;
			buf[1]=(unsigned char)0xd7;
			buf[2]=(unsigned char)0xae;
			buf[3]=(unsigned char)0xd6;
			buf[4]=(unsigned char)0xcd;
			buf[5]=(unsigned char)0xbe;
			buf[6]=(unsigned char)0xc7;
			buf[7]=(unsigned char)0xca;
			buf[8]=(unsigned char)0xd2;
			buf[9]=(unsigned char)0xc2;
			buf[10]=(unsigned char)0xeb;
			buf[11]=(unsigned char)0xc2;
			buf[12]=0;
			sPass=THCharset::a2t((char *)buf);
		}
		return sPass;
	}
	inline static THString GetLogFileName()
	{
		static THString sFilePath;
		if (sFilePath.IsEmpty())
		{
			sFilePath=THSysMisc::GetModuleFileName();
			sFilePath=THSysMisc::RenameFileExt(sFilePath,_T(".log"));
		}
		return sFilePath;
	}

	inline static int& GetStyle()
	{
		static int style=PRINTLOG_MODE;
		return style;
	}
	inline static UINT& GetHwndLimitLine()
	{
		static UINT li=0;
		return li;
	}
	inline static HWND& GetHwnd()
	{
		static HWND hWnd=NULL;
		return hWnd;
	}
	inline static THString& GetIncludeKey()
	{
		static THString str;
		return str;
	}

	inline static THString& GetExcludeKey()
	{
		static THString str;
		return str;
	}

	inline static THStringA& GetIncludeKeyA()
	{
		static THStringA str;
		return str;
	}

	inline static THStringA& GetExcludeKeyA()
	{
		static THStringA str;
		return str;
	}

	inline static BOOL IsEnableDebugLog(THString &log)
	{
		THString in=GetIncludeKey();
		if (!in.IsEmpty() && log.Find(in)==-1) return FALSE;
		THString ex=GetExcludeKey();
		if (!ex.IsEmpty() && log.Find(ex)!=-1) return FALSE;
		return TRUE;
	}

	inline static BOOL IsEnableDebugLogA(THStringA &log)
	{
		THStringA in=GetIncludeKeyA();
		if (!in.IsEmpty() && log.Find(in)==-1) return FALSE;
		THStringA ex=GetExcludeKeyA();
		if (!ex.IsEmpty() && log.Find(ex)!=-1) return FALSE;
		return TRUE;
	}

	inline static THString _MakeString(const TCHAR *lpszMsg,va_list pArg)
	{
		THString buf;
		int style=GetStyle();
		if (style&PRINTLOG_MODE_TIME)
		{
			THTime ti;
			buf.Format(_T("[%s.%03d]"),ti.Format(_T("%m-%d %H:%M:%S")),ti.GetMilliseconds());
		}
		if (style&PRINTLOG_MODE_SHORTTIME)
		{
			THTime ti;
			buf.Format(_T("[%s]"),ti.Format(_T("%H:%M:%S")));
		}
		if (style&PRINTLOG_MODE_THREADID)
			buf.AppendFormat(_T("[%d]->"),GetCurrentThreadId());
		if (pArg==NULL)
			buf+=lpszMsg;
		else
			buf.AppendFormatV(lpszMsg,pArg);
		if (style&PRINTLOG_MODE_RETURN)
			buf.AppendFormat(_T("\r\n"));
		if (!IsEnableDebugLog(buf))
			buf.Empty();
		return buf;
	}

	inline static THStringA _MakeStringA(const char *lpszMsg,va_list pArg)
	{
		THStringA buf;
		int style=GetStyle();
		if (style&PRINTLOG_MODE_TIME)
		{
			THTime ti;
			buf.Format("[%s.%03d]",ti.FormatA("%m-%d %H:%M:%S"),ti.GetMilliseconds());
		}
		if (style&PRINTLOG_MODE_SHORTTIME)
		{
			THTime ti;
			buf.Format("[%s]",ti.FormatA("%H:%M:%S"));
		}
		if (style&PRINTLOG_MODE_THREADID)
			buf.AppendFormat("[%d]->",GetCurrentThreadId());
		if (pArg==NULL)
			buf+=lpszMsg;
		else
			buf.AppendFormatV(lpszMsg,pArg);
		if (style&PRINTLOG_MODE_RETURN)
			buf.AppendFormat("\r\n");
		if (!IsEnableDebugLogA(buf))
			buf.Empty();
		return buf;
	}

	/**
	* @brief 输出文件Debug信息
	* @param lpszMsg		参数内容
	*/
	inline static void _DebugLogAndFile2(const TCHAR *lpszFile,const TCHAR* lpszMsg,va_list pArg)
	{
		THString str;
		str=_MakeString(lpszMsg,pArg);
		if (!str.IsEmpty())
		{
			OutputDebug(str);
			THFile file;
			if (file.Open(lpszFile?lpszFile:GetLogFileName(),THFile::modeWrite|THFile::modeNoTruncate|THFile::modeCreate))
			{
				file.SeekToEnd();
				file.Write((LPCTSTR)str,str.GetLength()*sizeof(TCHAR));
				file.Close();
			}
		}
	}

	/**
	* @brief 输出加密的文件Debug信息
	* @param lpszMsg		参数内容
	*/
	inline static void _DebugLogAndEncFile2(const TCHAR *lpszFile,const TCHAR* lpszMsg,va_list pArg)
	{
		static THMutex g_mutex;
		static THAes g_aes;
		THSingleLock lock(&g_mutex);
		THString str;
		str=_MakeString(lpszMsg,pArg);
		if (!str.IsEmpty())
		{
			//Encode output,not need to print
			//OutputDebug(str);
			int out;
			void *buf=g_aes.EncryptBuffer((LPCTSTR)str,str.GetLength()*sizeof(TCHAR),&out,GetPassString());
			if (buf)
			{
				THFile file;
				if (file.Open(lpszFile?lpszFile:GetLogFileName(),THFile::modeWrite|THFile::modeNoTruncate|THFile::modeCreate))
				{
					file.SeekToEnd();
					file.Write(&out,sizeof(int));
					file.Write(buf,out);
					file.Close();
				}
				g_aes.Free(buf);
			}
		}
	}
};

class THFnPrint
{
public:
	THFnPrint(THString fnname)
	{
		m_fnname=fnname;
		THDebug(_T("function %s start"),m_fnname);
	}
	virtual ~THFnPrint()
	{
		THDebug(_T("function %s exit"),m_fnname);
	}
protected:
	THString m_fnname;
};