#pragma once

#define THSYSTEM_FLOAT
#include <THSystem.h>
#include <THString.h>
#include <THCharset.h>
#include <THSyn.h>
//md5lib gzlib aeslib include
#include <THLibArith.h>
//md5 lib include
#include <md5lib\md5.h>
//gz lib include
//#include <gzlib\gzlib.h>
//zlib include
#include <zlib\zlib.h>
#include <THFile.h>
//aes lib include
#include <aeslib\aes.h>
#include <nettlelib\THNettleLib.h>
#include <THMemBuf.h>

/**
* @brief Nettle算法库封装类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2009-09-1 新建类
*/
class THNettle : public THNettleLib
{
public:
	static BOOL HashData(THMemBuf *buf,PTHNETTLEHASHRESULT ret,THNettleHashType type)
	{
		UINT len;
		void *pbuf=buf->GetBuf(&len);
		BOOL bRet=Hash((const uint8_t *)pbuf,len,ret,type);
		buf->ReleaseGetBufLock();
		return bRet;
	}
	static THString HashStr(THMemBuf *buf,THNettleHashType type)
	{
		UINT len;
		void *pbuf=buf->GetBuf(&len);
		THNETTLEHASHRESULT ret;
		Hash((const uint8_t *)pbuf,len,&ret,type);
		buf->ReleaseGetBufLock();
		return HashToString(&ret);
	}
	static THString HashStr(const void *data,unsigned int length,THNettleHashType type)
	{
		THNETTLEHASHRESULT ret;
		Hash((const uint8_t *)data,length,&ret,type);
		return HashToString(&ret);
	}
	static THString HashStr(THString str,THNettleHashType type)
	{
		char *buf=THCharset::t2a(str);
		THNETTLEHASHRESULT ret;
		if (buf)
		{
			Hash((const uint8_t *)buf,(unsigned int)strlen(buf),&ret,type);
			THCharset::free(buf);
		}
		return HashToString(&ret);
	}
	static THString HashFileStr(THString sFile,THNettleHashType type)
	{
		THNETTLEHASHRESULT ret;
		HashFile(sFile,&ret,type);
		return HashToString(&ret);
	}
	static THString HmacStr(THMemBuf *buf,const uint8_t *key,unsigned int keylength,THNettleHashType type)
	{
		UINT len;
		void *pbuf=buf->GetBuf(&len);
		THNETTLEHASHRESULT ret;
		Hmac((const uint8_t *)pbuf,len,key,keylength,&ret,type);
		buf->ReleaseGetBufLock();
		return HashToString(&ret);
	}
	static THString HmacStr(const void *data,unsigned int length,const uint8_t *key,unsigned int keylength,THNettleHashType type)
	{
		THNETTLEHASHRESULT ret;
		Hmac((const uint8_t *)data,length,key,keylength,&ret,type);
		return HashToString(&ret);
	}
	static THString HmacStr(const void *data,unsigned int length,THString key,THNettleHashType type)
	{
		THNETTLEHASHRESULT ret;
		char *buf=THCharset::t2a(key);
		if (buf)
		{
			Hmac((const uint8_t *)data,length,(const uint8_t *)buf,(unsigned int)strlen(buf),&ret,type);
			THCharset::free(buf);
		}
		return HashToString(&ret);
	}
	static THString HmacFileStr(THString sFile,const uint8_t *key,unsigned int keylength,THNettleHashType type)
	{
		THNETTLEHASHRESULT ret;
		HmacFile(sFile,key,keylength,&ret,type);
		return HashToString(&ret);
	}
	static THString HmacFileStr(THString sFile,THString key,THNettleHashType type)
	{
		THNETTLEHASHRESULT ret;
		char *buf=THCharset::t2a(key);
		if (buf)
		{
			HmacFile(sFile,(const uint8_t *)buf,(unsigned int)strlen(buf),&ret,type);
			THCharset::free(buf);
		}
		return HashToString(&ret);
	}
	static THString HashToString(PTHNETTLEHASHRESULT ret)
	{
		return THStringConv::BinToHexString(ret->m,ret->len);
	}
	static BOOL StringToHash(THString str,PTHNETTLEHASHRESULT ret)
	{
		if (str.GetLength()>MAX_NETTLEHASHRESULT*2) return FALSE;
		ret->type=THNettle_Hash_End;
		THStringConv::HexStringToBin(str,ret->m,&ret->len);
		return TRUE;
	}
	static BOOL CipherFile(THString str,const uint8_t *key,unsigned int keylength,THNettleCipherType type,BOOL bCipher)
	{
		unsigned int len;
		void *buf=THFile::GetContentToBuffer(str,&len);
		if (!buf) return FALSE;
		uint8_t *out=new uint8_t[len];
		BOOL bret=FALSE;
		if (out)
		{
			bret=Cipher((const uint8_t *)buf,len,key,keylength,out,type,bCipher);
			if (bret)
				bret=THFile::SaveFile(str,out,len);
		}
		delete [] out;
		THFile::FreeContentBuffer(buf);
		return bret;
	}
	static BOOL CipherFile(THString str,THString key,THNettleCipherType type,BOOL bCipher)
	{
		char *buf=THCharset::t2a(key);
		BOOL bret=FALSE;
		if (buf)
		{
			bret=CipherFile(str,(const uint8_t *)buf,(unsigned int)strlen(buf),type,bCipher);
			THCharset::free(buf);
		}
		return bret;
	}
	static BOOL CipherData(THMemBuf *buf,const uint8_t *key,unsigned int keylength,THNettleCipherType type,BOOL bCipher)
	{
		UINT len;
		void *pbuf=buf->GetBuf(&len);
		uint8_t *out=new uint8_t[len];
		if (!out) return FALSE;
		BOOL bRet=Cipher((const uint8_t *)pbuf,len,key,keylength,out,type,bCipher);
		memcpy(pbuf,out,len);
		delete [] out;
		buf->ReleaseGetBufLock();
		return bRet;
	}
	static BOOL CipherData(THMemBuf *buf,THString key,THNettleCipherType type,BOOL bCipher)
	{
		char *pbuf=THCharset::t2a(key);
		BOOL bret=FALSE;
		if (pbuf)
		{
			bret=CipherData(buf,(const uint8_t *)pbuf,(unsigned int)strlen(pbuf),type,bCipher);
			THCharset::free(pbuf);
		}
		return bret;
	}
	static THString Base64Encode(const void *data,unsigned int length)
	{
		THString sret;
		uint8_t *ret;
		if (ArmorEncode((const uint8_t *)data,length,&ret,THNettle_Armor_Base64))
		{
			sret=THCharset::a2t((const char *)ret);
			Base64Free(ret);
		}
		return sret;
	}
	static BOOL Base64Decode(THString str,void **ret,unsigned int *length)
	{
		char *pbuf=THCharset::t2a(str);
		if (!pbuf) return FALSE;
		BOOL bRet=ArmorDecode((const uint8_t *)pbuf,(uint8_t **)ret,length,THNettle_Armor_Base64);
		THCharset::free(pbuf);
		return bRet;
	}
	inline static void Base64Free(void *data){ArmorFree(data);}
};

typedef struct _MD5BUFFER
{
	_MD5BUFFER()
	{
		memset(m,0,sizeof(m));
	}
	unsigned char m[MD5_DIGEST_LENGTH];
}MD5BUFFER,*PMD5BUFFER;

/**
* @brief Md5处理类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-05-30 新建类
*/
/**<pre>
  使用Sample：
	THMd5 md5;
	THString ret=md5.CalcMd5Str(buf,buflen);
</pre>*/
class THMd5
{
public:
	THMd5(){}
	virtual ~THMd5(void){}

	/**
	* @brief 计算MD5返回结果的二进制数组
	* @param data		待计算的数据指针
	* @param datalen	数据长度
	* @param ret		用于保存返回结果的二进制数组地址
	*/
	static void CalcMd5(const void *data,unsigned long datalen,PMD5BUFFER ret)
	{
		MD5((const unsigned char *)data,datalen,ret->m);
	}

	/**
	* @brief 计算MD5返回字符串结果
	* @param data		待计算的数据指针
	* @param datalen	数据长度
	* @return		返回字符串结果
	*/
	static THString CalcMd5Str(const void *data,unsigned long datalen,BOOL bUpper=TRUE)
	{
		MD5BUFFER ret;
		CalcMd5(data,datalen,&ret);
		return GetMd5Str(&ret,bUpper);
	}

	/**
	* @brief 计算文件的计算文件的智能MD5，即只取前面4k，加最后4k，及每1m的前4k进行md5，返回结果的二进制数组
	* @param FilePath	待计算的文件名
	* @param ret		用于保存返回结果的二进制数组地址
	*/
	static void CalcFileSmartMd5(THString FilePath,PMD5BUFFER ret)
	{
		char *tmpbuffer=THCharset::t2a(FilePath);
		MD5_smartfile(tmpbuffer,ret->m);
		THCharset::free(tmpbuffer);
	}

	/**
	* @brief 计算文件的MD5，返回结果的二进制数组
	* @param FilePath	待计算的文件名
	* @param ret		用于保存返回结果的二进制数组地址
	*/
	static void CalcFileMd5(THString FilePath,PMD5BUFFER ret)
	{
		char *tmpbuffer=THCharset::t2a(FilePath);
		MD5_file(tmpbuffer,ret->m);
		THCharset::free(tmpbuffer);
	}

	/**
	* @brief 计算文件的MD5，返回字符串结果
	* @param FilePath	待计算的文件名
	* @return		返回字符串结果
	*/
	static THString CalcFileMd5Str(THString FilePath,BOOL bUpper=TRUE)
	{
		MD5BUFFER ret;
		CalcFileMd5(FilePath,&ret);
		return GetMd5Str(&ret,bUpper);
	}

	/**
	* @brief 计算文件的智能MD5，即只取前面4k，加最后4k，及每1m的前4k进行md5，返回字符串结果
	* @param FilePath	待计算的文件名
	* @return		返回字符串结果
	*/
	static THString CalcFileSmartMd5Str(THString FilePath,BOOL bUpper=TRUE)
	{
		MD5BUFFER ret;
		CalcFileSmartMd5(FilePath,&ret);
		return GetMd5Str(&ret,bUpper);
	}

	static THString GetMd5Str(PMD5BUFFER md,BOOL bUpper=TRUE)
	{
		TCHAR buf[33];
		const TCHAR uppercnv[]=_T("%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X");
		const TCHAR lowercnv[]=_T("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x");
		const TCHAR *cnv=uppercnv;
		if (!bUpper) cnv=lowercnv;
		_sntprintf (buf,33,cnv,
		md->m[0],md->m[1],md->m[2],md->m[3],md->m[4],md->m[5],md->m[6],md->m[7],
		md->m[8],md->m[9],md->m[10],md->m[11],md->m[12],md->m[13],md->m[14],md->m[15]);
		*(buf+32)='\0'; //last must be zero
		return THString(buf);
	}

	static BOOL GetMd5Buffer(THString md5str,PMD5BUFFER md)
	{
		if (!md) return FALSE;
		if (md5str.GetLength()!=32) return FALSE;
		for(int i=0;i<md5str.GetLength();i+=2)
		{
			THString ch=md5str.Mid(i,2);
			md->m[i/2]=(unsigned char)_tcstol(ch,'\0',16);
		}
		return TRUE;
	}

	/**
	* @brief 比较MD5值,如果数据指针有数据，使用数据指针的值，否则使用字符串的值
	* @param str1		比较字符串1
	* @param buf1		比较数据1
	* @param str2		比较字符串2
	* @param buf2		比较数据2
	* @return		返回字符串比较结果
	*/
	static BOOL IsMd5Same(THString str1,THString str2,const PMD5BUFFER buf1=NULL,const PMD5BUFFER buf2=NULL)
	{
		if (buf1) str1=GetMd5Str(buf1);
		if (buf2) str2=GetMd5Str(buf2);
		if (str1.IsEmpty() || str2.IsEmpty()) return FALSE;
		return (str1.CompareNoCase(str2)==0);
	}

	/**
	* @brief 计算字符串的md5，返回md5结果
	* @param str		待计算的字符串
	* @return		返回字符串结果
	*/
	static BOOL CalcStrMd5(THString str,PMD5BUFFER buf)
	{
		char *tmp=THCharset::t2a(str);
		if (tmp)
		{
			CalcMd5(tmp,(unsigned long)strlen(tmp),buf);
			THCharset::free(tmp);
			return TRUE;
		}
		return FALSE;
	}

	/**
	* @brief 计算字符串的md5，返回字符串结果
	* @param str		待计算的字符串
	* @return		返回字符串结果
	*/
	static THString CalcStrMd5Str(THString str,BOOL bUpper=TRUE)
	{
		MD5BUFFER ret;
		if (!CalcStrMd5(str,&ret)) return _T("");
		return GetMd5Str(&ret,bUpper);
	}

	/**
	* @brief 计算字符串的md5，返回md5结果
	* @param buf	待计算的缓存数据
	* @param pbuf	用于保存返回结果的二进制数组地址
	* @return		返回字符串结果
	*/
	static BOOL CalcBufMd5(THMemBuf *buf,PMD5BUFFER pbuf)
	{
		MD5BUFFER ret;
		CalcMd5(buf->GetBuf(NULL,FALSE),buf->GetBufferedSize(),pbuf);
		return TRUE;
	}

	/**
	* @brief 计算缓存数据的md5，返回字符串结果
	* @param buf	待计算的缓存数据
	* @return		返回字符串结果
	*/
	static THString CalcBufMd5Str(THMemBuf *buf,BOOL bUpper=TRUE)
	{
		MD5BUFFER ret;
		if (!CalcBufMd5(buf,&ret)) return _T("");
		return GetMd5Str(&ret,bUpper);
	}

	/**
	* @brief 比较是否为空MD5值
	* @param md	md5值
	* @return		返回结果
	*/
	static BOOL IsEmpty(PMD5BUFFER md)
	{
		if (!md) return TRUE;
		return (memcmp(md,"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0",16)==0);
	}

	/**
	* @brief 比较是否为空MD5值
	* @param md	md5值
	* @return		返回结果
	*/
	static BOOL IsEmpty(THString md)
	{
		md.Trim(_T(" 0"));
		return md.IsEmpty();
	}
};

/**
* @brief Aes处理类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-08-16 新建类
*/
/**<pre>
  注意：由于Aes加密是16位字符对齐的，因此本类各处理在一开始增加一长度位及特定标识进行标识，因此不是标准实现
  使用Sample：
	THAes aes;
	aes.EncryptFile(_T("c:\\CopyLocation.dll"),_T("c:\\CopyLocation.aes"),_T("123456"));
	aes.DecryptFile(_T("c:\\CopyLocation.aes"),_T("c:\\CopyLocation1.dll"),_T("123456"));
	aes.EncryptFile(_T("c:\\CopyLocation.txt"),_T("c:\\CopyLocation1.aes"),_T("123456"));
	aes.DecryptFile(_T("c:\\CopyLocation1.aes"),_T("c:\\CopyLocation1.txt"),_T("123456"));
	THString str=aes.EncryptText(_T("mytext"),_T("123456"));
	THDebug(_T("Encode:%s,Decode:%s"),str,aes.DecryptText(str,_T("123456")));
	int retlen;
	void *pret=aes.EncryptBuffer("124325",6,&retlen,_T("123456"));
	if (pret)
	{
		THDebug(THStringConv::FormatString(pret,retlen));
		void *ret1=aes.DecryptBuffer(pret,retlen,&retlen,_T("123456"));
		if (ret1)
		{
			THDebug(THStringConv::FormatString(ret1,retlen));
			aes.Free(ret1);
		}
		aes.Free(pret);
	}
</pre>*/
class THAes
{
public:
	THAes(){}
	virtual ~THAes(){}

	BOOL EncryptFile(THString sInFile,THString sOutFile,THString sPass)
	{
		char *input=THCharset::t2a(sInFile);
		char *output=THCharset::t2a(sOutFile);
		char *pass=THCharset::t2a(sPass);
		BOOL ret=FALSE;
		THSingleLock lock(&m_mutex);
		if (input && output && pass)
			ret=m_aes.AesEncryptFileEx(input,output,pass);
		if (input) THCharset::free(input);
		if (output) THCharset::free(output);
		if (pass) THCharset::free(pass);
		return ret;
	}
	BOOL DecryptFile(THString sInFile,THString sOutFile,THString sPass)
	{
		char *input=THCharset::t2a(sInFile);
		char *output=THCharset::t2a(sOutFile);
		char *pass=THCharset::t2a(sPass);
		BOOL ret=FALSE;
		THSingleLock lock(&m_mutex);
		if (input && output && pass)
			ret=m_aes.AesDecryptFileEx(input,output,pass);
		if (input) THCharset::free(input);
		if (output) THCharset::free(output);
		if (pass) THCharset::free(pass);
		return ret;
	}

	THString EncryptText(THString sText,THString sPass)
	{
		char *input=THCharset::t2a(sText);
		char *pass=THCharset::t2a(sPass);
		THString ret;
		if (input && pass)
		{
			THSingleLock lock(&m_mutex);
			char *chret=m_aes.AesEncryptTextEx(input,pass);
			if (chret)
			{
				ret=THCharset::a2t(chret);
				m_aes.AesFree(chret);
			}
		}
		if (input) THCharset::free(input);
		if (pass) THCharset::free(pass);
		return ret;
	}
	THString DecryptText(THString sText,THString sPass)
	{
		char *input=THCharset::t2a(sText);
		char *pass=THCharset::t2a(sPass);
		THString ret;
		if (input && pass)
		{
			THSingleLock lock(&m_mutex);
			char *chret=m_aes.AesDecryptTextEx(input,pass);
			if (chret)
			{
				ret=THCharset::a2t(chret);
				m_aes.AesFree(chret);
			}
		}
		if (input) THCharset::free(input);
		if (pass) THCharset::free(pass);
		return ret;
	}

	//EncryptBuffer为非标准实现，加入了保存大小，使解密后数据能知道准确大小
	void* EncryptBuffer(const void *buf,unsigned int nInLen,int *nOutLen,THString sPass)
	{
		if (nOutLen) *nOutLen=0;
		char *pass=THCharset::t2a(sPass);
		if (!pass) return NULL;
		THSingleLock lock(&m_mutex);
		void *ret=(void *)m_aes.AesEncryptBufferEx(buf,nInLen,nOutLen,pass);
		if (pass) THCharset::free(pass);
		return ret;
	}
	void* DecryptBuffer(const void *buf,unsigned int nInLen,int *nOutLen,THString sPass)
	{
		if (nOutLen) *nOutLen=0;
		char *pass=THCharset::t2a(sPass);
		if (!pass) return NULL;
		THSingleLock lock(&m_mutex);
		void *ret=(void *)m_aes.AesDecryptBufferEx(buf,nInLen,nOutLen,pass);
		if (pass) THCharset::free(pass);
		return ret;
	}
	//标准实现
	void* EncryptBufferSTD(const void *buf,unsigned int nInLen,int *nOutLen,THString sPass)
	{
		if (nOutLen) *nOutLen=0;
		char *pass=THCharset::t2a(sPass);
		if (!pass) return NULL;
		THSingleLock lock(&m_mutex);
		void *ret=(void *)m_aes.AesEncryptBuffer(buf,nInLen,nOutLen,pass);
		if (pass) THCharset::free(pass);
		return ret;
	}
	//标准实现
	void* DecryptBufferSTD(const void *buf,unsigned int nInLen,int *nOutLen,THString sPass)
	{
		if (nOutLen) *nOutLen=0;
		char *pass=THCharset::t2a(sPass);
		if (!pass) return NULL;
		THSingleLock lock(&m_mutex);
		void *ret=(void *)m_aes.AesDecryptBuffer(buf,nInLen,nOutLen,pass);
		if (pass) THCharset::free(pass);
		return ret;
	}
	void Free(void *buf){return m_aes.AesFree(buf);}

	THString GetLastError()
	{
		THSingleLock lock(&m_mutex);
		THString ret;
		char *tmp=m_aes.AesGetLastError();
		if (tmp)
		{
			ret=THCharset::a2t(tmp);
			m_aes.AesFree(tmp);
		}
		return ret;
	}
protected:
	THAsciiAes m_aes;
	THMutex m_mutex;
};

/**
* @brief 简单Rsa处理类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-08-19 新建类
*/
/**<pre>
  注意：该类加解密强度比较薄弱，是为了兼容在网页中使用制作的。
        网页文件为tinirsa.asp，测试网页例子为tinirsatest.asp。
		该类按asp中实现重写了C代码，asp代码中本身有bug，使用中请注意，如：公钥 63751 私钥 26583 模块 62449 编码 1234 时，解码错误。
		另外asp中不能对中文进行加解密处理，C实现中，两个问题都不存在。
  使用Sample：
	THTiniRsa trsa;
	trsa.GenKey();
	THString tstr=trsa.Encode("测试",4);
	THDebug(_T("%s"),tstr);
	unsigned int tret;
	void *tmp=trsa.Decode(tstr,&tret);
	if (tmp)
	{
		THDebug(_T("%s"),THStringConv::FormatString(tmp,tret));
		trsa.Free(tmp);
	}
</pre>*/
class THTiniRsa
{
public:
	void GenKey()
	{
		unsigned int lLngPhi;
		unsigned int q,p;
		THSysMisc::srand();
		do
		{
			do
			{
				//2 random primary numbers (0 to 1000)
				do
				{
					p = rand()%1000;
				}while(!IsPrime(p));
				do
				{
					q = rand()%1000;
				}while(!IsPrime(q));
				// n = product of 2 primes
				Modulus = p * q;
				// random decryptor (2 to n)
				PrivateKey = rand() % (Modulus - 2)  + 2;
				lLngPhi = (p - 1) * (q - 1);
				PublicKey = Euler(lLngPhi, PrivateKey);
			}while(PublicKey == 0 || PublicKey == 1);
			//Loop if we can't crypt/decrypt a byte
		}while(!TestCrypt(255));
	}

	unsigned int Crypt(unsigned int ch,unsigned int key)
	{
		unsigned int lLngMod;
		unsigned int lLngResult;
		if (key%2==0)
		{
			lLngResult = 1;
			for(unsigned int i=0;i<key/2;i++)
			{
				lLngMod = (ch * ch) % Modulus;
				//Mod may error on key generation
				lLngResult = (lLngMod * lLngResult) % Modulus;
			}
		}
		else
		{
			lLngResult = ch;
			for(unsigned int i=0;i<key/2;i++)
			{
				lLngMod = (ch * ch) % Modulus;
				// Mod may error on key generation
				lLngResult = (lLngMod * lLngResult) % Modulus;
			}
		}
		return lLngResult;
	}

	THString Encode(const void *buf,unsigned int len)
	{
		THString strret;
		if (len > 0)
		{
			const unsigned char*tmp=(const unsigned char*)buf;
			unsigned int ret;
			//unsigned char*out=new unsigned char[len];
			for(unsigned int i=0;i<len;i++)
			{
				ret=Crypt(*(tmp+i),PublicKey);
				strret.AppendFormat(_T("%04X"),ret);
			}
		}
		return strret;
	}

	void *Decode(THString msg,unsigned int *len)
	{
		if (msg.GetLength()==0 || msg.GetLength()%4!=0 || !len) return NULL;
		*len=msg.GetLength()/4;
		unsigned char *tmp=new unsigned char[*len];
		if (!tmp) return NULL;
		for(unsigned int i=0;i<(unsigned int)msg.GetLength();i+=4)
			*(tmp+i/4)=(unsigned char)Crypt(THStringConv::s2h(msg.Mid(i).Left(4)),PrivateKey);
		return tmp;
	}

	void Free(void *buf){if (buf) delete [] (char *)buf;}

	unsigned int GetPublicKey(){return PublicKey;}
	unsigned int GetPrivateKey(){return PrivateKey;}
	unsigned int GetModule(){return Modulus;}
	void SetKey(unsigned int publickey,unsigned int privatekey,unsigned int module)
	{
		PublicKey=publickey;
		PrivateKey=privatekey;
		Modulus=module;
	}
protected:
	BOOL TestCrypt(unsigned int ch)
	{
		unsigned int out = Crypt(ch, PublicKey);
		return (Crypt(out, PrivateKey)==ch);
	}

	unsigned int Euler(unsigned int pLngPHI, unsigned int pLngKey)
	{
		int lLngR[3];
		int lLngP[3];
		int lLngQ[3];

		unsigned int lLngCounter;
		int lLngResult;

		unsigned int ret = 0;

		lLngR[1] = pLngPHI;
		lLngR[0] = pLngKey;
		lLngP[1] = 0;
		lLngP[0] = 1;
		lLngQ[1] = 2;
		lLngQ[0] = 0;

		lLngCounter = (unsigned int)-1;

		while(lLngR[0]!=0)
		{
			lLngR[2] = lLngR[1];
			lLngR[1] = lLngR[0];
			lLngP[2] = lLngP[1];
			lLngP[1] = lLngP[0];
			lLngQ[2] = lLngQ[1];
			lLngQ[1] = lLngQ[0];
			lLngCounter = lLngCounter + 1;
			lLngR[0] = lLngR[2] % lLngR[1];
			lLngP[0] = ((lLngR[2]/lLngR[1]) * lLngP[1]) + lLngP[2];
			lLngQ[0] = ((lLngR[2]/lLngR[1]) * lLngQ[1]) + lLngQ[2];
		}

		lLngResult = (pLngKey * lLngP[1]) - (pLngPHI * lLngQ[1]);

		if (lLngResult > 0)
			ret = lLngP[1];
		else
			ret = abs(lLngP[1]) + pLngPHI;
		return ret;
	}

	BOOL IsPrime(unsigned int pLngNumber)
	{
		double lLngSquare;
		int lLngIndex;
		if (pLngNumber>2)
		{
			if((pLngNumber%2)!=0)
			{
				lLngSquare = sqrt((double)pLngNumber);
				for(lLngIndex=3;lLngIndex<=lLngSquare;lLngIndex+=2)
				{
					if((pLngNumber%lLngIndex)==0) return FALSE;
				}
				return TRUE;
			}
		}
		return FALSE;
	}
protected:
	unsigned int PrivateKey;
	unsigned int PublicKey;
	unsigned int Modulus;
};

/**
* @brief Base64处理类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-06-03 新建类
*/
/**<pre>
  使用Sample：
</pre>*/
class THBase64
{
public:
	static THString EncodeStr(THString orgbuf,BOOL bInsertBr=FALSE)
	{
		THString ret;
		char *tmp=THCharset::t2a(orgbuf);
		if (tmp)
		{
			ret=Encode(tmp,(int)strlen(tmp),bInsertBr);
			THCharset::free(tmp);
		}
		return ret;
	}

	static THString EncodeStrA(THStringA orgbuf,BOOL bInsertBr=FALSE) {return Encode(orgbuf,orgbuf.GetLength(),bInsertBr);}

	static THString Encode(const void *pEncBuf,int nSize,BOOL bInsertBr=FALSE)
	{
		//Base64编码字符集：
		THString m_sBase64Alphabet = _T("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");
		THString strResult;
		TCHAR *szOutput;
		//计算空间
		int size = (nSize + 2) / 57 * 2;
		size += nSize % 3 != 0 ? (nSize - nSize % 3 + 3) / 3 * 4 : nSize / 3 * 4;
		if (bInsertBr) size+=(nSize+3)/3*4/76+1;
		szOutput=strResult.GetBuffer(size + 1);
		memset(szOutput,0,sizeof(TCHAR)*(size+1));

		const char*szInput=(const char*)pEncBuf;

		int nBitsRemaining = 0, nPerRowCount = 0;//换行计数
		register int nBitStorage = 0, nScratch = 0;
		int i, lp, endlCount;

		for(i=0, lp=0, endlCount = 0; lp < nSize; lp++)
		{
			nBitStorage = (nBitStorage << 8) | (szInput[lp] & 0xff);//1 byte//the lowest-byte to 0 not cycle
			nBitsRemaining += 8;//读了一个字节，加八位

			do//nBitStorage"剩下的位"记录变量
			{
				nScratch = nBitStorage >> (nBitsRemaining - 6) & 0x3f;//提出最前的六位
				szOutput[i++] = m_sBase64Alphabet[nScratch];
				nPerRowCount++;
				if(nPerRowCount >= 76)
				{
					if (bInsertBr)
					{
						szOutput[i++] = '\r';
						szOutput[i++] = '\n';
						endlCount += 2;
					}
					nPerRowCount = 0;
				}
				nBitsRemaining -= 6;
			}while(nBitsRemaining >= 6);
		}

		if(nBitsRemaining != 0)
		{
			//原数据最后多一个两个字节时进入，编码未结束nBitsRemaining!=0
			nScratch = nBitStorage << (6-nBitsRemaining);//空位补0
			nScratch &= 0x3f;
			szOutput[i++] = m_sBase64Alphabet[nScratch];
			nPerRowCount++;
			if(nPerRowCount >= 76)
			{
				if (bInsertBr)
				{
					szOutput[i++] = '\r';
					szOutput[i++] = '\n';
					endlCount += 2;
				}
				nPerRowCount = 0;
			}
		} 

		// Pad with '=' as per RFC 1521
		while((i - endlCount) % 4 != 0)
		{
			szOutput[i++] = '=';
			nPerRowCount++;
			if(nPerRowCount >= 76)
			{
				if (bInsertBr)
				{
					szOutput[i++] = '\r';
					szOutput[i++] = '\n';
					endlCount += 2;
				}
				nPerRowCount = 0;
			}
		}
		strResult.ReleaseBuffer();
		return strResult;
	}

	static THString DecodeStr(THString text)
	{
		THString ret;
		void *data;
		int size;
		if (Decode(text,&data,&size))
		{
			ret=THCharset::a2t((char *)data,size);
			FreeBuffer(data);
		}
		return ret;
	}

	static BOOL Decode(THString text,void **data, int *size)
	{
		unsigned char *out = NULL;
		unsigned char tmp = 0;
		const TCHAR *c;
		int tmp2 = 0;
		int len = 0, n = 0;

		if (text.IsEmpty() || !data) return FALSE;

		//Normalize the input
		text.Replace(_T("\r"),_T(""));
		text.Replace(_T("\n"),_T(""));
		text.Replace(_T("\t"),_T(""));

		c = text;

		while (*c) {
			if (*c >= 'A' && *c <= 'Z') {
				tmp = *c - 'A';
			} else if (*c >= 'a' && *c <= 'z') {
				tmp = 26 + (*c - 'a');
			} else if (*c >= '0' && *c <= 57) {
				tmp = 52 + (*c - '0');
			} else if (*c == '+') {
				tmp = 62;
			} else if (*c == '/') {
				tmp = 63;
			} else if (*c == '=') {
				if (n == 3) {
					out = (unsigned char *)realloc(out, len + 2);
					if (!out) return FALSE;
					out[len] = (char)(tmp2 >> 10) & 0xff;
					len++;
					out[len] = (char)(tmp2 >> 2) & 0xff;
					len++;
				} else if (n == 2) {
					out = (unsigned char *)realloc(out, len + 1);
					if (!out) return FALSE;
					out[len] = (char)(tmp2 >> 4) & 0xff;
					len++;
				}
				break;
			}
			tmp2 = ((tmp2 << 6) | (tmp & 0xff));
			n++;
			if (n == 4) {
				out = (unsigned char *)realloc(out, len + 3);
				if (!out) return FALSE;
				out[len] = (char)((tmp2 >> 16) & 0xff);
				len++;
				out[len] = (char)((tmp2 >> 8) & 0xff);
				len++;
				out[len] = (char)(tmp2 & 0xff);
				len++;
				tmp2 = 0;
				n = 0;
			}
			c++;
		}

		out = (unsigned char *)realloc(out, len + 1);
		if (!out) return FALSE;
		out[len] = 0;

		*data = out;
		if (size)
			*size = len;
		return TRUE;
	}

	static void FreeBuffer(void *buf){free(buf);}
};

#define THGZIP_DEFLATE	0		///<deflate(RFC1951)，使用LZ77和哈弗曼进行编码（ie支持的deflate其实是zlib），数据直接是压缩内容
#define THGZIP_ZLIB		1		///<zlib(RFC1950)，zip封装deflate，数据以0x78开头
#define THGZIP_GZIP		2		///<gzip(RFC1952)，gz封装deflate，数据以0x1f 0x8b开头
/*
例子数据：
deflate                            2b 2a 49 2d 2e 01 00
zlib                         78 9c 2b 2a 49 2d 2e 01 00 06 98 02 33
gzip 1f 8b 08 00 00 00 00 00 00 0b 2b 2a 49 2d 2e 01 00 a2 e5 0b bc 05 00 00 00
*/

/**
* @brief GZip封装类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-08-13 新建类
* @2008-03-10 更正解压时缓冲不够的问题
*/
/**<pre>
  使用Sample：
</pre>*/
class THGZip
{
public:
	THGZip(){}
	virtual ~THGZip(){}

	/**
	* @brief 解码数据，自动侦测Zip数据类型
	* @param compr			压缩数据
	* @param comprLen		压缩数据大小
	* @param buf			传入动态内存管理类指针，注意，这里不会清空类中原有的数据
	* @return 返回是否成功
	*/
	static BOOL Decode2(const void *compr, unsigned long comprLen,THMemBuf *buf) {return ZipDecode2(compr,comprLen,buf,_GetZipMode(compr,comprLen));}

	/**
	* @brief 解码数据，推荐使用Decode2，Decode某些时候会解压错误，自动侦测Zip数据类型
	* @param buffer		压缩数据
	* @param size		压缩数据大小
	* @param retbuf		返回解压数据的指针，使用完需要调用THGZip::Free释放
	* @param retsize	返回解压数据的大小，不需要指定
	* @param calcsize	预计解压大小
	* @return 返回是否成功
	*/
	static BOOL Decode(const void *buffer,unsigned long size,void **retbuf,unsigned long *retsize,int mode=THGZIP_ZLIB,unsigned int calcsize=0) {return ZipDecode(buffer,size,retbuf,retsize,_GetZipMode(buffer,size),calcsize);}

	/**
	* @brief 解码数据，需要指定Zip数据类型
	* @param compr			压缩数据
	* @param comprLen		压缩数据大小
	* @param buf			传入动态内存管理类指针，注意，这里不会清空类中原有的数据
	* @param mode			THGZIP_XXX，解压的压缩格式，默认是THGZIP_ZLIB
	* @return 返回是否成功
	*/
	static BOOL ZipDecode2(const void *compr, unsigned long comprLen,THMemBuf *buf,int mode=THGZIP_ZLIB)
	{
		if (!buf) return FALSE;
		int winbit=_GetWinbitByMode(mode);
		int ret;
		unsigned have;
		z_stream strm;
		unsigned char out[16384];
		/* allocate inflate state */
		strm.zalloc = Z_NULL;
		strm.zfree = Z_NULL;
		strm.opaque = Z_NULL;
		strm.avail_in = 0;
		strm.next_in = Z_NULL;
		ret = inflateInit2(&strm,winbit);
		if (ret != Z_OK) return FALSE;
		/* decompress until deflate stream ends or end of file */
		strm.avail_in = comprLen;
		strm.next_in = (Bytef *)compr;
		/* run inflate() on input until output buffer not full */
		do {
			strm.avail_out = 16384;
			strm.next_out = out;
			ret = inflate(&strm, Z_NO_FLUSH);
			ASSERT(ret != Z_STREAM_ERROR);
			/* state not clobbered */
			switch (ret){
				case Z_NEED_DICT:
					ret = Z_DATA_ERROR;
				/* and fall through */
				case Z_DATA_ERROR:
				case Z_MEM_ERROR:
					(void)inflateEnd(&strm);
					return FALSE;
			}
			have = 16384 - strm.avail_out;
			if (!buf->AddBuf(out,have)) return FALSE;
		} while (strm.avail_out == 0);
		//(ret != Z_STREAM_END);
		/* clean up and return */
		(void)inflateEnd(&strm);
		return TRUE;
	}

	/**
	* @brief 解码数据，推荐使用Decode2，Decode某些时候会解压错误，需要指定Zip数据类型
	* @param buffer		压缩数据
	* @param size		压缩数据大小
	* @param retbuf		返回解压数据的指针，使用完需要调用THGZip::Free释放
	* @param retsize	返回解压数据的大小，不需要指定
	* @param mode		THGZIP_XXX，解压的压缩格式，默认是THGZIP_ZLIB
	* @param calcsize	预计解压大小
	* @return 返回是否成功
	*/
	static BOOL ZipDecode(const void *buffer,unsigned long size,void **retbuf,unsigned long *retsize,int mode=THGZIP_ZLIB,unsigned int calcsize=0)
	{
		if (!buffer || size<=0 || !retbuf || !retsize) return FALSE;
		int winbit=_GetWinbitByMode(mode);
		if (calcsize==0) calcsize=max((unsigned long)4096,size*3);
		while(1)
		{
			*retbuf=new char[calcsize];
			if (*retbuf==NULL) return FALSE;
			*retsize=calcsize;
			int ret=uncompress((Bytef *)*retbuf,retsize,(const Bytef *)buffer,size,winbit);
			if (ret==Z_OK) return TRUE;
			else if (ret==Z_MEM_ERROR || ret==Z_BUF_ERROR)
			{
				delete [] *retbuf;
				calcsize*=2;
				*retbuf=new char[calcsize];
				if (*retbuf==NULL) return FALSE;
			}
			else
			{
				delete [] *retbuf;
				*retbuf=NULL;
				return FALSE;
			}
		}
		return TRUE;
	}
	/**
	* @brief 编码数据
	* @param buffer			待压缩数据
	* @param size			待压缩数据大小
	* @param retbuf			返回压缩数据的指针，使用完需要调用THGZip::Free释放
	* @param retsize		返回压缩数据的大小，不需要赋值
	* @param mode			THGZIP_XXX，生成的压缩格式，默认是THGZIP_ZLIB
	* @param level			压缩质量，0-9，默认为-1，由zlib选择默认的6
	* @return 返回是否成功
	*/
	static BOOL ZipEncode(const void *buffer,unsigned long size,void **retbuf,unsigned long *retsize,int mode=THGZIP_ZLIB,int level=Z_DEFAULT_COMPRESSION)
	{
		if (!buffer || size<=0 || !retbuf || !retsize) return FALSE;
		int winbit=_GetWinbitByMode(mode);
		unsigned int calcsize=max((unsigned long)4096,size);
		while(1)
		{
			*retbuf=new char[calcsize];
			if (*retbuf==NULL) return FALSE;
			*retsize=calcsize;
			int ret=compress2((Bytef *)*retbuf,retsize,(const Bytef *)buffer,size,level,winbit);
			if (ret==Z_OK) return TRUE;
			else if (ret==Z_MEM_ERROR || ret==Z_BUF_ERROR)
			{
				delete [] *retbuf;
				calcsize*=2;
				*retbuf=new char[calcsize];
				if (*retbuf==NULL) return FALSE;
			}
			else
			{
				delete [] *retbuf;
				*retbuf=NULL;
				return FALSE;
			}
		}
		return TRUE;
	}

	/**
	* @brief 解压gzlib文件，只支持gz文件
	* @param zippath	gz文件路径
	* @param unzippath	解压目录
	* @return 返回是否成功
	*/
	static BOOL GZipDecodeFile(THString zippath,THString unzippath)
	{
		char *zpath=THCharset::t2a(zippath);
		if (!zpath) return FALSE;
		BOOL ret=FALSE;
		THFile of;
		if (of.Open(unzippath,THFile::modeCreate|THFile::modeWrite))
		{
			gzFile f=gzopen(zpath,"rb");
			if (f)
			{
				char buf[4096];
				while(1)
				{
					int read=gzread(f,buf,4096);
					if (read==0) ret=TRUE;
					if (read<=0) break;
					of.Write(buf,read);
				}
				gzclose(f);
			}
			of.Close();
		}
		if (zpath) THCharset::free(zpath);
		return ret;
	}

	/**
	* @brief 压缩gzlib文件，只支持gz文件
	* @param cntpath	压缩目录
	* @param zippath	gz文件路径
	* @return 返回是否成功
	*/
	static BOOL GZipEncodeFile(THString cntpath,THString zippath)
	{
		char *zpath=THCharset::t2a(zippath);
		if (!zpath) return FALSE;
		BOOL ret=FALSE;
		THFile of;
		if (of.Open(cntpath,THFile::modeRead))
		{
			gzFile f=gzopen(zpath,"wb");
			if (f)
			{
				char buf[4096];
				while(1)
				{
					UINT read=of.Read(buf,4096);
					if (read==0) break;
					gzwrite(f,buf,read);
				}
				gzclose(f);
			}
			of.Close();
		}
		if (zpath) THCharset::free(zpath);
		return ret;
	}

	static void Free(void *buf){delete [] (char *)buf;}
protected:
	static int _GetWinbitByMode(int mode)
	{
		int winbit;
		if (mode==THGZIP_DEFLATE) winbit=-MAX_WBITS;//deflate,-15~-1,-winbit得出winbit
		else if (mode==THGZIP_GZIP) winbit=MAX_WBITS+16;//gzip >15的减去16，得出winbit 1~15
		else winbit=MAX_WBITS;//zlib 0~15,直接使用
		return winbit;
	}
	static int _GetZipMode(const void *compr, unsigned long comprLen)
	{
		const unsigned char *c=(const unsigned char *)compr;
		if (comprLen>=1 && c[0]==0x78) return THGZIP_ZLIB;
		if (comprLen>=2 && c[0]==0x1f && c[1]==0x8b) return THGZIP_GZIP;
		return THGZIP_DEFLATE;
	}
};

#define CRC16_CCITT		0x1021
#define CRC16_DEFAULT	0x8005
#define CRC32_DEFAULT	0x04C10DB7
/**
* @brief Crc封装类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-05-12 新建类
*/
/**<pre>
	CRC算法简介：
		CRC校验的基本思想是利用线性编码理论，在发送端根据要传送的k位二进制码序列，
		以一定的规则产生一个校验用的监督码（既CRC码）r位，并附在信息后边，构成
		一个新的二进制码序列数共(k+r)位，最后发送出去。在接收端，则根据信息码和
		CRC码之间所遵循的规则进行检验，以确定传送中是否出错。 
		16位的CRC码产生的规则是先将要发送的二进制序列数左移16位（既乘以 ）后，再
		除以一个多项式，最后所得到的余数既是CRC码，如式（2-1）式所示，其中B(X)表
		示n位的二进制序列数，G(X)为多项式，Q(X)为整数，R(X)是余数（既CRC码）。 
		（2-1） 求CRC码所采用模2加减运算法则，既是不带进位和借位的按位加减，这种加
		减运算实际上就是逻辑上的异或运算，加法和减法等价，乘法和除法运算与普通代数
		式的乘除法运算是一样，符合同样的规律。生成CRC码的多项式如下，其中CRC-16和
		CRC-CCITT产生16位的CRC码，而CRC-32则产生的是32位的CRC码. 
		CRC-16：（美国二进制同步系统中采用） 
		CRC-CCITT：（由欧洲CCITT推荐） 
		CRC-32： 
		接收方将接收到的二进制序列数（包括信息码和CRC码）除以多项式，如果余数为0，
		则说明传输中无错误发生，否则说明传输有误，关于其原理这里不再多述。用软件
		计算CRC码时，接收方可以将接收到的信息码求CRC码，比较结果和接收到的CRC码是否相同。
</pre>*/
class THCrc
{
public:
	THCrc(){}
	virtual ~THCrc(){}

	static U16 Crc16( const void* pData, unsigned int nSize)
	{
		const unsigned char *aData=(const unsigned char *)pData;
		U16 nAccum = 0;
		static U32 Table_CRC16[256]={0x00000000,0x00008005,0x0000800F,0x0000000A,0x0000801B,0x0000001E,0x00000014,0x00008011,0x00008033,0x00000036,0x0000003C,0x00008039,0x00000028,0x0000802D,0x00008027,0x00000022,0x00008063,0x00000066,0x0000006C,0x00008069,0x00000078,0x0000807D,0x00008077,0x00000072,0x00000050,0x00008055,0x0000805F,0x0000005A,0x0000804B,0x0000004E,0x00000044,0x00008041,0x000080C3,0x000000C6,0x000000CC,0x000080C9,0x000000D8,0x000080DD,0x000080D7,0x000000D2,0x000000F0,0x000080F5,0x000080FF,0x000000FA,0x000080EB,0x000000EE,0x000000E4,0x000080E1,0x000000A0,0x000080A5,0x000080AF,0x000000AA,0x000080BB,0x000000BE,0x000000B4,0x000080B1,0x00008093,0x00000096,0x0000009C,0x00008099,0x00000088,0x0000808D,0x00008087,0x00000082,0x00008183,0x00000186,0x0000018C,0x00008189,0x00000198,0x0000819D,0x00008197,0x00000192,0x000001B0,0x000081B5,0x000081BF,0x000001BA,0x000081AB,0x000001AE,0x000001A4,0x000081A1,0x000001E0,0x000081E5,0x000081EF,0x000001EA,0x000081FB,0x000001FE,0x000001F4,0x000081F1,0x000081D3,0x000001D6,0x000001DC,0x000081D9,0x000001C8,0x000081CD,0x000081C7,0x000001C2,0x00000140,0x00008145,0x0000814F,0x0000014A,0x0000815B,0x0000015E,0x00000154,0x00008151,0x00008173,0x00000176,0x0000017C,0x00008179,0x00000168,0x0000816D,0x00008167,0x00000162,0x00008123,0x00000126,0x0000012C,0x00008129,0x00000138,0x0000813D,0x00008137,0x00000132,0x00000110,0x00008115,0x0000811F,0x0000011A,0x0000810B,0x0000010E,0x00000104,0x00008101,0x00008303,0x00000306,0x0000030C,0x00008309,0x00000318,0x0000831D,0x00008317,0x00000312,0x00000330,0x00008335,0x0000833F,0x0000033A,0x0000832B,0x0000032E,0x00000324,0x00008321,0x00000360,0x00008365,0x0000836F,0x0000036A,0x0000837B,0x0000037E,0x00000374,0x00008371,0x00008353,0x00000356,0x0000035C,0x00008359,0x00000348,0x0000834D,0x00008347,0x00000342,0x000003C0,0x000083C5,0x000083CF,0x000003CA,0x000083DB,0x000003DE,0x000003D4,0x000083D1,0x000083F3,0x000003F6,0x000003FC,0x000083F9,0x000003E8,0x000083ED,0x000083E7,0x000003E2,0x000083A3,0x000003A6,0x000003AC,0x000083A9,0x000003B8,0x000083BD,0x000083B7,0x000003B2,0x00000390,0x00008395,0x0000839F,0x0000039A,0x0000838B,0x0000038E,0x00000384,0x00008381,0x00000280,0x00008285,0x0000828F,0x0000028A,0x0000829B,0x0000029E,0x00000294,0x00008291,0x000082B3,0x000002B6,0x000002BC,0x000082B9,0x000002A8,0x000082AD,0x000082A7,0x000002A2,0x000082E3,0x000002E6,0x000002EC,0x000082E9,0x000002F8,0x000082FD,0x000082F7,0x000002F2,0x000002D0,0x000082D5,0x000082DF,0x000002DA,0x000082CB,0x000002CE,0x000002C4,0x000082C1,0x00008243,0x00000246,0x0000024C,0x00008249,0x00000258,0x0000825D,0x00008257,0x00000252,0x00000270,0x00008275,0x0000827F,0x0000027A,0x0000826B,0x0000026E,0x00000264,0x00008261,0x00000220,0x00008225,0x0000822F,0x0000022A,0x0000823B,0x0000023E,0x00000234,0x00008231,0x00008213,0x00000216,0x0000021C,0x00008219,0x00000208,0x0000820D,0x00008207,0x00000202};
		/*static U32 *Table_CRC16=NULL;
		if (!Table_CRC16)
		{
			Table_CRC16=new U32[256];
			if (!Table_CRC16) return 0;
			BuildTable16(CRC16_DEFAULT, Table_CRC16);
		}*/
		for (unsigned int i = 0; i < nSize; i++ )
			nAccum = ( nAccum << 8 ) ^ (U16)Table_CRC16[( nAccum >> 8 ) ^ *aData++];
		return nAccum;
	}

	static U16 Crc16Ccitt( const void* pData, unsigned int nSize)
	{
		const unsigned char *aData=(const unsigned char *)pData;
		U16 nAccum = 0;
		static U32 Table_CRC16CCITT[256]={0x00000000,0x00001021,0x00002042,0x00003063,0x00004084,0x000050A5,0x000060C6,0x000070E7,0x00008108,0x00009129,0x0000A14A,0x0000B16B,0x0000C18C,0x0000D1AD,0x0000E1CE,0x0000F1EF,0x00001231,0x00000210,0x00003273,0x00002252,0x000052B5,0x00004294,0x000072F7,0x000062D6,0x00009339,0x00008318,0x0000B37B,0x0000A35A,0x0000D3BD,0x0000C39C,0x0000F3FF,0x0000E3DE,0x00002462,0x00003443,0x00000420,0x00001401,0x000064E6,0x000074C7,0x000044A4,0x00005485,0x0000A56A,0x0000B54B,0x00008528,0x00009509,0x0000E5EE,0x0000F5CF,0x0000C5AC,0x0000D58D,0x00003653,0x00002672,0x00001611,0x00000630,0x000076D7,0x000066F6,0x00005695,0x000046B4,0x0000B75B,0x0000A77A,0x00009719,0x00008738,0x0000F7DF,0x0000E7FE,0x0000D79D,0x0000C7BC,0x000048C4,0x000058E5,0x00006886,0x000078A7,0x00000840,0x00001861,0x00002802,0x00003823,0x0000C9CC,0x0000D9ED,0x0000E98E,0x0000F9AF,0x00008948,0x00009969,0x0000A90A,0x0000B92B,0x00005AF5,0x00004AD4,0x00007AB7,0x00006A96,0x00001A71,0x00000A50,0x00003A33,0x00002A12,0x0000DBFD,0x0000CBDC,0x0000FBBF,0x0000EB9E,0x00009B79,0x00008B58,0x0000BB3B,0x0000AB1A,0x00006CA6,0x00007C87,0x00004CE4,0x00005CC5,0x00002C22,0x00003C03,0x00000C60,0x00001C41,0x0000EDAE,0x0000FD8F,0x0000CDEC,0x0000DDCD,0x0000AD2A,0x0000BD0B,0x00008D68,0x00009D49,0x00007E97,0x00006EB6,0x00005ED5,0x00004EF4,0x00003E13,0x00002E32,0x00001E51,0x00000E70,0x0000FF9F,0x0000EFBE,0x0000DFDD,0x0000CFFC,0x0000BF1B,0x0000AF3A,0x00009F59,0x00008F78,0x00009188,0x000081A9,0x0000B1CA,0x0000A1EB,0x0000D10C,0x0000C12D,0x0000F14E,0x0000E16F,0x00001080,0x000000A1,0x000030C2,0x000020E3,0x00005004,0x00004025,0x00007046,0x00006067,0x000083B9,0x00009398,0x0000A3FB,0x0000B3DA,0x0000C33D,0x0000D31C,0x0000E37F,0x0000F35E,0x000002B1,0x00001290,0x000022F3,0x000032D2,0x00004235,0x00005214,0x00006277,0x00007256,0x0000B5EA,0x0000A5CB,0x000095A8,0x00008589,0x0000F56E,0x0000E54F,0x0000D52C,0x0000C50D,0x000034E2,0x000024C3,0x000014A0,0x00000481,0x00007466,0x00006447,0x00005424,0x00004405,0x0000A7DB,0x0000B7FA,0x00008799,0x000097B8,0x0000E75F,0x0000F77E,0x0000C71D,0x0000D73C,0x000026D3,0x000036F2,0x00000691,0x000016B0,0x00006657,0x00007676,0x00004615,0x00005634,0x0000D94C,0x0000C96D,0x0000F90E,0x0000E92F,0x000099C8,0x000089E9,0x0000B98A,0x0000A9AB,0x00005844,0x00004865,0x00007806,0x00006827,0x000018C0,0x000008E1,0x00003882,0x000028A3,0x0000CB7D,0x0000DB5C,0x0000EB3F,0x0000FB1E,0x00008BF9,0x00009BD8,0x0000ABBB,0x0000BB9A,0x00004A75,0x00005A54,0x00006A37,0x00007A16,0x00000AF1,0x00001AD0,0x00002AB3,0x00003A92,0x0000FD2E,0x0000ED0F,0x0000DD6C,0x0000CD4D,0x0000BDAA,0x0000AD8B,0x00009DE8,0x00008DC9,0x00007C26,0x00006C07,0x00005C64,0x00004C45,0x00003CA2,0x00002C83,0x00001CE0,0x00000CC1,0x0000EF1F,0x0000FF3E,0x0000CF5D,0x0000DF7C,0x0000AF9B,0x0000BFBA,0x00008FD9,0x00009FF8,0x00006E17,0x00007E36,0x00004E55,0x00005E74,0x00002E93,0x00003EB2,0x00000ED1,0x00001EF0};
		/*static U32 *Table_CRC16CCITT=NULL;
		if (!Table_CRC16CCITT)
		{
			Table_CRC16CCITT=new U32[256];
			if (!Table_CRC16CCITT) return 0;
			BuildTable16(CRC16_CCITT, Table_CRC16CCITT);
		}*/
		for (unsigned int i = 0; i < nSize; i++ )
			nAccum = ( nAccum << 8 ) ^ (U16)Table_CRC16CCITT[( nAccum >> 8 ) ^ *aData++];
		return nAccum;
	}

	static U32 Crc32( const void* pData, unsigned int nSize)
	{
		const unsigned char *aData=(const unsigned char *)pData;
		U32 nAccum = 0;
		static U32 Table_CRC32[256]={0x00000000,0x04C10DB7,0x09821B6E,0x0D4316D9,0x130436DC,0x17C53B6B,0x1A862DB2,0x1E472005,0x26086DB8,0x22C9600F,0x2F8A76D6,0x2B4B7B61,0x350C5B64,0x31CD56D3,0x3C8E400A,0x384F4DBD,0x4C10DB70,0x48D1D6C7,0x4592C01E,0x4153CDA9,0x5F14EDAC,0x5BD5E01B,0x5696F6C2,0x5257FB75,0x6A18B6C8,0x6ED9BB7F,0x639AADA6,0x675BA011,0x791C8014,0x7DDD8DA3,0x709E9B7A,0x745F96CD,0x9821B6E0,0x9CE0BB57,0x91A3AD8E,0x9562A039,0x8B25803C,0x8FE48D8B,0x82A79B52,0x866696E5,0xBE29DB58,0xBAE8D6EF,0xB7ABC036,0xB36ACD81,0xAD2DED84,0xA9ECE033,0xA4AFF6EA,0xA06EFB5D,0xD4316D90,0xD0F06027,0xDDB376FE,0xD9727B49,0xC7355B4C,0xC3F456FB,0xCEB74022,0xCA764D95,0xF2390028,0xF6F80D9F,0xFBBB1B46,0xFF7A16F1,0xE13D36F4,0xE5FC3B43,0xE8BF2D9A,0xEC7E202D,0x34826077,0x30436DC0,0x3D007B19,0x39C176AE,0x278656AB,0x23475B1C,0x2E044DC5,0x2AC54072,0x128A0DCF,0x164B0078,0x1B0816A1,0x1FC91B16,0x018E3B13,0x054F36A4,0x080C207D,0x0CCD2DCA,0x7892BB07,0x7C53B6B0,0x7110A069,0x75D1ADDE,0x6B968DDB,0x6F57806C,0x621496B5,0x66D59B02,0x5E9AD6BF,0x5A5BDB08,0x5718CDD1,0x53D9C066,0x4D9EE063,0x495FEDD4,0x441CFB0D,0x40DDF6BA,0xACA3D697,0xA862DB20,0xA521CDF9,0xA1E0C04E,0xBFA7E04B,0xBB66EDFC,0xB625FB25,0xB2E4F692,0x8AABBB2F,0x8E6AB698,0x8329A041,0x87E8ADF6,0x99AF8DF3,0x9D6E8044,0x902D969D,0x94EC9B2A,0xE0B30DE7,0xE4720050,0xE9311689,0xEDF01B3E,0xF3B73B3B,0xF776368C,0xFA352055,0xFEF42DE2,0xC6BB605F,0xC27A6DE8,0xCF397B31,0xCBF87686,0xD5BF5683,0xD17E5B34,0xDC3D4DED,0xD8FC405A,0x6904C0EE,0x6DC5CD59,0x6086DB80,0x6447D637,0x7A00F632,0x7EC1FB85,0x7382ED5C,0x7743E0EB,0x4F0CAD56,0x4BCDA0E1,0x468EB638,0x424FBB8F,0x5C089B8A,0x58C9963D,0x558A80E4,0x514B8D53,0x25141B9E,0x21D51629,0x2C9600F0,0x28570D47,0x36102D42,0x32D120F5,0x3F92362C,0x3B533B9B,0x031C7626,0x07DD7B91,0x0A9E6D48,0x0E5F60FF,0x101840FA,0x14D94D4D,0x199A5B94,0x1D5B5623,0xF125760E,0xF5E47BB9,0xF8A76D60,0xFC6660D7,0xE22140D2,0xE6E04D65,0xEBA35BBC,0xEF62560B,0xD72D1BB6,0xD3EC1601,0xDEAF00D8,0xDA6E0D6F,0xC4292D6A,0xC0E820DD,0xCDAB3604,0xC96A3BB3,0xBD35AD7E,0xB9F4A0C9,0xB4B7B610,0xB076BBA7,0xAE319BA2,0xAAF09615,0xA7B380CC,0xA3728D7B,0x9B3DC0C6,0x9FFCCD71,0x92BFDBA8,0x967ED61F,0x8839F61A,0x8CF8FBAD,0x81BBED74,0x857AE0C3,0x5D86A099,0x5947AD2E,0x5404BBF7,0x50C5B640,0x4E829645,0x4A439BF2,0x47008D2B,0x43C1809C,0x7B8ECD21,0x7F4FC096,0x720CD64F,0x76CDDBF8,0x688AFBFD,0x6C4BF64A,0x6108E093,0x65C9ED24,0x11967BE9,0x1557765E,0x18146087,0x1CD56D30,0x02924D35,0x06534082,0x0B10565B,0x0FD15BEC,0x379E1651,0x335F1BE6,0x3E1C0D3F,0x3ADD0088,0x249A208D,0x205B2D3A,0x2D183BE3,0x29D93654,0xC5A71679,0xC1661BCE,0xCC250D17,0xC8E400A0,0xD6A320A5,0xD2622D12,0xDF213BCB,0xDBE0367C,0xE3AF7BC1,0xE76E7676,0xEA2D60AF,0xEEEC6D18,0xF0AB4D1D,0xF46A40AA,0xF9295673,0xFDE85BC4,0x89B7CD09,0x8D76C0BE,0x8035D667,0x84F4DBD0,0x9AB3FBD5,0x9E72F662,0x9331E0BB,0x97F0ED0C,0xAFBFA0B1,0xAB7EAD06,0xA63DBBDF,0xA2FCB668,0xBCBB966D,0xB87A9BDA,0xB5398D03,0xB1F880B4};
		/*static U32 *Table_CRC32=NULL;
		if (!Table_CRC32)
		{
			Table_CRC32=new U32[256];
			if (!Table_CRC32) return 0;
			BuildTable32(CRC32_DEFAULT,Table_CRC32);
		}*/
		for (unsigned int i = 0; i < nSize; i++ )
			nAccum = ( nAccum << 8 ) ^ Table_CRC32[( nAccum >> 24 ) ^ *aData++];
		return nAccum;
	}

protected:
	static void BuildTable16( U16 aPoly , U32* Table_CRC )
	{
		U16 i, j;
		U16 nData;
		U16 nAccum;
		for ( i = 0; i < 256; i++ )
		{
			nData = ( U16 )( i << 8 );
			nAccum = 0;
			for ( j = 0; j < 8; j++ )
			{
				if ( ( nData ^ nAccum ) & 0x8000 )
					nAccum = ( nAccum << 1 ) ^ aPoly;
				else
					nAccum <<= 1;
				nData <<= 1;
			}
			Table_CRC[i] = ( U32 )nAccum;
		}
	}

	static void BuildTable32( U32 aPoly , U32* Table_CRC )
	{
		U32 i, j;
		U32 nData;
		U32 nAccum;
		for ( i = 0; i < 256; i++ )
		{
			nData = (U32)( i << 24 );
			nAccum = 0;
			for ( j = 0; j < 8; j++ )
			{
				if ( ( nData ^ nAccum ) & 0x80000000 )
					nAccum = ( nAccum << 1 ) ^ aPoly;
				else
					nAccum <<= 1;
				nData <<= 1;
			}
			Table_CRC[i] = nAccum;
		}
	}
};

#pragma intrinsic(_lrotr,_lrotl)
#define rotr(x,n)	_lrotr(x,n)
#define rotl(x,n)	_lrotl(x,n)

#define f_rnd(i,a,b,c,d)                    \
        u = rotl(d * (d + d + 1), 5);       \
        t = rotl(b * (b + b + 1), 5);       \
        a = rotl(a ^ t, u) + l_key[i];      \
        c = rotl(c ^ u, t) + l_key[i + 1]

#define i_rnd(i,a,b,c,d)                    \
        u = rotl(d * (d + d + 1), 5);       \
        t = rotl(b * (b + b + 1), 5);       \
        c = rotr(c - l_key[i + 1], t) ^ u;  \
        a = rotr(a - l_key[i], u) ^ t

/**
* @brief Rc6封装类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-07-10 新建类
*/
/**<pre>
 This is an independent implementation of the encryption algorithm:
 which is a candidate algorithm in the Advanced Encryption Standard
 programme of the US National Institute of Standards and Technology.

Timing data for RC6 (rc6.c)

128 bit key:
Key Setup:    1632 cycles
Encrypt:       270 cycles =    94.8 mbits/sec
Decrypt:       226 cycles =   113.3 mbits/sec
Mean:          248 cycles =   103.2 mbits/sec

192 bit key:
Key Setup:    1885 cycles
Encrypt:       267 cycles =    95.9 mbits/sec
Decrypt:       235 cycles =   108.9 mbits/sec
Mean:          251 cycles =   102.0 mbits/sec

256 bit key:
Key Setup:    1877 cycles
Encrypt:       270 cycles =    94.8 mbits/sec
Decrypt:       227 cycles =   112.8 mbits/sec
Mean:          249 cycles =   103.0 mbits/sec
</pre>*/
class THRc6
{
public:
	THRc6()
	{
	}
	virtual ~THRc6()
	{
	}

	//key_len is bitlen
	virtual void SetPass(const U32 in_key[],const U32 key_len)
	{
		U32 i, j, k, a, b, l[8], t;
		l_key[0] = 0xb7e15163;
		for(k = 1; k < 44; ++k) l_key[k] = l_key[k - 1] + 0x9e3779b9;
		for(k = 0; k < key_len / 32; ++k) l[k] = in_key[k];
		t = (key_len / 32) - 1; // t = (key_len / 32);
		a = b = i = j = 0;
		for(k = 0; k < 132; ++k)
		{
			a = rotl(l_key[i] + a + b, 3); b += a;
			b = rotl(l[j] + b, b);
			l_key[i] = a; l[j] = b;
			i = (i == 43 ? 0 : i + 1); // i = (i + 1) % 44;  
			j = (j == t ? 0 : j + 1);  // j = (j + 1) % t;
		}
	}

	virtual void Encrypt(const U32 in_blk[4],U32 out_blk[4])
	{
		U32 a,b,c,d,t,u;

		a = in_blk[0]; b = in_blk[1] + l_key[0];
		c = in_blk[2]; d = in_blk[3] + l_key[1];

		f_rnd( 2,a,b,c,d); f_rnd( 4,b,c,d,a);
		f_rnd( 6,c,d,a,b); f_rnd( 8,d,a,b,c);
		f_rnd(10,a,b,c,d); f_rnd(12,b,c,d,a);
		f_rnd(14,c,d,a,b); f_rnd(16,d,a,b,c);
		f_rnd(18,a,b,c,d); f_rnd(20,b,c,d,a);
		f_rnd(22,c,d,a,b); f_rnd(24,d,a,b,c);
		f_rnd(26,a,b,c,d); f_rnd(28,b,c,d,a);
		f_rnd(30,c,d,a,b); f_rnd(32,d,a,b,c);
		f_rnd(34,a,b,c,d); f_rnd(36,b,c,d,a);
		f_rnd(38,c,d,a,b); f_rnd(40,d,a,b,c);

		out_blk[0] = a + l_key[42]; out_blk[1] = b;
		out_blk[2] = c + l_key[43]; out_blk[3] = d;
	}

	virtual void Decrypt(const U32 in_blk[4], U32 out_blk[4])
	{
		U32 a,b,c,d,t,u;
		d = in_blk[3]; c = in_blk[2] - l_key[43]; 
		b = in_blk[1]; a = in_blk[0] - l_key[42];
		i_rnd(40,d,a,b,c); i_rnd(38,c,d,a,b);
		i_rnd(36,b,c,d,a); i_rnd(34,a,b,c,d);
		i_rnd(32,d,a,b,c); i_rnd(30,c,d,a,b);
		i_rnd(28,b,c,d,a); i_rnd(26,a,b,c,d);
		i_rnd(24,d,a,b,c); i_rnd(22,c,d,a,b);
		i_rnd(20,b,c,d,a); i_rnd(18,a,b,c,d);
		i_rnd(16,d,a,b,c); i_rnd(14,c,d,a,b);
		i_rnd(12,b,c,d,a); i_rnd(10,a,b,c,d);
		i_rnd( 8,d,a,b,c); i_rnd( 6,c,d,a,b);
		i_rnd( 4,b,c,d,a); i_rnd( 2,a,b,c,d);
		out_blk[3] = d - l_key[1]; out_blk[2] = c; 
		out_blk[1] = b - l_key[0]; out_blk[0] = a; 
	}

	void DecryptECB(const U32 in_blk[4], U32 out_blk[4])
	{
		U32 x0,x1,x2,x3;
		U32 u,t;
		int i;
		x0=in_blk[0];
		x1=in_blk[1];
		x2=in_blk[2];
		x3=in_blk[3];
		x2=x2-l_key[(2*20)+3];
		x0=x0-l_key[(2*20)+2];
		for(i=20;i>=1;i--)
		{
			t=x0;x0=x3;x3=x2;x2=x1;x1=t;
			u=rotl(x3*(2*x3+1),5);
			t=rotl(x1*(2*x1+1),5);
			x2=rotr(x2-l_key[2*i+1],t)^u;
			x0=rotr(x0-l_key[2*i],u)^t;
		}
		x3=x3-l_key[1];
		x1=x1-l_key[0];
		out_blk[0]=x0;
		out_blk[1]=x1;
		out_blk[2]=x2;
		out_blk[3]=x3;
	}
protected:
	U32 l_key[44];
};

/**
* @brief Rc6加强类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-07-12 新建类
*/
/**<pre>
</pre>*/
class THRc6Block128 : public THRc6
{
public:
	//INitVector is 16 bytes buffer,key_len is bitlen
	void SetPass(const U32 in_key[],const U32 key_len,const U8 *InitVector)
	{
		//Super::Init();
		THRc6::SetPass(in_key,key_len);
		if (InitVector==NULL)
		{
			memset(IV,0,16);
			THRc6::Encrypt((U32 *)IV,(U32 *)IV);
			Reset();
		}
		else
		{
			memcpy(IV,InitVector,16);
			Reset();
		}
	}

	void Reset()
	{
		memcpy(CV,IV,16);
	}

	void Burn()
	{
		memset(IV,0xff,16);
		memset(CV,0xff,16);
	}

	void Encrypt(const U8 *in,U8 *out,U32 size)
	{
		const U8 *p1=in;
		U8 *p2=out;
		for(U32 i=1;i<=size/16;i++)
		{
			memcpy(p2,p1,16);
			Xor(p2,CV,16);
			THRc6::Encrypt((U32 *)p2,(U32 *)p2);
			memcpy(CV,p2,16);
			p1+=16;
			p2+=16;
		}
		if (size%16!=0)
		{
			THRc6::Encrypt((U32 *)CV,(U32 *)CV);
			memcpy(p2,p1,size%16);
			Xor(p2,CV,size%16);
		}
	}

	void Decrypt(const U8 *in,U8 *out,U32 size)
	{
		const U8 *p1=in;
		U8 *p2=out;
		U8 temp[16];
		for(U32 i=1;i<=size/16;i++)
		{
			memcpy(p2,p1,16);
			memcpy(temp,p1,16);
			THRc6::Decrypt((U32 *)p2,(U32 *)p2);
			Xor(p2,CV,16);
			memcpy(CV,temp,16);
			p1+=16;
			p2+=16;
		}
		if (size%16!=0)
		{
			THRc6::Encrypt((U32 *)CV,(U32 *)CV);
			memcpy(p2,p1,size%16);
			Xor(p2,CV,size%16);
		}
	}
protected:
	U8 IV[16];
	U8 CV[16];

	void Xor(U8 *out,U8 *in,int c)
	{
		for(int i=0;i<c;i++)
		{
			*(out+i)=(*(out+i))^(*(in+i));
		}
	}
};

#define WORDS_BIGENDIAN
#define f_1(x6, x5, x4, x3, x2, x1, x0)              \
           (((x1) & ((x0) ^ (x4))) ^ ((x2) & (x5)) ^ \
            ((x3) & (x6)) ^ (x0))

#define f_2(x6, x5, x4, x3, x2, x1, x0)                               \
           (((x2) & (((x1) & ~(x3)) ^ ((x4) & (x5)) ^ (x6) ^ (x0))) ^ \
            ((x4) & ((x1) ^ (x5))) ^ ((x3) & (x5)) ^ (x0)) 

#define f_3(x6, x5, x4, x3, x2, x1, x0)              \
           (((x3) & (((x1) & (x2)) ^ (x6) ^ (x0))) ^ \
            ((x1) & (x4)) ^ ((x2) & (x5)) ^ (x0))

#define f_4(x6, x5, x4, x3, x2, x1, x0)                                   \
           (((x4) & (((x5) & ~(x2)) ^ ((x3) & ~(x6)) ^ (x1) ^ (x6) ^ (x0))) ^ \
            ((x3) & (((x1) & (x2)) ^ (x5) ^ (x6))) ^                        \
            ((x2) & (x6)) ^ (x0))

#define f_5(x6, x5, x4, x3, x2, x1, x0)             \
           (((x0) & ((((x1) & (x2)) & (x3)) ^ ~(x5))) ^ \
            ((x1) & (x4)) ^ ((x2) & (x5)) ^ ((x3) & (x6)))

/*
 * Permutations phi_{i,j}, i=3,4,5, j=1,...,i.
 *
 * PASS = 3:
 *               6 5 4 3 2 1 0
 *               | | | | | | | (replaced by)
 *  phi_{3,1}:   1 0 3 5 6 2 4
 *  phi_{3,2}:   4 2 1 0 5 3 6
 *  phi_{3,3}:   6 1 2 3 4 5 0
 *
 * PASS = 4:
 *               6 5 4 3 2 1 0
 *               | | | | | | | (replaced by)
 *  phi_{4,1}:   2 6 1 4 5 3 0
 *  phi_{4,2}:   3 5 2 0 1 6 4
 *  phi_{4,3}:   1 4 3 6 0 2 5
 *  phi_{4,4}:   6 4 0 5 2 1 3
 *
 * PASS = 5:
 *               6 5 4 3 2 1 0
 *               | | | | | | | (replaced by)
 *  phi_{5,1}:   3 4 1 0 5 2 6
 *  phi_{5,2}:   6 2 1 0 3 4 5
 *  phi_{5,3}:   2 6 0 4 3 1 5
 *  phi_{5,4}:   1 5 3 2 0 4 6
 *  phi_{5,5}:   2 5 0 6 4 3 1
 */

#define rotate_right(x, n) (((x) >> (n)) | ((x) << (32-(n))))

#define FF_1(x7, x6, x5, x4, x3, x2, x1, x0, w) {			\
	register U32 temp;					\
	switch (state->passes) {					\
		case 3: temp = f_1(x1, x0, x3, x5, x6, x2, x4); break;	\
		case 4: temp = f_1(x2, x6, x1, x4, x5, x3, x0); break;	\
	default:case 5: temp = f_1(x3, x4, x1, x0, x5, x2, x6); break;	\
	}								\
	(x7) = rotate_right(temp, 7) + rotate_right((x7), 11) + (w);	\
	}

#define FF_2(x7, x6, x5, x4, x3, x2, x1, x0, w, c) {				\
	register U32 temp;						\
	switch (state->passes) {						\
		case 3: temp = f_2(x4, x2, x1, x0, x5, x3, x6); break;		\
		case 4: temp = f_2(x3, x5, x2, x0, x1, x6, x4); break;		\
	default:case 5: temp = f_2(x6, x2, x1, x0, x3, x4, x5); break;		\
	}									\
	(x7) = rotate_right(temp, 7) + rotate_right((x7), 11) + (w) + (c);	\
	}

#define FF_3(x7, x6, x5, x4, x3, x2, x1, x0, w, c) {				\
	register U32 temp;						\
	switch (state->passes) {						\
		case 3: temp = f_3(x6, x1, x2, x3, x4, x5, x0); break;		\
		case 4: temp = f_3(x1, x4, x3, x6, x0, x2, x5); break;		\
	default:case 5: temp = f_3(x2, x6, x0, x4, x3, x1, x5); break;		\
	}									\
	(x7) = rotate_right(temp, 7) + rotate_right((x7), 11) + (w) + (c);	\
	}

#define FF_4(x7, x6, x5, x4, x3, x2, x1, x0, w, c) {				\
	register U32 temp;						\
	switch (state->passes) {						\
		case 4: temp = f_4(x6, x4, x0, x5, x2, x1, x3); break;		\
	default:case 5: temp = f_4(x1, x5, x3, x2, x0, x4, x6); break;		\
	}									\
	(x7) = rotate_right(temp, 7) + rotate_right((x7), 11) + (w) + (c);	\
	}

#define FF_5(x7, x6, x5, x4, x3, x2, x1, x0, w, c) {				\
	register U32 temp = f_5(x2, x5, x0, x6, x4, x3, x1);		\
	(x7) = rotate_right(temp, 7) + rotate_right((x7), 11) + (w) + (c);	\
	}

/*
 * translate every four characters into a word.
 * assume the number of characters is a multiple of four.
 */

#define ch2uint(string, word, slen) {      \
  unsigned char *sp = string;              \
  U32    *wp = word;                \
  while (sp < (string) + (slen)) {         \
    *wp++ =  (U32)*sp            |  \
            ((U32)*(sp+1) <<  8) |  \
            ((U32)*(sp+2) << 16) |  \
            ((U32)*(sp+3) << 24);   \
    sp += 4;                               \
  }                                        \
}

/* translate each word into four characters */
#define uint2ch(word, string, wlen) {              \
  U32    *wp = word;                        \
  unsigned char *sp = string;                      \
  while (wp < (word) + (wlen)) {                   \
    *(sp++) = (unsigned char)( *wp        & 0xFF); \
    *(sp++) = (unsigned char)((*wp >>  8) & 0xFF); \
    *(sp++) = (unsigned char)((*wp >> 16) & 0xFF); \
    *(sp++) = (unsigned char)((*wp >> 24) & 0xFF); \
    wp++;                                          \
  }                                                \
}

#define HAVAL_VERSION		1

/**
* @brief Haval封装类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-07-13 新建类
*/
/**<pre>
	FPT 为指纹位数,有128,160,192,224,256几种
	PASS 为加密次数,有3,4,5几种
 *  specifies the interface to the HAVAL (V.1) hashing library.
 *
 *      HAVAL is a one-way hashing algorithm with the following
 *      collision-resistant property:
 *             It is computationally infeasible to find two or more
 *             messages that are hashed into the same fingerprint.
 *
</pre>*/
template<int FPT,int PASS>
class THHaval
{
public:
	static void CalcHaval(U8 *buf, size_t len, U8 *fingerprint)
	{
		THHavalState state;
		haval_start (&state, FPT, PASS);
		haval_hash (&state, buf, len);
		haval_end (&state, fingerprint);
	}

	static void Test(U8 *fingerprint)
	{
		THHavalState state;
		haval_start (&state, FPT, PASS);
		U8 buf[]={0xBE,0xF9,0x5D,0xC8,0x33,0xF3,0x5D,0xED};
		U8 buf1[]={0x69,0x65,0x62,0x6B,0x2A,0x30,0x37,0x00};
		haval_hash (&state, buf, 8);
		haval_hash (&state, buf1, 7);
		haval_end (&state, fingerprint);
	}

	//maxlen=0 for not limit
	static BOOL CalcFileHaval(THString sFile, U8 *fingerprint)
	{
		THHavalState state;
#define BLOCKSIZE 4096
		unsigned char buffer[BLOCKSIZE + 72];
		THFile file;
		if (!file.Open(sFile,THFile::modeRead)) return FALSE;
		haval_start (&state, FPT, PASS);
		while(1)
		{
			UINT nRead=file.Read(buffer,BLOCKSIZE);
			if (nRead == 0)
				break;
			haval_hash(&state, buffer, nRead);
		}
		file.Close();
		haval_end(&state, fingerprint);
		return TRUE;
	}
protected:
	typedef struct {
		U32    count[2];                /* number of bits in a message */
		U32    fingerprint[8];          /* current state of fingerprint */    
		U32    block[32];               /* buffer for a 32-word block */ 
		unsigned char remainder[32*4];         /* unhashed chars (No.<128) */   
		int fptlen;	/* fingerprint length */
		int passes;	/* number of passes, 3 or 4 or 5 */
	} THHavalState;
	static void haval_start (THHavalState *state, int fptlen, int passes)
	{
		state->count[0]       = state->count[1] = 0;   /* clear count */
		state->fingerprint[0] = 0x243F6A88L;           /* initial fingerprint */
		state->fingerprint[1] = 0x85A308D3L;
		state->fingerprint[2] = 0x13198A2EL;
		state->fingerprint[3] = 0x03707344L;
		state->fingerprint[4] = 0xA4093822L;
		state->fingerprint[5] = 0x299F31D0L;
		state->fingerprint[6] = 0x082EFA98L;
		state->fingerprint[7] = 0xEC4E6C89L;
		state->fptlen = fptlen;
		state->passes = passes;
	}
	static void haval_hash (THHavalState *state, unsigned char *str, unsigned int str_len)
	{
		unsigned int i,
				rmd_len,
				fill_len;

		/* calculate the number of bytes in the remainder */
		rmd_len  = (unsigned int)((state->count[0] >> 3) & 0x7F);
		fill_len = 128 - rmd_len;

		/* update the number of bits */
		if ((state->count[0] +=  (U32)str_len << 3)
							< ((U32)str_len << 3)) {
			state->count[1]++;
		}
		state->count[1] += (U32)str_len >> 29;

#ifdef WORDS_BIGENDIAN
		/* hash as many blocks as possible */
		if (rmd_len + str_len >= 128) {
		memcpy (&state->remainder[rmd_len], str, fill_len);
		ch2uint(state->remainder, state->block, 128);
		haval_hash_block (state);
		for (i = fill_len; i + 127 < str_len; i += 128){
			memcpy (state->remainder, str+i, 128);
			ch2uint(state->remainder, state->block, 128);
			haval_hash_block (state);
		}
		rmd_len = 0;
		} else {
		i = 0;
		}
		/* save the remaining input chars */
		memcpy (&state->remainder[rmd_len], str+i, str_len-i);
#else
		/* hash as many blocks as possible */
		if (rmd_len + str_len >= 128) {
		memcpy (((unsigned char *)state->block)+rmd_len, str, fill_len);
		haval_hash_block (state);
		for (i = fill_len; i + 127 < str_len; i += 128){
			memcpy ((unsigned char *)state->block, str+i, 128);
			haval_hash_block (state);
		}
		rmd_len = 0;
		} else {
		i = 0;
		}
		memcpy (((unsigned char *)state->block)+rmd_len, str+i, str_len-i);
#endif
	}
	static void haval_end (THHavalState *state, unsigned char *final_fpt)
	{
		unsigned char tail[10];
		unsigned int  rmd_len, pad_len;

		/*
		* save the version number, the number of passes, the fingerprint 
		* length and the number of bits in the unpadded message.
		*/
		tail[0] = (unsigned char)(((state->fptlen  & 0x3) << 6) |
									((state->passes  & 0x7) << 3) |
									(HAVAL_VERSION & 0x7));
		tail[1] = (unsigned char)((state->fptlen >> 2) & 0xFF);
		uint2ch (state->count, &tail[2], 2);

		/* pad out to 118 mod 128 */
		rmd_len = (unsigned int)((state->count[0] >> 3) & 0x7f);
		pad_len = (rmd_len < 118) ? (118 - rmd_len) : (246 - rmd_len);
		haval_hash (state, GetPadding(), pad_len);

		/*
		* append the version number, the number of passes,
		* the fingerprint length and the number of bits
		*/
		haval_hash (state, tail, 10);

		/* tailor the last output */
		haval_tailor(state);

		/* translate and save the final fingerprint */
		uint2ch (state->fingerprint, final_fpt, state->fptlen >> 5);

		/* clear the state information */
		memset ((unsigned char *)state, 0, sizeof(*state));
	}
	static void haval_hash_block (THHavalState *state)
	{
		register U32 t0 = state->fingerprint[0],    /* make use of */
							t1 = state->fingerprint[1],    /* internal registers */
							t2 = state->fingerprint[2],
							t3 = state->fingerprint[3],
							t4 = state->fingerprint[4],
							t5 = state->fingerprint[5],
							t6 = state->fingerprint[6],
							t7 = state->fingerprint[7],
							*w = state->block;

		/* Pass 1 */
		FF_1(t7, t6, t5, t4, t3, t2, t1, t0, *(w   ));
		FF_1(t6, t5, t4, t3, t2, t1, t0, t7, *(w+ 1));
		FF_1(t5, t4, t3, t2, t1, t0, t7, t6, *(w+ 2));
		FF_1(t4, t3, t2, t1, t0, t7, t6, t5, *(w+ 3));
		FF_1(t3, t2, t1, t0, t7, t6, t5, t4, *(w+ 4));
		FF_1(t2, t1, t0, t7, t6, t5, t4, t3, *(w+ 5));
		FF_1(t1, t0, t7, t6, t5, t4, t3, t2, *(w+ 6));
		FF_1(t0, t7, t6, t5, t4, t3, t2, t1, *(w+ 7));

		FF_1(t7, t6, t5, t4, t3, t2, t1, t0, *(w+ 8));
		FF_1(t6, t5, t4, t3, t2, t1, t0, t7, *(w+ 9));
		FF_1(t5, t4, t3, t2, t1, t0, t7, t6, *(w+10));
		FF_1(t4, t3, t2, t1, t0, t7, t6, t5, *(w+11));
		FF_1(t3, t2, t1, t0, t7, t6, t5, t4, *(w+12));
		FF_1(t2, t1, t0, t7, t6, t5, t4, t3, *(w+13));
		FF_1(t1, t0, t7, t6, t5, t4, t3, t2, *(w+14));
		FF_1(t0, t7, t6, t5, t4, t3, t2, t1, *(w+15));

		FF_1(t7, t6, t5, t4, t3, t2, t1, t0, *(w+16));
		FF_1(t6, t5, t4, t3, t2, t1, t0, t7, *(w+17));
		FF_1(t5, t4, t3, t2, t1, t0, t7, t6, *(w+18));
		FF_1(t4, t3, t2, t1, t0, t7, t6, t5, *(w+19));
		FF_1(t3, t2, t1, t0, t7, t6, t5, t4, *(w+20));
		FF_1(t2, t1, t0, t7, t6, t5, t4, t3, *(w+21));
		FF_1(t1, t0, t7, t6, t5, t4, t3, t2, *(w+22));
		FF_1(t0, t7, t6, t5, t4, t3, t2, t1, *(w+23));

		FF_1(t7, t6, t5, t4, t3, t2, t1, t0, *(w+24));
		FF_1(t6, t5, t4, t3, t2, t1, t0, t7, *(w+25));
		FF_1(t5, t4, t3, t2, t1, t0, t7, t6, *(w+26));
		FF_1(t4, t3, t2, t1, t0, t7, t6, t5, *(w+27));
		FF_1(t3, t2, t1, t0, t7, t6, t5, t4, *(w+28));
		FF_1(t2, t1, t0, t7, t6, t5, t4, t3, *(w+29));
		FF_1(t1, t0, t7, t6, t5, t4, t3, t2, *(w+30));
		FF_1(t0, t7, t6, t5, t4, t3, t2, t1, *(w+31));

		/* Pass 2 */
		FF_2(t7, t6, t5, t4, t3, t2, t1, t0, *(w+ 5), 0x452821E6L);
		FF_2(t6, t5, t4, t3, t2, t1, t0, t7, *(w+14), 0x38D01377L);
		FF_2(t5, t4, t3, t2, t1, t0, t7, t6, *(w+26), 0xBE5466CFL);
		FF_2(t4, t3, t2, t1, t0, t7, t6, t5, *(w+18), 0x34E90C6CL);
		FF_2(t3, t2, t1, t0, t7, t6, t5, t4, *(w+11), 0xC0AC29B7L);
		FF_2(t2, t1, t0, t7, t6, t5, t4, t3, *(w+28), 0xC97C50DDL);
		FF_2(t1, t0, t7, t6, t5, t4, t3, t2, *(w+ 7), 0x3F84D5B5L);
		FF_2(t0, t7, t6, t5, t4, t3, t2, t1, *(w+16), 0xB5470917L);

		FF_2(t7, t6, t5, t4, t3, t2, t1, t0, *(w   ), 0x9216D5D9L);
		FF_2(t6, t5, t4, t3, t2, t1, t0, t7, *(w+23), 0x8979FB1BL);
		FF_2(t5, t4, t3, t2, t1, t0, t7, t6, *(w+20), 0xD1310BA6L);
		FF_2(t4, t3, t2, t1, t0, t7, t6, t5, *(w+22), 0x98DFB5ACL);
		FF_2(t3, t2, t1, t0, t7, t6, t5, t4, *(w+ 1), 0x2FFD72DBL);
		FF_2(t2, t1, t0, t7, t6, t5, t4, t3, *(w+10), 0xD01ADFB7L);
		FF_2(t1, t0, t7, t6, t5, t4, t3, t2, *(w+ 4), 0xB8E1AFEDL);
		FF_2(t0, t7, t6, t5, t4, t3, t2, t1, *(w+ 8), 0x6A267E96L);

		FF_2(t7, t6, t5, t4, t3, t2, t1, t0, *(w+30), 0xBA7C9045L);
		FF_2(t6, t5, t4, t3, t2, t1, t0, t7, *(w+ 3), 0xF12C7F99L);
		FF_2(t5, t4, t3, t2, t1, t0, t7, t6, *(w+21), 0x24A19947L);
		FF_2(t4, t3, t2, t1, t0, t7, t6, t5, *(w+ 9), 0xB3916CF7L);
		FF_2(t3, t2, t1, t0, t7, t6, t5, t4, *(w+17), 0x0801F2E2L);
		FF_2(t2, t1, t0, t7, t6, t5, t4, t3, *(w+24), 0x858EFC16L);
		FF_2(t1, t0, t7, t6, t5, t4, t3, t2, *(w+29), 0x636920D8L);
		FF_2(t0, t7, t6, t5, t4, t3, t2, t1, *(w+ 6), 0x71574E69L);

		FF_2(t7, t6, t5, t4, t3, t2, t1, t0, *(w+19), 0xA458FEA3L);
		FF_2(t6, t5, t4, t3, t2, t1, t0, t7, *(w+12), 0xF4933D7EL);
		FF_2(t5, t4, t3, t2, t1, t0, t7, t6, *(w+15), 0x0D95748FL);
		FF_2(t4, t3, t2, t1, t0, t7, t6, t5, *(w+13), 0x728EB658L);
		FF_2(t3, t2, t1, t0, t7, t6, t5, t4, *(w+ 2), 0x718BCD58L);
		FF_2(t2, t1, t0, t7, t6, t5, t4, t3, *(w+25), 0x82154AEEL);
		FF_2(t1, t0, t7, t6, t5, t4, t3, t2, *(w+31), 0x7B54A41DL);
		FF_2(t0, t7, t6, t5, t4, t3, t2, t1, *(w+27), 0xC25A59B5L);

		/* Pass 3 */
		FF_3(t7, t6, t5, t4, t3, t2, t1, t0, *(w+19), 0x9C30D539L);
		FF_3(t6, t5, t4, t3, t2, t1, t0, t7, *(w+ 9), 0x2AF26013L);
		FF_3(t5, t4, t3, t2, t1, t0, t7, t6, *(w+ 4), 0xC5D1B023L);
		FF_3(t4, t3, t2, t1, t0, t7, t6, t5, *(w+20), 0x286085F0L);
		FF_3(t3, t2, t1, t0, t7, t6, t5, t4, *(w+28), 0xCA417918L);
		FF_3(t2, t1, t0, t7, t6, t5, t4, t3, *(w+17), 0xB8DB38EFL);
		FF_3(t1, t0, t7, t6, t5, t4, t3, t2, *(w+ 8), 0x8E79DCB0L);
		FF_3(t0, t7, t6, t5, t4, t3, t2, t1, *(w+22), 0x603A180EL);

		FF_3(t7, t6, t5, t4, t3, t2, t1, t0, *(w+29), 0x6C9E0E8BL);
		FF_3(t6, t5, t4, t3, t2, t1, t0, t7, *(w+14), 0xB01E8A3EL);
		FF_3(t5, t4, t3, t2, t1, t0, t7, t6, *(w+25), 0xD71577C1L);
		FF_3(t4, t3, t2, t1, t0, t7, t6, t5, *(w+12), 0xBD314B27L);
		FF_3(t3, t2, t1, t0, t7, t6, t5, t4, *(w+24), 0x78AF2FDAL);
		FF_3(t2, t1, t0, t7, t6, t5, t4, t3, *(w+30), 0x55605C60L);
		FF_3(t1, t0, t7, t6, t5, t4, t3, t2, *(w+16), 0xE65525F3L);
		FF_3(t0, t7, t6, t5, t4, t3, t2, t1, *(w+26), 0xAA55AB94L);

		FF_3(t7, t6, t5, t4, t3, t2, t1, t0, *(w+31), 0x57489862L);
		FF_3(t6, t5, t4, t3, t2, t1, t0, t7, *(w+15), 0x63E81440L);
		FF_3(t5, t4, t3, t2, t1, t0, t7, t6, *(w+ 7), 0x55CA396AL);
		FF_3(t4, t3, t2, t1, t0, t7, t6, t5, *(w+ 3), 0x2AAB10B6L);
		FF_3(t3, t2, t1, t0, t7, t6, t5, t4, *(w+ 1), 0xB4CC5C34L);
		FF_3(t2, t1, t0, t7, t6, t5, t4, t3, *(w   ), 0x1141E8CEL);
		FF_3(t1, t0, t7, t6, t5, t4, t3, t2, *(w+18), 0xA15486AFL);
		FF_3(t0, t7, t6, t5, t4, t3, t2, t1, *(w+27), 0x7C72E993L);

		FF_3(t7, t6, t5, t4, t3, t2, t1, t0, *(w+13), 0xB3EE1411L);
		FF_3(t6, t5, t4, t3, t2, t1, t0, t7, *(w+ 6), 0x636FBC2AL);
		FF_3(t5, t4, t3, t2, t1, t0, t7, t6, *(w+21), 0x2BA9C55DL);
		FF_3(t4, t3, t2, t1, t0, t7, t6, t5, *(w+10), 0x741831F6L);
		FF_3(t3, t2, t1, t0, t7, t6, t5, t4, *(w+23), 0xCE5C3E16L);
		FF_3(t2, t1, t0, t7, t6, t5, t4, t3, *(w+11), 0x9B87931EL);
		FF_3(t1, t0, t7, t6, t5, t4, t3, t2, *(w+ 5), 0xAFD6BA33L);
		FF_3(t0, t7, t6, t5, t4, t3, t2, t1, *(w+ 2), 0x6C24CF5CL);

		if (state->passes >= 4) {
		/* Pass 4. executed only when PASS =4 or 5 */
		FF_4(t7, t6, t5, t4, t3, t2, t1, t0, *(w+24), 0x7A325381L);
		FF_4(t6, t5, t4, t3, t2, t1, t0, t7, *(w+ 4), 0x28958677L);
		FF_4(t5, t4, t3, t2, t1, t0, t7, t6, *(w   ), 0x3B8F4898L);
		FF_4(t4, t3, t2, t1, t0, t7, t6, t5, *(w+14), 0x6B4BB9AFL);
		FF_4(t3, t2, t1, t0, t7, t6, t5, t4, *(w+ 2), 0xC4BFE81BL);
		FF_4(t2, t1, t0, t7, t6, t5, t4, t3, *(w+ 7), 0x66282193L);
		FF_4(t1, t0, t7, t6, t5, t4, t3, t2, *(w+28), 0x61D809CCL);
		FF_4(t0, t7, t6, t5, t4, t3, t2, t1, *(w+23), 0xFB21A991L);

		FF_4(t7, t6, t5, t4, t3, t2, t1, t0, *(w+26), 0x487CAC60L);
		FF_4(t6, t5, t4, t3, t2, t1, t0, t7, *(w+ 6), 0x5DEC8032L);
		FF_4(t5, t4, t3, t2, t1, t0, t7, t6, *(w+30), 0xEF845D5DL);
		FF_4(t4, t3, t2, t1, t0, t7, t6, t5, *(w+20), 0xE98575B1L);
		FF_4(t3, t2, t1, t0, t7, t6, t5, t4, *(w+18), 0xDC262302L);
		FF_4(t2, t1, t0, t7, t6, t5, t4, t3, *(w+25), 0xEB651B88L);
		FF_4(t1, t0, t7, t6, t5, t4, t3, t2, *(w+19), 0x23893E81L);
		FF_4(t0, t7, t6, t5, t4, t3, t2, t1, *(w+ 3), 0xD396ACC5L);

		FF_4(t7, t6, t5, t4, t3, t2, t1, t0, *(w+22), 0x0F6D6FF3L);
		FF_4(t6, t5, t4, t3, t2, t1, t0, t7, *(w+11), 0x83F44239L);
		FF_4(t5, t4, t3, t2, t1, t0, t7, t6, *(w+31), 0x2E0B4482L);
		FF_4(t4, t3, t2, t1, t0, t7, t6, t5, *(w+21), 0xA4842004L);
		FF_4(t3, t2, t1, t0, t7, t6, t5, t4, *(w+ 8), 0x69C8F04AL);
		FF_4(t2, t1, t0, t7, t6, t5, t4, t3, *(w+27), 0x9E1F9B5EL);
		FF_4(t1, t0, t7, t6, t5, t4, t3, t2, *(w+12), 0x21C66842L);
		FF_4(t0, t7, t6, t5, t4, t3, t2, t1, *(w+ 9), 0xF6E96C9AL);

		FF_4(t7, t6, t5, t4, t3, t2, t1, t0, *(w+ 1), 0x670C9C61L);
		FF_4(t6, t5, t4, t3, t2, t1, t0, t7, *(w+29), 0xABD388F0L);
		FF_4(t5, t4, t3, t2, t1, t0, t7, t6, *(w+ 5), 0x6A51A0D2L);
		FF_4(t4, t3, t2, t1, t0, t7, t6, t5, *(w+15), 0xD8542F68L);
		FF_4(t3, t2, t1, t0, t7, t6, t5, t4, *(w+17), 0x960FA728L);
		FF_4(t2, t1, t0, t7, t6, t5, t4, t3, *(w+10), 0xAB5133A3L);
		FF_4(t1, t0, t7, t6, t5, t4, t3, t2, *(w+16), 0x6EEF0B6CL);
		FF_4(t0, t7, t6, t5, t4, t3, t2, t1, *(w+13), 0x137A3BE4L);
		}

		if (state->passes == 5) {
		/* Pass 5. executed only when PASS = 5 */
		FF_5(t7, t6, t5, t4, t3, t2, t1, t0, *(w+27), 0xBA3BF050L);
		FF_5(t6, t5, t4, t3, t2, t1, t0, t7, *(w+ 3), 0x7EFB2A98L);
		FF_5(t5, t4, t3, t2, t1, t0, t7, t6, *(w+21), 0xA1F1651DL);
		FF_5(t4, t3, t2, t1, t0, t7, t6, t5, *(w+26), 0x39AF0176L);
		FF_5(t3, t2, t1, t0, t7, t6, t5, t4, *(w+17), 0x66CA593EL);
		FF_5(t2, t1, t0, t7, t6, t5, t4, t3, *(w+11), 0x82430E88L);
		FF_5(t1, t0, t7, t6, t5, t4, t3, t2, *(w+20), 0x8CEE8619L);
		FF_5(t0, t7, t6, t5, t4, t3, t2, t1, *(w+29), 0x456F9FB4L);

		FF_5(t7, t6, t5, t4, t3, t2, t1, t0, *(w+19), 0x7D84A5C3L);
		FF_5(t6, t5, t4, t3, t2, t1, t0, t7, *(w   ), 0x3B8B5EBEL);
		FF_5(t5, t4, t3, t2, t1, t0, t7, t6, *(w+12), 0xE06F75D8L);
		FF_5(t4, t3, t2, t1, t0, t7, t6, t5, *(w+ 7), 0x85C12073L);
		FF_5(t3, t2, t1, t0, t7, t6, t5, t4, *(w+13), 0x401A449FL);
		FF_5(t2, t1, t0, t7, t6, t5, t4, t3, *(w+ 8), 0x56C16AA6L);
		FF_5(t1, t0, t7, t6, t5, t4, t3, t2, *(w+31), 0x4ED3AA62L);
		FF_5(t0, t7, t6, t5, t4, t3, t2, t1, *(w+10), 0x363F7706L);

		FF_5(t7, t6, t5, t4, t3, t2, t1, t0, *(w+ 5), 0x1BFEDF72L);
		FF_5(t6, t5, t4, t3, t2, t1, t0, t7, *(w+ 9), 0x429B023DL);
		FF_5(t5, t4, t3, t2, t1, t0, t7, t6, *(w+14), 0x37D0D724L);
		FF_5(t4, t3, t2, t1, t0, t7, t6, t5, *(w+30), 0xD00A1248L);
		FF_5(t3, t2, t1, t0, t7, t6, t5, t4, *(w+18), 0xDB0FEAD3L);
		FF_5(t2, t1, t0, t7, t6, t5, t4, t3, *(w+ 6), 0x49F1C09BL);
		FF_5(t1, t0, t7, t6, t5, t4, t3, t2, *(w+28), 0x075372C9L);
		FF_5(t0, t7, t6, t5, t4, t3, t2, t1, *(w+24), 0x80991B7BL);

		FF_5(t7, t6, t5, t4, t3, t2, t1, t0, *(w+ 2), 0x25D479D8L);
		FF_5(t6, t5, t4, t3, t2, t1, t0, t7, *(w+23), 0xF6E8DEF7L);
		FF_5(t5, t4, t3, t2, t1, t0, t7, t6, *(w+16), 0xE3FE501AL);
		FF_5(t4, t3, t2, t1, t0, t7, t6, t5, *(w+22), 0xB6794C3BL);
		FF_5(t3, t2, t1, t0, t7, t6, t5, t4, *(w+ 4), 0x976CE0BDL);
		FF_5(t2, t1, t0, t7, t6, t5, t4, t3, *(w+ 1), 0x04C006BAL);
		FF_5(t1, t0, t7, t6, t5, t4, t3, t2, *(w+25), 0xC1A94FB6L);
		FF_5(t0, t7, t6, t5, t4, t3, t2, t1, *(w+15), 0x409F60C4L);
		}

		state->fingerprint[0] += t0;
		state->fingerprint[1] += t1;
		state->fingerprint[2] += t2;
		state->fingerprint[3] += t3;
		state->fingerprint[4] += t4;
		state->fingerprint[5] += t5;
		state->fingerprint[6] += t6;
		state->fingerprint[7] += t7;
	}

	static void haval_tailor (THHavalState *state)
	{
		U32 temp;

		if (state->fptlen == 128) {
		temp = (state->fingerprint[7] & 0x000000FFL) | 
				(state->fingerprint[6] & 0xFF000000L) | 
				(state->fingerprint[5] & 0x00FF0000L) | 
				(state->fingerprint[4] & 0x0000FF00L);
		state->fingerprint[0] += rotate_right(temp,  8);

		temp = (state->fingerprint[7] & 0x0000FF00L) | 
				(state->fingerprint[6] & 0x000000FFL) | 
				(state->fingerprint[5] & 0xFF000000L) | 
				(state->fingerprint[4] & 0x00FF0000L);
		state->fingerprint[1] += rotate_right(temp, 16);

		temp  = (state->fingerprint[7] & 0x00FF0000L) | 
				(state->fingerprint[6] & 0x0000FF00L) | 
				(state->fingerprint[5] & 0x000000FFL) | 
				(state->fingerprint[4] & 0xFF000000L);
		state->fingerprint[2] += rotate_right(temp, 24);

		temp = (state->fingerprint[7] & 0xFF000000L) | 
				(state->fingerprint[6] & 0x00FF0000L) | 
				(state->fingerprint[5] & 0x0000FF00L) | 
				(state->fingerprint[4] & 0x000000FFL);
		state->fingerprint[3] += temp;
		} else if (state->fptlen == 160) {
		temp = (state->fingerprint[7] &  (U32)0x3F) | 
				(state->fingerprint[6] & ((U32)0x7F << 25)) |  
				(state->fingerprint[5] & ((U32)0x3F << 19));
		state->fingerprint[0] += rotate_right(temp, 19);

		temp = (state->fingerprint[7] & ((U32)0x3F <<  6)) | 
				(state->fingerprint[6] &  (U32)0x3F) |  
				(state->fingerprint[5] & ((U32)0x7F << 25));
		state->fingerprint[1] += rotate_right(temp, 25);

		temp = (state->fingerprint[7] & ((U32)0x7F << 12)) | 
				(state->fingerprint[6] & ((U32)0x3F <<  6)) |  
				(state->fingerprint[5] &  (U32)0x3F);
		state->fingerprint[2] += temp;

		temp = (state->fingerprint[7] & ((U32)0x3F << 19)) | 
				(state->fingerprint[6] & ((U32)0x7F << 12)) |  
				(state->fingerprint[5] & ((U32)0x3F <<  6));
		state->fingerprint[3] += temp >> 6; 

		temp = (state->fingerprint[7] & ((U32)0x7F << 25)) | 
				(state->fingerprint[6] & ((U32)0x3F << 19)) |  
				(state->fingerprint[5] & ((U32)0x7F << 12));
		state->fingerprint[4] += temp >> 12;
		} else if (state->fptlen == 192) {
		temp = (state->fingerprint[7] &  (U32)0x1F) | 
				(state->fingerprint[6] & ((U32)0x3F << 26));
		state->fingerprint[0] += rotate_right(temp, 26);

		temp = (state->fingerprint[7] & ((U32)0x1F <<  5)) | 
				(state->fingerprint[6] &  (U32)0x1F);
		state->fingerprint[1] += temp;

		temp = (state->fingerprint[7] & ((U32)0x3F << 10)) | 
				(state->fingerprint[6] & ((U32)0x1F <<  5));
		state->fingerprint[2] += temp >> 5;

		temp = (state->fingerprint[7] & ((U32)0x1F << 16)) | 
				(state->fingerprint[6] & ((U32)0x3F << 10));
		state->fingerprint[3] += temp >> 10;

		temp = (state->fingerprint[7] & ((U32)0x1F << 21)) | 
				(state->fingerprint[6] & ((U32)0x1F << 16));
		state->fingerprint[4] += temp >> 16;

		temp = (state->fingerprint[7] & ((U32)0x3F << 26)) | 
				(state->fingerprint[6] & ((U32)0x1F << 21));
		state->fingerprint[5] += temp >> 21;
		} else if (state->fptlen == 224) {
		state->fingerprint[0] += (state->fingerprint[7] >> 27) & 0x1F;
		state->fingerprint[1] += (state->fingerprint[7] >> 22) & 0x1F;
		state->fingerprint[2] += (state->fingerprint[7] >> 18) & 0x0F;
		state->fingerprint[3] += (state->fingerprint[7] >> 13) & 0x1F;
		state->fingerprint[4] += (state->fingerprint[7] >>  9) & 0x0F;
		state->fingerprint[5] += (state->fingerprint[7] >>  4) & 0x1F;
		state->fingerprint[6] +=  state->fingerprint[7]        & 0x0F;
		}
	}

	static U8 *GetPadding()
	{
		static U8 padding[128] = {        /* constants for padding */
			0x01, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		};
		return padding;
	}
};

typedef THHaval<256,5> THHaval256;

static bool THFloatIsEqu(float a,float b){return ((a-b>=-FLT_EPSILON) && (a-b<=FLT_EPSILON));}
static bool THDoubleIsEqu(double a,double b){return ((a-b>=-DBL_EPSILON) && (a-b<=DBL_EPSILON));}

/**
* @brief 简单计算，统计类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2010-08-14 新建类
*/
template<typename TYPE>
class THSimpleCount
{
public:
	THSimpleCount(){m_min=m_max=m_sum=0;}
	virtual ~THSimpleCount(){}

	void Insert(TYPE val)
	{
		if (m_cnt.size()==0)
			m_min=m_max=m_sum=val;
		else
		{
			m_sum+=val;
			if (m_min>val) m_min=val;
			else if (m_max<val) m_max=val;
		}
		m_cnt.push_back(val);
	}
	TYPE Sum() {return m_sum;}
	TYPE Avg()
	{
		ASSERT(m_cnt.size()!=0);
		if (m_cnt.size()==0) return 0;
		return m_sum/m_cnt.size();
	}
	double AvgByDouble()
	{
		ASSERT(m_cnt.size()!=0);
		if (m_cnt.size()==0) return 0.0;
		return m_sum*1.0/m_cnt.size();
	}
	TYPE Max()
	{
		ASSERT(m_cnt.size()!=0);
		return m_max;
	}
	TYPE Min()
	{
		ASSERT(m_cnt.size()!=0);
		return m_min;
	}
	UINT Count(){return (UINT)m_cnt.size();}
	TYPE Begin()
	{
		ASSERT(m_cnt.size()!=0);
		return *m_cnt.begin();
	}
	TYPE End()
	{
		ASSERT(m_cnt.size()!=0);
		return *(--m_cnt.end());
	}
protected:
	vector<TYPE> m_cnt;
	TYPE m_max;
	TYPE m_min;
	TYPE m_sum;
};

/**
* @brief 映射类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2011-02-19 新建类
*/
/**<pre>
	在需要用一个短小的数和一个大数建立映射时使用。线程安全
	注意，只能由KEY通过GetValue来生成VALUE，不能通过VALUE来生成KEY
	THMapping<HWND,short> map;
	short idx=map.GetValue(hWnd);
	...
	hWnd=map.GetKey(idx);
</pre>*/
template<typename KEY,typename VALUE>
class THMapping
{
public:
	THMapping(){}
	virtual ~THMapping(){}
	VALUE GetValue(KEY key)
	{
		VALUE ret;
		m_mtx.Lock();
		map<KEY,VALUE>::iterator it=m_keys.find(key);
		if (it==m_keys.end())
		{
			ret=VALUE(m_values.size());
			m_keys[key]=ret;
			m_values[ret]=key;
		}
		else
			ret=it->second;
		m_mtx.Unlock();
		return ret;
	}
	KEY GetKey(VALUE val)
	{
		KEY ret=NULL;
		m_mtx.Lock();
		map<VALUE,KEY>::iterator it=m_values.find(val);
		if (it!=m_values.end()) ret=it->second;
		m_mtx.Unlock();
		return ret;
	}
protected:
	map<KEY,VALUE> m_keys;
	map<VALUE,KEY> m_values;
	THMutex m_mtx;
};
