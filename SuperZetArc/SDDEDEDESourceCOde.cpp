
#pragma region Headers
#define _WIN32_WINNT 0x0501 
#define WINVER 0x0501 
#define NTDDI_VERSION 0x05010000
//#define BOTDEBUG
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#pragma endregion

// Game.dll address
int GameDll = 0;
int _W3XGlobalClass;


BOOL UseWarnIsBadReadPtr = FALSE;

BOOL IsOkayPtr( void *ptr , unsigned int size = 4 )
{
	if ( UseWarnIsBadReadPtr )
		return IsBadReadPtr( ptr , size ) == 0;

	MEMORY_BASIC_INFORMATION mbi;
	if ( VirtualQuery( ptr , &mbi , sizeof( MEMORY_BASIC_INFORMATION ) ) == 0 )
		return FALSE;

	if ( ( int ) ptr + size > ( int ) mbi.BaseAddress + mbi.RegionSize )
		return FALSE;

	if ( ( int ) ptr < ( int ) mbi.BaseAddress )
		return FALSE;

	if ( mbi.State != MEM_COMMIT )
		return FALSE;

	if ( mbi.Protect != PAGE_READWRITE &&  mbi.Protect != PAGE_WRITECOPY && mbi.Protect != PAGE_READONLY )
		return FALSE;

	return TRUE;
}
union DWFP
{
	DWORD dw;
	float fl;
};

#ifdef BOTDEBUG
BOOL DEBUGSTARTED = FALSE;
char * laststringinfo = new char[ 256 ];
char * oldlaststringinfo = new char[ 256 ];
char * oldoldlaststringinfo = new char[ 256 ];
void UpdateDebug( )
{
	sprintf_s( laststringinfo , 256 , "%s" , "InitLogSystem" );
	sprintf_s( oldlaststringinfo , 256 , "%s" , "CreateFile" );
	FILE* pFile;
	fopen_s( &pFile , ".\\dbgaddr.bin" , "wb" );
	if ( pFile )
	{
		fwrite( &laststringinfo , 4 , 1 , pFile );
		fseek( pFile , 4 , SEEK_SET );
		fwrite( &oldlaststringinfo , 4 , 1 , pFile );
		fseek( pFile , 8 , SEEK_SET );
		fwrite( &oldoldlaststringinfo , 4 , 1 , pFile );
		fclose( pFile );
	}
}

void PrintDebugInfo( const char * debuginfo )
{
	sprintf_s( oldoldlaststringinfo , 256 , "%s" , oldlaststringinfo );
	sprintf_s( oldlaststringinfo , 256 , "%s" , laststringinfo );
	sprintf_s( laststringinfo , 256 , "%s" , debuginfo );
	if ( !DEBUGSTARTED )
	{
		DEBUGSTARTED = TRUE;
		UpdateDebug( );
	}
}

void PrintDebugInfo2( const char * debuginfo, int data )
{
	sprintf_s( oldoldlaststringinfo , 256 , "%s" , oldlaststringinfo );
	sprintf_s( oldlaststringinfo , 256 , "%s" , laststringinfo );
	sprintf_s( laststringinfo , 256 , "%s:%X" , debuginfo,data );
	if ( !DEBUGSTARTED )
	{
		DEBUGSTARTED = TRUE;
		UpdateDebug( );
	}
}




#endif

#define IsKeyPressed(CODE) (GetAsyncKeyState(CODE) & 0x8000) > 0



int Arcaddr = 0;

typedef void( __cdecl * mClearSelection )( );
mClearSelection ClearSelection;
BOOL IsGame( ) // my offset + public
{
	return ( *( int* ) ( ( UINT32 ) GameDll + 0xACF678 ) > 0 || *( int* ) ( ( UINT32 ) GameDll + 0xAB62A4 ) > 0 )/* && !IsLagScreen( )*/;
}

// get thread access (for Jass natives / other functions)
void SetTlsForMe( )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "Set thread access" );
#endif
	UINT32 Data = *( UINT32 * ) ( GameDll + 0xACEB4C );
	UINT32 TlsIndex = *( UINT32 * ) ( GameDll + 0xAB7BF4 );
	if ( TlsIndex )
	{
		UINT32 v5 = **( UINT32 ** ) ( *( UINT32 * ) ( *( UINT32 * ) ( GameDll + 0xACEB5C ) + 4 * Data ) + 44 );
		if ( !v5 || !( *( LPVOID * ) ( v5 + 520 ) ) )
		{
			Sleep( 1000 );
			SetTlsForMe( );
			return;
		}
		TlsSetValue( TlsIndex , *( LPVOID * ) ( v5 + 520 ) );
	}
	else
	{
		Sleep( 1000 );
		SetTlsForMe( );
		return;
	}
}


void TextPrint( char *szText , float fDuration )
{
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
			MOV		ECX , [ GAME_GlobalClass ]
			MOV		ECX , [ ECX ]
			CALL	GAME_PrintToScreen
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
	if ( UnitsCount > 0 )
	{
		*unitarray = ( int * ) *( int* ) ( UnitsOffset1 + 0x608 );
	}
#ifdef BOTDEBUG
	PrintDebugInfo( "EndGetUnitArray" );
#endif
	return UnitsCount;
}


typedef int( __fastcall * sub_6F26EC20 )( int unitaddr , int unused , unsigned int SLOTID );
sub_6F26EC20 GetItemInSlot;

//
typedef int( __fastcall * sub_6F0787D0 )( int unitaddr , int unused , int classid , int a3 , int a4 , int a5 , int a6 );
sub_6F0787D0 GetUnitAbilLevelReal;

int GetUnitAbilityLevel( int unitaddr , int id )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "GetAbilityLevel" );
#endif
	int abiladdr = GetUnitAbilLevelReal( unitaddr , 0 , id , 0 , 1 , 1 , 1 );
	if ( !abiladdr || !IsOkayPtr( ( void * ) abiladdr ) )
		return 0;

	int abilleveladdr = abiladdr + 80;
	if ( IsOkayPtr( ( void* ) abilleveladdr ) )
		return *( int * ) ( abiladdr + 80 ) + 1;
	else
		return 0;
}

int GetGlobalPlayerData( )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "GetGlobalPlayerData" );
#endif
	return *( int* ) ( 0xAB65F4 + GameDll );
}


int GetPlayerByNumber( int number )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "GetPlayerByNumber" );
#endif
	int arg1 = GetGlobalPlayerData( );
	int result = -1;
	if ( arg1 )
	{
		result = arg1 + ( number * 4 ) + 0x58;

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
	if ( GetGlobalPlayerData( ) )
	{
		int playerslotaddr = GetGlobalPlayerData( ) + 0x28;
		if ( IsOkayPtr( ( void* ) playerslotaddr ) )
			return ( int ) *( short * ) ( playerslotaddr );
		else
			return 0;
	}
	else
		return 0;
}


int GetLocalPlayer( )
{
	return GetPlayerByNumber( GetLocalPlayerNumber( ) );
}

BOOL DisableWindowActiveCheck = FALSE;

BOOL IsWindowActive( )
{
	if ( DisableWindowActiveCheck )
		return DisableWindowActiveCheck;

	return *( BOOL* ) ( GameDll + 0xA9E7A4 );
}

UINT GetUnitOwnerSlot( int unitaddr )
{
	return *( int* ) ( unitaddr + 88 );
}

BOOL IsPlayerEnemy( int unitaddr )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "GetPlayerEnemy" );
#endif
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

void GetMousePosition( float * x , float * y , float * z )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "Mouse info" );
#endif
	int offset1 = *( int* ) _W3XGlobalClass + 0x3BC;
	if ( offset1 != 0x3BC )
	{
		offset1 = *( int * ) offset1;
		if ( offset1 )
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

int GetSelectedOwnedUnit( )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "GetOwningSelectedUnit" );
#endif

	UINT32 plr = GetLocalPlayer( );
	if ( plr )
	{

		int unitaddr = 0; // = *(int*)((*(int*)plr+0x34)+0x1e0);

		__asm
		{
			MOV EAX , plr;
			MOV ECX , DWORD PTR DS : [ EAX + 0x34 ];
			MOV EAX , DWORD PTR DS : [ ECX + 0x1E0 ];
			MOV unitaddr , EAX;
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

#define BTN_Arc_SUICIDE 0xD0048
#define BTN_TINI_TOSS 0xD0278
#define BTN_TRAXES_FORCE 0xD02BD
#define BTN_VENG_SWAP 0xD026B

BOOL IsBtnCooldown1( int buttonid )
{

}


BOOL IsBtnCooldown2( int buttonid )
{

}

typedef signed int( __fastcall * sub_6F377CD0 )( int arg1 , int unused , int a2 , int a3 , int a4 , int a5 );
sub_6F377CD0 GetCooldownBtn = NULL;

BOOL GetBtnCooldown( int btnid , int itemid , BOOL type )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "GetCooldownBTN v2" );
#endif
	int arg0 = GetLocalPlayer( );
	if ( arg0 )
	{

		int arg1 = *( int* ) ( GetLocalPlayer( ) + 0x34 );
		int arg2 = btnid;
		int arg3 = GetLocalPlayerNumber( );
		if ( !GetCooldownBtn )
			GetCooldownBtn = ( sub_6F377CD0 ) ( GameDll + 0x377CD0 );
		return GetCooldownBtn( arg1 , itemid , arg2 , arg3 , itemid , type == 0 ) > 0;
	}
	return 0xDD;
}

BOOL IsForceActiveItemCooldown( int btnid , int item )
{
	return GetBtnCooldown( btnid , item , 1 );
}

/*
BOOL IsSuicideCooldown( int btnid )
{
return GetBtnCooldown( BTN_Arc_SUICIDE , 1 );
}
*/


int GetUnitClass( int unit_or_item_addr )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "GetUnitClass" );
#endif
	return *( int* ) ( unit_or_item_addr + 0x30 );
}

BOOL IsClassEqual( int unit_or_item_addr , char * classid )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "UnitCLass" );
#endif
	char unitclass[ 5 ];
	memset( unitclass , 0 , 5 );
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
	__asm MOV REG , DWORD PTR DS : [ X ] \
	__asm MOV REG , DWORD PTR DS : [ REG ]

void SendMoveAttackCommand( int cmdId , float X , float Y )
{
	int _MoveAttackCmd = GameDll + 0x339DD0;
	__asm
	{
		ADDR( _W3XGlobalClass , ECX );
		MOV ECX , DWORD PTR DS : [ ECX + 0x1B4 ];
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

BOOL SkipCooldownCheck = TRUE;


BOOL IsItemCooldown( int itemaddr )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "CheckItemCooldownv1" );
#endif
	if ( itemaddr == NULL )
		return TRUE;

	if ( SkipCooldownCheck )
	{
		return FALSE;
	}
	return !( ( *( UINT* ) ( itemaddr + 32 ) ) & 0x400u );
}

int commandjumpaddr;
void  sub_6F339DD0my( int a1 , int a2 , float a3 , float a4 , int a5 , int a6 , int a7 )
{
	commandjumpaddr = GameDll + 0x339DD0;
	__asm
	{

		PUSH a7;
		PUSH a6;
		PUSH a5;
		PUSH a4;
		ADDR( _W3XGlobalClass , ECX );
		MOV ECX , DWORD PTR DS : [ ECX + 0x1B4 ];
		PUSH a3;
		PUSH a2;
		PUSH a1;

		CALL commandjumpaddr;
	}
	Sleep( 2 );
}



void  CommandItemTarget( int cmd , int itemaddr , float targetx , float targety , int targetunitaddr , BOOL queue = FALSE )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "UseIt" );
#endif
	sub_6F339DD0my( cmd , itemaddr , targetx , targety , targetunitaddr , 4 , queue ? 5 : 4 );
}

int CmdWOTaddr;
void  sub_6F339C60my( int a1 , int a2 , unsigned int a3 , unsigned int a4 )
{
	CmdWOTaddr = GameDll + 0x339C60;
	__asm
	{

		PUSH a4;
		PUSH a3;
		ADDR( _W3XGlobalClass , ECX );
		MOV ECX , DWORD PTR DS : [ ECX + 0x1B4 ];
		PUSH a2;
		PUSH a1;

		CALL CmdWOTaddr;
	}
	Sleep( 2 );
}


int CmdPointAddr;
void  sub_6F339CC0my( int a1 , int a2 , float a3 , float a4 , int a5 , int a6 )
{
	CmdPointAddr = GameDll + 0x339CC0;
	__asm
	{

		PUSH a6;
		PUSH a5;
		PUSH a4;
		ADDR( _W3XGlobalClass , ECX );
		MOV ECX , DWORD PTR DS : [ ECX + 0x1B4 ];
		PUSH a3;
		PUSH a2;
		PUSH a1;

		CALL CmdPointAddr;
	}
	Sleep( 2 );
}

void  ItemOrSkillPoint( int cmd , int itemorskilladdr , float x , float y , int a5 , BOOL addque = FALSE )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "ItemSkillPOint" );
#endif
	sub_6F339CC0my( cmd , itemorskilladdr , x , y , a5 , addque ? 5 : 4 );
	Sleep( 2 );
}

void UseDetonator( )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "Use detonate" );
#endif
	sub_6F339C60my( 0xd024c , 0 , 1 , 4 );
}


typedef void( __cdecl *mPingMinimap )( float *x , float *y , float *duration );
mPingMinimap PingMinimap;

__declspec( naked ) DWFP __cdecl JJCos( float *radians )
{
	_asm
	{
		mov eax , GameDll
			add eax , 0x3B2A30
			jmp eax
	}
}

__declspec( naked ) DWFP __cdecl JJSin( float *radians )
{
	_asm
	{
		mov eax , GameDll
			add eax , 0x3B2A10
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


Location GetNextPoint( float x , float y , float distance , float angle )
{
	double PIPI = 3.14159;
	double degtorad = PIPI / 180.0;


	Location returnlocation = Location( );
	returnlocation.X = x + distance * ( float ) cos( angle * DEGTORAD );
	returnlocation.Y = y + distance * ( float ) sin( angle * DEGTORAD );
	return returnlocation;
}



void GetUnitLocation( int unitaddr , float * x , float * y )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "Unit 2d" );
#endif
	*x = *( float* ) ( unitaddr + 0x284 );
	*y = *( float* ) ( unitaddr + 0x288 );
}

void GetUnitLocation3D( int unitaddr , float * x , float * y , float * z )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "Unit 3d" );
#endif
	*x = *( float* ) ( unitaddr + 0x284 );
	*y = *( float* ) ( unitaddr + 0x288 );
	*z = *( float* ) ( unitaddr + 0x28C );
}

float GetUnitFloatStat( int unitaddr , DWORD statNum )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "UnitFloatState" );
#endif
	int _GetFloatStat = GameDll + 0x27AE90;
	float result = 0;
	__asm
	{
		PUSH statNum;
		LEA EAX , result
			PUSH EAX
			MOV ECX , unitaddr
			CALL _GetFloatStat
	}
	return result;
}


float GetUnitHP( int unitaddr )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "Unit hp" );
#endif
	return GetUnitFloatStat( unitaddr , 0 );
}


float GetUnitMaxHP( int unitaddr )
{
	return GetUnitFloatStat( unitaddr , 1 );
}

int GetUnitHPPercent( int unitaddr )
{

	return ( int ) ( ( 100.0f / GetUnitMaxHP( unitaddr ) ) * GetUnitHP( unitaddr ) );
}


BOOL IsHero( int unitaddr )
{
	unsigned int ishero = *( unsigned int* ) ( unitaddr + 48 );

	ishero = ishero >> 24;
	ishero = ishero - 64;

	return ishero < 0x19;
}

BOOL IsNotDolboeb( int unitaddr, BOOL onlimemcheck =FALSE )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "CheckDolboeb" );
#endif
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

		if ( onlimemcheck && !IsUnitDead( unitaddr ) )
			return TRUE;

		unsigned int isdolbany = *( unsigned int* ) ( unitaddr + 0x5C );
		BOOL returnvalue = isdolbany != 0x1001u && !IsUnitDead( unitaddr ) && ( isdolbany & 0x40000000u ) == 0;
#ifdef BOTDEBUG
		PrintDebugInfo( "CheckDolboeb - ENDCHECK" );
#endif
		return returnvalue;
	}
#ifdef BOTDEBUG
	PrintDebugInfo( "CheckDolboeb - BAD UNIT" );
#endif
	return FALSE;
}

float GetAngleBetweenPoints( float x1 , float y1 , float x2 , float y2 )
{
	return atan2( y2 - y1 , x2 - x1 ) * RADTODEG;
}

float Distance( float dX0 , float dY0 , float dX1 , float dY1 )
{
	return sqrt( ( dX1 - dX0 )*( dX1 - dX0 ) + ( dY1 - dY0 )*( dY1 - dY0 ) );
}

BOOL Enable3DPoint = FALSE;

float Distance3D( float x1 , float y1 , float z1 , float x2 , float y2 , float z2 )
{
	if ( Enable3DPoint )
	{
		double d[ ] = { abs( ( double ) x1 - ( double ) x2 ) , abs( ( double ) y1 - ( double ) y2 ) , abs( ( double ) z1 - ( double ) z2 ) };
		if ( d[ 0 ] < d[ 1 ] ) std::swap( d[ 0 ] , d[ 1 ] );
		if ( d[ 0 ] < d[ 2 ] ) std::swap( d[ 0 ] , d[ 2 ] );
		return ( float ) ( d[ 0 ] * sqrt( 1.0 + d[ 1 ] / d[ 0 ] + d[ 2 ] / d[ 0 ] ) );
	}
	else
	{
		return ( float ) sqrt( ( ( double ) x2 - ( double ) x1 )*( ( double ) x2 - ( double ) x1 ) + ( ( double ) y2 - ( double ) y1 )*( ( double ) y2 - ( double ) y1 ) );
	}
}

float DistanceBetweenLocs( Location loc1 , Location loc2 )
{
	return Distance( loc1.X , loc1.Y , loc2.X , loc2.Y );
}

Location GiveNextLocationFromLocAndAngle( Location startloc , float distance , float angle )
{
	return GetNextPoint( startloc.X , startloc.Y , distance , angle );
}

BOOL ArcFound = FALSE;


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

__declspec( naked ) void __cdecl AlternativeSelectUnit( int unitaddr , BOOL add )
{
	xxxoffset1 = GameDll + 0x3BDCB0;
	xxxoffset2 = GameDll + 0x3C791A;
	__asm
	{
		MOV ECX , 0x100001;
		PUSH EBX;
		CALL xxxoffset1;
		MOV EAX , unitaddr;
		JMP xxxoffset2;
	}
}

void __cdecl sub_6F3C7910( int xunitaddr , BOOL xflag )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "Need Select Unit" );
#endif
	if ( !xflag )
		ClearSelection( );
	AlternativeSelectUnit( xunitaddr , TRUE );
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


BOOL SelectMultipleUnits( int * units , unsigned int size )
{
	int selectcount = 0;
#ifdef BOTDEBUG
	PrintDebugInfo( "Select units" );
#endif
	BOOL fistselect = FALSE;
	for ( unsigned int i = 0; i < size; i++ )
	{
		if ( IsNotDolboeb( units[ i ] ) )
		{
#ifdef BOTDEBUG
			PrintDebugInfo( "Beg unit select" );
#endif
			selectcount++;
			if ( !fistselect )
			{
				fistselect = TRUE;
#ifdef BOTDEBUG
				PrintDebugInfo( "unit select 1" );
#endif
				sub_6F3C7910( units[ i ] , FALSE );
			}
			else
			{
#ifdef BOTDEBUG
				PrintDebugInfo( "unit select 2" );
#endif
				sub_6F3C7910( units[ i ] , TRUE );
			}
			Sleep( 3 );
		}
	}

	return selectcount > 0;
}






float GetDmgForUnit( int unitaddr , float dmgx )
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

	if ( GetUnitAbilityLevel( unitaddr , 'B014' ) ||
		 GetUnitAbilityLevel( unitaddr , 'B041' ) ||
		 GetUnitAbilityLevel( unitaddr , 'B08J' ) ||
		 GetUnitAbilityLevel( unitaddr , 'B0FP' ) ||
		 GetUnitAbilityLevel( unitaddr , 'A2T4' ) ||
		 GetUnitAbilityLevel( unitaddr , 'B0FQ' )
		 )
		 return 0.0f;

	float outdmg = dmgx * ( ( 100.0f - 26.0f ) / 100.0f );


	if ( Arcaddr > 0 )
	{
		int skilladdr = GetUnitAbilityLevel( Arcaddr , 'A2LL' );
		outdmg += ( skilladdr * 50.0f );
	}

	// Jugger(Nbbc) if skill A05G not exist - 100% protect

	if ( IsClassEqual( unitaddr , "Nbbc" ) )
	{
		if ( GetUnitAbilityLevel( unitaddr , 'A05G' ) == 0 )
		{
			return 0.0f;
		}
	}

	
	int nagashield = GetUnitAbilityLevel( unitaddr , 'BNms' );
	if ( nagashield > 0 )
	{
		outdmg = outdmg * ( 50.0f / 100.0f );
	}

	int huskarlvl = GetUnitAbilityLevel( unitaddr , 'A0QQ' );
	if ( huskarlvl > 0 )
	{
		float dmgprotectlvl = ( float ) huskarlvl + 3.0f;
		int hppercent = 100 - GetUnitHPPercent( unitaddr );
		int parts = hppercent / 7;
		dmgprotectlvl = dmgprotectlvl * parts;
		outdmg = outdmg * ( ( 100.0f - dmgprotectlvl ) / 100.0f );
	}

	int pudgelvl = GetUnitAbilityLevel( unitaddr , 'A06D' );
	if ( pudgelvl > 0 )
	{
		// 1 lvl = (1*2)+4 = 6
		// 2 lvl = (2*2)+4 = 8
		// 3 lvl = (3*2)+4 = 10
		// 4 lvl = (4*2)+4 = 12
		float dmgprotect = ( pudgelvl * 2.0f ) + 4.0f;
		outdmg = outdmg * ( ( 100.0f - dmgprotect ) / 100.0f );
	}



	int maginalvl = GetUnitAbilityLevel( unitaddr , 'A0KY' );
	if ( maginalvl > 0 )
	{
		// 1 lvl = (1*8)+18 = 26
		// 2 lvl = (2*8)+18 = 34
		// 3 lvl = (3*8)+18 = 42
		// 4 lvl = (4*8)+18 = 50
		float dmgprotect = ( maginalvl * 8.0f ) + 18.0f;
		outdmg = outdmg * ( ( 100.0f - dmgprotect ) / 100.0f );
	}

	int viperlvl = GetUnitAbilityLevel( unitaddr , 'A0MM' );
	if ( viperlvl > 0 )
	{
		// 1 lvl = (1*5)+5 = 10
		// 2 lvl = (2*5)+5 = 15
		// 3 lvl = (3*5)+5 = 20
		// 4 lvl = (4*5)+5 = 25
		float dmgprotect = ( viperlvl * 5.0f ) + 5.0f;
		outdmg = outdmg * ( ( 100.0f - dmgprotect ) / 100.0f );
	}


	for ( int i = 0; i < 6; i++ )
	{
		int nitem = GetItemInSlot( unitaddr , 0 , i );
		if ( nitem )
		{
			if ( IsClassEqual( nitem , "I04P" ) )
			{
				outdmg = outdmg * ( ( 100.0f - 15.0f ) / 100.0f );
			}
			else if ( IsClassEqual( nitem , "I0K6" ) )
			{
				outdmg = outdmg * ( ( 100.0f - 30.0f ) / 100.0f );
			}
		}
	}
#ifdef BOTDEBUG
	PrintDebugInfo( "EndCheckAbilDataDmg" );
#endif

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

typedef int( __fastcall * msub_6F6EEE20 )( int a1 , int a2 , void* a3 );
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
			sub esp , 0x08;
			mov eax , 0;
			add esp , 0x08;
			ret;
		}
	}

	__asm
	{
		mov ecx , 0x100001;
		sub esp , 0x08;
		push esi;
		call faceoffest1;
		mov eax , faceoffest5;
		mov esi , eax;
		mov edx , [ esi + 0x10 ];
		mov ecx , [ esi + 0x0C ];
		call faceoffest2;
		xor ecx , ecx;
		cmp[ eax + 0x0C ] , 0x2B61676C;
		setne cl;
		sub ecx , 0x01;
		and ecx , eax;
		mov eax , ecx;
		mov edx , [ esi ];
		mov eax , [ edx + 0x000000B8 ];
		mov ecx , esi;
		call eax;
		mov edx , [ eax ];
		mov edx , [ edx + 0x1C ];
		lea ecx , [ esp + 0x08 ];
		push ecx;
		mov ecx , eax;
		call edx;
		push faceoffest3;
		mov edx , eax;
		lea ecx , [ esp + 0x08 ];
		call faceoffest4;
		mov eax , [ esp + 0x04 ];
		pop esi;
		add esp , 0x08;
		ret;
	}
}




BOOL __cdecl IsUnitVisibleToPlayer( int unitaddr , int player )
{
	if ( player )
	{
		__asm
		{
			mov esi , unitaddr;
			mov eax , player;
			movzx eax , byte ptr[ eax + 0x30 ];
			mov edx , [ esi ];
			push 0x04;
			push 0x00;
			push eax;
			mov eax , [ edx + 0x000000FC ];
			mov ecx , esi;
			call eax;
		}
	}
	else
		return FALSE;
}

float __cdecl GetUnitFacing( int unitaddr )
{
#ifdef BOTDEBUG
	PrintDebugInfo( "UnitFace" );
#endif

	return GetUnitFacingReal( unitaddr ).fl;
}


BOOL SelectArc( )
{
	BOOL returnvalue = FALSE;
	if ( Arcaddr > 0 )
	{
		if ( GetSelectedOwnedUnit( ) == Arcaddr )
			return TRUE;

		if ( IsNotDolboeb( Arcaddr ) )
		{
#ifdef BOTDEBUG
			PrintDebugInfo( "Arc selected" );
#endif
			std::vector<int> unitstoselect;
			unitstoselect.push_back( Arcaddr );
			returnvalue = SelectMultipleUnits( &unitstoselect[ 0 ] , unitstoselect.size( ) );
			unitstoselect.clear( );
		}
		else
		{
#ifdef BOTDEBUG
			PrintDebugInfo( "Arc not selected 11" );
#endif
			if ( !IsHero( Arcaddr ) && !IsUnitDead( Arcaddr ) )
			{
				Arcaddr = 0;
			}
		}
	}


	Sleep( 3 );
	return returnvalue;
}


struct ArcActionStruct
{
	int action;
	int data;
};

DWORD WINAPI ArcThread( LPVOID )
{
	Sleep( 3000 );
#ifdef BOTDEBUG 
	PrintDebugInfo( "StartArcThread" );
#endif
	std::vector<BombStruct> BombList;
	BOOL EnableAutoExplode = FALSE;
	BOOL EnableDagger = TRUE;
	BOOL _Daggercooldown = FALSE;
	BOOL EnableForceStaff = TRUE;
	BOOL ArcAlly = FALSE;
	BOOL _Forcecooldown = FALSE;

	int forcestaffcooldown = 0;
	int daggercooldown = 0;
	int resettimer = 100;
	SetTlsForMe( );
	while ( TRUE )
	{
		if ( IsGame( ) )
		{
			TextPrint( "Unreal Arc Bot v5 by Absol" , 5.0f );
			TextPrint( "Thank dracol1ch for some dota info." , 0.01f );


		retryall:
			while ( IsGame( ) )
			{
#ifdef BOTDEBUG
				PrintDebugInfo( "InGame" );
#endif
				if ( !IsWindowActive( ) )
				{
					while ( !IsWindowActive( ) )
					{
						Sleep( 10 );
					}
					goto retryall;
				}
#ifdef BOTDEBUG
				PrintDebugInfo( "Window active" );
#endif
				if ( !ArcFound || Arcaddr == 0 )
				{
#ifdef BOTDEBUG
					PrintDebugInfo( "Find Arc!" );
#endif
					int * unitsarray = 0;
					int UnitsCount = GetUnitCountAndUnitArray( &unitsarray );
					//Поиск течиса
					if ( unitsarray && UnitsCount > 0 )
					{

						for ( int i = 0; i < UnitsCount; i++ )
						{
							if ( unitsarray[ i ] )
							{
#ifdef BOTDEBUG
								PrintDebugInfo2( "UnitArrayFound" , unitsarray[ i ] );
#endif
								if ( IsNotDolboeb( unitsarray[ i ] ) )
								{
#ifdef BOTDEBUG
									PrintDebugInfo( "UnitOkay" );
#endif
									if ( IsHero( unitsarray[ i ] ) )
									{
#ifdef BOTDEBUG
										PrintDebugInfo( "Hero" );
#endif
										if ( GetLocalPlayerNumber( ) == GetUnitOwnerSlot( unitsarray[ i ] ) || !IsPlayerEnemy( unitsarray[ i ] ) )
										{
											if ( IsClassEqual( unitsarray[ i ] , "N0MK" ) )
											{
#ifdef BOTDEBUG
												PrintDebugInfo( "ClassEqual" );
#endif
												Arcaddr = unitsarray[ i ];
												ArcFound = TRUE;
												if ( GetLocalPlayerNumber( ) == GetUnitOwnerSlot( unitsarray[ i ] ) )
												{
													ArcAlly = FALSE;
#ifdef BOTDEBUG
													PrintDebugInfo( "FoundArc" );
#endif
													TextPrint( "Found own Arc! " , 3.0f );
													if ( EnableAutoExplode )
													{
														TextPrint( "AutoExplode: AutoExplode" , 3.0f );
													}
													else if ( !EnableAutoExplode )
													{
														TextPrint( "AutoExplode: AutoKill " , 3.0f );
													}
													if ( EnableForceStaff )
													{
														TextPrint( "ForceStaff: ENABLED" , 3.0f );
													}
													else if ( !EnableForceStaff )
													{
														TextPrint( "ForceStaff: DISABLED" , 3.0f );
													}
													if ( EnableDagger )
													{
														TextPrint( "Dagger: ENABLED" , 3.0f );
													}
													else if ( !EnableDagger )
													{
														TextPrint( "Dagger: DISABLED" , 3.0f );
													}
													break;
													/*	float x;
														float y;
														float dur = 3.0f;
														GetUnitLocation3D( Arcaddr , &x , &y );
														PingMinimap( &x , &y , &dur );*/
												}
												else if ( !IsPlayerEnemy( unitsarray[ i ] ) )
												{
													ArcAlly = TRUE;
													TextPrint( "Found ally Arc" , 3.0f );
													if ( EnableForceStaff )
													{
														TextPrint( "ForceStaff: ENABLED" , 3.0f );
													}
													else if ( !EnableForceStaff )
													{
														TextPrint( "ForceStaff: DISABLED" , 3.0f );
													}
													if ( EnableDagger )
													{
														TextPrint( "Dagger: ENABLED" , 3.0f );
													}
													else if ( !EnableDagger )
													{
														TextPrint( "Dagger: DISABLED" , 3.0f );
													}
												}

											}
										}
									}

								}
							}

						}
#ifdef BOTDEBUG
						PrintDebugInfo( "No Arc found" );
#endif
					}



				}

				/*resettimer--;

				if ( resettimer <= 0 )
				{
				resettimer = 100;
				INIReader reader( ".\\..\\tbc.ini" );
				if ( reader.ParseError( ) == 0 )
				{
				EnableAutoExplode = reader.GetBOOLean( "Config" , "EnableAutoExplode" , FALSE );
				EnableForceStaff = reader.GetBOOLean( "Config" , "EnableForceStaff" , FALSE );
				}
				}*/

				if ( ArcFound )
				{
#ifdef BOTDEBUG
					PrintDebugInfo( "Arc found" );
#endif
					// Очистить список мин
					BombList.clear( );

					// Сохранить список мин

					if ( ArcFound )
					{
#ifdef BOTDEBUG
						PrintDebugInfo( "Find mines" );
#endif

						int * unitsarray = 0;
						int UnitsCount = GetUnitCountAndUnitArray( &unitsarray );
						if ( unitsarray && UnitsCount > 0 )
						{

							for ( int i = 0; i < UnitsCount; i++ )
							{
#ifdef BOTDEBUG
								PrintDebugInfo2( "UnitFOund" , unitsarray[ i ] );
#endif
								if ( /* !IsIgnoreUnit( unitsarray[ i ] ) &&*/ IsNotDolboeb( unitsarray[ i ],TRUE ) )
								{
#ifdef BOTDEBUG
									PrintDebugInfo( "NormalUnit33" );
#endif
									if ( GetLocalPlayerNumber( ) == GetUnitOwnerSlot( unitsarray[ i ] ) )
									{

#ifdef BOTDEBUG
										PrintDebugInfo( "OwnedUnitOkayFindBmb" );
#endif

										// 1lvl 1skill = n00O

										// 2lvl 1skill = n00P

										// 3lvl 1skill = n00Q

										// 4lvl 1skill = n00N

										// 1lvl 4skill = o018

										// 2lvl 4skill = o002

										// 3lvl 4skill = o00B

										// 4lvl 4skill = o01B



										if ( IsClassEqual( unitsarray[ i ] , "h0EG" ) )
										{
											BombStruct tmpstr = BombStruct( );
											tmpstr.unitaddr = unitsarray[ i ];
											tmpstr.dmg = 100.0f;
											tmpstr.range1 = 350.0f;
											tmpstr.range2 = 350.0f;
											tmpstr.remote = FALSE;
											GetUnitLocation3D( unitsarray[ i ] , &tmpstr.x , &tmpstr.y , &tmpstr.z );
											/*float durdur = 0.10f;
											PingMinimap( &tmpstr.x , &tmpstr.y , &durdur );*/
											BombList.push_back( tmpstr );
										}




									}
								}

							}
							//Поиск течиса


						}
#ifdef BOTDEBUG
						PrintDebugInfo( "End find mines" );
#endif
					}

					if ( IsKeyPressed( 0x58 ) && IsKeyPressed( 0x31 ) )
					{
						while ( IsKeyPressed( 0x58 ) && IsKeyPressed( 0x31 ) )
						{
							Sleep( 10 );
						}
						EnableForceStaff = !EnableForceStaff;

						if ( EnableForceStaff )
						{
							TextPrint( "ForceStaff: ENABLED" , 3.0f );
						}
						else if ( !EnableForceStaff )
						{
							TextPrint( "ForceStaff: DISABLED" , 3.0f );
						}
					}


					if ( IsKeyPressed( 0x58 ) && IsKeyPressed( 0x32 ) )
					{
						while ( IsKeyPressed( 0x58 ) && IsKeyPressed( 0x32 ) )
						{
							Sleep( 10 );
						}
						EnableDagger = !EnableDagger;

						if ( EnableDagger )
						{
							TextPrint( "Dagger: ENABLED" , 3.0f );
						}
						else if ( !EnableDagger )
						{
							TextPrint( "Dagger: DISABLED" , 3.0f );
						}
					}

					if ( IsKeyPressed( 0x58 ) && IsKeyPressed( 0x34 ) )
					{
						while ( IsKeyPressed( 0x58 ) && IsKeyPressed( 0x34 ) )
						{
							Sleep( 10 );
						}
						SkipCooldownCheck = !SkipCooldownCheck;

						if ( !SkipCooldownCheck )
						{
							TextPrint( "Item cooldown check : Enabled" , 3.0f );
							TextPrint( "Warning! It can be detected!" , 3.0f );
						}
						else if ( SkipCooldownCheck )
						{
							TextPrint( "Item cooldown check : SKIP" , 3.0f );
						}
					}

					if ( IsKeyPressed( 0x58 ) && IsKeyPressed( 0x35 ) )
					{
						while ( IsKeyPressed( 0x58 ) && IsKeyPressed( 0x35 ) )
						{
							Sleep( 10 );
						}
						DisableWindowActiveCheck = !DisableWindowActiveCheck;

						if ( !DisableWindowActiveCheck )
						{
							TextPrint( "Work only when window active : ENABLED" , 3.0f );
						}
						else if ( DisableWindowActiveCheck )
						{
							TextPrint( "Work only when window active : DISABLED" , 3.0f );
						}
					}

					if ( IsKeyPressed( 0x58 ) && IsKeyPressed( 0x36 ) )
					{
						while ( IsKeyPressed( 0x58 ) && IsKeyPressed( 0x36 ) )
						{
							Sleep( 10 );
						}
						Enable3DPoint = !Enable3DPoint;

						if ( Enable3DPoint )
						{
							TextPrint( "3D Point System : ENABLED" , 3.0f );
						}
						else if ( !Enable3DPoint )
						{
							TextPrint( "3D Point System : DISABLED" , 3.0f );
						}
					}







					if ( EnableForceStaff && Arcaddr > 0 )
					{
						if ( forcestaffcooldown > 0 )
						{
							forcestaffcooldown--;
						}
						if ( daggercooldown > 0 )
						{
							daggercooldown--;
						}

#ifdef BOTDEBUG
						PrintDebugInfo( "Init forcestaff" );
#endif
#ifdef BOTDEBUG
						PrintDebugInfo( "#1:OK" );
#endif
						if ( IsNotDolboeb( Arcaddr ) )
						{
#ifdef BOTDEBUG
							PrintDebugInfo( "dbg111" );
#endif
							//SelectMultipleUnits( unitstoselect , unitstoselect.size( ) );
						}
						else
						{
#ifdef BOTDEBUG
							PrintDebugInfo( "dbg222" );
#endif
							if ( !IsHero( Arcaddr ) && !IsUnitDead( Arcaddr ) )
							{
								Arcaddr = 0;
							}
							goto nextcheck;
						}
						/*int selectedunit1 = GetSelectedOwnedUnit( );
						if ( selectedunit1 == 0 || ( Arcaddr != selectedunit1 && !ArcAlly ) )
						{
						goto skipforcestaff;
						}*/
#ifdef BOTDEBUG
						PrintDebugInfo( "FindForcestaff1" );
#endif
						BOOL FoundForceStaff = FALSE;
						int itemid = 0;
						int itemaddr = 0;
						for ( int i = 0; i < 6; i++ )
						{
							itemaddr = GetItemInSlot( Arcaddr , 0 , i );
							if ( itemaddr > 0 && IsClassEqual( itemaddr , "I0HI" ) )
							{
								if ( SkipCooldownCheck )
								{
									if ( forcestaffcooldown > 0 )
									{

										if ( !_Forcecooldown )
										{
											TextPrint( "Forcestaff cooldown start." , 3.0f );
											_Forcecooldown = TRUE;
										}
										goto skipforcestaff;
									}
									else
									{
										if ( _Forcecooldown )
										{
											_Forcecooldown = FALSE;
											TextPrint( "Forcestaff cooldown end." , 3.0f );
											Sleep( 700 );
										}
										FoundForceStaff = TRUE;
										itemid = i;
									}
								}
								else
								{
									if ( IsItemCooldown( itemaddr )/*IsForceActiveItemCooldown( GetCMDbyItemSlot( i ) , itemaddr )*/ )
									{
										if ( !_Forcecooldown )
										{
											TextPrint( "Forcestaff cooldown start." , 3.0f );
											_Forcecooldown = TRUE;
										}
										goto skipforcestaff;
									}
									else
									{
										if ( _Forcecooldown )
										{
											_Forcecooldown = FALSE;
											TextPrint( "Forcestaff cooldown end." , 3.0f );
											Sleep( 700 );
										}
										FoundForceStaff = TRUE;
										itemid = i;
									}
								}
								break;
							}
						}
#ifdef BOTDEBUG
						PrintDebugInfo( "FindDagger1" );
#endif
						BOOL NeedDager = FALSE;
						int daggeritemid = 0;
						int daggeritemaddr = 0;

						if ( !EnableDagger )
						{

						}
						else
						{
							for ( int i = 0; i < 6; i++ )
							{
								daggeritemaddr = GetItemInSlot( Arcaddr , 0 , i );
								if ( daggeritemaddr > 0 && IsClassEqual( daggeritemaddr , "I04H" ) )
								{
									if ( SkipCooldownCheck )
									{
										if ( daggercooldown > 0 )
										{


											if ( !_Daggercooldown )
											{
												TextPrint( "Dagger cooldown start." , 3.0f );
												_Daggercooldown = TRUE;
											}

										}
										else
										{
											if ( _Daggercooldown )
											{
												_Daggercooldown = FALSE;
												TextPrint( "Dagger cooldown end." , 3.0f );
												Sleep( 700 );
											}

											NeedDager = TRUE;
											daggeritemid = i;
										}

									}
									else
									{
										if ( IsItemCooldown( daggeritemaddr )/*IsForceActiveItemCooldown( GetCMDbyItemSlot( i ) , daggeritemaddr )*/ )
										{
											if ( !_Daggercooldown )
											{
												TextPrint( "Dagger cooldown start." , 3.0f );
												_Daggercooldown = TRUE;
											}

										}
										else
										{
											if ( _Daggercooldown )
											{
												_Daggercooldown = FALSE;
												TextPrint( "Dagger cooldown end." , 3.0f );
												Sleep( 700 );
											}

											NeedDager = TRUE;
											daggeritemid = i;
										}

									}
									break;
								}
							}
						}


						if ( FoundForceStaff )
						{
#ifdef BOTDEBUG
							PrintDebugInfo( "Found force staff" );
#endif
							int * unitsarray = 0;
							int UnitsCount = GetUnitCountAndUnitArray( &unitsarray );

							if ( unitsarray && UnitsCount > 0 )
							{
#ifdef BOTDEBUG
								PrintDebugInfo( "GetUnitarray22" );
#endif
								for ( int i = 0; i < UnitsCount; i++ )
								{

#ifdef BOTDEBUG
									PrintDebugInfo2( "UnitGetdata" , unitsarray[ i ] );
#endif

									if ( IsNotDolboeb( unitsarray[ i ] ) )
									{
#ifdef BOTDEBUG
										PrintDebugInfo( "UnitOkka##!!!" );
#endif

										if ( IsHero( unitsarray[ i ] ) )
										{
#ifdef BOTDEBUG
											PrintDebugInfo( "herherher##!!!" );
#endif
											if ( GetLocalPlayerNumber( ) != GetUnitOwnerSlot( unitsarray[ i ] ) )
											{
												if ( IsPlayerEnemy( unitsarray[ i ] ) && IsUnitVisibleToPlayer( unitsarray[ i ] , GetLocalPlayer( ) ) )
												{
#ifdef BOTDEBUG
													PrintDebugInfo( "#555:OK" );
#endif
													float unitface = GetUnitFacing( unitsarray[ i ] );

													float targetunitx = 0.0f , targetunity = 0.0f , targetunitz = 0.0f;
													GetUnitLocation3D( unitsarray[ i ] , &targetunitx , &targetunity , &targetunitz );

													float Arcunitx = 0.0f , Arcunity = 0.0f , Arcunitz = 0.0f;
													GetUnitLocation3D( Arcaddr , &Arcunitx , &Arcunity , &Arcunitz );


													if ( Distance3D( Arcunitx , Arcunity , Arcunitz , targetunitx , targetunity , targetunitz ) < 780.0f || ( NeedDager && Distance3D( Arcunitx , Arcunity , Arcunitz , targetunitx , targetunity , targetunitz ) < ( 780.0f + 940.0f ) ) )
													{
														if ( NeedDager && Distance3D( Arcunitx , Arcunity , Arcunitz , targetunitx , targetunity , targetunitz ) < 780.0f )
														{
															NeedDager = FALSE;
														}

														Location startenemyloc = Location( );
														startenemyloc.X = targetunitx;
														startenemyloc.Y = targetunity;
														Location endenemyloc = GiveNextLocationFromLocAndAngle( startenemyloc , 570.0f , unitface );
														Location endenemyloc2 = GiveNextLocationFromLocAndAngle( startenemyloc , 400.0f , unitface );
														Location endenemyloc3 = GiveNextLocationFromLocAndAngle( startenemyloc , 250.0f , unitface );
														Location endenemyloc4 = GiveNextLocationFromLocAndAngle( startenemyloc , 100.0f , unitface );

														float outdmg = 0.0f;
														float enemyhp = GetUnitHP( unitsarray[ i ] );
														for ( unsigned int n = 0; n < BombList.size( ); n++ )
														{
															/*if ( IsIgnoreUnit( BombList[ n ].unitaddr ) )
																continue;*/
															if ( !BombList[ n ].remote )
															{
																if ( Distance3D( endenemyloc.X , endenemyloc.Y , BombList[ n ].z , BombList[ n ].x , BombList[ n ].y , BombList[ n ].z ) < BombList[ n ].range1 )
																{
																	outdmg += BombList[ n ].dmg;
																}
																else if ( Distance3D( endenemyloc2.X , endenemyloc2.Y , BombList[ n ].z , BombList[ n ].x , BombList[ n ].y , BombList[ n ].z ) < BombList[ n ].range1 )
																{
																	outdmg += BombList[ n ].dmg;
																}
																else if ( Distance3D( endenemyloc3.X , endenemyloc3.Y , BombList[ n ].z , BombList[ n ].x , BombList[ n ].y , BombList[ n ].z ) < BombList[ n ].range1 )
																{
																	outdmg += BombList[ n ].dmg;
																}
																else if ( Distance3D( endenemyloc4.X , endenemyloc4.Y , BombList[ n ].z , BombList[ n ].x , BombList[ n ].y , BombList[ n ].z ) < BombList[ n ].range1 )
																{
																	outdmg += BombList[ n ].dmg;
																}
															}

															/*	else if ( Distance3D( endenemyloc.X , endenemyloc.Y , BombList[ n ].z , BombList[ n ].x , BombList[ n ].y , BombList[ n ].z ) < BombList[ n ].range2 )
																{
																outdmg += ( BombList[ n ].dmg / 2.0f )* 0.80f;

																}*/
															if ( enemyhp < GetDmgForUnit( unitsarray[ i ] , outdmg ) )
																break;
														}



														if ( enemyhp < outdmg )
														{
#ifdef BOTDEBUG
															PrintDebugInfo( "FORCE IT!" );
#endif
															if ( SelectArc( ) )
															{
																int scmd = GetCMDbyItemSlot( itemid );

																if ( NeedDager )
																{
																	int dagcmd = GetCMDbyItemSlot( daggeritemid );

																	ItemOrSkillPoint( dagcmd , daggeritemaddr , targetunitx + 1.0f , targetunity - 1.01f , 0x100002 );
																	Sleep( 3 );
																	CommandItemTarget( scmd , itemaddr , targetunitx , targetunity , unitsarray[ i ] , TRUE );

																	daggercooldown = 481;
																	forcestaffcooldown = 810;

																}
																else
																{
																	CommandItemTarget( scmd , itemaddr , targetunitx , targetunity , unitsarray[ i ] );

																	forcestaffcooldown = 810;
																}
															}
															else
																goto skipforcestaff;
															Sleep( 300 );
														}

													}
												}
											}
										}
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



				}
			nextcheck:;



				Sleep( 25 );

			}
		}
		else
		{
			ArcFound = FALSE;
			Arcaddr = 0;
			Sleep( 3000 );
			while ( !IsGame( ) )
			{
				Sleep( 2000 );
			}

		}

	}

	return 0;
}

HANDLE ArcThrID;

BOOL WINAPI DllMain( HINSTANCE hDLL , UINT reason , LPVOID reserved )
{

	if ( reason == DLL_PROCESS_ATTACH )
	{
#ifdef BOTDEBUG
		if ( !DEBUGSTARTED )
		{
			DEBUGSTARTED = TRUE;
			UpdateDebug( );
		}
		PrintDebugInfo( "DLLMAIN" );
#endif
		DisableThreadLibraryCalls( hDLL );
		GameDll = ( int ) GetModuleHandle( "Game.dll" );
		if ( !GameDll )
		{
			MessageBox( 0 , "Error no game.dll found" , "Error" , MB_OK );
			return 0;
		}
	
		// Получаем координаты юнита

		GetItemInSlot = ( sub_6F26EC20 ) ( GameDll + 0x26EC20 );
		GetUnitAbilLevelReal = ( sub_6F0787D0 ) ( GameDll + 0x787D0 );

		ClearSelection = ( mClearSelection ) ( GameDll + 0x3BBAA0 );
	
		PingMinimap = ( mPingMinimap ) ( GameDll + 0x3B4650 );
		_W3XGlobalClass = GameDll + 0xAB4F80;
#ifdef BOTDEBUG
		PrintDebugInfo( "StartThread" );
#endif
		ArcThrID = CreateThread( 0 , 0 , ArcThread , 0 , 0 , 0 );
	}
	else if ( reason == DLL_PROCESS_DETACH )
	{
		// Убить поток :)
		TerminateThread( ArcThrID , 0 );
	}
	return TRUE;
}
