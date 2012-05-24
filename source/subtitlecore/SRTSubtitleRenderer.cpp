#include "stdafx.h"
#include "SRTSubtitleRenderer.h"

using namespace SubtitleCore;

SRTSubtitleRenderer::SRTSubtitleRenderer(SubtitleCoreConfigurationData& config, VideoInfo& vidInfo, IDWriteFactory* dwFactory)
	: m_SubCoreConfig(config)
	, m_VideoInfo(vidInfo)
	, m_DWriteFactory(dwFactory)
{
	m_SubtitleType = SBT_SRT;
}

bool SRTSubtitleRenderer::ParseScript(const std::wstring& script)
{
	UNREFERENCED_PARAMETER(script);
	return true;
}

bool SRTSubtitleRenderer::ParseLine(const std::wstring& line)
{
	UNREFERENCED_PARAMETER(line);
	return true;
}

bool SRTSubtitleRenderer::ParseFormatHeader(const std::wstring& header)
{
	UNREFERENCED_PARAMETER(header);
	return true;
}

bool SRTSubtitleRenderer::ParseFormatHeader(const unsigned char* header)
{
	UNREFERENCED_PARAMETER(header);
	return true;
}

bool SRTSubtitleRenderer::ParseData(const unsigned char* data)
{
	UNREFERENCED_PARAMETER(data);
	return false;
}

bool SRTSubtitleRenderer::ParseData(const unsigned char* data, ptrdiff_t offset)
{
	UNREFERENCED_PARAMETER(data);
	UNREFERENCED_PARAMETER(offset);
	return false;
}

bool SRTSubtitleRenderer::ParseData(const unsigned char* data, ptrdiff_t startOffset, ptrdiff_t endOffset)
{
	UNREFERENCED_PARAMETER(data);
	UNREFERENCED_PARAMETER(startOffset);
	UNREFERENCED_PARAMETER(endOffset);
	return false;
}

void SRTSubtitleRenderer::Invalidate()
{

}

size_t SRTSubtitleRenderer::GetSubtitlePictureCount(REFERENCE_TIME rtNow)
{
	UNREFERENCED_PARAMETER(rtNow);
	return 0U;
}

void SRTSubtitleRenderer::GetSubtitlePicture(REFERENCE_TIME rtNow, SubtitlePicture** ppOutSubPics)
{
	UNREFERENCED_PARAMETER(rtNow);
	UNREFERENCED_PARAMETER(ppOutSubPics);
}