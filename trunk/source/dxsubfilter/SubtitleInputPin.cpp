#include "stdafx.h"
#include "dxsubfilter_uuids.h"
#include "SubtitleInputPin.h"
#include "SubtitleRendererFactory.h"

#include "dxsubfilter.h"

using namespace DXSubFilter;

CSubtitleInputPin::CSubtitleInputPin(LPCWSTR pObjectName, CDXSubFilter *pTransformFilter,
	HRESULT * phr, LPCWSTR pName) 
	: CTransformInputPin(pObjectName, pTransformFilter, phr, pName)
	, m_bExternalSubtitlesLoaded(false)
	, m_CurrentSubtitleType(SubtitleCore::SBT_NONE)
	, m_SubtitleRenderer(nullptr)
{

}

CSubtitleInputPin::~CSubtitleInputPin()
{
}

//------------------------------------------------------------------------------
// DirectShow related overrides

HRESULT CSubtitleInputPin::CheckMediaType(const CMediaType* mtIn)
{
	// Check the input type. We reject if it's anything other than subtitle data.
	// We also reject if external subtitles have been loaded to prevent upstream from
	// connecting to us.
	HRESULT hr = S_FALSE;

	if (m_bExternalSubtitlesLoaded == false)
	{
		if (mtIn->majortype == MEDIATYPE_Subtitle ||
			mtIn->majortype == MEDIATYPE_Text)
		{
			hr = S_OK;
		}
	}
	return hr;
}

HRESULT CSubtitleInputPin::SetMediaType(const CMediaType* mtIn)
{
	// Set the base class media type (should always succeed)
	HRESULT hr = CBasePin::SetMediaType(mtIn);
	if (FAILED(hr)) {
		return hr;
	}

	return m_pTransformFilter->SetMediaType(PINDIR_INPUT,mtIn);
}

STDMETHODIMP CSubtitleInputPin::Disconnect()
{
	m_CurrentSubtitleType = SubtitleCore::SBT_NONE;
	return CTransformInputPin::Disconnect();
}

HRESULT CSubtitleInputPin::CompleteConnect(IPin *pReceivePin)
{
	m_CurrentSubtitleType = MapMediaTypeToSubtitleType(m_mt);
	
	return CTransformInputPin::CompleteConnect(pReceivePin);
}

STDMETHODIMP CSubtitleInputPin::BeginFlush()
{
	return CBaseInputPin::BeginFlush();
}

STDMETHODIMP CSubtitleInputPin::EndFlush()
{
	return CBaseInputPin::EndFlush();
}

STDMETHODIMP CSubtitleInputPin::EndOfStream()
{
	//return CTransformInputPin::EndOfStream();
	return S_OK;
}

STDMETHODIMP CSubtitleInputPin::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
	m_SubtitleRenderer->Invalidate();
	return CBasePin::NewSegment(tStart, tStop, dRate);
}

STDMETHODIMP CSubtitleInputPin::Receive(IMediaSample* pSample)
{
	HRESULT hr;
	BYTE* pBufferIn;
	long lBufferLength, lBufferSize;

	// Note to self: subtitle data is not timestamped.

	// Check everything is ok with BaseInputPin first
	hr = CBaseInputPin::Receive(pSample);
	if (FAILED(hr))
	{
		return hr;
	}

	// Get pointer to subtitle data
	hr = pSample->GetPointer(&pBufferIn);
	if (FAILED(hr))
	{
		return hr;
	}

	lBufferLength = pSample->GetActualDataLength();
	lBufferSize = pSample->GetSize();

	// The data should only be interpreted as a string for text-based subtitle formats.
	if (m_CurrentSubtitleType == SubtitleCore::SBT_ASS || 
		m_CurrentSubtitleType == SubtitleCore::SBT_SSA ||
		m_CurrentSubtitleType == SubtitleCore::SBT_SRT)
	{
		// Append null terminator to character stream if we can.
		// Convert UTF-8 to UTF-16 since using MBCS is kind of ugly. This is actually kind of confusing
		// so this may need to be revisited
		int numWChars = 0;
		if (lBufferSize > lBufferLength)
		{
			pBufferIn[lBufferLength] = '\0';
			lBufferLength += 1;

			numWChars = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<char*>(pBufferIn), lBufferLength, NULL, 0);
		}
		else
		{
			// Couldn't add null terminator so account for that by adding 1 to required buffer size.
			numWChars = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<char*>(pBufferIn), lBufferLength, NULL, 0) + 1;
		}

		// Initialize a vector to numWChars of junk data that will get overwritten by MultiByteToWideChar
		std::vector<wchar_t> wchData(numWChars, L'A');
		MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<char*>(pBufferIn), lBufferLength, &wchData[0], numWChars);
	
		// Always add null terminator.
		wchData[numWChars-1] = L'\0';

		// The sample should just be a single line of subtitle data
		std::wstring line(&wchData[0]);

		REFERENCE_TIME rtStart, rtEnd;
		hr = pSample->GetTime(&rtStart, &rtEnd);

		rtStart += m_tStart;
		rtEnd += m_tStart;
		m_SubtitleRenderer->ParseLine(line, rtStart, rtEnd);
	}

	return S_OK;
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// CSubtitleInputPin functions
void CSubtitleInputPin::LoadSubtitleRenderer(SubtitleCore::VideoInfo& targetVideoInfo)
{
	m_SubtitleRenderer = SubtitleCore::SubtitleRendererFactory::GetSingleton()->CreateSubtitleRenderer(m_CurrentSubtitleType, targetVideoInfo);

	if (m_bExternalSubtitlesLoaded)
	{
		m_SubtitleRenderer->ParseScript(m_ExternalSubtitleScript);
	}
}

void CSubtitleInputPin::LoadExternalSubtitles()
{
	IFileSourceFilter* pFileSource = nullptr;

	IFilterGraph* pGraph = m_pTransformFilter->GetFilterGraph();

	// Get filter enumerator
	IEnumFilters* pEnum = nullptr;
	IBaseFilter* pFilter = nullptr;

	// Enumerate all filters and query for IFileSourceFilter
	if (SUCCEEDED(pGraph->EnumFilters(&pEnum)))
	{
		HRESULT hr = E_FAIL;
		while (S_OK == pEnum->Next(1, &pFilter, 0))
		{
			hr = pFilter->QueryInterface(IID_IFileSourceFilter, 
										reinterpret_cast<void**>(&pFileSource));
			pFilter->Release();
			if (SUCCEEDED(hr))
			{
				break;
			}
		}
		pEnum->Release();

		// We found the file source filter so get current file name
		if (hr == S_OK)
		{
			LPOLESTR pszFileName = nullptr;

			pFileSource->GetCurFile(&pszFileName, NULL);
			pFileSource->Release();

			std::wstring sFileName(pszFileName);

			// Strip filename extension
			size_t periodPosition = sFileName.find_last_of(L".");
			std::wstring sFileNameNoExt = sFileName.substr(0, periodPosition);

			// See if we can find any external subtitles. We only attempt to match
			// root filename + subtitle extension.
			for (size_t i = 0; i < SubtitleCore::SubtitleFileExtensionsCount; i++)
			{
				std::wstring sNewFileName = sFileNameNoExt + 
											SubtitleCore::SubtitleFileExtensions[i];

				std::wifstream subtitleFile;
				subtitleFile.open(sNewFileName);
				if (subtitleFile.is_open())
				{
					m_bExternalSubtitlesLoaded = true;
					m_CurrentSubtitleType = MapFileExtToSubtitleType(SubtitleCore::SubtitleFileExtensions[i]);

					// Load subtitle data
					std::wstring line;

					while(std::getline(subtitleFile, line).eof() == false)
					{
						m_ExternalSubtitleScript.push_back(line);
					}
					m_ExternalSubtitleScript.shrink_to_fit();

					subtitleFile.close();

					// If we found VOBSUB subtitles, we also need to load the accompanying .sub file
					if (m_CurrentSubtitleType == SubtitleCore::SBT_VOBSUB)
					{
						sNewFileName = sFileNameNoExt + L".sub";
						
						subtitleFile.open(sNewFileName);

						if (subtitleFile.is_open())
						{
							subtitleFile.close();
						}
						else
						{
							// We failed to load accompanying .sub file so reset the fact that we
							// opened external subtitles.
							m_bExternalSubtitlesLoaded = false;
						}
					}
				}
			}
		}
	}
}

SubtitleCore::SubtitleType CSubtitleInputPin::MapFileExtToSubtitleType(const std::wstring& fileExt) const
{
	if (fileExt.compare(L".ass") == 0)
	{
		return SubtitleCore::SBT_ASS;
	}
	else if (fileExt.compare(L".srt") == 0)
	{
		return SubtitleCore::SBT_SRT;
	}
	else if (fileExt.compare(L".ssa") == 0)
	{
		return SubtitleCore::SBT_SSA;
	}
	else if (fileExt.compare(L".idx") == 0)
	{
		return SubtitleCore::SBT_VOBSUB;
	}
	else
	{
		return SubtitleCore::SBT_NONE;
	}
}

SubtitleCore::SubtitleType CSubtitleInputPin::MapMediaTypeToSubtitleType(const CMediaType& mt) const
{
	if (mt.subtype == MEDIASUBTYPE_ASS)
	{
		return SubtitleCore::SBT_ASS;
	}
	else if (mt.subtype == MEDIASUBTYPE_UTF8)
	{
		return SubtitleCore::SBT_SRT;
	}
	else if (mt.subtype == MEDIASUBTYPE_SSA)
	{
		return SubtitleCore::SBT_SSA;
	}
	else if (mt.subtype == MEDIASUBTYPE_VOBSUB)
	{
		return SubtitleCore::SBT_VOBSUB;
	}
	else
	{
		return SubtitleCore::SBT_NONE;
	}
}
//------------------------------------------------------------------------------
