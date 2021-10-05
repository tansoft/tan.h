#pragma once

//#include <THSystem.h>
//#include <THString.h>

/**
* @brief 共享内存影射类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-06-19 新建类
*/
/**<pre>
  使用Sample：
	THMapFile map;
	map.Init(_T("testname"),123);
	char buffer[123];
	map.GetBuffer(buffer,123,0);
	map.SetBuffer(buffer);
	map.SetBuffer(buffer,123,0);

	LPVOID buf=map.MapBuffer();
	//.....
	map.UnMapBuffer(buf);
</pre>*/
class THMapFile
{
public:
	THMapFile(const TCHAR *sMapName,DWORD dSize)
	{
		m_hObject=NULL;
		m_pMap=NULL;
		Init(sMapName,dSize);
	}
	THMapFile()
	{
		m_dSize=0;
		m_pMap=NULL;
		m_hObject=NULL;
		m_sMapName[0]='\0';
	}
	virtual ~THMapFile()
	{
		Close();
	}

	void Close()
	{
		UnMapBuffer();
		if (m_hObject)
		{
			CloseHandle(m_hObject);
			m_hObject=NULL;
		}
	}

	BOOL Init(const TCHAR *sMapName,DWORD dSize)
	{
		_tcscpy(m_sMapName,sMapName);
		m_dSize=dSize;
		return TRUE;
	}

	LPVOID MapBuffer()
	{
		if (m_pMap) return m_pMap;
		BOOL bNew=FALSE;
		HANDLE hmap=OpenFileMapping(FILE_MAP_ALL_ACCESS,FALSE,m_sMapName);
		if (!hmap)
		{
			hmap=CreateFileMapping(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,m_dSize,m_sMapName);
			if (!hmap) return NULL;
			m_hObject=hmap;
			bNew=TRUE;
		}
		LPVOID buf=MapViewOfFile(hmap,FILE_MAP_ALL_ACCESS,0,0,0);
		if (!buf) return NULL;
		if (bNew) memset(buf,0,m_dSize);
		return buf;
	}

	void UnMapBuffer()
	{
		if (!m_pMap) return;
		UnmapViewOfFile(m_pMap);
		m_pMap=NULL;
	}

	int GetBuffer(void *pBuffer,DWORD nLen,DWORD nOffest)
	{
		if (!pBuffer || nOffest>=m_dSize) return 0;
		if (nLen==-1)
			nLen=m_dSize-nOffest;
		if (m_pMap)
		{
			//buffer is locking
			memcpy(pBuffer,(char *)m_pMap+nOffest,nLen);
		}
		else
		{
			LPVOID lpBuf=MapBuffer();
			if(!lpBuf) return FALSE;
			memcpy(pBuffer,(char *)lpBuf+nOffest,nLen);
			UnmapViewOfFile(lpBuf);
		}
		return nLen;
	}

	BOOL SetBuffer(const void *buffer)
	{
		return SetBuffer(buffer,m_dSize,0);
	}

	BOOL SetBuffer(const void *buffer,DWORD nLen,DWORD nOffest)
	{
		if (!buffer || nOffest+nLen>m_dSize) return FALSE;
		if (m_pMap)
		{
			//buffer is locking
			memcpy((char *)m_pMap+nOffest,buffer,nLen);
		}
		else
		{
			LPVOID lpBuf=MapBuffer();
			if(!lpBuf) return FALSE;
			memcpy((char *)lpBuf+nOffest,buffer,nLen);
			UnmapViewOfFile(lpBuf);
		}
		return TRUE;
	}

private:
	HANDLE m_hObject;
	TCHAR m_sMapName[1024];
	DWORD m_dSize;
	LPVOID m_pMap;
};
