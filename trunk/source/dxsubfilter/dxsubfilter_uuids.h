// Custom GUIDs defined for DXSubFilter usage.

#pragma once

#include "stdafx.h"

// Custom macro to actually define a GUID instead of simply declaring it. For some reason,
// defining INITGUID so that DEFINE_GUID will actually produce a definition rather than a
// declaration causes compile errors with uuids.h from DirectShow BaseClasses.
#define DXSUBFILTER_DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
		EXTERN_C const GUID DECLSPEC_SELECTANY name \
				= { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }


// Custom media types defined by MKV for subtitles? At least Haali MKV splitter will set these
// {E487EB08-6B26-4BE9-9DD3-993434D313FD}		MEDIATYPE_Subtitle
DXSUBFILTER_DEFINE_GUID(MEDIATYPE_Subtitle,
	0xe487eb08, 0x6b26, 0x4be9, 0x9d, 0xd3, 0x99, 0x34, 0x34, 0xd3, 0x13, 0xfd);

// {87C0B230-03A8-4fdf-8010-B27A5848200D}		MEDIASUBTYPE_UTF8
DXSUBFILTER_DEFINE_GUID(MEDIASUBTYPE_UTF8,
	0x87c0b230, 0x03a8, 0x4fdf, 0x80, 0x10, 0xb2, 0x7a, 0x58, 0x48, 0x20, 0x0d);

// {3020560F-255A-4ddc-806E-6C5CC6DCD70A}		MEDIASUBTYPE_SSA
DXSUBFILTER_DEFINE_GUID(MEDIASUBTYPE_SSA,
	0x3020560f, 0x255a, 0x4ddc, 0x80, 0x6e, 0x6c, 0x5c, 0xc6, 0xdc, 0xd7, 0x0a);

// {326444F7-686F-47ff-A4B2-C8C96307B4C2}		MEDIASUBTYPE_ASS
DXSUBFILTER_DEFINE_GUID(MEDIASUBTYPE_ASS,
	0x326444f7, 0x686f, 0x47ff, 0xa4, 0xb2, 0xc8, 0xc9, 0x63, 0x07, 0xb4, 0xc2);

// {F7239E31-9599-4e43-8DD5-FBAF75CF37F1}		MEDIASUBTYPE_VOBSUB
DXSUBFILTER_DEFINE_GUID(MEDIASUBTYPE_VOBSUB,
	0xf7239e31, 0x9599, 0x4e43, 0x8d, 0xd5, 0xfb, 0xaf, 0x75, 0xcf, 0x37, 0xf1);

// {A33D2F7D-96BC-4337-B23B-A8B9FBC295E9}		FORMATTYPE_SubtitleInfo
DXSUBFILTER_DEFINE_GUID(FORMATTYPE_SubtitleInfo,
	0xa33d2f7d, 0x96bc, 0x4337, 0xb2, 0x3b, 0xa8, 0xb9, 0xfb, 0xc2, 0x95, 0xe9);

struct SubtitleInfo
{
	DWORD dwOffset;			// size (in bytes) of the structure/pointer to codec init data
	CHAR IsoLang[4];		// three letter lang code + terminating zero
	WCHAR TrackName[256];	// name of subtitle track
	
	// CodecPrivate data is appended to end of subtitle info
};


// {3B6ED1B8-ECF6-422A-8F07-48980E6482CE}		DXSubFilter
extern const wchar_t* DXSUBFILTER_NAME;
DXSUBFILTER_DEFINE_GUID(CLSID_DXSubFilter, 
	0x3b6ed1b8, 0xecf6, 0x422a, 0x8f, 0x7, 0x48, 0x98, 0xe, 0x64, 0x82, 0xce);

