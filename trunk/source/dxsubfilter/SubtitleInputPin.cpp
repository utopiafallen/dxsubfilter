#include "stdafx.h"
#include "dxsubfilter_uuids.h"
#include "SubtitleInputPin.h"

#include "dxsubfilter.h"

using namespace DXSubFilter;

CSubtitleInputPin::CSubtitleInputPin(LPCWSTR pObjectName, CDXSubFilter *pTransformFilter,
	HRESULT * phr, LPCWSTR pName) 
	: CTransformInputPin(pObjectName, pTransformFilter, phr, pName)
	, m_bExternalSubtitlesLoaded(false)
	, m_CurrentSubType(SubtitleCore::SBT_NONE)
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
	std::vector<wchar_t> wchData (numWChars, L'A');
	MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<char*>(pBufferIn), lBufferLength, &wchData[0], numWChars);
	
	// Always add null terminator.
	wchData[numWChars-1] = L'\0';

	// The sample should just be a single line of subtitle data
	std::wstring s(&wchData[0]);

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
