#include "stdafx.h"
#include "SRTSubtitleRenderer.h"
#include "SRTSubtitleEntry.h"

#include "CustomDrawingEffects.h"
#include "CustomTextRenderer.h"

#include "SCUConversions.h"
#include "DirectXHelpers.h"

using namespace SubtitleCore;

static const float fSpacer = 7.0f;
static const size_t uDIPPadding = 4;

SRTSubtitleRenderer::SRTSubtitleRenderer(SubtitleCoreConfigurationData& config, const VideoInfo& vidInfo, IDWriteFactory* dwFactory)
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
	, m_pWICFactory(nullptr)
	, m_pOutlineColorBrush(nullptr)
	, m_pShadowColorBrush(nullptr)
	, m_pCustomTextRenderer(nullptr)
	, m_pGlobalDrawingEffects(nullptr)
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
	m_fDPIScaleX = fDpiX * (1.0f / 96.0f);
	m_fDPIScaleY = fDpiY * (1.0f / 96.0f);

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
								SCU::ConvertFontPointToDIP(m_SubCoreConfig.m_FontSize),
								m_SubCoreConfig.m_SystemLocale.c_str(),
								&m_pDWTextFormat);
	}

	DWRITE_TEXT_ALIGNMENT textAlignment;
	DWRITE_PARAGRAPH_ALIGNMENT paraAlignment;

	SCU::ConvertDLAToDWRiteEnums(m_SubCoreConfig.m_LineAlignment, textAlignment, paraAlignment);

	if (SUCCEEDED(hr))
	{
		hr = m_pDWTextFormat->SetTextAlignment(textAlignment);
	}

	if (SUCCEEDED(hr))
	{
		hr = m_pDWTextFormat->SetParagraphAlignment(paraAlignment);
	}

	if (paraAlignment == DWRITE_PARAGRAPH_ALIGNMENT_CENTER || 
		paraAlignment == DWRITE_PARAGRAPH_ALIGNMENT_FAR)
	{
		m_fSubtitlePlacementDirection = -1.0f;
	}
	else
	{
		m_fSubtitlePlacementDirection = 1.0f;
	}

	if (SUCCEEDED(hr))
	{
		hr = m_pRT->CreateSolidColorBrush(ConvertABGRToD2DCOLORF(m_SubCoreConfig.m_FontPrimaryFillColor), 
											&m_pSolidColorBrush);
	}

	if (SUCCEEDED(hr))
	{
		hr = m_pRT->CreateSolidColorBrush(ConvertABGRToD2DCOLORF(m_SubCoreConfig.m_FontOutlineColor),
											&m_pOutlineColorBrush);
	}

	if (SUCCEEDED(hr))
	{
		hr = m_pRT->CreateSolidColorBrush(ConvertABGRToD2DCOLORF(m_SubCoreConfig.m_FontShadowColor),
											&m_pShadowColorBrush);
	}

	if (SUCCEEDED(hr))
	{
		m_pCustomTextRenderer = new CustomTextRenderer(m_pD2DFactory, m_pRT);
		m_pCustomTextRenderer->AddRef();

		if (m_SubCoreConfig.m_uFontBorderWidth > 0U)
		{
			m_pGlobalDrawingEffects = new DrawingEffectsCollection();
			m_pGlobalDrawingEffects->AddRef();

			m_pGlobalDrawingEffects->m_Effects.push_back(
				std::make_shared<OutlineAndFillDrawingEffect>(m_pSolidColorBrush, m_pOutlineColorBrush, 
				SCU::ConvertPixelsToDIP(m_SubCoreConfig.m_uFontBorderWidth, m_fDPIScaleX))
			);
		}
	}
}

SRTSubtitleRenderer::~SRTSubtitleRenderer()
{
	SafeRelease(&m_pGlobalDrawingEffects);
	SafeRelease(&m_pCustomTextRenderer);
	SafeRelease(&m_pShadowColorBrush);
	SafeRelease(&m_pOutlineColorBrush);
	SafeRelease(&m_pSolidColorBrush);
	SafeRelease(&m_pDWTextFormat);
	SafeRelease(&m_pRT);
	SafeRelease(&m_pWICBitmap);
	SafeRelease(&m_pWICFactory);
	SafeRelease(&m_pD2DFactory);
}
bool SRTSubtitleRenderer::ParseScript(const std::vector<std::wstring>& script)
{
	// Assume that first line in script is subtitle number and skip it.
	for (size_t i = 1; i < script.size(); i++)
	{
		// This should be a timestamp
		assert(CheckLineIsTimestamp(script[i]));

		REFERENCE_TIME rtStart, rtEnd;

		ComputeTimestamp(script[i], rtStart, rtEnd);

		// Merge lines until we reach an empty line which delineates the start of a new
		// subtitle block
		std::wstring mergedLine = script[i+1];
		size_t index = i+2;
		while (script[index].empty() == false)
		{
			mergedLine += L"\n" + script[index];
			++index;
		}
		// Move overall script parsing position to start of the next subtitle block
		i = index+1;

		// Parse merged line
		ParseLine(mergedLine, rtStart, rtEnd);
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
		for (auto it = subtitle_list.begin(), itEnd = subtitle_list.end(); it != itEnd; ++it)
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

			for (auto it = tagStack.begin(), itEnd = tagStack.end(); it != itEnd; ++it)
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
	if (m_SubtitleTimeSpans.size() > 0)
	{
		// Clear stale entries
		tTimeSpanSet staleEntries;
		for (auto it = m_ValidSubtitleTimes.begin(), itEnd = m_ValidSubtitleTimes.end(); it != itEnd; ++it)
		{
			if (it->second < rtNow)
			{
				staleEntries.insert(*it);
			}
		}
		for (auto it = staleEntries.begin(), itEnd = staleEntries.end(); it != itEnd; ++it)
		{
			m_ValidSubtitleTimes.erase(*it);
		}

		// Create a timespan from current time so we can search for valid subtitles
		auto searchValue = std::make_pair(rtNow - 100, rtNow + 100);

		// Find first time span greater than current time
		auto result = m_SubtitleTimeSpans.upper_bound(searchValue);

		// Search backwards to find the timespans that encompasses current time. Since we use a
		// set, we don't have to worry about duplicate entries.
		auto start = --result;
		for (;start != m_SubtitleTimeSpans.begin(); --start)
		{
			if (start->first <= rtNow && start->second >= rtNow)
			{
				m_ValidSubtitleTimes.insert(*start);
			}
			else
			{
				break;
			}
		}
	
		// Need to specifically check begin since we skip it
		if (m_SubtitleTimeSpans.begin()->first <= rtNow &&
			m_SubtitleTimeSpans.begin()->second >= rtNow)
		{
			m_ValidSubtitleTimes.insert(*m_SubtitleTimeSpans.begin());
		}
	}

	size_t count = 0;
	for (auto it = m_ValidSubtitleTimes.begin(), itEnd = m_ValidSubtitleTimes.end(); it != itEnd; ++it)
	{
		count += m_SubtitleMap[it->first].size();
	}

	return count;
}

void SRTSubtitleRenderer::GetSubtitlePicture(REFERENCE_TIME rtNow, SubtitlePicture** ppOutSubPics)
{
	D2D_POINT_2F origin;
	origin.x = static_cast<float>(m_SubCoreConfig.m_LineMarginLeft);
	origin.y = static_cast<float>(m_SubCoreConfig.m_LineMarginTop);

	// Check already rendered results and store them into output. 
	size_t renderedIndex = 0;
	std::vector<RenderedSubtitles*> renderedSubsToRemove;
	std::vector<std::pair<REFERENCE_TIME, REFERENCE_TIME>> subtitleSpansAlreadyRendered;
	for (auto renderedIt = m_RenderedSubtitles.begin(), itEnd = m_RenderedSubtitles.end(); renderedIt != itEnd; ++renderedIt)
	{
		auto renderedSpan = std::make_pair(renderedIt->StartTime, renderedIt->EndTime);
		if (renderedIt->StartTime <= rtNow && renderedIt->EndTime >= rtNow)
		{
			ppOutSubPics[renderedIndex++] = &renderedIt->SubPic;

			// Offset origin to account for cached results.
			origin.y += static_cast<float>(renderedIt->SubPic.m_uHeight) * m_fSubtitlePlacementDirection;

			subtitleSpansAlreadyRendered.push_back(renderedSpan);
			m_ValidSubtitleTimes.erase(renderedSpan);
		}
		else
		{
			renderedSubsToRemove.push_back(&(*renderedIt));
		}
	}

	// Clear out stale subs
	for (auto removeIt = renderedSubsToRemove.begin(), itEnd = renderedSubsToRemove.end(); removeIt != itEnd; ++removeIt)
	{
		m_RenderedSubtitles.remove(*(*removeIt));
	}

	// Render unrendered timespans
	HRESULT hr = S_OK;
	UNREFERENCED_PARAMETER(hr); // hr is only used for debugging purposes
	if (m_ValidSubtitleTimes.size() > 0)
	{
		size_t newSubIndex = renderedIndex;

		// Build DrawingContext and any global drawing effects
		DrawingContext context;
		context.m_pFillBrush = m_pSolidColorBrush;

		m_pRT->BeginDraw();
		{
			m_pRT->Clear();

			// Process time spans
			RenderedSubtitles rsub;
			for(auto it = m_ValidSubtitleTimes.begin(), itEnd = m_ValidSubtitleTimes.end(); it != itEnd; ++it)
			{
				std::vector<SRTSubtitleEntry> subtitleEntries = m_SubtitleMap[it->first];

				for(auto subIt = subtitleEntries.begin(), itEnd = subtitleEntries.end(); subIt != itEnd; ++subIt)
				{
					rsub.StartTime = subIt->StartTime;
					rsub.EndTime = subIt->EndTime;

					// Render the actual subtitle
					rsub.SubPic = RenderSRTSubtitleEntry(*subIt, origin, context, m_pGlobalDrawingEffects);

					m_RenderedSubtitles.push_back(rsub);
					ppOutSubPics[newSubIndex++] = &(m_RenderedSubtitles.back().SubPic);
				}
			}
		}
		hr = m_pRT->EndDraw();

		// Fill out the data for newly rendered subtitle pictures
		for (size_t i = renderedIndex; i < newSubIndex; i++)
		{
			FillSubtitlePictureData(*ppOutSubPics[i]);
		}
	}

	// Put back rendered timespans into valid timespans
	for (auto rspan = subtitleSpansAlreadyRendered.begin(), itEnd = subtitleSpansAlreadyRendered.end(); rspan != itEnd; ++rspan)
	{
		m_ValidSubtitleTimes.insert(*rspan);
	}


	//IWICBitmapEncoder *pEncoder = NULL;
	//IWICBitmapFrameEncode *pFrameEncode = NULL;
	//IWICStream *pStream = NULL;
	//if (SUCCEEDED(hr))
	//{
	//	hr = m_pWICFactory->CreateStream(&pStream);
	//}
	//WICPixelFormatGUID format = GUID_WICPixelFormatDontCare;
	//if (SUCCEEDED(hr))
	//{
	//	static const WCHAR filename[] = L"C:\\Users\\Xin Liu\\Desktop\\output.png";
	//	hr = pStream->InitializeFromFilename(filename, GENERIC_WRITE);
	//}
	//if (SUCCEEDED(hr))
	//{
	//	hr = m_pWICFactory->CreateEncoder(GUID_ContainerFormatPng, NULL, &pEncoder);
	//}
	//if (SUCCEEDED(hr))
	//{
	//	hr = pEncoder->Initialize(pStream, WICBitmapEncoderNoCache);
	//}
	//if (SUCCEEDED(hr))
	//{
	//	hr = pEncoder->CreateNewFrame(&pFrameEncode, NULL);
	//}
	//if (SUCCEEDED(hr))
	//{
	//	hr = pFrameEncode->Initialize(NULL);
	//}
	//if (SUCCEEDED(hr))
	//{
	//	hr = pFrameEncode->SetSize(m_VideoInfo.Width, m_VideoInfo.Height);
	//}
	//if (SUCCEEDED(hr))
	//{
	//	hr = pFrameEncode->SetPixelFormat(&format);
	//}
	//if (SUCCEEDED(hr))
	//{
	//	hr = pFrameEncode->WriteSource(m_pWICBitmap, NULL);
	//}
	//if (SUCCEEDED(hr))
	//{
	//	hr = pFrameEncode->Commit();
	//}
	//if (SUCCEEDED(hr))
	//{
	//	hr = pEncoder->Commit();
	//}
	//SafeRelease(&pStream);
	//SafeRelease(&pEncoder);
	//SafeRelease(&pFrameEncode);
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
	rtStart = SCU::ConvertTimeToReferenceTime(hours, minutes, seconds, milliseconds);

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

	rtEnd = SCU::ConvertTimeToReferenceTime(hours, minutes, seconds, milliseconds);
}


SubtitlePicture SRTSubtitleRenderer::RenderSRTSubtitleEntry(SRTSubtitleEntry& entry, D2D_POINT_2F& origin, DrawingContext& context,
															DrawingEffectsCollection* effects)
{
	HRESULT hr;
	UNREFERENCED_PARAMETER(hr); // hr is only used for debugging purposes

	std::vector<ID2D1PathGeometry*> pPathGeometries;

	IDWriteTextLayout* pTextLayout = nullptr;

	hr = m_pDWriteFactory->CreateTextLayout(entry.Text.c_str(), 
									entry.Text.length(),
									m_pDWTextFormat,
									static_cast<float>(m_VideoInfo.Width) - m_fHorizontalMargin,
									static_cast<float>(m_VideoInfo.Height) - m_fVerticalMargin,
									&pTextLayout);

	for (auto formatIt = entry.SubTextFormat.begin(), itEnd = entry.SubTextFormat.end(); formatIt != itEnd; ++formatIt)
	{
		pTextLayout->SetFontWeight(formatIt->Weight, formatIt->Range);
		pTextLayout->SetFontStyle(formatIt->Style, formatIt->Range);
		pTextLayout->SetUnderline(formatIt->Underline, formatIt->Range);
		pTextLayout->SetStrikethrough(formatIt->Strikethrough, formatIt->Range);
	}

	// Apply global drawing effects
	DWRITE_TEXT_RANGE everythingRange;
	everythingRange.length = entry.Text.length();
	everythingRange.startPosition = 0;
	pTextLayout->SetDrawingEffect(effects, everythingRange);

	DWRITE_TEXT_METRICS metrics;
	pTextLayout->GetMetrics(&metrics);

	DWRITE_OVERHANG_METRICS overhang;
	pTextLayout->GetOverhangMetrics(&overhang);

	// Draw text
	//m_pRT->DrawTextLayout(origin, pTextLayout, m_pSolidColorBrush);
	//UNREFERENCED_PARAMETER(context);
	//UNREFERENCED_PARAMETER(effects);
	hr = pTextLayout->Draw(&context, m_pCustomTextRenderer, origin.x, origin.y);

	// Need to manually release because SetDrawingEffect will AddRef()
	SafeRelease(&effects);

	SafeRelease(&pTextLayout);

	int originX, originY;
	size_t width, height;
	originX = static_cast<int>(metrics.left + origin.x - m_SubCoreConfig.m_fFontBorderWidth);
	originY = static_cast<int>(metrics.top + origin.y - m_SubCoreConfig.m_fFontBorderWidth);
	width = SCU::ConvertDIPToPixels(metrics.width + (overhang.right - overhang.left), m_fDPIScaleX) + m_SubCoreConfig.m_uFontBorderWidth + uDIPPadding;
	height = SCU::ConvertDIPToPixels(metrics.height, m_fDPIScaleY) + m_SubCoreConfig.m_uFontBorderWidth + uDIPPadding;

	// Offset for the next subtitle to be draw after us
	origin.y = origin.y + (metrics.height + fSpacer + m_SubCoreConfig.m_fFontBorderWidth) * m_fSubtitlePlacementDirection;

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

	// Why does the byte count not match up with stride and rectangle dimensions?
	subpic.m_uHeight = byteCount / pcbStride;

	unsigned char* data = static_cast<unsigned char*>(_aligned_malloc(byteCount, 16));
	
	memcpy(data, bitmapData, byteCount);

	subpic.m_uStride = pcbStride;
	subpic.m_Data = std::shared_ptr<unsigned char>(data, SubtitlePicture::Deleter<unsigned char>());

	pBitmapLock->Release();
}


// Saving off some code in case I want it later
//if (m_SubCoreConfig.m_FontBorderWidth > 0)
//{
//	std::vector<UINT16> glyphIndices (formatIt->Range.length, 0);
//	std::vector<UINT32> codePoints (formatIt->Range.length, 0);
//
//	for (size_t i = formatIt->Range.startPosition; i < formatIt->Range.length; i++)
//	{
//		codePoints[i] = entry.Text[i];
//	}
//
//	ID2D1PathGeometry* pPathGeometry = nullptr;
//	hr = m_pD2DFactory->CreatePathGeometry(&pPathGeometry);
//
//	ID2D1GeometrySink* pGeoSink = nullptr;
//	hr = pPathGeometry->Open(&pGeoSink);
//
//	formatIt->pFontFace->GetGlyphIndicesW(&codePoints[0], codePoints.size(), &glyphIndices[0]);
//	formatIt->pFontFace->GetGlyphRunOutline(SubtitleCoreUtilities::ConvertFontPointToDIP(m_SubCoreConfig.m_FontSize),
//											&glyphIndices[0],
//											NULL,
//											NULL,
//											glyphIndices.size(),
//											false,
//											false,
//											pGeoSink);
//	hr = pGeoSink->Close();
//
//	//hr = pPathGeometry->Widen(static_cast<float>(m_SubCoreConfig.m_FontBorderWidth), NULL, NULL, pGeoSink);
//
//	SafeRelease(&pGeoSink);
//	pPathGeometries.push_back(pPathGeometry);
//}

//HRESULT hr = S_OK;
//IDWriteFontCollection* pFontCollection = nullptr;
//hr = m_pDWriteFactory->GetSystemFontCollection(&pFontCollection);
//
//UINT32 fontIndex;
//BOOL fontExists;
//hr = pFontCollection->FindFamilyName(m_SubCoreConfig.m_FontName.c_str(), &fontIndex, &fontExists);
//
//IDWriteFontFamily* pFontFamily = nullptr;
//hr = pFontCollection->GetFontFamily(fontIndex, &pFontFamily);
//
//IDWriteFont* pFont = nullptr;
//hr = pFontFamily->GetFirstMatchingFont(trf.Weight, m_SubCoreConfig.m_FontStretch, trf.Style,
//										&pFont);
//
//hr = pFont->CreateFontFace(&(trf.pFontFace));
//
//SafeRelease(&pFont);
//
//SafeRelease(&pFontFamily);
//SafeRelease(&pFontCollection);

//for (auto it = pPathGeometries.begin(); it != pPathGeometries.end(); ++it)
//{
//	D2D1_RECT_F rect;
//	(*it)->GetBounds(NULL, &rect);
//	m_pRT->SetTransform(D2D1::Matrix3x2F::Translation(origin.x + metrics.left, origin.y + metrics.top));
//	m_pRT->DrawGeometry(*it, m_pOutlineColorBrush, 1.0f);//static_cast<float>(m_SubCoreConfig.m_FontBorderWidth));
//	(*it)->Release();
//}