

#define _WIN32_WINNT 0x0501 
#define WINVER 0x0501 
#define NTDDI_VERSION 0x05010000
//#define BOTDEBUG
#define WIN32_LEAN_AND_MEAN
#include <stdexcept>
#include <Windows.h>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <time.h>
#include <thread>  
// Game.dll address
int GameDll = 0;
int _W3XGlobalClass;


int UseWarnIsBadReadPtr = 1;
BOOL DebugActive = FALSE;

BOOL IsOkayPtr( void *ptr , unsigned int size = 4 )
{
	if ( UseWarnIsBadReadPtr == 1 )
	{
		BOOL returnvalue = FALSE;
		returnvalue = IsBadReadPtr( ptr , size ) == 0;
		if ( !returnvalue )
		{

#ifdef BOTDEBUG
			PrintDebugInfo( "ErrorErrorError1" );
#endif
		}
		return returnvalue;
	}
	else if ( UseWarnIsBadReadPtr == 2 )
	{
		MEMORY_BASIC_INFORMATION mbi;
		if ( VirtualQuery( ptr , &mbi , sizeof( MEMORY_BASIC_INFORMATION ) ) == 0 )
		{

#ifdef BOTDEBUG
			PrintDebugInfo( "ErrorErrorError2" );
#endif
			return FALSE;
		}

		if ( ( int ) ptr + size > ( int ) mbi.BaseAddress + mbi.RegionSize )
		{

#ifdef BOTDEBUG
			PrintDebugInfo( "ErrorErrorError2" );
#endif
			return FALSE;
		}


		if ( ( int ) ptr < ( int ) mbi.BaseAddress )
		{

#ifdef BOTDEBUG
			PrintDebugInfo( "ErrorErrorError2" );
#endif
			return FALSE;
		}


		if ( mbi.State != MEM_COMMIT )
		{

#ifdef BOTDEBUG
			PrintDebugInfo( "ErrorErrorError2" );
#endif
			return FALSE;
		}


		if ( mbi.Protect != PAGE_READWRITE &&  mbi.Protect != PAGE_WRITECOPY && mbi.Protect != PAGE_READONLY )
		{

#ifdef BOTDEBUG
			PrintDebugInfo( "ErrorErrorError2" );
#endif
			return FALSE;
		}

		return TRUE;
	}
	else
		return TRUE;
}

void * GetGlobalPlayerData( )
{
	if ( *( int * ) ( 0xBE4238 + GameDll ) > 0 )
	{
		if ( IsOkayPtr( ( void* ) ( 0xBE4238 + GameDll ) ) )
			return ( void * ) *( int* ) ( 0xBE4238 + GameDll );
		else
			return nullptr;
	}
	else
		return nullptr;
}

int GetPlayerByNumber( int number )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "GetPlayerByNumber" );
#endif
	void * arg1 = GetGlobalPlayerData( );
	int result = -1;
	if ( arg1 != nullptr && arg1 )
	{
		result = ( int ) arg1 + ( number * 4 ) + 0x58;

		if ( IsOkayPtr( ( void* ) result ) )
		{
			result = *( int* ) result;
		}
	}
	return result;
}

int GetLocalPlayerNumber( )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "LocalPlayerNumber" );
#endif
	void * gldata = GetGlobalPlayerData( );
	if ( gldata != nullptr && gldata )
	{


		int playerslotaddr = ( int ) gldata + 0x28;
		if ( IsOkayPtr( ( void* ) playerslotaddr ) )
			return (  int ) *(  short * ) ( playerslotaddr );
		else
			return -2;
	}
	else
		return -2;
}


int GetLocalPlayer( )
{
	return GetPlayerByNumber( GetLocalPlayerNumber( ) );
}


BOOL WINAPI DllMain( HINSTANCE hDLL , UINT reason , LPVOID reserved )
{
	if ( reason == DLL_PROCESS_ATTACH )
	{
		GameDll = ( int ) GetModuleHandle( "Game.dll" );
		int localplayer = GetLocalPlayer( );
		int globaldata = ( int ) GetLocalPlayerNumber( );

		char dama[ 256 ];
		sprintf_s( dama , "%X,%X" , localplayer , globaldata );
		MessageBox( 0 , dama , dama , 0 );
		return FALSE;
	}
	else if ( reason == DLL_PROCESS_DETACH )
	{

	}
	return TRUE;
}
