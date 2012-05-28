#include "stdafx.h"
#include "ISubtitleRenderer.h"

namespace SubtitleCore
{
	// This renderer does nothing.
	class NoSubtitleRenderer : public ISubtitleRenderer
	{
	public:
		NoSubtitleRenderer();
		~NoSubtitleRenderer();

		//==================================================================
		// ISubtitleRenderer implementation. See ISubtitleRenderer for info
		//==================================================================
		virtual bool ParseScript(const std::vector<std::wstring>& script);
		virtual bool ParseLine(const std::wstring& line, REFERENCE_TIME rtStart, REFERENCE_TIME rtEnd);
		virtual bool ParseLine(const std::wstring& line);
		virtual bool ParseFormatHeader(const std::wstring& header);
		virtual bool ParseFormatHeader(const unsigned char* header);
		virtual bool ParseData(const unsigned char* data);
		virtual bool ParseData(const unsigned char* data, ptrdiff_t offset);
		virtual bool ParseData(const unsigned char* data, ptrdiff_t startOffset, ptrdiff_t endOffset);
		virtual void Invalidate();
		virtual size_t GetSubtitlePictureCount(REFERENCE_TIME rtNow);
		virtual void GetSubtitlePicture(REFERENCE_TIME rtNow, SubtitlePicture** ppOutSubPics);
	};
}