#include "stdafx.h"
#include "SCSimplifiedGeometrySink.h"
#include "SCUMath.h"

using namespace SubtitleCore;

namespace
{
	__forceinline void ExtrudeAndJoinLineSegmentPair(float startPoints[], float endPoints[], float normals[4],
													 __m128 strokexmm)
	{
		// Make sure our memory alignment is correct
		assert(reinterpret_cast<ptrdiff_t>(startPoints) % 16 == 0);
		assert(reinterpret_cast<ptrdiff_t>(endPoints) % 16 == 0);

		__m128 normalsxmm = _mm_load_ps(normals);
		__m128 startxmm = _mm_load_ps(startPoints);
		__m128 endxmm = _mm_load_ps(endPoints);
		normalsxmm = _mm_mul_ps(normalsxmm, strokexmm);		// normal1 * stroke, normal2 * stroke
		startxmm = _mm_add_ps(startxmm, normalsxmm);		// start1 + normal1 * stroke, start2 + normal2 * stroke
		endxmm = _mm_add_ps(endxmm, normalsxmm);			// end1 + normal1 * stroke, end2 + normal2 * stroke;

		_mm_store_ps(startPoints, startxmm);
		_mm_store_ps(endPoints, endxmm);

		// Compute intersection point of extruded lines
		__m128 intersection = SCU::LineLineIntersectSSE2(startxmm, endxmm);

		// This intersection is now the new endPoint1 and startPoint2
		_mm_store_ps(normals, intersection); // We don't need normals anymore
		endPoints[0] = startPoints[2] = normals[0];
		endPoints[1] = startPoints[3] = normals[1];
	}
};

SCSimplifiedGeometrySink::FigureData::FigureData()
	: m_LineStartPoints()
	, m_LineEndPoints()
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

	// Validate our data is correct.
	std::for_each(m_FigureData.begin(), m_FigureData.end(), [](FigureData& data)
	{
		// The default line simplification will pass in line segments as a series of connected points
		// so if there's a mismatch of end points to start points, we need to manually close the segment.
		if (data.m_LineStartPoints.size() - data.m_LineEndPoints.size() == 1)
		{
			data.m_LineEndPoints.push_back(data.m_LineStartPoints[0]);
		}
		assert(data.m_LineEndPoints.size() == data.m_LineStartPoints.size());
		assert(data.m_LineEndPoints.size() % 2 == 0);
	});

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

#if _DEBUG
		std::for_each(m_FigureData.begin(), m_FigureData.end(), [stroke](FigureData& figureData)
#else
		Concurrency::parallel_for_each(m_FigureData.begin(), m_FigureData.end(), [stroke](FigureData& figureData)
#endif
		{
			_MM_ALIGN16 float normals[4] = {0.0f};
			__m128 strokexmm = _mm_set1_ps(stroke);

			// Handle the first pair of line segments separately.
			assert(SCU::LinesIntersect(&figureData.m_LineStartPoints[0].x, &figureData.m_LineEndPoints[0].x));

			SCU::ComputeLineNormal(&figureData.m_LineStartPoints[0].x, &figureData.m_LineEndPoints[0].x, normals);
			SCU::ComputeLineNormal(&figureData.m_LineStartPoints[1].x, &figureData.m_LineEndPoints[1].x, &normals[2]);

			ExtrudeAndJoinLineSegmentPair(&figureData.m_LineStartPoints[0].x, &figureData.m_LineEndPoints[0].x, normals, strokexmm);

			// Now do the same for the remaining line segments
			for (size_t i = 2, n = figureData.m_LineEndPoints.size(); i < n; i+=2)
			{
				D2D_POINT_2F& prevStartPoint = figureData.m_LineStartPoints[i-1];
				D2D_POINT_2F& prevEndPoint = figureData.m_LineEndPoints[i-1];
				D2D_POINT_2F& startPoint1 = figureData.m_LineStartPoints[i];
				D2D_POINT_2F& startPoint2 = figureData.m_LineStartPoints[i+1];
				D2D_POINT_2F& endPoint1 = figureData.m_LineEndPoints[i];
				D2D_POINT_2F& endPoint2 = figureData.m_LineEndPoints[i+1];

				// We assume that successive line segments are adjacent and intersect at their start/end points i.e., 
				// D2D does not simplify the geometry to line segments in random order. Check that assumption by ensuring
				// successive line segments actually do intersect.
				assert(SCU::LinesIntersect(&startPoint1.x, &endPoint1.x)); // Exploiting contiguity of vectors and memory layout of D2D_POINT_2F...

				// Extrude the line segment out along the normal by stroke width amount
				SCU::ComputeLineNormal(&startPoint1.x, &endPoint1.x, normals);
				SCU::ComputeLineNormal(&startPoint2.x, &endPoint2.x, &normals[2]);

				ExtrudeAndJoinLineSegmentPair(&startPoint1.x, &endPoint1.x, normals, strokexmm);

				// Fix up connection with the previous line segment
				__m128 startxmm = _mm_loadu_ps(&prevStartPoint.x);
				__m128 endxmm = _mm_loadu_ps(&prevEndPoint.x);
				__m128 intersection = SCU::LineLineIntersectSSE2(startxmm, endxmm);
				_mm_store_ps(normals, intersection);
				prevEndPoint.x = startPoint1.x = normals[0];
				prevEndPoint.y = startPoint1.y = normals[1];
			}

			// Special case the final intersection/widening because the last segment connects with the first segment
			size_t pointCount = figureData.m_LineEndPoints.size();
			_MM_ALIGN16 float starts[] = {
				figureData.m_LineStartPoints[pointCount-1].x,
				figureData.m_LineStartPoints[pointCount-1].y,
				figureData.m_LineStartPoints[0].x,
				figureData.m_LineStartPoints[0].x
			};

			_MM_ALIGN16 float ends[] = {
				figureData.m_LineEndPoints[pointCount-1].x,
				figureData.m_LineEndPoints[pointCount-1].y,
				figureData.m_LineEndPoints[0].x,
				figureData.m_LineEndPoints[0].y
			};

			float intersectX(-1.0f), intersectY(-1.0f);
			SCU::LineLineIntersectSSE2(starts, ends, intersectX, intersectY);
			figureData.m_LineEndPoints[pointCount-1].x = figureData.m_LineStartPoints[0].x = intersectX;
			figureData.m_LineEndPoints[pointCount-1].y = figureData.m_LineStartPoints[0].y = intersectY;
		});
	}
}
