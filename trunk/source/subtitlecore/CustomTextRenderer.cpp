#include "stdafx.h"
#include "CustomTextRenderer.h"

#include "CustomDrawingEffects.h"

#include "ICustomDrawingEffectRenderer.h"
#include "DrawingEffectRendererFactory.h"

#include "DirectXHelpers.h"

using namespace SubtitleCore;

CustomTextRenderer::CustomTextRenderer(ID2D1Factory* pD2DFactory, ID2D1RenderTarget* pRT)
	: m_uRefCount(0)
	, m_pD2DFactory(pD2DFactory)
	, m_pRT(pRT)
{
	pD2DFactory->AddRef();
	pRT->AddRef();
}

CustomTextRenderer::~CustomTextRenderer()
{
	SafeRelease(&m_pD2DFactory);
	SafeRelease(&m_pRT);
}

STDMETHODIMP CustomTextRenderer::DrawInlineObject(
	__maybenull void* clientDrawingContext,
	FLOAT originX,
	FLOAT originY,
	IDWriteInlineObject* inlineObject,
	BOOL isSideways,
	BOOL isRightToLeft,
	IUnknown* clientDrawingEffect
	)
{
	UNREFERENCED_PARAMETER(clientDrawingContext);
	UNREFERENCED_PARAMETER(originX);
	UNREFERENCED_PARAMETER(originY);
	UNREFERENCED_PARAMETER(inlineObject);
	UNREFERENCED_PARAMETER(isSideways);
	UNREFERENCED_PARAMETER(isRightToLeft);
	UNREFERENCED_PARAMETER(clientDrawingEffect);
	// Not implemented
	return E_NOTIMPL;
}

STDMETHODIMP_(unsigned long) CustomTextRenderer::AddRef()
{
	return InterlockedIncrement(&m_uRefCount);
}

STDMETHODIMP_(unsigned long) CustomTextRenderer::Release()
{
	unsigned long newCount = InterlockedDecrement(&m_uRefCount);

	if (newCount == 0)
	{
		delete this;
		return 0;
	}

	return newCount;
}

STDMETHODIMP CustomTextRenderer::IsPixelSnappingDisabled(
	__maybenull void* clientDrawingContext,
	__out BOOL* isDisabled
	)
{
	UNREFERENCED_PARAMETER(clientDrawingContext);

	*isDisabled = FALSE;
	return S_OK;
}

STDMETHODIMP CustomTextRenderer::GetCurrentTransform(
	__maybenull void* clientDrawingContext,
	__out DWRITE_MATRIX* transform
	)
{
	UNREFERENCED_PARAMETER(clientDrawingContext);

	//forward the render target's transform
	m_pRT->GetTransform(reinterpret_cast<D2D1_MATRIX_3X2_F*>(transform));
	return S_OK;
}

STDMETHODIMP CustomTextRenderer::GetPixelsPerDip(
	__maybenull void* clientDrawingContext,
	__out FLOAT* pixelsPerDip
	)
{
	UNREFERENCED_PARAMETER(clientDrawingContext);

	float x, yUnused;

	m_pRT->GetDpi(&x, &yUnused);
	*pixelsPerDip = x * (1.0f/96.0f);

	return S_OK;
}

STDMETHODIMP CustomTextRenderer::QueryInterface(
	IID const& riid,
	void** ppvObject
	)
{
	if (__uuidof(IDWriteTextRenderer) == riid)
	{
		*ppvObject = this;
	}
	else if (__uuidof(IDWritePixelSnapping) == riid)
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

STDMETHODIMP CustomTextRenderer::DrawStrikethrough(
	void* clientDrawingContext,
	FLOAT baselineOriginX,
	FLOAT baselineOriginY,
	__in DWRITE_STRIKETHROUGH const* strikethrough,
	IUnknown* clientDrawingEffect
	)
{
	HRESULT hr;

	DrawingContext* context = static_cast<DrawingContext*>(clientDrawingContext);
	assert(context);

	D2D1_RECT_F rect = D2D1::RectF(
		0,
		strikethrough->offset,
		strikethrough->width,
		strikethrough->offset + strikethrough->thickness
		);

	ID2D1RectangleGeometry* pRectangleGeometry = NULL;
	hr = m_pD2DFactory->CreateRectangleGeometry(
			&rect, 
			&pRectangleGeometry
			);

	// Initialize a matrix to translate the origin of the strikethrough
	D2D1::Matrix3x2F const matrix = D2D1::Matrix3x2F(
		1.0f, 0.0f,
		0.0f, 1.0f,
		baselineOriginX, baselineOriginY
		);

	ID2D1TransformedGeometry* pTransformedGeometry = NULL;
	if (SUCCEEDED(hr))
	{
		hr = m_pD2DFactory->CreateTransformedGeometry(
			pRectangleGeometry,
			&matrix,
			&pTransformedGeometry
			);
	}

	// Render client drawing effects
	if (clientDrawingEffect)
	{
		DrawingEffectsCollection* pEffectsCollection = nullptr;
		clientDrawingEffect->QueryInterface(&pEffectsCollection);

		for (auto it = pEffectsCollection->m_Effects.begin(), itEnd = pEffectsCollection->m_Effects.end(); it != itEnd; ++it)
		{
			auto renderer = DrawingEffectsRendererFactory::GetSingleton()->CreateEffectRenderer(**it);
			renderer->Render(m_pRT, pTransformedGeometry);
		}
	}
	else
	{
		// Draw the outline of the rectangle
		m_pRT->DrawGeometry(
			pTransformedGeometry,
			context->m_pFillBrush
			);

		// Fill in the rectangle
		m_pRT->FillGeometry(
			pTransformedGeometry,
			context->m_pFillBrush
			);
	}

	SafeRelease(&pRectangleGeometry);
	SafeRelease(&pTransformedGeometry);

	return S_OK;
}

STDMETHODIMP CustomTextRenderer::DrawUnderline(
	void* clientDrawingContext,
	FLOAT baselineOriginX,
	FLOAT baselineOriginY,
	__in DWRITE_UNDERLINE const* underline,
	IUnknown* clientDrawingEffect
	)
{
	HRESULT hr;

	DrawingContext* context = static_cast<DrawingContext*>(clientDrawingContext);
	assert(context);

	D2D1_RECT_F rect = D2D1::RectF(
		0,
		underline->offset,
		underline->width,
		underline->offset + underline->thickness
		);

	ID2D1RectangleGeometry* pRectangleGeometry = NULL;
	hr = m_pD2DFactory->CreateRectangleGeometry(
			&rect, 
			&pRectangleGeometry
			);

	// Initialize a matrix to translate the origin of the underline
	D2D1::Matrix3x2F const matrix = D2D1::Matrix3x2F(
		1.0f, 0.0f,
		0.0f, 1.0f,
		baselineOriginX, baselineOriginY + underline->offset
		);

	ID2D1TransformedGeometry* pTransformedGeometry = NULL;
	if (SUCCEEDED(hr))
	{
		hr = m_pD2DFactory->CreateTransformedGeometry(
			pRectangleGeometry,
			&matrix,
			&pTransformedGeometry
			);
	}

	// Client drawing effect
	if (clientDrawingEffect)
	{
		DrawingEffectsCollection* pEffectsCollection = nullptr;
		clientDrawingEffect->QueryInterface(&pEffectsCollection);

		for (auto it = pEffectsCollection->m_Effects.begin(), itEnd = pEffectsCollection->m_Effects.end(); it != itEnd; ++it)
		{
			auto renderer = DrawingEffectsRendererFactory::GetSingleton()->CreateEffectRenderer(**it);
			renderer->Render(m_pRT, pTransformedGeometry);
		}
	}
	else
	{
		// Draw the outline of the rectangle
		m_pRT->DrawGeometry(
			pTransformedGeometry,
			context->m_pFillBrush
			);

		// Fill in the rectangle
		m_pRT->FillGeometry(
			pTransformedGeometry,
			context->m_pFillBrush
			);
	}

	SafeRelease(&pRectangleGeometry);
	SafeRelease(&pTransformedGeometry);

	return S_OK;
}

STDMETHODIMP CustomTextRenderer::DrawGlyphRun(
	void* clientDrawingContext,
	FLOAT baselineOriginX,
	FLOAT baselineOriginY,
	DWRITE_MEASURING_MODE measuringMode,
	__in DWRITE_GLYPH_RUN const* glyphRun,
	__in DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
	IUnknown* clientDrawingEffect
	)
{
	HRESULT hr = S_OK;

	UNREFERENCED_PARAMETER(glyphRunDescription);
	UNREFERENCED_PARAMETER(measuringMode);
	UNREFERENCED_PARAMETER(hr);

	DrawingContext* context = static_cast<DrawingContext*>(clientDrawingContext);
	assert(context);

	// Create the path geometry.
	ID2D1PathGeometry* pPathGeometry = NULL;
	hr = m_pD2DFactory->CreatePathGeometry(
			&pPathGeometry
			);

	// Write to the path geometry using the geometry sink.
	ID2D1GeometrySink* pSink = NULL;
	hr = pPathGeometry->Open(&pSink);

	// Get the glyph run outline geometries back from DirectWrite and place them within the
	// geometry sink.
	hr = glyphRun->fontFace->GetGlyphRunOutline(
		glyphRun->fontEmSize,
		glyphRun->glyphIndices,
		glyphRun->glyphAdvances,
		glyphRun->glyphOffsets,
		glyphRun->glyphCount,
		glyphRun->isSideways,
		glyphRun->bidiLevel%2,
		pSink
		);

	// Close the geometry sink
	hr = pSink->Close();

	// Simplify the geometry
	ID2D1PathGeometry* pSimplifiedPathGeometry = nullptr;
	hr = m_pD2DFactory->CreatePathGeometry(&pSimplifiedPathGeometry);
	hr = pSimplifiedPathGeometry->Open(&pSink);
	hr = pPathGeometry->Simplify(D2D1_GEOMETRY_SIMPLIFICATION_OPTION_LINES, NULL, pSink);
	hr = pSink->Close();

	// Initialize a matrix to translate the origin of the glyph run.
	D2D1::Matrix3x2F transform = D2D1::Matrix3x2F(
		1.0f, 0.0f,
		0.0f, 1.0f,
		baselineOriginX, baselineOriginY
		);

	// Create the transformed geometry
	ID2D1TransformedGeometry* pTransformedGeometry = NULL;
	hr = m_pD2DFactory->CreateTransformedGeometry(
		pSimplifiedPathGeometry,
		&transform,
		&pTransformedGeometry
		);

	// Client drawing effect
	if (clientDrawingEffect)
	{
		DrawingEffectsCollection* pEffectsCollection = nullptr;
		clientDrawingEffect->QueryInterface(&pEffectsCollection);

		for (auto it = pEffectsCollection->m_Effects.begin(), itEnd = pEffectsCollection->m_Effects.end(); it != itEnd; ++it)
		{
			auto renderer = DrawingEffectsRendererFactory::GetSingleton()->CreateEffectRenderer(**it);
			renderer->Render(m_pRT, pTransformedGeometry);
		}
	}
	else
	{
		// Draw the outline of the glyph run
		m_pRT->DrawGeometry(
			pTransformedGeometry,
			context->m_pFillBrush
			);

		// Fill in the glyph run
		m_pRT->FillGeometry(
			pTransformedGeometry,
			context->m_pFillBrush
			);
	}

	SafeRelease(&pPathGeometry);
	SafeRelease(&pSink);
	SafeRelease(&pTransformedGeometry);

	return hr;
}







