// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "registryhelpers.h"
#include "dxsubfilter_uuids.h"
#include "dxsubfilter.h"
#include "SubtitleRendererFactory.h"

//------------------------------------------------------------------------------
// Global filter information for proper DirectShow registration

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
	{ &MEDIATYPE_Subtitle, &MEDIASUBTYPE_UTF8 },
	{ &MEDIATYPE_Subtitle, &MEDIASUBTYPE_SSA },
	{ &MEDIATYPE_Subtitle, &MEDIASUBTYPE_ASS },
	{ &MEDIATYPE_Subtitle, &MEDIASUBTYPE_VOBSUB },
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
	sudInputVideoPin,
	sudInputSubtitlePin,
	sudOutputPin,
};

AMOVIESETUP_FILTER sudDXSubFilterReg = {
	&CLSID_DXSubFilter,				// Filter CLSID.
	L"DXSubFilter",					// Filter name.
	MERIT_NORMAL,					// Merit.
	ARRAYSIZE(sudDXSubFilterPins),	// Number of pin types.
	sudDXSubFilterPins				// Pointer to pin information.
};
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
// The following functions are actually defined in BaseClasses, but we need to give their
// function signature here so we can call them correctly.
extern "C" void __cdecl __security_init_cookie(void);
extern "C" BOOL WINAPI _DllEntryPoint(HINSTANCE, ULONG, __inout_opt LPVOID);

// Used to determine what the purpose was of loading the DLL so we can correctly perform the
// desired registry operations.
enum REASON_FOR_ENTRY { REGISTERING, UNREGISTERING, NORMAL };
REASON_FOR_ENTRY g_ReasonForEntry = NORMAL;

// DLL Entry Point for DirectShow filters.
DECLSPEC_NOINLINE
BOOL 
WINAPI
DllEntryPoint(
	HINSTANCE hInstance, 
	ULONG ulReason, 
	__inout_opt LPVOID pv
	)
{
	if ( ulReason == DLL_PROCESS_ATTACH ) {
		// Must happen before any other code is executed.  Thankfully - it's re-entrant
		__security_init_cookie();
	}

	return _DllEntryPoint(hInstance, ulReason, pv);
}

//	DLL Entry Point	for normal DLLs.
BOOL APIENTRY DllMain( HMODULE hModule,
					   DWORD  ul_reason_for_call,
					   LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		// Initialize SubtitleRendererFactory. Must only be done once.
		SubtitleCore::SubtitleRendererFactory::InitializeSingleton();
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}

	// We perform registry operations here because we use HKCU to store our configuration data,
	// which means multiple user accounts can have different configuration settings for the filter.
	// Doing it in DllRegister/Unregister would only take effect when the filter was initially 
	// registered.
	if (g_ReasonForEntry == UNREGISTERING)
	{
		DXSubFilter::RemoveFilterConfigurationDataFromRegistry();
	}
	else
	{
		if (DXSubFilter::LoadFilterConfigurationFromRegistry() == false)
		{
			DXSubFilter::WriteFilterConfigurationDataToRegistry();
			DXSubFilter::LoadFilterConfigurationFromRegistry();
		}
	}

	// Microsoft DirectShow fail: DllMain is actually called by RegSvr32, but we need to run 
	// DllEntryPoint to run the proper DirectShow specific stuff. Consequently, we need to 
	// manually call DllEntryPoint ourselves. This solves the problem of our filter registering
	// incorrectly as RegSvr32.exe instead of itself.
	return DllEntryPoint(reinterpret_cast<HINSTANCE>(hModule), ul_reason_for_call, lpReserved);
}

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// To be self registering, OLE servers must export functions named 
// DllRegisterServer and DllUnregisterServer. These are the functions that are
// called when RegSrv32.exe is run.

STDAPI DllRegisterServer()
{
	g_ReasonForEntry = REGISTERING;
	return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
	g_ReasonForEntry = UNREGISTERING;
	return AMovieDllRegisterServer2(FALSE);;
}

