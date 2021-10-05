#pragma once

typedef unsigned char BYTE;

class THAsciiAes
{
public:
	char *AesEncryptBuffer(const void *chInput,int nInLen,int *nOutLen,const char *key);
	char *AesDecryptBuffer(const void *chInput,int nInLen,int *nOutLen,const char *key);
	//Ex为非标准实现，加入了保存大小，使解密后数据能知道准确大小
	char *AesEncryptBufferEx(const void *chInput,int nInLen,int *nOutLen,const char *key);
	char *AesDecryptBufferEx(const void *chInput,int nInLen,int *nOutLen,const char *key);
	char *AesEncryptTextEx(const char *chInput,const char *key);
	char *AesDecryptTextEx(const char *chInput,const char *key);
	BOOL AesEncryptFileEx(const char *chInputFile,const char *chOutputFile,const char *key);
	BOOL AesDecryptFileEx(const char *chInputFile,const char *chOutputFile,const char *key);
	char *AesGetLastError();
	void AesFree(void *buf);
protected:
	void Cipher(const BYTE input[16],BYTE output[16]);
	void InvCipher(const BYTE input[16],BYTE output[16]);
	void Init(BYTE keyBytes[16]);
	void AddRoundKey(int round);
	void SubBytes();
	void ShiftRows();
	void MixColumns();
	void InvSubBytes();
	void InvShiftRows();
	void InvMixColumns();
	void KeyExpansion();
	void GeneMixCol();
	void SubWord(BYTE word[4]);
	void RotWord(BYTE word[4]);
	void Dump();
	BYTE GetHex(const char *ch);
	char* _AesFormat(const char *buf,int len);

	inline BYTE multby02(BYTE b);
	inline BYTE multby03(BYTE b);
	BYTE multby09(BYTE b);
	BYTE multby0b(BYTE b);
	BYTE multby0d(BYTE b);
	BYTE multby0e(BYTE b);
	char *DumpTwoByTwo(const BYTE *a,int tr,int tc);

    BYTE key[16];     // the seed key. size will be 4 * keySize from ctor.
	BYTE w[44][4];      // key schedule array. 
    BYTE State[4][4];  // State matrix
	int MixColData[6][256];
	char ErrorBuffer[4096]; // Last Error Content
};