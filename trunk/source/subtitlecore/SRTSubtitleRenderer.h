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
		ID3D10Device1* m_pDevice;
		ID3D10Texture2D* m_pTexture;

		// Parsed subtitle info, keyed on subtitle start time.
		std::unordered_map<REFERENCE_TIME, std::vector<SRTSubtitleEntry>> m_SubtitleMap;

		std::set<std::pair<REFERENCE_TIME, REFERENCE_TIME>> m_SubtitleTimeSpans;

		// Valid time spans that encompass the most recently requested playback time
		std::vector<std::pair<REFERENCE_TIME, REFERENCE_TIME>> m_ValidSubtitleTimes;

		// Rendered subtitles
		struct RenderedSubtitles
		{
			SubtitlePicture SubPic;
			REFERENCE_TIME EndTime;
		};
		std::unordered_map<REFERENCE_TIME, std::vector<RenderedSubtitles>> m_RenderedSubtitles;

	private: // Functions

		// Returns whether or not a line is a timestamp
		bool CheckLineIsTimestamp(const std::wstring& line);

		// Computes timestamps and stores them into rtStart and rtEnd
		void ComputeTimestamp(const std::wstring& line, REFERENCE_TIME& rtStart, REFERENCE_TIME& rtEnd);
	};
};

#endif