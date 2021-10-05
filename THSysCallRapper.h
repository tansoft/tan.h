#pragma once

#include <THIni.h>
#include <THScript.h>

typedef void (WINAPI *THSysCallVoidFunction)();
typedef void (WINAPI *THSysCallVoidFunction1)(unsigned int);
typedef void (WINAPI *THSysCallVoidFunction2)(unsigned int,unsigned int);
typedef void (WINAPI *THSysCallVoidFunction3)(unsigned int,unsigned int,unsigned int);
typedef void (WINAPI *THSysCallVoidFunction4)(unsigned int,unsigned int,unsigned int,unsigned int);
typedef void (WINAPI *THSysCallVoidFunction5)(unsigned int,unsigned int,unsigned int,unsigned int,unsigned int);
typedef void (WINAPI *THSysCallVoidFunction6)(unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int);
typedef void (WINAPI *THSysCallVoidFunction7)(unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int);
typedef void (WINAPI *THSysCallVoidFunction8)(unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int);
typedef void (WINAPI *THSysCallVoidFunction9)(unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int);
typedef void (WINAPI *THSysCallVoidFunction10)(unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int);
typedef void (WINAPI *THSysCallVoidFunction11)(unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int);
typedef void (WINAPI *THSysCallVoidFunction12)(unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int);
typedef void (WINAPI *THSysCallVoidFunction13)(unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int);
typedef void (WINAPI *THSysCallVoidFunction14)(unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int);
typedef void (WINAPI *THSysCallVoidFunction15)(unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int);
typedef void (WINAPI *THSysCallVoidFunction16)(unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int);
typedef void (WINAPI *THSysCallVoidFunction17)(unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int);
typedef void (WINAPI *THSysCallVoidFunction18)(unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int);
typedef void (WINAPI *THSysCallVoidFunction19)(unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int);
typedef void (WINAPI *THSysCallVoidFunction20)(unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int);

typedef int (WINAPI *THSysCallRetPointFunction)();
typedef int (WINAPI *THSysCallRetPointFunction1)(unsigned int);
typedef int (WINAPI *THSysCallRetPointFunction2)(unsigned int,unsigned int);
typedef int (WINAPI *THSysCallRetPointFunction3)(unsigned int,unsigned int,unsigned int);
typedef int (WINAPI *THSysCallRetPointFunction4)(unsigned int,unsigned int,unsigned int,unsigned int);
typedef int (WINAPI *THSysCallRetPointFunction5)(unsigned int,unsigned int,unsigned int,unsigned int,unsigned int);
typedef int (WINAPI *THSysCallRetPointFunction6)(unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int);
typedef int (WINAPI *THSysCallRetPointFunction7)(unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int);
typedef int (WINAPI *THSysCallRetPointFunction8)(unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int);
typedef int (WINAPI *THSysCallRetPointFunction9)(unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int);
typedef int (WINAPI *THSysCallRetPointFunction10)(unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int);
typedef int (WINAPI *THSysCallRetPointFunction11)(unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int);
typedef int (WINAPI *THSysCallRetPointFunction12)(unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int);
typedef int (WINAPI *THSysCallRetPointFunction13)(unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int);
typedef int (WINAPI *THSysCallRetPointFunction14)(unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int);
typedef int (WINAPI *THSysCallRetPointFunction15)(unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int);
typedef int (WINAPI *THSysCallRetPointFunction16)(unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int);
typedef int (WINAPI *THSysCallRetPointFunction17)(unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int);
typedef int (WINAPI *THSysCallRetPointFunction18)(unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int);
typedef int (WINAPI *THSysCallRetPointFunction19)(unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int);
typedef int (WINAPI *THSysCallRetPointFunction20)(unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int);

/**
* @brief ϵͳApi���÷�װ��
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2009-09-27 �½���
*/
/**<pre>
�÷���
	ΪһЩ��ű�����ϵͳ�����ṩ�Զ���ӳ��
</pre>*/
class THSysCallRapper
{
protected:
	THSysCallRapper()
	{
		m_bCachedFile=FALSE;
	}

	virtual ~THSysCallRapper()
	{
	}

	void InitCachedFile()
	{
		m_bCachedFile=TRUE;
		m_ini.Init(THSysMisc::GetSystemPath()+_T("\\THSysCallCachedList.txt"));
	}

	HMODULE CachedLoadDll(THString dll)
	{
		dll.MakeLower();
		if (THSysMisc::GetFileExt(dll).IsEmpty())
			dll+=_T(".dll");
		dllmap::iterator dit=m_dllmap.find(dll);
		if (dit==m_dllmap.end())
		{
			HMODULE mod=LoadLibrary(dll);
			if (!mod) return NULL;
			m_dllmap.insert(dllmap::value_type(dll,mod));
			return mod;
		}
		return dit->second;
	}
	void *CachedLoadFn(HMODULE hmod,char *fn)
	{
		if (!hmod) return NULL;
		return GetProcAddress(hmod,fn);
	}
	BOOL m_bCachedFile;
	THIni m_ini;
	typedef map<THString,HMODULE> dllmap;
	dllmap m_dllmap;
	typedef map<THString,void *> fnmap;
	fnmap m_fnmap;
public:
	static THSysCallRapper* GetInstance(BOOL bCachedWithFile=TRUE)
	{
		static THSysCallRapper *g_rapper=NULL;
		if (g_rapper==NULL)
		{
			g_rapper=new THSysCallRapper();
			ASSERT(g_rapper);
			if (bCachedWithFile)
				g_rapper->InitCachedFile();
		}
		return g_rapper;
	}

	/**
	* @brief ��ʼ��·��
	* @param fn			����ϵͳ�������֣������Ҫָ��Dll���ƣ�ʹ��!���֣��磺GDI32.DLL!Bitblt��ע�����ں���ָ���Ǹ���fn�����ֵģ���ͬdll���������ͬ��fn�����ɴ���������Ҫע��
	* @param args		���ò�������arg1-arg20��Ϊ�����б���0��ʼ��ȡ����ȡ����ֵ��Ϊ�����������������ȫΪ���֣���ʾΪ���֣�����Ϊ�ַ���
	* @param rettype	��ʶ����ֵ���ͣ�0Ϊ����Ҫ���أ�1Ϊ������ֵ��2Ϊ����������ֵ����ָ�룩��3Ϊ�����ַ���
	* @param pretstr	���淵��ֵ
	* @return �Ƿ�ɹ�
	*/
	BOOL SysCall(THString fn,THString args,int rettype,THString *pretstr=NULL)
	{
		THString dll;
		if (fn.IsEmpty() || (rettype!=0 && pretstr==NULL)) return FALSE;
		if (fn.Find(_T("!"))!=-1)
		{
			dll=fn.Left(fn.Find(_T("!")));
			fn=fn.Mid(fn.Find(_T("!"))+1);
		}
		void *pfn=NULL;
		fnmap::iterator fit=m_fnmap.find(fn);
		if (fit==m_fnmap.end())
		{
			char *ch=THCharset::t2a(fn);
			if (ch)
			{
				//need to find mapping
				//���������ļ�¼���Ƿ��и�Api��������Dll�ļ�¼
				if (dll.IsEmpty() && m_bCachedFile)
					dll=m_ini.GetString(_T("FnMapping"),fn);
				//�ڳ��õ�Dll�в��Һ���
				if (dll.IsEmpty())
				{
					//�ҵ���¼
					THStringArray ar;
					//base
					ar.Add(_T("kernel32.dll"));
					ar.Add(_T("user32.dll"));
					ar.Add(_T("gdi32.dll"));
					ar.Add(_T("advapi32.dll"));
					ar.Add(_T("shell32.dll"));
					ar.Add(_T("comdlg32.dll"));
					ar.Add(_T("comctl32.dll"));
					ar.Add(_T("psapi.dll"));
					//ole
					ar.Add(_T("shlwapi.dll"));
					ar.Add(_T("oledlg.dll"));
					ar.Add(_T("ole32.dll"));
					ar.Add(_T("oleaut32.dll"));
					//network
					ar.Add(_T("urlmon.dll"));
					ar.Add(_T("ws2_32.dll"));
					ar.Add(_T("winsock.dll"));
					ar.Add(_T("iphlpapi.dll"));
					ar.Add(_T("wininet.dll"));
					for(int i=0;i<ar.GetSize();i++)
					{
						pfn=CachedLoadFn(CachedLoadDll(ar[i]),ch);
						if (pfn)
						{
							if (m_bCachedFile) m_ini.WriteString(_T("FnMapping"),fn,ar[i]);
							m_fnmap.insert(fnmap::value_type(fn,pfn));
							break;
						}
					}
				}
				else
				{
					pfn=CachedLoadFn(CachedLoadDll(dll),ch);
				}
				THCharset::free(ch);
			}
		}
		else
			pfn=fit->second;
		if (!pfn) return FALSE;
		//�������
		THString tmpstr[20];
		unsigned int parg[20];
		int argcnt=0;
		int i=1;
		while(i<=20)
		{
			tmpstr[i-1]=THSimpleXml::GetParam(args,_T("arg")+THi2s(i));
			if (tmpstr[i-1].IsEmpty()) break;
			if (THStringConv::IsStringNumber(tmpstr[i-1]))
				parg[i-1]=THs2u(tmpstr[i-1]);
			else
				parg[i-1]=(UINT)(UINT_PTR)(const TCHAR *)tmpstr[i-1];
			argcnt++;
			i++;
		}
		//��ȡ����ָ��
		//THSysCallVoidFunction
		if (rettype==0)
		{
			switch(argcnt)
			{
				case 0:((THSysCallVoidFunction)(pfn))();break;
				case 1:((THSysCallVoidFunction1)(pfn))(parg[0]);break;
				case 2:((THSysCallVoidFunction2)(pfn))(parg[0],parg[1]);break;
				case 3:((THSysCallVoidFunction3)(pfn))(parg[0],parg[1],parg[2]);break;
				case 4:((THSysCallVoidFunction4)(pfn))(parg[0],parg[1],parg[2],parg[3]);break;
				case 5:((THSysCallVoidFunction5)(pfn))(parg[0],parg[1],parg[2],parg[3],parg[4]);break;
				case 6:((THSysCallVoidFunction6)(pfn))(parg[0],parg[1],parg[2],parg[3],parg[4],parg[5]);break;
				case 7:((THSysCallVoidFunction7)(pfn))(parg[0],parg[1],parg[2],parg[3],parg[4],parg[5],parg[6]);break;
				case 8:((THSysCallVoidFunction8)(pfn))(parg[0],parg[1],parg[2],parg[3],parg[4],parg[5],parg[6],parg[7]);break;
				case 9:((THSysCallVoidFunction9)(pfn))(parg[0],parg[1],parg[2],parg[3],parg[4],parg[5],parg[6],parg[7],parg[8]);break;
				case 10:((THSysCallVoidFunction10)(pfn))(parg[0],parg[1],parg[2],parg[3],parg[4],parg[5],parg[6],parg[7],parg[8],parg[9]);break;
				case 11:((THSysCallVoidFunction11)(pfn))(parg[0],parg[1],parg[2],parg[3],parg[4],parg[5],parg[6],parg[7],parg[8],parg[9],parg[10]);break;
				case 12:((THSysCallVoidFunction12)(pfn))(parg[0],parg[1],parg[2],parg[3],parg[4],parg[5],parg[6],parg[7],parg[8],parg[9],parg[10],parg[11]);break;
				case 13:((THSysCallVoidFunction13)(pfn))(parg[0],parg[1],parg[2],parg[3],parg[4],parg[5],parg[6],parg[7],parg[8],parg[9],parg[10],parg[11],parg[12]);break;
				case 14:((THSysCallVoidFunction14)(pfn))(parg[0],parg[1],parg[2],parg[3],parg[4],parg[5],parg[6],parg[7],parg[8],parg[9],parg[10],parg[11],parg[12],parg[13]);break;
				case 15:((THSysCallVoidFunction15)(pfn))(parg[0],parg[1],parg[2],parg[3],parg[4],parg[5],parg[6],parg[7],parg[8],parg[9],parg[10],parg[11],parg[12],parg[13],parg[14]);break;
				case 16:((THSysCallVoidFunction16)(pfn))(parg[0],parg[1],parg[2],parg[3],parg[4],parg[5],parg[6],parg[7],parg[8],parg[9],parg[10],parg[11],parg[12],parg[13],parg[14],parg[15]);break;
				case 17:((THSysCallVoidFunction17)(pfn))(parg[0],parg[1],parg[2],parg[3],parg[4],parg[5],parg[6],parg[7],parg[8],parg[9],parg[10],parg[11],parg[12],parg[13],parg[14],parg[15],parg[16]);break;
				case 18:((THSysCallVoidFunction18)(pfn))(parg[0],parg[1],parg[2],parg[3],parg[4],parg[5],parg[6],parg[7],parg[8],parg[9],parg[10],parg[11],parg[12],parg[13],parg[14],parg[15],parg[16],parg[17]);break;
				case 19:((THSysCallVoidFunction19)(pfn))(parg[0],parg[1],parg[2],parg[3],parg[4],parg[5],parg[6],parg[7],parg[8],parg[9],parg[10],parg[11],parg[12],parg[13],parg[14],parg[15],parg[16],parg[17],parg[18]);break;
				case 20:((THSysCallVoidFunction20)(pfn))(parg[0],parg[1],parg[2],parg[3],parg[4],parg[5],parg[6],parg[7],parg[8],parg[9],parg[10],parg[11],parg[12],parg[13],parg[14],parg[15],parg[16],parg[17],parg[18],parg[19]);break;
				default:
					return FALSE;
			}
		}
		else
		{
			int ret=0;
			switch(argcnt)
			{
				case 0:ret=((THSysCallRetPointFunction)(pfn))();break;
				case 1:ret=((THSysCallRetPointFunction1)(pfn))(parg[0]);break;
				case 2:ret=((THSysCallRetPointFunction2)(pfn))(parg[0],parg[1]);break;
				case 3:ret=((THSysCallRetPointFunction3)(pfn))(parg[0],parg[1],parg[2]);break;
				case 4:ret=((THSysCallRetPointFunction4)(pfn))(parg[0],parg[1],parg[2],parg[3]);break;
				case 5:ret=((THSysCallRetPointFunction5)(pfn))(parg[0],parg[1],parg[2],parg[3],parg[4]);break;
				case 6:ret=((THSysCallRetPointFunction6)(pfn))(parg[0],parg[1],parg[2],parg[3],parg[4],parg[5]);break;
				case 7:ret=((THSysCallRetPointFunction7)(pfn))(parg[0],parg[1],parg[2],parg[3],parg[4],parg[5],parg[6]);break;
				case 8:ret=((THSysCallRetPointFunction8)(pfn))(parg[0],parg[1],parg[2],parg[3],parg[4],parg[5],parg[6],parg[7]);break;
				case 9:ret=((THSysCallRetPointFunction9)(pfn))(parg[0],parg[1],parg[2],parg[3],parg[4],parg[5],parg[6],parg[7],parg[8]);break;
				case 10:ret=((THSysCallRetPointFunction10)(pfn))(parg[0],parg[1],parg[2],parg[3],parg[4],parg[5],parg[6],parg[7],parg[8],parg[9]);break;
				case 11:ret=((THSysCallRetPointFunction11)(pfn))(parg[0],parg[1],parg[2],parg[3],parg[4],parg[5],parg[6],parg[7],parg[8],parg[9],parg[10]);break;
				case 12:ret=((THSysCallRetPointFunction12)(pfn))(parg[0],parg[1],parg[2],parg[3],parg[4],parg[5],parg[6],parg[7],parg[8],parg[9],parg[10],parg[11]);break;
				case 13:ret=((THSysCallRetPointFunction13)(pfn))(parg[0],parg[1],parg[2],parg[3],parg[4],parg[5],parg[6],parg[7],parg[8],parg[9],parg[10],parg[11],parg[12]);break;
				case 14:ret=((THSysCallRetPointFunction14)(pfn))(parg[0],parg[1],parg[2],parg[3],parg[4],parg[5],parg[6],parg[7],parg[8],parg[9],parg[10],parg[11],parg[12],parg[13]);break;
				case 15:ret=((THSysCallRetPointFunction15)(pfn))(parg[0],parg[1],parg[2],parg[3],parg[4],parg[5],parg[6],parg[7],parg[8],parg[9],parg[10],parg[11],parg[12],parg[13],parg[14]);break;
				case 16:ret=((THSysCallRetPointFunction16)(pfn))(parg[0],parg[1],parg[2],parg[3],parg[4],parg[5],parg[6],parg[7],parg[8],parg[9],parg[10],parg[11],parg[12],parg[13],parg[14],parg[15]);break;
				case 17:ret=((THSysCallRetPointFunction17)(pfn))(parg[0],parg[1],parg[2],parg[3],parg[4],parg[5],parg[6],parg[7],parg[8],parg[9],parg[10],parg[11],parg[12],parg[13],parg[14],parg[15],parg[16]);break;
				case 18:ret=((THSysCallRetPointFunction18)(pfn))(parg[0],parg[1],parg[2],parg[3],parg[4],parg[5],parg[6],parg[7],parg[8],parg[9],parg[10],parg[11],parg[12],parg[13],parg[14],parg[15],parg[16],parg[17]);break;
				case 19:ret=((THSysCallRetPointFunction19)(pfn))(parg[0],parg[1],parg[2],parg[3],parg[4],parg[5],parg[6],parg[7],parg[8],parg[9],parg[10],parg[11],parg[12],parg[13],parg[14],parg[15],parg[16],parg[17],parg[18]);break;
				case 20:ret=((THSysCallRetPointFunction20)(pfn))(parg[0],parg[1],parg[2],parg[3],parg[4],parg[5],parg[6],parg[7],parg[8],parg[9],parg[10],parg[11],parg[12],parg[13],parg[14],parg[15],parg[16],parg[17],parg[18],parg[19]);break;
				default:
					return FALSE;
			}
			if (rettype==1)
				pretstr->Format(_T("%d"),rettype);
			else if (rettype==2)
				pretstr->Format(_T("%u"),(unsigned int)rettype);
			else if (rettype==3)
			{
				if (rettype==0)
					pretstr->Empty();
				else
					*pretstr=(const TCHAR *)(INT_PTR)rettype;
			}
		}
		return TRUE;
	}
};
