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
//	DLL Entry Point	for normal DLLs.
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

// DLL Entry Point for DirectShow filters. Note that BaseClasses already provides an
// implementation for us that we can probably just reuse, but I'm leaving this commented
// definition here in case we need to provide a custom implementation.
//extern "C"
//DECLSPEC_NOINLINE
//BOOL 
//WINAPI
//DllEntryPoint(
//    HINSTANCE hInstance, 
//    ULONG ulReason, 
//    __inout_opt LPVOID pv
//    )
//{
//    if ( ulReason == DLL_PROCESS_ATTACH ) {
//        // Must happen before any other code is executed.  Thankfully - it's re-entrant
//        __security_init_cookie();
//    }
//    return _DllEntryPoint(hInstance, ulReason, pv);
//}

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

