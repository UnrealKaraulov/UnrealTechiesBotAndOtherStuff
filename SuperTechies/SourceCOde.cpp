#define DEBUG
#pragma region Headers
#define _WIN32_WINNT 0x0501 
#define WINVER 0x0501 
#define NTDDI_VERSION 0x05010000
//#define BOTDEBUG
#define WIN32_LEAN_AND_MEAN
#include <stdexcept>
#include <Windows.h>
#include <string>
#include <concurrent_vector.h>
#define safevector concurrency::concurrent_vector

#include <sstream>
#include <algorithm>
#include <time.h>
#include <thread>  
#include <TlHelp32.h>
#include <exception>
#define _USE_MATH_DEFINES
#include <math.h>
#pragma endregion

//#define BOTDEBUG
void PrintDebugInfo( const char * debuginfo );

#ifdef BOTDEBUG
#include <tlhelp32.h>
void DebugStr( const char* szDllPath );
#endif

DWORD __stdcall temptechiesthread( LPVOID );
// Game.dll address
int GameDll = 0;
int _W3XGlobalClass;

int UseWarnIsBadReadPtr = 1;
int DebugActive = 0;

int techiesaddr = 0;
safevector<int> unitstoselect;

time_t LatestTime = time( NULL );

int latestunit = 0;

int latestunitclass = 0;

DWORD latestvalue1 = 0;
DWORD latestvalue2 = 0;

char TechiesCrash[ 150 ];

int ExpertModeEnabled = 0;
int Enable3DPoint = 0;
int StealthMode = 0;

int CATCH( unsigned int code, struct _EXCEPTION_POINTERS *ep )
{
	return ExceptionContinueExecution;
}

int IsOkayPtr( void *ptr, unsigned int size = 4 )
{
	if ( UseWarnIsBadReadPtr == 1 )
	{
		int returnvalue = IsBadReadPtr( ptr, size ) == 0;
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
		if ( VirtualQuery( ptr, &mbi, sizeof( MEMORY_BASIC_INFORMATION ) ) == 0 )
		{

#ifdef BOTDEBUG
			PrintDebugInfo( "ErrorErrorError2" );
#endif
			return 0;
		}

		if ( ( int )ptr + size > ( int )mbi.BaseAddress + mbi.RegionSize )
		{

#ifdef BOTDEBUG
			PrintDebugInfo( "ErrorErrorError2" );
#endif
			return 0;
		}


		if ( ( int )ptr < ( int )mbi.BaseAddress )
		{

#ifdef BOTDEBUG
			PrintDebugInfo( "ErrorErrorError2" );
#endif
			return 0;
		}


		if ( mbi.State != MEM_COMMIT )
		{

#ifdef BOTDEBUG
			PrintDebugInfo( "ErrorErrorError2" );
#endif
			return 0;
		}


		if ( mbi.Protect != PAGE_READWRITE && mbi.Protect != PAGE_WRITECOPY && mbi.Protect != PAGE_READONLY )
		{

#ifdef BOTDEBUG
			PrintDebugInfo( "ErrorErrorError2" );
#endif
			return 0;
		}

		return 1;
	}
	else
		return 1;
}

union DWFP
{
	DWORD dw;
	float fl;
};


struct DebugStruct
{
	int addr;
	int procid;
};


void PrintDebugInfo( const char * debuginfo )
{
#ifdef BOTDEBUG
	if ( !DebugActive )
		return;
	DebugStr( debuginfo );
#endif
	sprintf_s( TechiesCrash, sizeof( TechiesCrash ), "%s", debuginfo );
}


#ifdef BOTDEBUG

int currentid = 0;
char dedata[ 50 ][ 256 ];
void * xxaddr;


void DebugStr( const char* szDllPath )
{
	if ( !DebugActive )
		return;


	if ( currentid < 50 )
	{
		CopyMemory( dedata[ currentid ], szDllPath, strlen( szDllPath ) + 1 );
		currentid++;
	}
	else
	{
		for ( int i = 0; i < 49; i++ )
		{
			CopyMemory( dedata[ i ], dedata[ i + 1 ], 256 );
		}
		CopyMemory( dedata[ 49 ], szDllPath, strlen( szDllPath ) + 1 );
	}
}



char * laststringinfo = new char[ 256 ];


void PrintDebugInfo2( const char * debuginfo, int data )
{
	if ( !DebugActive )
		return;
	sprintf_s( laststringinfo, 256, "%s:%X", debuginfo, data );
	DebugStr( laststringinfo );
}

void InitDebug( )
{
	xxaddr = &dedata;
	FILE * f = NULL;
	if ( fopen_s( &f, "debug.bin", "wb" ) == NOERROR )
	{
		fwrite( &xxaddr, 4, 1, f );
		fclose( f );
	}
}


#endif

#define IsKeyPressed(CODE) (GetAsyncKeyState(CODE) & 0x8000) > 0


int IsGame( )
{
	if ( !GameDll )
		return 0;

	int _GameUI = GameDll + 0x93631C;

	int * InGame = ( int * )( GameDll + 0xACE66C );

	if ( !InGame )
		return 0;


	return *( int* )*InGame == _GameUI;
}

LPVOID TlsValue;
DWORD TlsIndex;
DWORD _W3XTlsIndex;

DWORD GetIndex( )
{
	return *( DWORD* )( _W3XTlsIndex );
}

DWORD GetW3TlsForIndex( DWORD index )
{
	DWORD pid = GetCurrentProcessId( );
	THREADENTRY32 te32;
	HANDLE hSnap = CreateToolhelp32Snapshot( TH32CS_SNAPTHREAD, pid );
	te32.dwSize = sizeof( THREADENTRY32 );

	if ( Thread32First( hSnap, &te32 ) )
	{
		do
		{
			if ( te32.th32OwnerProcessID == pid )
			{
				HANDLE hThread = OpenThread( THREAD_ALL_ACCESS, 0, te32.th32ThreadID );
				CONTEXT ctx = { CONTEXT_SEGMENTS };
				LDT_ENTRY ldt;
				GetThreadContext( hThread, &ctx );
				GetThreadSelectorEntry( hThread, ctx.SegFs, &ldt );
				DWORD dwThreadBase = ldt.BaseLow | ( ldt.HighWord.Bytes.BaseMid <<
					16 ) | ( ldt.HighWord.Bytes.BaseHi << 24 );
				CloseHandle( hThread );
				if ( dwThreadBase == NULL )
					continue;
				DWORD* dwTLS = *( DWORD** )( dwThreadBase + 0xE10 + 4 * index );
				if ( dwTLS == NULL )
					continue;
				return ( DWORD )dwTLS;
			}
		} while ( Thread32Next( hSnap, &te32 ) );
	}

	return NULL;
}

void SetTlsForMe( )
{
	TlsIndex = GetIndex( );
	LPVOID tls = ( LPVOID )GetW3TlsForIndex( TlsIndex );
	TlsValue = tls;
}


void TextPrint( char *szText, float fDuration )
{
	if ( !IsGame( ) )
		return;
	if ( StealthMode )
		return;
#ifdef BOTDEBUG
	PrintDebugInfo( "PrintText" );
#endif
	UINT32 dwDuration = *( ( UINT32 * )&fDuration );
	int GAME_GlobalClass = GameDll + 0xAB4F80;
	int GAME_PrintToScreen = GameDll + 0x2F8E40;
	__asm
	{
		PUSH	0xFFFFFFFF;
		PUSH	dwDuration;
		PUSH	szText;
		PUSH	0x0;
		PUSH	0x0;
		MOV		ECX, [ GAME_GlobalClass ];
		MOV		ECX, [ ECX ];
		CALL	GAME_PrintToScreen;
	}
}

void TextPrint2( char* text, float StayUpTime )
{
	int GAME_GlobalClass = GameDll + 0xAB4F80;
	int GAME_PrintToScreen = GameDll + 0x2F3CF0;

	__asm
	{
		PUSH 0xFFFED312;
		PUSH StayUpTime;
		PUSH text;
		MOV		ECX, [ GAME_GlobalClass ];
		MOV		ECX, [ ECX ];
		MOV EAX, GAME_PrintToScreen;
		CALL EAX;
	}
}

time_t lastaccess = 0;


std::string LastString = "";

void TextPrintUnspammed( char *szText )
{
	if ( !IsGame( ) || !szText || szText[ 0 ] == '\0' )
		return;
	if ( StealthMode )
		return;
	time_t nextaccess = time( 0 );

	if ( !ExpertModeEnabled )
		nextaccess += 1;

	if ( nextaccess > lastaccess + 1 || ( ExpertModeEnabled && LastString != szText ) )
	{
		LastString = szText;
		float fDuration = 1.3f;
		if ( !ExpertModeEnabled )
			fDuration += 0.7f;
		lastaccess = nextaccess;
#ifdef BOTDEBUG
		PrintDebugInfo( "PrintText" );
#endif
		UINT32 dwDuration = *( ( UINT32 * )&fDuration );
		int GAME_GlobalClass = GameDll + 0xAB4F80;
		int GAME_PrintToScreen = GameDll + 0x2F8E40;
		__asm
		{
			PUSH	0xFFFFFFFF
			PUSH	dwDuration
			PUSH	szText
			PUSH	0x0
			PUSH	0x0
			MOV		ECX, [ GAME_GlobalClass ]
			MOV		ECX, [ ECX ]
			CALL	GAME_PrintToScreen
		}
	}
}

int GetUnitCount( )
{
	int GlobalClassOffset = *( int* )( GameDll + 0xAB4F80 );
	if ( GlobalClassOffset )
	{
		int UnitsOffset1 = *( int* )( GlobalClassOffset + 0x3BC );
		if ( UnitsOffset1 > 0 )
		{
			int  *  UnitsCount = ( int * )( UnitsOffset1 + 0x604 );
			if ( UnitsCount )
			{
				return *UnitsCount;
			}
		}
	}
	return 0;
}

// Pure code: Get unit count and units array
int * GetUnitCountAndUnitArray( int ** unitarray )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "GetUnitArray" );
#endif
	int GlobalClassOffset = *( int* )( GameDll + 0xAB4F80 );
	if ( GlobalClassOffset )
	{

		int UnitsOffset1 = *( int* )( GlobalClassOffset + 0x3BC );
		if ( UnitsOffset1 > 0 )
		{
			int  *  UnitsCount = ( int * )( UnitsOffset1 + 0x604 );
			if ( *( int* )UnitsCount > 0 )
			{
				if ( IsOkayPtr( ( void* )( UnitsOffset1 + 0x608 ) ) )
				{
#ifdef BOTDEBUG
					PrintDebugInfo( "OkayUnitArray" );
#endif
					*unitarray = ( int * ) *( int* )( UnitsOffset1 + 0x608 );
					return UnitsCount;
				}
				else
				{
					return 0;
				}
			}
		}
	}
	return 0;
}



int JMPADDR;

__declspec( naked ) void __fastcall sub_6F424B80( void *a1, int unused, int a2, int a3, int a4, int a5, int a6 )
{
	JMPADDR = GameDll + 0x424B80;
	__asm
	{
		JMP JMPADDR;
	}
}

__declspec( naked ) signed int __fastcall sub_6F424CE0( int a1, int unused, int a2, int a3, int a4, int a5 )
{
	JMPADDR = GameDll + 0x424CE0;
	__asm
	{
		JMP JMPADDR;
	}
}

__declspec( naked ) int __fastcall sub_6F425490( int a1, int unused, int a2 )
{
	JMPADDR = GameDll + 0x425490;
	__asm
	{
		JMP JMPADDR;
	}
}

__declspec( naked ) int __fastcall sub_6F332700( void *a1, int unused )
{
	JMPADDR = GameDll + 0x332700;
	__asm
	{
		JMP JMPADDR;
	}
}

__declspec( naked ) int __fastcall sub_6F03FA30( UINT a1, UINT a2 )
{
	JMPADDR = GameDll + 0x3FA30;
	__asm
	{
		JMP JMPADDR;
	}
}

typedef int( __fastcall * sub_6F26EC20 )( int unitaddr, int unused, unsigned int SLOTID );
sub_6F26EC20 GetItemInSlot;

//
typedef void *( __fastcall * sub_6F0787D0 )( int unitaddr, int unused, int classid, int a3, int a4, int a5, int a6 );
sub_6F0787D0 GetAbility;

unsigned int cooldownflag = 0x200;
unsigned int xavaiableflag = 0x8000;



/*

sub_6F339DD0my: (a1):d0028 (a2):c0114e4 (a3):-411.262 (a4):-347.085 (a5):bfdc104 (a6):4 (a7):4
//ETH - I0LT

sub_6F339DD0my: (a1):d0029 (a2):c010a3c (a3):-443.463 (a4):-383.475 (a5):bfdc104 (a6):100004 (a7):4
//ORCH - I012

sub_6F339CC0my: (a1):d002a (a2):c010f14 (a3):-499.668 (a4):-494.432 (a5):1100002 (a6):4
//HUETA - I0O3
*/


int IsAbilityCooldown( int unitaddr, int id )
{
	void * abiladdr = GetAbility( unitaddr, 0, id, 0, 1, 1, 1 );
	if ( !abiladdr || !IsOkayPtr( ( void * )abiladdr ) )
		return -1;

	int avilityflag = ( int )abiladdr + 32;
	if ( IsOkayPtr( ( void* )avilityflag ) )
	{
		unsigned int  state = *( unsigned int  * )( avilityflag );
		return state & cooldownflag;
	}
	else
		return -1;
}

int IsAbilityHidden( int unitaddr, int id )
{
	void * abiladdr = GetAbility( unitaddr, 0, id, 0, 1, 1, 1 );
	if ( !abiladdr || !IsOkayPtr( ( void * )abiladdr ) )
		return -1;

	int avilityflag = ( int )abiladdr + 32;
	if ( IsOkayPtr( ( void* )avilityflag ) )
	{
		unsigned int  state = *( unsigned int  * )( avilityflag );
		return state & xavaiableflag;
	}
	else
		return -1;
}

void PrintCooldownFlag( int unitaddr, int id )
{
	void * abiladdr = GetAbility( unitaddr, 0, id, 0, 1, 1, 1 );
	if ( !abiladdr || !IsOkayPtr( ( void * )abiladdr ) )
		return;

	int abilleveladdr = ( int )abiladdr + 4;
	if ( IsOkayPtr( ( void* )abilleveladdr ) )
	{
		int xid = id;
		char cc1 = *( char* )( ( int )( &xid ) );
		char cc2 = *( char* )( ( int )( &xid ) + 1 );
		char cc3 = *( char* )( ( int )( &xid ) + 2 );
		char cc4 = *( char* )( ( int )( &xid ) + 3 );
		char cc5[ ] = { cc4, cc3, cc2, cc1, '\0' };
		unsigned int  state = *( unsigned int  * )( abilleveladdr );
		char * printdate = new char[ 100 ];
		memset( printdate, 0, 100 );
		sprintf_s( printdate, 100, "%s->%X->%X", cc5, state, ( unsigned int )abiladdr );
		TextPrint( printdate, 2.0f );
		Sleep( 500 );
		delete[ ]printdate;
	}
}


void * GetGlobalPlayerData( )
{
	if ( *( int * )( 0xAB65F4 + GameDll ) > 0 )
	{
		if ( IsOkayPtr( ( void* )( 0xAB65F4 + GameDll ) ) )
			return ( void * ) *( int* )( 0xAB65F4 + GameDll );
		else
			return nullptr;
	}
	else
		return nullptr;
}

void RemoveUnitHandler( int unitaddr )
{
	// Update unit list force
}

int GetPlayerByNumber( int number )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "GetPlayerByNumber" );
#endif
	void * arg1 = GetGlobalPlayerData( );
	int result = -1;
	if ( number >= 0 && number <= 12 && arg1 != nullptr && arg1 )
	{
		result = ( int )arg1 + ( number * 4 ) + 0x58;

		if ( IsOkayPtr( ( void* )result ) )
		{
			result = *( int* )result;
		}
		/*__asm
		{
		mov ecx , arg1;
		mov eax , number;
		mov eax , [ ecx + eax * 4 + 0x58 ];
		mov result , eax;
		}*/
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
		int playerslotaddr = ( int )gldata + 0x28;
		if ( IsOkayPtr( ( void* )playerslotaddr ) )
			return ( int ) *( short * )( playerslotaddr );
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

int DisableWindowActiveCheck = 1;

int IsWindowActive( int SkipNoCheck = 0 )
{
	if ( DisableWindowActiveCheck && !SkipNoCheck )
		return DisableWindowActiveCheck;

	return *( int* )( GameDll + 0xA9E7A4 );
}

UINT GetUnitOwnerSlot( int unitaddr )
{
	if ( IsOkayPtr( ( void* )( unitaddr + 88 ) ) )
		return *( int* )( unitaddr + 88 );
	return -1;
}

int IsPlayerEnemy( int unitaddr )
{
	int teamplayer1 = GetLocalPlayerNumber( ) > 5 ? 1 : 2;
	int teamplayer2 = GetUnitOwnerSlot( unitaddr ) > 5 ? 1 : 2;
#ifdef BOTDEBUG
	PrintDebugInfo( "EndGetPlayerEnemy" );
#endif
	return teamplayer1 != teamplayer2;
}

int IsUnitDead( int unitaddr )
{
	unsigned int isdolbany = *( unsigned int* )( unitaddr + 0x5C );
	int UnitNotDead = ( ( isdolbany & 0x100u ) == 0 );
	return UnitNotDead == 0;
}

void GetMousePosition( float * x, float * y, float * z )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "Mouse info" );
#endif
	int globalclass = *( int* )_W3XGlobalClass;

	int offset1 = globalclass + 0x3BC;

	if ( globalclass > 0 )
	{


		if ( IsOkayPtr( ( void* )offset1 ) )
		{
			offset1 = *( int * )offset1;
			if ( IsOkayPtr( ( void* )offset1 ) )
			{
				*x = *( float* )( offset1 + 0x310 );
				*y = *( float* )( offset1 + 0x310 + 4 );
				*z = *( float* )( offset1 + 0x310 + 4 + 4 );
			}
			else
			{
				*x = 0.0f;
				*y = 0.0f;
				*z = 0.0f;
			}
		}
		else
		{
			*x = 0.0f;
			*y = 0.0f;
			*z = 0.0f;
		}

	}
}

int GetSelectedOwnedUnit( )
{

	int plr = GetLocalPlayer( );
	if ( plr != -1 && plr )
	{

		int unitaddr = 0; // = *(int*)((*(int*)plr+0x34)+0x1e0);

		__asm
		{
			MOV EAX, plr;
			MOV ECX, DWORD PTR DS : [EAX + 0x34];
			MOV EAX, DWORD PTR DS : [ECX + 0x1E0];
			MOV unitaddr, EAX;
		}


		if ( unitaddr > 0 )
		{
			if ( GetUnitOwnerSlot( unitaddr ) == GetLocalPlayerNumber( ) )
			{
#ifdef BOTDEBUG
				PrintDebugInfo( "SelectOwningUnit" );
#endif
				return unitaddr;
			}
		}
#ifdef BOTDEBUG
		PrintDebugInfo( "No owned unit" );
#endif
	}
	return NULL;
}


/*
int IsButtonCooldown( DWORD buttonid )
{
if ( v22 )
v11 = sub_6F420B40( v6 , a3 , v17 , a5 );
else
v11 = sub_6F420A80( ( void * ) v6 , a3 , v17 , a5 );


}
*/

#define BTN_TECHIES_SUICIDE 0xD0048
#define BTN_TINI_TOSS 0xD0278
#define BTN_TRAXES_FORCE 0xD02BD
#define BTN_VENG_SWAP 0xD026B

int IsBtnCooldown1( int buttonid )
{

}


int IsBtnCooldown2( int buttonid )
{

}




/*
int IsSuicideCooldown( int btnid )
{
return GetBtnCooldown( BTN_TECHIES_SUICIDE , 1 );
}
*/


int GetUnitItemCODE( int unit_or_item_addr )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "GetUnitItemCODE" );
#endif
	return *( int* )( unit_or_item_addr + 0x30 );
}

typedef int( __fastcall * sub_6F32C880 )( int unit_item_code, int unused );
sub_6F32C880 GetTypeInfo = NULL;


char * GetUnitName( int unitaddr )
{
	int unitcode = GetUnitItemCODE( unitaddr );
	if ( unitcode > 0 )
	{

		int v3 = GetTypeInfo( unitcode, 0 );
		int v4, v5;
		if ( v3 && ( v4 = *( int * )( v3 + 40 ) ) != 0 )
		{
			v5 = v4 - 1;
			if ( v5 >= ( unsigned int )0 )
				v5 = 0;
			return ( char * ) *( int * )( *( int * )( v3 + 44 ) + 4 * v5 );
		}
		else
		{
			return "Default string";
		}
	}
	return "Default String";
}

void PrintClassAddress( int unitaddr )
{
	int unitcode = GetUnitItemCODE( unitaddr );
	if ( unitcode > 0 )
	{
		int v3 = GetTypeInfo( unitcode, 0 );
		char * printdada = new char[ 200 ];
		sprintf_s( printdada, 200, "ClassAddr:%X", v3 );
		TextPrint( printdada, 15.0f );

	}

}


void SetUnitColor( int unitaddr, UINT color )
{
	int unitdata = *( int* )( unitaddr + 0x28 );
	if ( unitdata > 0 )
	{
		*( UINT* )( unitdata + 0x328 ) = color;
		*( UINT* )( unitdata + 0x320 ) = 0;
		*( UINT* )( unitdata + 0x316 ) = 0;
		if ( !( *( UINT* )( unitdata + 0x312 ) & 0x800 ) )
		{
			*( UINT* )( unitdata + 0x312 ) = *( UINT* )( unitdata + 0x316 ) | 0x800;
		}

	}
}

int IsClassEqual( int unit_or_item_addr, char * classid )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "UnitCLass" );
#endif
	char unitclass[ 5 ];
	memset( unitclass, 0, 5 );

	if ( IsOkayPtr( ( void* )( unit_or_item_addr + 0x30 ) ) )
	{
		*( BYTE* )&unitclass[ 0 ] = *( BYTE* )( unit_or_item_addr + 0x30 + 3 );
		*( BYTE* )&unitclass[ 1 ] = *( BYTE* )( unit_or_item_addr + 0x30 + 2 );
		*( BYTE* )&unitclass[ 2 ] = *( BYTE* )( unit_or_item_addr + 0x30 + 1 );
		*( BYTE* )&unitclass[ 3 ] = *( BYTE* )( unit_or_item_addr + 0x30 + 0 );
		if ( strlen( classid ) == 4 )
		{
			if ( unitclass[ 0 ] == classid[ 0 ] && unitclass[ 1 ] == classid[ 1 ] &&
				unitclass[ 2 ] == classid[ 2 ] && unitclass[ 3 ] == classid[ 3 ] )
				return 1;
		}
	}

	return 0;
}

struct UnitLocation
{
	float X;
	float Y;
	float Z;
};

struct Location
{
	float X;
	float Y;
};


#define ADDR(X,REG)\
	__asm MOV REG, DWORD PTR DS : [ X ] \
	__asm MOV REG, DWORD PTR DS : [ REG ]

void SendMoveAttackCommand( int cmdId, float X, float Y )
{
	int _MoveAttackCmd = GameDll + 0x339DD0;

	if ( *( int* )_W3XGlobalClass > 0 )
	{
		__asm
		{
			ADDR( _W3XGlobalClass, ECX );
			MOV ECX, DWORD PTR DS : [ECX + 0x1B4];
			PUSH 0;
			PUSH 6;
			PUSH 0;
			PUSH Y;
			PUSH X;
			PUSH 0;
			PUSH cmdId;

			CALL _MoveAttackCmd;
		}
	}
}


int GetCMDbyItemSlot( int slot ) // ot 1 do 6
{

	return ( 0xd0028 + slot );
}



int IsItemCooldown( int itemaddr )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "CheckItemCooldownv1" );
#endif
	if ( itemaddr == NULL )
		return 1;

	return !( ( *( UINT* )( itemaddr + 32 ) ) & 0x400u );
}

int commandjumpaddr;
void  sub_6F339DD0my( int a1, int a2, float a3, float a4, int a5, int a6, int a7 )
{
	commandjumpaddr = GameDll + 0x339DD0;
	if ( *( int* )_W3XGlobalClass > 0 )
	{
		__asm
		{

			PUSH a7;
			PUSH a6;
			PUSH a5;
			PUSH a4;
			ADDR( _W3XGlobalClass, ECX );
			MOV ECX, DWORD PTR DS : [ECX + 0x1B4];
			PUSH a3;
			PUSH a2;
			PUSH a1;
			CALL commandjumpaddr;
		}
	}
}



void  CommandItemTarget( int cmd, int itemaddr, float targetx, float targety, int targetunitaddr, int queue = 0 )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "UseIt" );
#endif
	sub_6F339DD0my( cmd, itemaddr, targetx, targety, targetunitaddr, 4, queue ? 5 : 4 );
}

int CmdWOTaddr;
void  sub_6F339C60my( int a1, int a2, unsigned int a3, unsigned int a4 )
{
	CmdWOTaddr = GameDll + 0x339C60;
	if ( *( int* )_W3XGlobalClass > 0 )
	{
		__asm
		{

			PUSH a4;
			PUSH a3;
			ADDR( _W3XGlobalClass, ECX );
			MOV ECX, DWORD PTR DS : [ECX + 0x1B4];
			PUSH a2;
			PUSH a1;

			CALL CmdWOTaddr;
		}
	}
}


int CmdPointAddr;
void  sub_6F339CC0my( int a1, int a2, float a3, float a4, int a5, int a6 )
{
	CmdPointAddr = GameDll + 0x339CC0;
	if ( *( int* )_W3XGlobalClass > 0 )
	{
		__asm
		{

			PUSH a6;
			PUSH a5;
			PUSH a4;
			ADDR( _W3XGlobalClass, ECX );
			MOV ECX, DWORD PTR DS : [ECX + 0x1B4];
			PUSH a3;
			PUSH a2;
			PUSH a1;

			CALL CmdPointAddr;
		}
	}
}

void  ItemOrSkillPoint( int cmd, int itemorskilladdr, float x, float y, int a5, int addque = 0 )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "ItemSkillPOint" );
#endif
	sub_6F339CC0my( cmd, itemorskilladdr, x, y, a5, addque ? 5 : 4 );
}

void UseDetonator( )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "Use detonate" );
#endif
	sub_6F339C60my( 0xd024c, 0, 1, 4 );
}


typedef void( __cdecl *mPingMinimap )( float *x, float *y, float *duration );
mPingMinimap PingMinimap;

Location GetNextPoint( float x, float y, float distance, float angle )
{
	Location returnlocation = Location( );
	returnlocation.X = x + distance * cos( angle );
	returnlocation.Y = y + distance * sin( angle );
	return returnlocation;
}



void GetUnitLocation( int unitaddr, float * x, float * y )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "Unit 2d" );
#endif
	*x = *( float* )( unitaddr + 0x284 );
	*y = *( float* )( unitaddr + 0x288 );
}

void GetUnitLocation3D( int unitaddr, float * x, float * y, float * z )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "Unit 3d" );
#endif
	*x = *( float* )( unitaddr + 0x284 );
	*y = *( float* )( unitaddr + 0x288 );
	*z = *( float* )( unitaddr + 0x28C );
}

float GetUnitFloatStat( int unitaddr, DWORD statNum )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "UnitFloatState" );
#endif
	int _GetFloatStat = GameDll + 0x27AE90;
	float result = 0;
	__asm
	{
		PUSH statNum;
		LEA EAX, result
			PUSH EAX
			MOV ECX, unitaddr
			CALL _GetFloatStat
	}
	return result;
}


float GetUnitHP( int unitaddr )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "Unit hp" );
#endif
	return GetUnitFloatStat( unitaddr, 0 );
}


float GetUnitMaxHP( int unitaddr )
{
	return GetUnitFloatStat( unitaddr, 1 );
}

int GetUnitHPPercent( int unitaddr )
{
	return ( int )( ( GetUnitHP( unitaddr ) / GetUnitMaxHP( unitaddr ) ) * 100.f );
}


int IsHero( int unitaddr )
{
	if ( IsOkayPtr( ( void* )( unitaddr + 48 ) ) )
	{
		unsigned int ishero = *( unsigned int* )( unitaddr + 48 );
		ishero = ishero >> 24;
		ishero = ishero - 64;
		return ishero < 0x19;
	}
	return 0;
}

float GetUnitTimer( int unitaddr )
{
	int unitdataddr = *( int* )( unitaddr + 0x28 );
	if ( unitdataddr <= 0 )
		return 0.0f;
#ifdef BOTDEBUG
	PrintDebugInfo( "CheckBadUnit - ENDCHExCK5" );
#endif
	if ( IsOkayPtr( ( void* )( unitdataddr + 0xA0 ), 4 ) )
		return *( float* )( unitdataddr + 0xA0 );
	return 0.0f;
}

// Проверяет юнит или не юнит
int __stdcall IsNotBadUnit( int unitaddr, int onlymem = 0 )
{
	if ( unitaddr > 0 && IsOkayPtr( ( void* )unitaddr, 0x100 ) )
	{
		int UnitVtable = GameDll + 0x931934;
		int xaddraddr = ( int )&UnitVtable;

		if ( *( BYTE* )xaddraddr != *( BYTE* )unitaddr )
			return 0;
		else if ( *( BYTE* )( xaddraddr + 1 ) != *( BYTE* )( unitaddr + 1 ) )
			return 0;
		else if ( *( BYTE* )( xaddraddr + 2 ) != *( BYTE* )( unitaddr + 2 ) )
			return 0;
		else if ( *( BYTE* )( xaddraddr + 3 ) != *( BYTE* )( unitaddr + 3 ) )
			return 0;

		unsigned int x1 = *( unsigned int* )( unitaddr + 0xC );
		unsigned int y1 = *( unsigned int* )( unitaddr + 0x10 );

		int udata = *( int* )( unitaddr + 0x28 );


		if ( x1 == 0xFFFFFFFF || y1 == 0xFFFFFFFF || udata == 0 )
		{

			return 0;
		}

		if ( onlymem )
			return 1;

		unsigned int unitflag = *( unsigned int* )( unitaddr + 0x20 );
		unsigned int unitflag2 = *( unsigned int* )( unitaddr + 0x5C );

		if ( unitflag & 1u )
		{

			return 0;
		}

		if ( !( unitflag & 2u ) )
		{

			return 0;
		}

		if ( unitflag2 & 0x100u )
		{

			return 0;
		}

		/*	if ( unitflag2 == 0x1001u )
		{
		if ( SetInfoObjDebugVal )
		{
		PrintText( "Flag 4 bad" );
		}
		return 0;
		}
		*/
		return 1;
	}

	return 0;
}





float Distance( float dX0, float dY0, float dX1, float dY1 )
{
	return sqrt( ( dX1 - dX0 )*( dX1 - dX0 ) + ( dY1 - dY0 )*( dY1 - dY0 ) );
}


float Distance3D( float x1, float y1, float z1, float x2, float y2, float z2 )
{
	if ( Enable3DPoint )
	{
		double d[ ] = { abs( ( double )x1 - ( double )x2 ), abs( ( double )y1 - ( double )y2 ), abs( ( double )z1 - ( double )z2 ) };
		if ( d[ 0 ] < d[ 1 ] ) std::swap( d[ 0 ], d[ 1 ] );
		if ( d[ 0 ] < d[ 2 ] ) std::swap( d[ 0 ], d[ 2 ] );
		return ( float )( d[ 0 ] * sqrt( 1.0 + d[ 1 ] / d[ 0 ] + d[ 2 ] / d[ 0 ] ) );
	}
	else
	{
		return Distance( x1, y1, x2, y2 );
	}
}

float DistanceBetweenLocs( Location loc1, Location loc2 )
{
	return Distance( loc1.X, loc1.Y, loc2.X, loc2.Y );
}

Location GiveNextLocationFromLocAndAngle( Location startloc, float distance, float angle )
{
	return GetNextPoint( startloc.X, startloc.Y, distance, angle );
}

int TechiesFound = 0;


struct BombStruct
{
	int unitaddr;
	float dmg;
	float range1;
	float range2;
	float x;
	float y;
	float z;
	int remote;
};



void __cdecl sub_6F3C7910( int xunitaddr )
{
	DWORD _UnitSelect = 0x381710 + GameDll;

	if ( !IsNotBadUnit( xunitaddr ) )
	{
		return;
	}

	__asm
	{
		MOV EAX, DWORD PTR DS : [_W3XGlobalClass];
		MOV EAX, DWORD PTR DS : [EAX];
		MOV ESI, DWORD PTR DS : [EAX + 0x24C];
		PUSH 0;
		PUSH 0;
		PUSH 0;
		PUSH xunitaddr;
		MOV ECX, ESI;
		CALL _UnitSelect;
	}
}




int SelectUnit( int xunitaddr )
{
	if ( !IsNotBadUnit( xunitaddr ) )
	{
		return -1;
	}

	if ( *( BYTE * )( xunitaddr + 32 ) & 2 )
	{
		void * playerseldata = *( void** )( GetLocalPlayer( ) + 0x34 );
		WORD playerslot = GetLocalPlayerNumber( );
		sub_6F424B80( playerseldata, 0, xunitaddr, playerslot, 0, 1, 1 );
		sub_6F425490( ( int )playerseldata, 0, 0 );
		return sub_6F332700( 0, 0 );
	}

	return -1;
}


int __cdecl SelectAllUnits( )
{
	int myselectedunits = 0;
#ifdef BOTDEBUG
	PrintDebugInfo( "Select units" );
#endif

	__asm
	{
		MOV EDX, GameDll;
		ADD EDX, 0x3BBAA0;
		CALL EDX;
	}


	int DefaultUnitCount = GetUnitCount( );
	auto unittoselectlocal = unitstoselect;

	for ( unsigned int i = 0; i < unittoselectlocal.size( ); i++ )
	{
		if ( myselectedunits < 11 )
		{
			Sleep( 5 );
			if ( !IsGame( ) || DefaultUnitCount != GetUnitCount( ) )
			{
				return 0;
			}

			if ( SelectUnit( unittoselectlocal[ i ] ) != -1 )
				myselectedunits++;
		}
	}

	Sleep( 10 );

	if ( !IsGame( ) || DefaultUnitCount != GetUnitCount( ) )
	{
		return 0;
	}
	return myselectedunits > 0;
}


float GetProtectForUnit( int unitaddr )
{
	float Armor = 0.0f;

	if ( IsOkayPtr( ( void* )( unitaddr + 0xE0 ) ) )
	{
		Armor = *( float* )( unitaddr + 0xE0 );
	}

	if ( Armor < 0.0f )
		Armor = ( float )( 1.0 - pow( 0.94, -Armor ) );
	else
		Armor = ( float )( ( 0.06 * Armor ) / ( 1 + 0.06 * Armor ) );
	return Armor;
}

float GetProtectForProtect( float Armor )
{
	if ( Armor < 0.0f )
		Armor = ( float )( 1.0 - pow( 0.94, -Armor ) );
	else
		Armor = ( float )( ( 0.06 * Armor ) / ( 1 + 0.06 * Armor ) );
	return Armor;
}

int GetUnitAbilityLevel( int unitaddr, int id, int checkavaiable = 0, int CheckMemAccess = 0 )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "GetAbilityLevel" );
#endif

	if ( CheckMemAccess )
		if ( !IsNotBadUnit( unitaddr ) )
			return 0;

	void * abiladdr = ( void * )GetAbility( unitaddr, 0, id, 0, 1, 1, 1 );
	if ( !abiladdr || !IsOkayPtr( ( void * )abiladdr ) )
		return 0;

	int abilleveladdr = ( int )abiladdr + 80;
	int abilavaiable = ( int )abiladdr + 32;
	if ( checkavaiable )
	{
		if ( IsOkayPtr( ( void* )abilavaiable ) )
		{
			unsigned int avaiableflag = *( unsigned int * )( abilavaiable );
			if ( avaiableflag & xavaiableflag )
				return 0;
			if ( IsOkayPtr( ( void* )abilleveladdr ) )
				return *( int * )( abilleveladdr )+1;
			else
				return 0;
		}
		else
			return 0;
	}
	else
	{
		if ( IsOkayPtr( ( void* )abilleveladdr ) )
			return *( int * )( abilleveladdr )+1;
		else
			return 0;
	}
}


std::string bufflist;


float GetDmgForUnit( int unitaddr, float dmg, int dmgtype = 1/* 1 : magical , 2 : physical*/ )
{
	bufflist.clear( );
#ifdef BOTDEBUG
	PrintDebugInfo( "CheckUnitDmg" );
#endif

	// HEROES:
	// H00Q - HUSKAR
	// U00F - PUDGE
	// Edem - MAGINA
	// EC77 - VIPER
	//
	//
	// ITEMS:
	// I04P = -15% magical dmg
	// I0K6 = -30% magical dmg
	//
	// SKILS:
	// A0QQ - HUSKAR 3 skill
	// A06D - PUDGE 3 skill
	// A0KY - MAGINE 3 skill
	// A0MM - VIPER 3 skill
	//
	// BUFFS:
	// B014
	// B041
	// B08J
	// B0FP
	// B0FQ
	// A2T4
	// BNms - 50%

	if ( !IsNotBadUnit( unitaddr, 1 ) )
	{
		return 0.0f;
	}

	if ( GetUnitAbilityLevel( unitaddr, 'B07D', 0, 1 ) ||
		GetUnitAbilityLevel( unitaddr, 'B08J', 0, 1 ) ||
		GetUnitAbilityLevel( unitaddr, 'B05S', 0, 1 ) )
	{
		bufflist = std::string( "[100% shield]" );
		return 0.0f;
	}



	if ( dmgtype == 1 && ( GetUnitAbilityLevel( unitaddr, 'B014', 0, 1 ) ||
		GetUnitAbilityLevel( unitaddr, 'B041', 0, 1 ) ||
		GetUnitAbilityLevel( unitaddr, 'B0FP', 0, 1 ) ||
		GetUnitAbilityLevel( unitaddr, 'A2T4', 0, 1 ) ||
		GetUnitAbilityLevel( unitaddr, 'B0FQ', 0, 1 )

		) )
	{
		bufflist = std::string( "[100% magic shield]" );
		return 0.0f;
	}

	if ( dmgtype == 2 && GetUnitAbilityLevel( unitaddr, 'Aetl', 0, 1 ) )
	{
		bufflist = std::string( "[100% phys shield]" );
		return 0.0f;
	}

	float outdmg = 0.0f;
	outdmg -= 15.0f;

	if ( dmgtype == 1 )
		outdmg = dmg * ( ( 100.0f - 26.0f ) / 100.0f );
	else if ( dmgtype == 2 )
		outdmg = dmg - ( dmg * GetProtectForUnit( unitaddr ) );


	// Jugger(Nbbc) if skill A05G not exist - 100% protect



	//H00J H00I



	if ( dmgtype == 1 && IsClassEqual( unitaddr, "Nbbc" ) )
	{
		if ( IsNotBadUnit( unitaddr, 1 ) && GetUnitAbilityLevel( unitaddr, 'A05G', 1 ) == 0 )
		{

			bufflist += std::string( "[magic 100%]" );

			return 0.0f;
		}
	}

	if ( dmgtype == 1 && ( IsClassEqual( unitaddr, "H00I" ) || IsClassEqual( unitaddr, "H00J" ) ) )
	{
		bufflist += std::string( "[magic 10%]" );
		outdmg = outdmg * ( ( 100.0f - 10.0f ) / 100.0f );
	}


	int medusashield = GetUnitAbilityLevel( unitaddr, 'BNms' );
	if ( medusashield > 0 )
	{
		bufflist += std::string( "[50% shield]" );
		outdmg = outdmg * ( ( 100.0f - 50.0f ) / 100.0f );
	}


	int captainbuff = GetUnitAbilityLevel( unitaddr, 'B09U' );
	if ( captainbuff > 0 )
	{
		bufflist += std::string( "[50% shield]" );
		outdmg = outdmg * ( ( 100.0f - 50.0f ) / 100.0f );
	}


	int warcrysven = GetUnitAbilityLevel( unitaddr, 'B0BV' );
	if ( warcrysven > 0 && dmgtype == 2 )
	{
		float addprotect = GetProtectForProtect( warcrysven * 4.0f );
		int addprotectp = ( int )( 100 * addprotect );
		std::string( "[phys " ) + std::to_string( addprotectp ) + std::string( "%]" );
		outdmg = dmg - addprotect;
	}



	int huskarlvl = GetUnitAbilityLevel( unitaddr, 'A0QQ' );
	if ( dmgtype == 1 && huskarlvl > 0 )
	{

		float dmgprotectlvl = ( float )huskarlvl + 3.0f;
		int hppercent = 100 - GetUnitHPPercent( unitaddr );
		int parts = hppercent / 7 + 1;
		dmgprotectlvl = dmgprotectlvl * parts;

		bufflist += std::string( "[magic " ) + std::to_string( ( int )dmgprotectlvl ) + std::string( "%]" );

		outdmg = outdmg * ( ( 100.0f - dmgprotectlvl ) / 100.0f );
	}

	int pudgelvl = GetUnitAbilityLevel( unitaddr, 'A06D' );
	if ( dmgtype == 1 && pudgelvl > 0 )
	{
		// 1 lvl = (1*2)+4 = 6
		// 2 lvl = (2*2)+4 = 8
		// 3 lvl = (3*2)+4 = 10
		// 4 lvl = (4*2)+4 = 12
		float dmgprotect = ( pudgelvl * 2.0f ) + 4.0f;
		bufflist += std::string( "[magic " ) + std::to_string( dmgprotect ) + std::string( "%]" );

		outdmg = outdmg * ( ( 100.0f - dmgprotect ) / 100.0f );

	}


	int rubiclvl = GetUnitAbilityLevel( unitaddr, 'B0EO' );
	if ( dmgtype == 1 && pudgelvl > 0 )
	{
		// 1 lvl = (1*2)+4 = 6
		// 2 lvl = (2*2)+4 = 8
		// 3 lvl = (3*2)+4 = 10
		// 4 lvl = (4*2)+4 = 12
		float dmgprotect = rubiclvl * 5.0f;
		bufflist += std::string( "[magic " ) + std::to_string( dmgprotect ) + std::string( "%]" );

		outdmg = outdmg * ( ( 100.0f - dmgprotect ) / 100.0f );

	}




	int spectralvl = GetUnitAbilityLevel( unitaddr, 'A0NA' );
	if ( spectralvl > 0 )
	{
		// 1 lvl = (1*4)+6 = 10
		// 2 lvl = (2*4)+6 = 12
		// 3 lvl = (3*4)+6 = 16
		// 4 lvl = (4*4)+6 = 22
		float dmgprotect = ( spectralvl * 4.0f ) + 6.0f;
		bufflist += std::string( "[all " ) + std::to_string( dmgprotect ) + std::string( "%]" );

		outdmg = outdmg * ( ( 100.0f - dmgprotect ) / 100.0f );
	}


	int maginalvl = GetUnitAbilityLevel( unitaddr, 'A0KY' );
	if ( dmgtype == 1 && maginalvl > 0 )
	{
		// 1 lvl = (1*8)+18 = 26
		// 2 lvl = (2*8)+18 = 34
		// 3 lvl = (3*8)+18 = 42
		// 4 lvl = (4*8)+18 = 50
		float dmgprotect = ( maginalvl * 8.0f ) + 18.0f;
		bufflist += std::string( "[magic " ) + std::to_string( dmgprotect ) + std::string( "%]" );
		outdmg = outdmg * ( ( 100.0f - dmgprotect ) / 100.0f );
	}

	int viperlvl = GetUnitAbilityLevel( unitaddr, 'A0MM' );
	if ( dmgtype == 1 && viperlvl > 0 )
	{
		// 1 lvl = (1*5)+5 = 10
		// 2 lvl = (2*5)+5 = 15
		// 3 lvl = (3*5)+5 = 20
		// 4 lvl = (4*5)+5 = 25
		float dmgprotect = ( viperlvl * 5.0f ) + 5.0f;
		bufflist += std::string( "[magic " ) + std::to_string( dmgprotect ) + std::string( "%]" );
		outdmg = outdmg * ( ( 100.0f - dmgprotect ) / 100.0f );
	}

	if ( dmgtype == 1 )
	{
		for ( int i = 0; i < 6; i++ )
		{
			int nitem = GetItemInSlot( unitaddr, 0, i );
			if ( nitem )
			{
				if ( IsClassEqual( nitem, "I04P" ) )
				{
					bufflist += std::string( "[magic 15%]" );
					outdmg = outdmg * ( ( 100.0f - 15.0f ) / 100.0f );
				}
				else if ( IsClassEqual( nitem, "I0K6" ) )
				{
					bufflist += std::string( "[magic 30%]" );
					outdmg = outdmg * ( ( 100.0f - 30.0f ) / 100.0f );
				}
			}
		}


		if ( GetUnitAbilityLevel( unitaddr, 'Aetl', 0, 1 ) )
		{
			bufflist = std::string( "[+50% magic]" );
			outdmg = outdmg * 1.5f;
		}

	}
	else if ( dmgtype == 2 )
	{

	}

	if ( !IsNotBadUnit( unitaddr, 1 ) )
		return 0.0f;

	return outdmg;
}

char facici[ 50 ];

float  GetUnitFacing( int unitaddr )
{
	int unitdataoffset = *( int* )( unitaddr + 0x28 );
	if ( unitdataoffset > 0 )
	{
		double atan2value = atan2( ( double ) *( float* )( unitdataoffset + 0x10c ), ( double ) *( float* )( unitdataoffset + 0x108 ) );
		return  ( float )atan2value;
	}
	return 100.0f;
}


int __cdecl IsUnitVisibleToPlayer( int unitaddr, int player )
{
	if ( player != -1 && player )
	{
		__asm
		{
			mov esi, unitaddr;
			mov eax, player;
			movzx eax, byte ptr[ eax + 0x30 ];
			mov edx, [ esi ];
			push 0x04;
			push 0x00;
			push eax;
			mov eax, [ edx + 0x000000FC ];
			mov ecx, esi;
			call eax;
		}
	}
	else
		return 0;
}


int SelectTechies( )
{
	int returnvalue = 0;
	if ( techiesaddr > 0 )
	{
		if ( GetSelectedOwnedUnit( ) == techiesaddr )
			return 1;

		if ( IsNotBadUnit( techiesaddr ) )
		{
#ifdef BOTDEBUG
			PrintDebugInfo( "Techies selected" );
#endif
			unitstoselect.clear( );
			unitstoselect.push_back( techiesaddr );
			returnvalue = SelectAllUnits( );
			unitstoselect.clear( );
		}
		else
		{
#ifdef BOTDEBUG
			PrintDebugInfo( "Techies not selected 11" );
#endif
			if ( !IsNotBadUnit( techiesaddr, techiesaddr ) && !IsHero( techiesaddr ) )
				techiesaddr = 0;

		}
	}

	return returnvalue;
}


struct TechiesActionStruct
{
	int action;
	int data;
};
safevector<BombStruct> BombList;

int EnableAutoExplode = 0;
int EnableDagger = 0;
int FunModeEnabled = 0;

int _Daggercooldown = 0;
int EnableForceStaff = 1;
int TechiesType = -1;
int TechiesSlot = -1;

int _Forcecooldown = 0;
int forcestaffcooldown = 0;
int daggercooldown = 0;

int IsFirstFun = 1;
char OldTechiesMissile[ 256 ];
int oldlen = 0;
char * funtechiesmissile = "Abilities\\Weapons\\BlackKeeperMissile\\BlackKeeperMissile.mdl\x0\xA\x0\xA";

void TechiesFunMode( int enabled )
{

	if ( IsFirstFun )
	{

		int ClassAddr = GetTypeInfo( 'H00K', 0 );
		if ( ClassAddr > 0 )
		{
			int offset2 = ClassAddr + 0x44;
			offset2 = *( int* )offset2;
			if ( offset2 > 0 )
			{

				offset2 = *( int* )offset2;
				if ( offset2 > 0 )
				{
					char * modelname = 0;
					IsFirstFun = 0;
					*( int* )&modelname = offset2;
					sprintf_s( OldTechiesMissile, 256, "%s", modelname );
					int strlenold = strlen( OldTechiesMissile );
					oldlen = strlenold + 1;
					OldTechiesMissile[ strlenold ] = '\x0';
					OldTechiesMissile[ strlenold + 1 ] = '\xA';
				}
				else
					return;
			}

		}
	}

	if ( !IsFirstFun )
	{

		if ( enabled )
		{
			int ClassAddr = GetTypeInfo( 'H00K', 0 );
			if ( ClassAddr > 0 )
			{
				int offset2 = ClassAddr + 0x44;
				offset2 = *( int* )offset2;
				if ( offset2 )
				{

					offset2 = *( int* )offset2;
					if ( offset2 )
					{
						char * modelname = 0;
						*( int* )&modelname = offset2;
						CopyMemory( modelname, funtechiesmissile, strlen( funtechiesmissile ) + 2 );
					}
				}
			}
			else
			{
				TextPrint( "Error! Bad class!", 5.0f );
			}
		}
		else
		{
			int ClassAddr = GetTypeInfo( 'H00K', 0 );
			if ( ClassAddr > 0 )
			{
				int offset2 = ClassAddr + 0x44;
				offset2 = *( int* )offset2;
				if ( offset2 )
				{

					offset2 = *( int* )offset2;
					if ( offset2 )
					{
						char * modelname = 0;
						*( int* )&modelname = offset2;
						CopyMemory( modelname, OldTechiesMissile, oldlen );
						//modelname[ strlen( modelname ) + 1 ] = '\xA';
					}
				}
			}
			else
			{
				TextPrint( "Error! Bad class!", 5.0f );
			}
		}
	}
}


int techiesoldaddr = 0;
int ownunitaddr = 0;
int NeedOldTechies = 0;

void SetTechiesNewAddrForAlly( )
{
	NeedOldTechies = 1;
	if ( TechiesType == 1 )
	{
		if ( ownunitaddr == 0 )
		{
			int ownunit = GetSelectedOwnedUnit( );
			if ( IsNotBadUnit( ownunit ) && IsHero( ownunit ) )
			{
				techiesoldaddr = techiesaddr;
				techiesaddr = ownunit;
				ownunitaddr = ownunit;
			}
		}
		else if ( !IsNotBadUnit( ownunitaddr, 1 ) || ( !IsHero( ownunitaddr ) && !IsUnitDead( ownunitaddr ) ) )
		{
			ownunitaddr = 0;
		}
	}
}

void SetOldTechiesAddr( )
{
	if ( NeedOldTechies && TechiesType == 1 )
	{
		NeedOldTechies = 0;
		if ( techiesoldaddr != 0 )
			techiesaddr = techiesoldaddr;
	}
}

int TechiesEnemyTimer = 300;

void JustTechies( )
{
	LatestTime = time( NULL );
retryall:
	if ( IsGame( ) )
	{
#ifdef BOTDEBUG
		PrintDebugInfo( "InGame" );
#endif
		if ( !IsWindowActive( ) )
		{
			while ( !IsWindowActive( ) )
			{
				Sleep( 200 );
				LatestTime = time( NULL );
			}
			Sleep( 1000 );
			goto retryall;
		}
#ifdef BOTDEBUG
		PrintDebugInfo( "Window active" );
#endif
		LatestTime = time( NULL );

		if ( !TechiesFound || techiesaddr == 0 || !IsOkayPtr( ( void* )techiesaddr ) )
		{
#ifdef BOTDEBUG
			PrintDebugInfo( "Find techies!" );
#endif
			TechiesFound = 0;
			techiesaddr = 0;


			int * unitsarray = 0;
			int * UnitsCount = GetUnitCountAndUnitArray( &unitsarray );
			//Поиск течиса
			if ( UnitsCount > 0 && *( int* )UnitsCount > 0 && unitsarray )
			{

				for ( int i = 0; i < *( int* )UnitsCount; i++ )
				{

					latestunit = unitsarray[ i ];
					if ( unitsarray[ i ] )
					{
#ifdef BOTDEBUG
						PrintDebugInfo2( "UnitArrayFound", unitsarray[ i ] );
#endif
						if ( IsNotBadUnit( unitsarray[ i ] ) )
						{
#ifdef BOTDEBUG
							PrintDebugInfo( "UnitOkay" );
#endif
							if ( IsHero( unitsarray[ i ] ) > 0 )
							{
								if ( IsClassEqual( unitsarray[ i ], "H00K" ) )
								{
									TechiesSlot = GetUnitOwnerSlot( unitsarray[ i ] );
									if ( GetLocalPlayerNumber( ) == TechiesSlot || !IsPlayerEnemy( unitsarray[ i ] ) )
									{
										if ( TechiesType == 0 )
										{
											TextPrint( GetUnitName( unitsarray[ i ] ), 5.0f );
										}
										//PrintClassAddress( unitsarray[ i ] );
#ifdef BOTDEBUG
										PrintDebugInfo( "ClassEqual" );
#endif
										techiesaddr = unitsarray[ i ];
										TechiesFound = 1;
										if ( GetLocalPlayerNumber( ) == TechiesSlot )
										{
											TechiesType = 0;
#ifdef BOTDEBUG
											PrintDebugInfo( "FoundTechies" );
#endif

											//	if ( TechiesType == 0 )
											{
												TextPrint( "|cFF00FF00Found!|r", 3.0f );
												if ( EnableAutoExplode == 1 )
												{
													TextPrint( "AutoExplode: |cFFEF2020AutoExplode|r", 3.0f );
												}
												else if ( EnableAutoExplode == 0 )
												{
													TextPrint( "AutoExplode: |cFF00FF00AutoKill|r ", 3.0f );
												}
												else if ( EnableAutoExplode == 2 )
												{
													TextPrint( "AutoExplode: |cFFFF0000Disabled|r ", 3.0f );
												}
												if ( EnableForceStaff )
												{
													TextPrint( "ForceStaff: |cFF00FF00ENABLED|r", 3.0f );
												}
												else if ( !EnableForceStaff )
												{
													TextPrint( "ForceStaff: |cFFFF0000DISABLED|r", 3.0f );
												}
												if ( EnableDagger )
												{
													TextPrint( "Dagger: |cFF00FF00ENABLED|r", 3.0f );
												}
												else if ( !EnableDagger )
												{
													TextPrint( "Dagger: |cFFFF0000DISABLED|r", 3.0f );
												}
											}
											break;
											/*	float x;
											float y;
											float dur = 3.0f;
											GetUnitLocation3D( techiesaddr , &x , &y );
											PingMinimap( &x , &y , &dur );*/
										}
										else
										{
											if ( !IsPlayerEnemy( unitsarray[ i ] ) )
											{
												TechiesType = 1;
												TextPrint( "|cFFFF7C00Found ally techies|r", 3.0f );
												if ( EnableForceStaff )
												{
													TextPrint( "ForceStaff: |cFF00FF00ENABLED|r", 3.0f );
												}
												else if ( !EnableForceStaff )
												{
													TextPrint( "ForceStaff: |cFFFF0000DISABLED|r", 3.0f );
												}
												if ( EnableDagger )
												{
													TextPrint( "Dagger: |cFF00FF00ENABLED|r", 3.0f );
												}
												else if ( !EnableDagger )
												{
													TextPrint( "Dagger: |cFFFF0000DISABLED|r", 3.0f );
												}
											}
											else
											{
												TechiesType = 2;
											}
										}
									}
								}
							}

						}
						else if ( !IsNotBadUnit( unitsarray[ i ], 1 ) )
						{
							break;
						}
					}

				}


				if ( !TechiesFound )
				{

					techiesaddr = 0;
#ifdef BOTDEBUG
					PrintDebugInfo( "No techies found" );
#endif
					Sleep( 2000 );
					return;
				}
			}



		}
		else
		{
			if ( TechiesSlot != GetUnitOwnerSlot( techiesaddr ) )
			{
				TechiesFound = 0;
				techiesaddr = 0;
				TextPrint( "Techies: |cFF00FF00System reloaded|r", 3.0f );
			}
		}
		LatestTime = time( NULL );
		if ( TechiesFound )
		{

#ifdef BOTDEBUG
			PrintDebugInfo( "techies found" );
#endif
			// Очистить список мин
			BombList.clear( );

			// Сохранить список мин

			if ( TechiesFound )
			{
#ifdef BOTDEBUG
				PrintDebugInfo( "Find mines" );
#endif

				int * unitsarray = 0;
				int * UnitsCount = GetUnitCountAndUnitArray( &unitsarray );
				if ( UnitsCount > 0 && *( int* )UnitsCount > 0 && unitsarray )
				{

					for ( int i = 0; i < *( int* )UnitsCount; i++ )
					{

						latestunit = unitsarray[ i ];
#ifdef BOTDEBUG
						PrintDebugInfo2( "UnitFOund", unitsarray[ i ] );
#endif
						if ( /* !IsIgnoreUnit( unitsarray[ i ] ) &&*/ IsNotBadUnit( unitsarray[ i ] ) && !IsHero( unitsarray[ i ] ) && IsHero( unitsarray[ i ] ) != -1 )
						{
#ifdef BOTDEBUG
							PrintDebugInfo( "NormalUnit33" );
#endif
							if ( GetLocalPlayerNumber( ) == GetUnitOwnerSlot( unitsarray[ i ] ) )
							{


								// 1lvl 1skill = n00O

								// 2lvl 1skill = n00P

								// 3lvl 1skill = n00Q

								// 4lvl 1skill = n00N

								// 1lvl 4skill = o018

								// 2lvl 4skill = o002

								// 3lvl 4skill = o00B

								// 4lvl 4skill = o01B



								if ( IsClassEqual( unitsarray[ i ], "n00O" ) )
								{
									BombStruct tmpstr = BombStruct( );
									tmpstr.unitaddr = unitsarray[ i ];
									tmpstr.dmg = 300.0f;
									tmpstr.range1 = 200.0f;
									tmpstr.range2 = 500.0f;
									tmpstr.remote = 0;
									GetUnitLocation3D( unitsarray[ i ], &tmpstr.x, &tmpstr.y, &tmpstr.z );
									/*float durdur = 0.10f;
									PingMinimap( &tmpstr.x , &tmpstr.y , &durdur );*/
									BombList.push_back( tmpstr );
								}


								if ( IsClassEqual( unitsarray[ i ], "n00P" ) )
								{
									BombStruct tmpstr = BombStruct( );
									tmpstr.unitaddr = unitsarray[ i ];
									tmpstr.dmg = 400.0f;
									tmpstr.range1 = 200.0f;
									tmpstr.range2 = 500.0f;
									tmpstr.remote = 0;
									GetUnitLocation3D( unitsarray[ i ], &tmpstr.x, &tmpstr.y, &tmpstr.z );
									/*float durdur = 0.10f;
									PingMinimap( &tmpstr.x , &tmpstr.y , &durdur );*/
									BombList.push_back( tmpstr );
								}

								if ( IsClassEqual( unitsarray[ i ], "n00Q" ) )
								{
									BombStruct tmpstr = BombStruct( );
									tmpstr.unitaddr = unitsarray[ i ];
									tmpstr.dmg = 500.0f;
									tmpstr.range1 = 200.0f;
									tmpstr.range2 = 500.0f;
									tmpstr.remote = 0;
									GetUnitLocation3D( unitsarray[ i ], &tmpstr.x, &tmpstr.y, &tmpstr.z );
									/*float durdur = 0.10f;
									PingMinimap( &tmpstr.x , &tmpstr.y , &durdur );*/
									BombList.push_back( tmpstr );
								}


								if ( IsClassEqual( unitsarray[ i ], "n00N" ) )
								{
									BombStruct tmpstr = BombStruct( );
									tmpstr.unitaddr = unitsarray[ i ];
									tmpstr.dmg = 600.0f;
									tmpstr.range1 = 200.0f;
									tmpstr.range2 = 500.0f;
									tmpstr.remote = 0;
									GetUnitLocation3D( unitsarray[ i ], &tmpstr.x, &tmpstr.y, &tmpstr.z );
									/*float durdur = 0.10f;
									PingMinimap( &tmpstr.x , &tmpstr.y , &durdur );*/
									BombList.push_back( tmpstr );
								}

								if ( TechiesType == 0 )
								{

									if ( IsClassEqual( unitsarray[ i ], "o018" ) )
									{
										BombStruct tmpstr = BombStruct( );
										tmpstr.unitaddr = unitsarray[ i ];
										tmpstr.dmg = 300.0f;
										tmpstr.range1 = 399.0f;
										tmpstr.range2 = 0.0f;
										tmpstr.remote = 1;
										GetUnitLocation3D( unitsarray[ i ], &tmpstr.x, &tmpstr.y, &tmpstr.z );
										/*	float durdur = 0.10f;
										PingMinimap( &tmpstr.x , &tmpstr.y , &durdur );*/
										BombList.push_back( tmpstr );
									}

									if ( IsClassEqual( unitsarray[ i ], "o002" ) )
									{
										BombStruct tmpstr = BombStruct( );
										tmpstr.unitaddr = unitsarray[ i ];
										tmpstr.dmg = 450.0f;
										tmpstr.range1 = 409.0f;
										tmpstr.range2 = 0.0f;
										tmpstr.remote = 1;
										GetUnitLocation3D( unitsarray[ i ], &tmpstr.x, &tmpstr.y, &tmpstr.z );
										/*float durdur = 0.10f;
										PingMinimap( &tmpstr.x , &tmpstr.y , &durdur );*/
										BombList.push_back( tmpstr );
									}

									if ( IsClassEqual( unitsarray[ i ], "o00B" ) )
									{
										BombStruct tmpstr = BombStruct( );
										tmpstr.unitaddr = unitsarray[ i ];
										tmpstr.dmg = 600.0f;
										tmpstr.range1 = 424.0f;
										tmpstr.range2 = 0.0f;
										tmpstr.remote = 1;
										GetUnitLocation3D( unitsarray[ i ], &tmpstr.x, &tmpstr.y, &tmpstr.z );
										/*float durdur = 0.10f;
										PingMinimap( &tmpstr.x , &tmpstr.y , &durdur );*/
										BombList.push_back( tmpstr );
									}

									if ( IsClassEqual( unitsarray[ i ], "o01B" ) )
									{
										BombStruct tmpstr = BombStruct( );
										tmpstr.unitaddr = unitsarray[ i ];
										tmpstr.dmg = 750.0f;
										tmpstr.range1 = 424.0f;
										tmpstr.range2 = 0.0f;
										tmpstr.remote = 1;
										GetUnitLocation3D( unitsarray[ i ], &tmpstr.x, &tmpstr.y, &tmpstr.z );
										/*float durdur = 0.10f;
										PingMinimap( &tmpstr.x , &tmpstr.y , &durdur );*/
										BombList.push_back( tmpstr );
									}

								}
							}
						}
						else if ( !IsNotBadUnit( unitsarray[ i ], 1 ) )
						{
							break;
						}
					}
					//Поиск течиса


				}
#ifdef BOTDEBUG
				PrintDebugInfo( "End find mines" );
#endif
			}

			if ( TechiesType == 0 )
			{
				if ( IsKeyPressed( 0x58 ) && IsKeyPressed( 0x31 ) )
				{
					while ( IsKeyPressed( 0x58 ) && IsKeyPressed( 0x31 ) )
					{
						Sleep( 10 );
						LatestTime = time( NULL );
					}
					EnableAutoExplode++;

					if ( EnableAutoExplode > 2 )
						EnableAutoExplode = 0;

					if ( EnableAutoExplode == 1 )
					{
						TextPrint( "AutoExplode: |cFFEF2020AutoExplode|r", 3.0f );
					}
					else if ( EnableAutoExplode == 0 )
					{
						TextPrint( "AutoExplode: |cFF00FF00AutoKill|r ", 3.0f );
					}
					else if ( EnableAutoExplode == 2 )
					{
						TextPrint( "AutoExplode: |cFFFF0000Disabled|r ", 3.0f );
					}
					return;
				}
			}

			if ( IsKeyPressed( 0x58 ) && IsKeyPressed( 0x32 ) )
			{
				while ( IsKeyPressed( 0x58 ) && IsKeyPressed( 0x32 ) )
				{
					LatestTime = time( NULL );
					Sleep( 10 );
				}
				EnableForceStaff = !EnableForceStaff;

				if ( EnableForceStaff )
				{
					TextPrint( "ForceStaff: |cFF00FF00ENABLED|r", 3.0f );
				}
				else if ( !EnableForceStaff )
				{
					TextPrint( "ForceStaff: |cFFFF0000DISABLED|r", 3.0f );
				}
				return;
			}


			if ( IsKeyPressed( 0x58 ) && IsKeyPressed( 0x33 ) )
			{
				while ( IsKeyPressed( 0x58 ) && IsKeyPressed( 0x33 ) )
				{
					LatestTime = time( NULL );
					Sleep( 10 );
				}
				EnableDagger = !EnableDagger;

				if ( EnableDagger )
				{
					TextPrint( "Dagger: |cFF00FF00ENABLED|r", 3.0f );
				}
				else if ( !EnableDagger )
				{
					TextPrint( "Dagger: |cFFFF0000DISABLED|r", 3.0f );
				}
				return;
			}
			if ( ExpertModeEnabled )
			{
				if ( IsKeyPressed( 0x58 ) && IsKeyPressed( 0x34 ) )
				{
					while ( IsKeyPressed( 0x58 ) && IsKeyPressed( 0x34 ) )
					{
						LatestTime = time( NULL );
						Sleep( 10 );
					}

					int tempbool = !StealthMode;

					StealthMode = 0;


					if ( tempbool )
					{
						TextPrint( "Stealt hMode : |cFF00FF00ENABLED|r", 3.0f );
					}
					else if ( !tempbool )
					{
						TextPrint( "Stealth Mode : |cFFFF0000DISABLED|r", 3.0f );
					}

					StealthMode = tempbool;
					return;
				}

			}

			if ( ExpertModeEnabled )
			{
				if ( IsKeyPressed( 0x58 ) && IsKeyPressed( 0x35 ) )
				{
					while ( IsKeyPressed( 0x58 ) && IsKeyPressed( 0x35 ) )
					{
						LatestTime = time( NULL );
						Sleep( 10 );
					}
					UseWarnIsBadReadPtr++;
					if ( UseWarnIsBadReadPtr > 3 )
						UseWarnIsBadReadPtr = 1;

					if ( UseWarnIsBadReadPtr == 2 )
					{
						TextPrint( "[SLOW BUT STABLE]|cFFFFCC00Use VirtualQuery to detect bad memory!|r", 3.0f );
					}
					else if ( UseWarnIsBadReadPtr == 1 )
					{
						TextPrint( "[FAST]|cFFFF5B00Use IsBadReadPtr (__try -catch) to detect bad memory!|r", 3.0f );
					}
					else if ( UseWarnIsBadReadPtr == 3 )
					{
						TextPrint( "[VERY FAST]|cFFFF0000Disable bad memory detection!|r", 3.0f );
					}
					return;
				}
			}

			if ( ExpertModeEnabled )
			{
				if ( IsKeyPressed( 0x58 ) && IsKeyPressed( 0x36 ) )
				{
					while ( IsKeyPressed( 0x58 ) && IsKeyPressed( 0x36 ) )
					{
						LatestTime = time( NULL );
						Sleep( 10 );
					}
					DisableWindowActiveCheck = !DisableWindowActiveCheck;

					if ( !DisableWindowActiveCheck )
					{
						TextPrint( "Work only when window active : |cFF00FF00ENABLED|r", 3.0f );
					}
					else if ( DisableWindowActiveCheck )
					{
						TextPrint( "Work only when window active : |cFFFF0000DISABLED|r", 3.0f );
					}
					return;
				}
			}

			if ( ExpertModeEnabled )
			{

				if ( IsKeyPressed( 0x58 ) && IsKeyPressed( 0x37 ) )
				{
					while ( IsKeyPressed( 0x58 ) && IsKeyPressed( 0x37 ) )
					{
						LatestTime = time( NULL );
						Sleep( 10 );
					}
					Enable3DPoint = !Enable3DPoint;

					if ( Enable3DPoint )
					{
						TextPrint( "3D Point System : |cFF00FF00ENABLED|r", 3.0f );
					}
					else if ( !Enable3DPoint )
					{
						TextPrint( "3D Point System : |cFFFF0000DISABLED|r", 3.0f );
					}
					return;
				}
			}

			if ( IsKeyPressed( 0x58 ) && IsKeyPressed( 0x38 ) )
			{
				while ( IsKeyPressed( 0x58 ) && IsKeyPressed( 0x38 ) )
				{
					LatestTime = time( NULL );
					Sleep( 10 );
				}
				ExpertModeEnabled = !ExpertModeEnabled;

				if ( ExpertModeEnabled )
				{
					TextPrint( "Expert mode: |cFF00FF00ENABLED|r", 3.0f );
				}
				else if ( !ExpertModeEnabled )
				{
					TextPrint( "Expert mode: |cFFFF0000DISABLED|r", 3.0f );
				}
				return;
			}


			if ( ExpertModeEnabled )
			{

				if ( IsKeyPressed( 0x58 ) && IsKeyPressed( 0x39 ) )
				{
					while ( IsKeyPressed( 0x58 ) && IsKeyPressed( 0x39 ) )
					{
						LatestTime = time( NULL );
						Sleep( 10 );
					}
					FunModeEnabled = !FunModeEnabled;

					if ( FunModeEnabled )
					{
						TextPrint( "Fun mode: |cFF00FF00ENABLED|r", 3.0f );
					}
					else if ( !FunModeEnabled )
					{
						TextPrint( "Fun mode: |cFFFF0000DISABLED|r", 3.0f );
					}
					TechiesFunMode( FunModeEnabled );
					return;
				}
			}

#ifdef BOTDEBUG
			if ( IsKeyPressed( 0x58 ) && IsKeyPressed( '-' ) )
			{
				while ( IsKeyPressed( 0x58 ) && IsKeyPressed( '-' ) )
				{
					LatestTime = time( NULL );
					Sleep( 10 );
				}
				DebugActive = !DebugActive;

				if ( DebugActive )
				{
					InitDebug( );
					TextPrint( "Debug system : ENABLED", 3.0f );
					TextPrint( "Warning! This can make slow WAR3 playing.", 3.0f );
				}
				else if ( !DebugActive )
				{
					TextPrint( "Debug system : DISABLED", 3.0f );
				}
				return;
			}
#endif

			if ( !IsGame( ) )
				return;

			int DefaultUnitCount = GetUnitCount( );

			if ( EnableAutoExplode == 1 && TechiesType == 0 )
			{
#ifdef BOTDEBUG
				PrintDebugInfo( "Init autoexplode" );
#endif
				int * unitsarray = 0;
				int * UnitsCount = GetUnitCountAndUnitArray( &unitsarray );

				if ( UnitsCount > 0 && *( int* )UnitsCount > 0 && unitsarray )
				{

					for ( int i = 0; i < *( int* )UnitsCount; i++ )
					{

						latestunit = unitsarray[ i ];
						int skipped = 0;
						goto skip2try;

					try2:;

						skipped = 1;
						Sleep( 200 );

						if ( !IsGame( ) || DefaultUnitCount != GetUnitCount( ) )
							return;

					skip2try:;
#ifdef BOTDEBUG
						PrintDebugInfo2( "UnitCOunt333", unitsarray[ i ] );
#endif
						if ( IsNotBadUnit( unitsarray[ i ] ) && IsHero( unitsarray[ i ] ) )
						{
#ifdef BOTDEBUG
							PrintDebugInfo( "OkayUnitLOLOLO3" );
#endif
							if ( GetLocalPlayerNumber( ) != GetUnitOwnerSlot( unitsarray[ i ] ) )
							{
								if ( IsPlayerEnemy( unitsarray[ i ] ) && IsUnitVisibleToPlayer( unitsarray[ i ], GetLocalPlayer( ) ) && GetDmgForUnit( unitsarray[ i ], 600.0f ) > 0.0f )
								{



#ifdef BOTDEBUG
									PrintDebugInfo( "Found enemy" );
#endif
									float unitx = 0.0f, unity = 0.0f, unitz = 0.0f;
									GetUnitLocation3D( unitsarray[ i ], &unitx, &unity, &unitz );
									unitstoselect.clear( );
									for ( unsigned int n = 0; n < BombList.size( ); n++ )
									{
										if ( !BombList[ n ].remote )
											continue;

										if ( Distance3D( unitx, unity, unitz, BombList[ n ].x, BombList[ n ].y, BombList[ n ].z ) < BombList[ n ].range1 )
										{
											unitstoselect.push_back( BombList[ n ].unitaddr );
										}
									}

									if ( unitstoselect.size( ) > 0 )
									{
										if ( !skipped && !ExpertModeEnabled )
										{
											goto try2;
										}


										TextPrintUnspammed( "[AutoExplode!]" );



#ifdef BOTDEBUG
										PrintDebugInfo( "Need explode" );
#endif
										while ( SelectAllUnits( ) )
										{

#ifdef BOTDEBUG
											PrintDebugInfo( "Units selected" );
#endif
											//	Sleep( 5 );

											UseDetonator( );
										}

										//	Sleep( 2 );

#ifdef BOTDEBUG
										PrintDebugInfo( "Detonator used" );
#endif
										//	Sleep( 5 );
										unitstoselect.clear( );


										SelectTechies( );
										if ( ExpertModeEnabled )
											Sleep( 20 );
										else
											Sleep( 50 );
										return;
									}
								}
							}

						}
						else if ( !IsNotBadUnit( unitsarray[ i ], 1 ) )
						{
							break;
						}
					}



				}

			}
			else if ( EnableAutoExplode == 0 && TechiesType == 0 )
			{
#ifdef BOTDEBUG
				PrintDebugInfo( "Init autokill" );
#endif
				int * unitsarray = 0;
				int * UnitsCount = GetUnitCountAndUnitArray( &unitsarray );

				if ( UnitsCount > 0 && *( int* )UnitsCount > 0 && unitsarray )
				{

					for ( int i = 0; i < *( int* )UnitsCount; i++ )
					{

						latestunit = unitsarray[ i ];
						int skipped = 0;
						goto skip22try;
					try22:;
						skipped = 1;
						Sleep( 200 );
					skip22try:;
						if ( !IsGame( ) || DefaultUnitCount != GetUnitCount( ) )
							return;

#ifdef BOTDEBUG
						PrintDebugInfo( "UnitGetGetCCCC" );
#endif
						if ( IsNotBadUnit( unitsarray[ i ] ) && IsHero( unitsarray[ i ] ) )
						{
#ifdef BOTDEBUG
							PrintDebugInfo2( "UnitGetGetCCCC", unitsarray[ i ] );
#endif
							if ( GetLocalPlayerNumber( ) != GetUnitOwnerSlot( unitsarray[ i ] ) )
							{
								if ( IsPlayerEnemy( unitsarray[ i ] ) && IsUnitVisibleToPlayer( unitsarray[ i ], GetLocalPlayer( ) ) )
								{
#ifdef BOTDEBUG
									PrintDebugInfo( "Found emeny" );
#endif
									float okaydmg = 0.0f;
									float unitx = 0.0f, unity = 0.0f, unitz = 0.0f;
									GetUnitLocation3D( unitsarray[ i ], &unitx, &unity, &unitz );
									/*float durdur = 0.19f;
									PingMinimap( &unitx , &unity , &durdur );*/
									unitstoselect.clear( );

									int BombsFound = 0;

									int xxokaydmg = 0;

									for ( unsigned int n = 0; n < BombList.size( ); n++ )
									{
										if ( !BombList[ n ].remote )
											continue;

										if ( Distance3D( unitx, unity, unitz, BombList[ n ].x, BombList[ n ].y, BombList[ n ].z ) < BombList[ n ].range1 )
										{
											BombsFound++;
											unitstoselect.push_back( BombList[ n ].unitaddr );
											okaydmg += GetDmgForUnit( unitsarray[ i ], BombList[ n ].dmg );
											if ( okaydmg > GetUnitHP( unitsarray[ i ] ) )
											{
												xxokaydmg = 1;
												break;
											}
										}
									}

									if ( unitstoselect.size( ) > 0 )
									{
#ifdef BOTDEBUG
										PrintDebugInfo( "Need kill 1" );
#endif
										if ( xxokaydmg )
										{


											char * printdata = new char[ 1024 ];
											sprintf_s( printdata, 1024, "[AutoKill->OK]: [ %s ]|cFFFFFFFFHP: |r|cFFFF0000%i|r|cFFFFFFFF. DMG: |r|cFFFFCC00%i|r. Count: %i\n%s", GetUnitName( unitsarray[ i ] ), ( int )GetUnitHP( unitsarray[ i ] ), ( int )okaydmg, BombsFound, bufflist.size( ) > 0 ? bufflist.c_str( ) : "No buff found" );
											TextPrintUnspammed( printdata );
											delete[ ]printdata;

											if ( !skipped && !ExpertModeEnabled )
											{
												goto try22;
											}
#ifdef BOTDEBUG
											PrintDebugInfo( "Need kill 2" );
#endif
											/*	for ( unsigned int x = 0; x < unitstoselect.size( ); x++ )
											{
											AddUnitToIgnore( unitstoselect[ x ] );
											}*/
#ifdef BOTDEBUG
											PrintDebugInfo( "SelectUnits" );
#endif
											while ( SelectAllUnits( ) )
											{
#ifdef BOTDEBUG
												PrintDebugInfo( "UnitsSelected" );
#endif
												//		Sleep( 5 );

												UseDetonator( );
											}
											//Sleep( 2 );

#ifdef BOTDEBUG
											PrintDebugInfo( "Detonate" );
#endif
											//		Sleep( 5 );

											unitstoselect.clear( );


#ifdef BOTDEBUG
											PrintDebugInfo( "Select techies" );
#endif
											SelectTechies( );

											if ( ExpertModeEnabled )
												Sleep( 30 );
											else
												Sleep( 50 );
											return;
										}
										else
										{
											char * printdata = new char[ 1024 ];
											sprintf_s( printdata, 1024, "[AutoKill]: [ %s ]|cFFFFFFFFHP: |r|cFFFF0000%i|r|cFFFFFFFF. DMG: |r|cFFFFCC00%i|r. Count: %i\n%s", GetUnitName( unitsarray[ i ] ), ( int )GetUnitHP( unitsarray[ i ] ), ( int )okaydmg, BombsFound, bufflist.size( ) > 0 ? bufflist.c_str( ) : "No buff found" );
											TextPrintUnspammed( printdata );
											delete[ ]printdata;
										}

									}
								}
							}

						}
						else if ( !IsNotBadUnit( unitsarray[ i ], 1 ) )
						{
							break;
						}
					}



				}

			}


			if ( TechiesType == 1 )
			{
				if ( GetSelectedOwnedUnit( ) > 0 )
				{
					if ( !IsNotBadUnit( GetSelectedOwnedUnit( ) ) && IsHero( GetSelectedOwnedUnit( ) ) )
					{
						techiesaddr = GetSelectedOwnedUnit( );
					}
				}

			}


			if ( EnableForceStaff && techiesaddr > 0 && EnableAutoExplode == 0 )
			{
				if ( forcestaffcooldown > 0 )
				{
					forcestaffcooldown--;
				}
				if ( daggercooldown > 0 )
				{
					daggercooldown--;
				}

				if ( TechiesType == 1 )
					SetTechiesNewAddrForAlly( );
#ifdef BOTDEBUG
				PrintDebugInfo( "Init forcestaff" );
#endif
				if ( IsNotBadUnit( techiesaddr ) )
				{
#ifdef BOTDEBUG
					PrintDebugInfo( "dbg111" );
#endif
					//SelectAllUnits( unitstoselect , unitstoselect.size( ) );
				}
				else
				{
#ifdef BOTDEBUG
					PrintDebugInfo( "dbg222" );
#endif
					if ( !IsHero( techiesaddr ) && !IsUnitDead( techiesaddr ) )
					{
						techiesaddr = 0;
					}
					SetOldTechiesAddr( );
					return;
				}
				/*int selectedunit1 = GetSelectedOwnedUnit( );
				if ( selectedunit1 == 0 || ( techiesaddr != selectedunit1 && TechiesType == 0 ) )
				{
				goto skipforcestaff;
				}*/
#ifdef BOTDEBUG
				PrintDebugInfo( "FindForcestaff1" );
#endif


				int ForceStaffFound = 0;
				int forcestaffitemid = 0;
				int itemaddr = 0;
				for ( int i = 0; i < 6; i++ )
				{
					itemaddr = GetItemInSlot( techiesaddr, 0, i );
					if ( itemaddr > 0 && IsClassEqual( itemaddr, "I0HI" ) )
					{
						if ( IsAbilityCooldown( techiesaddr, 'A19M' ) )
						{
							if ( !_Forcecooldown )
							{
								TextPrint( "|cFFFF6700F|r|cFFFE6401o|r|cFFFD6102r|r|cFFFC5E03c|r|cFFFB5A04e|r|cFFFA5705s|r|cFFF95406t|r|cFFF95107a|r|cFFF84E08f|r|cFFF74B09f|r|cFFF6470A |r|cFFF5440Bc|r|cFFF4410Co|r|cFFF33E0Co|r|cFFF23B0Dl|r|cFFF1380Ed|r|cFFF0340Fo|r|cFFEF3110w|r|cFFEE2E11n|r|cFFEE2B12 |r|cFFED2813s|r|cFFEC2514t|r|cFFEB2115a|r|cFFEA1E16r|r|cFFE91B17t|r|cFFE81818.|r", 3.0f );
								_Forcecooldown = 1;
							}
						}
						else
						{
							if ( _Forcecooldown )
							{
								_Forcecooldown = 0;
								TextPrint( "|cFF21FF00F|r|cFF31FF00o|r|cFF42FF00r|r|cFF52FF00c|r|cFF63FF00e|r|cFF73FF00s|r|cFF84FF00t|r|cFF94FF00a|r|cFFA5FF00f|r|cFFB5FF00f|r|cFFC6FF00 |r|cFFD6FF00co|r|cFFC6FF00o|r|cFFB5FF00l|r|cFFA5FF00d|r|cFF94FF00o|r|cFF84FF00w|r|cFF73FF00n|r|cFF63FF00 |r|cFF52FF00e|r|cFF42FF00n|r|cFF31FF00d|r|cFF21FF00.|r", 3.0f );
								Sleep( 20 );
								if ( !IsGame( ) || DefaultUnitCount != GetUnitCount( ) )
									return;
							}
							ForceStaffFound = 1;
							forcestaffitemid = i;
						}

						break;
					}
				}
#ifdef BOTDEBUG
				PrintDebugInfo( "FindDagger1" );
#endif
				int DaggerFound = 0;
				int daggeritemid = 0;
				int daggeritemaddr = 0;

				if ( !EnableDagger )
				{

				}
				else
				{
					for ( int i = 0; i < 6; i++ )
					{
						daggeritemaddr = GetItemInSlot( techiesaddr, 0, i );
						if ( daggeritemaddr > 0 && IsClassEqual( daggeritemaddr, "I04H" ) )
						{

							if ( IsAbilityCooldown( techiesaddr, 'AIbk' ) )
							{
								if ( !_Daggercooldown )
								{
									TextPrint( "|cFFFF6700D|r|cFFFE6301a|r|cFFFD5F02g|r|cFFFC5C03g|r|cFFFB5805e|r|cFFFA5406r|r|cFFF85007 |r|cFFF74D08c|r|cFFF64909o|r|cFFF5450Ao|r|cFFF4410Bl|r|cFFF33E0Dd|r|cFFF23A0Eo|r|cFFF1360Fw|r|cFFF03210n|r|cFFEF2F11 |r|cFFED2B12s|r|cFFEC2713t|r|cFFEB2315a|r|cFFEA2016r|r|cFFE91C17t|r|cFFE81818.|r", 3.0f );
									_Daggercooldown = 1;
								}

							}
							else
							{
								if ( _Daggercooldown )
								{
									_Daggercooldown = 0;
									TextPrint( "|cFF21FF00D|r|cFF35FF00a|r|cFF49FF00g|r|cFF5DFF00g|r|cFF71FF00e|r|cFF86FF00r|r|cFF9AFF00 |r|cFFAEFF00c|r|cFFC2FF00o|r|cFFD6FF00ol|r|cFFC2FF00d|r|cFFAEFF00o|r|cFF9AFF00w|r|cFF86FF00n|r|cFF71FF00 |r|cFF5DFF00e|r|cFF49FF00n|r|cFF35FF00d|r|cFF21FF00.|r", 3.0f );
									Sleep( 20 );
									if ( !IsGame( ) || DefaultUnitCount != GetUnitCount( ) )
										return;
								}

								DaggerFound = 1;
								daggeritemid = i;
							}


							break;
						}
					}
				}

				if ( _Forcecooldown )
				{
					goto skipforcestaff;
				}

				if ( ForceStaffFound )
				{
#ifdef BOTDEBUG
					PrintDebugInfo( "Found force staff" );
#endif
					int * unitsarray = 0;
					int * UnitsCount = GetUnitCountAndUnitArray( &unitsarray );

					if ( UnitsCount > 0 && *( int* )UnitsCount > 0 && unitsarray )
					{
#ifdef BOTDEBUG
						PrintDebugInfo( "GetUnitarray22" );
#endif
						for ( int i = 0; i < *( int* )UnitsCount; i++ )
						{

							latestunit = unitsarray[ i ];
#ifdef BOTDEBUG
							PrintDebugInfo2( "UnitGetdata", unitsarray[ i ] );
#endif

							if ( IsNotBadUnit( unitsarray[ i ] ) )
							{
#ifdef BOTDEBUG
								PrintDebugInfo( "UnitOkka##!!!" );
#endif

								if ( IsHero( unitsarray[ i ] ) )
								{
									if ( GetLocalPlayerNumber( ) != GetUnitOwnerSlot( unitsarray[ i ] ) )
									{
										if ( IsPlayerEnemy( unitsarray[ i ] ) && IsUnitVisibleToPlayer( unitsarray[ i ], GetLocalPlayer( ) ) )
										{
#ifdef BOTDEBUG
											PrintDebugInfo( "#555:OK" );
#endif
											float unitface = GetUnitFacing( unitsarray[ i ] );

											float targetunitx = 0.0f, targetunity = 0.0f, targetunitz = 0.0f;
											GetUnitLocation3D( unitsarray[ i ], &targetunitx, &targetunity, &targetunitz );

											float techiesunitx = 0.0f, techiesunity = 0.0f, techiesunitz = 0.0f;
											GetUnitLocation3D( techiesaddr, &techiesunitx, &techiesunity, &techiesunitz );


											if ( unitface != 100.0f && ( Distance3D( techiesunitx, techiesunity, techiesunitz, targetunitx, targetunity, targetunitz ) < 650.0f || ( DaggerFound && Distance3D( techiesunitx, techiesunity, techiesunitz, targetunitx, targetunity, targetunitz ) < ( 700.0f + 940.0f ) ) ) )
											{
												if ( DaggerFound && Distance3D( techiesunitx, techiesunity, techiesunitz, targetunitx, targetunity, targetunitz ) < 650.0f )
												{
													DaggerFound = 0;
												}

												Location startenemyloc = Location( );
												startenemyloc.X = targetunitx;
												startenemyloc.Y = targetunity;
												Location endenemyloc = GiveNextLocationFromLocAndAngle( startenemyloc, 500.0f, unitface );
												float outdmg = 0.0f;
												float enemyhp = GetUnitHP( unitsarray[ i ] );
												for ( unsigned int n = 0; n < BombList.size( ); n++ )
												{
													/*if ( IsIgnoreUnit( BombList[ n ].unitaddr ) )
													continue;*/
													if ( !BombList[ n ].remote )
													{
														if ( Distance3D( endenemyloc.X, endenemyloc.Y, BombList[ n ].z, BombList[ n ].x, BombList[ n ].y, BombList[ n ].z ) < BombList[ n ].range1 )
														{
															outdmg += GetDmgForUnit( unitsarray[ i ], BombList[ n ].dmg, 2 );
														}
													}
													else if ( TechiesType == 0 )
													{
														if ( Distance3D( endenemyloc.X, endenemyloc.Y, BombList[ n ].z, BombList[ n ].x, BombList[ n ].y, BombList[ n ].z ) < ( BombList[ n ].range1 - 20.0f ) )
														{
															outdmg += GetDmgForUnit( unitsarray[ i ], BombList[ n ].dmg );
														}
													}
													/*	else if ( Distance3D( endenemyloc.X , endenemyloc.Y , BombList[ n ].z , BombList[ n ].x , BombList[ n ].y , BombList[ n ].z ) < BombList[ n ].range2 )
													{
													outdmg += ( BombList[ n ].dmg / 2.0f )* 0.80f;

													}*/
													if ( enemyhp < outdmg )
														break;
												}



												if ( enemyhp < outdmg && outdmg > 10.0f )
												{


#ifdef BOTDEBUG
													PrintDebugInfo( "FORCE IT!" );
#endif
													if ( SelectTechies( ) )
													{
														int scmd = GetCMDbyItemSlot( forcestaffitemid );

														Sleep( 10 );

														if ( IsNotBadUnit( unitsarray[ i ] ) && IsGame( ) )
														{
															float newunitface = GetUnitFacing( unitsarray[ i ] );

															if ( newunitface + 0.07 > unitface && newunitface - 0.07 < unitface )
															{

																if ( DaggerFound )
																{
																	int dagcmd = GetCMDbyItemSlot( daggeritemid );

																	ItemOrSkillPoint( dagcmd, daggeritemaddr, targetunitx + 1.0f, targetunity - 1.01f, 0x100002 );

																	Sleep( 10 );

																	if ( !IsGame( ) )
																	{
																		return;
																	}

																	CommandItemTarget( scmd, itemaddr, targetunitx, targetunity, unitsarray[ i ], 1 );

																	daggercooldown = 481;
																	forcestaffcooldown = 810;

																}
																else
																{
																	CommandItemTarget( scmd, itemaddr, targetunitx, targetunity, unitsarray[ i ] );

																	forcestaffcooldown = 810;
																}
															}
														}
													}
													else
													{
														SetOldTechiesAddr( );
														goto skipforcestaff;
													}
													Sleep( 20 );
													if ( !IsGame( ) || DefaultUnitCount != GetUnitCount( ) )
														return;
												}

											}
										}
									}
								}
							}
							else if ( !IsNotBadUnit( unitsarray[ i ], 1 ) )
							{
								break;
							}
						}
					}
					// ITEM ID : I0HI
					//Теперь снова найти течиса и проверить есть ли у него форсстаф


					//Если у течиса имеется форстаф то ищем героев в радиусе действия

					//Высчитываем угол поворота каждого из этих героев,если на пути встречаются мины
					//то толкнуть героя на них

				}
			}
			else
			{

			}

		skipforcestaff:;
			// X + 3
			if ( IsKeyPressed( 0x58 ) && IsKeyPressed( 0x34 ) && TechiesType == 0 )
			{
#ifdef BOTDEBUG
				PrintDebugInfo( "InotMouseKIll" );
#endif
				float x = 0;
				float y = 0;
				float z = 0;
				GetMousePosition( &x, &y, &z );
				/*float dur = 0.2f;
				PingMinimap( &x , &y , &dur );*/



				unitstoselect.clear( );

				for ( unsigned int n = 0; n < BombList.size( ); n++ )
				{
					if ( !BombList[ n ].remote )
						continue;

					if ( Distance3D( x, y, z, BombList[ n ].x, BombList[ n ].y, BombList[ n ].z ) < 100 )
					{
						unitstoselect.push_back( BombList[ n ].unitaddr );
					}


				}

				if ( unitstoselect.size( ) > 0 )
				{
					/*for ( unsigned int x = 0; x < unitstoselect.size( ); x++ )
					{

					AddUnitToIgnore( unitstoselect[ x ] );
					}*/
#ifdef BOTDEBUG
					PrintDebugInfo( "Select units" );
#endif

					while ( SelectAllUnits( ) )
					{

#ifdef BOTDEBUG
						PrintDebugInfo( "Units selected!" );
#endif

						//	Sleep( 5 );

						UseDetonator( );
					}
					//Sleep( 2 );

#ifdef BOTDEBUG
					PrintDebugInfo( "Detonate used" );
#endif
					//	Sleep( 5 );
					unitstoselect.clear( );


#ifdef BOTDEBUG
					PrintDebugInfo( "Techies selected" );
#endif
					SelectTechies( );
					if ( ExpertModeEnabled )
						Sleep( 50 );
					else
						Sleep( 100 );
					if ( !IsGame( ) || DefaultUnitCount != GetUnitCount( ) )
						return;

				}



				Sleep( 10 );
				if ( !IsGame( ) || DefaultUnitCount != GetUnitCount( ) )
					return;
			}


		}

	}
}

int TechiesThread( )
{
	try
	{

		Sleep( 3500 );
		LatestTime = time( NULL );
		GameDll = ( int )GetModuleHandle( "Game.dll" );
		if ( !GameDll )
		{
			MessageBox( 0, "Error no game.dll found", "Error", MB_OK );
			return 1;
		}

		if ( GetModuleHandle( "UnrealTechiesBot_final.mix" ) ||
			GetModuleHandle( "UnrealTechiesBot_final_hotfix1.mix" ) ||
			GetModuleHandle( "UnrealTechiesBot_v14.mix" ) ||
			GetModuleHandle( "UnrealTechiesBot_v13.mix" ) ||
			GetModuleHandle( "UnrealTechiesBot_v12.mix" ) ||
			GetModuleHandle( "UnrealTechiesBot_v12_debug.mix" ) ||
			GetModuleHandle( "UnrealTechiesBot_v11_debug.mix" ) ||
			GetModuleHandle( "UnrealTechiesBot_v10_debug.mix" ) ||
			GetModuleHandle( "UnrealTechiesBot_v11.mix" ) ||
			GetModuleHandle( "UnrealTechiesBot_v10.mix" ) ||
			GetModuleHandle( "UnrealTechiesBot_v9.mix" ) ||
			GetModuleHandle( "UnrealTechiesBot_v8.mix" ) ||
			GetModuleHandle( "UnrealTechiesBot_v7.mix" ) ||
			GetModuleHandle( "UnrealTechiesBot_v6.mix" ) ||
			GetModuleHandle( "UnrealTechiesBot_v5.mix" ) ||
			GetModuleHandle( "UnrealTechiesBot_v4.mix" ) ||
			GetModuleHandle( "UnrealTechiesBot_v3.mix" ) ||
			GetModuleHandle( "UnrealTechiesBot_v2.mix" ) ||
			GetModuleHandle( "UnrealTechiesBot_v1.mix" ) ||
			GetModuleHandle( "UnrealTechiesBot.mix" ) ||
			GetModuleHandle( "SuperTechies.mix" ) ||
			GetModuleHandle( "TechiesBot.mix" ) ||
			GetModuleHandle( "Techies.mix" ) ||
			GetModuleHandle( "UnrealTechies.mix" ) ||
			GetModuleHandle( "UnrealTechiesBot_Final_FIX1.mix" ) ||
			GetModuleHandle( "UnrealTechiesBot_Final_FIX2.mix" ) ||
			GetModuleHandle( "UnrealTechiesBot_Final_FIX3.mix" ) ||
			GetModuleHandle( "UnrealTechiesBot_Final_FIX4.mix" ) ||
			GetModuleHandle( "UnrealTechiesBot_Final_FIX5.mix" ) ||
			GetModuleHandle( "UnrealTechiesBot_Final_FIX6.mix" ) ||
			GetModuleHandle( "UnrealTechiesBot_Final_FIX9.mix" ) ||
			GetModuleHandle( "UnrealTechiesBot_Final_FIX10.mix" ) ||
			GetModuleHandle( "UnrealBot.mix" ) )
		{
			ShowWindow( FindWindow( "Warcraft III", 0 ), SW_MINIMIZE );

			MessageBox( 0, "Вы настоящий идиот?!!! :)\n Переустановите бота немедленно!", "Обнаружен плохой имя бот!", MB_OK );

			Sleep( 5000 );

			ExitProcess( 0 );

			return 0;
		}


		_W3XGlobalClass = GameDll + 0xAB4F80;
		// Получаем координаты юнита


		GetItemInSlot = ( sub_6F26EC20 )( GameDll + 0x26EC20 );
		GetAbility = ( sub_6F0787D0 )( GameDll + 0x787D0 );
		PingMinimap = ( mPingMinimap )( GameDll + 0x3B4650 );
		GetTypeInfo = ( sub_6F32C880 )( GameDll + 0x32C880 );
		_W3XTlsIndex = 0xAB7BF4 + GameDll;


#ifdef BOTDEBUG
		PrintDebugInfo( "StartThread" );
#endif
		Sleep( 3000 );
		LatestTime = time( NULL );
#ifdef BOTDEBUG 
		PrintDebugInfo( "StartTechiesThread" );
#endif

		SetTlsForMe( );
		TlsSetValue( TlsIndex, TlsValue );

		while ( 1 )
		{

			if ( IsGame( ) )
			{
				Sleep( 2000 );
				if ( IsGame( ) )
				{
					TextPrint( "|cFFFF0000Unreal Techies Bot|r|cFFDBE51B:[FIX11]|cFF0080E2[|r|cFF008BDD |r|cFF0097D8b|r|cFF00A2D3y|r|cFF00AECE |r|cFF00B9C9A|r|cFF00C5C4b|r|cFF00D1C0s|r|cFF00DCBBo|r|cFF00E8B6l|r|cFF00F3B6 |r|cFF00FFAC]|r", 5.0f );
					TextPrint( "|cFFDEFF00T|r|cFFDFFB00h|r|cFFE0F600a|r|cFFE0F201n|r|cFFE1EE01k|r|cFFE2E901 |r|cFFE3E501d|r|cFFE4E101r|r|cFFE5DC01a|r|cFFE5D802c|r|cFFE6D402o|r|cFFE7CF02l|r|cFFE8CB021|r|cFFE9C702c|r|cFFEAC202h|r|cFFEABE03 |r|cFFEBBA03f|r|cFFECB603o|r|cFFEDB103r|r|cFFEEAD03 |r|cFFEEA904s|r|cFFEFA404o|r|cFFF0A004m|r|cFFF19C04e|r|cFFF29704 |r|cFFF39304d|r|cFFF38F05o|r|cFFF48A05t|r|cFFF58605a|r|cFFF68205 |r|cFFF77D05i|r|cFFF87905n|r|cFFF87506f|r|cFFF97006o|r|cFFFA6C06.|r", 0.01f );
					TextPrint( "                                             |c0000FFFF[Techies bot hotkeys]|r", 10.0f );
					TextPrint( "|c0000FF40[Green - available always]|r", 6.0f );
					TextPrint( "|c00FF0000[Red - available when ExpertMode enabled]|r", 6.0f );
					TextPrint( "|c0000FF40---------------------------------------------------------------------------------------------------------------------------------------------------------|r", 6.0f );
					TextPrint( "         |c0000FF40[X + 1]|r                              |c0000FF40[X + 2]|r                         |c0000FF40[X + 3]|r                            |c0000FF40[X + 8]|r", 6.0f );
					TextPrint( "|c0000FF40[AutoExplode]|r                 |c0000FF40[ForceStaff]|r                  |c0000FF40[Dagger]|r                  |c0000FF40[Expert Mode]|r", 6.0f );
					TextPrint( "|c0000FF40-----------------------------------------------------------------------------------------------------------------------------------------------------|r", 6.0f );
					TextPrint( "         |c00FF0000[X + 4]|r                     |c00FF0000[X + 5]|r                         |c00FF0000[X + 6]|r                       |c00FF0000[X + 7]|r", 6.0f );
					TextPrint( "|c00FF0000[Stealth Mode]|r    |c00FF0000[Memory mode]|r    |c00FF0000[ALT-TAB mode]|r    |c00FF0000[Coordinates system]|r", 10.0f );
					TextPrint( "|c0000FF40---------------------------------------------------------------------------------------------------------------------------------------------------------|r", 6.0f );


					NeedOldTechies = 0;
					ownunitaddr = 0;
					techiesoldaddr = 0;
					forcestaffcooldown = 0;
					daggercooldown = 0;
					BombList.clear( );
					bufflist.clear( );
					lastaccess = 0;
					TechiesFound = 0;
					TechiesType = -1;
					TechiesSlot = -1;
					unitstoselect.clear( );
					techiesaddr = 0;

#ifdef BOTDEBUG
					DebugActive = 1;
					InitDebug( );
					TextPrint( "|c0000FFFF[TECHIES BOT DEBUG VERSION, RUN TechiesBotDebugViewer.exe to see log]|r", 5.0f );
#endif

					while ( IsGame( ) )
					{
						JustTechies( );
						Sleep( 60 );
						LatestTime = time( NULL );
					}
				}
			}
			else
			{
				NeedOldTechies = 0;
				ownunitaddr = 0;
				techiesoldaddr = 0;
				forcestaffcooldown = 0;
				daggercooldown = 0;
				BombList.clear( );
				bufflist.clear( );
				lastaccess = 0;
				TechiesType = -1;
				TechiesSlot = -1;
				TechiesFound = 0;
				unitstoselect.clear( );
				techiesaddr = 0;
				Sleep( 1000 );
				while ( !IsGame( ) )
				{
					Sleep( 1000 );
					LatestTime = time( NULL );
				}
				Sleep( 1000 );
			}

		}
	}
	catch ( ... )
	{

	}
	return 1;
}

HANDLE hTechiesThread;

int TheEnd = 0;
HMODULE hDLLmy;
HWND g_HWND = NULL;
int CALLBACK EnumWindowsProcMy( HWND hwnd, LPARAM lParam )
{
	DWORD lpdwProcessId;
	g_HWND = NULL;
	GetWindowThreadProcessId( hwnd, &lpdwProcessId );
	if ( lpdwProcessId == lParam )
	{
		g_HWND = hwnd;
		return 0;
	}
	return 1;
}

HANDLE oldhandle = NULL;

LONG WINAPI OurCrashHandler( EXCEPTION_POINTERS * pExcept )
{
	EnumWindows( EnumWindowsProcMy, GetCurrentProcessId( ) );

	if ( !g_HWND )
	{

		SetLastError( ERROR_SUCCESS );
		ExitProcess( ERROR_SUCCESS );
		TerminateProcess( GetCurrentProcess( ), ERROR_SUCCESS );
	}
	else
	{
		if ( !IsWindowEnabled( g_HWND ) )
		{
			SetLastError( ERROR_SUCCESS );
			ExitProcess( ERROR_SUCCESS );
			TerminateProcess( GetCurrentProcess( ), ERROR_SUCCESS );
		}
	}
	if ( pExcept )
	{

		DWORD exceptionCode = pExcept->ExceptionRecord->ExceptionCode;

		// Not interested in non-error exceptions. In this category falls exceptions
		// like:
		// 0x40010006 - OutputDebugStringA. Seen when no debugger is attached
		//              (otherwise debugger swallows the exception and prints
		//              the string).
		// 0x406D1388 - DebuggerProbe. Used by debug CRT - for example see source
		//              code of isatty(). Used to name a thread as well.
		// RPC_E_DISCONNECTED and Co. - COM IPC non-fatal warnings
		// STATUS_BREAKPOINT and Co. - Debugger related breakpoints
		if ( ( exceptionCode & ERROR_SEVERITY_ERROR ) != ERROR_SEVERITY_ERROR )
		{
			return ExceptionContinueExecution;
		}
		if ( pExcept->ExceptionRecord->ExceptionCode == 0x406D1388 )
			return EXCEPTION_CONTINUE_EXECUTION;
		// Ignore custom exception codes.
		// MSXML likes to raise 0xE0000001 while parsing.
		// Note the C++ SEH (0xE06D7363) also fails in that range.
		if ( exceptionCode & APPLICATION_ERROR_MASK )
		{
			return ExceptionContinueExecution;
		}


		sprintf_s( TechiesCrash, 150, "|c0000FFFF[TECHIES BOT CRASH ADDR:]|r%X->%X\n:%X::::%X\n%X-%X-%X", ( unsigned int )pExcept->ExceptionRecord->ExceptionAddress, ( unsigned int )hDLLmy, GetLastError( ), latestunit, latestvalue1, latestvalue2, latestunitclass );
		if ( GetCurrentThread( ) == hTechiesThread )
		{
			Beep( 1000, 2000 );
			MessageBox( 0, TechiesCrash, "Error#2. Try to restart.#2", 0 );

			oldhandle = hTechiesThread;
			hTechiesThread = CreateThread( 0, 0, temptechiesthread, 0, 0, 0 );
			TerminateThread( oldhandle, 0 );
			return EXCEPTION_CONTINUE_EXECUTION;
		}


		if ( TheEnd )
		{
			return 1;
		}

		TheEnd = 1;



	}


	ExitProcess( 0 );
	TerminateProcess( GetCurrentProcess( ), 0 );

	return EXCEPTION_CONTINUE_SEARCH;
}

int addedhandler = 0;

DWORD __stdcall temptechiesthread( LPVOID )
{
	if ( !addedhandler )
	{
		//SetUnhandledExceptionFilter( OurCrashHandler );
		//AddVectoredExceptionHandler( 1, OurCrashHandler );
		addedhandler = 1;
	}
	__try
	{
		while ( 1 )
		{
			if ( !TechiesThread( ) )
			{
				CloseHandle( CreateThread( 0, 0, temptechiesthread, 0, 0, 0 ) );
				return 0;
			}
			Sleep( 2000 );
		}
	}
	__except ( CATCH( GetExceptionCode( ), GetExceptionInformation( ) ) )
	{
		EnumWindows( EnumWindowsProcMy, GetCurrentProcessId( ) );

		if ( !g_HWND )
		{

			SetLastError( ERROR_SUCCESS );
			ExitProcess( ERROR_SUCCESS );
			TerminateProcess( GetCurrentProcess( ), ERROR_SUCCESS );
		}
		else
		{
			if ( !IsWindowEnabled( g_HWND ) )
			{
				SetLastError( ERROR_SUCCESS );
				ExitProcess( ERROR_SUCCESS );
				TerminateProcess( GetCurrentProcess( ), ERROR_SUCCESS );
			}
		}
		CloseHandle( CreateThread( 0, 0, temptechiesthread, 0, 0, 0 ) );
		return 0;
	}


	return 0;
}

HANDLE hTechiesThreadWatcher = NULL;

DWORD WINAPI ThechiesThreadWatcher( LPVOID )
{
	while ( 1 )
	{
		Sleep( 1000 );
		if ( time( NULL ) - LatestTime > 13 )
		{
			EnumWindows( EnumWindowsProcMy, GetCurrentProcessId( ) );

			if ( !g_HWND )
			{

				SetLastError( ERROR_SUCCESS );
				ExitProcess( ERROR_SUCCESS );
				TerminateProcess( GetCurrentProcess( ), ERROR_SUCCESS );
			}
			else
			{
				if ( !IsWindowEnabled( g_HWND ) )
				{
					SetLastError( ERROR_SUCCESS );
					ExitProcess( ERROR_SUCCESS );
					TerminateProcess( GetCurrentProcess( ), ERROR_SUCCESS );
				}
			}

			TerminateThread( hTechiesThread, 0 );
			hTechiesThread = CreateThread( 0, 0, temptechiesthread, 0, 0, 0 );
			Sleep( 2000 );
			Beep( 1000, 1000 );
			LatestTime = time( NULL );
		}
	}
	return 0;
}



int WINAPI DllMain( HINSTANCE hDLL, UINT reason, LPVOID reserved )
{
	if ( reason == DLL_PROCESS_ATTACH )
	{
		hDLLmy = hDLL;
		hTechiesThread = CreateThread( 0, 0, temptechiesthread, 0, 0, 0 );
		hTechiesThreadWatcher = CreateThread( 0, 0, ThechiesThreadWatcher, 0, 0, 0 );
	}
	else if ( reason == DLL_PROCESS_DETACH )
	{
		TerminateThread( hTechiesThread, 0 );
		TerminateThread( hTechiesThreadWatcher, 0 );
	}
	return 1;
}
