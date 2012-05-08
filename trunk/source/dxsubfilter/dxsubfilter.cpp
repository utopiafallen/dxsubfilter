// dxsubfilter.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "dxsubfilter.h"


// This is an example of an exported variable
DXSUBFILTER_API int ndxsubfilter=0;

// This is an example of an exported function.
DXSUBFILTER_API int fndxsubfilter(void)
{
	return 42;
}

// This is the constructor of a class that has been exported.
// see dxsubfilter.h for the class definition
CDXSubFilter::CDXSubFilter(LPUNKNOWN pUnk, HRESULT* phr) 
	: CTransInPlaceFilter(DXSUBFILTER_NAME, pUnk, CLSID_DXSubFilter, phr)
{

}

CDXSubFilter::~CDXSubFilter()
{

}

CUnknown* CDXSubFilter::CreateInstance(LPUNKNOWN pUnk, HRESULT* phr)
{
	return new CDXSubFilter(pUnk, phr);
}

HRESULT CDXSubFilter::CheckInputType(const CMediaType* mtIn)
{
	return S_OK;
}

HRESULT CDXSubFilter::Transform(IMediaSample* pSample)
{
	return S_OK;
}

HRESULT CDXSubFilter::Transform(IMediaSample* pIn, IMediaSample* pOut)
{
	return S_OK;
}
