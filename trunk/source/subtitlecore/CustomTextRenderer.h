#ifndef SCCUSTOMTEXTRENDERER_H
#define SCCUSTOMTEXTRENDERER_H
#pragma once

#include "stdafx.h"

namespace SubtitleCore
{
	// CustomTextRenderer
	//	This class provides custom text rendering capabilities for use by any subtitle renderer.
	//	Renderers that need a custom drawing effect define a CustomDrawingEffect that is applied
	//	to a IDWriteTextLayout object and then provide an instance of this class to 
	//	IDWriteTextLayout::Draw(). Internally, this class handles different drawing effects
	//	by querying the DrawingEffectRendererFactory for the right instance of an 
	//	ICustomDrawingEffectRenderer based on the specified drawing effect. Thus, to add support
	//	for any new CustomDrawingEffect, one only needs to define the new drawing effect, its
	//	accompanying renderer, and update the create in DrawingEffectRendererFactory.
	class CustomTextRenderer : public IDWriteTextRenderer
	{
	public:
		CustomTextRenderer(ID2D1Factory* pD2DFactory, ID2D1RenderTarget* pRT);
		virtual ~CustomTextRenderer();

		//===================================================
		// IDWritePixelSnapping implementations
		//===================================================
		STDMETHOD(IsPixelSnappingDisabled)(
			__maybenull void* clientDrawingContext,
			__out BOOL* isDisabled
			);

		STDMETHOD(GetCurrentTransform)(
			__maybenull void* clientDrawingContext,
			__out DWRITE_MATRIX* transform
			);

		STDMETHOD(GetPixelsPerDip)(
			__maybenull void* clientDrawingContext,
			__out FLOAT* pixelsPerDip
			);

		//===================================================
		// IDWriteTextRenderer implementations
		//===================================================
		// Drawing context must not be null for the draw functions.
		STDMETHOD(DrawGlyphRun)(
			void* clientDrawingContext,
			FLOAT baselineOriginX,
			FLOAT baselineOriginY,
			DWRITE_MEASURING_MODE measuringMode,
			__in DWRITE_GLYPH_RUN const* glyphRun,
			__in DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
			IUnknown* clientDrawingEffect
			);

		STDMETHOD(DrawUnderline)(
			void* clientDrawingContext,
			FLOAT baselineOriginX,
			FLOAT baselineOriginY,
			__in DWRITE_UNDERLINE const* underline,
			IUnknown* clientDrawingEffect
			);

		STDMETHOD(DrawStrikethrough)(
			void* clientDrawingContext,
			FLOAT baselineOriginX,
			FLOAT baselineOriginY,
			__in DWRITE_STRIKETHROUGH const* strikethrough,
			IUnknown* clientDrawingEffect
			);

		// This is not implemented. Do not use.
		STDMETHOD(DrawInlineObject)(
			void* clientDrawingContext,
			FLOAT originX,
			FLOAT originY,
			IDWriteInlineObject* inlineObject,
			BOOL isSideways,
			BOOL isRightToLeft,
			IUnknown* clientDrawingEffect
			);

		//===================================================
		// IUnknown implementations
		//===================================================
		unsigned long STDMETHODCALLTYPE AddRef();
		unsigned long STDMETHODCALLTYPE Release();
		HRESULT STDMETHODCALLTYPE QueryInterface(
			IID const& riid,
			void** ppvObject
			);

	private:
		unsigned long m_uRefCount;
		ID2D1Factory* m_pD2DFactory;
		ID2D1RenderTarget* m_pRT;

		CustomTextRenderer(const CustomTextRenderer&);
		CustomTextRenderer& operator= (const CustomTextRenderer&);
	};
};

#endif