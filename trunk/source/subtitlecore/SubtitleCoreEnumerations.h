// Header to contain all global enumerations and constant values that a user of SubtitleCore
// needs to know.
#ifndef SUBTITLECORE_ENUMERATIONS_H
#define SUBTITLECORE_ENUMERATIONS_H
#pragma once

#include <string>

namespace SubtitleCore
{
	enum SubtitleType { SBT_ASS, SBT_SSA, SBT_SRT, SBT_VOBSUB, SBT_NONE };
	enum SubtitlePictureFormat { SBPF_RGBA32 };

	static const std::wstring SubtitleFileExtensions[] = { 
		L".ass", 
		L".ssa", 
		L".srt", 
		L".idx", // Assume user will properly load matching .sub for us
	};
	static const size_t SubtitleFileExtensionsCount = ARRAYSIZE(SubtitleFileExtensions);

	// This struct stores all the SubtitleCore configuration data that was read in from the
	// registry.
	struct SubtitleCoreConfigurationData
	{
		// Default alignment of text on the screen
		enum DefaultLineAlignment { LA_TOPLEFT, LA_TOPMIDDLE, LA_TOPRIGHT,
									LA_CENTERLEFT, LA_CENTERMIDDLE, LA_CENTERRIGHT,
									LA_BOTTOMLEFT, LA_BOTTOMMIDDLE, LA_BOTTOMRIGHT,
		};

		// Margin configuration, given in pixels
		DefaultLineAlignment m_LineAlignment;
		unsigned int m_LineMarginLeft;
		unsigned int m_LineMarginRight;
		unsigned int m_LineMarginTop;
		unsigned int m_LineMarginBottom;

		// Text formatting. Color code is stored and interpreted as ABGR (so R is LSB since
		// we're little-endian).
		unsigned int m_FontPrimaryFillColor;
		unsigned int m_FontSecondaryFillColor;
		unsigned int m_FontOutlineColor;
		unsigned int m_FontShadowColor;
		unsigned int m_FontBorderWidth;
		unsigned int m_FontShadowDepth;
		unsigned int m_FontSize;
		std::wstring m_FontName;

		//Font code page, script and style support TBD.

		// Number of subtitles to buffer for rendering ahead of video playback
		unsigned int m_SubtitleBufferSize;

		// Default constructor will initialize this struct to default values
		SubtitleCoreConfigurationData() : m_LineAlignment(LA_BOTTOMMIDDLE), m_LineMarginLeft(20),
			m_LineMarginRight(20), m_LineMarginTop(20), m_LineMarginBottom(20),
			m_FontPrimaryFillColor(0xFFFFFFFF), m_FontSecondaryFillColor(0xFFFFFFFF),
			m_FontOutlineColor(0xFF000000), m_FontShadowColor(0xFF000000),
			m_FontBorderWidth(2), m_FontShadowDepth(2),
			m_FontSize(18),
			m_FontName(L"Tahoma"),
			m_SubtitleBufferSize(0)
		{
		}
	};

	// This struct stores relevant video info needed to properly render subtitles.
	struct VideoInfo
	{
		// Given in pixels
		size_t Width;
		size_t Height;

		VideoInfo() : Width(0), Height(0) {}
	};
};

#endif