#include "stdafx.h"
#include "dxsubfilter_uuids.h"
#include "SubtitleInputPin.h"

using namespace DXSubFilter;

CSubtitleInputPin::CSubtitleInputPin(LPCWSTR pObjectName, CTransformFilter *pTransformFilter,
	HRESULT * phr, LPCWSTR pName) 
	: CTransformInputPin(pObjectName, pTransformFilter, phr, pName)
	, m_bExternalSubtitlesLoaded(false)
	, m_SubType(SubtitleCore::SBT_NONE)
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

HRESULT CSubtitleInputPin::BreakConnect()
{
	return CTransformInputPin::BreakConnect();
}

HRESULT CSubtitleInputPin::CompleteConnect(IPin *pReceivePin)
{
	return CTransformInputPin::CompleteConnect(pReceivePin);
}

STDMETHODIMP CSubtitleInputPin::EndOfStream()
{
	return S_OK;
}

STDMETHODIMP CSubtitleInputPin::Receive(IMediaSample* pSample)
{
	HRESULT hr;
	BYTE* pBufferIn;
	long lBufferLength;

	// Note to self: subtitle data is not timestamped.

	hr = pSample->GetPointer(&pBufferIn);
	if (FAILED(hr))
	{
		return hr;
	}

	lBufferLength = pSample->GetActualDataLength();

	// Append null terminator to character stream
	pBufferIn[lBufferLength] = '\0';

	// The sample should just be a single line of subtitle data
	std::wstring s(reinterpret_cast<wchar_t*>(pBufferIn));

	return S_OK;
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
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

				std::ifstream subtitleFile;
				subtitleFile.open(sNewFileName);
				if (subtitleFile.is_open())
				{
					m_bExternalSubtitlesLoaded = true;
					subtitleFile.close();
				}
			}
		}
	}
}
//------------------------------------------------------------------------------
