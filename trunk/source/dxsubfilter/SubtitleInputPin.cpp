#include "stdafx.h"
#include "dxsubfilter_uuids.h"
#include "SubtitleInputPin.h"

using namespace DXSubFilter;

CSubtitleInputPin::CSubtitleInputPin(LPCWSTR pObjectName, CTransformFilter *pTransformFilter,
	HRESULT * phr, LPCWSTR pName) : CTransformInputPin(pObjectName, pTransformFilter, phr, pName)
{

}

CSubtitleInputPin::~CSubtitleInputPin()
{

}

HRESULT CSubtitleInputPin::CheckMediaType(const CMediaType* mtIn)
{
	// Check the input type. We reject if it's anything other than subtitle data
    HRESULT hr = S_OK;

	if (mtIn->majortype != MEDIATYPE_Subtitle)
	{
		hr = S_FALSE;
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
	std::string s(reinterpret_cast<char*>(pBufferIn));

	return S_OK;
}