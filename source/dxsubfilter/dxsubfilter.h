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

// This class is exported from the dxsubfilter.dll
//class DXSUBFILTER_API CDXSubFilter
//{
//public:
//	CDXSubFilter(void);
//	// TODO: add your methods here.
//};

class __declspec(uuid("3B6ED1B8-ECF6-422A-8F07-48980E6482CE")) CDXSubFilter : public CTransformFilter
{
public: // Functions
	static CUnknown* WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT* phr);
	
	CDXSubFilter(LPUNKNOWN pUnk);
	virtual ~CDXSubFilter();

	//===================================================
	// CTransformFilter abstract overrides
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
};

extern DXSUBFILTER_API int ndxsubfilter;

DXSUBFILTER_API int fndxsubfilter(void);
