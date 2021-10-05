#pragma once

#include <THString.h>
#include <THIni.h>
#include <THSysMisc.h>
#include <THGuid.h>
#include <THSoftwareAuth.h>
#include <THFileVersionInfo.h>

#define SID_THPLUGIN	8136

//返回FALSE中断循环
typedef BOOL (*BatchQueryFn)(LPCSTR fnName,void *fn,void *adddata);

/**
* @brief 提供给插件的系统信息接口类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-09-08 新建类
*/
class ITHPluginSystemInterface
{
public:
	ITHPluginSystemInterface(){m_nPluginSysVersion=0;}
	virtual void CauseEvent(LPCSTR event,void *param1,void *param2)=0;
	//bQuerySys 是否也查找系统函数
	virtual void BatchQueryInterface(LPCSTR fnName,BOOL bQuerySys,BatchQueryFn fn,void *adddata)=0;
	//系统的guid为"",查找所有为-
	virtual void *QueryInterface(LPCSTR guid,LPCSTR fnName,int nIndex)=0;
	UINT m_nPluginSysVersion;
};

/**
* @brief 插件信息接口类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-09-08 新建类
*/
class ITHPluginInterface
{
public:
	//返回失败为加载出错，可能提供的系统环境不支持该插件
	virtual BOOL Init()=0;
	virtual BOOL Free()=0;
	virtual void *QueryInterface(LPCSTR fnName)=0;
	virtual void OnEvent(LPCSTR event,void *param1,void *param2)=0;
};

typedef ITHPluginInterface *(*QueryPluginFn)(ITHPluginSystemInterface *sys);

/**
* @brief 通用插件信息类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-08-30 新建类
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

	THGuid guid;					///< 插件Guid，Comments
	THString sFilePath;			///< 插件文件路径
	THString sName;				///< 插件名称，FileDescription
	THString sCompany;			///< 插件发行者，CompanyName
	THString sAuthor;				///< 插件作者，LegalTrademarks
	UINT nVersion;					///< 插件版本，FileVersion 0-1
	//UINT nMinRunTime;		///< 插件需要的加载环境的最小版本，0为没有指定，通过请求函数获得，或直接在插件中判断！！
	UINT nType;						///< 插件类型，FileVersion 2-3
	int nTanAuthState;			///< 天塑插件验证服务状态，0:不使用，1:验证中，2:通过验证，3:未通过验证
	BOOL bBlock;					///< 是否被禁止加载
	THSoftwareAuth auth;		///< 授权请求器
	ITHPluginInterface *face;	///< 插件接口
	BOOL bNeedLoad;			///<< 授权测试完后是否需要加载

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
	HMODULE hModule;		///<< 插件句柄
	THString sCatchFn;			///<< 缓存的函数名
	void *sCatchPointer;			///<< 缓存的函数指针
};

#define PLUGINRUNTIME_AUTHNORETULT	0x0				///<<插件验证尚未成功
#define PLUGINRUNTIME_AUTHOK				0x1				///<<插件验证没有问题
#define PLUGINRUNTIME_AUTHFAIL			0x2				///<<插件验证有问题
#define PLUGINRUNTIME_RUNNORETULT	0x0				///<<插件没有加载设置
#define PLUGINRUNTIME_RUNBLOCK			0x4				///<<插件被禁止加载
#define PLUGINRUNTIME_RUNALWAYS		0x8				///<<插件被强制允许加载

/**
* @brief 通用插件封装类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-08-28 新建类
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
	* @brief 初始化
	* @param sPluginPath	指定插件目录
	* @param bCheckAuth		是否通过天塑插件检查服务检查插件安全性
	* @param sPluginExt		指定插件文件后缀名
	* @param nSysVersion	插件系统版本
	*/
	void Init(THString sPluginPath,BOOL bCheckAuth=TRUE,THString sPluginExt=_T("plg"),UINT nSysVersion=0)
	{
		m_sPluginPath=sPluginPath;
		m_bCheckPluginAuth=bCheckAuth;
		//是否使用标准资源字段作为插件标识（安全性考虑，加载插件信息时没有加载插件dll）
		//m_bResInfo=bResInfo;
		m_sPluginExt=sPluginExt;
		m_nPluginSysVersion=nSysVersion;
	}

	/**
	* @brief 加载指定插件
	* @param guid	指定插件guid
	* @return 是否成功，如果插件已经加载，返回成功
	*/
	BOOL LoadPlugin(const THGuid guid)
	{
		THPluginInfo *info;
		if (!m_info.GetAt(guid.GetString(),info)) return FALSE;
		return LoadPlugin(info);
	}

	/**
	* @brief 加载指定插件
	* @param sLibName	指定插件文件名
	* @return 是否成功，如果插件已经加载，返回成功
	*/
	BOOL LoadPlugin(THString sLibName)
	{
		THPluginInfo *info;
		info=UpdatePluginInfo(sLibName);
		return LoadPlugin(info);
	}

	/**
	* @brief 加载指定插件
	* @param info	指定插件信息
	* @return 是否成功，如果插件已经加载，返回成功
	*/
	BOOL LoadPlugin(THPluginInfo *info)
	{
		if (!info) return FALSE;
		return info->LoadDll(this);
	}

	/**
	* @brief 加载所有插件
	* @param bLoad 读取信息后是否加载
	* @return 是否成功
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
	* @brief 更新指定插件信息
	* @param sPath	插件文件路径
	* @return 插件信息结构体
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
	* @brief 释放指定插件
	* @param info	指定插件信息
	* @return 是否成功，如果没有加载插件返回成功
	*/
	BOOL FreePlugin(THPluginInfo *info)
	{
		return info->UnLoadDll();
	}

	/**
	* @brief 释放指定插件
	* @param guid	指定插件guid
	* @return 是否成功，如果没有加载插件返回成功
	*/
	BOOL FreePlugin(const THGuid guid)
	{
		THPluginInfo *info;
		if (!m_info.GetAt(guid.GetString(),info)) return TRUE;
		return FreePlugin(info);
	}

	/**
	* @brief 释放所有插件
	* @return 是否成功
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
	* @brief 获取所有插件信息
	* @return 是否成功
	*/
	BOOL GetPluginList()
	{
		LoadAllPlugin(FALSE);
	}

	/**
	* @brief 设置禁止加载的插件
	* @param guid	指定插件guid
	* @return 是否成功
	*/
	BOOL AddBlockPlugin(const THString guid)
	{
		return m_blockmap.SetAt(guid,1);
	}

	/**
	* @brief 删除禁止加载的插件
	* @param guid	指定插件guid
	* @return 是否成功
	*/
	BOOL RemoveBlockPlugin(const THString guid)
	{
		return m_blockmap.RemoveAt(guid);
	}

	/**
	* @brief 设置禁止加载的插件类型
	* @param type	指定插件类型
	* @return 是否成功
	*/
	BOOL AddBlockType(UINT type)
	{
		return m_blocktype.SetAt(type,1);
	}

	/**
	* @brief 删除禁止加载的插件类型
	* @param type	指定插件类型
	* @return 是否成功
	*/
	BOOL RemoveBlockType(UINT type)
	{
		return m_blocktype.RemoveAt(type);
	}

	/**
	* @brief 保存设置
	* @param ini		Ini接口
	* @param session	Session
	* @return 是否成功
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
	* @brief 加载设置
	* @param ini	Ini接口
	* @param session	Session
	* @return 是否成功
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
	* @brief 系统支持函数接口
	* @param fnName	系统函数名
	* @return 返回函数指针
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
	* @brief 查询接口，系统接口继承
	* @param guid	指定插件的guid，为空表示查找系统接口，为-表示查找任意接口
	* @param fnName	系统函数名
	* @param nIndex	找第几个
	* @return 返回函数指针
	*/
	virtual void *QueryInterface(LPCSTR guid,LPCSTR fnName,int nIndex=0)
	{
		THString g=THCharset::a2t(guid);
		THString f=THCharset::a2t(fnName);
		return QueryInterfaceT(g,f,nIndex);
	}

	/**
	* @brief 查询接口
	* @param guid	指定插件的guid，为空表示查找系统接口，为-表示查找任意接口
	* @param fnName	系统函数名
	* @param nIndex	找第几个
	* @return 返回函数指针
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
				if (MessageBox(NULL,strErrMsg+_T("\r\n是否继续加载该插件？"),_T("插件存在问题"),MB_ICONQUESTION|MB_YESNO)==IDYES)
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
				str.Format(_T("无法从天塑在线认证插件%s，插件信息：\r\n名称：%s\r\n公司：%s\r\n作者：%s\r\n是否继续加载该插件？"),THSysMisc::GetFileName(info->sFilePath),info->sName,info->sCompany,info->sAuthor);
				if (MessageBox(NULL,str,_T("无法确保插件安全性"),MB_ICONQUESTION|MB_YESNO)==IDYES)
				{
					SaveAuthWithLocalReg(md5,PLUGINRUNTIME_RUNALWAYS);
					LoadPlugin((THPluginInfo *)adddata);
				}
			}
		}
	}

	//返回0为不确定，返回PLUGINRUNTIME_XXX
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
