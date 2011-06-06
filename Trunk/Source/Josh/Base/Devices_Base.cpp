// Devices_Base.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "Devices_Base.h"

// Export something so that a lib is created
void DEVICES_BASE_API DoNothing( void ) {};

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}

