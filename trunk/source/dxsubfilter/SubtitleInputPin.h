// Custom subtitle input pin class to handle receiving subtitle data.
#pragma once

#include "stdafx.h"

namespace DXSubFilter
{
	class CSubtitleInputPin : public CTransformInputPin
	{
	public:
		CSubtitleInputPin(
			LPCWSTR pObjectName,
			CTransformFilter *pTransformFilter,
			HRESULT * phr,
			LPCWSTR pName);
	
		virtual ~CSubtitleInputPin();

		//===================================================
		// CTransformInputPin overrides.
		//===================================================

		// Check that we can support this output type. Overridden so that this pin will never
		// receive video data.
		HRESULT CheckMediaType(const CMediaType* mtIn);

		// Set the media type for this connection. Upstream calls it. Overridden because we
		// don't want to call CheckInputType on the transform filter.
		HRESULT SetMediaType(const CMediaType* mtIn);

		// Here's the next block of data from the stream.
		// AddRef it yourself if you need to hold it beyond the end
		// of this call. We override this because CTransformInputPin's Receive will call 
		// Receive on the filter, which in turn calls the filter's Transform, but we don't want 
		// to transform the raw subtitle data. Instead, the raw subtitle data should be handed 
		// to the subtitle rendering core for internal processing that will then be used during
		// the transform process for the video input.
		STDMETHODIMP Receive(IMediaSample * pSample);

	protected:
	};
};