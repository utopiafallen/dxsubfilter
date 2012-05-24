// SRT Subtitle renderer
#ifndef SRTSUBTITLERENDERER_H
#define SRTSUBTITLERENDERER_H
#pragma once

#include "stdafx.h"
#include "ISubtitleRenderer.h"
#include "SubtitleCoreEnumerations.h"

namespace SubtitleCore
{
	class SRTSubtitleRenderer : public ISubtitleRenderer
	{
	public:
		// All constructor parameters are expected to be valid.
		SRTSubtitleRenderer(SubtitleCoreConfigurationData& config, VideoInfo& vidInfo, IDWriteFactory* dwFactory);

		//==================================================================
		// ISubtitleRenderer implementation. See ISubtitleRenderer for info
		//==================================================================
		virtual bool ParseScript(const std::wstring& script);
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