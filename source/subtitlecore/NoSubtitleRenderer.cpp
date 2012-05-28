#include "stdafx.h"
#include "NoSubtitleRenderer.h"

using namespace SubtitleCore;

NoSubtitleRenderer::NoSubtitleRenderer()
{
	m_SubtitleType = SBT_NONE;
}

NoSubtitleRenderer::~NoSubtitleRenderer()
{

}
bool NoSubtitleRenderer::ParseScript(const std::vector<std::wstring>& script)
{
	UNREFERENCED_PARAMETER(script);
	return false;
}

bool NoSubtitleRenderer::ParseLine(const std::wstring& line)
{
	UNREFERENCED_PARAMETER(line);
	return false;
}

bool NoSubtitleRenderer::ParseLine(const std::wstring& line, REFERENCE_TIME rtStart, REFERENCE_TIME rtEnd)
{
	UNREFERENCED_PARAMETER(line);
	UNREFERENCED_PARAMETER(rtStart);
	UNREFERENCED_PARAMETER(rtEnd);
	return false;
}

bool NoSubtitleRenderer::ParseFormatHeader(const std::wstring& header)
{
	UNREFERENCED_PARAMETER(header);
	return true;
}

bool NoSubtitleRenderer::ParseFormatHeader(const unsigned char* header)
{
	UNREFERENCED_PARAMETER(header);
	return true;
}

bool NoSubtitleRenderer::ParseData(const unsigned char* data)
{
	UNREFERENCED_PARAMETER(data);
	return false;
}

bool NoSubtitleRenderer::ParseData(const unsigned char* data, ptrdiff_t offset)
{
	UNREFERENCED_PARAMETER(data);
	UNREFERENCED_PARAMETER(offset);
	return false;
}

bool NoSubtitleRenderer::ParseData(const unsigned char* data, ptrdiff_t startOffset, ptrdiff_t endOffset)
{
	UNREFERENCED_PARAMETER(data);
	UNREFERENCED_PARAMETER(startOffset);
	UNREFERENCED_PARAMETER(endOffset);
	return false;
}

void NoSubtitleRenderer::Invalidate()
{

}

size_t NoSubtitleRenderer::GetSubtitlePictureCount(REFERENCE_TIME rtNow)
{
	UNREFERENCED_PARAMETER(rtNow);
	return 0;
}

void NoSubtitleRenderer::GetSubtitlePicture(REFERENCE_TIME rtNow, SubtitlePicture** ppOutSubPics)
{
	UNREFERENCED_PARAMETER(rtNow);
	UNREFERENCED_PARAMETER(ppOutSubPics);
}