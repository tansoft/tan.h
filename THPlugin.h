#pragma once

#include <THString.h>
#include <THIni.h>
#include <THSysMisc.h>
#include <THGuid.h>
#include <THSoftwareAuth.h>
#include <THFileVersionInfo.h>

#define SID_THPLUGIN	8136

//����FALSE�ж�ѭ��
typedef BOOL (*BatchQueryFn)(LPCSTR fnName,void *fn,void *adddata);

/**
* @brief �ṩ�������ϵͳ��Ϣ�ӿ���
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-09-08 �½���
*/
class ITHPluginSystemInterface
{
public:
	ITHPluginSystemInterface(){m_nPluginSysVersion=0;}
	virtual void CauseEvent(LPCSTR event,void *param1,void *param2)=0;
	//bQuerySys �Ƿ�Ҳ����ϵͳ����
	virtual void BatchQueryInterface(LPCSTR fnName,BOOL bQuerySys,BatchQueryFn fn,void *adddata)=0;
	//ϵͳ��guidΪ"",��������Ϊ-
	virtual void *QueryInterface(LPCSTR guid,LPCSTR fnName,int nIndex)=0;
	UINT m_nPluginSysVersion;
};

/**
* @brief �����Ϣ�ӿ���
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-09-08 �½���
*/
class ITHPluginInterface
{
public:
	//����ʧ��Ϊ���س��������ṩ��ϵͳ������֧�ָò��
	virtual BOOL Init()=0;
	virtual BOOL Free()=0;
	virtual void *QueryInterface(LPCSTR fnName)=0;
	virtual void OnEvent(LPCSTR event,void *param1,void *param2)=0;
};

typedef ITHPluginInterface *(*QueryPluginFn)(ITHPluginSystemInterface *sys);

/**
* @brief ͨ�ò����Ϣ��
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-08-30 �½���
*/
class THPluginInfo
{
public:
	THPluginInfo()
	{
		hModule=(HMODULE)INVALID_HANDLE_VALUE;
		nVersion=0;
		nType=0;
		nTanAuthState=0;
		bBlock=FALSE;
		face=NULL;
		sCatchPointer=NULL;
		bNeedLoad=FALSE;
	}
	virtual ~THPluginInfo()
	{
		UnLoadDll();
	}

	THGuid guid;					///< ���Guid��Comments
	THString sFilePath;			///< ����ļ�·��
	THString sName;				///< ������ƣ�FileDescription
	THString sCompany;			///< ��������ߣ�CompanyName
	THString sAuthor;				///< ������ߣ�LegalTrademarks
	UINT nVersion;					///< ����汾��FileVersion 0-1
	//UINT nMinRunTime;		///< �����Ҫ�ļ��ػ�������С�汾��0Ϊû��ָ����ͨ����������ã���ֱ���ڲ�����жϣ���
	UINT nType;						///< ������ͣ�FileVersion 2-3
	int nTanAuthState;			///< ���ܲ����֤����״̬��0:��ʹ�ã�1:��֤�У�2:ͨ����֤��3:δͨ����֤
	BOOL bBlock;					///< �Ƿ񱻽�ֹ����
	THSoftwareAuth auth;		///< ��Ȩ������
	ITHPluginInterface *face;	///< ����ӿ�
	BOOL bNeedLoad;			///<< ��Ȩ��������Ƿ���Ҫ����

	BOOL OnEvent(THString event,void *param1,void *param2)
	{
		if (hModule==INVALID_HANDLE_VALUE) return FALSE;
		if (!face) return FALSE;
		THCharsetBuf bufa(event);
		face->OnEvent(bufa.t2a(),param1,param2);
		return TRUE;
	}

	void *QueryInterface(THString fnName)
	{
		THCharsetBuf bufa(fnName);
		if (hModule==INVALID_HANDLE_VALUE) return NULL;
		if (!face) return NULL;
		if (fnName==sCatchFn) return sCatchPointer;
		sCatchFn=fnName;
		sCatchPointer=face->QueryInterface(bufa.t2a());
		return sCatchPointer;
	}

	BOOL LoadDll(ITHPluginSystemInterface *sysface)
	{
		if (bBlock) return FALSE;
		if (hModule!=INVALID_HANDLE_VALUE) return TRUE;
		hModule=LoadLibrary(sFilePath);
		if (hModule==INVALID_HANDLE_VALUE) return FALSE;
		QueryPluginFn fn=(QueryPluginFn)GetProcAddress(hModule,"PluginInterface");
		if (!fn)
		{
			UnLoadDll();
			return FALSE;
		}
		face=fn(sysface);
		if (!face)
		{
			UnLoadDll();
			return FALSE;
		}
		return face->Init();
	}
	BOOL UnLoadDll()
	{
		if (face)
			face->Free();
		if (hModule!=INVALID_HANDLE_VALUE)
		{
			FreeLibrary(hModule);
			hModule=(HMODULE)INVALID_HANDLE_VALUE;
		}
		sCatchFn.Empty();
		sCatchPointer=NULL;
		face=NULL;
		return TRUE;
	}
protected:
	HMODULE hModule;		///<< ������
	THString sCatchFn;			///<< ����ĺ�����
	void *sCatchPointer;			///<< ����ĺ���ָ��
};

#define PLUGINRUNTIME_AUTHNORETULT	0x0				///<<�����֤��δ�ɹ�
#define PLUGINRUNTIME_AUTHOK				0x1				///<<�����֤û������
#define PLUGINRUNTIME_AUTHFAIL			0x2				///<<�����֤������
#define PLUGINRUNTIME_RUNNORETULT	0x0				///<<���û�м�������
#define PLUGINRUNTIME_RUNBLOCK			0x4				///<<�������ֹ����
#define PLUGINRUNTIME_RUNALWAYS		0x8				///<<�����ǿ���������

/**
* @brief ͨ�ò����װ��
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-08-28 �½���
*/
class THPlugin : private ITHSoftwareAuthResult,private ITHPluginSystemInterface
{
public:
	THPlugin()
	{
		m_bCheckPluginAuth=TRUE;
		//m_bResInfo=TRUE;
		m_info.SetFreeProc(FreePluginInfo);
	}

	virtual ~THPlugin()
	{
		FreeAllPlugin();
	}

	/**
	* @brief ��ʼ��
	* @param sPluginPath	ָ�����Ŀ¼
	* @param bCheckAuth		�Ƿ�ͨ�����ܲ��������������ȫ��
	* @param sPluginExt		ָ������ļ���׺��
	* @param nSysVersion	���ϵͳ�汾
	*/
	void Init(THString sPluginPath,BOOL bCheckAuth=TRUE,THString sPluginExt=_T("plg"),UINT nSysVersion=0)
	{
		m_sPluginPath=sPluginPath;
		m_bCheckPluginAuth=bCheckAuth;
		//�Ƿ�ʹ�ñ�׼��Դ�ֶ���Ϊ�����ʶ����ȫ�Կ��ǣ����ز����Ϣʱû�м��ز��dll��
		//m_bResInfo=bResInfo;
		m_sPluginExt=sPluginExt;
		m_nPluginSysVersion=nSysVersion;
	}

	/**
	* @brief ����ָ�����
	* @param guid	ָ�����guid
	* @return �Ƿ�ɹ����������Ѿ����أ����سɹ�
	*/
	BOOL LoadPlugin(const THGuid guid)
	{
		THPluginInfo *info;
		if (!m_info.GetAt(guid.GetString(),info)) return FALSE;
		return LoadPlugin(info);
	}

	/**
	* @brief ����ָ�����
	* @param sLibName	ָ������ļ���
	* @return �Ƿ�ɹ����������Ѿ����أ����سɹ�
	*/
	BOOL LoadPlugin(THString sLibName)
	{
		THPluginInfo *info;
		info=UpdatePluginInfo(sLibName);
		return LoadPlugin(info);
	}

	/**
	* @brief ����ָ�����
	* @param info	ָ�������Ϣ
	* @return �Ƿ�ɹ����������Ѿ����أ����سɹ�
	*/
	BOOL LoadPlugin(THPluginInfo *info)
	{
		if (!info) return FALSE;
		return info->LoadDll(this);
	}

	/**
	* @brief �������в��
	* @param bLoad ��ȡ��Ϣ���Ƿ����
	* @return �Ƿ�ɹ�
	*/
	BOOL LoadAllPlugin(BOOL bLoad=TRUE)
	{
		if (m_sPluginPath.IsEmpty()) return FALSE;
		THStringArray arFile;
		THString sExt=_T("*.plg");
		if (!m_sPluginExt.IsEmpty()) sExt=_T("*.")+m_sPluginExt;
		if (!THSysMisc::FindFiles(m_sPluginPath,&arFile,NULL,sExt,FALSE,PATHMODE_FULLPATH)) return FALSE;
		THPluginInfo *info;
		for(int i=0;i<arFile.GetSize();i++)
		{
			info=UpdatePluginInfo(arFile[i]);
			if (info)
			{
				THString md5=THMd5::CalcFileMd5Str(arFile[i]);
				int ret=CheckAuthWithLocalReg(md5);
				if (m_bCheckPluginAuth)
				{
					if (ret&PLUGINRUNTIME_AUTHOK) info->auth.SetAuthState(AuthState_Success);
					if (ret&PLUGINRUNTIME_AUTHFAIL) info->auth.SetAuthState(AuthState_Fail);
					if (((ret&PLUGINRUNTIME_AUTHOK) && !(ret&PLUGINRUNTIME_RUNBLOCK)) ||
						ret&PLUGINRUNTIME_RUNALWAYS)
					{
						if (bLoad) LoadPlugin(info);
					}
					if (info->auth.GetAuthState()==AuthState_None)
					{
						info->bNeedLoad=bLoad;
						info->auth.StartCheckAuth(SID_THPLUGIN,md5);
					}
				}
				else
				{
					if (!(ret&PLUGINRUNTIME_RUNBLOCK) && bLoad) LoadPlugin(info);
				}
			}
		}
		return TRUE;
	}

	/**
	* @brief ����ָ�������Ϣ
	* @param sPath	����ļ�·��
	* @return �����Ϣ�ṹ��
	*/
	THPluginInfo *UpdatePluginInfo(THString sPath)
	{
		THPluginInfo *pInfo=new THPluginInfo;
		if (!pInfo) return NULL;
		pInfo->auth.SetEventHandler(this,pInfo);
		//if (m_bResInfo)
		{
			//resource mode
			THFileVersionInfo ver;
			ver.Init(sPath);
			pInfo->guid.SetString(ver.GetComments());
			pInfo->sFilePath=sPath;
			pInfo->sName=ver.GetFileDescription();
			pInfo->sCompany=ver.GetCompanyName();
			pInfo->sAuthor=ver.GetLegalTrademarks();
			pInfo->nVersion=ver.GetFileVersionMS();
			pInfo->nType=ver.GetFileVersionLS();
		}
		if (pInfo->guid.IsEmpty())
		{
			delete pInfo;
			return NULL;
		}
		if (!m_blockmap.GetKeyPos(pInfo->guid.GetString()).IsEmpty())
			pInfo->bBlock=TRUE;
		if (!m_blocktype.GetKeyPos(pInfo->nType).IsEmpty())
			pInfo->bBlock=TRUE;
		THPluginInfo *pInfo1;
		if (m_info.GetAt(pInfo->guid.GetString(),pInfo1))
		{
			delete pInfo;
			return pInfo1;
		}
		m_info.SetAt(pInfo->guid.GetString(),pInfo);
		return pInfo;
	}

	/**
	* @brief �ͷ�ָ�����
	* @param info	ָ�������Ϣ
	* @return �Ƿ�ɹ������û�м��ز�����سɹ�
	*/
	BOOL FreePlugin(THPluginInfo *info)
	{
		return info->UnLoadDll();
	}

	/**
	* @brief �ͷ�ָ�����
	* @param guid	ָ�����guid
	* @return �Ƿ�ɹ������û�м��ز�����سɹ�
	*/
	BOOL FreePlugin(const THGuid guid)
	{
		THPluginInfo *info;
		if (!m_info.GetAt(guid.GetString(),info)) return TRUE;
		return FreePlugin(info);
	}

	/**
	* @brief �ͷ����в��
	* @return �Ƿ�ɹ�
	*/
	BOOL FreeAllPlugin()
	{
		THPosition pos=m_info.GetStartPosition();
		THPluginInfo *info;
		THString guid;
		while(!pos.IsEmpty())
		{
			if (m_info.GetNextPosition(pos,guid,info))
			{
				info->UnLoadDll();
			}
		}
		return TRUE;
	}

	/**
	* @brief ��ȡ���в����Ϣ
	* @return �Ƿ�ɹ�
	*/
	BOOL GetPluginList()
	{
		LoadAllPlugin(FALSE);
	}

	/**
	* @brief ���ý�ֹ���صĲ��
	* @param guid	ָ�����guid
	* @return �Ƿ�ɹ�
	*/
	BOOL AddBlockPlugin(const THString guid)
	{
		return m_blockmap.SetAt(guid,1);
	}

	/**
	* @brief ɾ����ֹ���صĲ��
	* @param guid	ָ�����guid
	* @return �Ƿ�ɹ�
	*/
	BOOL RemoveBlockPlugin(const THString guid)
	{
		return m_blockmap.RemoveAt(guid);
	}

	/**
	* @brief ���ý�ֹ���صĲ������
	* @param type	ָ���������
	* @return �Ƿ�ɹ�
	*/
	BOOL AddBlockType(UINT type)
	{
		return m_blocktype.SetAt(type,1);
	}

	/**
	* @brief ɾ����ֹ���صĲ������
	* @param type	ָ���������
	* @return �Ƿ�ɹ�
	*/
	BOOL RemoveBlockType(UINT type)
	{
		return m_blocktype.RemoveAt(type);
	}

	/**
	* @brief ��������
	* @param ini		Ini�ӿ�
	* @param session	Session
	* @return �Ƿ�ɹ�
	*/
	BOOL SaveSettings(ITHIni *ini,THString session)
	{
		THIniAutoWBool(ini,session,bCheckPluginAuth,TRUE);
		THIniAutoWString(ini,session,sPluginPath,_T(""));
		THIniAutoWString(ini,session,sPluginExt,_T(""));
		THIniAutoWInt(ini,session,nPluginSysVersion,0);
		THPosition pos;
		pos=m_blockmap.GetStartPosition();
		THString bs,ret;
		int n;
		while(!pos.IsEmpty())
		{
			if (m_blockmap.GetNextPosition(pos,bs,n))
				ret+=bs+_T("|");
		}
		THIniWString(ini,session,_T("BlockPluginGuid"),ret,_T(""));
		ret.Empty();
		UINT nType;
		while(!pos.IsEmpty())
		{
			if (m_blocktype.GetNextPosition(pos,nType,n))
				ret.AppendFormat(_T("%u|"),nType);
		}
		THIniWString(ini,session,_T("BlockPluginType"),ret,_T(""));
		return TRUE;
	}

	/**
	* @brief ��������
	* @param ini	Ini�ӿ�
	* @param session	Session
	* @return �Ƿ�ɹ�
	*/
	BOOL LoadSettings(ITHIni *ini,THString session)
	{
		THIniAutoRBool(ini,session,bCheckPluginAuth,TRUE);
		THIniAutoRString(ini,session,sPluginPath,_T(""));
		THIniAutoRString(ini,session,sPluginExt,_T(""));
		THIniAutoRInt(ini,session,nPluginSysVersion,0);
		THString ret;
		THIniRString(ini,session,_T("BlockGuid"),ret,_T(""));
		m_blockmap.RemoveAll();
		THStringToken t(ret,_T("|"));
		while(t.IsMoreTokens())
		{
			ret=t.GetNextToken();
			if (!ret.IsEmpty())
				m_blockmap.SetAt(ret,1);
		}
		THIniRString(ini,session,_T("BlockType"),ret,_T(""));
		m_blocktype.RemoveAll();
		t.Init(ret,_T("|"));
		while(t.IsMoreTokens())
		{
			ret=t.GetNextToken();
			if (!ret.IsEmpty())
				m_blocktype.SetAt(THs2u(ret),1);
		}
		return TRUE;
	}

	/**
	* @brief ϵͳ֧�ֺ����ӿ�
	* @param fnName	ϵͳ������
	* @return ���غ���ָ��
	*/
	virtual void *QuerySystemInterface(THString fnName){return NULL;}

protected:
	virtual void CauseEvent(LPCSTR event,void *param1,void *param2)
	{
		THString e=THCharset::a2t(event);
		THPosition pos=m_info.GetStartPosition();
		THString key;
		THPluginInfo *info;
		while(!pos.IsEmpty())
		{
			if (m_info.GetNextPosition(pos,key,info) && info) info->OnEvent(e,param1,param2);
		}
	}

	virtual void BatchQueryInterface(LPCSTR fnName,BOOL bQuerySys,BatchQueryFn bfn,void *adddata)
	{
		void *fn;
		if (bQuerySys)
		{
			THString f=THCharset::a2t(fnName);
			fn=QuerySystemInterface(f);
			if (fn && !bfn(fnName,fn,adddata)) return;
		}
		THPosition pos=m_info.GetStartPosition();
		THString key;
		THPluginInfo *info;
		while(!pos.IsEmpty())
		{
			if (m_info.GetNextPosition(pos,key,info))
			{
				if (info)
				{
					fn=info->QueryInterface(THCharset::a2t(fnName));
					if (fn && !bfn(fnName,fn,adddata)) return;
				}
			}
		}
		return;
	}

	/**
	* @brief ��ѯ�ӿڣ�ϵͳ�ӿڼ̳�
	* @param guid	ָ�������guid��Ϊ�ձ�ʾ����ϵͳ�ӿڣ�Ϊ-��ʾ��������ӿ�
	* @param fnName	ϵͳ������
	* @param nIndex	�ҵڼ���
	* @return ���غ���ָ��
	*/
	virtual void *QueryInterface(LPCSTR guid,LPCSTR fnName,int nIndex=0)
	{
		THString g=THCharset::a2t(guid);
		THString f=THCharset::a2t(fnName);
		return QueryInterfaceT(g,f,nIndex);
	}

	/**
	* @brief ��ѯ�ӿ�
	* @param guid	ָ�������guid��Ϊ�ձ�ʾ����ϵͳ�ӿڣ�Ϊ-��ʾ��������ӿ�
	* @param fnName	ϵͳ������
	* @param nIndex	�ҵڼ���
	* @return ���غ���ָ��
	*/
	virtual void *QueryInterfaceT(THString g,THString f,int nIndex=0)
	{
		void *fn=NULL;
		if (g.IsEmpty() || g==_T("-"))
		{
			fn=QuerySystemInterface(f);
			if (fn)
			{
				if (nIndex<=0) return fn;
				nIndex--;
			}
		}
		THPosition pos=m_info.GetStartPosition();
		THString key;
		THPluginInfo *info;
		while(!pos.IsEmpty())
		{
			if (m_info.GetNextPosition(pos,key,info))
			{
				if (info)
				{
					if (g.IsEmpty() || g==_T("-") || info->guid.IsSame(g))
					{
						fn=info->QueryInterface(f);
						if (fn)
						{
							if (nIndex<=0) return fn;
							nIndex--;
						}
					}
				}
			}
		}
		return fn;
	}

	virtual void OnAuthResult(THSoftwareAuth *cls,THAuthState Result,THString strErrMsg,void *adddata)
	{
		THPluginInfo *info=(THPluginInfo *)adddata;
		if (!info) return;
		THString md5=THMd5::CalcFileMd5Str(info->sFilePath);
		if (Result==AuthState_Success)
		{
			SaveAuthWithLocalReg(md5,PLUGINRUNTIME_AUTHOK);
			 if (info->bNeedLoad) LoadPlugin(info);
		}
		else if (Result==AuthState_Fail)
		{
			SaveAuthWithLocalReg(md5,PLUGINRUNTIME_AUTHFAIL);
			if (info->bNeedLoad)
			{
				if (MessageBox(NULL,strErrMsg+_T("\r\n�Ƿ�������ظò����"),_T("�����������"),MB_ICONQUESTION|MB_YESNO)==IDYES)
				{
					SaveAuthWithLocalReg(md5,PLUGINRUNTIME_RUNALWAYS);
					LoadPlugin(info);
				}
			}
		}
		else if (Result==AuthState_NetWorkFail)
		{
			if (info->bNeedLoad)
			{
				THString str;
				str.Format(_T("�޷�������������֤���%s�������Ϣ��\r\n���ƣ�%s\r\n��˾��%s\r\n���ߣ�%s\r\n�Ƿ�������ظò����"),THSysMisc::GetFileName(info->sFilePath),info->sName,info->sCompany,info->sAuthor);
				if (MessageBox(NULL,str,_T("�޷�ȷ�������ȫ��"),MB_ICONQUESTION|MB_YESNO)==IDYES)
				{
					SaveAuthWithLocalReg(md5,PLUGINRUNTIME_RUNALWAYS);
					LoadPlugin((THPluginInfo *)adddata);
				}
			}
		}
	}

	//����0Ϊ��ȷ��������PLUGINRUNTIME_XXX
	int CheckAuthWithLocalReg(THString md5)
	{
		THReg reg;
		if (!reg.Open(HKEY_LOCAL_MACHINE,_T("Software\\Tansoft\\AuthResult\\HttpServer"))) return 0;
		return reg.ReadDword(md5);
	}

	void SaveAuthWithLocalReg(THString md5,int RunType)
	{
		THReg reg;
		if (!reg.Open(HKEY_LOCAL_MACHINE,_T("Software\\Tansoft\\AuthResult\\HttpServer"))) return;
		int org=reg.ReadDword(md5);
		if (RunType&PLUGINRUNTIME_AUTHOK)
		{
			org&=~PLUGINRUNTIME_AUTHFAIL;
			RunType&=~PLUGINRUNTIME_AUTHFAIL;
		}
		if (RunType&PLUGINRUNTIME_AUTHFAIL)
		{
			org&=~PLUGINRUNTIME_AUTHOK;
		}
		if (RunType&PLUGINRUNTIME_RUNBLOCK)
		{
			org&=~PLUGINRUNTIME_RUNALWAYS;
			RunType&=~PLUGINRUNTIME_RUNALWAYS;
		}
		if (RunType&PLUGINRUNTIME_RUNALWAYS)
		{
			org&=~PLUGINRUNTIME_RUNBLOCK;
		}
		org|=RunType;
		reg.WriteDword(md5,org);
	}

	BOOL m_bCheckPluginAuth;
	//BOOL m_bResInfo;
	THString m_sPluginPath;
	THString m_sPluginExt;
	THStringMap<THPluginInfo *> m_info;
	THStringMap<int> m_blockmap;
	THMap<UINT,int> m_blocktype;
	static void FreePluginInfo(void *key,void *value,void *adddata)
	{
		delete (THPluginInfo *)value;
	}
};
