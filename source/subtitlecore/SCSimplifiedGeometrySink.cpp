#include "stdafx.h"
#include "SCSimplifiedGeometrySink.h"
#include "SCUMath.h"

using namespace SubtitleCore;

namespace
{
	static const UINT DEFAULT_PREALLOCATE_COUNT = 64;
};

SCSimplifiedGeometrySink::FigureData::FigureData()
	: m_LineStartPoints(DEFAULT_PREALLOCATE_COUNT)
	, m_LineEndPoints(DEFAULT_PREALLOCATE_COUNT)
{
}

SCSimplifiedGeometrySink::SCSimplifiedGeometrySink()
	: m_uRefCount(1)
	, m_CurrentState(NONE)
	, m_CurrentFigureIndex(-1)
	, m_fStrokeWidth(1.0f)
{
}

STDMETHODIMP_(unsigned long) SCSimplifiedGeometrySink::AddRef()
{
	return InterlockedIncrement(&m_uRefCount);
}

STDMETHODIMP_(unsigned long) SCSimplifiedGeometrySink::Release()
{
	unsigned long newCount = InterlockedDecrement(&m_uRefCount);

	if (newCount == 0)
	{
		delete this;
		return 0;
	}

	return newCount;
}

STDMETHODIMP SCSimplifiedGeometrySink::QueryInterface(
	IID const& riid,
	void** ppvObject
	)
{
	if (__uuidof(ID2D1SimplifiedGeometrySink) == riid)
	{
		*ppvObject = this;
	}
	else if (__uuidof(IUnknown) == riid)
	{
		*ppvObject = this;
	}
	else
	{
		*ppvObject = NULL;
		return E_FAIL;
	}

	AddRef();

	return S_OK;
}

STDMETHODIMP_(void) SCSimplifiedGeometrySink::SetFillMode(D2D1_FILL_MODE fillMode)
{
	UNREFERENCED_PARAMETER(fillMode);
}

STDMETHODIMP_(void) SCSimplifiedGeometrySink::SetSegmentFlags(D2D1_PATH_SEGMENT vertexFlags)
{
	UNREFERENCED_PARAMETER(vertexFlags);
}

STDMETHODIMP_(void) SCSimplifiedGeometrySink::BeginFigure(D2D1_POINT_2F startPoint, D2D1_FIGURE_BEGIN figureBegin)
{
	UNREFERENCED_PARAMETER(figureBegin);
	if (m_CurrentState != INVALID)
	{
		m_CurrentState = FIGURE_BEGUN;
		
		m_CurrentFigureIndex = m_FigureData.size();
		m_FigureData.push_back(FigureData());
		m_FigureData[m_CurrentFigureIndex].m_LineStartPoints.push_back(startPoint);
	}
}

STDMETHODIMP_(void) SCSimplifiedGeometrySink::AddBeziers(CONST D2D1_BEZIER_SEGMENT* beziers, UINT beziersCount)
{
	UNREFERENCED_PARAMETER(beziers);
	UNREFERENCED_PARAMETER(beziersCount);

	m_CurrentState = INVALID;
}

STDMETHODIMP_(void) SCSimplifiedGeometrySink::AddLines(CONST D2D1_POINT_2F* points, UINT pointsCount)
{
	if (m_CurrentState != INVALID && m_CurrentState == FIGURE_BEGUN)
	{
		// For efficiency with SSE2, we end up duplicating begin and end points so that we have discrete
		// line segments stored.
		for (size_t i = 0; i < pointsCount; i++)
		{
			m_FigureData[m_CurrentFigureIndex].m_LineEndPoints.push_back(points[i]);
			m_FigureData[m_CurrentFigureIndex].m_LineStartPoints.push_back(points[i]);
		}
	}
	else
	{
		m_CurrentState = INVALID;
	}
}

STDMETHODIMP_(void) SCSimplifiedGeometrySink::EndFigure(D2D1_FIGURE_END figureEnd)
{
	UNREFERENCED_PARAMETER(figureEnd);

#ifdef _DEBUG
	// Validate our data is correct.
	std::for_each(m_FigureData.begin(), m_FigureData.end(), [](const FigureData& data)
	{
		assert(data.m_LineEndPoints.size() == data.m_LineStartPoints.size());
		assert(data.m_LineEndPoints.size() % 2 == 0);
	});
#endif

	if (m_CurrentState == FIGURE_BEGUN)
	{
		m_CurrentState = FIGURE_ENDED;

		m_CurrentFigureIndex = -1;
	}
	else
	{
		m_CurrentState = INVALID;
	}
}

STDMETHODIMP SCSimplifiedGeometrySink::Close()
{
	if (m_CurrentState == FIGURE_ENDED)
	{
		m_CurrentState = NONE;
		return S_OK;
	}
	else
	{
		m_CurrentState = NONE;
		return E_FAIL;
	}
}

void SCSimplifiedGeometrySink::WidenOutline(float stroke)
{
	if (std::fabsf(m_fStrokeWidth - stroke) > FLT_EPSILON)
	{
		m_fStrokeWidth = stroke;

		std::for_each(m_FigureData.begin(), m_FigureData.end(), [stroke](FigureData& figureData)
		{
			for (size_t i = 0, n = figureData.m_LineEndPoints.size(); i < n; i+=2)
			{
				D2D_POINT_2F& startPoint1 = figureData.m_LineStartPoints[i];
				D2D_POINT_2F& startPoint2 = figureData.m_LineStartPoints[i+1];
				D2D_POINT_2F& endPoint1 = figureData.m_LineEndPoints[i];
				D2D_POINT_2F& endPoint2 = figureData.m_LineEndPoints[i+1];

				// We assume that successive line segments are adjacent and intersect at their start/end points i.e., 
				// D2D does not simplify the geometry to line segments in random order. Check that assumption by ensuring
				// successive line segments actually do intersect.
				assert(SCU::LinesIntersect(&startPoint1.x, &endPoint1.x)); // Exploiting contiguity of vectors and memory layout of D2D_POINT_2F...
				
				// Make sure our memory alignment is correct
				assert(reinterpret_cast<UINT>(&startPoint1) % 16 == 0);
				assert(reinterpret_cast<UINT>(&endPoint1) % 16 == 0);

				// Extrude the line segment out along the normal by stroke width amount
				_MM_ALIGN16 float normals[4] = {0.0f};
				SCU::ComputeLineNormal(&startPoint1.x, &endPoint1.x, normals);
				SCU::ComputeLineNormal(&startPoint2.x, &endPoint2.x, &normals[2]);

				__m128 strokexmm = _mm_set1_ps(stroke);
				__m128 normalsxmm = _mm_load_ps(normals);
				__m128 startxmm = _mm_load_ps(&startPoint1.x);
				__m128 endxmm = _mm_load_ps(&endPoint1.x);
				normalsxmm = _mm_mul_ps(normalsxmm, strokexmm);		// normal1 * stroke, normal2 * stroke
				startxmm = _mm_add_ps(startxmm, normalsxmm);		// start1 + normal1 * stroke, start2 + normal2 * stroke
				endxmm = _mm_add_ps(endxmm, normalsxmm);			// end1 + normal1 * stroke, end2 + normal2 * stroke;

				_mm_store_ps(&startPoint1.x, startxmm);
				_mm_store_ps(&endPoint1.x, endxmm);

				// Compute intersection point of extruded lines
				__m128 intersection = SCU::LineLineIntersectSSE2(startxmm, endxmm);

				// This intersection is now the new endPoint1 and startPoint2
				_mm_store_ps(normals, intersection); // We don't need normals anymore
				endPoint1.x = startPoint2.x = normals[0];
				endPoint1.y = startPoint2.y = normals[1];
			}

			// Special case the final intersection/widening because the last segment connects with the first segment
		});
	}
}
