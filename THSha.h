#pragma once

typedef void (*_SHADisplayCallBack)(char *buffer,void *data);

typedef enum _SHATYPE{
	SHA1,
	SHA256
}SHA_TYPE;

typedef struct _SHA_CTX{
	SHA_TYPE type;
	unsigned long state[8];//SHA1:5 SHA256:8
	unsigned long count[2];
	unsigned char buffer[64];
	_SHADisplayCallBack callbackfn;
	void *callbackdata;
}SHA_CTX;


//使用函数
//该类慎用final时会有死循环：如处理：d3b0668687e863369f70170b50325577a5ce84241344654910LJDEZ6
//char *SHAFile(char *path,SHA_TYPE Type,_SHADisplayCallBack cb=NULL,void *data=NULL);
//char *SHAString(char *str,SHA_TYPE Type,_SHADisplayCallBack cb=NULL,void *data=NULL);
//char *SHAData(char *buf,int len,SHA_TYPE Type,_SHADisplayCallBack cb=NULL,void *data=NULL);

void _SHATransform(SHA_CTX* context, unsigned char *buffer);
void _SHAInit(SHA_CTX* context,SHA_TYPE Type,_SHADisplayCallBack cb,void *data);
void _SHAUpdate(SHA_CTX* context, unsigned char* data, unsigned int len);
void _SHAFinal(SHA_CTX* context);
void _SHAGetResult(SHA_CTX* context,char *buffer);
void _SHAPrintProcess(SHA_CTX* context);



#include "stdAfx.h"
#include <stdio.h>
#include <string.h>
//#include "sha.h"

unsigned long K1[4]={0x5A827999,0x6ED9EBA1,0x8F1BBCDC,0xCA62C1D6};
unsigned long K256[64]={0x428A2F98,0x71374491,0xB5C0FBCF,0xE9B5DBA5,0x3956C25B,0x59F111F1,
				0x923F82A4,0xAB1C5ED5,0xD807AA98,0x12835B01,0x243185BE,0x550C7DC3,
				0x72BE5D74,0x80DEB1FE,0x9BDC06A7,0xC19BF174,0xE49B69C1,0xEFBE4786,
				0x0FC19DC6,0x240CA1CC,0x2DE92C6F,0x4A7484AA,0x5CB0A9DC,0x76F988DA,
				0x983E5152,0xA831C66D,0xB00327C8,0xBF597FC7,0xC6E00BF3,0xD5A79147,
				0x06CA6351,0x14292967,0x27B70A85,0x2E1B2138,0x4D2C6DFC,0x53380D13,
				0x650A7354,0x766A0ABB,0x81C2C92E,0x92722C85,0xA2BFE8A1,0xA81A664B,
				0xC24B8B70,0xC76C51A3,0xD192E819,0xD6990624,0xF40E3585,0x106AA070,
				0x19A4C116,0x1E376C08,0x2748774C,0x34B0BCB5,0x391C0CB3,0x4ED8AA4A,
				0x5B9CCA4F,0x682E6FF3,0x748F82EE,0x78A5636F,0x84C87814,0x8CC70208,
				0x90BEFFFA,0xA4506CEB,0xBEF9A3F7,0xC67178F2};

inline unsigned long SHR(unsigned long lX,int n)
{
	return lX>>n;
}

inline unsigned long ROTR(unsigned long lX,int n)
{
	return (lX>>n)|(lX<<(32-n));
}

inline unsigned long ROTL(unsigned long lX,int n)
{
	return (lX<<n)|(lX>>(32-n));
}


inline unsigned long Ch(unsigned long x,unsigned long y,unsigned long z)
{
	return (x&y)^((~x)&z);
}

inline unsigned long Maj(unsigned long x,unsigned long y,unsigned long z)
{
	return (x&y)^(x&z)^(y&z);
}

inline unsigned long Parity(unsigned long x,unsigned long y,unsigned long z)
{
	return x^y^z;
}

typedef unsigned long (*_SHAFunction)(unsigned long a,unsigned long b,unsigned long c);
_SHAFunction ft[4]={Ch,Parity,Maj,Parity};

inline unsigned long AddUnsigned(unsigned long lX,unsigned long lY)
{
	return lX+lY;
	//not need in the 32 bit machine
/*	unsigned long lX4,lY4,lX8,lY8,lResult;
	lX8=lX&0x80000000;
	lY8=lY&0x80000000;
	lX4=lX&0x40000000;
	lY4=lY&0x40000000;
	lResult=(lX&0x3FFFFFFF)+(lY&0x3FFFFFFF);
	if (lX4&lY4)
		lResult=lResult^0x80000000^lX8^lY8;
	else if (lX4|lY4)
	{
		if (lResult&0x40000000)
			lResult=lResult^0xC0000000^lX8^lY8;
		else
			lResult=lResult^0x40000000^lX8^lY8;
	}
	else
		lResult=lResult^lX8^lY8;
	return lResult;*/
}

inline unsigned long Sigma0(unsigned long x)
{
	return ROTR(x,2)^ROTR(x,13)^ROTR(x,22);
}

inline unsigned long Sigma1(unsigned long x)
{
	return ROTR(x,6)^ROTR(x,11)^ROTR(x,25);
}

inline unsigned long Gamma0(unsigned long x)
{
	return ROTR(x,7)^ROTR(x,18)^SHR(x,3);
}

inline unsigned long Gamma1(unsigned long x)
{
	return ROTR(x,17)^ROTR(x,19)^SHR(x,10);
}

//哈希每个单独的512bit
void _SHATransform(SHA_CTX* context,unsigned char *buffer)
{
	//必须把低位转成高位
	for(int i=0;i<64;i+=4)
	{
		unsigned char tmp=*(buffer+i);
		*(buffer+i)=*(buffer+i+3);
		*(buffer+i+3)=tmp;
		tmp=*(buffer+i+1);
		*(buffer+i+1)=*(buffer+i+2);
		*(buffer+i+2)=tmp;
	}
	unsigned long W[80];
	unsigned long a,b,c,d,e,f,g,h,j,T,T1,T2,Round=79;
	a=context->state[0];b=context->state[1];
	c=context->state[2];d=context->state[3];
	e=context->state[4];
	if (context->type==SHA256)
	{
		f=context->state[5];g=context->state[6];h=context->state[7];
		Round=63;
	}
	for (j=0;j<=Round;j++)
	{
		if (j<16)
			W[j]=*(((unsigned long *)buffer)+j);
		else
		{
			if (context->type==SHA1)
				W[j]=ROTL(W[j-3]^W[j-8]^W[j-14]^W[j-16],1);
			else if (context->type==SHA256)
				W[j]=AddUnsigned(AddUnsigned(AddUnsigned(Gamma1(W[j-2]),W[j-7]),Gamma0(W[j-15])),W[j-16]);
		}
		if (context->type==SHA1)
		{
			T=AddUnsigned(AddUnsigned(AddUnsigned(AddUnsigned(ROTL(a,5),(ft[j/20])(b,c,d)),e),K1[j/20]),W[j]);
			e=d;
			d=c;
			c=ROTL(b,30);
			b=a;
			a=T;
		}
		else if (context->type==SHA256)
		{
			T1=AddUnsigned(AddUnsigned(AddUnsigned(AddUnsigned(h,Sigma1(e)),Ch(e,f,g)),K256[j]),W[j]);
			T2=AddUnsigned(Sigma0(a),Maj(a,b,c));
			h=g;
			g=f;
			f=e;
			e=AddUnsigned(d,T1);
			d=c;
			c=b;
			b=a;
			a=AddUnsigned(T1,T2);
		}
	}
	context->state[0]=AddUnsigned(a,context->state[0]);
	context->state[1]=AddUnsigned(b,context->state[1]);
	context->state[2]=AddUnsigned(c,context->state[2]);
	context->state[3]=AddUnsigned(d,context->state[3]);
	context->state[4]=AddUnsigned(e,context->state[4]);
	if (context->type==SHA256)
	{
		context->state[5]=AddUnsigned(f,context->state[5]);
		context->state[6]=AddUnsigned(g,context->state[6]);
		context->state[7]=AddUnsigned(h,context->state[7]);
	}
	_SHAPrintProcess(context);
}

//初始化函数
void _SHAInit(SHA_CTX* context,SHA_TYPE Type,_SHADisplayCallBack cb,void *data)
{
	unsigned long HASH1[5]={
		0x67452301,0xEFCDAB89,0x98BADCFE,0x10325476,0xC3D2E1F0};
	unsigned long HASH256[8]={
		0x6A09E667,0xBB67AE85,0x3C6EF372,0xA54FF53A,
		0x510E527F,0x9B05688C,0x1F83D9AB,0x5BE0CD19};
	context->type=Type;
	if (context->type==SHA1)
		memcpy(context->state,HASH1,sizeof(HASH1));
	else
		memcpy(context->state,HASH256,sizeof(HASH256));
	context->count[0]=0;
	context->count[1]=0;
	context->callbackfn=cb;
	context->callbackdata=data;
}

//执行该函数对加入的data进行hash值的更新
void _SHAUpdate(SHA_CTX* context, unsigned char* data, unsigned int len)
{
	unsigned int i,j;
	j=context->count[0];
	context->count[1]+=len;
	if (j+len>63)
	{
		memcpy(&context->buffer[j],data,(i=64-j));
		_SHATransform(context,context->buffer);
		for(;i+63<len;i+=64)
			_SHATransform(context,&data[i]);
	}
	else
		i=0;
	if (len-i>0)
	{
		memcpy(&context->buffer[j],&data[i],len-i);
		context->count[0]+=len-i;
	}
}

//处理最后的结尾
void _SHAFinal(SHA_CTX* context)
{
	unsigned long count[2];
	count[0]=context->count[1]>>29;
	count[1]=context->count[1]<<3;
	unsigned char buf[4];
	_SHAUpdate(context,(unsigned char *)"\200", 1);
	while(context->count[0]!=64-8)
		_SHAUpdate(context,(unsigned char *)"\0",1);
	memcpy(buf,(char *)&count[0],4);
	_SHAUpdate(context,(unsigned char *)&buf[3],1);
	_SHAUpdate(context,(unsigned char *)&buf[2],1);
	_SHAUpdate(context,(unsigned char *)&buf[1],1);
	_SHAUpdate(context,(unsigned char *)&buf[0],1);
	memcpy(buf,(char *)&count[1],4);
	_SHAUpdate(context,(unsigned char *)&buf[3],1);
	_SHAUpdate(context,(unsigned char *)&buf[2],1);
	_SHAUpdate(context,(unsigned char *)&buf[1],1);
	_SHAUpdate(context,(unsigned char *)&buf[0],1);
}

//把结果转换成16进制
inline void _SHAGetResult(SHA_CTX* context,char *buffer)
{
	*buffer='\0';
	if (context->type==SHA1)
		sprintf(buffer,"%08X%08X%08X%08X%08X",
			context->state[0],context->state[1],context->state[2],context->state[3],context->state[4]);
	else if (context->type==SHA256)
		sprintf(buffer,"%08X%08X%08X%08X%08X%08X%08X%08X",
			context->state[0],context->state[1],context->state[2],context->state[3],
			context->state[4],context->state[5],context->state[6],context->state[7]);
}

//打印中间结果
inline void _SHAPrintProcess(SHA_CTX* context)
{
	if (context->callbackfn)
	{
		char buffer[200];
		_SHAGetResult(context,buffer);
		(context->callbackfn)(buffer,context->callbackdata);
	}
}

//得到字符串的结果
/*char *SHAString(char *str,SHA_TYPE Type,_SHADisplayCallBack cb,void *data)
{
	SHA_CTX context;
	char buffer[200];

	_SHAInit(&context,Type,cb,data);
	_SHAUpdate(&context,(unsigned char *)str,(int)strlen(str));
	_SHAFinal(&context);
	_SHAGetResult(&context,buffer);
	return strdup(buffer);
}*/

//得到数据串的结果
/*char *SHAData(char *buf,int len,SHA_TYPE Type,_SHADisplayCallBack cb,void *data)
{
	SHA_CTX context;
	char buffer[200];

	_SHAInit(&context,Type,cb,data);
	_SHAUpdate(&context,(unsigned char *)buf,len);
	_SHAFinal(&context);
	_SHAGetResult(&context,buffer);
	return strdup(buffer);
}*/

//得到文件的结果
/*char *SHAFile(char *path,SHA_TYPE Type,_SHADisplayCallBack cb,void *data)
{
	int i;
	SHA_CTX context;
	unsigned char filebuf[16384];
	char buffer[200];
	FILE* file;
	if (!(file=fopen(path,"rb"))) return NULL;

	_SHAInit(&context,Type,cb,data);
	while (!feof(file))
	{
		i=(int)fread(filebuf,1,16384,file);
		_SHAUpdate(&context,filebuf,i);
	}
	_SHAFinal(&context);
	fclose(file);
	_SHAGetResult(&context,buffer);
	return strdup(buffer);
}*/
