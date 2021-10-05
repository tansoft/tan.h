#pragma once

#define THSYSTEM_INCLUDE_OBJIDL
#define THSYSTEM_INCLUDE_SHELLOBJIDL
#include <THSystem.h>
#include <THCharset.h>

/**
* @brief 快捷方式处理基类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-03-16 新建类
*/
class THLinkFile
{
public:
	static BOOL CreateLink(THString sDesc,THString sLnkFile,THString sDestFile,THString sDestPath,THString sFileParam,THString sIconFile,int IconIndex=0)
	{
		if (sDesc.IsEmpty() || sLnkFile.IsEmpty() || sDestFile.IsEmpty()) return FALSE;
		if (FAILED(CoInitialize(NULL))) return FALSE;
		BOOL bRet=FALSE;
		IShellLink* psl;
		if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,IID_IShellLink,(void **)&psl)))
		{
			IPersistFile* ppf;
			// Set the path to the shortcut target and add the
			// description.
			psl->SetDescription(sDesc);
			psl->SetPath(sDestFile);
			if (!sDestPath.IsEmpty()) psl->SetWorkingDirectory(sDestPath);
			if (!sFileParam.IsEmpty()) psl->SetArguments(sFileParam);
			if (!sIconFile.IsEmpty()) psl->SetIconLocation(sIconFile,IconIndex);
			// Query IShellLink for the IPersistFile interface for saving the
			// shortcut in persistent storage.
			if (SUCCEEDED(psl->QueryInterface(IID_IPersistFile,(void **)&ppf)))
			{
				wchar_t *tmp=THCharset::t2w(sLnkFile);
				if (tmp)
				{
					// Save the link by calling IPersistFile::Save.
					if (SUCCEEDED(ppf->Save(tmp,TRUE))) bRet=TRUE;
					THCharset::free(tmp);
				}
				ppf->Release();
			}
			psl->Release();
		}
		CoUninitialize();
		return bRet;
	}

	static THString GetExePath(THString sLnkFile)
	{
		THString sRet;
		if (sLnkFile.IsEmpty()) return sRet;
		if (SUCCEEDED(CoInitialize(NULL)))
		{
			IShellLink* psl;
			if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,IID_IShellLink,(void **)&psl)))
			{
				IPersistFile* ppf;
				if (SUCCEEDED(psl->QueryInterface(IID_IPersistFile,(void **)&ppf)))
				{
					wchar_t *tmp=THCharset::t2w(sLnkFile);
					if (tmp)
					{
						// Save the link by calling IPersistFile::Save.
						if (SUCCEEDED(ppf->Load(tmp,STGM_READ)))
						{
							psl->GetPath(sRet.GetBuffer(1024), 1024, NULL, SLGP_UNCPRIORITY);
							sRet.ReleaseBuffer();
						}
						THCharset::free(tmp);
					}
					ppf->Release();
				}
				psl->Release();
			}
			CoUninitialize();
		}
		return sRet;
	}

	//替换快捷方式指向的文件
	static BOOL SetLinkFilePath(THString sLnkFile,THString sNewPath)
	{
		BOOL ret=FALSE;
		if (!sLnkFile.IsEmpty() && THSysMisc::IsFile(sLnkFile))
		{
			if (SUCCEEDED(CoInitialize(NULL)))
			{
				IShellLink* psl;
				if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,IID_IShellLink,(void **)&psl)))
				{
					IPersistFile* ppf;
					if (SUCCEEDED(psl->QueryInterface(IID_IPersistFile,(void **)&ppf)))
					{
						wchar_t *tmp=THCharset::t2w(sLnkFile);
						if (tmp)
						{
							// Save the link by calling IPersistFile::Save.
							if (SUCCEEDED(ppf->Load(tmp,STGM_READWRITE)))
							{
								if (SUCCEEDED(psl->SetPath(sNewPath)))
									if (SUCCEEDED(ppf->Save(tmp,TRUE)))
										ret=TRUE;
							}
							THCharset::free(tmp);
						}
						ppf->Release();
					}
					psl->Release();
				}
				CoUninitialize();
			}
		}
		return ret;
	}
};
