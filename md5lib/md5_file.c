#include <stdio.h>
#include <stdlib.h>
#include "md5.h"
/*
created by powerplane, for checking the md5 sum of a file
*/

unsigned char *MD5_file(const char* fullpath,unsigned char *md)
{
	MD5_CTX c;
	static unsigned char m_buf[MD5_DIGEST_LENGTH];
	char fbuf[4096];
	FILE* f =NULL;
	size_t nRead = 0;
	
	if(md == NULL)
		md = m_buf;

	if (fullpath == NULL) return NULL;
	
	f = fopen(fullpath,"rb");

	if(!f){
		md = NULL;
		goto END;
	}

	MD5_Init(&c);

	while(nRead = fread(fbuf,sizeof(char), sizeof(fbuf), f)){
		MD5_Update( &c, fbuf, (unsigned long)nRead );
	}

	if(ferror(f) != 0 ){ //fread error happens!
		md = NULL;
		goto END;
	}

	MD5_Final(md,&c);

END:
	if (f) fclose(f);
	return md;
}


unsigned char *MD5_smartfile(const char* fullpath,unsigned char *md)
{
	MD5_CTX c;
	static unsigned char m_buf[MD5_DIGEST_LENGTH];
	char fbuf[4096];
	FILE* f =NULL;
	size_t nRead = 0;
	size_t nSum = 0;
	
	if(md == NULL)
		md = m_buf;

	if (fullpath == NULL) return NULL;
	
	f = fopen(fullpath,"rb");

	if(!f){
		md = NULL;
		goto END;
	}

	MD5_Init(&c);

	while(nRead = fread(fbuf,sizeof(char), sizeof(fbuf), f)){
		MD5_Update( &c, fbuf, (unsigned long)nRead );
		nSum += nRead;
		if (fseek(f,1*1024*1024,SEEK_CUR) == -1) break;
	}

	if (nSum >= 4096 && fseek(f,-sizeof(fbuf),SEEK_END) != -1){
		if (nRead = fread(fbuf,sizeof(char), sizeof(fbuf), f))
			MD5_Update( &c, fbuf, (unsigned long)nRead );
	}

	if(ferror(f) != 0 ){ //fread error happens!
		md = NULL;
		goto END;
	}

	MD5_Final(md,&c);

END:
	if (f) fclose(f);
	return md;
}
