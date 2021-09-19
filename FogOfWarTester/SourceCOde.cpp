
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
#include <Shellapi.h>
#include <MinHook.h>
#pragma comment(lib,"libMinHook.x86.lib")

#pragma endregion

#ifdef BOTDEBUG
#include <tlhelp32.h>
void PrintDebugInfo( const char * debuginfo );
#endif

// Game.dll address
int GameDll = 0;
int _W3XGlobalClass;

int UseWarnIsBadReadPtr = 1;
BOOL DebugActive = FALSE;

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

int pid = 0;
int debaddr = 0;


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
		CopyMemory( dedata[ currentid ] , szDllPath , strlen( szDllPath ) + 1 );
		currentid++;
	}
	else
	{
		for ( int i = 0; i < 49; i++ )
		{
			CopyMemory( dedata[ i ] , dedata[ i + 1 ] , 256 );
		}
		CopyMemory( dedata[ 49 ] , szDllPath , strlen( szDllPath ) + 1 );
	}
}



char * laststringinfo = new char[ 256 ];

void PrintDebugInfo( const char * debuginfo )
{
	if ( !DebugActive )
		return;
	DebugStr( debuginfo );
}

void PrintDebugInfo2( const char * debuginfo , int data )
{
	if ( !DebugActive )
		return;
	sprintf_s( laststringinfo , 256 , "%s:%X" , debuginfo , data );
	DebugStr( laststringinfo );
}

void InitDebug( )
{
	xxaddr = &dedata;
	FILE * f = NULL;
	if ( fopen_s( &f , "debug.bin" , "wb" ) == NOERROR )
	{
		fwrite( &xxaddr , 4 , 1 , f );
		fclose( f );
	}
}


#endif

#define IsKeyPressed(CODE) (GetAsyncKeyState(CODE) & 0x8000) > 0




typedef void( __cdecl * mClearSelection )( );
mClearSelection ClearSelection;
BOOL IsGame( ) // my offset + public
{
	return ( *( int* ) ( ( UINT32 ) GameDll + 0xBE6530 ) > 0 || *( int* ) ( ( UINT32 ) GameDll + 0xBE6530 ) > 0 )/* && !IsLagScreen( )*/;
}

// get thread access (for Jass natives / other functions)
void SetTlsForMe( )
{
	UINT32 Data = *( UINT32 * ) ( GameDll + 0xBB8978 );
	UINT32 TlsIndex = *( UINT32 * ) ( GameDll + 0xBB8628 );
	if ( TlsIndex )
	{
		UINT32 v5 = **( UINT32 ** ) ( *( UINT32 * ) ( *( UINT32 * ) ( GameDll + 0xBB896C ) + 4 * Data ) + 44 );
		if ( !v5 || !( *( LPVOID * ) ( v5 + 520 ) ) )
		{
			Sleep( 1000 );
			SetTlsForMe( );
			return;
		}
		TlsSetValue( TlsIndex, *( LPVOID * ) ( v5 + 520 ) );
	}
	else
	{
		Sleep( 1000 );
		SetTlsForMe( );
		return;
	}
}


void TextPrint( char *szText, float fDuration )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "PrintText" );
#endif
	UINT32 dwDuration = *( ( UINT32 * ) &fDuration );
	UINT32 GAME_GlobalClass = GameDll + 0xBE6350;
	UINT32 GAME_PrintToScreen = GameDll + 0x357640;
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


time_t lastaccess = 0;

void TextPrintUnspammed( char *szText )
{
	time_t nextaccess = time( 0 );
	if ( nextaccess > lastaccess + 3 )
	{
		float fDuration = 2.0f;
		lastaccess = nextaccess;
#ifdef BOTDEBUG
		PrintDebugInfo( "PrintText" );
#endif
		UINT32 dwDuration = *( ( UINT32 * ) &fDuration );
		UINT32 GAME_GlobalClass = GameDll + 0xAB4F80;
		UINT32 GAME_PrintToScreen = GameDll + 0x2F8E40;
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
UINT GetUnitCountAndUnitArray( int ** unitarray )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "GetUnitArray" );
#endif
	int GlobalClassOffset = *( int* ) ( GameDll + 0xAB4F80 );
	int UnitsOffset1 = *( int* ) ( GlobalClassOffset + 0x3BC );
	int UnitsCount = *( int* ) ( UnitsOffset1 + 0x604 );
	if ( UnitsCount > 0 && UnitsOffset1 > 0 )
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
	return 0;
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
	void * abiladdr = GetAbility( unitaddr, 0, id, 0, 1, 1, 1 );
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
	void * abiladdr = GetAbility( unitaddr, 0, id, 0, 1, 1, 1 );
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
	void * abiladdr = GetAbility( unitaddr, 0, id, 0, 1, 1, 1 );
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


float DEGTORAD = 3.14159f / 180.0f;
float RADTODEG = 180.0f / 3.14159f;


#define ADDR(X,REG)\
	__asm MOV REG, DWORD PTR DS : [ X ] \
	__asm MOV REG, DWORD PTR DS : [ REG ]

void SendMoveAttackCommand( int cmdId, float X, float Y )
{
	int _MoveAttackCmd = GameDll + 0x339DD0;
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
	Sleep( 2 );
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
	Sleep( 2 );
}


int CmdPointAddr;
void  sub_6F339CC0my( int a1, int a2, float a3, float a4, int a5, int a6 )
{
	CmdPointAddr = GameDll + 0x339CC0;
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
	Sleep( 2 );
}

void  ItemOrSkillPoint( int cmd, int itemorskilladdr, float x, float y, int a5, BOOL addque = FALSE )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "ItemSkillPOint" );
#endif
	sub_6F339CC0my( cmd, itemorskilladdr, x, y, a5, addque ? 5 : 4 );
	Sleep( 2 );
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

__declspec( naked ) DWFP __cdecl JJCos( float *radians )
{
	_asm
	{
		mov eax, GameDll
			add eax, 0x3B2A30
			jmp eax
	}
}

__declspec( naked ) DWFP __cdecl JJSin( float *radians )
{
	_asm
	{
		mov eax, GameDll
			add eax, 0x3B2A10
			jmp eax
	}
}

/*Location GetNextPoint( float x , float y , float distance , float angle )
{
#ifdef BOTDEBUG
PrintDebugInfo( "Next Location" );
#endif
float PIPI = 3.14159f;
float degtorad = PIPI / 180.0f;
Location returnlocation = Location( );

float dectoangle = angle * degtorad;
returnlocation.X = x + ( distance * ( float ) JJCos( &dectoangle ).fl );
returnlocation.Y = y + ( distance * ( float ) JJSin( &dectoangle ).fl );
return returnlocation;
}*/


Location GetNextPoint( float x, float y, float distance, float angle )
{
	double PIPI = 3.14159;
	double degtorad = PIPI / 180.0;


	Location returnlocation = Location( );
	returnlocation.X = x + distance * ( float ) cos( angle * DEGTORAD );
	returnlocation.Y = y + distance * ( float ) sin( angle * DEGTORAD );
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

	return ( int ) ( ( 100.0f / GetUnitMaxHP( unitaddr ) ) * GetUnitHP( unitaddr ) );
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
	return -1;
}



BOOL IsNotBadUnit( int unitaddr, BOOL onlymemcheck = FALSE )
{

	if ( unitaddr > 0 && IsOkayPtr( ( void* ) unitaddr ) )
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
		return returnvalue;
	}
#ifdef BOTDEBUG
	PrintDebugInfo( "CheckBadUnit - BAD UNIT" );
#endif
	return FALSE;
}

float GetAngleBetweenPoints( float x1, float y1, float x2, float y2 )
{
	return atan2( y2 - y1, x2 - x1 ) * RADTODEG;
}

float Distance( float dX0, float dY0, float dX1, float dY1 )
{
	return sqrt( ( dX1 - dX0 )*( dX1 - dX0 ) + ( dY1 - dY0 )*( dY1 - dY0 ) );
}

BOOL Enable3DPoint = FALSE;

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
		return ( float ) sqrt( ( ( double ) x2 - ( double ) x1 )*( ( double ) x2 - ( double ) x1 ) + ( ( double ) y2 - ( double ) y1 )*( ( double ) y2 - ( double ) y1 ) );
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





int JMPADDR = GameDll + 0x3C791A;
int xlocalplayernumber;
int xlocalplayer;
unsigned int xunknown1;
int xselectunitaddaddr;
int xselectunitnormaladdr;
int xupdateselection;

int xxxoffset1 = 0;
int xxxoffset2 = 0;

__declspec( naked ) void __cdecl AlternativeSelectUnit( int unitaddr, BOOL add )
{
	xxxoffset1 = GameDll + 0x3BDCB0;
	xxxoffset2 = GameDll + 0x3C791A;
	__asm
	{
		MOV ECX, 0x100001;
		PUSH EBX;
		CALL xxxoffset1;
		MOV EAX, unitaddr;
		JMP xxxoffset2;
	}
}

void __cdecl sub_6F3C7910( int xunitaddr, BOOL xflag )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "Need Select Unit" );
#endif
	if ( !xflag )
		ClearSelection( );
	AlternativeSelectUnit( xunitaddr, TRUE );
	return;

}


/*
void __cdecl sub_6F3C7910( int unitaddr , BOOL add )
{
int JumpAddr = 0x3C791C + GameDll;
__asm
{
PUSH eax;
PUSH ebx;
mov eax , unitaddr;
mov ebx , unitaddr;
CALL JumpAddr;
}

return;
}*/


BOOL SelectMultipleUnits( int * units, unsigned int size )
{
	int selectcount = 0;
#ifdef BOTDEBUG
	PrintDebugInfo( "Select units" );
#endif
	BOOL fistselect = FALSE;
	for ( unsigned int i = 0; i < size; i++ )
	{
		if ( IsNotBadUnit( units[ i ] ) )
		{
			selectcount++;
			if ( !fistselect )
			{
				fistselect = TRUE;
				sub_6F3C7910( units[ i ], FALSE );
			}
			else
			{
				sub_6F3C7910( units[ i ], TRUE );
			}
			Sleep( 3 );
		}
	}

	return selectcount > 0;
}


float GetProtectForUnit( int unitaddr )
{
	float Armor = 0.1f;

	if ( IsOkayPtr( ( void* ) ( unitaddr + 0xE0 ) ) )
	{
		Armor = *( float* ) ( unitaddr + 0xE0 );
	}

	return ( Armor * 0.06f ) / ( 1 + Armor * 0.06f ); // 1 = 0.06
}
void * abiladdr;

int GetUnitAbilityLevel( int unitaddr, int id, BOOL checkavaiable = FALSE, BOOL CheckMemAccess = FALSE )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "GetAbilityLevel" );
#endif


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



float GetDmgForUnit( int unitaddr, float dmg, int dmgtype = 1/* 1 : magical , 2 : physical*/ )
{

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


	if ( IsNotBadUnit( unitaddr, TRUE ) && dmgtype == 1 && ( GetUnitAbilityLevel( unitaddr, 'B014', FALSE, TRUE ) ||
		GetUnitAbilityLevel( unitaddr, 'B041', FALSE, TRUE ) ||
		GetUnitAbilityLevel( unitaddr, 'B08J', FALSE, TRUE ) ||
		GetUnitAbilityLevel( unitaddr, 'B0FP', FALSE, TRUE ) ||
		GetUnitAbilityLevel( unitaddr, 'A2T4', FALSE, TRUE ) ||
		GetUnitAbilityLevel( unitaddr, 'B0FQ', FALSE, TRUE )
		) )
		return 0.0f;


	float outdmg = 0.0f;
	if ( dmgtype == 1 )
		outdmg = dmg * ( ( 100.0f - 26.0f ) / 100.0f );
	else if ( dmgtype == 2 )
		outdmg = dmg - dmg * GetProtectForUnit( unitaddr );
	// Jugger(Nbbc) if skill A05G not exist - 100% protect

	outdmg -= 25.0f;

	//H00J H00I



	if ( dmgtype == 1 && IsClassEqual( unitaddr, "Nbbc" ) )
	{

		if ( IsNotBadUnit( unitaddr, TRUE ) && GetUnitAbilityLevel( unitaddr, 'A05G', TRUE ) == 0 )
		{
			return 0.0f;
		}
	}

	if ( dmgtype == 1 && ( IsClassEqual( unitaddr, "H00I" ) || IsClassEqual( unitaddr, "H00J" ) ) )
	{
		outdmg = outdmg * ( ( 100.0f - 10.5f ) / 100.0f );
	}


	if ( !IsNotBadUnit( unitaddr, TRUE ) )
		return 0.0f;
	int medusashield = GetUnitAbilityLevel( unitaddr, 'BNms' );
	if ( medusashield > 0 )
	{
		outdmg = outdmg * ( ( 100.0f - 50.0f ) / 100.0f );
	}
	if ( !IsNotBadUnit( unitaddr, TRUE ) )
		return 0.0f;
	int huskarlvl = GetUnitAbilityLevel( unitaddr, 'A0QQ' );
	if ( dmgtype == 1 && huskarlvl > 0 )
	{
		float dmgprotectlvl = ( float ) huskarlvl + 3.0f;
		int hppercent = 100 - GetUnitHPPercent( unitaddr );
		int parts = hppercent / 7;
		dmgprotectlvl = dmgprotectlvl * parts;
		outdmg = outdmg * ( ( 100.0f - dmgprotectlvl ) / 100.0f );
	}
	if ( !IsNotBadUnit( unitaddr, TRUE ) )
		return 0.0f;
	int pudgelvl = GetUnitAbilityLevel( unitaddr, 'A06D' );
	if ( dmgtype == 1 && pudgelvl > 0 )
	{
		// 1 lvl = (1*2)+4 = 6
		// 2 lvl = (2*2)+4 = 8
		// 3 lvl = (3*2)+4 = 10
		// 4 lvl = (4*2)+4 = 12
		float dmgprotect = ( pudgelvl * 2.0f ) + 4.0f;
		outdmg = outdmg * ( ( 100.0f - dmgprotect ) / 100.0f );
	}
	if ( !IsNotBadUnit( unitaddr, TRUE ) )
		return 0.0f;
	int spectralvl = GetUnitAbilityLevel( unitaddr, 'A0NA' );
	if ( dmgtype == 1 && spectralvl > 0 )
	{
		// 1 lvl = (1*4)+6 = 10
		// 2 lvl = (2*4)+6 = 12
		// 3 lvl = (3*4)+6 = 16
		// 4 lvl = (4*4)+6 = 22
		float dmgprotect = ( spectralvl * 4.0f ) + 6.0f;
		outdmg = outdmg * ( ( 100.0f - dmgprotect ) / 100.0f );
	}
	if ( !IsNotBadUnit( unitaddr, TRUE ) )
		return 0.0f;


	int maginalvl = GetUnitAbilityLevel( unitaddr, 'A0KY' );
	if ( dmgtype == 1 && maginalvl > 0 )
	{
		// 1 lvl = (1*8)+18 = 26
		// 2 lvl = (2*8)+18 = 34
		// 3 lvl = (3*8)+18 = 42
		// 4 lvl = (4*8)+18 = 50
		float dmgprotect = ( maginalvl * 8.0f ) + 18.0f;
		outdmg = outdmg * ( ( 100.0f - dmgprotect ) / 100.0f );
	}
	if ( !IsNotBadUnit( unitaddr, TRUE ) )
		return 0.0f;

	int viperlvl = GetUnitAbilityLevel( unitaddr, 'A0MM' );
	if ( dmgtype == 1 && viperlvl > 0 )
	{
		// 1 lvl = (1*5)+5 = 10
		// 2 lvl = (2*5)+5 = 15
		// 3 lvl = (3*5)+5 = 20
		// 4 lvl = (4*5)+5 = 25
		float dmgprotect = ( viperlvl * 5.0f ) + 5.0f;
		outdmg = outdmg * ( ( 100.0f - dmgprotect ) / 100.0f );
	}
	if ( !IsNotBadUnit( unitaddr, TRUE ) )
		return 0.0f;

	if ( dmgtype == 1 )
	{
		for ( int i = 0; i < 6; i++ )
		{
			int nitem = GetItemInSlot( unitaddr, 0, i );
			if ( nitem )
			{
				if ( IsClassEqual( nitem, "I04P" ) )
				{
					outdmg = outdmg * ( ( 100.0f - 15.0f ) / 100.0f );
				}
				else if ( IsClassEqual( nitem, "I0K6" ) )
				{
					outdmg = outdmg * ( ( 100.0f - 30.0f ) / 100.0f );
				}
			}
		}
	}
	else if ( dmgtype == 2 )
	{

	}

	return outdmg;
}





/*DWFP __cdecl GetUnitFacingReal(int unitaddr )
{
DWFP returnvalue;

__asm
{
sub esp , 8
mov eax , unitaddr;
call jumpfaceaddr;
}
return returnvalue;
}*/

typedef int( __fastcall * msub_6F6EEE20 )( int a1, int a2, void* a3 );
msub_6F6EEE20 sub_6F6EEE20;


int faceoffest1;
int faceoffest2;
int faceoffest3;
int faceoffest4;
int faceoffest5;

__declspec( naked ) DWFP __cdecl GetUnitFacingReal( int unitaddr )
{
	faceoffest5 = unitaddr;
	faceoffest1 = GameDll;
	faceoffest1 += 0x3BDCB0;
	faceoffest2 = GameDll;
	faceoffest2 += 0x3FA30;
	faceoffest3 = GameDll;
	faceoffest3 += 0xAAE60C;
	faceoffest4 = GameDll;
	faceoffest4 += 0x6EEE20;
	if ( !faceoffest5 )
	{
		__asm
		{
			sub esp, 0x08;
			mov eax, 0;
			add esp, 0x08;
			ret;
		}
	}

	__asm
	{
		mov ecx, 0x100001;
		sub esp, 0x08;
		push esi;
		call faceoffest1;
		mov eax, faceoffest5;
		mov esi, eax;
		mov edx, [ esi + 0x10 ];
		mov ecx, [ esi + 0x0C ];
		call faceoffest2;
		xor ecx, ecx;
		cmp[ eax + 0x0C ], 0x2B61676C;
		setne cl;
		sub ecx, 0x01;
		and ecx, eax;
		mov eax, ecx;
		mov edx, [ esi ];
		mov eax, [ edx + 0x000000B8 ];
		mov ecx, esi;
		call eax;
		mov edx, [ eax ];
		mov edx, [ edx + 0x1C ];
		lea ecx, [ esp + 0x08 ];
		push ecx;
		mov ecx, eax;
		call edx;
		push faceoffest3;
		mov edx, eax;
		lea ecx, [ esp + 0x08 ];
		call faceoffest4;
		mov eax, [ esp + 0x04 ];
		pop esi;
		add esp, 0x08;
		ret;
	}
}



float __cdecl GetUnitFacing( int unitaddr )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "UnitFace" );
#endif

	return GetUnitFacingReal( unitaddr ).fl;
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

int techiesaddr = 0;

BOOL SelectTechies( )
{
	BOOL returnvalue = FALSE;
	if ( techiesaddr > 0 )
	{
		if ( GetSelectedOwnedUnit( ) == techiesaddr )
			return TRUE;

		if ( IsNotBadUnit( techiesaddr ) )
		{
#ifdef BOTDEBUG
			PrintDebugInfo( "Techies selected" );
#endif
			std::vector<int> unitstoselect;
			unitstoselect.push_back( techiesaddr );
			returnvalue = SelectMultipleUnits( &unitstoselect[ 0 ], unitstoselect.size( ) );
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


	Sleep( 3 );
	return returnvalue;
}


struct TechiesActionStruct
{
	int action;
	int data;
};
std::vector<BombStruct> BombList;
BOOL EnableAutoExplode = FALSE;
BOOL EnableDagger = FALSE;
BOOL FunModeEnabled = FALSE;
BOOL _Daggercooldown = FALSE;
BOOL EnableForceStaff = TRUE;
BOOL TechiesAlly = FALSE;
BOOL _Forcecooldown = FALSE;
int forcestaffcooldown = 0;
int daggercooldown = 0;
int resettimer = 100;
int justsleeptimer = 8888;

BOOL IsFirstFun = TRUE;
char OldTechiesMissile[ 256 ];
int oldlen = 0;
char * funtechiesmissile = "Units\\Other\\Orccar\\Orccar.mdl\x0\xA\x0\xA";

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

int GetMyUnitWithForceStaff( )
{
	int * unitsarray = 0;
	int UnitsCount = GetUnitCountAndUnitArray( &unitsarray );
	//Поиск течиса
	if ( UnitsCount > 0 && unitsarray )
	{

		for ( int i = 0; i < UnitsCount; i++ )
		{
			if ( unitsarray[ i ] )
			{
				if ( IsNotBadUnit( unitsarray[ i ] ) )
				{
					if ( IsHero( unitsarray[ i ] ) > 0 )
					{
						if ( GetLocalPlayerNumber( ) == GetUnitOwnerSlot( unitsarray[ i ] ) )
						{
							for ( int i = 0; i < 6; i++ )
							{
								int itemaddr = GetItemInSlot( unitsarray[ i ], 0, i );
								if ( itemaddr > 0 && IsClassEqual( itemaddr, "I0HI" ) )
								{
									return unitsarray[ i ];
								}
							}
						}
					}
				}
			}
		}
	}

	return 0;
}


void SetTechiesNewAddrForAlly( )
{
	NeedOldTechies = TRUE;
	if ( TechiesAlly )
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
		else if ( !IsNotBadUnit( ownunitaddr, TRUE ) || ( !IsHero( ownunitaddr ) && !IsUnitDead( ownunitaddr ) ) )
		{
			ownunitaddr = 0;
		}
	}
}

void SetOldTechiesAddr( )
{
	if ( NeedOldTechies && TechiesAlly )
	{
		NeedOldTechies = FALSE;
		if ( techiesoldaddr != 0 )
			techiesaddr = techiesoldaddr;
	}
}
char abilfound[ 128 ];

typedef int *( __fastcall * sub_6F408910 )( int a1, int unused );
sub_6F408910 sub_6F4089C0_org;
sub_6F408910 sub_6F4089C0_ptr;

DWORD latesttick = GetTickCount( );

float BetWe2[ 5 ];
int id = 0;

extern "C" __declspec( dllexport ) float GetLatestFloat( )
{
	return BetWe2[0];
}

int *__fastcall sub_6F4089C0my( int a1, int unused )
{
	
	if ( id > 4 )
		id = 0;
	

	BetWe2[id] = ( float ) ( GetTickCount( ) - latesttick );

	latesttick = GetTickCount( );

	id++;

	return sub_6F4089C0_ptr( a1, unused );
}

void JustTechies( )
{
	Sleep( 1000 );
	while ( !GetSelectedOwnedUnit( ) )
	{

		Sleep( 1000 );

	}



	/*int addr1 = GetTypeInfo( 'mlbH', 0 );
	int addr2 = GetTypeInfo( 'Hblm', 0 );

	char data[ 120 ];
	sprintf_s( data, 120, "Addr:%X,%X", addr1, addr2 );



	TextPrint( data, 20.0f );
	Sleep( 5000 );

	return;*/
	int myunit = GetSelectedOwnedUnit( );
	/*
	int seconds = 0;
	char printt[ 100 ];
	if ( IsAbilityCooldown( myunit, 'AHds') )
	{
	TextPrint( "|cFF00FF00Use Found!!|r", 3.0f );
	while ( IsAbilityCooldown( myunit, 'AHds' ) )
	{
	Sleep( 1000 );
	seconds++;

	sprintf_s( printt, 100, "%i second. Please wait...", seconds );
	TextPrint( printt, 3.0f );
	}



	sprintf_s( printt, 100, "%i second", seconds );
	TextPrint( printt, 3.0f );

	return;
	}

	return;
	*/
	char * brutearray = "*QWERTYUIOPASDFGHJKLZXCVBNM1234567890qwerasdfzxcvtyuighjkbnmopl!@#$%^&*(){}[]-=\\/_|+-<>?,.:;'\"_\0";
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

					if ( GetUnitAbilityLevel( myunit, testabilcode ) > 0 )
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

	TextPrint( "|cFF00FF00END!|r", 3.0f );
	while ( GetSelectedOwnedUnit( ) )
	{

		Sleep( 1000 );

	}


}


void TechiesThread( )
{

	Sleep( 3500 );

	GameDll = ( int ) GetModuleHandle( "Game.dll" );
	if ( !GameDll )
	{
		MessageBox( 0, "Error no game.dll found", "Error", MB_OK );
		return;
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
		 GetModuleHandle( "UnrealBot.mix" ) )
	{
		ShowWindow( FindWindow( "Warcraft III", 0 ), SW_MINIMIZE );

		MessageBox( 0, "Идиот детектед? :)\n Переустановите бота!", "Обнаружен плохой бот!", MB_OK );

		Sleep( 5000 );

		ExitProcess( 0 );

		return;
	}


	_W3XGlobalClass = GameDll + 0xBE6350;
	// Получаем координаты юнита


	ClearSelection = ( mClearSelection ) ( GameDll + 0x3BBAA0 );
	GetItemInSlot = ( sub_6F26EC20 ) ( GameDll + 0x26EC20 );
	GetAbility = ( sub_6F0787D0 ) ( GameDll + 0x46F440 );
	PingMinimap = ( mPingMinimap ) ( GameDll + 0x3B4650 );
	GetTypeInfo = ( sub_6F32C880 ) ( GameDll + 0x327020 );
#ifdef BOTDEBUG
	PrintDebugInfo( "StartThread" );
#endif
	Sleep( 3000 );
#ifdef BOTDEBUG 
	PrintDebugInfo( "StartTechiesThread" );
#endif

	SetTlsForMe( );
	while ( TRUE )
	{
		if ( IsGame( ) )
		{
			JustTechies( );
		}
		else
		{
			TechiesFound = FALSE;
			techiesaddr = 0;
			Sleep( 3000 );
			while ( !IsGame( ) )
			{
				Sleep( 2000 );
			}

		}
	}
	return;
}

HANDLE ttt;


HANDLE TechiesThrID;

DWORD __stdcall temptechiesthread( LPVOID )
{
	while ( true )
	{
		try
		{
			TechiesThread( );
		}
		catch ( ... )
		{
			try
			{
				ttt = CreateThread( 0, 0, temptechiesthread, 0, 0, 0 );
				return 0;
			}
			catch ( ... )
			{
				MessageBox( 0, "Unreal Fatal Error", "OIIIUbKA", 0 );
			}
			return 0;
		}
		Sleep( 2000 );
	}
	return 0;
}



BOOL WINAPI DllMain( HINSTANCE hDLL, UINT reason, LPVOID reserved )
{
	if ( reason == DLL_PROCESS_ATTACH )
	{
		MH_Initialize( );
		GameDll = ( int ) GetModuleHandle( "Game.dll" );

		sub_6F4089C0_org = ( sub_6F408910 ) ( GameDll + 0x408910 );
		MH_CreateHook( sub_6F4089C0_org, &sub_6F4089C0my, reinterpret_cast< void** >( &sub_6F4089C0_ptr ) );
		MH_EnableHook( sub_6F4089C0_org );
		//ttt = CreateThread( 0, 0, temptechiesthread, 0, 0, 0 );
	}
	else if ( reason == DLL_PROCESS_DETACH )
	{
		MH_Uninitialize( );
		//TerminateThread( ttt, 0 );
	}
	return TRUE;
}
