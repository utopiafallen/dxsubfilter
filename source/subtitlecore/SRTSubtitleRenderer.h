// SRT Subtitle renderer
#ifndef SRTSUBTITLERENDERER_H
#define SRTSUBTITLERENDERER_H
#pragma once

#include "stdafx.h"
#include "ISubtitleRenderer.h"
#include "SubtitleCoreEnumerations.h"
#include "SubtitlePicture.h"
#include "SRTSubtitleEntry.h"

namespace SubtitleCore
{
	// SRTSubtitleRenderer
	//	Class to handle rendering SRT subtitles. Because no official format specification exists
	//	for SRT and different players support different features added into SRT (most notably by
	//	VSFilter's hackish way of treating SRT subtitles as a special case of ASS), we will support
	//	a very strict subset of the potential features of the format. Specifically, we support:
	//		<b></b>			HTML-Bold
	//		<u></u>			HTML-Underline
	//		<i></i>			HTML-Italic
	//		<s></s>			HTML-Strikethrough
	//	and that is it. All other tags will simply be displayed as if it were normal text. SRT
	//	really should just be a plain-text format; styling and positioning should be done through
	//	ASS. 
	class SRTSubtitleRenderer : public ISubtitleRenderer
	{
	public:
		// All constructor parameters are expected to be valid.
		SRTSubtitleRenderer(SubtitleCoreConfigurationData& config, VideoInfo& vidInfo, IDWriteFactory* dwFactory);
		~SRTSubtitleRenderer();

		//==================================================================
		// ISubtitleRenderer implementation. See ISubtitleRenderer for info
		//==================================================================
		virtual bool ParseScript(const std::vector<std::wstring>& script);
		virtual bool ParseLine(const std::wstring& line, REFERENCE_TIME rtStart, REFERENCE_TIME rtEnd);

		// This function does nothing as SRT subtitles do not have a timecode embedded in each
		// line, so the timestamps must be passed in.
		virtual bool ParseLine(const std::wstring& line);

		// The following functions do nothing in this implementation as SRT does not have a 
		// script header
		virtual bool ParseFormatHeader(const std::wstring& header);
		virtual bool ParseFormatHeader(const unsigned char* header);

		// SRT is text based so no binary processing needed
		virtual bool ParseData(const unsigned char* data);
		virtual bool ParseData(const unsigned char* data, ptrdiff_t offset);
		virtual bool ParseData(const unsigned char* data, ptrdiff_t startOffset, ptrdiff_t endOffset);

		virtual void Invalidate();

		virtual size_t GetSubtitlePictureCount(REFERENCE_TIME rtNow);
		virtual void GetSubtitlePicture(REFERENCE_TIME rtNow, SubtitlePicture** ppOutSubPics);

	private: // Data
		typedef std::set<std::pair<REFERENCE_TIME, REFERENCE_TIME>> tTimeSpanSet;

		SubtitleCoreConfigurationData m_SubCoreConfig;
		VideoInfo m_VideoInfo;

		// DirectWrite and Direct2D members
		IDWriteFactory* m_pDWriteFactory;
		IWICImagingFactory* m_pWICFactory;
		ID2D1Factory* m_pD2DFactory;

		IDWriteTextFormat* m_pDWTextFormat;

		ID2D1RenderTarget* m_pRT;
		IWICBitmap* m_pWICBitmap;

		ID2D1SolidColorBrush* m_pSolidColorBrush;
		ID2D1SolidColorBrush* m_pOutlineColorBrush;
		ID2D1SolidColorBrush* m_pShadowColorBrush;

		float m_fDPIScaleX;
		float m_fDPIScaleY;

		// What direction to place overlapping subtitles. -1 places it above an existing subtitle
		// while 1 places it below.
		float m_fSubtitlePlacementDirection;

		// Sum of left + right and top + bottom margins
		float m_fHorizontalMargin;
		float m_fVerticalMargin;

		// Parsed subtitle info, keyed on subtitle start time.
		std::unordered_map<REFERENCE_TIME, std::vector<SRTSubtitleEntry>> m_SubtitleMap;

		tTimeSpanSet m_SubtitleTimeSpans;

		// Valid time spans that encompass the most recently requested playback time
		tTimeSpanSet m_ValidSubtitleTimes;

		// Rendered subtitles. Used to cache subtitles to prevent redrawing them unnecessarily.
		struct RenderedSubtitles
		{
			SubtitlePicture SubPic;
			REFERENCE_TIME EndTime;
			REFERENCE_TIME StartTime;

			bool operator==(const RenderedSubtitles& other)
			{
				return SubPic == other.SubPic &&
					StartTime == other.StartTime &&
					EndTime == other.EndTime;
			}
		};
		std::list<RenderedSubtitles> m_RenderedSubtitles;

	private: // Functions

		// Returns whether or not a line is a timestamp
		bool CheckLineIsTimestamp(const std::wstring& line);

		// Computes timestamps and stores them into rtStart and rtEnd
		void ComputeTimestamp(const std::wstring& line, REFERENCE_TIME& rtStart, REFERENCE_TIME& rtEnd);

		// Renders an SRTSubtitleEntry and generates a SubtitlePicture. Can only be called between
		// BeginDraw() and EndDraw(). origin will be modified to contain the origin that this 
		// subtitle drew itself it. Data in each subpic has not been filled out; call
		// FillSubtitlePictureData once the drawing operations are done to fill out the data.
		SubtitlePicture RenderSRTSubtitleEntry(SRTSubtitleEntry& entry, D2D_POINT_2F& origin);

		void FillSubtitlePictureData(SubtitlePicture& subpic);
	};
};

#endif