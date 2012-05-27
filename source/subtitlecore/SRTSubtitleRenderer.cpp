#include "stdafx.h"
#include "SRTSubtitleRenderer.h"
#include "SRTSubtitleEntry.h"

#include "Conversions.h"
#include "DirectXHelpers.h"

using namespace SubtitleCore;

static const float fSpacer = 7.0f;

SRTSubtitleRenderer::SRTSubtitleRenderer(SubtitleCoreConfigurationData& config, VideoInfo& vidInfo, IDWriteFactory* dwFactory)
	: m_SubCoreConfig(config)
	, m_VideoInfo(vidInfo)
	, m_fHorizontalMargin(static_cast<float>(config.m_LineMarginLeft + config.m_LineMarginRight))
	, m_fVerticalMargin(static_cast<float>(config.m_LineMarginBottom + config.m_LineMarginTop))
	, m_pDWriteFactory(dwFactory)
	, m_pDWTextFormat(nullptr)
	, m_pD2DFactory(nullptr)
	, m_pRT(nullptr)
	, m_pWICBitmap(nullptr)
	, m_pSolidColorBrush(nullptr)
	, m_pDevice(nullptr)
	, m_pTexture(nullptr)
	, m_pWICFactory(nullptr)
{
	m_SubtitleType = SBT_SRT;

	HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);

	hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IWICImagingFactory,
        reinterpret_cast<void **>(&m_pWICFactory)
        );

	float fDpiX, fDpiY;
	m_pD2DFactory->GetDesktopDpi(&fDpiX, &fDpiY);
	m_fDPIScaleX = fDpiX / 96.0f;
	m_fDPIScaleY = fDpiY / 96.0f;

	if (SUCCEEDED(hr))
	{
		hr = m_pWICFactory->CreateBitmap(
			m_VideoInfo.Width,
            m_VideoInfo.Height,
            GUID_WICPixelFormat32bppPBGRA,
            WICBitmapCacheOnLoad,
            &m_pWICBitmap
            );
	}

	if (SUCCEEDED(hr))
    {
		D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
				D2D1_RENDER_TARGET_TYPE_DEFAULT,
				D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
				96,
				96);
        hr = m_pD2DFactory->CreateWicBitmapRenderTarget(
            m_pWICBitmap,
            &props,
            &m_pRT
			);
    }

	if (SUCCEEDED(hr))
	{
		hr = m_pDWriteFactory->CreateTextFormat(m_SubCoreConfig.m_FontName.c_str(),
								NULL,
								m_SubCoreConfig.m_FontWeight,
								m_SubCoreConfig.m_FontStyle,
								m_SubCoreConfig.m_FontStretch,
								SubtitleCoreUtilities::ConvertFontPointToDIP(m_SubCoreConfig.m_FontSize),
								m_SubCoreConfig.m_SystemLocale.c_str(),
								&m_pDWTextFormat);
	}

	DWRITE_TEXT_ALIGNMENT textAlignment;
	DWRITE_PARAGRAPH_ALIGNMENT paraAlignment;

	SubtitleCoreUtilities::ConvertDLAToDWRiteEnums(m_SubCoreConfig.m_LineAlignment, textAlignment, paraAlignment);

	if (SUCCEEDED(hr))
	{
		hr = m_pDWTextFormat->SetTextAlignment(textAlignment);
	}

	if (SUCCEEDED(hr))
	{
		hr = m_pDWTextFormat->SetParagraphAlignment(paraAlignment);
	}

	if (paraAlignment == DWRITE_PARAGRAPH_ALIGNMENT_CENTER || paraAlignment == DWRITE_PARAGRAPH_ALIGNMENT_FAR)
	{
		m_fSubtitlePlacementDirection = -1.0f;
	}
	else
	{
		m_fSubtitlePlacementDirection = 1.0f;
	}

	//hr = D3D10CreateDevice1(NULL, D3D10_DRIVER_TYPE_HARDWARE, NULL, 
	//	D3D10_CREATE_DEVICE_BGRA_SUPPORT, D3D10_FEATURE_LEVEL_10_1, D3D10_1_SDK_VERSION, &m_pDevice);

	//if (SUCCEEDED(hr))
	//{
	//	D3D10_TEXTURE2D_DESC texDesc;
	//	ZeroMemory(&texDesc, sizeof(texDesc));
	//	texDesc.ArraySize = 1;
	//	texDesc.BindFlags = D3D10_BIND_RENDER_TARGET;
	//	texDesc.CPUAccessFlags = 0;
	//	texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	//	texDesc.Height = m_VideoInfo.Height;
	//	texDesc.Width = m_VideoInfo.Width;
	//	texDesc.MipLevels = 1;
	//	texDesc.MiscFlags = 0;
	//	texDesc.SampleDesc.Count = 1;
	//	texDesc.SampleDesc.Quality = 0;
	//	texDesc.Usage = D3D10_USAGE_DEFAULT;

	//	hr = m_pDevice->CreateTexture2D(&texDesc, NULL, &m_pTexture);
	//}

	//IDXGISurface* pDXGISurface = nullptr;
	//if (SUCCEEDED(hr))
	//{
	//	hr = m_pTexture->QueryInterface(&pDXGISurface);
	//}

	//if (SUCCEEDED(hr))
	//{
	//	// Create a D2D render target which can draw into our offscreen D3D
	//	// surface. Given that we use a constant size for the texture, we
	//	// fix the DPI at 96.
	//	D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
	//			D2D1_RENDER_TARGET_TYPE_DEFAULT,
	//			D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
	//			96,
	//			96);

	//	hr = m_pD2DFactory->CreateDxgiSurfaceRenderTarget(	pDXGISurface,
	//														&props,
	//														&m_pRT);
	//}

	//SafeRelease(&pDXGISurface);

	if (SUCCEEDED(hr))
	{
		hr = m_pRT->CreateSolidColorBrush(ConvertABGRToD2DCOLORF(m_SubCoreConfig.m_FontPrimaryFillColor), 
											&m_pSolidColorBrush);
	}
}

SRTSubtitleRenderer::~SRTSubtitleRenderer()
{
	SafeRelease(&m_pDWTextFormat);
	SafeRelease(&m_pD2DFactory);
	SafeRelease(&m_pRT);
	SafeRelease(&m_pWICBitmap);
	SafeRelease(&m_pSolidColorBrush);
	SafeRelease(&m_pDevice);
	SafeRelease(&m_pTexture);
	SafeRelease(&m_pWICFactory);
}
bool SRTSubtitleRenderer::ParseScript(const std::vector<std::wstring>& script)
{
	// Assume that first line in script is subtitle number and skip it.
	for (size_t i = 1; i < script.size(); i++)
	{
		// This should be a timestamp
		if (CheckLineIsTimestamp(script[i]))
		{
			REFERENCE_TIME rtStart, rtEnd;

			ComputeTimestamp(script[i], rtStart, rtEnd);

			// Merge lines until we reach an empty line which delineates the start of a new
			// subtitle block
			std::wstring mergedLine;
			size_t index = i+1;
			while (script[index].empty() == false)
			{
				mergedLine += script[index] + L"\n";
				++index;
			}
			// Move overall script parsing position to start of the next subtitle block
			i = index+1;

			// Parse merged line
			ParseLine(mergedLine, rtStart, rtEnd);
		}
		else
		{
			// Something bad happened
			return false;
		}
	}

	return true;
}

bool SRTSubtitleRenderer::ParseLine(const std::wstring& line)
{
	UNREFERENCED_PARAMETER(line);
	return false;
}

bool SRTSubtitleRenderer::ParseLine(const std::wstring& line, REFERENCE_TIME rtStart, REFERENCE_TIME rtEnd)
{
	SRTSubtitleEntry* entry = nullptr;

	auto& subtitle_list = m_SubtitleMap[rtStart];

	if (subtitle_list.size() == 0)
	{
		// Subtitle list with this start time was empty so add a new entry
		subtitle_list.push_back(SRTSubtitleEntry());

		entry = &subtitle_list[0];
		entry->StartTime = rtStart;
		entry->EndTime = rtEnd;

		// Add this to set of timespans
		m_SubtitleTimeSpans.insert(std::make_pair(rtStart, rtEnd));
	}
	else
	{
		// Check to make sure this isn't a duplicate
		for (auto it = subtitle_list.begin(); it != subtitle_list.end(); ++it)
		{
			if (it->Text.compare(line) == 0)
			{
				// Do nothing
				return true;
			}
		}

		subtitle_list.push_back(SRTSubtitleEntry());

		entry = &subtitle_list[subtitle_list.size() - 1];
		entry->StartTime = rtStart;
		entry->EndTime = rtEnd;

		// Add this to set of timespans
		m_SubtitleTimeSpans.insert(std::make_pair(rtStart, rtEnd));
	}

	std::wstring finalLine;

	// Check for supported HTML tags
	size_t startPos, openBracketPos, closeBracketPos;
	size_t finalStartPos, finalEndPos;
	std::vector<std::wstring> tagStack;

	startPos = 0;

	while ((openBracketPos = line.find(L"<", startPos)) != std::wstring::npos)
	{
		finalStartPos = finalLine.size();
		finalLine += line.substr(startPos, openBracketPos - startPos);
		finalEndPos = finalLine.size();

		if (finalEndPos != finalStartPos)
		{
			SRTSubtitleEntry::TextRangeFormat trf;

			trf.Range.length = finalEndPos - finalStartPos;
			trf.Range.startPosition = finalStartPos;
			trf.Style = m_SubCoreConfig.m_FontStyle;
			trf.Weight = m_SubCoreConfig.m_FontWeight;
			trf.Strikethrough = FALSE;
			trf.Underline = FALSE;

			for (auto it = tagStack.begin(); it != tagStack.end(); ++it)
			{
				if ((*it).compare(L"b") == 0)
				{
					trf.Weight = DWRITE_FONT_WEIGHT_BOLD;
				}
				else if ((*it).compare(L"i") == 0)
				{
					trf.Style = DWRITE_FONT_STYLE_ITALIC;
				}
				else if ((*it).compare(L"u") == 0)
				{
					trf.Underline = TRUE;
				}
				else if ((*it).compare(L"s") == 0)
				{
					trf.Strikethrough = TRUE;
				}
			}

			entry->SubTextFormat.push_back(trf);
		}

		// Find close bracket
		closeBracketPos = line.find(L">", openBracketPos);

		std::wstring tag = line.substr(openBracketPos+1, closeBracketPos - openBracketPos - 1);

		// Is this a closing tag?
		if (tag.find(L"/") != std::wstring::npos)
		{
			// Malformed scripts are just going to break
			tagStack.pop_back();
		}
		else
		{
			tagStack.push_back(tag);
		}

		startPos = closeBracketPos+1;
	}

	if (startPos != line.size() - 1)
	{
		// Copy remainder line
		finalLine += line.substr(startPos);
	}

	entry->Text = finalLine;

	if (m_SubCoreConfig.m_SubtitleBufferSize > 0)
	{
		// Spawn task to render subtitle
	}

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
	m_SubtitleMap.clear();
	m_RenderedSubtitles.clear();
	m_ValidSubtitleTimes.clear();
	m_SubtitleTimeSpans.clear();
}

size_t SRTSubtitleRenderer::GetSubtitlePictureCount(REFERENCE_TIME rtNow)
{
	// Check to see if rtNow falls within previously created list of valid time spans
	bool bStillWithinSpans = false;
	for (auto it = m_ValidSubtitleTimes.begin(); it != m_ValidSubtitleTimes.end(); ++it)
	{
		if (it->first <= rtNow && it->second >= rtNow)
		{
			bStillWithinSpans = true;
			break;
		}
	}

	if (!bStillWithinSpans)
	{
		m_ValidSubtitleTimes.clear();

		// Get closest subtitle time spans
		auto searchValue = std::make_pair(rtNow - 100, rtNow + 100);

		// Find first time span greater than current time
		auto result = m_SubtitleTimeSpans.upper_bound(searchValue);

		// Search backwards to find the first timespan that encompasses current time
		auto start = result;
		for (;start != m_SubtitleTimeSpans.begin(); --start)
		{
			if (start->first <= rtNow && start->second >= rtNow)
			{
				break;
			}
		}
	
		// Need to specifically check begin since we skip it
		if (m_SubtitleTimeSpans.begin()->first <= rtNow &&
			m_SubtitleTimeSpans.begin()->second >= rtNow)
		{
			start = m_SubtitleTimeSpans.begin();
		}

		// Add list of start times to valid subtitle times
		for (auto it = start; it != result; ++it)
		{
			m_ValidSubtitleTimes.push_back(*it);
		}
	}

	size_t count = 0;
	if (m_SubCoreConfig.m_SubtitleBufferSize > 0)
	{
		// Just check the rendered subtitles
		//for (auto it = m_ValidSubtitleTimes.begin(); it != m_ValidSubtitleTimes.end()
	}
	else
	{
		for (auto it = m_ValidSubtitleTimes.begin(); it != m_ValidSubtitleTimes.end(); ++it)
		{
			count += m_SubtitleMap[it->first].size();
		}
	}

	return count;
}

void SRTSubtitleRenderer::GetSubtitlePicture(REFERENCE_TIME rtNow, SubtitlePicture** ppOutSubPics)
{
	// Check already rendered results and store them into output
	size_t renderedIndex = 0;
	std::vector<RenderedSubtitles*> renderedSubsToRemove;
	std::vector<std::pair<REFERENCE_TIME, REFERENCE_TIME>> subtitleSpansAlreadyRendered;
	for (auto renderedIt = m_RenderedSubtitles.begin(); renderedIt != m_RenderedSubtitles.end(); ++renderedIt)
	{
		if (renderedIt->StartTime <= rtNow && renderedIt->EndTime >= rtNow)
		{
			ppOutSubPics[renderedIndex++] = &renderedIt->SubPic;

			auto renderedSpan = std::make_pair(renderedIt->StartTime, renderedIt->EndTime);
			subtitleSpansAlreadyRendered.push_back(renderedSpan);
			m_ValidSubtitleTimes.remove(renderedSpan);
		}
		else
		{
			renderedSubsToRemove.push_back(&(*renderedIt));
		}
	}

	// Clear out stale subs
	for (auto removeIt = renderedSubsToRemove.begin(); removeIt != renderedSubsToRemove.end(); ++removeIt)
	{
		m_RenderedSubtitles.remove(*(*removeIt));
	}

	// Render unrendered timespans
	HRESULT hr = S_OK;
	UNREFERENCED_PARAMETER(hr); // hr is only used for debugging purposes

	size_t newSubIndex = renderedIndex;
	RenderedSubtitles rsub;

	m_pRT->BeginDraw();
	{
		m_pRT->Clear();

		D2D_POINT_2F origin;

		// Process the first entry first so we can offset the remaining subtitles properly
		std::vector<SRTSubtitleEntry> subtitleEntries = 
			m_SubtitleMap[m_ValidSubtitleTimes.begin()->first];

		IDWriteTextLayout* pTextLayout = nullptr;

		hr = m_pDWriteFactory->CreateTextLayout(subtitleEntries.begin()->Text.c_str(), 
									subtitleEntries.begin()->Text.length(),
									m_pDWTextFormat,
									static_cast<float>(m_VideoInfo.Width) - m_fHorizontalMargin,
									static_cast<float>(m_VideoInfo.Height) - m_fVerticalMargin,
									&pTextLayout);

		for (auto formatIt = subtitleEntries.begin()->SubTextFormat.begin(); 
			formatIt != subtitleEntries.begin()->SubTextFormat.end(); 
			++formatIt)
		{
			pTextLayout->SetFontWeight(formatIt->Weight, formatIt->Range);
			pTextLayout->SetFontStyle(formatIt->Style, formatIt->Range);
			pTextLayout->SetUnderline(formatIt->Underline, formatIt->Range);
			pTextLayout->SetStrikethrough(formatIt->Strikethrough, formatIt->Range);
		}

		DWRITE_TEXT_METRICS metrics;
		pTextLayout->GetMetrics(&metrics);

		origin.x = static_cast<float>(m_SubCoreConfig.m_LineMarginLeft);
		origin.y = static_cast<float>(m_SubCoreConfig.m_LineMarginTop);

		m_pRT->DrawTextLayout(origin, pTextLayout, m_pSolidColorBrush);

		origin.y = origin.y + (metrics.height + fSpacer) * m_fSubtitlePlacementDirection + m_fVerticalMargin;

		SafeRelease(&pTextLayout);

		int originX, originY;
		size_t width, height;
		originX = static_cast<int>(metrics.left + origin.x);
		originY = static_cast<int>(metrics.top + origin.y);
		ConvertDIPToPixels(metrics.width, metrics.height, width, height);

		rsub.StartTime = m_ValidSubtitleTimes.begin()->first;
		rsub.EndTime = m_ValidSubtitleTimes.begin()->second;
		rsub.SubPic = SubtitlePicture(originX, originY, width, height, width, SBPF_PBGRA32, nullptr);
			
		m_RenderedSubtitles.push_back(rsub);
		ppOutSubPics[newSubIndex++] = &(m_RenderedSubtitles.back().SubPic);

		// Process remaining SRT entries for the first time span
		for(auto subIt = subtitleEntries.begin()+1; subIt != subtitleEntries.end(); ++subIt)
		{
			rsub.StartTime = subIt->StartTime;
			rsub.EndTime = subIt->EndTime;
			rsub.SubPic = RenderSRTSubtitleEntry(*subIt, origin);

			m_RenderedSubtitles.push_back(rsub);
			ppOutSubPics[newSubIndex++] = &(m_RenderedSubtitles.back().SubPic);
		}

		// Process remaining time spans
		auto it = m_ValidSubtitleTimes.begin();
		for(++it; it != m_ValidSubtitleTimes.end(); ++it)
		{
			subtitleEntries = m_SubtitleMap[it->first];

			for(auto subIt = subtitleEntries.begin(); subIt != subtitleEntries.end(); ++subIt)
			{
				rsub.StartTime = subIt->StartTime;
				rsub.EndTime = subIt->EndTime;
				rsub.SubPic = RenderSRTSubtitleEntry(*subIt, origin);

				m_RenderedSubtitles.push_back(rsub);
				ppOutSubPics[newSubIndex++] = &(m_RenderedSubtitles.back().SubPic);
			}
		}
	}
	hr = m_pRT->EndDraw();

	// Put back rendered timespans into valid timespans
	for (auto rspan = subtitleSpansAlreadyRendered.begin(); rspan != subtitleSpansAlreadyRendered.end(); ++rspan)
	{
		m_ValidSubtitleTimes.push_back(*rspan);
	}

	// Fill out the data for newly rendered subtitle pictures
	for (size_t i = renderedIndex; i < newSubIndex; i++)
	{
		FillSubtitlePictureData(*ppOutSubPics[i]);
	}

	IWICBitmapEncoder *pEncoder = NULL;
	IWICBitmapFrameEncode *pFrameEncode = NULL;
	IWICStream *pStream = NULL;
	if (SUCCEEDED(hr))
	{
		hr = m_pWICFactory->CreateStream(&pStream);
	}
	WICPixelFormatGUID format = GUID_WICPixelFormatDontCare;
	if (SUCCEEDED(hr))
	{
		static const WCHAR filename[] = L"C:\\Users\\Xin Liu\\Desktop\\output.png";
		hr = pStream->InitializeFromFilename(filename, GENERIC_WRITE);
	}
	if (SUCCEEDED(hr))
	{
		hr = m_pWICFactory->CreateEncoder(GUID_ContainerFormatPng, NULL, &pEncoder);
	}
	if (SUCCEEDED(hr))
	{
		hr = pEncoder->Initialize(pStream, WICBitmapEncoderNoCache);
	}
	if (SUCCEEDED(hr))
	{
		hr = pEncoder->CreateNewFrame(&pFrameEncode, NULL);
	}
	if (SUCCEEDED(hr))
	{
		hr = pFrameEncode->Initialize(NULL);
	}
	if (SUCCEEDED(hr))
	{
		hr = pFrameEncode->SetSize(m_VideoInfo.Width, m_VideoInfo.Height);
	}
	if (SUCCEEDED(hr))
	{
		hr = pFrameEncode->SetPixelFormat(&format);
	}
	if (SUCCEEDED(hr))
	{
		hr = pFrameEncode->WriteSource(m_pWICBitmap, NULL);
	}
	if (SUCCEEDED(hr))
	{
		hr = pFrameEncode->Commit();
	}
	if (SUCCEEDED(hr))
	{
		hr = pEncoder->Commit();
	}
	SafeRelease(&pStream);
	SafeRelease(&pEncoder);
	SafeRelease(&pFrameEncode);
}

bool SRTSubtitleRenderer::CheckLineIsTimestamp(const std::wstring& line)
{
	return (line.find(L"-->") != std::wstring::npos &&
		line.find(L":") != std::wstring::npos && 
		line.find(L",") != std::wstring::npos);
}

void SRTSubtitleRenderer::ComputeTimestamp(const std::wstring& line, REFERENCE_TIME& rtStart, REFERENCE_TIME& rtEnd)
{
	size_t startPos, endPos;
	size_t hours, minutes, seconds, milliseconds;

	startPos = 0;

	// Get hours
	endPos = line.find(L":");
	hours = boost::lexical_cast<size_t>(line.substr(startPos, endPos - startPos));

	// Get minutes
	startPos = endPos + 1;
	endPos = line.find(L":", startPos);
	minutes = boost::lexical_cast<size_t>(line.substr(startPos, endPos - startPos));

	// Get seconds
	startPos = endPos + 1;
	endPos = line.find(L",", startPos);
	seconds = boost::lexical_cast<size_t>(line.substr(startPos, endPos - startPos));

	// Get milliseconds
	startPos = endPos + 1;
	endPos = line.find(L" ", startPos);
	milliseconds = boost::lexical_cast<size_t>(line.substr(startPos, endPos - startPos));

	// Compute starting time
	rtStart = SubtitleCoreUtilities::ConvertTimeToReferenceTime(hours, minutes, seconds, milliseconds);

	// Move to ending timestamp
	startPos = line.find(L" ", endPos+1) + 1;

	// Get hours
	endPos = line.find(L":", startPos);
	hours = boost::lexical_cast<size_t>(line.substr(startPos, endPos - startPos));

	// Get minutes
	startPos = endPos + 1;
	endPos = line.find(L":", startPos);
	minutes = boost::lexical_cast<size_t>(line.substr(startPos, endPos - startPos));

	// Get seconds
	startPos = endPos + 1;
	endPos = line.find(L",", startPos);
	seconds = boost::lexical_cast<size_t>(line.substr(startPos, endPos - startPos));

	// Get milliseconds
	startPos = endPos + 1;
	milliseconds = boost::lexical_cast<size_t>(line.substr(startPos));

	rtEnd = SubtitleCoreUtilities::ConvertTimeToReferenceTime(hours, minutes, seconds, milliseconds);
}

template<typename T>
void SRTSubtitleRenderer::ConvertDIPToPixels(float dipX, float dipY, T& outX, T& outY)
{
	// Round up
	outX = static_cast<T>(dipX * m_fDPIScaleX + 0.5f);
	outY = static_cast<T>(dipY * m_fDPIScaleY + 0.5f);
}

SubtitlePicture SRTSubtitleRenderer::RenderSRTSubtitleEntry(SRTSubtitleEntry& entry, D2D_POINT_2F& origin)
{
	IDWriteTextLayout* pTextLayout = nullptr;
	DWRITE_TEXT_METRICS metrics;
	HRESULT hr;
	UNREFERENCED_PARAMETER(hr); // hr is only used for debugging purposes

	hr = m_pDWriteFactory->CreateTextLayout(entry.Text.c_str(), 
									entry.Text.length(),
									m_pDWTextFormat,
									static_cast<float>(m_VideoInfo.Width) - m_fHorizontalMargin,
									static_cast<float>(m_VideoInfo.Height) - m_fVerticalMargin,
									&pTextLayout);

	for (auto formatIt = entry.SubTextFormat.begin(); formatIt != entry.SubTextFormat.end(); ++formatIt)
	{
		pTextLayout->SetFontWeight(formatIt->Weight, formatIt->Range);
		pTextLayout->SetFontStyle(formatIt->Style, formatIt->Range);
		pTextLayout->SetUnderline(formatIt->Underline, formatIt->Range);
		pTextLayout->SetStrikethrough(formatIt->Strikethrough, formatIt->Range);
	}

	pTextLayout->GetMetrics(&metrics);

	m_pRT->DrawTextLayout(origin, pTextLayout, m_pSolidColorBrush);

	// Offset for the next subtitle to be draw after us
	origin.y = origin.y + (metrics.height + fSpacer) * m_fSubtitlePlacementDirection + m_fVerticalMargin;

	SafeRelease(&pTextLayout);

	int originX, originY;
	size_t width, height;
	originX = static_cast<int>(metrics.left + origin.x);
	originY = static_cast<int>(metrics.top + origin.y);
	ConvertDIPToPixels(metrics.width, metrics.height, width, height);

	return SubtitlePicture(originX, originY, width, height, width, SBPF_PBGRA32, nullptr);
}

void SRTSubtitleRenderer::FillSubtitlePictureData(SubtitlePicture& subpic)
{
	HRESULT hr;
	UNREFERENCED_PARAMETER(hr); // hr is only used for debugging purposes

	WICRect rect;
	rect.X = subpic.m_iOriginX;
	rect.Y = subpic.m_iOriginY;
	rect.Height = subpic.m_uHeight;
	rect.Width = subpic.m_uWidth;

	IWICBitmapLock* pBitmapLock = nullptr;
	hr = m_pWICBitmap->Lock(&rect, WICBitmapLockRead, &pBitmapLock);

	UINT pcbStride;
	pBitmapLock->GetStride(&pcbStride);

	size_t byteCount = 0;
	BYTE* bitmapData = nullptr;
	pBitmapLock->GetDataPointer(&byteCount, &bitmapData);

	unsigned char* data = static_cast<unsigned char*>(_aligned_malloc(byteCount, 16));
	
	memcpy(data, bitmapData, byteCount);

	subpic.m_uStride = pcbStride;
	subpic.m_Data = std::shared_ptr<unsigned char>(data, SubtitlePicture::Deleter<unsigned char>());
}