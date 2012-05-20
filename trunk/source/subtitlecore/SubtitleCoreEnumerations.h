// Header to contain all global enumerations and constant values that a user of SubtitleCore
// needs to know.
#pragma once

#include "stdafx.h"

namespace SubtitleCore
{
	enum SubtitleType { SBT_ASS, SBT_SSA, SBT_SRT, SBT_VOBSUB, SBT_NONE };

	static const std::wstring SubtitleFileExtensions[] = { 
		L".ass", 
		L".ssa", 
		L".srt", 
		L".idx", 
		L".sub",
	};
	static const size_t SubtitleFileExtensionsCount = ARRAYSIZE(SubtitleFileExtensions);
};