// Convenience functions for DirectWrite/Direct2D interfacing. Functions left in the global
// namespace indicate that they aren't intrinsically tied to implementation details in 
// SubtitleCore
#ifndef SCDXHELEPERS_H
#define SCDXHELEPERS_H
#pragma once
#include <D2DBaseTypes.h>
#include "SubtitleCoreEnumerations.h"

template <class Interface>
inline void SafeRelease(Interface** pInterfaceToRelease)
{
	if (*pInterfaceToRelease)
	{
		(*pInterfaceToRelease)->Release();
		(*pInterfaceToRelease) = nullptr;
	}
}

// Note that the color is expected to be in ABGR with R in the LSB based on the assumption that
// color is given in hexadecimal.
inline D2D_COLOR_F ConvertABGRToD2DCOLORF(unsigned int ABGR)
{
	static const float normalize = 1.0f/255.0f;
	D2D_COLOR_F result;
	result.r = static_cast<float>(ABGR & 0x000000ff) * normalize;
	result.g = static_cast<float>((ABGR & 0x0000ff00) >> 8) * normalize;
	result.b = static_cast<float>((ABGR & 0x00ff0000) >> 16) * normalize;
	result.a = static_cast<float>((ABGR & 0xff000000) >> 24) * normalize;

	return result;
}

// SCU - SubtitleCoreUtitlies
namespace SCU
{
	using namespace SubtitleCore;

	inline size_t ConvertDIPToPixels(float dip, float fDPIScale)
	{
		// Round up
		return static_cast<size_t>(dip * fDPIScale + 0.5f);
	}

	inline float ConvertPixelsToDIP(size_t pixels, float fDPIScale)
	{
		return static_cast<float>(pixels) / fDPIScale;
	}

	inline float ConvertFontPointToDIP(size_t pt)
	{
		// Note to self: See the following link for explanation of magic numbers:
		// http://msdn.microsoft.com/en-us/library/ff684173(v=vs.85).aspx

		static const float conversion_factor = 96.0f/72.0f;
		return static_cast<float>(pt) * conversion_factor;
	}

	inline void ConvertDLAToDWRiteEnums(SubtitleCoreConfigurationData::DefaultLineAlignment DLA, 
								DWRITE_TEXT_ALIGNMENT& outTextAlign,
								DWRITE_PARAGRAPH_ALIGNMENT& outParaAlign)
	{
		switch(DLA)
		{
		case SubtitleCoreConfigurationData::LA_TOPLEFT:
			{
				outTextAlign = DWRITE_TEXT_ALIGNMENT_LEADING;
				outParaAlign = DWRITE_PARAGRAPH_ALIGNMENT_NEAR;
				break;
			}
		case SubtitleCoreConfigurationData::LA_TOPMIDDLE:
			{
				outTextAlign = DWRITE_TEXT_ALIGNMENT_CENTER;
				outParaAlign = DWRITE_PARAGRAPH_ALIGNMENT_NEAR;
				break;
			}
		case SubtitleCoreConfigurationData::LA_TOPRIGHT:
			{
				outTextAlign = DWRITE_TEXT_ALIGNMENT_TRAILING;
				outParaAlign = DWRITE_PARAGRAPH_ALIGNMENT_NEAR;
				break;
			}
		case SubtitleCoreConfigurationData::LA_CENTERLEFT:
			{
				outTextAlign = DWRITE_TEXT_ALIGNMENT_LEADING;
				outParaAlign = DWRITE_PARAGRAPH_ALIGNMENT_CENTER;
				break;
			}
		case SubtitleCoreConfigurationData::LA_CENTERMIDDLE:
			{
				outTextAlign = DWRITE_TEXT_ALIGNMENT_CENTER;
				outParaAlign = DWRITE_PARAGRAPH_ALIGNMENT_CENTER;
				break;
			}
		case SubtitleCoreConfigurationData::LA_CENTERRIGHT:
			{
				outTextAlign = DWRITE_TEXT_ALIGNMENT_TRAILING;
				outParaAlign = DWRITE_PARAGRAPH_ALIGNMENT_CENTER;
				break;
			}
		case SubtitleCoreConfigurationData::LA_BOTTOMLEFT:
			{
				outTextAlign = DWRITE_TEXT_ALIGNMENT_LEADING;
				outParaAlign = DWRITE_PARAGRAPH_ALIGNMENT_FAR;
				break;
			}
		case SubtitleCoreConfigurationData::LA_BOTTOMMIDDLE:
			{
				outTextAlign = DWRITE_TEXT_ALIGNMENT_CENTER;
				outParaAlign = DWRITE_PARAGRAPH_ALIGNMENT_FAR;
				break;
			}
		case SubtitleCoreConfigurationData::LA_BOTTOMRIGHT:
			{
				outTextAlign = DWRITE_TEXT_ALIGNMENT_TRAILING;
				outParaAlign = DWRITE_PARAGRAPH_ALIGNMENT_FAR;
				break;
			}
		default:
			{
				outTextAlign = DWRITE_TEXT_ALIGNMENT_CENTER;
				outParaAlign = DWRITE_PARAGRAPH_ALIGNMENT_FAR;
				break;
			}
		}
	}
};

#endif