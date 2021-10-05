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

//ע�� FILEMODE_ �Ƕ���ʹ�õģ�FILEMODE_MASK������
#define FILEMODE_READ					1		//��
#define FILEMODE_READWRITE				2		//��д
#define FILEMODE_CREATEOREXIST			3		//��д��������򿪣��������½�
#define FILEMODE_ALWAYSCREATE			4		//��д�������½�
#define FILEMODE_MASK					0x0ff	//�ļ��򿪷�ʽMask
#define FILEMODE_MASK_SHAREEXCLUSIVE	0x100	//����ʹ�ã���������ʽ���ļ�

/**
* @brief �ļ�������������
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-09-07 �½���
*/
/**<pre>
	����SetFreeor��DeleteObjectByFreeor�����ͷ�����
	���紫��ITHFileStore����a������ϣ������a���ͷţ�����ǰ����SetFreeor���ö���ָ��Ϊ��a����a���ͷ�ʱ���ǵ���DeleteObjectByFreeor������this
	ϵͳ�е���THIni��THScript��THServer�ȵ���ʱֻ��Ҫָ������bAutoFree�����л��Զ�����SetFreeor��DeleteObjectByFreeor
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
	* @brief �����ͷŴ������Ķ���
	* @param freeor		����ָ��
	*/
	virtual void SetFreeor(void *freeor){m_freeobj=freeor;}

	/**
	* @brief �����ͷŴ�����
	* @param freeor		����ָ��
	*/
	virtual void DeleteObjectByFreeor(void *freeor){if (freeor==m_freeobj) delete this;}

	/**
	* @brief �½�����������
	* @return �½�����ʵ�������������ʵ��ָ��
	*/
	virtual ITHFileStore *CreateObject()=0;

	/**
	* @brief ���û���Ŀ¼
	* @param path		����Ŀ¼��֧��/��Ŀ¼��ʾ
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
	* @brief ��ȫ·����ȡ���·��
	* @param fullpath		���Ŀ¼������Ŀ¼��֧��/��Ŀ¼��ʾ��֧���ļ�ֱ�Ӵ���
	* @param bReversalSign	�Ƿ񷵻�'/'��ʽ
	* @return ���·��
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
	* @brief �����Ŀ¼��ȡȫ·��
	* @param relpath		���Ŀ¼������Ŀ¼��֧��/��Ŀ¼��ʾ��֧���ļ�ֱ�Ӵ���
	* @param curpath		��ǰĿ¼��֧�����Ŀ¼������Ŀ¼�����ڼ������Ŀ¼
	* @return ȫ·��
	*/
	virtual THString GetFullPath(THString relpath,THString curpath=_T(""))
	{
		//û�д���·���г��ֵ�..\\��.\\��ֻ�д���ͷ������Щ�ַ������
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
	* @brief ��ȡ�ļ�����·���������������·������parentpath
	* @param sPath		�ļ���
	* @return �ļ�·��
	*/
	virtual THString GetCurPath(THString sPath)
	{
		return GetParentPath(sPath);
	}

	/**
	* @brief ��ȡȫ·���е��ļ���
	* @param sPath		ȫ·���ļ���
	* @return �ļ���
	*/
	virtual THString GetFileBaseName(THString sPath)
	{
		sPath.Replace(_T("/"),_T("\\"));
		int pos=sPath.ReverseFind('\\');
		if (pos!=-1) sPath=sPath.Mid(pos+1);
		return sPath;
	}

	/**
	* @brief ��ȡ�ļ���׺��������Сд
	* @param sPath		�ļ���
	* @return �ļ�·��
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
	* @brief ����ϵͳ����
	* @return ����ϵͳ����
	*/
	virtual THString GetStoreName()=0;

	/**
	* @brief ·���Ƿ�ΪĿ¼
	* @param sPath		·��
	* @return �Ƿ�ΪĿ¼
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
	* @brief ·���Ƿ�Ϊ�ļ�
	* @param sPath		·��
	* @return �Ƿ�Ϊ�ļ�
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
	* @brief ����Ŀ¼���༶Ŀ¼Ҳ֧�ִ���
	* @param sPath		·��
	* @return �Ƿ�ɹ�
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
	* @brief ɾ��Ŀ¼���༶Ŀ¼Ҳ֧��ɾ��
	* @param sPath		·��
	* @return �Ƿ�ɹ�
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
	* @brief ������֧��Ŀ¼���ļ�
	* @param sPath		ԭ����
	* @param sNewName	�����֣�����ҪĿ¼
	* @return �Ƿ�ɹ�
	*/
	virtual BOOL Rename(THString sPath,THString sNewName)
	{
		THString sParentPath=GetParentPath(sPath);
		return MoveFile(sPath,sParentPath+_T("\\")+sNewName);
	}

	/**
	* @brief ����Ŀ¼���ļ�
	* @param sPath		ԭ�����ļ�Ϊ��һ���ƣ�Ŀ¼ΪȫĿ¼����
	* @param sNewPath	����Ŀ��
	* @param fs			����Ŀ����ļ�ϵͳ��ΪNULLΪ��ǰ�ļ�ϵͳ
	* @return �Ƿ�ɹ�
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
	* @brief �ƶ�Ŀ¼���ļ�
	* @param sPath		ԭ�����ļ�Ϊ��һ�ƶ���Ŀ¼ΪȫĿ¼�ƶ�
	* @param sNewPath	�ƶ�Ŀ��
	* @param fs			�ƶ�Ŀ����ļ�ϵͳ��ΪNULLΪ��ǰ�ļ�ϵͳ
	* @return �Ƿ�ɹ�
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
	* @brief ɾ��Ŀ¼���ļ�
	* @param sPath		ԭ�����ļ���Ŀ¼
	* @return �Ƿ�ɹ�
	*/
	virtual BOOL DeleteFile(THString sPath)
	{
		sPath=GetFullPath(sPath);
		if (IsPathFile(sPath)) return _DeleteFile(sPath);
		else if (IsPathDir(sPath)) return DeleteDirectory(sPath);
		return TRUE;
	}

	/**
	* @brief �����ļ�
	* @param sPath		���ҵ�Ŀ¼
	* @param arDir		��Ŀ¼�������Ϊ�գ�����Ϊ�ļ���
	* @param arFile		�ļ��������Ϊ�գ�����Ϊ�ļ���
	* @return �Ƿ�ɹ�
	*/
	virtual BOOL FindFiles(THString sPath,THStringArray *arDir,THStringArray *arFile)=0;

	/**
	* @brief ��ȡ�ļ���Ϣ
	* @param sFile		ָ���ļ�
	* @param size		�ļ���С����Ϊ��
	* @param ct			�ļ��������ڣ���Ϊ��
	* @param mt			�ļ��޸����ڣ���Ϊ��
	* @return �Ƿ�Ϊ�ļ�
	*/
	virtual BOOL GetFileAttrib(THString sFile,UINT *size,THTime *ct,THTime *mt)=0;

	/**
	* @brief ���ļ�
	* @param sFile		�ļ�
	* @param nMode		�򿪷�ʽ��FILEMODE_XXX
	* @return �����ļ������ţ�-1Ϊ��ʧ��
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
	* @brief �ر��ļ�
	* @param idx		�ļ�����
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
	* @brief �ر����д��ļ�
	*/
	virtual void CloseAllFile()
	{
		m_idxmap.RemoveAll();
		m_curoff.RemoveAll();
		_CloseAllFile();
	}

	/**
	* @brief ��ȡ�ļ�����
	* @return �����ļ�����
	*/
	virtual UINT GetFileSize(int idx)=0;

	/**
	* @brief ��ȡ��ǰ�ļ�λ��
	* @return �����ļ���ǰλ��
	*/
	virtual UINT GetPosition(int idx)
	{
		if (m_bUseMemCache)
			return m_curoff[idx];
		return _GetPosition(idx);
	}

	/**
	* @brief ��λ�ļ�
	* @param idx		�ļ�������
	* @param nStart		��ʼ��ʽ�����SEEK_XX
	* @param nPos		����ƫ��
	* @return ����ʵ�ʶ�λλ��
	*/
	virtual UINT SeekFile(int idx,int nStart,int nPos)
	{
		UINT off=_SeekFile(idx,nStart,nPos);
		if (m_bUseMemCache)
			m_curoff.SetAt(idx,off);
		return off;
	}

	/**
	* @brief ��ȡ�ļ���������
	* @param idx		�ļ�������
	* @param buf		����ָ��
	* @param len		���峤��
	* @param pos		��ȡλ��,-1Ϊ��ָ��,����ǰλ��
	* @return ����ʵ�ʶ�ȡ����
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
	* @brief ��ȡ�����ļ���������
	* @param sFile		�ļ���
	* @param len		���ض�ȡ�ĳ��ȣ���Ϊ��
	* @return ���ػ���ָ�룬��Ҫ����FreeBuf�ͷ�
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
	* @brief ��ȡ�����ļ���������
	* @param idx		�ļ�������
	* @param len		���ض�ȡ�ĳ��ȣ���Ϊ��
	* @return ���ػ���ָ�룬��Ҫ����FreeBuf�ͷ�
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
	* @brief ��ȡ�����ļ�������GZipѹ����������
	* @param sFile		�ļ���
	* @param len		���ض�ȡ�ĳ��ȣ���Ϊ��
	* @return ���ػ���ָ�룬��Ҫ����FreeBuf�ͷ�
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
	* @brief ��ȡ�����ļ�������GZipѹ����������
	* @param idx		�ļ�������
	* @param len		���ض�ȡ�ĳ��ȣ���Ϊ��
	* @return ���ػ���ָ�룬��Ҫ����FreeBuf�ͷ�
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
	* @brief �ͷ�ReadWholeFile��ReadWholeGZipFile������ڴ�
	* @param buf		����ָ��
	*/
	virtual void FreeBuf(void *buf){if (buf) delete [] buf;}

	/**
	* @brief д�ļ���������
	* @param idx		�ļ�������
	* @param buf		����ָ��
	* @param len		���峤��
	* @param pos		д��λ��,-1Ϊ��ָ��,����ǰλ��
	* @return ����д���ֽ���
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
	* @brief д�������ļ���������
	* @param sFile		�ļ���
	* @param buf		����
	* @param len		����
	* @return �Ƿ�ɹ�
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
	* @brief д�������ļ���������
	* @param idx		�ļ�������
	* @param buf		����
	* @param len		����
	* @return �Ƿ�ɹ�
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
	* @brief �½���Ӧ��ITHIni����
	* @return ITHIni����ʵ��ָ��
	* /
	virtual ITHIni* CreateIniObject(THString sPath)=0;

	/**
	* @brief �ͷŶ�Ӧ��ITHIni����
	* /
	virtual void ReleaseIniObject(ITHIni *pIni)=0;
*/
//ini operate
	/**
	* @brief ��ȡ�ַ���������THFileStore�⣬Ĭ��Ini������
	* @param sFile		Ini�����ļ���
	* @param sApp		������
	* @param sKey		�ֶ�����
	* @param sDefValue	Ĭ��ֵ
	* @return ��ȡ��ֵ
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
	* @brief �����ַ���������THFileStore�⣬Ĭ��Ini������
	* @param sFile		Ini�����ļ���
	* @param sApp		������
	* @param sKey		�ֶ�����
	* @param sValue		ֵ
	* @return �Ƿ�ɹ�
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
	* @brief ��ȡ���������ݣ�����THFileStore�⣬Ĭ��Ini������
	* @param sFile		Ini�����ļ���
	* @param sApp		������
	* @param sKey		�ֶ�����
	* @param pBuffer	����ָ��
	* @param uSize		���ݴ�С
	* @return ��ȡ��ֵ
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
	* @brief ������������ݣ�����THFileStore�⣬Ĭ��Ini������
	* @param sFile		Ini�����ļ���
	* @param sApp		������
	* @param sKey		�ֶ�����
	* @param pBuffer	����ָ��
	* @param uSize		���ݴ�С
	* @return �Ƿ�ɹ�
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
	* @brief ö���ļ��е����жΣ�����THFileStore�⣬Ĭ��Ini������
	* @param sFile		Ini�����ļ���
	* @param arKey		���ؼ�ֵ������
	* @return �Ƿ�ɹ�
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
	* @brief ö�ٶ��е����м�ֵ��������THFileStore�⣬Ĭ��Ini������
	* @param sFile		Ini�����ļ���
	* @param sKey		��ֵ��
	* @param arKey		���ؼ�ֵ������
	* @param arValue	���ؼ�ֵ��������
	* @return �Ƿ�ɹ�
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
	* @brief ���û������
	* @param CacheSize	�����С������СΪ0ʱ����ʾ��ʹ�û���
	* @param sKey		��ֵ��
	* @param arKey		���ؼ�ֵ������
	* @param arValue	���ؼ�ֵ��������
	* @return �Ƿ�ɹ�
	*/
	virtual void SetMemCacheSettings(UINT CacheSize)
	{
		if (CacheSize==0)
		{
			m_bUseMemCache=FALSE;
			m_cache.EmptyAllBuffer();
			//later me,is it ok?�ı䷽ʽ��Ӱ�컺���ļ���ָ��λ��m_curoff
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
	virtual THString FileDialogOpenExistOne(HWND hOwner=NULL,THString sFileName=_T(""),THString sFilter=_T("�����ļ�(*.*)|*.*||"),THString sExt=_T(""),THString sInitDir=_T(""),THString sTitle=_T(""))=0;
	virtual THString FileDialogSaveOne(HWND hOwner=NULL,THString sFileName=_T("δ����"),THString sFilter=_T("�����ļ�(*.*)|*.*||"),THString sExt=_T(""),THString sInitDir=_T(""),THString sTitle=_T(""))=0;
	virtual THString DirDialog(HWND hWnd=NULL,THString sTitle=_T("��ѡ��Ŀ¼"))=0;
	virtual THString BaseDirDialog(HWND hWnd=NULL,THString sTitle=_T("��ѡ���׼·��"))=0;
protected:
	virtual UINT _GetRealSeekPos(int idx,int nStart,int nPos)
	{
		//laterme,�Ƿ�����������
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

	//ע�⣬���ú�obj�ͻᱻ�޸�ɾ��
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
	* @brief ��ʵ�ӿڵĴ��ļ�
	* @param sFile		�ļ�
	* @param nMode		�򿪷�ʽ��FILEMODE_XXX
	* @return �����ļ������ţ�-1Ϊ��ʧ��
	*/
	virtual int _OpenFile(THString sFile,int nMode=FILEMODE_READ)=0;

	/**
	* @brief ��ʵ�ӿڵĹر��ļ�
	* @param idx		�ļ�����
	*/
	virtual void _CloseFile(int idx)=0;

	/**
	* @brief ��ʵ�ӿڵĹر����д��ļ�
	*/
	virtual void _CloseAllFile()=0;

	/**
	* @brief ��ʵ�ӿڵĻ�ȡ��ǰ�ļ�λ��
	* @return �����ļ���ǰλ��
	*/
	virtual UINT _GetPosition(int idx)=0;

	/**
	* @brief ��ʵ�ӿڵĶ�λ�ļ�
	* @param idx		�ļ�������
	* @param nStart		��ʼ��ʽ�����SEEK_XX
	* @param nPos		����ƫ��
	* @return ����ʵ�ʶ�λλ��
	*/
	virtual UINT _SeekFile(int idx,int nStart,int nPos)=0;

	/**
	* @brief ��ʵ�ӿڵĶ��ļ�
	* @param idx		�ļ�������
	* @param buf		����ָ��
	* @param len		���峤��
	* @param pos		��ȡλ��,-1Ϊ��ָ��,����ǰλ��
	* @return ����ʵ�ʶ�ȡ����
	*/
	virtual UINT _ReadFile(int idx,void *buf,UINT len,int pos=-1)=0;

	/**
	* @brief ��ʵ�ӿڵ�д�ļ�
	* @param idx		�ļ�������
	* @param buf		����ָ��
	* @param len		���峤��
	* @param pos		д��λ��,-1Ϊ��ָ��,����ǰλ��
	* @return ����д���ֽ���
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
		//ʹ��adddata�е�idx��Ϊidx����Ϊ����ϵͳ�У����idxʹ����ͬ��bufidx�����ӻ��������
		int idx=(int)(INT_PTR)curadddata;
		len=_ReadFile(idx,buf,len,off);
		return TRUE;
	}

	virtual BOOL BufferNeedWrite(int bufidx,UINT off,const void *buf,UINT len,void *adddata,void *curadddata)
	{
		//ʹ��adddata�е�idx��Ϊidx����Ϊ����ϵͳ�У����idxʹ����ͬ��bufidx�����ӻ��������
		int idx=(int)(INT_PTR)curadddata;
		len=_WriteFile(idx,buf,len,off);
		return TRUE;
	}

	THString m_basepath;
	void *m_freeobj;
	//mem cache settings
	BOOL m_bUseMemCache;
	//�ļ�����Ӧ���ļ���������
	//string=filename,int=bufidx
	THStringMap<int> m_namemap;
	//��Ϊ���idx���ܹ���ͬһ���ļ�������ʹ��idxת���������ͬ���ļ����浽ͬһ����ռ�
	//����򿪵��ļ������Ӧ���ļ���������
	//int=fileidx,int=bufidx
	THMap<int,int> m_idxmap;
	THMemCache<MEMCACHE_SIZE,MEMCACHE_COUNT> m_cache;
	//���ļ��򿪺͹رյ�ʱ�򣬴���m_idxmap��m_namemap��������pos�ı��ʱ�򣬴���m_curoff
	//����ʹ�û���ϵͳʱ���ļ���seekposû�и��µ�����
	//int=fileidx,UINT=curoff
	THMap<int,UINT> m_curoff;
	//fixme �ļ��ı��change notify
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
* @brief �ļ������������ַ���ת����������Ϣ���棬THFileStoreFile2String���ɵ�StringҲ����ʹ�ø����ȡ�����ݵ�
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-07-26 �½���
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
* @brief �ļ����������ļ����ַ���ת����������Ϣ����
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-08-22 �½���
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
* @brief ������ѡ����
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-09-12 �½���
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

	//fsname Ϊѡ��Ĵ���������
	static THString SelectOpenExistOneFile(THString &fsname,HINSTANCE hinst,HWND hWnd=NULL,THString sFileName=_T(""),THString sFilter=_T("�����ļ�(*.*)|*.*||"),THString sExt=_T(""),THString sInitDir=_T(""),THString sTitle=_T("��ѡ��򿪵��ļ�"))
	{
		ITHFileStore *fs=SelectFileStore(hinst,hWnd);
		if (fs)
		{
			fsname=fs->GetStoreName();
			return fs->FileDialogOpenExistOne(hWnd,sFileName,sFilter,sExt,sInitDir,sTitle);
		}
		return _T("");
	}

	//fsname Ϊѡ��Ĵ���������
	static THString SelectSaveFile(THString &fsname,HINSTANCE hinst,HWND hWnd=NULL,THString sFileName=_T("δ����"),THString sFilter=_T("�����ļ�(*.*)|*.*||"),THString sExt=_T(""),THString sInitDir=_T(""),THString sTitle=_T("��ѡ�񱣴���ļ�"))
	{
		ITHFileStore *fs=SelectFileStore(hinst,hWnd);
		if (fs)
		{
			fsname=fs->GetStoreName();
			return fs->FileDialogSaveOne(hWnd,sFileName,sFilter,sExt,sInitDir,sTitle);
		}
		return _T("");
	}

	//���ú�Ӧ����setfreeor������fs���ͷ�
	static ITHFileStore *SelectBaseDirectory(HINSTANCE hinst,HWND hWnd=NULL,THString sTitle=_T("��ѡ��Ŀ¼"))
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

	static ITHFileStore* SelectFileStore(HINSTANCE hinst,HWND hWnd=NULL,THString sTitle=_T("��ѡ����Ҫʹ�õ��ļ�ϵͳ"))
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
		if (!dlg.ShowMulSelectDialog(hinst,hWnd,sTitle,sItemData,&sRet,i,_T("��ѡ���ļ�ϵͳ��"),MULSELECTDIALOGTYPE_RADIOBUTTON)==IDOK) return NULL;
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