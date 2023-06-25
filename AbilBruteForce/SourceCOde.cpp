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
#include <vector>
#include <sstream>
#include <algorithm>
#include <time.h>
#include <thread>  
#include <TlHelp32.h>
#include <exception>
#define _USE_MATH_DEFINES
#include <math.h>
#pragma endregion
void * abiladdr;
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
BOOL DebugActive = FALSE;

int any_techies_addr = 0;
std::vector<int> unitstoselect;

time_t LatestTime = time( NULL );

int latestunit = 0;

int latestunitclass = 0;

DWORD latestvalue1 = 0;
DWORD latestvalue2 = 0;

char TechiesCrash[ 150 ];

BOOL ExpertModeEnabled = FALSE;
BOOL Enable3DPoint = FALSE;
BOOL StealthMode = FALSE;

int CATCH( unsigned int code, struct _EXCEPTION_POINTERS *ep )
{
	return ExceptionContinueExecution;
}

BOOL IsOkayPtr( void *ptr, unsigned int size = 4 )
{
	if ( UseWarnIsBadReadPtr == 1 )
	{
		BOOL returnvalue = FALSE;
		returnvalue = IsBadReadPtr( ptr, size ) == 0;
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


BOOL IsGame( ) // my offset + public
{
	if ( GameDll <= 0 )
		return FALSE;

	return *( int* ) ( GameDll + 0xAB7E98 ) > 0 && ( *( int* ) ( GameDll + 0xACF678 ) > 0 || *( int* ) ( GameDll + 0xAB62A4 ) > 0 )/* && !IsLagScreen( )*/;
}

LPVOID TlsValue;
DWORD TlsIndex;
DWORD _W3XTlsIndex;

DWORD GetIndex( )
{
	return *( DWORD* ) ( _W3XTlsIndex );
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
				HANDLE hThread = OpenThread( THREAD_ALL_ACCESS, FALSE, te32.th32ThreadID );
				CONTEXT ctx = { CONTEXT_SEGMENTS };
				LDT_ENTRY ldt;
				GetThreadContext( hThread, &ctx );
				GetThreadSelectorEntry( hThread, ctx.SegFs, &ldt );
				DWORD dwThreadBase = ldt.BaseLow | ( ldt.HighWord.Bytes.BaseMid <<
													 16 ) | ( ldt.HighWord.Bytes.BaseHi << 24 );
				CloseHandle( hThread );
				if ( dwThreadBase == NULL )
					continue;
				DWORD* dwTLS = *( DWORD** ) ( dwThreadBase + 0xE10 + 4 * index );
				if ( dwTLS == NULL )
					continue;
				return ( DWORD ) dwTLS;
			}
		}
		while ( Thread32Next( hSnap, &te32 ) );
	}

	return NULL;
}

void SetTlsForMe( )
{
	TlsIndex = GetIndex( );
	LPVOID tls = ( LPVOID ) GetW3TlsForIndex( TlsIndex );
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
	UINT32 dwDuration = *( ( UINT32 * ) &fDuration );
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

time_t LastChatAccess = 0;

void TextPrintUnspammed( char *szText )
{
	if ( !IsGame( ) )
		return;
	if ( StealthMode )
		return;
	time_t nextaccess = time( 0 );

	if ( !ExpertModeEnabled )
		nextaccess += 1;

	if ( nextaccess > LastChatAccess + 1 )
	{
		float fDuration = 1.3f;
		if ( !ExpertModeEnabled )
			fDuration += 0.7f;
		LastChatAccess = nextaccess;
#ifdef BOTDEBUG
		PrintDebugInfo( "PrintText" );
#endif
		UINT32 dwDuration = *( ( UINT32 * ) &fDuration );
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

// Pure code: Get unit count and units array
int * GetUnitCountAndUnitArray( int ** unitarray )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "GetUnitArray" );
#endif
	int GlobalClassOffset = *( int* ) ( GameDll + 0xAB4F80 );
	if ( GlobalClassOffset )
	{

		int UnitsOffset1 = *( int* ) ( GlobalClassOffset + 0x3BC );
		int  *  UnitsCount = ( int * ) ( UnitsOffset1 + 0x604 );
		if ( *( int* ) UnitsCount > 0 && UnitsOffset1 > 0 )
		{
			if ( IsOkayPtr( ( void* ) ( UnitsOffset1 + 0x608 ) ) )
			{
#ifdef BOTDEBUG
				PrintDebugInfo( "OkayUnitArray" );
#endif
				*unitarray = ( int * ) *( int* ) ( UnitsOffset1 + 0x608 );
				return UnitsCount;
			}
			else
			{
				return 0;
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

__declspec( naked ) int __fastcall UpdatePlayerSelection( int a1, int unused, int a2 )
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


BOOL IsAbilityCooldown( int unitaddr, int id )
{
	 abiladdr = GetAbility( unitaddr, 0, id, 0, 1, 1, 1 );
	if ( abiladdr == nullptr || abiladdr <= 0 || !IsOkayPtr( ( void * ) abiladdr ) )
		return -1;

	int avilityflag = ( int ) abiladdr + 32;
	if ( IsOkayPtr( ( void* ) avilityflag ) )
	{
		unsigned int  state = *( unsigned int  * ) ( avilityflag );
		return state & cooldownflag;
	}
	else
		return -1;

}

BOOL IsAbilityHidden( int unitaddr, int id )
{
	 abiladdr = GetAbility( unitaddr, 0, id, 0, 1, 1, 1 );
	if ( abiladdr == nullptr || abiladdr <= 0 || !IsOkayPtr( ( void * ) abiladdr ) )
		return -1;

	int avilityflag = ( int ) abiladdr + 32;
	if ( IsOkayPtr( ( void* ) avilityflag ) )
	{
		unsigned int  state = *( unsigned int  * ) ( avilityflag );
		return state & xavaiableflag;
	}
	else
		return -1;
}

void PrintCooldownFlag( int unitaddr, int id )
{
 abiladdr = GetAbility( unitaddr, 0, id, 0, 1, 1, 1 );
	if ( abiladdr == nullptr || abiladdr <= 0 || !IsOkayPtr( ( void * ) abiladdr ) )
		return;

	int abilleveladdr = ( int ) abiladdr + 4;
	if ( IsOkayPtr( ( void* ) abilleveladdr ) )
	{
		int xid = id;
		char cc1 = *( char* ) ( ( int ) ( &xid ) );
		char cc2 = *( char* ) ( ( int ) ( &xid ) + 1 );
		char cc3 = *( char* ) ( ( int ) ( &xid ) + 2 );
		char cc4 = *( char* ) ( ( int ) ( &xid ) + 3 );
		char cc5[ ] = { cc4, cc3, cc2, cc1, '\0' };
		unsigned int  state = *( unsigned int  * ) ( abilleveladdr );
		char * printdate = new char[ 100 ];
		memset( printdate, 0, 100 );
		sprintf_s( printdate, 100, "%s->%X->%X", cc5, state, abiladdr );
		TextPrint( printdate, 2.0f );
		Sleep( 500 );
		delete[ ]printdate;
	}
}


void * GetGlobalPlayerData( )
{
	if ( *( int * ) ( 0xAB65F4 + GameDll ) > 0 )
	{
		if ( IsOkayPtr( ( void* ) ( 0xAB65F4 + GameDll ) ) )
			return ( void * ) *( int* ) ( 0xAB65F4 + GameDll );
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
	if ( number >= 0 && number <= 12 && arg1 != nullptr && arg1 )
	{
		result = ( int ) arg1 + ( number * 4 ) + 0x58;

		if ( IsOkayPtr( ( void* ) result ) )
		{
			result = *( int* ) result;
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
		int playerslotaddr = ( int ) gldata + 0x28;
		if ( IsOkayPtr( ( void* ) playerslotaddr ) )
			return ( int ) *( short * ) ( playerslotaddr );
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

BOOL DisableWindowActiveCheck = TRUE;

BOOL IsWindowActive( BOOL SkipNoCheck = FALSE )
{
	if ( DisableWindowActiveCheck && !SkipNoCheck )
		return DisableWindowActiveCheck;

	return *( BOOL* ) ( GameDll + 0xA9E7A4 );
}

UINT GetUnitOwnerSlot( int unitaddr )
{
	if ( IsOkayPtr( ( void* ) ( unitaddr + 88 ) ) )
		return *( int* ) ( unitaddr + 88 );
	return -1;
}

BOOL IsPlayerEnemy( int unitaddr )
{
	int teamplayer1 = GetLocalPlayerNumber( ) > 5 ? 1 : 2;
	int teamplayer2 = GetUnitOwnerSlot( unitaddr ) > 5 ? 1 : 2;
#ifdef BOTDEBUG
	PrintDebugInfo( "EndGetPlayerEnemy" );
#endif
	return teamplayer1 != teamplayer2;
}

BOOL IsUnitDead( int unitaddr )
{
	unsigned int isdolbany = *( unsigned int* ) ( unitaddr + 0x5C );
	BOOL UnitNotDead = ( ( isdolbany & 0x100u ) == 0 );
	return UnitNotDead == FALSE;
}

void GetMousePosition( float * x, float * y, float * z )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "Mouse info" );
#endif
	int globalclass = *( int* ) _W3XGlobalClass;

	int offset1 = globalclass + 0x3BC;

	if ( globalclass > 0 )
	{


		if ( IsOkayPtr( ( void* ) offset1 ) )
		{
			offset1 = *( int * ) offset1;
			if ( IsOkayPtr( ( void* ) offset1 ) )
			{
				*x = *( float* ) ( offset1 + 0x310 );
				*y = *( float* ) ( offset1 + 0x310 + 4 );
				*z = *( float* ) ( offset1 + 0x310 + 4 + 4 );
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
			MOV ECX, DWORD PTR DS : [ EAX + 0x34 ];
			MOV EAX, DWORD PTR DS : [ ECX + 0x1E0 ];
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
BOOL IsButtonCooldown( DWORD buttonid )
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

BOOL IsBtnCooldown1( int buttonid )
{

}


BOOL IsBtnCooldown2( int buttonid )
{

}




/*
BOOL IsSuicideCooldown( int btnid )
{
return GetBtnCooldown( BTN_TECHIES_SUICIDE , 1 );
}
*/


int GetUnitItemCODE( int unit_or_item_addr )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "GetUnitItemCODE" );
#endif
	return *( int* ) ( unit_or_item_addr + 0x30 );
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
		if ( v3 && ( v4 = *( int * ) ( v3 + 40 ) ) != 0 )
		{
			v5 = v4 - 1;
			if ( v5 >= ( unsigned int ) 0 )
				v5 = 0;
			return ( char * ) *( int * ) ( *( int * ) ( v3 + 44 ) + 4 * v5 );
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
	int unitdata = *( int* ) ( unitaddr + 0x28 );
	if ( unitdata > 0 )
	{
		*( UINT* ) ( unitdata + 0x328 ) = color;
		*( UINT* ) ( unitdata + 0x320 ) = 0;
		*( UINT* ) ( unitdata + 0x316 ) = 0;
		if ( !( *( UINT* ) ( unitdata + 0x312 ) & 0x800 ) )
		{
			*( UINT* ) ( unitdata + 0x312 ) = *( UINT* ) ( unitdata + 0x316 ) | 0x800;
		}

	}
}

BOOL IsClassEqual( int unit_or_item_addr, char * classid )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "UnitCLass" );
#endif
	char unitclass[ 5 ];
	memset( unitclass, 0, 5 );

	if ( IsOkayPtr( ( void* ) ( unit_or_item_addr + 0x30 ) ) )
	{
		*( BYTE* ) &unitclass[ 0 ] = *( BYTE* ) ( unit_or_item_addr + 0x30 + 3 );
		*( BYTE* ) &unitclass[ 1 ] = *( BYTE* ) ( unit_or_item_addr + 0x30 + 2 );
		*( BYTE* ) &unitclass[ 2 ] = *( BYTE* ) ( unit_or_item_addr + 0x30 + 1 );
		*( BYTE* ) &unitclass[ 3 ] = *( BYTE* ) ( unit_or_item_addr + 0x30 + 0 );
		if ( strlen( classid ) == 4 )
		{
			if ( unitclass[ 0 ] == classid[ 0 ] && unitclass[ 1 ] == classid[ 1 ] &&
				 unitclass[ 2 ] == classid[ 2 ] && unitclass[ 3 ] == classid[ 3 ] )
				 return TRUE;
		}
	}

	return FALSE;
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

	if ( *( int* ) _W3XGlobalClass > 0 )
	{
		__asm
		{
			ADDR( _W3XGlobalClass, ECX );
			MOV ECX, DWORD PTR DS : [ ECX + 0x1B4 ];
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



BOOL IsItemCooldown( int itemaddr )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "CheckItemCooldownv1" );
#endif
	if ( itemaddr == NULL )
		return TRUE;

	return !( ( *( UINT* ) ( itemaddr + 32 ) ) & 0x400u );
}

int commandjumpaddr;
void  sub_6F339DD0my( int a1, int a2, float a3, float a4, int a5, int a6, int a7 )
{
	commandjumpaddr = GameDll + 0x339DD0;
	if ( *( int* ) _W3XGlobalClass > 0 )
	{
		__asm
		{

			PUSH a7;
			PUSH a6;
			PUSH a5;
			PUSH a4;
			ADDR( _W3XGlobalClass, ECX );
			MOV ECX, DWORD PTR DS : [ ECX + 0x1B4 ];
			PUSH a3;
			PUSH a2;
			PUSH a1;
			CALL commandjumpaddr;
		}
	}
}



void  CommandItemTarget( int cmd, int itemaddr, float targetx, float targety, int targetunitaddr, BOOL queue = FALSE )
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
	if ( *( int* ) _W3XGlobalClass > 0 )
	{
		__asm
		{

			PUSH a4;
			PUSH a3;
			ADDR( _W3XGlobalClass, ECX );
			MOV ECX, DWORD PTR DS : [ ECX + 0x1B4 ];
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
	if ( *( int* ) _W3XGlobalClass > 0 )
	{
		__asm
		{

			PUSH a6;
			PUSH a5;
			PUSH a4;
			ADDR( _W3XGlobalClass, ECX );
			MOV ECX, DWORD PTR DS : [ ECX + 0x1B4 ];
			PUSH a3;
			PUSH a2;
			PUSH a1;

			CALL CmdPointAddr;
		}
	}
}

void  ItemOrSkillPoint( int cmd, int itemorskilladdr, float x, float y, int a5, BOOL addque = FALSE )
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
	*x = *( float* ) ( unitaddr + 0x284 );
	*y = *( float* ) ( unitaddr + 0x288 );
}

void GetUnitLocation3D( int unitaddr, float * x, float * y, float * z )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "Unit 3d" );
#endif
	*x = *( float* ) ( unitaddr + 0x284 );
	*y = *( float* ) ( unitaddr + 0x288 );
	*z = *( float* ) ( unitaddr + 0x28C );
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
	return ( int ) ( ( GetUnitHP( unitaddr ) / GetUnitMaxHP( unitaddr ) ) * 100.f );
}


BOOL IsHero( int unitaddr )
{
	if ( IsOkayPtr( ( void* ) ( unitaddr + 48 ) ) )
	{
		unsigned int ishero = *( unsigned int* ) ( unitaddr + 48 );
		ishero = ishero >> 24;
		ishero = ishero - 64;
		return ishero < 0x19;
	}
	return FALSE;
}

float GetUnitTimer( int unitaddr )
{
	int unitdataddr = *( int* ) ( unitaddr + 0x28 );
	if ( unitdataddr <= 0 )
		return 0.0f;
#ifdef BOTDEBUG
	PrintDebugInfo( "CheckBadUnit - ENDCHExCK5" );
#endif
	if ( IsOkayPtr( ( void* ) ( unitdataddr + 0xA0 ), 4 ) )
		return *( float* ) ( unitdataddr + 0xA0 );
	return 0.0f;
}


BOOL IsNotBadUnit( int unitaddr, BOOL onlymemcheck = FALSE )
{

	if ( unitaddr > 0 && IsOkayPtr( ( void* ) unitaddr, 0x290 ) )
	{
		int xaddr = GameDll + 0x931934;
		int xaddraddr = ( int ) &xaddr;



		if ( *( BYTE* ) xaddraddr != *( BYTE* ) unitaddr )
			return FALSE;
		else if ( *( BYTE* ) ( xaddraddr + 1 ) != *( BYTE* ) ( unitaddr + 1 ) )
			return FALSE;
		else if ( *( BYTE* ) ( xaddraddr + 2 ) != *( BYTE* ) ( unitaddr + 2 ) )
			return FALSE;
		else if ( *( BYTE* ) ( xaddraddr + 3 ) != *( BYTE* ) ( unitaddr + 3 ) )
			return FALSE;


		if ( !IsOkayPtr( ( void* ) ( unitaddr + 0x5C ) ) )
		{
			return FALSE;
		}

		if ( onlymemcheck )
			return TRUE;

		unsigned int isdolbany = *( unsigned int* ) ( unitaddr + 0x5C );

		BOOL returnvalue = isdolbany != 0x1001u && !IsUnitDead( unitaddr ) && ( isdolbany & 0x40000000u ) == 0;
#ifdef BOTDEBUG
		PrintDebugInfo( "CheckBadUnit - ENDCHECK" );
#endif


		if ( returnvalue )
		{
			if ( ( *( BYTE * ) ( unitaddr + 32 ) & 2 ) && ( *( BYTE * ) ( unitaddr + 32 ) & 8 ) == 0 )
			{
				float unittimer = GetUnitTimer( unitaddr );

				if ( unittimer > 2.0 )
				{
					latestunitclass = GetUnitItemCODE( unitaddr );

					latestvalue1 = *( UINT * ) ( unitaddr + 12 );
					latestvalue2 = *( UINT* ) ( unitaddr + 16 );
					int testval = *( UINT* ) ( unitaddr + 12 ) & *( UINT* ) ( unitaddr + 16 );
					if ( testval != -1 && *( UINT* ) ( unitaddr + 12 ) != -1 && *( UINT* ) ( unitaddr + 16 ) != -1 )
					{
						int valnew = sub_6F03FA30( *( UINT* ) ( unitaddr + 12 ), *( UINT* ) ( unitaddr + 16 ) );
						int val2 = 0;
						if ( *( UINT * ) ( valnew + 12 ) == 727803756 )
							val2 = valnew;
#ifdef BOTDEBUG
						PrintDebugInfo( "CheckBadUnit - ENDCHECK2" );
#endif
						if ( val2 )
						{
							if ( !*( int * ) ( val2 + 32 ) )
							{


#ifdef BOTDEBUG
								PrintDebugInfo( "CheckBadUnit - ENDCHECK4" );
#endif
								return TRUE;
							}
						}
					}
				}
			}

		}
	}
#ifdef BOTDEBUG
	PrintDebugInfo( "CheckBadUnit - BAD UNIT" );
#endif
	return FALSE;
}

float Distance( float dX0, float dY0, float dX1, float dY1 )
{
	return sqrt( ( dX1 - dX0 )*( dX1 - dX0 ) + ( dY1 - dY0 )*( dY1 - dY0 ) );
}


float Distance3D( float x1, float y1, float z1, float x2, float y2, float z2 )
{
	if ( Enable3DPoint )
	{
		double d[ ] = { abs( ( double ) x1 - ( double ) x2 ), abs( ( double ) y1 - ( double ) y2 ), abs( ( double ) z1 - ( double ) z2 ) };
		if ( d[ 0 ] < d[ 1 ] ) std::swap( d[ 0 ], d[ 1 ] );
		if ( d[ 0 ] < d[ 2 ] ) std::swap( d[ 0 ], d[ 2 ] );
		return ( float ) ( d[ 0 ] * sqrt( 1.0 + d[ 1 ] / d[ 0 ] + d[ 2 ] / d[ 0 ] ) );
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

BOOL TechiesFound = FALSE;


struct BombStruct
{
	int unitaddr;
	float dmg;
	float range1;
	float range2;
	float x;
	float y;
	float z;
	BOOL remote;
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
		MOV EAX, DWORD PTR DS : [ _W3XGlobalClass ];
		MOV EAX, DWORD PTR DS : [ EAX ];
		MOV ESI, DWORD PTR DS : [ EAX + 0x24C ];
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

	if ( *( BYTE * ) ( xunitaddr + 32 ) & 2 )
	{
		void * playerseldata = *( void** ) ( GetLocalPlayer( ) + 0x34 );
		WORD playerslot = GetLocalPlayerNumber( );
		sub_6F424B80( playerseldata, 0, xunitaddr, playerslot, 0, 1, 1 );
		UpdatePlayerSelection( ( int ) playerseldata, 0, 0 );
		return sub_6F332700( 0, 0 );
	}

	return -1;
}


BOOL __cdecl SelectAllUnits( )
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

	Sleep( 10 );
	if ( !IsGame( ) )
	{
		return 0;
	}

	while ( unitstoselect.size( ) > 0 && myselectedunits < 11 && SelectUnit( unitstoselect.back( ) ) != -1 )
	{
		myselectedunits++;
		unitstoselect.pop_back( );
		Sleep( 5 );
	}

	Sleep( 10 );

	if ( !IsGame( ) )
	{
		return 0;
	}
	return myselectedunits > 0;
}


float GetProtectForUnit( int unitaddr )
{
	float Armor = 0.0f;

	if ( IsOkayPtr( ( void* ) ( unitaddr + 0xE0 ) ) )
	{
		Armor = *( float* ) ( unitaddr + 0xE0 );
	}

	if ( Armor < 0.0f )
		Armor = ( float ) ( 1.0 - pow( 0.94, -Armor ) );
	else
		Armor = ( float ) ( ( 0.06 * Armor ) / ( 1 + 0.06 * Armor ) );
	return Armor;
}

float GetProtectForProtect( float Armor )
{
	if ( Armor < 0.0f )
		Armor = ( float ) ( 1.0 - pow( 0.94, -Armor ) );
	else
		Armor = ( float ) ( ( 0.06 * Armor ) / ( 1 + 0.06 * Armor ) );
	return Armor;
}

int GetUnitAbilityLevel( int unitaddr, int id, BOOL checkavaiable = FALSE, BOOL CheckMemAccess = FALSE )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "GetAbilityLevel" );
#endif

	if ( CheckMemAccess )
	if ( !IsNotBadUnit( unitaddr ) )
		return 0;

	 abiladdr = ( void * ) GetAbility( unitaddr, 0, id, 0, 1, 1, 1 );
	if ( abiladdr == nullptr || abiladdr <= 0 || !IsOkayPtr( ( void * ) abiladdr ) )
		return 0;

	int abilleveladdr = ( int ) abiladdr + 80;
	int abilavaiable = ( int ) abiladdr + 32;
	if ( checkavaiable )
	{
		if ( IsOkayPtr( ( void* ) abilavaiable ) )
		{
			unsigned int avaiableflag = *( unsigned int * ) ( abilavaiable );
			if ( avaiableflag & xavaiableflag )
				return 0;
			if ( IsOkayPtr( ( void* ) abilleveladdr ) )
				return *( int * ) ( abilleveladdr ) +1;
			else
				return 0;
		}
		else
			return 0;
	}
	else
	{
		if ( IsOkayPtr( ( void* ) abilleveladdr ) )
			return *( int * ) ( abilleveladdr ) +1;
		else
			return 0;
	}
}


std::string PrintBuffListStr;


float GetDmgForUnit( int unitaddr, float dmg, int dmgtype = 1/* 1 : magical , 2 : physical*/ )
{
	PrintBuffListStr.clear( );
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

	if ( !IsNotBadUnit( unitaddr, TRUE ) )
	{
		return 0.0f;
	}

	if ( dmgtype == 1 && ( GetUnitAbilityLevel( unitaddr, 'B014', FALSE, TRUE ) ||
		GetUnitAbilityLevel( unitaddr, 'B041', FALSE, TRUE ) ||
		GetUnitAbilityLevel( unitaddr, 'B08J', FALSE, TRUE ) ||
		GetUnitAbilityLevel( unitaddr, 'B0FP', FALSE, TRUE ) ||
		GetUnitAbilityLevel( unitaddr, 'A2T4', FALSE, TRUE ) ||
		GetUnitAbilityLevel( unitaddr, 'B0FQ', FALSE, TRUE ) ||
		GetUnitAbilityLevel( unitaddr, 'B05S', FALSE, TRUE )
		) )
	{
		PrintBuffListStr = std::string( "100% shield" );
		return 0.0f;
	}

	float outdmg = 0.0f;
	outdmg -= 5.0f;

	if ( dmgtype == 1 )
		outdmg = dmg * ( ( 100.0f - 26.0f ) / 100.0f );
	else if ( dmgtype == 2 )
		outdmg = dmg - ( dmg * GetProtectForUnit( unitaddr ) );


	// Jugger(Nbbc) if skill A05G not exist - 100% protect



	//H00J H00I



	if ( dmgtype == 1 && IsClassEqual( unitaddr, "Nbbc" ) )
	{
		if ( IsNotBadUnit( unitaddr, TRUE ) && GetUnitAbilityLevel( unitaddr, 'A05G', TRUE ) == 0 )
		{

			PrintBuffListStr += std::string( "[magic 100%]" );

			return 0.0f;
		}
	}

	if ( dmgtype == 1 && ( IsClassEqual( unitaddr, "H00I" ) || IsClassEqual( unitaddr, "H00J" ) ) )
	{
		PrintBuffListStr += std::string( "[magic 10%]" );
		outdmg = outdmg * ( ( 100.0f - 10.0f ) / 100.0f );
	}


	int medusashield = GetUnitAbilityLevel( unitaddr, 'BNms' );
	if ( medusashield > 0 )
	{
		PrintBuffListStr += std::string( "[all 50%]" );
		outdmg = outdmg * ( ( 100.0f - 50.0f ) / 100.0f );
	}


	int captainbuff = GetUnitAbilityLevel( unitaddr, 'B09U' );
	if ( captainbuff > 0 )
	{
		PrintBuffListStr += std::string( "[all 50%]" );
		outdmg = outdmg * ( ( 100.0f - 50.0f ) / 100.0f );
	}


	int warcrysven = GetUnitAbilityLevel( unitaddr, 'B0BV' );
	if ( warcrysven > 0 && dmgtype == 2 )
	{
		float addprotect = GetProtectForProtect( warcrysven * 4.0f );
		int addprotectp = ( int ) ( 100 * addprotect );
		std::string( "[phys " ) + std::to_string( addprotectp ) + std::string( "%]" );
		outdmg = dmg - addprotect;
	}



	int huskarlvl = GetUnitAbilityLevel( unitaddr, 'A0QQ' );
	if ( dmgtype == 1 && huskarlvl > 0 )
	{

		float dmgprotectlvl = ( float ) huskarlvl + 3.0f;
		int hppercent = 100 - GetUnitHPPercent( unitaddr );
		int parts = hppercent / 7 + 1;
		dmgprotectlvl = dmgprotectlvl * parts;

		PrintBuffListStr += std::string( "[magic " ) + std::to_string( ( int ) dmgprotectlvl ) + std::string( "%]" );

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
		PrintBuffListStr += std::string( "[magic " ) + std::to_string( dmgprotect ) + std::string( "%]" );

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
		PrintBuffListStr += std::string( "[magic " ) + std::to_string( dmgprotect ) + std::string( "%]" );

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
		PrintBuffListStr += std::string( "[all " ) + std::to_string( dmgprotect ) + std::string( "%]" );

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
		PrintBuffListStr += std::string( "[magic " ) + std::to_string( dmgprotect ) + std::string( "%]" );
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
		PrintBuffListStr += std::string( "[magic " ) + std::to_string( dmgprotect ) + std::string( "%]" );
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
					PrintBuffListStr += std::string( "[magic 15%]" );
					outdmg = outdmg * ( ( 100.0f - 15.0f ) / 100.0f );
				}
				else if ( IsClassEqual( nitem, "I0K6" ) )
				{
					PrintBuffListStr += std::string( "[magic 30%]" );
					outdmg = outdmg * ( ( 100.0f - 30.0f ) / 100.0f );
				}
			}
		}
	}
	else if ( dmgtype == 2 )
	{

	}

	if ( !IsNotBadUnit( unitaddr, TRUE ) )
		return 0.0f;

	return outdmg;
}

char facici[ 50 ];

float  GetUnitFacing( int unitaddr )
{
	int unitdataoffset = *( int* ) ( unitaddr + 0x28 );
	if ( unitdataoffset > 0 )
	{
		double atan2value = atan2( ( double ) *( float* ) ( unitdataoffset + 0x10c ), ( double ) *( float* ) ( unitdataoffset + 0x108 ) );
		return  ( float ) atan2value;
	}
	return 100.0f;
}


BOOL __cdecl IsUnitVisibleToPlayer( int unitaddr, int player )
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
		return FALSE;
}


BOOL SelectTechies( )
{
	BOOL returnvalue = FALSE;
	if ( any_techies_addr > 0 )
	{
		if ( GetSelectedOwnedUnit( ) == any_techies_addr )
			return TRUE;

		if ( IsNotBadUnit( any_techies_addr ) )
		{
#ifdef BOTDEBUG
			PrintDebugInfo( "Techies selected" );
#endif
			unitstoselect.clear( );
			unitstoselect.push_back( any_techies_addr );
			returnvalue = SelectAllUnits( );
			unitstoselect.clear( );
		}
		else
		{
#ifdef BOTDEBUG
			PrintDebugInfo( "Techies not selected 11" );
#endif
			if ( !IsNotBadUnit( any_techies_addr, any_techies_addr ) && !IsHero( any_techies_addr ) )
				any_techies_addr = 0;

		}
	}

	return returnvalue;
}


struct TechiesActionStruct
{
	int action;
	int data;
};
std::vector<BombStruct> BombList;

int EnableAutoExplode = 0;
BOOL EnableDagger = FALSE;
BOOL FunModeEnabled = FALSE;

BOOL _Daggercooldown = FALSE;
BOOL EnableForceStaff = TRUE;
int TechiesType = -1;
int TechiesSlot = -1;

BOOL _Forcecooldown = FALSE;
int forcestaffcooldown = 0;
int daggercooldown = 0;

BOOL IsFirstFun = TRUE;
char OldTechiesMissile[ 256 ];
int oldlen = 0;
char * funtechiesmissile = "Abilities\\Weapons\\BlackKeeperMissile\\BlackKeeperMissile.mdl\x0\xA\x0\xA";

void TechiesFunMode( BOOL enabled )
{

	if ( IsFirstFun )
	{

		int ClassAddr = GetTypeInfo( 'H00K', 0 );
		if ( ClassAddr > 0 )
		{
			int offset2 = ClassAddr + 0x44;
			offset2 = *( int* ) offset2;
			if ( offset2 > 0 )
			{

				offset2 = *( int* ) offset2;
				if ( offset2 > 0 )
				{
					char * modelname = 0;
					IsFirstFun = FALSE;
					*( int* ) &modelname = offset2;
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
				offset2 = *( int* ) offset2;
				if ( offset2 )
				{

					offset2 = *( int* ) offset2;
					if ( offset2 )
					{
						char * modelname = 0;
						*( int* ) &modelname = offset2;
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
				offset2 = *( int* ) offset2;
				if ( offset2 )
				{

					offset2 = *( int* ) offset2;
					if ( offset2 )
					{
						char * modelname = 0;
						*( int* ) &modelname = offset2;
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
BOOL NeedOldTechies = FALSE;

void SetTechiesNewAddrForAlly( )
{
	NeedOldTechies = TRUE;
	if ( TechiesType == 1 )
	{
		if ( ownunitaddr == 0 )
		{
			int ownunit = GetSelectedOwnedUnit( );
			if ( IsNotBadUnit( ownunit ) && IsHero( ownunit ) )
			{
				techiesoldaddr = any_techies_addr;
				any_techies_addr = ownunit;
				ownunitaddr = ownunit;
			}
		}
		else if ( !IsNotBadUnit( ownunitaddr, TRUE ) || ( !IsHero( ownunitaddr ) && !IsUnitDead( ownunitaddr ) ) )
		{
			ownunitaddr = 0;
		}
	}
}

void SetOldTechiesAddr( )
{
	if ( NeedOldTechies && TechiesType == 1 )
	{
		NeedOldTechies = FALSE;
		if ( techiesoldaddr != 0 )
			any_techies_addr = techiesoldaddr;
	}
}

int TechiesEnemyTimer = 300;
char abilfound[ 128 ];

void DetonateIfNeed( )
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

		Sleep( 100 );

		if ( IsKeyPressed( '1' ) )
		{
			while ( IsKeyPressed( '1' ) )
			{
				Sleep( 10 );
			}
		}

		int selun = GetSelectedOwnedUnit( );

		char * brutearray = "QWERTYUIOPASDFGHJKLZXCVBNM1234567890qwerasdfzxcvtyuighjkbnmopl!";
		int brutesize = strlen( brutearray ) + 1;

		for ( int i1 = 0; i1 < brutesize; i1++ )
		{
			for ( int i2 = 0; i2 < brutesize; i2++ )
			{
				for ( int i3 = 0; i3 < brutesize; i3++ )
				{
					for ( int i4 = 0; i4 < brutesize; i4++ )
					{
						char data[ 5 ];
						data[ 0 ] = brutearray[ i1 ];
						data[ 1 ] = brutearray[ i2 ];
						data[ 2 ] = brutearray[ i3 ];
						data[ 3 ] = brutearray[ i4 ];
						data[ 4 ] = brutearray[ '\0' ];

						int testabilcode = 0;


						CopyMemory( &testabilcode, &data[ 0 ], 4 );

						if ( GetUnitAbilityLevel( selun, testabilcode ) > 0 )
						{



							data[ 0 ] = brutearray[ i4 ];
							data[ 1 ] = brutearray[ i3 ];
							data[ 2 ] = brutearray[ i2 ];
							data[ 3 ] = brutearray[ i1 ];
							data[ 4 ] = brutearray[ '\0' ];

							sprintf_s( abilfound, 128, "Abil found[%s]:[%X]", data, abiladdr );

							TextPrint( abilfound, 10.0 );
						}
					}
				}
			}
		}
	}
}

BOOL TechiesThread( )
{
	try
	{

		Sleep( 3500 );
		LatestTime = time( NULL );
		GameDll = ( int ) GetModuleHandle( "Game.dll" );
		if ( !GameDll )
		{
			MessageBox( 0, "Error no game.dll found", "Error", MB_OK );
			return TRUE;
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
			 GetModuleHandle( "UnrealBot.mix" ) )
		{
			ShowWindow( FindWindow( "Warcraft III", 0 ), SW_MINIMIZE );

			MessageBox( 0, "Идиот детектед? :)\n Переустановите бота!", "Обнаружен плохой имя бот!", MB_OK );

			Sleep( 5000 );

			ExitProcess( 0 );

			return FALSE;
		}


		_W3XGlobalClass = GameDll + 0xAB4F80;
		// Получаем координаты юнита


		GetItemInSlot = ( sub_6F26EC20 ) ( GameDll + 0x26EC20 );
		GetAbility = ( sub_6F0787D0 ) ( GameDll + 0x787D0 );
		PingMinimap = ( mPingMinimap ) ( GameDll + 0x3B4650 );
		GetTypeInfo = ( sub_6F32C880 ) ( GameDll + 0x32C880 );
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

		while ( TRUE )
		{

			if ( IsGame( ) )
			{
				Sleep( 2000 );
				if ( IsGame( ) )
				{
					TextPrint( "|cFFFF0000Unreal Techies Bot|r|cFFDBE51B |r|cFF003AFFF|r|cFF0046FAi|r|cFF0051F5n|r|cFF005DF0a|r|cFF0068EBl|r|cFF0074E7|r[FIX10]|cFF0080E2[|r|cFF008BDD |r|cFF0097D8b|r|cFF00A2D3y|r|cFF00AECE |r|cFF00B9C9A|r|cFF00C5C4b|r|cFF00D1C0s|r|cFF00DCBBo|r|cFF00E8B6l|r|cFF00F3B6 |r|cFF00FFAC]|r", 5.0f );
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


					NeedOldTechies = FALSE;
					ownunitaddr = 0;
					techiesoldaddr = 0;
					forcestaffcooldown = 0;
					daggercooldown = 0;
					BombList.clear( );
					PrintBuffListStr.clear( );
					LastChatAccess = 0;
					TechiesFound = FALSE;
					TechiesType = -1;
					TechiesSlot = -1;
					unitstoselect.clear( );
					any_techies_addr = 0;

#ifdef BOTDEBUG
					DebugActive = TRUE;
					InitDebug( );
					TextPrint( "|c0000FFFF[TECHIES BOT DEBUG VERSION, RUN TechiesBotDebugViewer.exe to see log]|r", 5.0f );
#endif

					while ( IsGame( ) )
					{
						DetonateIfNeed( );
						Sleep( 60 );
						LatestTime = time( NULL );
					}
				}
			}
			else
			{
				NeedOldTechies = FALSE;
				ownunitaddr = 0;
				techiesoldaddr = 0;
				forcestaffcooldown = 0;
				daggercooldown = 0;
				BombList.clear( );
				PrintBuffListStr.clear( );
				LastChatAccess = 0;
				TechiesType = -1;
				TechiesSlot = -1;
				TechiesFound = FALSE;
				unitstoselect.clear( );
				any_techies_addr = 0;
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
	return TRUE;
}

HANDLE hTechiesThread;

BOOL TheEnd = FALSE;
HMODULE hDLLmy;
HWND g_HWND = NULL;
BOOL CALLBACK EnumWindowsProcMy( HWND hwnd, LPARAM lParam )
{
	DWORD lpdwProcessId;
	g_HWND = NULL;
	GetWindowThreadProcessId( hwnd, &lpdwProcessId );
	if ( lpdwProcessId == lParam )
	{
		g_HWND = hwnd;
		return FALSE;
	}
	return TRUE;
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


		sprintf_s( TechiesCrash, 150, "|c0000FFFF[TECHIES BOT CRASH ADDR:]|r%X->%X\n:%X::::%X\n%X-%X-%X", pExcept->ExceptionRecord->ExceptionAddress, hDLLmy, GetLastError( ), latestunit, latestvalue1, latestvalue2, latestunitclass );
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

		TheEnd = TRUE;



	}


	ExitProcess( 0 );
	TerminateProcess( GetCurrentProcess( ), 0 );

	return EXCEPTION_CONTINUE_SEARCH;
}

BOOL addedhandler = FALSE;

DWORD __stdcall temptechiesthread( LPVOID )
{
	if ( !addedhandler )
	{
		//SetUnhandledExceptionFilter( OurCrashHandler );
		//AddVectoredExceptionHandler( 1, OurCrashHandler );
		addedhandler = TRUE;
	}
	__try
	{
		while ( TRUE )
		{
			if ( !TechiesThread( ) )
			{
				CreateThread( 0, 0, temptechiesthread, 0, 0, 0 );
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
		CreateThread( 0, 0, temptechiesthread, 0, 0, 0 );
		return 0;
	}


	return 0;
}

HANDLE hTechiesThreadWatcher = NULL;

DWORD WINAPI ThechiesThreadWatcher( LPVOID )
{
	while ( TRUE )
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



BOOL WINAPI DllMain( HINSTANCE hDLL, UINT reason, LPVOID reserved )
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
	return TRUE;
}
