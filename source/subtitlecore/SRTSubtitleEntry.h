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
			TextRangeFormat(const TextRangeFormat& other) 
				: pFontFace(other.pFontFace)
				, Range(other.Range)
				, Style(other.Style)
				, Weight(other.Weight)
				, Underline(other.Underline)
				, Strikethrough(other.Strikethrough)
			{
				pFontFace = other.pFontFace;

				if (pFontFace)
				{
					pFontFace->AddRef();
				}
			}
			~TextRangeFormat() { SafeRelease(&pFontFace); }

			TextRangeFormat& operator= (TextRangeFormat& other)
			{
				Range = other.Range;
				Style = other.Style;
				Weight = other.Weight;
				Underline = other.Underline;
				Strikethrough = other.Strikethrough;

				if (pFontFace)
				{
					pFontFace->Release();
				}

				pFontFace = other.pFontFace;

				if (pFontFace)
				{
					pFontFace->AddRef();
				}

				return *this;
			}
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