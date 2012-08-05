// DirectShow filter for SubtitleCore
//

#include "stdafx.h"
#include "dxsubfilter.h"
#include "SubtitleInputPin.h"
#include "SubtitleRendererFactory.h"
#include "SubPicBlender.h"

using namespace DXSubFilter;

CDXSubFilter::CDXSubFilter(LPUNKNOWN pUnk) 
	: CTransformFilter(DXSUBFILTER_NAME, pUnk, CLSID_DXSubFilter)
	, m_SubCoreConfigData(g_SubtitleCoreConfigData)
	, m_pInputSubtitlePin(nullptr)
	, m_pAlignedBuffer(nullptr)
	, m_pSubPicBlender(nullptr)
	, m_uAlignedBufferLength(0)
	, m_InputStrideY(0)
	, m_InputStrideUV(0)
	, m_OutputStrideUV(0)
	, m_OutputStrideY(0)
	, m_rtStart(0)
	, m_rtEnd(0)
	, m_dPlaybackRate(1.0)
{
	// Just in case the CTransformFilter constructor doesn't default these to null
	m_pInput = nullptr;
	m_pOutput = nullptr;

	// Update SubtitleRendererFactory with new configuration data read in from registry
	SubtitleCore::SubtitleRendererFactory::GetSingleton()->SetSubtitleCoreConfig(m_SubCoreConfigData);
}

CDXSubFilter::~CDXSubFilter()
{
	delete m_pInputSubtitlePin;
	
	if (m_pAlignedBuffer)
	{
		_aligned_free(m_pAlignedBuffer);
		m_pAlignedBuffer = nullptr;
	}
}

//------------------------------------------------------------------------------
// DirectShow related overrides

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

			// Attempt to load external subtitles first before upstream tries to connect to the
			// subtitle input pin.
			m_pInputSubtitlePin->LoadExternalSubtitles();
		}
		return m_pInputSubtitlePin;
	default:
		return nullptr;
	}
}

HRESULT CDXSubFilter::CheckInputType(const CMediaType* mtIn)
{
	// We only accept video. CSubtitleInputPin handles checking for subtitle data.
	if (mtIn->majortype == MEDIATYPE_Video)
	{
		// Reject connection if no external subtitles or embedded subtitles are found so we don't
		// get added to the graph and do nothing.
		if (m_pInputSubtitlePin->IsExternalSubtitlesLoaded() || CheckForEmbeddedSubtitles())
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
			// No subtitles found so reject connection.
			return VFW_E_TYPE_NOT_ACCEPTED;
		}
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

					ComputeStrides();

					if (m_pAlignedBuffer)
					{
						_aligned_free(m_pAlignedBuffer);
						m_pAlignedBuffer = nullptr;
					}

					// Video sample size shouldn't ever change
					m_pAlignedBuffer = static_cast<BYTE*>(_aligned_malloc(m_InputVideoType.lSampleSize, 16));
					m_uAlignedBufferLength = m_InputVideoType.lSampleSize;

					AssignSubPicBlender();

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
			// Input matches output so we're good to go
			ComputeStrides();

			if (m_pAlignedBuffer)
			{
				_aligned_free(m_pAlignedBuffer);
				m_pAlignedBuffer = nullptr;
			}

			// Video sample size shouldn't ever change
			m_pAlignedBuffer = static_cast<BYTE*>(_aligned_malloc(m_InputVideoType.lSampleSize, 16));
			m_uAlignedBufferLength = m_InputVideoType.lSampleSize;

			AssignSubPicBlender();

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
			// Copy settings from input
			*pMediaType = m_InputVideoType;

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

			// Make sure media type is still coherent
			CorrectVideoMediaType(pMediaType);
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

HRESULT CDXSubFilter::NewSegment(
						REFERENCE_TIME tStart,
						REFERENCE_TIME tStop,
						double dRate)
{
	m_rtStart = tStart;
	m_rtEnd = tStop;
	m_dPlaybackRate = dRate;

	return CTransformFilter::NewSegment(tStart, tStop, dRate);
}

HRESULT CDXSubFilter::Transform(IMediaSample * pIn, IMediaSample *pOut)
{
	HRESULT hr;

	REFERENCE_TIME rtNow;
	BYTE* pBufferIn, *pBufferOut;
	long lBufferLength;

	// Check for a format change. If the actual video type is changed during playback, we're
	// totally screwed because we can't handle that. However, it's possible that the stride
	// of the allocated sample changed, in which case we need to account for that.
	AM_MEDIA_TYPE* pmt = nullptr;
	hr = pOut->GetMediaType(&pmt);
	if (hr == S_OK)
	{
		CMediaType mt(*pmt);
		m_pOutput->SetMediaType(&mt);
		DeleteMediaType(pmt);

		ComputeStrides();
	}

	hr = pIn->GetMediaType(&pmt);
	if (hr == S_OK)
	{
		CMediaType mt(*pmt);
		m_pInput->SetMediaType(&mt);
		DeleteMediaType(pmt);

		ComputeStrides();
	}

	ASSERT(m_pInput->CurrentMediaType().subtype == m_pOutput->CurrentMediaType().subtype);

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

	REFERENCE_TIME rtStart, rtEnd;
	UNREFERENCED_PARAMETER(rtEnd);
	hr = pIn->GetTime(&rtStart, &rtEnd);

	// Get current playback time
	rtNow = CalcCurrentTime(rtStart);

	// Check that input sample size is the same as when we first connected. This shouldn't have
	// changed.
	ASSERT(static_cast<size_t>(lBufferLength) == m_uAlignedBufferLength);

	// Copy data into an aligned buffer for easier SSE processing
	memcpy(m_pAlignedBuffer, pBufferIn, lBufferLength);

	// NOTE TO SELF: Handle VFR video? That'll probably fuck with animation.

	// Get subtitle data and overlay onto video frame. Or maybe pass in raw video data into 
	// subtitle rendering core and let it do the overlaying? We'll see...
	size_t subpicCount = m_pInputSubtitlePin->m_SubtitleRenderer->GetSubtitlePictureCount(rtNow);
	if (subpicCount > 0)
	{
		std::vector<SubtitleCore::SubtitlePicture*> subpics(subpicCount, nullptr);
		m_pInputSubtitlePin->m_SubtitleRenderer->GetSubtitlePicture(rtNow, &subpics[0]);

		for (auto it = subpics.begin(), itEnd = subpics.end(); it != itEnd; ++it)
		{
			(*m_pSubPicBlender)(*it, m_pAlignedBuffer, m_uInputVideoWidth, m_uInputVideoHeight);
		}
	}

	// Copy buffer to output
	CopyBuffer(m_pAlignedBuffer, pBufferOut, lBufferLength);

	pOut->SetActualDataLength(lBufferLength);
	pOut->SetDiscontinuity(pIn->IsDiscontinuity());

	return S_OK;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// These are all the non-DirectShow related functions

REFERENCE_TIME CDXSubFilter::CalcCurrentTime(REFERENCE_TIME rtOffset) const
{
	return m_rtStart + rtOffset;
}

bool CDXSubFilter::CheckVideoSubtypeIs8Bit(const CMediaType* pMediaType) const
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

bool CDXSubFilter::CheckVideoSubtypeIs16Bit(const CMediaType* pMediaType) const
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

bool CDXSubFilter::CheckForEmbeddedSubtitles() const
{
	bool bFoundEmbeddedSubtitles = false;

	// Get filter enumerator
	IEnumFilters* pEnum = nullptr;
	IBaseFilter* pFilter = nullptr;

	// Search for splitter
	if (SUCCEEDED(m_pGraph->EnumFilters(&pEnum)))
	{
		HRESULT hr = E_FAIL;
		while (S_OK == pEnum->Next(1, &pFilter, 0))
		{
			IAMStreamSelect* pStreamSelect = nullptr;
			hr = pFilter->QueryInterface(IID_IAMStreamSelect, 
										reinterpret_cast<void**>(&pStreamSelect));

			// If we found the splitter, don't release its filter pointer until we're done
			if (SUCCEEDED(hr))
			{
				// We don't actually care about the IAMStreamSelect interface pointer itself, we
				// just use it to verify that we got a splitter so release it.
				pStreamSelect->Release();
				break;
			}
			else
			{
				pFilter->Release();
			}
		}
		pEnum->Release();

		// Enumerate the pins on the splitter
		if (hr == S_OK)
		{
			IEnumPins* pEnumPins = nullptr;
			IPin* pPin = nullptr;

			hr = pFilter->EnumPins(&pEnumPins);
			if (SUCCEEDED(hr))
			{
				IEnumMediaTypes* pEnumMT = nullptr;

				while (S_OK == pEnumPins->Next(1, &pPin, 0))
				{
					hr = pPin->EnumMediaTypes(&pEnumMT);

					// Enumerate the preferred media types of this pin
					if (SUCCEEDED(hr))
					{
						AM_MEDIA_TYPE* pMediaType = nullptr;

						// Enumerator will allocate AM_MEDIA_TYPE structure for us so we need
						// to manually delete it.
						while (S_OK == pEnumMT->Next(1, &pMediaType, 0))
						{
							if (pMediaType->majortype == MEDIATYPE_Text ||
								pMediaType->majortype == MEDIATYPE_Subtitle)
							{
								bFoundEmbeddedSubtitles = true;
							}
							DeleteMediaType(pMediaType);

							if (bFoundEmbeddedSubtitles)
							{
								break;
							}
						}
						
						pEnumMT->Release();

						if (bFoundEmbeddedSubtitles)
						{
							break;
						}
					}
				}
				pEnumPins->Release();
			}
			pFilter->Release();
		}
	}
	return bFoundEmbeddedSubtitles;
}

void CDXSubFilter::CopyBuffer(BYTE* pBufferIn, BYTE* pBufferOut, size_t srcActualDataLength) const
{
	size_t inputRows;

	BITMAPINFOHEADER& bmiIn = reinterpret_cast<VIDEOINFOHEADER2*>(m_InputVideoType.pbFormat)->bmiHeader;

	// Compute input and output strides
	bool b16BitVideo = CheckVideoSubtypeIs16Bit(&m_InputVideoType);
	if (b16BitVideo)
	{
		// All the PXXX formats have Y on its own plane and then UV interleaved in the same plane
		// (which means the UV plane has the same stride as Y plane) so all we need to do is to
		// copy all the rows over while accounting for output stride padding.
		inputRows = srcActualDataLength / m_InputStrideY;

		for (size_t i = 0; i < inputRows; i++)
		{
			BYTE* pDest = pBufferOut + m_OutputStrideY * i;
			BYTE* pSrc = pBufferIn + m_InputStrideY * i;

			memcpy(pDest, pSrc, m_InputStrideY);
		}
	}
	else
	{
		// Note to self: Leaving in parallel_for memcpy in case we want to use it later

		// Copy the Y plane. For the packed formats, this will copy all the data we need.
		for (LONG i = 0; i < bmiIn.biHeight; i++)
		{
			BYTE* pDest = pBufferOut + m_OutputStrideY * i;
			BYTE* pSrc = pBufferIn + m_InputStrideY * i;

			memcpy(pDest, pSrc, m_InputStrideY);
		}

		//Concurrency::parallel_for(0L, bmiIn.biHeight, [&](LONG i){
		//	BYTE* pDest = pBufferOut + m_OutputStrideY * i;
		//	BYTE* pSrc = pBufferIn + m_InputStrideY * i;

		//	memcpy(pDest, pSrc, m_InputStrideY);
		//});

		inputRows = (srcActualDataLength - (m_InputStrideY * bmiIn.biHeight)) / m_InputStrideUV;

		ptrdiff_t dstOffset = m_OutputStrideY * bmiIn.biHeight;
		ptrdiff_t srcOffset = m_InputStrideY * bmiIn.biHeight;

		// For the packed formats, this for loop won't execute because inputRows will be 0.
		// For planar formats, this will copy the remaining U and V planes.
		for (size_t i = 0; i < inputRows; i++)
		{
			BYTE* pDest = pBufferOut + dstOffset + m_OutputStrideUV * i;
			BYTE* pSrc = pBufferIn + srcOffset + m_InputStrideUV * i;

			memcpy(pDest, pSrc, m_InputStrideUV);
		}

		//Concurrency::parallel_for(0U, inputRows, [&](size_t i){
		//	BYTE* pDest = pBufferOut + dstOffset + m_OutputStrideUV * i;
		//	BYTE* pSrc = pBufferIn + srcOffset + m_InputStrideUV * i;

		//	memcpy(pDest, pSrc, m_InputStrideUV);
		//});
	}
}

void CDXSubFilter::ExtractYUV(BYTE* pBufferIn, BYTE* pPlanes[3]) const
{
	//TODO: Finish this
}

void CDXSubFilter::ComputeStrides()
{
	// Default UV stride to 1 so that we don't divide by 0 in CopyBuffer.
	m_InputStrideUV = 1;
	m_OutputStrideUV = 1;

	CMediaType& mtOut = m_pOutput->CurrentMediaType();
	BITMAPINFOHEADER& bmiOut = reinterpret_cast<VIDEOINFOHEADER2*>(mtOut.pbFormat)->bmiHeader;

	BITMAPINFOHEADER& bmiIn = reinterpret_cast<VIDEOINFOHEADER2*>(m_InputVideoType.pbFormat)->bmiHeader;

	m_uInputVideoWidth = bmiIn.biWidth;
	m_uInputVideoHeight = bmiIn.biHeight;

	// Compute input and output strides
	bool b16BitVideo = CheckVideoSubtypeIs16Bit(&m_InputVideoType);
	if (b16BitVideo)
	{
		// 2 bytes per pixel. We don't support any packed 10/16-bit formats so this is always true.
		m_InputStrideY = bmiIn.biWidth * 2;
		m_OutputStrideY = bmiOut.biWidth * 2;

		// Since UV is interleaved in the same plane, it has the same stride as the Y plane
		m_InputStrideUV = m_InputStrideY;
		m_OutputStrideUV = m_OutputStrideY;
	}
	else
	{
		if (mtOut.subtype == MEDIASUBTYPE_AYUV)
		{
			// Packed 8-bit 4:4:4
			m_InputStrideY = bmiIn.biWidth * 4;
			m_OutputStrideY = bmiOut.biWidth * 4;
		}
		else if (mtOut.subtype == MEDIASUBTYPE_YUY2)
		{
			// Packed 4:2:2
			m_InputStrideY = bmiIn.biWidth * 2;
			m_OutputStrideY = bmiOut.biWidth * 2;
		}
		else if (mtOut.subtype == MEDIASUBTYPE_YV12)
		{
			// Note to self: YV12 is 12bpp because each UV value corresponds to color data for 2x2 pixel
			// block so each U and V value contribute 1/4 the data to a single pixel i.e. 2-bits from
			// U and V for a total of 4-bits of data for a pixel. Y is full-res at 8-bits so total bit
			// depth is 12-bits.

			// Planar 4:2:0
			m_InputStrideY = bmiIn.biWidth;
			m_OutputStrideY = bmiOut.biWidth;

			// UV stride is half of Y stride
			m_InputStrideUV = m_InputStrideY / 2;
			m_OutputStrideUV = m_OutputStrideY / 2;
		}
		else if(mtOut.subtype == MEDIASUBTYPE_NV12)
		{
			// Planar 4:2:0
			m_InputStrideY = bmiIn.biWidth;
			m_OutputStrideY = bmiOut.biWidth;

			// UV is interleaved into a single plan so it shares the same stride as Y
			m_InputStrideUV = m_InputStrideY;
			m_OutputStrideUV = m_OutputStrideY;
		}
		else
		{
			// Should never reach here
		}
	}

	// Update SubtitleRendererFactory with new video info
	SubtitleCore::VideoInfo vidInfo;
	vidInfo.Height = bmiIn.biHeight;
	vidInfo.Width = bmiIn.biWidth;
	SubtitleCore::SubtitleRendererFactory::GetSingleton()->SetVideoInfo(vidInfo);
}

void CDXSubFilter::CorrectVideoMediaType(CMediaType* pMediaType) const
{
	// Recompute certain parameters of MediaType
	BITMAPINFOHEADER& bmiIn = reinterpret_cast<VIDEOINFOHEADER2*>(pMediaType->pbFormat)->bmiHeader;

	const GUID subtype = pMediaType->subtype;
	if (subtype == MEDIASUBTYPE_P010)
	{
		bmiIn.biCompression = '010P';
		bmiIn.biBitCount = 24;
		bmiIn.biSizeImage = (bmiIn.biHeight * bmiIn.biWidth * 3);
	}
	else if (subtype == MEDIASUBTYPE_P016)
	{
		bmiIn.biCompression = '610P';
		bmiIn.biBitCount = 24;
		bmiIn.biSizeImage = (bmiIn.biHeight * bmiIn.biWidth * 3);
	}
	else if (subtype == MEDIASUBTYPE_P210)
	{
		bmiIn.biCompression = '012P';
		bmiIn.biBitCount = 24;
		bmiIn.biSizeImage = (bmiIn.biHeight * bmiIn.biWidth * 4);
	}
	else if (subtype == MEDIASUBTYPE_P216)
	{
		bmiIn.biCompression = '612P';
		bmiIn.biBitCount = 24;
		bmiIn.biSizeImage = (bmiIn.biHeight * bmiIn.biWidth * 4);
	}
	else if (subtype == MEDIASUBTYPE_NV12)
	{
		bmiIn.biCompression = '21VN';
		bmiIn.biBitCount = 12;
		bmiIn.biSizeImage = (bmiIn.biHeight * bmiIn.biWidth * 3) / 2;
	}
	else if (subtype == MEDIASUBTYPE_YV12)
	{
		bmiIn.biCompression = '21VY';
		bmiIn.biBitCount = 12;
		bmiIn.biSizeImage = (bmiIn.biHeight * bmiIn.biWidth * 3) / 2;
	}
	else if (subtype == MEDIASUBTYPE_YUY2)
	{
		// No horizontal downsampling and 2:1 vertical downsampling so
		// U and V are both H/2 so added together take up H memory
		bmiIn.biCompression = '2YUY';
		bmiIn.biBitCount = 16;
		bmiIn.biSizeImage = (bmiIn.biHeight * bmiIn.biWidth * 2);
	}
	else if (subtype == MEDIASUBTYPE_AYUV)
	{
		bmiIn.biCompression = 'VUYA';
		bmiIn.biBitCount = 32;
		bmiIn.biSizeImage = (bmiIn.biHeight * bmiIn.biWidth * 4);
	}
	else
	{
		// Should never reach here
	}

	pMediaType->lSampleSize = bmiIn.biSizeImage;
}

void CDXSubFilter::AssignSubPicBlender()
{
	if (m_InputVideoType.subtype == MEDIASUBTYPE_NV12)
	{
		m_pSubPicBlender = std::make_shared<BlendBGRAWithNV12BT601>();
	}
	else
	{
		m_pSubPicBlender = nullptr;
	}
}
//------------------------------------------------------------------------------