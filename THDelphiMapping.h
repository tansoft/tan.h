#pragma once

/**
* @brief Delphi函数映射，快速进行Delphi代码移植
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-07-13 新建头文件
*/

//base function mapping
#define Move(src,dist,size)		memcpy(dist,src,size)
#define FillChar(buf,size,ch)	memset(buf,ch,size)
template<typename TVAL> static inline void Inc(TVAL &val,int opt=1){val=val+opt;}
template<typename TVAL> static inline void Dec(TVAL &val,int opt=1){val=val-opt;}

//operator mapping
//#define :=				=
//#define $				0x
//#define @				&
#define and				&
#define not				~
#define or				|
#define xor				^
#define shr				>>
#define shl				<<
#define div				/
#define mod				%
#define Sizeof(obj)		sizeof(obj)
#define begin			{
#define	end				}

//base type mapping
typedef	long			longword;
typedef DWORD			DWord;
typedef DWORD *			PDWord;

//base keyword mapping
#define	procedure
#define	then

//support no argument list function
#pragma warning(disable:4551)
