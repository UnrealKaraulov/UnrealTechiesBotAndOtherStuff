#include <Windows.h>
#include <stdio.h>
#include <sstream>
#include <MinHook.h>
#include <iostream>    
#include <iomanip>

#pragma comment(lib,"libMinHook.x86.lib")


void WriteLogFile( const char* szString )
{
	FILE* pFile;
	fopen_s( &pFile , "logFile.txt" , "a" );
	if ( pFile )
	{
		fprintf_s( pFile , "%s\n" , szString );
		fclose( pFile );
	}
}


typedef int( __stdcall * sub_6F339C60 )( int a1 , int a2 , unsigned int a3 , unsigned int a4 );
typedef int( __stdcall * sub_6F339CC0 )( int a1 , int a2 , float a3 , float a4 , int a5 , int a6 );
typedef int( __stdcall * sub_6F339D50 )( int a1 , int a2 , int a3 , unsigned int a4 , unsigned int a5 );
typedef int( __stdcall * sub_6F339DD0 )( int a1 , int a2 , float a3 , float a4 , int a5 , int a6 , int a7 );
typedef int( __stdcall * sub_6F339E60 )( int a1 , int a2 , float a3 , float a4 , int a5 , int a6 , int a7 , int a8 );
typedef int( __stdcall * sub_6F339F00 )( int a1 , int a2 , int a3 , unsigned int a4 , unsigned int a5 );
typedef int( __stdcall * sub_6F339F80 )( int a1 , int a2 , float a3 , float a4 , int a5 , int a6 , int a7 );
typedef int( __stdcall * sub_6F33A010 )( int a1 , int a2 , float a3 , float a4 , int a5 , int a6 , int a7 , int a8 );


typedef int( __fastcall * sub_6F24CDF0 )( void *a1, void *a2, int a3, int a4, int a5 );

typedef int *( __fastcall * sub_6F203D60 )( void *a1, int a0, int a2, signed int a3, int a4 );
sub_6F203D60 sub_6F203D60org;
sub_6F203D60 sub_6F203D60ptr;


int * __fastcall sub_6F203D60my( void *a1, int a0, int a2, signed int a3, int a4 )
{

	int * retval =  sub_6F203D60ptr( a1, a0, a2, a3, a4 );

		std::stringstream ss;
	ss << "sub_6F203D60my:" << " (a1):" << std::hex << a1 << " (a2):" << a2 << " (a3):" << std::hex << a3 << " (a4):" << std::hex << a4 << " (a0):" << std::hex << a0 << " (return):";

	ss << std::hex << retval << std::endl;
	WriteLogFile( ss.str( ).c_str( ) );

	return retval;
}

typedef int (__fastcall * sub_6F4815A0)( int a1, int a0, int a2, int a3, int a4 );
sub_6F4815A0 sub_6F4815A0org;
sub_6F4815A0 sub_6F4815A0ptr;

int __fastcall sub_6F4815A0my( int a1, int a0, int a2, int a3, int a4 )
{
	int retvalue = sub_6F4815A0ptr( a1, a0, a2, a3, a4 );


	std::stringstream ss;
	ss << "sub_6F4815A0my:" << " (a1):" << std::hex << a1 << " (a2):" << a2 << " (a3):" << std::hex << a3 << " (a4):" << std::hex << a4 << " (a0):" << std::hex << a0 << " (return):";

	ss << std::hex << retvalue << std::endl;
	WriteLogFile( ss.str( ).c_str( ) );


	return retvalue;
}

sub_6F24CDF0 sub_6F24CDF0org;
sub_6F24CDF0 sub_6F24CDF0ptr;

int __fastcall sub_6F24CDF0my( void *a1, void *a2, int a3, int a4, int a5 )
{
	int retval = sub_6F24CDF0ptr( a1, a2, a3, a4, a5 );


	std::stringstream ss;
	ss << "sub_6F24CDF0my:" << " (a1):" << std::hex << a1 << " (a2):" << a2 << " (a3):" << std::hex << a3 << " (a4):" << std::hex << a4 <<  " (a5):" << std::hex << a5 << " (return):";

	ss << std::hex << retval << std::endl;
	WriteLogFile( ss.str( ).c_str( ) );

	return retval;
}



sub_6F339C60 sub_6F339C60org;
sub_6F339C60 sub_6F339C60ptr;

sub_6F339CC0 sub_6F339CC0org;
sub_6F339CC0 sub_6F339CC0ptr;

sub_6F339D50 sub_6F339D50org;
sub_6F339D50 sub_6F339D50ptr;

sub_6F339DD0 sub_6F339DD0org;
sub_6F339DD0 sub_6F339DD0ptr;

sub_6F339E60 sub_6F339E60org;
sub_6F339E60 sub_6F339E60ptr;

sub_6F339F00 sub_6F339F00org;
sub_6F339F00 sub_6F339F00ptr;

sub_6F339F80 sub_6F339F80org;
sub_6F339F80 sub_6F339F80ptr;

sub_6F33A010 sub_6F33A010org;
sub_6F33A010 sub_6F33A010ptr;


int __stdcall sub_6F339C60my( int a1 , int a2 , unsigned int a3 , unsigned int a4 )
{
	std::stringstream ss;
	ss << "sub_6F339C60my:" << " (a1):" << std::hex << a1 << " (a2):" << a2 << " (a3):" << std::hex << a3 << " (a4):" << std::hex << a4 << " (return):";

	int retvalue = sub_6F339C60ptr( a1 , a2 , a3 , a4 );
	ss << std::hex << retvalue << std::endl;
	WriteLogFile( ss.str( ).c_str( ) );
	return retvalue;
}
int __stdcall sub_6F339CC0my( int a1 , int a2 , float a3 , float a4 , int a5 , int a6 )
{
	std::stringstream ss;
	ss << "sub_6F339CC0my:" << " (a1):" << std::hex << a1 << " (a2):" << std::hex << a2 << " (a3):" << a3 << " (a4):" << a4 << " (a5):" << std::hex << a5 << " (a6):" << std::hex << a6 << " (return):";

	int retvalue = sub_6F339CC0ptr( a1 , a2 , a3 , a4 , a5 , a6 );
	ss << std::hex << retvalue << std::endl;
	WriteLogFile( ss.str( ).c_str( ) );
	return retvalue;
}
int __stdcall sub_6F339D50my( int a1 , int a2 , int a3 , unsigned int a4 , unsigned int a5 )
{
	std::stringstream ss;
	ss << "sub_6F339D50my:" << " (a1):" << std::hex << a1 << " (a2):" << std::hex << a2 << " (a3):" << std::hex << a3 << " (a4):" << std::hex << a4 << " (a5):" << std::hex << a5 << " (return):";

	int retvalue = sub_6F339D50ptr( a1 , a2 , a3 , a4 , a5 );
	ss << std::hex << retvalue << std::endl;
	WriteLogFile( ss.str( ).c_str( ) );
	return retvalue;
}
int __stdcall sub_6F339DD0my( int a1 , int a2 , float a3 , float a4 , int a5 , int a6 , int a7 )
{
	std::stringstream ss;
	ss << "sub_6F339DD0my:" << " (a1):" << std::hex << a1 << " (a2):" << std::hex << a2 << " (a3):" << a3 << " (a4):" << a4 << " (a5):" << std::hex << a5 << " (a6):" << std::hex << a6 << " (a7):" << std::hex << a7 << " (return):";

	int retvalue = sub_6F339DD0ptr( a1 , a2 , a3 , a4 , a5 , a6 , a7 );
	ss << std::hex << retvalue << std::endl;
	WriteLogFile( ss.str( ).c_str( ) );
	return retvalue;
}
int __stdcall sub_6F339E60my( int a1 , int a2 , float a3 , float a4 , int a5 , int a6 , int a7 , int a8 )
{
	std::stringstream ss;
	ss << "sub_6F339E60my:" << " (a1):" << std::hex << a1 << " (a2):" << std::hex << a2 << " (a3):" << a3 << " (a4):" << a4 << " (a5):" << std::hex << a5 << " (a6):" << std::hex << a6 << " (a7):" << std::hex << a7 << " (a8):" << std::hex << a8 << " (return):";

	int retvalue = sub_6F339E60ptr( a1 , a2 , a3 , a4 , a5 , a6 , a7 , a8 );
	ss << std::hex << retvalue << std::endl;
	WriteLogFile( ss.str( ).c_str( ) );
	return retvalue;
}
int __stdcall sub_6F339F00my( int a1 , int a2 , int a3 , unsigned int a4 , unsigned int a5 )
{
	std::stringstream ss;
	ss << "sub_6F339F00my:" << " (a1):" << std::hex << a1 << " (a2):" << std::hex << a2 << " (a3):" << std::hex << a3 << " (a4):" << std::hex << a4 << " (a5):" << std::hex << a5 << " (return):";

	int retvalue = sub_6F339F00ptr( a1 , a2 , a3 , a4 , a5 );
	ss << std::hex << retvalue << std::endl;
	WriteLogFile( ss.str( ).c_str( ) );
	return retvalue;
}
int __stdcall sub_6F339F80my( int a1 , int a2 , float a3 , float a4 , int a5 , int a6 , int a7 )
{
	std::stringstream ss;
	ss << "sub_6F339F80my:" << " (a1):" << std::hex << a1 << " (a2):" << std::hex << a2 << " (a3):" << a3 << " (a4):" << a4 << " (a5):" << std::hex << a5 << " (a6):" << std::hex << a6 << " (a7):" << std::hex << a7 << " (return):";

	int retvalue = sub_6F339F80ptr( a1 , a2 , a3 , a4 , a5 , a6 , a7 );
	ss << std::hex << retvalue << std::endl;
	WriteLogFile( ss.str( ).c_str( ) );
	return retvalue;
}
int __stdcall sub_6F33A010my( int a1 , int a2 , float a3 , float a4 , int a5 , int a6 , int a7 , int a8 )
{
	std::stringstream ss;
	ss << "sub_6F33A010my:" << " (a1):" << std::hex << a1 << " (a2):" << std::hex << a2 << " (a3):" << a3 << " (a4):" << a4 << " (a5):" << std::hex << a5 << " (a6):" << std::hex << a6 << " (a7):" << std::hex << a7 << " (a8):" << std::hex << a8 << " (return):";

	int retvalue = sub_6F33A010ptr( a1 , a2 , a3 , a4 , a5 , a6 , a7 , a8 );
	ss << std::hex << retvalue << std::endl;
	WriteLogFile( ss.str( ).c_str( ) );
	return retvalue;
}
struct Packet
{
	DWORD PacketClassPtr;	//+00, some unknown, but needed, Class Pointer
	BYTE* PacketData;		//+04
	DWORD _1;				//+08, zero
	DWORD _2;				//+0C, ??
	DWORD Size;				//+10, size of PacketData
	DWORD _3;				//+14, 0xFFFFFFFF
};


typedef int( __fastcall * GAME_SendPacket_p ) ( Packet* packet , int zero );
GAME_SendPacket_p GAME_SendPacket;
GAME_SendPacket_p GAME_SendPacketptr;




int __fastcall GAME_SendPacket_Intercept( Packet* packet , int zero )
{
	std::stringstream ss;
	char bytes[ ] = { 0x01 , 0x00 };
	ss << "Packet [" << packet->PacketClassPtr << ", " << packet->_1 << ", " << packet->_2 << ", " << packet->_3 << "]";
	ss << std::hex << std::setfill( '0' );
	for ( int i = 0; i < packet->Size; i++ )
	{
		ss << std::setw( 2 ) << static_cast<unsigned>( packet->PacketData[ i ] );
	}
	ss << std::endl;
	WriteLogFile( ss.str( ).c_str( ) );
	
	return GAME_SendPacketptr( packet , zero );
}

bool PatchMemory( void* addr , void* data , DWORD size )
{
	DWORD oldProtect;
	if ( !VirtualProtect( addr , size , PAGE_READWRITE , &oldProtect ) )
		return false;

	memcpy( addr , data , size );
	return VirtualProtect( addr , size , oldProtect , &oldProtect ) ? true : false;
}
bool InterceptCall( DWORD instAddr , DWORD func )
{
	// asm code uses offsets from next position instead of direct address,
	// 5 is current CALL instruction code length (instID + funcOffset).

	// ensure we have CALL instruction here
	DWORD funcOffset = func - ( instAddr + 5 );
	return PatchMemory( ( void* ) ( instAddr + 1 ) , &funcOffset , 4 );
	return false;
}
bool InterceptCallSafe( DWORD instAddr , DWORD oldFunc , DWORD func )
{
	// ensure we have old func offset here, to prevent wrong patch
	DWORD oldFuncOffset = oldFunc - ( instAddr + 5 );
	if ( *( ( DWORD* ) ( instAddr + 1 ) ) == oldFuncOffset )
		return InterceptCall( instAddr , func );

	return false;
}

DWORD GAME_SendPacket_I;

BOOL WINAPI DllMain( HINSTANCE hDLL , UINT reason , LPVOID reserved )
{
	if ( reason == DLL_PROCESS_ATTACH )
	{
		MH_Initialize( );

		sub_6F339C60org = ( sub_6F339C60 ) ( ( ( int ) GetModuleHandle( "Game.dll" ) ) + 0x339C60 );
		MH_CreateHook( sub_6F339C60org , &sub_6F339C60my , reinterpret_cast< void** >( &sub_6F339C60ptr ) );

		sub_6F339CC0org = ( sub_6F339CC0 ) ( ( ( int ) GetModuleHandle( "Game.dll" ) ) + 0x339CC0 );
		MH_CreateHook( sub_6F339CC0org , &sub_6F339CC0my , reinterpret_cast< void** >( &sub_6F339CC0ptr ) );

		sub_6F339D50org = ( sub_6F339D50 ) ( ( ( int ) GetModuleHandle( "Game.dll" ) ) + 0x339D50 );
		MH_CreateHook( sub_6F339D50org , &sub_6F339D50my , reinterpret_cast< void** >( &sub_6F339D50ptr ) );

		sub_6F339DD0org = ( sub_6F339DD0 ) ( ( ( int ) GetModuleHandle( "Game.dll" ) ) + 0x339DD0 );
		MH_CreateHook( sub_6F339DD0org , &sub_6F339DD0my , reinterpret_cast< void** >( &sub_6F339DD0ptr ) );

		sub_6F339E60org = ( sub_6F339E60 ) ( ( ( int ) GetModuleHandle( "Game.dll" ) ) + 0x339E60 );
		MH_CreateHook( sub_6F339E60org , &sub_6F339E60my , reinterpret_cast< void** >( &sub_6F339E60ptr ) );

		sub_6F339F00org = ( sub_6F339F00 ) ( ( ( int ) GetModuleHandle( "Game.dll" ) ) + 0x339F00 );
		MH_CreateHook( sub_6F339F00org , &sub_6F339F00my , reinterpret_cast< void** >( &sub_6F339F00ptr ) );

		sub_6F339F80org = ( sub_6F339F80 ) ( ( ( int ) GetModuleHandle( "Game.dll" ) ) + 0x339F80 );
		MH_CreateHook( sub_6F339F80org , &sub_6F339F80my , reinterpret_cast< void** >( &sub_6F339F80ptr ) );

		sub_6F33A010org = ( sub_6F33A010 ) ( ( ( int ) GetModuleHandle( "Game.dll" ) ) + 0x33A010 );
		MH_CreateHook( sub_6F33A010org, &sub_6F33A010my, reinterpret_cast< void** >( &sub_6F33A010ptr ) );
		
		
		sub_6F24CDF0org = ( sub_6F24CDF0 ) ( ( ( int ) GetModuleHandle( "Game.dll" ) ) + 0x24CDF0 );
		MH_CreateHook( sub_6F24CDF0org, &sub_6F24CDF0my, reinterpret_cast< void** >( &sub_6F24CDF0ptr ) );
	
		sub_6F4815A0org = ( sub_6F4815A0 ) ( ( ( int ) GetModuleHandle( "Game.dll" ) ) + 0x4815A0 );
		MH_CreateHook( sub_6F4815A0org, &sub_6F4815A0my, reinterpret_cast< void** >( &sub_6F4815A0ptr ) );

		sub_6F203D60org = ( sub_6F203D60 ) ( ( ( int ) GetModuleHandle( "Game.dll" ) ) + 0x203D60 );
		MH_CreateHook( sub_6F203D60org, &sub_6F203D60my, reinterpret_cast< void** >( &sub_6F203D60ptr ) );

		

	/*	GAME_SendPacket = ( GAME_SendPacket_p ) ( ( ( int ) GetModuleHandle( "Game.dll" ) ) + 0x54D970 );

		DWORD oldProtect;
		VirtualProtect( (void*) GAME_SendPacket , 50 , PAGE_EXECUTE_READWRITE , &oldProtect );
		MH_CreateHook( GAME_SendPacket , &GAME_SendPacket_Intercept , reinterpret_cast< void** >( &GAME_SendPacketptr ) );
		VirtualProtect( ( void* ) GAME_SendPacket , 50 , oldProtect , 0 );
		*/
		/*

		sub_6F339C60
		sub_6F339CC0
		sub_6F339D50
		sub_6F339DD0
		sub_6F339E60
		sub_6F339F00
		sub_6F339F80
		sub_6F33A010
		*/

		MH_EnableHook( sub_6F339C60org );
		MH_EnableHook( sub_6F339CC0org );
		MH_EnableHook( sub_6F339D50org );
		MH_EnableHook( sub_6F339DD0org );
		MH_EnableHook( sub_6F339E60org );
		MH_EnableHook( sub_6F339F00org );
		MH_EnableHook( sub_6F339F80org );
		MH_EnableHook( sub_6F33A010org );
	//	MH_EnableHook( sub_6F24CDF0org );
	//	MH_EnableHook( sub_6F203D60org );
	}
	else if ( reason == DLL_PROCESS_DETACH )
	{
		MH_DisableHook( sub_6F339C60org );
		MH_DisableHook( sub_6F339CC0org );
		MH_DisableHook( sub_6F339D50org );
		MH_DisableHook( sub_6F339DD0org );
		MH_DisableHook( sub_6F339E60org );
		MH_DisableHook( sub_6F339F00org );
		MH_DisableHook( sub_6F339F80org );
		MH_DisableHook( sub_6F33A010org );
		MH_Uninitialize( );
	}
	return TRUE;
}