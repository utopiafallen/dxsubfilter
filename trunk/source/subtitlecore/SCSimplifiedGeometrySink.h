#ifndef SCSIMPLIFIEDGEOMETRYSINK_H
#define SCSIMPLIFIEDGEOMETRYSINK_H
#pragma once

#include "stdafx.h"
#include "SCUMemory.h"

namespace SubtitleCore
{
	// This class implements a specialized ID2D1SimplifiedGeometrySink that custom effects renderers can optionally
	// use. The main purpose of this class is to provide a clean way to created widened outlines via scanline 
	// rasterization. It is an error if non-line geometry is added to this sink as we have no implementation for 
	// them.
	class SCSimplifiedGeometrySink : public ID2D1SimplifiedGeometrySink
	{
		enum STATE { NONE, FIGURE_BEGUN, FIGURE_ENDED, INVALID };

	public:
		// Constructor will increment ref-count so no need to explicitly call AddRef after construction.
		SCSimplifiedGeometrySink();
		virtual ~SCSimplifiedGeometrySink() { m_uRefCount = 0; }

		//===================================================
		// ID2D1SimplifiedGeometrySink implementations
		//===================================================
		STDMETHOD_(void, SetFillMode)(D2D1_FILL_MODE fillMode);
	
		STDMETHOD_(void, SetSegmentFlags)(D2D1_PATH_SEGMENT vertexFlags);
	
		STDMETHOD_(void, BeginFigure)(D2D1_POINT_2F startPoint, D2D1_FIGURE_BEGIN figureBegin);
	
		STDMETHOD_(void, AddLines)(__in_ecount(pointsCount) CONST D2D1_POINT_2F *points,
								   UINT pointsCount);
	
		// Unimplemented. Close will return an error if this was called.
		STDMETHOD_(void, AddBeziers)(__in_ecount(beziersCount) CONST D2D1_BEZIER_SEGMENT *beziers,
									 UINT beziersCount);
	
		STDMETHOD_(void, EndFigure)(D2D1_FIGURE_END figureEnd);
	
		STDMETHOD(Close)();

		//===================================================
		// IUnknown implementations
		//===================================================
		unsigned long STDMETHODCALLTYPE AddRef();
		unsigned long STDMETHODCALLTYPE Release();
		HRESULT STDMETHODCALLTYPE QueryInterface(
			IID const& riid,
			void** ppvObject
			);

		//===================================================
		// SCSimplifiedGeometrySink functions
		//===================================================
		// Widens the outline of the geometry by the specified stroke amount. This is an expensive process so avoid
		// calling it more than necessary and cache its results whenever possible.
		virtual void WidenOutline(float stroke);

	protected:
		STATE m_CurrentState;

		typedef std::vector<D2D1_POINT_2F, SCU::aligned_allocator<D2D1_POINT_2F>> tAlignedPointVector;
		tAlignedPointVector m_LineStartPoints;
		tAlignedPointVector m_LineEndPoints;

	private:
		UINT m_uRefCount;
	};
};

#endif