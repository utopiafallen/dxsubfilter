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
	}
}
