// dxsubfilter.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "dxsubfilter.h"
#include "SubtitleInputPin.h"

using namespace DXSubFilter;

// This is the constructor of a class that has been exported.
// see dxsubfilter.h for the class definition
CDXSubFilter::CDXSubFilter(LPUNKNOWN pUnk) 
	: CTransformFilter(DXSUBFILTER_NAME, pUnk, CLSID_DXSubFilter)
	, m_pInputSubtitlePin(nullptr)
{
	// Just in case the CTransformFilter constructor doesn't default these to null
	m_pInput = nullptr;
	m_pOutput = nullptr;
}

CDXSubFilter::~CDXSubFilter()
{
	// Do I need to manually delete m_pInput and m_pOutput?
	delete m_pInputSubtitlePin;
}

CUnknown* CDXSubFilter::CreateInstance(LPUNKNOWN pUnk, HRESULT* phr)
{
	return new CDXSubFilter(pUnk);
}

int CDXSubFilter::GetPinCount()
{
	return m_iPinCount;
}

CBasePin* CDXSubFilter::GetPin(int n)
{
	HRESULT hr = S_OK;
	switch (n)
	{
	case 0:
		if (!m_pInput)
		{
			m_pInput = new CTransformInputPin(L"Video", this, &hr, L"Video");

			if (FAILED(hr))
			{
				return nullptr;
			}
		}
		return m_pInput;
	case 1:
		if (!m_pOutput)
		{
			m_pOutput = new CTransformOutputPin(L"Output", this, &hr, L"Output");

			if (FAILED(hr))
			{
				return nullptr;
			}
		}
		return m_pOutput;
	case 2:
		if (!m_pInputSubtitlePin)
		{
			m_pInputSubtitlePin = new CSubtitleInputPin(L"Subtitle", this, &hr, L"Subtitle");

			if (FAILED(hr))
			{
				return nullptr;
			}
		}
		return m_pInputSubtitlePin;
	default:
		return nullptr;
	}
}

bool CDXSubFilter::CheckVideoSubtypeIs8Bit(const CMediaType* pMediaType)
{
	bool result = false;

	GUID subtype = pMediaType->subtype;
	for (size_t i = 0; i < DXSUBFILTER_SUPPORTED_VIDEO_SUBTYPES_8BIT_COUNT; i++)
	{
		if (subtype == DXSUBFILTER_SUPPORTED_VIDEO_SUBTYPES_8BIT[i])
		{
			result = true;
			break;
		}
	}

	return result;
}

bool CDXSubFilter::CheckVideoSubtypeIs16Bit(const CMediaType* pMediaType)
{
	bool result = false;

	GUID subtype = pMediaType->subtype;
	for (size_t i = 0; i < DXSUBFILTER_SUPPORTED_VIDEO_SUBTYPES_16BIT_COUNT; i++)
	{
		if (subtype == DXSUBFILTER_SUPPORTED_VIDEO_SUBTYPES_16BIT[i])
		{
			result = true;
			break;
		}
	}

	return result;
}

HRESULT CDXSubFilter::CheckInputType(const CMediaType* mtIn)
{
	// We only accept video. CSubtitleInputPin handles checking for subtitle data.
	if (mtIn->majortype == MEDIATYPE_Video)
	{
		// Check to see if the video subtype is one of the supported subtypes
		GUID subtype = mtIn->subtype;
		for (size_t i = 0; i < DXSUBFILTER_SUPPORTED_VIDEO_SUBTYPES_8BIT_COUNT; i++)
		{
			if (subtype == DXSUBFILTER_SUPPORTED_VIDEO_SUBTYPES_8BIT[i])
			{
				return S_OK;
			}
		}

		for (size_t i = 0; i < DXSUBFILTER_SUPPORTED_VIDEO_SUBTYPES_16BIT_COUNT; i++)
		{
			if (subtype == DXSUBFILTER_SUPPORTED_VIDEO_SUBTYPES_16BIT[i])
			{
				return S_OK;
			}
		}

		// Not a supported subtype so fail
		return VFW_E_TYPE_NOT_ACCEPTED;
	}
	else
	{
		return VFW_E_TYPE_NOT_ACCEPTED;
	}
}

HRESULT CDXSubFilter::CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut)
{
	// This should only be called by the video input/output pins. The subtitle input pin
	// handles everything internally.
	if (mtIn->majortype == MEDIATYPE_Video)
	{
		// Accept all combinations of our supported video formats for input/output. We will force
		// upstream to reconnect during the CompleteConnect call if the formats don't match.
		return S_OK;
	}
	else
	{
		return VFW_E_TYPE_NOT_ACCEPTED;
	}
}

HRESULT CDXSubFilter::CompleteConnect(PIN_DIRECTION direction, IPin* pReceivePin)
{
	if (direction == PINDIR_OUTPUT)
	{
		CMediaType mtOut = m_pOutput->CurrentMediaType();
		if (mtOut != m_pInput->CurrentMediaType())
		{
			// Force upstream to reconnect with the same proposed format as the output
			if (SUCCEEDED(m_pInput->QueryAccept(&mtOut)))
			{
				if (SUCCEEDED(ReconnectPin(m_pInput, &mtOut)))
				{
					m_pInput->SetMediaType(&mtOut);

					// Update the fact that we've reconnected the input on a new format
					m_InputVideoType = mtOut;

					return S_OK;
				}
				else
				{
					return VFW_E_TYPE_NOT_ACCEPTED;
				}
			}
			else
			{
				// If upstream doesn't accept the output format, much sadness occurs :(
				return VFW_E_TYPE_NOT_ACCEPTED;
			}
		}
		else
		{
			return S_OK;
		}
	}
	else
	{
		return S_OK;
	}
}

HRESULT CDXSubFilter::DecideBufferSize(IMemAllocator * pAllocator, ALLOCATOR_PROPERTIES *pProp)
{
	if (m_pInput->IsConnected() == false)
	{
		return E_UNEXPECTED;
	}
	else
	{
		// Our input and output formats should always match so we don't really need to do anything
		// special for the downstream allocator settings
		AM_MEDIA_TYPE mt;
		HRESULT hr = m_pOutput->ConnectionMediaType(&mt);
		if (FAILED(hr))
		{
			return hr;
		}

		// Check to make sure input and output types match. This check should never fail, but who knows...
		if (mt.subtype != m_InputVideoType.subtype)
		{
			hr = E_FAIL;
		}
		else
		{
			// Set desired allocator properties
		
			// Size in bytes of each buffer
			ASSERT(mt.formattype == FORMAT_VideoInfo2);
			pProp->cbBuffer = DIBSIZE(reinterpret_cast<VIDEOINFOHEADER2*>(mt.pbFormat)->bmiHeader);
		
			// Number of buffers
			if (pProp->cBuffers == 0)
			{
				pProp->cBuffers = 1;
			}

			ALLOCATOR_PROPERTIES Actual;
			hr = pAllocator->SetProperties(pProp, &Actual);
			if (FAILED(hr))
			{
				return hr;
			}

			// Check to make sure the actual results match the desired results
			if (pProp->cbBuffer > Actual.cbBuffer)
			{
				hr = E_FAIL;
			}
			else
			{
				hr = S_OK;
			}
		}
		FreeMediaType(mt);

		return hr;
	}
}

HRESULT CDXSubFilter::GetMediaType(int iPosition, CMediaType *pMediaType)
{
	HRESULT hr = S_OK;

	if (iPosition < 0)
	{
		hr = E_INVALIDARG;
	}
	else if (m_pInput->IsConnected() == TRUE)
	{
		// Expose all the supported formats. During CompleteConnect, we will force upstream to 
		// reconnect with the format chosen by downstream since most video decoders are able to
		// output all the inputs we accept, but not all video renderers are able to accept all
		// the formats we accept. This is basically for VMR/EVR compatibility, which rely on 
		// Direct3D video surfaces, which is dependant upon GPU.
		pMediaType->SetType(&MEDIATYPE_Video);

		// Try input format first
		if (iPosition == 0)
		{
			*pMediaType = m_InputVideoType;
		}
		else
		{
			// Subtract 1 because 0 is used to return our stored input media type
			int index = iPosition - 1; 

			// If we have 10/16-bit input, expose those as output first.
			if (CheckVideoSubtypeIs16Bit(&m_InputVideoType))
			{
				if (index < DXSUBFILTER_SUPPORTED_VIDEO_SUBTYPES_16BIT_COUNT)
				{
					pMediaType->subtype = DXSUBFILTER_SUPPORTED_VIDEO_SUBTYPES_16BIT[index];
				}
				else
				{
					// Gone through all the 10/16-bit formats, now check 8-bit
					index = index - DXSUBFILTER_SUPPORTED_VIDEO_SUBTYPES_16BIT_COUNT;

					if (index < DXSUBFILTER_SUPPORTED_VIDEO_SUBTYPES_8BIT_COUNT)
					{
						pMediaType->subtype = DXSUBFILTER_SUPPORTED_VIDEO_SUBTYPES_8BIT[index];
					}
					else
					{
						// Out of items
						hr = VFW_S_NO_MORE_ITEMS;
					}
				}
			}
			else	// 8-bit input
			{
				if (index < DXSUBFILTER_SUPPORTED_VIDEO_SUBTYPES_8BIT_COUNT)
				{
					pMediaType->subtype = DXSUBFILTER_SUPPORTED_VIDEO_SUBTYPES_8BIT[index];
				}
				else
				{
					// Out of items
					hr = VFW_S_NO_MORE_ITEMS;
				}
			}
		}
	}
	else
	{
		hr = VFW_S_NO_MORE_ITEMS;
	}

	return hr;
}

HRESULT CDXSubFilter::SetMediaType(PIN_DIRECTION direction,const CMediaType *pmt)
{
	// Call base class SetMediaType first and see if everything's ok
	HRESULT hr = CTransformFilter::SetMediaType(direction, pmt);
	if (FAILED(hr))
	{
		return hr;
	}
	else
	{
		// Save the input media type from video decoder
		if (direction == PINDIR_INPUT && pmt->majortype == MEDIATYPE_Video)
		{
			m_InputVideoType = *pmt;
		}
		return S_OK;
	}
}

HRESULT CDXSubFilter::Transform(IMediaSample * pIn, IMediaSample *pOut)
{
	BYTE* pBufferIn, *pBufferOut;
	long lBufferLength;
	HRESULT hr;

	// Get input buffer and size
	hr = pIn->GetPointer(&pBufferIn);
	if (FAILED(hr))
	{
		return hr;
	}
	lBufferLength = pIn->GetActualDataLength();

	// Get output buffer
	hr = pOut->GetPointer(&pBufferOut);
	if (FAILED(hr))
	{
		return hr;
	}

	// Copy data to output buffer. CTransformFilter will have already allocated pOut using the
	// downstream filter's memory allocator so we SHOULD be able to just copy the data
	memcpy(pBufferOut, pBufferIn, lBufferLength);

	pOut->SetActualDataLength(lBufferLength);

	// Get subtitle data and overlay onto video frame. Or maybe pass in raw video data into 
	// subtitle rendering core and let it do the overlaying? We'll see... (NB: Use output buffer
	// video data)

	return S_OK;
}

