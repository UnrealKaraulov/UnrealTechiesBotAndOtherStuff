#include <Windows.h>
#include <string>


void MessageBoxCurrentPEBaddr( )
{
	char message[ 128 ];
	int PEBADDR = 0;

	__asm
	{
		mov eax, fs:[ 0x30 ];
		mov PEBADDR, eax;
	}
	sprintf_s( message, 128, "%X", PEBADDR );
	MessageBox( 0, message, message, 0 );
	
}



BOOL WINAPI DllMain( HINSTANCE hDLL, UINT reason, LPVOID reserved )
{
	if ( reason == DLL_PROCESS_ATTACH )
	{
		MessageBoxCurrentPEBaddr( );
	}
	return TRUE;
}
