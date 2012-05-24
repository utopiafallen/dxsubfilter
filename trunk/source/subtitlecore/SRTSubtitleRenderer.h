// SRT Subtitle renderer
#ifndef SRTSUBTITLERENDERER_H
#define SRTSUBTITLERENDERER_H
#pragma once

#include "stdafx.h"
#include "ISubtitleRenderer.h"
#include "SubtitleCoreEnumerations.h"

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

		//==================================================================
		// ISubtitleRenderer implementation. See ISubtitleRenderer for info
		//==================================================================
		virtual bool ParseScript(const std::vector<std::wstring>& script);
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

	private:
		SubtitleCoreConfigurationData m_SubCoreConfig;
		VideoInfo m_VideoInfo;
		IDWriteFactory* m_DWriteFactory;
	};
};

#endif