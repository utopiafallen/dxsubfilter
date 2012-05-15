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

namespace DXSubFilter
{
	static const GUID DXSUBFILTER_SUPPORTED_VIDEO_SUBTYPES_8BIT[] = {
		MEDIASUBTYPE_AYUV,	// Packed 4:4:4
		MEDIASUBTYPE_YUY2,	// Packed 4:2:2
		MEDIASUBTYPE_YV12,	// Planar 4:2:0
		MEDIASUBTYPE_NV12,	// Planar/Packed 4:2:0
	};

	static const GUID DXSUBFILTER_SUPPORTED_VIDEO_SUBTYPES_16BIT[] = {
		MEDIASUBTYPE_P210,	// Planar 4:2:2
		MEDIASUBTYPE_P216,	// Planar 4:2:2
		MEDIASUBTYPE_P010,	// Planar 4:2:0
		MEDIASUBTYPE_P016,	// Planar 4:2:0
	};

	static const size_t DXSUBFILTER_SUPPORTED_VIDEO_SUBTYPES_8BIT_COUNT = 
						ARRAYSIZE(DXSUBFILTER_SUPPORTED_VIDEO_SUBTYPES_8BIT);

	static const size_t DXSUBFILTER_SUPPORTED_VIDEO_SUBTYPES_16BIT_COUNT = 
						ARRAYSIZE(DXSUBFILTER_SUPPORTED_VIDEO_SUBTYPES_16BIT);

	class __declspec(uuid("3B6ED1B8-ECF6-422A-8F07-48980E6482CE")) CDXSubFilter : public CTransformFilter
	{
	public: // Functions
		static CUnknown* WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT* phr);
	
		CDXSubFilter(LPUNKNOWN pUnk);
		virtual ~CDXSubFilter();

		//===================================================
		// CTransformFilter overrides
		//===================================================

		// Perform transform
		virtual HRESULT Transform(IMediaSample * pIn, IMediaSample *pOut);

		// Check if we can support mtIn
		virtual HRESULT CheckInputType(const CMediaType* mtIn);

		// Check if we can support the transform from this input to this output
		virtual HRESULT CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut);

		// Call the SetProperties function with appropriate arguments
		virtual HRESULT DecideBufferSize(
							IMemAllocator * pAllocator,
							ALLOCATOR_PROPERTIES *pprop);

		// Suggest OUTPUT pin media types
		virtual HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);
		
		// Called when upstream filter decides what media type to pass into us
		virtual HRESULT SetMediaType(PIN_DIRECTION direction,const CMediaType *pmt);

		// Override because CTransformFilter only has 2 pins, but we have 3
		virtual int GetPinCount();
		virtual CBasePin* GetPin(int n);

	protected:
		// We use m_pInput inherited from CTransformFilter for our video input so we use a second
		// pin to accept text subtitle data.
		CTransformInputPin* m_pInputSubtitlePin;

		// Save the video format that upstream decided on so we can pass the same format downstream.
		// We will not perform any video format conversions. However, in the special case of 
		// 10/16-bit input, we will offer 8-bit formats as output so that we can force the video
		// decoder to output in 8-bit if the video renderer can't accept 10/16-bit input.
		CMediaType m_InputVideoType;

	private:
		static const int m_iPinCount = 3;

		// Returns true if the passed in MediaType is one of the 8-bit video types
		bool CheckVideoSubtypeIs8Bit(const CMediaType* pMediaType);

		// Returns true if the passed in MediaType is one of the 10/16-bit video types
		bool CheckVideoSubtypeIs16Bit(const CMediaType* pMediaType);
	};
};
