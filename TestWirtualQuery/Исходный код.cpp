#include <Windows.h>
#include <string>

#include <iostream>
BOOL IsOkayPtr( void *ptr )
{
	MEMORY_BASIC_INFORMATION mbi;
	if ( VirtualQuery( ptr , &mbi , sizeof( MEMORY_BASIC_INFORMATION ) ) == 0 )
		return FALSE;

	char printdata[ 500 ];
	sprintf_s( printdata , 500 , "%X----%X----%u" , mbi.AllocationBase , mbi.BaseAddress , mbi.RegionSize );
	std::cout << printdata << std::endl;

	return TRUE;
}


int main( )
{
	/*byte * huite = new byte[ 256 ];

	char pppp[ 100 ];

	sprintf_s( pppp , 100 , "%X" , (int)&huite[ 0 ] );


	std::cout << pppp;
	std::cout << std::endl;
	


	int addr = ( int ) &huite[ 100 ];

	IsOkayPtr( ( void* ) addr );

	addr = ( int ) &huite[ 5000 ];

	IsOkayPtr( ( void* ) addr );
	system( "pause" );*/

	
	

	return 0;
}