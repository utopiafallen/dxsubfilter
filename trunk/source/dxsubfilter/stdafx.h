// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#ifndef STDAFX_H
#define STDAFX_H
#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers.
// Windows Header Files:
#include <windows.h>
#include <assert.h>

// Microsoft Concurrency Runtime
#include <ppl.h>

// SSE intrinsics
#include <emmintrin.h>
#include <xmmintrin.h>

// STL
#include <string>
#include <memory>
#include <fstream>
#include <unordered_map>

// DirectShow
#include <DShow.h>
#include <dvdmedia.h>
#include "streams.h"

#endif


