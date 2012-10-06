#ifndef ICUSTOMDRAWINGEFFECTRENDERER_H
#define ICUSTOMDRAWINGEFFECTRENDERER_H
#pragma once

struct ID2D1RenderTarget;
struct ID2D1TransformedGeometry;

namespace SubtitleCore
{
	class SCSimplifiedGeometrySink;

	// ICustomDrawingEffectRenderer
	//	Pure abstract interface that all CustomDrawingEffect renderers implement. Allows usage of different
	//	effect renderers without needing to know the underlying details.
	class ICustomDrawingEffectRenderer
	{
	public:
		// Render the geometry with the current effect to the specified render target.
		virtual void Render(ID2D1RenderTarget* pRT, ID2D1TransformedGeometry* pTransformedGeometry, SCSimplifiedGeometrySink* pSimplifiedGeo = nullptr) = 0;
	};
};

#endif