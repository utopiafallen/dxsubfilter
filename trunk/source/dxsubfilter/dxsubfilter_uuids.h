// Custom GUIDs defined for DXSubFilter usage and accompanying DirectShow names.

#pragma once

#include "stdafx.h"

// Custom macro to actually define a GUID instead of simply declaring it. For some reason,
// defining INITGUID so that DEFINE_GUID will actually produce a definition rather than a
// declaration causes compile errors with uuids.h from DirectShow BaseClasses.
#define DXSUBFILTER_DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        EXTERN_C const GUID DECLSPEC_SELECTANY name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }


// Custom media types defined by MKV? At least Haali MKV splitter will return these

// {3B6ED1B8-ECF6-422A-8F07-48980E6482CE}		DXSubFilter
extern const wchar_t* DXSUBFILTER_NAME;
DXSUBFILTER_DEFINE_GUID(CLSID_DXSubFilter, 
0x3b6ed1b8, 0xecf6, 0x422a, 0x8f, 0x7, 0x48, 0x98, 0xe, 0x64, 0x82, 0xce);

