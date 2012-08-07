#pragma once

#include "stdafx.h"
#include "SubtitleCoreEnumerations.h"
#include "ISubtitleRenderer.h"

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

		// Overridden to handle subtitle stream switching. These are not called if external
		// subtitle are loaded as this pin will reject all incoming connection attempts in that
		// case.
		STDMETHODIMP Disconnect();
		HRESULT CompleteConnect(IPin *pReceivePin);

		// Overridden to skip transform filter calls so seeking doesn't break.
		STDMETHODIMP BeginFlush(void);
		STDMETHODIMP EndFlush(void);
		STDMETHODIMP NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);

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

		// Called by the parent filter once it's appropriate to load a subtitle renderer. If
		// external subtitles were loaded, they will be parsed at this point.
		void LoadSubtitleRenderer(SubtitleCore::VideoInfo& targetVideoInfo);

		bool IsExternalSubtitlesLoaded() const { return m_bExternalSubtitlesLoaded; }

	public: // Data

		// Corresponding subtitle renderer based on our current subtitle type. Made public for
		// convenience since this pin isn't actually responsible for initiating rendering of
		// subpics.
		std::shared_ptr<SubtitleCore::ISubtitleRenderer> m_SubtitleRenderer;

	protected: // Data

		// Flagged to true if we loaded external subtitles
		bool m_bExternalSubtitlesLoaded;

		// The contents of the external subtitles
		std::vector<std::wstring> m_ExternalSubtitleScript;

		// Set to the current subtitle type that we're rendering
		SubtitleCore::SubtitleType m_CurrentSubtitleType;

	protected: // Functions

		// Maps a file extension to the appropriate SubtitleType
		SubtitleCore::SubtitleType MapFileExtToSubtitleType(const std::wstring& fileExt) const;

		// Maps a media subtype to the appropriate SubtitleType
		SubtitleCore::SubtitleType MapMediaTypeToSubtitleType(const CMediaType& mt) const;
	};
};