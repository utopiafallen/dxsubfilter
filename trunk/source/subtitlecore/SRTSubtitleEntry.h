// Internal representation of an SRT subtitle line.
#ifndef SRTSUBTITLEENTRY_H
#define SRTSUBTITLEENTRY_H
#pragma once

#include "stdafx.h"
#include "DirectXHelpers.h"

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

			// Font face for this particular formatting
			IDWriteFontFace* pFontFace;

			TextRangeFormat() : pFontFace(nullptr) {}
			~TextRangeFormat() { SafeRelease(&pFontFace); }
		};

		// Contains text formatting spans 
		std::vector<TextRangeFormat> SubTextFormat;

		std::wstring Text;

		REFERENCE_TIME StartTime;
		REFERENCE_TIME EndTime;

		SRTSubtitleEntry() : StartTime(0), EndTime(0) {}
	};
}

#endif