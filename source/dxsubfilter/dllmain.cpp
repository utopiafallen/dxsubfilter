// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "dxsubfilter_uuids.h"
#include "dxsubfilter.h"

//------------------------------------------------------------------------------
/* List of class IDs and creator functions for the class factory. This
   provides the link between the OLE entry point in the DLL and an object
   being created. The class factory will call the static CreateInstance
   function when it is asked to create a CLSID_SystemClock object */

CFactoryTemplate g_Templates[1] = {
    {L"CDXSubFilter", &CLSID_DXSubFilter, CDXSubFilter::CreateInstance}
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
/*		DLL Entry Point			*/
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// To be self registering, OLE servers must export functions named 
// DllRegisterServer and DllUnregisterServer. These are the functions that are
// called when RegSrv32.exe is run.

STDAPI DllRegisterServer()
{
	return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllRegisterServer2(BOOL b)
{
	return AMovieDllRegisterServer2(b);
}

STDAPI DllUnregisterServer()
{
	return AMovieDllRegisterServer2(FALSE);
}

