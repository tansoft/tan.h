
#include <windows.h>
#include <stdio.h>
#include "aes.h"


    const BYTE Sbox[16][16]={  // populate the Sbox matrix
//#typedef BYTE unsigned char
/* 0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f */
/*0*/  {0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76},
/*1*/  {0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0},
/*2*/  {0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15},
/*3*/  {0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75},
/*4*/  {0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84},
/*5*/  {0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf},
/*6*/  {0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8},
/*7*/  {0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2},
/*8*/  {0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73},
/*9*/  {0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb},
/*a*/  {0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79},
/*b*/  {0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08},
/*c*/  {0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a},
/*d*/  {0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e},
/*e*/  {0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf},
/*f*/  {0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16}};

	const BYTE iSbox[16][16]={  // populate the iSbox matrix
/* 0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f */
/*0*/  {0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb},
/*1*/  {0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb},
/*2*/  {0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e},
/*3*/  {0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25},
/*4*/  {0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92},
/*5*/  {0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84},
/*6*/  {0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06},
/*7*/  {0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b},
/*8*/  {0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73},
/*9*/  {0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e},
/*a*/  {0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b},
/*b*/  {0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4},
/*c*/  {0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f},
/*d*/  {0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef},
/*e*/  {0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61},
/*f*/  {0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d}};
		
	const BYTE Rcon[11][4]={
		{0x00, 0x00, 0x00, 0x00},  
		{0x01, 0x00, 0x00, 0x00},
		{0x02, 0x00, 0x00, 0x00},
		{0x04, 0x00, 0x00, 0x00},
		{0x08, 0x00, 0x00, 0x00},
		{0x10, 0x00, 0x00, 0x00},
		{0x20, 0x00, 0x00, 0x00},
		{0x40, 0x00, 0x00, 0x00},
		{0x80, 0x00, 0x00, 0x00},
		{0x1b, 0x00, 0x00, 0x00},
		{0x36, 0x00, 0x00, 0x00}};
//测试例子
//	BYTE keyBytes[16]={0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
//						0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
//	BYTE input[16]={0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
//						0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
//	BYTE output[16],outout[16];
//	Init(keyBytes);
//	Cipher(input,output);
//	InvCipher(output,outout);

BOOL THAsciiAes::AesEncryptFileEx(const char *chInputFile,const char *chOutputFile,const char *key)
{
	BYTE keyBytes[16],input[16],output[16];
	FILE *fr,*fw;
	long size;
	//拷贝密码串
	if (strlen(key)>=16)
		memcpy(keyBytes,key,sizeof(keyBytes));
	else
	{
		memset(keyBytes,0,sizeof(keyBytes));
		strcpy((char *)keyBytes,key);
	}
	//初始化
	Init(keyBytes);
	//打开文件
	if ((fr=fopen(chInputFile,"rb"))==NULL)
	{
		strcpy(ErrorBuffer,"打开源文件错误！");
		return FALSE;
	}
	DeleteFile(chOutputFile);
	if ((fw=fopen(chOutputFile,"wb"))==NULL)
	{
		strcpy(ErrorBuffer,"打开输出文件错误！");
		return FALSE;
	}
	//读取文件大小
	fseek(fr,0,2);
	size=ftell(fr);
	fseek(fr,0,0);
	//写入文件大小和特征
	memcpy(input,&size,sizeof(long));
	memcpy((char *)input+sizeof(long),"THAES",5);
	Cipher(input,output);
	fwrite(output,1,sizeof(output),fw);
	//循环加密内容
	while(fread(input,1,sizeof(input),fr)!=0)
	{
		Cipher(input,output);
		fwrite(output,1,sizeof(output),fw);
	}
	fclose(fr);
	fclose(fw);
	ErrorBuffer[0]='\0';
	return TRUE;
}

BOOL THAsciiAes::AesDecryptFileEx(const char *chInputFile,const char *chOutputFile,const char *key)
{
	BYTE keyBytes[16],input[16],output[16];
	FILE *fr,*fw;
	long size;
	//拷贝密码串
	if (strlen(key)>=16)
		memcpy(keyBytes,key,sizeof(keyBytes));
	else
	{
		memset(keyBytes,0,sizeof(keyBytes));
		strcpy((char *)keyBytes,key);
	}
	//初始化
	Init(keyBytes);
	//打开文件
	if ((fr=fopen(chInputFile,"rb"))==NULL)
	{
		strcpy(ErrorBuffer,"打开源文件错误！");
		return FALSE;
	}
	DeleteFile(chOutputFile);
	if ((fw=fopen(chOutputFile,"wb"))==NULL)
	{
		strcpy(ErrorBuffer,"打开输出文件错误！");
		return FALSE;
	}
	//读取加密文件大小和记录的文件大小
	fread(input,1,sizeof(input),fr);
	InvCipher(input,output);
	if (strncmp((char *)output+sizeof(long),"THAES",5)!=0)
	{
		strcpy(ErrorBuffer,"密码错误！");
		return FALSE;
	}
	memcpy(&size,output,sizeof(long));
	//循环解密内容
	long tmp=sizeof(input);
	while(size>0)
	{
		if (fread(input,1,sizeof(input),fr)!=sizeof(input))
		{
			strcpy(ErrorBuffer,"读取文件错误，文件不是已加密文件或者被损坏！");
			return FALSE;
		}
		InvCipher(input,output);
		if (size<16) tmp=size;
		fwrite(output,1,tmp,fw);
		size-=tmp;
	}
	fclose(fr);
	fclose(fw);
	ErrorBuffer[0]='\0';
	return TRUE;
}

char* THAsciiAes::_AesFormat(const char *buf,int len)
{
	char *ret=(char *)malloc(len*2+1);
	if (!ret) return NULL;
	*ret='\0';
	for(int i=0;i<len;i++)
		sprintf(ret+i*2,"%02x",(unsigned char)*(buf+i));
	return ret;
}

char *THAsciiAes::AesEncryptTextEx(const char *chInput,const char *key)
{
	int len=(int)strlen(chInput);
	int out;
	char *tmp=AesEncryptBufferEx(chInput,len,&out,key);
	if (!tmp) return NULL;
	//format string
	char *display=_AesFormat(tmp,out);
	AesFree(tmp);
	return display;
}

char *THAsciiAes::AesDecryptTextEx(const char *chInput,const char *key)
{
	int len=(int)strlen(chInput);
	//申请字符串缓冲
	int size=(int)strlen(chInput)/2;
	char *inbuffer=(char *)malloc(size);
	if (!inbuffer)
	{
		strcpy(ErrorBuffer,"申请内存空间错误！");
		return NULL;
	}
	for(int i=0;i<size;i++)
		*(inbuffer+i)=GetHex(chInput+i*2);
	int out;
	char *ret=AesDecryptBufferEx(inbuffer,size,&out,key);
	AesFree(inbuffer);
	char *rettmp=(char *)malloc(out+1);
	if (rettmp)
	{
		memcpy(rettmp,ret,out);
		*(rettmp+out)='\0';
	}
	AesFree(ret);
	return rettmp;
}

char* THAsciiAes::AesEncryptBuffer(const void *chInput,int nInLen,int *nOutLen,const char *key)
{
	if (nOutLen) *nOutLen=0;
	BYTE keyBytes[16];
	//拷贝密码串
	if (strlen(key)>=16)
		memcpy(keyBytes,key,sizeof(keyBytes));
	else
	{
		memset(keyBytes,0,sizeof(keyBytes));
		strcpy((char *)keyBytes,key);
	}
	//初始化
	Init(keyBytes);
	//申请字符串缓冲
	long tmpsize=nInLen;
	int size=(((int)(tmpsize/16))+1)*16;
	char *buffer=(char *)malloc(size);
	if (!buffer)
	{
		strcpy(ErrorBuffer,"申请内存空间错误！");
		return NULL;
	}
	BYTE input[16];
	int i=0;
	if (nOutLen) *nOutLen=size;
	size=sizeof(input);
	while(tmpsize>0)
	{
		if (tmpsize<size)
		{
			memset(input,0,sizeof(input));
			memcpy(input,(BYTE *)chInput+i*sizeof(input),tmpsize);
			Cipher(input,(BYTE *)buffer+i*sizeof(input));
			size=tmpsize;
		}
		else
			Cipher((BYTE *)chInput+i*sizeof(input),(BYTE *)buffer+i*sizeof(input));
		tmpsize-=size;
		i++;
	}
	ErrorBuffer[0]='\0';
	return buffer;
}

char* THAsciiAes::AesEncryptBufferEx(const void *chInput,int nInLen,int *nOutLen,const char *key)
{
	if (nOutLen) *nOutLen=0;
	BYTE keyBytes[16];
	//拷贝密码串
	if (strlen(key)>=16)
		memcpy(keyBytes,key,sizeof(keyBytes));
	else
	{
		memset(keyBytes,0,sizeof(keyBytes));
		strcpy((char *)keyBytes,key);
	}
	//初始化
	Init(keyBytes);
	//申请字符串缓冲
	long tmpsize=nInLen;
	int size=(((int)(tmpsize/16))+2)*16;//+2是因为第一段为记录字符串长度和标记,之后的是为了取整
	char *buffer=(char *)malloc(size);
	if (!buffer)
	{
		strcpy(ErrorBuffer,"申请内存空间错误！");
		return NULL;
	}
	BYTE input[16];
	//写入大小和特征
	memcpy(input,&tmpsize,sizeof(long));
	memcpy((char *)input+sizeof(long),"THAES",5);
	Cipher(input,(BYTE *)buffer);
	int i=1;
	if (nOutLen) *nOutLen=size;
	size=sizeof(input);
	while(tmpsize>0)
	{
		if (tmpsize<size)
		{
			memcpy(input,(BYTE *)chInput+(i-1)*sizeof(input),tmpsize);
			Cipher(input,(BYTE *)buffer+i*sizeof(input));
			size=tmpsize;
		}
		else
			Cipher((BYTE *)chInput+(i-1)*sizeof(input),(BYTE *)buffer+i*sizeof(input));
		tmpsize-=size;
		i++;
	}
	ErrorBuffer[0]='\0';
	return buffer;
}


char *THAsciiAes::AesDecryptBuffer(const void *chInput,int nInLen,int *nOutLen,const char *key)
{
	if (nOutLen) *nOutLen=0;
	BYTE keyBytes[16];
	//拷贝密码串
	if (strlen(key)>=16)
		memcpy(keyBytes,key,sizeof(keyBytes));
	else
	{
		memset(keyBytes,0,sizeof(keyBytes));
		strcpy((char *)keyBytes,key);
	}
	//初始化
	Init(keyBytes);
	if (nInLen%16!=0)
	{
		strcpy(ErrorBuffer,"密文长度错误！");
		return NULL;
	}
	char *buffer=(char *)malloc(nInLen);
	if (!buffer)
	{
		strcpy(ErrorBuffer,"申请内存空间错误！");
		return NULL;
	}
	for(int i=0;i<nInLen/16;i++)
		InvCipher((BYTE *)chInput+i*16,(BYTE *)buffer+i*16);
	if (nOutLen) *nOutLen=nInLen;
	ErrorBuffer[0]='\0';
	return buffer;
}

char *THAsciiAes::AesDecryptBufferEx(const void *chInput,int nInLen,int *nOutLen,const char *key)
{
	if (nOutLen) *nOutLen=0;
	BYTE keyBytes[16];
	//拷贝密码串
	if (strlen(key)>=16)
		memcpy(keyBytes,key,sizeof(keyBytes));
	else
	{
		memset(keyBytes,0,sizeof(keyBytes));
		strcpy((char *)keyBytes,key);
	}
	//初始化
	Init(keyBytes);
	if (nInLen%16!=0)
	{
		strcpy(ErrorBuffer,"密文长度错误！");
		return NULL;
	}
	BYTE tmpbuf[16];
	InvCipher((const BYTE *)chInput,tmpbuf);
	if (strncmp((char *)tmpbuf+sizeof(long),"THAES",5)!=0)
	{
		strcpy(ErrorBuffer,"密码错误！");
		return NULL;
	}
	long size;
	memcpy(&size,tmpbuf,sizeof(long));
	char *buffer=(char *)malloc(nInLen);
	if (!buffer)
	{
		strcpy(ErrorBuffer,"申请内存空间错误！");
		return NULL;
	}
	for(int i=0;i<nInLen/16;i++)
		InvCipher((BYTE *)chInput+(i+1)*16,(BYTE *)buffer+i*16);
	if (nOutLen) *nOutLen=size;
	ErrorBuffer[0]='\0';
	return buffer;
}

char *THAsciiAes::AesGetLastError()
{
	return strdup(ErrorBuffer);
}

void THAsciiAes::AesFree(void *buf)
{
	if (buf) free(buf);
}

void THAsciiAes::Init(BYTE keyBytes[16])
{
	ErrorBuffer[0]='\0';
	memcpy(key,keyBytes,16);
	KeyExpansion();  // expand the seed key into a key schedule and store in w
	GeneMixCol();
}

void THAsciiAes::Cipher(const BYTE input[16],BYTE output[16])  // encipher 16-bit input
{
	for(int i=0;i<16;++i)
		State[i%4][i/4]=input[i];

	AddRoundKey(0);
	for(int round=1;round<=9;++round)// main round loop
	{
		SubBytes(); 
		ShiftRows();  
		MixColumns(); 
		AddRoundKey(round);
	}

	SubBytes();
	ShiftRows();
	AddRoundKey(10);
            
	for(i=0;i<16;i++)
		output[i]=State[i%4][i/4];
}

void THAsciiAes::InvCipher(const BYTE input[16],BYTE output[16])  // decipher 16-bit input
{
	for(int i=0;i<16;++i)
		State[i%4][i/4]=input[i];

	AddRoundKey(10);

	for(int round=9;round>=1;--round)
	{
		InvShiftRows();
		InvSubBytes();
		AddRoundKey(round);
		InvMixColumns();
	}

	InvShiftRows();
	InvSubBytes();
	AddRoundKey(0);

	for(i=0;i<16;i++)
		output[i]=State[i%4][i/4];
}

void THAsciiAes::AddRoundKey(int round)
{
    for(int r=0;r<4;r++)
		for(int c=0;c<4;c++)
			State[r][c]=(BYTE)((int)State[r][c]^(int)w[(round*4)+c][r]);
}

void THAsciiAes::SubBytes()
{
	for(int r=0;r<4;r++)
		for(int c=0;c<4;c++)
			State[r][c]=Sbox[(State[r][c]>>4)][State[r][c]&0x0f];
}

void THAsciiAes::ShiftRows()
{
	BYTE temp[4][4];
	int r,c;
	for(r=0;r<4;r++)  // copy State into temp[]
        for (c=0;c<4;c++)
			temp[r][c]=State[r][c];
	for(r=0;r<4;r++)  // shift temp into State
		for (c=0;c<4;c++)
			State[r][c]=temp[r][(c+r)%4];
}

void THAsciiAes::MixColumns()
{
	BYTE temp[4][4];
	for(int r=0;r<4;r++)  // copy State into temp[]
        for (int c=0;c<4;c++)
			temp[r][c]=State[r][c];

	for(int c=0;c<4;c++)
	{
		State[0][c]=(BYTE)(MixColData[0][temp[0][c]]^MixColData[1][temp[1][c]]^
					(int)temp[2][c]^(int)temp[3][c]);
		State[1][c]=(BYTE)((int)temp[0][c]^MixColData[0][temp[1][c]]^
					MixColData[1][temp[2][c]]^(int)temp[3][c]);
		State[2][c]=(BYTE)((int)temp[0][c]^(int)temp[1][c]^
					MixColData[0][temp[2][c]]^MixColData[1][temp[3][c]]);
		State[3][c]=(BYTE)(MixColData[1][temp[0][c]]^(int)temp[1][c]^
					(int)temp[2][c]^MixColData[0][temp[3][c]]);
	}
}

void THAsciiAes::InvSubBytes()
{
	for(int r=0;r<4;r++)
		for(int c=0;c<4;c++)
			State[r][c]=iSbox[(State[r][c]>>4)][State[r][c]&0x0f];
}

void THAsciiAes::InvShiftRows()
{
	BYTE temp[4][4];
	int r,c;
	for(r=0;r<4;r++)  // copy State into temp[]
        for(c=0;c<4;c++)
			temp[r][c]=State[r][c];
	for(r=0;r<4;r++)  // shift temp into State
        for(c=0;c<4;c++)
			State[r][(c+r)%4]=temp[r][c];
}

void THAsciiAes::InvMixColumns()
{
	BYTE temp[4][4];
	for(int r=0;r<4;r++)  // copy State into temp[]
        for (int c=0;c<4;c++)
			temp[r][c]=State[r][c];
	
	for(int c=0;c<4;c++)
	{
        State[0][c]=(BYTE)(MixColData[5][temp[0][c]]^MixColData[3][temp[1][c]]^
					MixColData[4][temp[2][c]]^MixColData[2][temp[3][c]]);
        State[1][c]=(BYTE)(MixColData[2][temp[0][c]]^MixColData[5][temp[1][c]]^
					MixColData[3][temp[2][c]]^MixColData[4][temp[3][c]]);
        State[2][c]=(BYTE)(MixColData[4][temp[0][c]]^MixColData[2][temp[1][c]]^
					MixColData[5][temp[2][c]]^MixColData[3][temp[3][c]]);
        State[3][c]=(BYTE)(MixColData[3][temp[0][c]]^MixColData[4][temp[1][c]]^
					MixColData[2][temp[2][c]]^MixColData[5][temp[3][c]]);
	}
}

void THAsciiAes::GeneMixCol()
{
	for(int i=0;i<256;i++)
	{
		MixColData[0][i]=multby02(i);
		MixColData[1][i]=multby03(i);
		MixColData[2][i]=multby09(i);
		MixColData[3][i]=multby0b(i);
		MixColData[4][i]=multby0d(i);
		MixColData[5][i]=multby0e(i);
	}
}

inline BYTE THAsciiAes::multby02(BYTE b)
{
    if (b<0x80)
	    return b<<1;
    else
	    return (BYTE)((int)(b<<1)^(int)(0x1b));
}

inline BYTE THAsciiAes::multby03(BYTE b)
{
    return (BYTE)((int)multby02(b)^(int)b);
}

BYTE THAsciiAes::multby09(BYTE b)
{
    return (BYTE)((int)multby02(multby02(multby02(b)))^(int)b);
}

BYTE THAsciiAes::multby0b(BYTE b)
{
	return (BYTE)((int)multby02(multby02(multby02(b)))^(int)multby02(b)^(int)b);
}

BYTE THAsciiAes::multby0d(BYTE b)
{
	return (BYTE)((int)multby02(multby02(multby02(b)))^(int)multby02(multby02(b))^(int)(b));
}

BYTE THAsciiAes::multby0e(BYTE b)
{
	return (BYTE)((int)multby02(multby02(multby02(b)))^(int)multby02(multby02(b))^(int)multby02(b));
}

void THAsciiAes::KeyExpansion()
{
	BYTE temp[4];
	for(int row=0;row<4;++row)
	{
		w[row][0]=key[4*row];
		w[row][1]=key[4*row+1];
		w[row][2]=key[4*row+2];
		w[row][3]=key[4*row+3];
	}
	for(row=4;row<44;++row)
	{
		temp[0]=w[row-1][0];
		temp[1]=w[row-1][1];
		temp[2]=w[row-1][2];
		temp[3]=w[row-1][3];

		if(row%4==0)
        {
			RotWord(temp);
			SubWord(temp);
			temp[0]=(BYTE)((int)temp[0]^(int)Rcon[row/4][0]);
			temp[1]=(BYTE)((int)temp[1]^(int)Rcon[row/4][1]);
			temp[2]=(BYTE)((int)temp[2]^(int)Rcon[row/4][2]);
			temp[3]=(BYTE)((int)temp[3]^(int)Rcon[row/4][3]);
        }
        
        // w[row] = w[row-Nk] xor temp
        w[row][0]=(BYTE)((int)w[row-4][0]^(int)temp[0]);
        w[row][1]=(BYTE)((int)w[row-4][1]^(int)temp[1]);
        w[row][2]=(BYTE)((int)w[row-4][2]^(int)temp[2]);
        w[row][3]=(BYTE)((int)w[row-4][3]^(int)temp[3]);
	}
}

void THAsciiAes::SubWord(BYTE word[4])
{
	//is it same as the SubBytes?
	BYTE result[4];
	result[0]=Sbox[word[0]>>4][word[0]&0x0f];
	result[1]=Sbox[word[1]>>4][word[1]&0x0f];
	result[2]=Sbox[word[2]>>4][word[2]&0x0f];
	result[3]=Sbox[word[3]>>4][word[3]&0x0f];
	memcpy(word,result,4);
}

void THAsciiAes::RotWord(BYTE word[4])
{
	BYTE result[4];
	result[0]=word[1];
	result[1]=word[2];
	result[2]=word[3];
	result[3]=word[0];
	memcpy(word,result,4);
}

void THAsciiAes::Dump()
{
	char DisBuffer[10240],*buffer;
	DisBuffer[0]='\0';
	buffer=DumpTwoByTwo(&key[0],1,16);
	strcat(DisBuffer,buffer);
	free(buffer);
	buffer=DumpTwoByTwo(&Sbox[0][0],16,16);
	strcat(DisBuffer,buffer);
	free(buffer);
	buffer=DumpTwoByTwo(&w[0][0],44,4);
	strcat(DisBuffer,buffer);
	free(buffer);
	buffer=DumpTwoByTwo(&State[0][0],4,4);
	strcat(DisBuffer,buffer);
	free(buffer);
}

char *THAsciiAes::DumpTwoByTwo(const BYTE *a,int tr,int tc)
{
	char Buffer[1024],Tmp[10];
	Buffer[0]='\0';
	for(int r=0;r<tr;r++)
	{
        for(int c=0;c<tc;c++)
		{
			sprintf(Tmp,"%2x ",a[r*tc+c]);
			strcat(Buffer,Tmp);
		}
		strcat(Buffer,"\n");
	}
	return strdup(Buffer);
}

BYTE THAsciiAes::GetHex(const char *ch)
{
	BYTE tmp=0;
	if (*ch>='0'&&*ch<='9') tmp=(*ch-'0')*16;
	else if (*ch>='A'&&*ch<='F') tmp=(*ch-'A'+10)*16;
	else if (*ch>='a'&&*ch<='f') tmp=(*ch-'a'+10)*16;
	ch++;
	if (*ch>='0'&&*ch<='9') tmp+=*ch-'0';
	else if (*ch>='A'&&*ch<='F') tmp+=*ch-'A'+10;
	else if (*ch>='a'&&*ch<='f') tmp+=*ch-'a'+10;
	return tmp;
}
