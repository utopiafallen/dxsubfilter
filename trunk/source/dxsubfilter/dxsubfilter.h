// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the DXSUBFILTER_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// DXSUBFILTER_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#pragma once

#ifdef DXSUBFILTER_EXPORTS
#define DXSUBFILTER_API __declspec(dllexport)
#else
#define DXSUBFILTER_API __declspec(dllimport)
#endif

#include "stdafx.h"
#include "dxsubfilter_uuids.h"
#include "registryhelpers.h"

namespace DXSubFilter
{
	// Supported video types. Note that the ordering matters as this is the order formats are
	// exposed for connection. Any modifications to these lists must have corresponding updates
	// in CDXSubFilter::ComputeStrides(), CDXSubFilter::CopyBuffer(), and 
	// CDXSubFilter::CorrectVideoMediaType(). Filter setup data should be updated as well in
	// dllmain.cpp
	static const GUID DXSUBFILTER_SUPPORTED_VIDEO_SUBTYPES_8BIT[] = {
		MEDIASUBTYPE_NV12,	// Planar 4:2:0
		MEDIASUBTYPE_YV12,	// Planar 4:2:0
		MEDIASUBTYPE_YUY2,	// Packed 4:2:2
		MEDIASUBTYPE_AYUV,	// Packed 4:4:4
	};

	static const GUID DXSUBFILTER_SUPPORTED_VIDEO_SUBTYPES_16BIT[] = {
		MEDIASUBTYPE_P010,	// Planar 4:2:0
		MEDIASUBTYPE_P016,	// Planar 4:2:0
		MEDIASUBTYPE_P210,	// Planar 4:2:2
		MEDIASUBTYPE_P216,	// Planar 4:2:2
	};

	static const size_t DXSUBFILTER_SUPPORTED_VIDEO_SUBTYPES_8BIT_COUNT = 
						ARRAYSIZE(DXSUBFILTER_SUPPORTED_VIDEO_SUBTYPES_8BIT);

	static const size_t DXSUBFILTER_SUPPORTED_VIDEO_SUBTYPES_16BIT_COUNT = 
						ARRAYSIZE(DXSUBFILTER_SUPPORTED_VIDEO_SUBTYPES_16BIT);



	// Forward declarations
	class CSubtitleInputPin;
	class SubPicBlender;


	class __declspec(uuid("3B6ED1B8-ECF6-422A-8F07-48980E6482CE")) CDXSubFilter : public CTransformFilter
	{
	public: // Functions
		static CUnknown* WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT* phr);
		//static void CALLBACK Loading(BOOL bLoading, const CLSID* rclsid);
	
		CDXSubFilter(LPUNKNOWN pUnk);
		virtual ~CDXSubFilter();

		//===================================================
		// CTransformFilter overrides
		//===================================================

		// Override to save tStart and tStop. These are actually given in time relative to 
		// playback duration i.e. absolute playback position. We can then use these values to
		// calculate time stamps of input samples.
		virtual HRESULT NewSegment(
						REFERENCE_TIME tStart,
						REFERENCE_TIME tStop,
						double dRate);

		// Perform transform. Note that this only handles the overlaying of the subtitle onto
		// the video frame. Subtitle data is received and processed internally in CSubtitleInputPin
		virtual HRESULT Transform(IMediaSample * pIn, IMediaSample *pOut);

		// Check if we can support mtIn
		virtual HRESULT CheckInputType(const CMediaType* mtIn);

		// Check if we can support the transform from this input to this output
		virtual HRESULT CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut);

		// We override this so that we can perform additional checking related to 10/16-bit
		// support
		virtual HRESULT CompleteConnect(PIN_DIRECTION direction, IPin* pReceivePin);

		// Decide the buffer size to be allocated by downstream filter. This is only called by
		// the output pin as the input pin just accepts the allocator and allocator properties
		// decided by the upstream filter.
		virtual HRESULT DecideBufferSize(
							IMemAllocator * pAllocator,
							ALLOCATOR_PROPERTIES *pProp);

		// Suggest OUTPUT pin media types
		virtual HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);
		
		// Called when upstream filter decides what media type to pass into us
		virtual HRESULT SetMediaType(PIN_DIRECTION direction,const CMediaType *pmt);

		// Override because CTransformFilter only has 2 pins, but we have 3
		virtual int GetPinCount();
		virtual CBasePin* GetPin(int n);

	protected: // Data
		friend class CSubtitleInputPin;

		// We use m_pInput inherited from CTransformFilter for our video input so we use a second
		// pin to accept text subtitle data.
		CSubtitleInputPin* m_pInputSubtitlePin;

		// Save the video format that upstream decided on so we can pass the same format downstream.
		// We will not perform any video format conversions. However, in the special case of 
		// 10/16-bit input, we will offer 8-bit formats as output so that we can force the video
		// decoder to output in 8-bit if the video renderer can't accept 10/16-bit input.
		CMediaType m_InputVideoType;

		// Store stride values so we don't have to recompute every time we need to copy
		size_t m_InputStrideY, m_InputStrideUV;
		size_t m_OutputStrideY, m_OutputStrideUV;

		// Aligned buffer to copy input sample to. Being aligned will make SSE2 processing easier
		// and since the input sample size won't change (I don't think), we can allocate once
		// and re-use.
		BYTE* m_pAlignedBuffer;
		size_t m_uAlignedBufferLength;

		// Used to supplement tStart from CBaseFilter to allow us to calculate current playback
		// time without needing to query the filter graph manager, which can lead to deadlocks.
		REFERENCE_TIME m_rtStart, m_rtEnd;
		double m_dPlaybackRate;

		// Store subtitle core settings we loaded in from the registry. We just copy the results
		// from g_SubtitleCoreConfigData which should have been populated during DLL loading.
		SubtitleCore::SubtitleCoreConfigurationData m_SubCoreConfigData;

		// Functor that will handle blending subpics to video frame
		std::shared_ptr<SubPicBlender> m_pSubPicBlender;

	protected: // Functions

		// This is a single massive function that will handle properly blitting video data
		// from input media sample to output media sample. This is necessary because different
		// video formats have different striding in their data layout and the output format
		// itself could have additional padding depending on renderer requirements
		void CopyBuffer(BYTE* pBufferIn, BYTE* pBufferOut, size_t srcActualDataLength) const;

		// Given the input buffer, this function will extract the Y, U, and V planes from the
		// data stream and store the pointers into the output array.
		void ExtractYUV(BYTE* pBufferIn, BYTE* pPlanes[3]) const;

		// Computes input and output strides and stores them into the appropriate member variables.
		// Should be called every time the input or output media types changes.
		void ComputeStrides();

		// Re-adjusts a fully specified MediaType to ensure all its parameters are coherent.
		// NOTE: The passed in media type must have its VIDEOINFOHEADER/VIDEOINFOHEADER2 already
		// filled in. Easiest way to do that is to copy from m_pInputVideoType before passing 
		// into this function
		void CorrectVideoMediaType(CMediaType* pMediaType) const;

		// Returns true if the passed in MediaType is one of the 8-bit video types
		bool CheckVideoSubtypeIs8Bit(const CMediaType* pMediaType) const;

		// Returns true if the passed in MediaType is one of the 10/16-bit video types
		bool CheckVideoSubtypeIs16Bit(const CMediaType* pMediaType) const;

		// Checks for embedded subtitles by searching for the splitter, enumerating its pins,
		// and seeing if any of them expose MEDIATYPE_Subtitle or MEDIATYPE_Text as a connection
		// format. This assumes that splitters don't expose output pins if there is no embedded
		// subtitle data.
		bool CheckForEmbeddedSubtitles() const;

		// Assigns the correct subpic blender based on input video type
		void AssignSubPicBlender();

		// Calculate current playback time relative to playback duration.
		REFERENCE_TIME CalcCurrentTime(REFERENCE_TIME rtOffset) const;

	private: // Data

		static const int m_iPinCount = 3;

		size_t m_uInputVideoWidth;
		size_t m_uInputVideoHeight;

	private: // Functions
	};
};
