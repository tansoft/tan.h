/**************************************************************************************************
 *
 * reader header file
 * name : reader.h
 *
 *
 **************************************************************************************************/

#ifndef IC_CARD_LX900_H
#define IC_CARD_LX900_H

enum myCardType
{
	CT_UNKNOWN, 

	CT_SLE4432, CT_SLE4442, 

	CT_SLE4418, CT_SLE4428, 

	CT_SLE4406, CT_SLE4436,

	CT_AT88SC101, CT_AT88SC102,

	CT_SLE44C42S,
	CT_SLE44C80,

	CT_X76F640,

	CT_AT24C01A,
	CT_AT24C02,
	CT_AT24C04,
	CT_AT24C08,
	CT_AT24C16,
	CT_AT24C32,
	CT_AT24C64,
	CT_AT24C128,
	CT_AT24C256,
	CT_AT24C512,

	CT_AT88SC1604,
	CT_AT88SC1608,
	CT_AT88SC153,
	
	CT_AT45D041,

	CT_NO_CARD
};

enum myErrorCode
{
	ERR_SUCCESS = 0,

	ERR_NO_CARD,
	ERR_TIMEOUT,
	ERR_COUNTER,
	ERR_PSC1,
	ERR_PSC2,

	ERR_LENGTH,

	ERR_NOT_ENOUGH_MEMORY,
	ERR_CREATE_PORT_FILE,
	ERR_GET_COMM_STATE,
	ERR_SET_COMM_STATE,
	ERR_SET_COMM_MASK,
	ERR_SET_COMM_TIMEOUTS,
	ERR_CREATE_THREAD,
	ERR_NULL_PORT_HANDLE,

	RET_COMP_EQUAL = 200,
	RET_COMP_UNEQUAL
};


#ifdef _cplusplus
extern "C" {
#endif

//
//
int WINAPI OpenPort( char *PortName, int *PortHandle );
int WINAPI SelectPort( int PortHandle );
int WINAPI ClosePort( );

//
//
int WINAPI PowerOn( );
int WINAPI PowerOff( );

//
//
int WINAPI CardPresent( );
int WINAPI CardChanged( );
int WINAPI GetCardType( char *ATRBuffer );
char * WINAPI GetCardName( int CardType );
int WINAPI ReaderVersion( );

//
//
int WINAPI IssueCommandAndWait( char Command, char *Data );
int WINAPI IssueCommandNoWait( char Command, char *Data );
void WINAPI PackBufferBinary( char *Out, char In1, char *In2 );


//
//	SLE4428
//
int WINAPI Read_4428_With_PB( int StartPos, int NOB, char *Bfr, char *PB_Bfr );
int WINAPI Read_4428_NO_PB( int StartPos, int NOB, char *Bfr );
int WINAPI Write_4428( int StartPos, char DestByte, char PBSetFlag ); // PBSetFlag == 1 then set PB
int WINAPI Verify_4428_PSC( char PSC1, char PSC2 );
int WINAPI Read_4428_SM( char *SM_Bfr, char *SM_PB_Bfr ); // SM_Bfr, SM_PB_Bfr : char [ 3 ]

//
//	SLE4442 
//
int WINAPI Read_4442_With_PB( int StartPos, int NOB, char *Bfr, char *PB_Bfr );
int WINAPI Read_4442_NO_PB( int StartPos, int NOB, char *Bfr );
int WINAPI Read_4442_PB( char *PB_Bfr ); // bit buffer
int WINAPI Write_4442_PB( int Address ); // bit address
int WINAPI Write_4442( int StartPos, char DestByte, char PBSetFlag ); // PBSetFlag == 1 then set PB
int	WINAPI Write_4442_Array( int StartPos, int NOB, char *Bfr, char *PBSetFlag );
int WINAPI Verify_4442_PSC( char PSC1, char PSC2, char PSC3 );
int WINAPI Read_4442_SM( char *SM_Bfr ); // SM_Bfr: char [ 4 ]
int WINAPI Write_4442_SM( int SMAddress, char SMByte );

//
//	AT88SC102
//
int WINAPI Read_102_Bit( int StartBitAddress, int NOBit, char *Data );
int WINAPI Write_102_Bit( int StartBitAddress, int NOBit, char *Data );
int WINAPI Read_102_Byte( int ByteAddress, char *Data );
int WINAPI Write_102_Byte( int ByteAddress, char Data );
int WINAPI Read_102_Byte_Array( int ByteAddress, int NOB, char *Data );
int WINAPI Write_102_Byte_Array( int ByteAddress, int NOB, char *Data );
int WINAPI Read_102_Word( int WordAddress, char *Data );
int WINAPI Write_102_Word( int WordAddress, char *Data );
int WINAPI Erase_102_Word( int WordAddress );
int WINAPI Erase_102_Word_Array( int WordAddress, int WordCount );
int WINAPI Verify_102_SC( char *SCArray );
int WINAPI Verify_102_EZ1( char *EZ1Array );
int WINAPI Verify_102_EZ2( char *EZ2Array );
int WINAPI Erase_102_Global( );
int WINAPI Erase_102_AZ1( char *EZArray );
int WINAPI Erase_102_AZ2( char *EZArray );
int WINAPI Fuse_High_102( );
int WINAPI Fuse_Low_102( );

//
//	SLE4406
//
int WINAPI Read_4406( char *Data );

//
//	SLE4436
// 
int WINAPI Read_4436( char *Data );

//
//	X76F640
//
int WINAPI Read_X76F640( char *Data );

//
//	AT24C01A/02
//
int WINAPI Read_AT24C01A( int Address, int NOB, char *Data );
int WINAPI Write_AT24C01A_Byte( int Address, char Data );
int WINAPI Write_AT24C01A_Page( int Address, int NOB, char *Data );
//	AT24C04/08/16
int WINAPI Read_AT24C16( int Address, int NOB, char *Data );
int WINAPI Write_AT24C16_Byte( int Address, char Data );
int WINAPI Write_AT24C16_Page( int Address, int NOB, char *Data );
//	AT24C32/64
int WINAPI Read_AT24C64( int Address, int NOB, char *Data );
int WINAPI Write_AT24C64_Byte( int Address, char Data );
int WINAPI Write_AT24C64_Page( int Address, int NOB, char *Data );
//	AT24C128/256
int WINAPI Read_AT24C256( int Address, int NOB, char *Data );
int WINAPI Write_AT24C256_Byte( int Address, char Data );
int WINAPI Write_AT24C256_Page( int Address, int NOB, char *Data );

//
//	AT88SC1608
//
int WINAPI Select_1608_User_Zone( int ZoneAddr ); // ZoneAddr = 0 - 8
int WINAPI Read_1608_User_Zone( int ByteAddr, int NOB, char *Data );
int WINAPI Write_1608_User_Zone( int ByteAddr, int NOB, char *Data );
int WINAPI Read_1608_Configuration( int ByteAddr, int NOB, char *Data );
int WINAPI Write_1608_Configuration( int ByteAddr, int NOB, char *Data );
int WINAPI Read_1608_Fuses( char *Fuses );
int WINAPI Write_1608_Fuses( );
int WINAPI Verify_1608_Password( int ReadWrite, int SetNumber, char Password0, char Password1, char Password2 );
int WINAPI Init_1608_Authentication( char *Q0 );
int WINAPI Verify_1608_Authentication( char *Q1 );
int WINAPI Authenticate_1608( char *Q0, char *GC );

int WINAPI Compute_Challenge( char *Q0, char *GC, char *Ci, char *Q1, char *Q2 );

//
//	AT88SC153
//
int WINAPI Write_153( int ZoneNumber, int Address, int NOB, char *Data );
int WINAPI Read_153( int ZoneNumber, int Address, int NOB, char *Data );
int WINAPI Verify_Password_153( int ReadWrite, int SetNumber, char Password0, char Password1, char Password2 );
int WINAPI InitializeAuthentication_153( char *Q );
int WINAPI VerifyAuthentication_153( char *CH );
int WINAPI ReadFuse_153( char *Fuse );
int WINAPI BlowFuse_153( int FuseNumber );

//
//	AT88SC1604
//
int WINAPI Read_1604_Bit( int StartBitAddress, int NOBit, char *Data );
int WINAPI Write_1604_Bit( int StartBitAddress, int NOBit, char *Data );
int WINAPI Read_1604_Byte( int ByteAddress, char *Data );
int WINAPI Write_1604_Byte( int ByteAddress, char Data );
int WINAPI Read_1604_Byte_Array( int ByteAddress, int NOByte, char *Data );
int WINAPI Write_1604_Byte_Array( int ByteAddress, int NOByte, char *Data );
int WINAPI Read_1604_Word( int WordAddress, char *Data );
int WINAPI Write_1604_Word( int WordAddress, char *Data );
int WINAPI Erase_1604_Byte( int ByteAddress );
int WINAPI Erase_1604_Byte_Array( int ByteAddress, int ByteCount );
int WINAPI Verify_1604_SC( char *SCArray );
int WINAPI Verify_1604_SC1( char *SCArray );
int WINAPI Verify_1604_SC2( char *SCArray );
int WINAPI Verify_1604_SC3( char *SCArray );
int WINAPI Verify_1604_SC4( char *SCArray );
int WINAPI Verify_1604_EZ1( char *EZArray );
int WINAPI Verify_1604_EZ2( char *EZArray );
int WINAPI Verify_1604_EZ3( char *EZArray );
int WINAPI Verify_1604_EZ4( char *EZArray );
int WINAPI Erase_1604_Global( );
int WINAPI Erase_1604_AZ1( char *EZArray );
int WINAPI Erase_1604_AZ2( char *EZArray );
int WINAPI Fuse_High_1604( );
int WINAPI Fuse_Low_1604( );
int WINAPI Fuse_CMZ_1604( );
int WINAPI Fuse_ISSUER_1604( );

//
//	AT45D021, AT45D041
//
int WINAPI Reset_AT45D041( );
int WINAPI Status_AT45D041( char *sByte );
int WINAPI Read_AT45D041( int PageNo, int Address, int NOB, char *Data );
int WINAPI Program_Main_AT45D041( int PageNo, int Address, int NOB, char *Data );
int WINAPI Write_AT45D041( int PageNo, int Address, int NOB, char *Data );
int WINAPI Read_Buffer_AT45D041( int BuffNo, int Address, int NOB, char *Data );
int WINAPI Write_Buffer_AT45D041( int BuffNo, int Address, int NOB, char *Data );
int WINAPI Transfer_AT45D041( int PageNo, int BuffNo );
int WINAPI Compare_AT45D041( int PageNo, int BuffNo );
int WINAPI WriteInErase_AT45D041( int BuffNo, int PageNo );
int WINAPI WriteNoErase_AT45D041( int BuffNo, int PageNo );
int WINAPI Rewrite_AT45D041( int BuffNo, int PageNo );

//
//
void WINAPI Des64( unsigned char *input, unsigned char *output, unsigned char *key, int Encrypt );

int WINAPI OpenReader( char *ComStr, HWND NotifyWindow, UINT NotifyMsg );
int WINAPI CloseReader( );

#ifdef _cplusplus
}
#endif

#endif	//	IC_CARD_LX900_H
