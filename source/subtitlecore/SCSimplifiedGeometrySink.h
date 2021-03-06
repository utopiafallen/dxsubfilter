#ifndef SCSIMPLIFIEDGEOMETRYSINK_H
#define SCSIMPLIFIEDGEOMETRYSINK_H
#pragma once

#include "stdafx.h"
#include "SCUMemory.h"

namespace SubtitleCore
{
	// Global typedefs that will probably be useful if the user is including this header.
	typedef std::vector<D2D1_POINT_2F, SCU::aligned_allocator<D2D1_POINT_2F>> tAlignedVectorD2DPOINT2F;

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
		virtual ~SCSimplifiedGeometrySink() { m_uRefCount = 0; m_fStrokeWidth = 1.0f; }

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
		// calling it more than necessary and cache its results whenever possible. Multiple calls with the same
		// stroke (within an epsilon) are guaranteed to be no-ops.
		virtual void WidenOutline(float stroke);

		// Returns the current stroke width.
		float GetCurrentStroke() const { return m_fStrokeWidth; }

	protected:
		STATE m_CurrentState;

		// Simple aggregation of data into figures. This will allow us to process figures concurrently in the future.
		struct FigureData
		{
			tAlignedVectorD2DPOINT2F m_LineStartPoints;
			tAlignedVectorD2DPOINT2F m_LineEndPoints;

			FigureData();
		};
		std::vector<FigureData>	m_FigureData;
		int						m_CurrentFigureIndex;

	private:
		UINT m_uRefCount;
		float m_fStrokeWidth;
	};
};

#endif