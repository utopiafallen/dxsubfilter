// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "dxsubfilter_uuids.h"
#include "dxsubfilter.h"

//------------------------------------------------------------------------------
// Global filter information for proper DirectShow registration as well as registry
// location.

AMOVIESETUP_MEDIATYPE sudVideoMediaTypes[] = {
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_AYUV },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_YUY2 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_YV12 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_NV12 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_P210 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_P216 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_P010 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_P016 },
};

AMOVIESETUP_MEDIATYPE sudSubtitleMediaTypes[] = {
	{ &MEDIATYPE_Text, &MEDIASUBTYPE_None },
};

AMOVIESETUP_PIN sudOutputPin = {
    L"",								// Obsolete, not used.
    FALSE,								// Is this pin rendered?
    TRUE,								// Is it an output pin?
    FALSE,								// Can the filter create zero instances?
    FALSE,								// Does the filter create multiple instances?
    &GUID_NULL,							// Obsolete.
    NULL,								// Obsolete.
    ARRAYSIZE(sudVideoMediaTypes),      // Number of media types.
    sudVideoMediaTypes					// Pointer to media types.
};

AMOVIESETUP_PIN sudInputVideoPin = {
	L"",								// Obsolete, not used.
    FALSE,								// Is this pin rendered?
    FALSE,								// Is it an output pin?
    FALSE,								// Can the filter create zero instances?
    FALSE,								// Does the filter create multiple instances?
    &GUID_NULL,							// Obsolete.
    NULL,								// Obsolete.
    ARRAYSIZE(sudVideoMediaTypes),      // Number of media types.
    sudVideoMediaTypes					// Pointer to media types.
};

AMOVIESETUP_PIN sudInputSubtitlePin = {
	L"",								// Obsolete, not used.
    FALSE,								// Is this pin rendered?
    FALSE,								// Is it an output pin?
    FALSE,								// Can the filter create zero instances?
    FALSE,								// Does the filter create multiple instances?
    &GUID_NULL,							// Obsolete.
    NULL,								// Obsolete.
    ARRAYSIZE(sudSubtitleMediaTypes),   // Number of media types.
    sudSubtitleMediaTypes				// Pointer to media types.
};

AMOVIESETUP_PIN sudDXSubFilterPins[] = {
	sudInputSubtitlePin,
	sudInputVideoPin,
	sudOutputPin,
};

AMOVIESETUP_FILTER sudDXSubFilterReg = {
    &CLSID_DXSubFilter,				// Filter CLSID.
	L"DXSubFilter",					// Filter name.
    MERIT_NORMAL,					// Merit.
	ARRAYSIZE(sudDXSubFilterPins),	// Number of pin types.
    sudDXSubFilterPins				// Pointer to pin information.
};

static const wchar_t* DXSUBFILTER_SUBKEY_NAME = L"Software\\dxsubfilter";

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/* List of class IDs and creator functions for the class factory. This
   provides the link between the OLE entry point in the DLL and an object
   being created. The class factory will call the static CreateInstance
   function when it is asked to create a CLSID_DXSubFilter object */

CFactoryTemplate g_Templates[1] = {
    {
		L"CDXSubFilter",							// Name
		&CLSID_DXSubFilter,							// CLSID
		DXSubFilter::CDXSubFilter::CreateInstance,	// Creation function
		NULL,										// Initialization function (optional)
		&sudDXSubFilterReg							// Pointer to filter information
	}
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Registry functions. Configuration data is stored in HCKU/Software/dxsubfilter

// Returns true if filter configuration data was loaded successfully from registry.
// Returns false if filter configuration data was not found in registry.
bool LoadFilterConfigurationFromRegistry()
{
	HKEY hDXSubFilterKey;

	LONG lRegOpenResult = RegOpenKeyEx(HKEY_CURRENT_USER, DXSUBFILTER_SUBKEY_NAME, 0, 
										KEY_READ, &hDXSubFilterKey);

	if (lRegOpenResult == ERROR_SUCCESS)
	{
		// Load data. Will probably store this in a struct whose pointer is passed into the 
		// initialization function of CDXSubFilter
		UNREFERENCED_PARAMETER(hDXSubFilterKey);
		return true;
	}
	else
	{
		return false;
	}
}

// Write default filter configuration data to registry.
void WriteFilterConfigurationDataToRegistry()
{
	HKEY hDXSubFilterKey;

	LONG lRegCreateResult = RegCreateKeyEx(HKEY_CURRENT_USER, DXSUBFILTER_SUBKEY_NAME, 0,
		NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE | KEY_READ, NULL, &hDXSubFilterKey, NULL);

	if (lRegCreateResult == ERROR_SUCCESS)
	{
		// Write data.
		UNREFERENCED_PARAMETER(hDXSubFilterKey);
	}
}

// Remove filter configuration data from registry.
void RemoveFilterConfigurationDataFromRegistry()
{
	RegDeleteKeyEx(HKEY_CURRENT_USER, DXSUBFILTER_SUBKEY_NAME, KEY_WOW64_32KEY, 0);
}
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

// DLL Entry Point for DirectShow filters.
//extern "C" void __cdecl __security_init_cookie(void);
//extern "C" BOOL WINAPI _DllEntryPoint(HINSTANCE, ULONG, __inout_opt LPVOID);
//
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
//
//	// Check registry for configuration data. If none is found, store default settings to registry.
//	if (LoadFilterConfigurationFromRegistry() == false)
//	{
//		WriteFilterConfigurationDataToRegistry();
//	}
//
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
	RemoveFilterConfigurationDataFromRegistry();
	return AMovieDllRegisterServer2(FALSE);
}

