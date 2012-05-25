// Internal representation of an SRT subtitle line.
#ifndef SRTSUBTITLEENTRY_H
#define SRTSUBTITLEENTRY_H
#pragma once

#include "stdafx.h"

namespace SubtitleCore
{
	// No custom font style. SRT always uses global SubtitleCore configuration
	struct SRTSubtitleEntry
	{
		struct TextRangeFormat
		{
			DWRITE_TEXT_RANGE Range;
			DWRITE_FONT_WEIGHT Weight;
			DWRITE_FONT_STYLE Style;
			BOOL Underline;
			BOOL Strikethrough;
		};

		// Contains spans where text formatting differs from main formatting, e.g.
		// "Hello <b>world</b>" "world" would be stored in SubTextFormat.
		std::vector<TextRangeFormat> SubTextFormat;

		std::wstring Text;

		REFERENCE_TIME EndTime;

		SRTSubtitleEntry() : EndTime(0) {}
	};
}

#endif