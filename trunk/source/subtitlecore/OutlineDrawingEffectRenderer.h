#ifndef OUTLINEDRAWINGEFFECTRENDERER_H
#define OUTLINEDRAWINGEFFECTRENDERER_H
#pragma once

#include "ICustomDrawingEffectRenderer.h"

namespace SubtitleCore
{
	// Forward declarations
	struct OutlineAndFillDrawingEffect;
	struct OutlineOnlyDrawingEffect;

	// OutlineFillEffectRenderer
	//	Renders both the outline and fill.
	class OutlineFillEffectRenderer : public ICustomDrawingEffectRenderer
	{
	public:
		OutlineFillEffectRenderer (const OutlineAndFillDrawingEffect* effect) 
			: m_pEffect(effect) 
		{}

		virtual void Render(ID2D1RenderTarget* pRT, ID2D1TransformedGeometry* pTransformedGeometry, SCSimplifiedGeometrySink* pSimplifiedGeo = nullptr);

	private:
		const OutlineAndFillDrawingEffect* m_pEffect;
	};

	// OutlineOnlyEffectRenderer
	//	Renders only an outline.
	class OutlineOnlyEffectRenderer : public ICustomDrawingEffectRenderer
	{
	public:
		OutlineOnlyEffectRenderer (const OutlineOnlyDrawingEffect* effect) 
			: m_pEffect(effect)
		{}

		virtual void Render(ID2D1RenderTarget* pRT, ID2D1TransformedGeometry* pTransformedGeometry, SCSimplifiedGeometrySink* pSimplifiedGeo = nullptr);

	private:
		const OutlineOnlyDrawingEffect* m_pEffect;
	};
};

#endif