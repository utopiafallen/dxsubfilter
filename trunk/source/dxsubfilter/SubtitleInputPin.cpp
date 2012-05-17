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

STDMETHODIMP CSubtitleInputPin::Receive(IMediaSample* pSample)
{
	HRESULT hr;
	BYTE* pBufferIn;
	REFERENCE_TIME sampleStartTime, sampleEndTime;
	long lBufferLength;

	hr = pSample->GetMediaTime(&sampleStartTime, &sampleEndTime);
	if (FAILED(hr))
	{
		return hr;
	}

	hr = pSample->GetPointer(&pBufferIn);
	if (FAILED(hr))
	{
		return hr;
	}

	lBufferLength = pSample->GetActualDataLength();

	// The sample should just be a single line of subtitle data
	std::string s(reinterpret_cast<char*>(pBufferIn));

	return S_OK;
}