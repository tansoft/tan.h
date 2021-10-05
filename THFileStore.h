#pragma once

#include <THString.h>
#include <THSystem.h>
#include <THStruct.h>
#include <THCharset.h>
#include <THFile.h>
#include <THTime.h>
#include <THMemCache.h>
#include <THMemBuf.h>
#include <THSysMisc.h>
#include <THBaseDialog.h>
#include <THArith.h>

#define MEMCACHE_SIZE			16384
#define MEMCACHE_COUNT			256	//256*16384=4M

//注意 FILEMODE_ 是独立使用的，FILEMODE_MASK是掩码
#define FILEMODE_READ					1		//读
#define FILEMODE_READWRITE				2		//读写
#define FILEMODE_CREATEOREXIST			3		//读写，存在则打开，不存在新建
#define FILEMODE_ALWAYSCREATE			4		//读写，总是新建
#define FILEMODE_MASK					0x0ff	//文件打开方式Mask
#define FILEMODE_MASK_SHAREEXCLUSIVE	0x100	//附加使用，不允许共享方式打开文件

/**
* @brief 文件储存器描述类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-09-07 新建类
*/
/**<pre>
	引入SetFreeor和DeleteObjectByFreeor处理释放问题
	例如传入ITHFileStore给类a，而且希望由类a来释放，传入前调用SetFreeor设置对象指针为类a，类a中释放时总是调用DeleteObjectByFreeor并传入this
	系统中的类THIni，THScript，THServer等调用时只需要指定参数bAutoFree，类中会自动调用SetFreeor和DeleteObjectByFreeor
</pre>
*/
class ITHFileStore : private ITHMemCacheNotify
{
public:
	ITHFileStore()
	{
		m_freeobj=NULL;
		SetMemCacheSettings(0);
		m_cache.SetBufferObject(this,NULL);
	}
	virtual ~ITHFileStore(){}

	/**
	* @brief 设置释放储存器的对象
	* @param freeor		对象指针
	*/
	virtual void SetFreeor(void *freeor){m_freeobj=freeor;}

	/**
	* @brief 尝试释放储存器
	* @param freeor		对象指针
	*/
	virtual void DeleteObjectByFreeor(void *freeor){if (freeor==m_freeobj) delete this;}

	/**
	* @brief 新建储存器对象
	* @return 新建具体实例储存器对象的实例指针
	*/
	virtual ITHFileStore *CreateObject()=0;

	/**
	* @brief 设置基础目录
	* @param path		基础目录，支持/的目录表示
	*/
	virtual void SetBaseDirectory(THString path)
	{
		m_basepath=path;
		m_basepath.Replace(_T("/"),_T("\\"));
		if (m_basepath.GetLength()>0 && m_basepath.GetAt(m_basepath.GetLength()-1)=='\\')
			m_basepath=m_basepath.Left(m_basepath.GetLength()-1);
	}

	virtual BOOL SetStoreAttrib(THString key,THString value){return FALSE;}

	virtual THString GetStoreAttrib(THString key){return _T("");}

	virtual THString GetBaseDirectory(){return m_basepath;}

	virtual THString GetRootPath(){return m_basepath;}

	virtual THString GetParentPath(THString relpath)
	{
		if (relpath.IsEmpty()) return m_basepath;
		relpath.Replace(_T("/"),_T("\\"));
		if (_tcsnicmp(m_basepath,relpath,m_basepath.GetLength())!=0)
		{
			//the path is relpath
			relpath=GetFullPath(relpath);
		}
		if (relpath.GetAt(relpath.GetLength()-1)=='\\') relpath=relpath.Left(relpath.GetLength()-1);
		if (relpath.ReverseFind('\\')!=-1)
			relpath=relpath.Left(relpath.ReverseFind('\\'));
		//always root path
		if (relpath.GetLength()<=m_basepath.GetLength()) return m_basepath;
		return relpath;
	}

	/**
	* @brief 由全路径获取相对路径
	* @param fullpath		相对目录或完整目录，支持/的目录表示，支持文件直接处理
	* @param bReversalSign	是否返回'/'格式
	* @return 相对路径
	*/
	virtual THString GetRelPath(THString fullpath,BOOL bReversalSign=FALSE)
	{
		fullpath=GetFullPath(fullpath);
		fullpath=fullpath.Mid(m_basepath.GetLength());
		if (fullpath.Left(1)=='\\' || fullpath.Left(1)=='/') fullpath=fullpath.Mid(1);
		if (bReversalSign) fullpath.Replace(_T("\\"),_T("/"));
		return fullpath;
	}

	/**
	* @brief 由相对目录获取全路径
	* @param relpath		相对目录或完整目录，支持/的目录表示，支持文件直接处理
	* @param curpath		当前目录，支持相对目录或完整目录，用于计算相对目录
	* @return 全路径
	*/
	virtual THString GetFullPath(THString relpath,THString curpath=_T(""))
	{
		//没有处理路径中出现的..\\和.\\，只有处理开头含有这些字符的情况
		if (relpath.IsEmpty()) return m_basepath;
		relpath.Replace(_T("/"),_T("\\"));
		//the path is a fullpath
		if (_tcsnicmp(m_basepath,relpath,m_basepath.GetLength())==0)
		{
			if (relpath.GetAt(relpath.GetLength()-1)=='\\') relpath=relpath.Left(relpath.GetLength()-1);
			return relpath;
		}
		//the path is a abslote path
		if (relpath.Find(_T(":"),0)!=-1) return relpath;
		//the path is root rel path
		if (relpath==_T("\\")) return m_basepath;
		if (relpath.GetAt(0)=='\\') return m_basepath+relpath;
		//make sure have current path
		if (!curpath.IsEmpty()) curpath=GetFullPath(curpath);
		//parse the ..\\ symbol
		while(1)
		{
			if (relpath.Find(_T("..\\"),0)==0)
			{
				relpath=relpath.Mid(3);
				curpath=GetParentPath(curpath);
			}
			else if (relpath.Find(_T(".\\"),0)==0)
			{
				relpath=relpath.Mid(2);
			}
			else
				break;
		}
		if (curpath.IsEmpty()) curpath=m_basepath;
		if (curpath.GetAt(curpath.GetLength()-1)=='\\') curpath=curpath.Left(curpath.GetLength()-1);
		return curpath+_T("\\")+relpath;
	}

	/**
	* @brief 获取文件所在路径，如果给出的是路径等于parentpath
	* @param sPath		文件名
	* @return 文件路径
	*/
	virtual THString GetCurPath(THString sPath)
	{
		return GetParentPath(sPath);
	}

	/**
	* @brief 获取全路径中的文件名
	* @param sPath		全路径文件名
	* @return 文件名
	*/
	virtual THString GetFileBaseName(THString sPath)
	{
		sPath.Replace(_T("/"),_T("\\"));
		int pos=sPath.ReverseFind('\\');
		if (pos!=-1) sPath=sPath.Mid(pos+1);
		return sPath;
	}

	/**
	* @brief 获取文件后缀名，总是小写
	* @param sPath		文件名
	* @return 文件路径
	*/
	virtual THString GetFileExt(THString sPath)
	{
		THString sExt;
		sPath.Replace(_T("/"),_T("\\"));
		int pos=sPath.ReverseFind('.');
		int pos2=sPath.ReverseFind('\\');
		if (pos!=-1 && pos > pos2) sExt=sPath.Mid(pos+1);
		sExt.MakeLower();
		return sExt;
	}

	/**
	* @brief 储存系统名称
	* @return 储存系统名称
	*/
	virtual THString GetStoreName()=0;

	/**
	* @brief 路径是否为目录
	* @param sPath		路径
	* @return 是否为目录
	*/
	virtual BOOL IsPathDir(THString sPath)
	{
		sPath=GetFullPath(sPath);
		if (_tcsicmp(sPath,GetRootPath())==0) return TRUE;
		THString sParentPath=GetParentPath(sPath);
		THString sFile=GetFileBaseName(sPath);
		THStringArray data;
		if (!FindFiles(sParentPath,&data,NULL)) return FALSE;
		for(int i=0;i<data.GetSize();i++)
			if (data[i].CompareNoCase(sFile)==0) return TRUE;
		return FALSE;
	}

	/**
	* @brief 路径是否为文件
	* @param sPath		路径
	* @return 是否为文件
	*/
	virtual BOOL IsPathFile(THString sPath)
	{
		sPath=GetFullPath(sPath);
		if (_tcsicmp(sPath,GetRootPath())==0) return FALSE;
		THString sParentPath=GetParentPath(sPath);
		THString sFile=GetFileBaseName(sPath);
		THStringArray data;
		if (!FindFiles(sParentPath,NULL,&data)) return FALSE;
		for(int i=0;i<data.GetSize();i++)
			if (data[i].CompareNoCase(sFile)==0) return TRUE;
		return FALSE;
	}

	/**
	* @brief 创建目录，多级目录也支持创建
	* @param sPath		路径
	* @return 是否成功
	*/
	virtual BOOL CreateDirectory(THString sPath)
	{
		if (IsPathDir(sPath)) return TRUE;
		THString sParentPath=GetParentPath(sPath);
		if (_tcsicmp(sParentPath,GetRootPath())!=0)
			if (!CreateDirectory(sParentPath)) return FALSE;
		return _CreateDirectory(sPath);
	}

	/**
	* @brief 删除目录，多级目录也支持删除
	* @param sPath		路径
	* @return 是否成功
	*/
	virtual BOOL DeleteDirectory(THString sPath)
	{
		sPath=GetFullPath(sPath);
		if (!IsPathDir(sPath)) return FALSE;
		THStringArray dir,file;
		if (!FindFiles(sPath,&dir,&file)) return FALSE;
		for(int i=0;i<dir.GetSize();i++)
			if (!DeleteDirectory(sPath+_T("\\")+dir[i])) return FALSE;
		for(int i=0;i<file.GetSize();i++)
			if (!DeleteFile(sPath+_T("\\")+file[i])) return FALSE;
		return _DeleteDirectory(sPath);
	}

	/**
	* @brief 改名，支持目录和文件
	* @param sPath		原对象
	* @param sNewName	新名字，不需要目录
	* @return 是否成功
	*/
	virtual BOOL Rename(THString sPath,THString sNewName)
	{
		THString sParentPath=GetParentPath(sPath);
		return MoveFile(sPath,sParentPath+_T("\\")+sNewName);
	}

	/**
	* @brief 复制目录和文件
	* @param sPath		原对象，文件为单一复制，目录为全目录复制
	* @param sNewPath	复制目标
	* @param fs			复制目标的文件系统，为NULL为当前文件系统
	* @return 是否成功
	*/
	virtual BOOL CopyFile(THString sPath,THString sNewPath,ITHFileStore *fs=NULL)
	{
		if (fs==NULL) fs=this;
		sPath=GetFullPath(sPath);
		sNewPath=fs->GetFullPath(sNewPath);
		if (IsPathFile(sPath))
		{
			return _CopyFile(sPath,sNewPath,fs);
		}
		else if (IsPathDir(sPath))
		{
			THStringArray dir,file;
			if (!FindFiles(sPath,&dir,&file)) return FALSE;
			for(int i=0;i<dir.GetSize();i++)
			{
				if (!fs->CreateDirectory(sNewPath+_T("\\")+dir[i])) return FALSE;
				if (!CopyFile(sPath+_T("\\")+dir[i],sNewPath+_T("\\")+dir[i],fs)) return FALSE;
			}
			for(int i=0;i<file.GetSize();i++)
				if (!_CopyFile(sPath+_T("\\")+file[i],sNewPath+_T("\\")+file[i],fs)) return FALSE;
			return TRUE;
		}
		return FALSE;
	}

	/**
	* @brief 移动目录和文件
	* @param sPath		原对象，文件为单一移动，目录为全目录移动
	* @param sNewPath	移动目标
	* @param fs			移动目标的文件系统，为NULL为当前文件系统
	* @return 是否成功
	*/
	virtual BOOL MoveFile(THString sPath,THString sNewPath,ITHFileStore *fs=NULL)
	{
		if (fs==NULL) fs=this;
		sPath=GetFullPath(sPath);
		sNewPath=fs->GetFullPath(sNewPath);
		if (IsPathFile(sPath)) return _MoveFile(sPath,sNewPath,fs);
		if (!CopyFile(sPath,sNewPath,fs)) return FALSE;
		return DeleteDirectory(sPath);
	}

	/**
	* @brief 删除目录和文件
	* @param sPath		原对象，文件或目录
	* @return 是否成功
	*/
	virtual BOOL DeleteFile(THString sPath)
	{
		sPath=GetFullPath(sPath);
		if (IsPathFile(sPath)) return _DeleteFile(sPath);
		else if (IsPathDir(sPath)) return DeleteDirectory(sPath);
		return TRUE;
	}

	/**
	* @brief 查找文件
	* @param sPath		查找的目录
	* @param arDir		子目录结果，可为空，返回为文件名
	* @param arFile		文件结果，可为空，返回为文件名
	* @return 是否成功
	*/
	virtual BOOL FindFiles(THString sPath,THStringArray *arDir,THStringArray *arFile)=0;

	/**
	* @brief 获取文件信息
	* @param sFile		指定文件
	* @param size		文件大小，可为空
	* @param ct			文件创建日期，可为空
	* @param mt			文件修改日期，可为空
	* @return 是否为文件
	*/
	virtual BOOL GetFileAttrib(THString sFile,UINT *size,THTime *ct,THTime *mt)=0;

	/**
	* @brief 打开文件
	* @param sFile		文件
	* @param nMode		打开方式，FILEMODE_XXX
	* @return 返回文件索引号，-1为打开失败
	*/
	virtual int OpenFile(THString sFile,int nMode=FILEMODE_READ)
	{
		int idx=_OpenFile(sFile,nMode);
		if (m_bUseMemCache && idx!=-1)
		{
			sFile=GetFullPath(sFile);
			int bufidx;
			if (!m_namemap.GetAt(sFile,bufidx))
			{
				bufidx=GetTickCount();
				m_namemap.SetAt(sFile,bufidx);
			}
			m_idxmap.SetAt(idx,bufidx);
			m_curoff.SetAt(idx,0);
		}
		return idx;
	}

	/**
	* @brief 关闭文件
	* @param idx		文件索引
	*/
	virtual void CloseFile(int idx)
	{
		if (m_bUseMemCache)
		{
			m_idxmap.RemoveAt(idx);
			m_curoff.RemoveAt(idx);
		}
		_CloseFile(idx);
	}

	/**
	* @brief 关闭所有打开文件
	*/
	virtual void CloseAllFile()
	{
		m_idxmap.RemoveAll();
		m_curoff.RemoveAll();
		_CloseAllFile();
	}

	/**
	* @brief 获取文件长度
	* @return 返回文件长度
	*/
	virtual UINT GetFileSize(int idx)=0;

	/**
	* @brief 获取当前文件位置
	* @return 返回文件当前位置
	*/
	virtual UINT GetPosition(int idx)
	{
		if (m_bUseMemCache)
			return m_curoff[idx];
		return _GetPosition(idx);
	}

	/**
	* @brief 定位文件
	* @param idx		文件索引号
	* @param nStart		开始方式，详见SEEK_XX
	* @param nPos		到达偏移
	* @return 返回实际定位位置
	*/
	virtual UINT SeekFile(int idx,int nStart,int nPos)
	{
		UINT off=_SeekFile(idx,nStart,nPos);
		if (m_bUseMemCache)
			m_curoff.SetAt(idx,off);
		return off;
	}

	/**
	* @brief 读取文件，带缓存
	* @param idx		文件索引号
	* @param buf		缓冲指针
	* @param len		缓冲长度
	* @param pos		读取位置,-1为不指定,即当前位置
	* @return 返回实际读取长度
	*/
	virtual UINT ReadFile(int idx,void *buf,UINT len,int pos=-1)
	{
		if (m_bUseMemCache)
		{
			if (pos==-1) pos=GetPosition(idx);
			int bufidx=m_idxmap[idx];
			if (m_cache.BufferedRead(bufidx,(UINT)pos,buf,len,(void *)(INT_PTR)idx))
			{
				m_curoff.SetAt(idx,pos+len);
				return len;
			}
			return 0;
		}
		else
		{
			return _ReadFile(idx,buf,len,pos);
		}
	}

	/**
	* @brief 读取整个文件，带缓存
	* @param sFile		文件名
	* @param len		返回读取的长度，可为空
	* @return 返回缓冲指针，需要调用FreeBuf释放
	*/
	virtual void *ReadWholeFile(LPCTSTR sFile,UINT *len=NULL)
	{
		if (len) *len=0;
		int idx=OpenFile(sFile);
		if (idx==-1) return NULL;
		void *buf=ReadWholeFile(idx,len);
		CloseFile(idx);
		return buf;
	}

	/**
	* @brief 读取整个文件，带缓存
	* @param idx		文件索引号
	* @param len		返回读取的长度，可为空
	* @return 返回缓冲指针，需要调用FreeBuf释放
	*/
	virtual void *ReadWholeFile(int idx,UINT *len=NULL)
	{
		UINT size=GetFileSize(idx);
		if (size>0)
		{
			void *buf=new char[size];
			if (buf)
			{
				SeekFile(idx,SEEK_SET,0);
				size=ReadFile(idx,buf,size);
				if (len) *len=size;
				return buf;
			}
		}
		if (len) *len=0;
		return NULL;
	}

	/**
	* @brief 读取整个文件，并以GZip压缩，带缓存
	* @param sFile		文件名
	* @param len		返回读取的长度，可为空
	* @return 返回缓冲指针，需要调用FreeBuf释放
	*/
	virtual void *ReadWholeGZipFile(LPCTSTR sFile,UINT *len=NULL)
	{
		if (len) *len=0;
		int idx=OpenFile(sFile);
		if (idx==-1) return NULL;
		void *buf=ReadWholeGZipFile(idx,len);
		CloseFile(idx);
		return buf;
	}

	/**
	* @brief 读取整个文件，并以GZip压缩，带缓存
	* @param idx		文件索引号
	* @param len		返回读取的长度，可为空
	* @return 返回缓冲指针，需要调用FreeBuf释放
	*/
	virtual void *ReadWholeGZipFile(int idx,UINT *len=NULL)
	{
		void *ret=NULL;
		unsigned long glen=0;
		UINT tlen;
		void *tmp=ReadWholeFile(idx,&tlen);
		if (tmp)
		{
			void *gret;
			if (THGZip::ZipEncode(tmp,tlen,&gret,&glen,THGZIP_GZIP))
			{
				ret=new char[glen];
				if (ret)
					memcpy(ret,gret,glen);
				else
					glen=0;
				THGZip::Free(gret);
			}
			FreeBuf(tmp);
		}
		if (len) *len=glen;
		return ret;
	}

	/**
	* @brief 释放ReadWholeFile和ReadWholeGZipFile申请的内存
	* @param buf		缓冲指针
	*/
	virtual void FreeBuf(void *buf){if (buf) delete [] buf;}

	/**
	* @brief 写文件，带缓存
	* @param idx		文件索引号
	* @param buf		缓冲指针
	* @param len		缓冲长度
	* @param pos		写入位置,-1为不指定,即当前位置
	* @return 返回写入字节数
	*/
	virtual UINT WriteFile(int idx,const void *buf,UINT len,int pos=-1)
	{
		if (m_bUseMemCache)
		{
			if (pos==-1) pos=GetPosition(idx);
			int bufidx=m_idxmap[idx];
			if (m_cache.BufferedWrite(bufidx,(UINT)pos,buf,len,(void *)(INT_PTR)idx))
			{
				m_curoff.SetAt(idx,pos+len);
				return len;
			}
			return 0;
		}
		else
		{
			return _WriteFile(idx,buf,len,pos);
		}
	}

	/**
	* @brief 写入整个文件，带缓存
	* @param sFile		文件名
	* @param buf		数据
	* @param len		长度
	* @return 是否成功
	*/
	virtual BOOL WriteWholeFile(LPCTSTR sFile,const void *buf,UINT len)
	{
		int idx=OpenFile(sFile,FILEMODE_ALWAYSCREATE);
		if (idx==-1) return FALSE;
		BOOL bret=WriteWholeFile(idx,buf,len);
		CloseFile(idx);
		return bret;
	}

	/**
	* @brief 写入整个文件，带缓存
	* @param idx		文件索引号
	* @param buf		数据
	* @param len		长度
	* @return 是否成功
	*/
	virtual BOOL WriteWholeFile(int idx,const void *buf,UINT len)
	{
		if (buf && len>0)
		{
			SeekFile(idx,SEEK_SET,0);
			return (WriteFile(idx,buf,len)==len);
		}
		return FALSE;
	}

/*	/**
	* @brief 新建对应的ITHIni对象
	* @return ITHIni对象实例指针
	* /
	virtual ITHIni* CreateIniObject(THString sPath)=0;

	/**
	* @brief 释放对应的ITHIni对象
	* /
	virtual void ReleaseIniObject(ITHIni *pIni)=0;
*/
//ini operate
	/**
	* @brief 获取字符串，除了THFileStore外，默认Ini处理函数
	* @param sFile		Ini配置文件名
	* @param sApp		段名称
	* @param sKey		字段名称
	* @param sDefValue	默认值
	* @return 获取的值
	*/
	virtual THString IniGetString(THString sFile,THString sApp,THString sKey,THString sDefValue=_T(""))
	{
		THDoubleStringMap<int> obj;
		THStringArray ar;
		if (ParseIniObject(sFile,obj,ar))
		{
			THPosition pos=obj.GetStartPosition();
			THString ss,sk;
			int idx;
			while(!pos.IsEmpty())
			{
				if (obj.GetNextPosition(pos,ss,sk,idx))
				{
					if (ss.CompareNoCase(sApp)==0 && sk.CompareNoCase(sKey)==0)
						return ar[idx];
				}
			}
		}
		return sDefValue;
	}

	/**
	* @brief 保存字符串，除了THFileStore外，默认Ini处理函数
	* @param sFile		Ini配置文件名
	* @param sApp		段名称
	* @param sKey		字段名称
	* @param sValue		值
	* @return 是否成功
	*/
	virtual BOOL IniWriteString(THString sFile,THString sApp,THString sKey,THString sValue)
	{
		THDoubleStringMap<int> obj;
		THStringArray ar;
		int saveidx=-1;
		if (ParseIniObject(sFile,obj,ar))
		{
			THPosition pos=obj.GetStartPosition();
			THString ss,sk;
			int idx;
			while(!pos.IsEmpty())
			{
				if (obj.GetNextPosition(pos,ss,sk,idx))
				{
					if (ss.CompareNoCase(sApp)==0 && sk.CompareNoCase(sKey)==0)
					{
						saveidx=idx;
						ar[idx]=sValue;
						break;
					}
				}
			}
		}
		if (saveidx==-1)
		{
			saveidx=(int)ar.Add(sValue);
			obj.SetAt(sApp,sKey,saveidx);
		}
		return SaveIniObject(sFile,obj,ar);
	}

	/**
	* @brief 获取二进制数据，除了THFileStore外，默认Ini处理函数
	* @param sFile		Ini配置文件名
	* @param sApp		段名称
	* @param sKey		字段名称
	* @param pBuffer	数据指针
	* @param uSize		数据大小
	* @return 获取的值
	*/
	virtual BOOL IniGetBin(THString sFile,THString sApp,THString sKey,void *pBuffer,UINT uSize)
	{
		THString ret=IniGetString(sFile,sApp,sKey);
		if (ret.IsEmpty()) return FALSE;
		if (ret.GetLength()!=(uSize+1)*2 || ret.GetLength()<=2) return FALSE;
		U8 sum=0;
		UINT len=sizeof(U8);
		if (!THStringConv::HexStringToBin(ret.Mid(ret.GetLength()-2),&sum,&len)) return FALSE;
		len=uSize;
		if (!THStringConv::HexStringToBin(ret.Left(ret.GetLength()-2),pBuffer,&len)) return FALSE;
		if (len!=uSize) return FALSE;
		U8 sum1=0;
		for(UINT i=0;i<uSize;i++)
			sum1+=*((U8 *)pBuffer+i);
		return (sum1==sum);
	}

	/**
	* @brief 保存二进制数据，除了THFileStore外，默认Ini处理函数
	* @param sFile		Ini配置文件名
	* @param sApp		段名称
	* @param sKey		字段名称
	* @param pBuffer	数据指针
	* @param uSize		数据大小
	* @return 是否成功
	*/
	virtual BOOL IniWriteBin(THString sFile,THString sApp,THString sKey,void *pBuffer,UINT uSize)
	{
		if (!pBuffer || uSize==0) return FALSE;
		U8 *pbuf=(U8 *)pBuffer;
		THString sValue;
		U8 sum=0;
		for(UINT i=0;i<uSize;i++)
		{
			sValue.AppendFormat(_T("%02X"),*(pbuf+i));
			sum+=*(pbuf+i);
		}
		sValue.AppendFormat(_T("%02X"),sum);
		return IniWriteString(sFile,sApp,sKey,sValue);
	}

	/**
	* @brief 枚举文件中的所有段，除了THFileStore外，默认Ini处理函数
	* @param sFile		Ini配置文件名
	* @param arKey		返回键值名数组
	* @return 是否成功
	*/
	virtual BOOL IniEnumAllStruct(THString sFile,THStringArray *arKey)
	{
		if (!arKey) return FALSE;
		THDoubleStringMap<int> obj;
		THStringArray ar;
		if (ParseIniObject(sFile,obj,ar))
		{
			THPosition pos=obj.GetStartPosition();
			THString ss,sk;
			int idx;
			while(!pos.IsEmpty())
			{
				if (obj.GetNextPosition(pos,ss,sk,idx))
				{
					BOOL bFound=FALSE;
					for(int i=0;i<arKey->GetSize();i++)
					{
						if (ss.CompareNoCase(arKey->GetAt(i))==0)
						{
							bFound=TRUE;
							break;
						}
					}
					if (!bFound)
						arKey->Add(ss);
				}
			}
			return TRUE;
		}
		return FALSE;
	}

	/**
	* @brief 枚举段中的所有键值名，除了THFileStore外，默认Ini处理函数
	* @param sFile		Ini配置文件名
	* @param sKey		键值名
	* @param arKey		返回键值名数组
	* @param arValue	返回键值内容数组
	* @return 是否成功
	*/
	virtual BOOL IniEnumStruct(THString sFile,const THString sKey,THStringArray *arKey,THStringArray *arValue)
	{
		if (!arKey || !arValue) return FALSE;
		THDoubleStringMap<int> obj;
		THStringArray ar;
		if (ParseIniObject(sFile,obj,ar))
		{
			THPosition pos=obj.GetStartPosition();
			THString ss,sk;
			int idx;
			while(!pos.IsEmpty())
			{
				if (obj.GetNextPosition(pos,ss,sk,idx))
				{
					if (ss.CompareNoCase(sKey)==0)
					{
						arKey->Add(sk);
						arValue->Add(ar[idx]);
					}
				}
			}
			return TRUE;
		}
		return FALSE;
	}

	/**
	* @brief 设置缓存参数
	* @param CacheSize	缓存大小，当大小为0时，表示不使用缓存
	* @param sKey		键值名
	* @param arKey		返回键值名数组
	* @param arValue	返回键值内容数组
	* @return 是否成功
	*/
	virtual void SetMemCacheSettings(UINT CacheSize)
	{
		if (CacheSize==0)
		{
			m_bUseMemCache=FALSE;
			m_cache.EmptyAllBuffer();
			//later me,is it ok?改变方式会影响缓存文件的指针位置m_curoff
		}
		else 
		{
			m_bUseMemCache=TRUE;
			if (CacheSize<MEMCACHE_SIZE*10)
				m_cache.SetLimitCount(10);
			else
				m_cache.SetLimitCount(CacheSize/MEMCACHE_SIZE);
		}
	}
	virtual void EmptyCache(){m_cache.EmptyAllBuffer();}
//filedialog settings
	virtual THString FileDialogOpenExistOne(HWND hOwner=NULL,THString sFileName=_T(""),THString sFilter=_T("所有文件(*.*)|*.*||"),THString sExt=_T(""),THString sInitDir=_T(""),THString sTitle=_T(""))=0;
	virtual THString FileDialogSaveOne(HWND hOwner=NULL,THString sFileName=_T("未命名"),THString sFilter=_T("所有文件(*.*)|*.*||"),THString sExt=_T(""),THString sInitDir=_T(""),THString sTitle=_T(""))=0;
	virtual THString DirDialog(HWND hWnd=NULL,THString sTitle=_T("请选择目录"))=0;
	virtual THString BaseDirDialog(HWND hWnd=NULL,THString sTitle=_T("请选择基准路径"))=0;
protected:
	virtual UINT _GetRealSeekPos(int idx,int nStart,int nPos)
	{
		//laterme,是否有性能问题
		UINT tlen=GetFileSize(idx);
		UINT retpos=0;
		if (nStart==SEEK_SET)
			retpos=(nPos<0)?0:nPos;
		else if (nStart==SEEK_CUR)
		{
			UINT curpos=GetPosition(idx);
			if (nPos>=0)
				retpos=curpos+nPos;
			else
				retpos=(curpos<(UINT)(-nPos))?0:curpos-((UINT)(-nPos));
		}
		else if (nStart==SEEK_END)
		{
			if (nPos<0 && tlen<(UINT)(-nPos))
				retpos=0;
			else
				retpos=tlen+nPos;
		}
		if (retpos>tlen) retpos=tlen;
		return retpos;
	}

	//注意，调用后obj就会被修改删除
	virtual BOOL SaveIniObject(THString sFile,THDoubleStringMap<int> &obj,THStringArray &ar)
	{
		THString ret;
		THString ss,ss1,sk;
		int idx;
		while(1)
		{
			THPosition pos=obj.GetStartPosition();
			if (pos.IsEmpty()) break;
			if (!obj.GetNextPosition(pos,ss,sk,idx)) break;
			ret.AppendFormat(_T("[%s]\r\n%s=%s\r\n"),ss,sk,ar[idx]);
			obj.RemoveAt(ss,sk);
			pos=obj.GetStartPosition();
			while(!pos.IsEmpty())
			{
				if (obj.GetNextPosition(pos,ss1,sk,idx))
				{
					if (ss.CompareNoCase(ss1)==0)
					{
						ret.AppendFormat(_T("%s=%s\r\n"),sk,ar[idx]);
						obj.RemoveAt(ss1,sk);
						pos=obj.GetStartPosition();
					}
				}
			}
			ret.AppendFormat(_T("\r\n"));
		}
		if (ret.IsEmpty()) ret=_T("\r\n");
		char *tmp=THCharset::t2a(ret);
		BOOL bret=FALSE;
		if (tmp)
		{
			bret=WriteWholeFile(sFile,tmp,(UINT)strlen(tmp));
			THCharset::free(tmp);
		}
		return bret;
	}

	virtual BOOL ParseIniObject(THString sFile,THDoubleStringMap<int> &obj,THStringArray &ar)
	{
		UINT len;
		void *buf=ReadWholeFile(sFile,&len);
		if (buf && len>0)
		{
			THString allbuf=THCharset::a2t((char *)buf,len);
			FreeBuf(buf);
			//0 for search key
			//1 for search content
			int type=0;
			THStringArray ret;
			while(!allbuf.IsEmpty())
			{
				if (type==0)
				{
					ret.RemoveAll();
					if (!THMatchSearch::MatchSearch(&allbuf,_T("[%s]"),&ret)) break;
					ret[0].Trim();
					if (!ret[0].IsEmpty()) type=1;
				}
				if (type==1)
				{
					int end=allbuf.Find(_T("["),0);
					if (end==-1) end=allbuf.GetLength();
 					THString str=allbuf.Left(end);
					str.Replace(_T("\r\n"),_T("\n"));
					str.Replace(_T("\r"),_T("\n"));
					THStringToken t(str,_T("\n"));
					THStringArray ar1;
					while(t.IsMoreTokens())
					{
                        str=t.GetNextToken();
						if (THMatchSearch::MatchSearch(&str,_T("%s=%s"),&ar1))
						{
							ar1[0].Trim();
							ar1[1].Trim();
							//the value(ar1[1]) can be empty
							if (!ar1[0].IsEmpty())
							{
								int idx=(int)ar.Add(ar1[1]);
								obj.SetAt(ret[0],ar1[0],idx);
							}
						}
					}
					allbuf=allbuf.Mid(end);
					if (allbuf.IsEmpty()) break;
					type=0;
				}
			}
			return TRUE;
		}
		return FALSE;
	}

	/**
	* @brief 真实接口的打开文件
	* @param sFile		文件
	* @param nMode		打开方式，FILEMODE_XXX
	* @return 返回文件索引号，-1为打开失败
	*/
	virtual int _OpenFile(THString sFile,int nMode=FILEMODE_READ)=0;

	/**
	* @brief 真实接口的关闭文件
	* @param idx		文件索引
	*/
	virtual void _CloseFile(int idx)=0;

	/**
	* @brief 真实接口的关闭所有打开文件
	*/
	virtual void _CloseAllFile()=0;

	/**
	* @brief 真实接口的获取当前文件位置
	* @return 返回文件当前位置
	*/
	virtual UINT _GetPosition(int idx)=0;

	/**
	* @brief 真实接口的定位文件
	* @param idx		文件索引号
	* @param nStart		开始方式，详见SEEK_XX
	* @param nPos		到达偏移
	* @return 返回实际定位位置
	*/
	virtual UINT _SeekFile(int idx,int nStart,int nPos)=0;

	/**
	* @brief 真实接口的读文件
	* @param idx		文件索引号
	* @param buf		缓冲指针
	* @param len		缓冲长度
	* @param pos		读取位置,-1为不指定,即当前位置
	* @return 返回实际读取长度
	*/
	virtual UINT _ReadFile(int idx,void *buf,UINT len,int pos=-1)=0;

	/**
	* @brief 真实接口的写文件
	* @param idx		文件索引号
	* @param buf		缓冲指针
	* @param len		缓冲长度
	* @param pos		写入位置,-1为不指定,即当前位置
	* @return 返回写入字节数
	*/
	virtual UINT _WriteFile(int idx,const void *buf,UINT len,int pos=-1)=0;

	virtual BOOL _CreateDirectory(THString sPath)=0;
	virtual BOOL _DeleteDirectory(THString sPath)=0;
	virtual BOOL _CopyFile(THString sPath,THString sNewPath,ITHFileStore *fs=NULL)
	{
		if (fs==NULL) fs=this;
		int ridx=OpenFile(sPath,FILEMODE_READ);
		if (ridx==-1) return FALSE;
		int widx=fs->OpenFile(sNewPath,FILEMODE_ALWAYSCREATE);
		if (widx==-1)
		{
			CloseFile(ridx);
			return FALSE;
		}
		UINT len=GetFileSize(ridx);
		char buf[4096];
		UINT r;
		BOOL bRet=TRUE;
		while(len>0)
		{
			r=min(len,4096);
			r=ReadFile(ridx,buf,r);
			if (r==0) break;
			if (fs->WriteFile(widx,buf,r)!=r)
			{
				bRet=FALSE;
				break;
			}
			len-=r;
		}
		CloseFile(ridx);
		fs->CloseFile(widx);
		return TRUE;
	}
	virtual BOOL _MoveFile(THString sPath,THString sNewPath,ITHFileStore *fs=NULL)
	{
		if (!_CopyFile(sPath,sNewPath,fs)) return FALSE;
		return DeleteFile(sPath);
	}
	virtual BOOL _DeleteFile(THString sPath)=0;

	virtual BOOL BufferNeedRead(int bufidx,UINT off,void *buf,UINT &len,void *adddata,void *curadddata)
	{
		//使用adddata中的idx作为idx，因为缓存系统中，会把idx使用相同的bufidx来增加缓存击中率
		int idx=(int)(INT_PTR)curadddata;
		len=_ReadFile(idx,buf,len,off);
		return TRUE;
	}

	virtual BOOL BufferNeedWrite(int bufidx,UINT off,const void *buf,UINT len,void *adddata,void *curadddata)
	{
		//使用adddata中的idx作为idx，因为缓存系统中，会把idx使用相同的bufidx来增加缓存击中率
		int idx=(int)(INT_PTR)curadddata;
		len=_WriteFile(idx,buf,len,off);
		return TRUE;
	}

	THString m_basepath;
	void *m_freeobj;
	//mem cache settings
	BOOL m_bUseMemCache;
	//文件名对应的文件储存索引
	//string=filename,int=bufidx
	THStringMap<int> m_namemap;
	//因为多个idx可能共用同一个文件，所以使用idx转换将会把相同的文件缓存到同一缓冲空间
	//多个打开的文件句柄对应的文件储存索引
	//int=fileidx,int=bufidx
	THMap<int,int> m_idxmap;
	THMemCache<MEMCACHE_SIZE,MEMCACHE_COUNT> m_cache;
	//在文件打开和关闭的时候，处理m_idxmap和m_namemap，在所有pos改变的时候，处理m_curoff
	//处理使用缓存系统时，文件的seekpos没有更新的问题
	//int=fileidx,UINT=curoff
	THMap<int,UINT> m_curoff;
	//fixme 文件改变的change notify
};

//Add Support FileStore Handler
#include "THFileStore.inc"
#include "THFtpFileStore.inc"
#include "THHttpFileStore.inc"
#include "THResFileStore.inc"
#include "THImgFileStore.inc"

#define CheckStore(store) {store s;\
						if (s.GetStoreName()==sStoreName){\
						ps=s.CreateObject();\
						if (!ps) return NULL;\
						ps->SetBaseDirectory(sBasePath);\
						return ps;}}
/**
* @brief 文件储存器类与字符串转换，用于信息保存，THFileStoreFile2String生成的String也可以使用该类读取，兼容的
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-07-26 新建类
*/
class THFileStore2String
{
public:

	static ITHFileStore *String2FileStore(THString str)
	{
		if (str.IsEmpty()) return NULL;
		THStringToken t(str,_T("|"));
		THString sStoreName=t.GetNextToken();
		THString sBasePath=t.GetNextToken();
		ITHFileStore *ps;
		//Add Support FileStore Handler
		CheckStore(THFileStore);
		CheckStore(THResFileStore);
		CheckStore(THImgFileStore);
		return NULL;
	}

	static THString FileStore2String(ITHFileStore *st)
	{
		THString ret;
		if (st)
		{
			ret=st->GetStoreName()+_T("|")+st->GetBaseDirectory();
		}
		return ret;
	}
};

//fixme , FileStore Select Dialog

#define CheckFile(store) {store s;\
						if (s.GetStoreName()==sStoreName){\
						ps=s.CreateObject();\
						if (!ps) return FALSE;\
						ps->SetBaseDirectory(sBasePath);\
						UINT len;\
						void *buf=ps->ReadWholeFile(sFilePath,&len);\
						if (!buf) return FALSE;\
						mbuf->Empty();\
						mbuf->AddBuf(buf,len);\
						ps->FreeBuf(buf);\
						delete ps;\
						return TRUE;}}

/**
* @brief 文件储存器中文件与字符串转换，用于信息保存
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-08-22 新建类
*/
class THFileStoreFile2String
{
public:

	static BOOL String2FileStoreFile(THString str,THMemBuf *mbuf)
	{
		if (str.IsEmpty() || !mbuf) return FALSE;
		THStringToken t(str,_T("|"));
		THString sStoreName=t.GetNextToken();
		THString sBasePath=t.GetNextToken();
		THString sFilePath=t.GetNextToken();
		ITHFileStore *ps;
		//Add Support FileStore Handler
		CheckFile(THFileStore);
		CheckFile(THResFileStore);
		CheckFile(THImgFileStore);
		return FALSE;
	}

	static THString FileStoreFile2String(ITHFileStore *st,THString sFileName)
	{
		THString ret;
		if (st)
			ret=st->GetStoreName()+_T("|")+st->GetBaseDirectory()+_T("|")+st->GetRelPath(sFileName);
		return ret;
	}
};

#define AddStore(a) {a store;ITHFileStore *ps=store.CreateObject();ps->SetFreeor(&g_select);g_select.AddFS(ps);}

/**
* @brief 储存器选择类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-09-12 新建类
*/
class THFileStoreSelect
{
public:
	THFileStoreSelect()
	{
		m_list.SetFreeProc(FreeStoreList,this);
	}
	virtual ~THFileStoreSelect()
	{
		m_list.RemoveAll();
	}

	static THFileStoreSelect *GetInstance()
	{
		static BOOL bInit=FALSE;
		static THFileStoreSelect g_select;
		if (!bInit)
		{
			bInit=TRUE;
			//Add Support FileStore Handler
			AddStore(THFileStore);
			AddStore(THResFileStore);
			AddStore(THImgFileStore);
		}
		return &g_select;
	}

	static AddFileStore(ITHFileStore *store)
	{
		GetInstance()->AddFS(store);
	}

	//fsname 为选择的储存器名称
	static THString SelectOpenExistOneFile(THString &fsname,HINSTANCE hinst,HWND hWnd=NULL,THString sFileName=_T(""),THString sFilter=_T("所有文件(*.*)|*.*||"),THString sExt=_T(""),THString sInitDir=_T(""),THString sTitle=_T("请选择打开的文件"))
	{
		ITHFileStore *fs=SelectFileStore(hinst,hWnd);
		if (fs)
		{
			fsname=fs->GetStoreName();
			return fs->FileDialogOpenExistOne(hWnd,sFileName,sFilter,sExt,sInitDir,sTitle);
		}
		return _T("");
	}

	//fsname 为选择的储存器名称
	static THString SelectSaveFile(THString &fsname,HINSTANCE hinst,HWND hWnd=NULL,THString sFileName=_T("未命名"),THString sFilter=_T("所有文件(*.*)|*.*||"),THString sExt=_T(""),THString sInitDir=_T(""),THString sTitle=_T("请选择保存的文件"))
	{
		ITHFileStore *fs=SelectFileStore(hinst,hWnd);
		if (fs)
		{
			fsname=fs->GetStoreName();
			return fs->FileDialogSaveOne(hWnd,sFileName,sFilter,sExt,sInitDir,sTitle);
		}
		return _T("");
	}

	//调用后应调用setfreeor，管理fs的释放
	static ITHFileStore *SelectBaseDirectory(HINSTANCE hinst,HWND hWnd=NULL,THString sTitle=_T("请选择目录"))
	{
		ITHFileStore *fs=SelectFileStore(hinst,hWnd);
		if (fs)
		{
			THString sPath=fs->BaseDirDialog(hWnd,sTitle);
			if (sPath.IsEmpty()) return NULL;
			fs=fs->CreateObject();
			fs->SetBaseDirectory(sPath);
			return fs;
		}
		return NULL;
	}

	static ITHFileStore* SelectFileStore(HINSTANCE hinst,HWND hWnd=NULL,THString sTitle=_T("请选择需要使用的文件系统"))
	{
		return GetInstance()->SelectFS(hinst,hWnd,sTitle);
	}

	static void GetFileStoreNames(THStringArray *ar)
	{
		GetInstance()->GetFSNames(ar);
	}
protected:
	THList<ITHFileStore *> m_list;
	static void FreeStoreList(void *key,void *value,void *adddata)
	{
		((ITHFileStore *)key)->DeleteObjectByFreeor(adddata);
	}
	ITHFileStore* SelectFS(HINSTANCE hinst,HWND hWnd,THString sTitle)
	{
		THString sItemData=_T("<selected0>1</selected0>");
		THPosition pos=m_list.GetStartPosition();
		int i=0;
		ITHFileStore *fs;
		THMap<int,ITHFileStore *>map;
		while(!pos.IsEmpty())
		{
			if (m_list.GetNextPosition(pos,fs))
			{
				sItemData.AppendFormat(_T("<item%d>%s</item%d>"),i,fs->GetStoreName(),i);
				map.SetAt(i,fs);
				i++;
			}
		}
		//if only one fs,just return it
		if (i==1) return map[0];
		THString sRet;
		THBaseMulSelectDialog dlg;
		if (!dlg.ShowMulSelectDialog(hinst,hWnd,sTitle,sItemData,&sRet,i,_T("请选择文件系统："),MULSELECTDIALOGTYPE_RADIOBUTTON)==IDOK) return NULL;
		for(int j=0;j<i;i++)
		{
			if (THs2i(THSimpleXml::GetParam(sRet,THFormat(_T("ret%d"),j))))
				return map[j];
		}
		return NULL;
	}
	void AddFS(ITHFileStore *store)
	{
		if (!store) return;
		THString storename=store->GetStoreName();
		THPosition pos=m_list.GetStartPosition();
		ITHFileStore *fs;
		while(!pos.IsEmpty())
		{
			if (m_list.GetNextPosition(pos,fs))
			{
				if (fs->GetStoreName()==storename) return;
			}
		}
		m_list.AddTail(store);
	}
	void GetFSNames(THStringArray *ar)
	{
		if (!ar) return;
		THPosition pos=m_list.GetStartPosition();
		ITHFileStore *fs;
		while(!pos.IsEmpty())
		{
			if (m_list.GetNextPosition(pos,fs))
				ar->Add(fs->GetStoreName());
		}
	}
};