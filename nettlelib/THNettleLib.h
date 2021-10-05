#pragma once

#define MAX_NETTLEHASHRESULT	32

enum THNettleHashType{
	THNettle_Hash_Md2,
	THNettle_Hash_Md4,
	THNettle_Hash_Md5,
	THNettle_Hash_Sha1,
	THNettle_Hash_Sha256,
	THNettle_Hash_End
};

enum THNettleCipherType{
	THNettle_Cipher_Aes128,
	THNettle_Cipher_Aes192,
	THNettle_Cipher_Aes256,
	THNettle_Cipher_Arcfour128,
	THNettle_Cipher_Cast128,
	THNettle_Cipher_Serpent128,
	THNettle_Cipher_Serpent192,
	THNettle_Cipher_Serpent256,
	THNettle_Cipher_Twofish128,
	THNettle_Cipher_Twofish192,
	THNettle_Cipher_Twofish256,
	THNettle_Cipher_Arctwo40,
	THNettle_Cipher_Arctwo64,
	THNettle_Cipher_Arctwo128,
	THNettle_Cipher_Arctwo_gutmann128,
	THNettle_Cipher_End
};

enum THNettleArmorType{
	THNettle_Armor_Base16,
	THNettle_Armor_Base64,
	THNettle_Armor_Xxtea,
	THNettle_Armor_GZip,
	THNettle_Armor_Lzma,	//lzma算法
	THNettle_Armor_7Zip,	//7zip是使用lzma算法的文件格式
	THNettle_Armor_Rar,
	THNettle_Armor_End
};

typedef struct _THNETTLEHASHRESULT
{
public:
	_THNETTLEHASHRESULT()
	{
		memset(m,0,sizeof(m));
		len=0;
		type=THNettle_Hash_End;
	}
	_THNETTLEHASHRESULT(const struct _THNETTLEHASHRESULT &buf)
	{
		memcpy(m,buf.m,sizeof(m));
		len=buf.len;
		type=buf.type;
	}
	unsigned char m[MAX_NETTLEHASHRESULT];
	THNettleHashType type;
	unsigned int len;
	void Empty()
	{
		memset(m,0,sizeof(m));
		len=0;
		type=THNettle_Hash_End;
	}
}THNETTLEHASHRESULT,*PTHNETTLEHASHRESULT;

typedef int BOOL;

/**
* @brief Nettle算法库底层封装类，隔离所有函数
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2009-09-1 新建类
*/
/**<pre>
  支持算法详见，nettlelib\nettle.pdf
  rsa dsa 有问题，mpz_t 及 相关操作函数问题
  可支持 yarrow,des_key_schedule,des_cblock,des_check_key,des_cbc_cksum,des_cbc_encrypt,des_ecb2_encrypt,des_ecb3_encrypt,des_ecb_encrypt,des_ede2_cbc_encrypt,des_ede3_cbc_encrypt,des_is_weak_key,des_key_sched,des_ncbc_encrypt,des_set_key,des_set_odd_parity，目前没有封装出来
</pre>*/
class THNettleLib
{
public:
	static BOOL Hash(const uint8_t *data,unsigned int length,PTHNETTLEHASHRESULT ret,THNettleHashType type);
	static BOOL HashFile(const TCHAR *sFile,PTHNETTLEHASHRESULT ret,THNettleHashType type);
	static BOOL Hmac(const uint8_t *data,unsigned int length,const uint8_t *key,unsigned int keylength,PTHNETTLEHASHRESULT ret,THNettleHashType type);
	static BOOL HmacFile(const TCHAR *sFile,const uint8_t *key,unsigned int keylength,PTHNETTLEHASHRESULT ret,THNettleHashType type);
	static BOOL Cipher(const uint8_t *data,unsigned int length,const uint8_t *key,unsigned int keylength,uint8_t *out,THNettleCipherType type,BOOL bCipher);
	static BOOL Cipher_CBC(const uint8_t *data,unsigned int length,const uint8_t *key,unsigned int keylength,uint8_t *out,const uint8_t *iiv,unsigned int iivlength,THNettleCipherType type,BOOL bCipher);
	static BOOL Cipher_CTR(const uint8_t *data,unsigned int length,const uint8_t *key,unsigned int keylength,uint8_t *out,const uint8_t *ictr,unsigned int ictrlength,THNettleCipherType type);
	//base64
	static BOOL ArmorEncode(const uint8_t *data,unsigned int length,uint8_t **out,THNettleArmorType type);
	static BOOL ArmorDecode(const uint8_t *text,uint8_t **out,unsigned int *outlength,THNettleArmorType type);
	static void ArmorFree(void *buf);

	typedef struct _THNettleArithMap{
		void *buf;
	}THNettleArithMap;
	static THNettleArithMap g_nettlehashmap[];
	static THNettleArithMap g_nettleciphermap[];
	static THNettleArithMap g_nettlearmormap[];
};
