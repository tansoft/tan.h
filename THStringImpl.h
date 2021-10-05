#pragma once

#define THSYSTEM_INCLUDE_OLEAUTO
#define THSYSTEM_INCLUDE_TCHAR
#define THSYSTEM_INCLUDE_STDIO
#include <THSystem.h>
//#include <THSyn.h>

/**
* @brief Ansi字符集函数映射类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-03-20 新建类
*/
class THCharsetConvertMapA
{
public:
	static inline char *strcpy(char *src,const char *dist){return ::strcpy(src,dist);}
	static inline size_t strlen(const char *src){return ::strlen(src);}
	static inline char *strchr(const char *src,int c){return ::strchr(src,c);}
	static inline char *strlwr(char *src){return ::strlwr(src);}
	static inline char *strupr(char *src){return ::strupr(src);}
	static inline int strcmp(const char *str1,const char *str2){return ::strcmp(str1,str2);}
	static inline int stricmp(const char *str1,const char *str2){return ::stricmp(str1,str2);}
	static inline char *strstr(const char *str1,const char *str2){return ::strstr(str1,str2);}
	static inline char *strrchr(const char *str1,int ch){return ::strrchr(str1,ch);}
	static inline char *strncpy(char *src,const char *dist,size_t len){return ::strncpy(src,dist,len);}
	static inline char *strncat(char *src,const char *dist,size_t len){return ::strncat(src,dist,len);}
	static inline char *strcat(char *dist,const char *src){return ::strcat(dist,src);}
	static inline char *strpbrk(const char *src,const char *dist){return ::strpbrk(src,dist);}
	static inline long atol(const char *src){return ::atol(src);}
	static inline double atof(const char *src){return ::atof(src);}
	static inline int vsprintf(char *ret,const char *fmt,va_list list){return ::vsprintf(ret,fmt,list);}
	static inline BSTR SysAllocStringLen(const char *str,int len)
	{
		int nLen = ::MultiByteToWideChar(CP_ACP, 0, str, len, NULL, NULL);
		BSTR bstr = ::SysAllocStringLen(NULL, nLen);
		::MultiByteToWideChar(CP_ACP, 0, str, len, bstr, nLen);
		return bstr;
	}
};

/**
* @brief Unicode字符集函数映射类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-03-20 新建类
*/
class THCharsetConvertMapW
{
public:
	static inline wchar_t* strcpy(wchar_t *src,const wchar_t *dist){return ::wcscpy(src,dist);}
	static inline size_t strlen(const wchar_t *src){return ::wcslen(src);}
	static inline wchar_t *strchr(const wchar_t *src,int c){return ::wcschr(src,c);}
	static inline wchar_t *strlwr(wchar_t *src){return ::wcslwr(src);}
	static inline wchar_t *strupr(wchar_t *src){return ::wcsupr(src);}
	static inline int strcmp(const wchar_t *str1,const wchar_t *str2){return ::wcscmp(str1,str2);}
	static inline int stricmp(const wchar_t *str1,const wchar_t *str2){return ::wcsicmp(str1,str2);}
	static inline wchar_t *strstr(const wchar_t *str1,const wchar_t *str2){return ::wcsstr(str1,str2);}
	static inline wchar_t *strrchr(const wchar_t *str1,int ch){return ::wcsrchr(str1,ch);}
	static inline wchar_t *strncpy(wchar_t *src,const wchar_t *dist,size_t len){return ::wcsncpy(src,dist,len);}
	static inline wchar_t *strncat(wchar_t *src,const wchar_t *dist,size_t len){return ::wcsncat(src,dist,len);}
	static inline wchar_t *strcat(wchar_t *dist,const wchar_t *src){return ::wcscat(dist,src);}
	static inline wchar_t *strpbrk(const wchar_t *src,const wchar_t *dist){return ::wcspbrk(src,dist);}
	static inline long atol(const wchar_t *src){return ::_wtol(src);}
	static inline double atof(const wchar_t *src){return ::_wtof(src);}
	static inline int vsprintf(wchar_t *ret,const wchar_t *fmt,va_list list){return ::vswprintf(ret,fmt,list);}
	static inline BSTR SysAllocStringLen(const wchar_t *str,int len)
	{
		return ::SysAllocStringLen(str, len);
	}
};

#ifdef UNICODE
	typedef THCharsetConvertMapW THCharsetConvertMapT;
#else
	typedef THCharsetConvertMapA THCharsetConvertMapT;
#endif

#define DEFAULT_ALLOC_SIZE		256

/**
* @brief 字符串模板封装类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-03-20 新建类
*/
template <typename TCH,typename TCLS>
class THStringTempl
{
public:
	THStringTempl()
	{
		m_pStr=NULL;
		m_nBufLen=0;
		AllocBuffer(0);
	}
	THStringTempl(const THStringTempl<TCH,TCLS>& stringSrc)
	{
		m_pStr=NULL;
		m_nBufLen=0;
		SetString(stringSrc);
	}
	THStringTempl(TCH ch, UINT nRepeat = 1)
	{
		m_pStr=NULL;
		m_nBufLen=0;
		for(UINT i=0;i<nRepeat;i++)
			SetString(ch);
	}
	THStringTempl(const TCH *lpsz)
	{
		m_pStr=NULL;
		m_nBufLen=0;
		SetString(lpsz);
	}
	THStringTempl(const TCH *lpch, UINT nLength)
	{
		m_pStr=NULL;
		m_nBufLen=0;
		SetString(lpch,nLength);
	}
	/*THStringTempl(const unsigned TCH* psz)
	{
		m_pStr=NULL;
		m_nBufLen=0;
		SetString((const TCH *)psz);
	}*/
	~THStringTempl()
	{
		if (m_pStr)
		{
			delete [] m_pStr;
			m_pStr=NULL;
		}
		m_nBufLen=0;
	}

	int GetLength() const{return TCLS::strlen(m_pStr);}
	BOOL IsEmpty() const{return TCLS::strlen(m_pStr)==0;}
	void Empty(){m_pStr[0]='\0';}
	TCH GetAt(UINT nIndex) const{return m_pStr[nIndex];}
	TCH operator[](int nIndex) const{return m_pStr[nIndex];}
	void SetAt(UINT nIndex, TCH ch){m_pStr[nIndex]=ch;}
	operator const TCH *() const{return m_pStr;}

	const THStringTempl<TCH,TCLS>& operator=(const THStringTempl<TCH,TCLS>& stringSrc)
	{
		SetString((const TCH *)stringSrc);
		return(*this);
	}
	const THStringTempl<TCH,TCLS>& operator=(TCH ch)
	{
		SetString(ch);
		return(*this);
	}
	const THStringTempl<TCH,TCLS>& operator=(const TCH * lpsz)
	{
		SetString(lpsz);
		return(*this);
	}
	const THStringTempl<TCH,TCLS>& operator=(const unsigned char* psz)
	{
		SetString((const TCH *)psz);
		return(*this);
	}

	const THStringTempl<TCH,TCLS>& operator+=(const THStringTempl<TCH,TCLS>& string)
	{
		Append((const TCH *)string);
		return(*this);
	}
	const THStringTempl<TCH,TCLS>& operator+=(TCH ch)
	{
		Append(ch);
		return(*this);
	}
	const THStringTempl<TCH,TCLS>& operator+=(const TCH * lpsz)
	{
		Append(lpsz);
		return(*this);
	}

	int Compare(const TCH * lpsz) const{return TCLS::strcmp(m_pStr,lpsz);}
	int CompareNoCase(const TCH * lpsz) const{return TCLS::stricmp(m_pStr,lpsz);}
	int Collate(const TCH * lpsz) const{return TCLS::strcoll(m_pStr,lpsz);}
	int CollateNoCase(const TCH * lpsz) const{return TCLS::stricoll(m_pStr,lpsz);}
	THStringTempl<TCH,TCLS> Mid(UINT nFirst, UINT nCount) const
	{
		THStringTempl<TCH,TCLS> temp;
		if((nFirst+nCount)>(UINT)GetLength()) nCount=GetLength()-nFirst;
		if(nFirst>(UINT)GetLength()) return temp;
		// optimize case of returning entire string
		if((nFirst == 0) && ((nFirst+nCount)==(UINT)GetLength())) return( *this );
		temp.SetString(m_pStr+nFirst,nCount);
		return temp;
	}
	THStringTempl<TCH,TCLS> Mid(UINT nFirst) const{return Mid(nFirst,(UINT)GetLength()-nFirst);}
	THStringTempl<TCH,TCLS> Left(UINT nCount) const
	{
		THStringTempl<TCH,TCLS> temp;
		if (nCount>(UINT)GetLength()) nCount=(UINT)GetLength();
		temp.SetString(m_pStr,nCount);
		return temp;
	}
	THStringTempl<TCH,TCLS> Right(UINT nCount) const
	{
		UINT len=(UINT)GetLength();
		if (nCount<=len) return Mid(nCount,len-nCount);
		return THStringTempl<TCH,TCLS>();
	}
	THStringTempl<TCH,TCLS> SpanIncluding(const TCH * lpszCharSet) const {return Left(TCLS::strspn(m_pStr,lpszCharSet));}
	THStringTempl<TCH,TCLS> SpanExcluding(const TCH * lpszCharSet) const {return Left(TCLS::strcspn(m_pStr,lpszCharSet));}
	void MakeUpper(){TCLS::strupr(m_pStr);}
	void MakeLower(){TCLS::strlwr(m_pStr);}
	void MakeReverse(){TCLS::strrev(m_pStr);}
	THStringTempl<TCH,TCLS>& TrimRight(TCH chTarget=' ')
	{
		for(UINT i=GetLength();i>=0;i--)
		{
			if (GetAt(i)==chTarget)
				SetAt(i,'\0');
			else
				break;
		}
		return (*this);
	}
	THStringTempl<TCH,TCLS>& TrimRight(const TCH * lpszTargets)
	{
		for(UINT i=GetLength();i>=0;i--)
		{
			if (TCLS::strchr(lpszTargets,GetAt(i)))
				SetAt(i,'\0');
			else
				break;
		}
		return (*this);
	}
	THStringTempl<TCH,TCLS>& TrimLeft(TCH chTarget=' ')
	{
		UINT i=0;
		UINT len=GetLength();
		while(GetAt(i)!=chTarget && i<len) i++;
		if (i!=0)
		{
			if (GetAt(i-1)==chTarget)
				TCLS::strcpy(m_pStr,m_pStr+i);
		}
		return (*this);
	}
	THStringTempl<TCH,TCLS>& TrimLeft(const TCH * lpszTargets)
	{
		UINT i=0;
		UINT len=GetLength();
		THStringTempl<TCH,TCLS> temp;
		temp=lpszTargets;
		while(temp.Find(GetAt(i))!=-1 && i<len) i++;
		if (i==0) return;
		if (temp.Find(GetAt(i-1))!=-1)
			TCLS::strcpy(m_pStr,m_pStr+i);
		return (*this);
	}
	THStringTempl<TCH,TCLS>& Trim(TCH chTarget=' '){Trim(chTarget);Trim(chTarget);return (*this);}
	THStringTempl<TCH,TCLS>& Trim(const TCH * lpszTarget){Trim(lpszTarget);Trim(lpszTarget);return (*this);}
	UINT Replace(TCH chOld, TCH chNew)
	{
		UINT cnt=0;
		for(UINT i=0;i<GetLength();i++)
		{
			if (chOld==GetAt(i))
			{
				cnt++;
				SetAt(i,chNew);
			}
		}
		return cnt;
	}
	UINT Replace(const TCH * lpszOld, const TCH * lpszNew)
	{
		// loop once to figure out the size of the result string
		int nSourceLen = TCLS::strlen(lpszOld);
		if(nSourceLen==0) return 0;
		int nReplacementLen = TCLS::strlen(lpszNew);
		UINT nCount = 0;
		{
			TCH *pszStart = m_pStr;
			TCH *pszEnd = m_pStr+GetLength();
			while(pszStart < pszEnd)
			{
				TCH *pszTarget;
				while((pszTarget=TCLS::strstr(pszStart,lpszOld))!=NULL)
				{
					nCount++;
					pszStart=pszTarget+nSourceLen;
				}
				if (pszStart)
					pszStart+=TCLS::strlen(pszStart)+1;
			}
		}
		// if any changes were made, make them
		if(nCount>0)
		{
			// if the buffer is too small, just
			//   allocate a new buffer (slow but sure)
			UINT nOldLength = GetLength();
			UINT nNewLength = nOldLength+(nReplacementLen-nSourceLen)*nCount;
			if (!AllocBuffer(max(nNewLength,nOldLength))) return 0;
			TCH *pszStart = m_pStr;
			TCH *pszEnd = pszStart+nOldLength;

			// loop again to actually do the work
			while(pszStart<pszEnd)
			{
				TCH* pszTarget;
				while((pszTarget=TCLS::strstr(pszStart,lpszOld))!=NULL)
				{
					int nBalance = nOldLength-int(pszTarget-m_pStr+nSourceLen);
					memmove(pszTarget+nReplacementLen, pszTarget+nSourceLen, nBalance*sizeof(TCH));
					memcpy(pszTarget, lpszNew, nReplacementLen*sizeof(TCH));
					pszStart = pszTarget+nReplacementLen;
					pszTarget[nReplacementLen+nBalance]=0;
					nOldLength+=(nReplacementLen-nSourceLen);
				}
				if (pszStart)
					pszStart+=TCLS::strlen(pszStart)+1;
			}
			m_pStr[nNewLength]='\0';
		}
		return nCount;
	}

	UINT Remove(TCH chRemove)
	{
		UINT nLength = GetLength();
		TCH *pszSource = m_pStr;
		TCH *pszDest = m_pStr;
		TCH *pszEnd = m_pStr+nLength;
		while(pszSource < pszEnd)
		{
			TCH *pszNewSource = pszSource+1;
			if( *pszSource != chRemove )
			{
				// Copy the source to the destination.  Remember to copy all bytes of an MBCS character
				TCH *pszNewDest = pszDest+(pszNewSource-pszSource);
				while(pszDest != pszNewDest)
				{
					*pszDest = *pszSource;
					pszSource++;
					pszDest++;
				}
			}
			pszSource = pszNewSource;
		}
		*pszDest = 0;
		UINT nCount=(UINT)(pszSource-pszDest);
		m_pStr[nLength-nCount]='\0';
		return(nCount);
	}

	UINT Insert(UINT nIndex, TCH ch)
	{
		if(nIndex>GetLength()) nIndex=GetLength();
		UINT nNewLength=GetLength()+1;
		if (AllocBuffer(nNewLength))
		{
			// move existing bytes down
			//If some regions of the source area and the destination overlap, memmove ensures that the original source characters in the overlapping region are copied before being overwritten.
			memmove(m_pStr+nIndex+1,m_pStr+nIndex,(nNewLength-nIndex)*sizeof(TCH));
			m_pStr[nIndex]=ch;
		}
		return GetLength();
	}
	UINT Insert(UINT nIndex, const TCH * pstr)
	{
		if(nIndex>GetLength()) nIndex=GetLength();
		UINT distlen=(UINT)TCLS::strlen(pstr);
		UINT nNewLength=GetLength()+distlen;
		if (AllocBuffer(nNewLength))
		{
			// move existing bytes down
			//If some regions of the source area and the destination overlap, memmove ensures that the original source characters in the overlapping region are copied before being overwritten.
			memmove(m_pStr+nIndex+1,m_pStr+nIndex,(nNewLength-nIndex)*sizeof(TCH));
			memcpy(m_pStr+nIndex,pstr,distlen*sizeof(TCH));
		}
		return GetLength();
	}

	UINT Delete(UINT nIndex, UINT nCount = 1)
	{
		UINT nLength = GetLength();
		if((nCount+nIndex)>nLength) nCount=nLength-nIndex;
		if(nCount>0)
		{
			int nNewLength = nLength-nCount;
			int nXCHARsToCopy = nLength-(nIndex+nCount)+1;
			memmove(m_pStr+nIndex,m_pStr+nIndex+nCount,nXCHARsToCopy*sizeof(TCH));
		}
		return GetLength();
	}

	int ReverseFind(TCH ch) const
	{
		// find last single character
		TCH *ret=TCLS::strrchr(m_pStr,ch);
		// return -1 if not found, distance from beginning otherwise
		return ((ret==NULL)?-1:int(ret-m_pStr));
	}

	int Find(TCH ch, UINT nStart=0) const
	{
		TCH *ret=TCLS::strchr(m_pStr+nStart,ch);
		if (!ret) return -1;
		return (int)(m_pStr+nStart-ret);
	}

	int FindOneOf(const TCH * lpszCharSet) const
	{
		THStringTempl<TCH,TCLS> temp;
		temp=lpszCharSet;
		for(UINT i=0;i<GetLength();i++)
		{
			if (temp.Find(GetAt(i))!=-1) return i;
		}
		return -1;
	}

	int Find(const TCH * lpszSub, int nStart=0) const
	{
		TCH *ret=TCLS::strstr(m_pStr+nStart,lpszSub);
		if (!ret) return -1;
		return (int)(m_pStr+nStart-ret);
	}

	void Format(const TCH * lpszFormat, ...)
	{
		va_list argList;
		va_start(argList, lpszFormat);
		TCLS::vsprintf(m_pStr, lpszFormat, argList);//how to get enough buffer
		va_end(argList);
	}

	void FormatV(const TCH * pszFormat, va_list argList){TCLS::vsprintf(m_pStr,pszFormat,argList);}

	void AppendFormat(const TCH * lpszFormat, ...)
	{
		va_list argList;
		va_start(argList, lpszFormat);
		TCLS::vsprintf(m_pStr+GetLength(), lpszFormat, argList); 
		va_end(argList);
	}

	void AppendFormatV(const TCH * pszFormat, va_list argList){TCLS::vsprintf(m_pStr+GetLength(),pszFormat,argList);}

	//not support now
	//void Format(UINT nFormatID, ...);
	//void FormatMessage(const TCH * lpszFormat, ...);
	//void FormatMessage(UINT nFormatID, ...);

	BOOL SetString(const TCH *lpszStr,UINT nLen)
	{
		if (!AllocBuffer(nLen)) return FALSE;
		TCLS::strncpy(m_pStr,lpszStr,nLen);
		m_pStr[nLen]='\0';
		return TRUE;
	}

	BOOL SetString(const TCH *lpszStr)
	{
		UINT len=(UINT)TCLS::strlen(lpszStr);
		if (!AllocBuffer(len)) return FALSE;
		TCLS::strcpy(m_pStr,lpszStr);
		return TRUE;
	}

	BOOL SetString(const TCH ch)
	{
		if (!AllocBuffer(1)) return FALSE;
		m_pStr[0]=ch;
		m_pStr[1]='\0';
		return TRUE;
	}

	BOOL Append(const TCH *lpszStr,UINT nLen)
	{
		UINT nOrgLen=GetLength();
		if (!AllocBuffer(nLen+nOrgLen)) return FALSE;
		TCLS::memcpy(m_pStr+nOrgLen,lpszStr,nLen*sizeof(TCH));
		m_pStr[nLen+nOrgLen]='\0';
		return TRUE;
	}

	BOOL Append(const TCH *lpszStr)
	{
		UINT len=(UINT)TCLS::strlen(lpszStr);
		if (!AllocBuffer(len,FALSE)) return FALSE;
		TCLS::strcat(m_pStr,lpszStr);
		return TRUE;
	}

	BOOL Append(const TCH ch)
	{
		UINT nOrgLen=GetLength();
		if (!AllocBuffer(1+nOrgLen)) return FALSE;
		m_pStr[nOrgLen]=ch;
		m_pStr[nOrgLen+1]='\0';
		return TRUE;
	}

/*	BOOL LoadString(UINT nID)
	{
		HRSRC hResource;
		hResource = ::FindResource(hInstance,MAKEINTRESOURCE(((nID>>4)+1)),RT_STRING);
		if(hResource==NULL) return FALSE;
		const ATLSTRINGRESOURCEIMAGE* pImage;
		const ATLSTRINGRESOURCEIMAGE* pImageEnd;
		ULONG nResourceSize;
		HGLOBAL hGlobal;
		UINT iIndex;
		hGlobal = ::LoadResource( hInstance, hResource );
		if(hGlobal == NULL) return FALSE;
		pImage = (const ATLSTRINGRESOURCEIMAGE*)::LockResource( hGlobal );
		if( pImage == NULL ) return FALSE;
		nResourceSize = ::SizeofResource( hInstance, hResource );
		pImageEnd = (const ATLSTRINGRESOURCEIMAGE*)(LPBYTE( pImage )+nResourceSize);
		iIndex = id&0x000f;
		while( (iIndex > 0) && (pImage < pImageEnd) )
		{
			pImage = (const ATLSTRINGRESOURCEIMAGE*)(LPBYTE( pImage )+(sizeof( ATLSTRINGRESOURCEIMAGE )+(pImage->nLength*sizeof( WCHAR ))));
			iIndex--;
		}
		if( pImage >= pImageEnd ) return FALSE;
		if( pImage->nLength == 0 ) return FALSE;
		int nLength = StringTraits::GetBaseTypeLength( pImage->achString, pImage->nLength );
		PXSTR pszBuffer = GetBuffer( nLength );
		StringTraits::ConvertToBaseType( pszBuffer, nLength, pImage->achString, pImage->nLength );
		ReleaseBufferSetLength( nLength );
		return TRUE;
	}*/

	// OLE BSTR support (use for OLE automation)
	// return a BSTR initialized with this THStringTempl<TCH,TCLS>'s data
	BSTR AllocSysString() const
	{
		wchar_t *tmp=TCLS::tow(m_pStr);
		BSTR str;
		if (tmp)
		{
			str=::SysAllocString(tmp);
			TCLS::free(tmp);
		}
		return str;
	}
	// reallocates the passed BSTR, copies content of this THStringTempl<TCH,TCLS> to it
	BSTR SetSysString(BSTR* pbstr) const
	{
		wchar_t *tmp=TCLS::tow(m_pStr);
		if (tmp)
		{
			::SysReAllocStringLen(pbstr,tmp,GetLength());
			TCLS::free(tmp);
		}
		return *pbstr;
	}

	TCH *GetBuffer(UINT nMinBufLength=0)
	{
		if (nMinBufLength!=0 && !AllocBuffer(nMinBufLength)) return NULL;
		return m_pStr;
	}

	void ReleaseBuffer(UINT nNewLength = -1) {/*not need to do anything*/}

	TCH *GetBufferSetLength(UINT nNewLength) {return GetBuffer(nNewLength);}
	void FreeExtra() {/*not need to do anything*/}

	TCH *LockBuffer() {/*not need to do anything*/}
	void UnlockBuffer() {/*not need to do anything*/}

	UINT GetAllocLength() const{return m_nBufLen;}
protected:
	TCH *m_pStr;
	UINT m_nBufLen;

	BOOL AllocBuffer(UINT nLen,BOOL bCopy=TRUE)
	{
		if (!bCopy && m_pStr) nLen+=TCLS::strlen(m_pStr);
		if (nLen+1>m_nBufLen)
		{
			//realloc buffer
			UINT size=(nLen+DEFAULT_ALLOC_SIZE)/DEFAULT_ALLOC_SIZE;//+1-1刚好抵消了
			TCH *tmp=new TCH[size];
			if (!tmp)
			{
				Empty();
				return FALSE;
			}
			if (m_pStr)
			{
				TCLS::strcpy(tmp,m_pStr);
				delete [] m_pStr;
			}
			else
				tmp[0]='\0';
			m_pStr=tmp;
			m_nBufLen=size;
		}
		return TRUE;
	}
};

// Compare helpers
/*bool operator==(const THStringTempl<TCH,TCLS>& s1, const THStringTempl<TCH,TCLS>& s2) {return s1.Compare(s2.GetBuffer())==0;}
bool operator==(const THStringTempl<TCH,TCLS>& s1, const TCH * s2) {return s1.Compare(s2)==0;}
bool operator==(const TCH * s1, const THStringTempl<TCH,TCLS>& s2) {return s2.Compare(s1)==0;}
bool operator!=(const THStringTempl<TCH,TCLS>& s1, const THStringTempl<TCH,TCLS>& s2) {return s1.Compare(s2.GetBuffer())!=0;}
bool operator!=(const THStringTempl<TCH,TCLS>& s1, const TCH * s2) {return s1.Compare(s2)!=0;}
bool operator!=(const TCH * s1, const THStringTempl<TCH,TCLS>& s2) {return s2.Compare(s1)!=0;}
bool operator<(const THStringTempl<TCH,TCLS>& s1, const THStringTempl<TCH,TCLS>& s2) {return s1.Compare(s2.GetBuffer())<0;}
bool operator<(const THStringTempl<TCH,TCLS>& s1, const TCH * s2) {return s1.Compare(s2)<0;}
bool operator<(const TCH * s1, const THStringTempl<TCH,TCLS>& s2) {return s2.Compare(s1)>=0;}
bool operator>(const THStringTempl<TCH,TCLS>& s1, const THStringTempl<TCH,TCLS>& s2) {return s1.Compare(s2.GetBuffer())>0;}
bool operator>(const THStringTempl<TCH,TCLS>& s1, const TCH * s2) {return s1.Compare(s2)>0;}
bool operator>(const TCH * s1, const THStringTempl<TCH,TCLS>& s2) {return s2.Compare(s1)<=0;}
bool operator<=(const THStringTempl<TCH,TCLS>& s1, const THStringTempl<TCH,TCLS>& s2) {return s1.Compare(s2.GetBuffer())<=0;}
bool operator<=(const THStringTempl<TCH,TCLS>& s1, const TCH * s2) {return s1.Compare(s2)<=0;}
bool operator<=(const TCH * s1, const THStringTempl<TCH,TCLS>& s2) {return s2.Compare(s1)>0;}
bool operator>=(const THStringTempl<TCH,TCLS>& s1, const THStringTempl<TCH,TCLS>& s2) {return s1.Compare(s2.GetBuffer())>=0;}
bool operator>=(const THStringTempl<TCH,TCLS>& s1, const TCH * s2) {return s1.Compare(s2)>=0;}
bool operator>=(const TCH * s1, const THStringTempl<TCH,TCLS>& s2) {return s2.Compare(s1)<0;}
*/
typedef THStringTempl<char,THCharsetConvertMapA> CStringA;
typedef THStringTempl<wchar_t,THCharsetConvertMapW> CStringW;
#ifdef _UNICODE
	typedef THStringTempl<wchar_t,THCharsetConvertMapT> CString;
#else
	typedef THStringTempl<char,THCharsetConvertMapT> CString;
#endif

CStringA operator+(const CStringA& string1,const CStringA& string2)
{
	CStringA temp;
	temp.SetString((LPCSTR)string1);
	temp.Append((LPCSTR)string2);
	return temp;
}
CStringA operator+(const CStringA& string, char ch)
{
	CStringA temp;
	temp.SetString((LPCSTR)string);
	temp.Append(ch);
	return temp;
}
CStringA operator+(char ch, const CStringA& string)
{
	CStringA temp;
	temp.SetString(ch);
	temp.Append((LPCSTR)string);
	return temp;
}
CStringA operator+(const CStringA& string, const char * lpsz)
{
	CStringA temp;
	temp.SetString((LPCSTR)string);
	temp.Append(lpsz);
	return temp;
}
CStringA operator+(const char * lpsz, const CStringA& string)
{
	CStringA temp;
	temp.SetString(lpsz);
	temp.Append((LPCSTR)string);
	return temp;
}

CStringW operator+(const CStringW& string1,const CStringW& string2)
{
	CStringW temp;
	temp.SetString((LPCWSTR)string1);
	temp.Append((LPCWSTR)string2);
	return temp;
}
CStringW operator+(const CStringW& string, wchar_t ch)
{
	CStringW temp;
	temp.SetString((LPCWSTR)string);
	temp.Append(ch);
	return temp;
}
CStringW operator+(wchar_t ch, const CStringW& string)
{
	CStringW temp;
	temp.SetString(ch);
	temp.Append((LPCWSTR)string);
	return temp;
}
CStringW operator+(const CStringW& string, const wchar_t * lpsz)
{
	CStringW temp;
	temp.SetString((LPCWSTR)string);
	temp.Append(lpsz);
	return temp;
}
CStringW operator+(const wchar_t * lpsz, const CStringW& string)
{
	CStringW temp;
	temp.SetString(lpsz);
	temp.Append((LPCWSTR)string);
	return temp;
}

bool operator==(const CStringA& s1, const CStringA& s2) {return s1.Compare((LPCSTR)s2)==0;}
bool operator==(const CStringA& s1, const char * s2) {return s1.Compare(s2)==0;}
bool operator==(const char * s1, const CStringA& s2) {return s2.Compare(s1)==0;}
bool operator!=(const CStringA& s1, const CStringA& s2) {return s1.Compare((LPCSTR)s2)!=0;}
bool operator!=(const CStringA& s1, const char * s2) {return s1.Compare(s2)!=0;}
bool operator!=(const char * s1, const CStringA& s2) {return s2.Compare(s1)!=0;}
bool operator<(const CStringA& s1, const CStringA& s2) {return s1.Compare((LPCSTR)s2)<0;}
bool operator<(const CStringA& s1, const char * s2) {return s1.Compare(s2)<0;}
bool operator<(const char * s1, const CStringA& s2) {return s2.Compare(s1)>=0;}
bool operator>(const CStringA& s1, const CStringA& s2) {return s1.Compare((LPCSTR)s2)>0;}
bool operator>(const CStringA& s1, const char * s2) {return s1.Compare(s2)>0;}
bool operator>(const char * s1, const CStringA& s2) {return s2.Compare(s1)<=0;}
bool operator<=(const CStringA& s1, const CStringA& s2) {return s1.Compare((LPCSTR)s2)<=0;}
bool operator<=(const CStringA& s1, const char * s2) {return s1.Compare(s2)<=0;}
bool operator<=(const char * s1, const CStringA& s2) {return s2.Compare(s1)>0;}
bool operator>=(const CStringA& s1, const CStringA& s2) {return s1.Compare((LPCSTR)s2)>=0;}
bool operator>=(const CStringA& s1, const char * s2) {return s1.Compare(s2)>=0;}
bool operator>=(const char * s1, const CStringA& s2) {return s2.Compare(s1)<0;}

bool operator==(const CStringW& s1, const CStringW& s2) {return s1.Compare((LPCWSTR)s2)==0;}
bool operator==(const CStringW& s1, const wchar_t * s2) {return s1.Compare(s2)==0;}
bool operator==(const wchar_t * s1, const CStringW& s2) {return s2.Compare(s1)==0;}
bool operator!=(const CStringW& s1, const CStringW& s2) {return s1.Compare((LPCWSTR)s2)!=0;}
bool operator!=(const CStringW& s1, const wchar_t * s2) {return s1.Compare(s2)!=0;}
bool operator!=(const wchar_t * s1, const CStringW& s2) {return s2.Compare(s1)!=0;}
bool operator<(const CStringW& s1, const CStringW& s2) {return s1.Compare((LPCWSTR)s2)<0;}
bool operator<(const CStringW& s1, const wchar_t * s2) {return s1.Compare(s2)<0;}
bool operator<(const wchar_t * s1, const CStringW& s2) {return s2.Compare(s1)>=0;}
bool operator>(const CStringW& s1, const CStringW& s2) {return s1.Compare((LPCWSTR)s2)>0;}
bool operator>(const CStringW& s1, const wchar_t * s2) {return s1.Compare(s2)>0;}
bool operator>(const wchar_t * s1, const CStringW& s2) {return s2.Compare(s1)<=0;}
bool operator<=(const CStringW& s1, const CStringW& s2) {return s1.Compare((LPCWSTR)s2)<=0;}
bool operator<=(const CStringW& s1, const wchar_t * s2) {return s1.Compare(s2)<=0;}
bool operator<=(const wchar_t * s1, const CStringW& s2) {return s2.Compare(s1)>0;}
bool operator>=(const CStringW& s1, const CStringW& s2) {return s1.Compare((LPCWSTR)s2)>=0;}
bool operator>=(const CStringW& s1, const wchar_t * s2) {return s1.Compare(s2)>=0;}
bool operator>=(const wchar_t * s1, const CStringW& s2) {return s2.Compare(s1)<0;}


/*

#define DEFAULT_ALLOC_SIZE		256

template <typename TCH,typename TCLS>
class THStringTempl<TCH,TCLS>Templ
{
public:
	THStringTempl<TCH,TCLS>Templ() 
	{
		m_chStr=NULL;
		m_nBufLen=0;
		AllocBuffer();
	}
	BOOL AllocBuffer(const TCH *pSrc=NULL,UINT nLen=0)
	{
		UINT nUseSize=0;
		if (m_chStr) nUseSize=(UINT)TCLS::strlen(m_chStr);
		UINT nCnt=(nUseSize+nLen+DEFAULT_ALLOC_SIZE-1)/DEFAULT_ALLOC_SIZE;
		if (m_nBufLen<nCnt*DEFAULT_ALLOC_SIZE)
		{
			THSingleLock lock(&m_mtx);
			//need realloc buffer
			TCH *tmp=new TCH[nCnt*DEFAULT_ALLOC_SIZE];
			if (!tmp)
			{
				if (m_chStr)
				{
					delete [] m_chStr;
					m_chStr=NULL;
					m_nBufLen=0;
				}
				return FALSE;
			}
			if (m_chStr)
				TCLS::strcpy(tmp,m_chStr);
			else
				tmp[0]='\0';
			if (pSrc)
				TCLS::strcat(tmp,pSrc);
			delete [] m_chStr;
			m_chStr=tmp;
			m_nBufLen=nCnt*DEFAULT_ALLOC_SIZE;
		}
		else
		{
			if (pSrc) strcat(m_chStr,pSrc);
		}
		return TRUE;
	}
	THStringTempl<TCH,TCLS>Templ(const TCH *szInputSource) 
	{
		m_chStr=NULL;
		m_nBufLen=0;
		if (szInputSource)
			AllocBuffer(szInputSource,(UINT)TCLS::strlen(szInputSource));
	}
	THStringTempl<TCH,TCLS>Templ(const THStringTempl<TCH,TCLS>Templ<TCH,TCLS> &szInputSource) 
	{ 
		m_chStr=NULL;
		m_nBufLen=0;
		AllocBuffer(szInputSource.GetData(),szInputSource.GetLength());
	}
	~THStringTempl<TCH,TCLS>Templ()
	{
		if (m_chStr)
		{
			delete [] m_chStr;
			m_chStr=NULL;
			m_nBufLen=0;
		}
	}

	// Methods
public:
	void Empty()
	{
		if (m_chStr)
			m_chStr[0]='\0';
	}

	long GetLength() const
	{ 
		return m_chStr ? (long)TCLS::strlen(m_chStr) : 0;
	}

	bool IsEmpty() const
	{
		return GetLength()==0;
	}

	TCH *GetData() const
	{
		return m_chStr;
	}

	TCH *GetBuffer(int index=0)
	{
		AllocBuffer(NULL,index);
		//m_mtx.Lock();
		return m_chStr;
	}

	TCH *GetBufferSetLength(int nLen)
	{
		AllocBuffer(NULL,nLen);
		//m_mtx.Lock();
		return m_chStr;
	}

	void ReleaseBuffer()
	{
		//m_mtx.Unlock();
	}

	operator const TCH*() const
	{
		return GetData();
	}

	operator TCH*()
	{
		return GetData();
	}

	TCH GetAt(long nIndex)
	{
		return nIndex<GetLength() ? m_chStr[nIndex] : 0;
	}

	TCH RawGetAt(int nIndex) // no sanity check, use only in performance issues
	{
		return m_chStr[nIndex];
	}

	TCH operator[](int nIndex)
	{
		return GetAt(nIndex);
	}

	void SetAt(long nIndex, TCH ch)
	{
		if (nIndex<GetLength())
			m_chStr[nIndex] = ch;
	}

	void RawSetAt(long nIndex, TCH ch) // no sanity check, use only in performance issues
	{
		m_chStr[nIndex] = ch;
	}

	void Copy(const TCH *szInputSource)
	{
		Empty();
		if (!szInputSource) return;
		AllocBuffer(szInputSource,(UINT)TCLS::strlen(szInputSource));
	}

	void Copy(const THStringTempl<TCH,TCLS>Templ<TCH,TCLS> &szInputSource)
	{
		Copy( szInputSource.GetData() );
	}

	void operator =(TCH *szInputSource)
	{
		Copy( szInputSource );
	}

	void operator =(const THStringTempl<TCH,TCLS>Templ<TCH,TCLS> &szInputSource)
	{
		Copy( szInputSource );
	}

	void MakeLower()
	{
		if (GetLength()>0)
			TCLS::strlwr(m_chStr);
	}

	void MakeUpper()
	{
		if (GetLength()>0)
			TCLS::strupr(m_chStr);
	}

	int Compare(const TCH *szCompareString) const
	{
		if (!m_chStr) return -1;
		return szCompareString ? TCLS::strcmp(m_chStr,szCompareString) : -1;
	}

	int Compare(THStringTempl<TCH,TCLS>Templ<TCH,TCLS> &szCompareString) const
	{
		if (!m_chStr) return -1;
		return TCLS::strcmp(m_chStr,szCompareString.GetData());
	}

	int CompareNoCase(const TCH *szCompareString) const
	{
		if (!m_chStr) return -1;
		return szCompareString ? TCLS::stricmp(m_chStr,szCompareString) : -1;
	}

	int CompareNoCase(THStringTempl<TCH,TCLS>Templ<TCH,TCLS> &szCompareString) const
	{
		if (!m_chStr) return -1;
		return TCLS::stricmp(GetData(),szCompareString.GetData());
	}

	bool operator ==(THStringTempl<TCH,TCLS>Templ<TCH,TCLS> &szCompareString) const
	{
		return Compare(szCompareString)==0;
	}

	bool operator !=(THStringTempl<TCH,TCLS>Templ<TCH,TCLS> &szCompareString) const
	{
		return Compare(szCompareString)!=0;
	}

	bool operator ==(const TCH *szCompareString) const
	{
		return Compare(szCompareString)==0;
	}

	bool operator !=(const TCH *szCompareString) const
	{
		return Compare(szCompareString)!=0;
	}

	long Find(const THStringTempl<TCH,TCLS>Templ<TCH,TCLS> &szStringToFind,long nStartIndex=0) const // find string
	{
		if (!szStringToFind.IsEmpty() && nStartIndex<GetLength())
		{
			TCH *szResult = TCLS::strstr(m_chStr+nStartIndex, szStringToFind);
			return szResult ? (long)(szResult - m_chStr) : -1;
		}
		else
			return -1;
	}

/*	long Find(const TCH sCharToFind,long nStartIndex=0) // find char
	{
		TCH szTemp[2];
		szTemp[0] = sCharToFind;
		szTemp[1] = '\0';

		return Find(nStartIndex, szTemp);
	}* /

	long FindNoCase(TCH *szStringToFind,long nStartIndex) const// find string (without case)
	{
		if (!szStringToFind)
			return -1;

		THStringTempl<TCH,TCLS>Templ<TCH,TCLS> szTempLower = GetData();
		szTempLower.ToLower();
		THStringTempl<TCH,TCLS>Templ<TCH,TCLS> szStringToFindLower = szStringToFind;
		szStringToFindLower.ToLower();
		return szTempLower.Find(nStartIndex, szStringToFindLower.GetData());
	}

	long FindNoCase(TCH szCharToFind,long nStartIndex) const // find char (without case)
	{
		TCH szTemp[2];
		szTemp[0] = szCharToFind;
		szTemp[1] = '\0';
		return FindNoCase(nStartIndex, szTemp);
	}

	long ReverseFind(TCH ch,long nStartIndex=-1) const
	{
		if (nStartIndex==-1) nStartIndex=GetLength()-1;
		if (nStartIndex<GetLength())
		{
			TCH *szResult = TCLS::strrchr(m_chStr+nStartIndex, ch);
			return szResult ? (long)(szResult - m_chStr) : -1;
		}
		else
			return -1;
	}

	THStringTempl<TCH,TCLS>Templ<TCH,TCLS> Left(long nCount) const
	{
		THStringTempl<TCH,TCLS>Templ<TCH,TCLS> temp;
		TCH *szTemp=temp.GetBuffer(nCount+1);
		szTemp[0]='\0';
		if (nCount>0 && GetLength()>0)
		{
			int nLen = nCount>GetLength() ? GetLength() : nCount;
			TCLS::strncat(szTemp, m_chStr, nLen);
			szTemp[nCount]='\0';
		}
		temp.ReleaseBuffer();
		return temp;
	}

	THStringTempl<TCH,TCLS>Templ<TCH,TCLS> ExcludeLeft(long nCount) // remove first nCount chars
	{
		return Right(GetLength()-nCount);
	}

	inline THStringTempl<TCH,TCLS>Templ<TCH,TCLS> Mid(long nIndex) const
	{
		return Mid(nIndex,GetLength()-nIndex);
	}

	THStringTempl<TCH,TCLS>Templ<TCH,TCLS> Mid(long nIndex, long nCount) const
	{
		THStringTempl<TCH,TCLS>Templ<TCH,TCLS> temp;
		TCH *szTemp=temp.GetBuffer(nCount+1);
		szTemp[0]='\0';
		if (nIndex>=0 && nCount>0 && GetLength()>0)
		{
			int nLen=(nCount+nIndex)>GetLength() ? GetLength()-nIndex : nCount;
			TCLS::strncat(szTemp, m_chStr+nIndex, nLen);
			szTemp[nLen]='\0';
		}
		temp.ReleaseBuffer();
		return temp;
	}

	THStringTempl<TCH,TCLS>Templ<TCH,TCLS> Right(long nCount) const
	{
		return Mid(GetLength()-nCount, nCount);
	}

	void InternalLeft(long nCount) //  applies to *this* object
	{
		THStringTempl<TCH,TCLS>Templ<TCH,TCLS> t;
		t = Left(nCount);
		*this = t;
	}

	void InternalExcludeLeft(long nCount) // remove first nCount chars,  applies to *this* object
	{
		InternalRight(GetLength()-nCount);
	}

	void InternalMid(long nIndex, long nCount) //  applies to *this* object
	{
		THStringTempl<TCH,TCLS>Templ<TCH,TCLS> t;
		t = Mid(nIndex,nCount);
		*this = t;
	}

	void InternalRight(long nCount) //  applies to *this* object
	{
		InternalMid(GetLength()-nCount, nCount);
	}

	THStringTempl<TCH,TCLS>Templ<TCH,TCLS> Tokenize(TCH *szTokens, long &nStartIndex)
	{
		THStringTempl<TCH,TCLS>Templ<TCH,TCLS> s;
		if (nStartIndex<0 || nStartIndex>=GetLength())
			return s;

		if (TCLS::strlen(szTokens)==0)
			return s;

		char c = szTokens[0];
		// long Find(long nStartIndex, char *szStringToFind) // find string
		long nIndex = Find(nStartIndex, c);
		if (nIndex<0)
		{
			s = ExcludeLeft(nStartIndex); // copy current string
			nStartIndex = GetLength();
		}
		else
		{
			s = Mid(nStartIndex,nIndex-nStartIndex);
			nStartIndex = nIndex+1;
		}

		return s;
	}

	void Append(const TCH *szAdditionalString)
	{
		if (szAdditionalString)
			AllocBuffer(szAdditionalString,(UINT)TCLS::strlen(szAdditionalString));
	}

	void SetString(const TCH *szStr)
	{
		Empty();
		AllocBuffer(szStr,(UINT)TCLS::strlen(szStr));
	}

	void SetString(const TCH *szStr,int len)
	{
		Empty();
		TCH *tmp=new TCH[len+1];
		if (tmp)
		{
			memcpy(tmp,szStr,len);
			tmp[len]='\0';
			SetString(tmp);
			delete [] tmp;
		}
	}

	void Append(const THStringTempl<TCH,TCLS>Templ<TCH,TCLS> &szAdditionalString)
	{
		if (!szAdditionalString.IsEmpty())
			Append( szAdditionalString.GetData() );
	}

	void Append(TCH sAdditionalChar)
	{
		TCH szAdditionalString[2];
		szAdditionalString[0] = sAdditionalChar;
		szAdditionalString[1] = '\0';
		Append(szAdditionalString);
	}

	void operator +=(THStringTempl<TCH,TCLS>Templ<TCH,TCLS> &szAdditionalString)
	{
		Append(szAdditionalString);
	}

	void operator +=(TCH *szAdditionalString)
	{
		Append(szAdditionalString);
	}

	void operator +=(TCH sAdditionalChar)
	{
		Append(sAdditionalChar);
	}

	THStringTempl<TCH,TCLS>Templ<TCH,TCLS> operator +(THStringTempl<TCH,TCLS>Templ<TCH,TCLS> &szAdditionalString)
	{
		THStringTempl<TCH,TCLS>Templ<TCH,TCLS> szTemp = *this;
		szTemp.Append(szAdditionalString);
		return szTemp;
	}

	THStringTempl<TCH,TCLS>Templ<TCH,TCLS> operator +(const THStringTempl<TCH,TCLS>Templ<TCH,TCLS> &szAdditionalString)
	{
		THStringTempl<TCH,TCLS>Templ<TCH,TCLS> szTemp = *this;
		szTemp.Append(szAdditionalString);
		return szTemp;
	}

/*	THStringTempl<TCH,TCLS>Templ<TCH,TCLS> operator +(TCH *szAdditionalString)
	{
		THStringTempl<TCH,TCLS>Templ<TCH,TCLS> szTemp = *this;
		szTemp.Append(szAdditionalString);
		return szTemp;
	}

	THStringTempl<TCH,TCLS>Templ<TCH,TCLS> operator +(TCH sAdditionalChar)
	{
		THStringTempl<TCH,TCLS>Templ<TCH,TCLS> szTemp = *this;
		szTemp.Append(sAdditionalChar);
		return szTemp;
	}
* /
	// binary + operator (global, with const parameters)
	friend THStringTempl<TCH,TCLS>Templ<TCH,TCLS> operator+(const THStringTempl<TCH,TCLS>Templ<TCH,TCLS> &sz1, const THStringTempl<TCH,TCLS>Templ<TCH,TCLS> &sz2)
	{
		THStringTempl<TCH,TCLS>Templ<TCH,TCLS> s = sz1;
		s += (THStringTempl<TCH,TCLS>Templ<TCH,TCLS> &)sz2;
		return s;
	}

/*	friend THStringTempl<TCH,TCLS>Templ<TCH,TCLS> operator+(const THStringTempl<TCH,TCLS>Templ<TCH,TCLS> &sz1, const TCH *sz2)
	{
		THStringTempl<TCH,TCLS>Templ<TCH,TCLS> s = sz1;
		s += (TCH*)sz2;
		return s;
	}

	friend THStringTempl<TCH,TCLS>Templ<TCH,TCLS> operator+(const TCH *sz1,const THStringTempl<TCH,TCLS>Templ<TCH,TCLS> &sz2)
	{
		THStringTempl<TCH,TCLS>Templ<TCH,TCLS> s = (TCH*)sz1;
		s += sz2;
		return s;
	}

	friend THStringTempl<TCH,TCLS>Templ<TCH,TCLS> operator+(THStringTempl<TCH,TCLS>Templ<TCH,TCLS> &sz1, THStringTempl<TCH,TCLS>Templ<TCH,TCLS> &sz2)
	{
		THStringTempl<TCH,TCLS>Templ<TCH,TCLS> s = sz1;
		s += (THStringTempl<TCH,TCLS>Templ<TCH,TCLS> &)sz2;
		return s;
	}

	friend THStringTempl<TCH,TCLS>Templ<TCH,TCLS> operator+(THStringTempl<TCH,TCLS>Templ<TCH,TCLS> &sz1, TCH *sz2)
	{
		THStringTempl<TCH,TCLS>Templ<TCH,TCLS> s = sz1;
		s += (TCH*)sz2;
		return s;
	}

	friend THStringTempl<TCH,TCLS>Templ<TCH,TCLS> operator+(TCH *sz1,THStringTempl<TCH,TCLS>Templ<TCH,TCLS> &sz2)
	{
		THStringTempl<TCH,TCLS>Templ<TCH,TCLS> s = (TCH*)sz1;
		s += sz2;
		return s;
	}
* /
#if defined(_ATL_VER) || defined(_MFC_VER)

	THStringTempl<TCH,TCLS>Templ(const CString &szInputSource) 
	{ 
		m_chStr = NULL;
		Copy((const TCH *)szInputSource);
	}
/*
	void operator =(const CString &szInputSource)
	{
		m_chStr = NULL;
		Copy((const TCH *)szInputSource);
	}
* /
	bool operator ==(const CString &szStr) const
	{
		return Compare(szStr)==0;
	}
/*
	void operator +=(CString &szAdditionalString)
	{
		Append(szAdditionalString);
	}

	THStringTempl<TCH,TCLS>Templ<TCH,TCLS> operator +(CString &szAdditionalString)
	{
		THStringTempl<TCH,TCLS>Templ<TCH,TCLS> szTemp = *this;
		szTemp.Append(szAdditionalString);
		return szTemp;
	}

	// binary + operator (global, with const parameters)
	friend THStringTempl<TCH,TCLS>Templ<TCH,TCLS> operator+(const THStringTempl<TCH,TCLS>Templ<TCH,TCLS> &sz1, const CString &sz2)
	{
		THStringTempl<TCH,TCLS>Templ<TCH,TCLS> s = sz1;
		s += (THStringTempl<TCH,TCLS>Templ<TCH,TCLS> &)sz2;
		return s;
	}

	friend THStringTempl<TCH,TCLS>Templ<TCH,TCLS> operator+(const CString &sz1, const THStringTempl<TCH,TCLS>Templ<TCH,TCLS> &sz2)
	{
		THStringTempl<TCH,TCLS>Templ<TCH,TCLS> s = sz1;
		s += (THStringTempl<TCH,TCLS>Templ<TCH,TCLS> &)sz2;
		return s;
	}
* /
#endif
	THStringTempl<TCH,TCLS>Templ<TCH,TCLS>& TrimLeft(TCH *ch)
	{
		if (IsEmpty() || !ch) return this;
		THStringTempl<TCH,TCLS>Templ<TCH,TCLS> sTemp=ch;
		long nSize = GetLength();
		long nAmount = 0;
		bool bContinue = true;
		for (long i=0; i<nSize && bContinue; i++)
		{
			if (sTemp.Find(RawGetAt(i))!=-1)
				nAmount++;
			else
				bContinue = false;
		}
		if (!bContinue || nAmount>0)
			InternalExcludeLeft( nAmount );
		return this;
	}

	THStringTempl<TCH,TCLS>Templ<TCH,TCLS>& TrimLeft(TCH ch=' ')
	{
		if (IsEmpty()) return *this;
		long nSize = GetLength();
		long nAmount = 0;
		bool bContinue = true;
		for (long i=0; i<nSize && bContinue; i++)
		{
			if (RawGetAt(i)==ch)
				nAmount++;
			else
				bContinue = false;
		}
		if (!bContinue || nAmount>0)
			InternalExcludeLeft( nAmount );
		return *this;
	}

	THStringTempl<TCH,TCLS>Templ<TCH,TCLS>& TrimRight(TCH *ch)
	{
		if (IsEmpty() || !ch) return *this;
		THStringTempl<TCH,TCLS>Templ<TCH,TCLS> sTemp=ch;
		long nSize = GetLength();
		long nAmount = 0;
		bool bContinue = true;
		for (long i=0; i<nSize && bContinue; i++)
		{
			if (sTemp.Find(Mid(nSize-1-i,1))!=-1)
				nAmount++;
			else
				bContinue = false;
		}
		if (!bContinue || nAmount>0)
			InternalLeft( GetLength()-nAmount );
		return *this;
	}

	THStringTempl<TCH,TCLS>Templ<TCH,TCLS>& TrimRight(TCH ch=' ')
	{
		if (IsEmpty()) return *this;
		long nSize = GetLength();
		long nAmount = 0;
		bool bContinue = true;
		for (long i=0; i<nSize && bContinue; i++)
		{
			if (RawGetAt(nSize-1-i)==ch)
				nAmount++;
			else
				bContinue = false;
		}
		if (!bContinue || nAmount>0)
			InternalLeft( GetLength()-nAmount );
		return *this;
	}

	THStringTempl<TCH,TCLS>Templ<TCH,TCLS>& Trim(TCH ch=' ')
	{
		TrimLeft(ch);
		TrimRight(ch);
		return *this;
	}

	THStringTempl<TCH,TCLS>Templ<TCH,TCLS>& Trim(TCH *ch)
	{
		TrimLeft(ch);
		TrimRight(ch);
		return *this;
	}

	long GetNumber()
	{
		if (IsEmpty()) return 0L;
		long longValue = 0;
		const TCH sstr[15]={'0','1','2','3','4','5','6','7','8','9','.',',','-','+','\0'};
		TCH *c = TCLS::strpbrk(GetData(), sstr);
		if (c) longValue=TCLS::atol(c);
		return longValue;
	}

	float GetFloat()
	{
		if (IsEmpty()) return 0.0f;
		float floatValue = 0;
		const TCH sstr[15]={'0','1','2','3','4','5','6','7','8','9','.',',','-','+','\0'};
		TCH *c = TCLS::strpbrk(GetData(), sstr);
		if (c) floatValue=TCLS::atof(c);
		return floatValue;
	}

	void ReplaceChar(TCH ch, TCH sReplacemenTCH)
	{
		if (IsEmpty()) return;
		long nSize = GetLength();
		long nIndex=0;
		long nIndex2;
		while ( nIndex<nSize && (nIndex2=Find(nIndex,ch))>-1 )
		{
			RawSetAt( nIndex2, sReplacemenTCH);
			nIndex = ++nIndex2; // ensure no infinite loop trap
		}
	}

	void ReplaceCharWiTHStringTempl<TCH,TCLS>(TCH ch, TCH *szReplacementString) // szReplacementString==NULL allowed
	{
		if (IsEmpty()) return;
		THStringTempl<TCH,TCLS>Templ<TCH,TCLS> szTemp;
		long nSize = GetLength();
		long nIndex=0;
		long nIndex2;
		while ( nIndex<nSize && (nIndex2=Find(nIndex,ch))>-1 )
		{
			// copy first portion of string
			if (nIndex2>nIndex)
				szTemp += Mid(nIndex, nIndex2-nIndex);
			// then copy replacement string
			if (szReplacementString)
				szTemp += szReplacementString;
			nIndex = ++nIndex2; // ensure no infinite loop trap
		}
		// don't forget to copy the remainder of the input string, if any
		if (nIndex<nSize) szTemp += ExcludeLeft(nIndex);
		*this = szTemp;
	}

	void ReplaceCharWiTHStringTempl<TCH,TCLS>(TCH ch, THStringTempl<TCH,TCLS>Templ<TCH,TCLS> &szReplacementString)
	{
		ReplaceCharWiTHStringTempl<TCH,TCLS>(ch, szReplacementString.GetData() );
	}

	void Replace(const TCH *src,const TCH *dist)
	{
		if (IsEmpty()) return;
		if (!src || !dist) return;
		THStringTempl<TCH,TCLS>Templ<TCH,TCLS> szTemp;
		long nSize = GetLength();
		long nIndex=0;
		long nIndex2;
		while ( nIndex<nSize && (nIndex2=Find(src,nIndex))>-1 )
		{
			// copy first portion of string
			if (nIndex2>nIndex)
				szTemp += Mid(nIndex, nIndex2-nIndex);
			// then copy replacement string
			if (dist)
				szTemp.Append(dist);
			nIndex = ++nIndex2; // ensure no infinite loop trap
		}
		// don't forget to copy the remainder of the input string, if any
		if (nIndex<nSize) szTemp += ExcludeLeft(nIndex);
		*this = szTemp;
	}

	void Format(const TCH *pszFormat,...) // do not use, m_chStr does not have the right buffer allocated
	{ // missing here is a preliminary loop that sums the amount of bytes required to expand pszFormat according to argList %s, %d, ...
		va_list argList;
		va_start(argList, pszFormat);
		TCLS::vsprintf(m_chStr, pszFormat, argList); 
		va_end(argList);
	}

	void FormatV(const TCH *pszFormat,va_list argList)
	{
		TCLS::vsprintf(m_chStr,pszFormat,argList);
	}

	void AppendFormat(const TCH *pszFormat,...)
	{
		va_list argList;
		va_start( argList, pszFormat );
		AppendFormatV( pszFormat, argList );
		va_end( argList );
	}

	void AppendFormatV(const TCH *pszFormat,va_list argList)
	{
		THStringTempl<TCH,TCLS>Templ<TCH,TCLS> sTemp;
		sTemp.FormatV(pszFormat,argList);
		Append(sTemp);
	}

	BSTR AllocSysString() const
	{
		BSTR bstr = TCLS::SysAllocStringLen(m_chStr, GetLength());
		return bstr;
	}
protected:
	TCH *m_chStr;
	UINT m_nBufLen;
	THMutex m_mtx;
};

typedef THStringTempl<TCH,TCLS>Templ<char,THCharsetConvertMapA> THStringTempl<TCH,TCLS>A;
typedef THStringTempl<TCH,TCLS>Templ<wchar_t,THCharsetConvertMapW> THStringTempl<TCH,TCLS>W;
typedef THStringTempl<TCH,TCLS>Templ<TCH,THCharsetConvertMapT> THStringTempl<TCH,TCLS>;

*/
