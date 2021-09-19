#include <Windows.h>
#include <vector>

std::vector<char *> tempstringlist;
std::vector<char *> tempstringlistforfree;

typedef void( __stdcall * Callback )( char* text );
Callback Handler = 0;

int __stdcall callbackprint( char* text )
{
	tempstringlist.push_back( text );
	return 0;
}

unsigned long __stdcall ThreadForWriteText2( void * )
{
	if ( tempstringlist.size( ) > 0 )
	{
		int current = 0;
		while ( tempstringlist.size( ) )
		{
			char * text = tempstringlist[ 0 ];
			current++;
			if ( tempstringlist.size( ) - current < 25 || strstr( text , "ErrorErrorError") )
			{
				Handler( text );
			}
			tempstringlist.erase( tempstringlist.begin( ) + 0 );
			tempstringlistforfree.push_back( text );
		}

		Sleep( 400 );
		
		while ( tempstringlistforfree.size( ) )
		{
			VirtualFree( tempstringlistforfree[ 0 ] , 0 , MEM_RELEASE );
			tempstringlistforfree.erase( tempstringlistforfree.begin( ) + 0 );
		}
		

		/*for ( unsigned int i = 0; i < tempstringlist.size( ); i++ )
		{
			
		}*/
		tempstringlist.clear( );
	}
	return 0;
}

unsigned long __stdcall ThreadForWriteText1( void * )
{

	Sleep( 3000 );

	while ( true )
	{
		Sleep(1000 );
		if ( tempstringlist.size( ) > 0 )
		{
			WaitForSingleObject( CreateThread( 0 , 0 , ThreadForWriteText2 , 0 , 0 , 0 ) , INFINITE );
		}
	}

	return 0;
}

extern "C" __declspec( dllexport )
int __stdcall SetCallback( Callback handler )
{

	Handler = handler;
	CreateThread( 0 , 0 , ThreadForWriteText1 , 0 , 0 , 0 );
	return ( int ) &callbackprint;
}