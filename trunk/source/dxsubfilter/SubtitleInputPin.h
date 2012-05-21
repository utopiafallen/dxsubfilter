#pragma once

#include "stdafx.h"
#include "SubtitleCoreEnumerations.h"

namespace DXSubFilter
{
	class CDXSubFilter;

	//=======================================================================================
	// CSubtitleInputPin
	//	Custom subtitle input pin class to handle receiving subtitle data. We prioritize
	//	external subtitles over embedded subtitles so we will fail connections if we 
	//	succeeded in loading external subtitles.
	//=======================================================================================
	class CSubtitleInputPin : public CTransformInputPin
	{
	public:
		CSubtitleInputPin(
			LPCWSTR pObjectName,
			CDXSubFilter *pTransformFilter,
			HRESULT * phr,
			LPCWSTR pName);
	
		virtual ~CSubtitleInputPin();

		//===================================================
		// CTransformInputPin overrides.
		//===================================================

		// Overridden to handle subtitle stream switching (maybe?)
		HRESULT BreakConnect();
		HRESULT CompleteConnect(IPin *pReceivePin);

		// Overridden to do skip transform filter calls so seeking doesn't break.
		STDMETHODIMP BeginFlush(void);
		STDMETHODIMP EndFlush(void);

		// Haali splitter sends EOS when there are no subtitles to display, but we shouldn't
		// propagate this notification downstream so we override it.
		STDMETHODIMP EndOfStream();

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

		//===================================================
		// CSubtitleInputPin functions.
		//===================================================

		// Attempt to load external subtitles. Call this before a connection attempt is made
		// from the upstream filter. CDXSubFilter calls this in GetPin(). Call 
		// IsExternalSubtitlesLoaded() to check if this succeeded.
		void LoadExternalSubtitles();

		bool IsExternalSubtitlesLoaded() { return m_bExternalSubtitlesLoaded; }
	protected:

		// Flagged to true if we loaded external subtitles
		bool m_bExternalSubtitlesLoaded;

		// Set
		SubtitleCore::SubtitleType m_SubType;
	};
};