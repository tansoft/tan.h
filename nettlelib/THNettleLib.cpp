
#include "md2.h"
#include "md4.h"
#include "md5.h"
#include "sha.h"
#include "aes.h"
#include "des.h"
#include "base16.h"
#include "base64.h"
#include "hmac.h"
#include "twofish.h"
#include "blowfish.h"
#include "cbc.h"
#include "ctr.h"
#include "hmac.h"
#include "nettle-meta.h"
#include <memory.h>
#include <string.h>
#include <stdio.h>
#include <tchar.h>
#include "THNettleLib.h"

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef NULL
#define NULL 0
#endif

#define mymin(a,b) ((a)<(b)?(a):(b))

THNettleLib::THNettleArithMap THNettleLib::g_nettlehashmap[]={
	{(void *)&nettle_md2},
	{(void *)&nettle_md4},
	{(void *)&nettle_md5},
	{(void *)&nettle_sha1},
	{(void *)&nettle_sha256},
	{0}//Hash End
};

THNettleLib::THNettleArithMap THNettleLib::g_nettleciphermap[]={
	{(void *)&nettle_aes128},
	{(void *)&nettle_aes192},
	{(void *)&nettle_aes256},
	{(void *)&nettle_arcfour128},
	{(void *)&nettle_cast128},
	{(void *)&nettle_serpent128},
	{(void *)&nettle_serpent192},
	{(void *)&nettle_serpent256},
	{(void *)&nettle_twofish128},
	{(void *)&nettle_twofish192},
	{(void *)&nettle_twofish256},
	{(void *)&nettle_arctwo40},
	{(void *)&nettle_arctwo64},
	{(void *)&nettle_arctwo128},
	{(void *)&nettle_arctwo_gutmann128},
	{0}//Cipher End
};

THNettleLib::THNettleArithMap THNettleLib::g_nettlearmormap[]={
	{(void *)&nettle_base16},
	{(void *)&nettle_base64},
	{0}//Armor End
};

BOOL THNettleLib::Hash(const uint8_t *data,unsigned int length,PTHNETTLEHASHRESULT ret,THNettleHashType type)
{
	if (type>=THNettle_Hash_End || ret==NULL || data==NULL) return FALSE;
	ret->type=type;
	struct nettle_hash *hash=(struct nettle_hash *)g_nettlehashmap[type].buf;
	ret->len=hash->digest_size;
	uint8_t *ctx=new uint8_t[hash->context_size];
	hash->init(ctx);
	hash->update(ctx,length,data);
	hash->digest(ctx,hash->digest_size,ret->m);
	delete [] ctx;
	return TRUE;
}

BOOL THNettleLib::HashFile(const TCHAR *sFile,PTHNETTLEHASHRESULT ret,THNettleHashType type)
{
	if (type>=THNettle_Hash_End || sFile==NULL || ret==NULL) return FALSE;
	FILE *f=_tfopen(sFile,"rb");
	if (!f) return FALSE;
	ret->type=type;
	struct nettle_hash *hash=(struct nettle_hash *)g_nettlehashmap[type].buf;
	ret->len=hash->digest_size;
	uint8_t *ctx=new uint8_t[hash->context_size];
	hash->init(ctx);
	uint8_t buf[4096];
	while(1)
	{
		unsigned int done=(unsigned int)fread(buf,1,sizeof(buf),f);
		if (done) hash->update(ctx,done,buf);
		if (done<sizeof(buf)) break;
	}
	fclose(f);
	hash->digest(ctx,hash->digest_size,ret->m);
	delete [] ctx;
	return TRUE;
}

BOOL THNettleLib::Hmac(const uint8_t *data,unsigned int length,const uint8_t *key,unsigned int keylength,PTHNETTLEHASHRESULT ret,THNettleHashType type)
{
	if (type>=THNettle_Hash_End || ret==NULL || key==NULL || data==NULL) return FALSE;
	ret->type=type;
	struct nettle_hash *hash=(struct nettle_hash *)g_nettlehashmap[type].buf;
	ret->len=hash->digest_size;
	uint8_t *in=new uint8_t[hash->context_size];
	uint8_t *out=new uint8_t[hash->context_size];
	uint8_t *state=new uint8_t[hash->context_size];
	hmac_set_key(out,in,state,hash,keylength,key);
	hmac_update(state,hash,length,data);
	hmac_digest(out,in,state,hash,hash->digest_size,ret->m);
	delete [] in;
	delete [] out;
	delete [] state;
	return TRUE;
}

BOOL THNettleLib::HmacFile(const TCHAR *sFile,const uint8_t *key,unsigned int keylength,PTHNETTLEHASHRESULT ret,THNettleHashType type)
{
	if (type>=THNettle_Hash_End || sFile==NULL || key==NULL || ret==NULL) return FALSE;
	FILE *f=_tfopen(sFile,"rb");
	if (!f) return FALSE;
	ret->type=type;
	struct nettle_hash *hash=(struct nettle_hash *)g_nettlehashmap[type].buf;
	ret->len=hash->digest_size;
	uint8_t *in=new uint8_t[hash->context_size];
	uint8_t *out=new uint8_t[hash->context_size];
	uint8_t *state=new uint8_t[hash->context_size];
	hmac_set_key(out,in,state,hash,keylength,key);
	uint8_t buf[4096];
	while(1)
	{
		unsigned int done=(unsigned int)fread(buf,1,sizeof(buf),f);
		if (done) hmac_update(state,hash,done,buf);
		if (done<sizeof(buf)) break;
	}
	fclose(f);
	hmac_digest(out,in,state,hash,hash->digest_size,ret->m);
	delete [] in;
	delete [] out;
	delete [] state;
	return TRUE;
}

BOOL THNettleLib::Cipher(const uint8_t *data,unsigned int length,const uint8_t *key,unsigned int keylength,uint8_t *out,THNettleCipherType type,BOOL bCipher)
{
	if (type>=THNettle_Cipher_End || data==NULL || key==NULL || out==NULL) return FALSE;
	struct nettle_cipher *cipher=(struct nettle_cipher *)g_nettleciphermap[type].buf;
	uint8_t *ctx=new uint8_t[cipher->context_size];
	if (bCipher)
	{
		cipher->set_encrypt_key(ctx,keylength,key);
		cipher->encrypt(ctx,length,out,data);
	}
	else
	{
		cipher->set_decrypt_key(ctx,keylength,key);
		cipher->decrypt(ctx,length,out,data);
	}
	delete [] ctx;
	return TRUE;
}

BOOL THNettleLib::Cipher_CBC(const uint8_t *data,unsigned int length,const uint8_t *key,unsigned int keylength,uint8_t *out,const uint8_t *iiv,unsigned int iivlength,THNettleCipherType type,BOOL bCipher)
{
	if (type>=THNettle_Cipher_End || data==NULL || key==NULL || out==NULL) return FALSE;
	struct nettle_cipher *cipher=(struct nettle_cipher *)g_nettleciphermap[type].buf;
	uint8_t *ctx=new uint8_t[cipher->context_size];
	uint8_t *iv=new uint8_t[cipher->block_size];
	memset(iv,0,cipher->block_size);
	memcpy(iv,iiv,mymin(cipher->block_size,iivlength));
	if (bCipher)
	{
		cipher->set_encrypt_key(ctx,keylength,key);
		cbc_encrypt(ctx,cipher->encrypt,cipher->block_size,iv,length,out,data);
	}
	else
	{
		cipher->set_decrypt_key(ctx,keylength,key);
		cbc_decrypt(ctx,cipher->encrypt,cipher->block_size,iv,length,out,data);
	}
	delete [] iv;
	delete [] ctx;
	return TRUE;
}

BOOL THNettleLib::Cipher_CTR(const uint8_t *data,unsigned int length,const uint8_t *key,unsigned int keylength,uint8_t *out,const uint8_t *ictr,unsigned int ictrlength,THNettleCipherType type)
{
	if (type>=THNettle_Cipher_End || data==NULL || key==NULL || out==NULL) return FALSE;
	struct nettle_cipher *cipher=(struct nettle_cipher *)g_nettleciphermap[type].buf;
	uint8_t *ctx=new uint8_t[cipher->context_size];
	uint8_t *ctr=new uint8_t[cipher->block_size];
	cipher->set_encrypt_key(ctx,keylength,key);
	memset(ctr,0,cipher->block_size);
	memcpy(ctr,ictr,mymin(cipher->block_size,ictrlength));
	ctr_crypt(ctx,cipher->encrypt,cipher->block_size,ctr,length,out,data);
	delete [] ctr;
	delete [] ctx;
	return TRUE;
}

BOOL THNettleLib::ArmorEncode(const uint8_t *data,unsigned int length,uint8_t **out,THNettleArmorType type)
{
	if (type>=THNettle_Armor_End || data==NULL || out==NULL || *out==NULL) return FALSE;
	struct nettle_armor *armor=(struct nettle_armor *)g_nettlearmormap[type].buf;
	uint8_t *ctx=new uint8_t[armor->encode_context_size];
	uint8_t *buffer=new uint8_t[armor->encode_length(length)+armor->encode_final_length+1];
	armor->encode_init(ctx);
	unsigned int done=armor->encode_update(ctx,buffer,length,data);
	done+=armor->encode_final(ctx,buffer+done);
	*(buffer+done)='\0';
	*out=buffer;
	return TRUE;
}

BOOL THNettleLib::ArmorDecode(const uint8_t *text,uint8_t **out,unsigned int *outlength,THNettleArmorType type)
{
	if (type>=THNettle_Armor_End || text==NULL || out==NULL || *out==NULL || outlength==NULL) return FALSE;
	struct nettle_armor *armor=(struct nettle_armor *)g_nettlearmormap[type].buf;
	uint8_t *ctx=new uint8_t[armor->decode_context_size];
	uint32_t length=(uint32_t)strlen((const char *)text);
	*out=new uint8_t[armor->decode_length(length)];
	armor->decode_init(ctx);
	BOOL ret=armor->decode_update(ctx,outlength,*out,length,text);
	armor->decode_final(ctx);
	return ret;
}

void THNettleLib::ArmorFree(void *buf)
{
	if (buf) delete [] buf;
}
