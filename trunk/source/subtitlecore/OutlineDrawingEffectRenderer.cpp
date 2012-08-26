#include "stdafx.h"
#include "OutlineDrawingEffectRenderer.h"

#include "CustomDrawingEffects.h"

using namespace SubtitleCore;

void OutlineFillEffectRenderer::Render(ID2D1RenderTarget* pRT, ID2D1TransformedGeometry* pTransformedGeometry)
{
	// Draw the outline of the rectangle. Stroke width is centered on the geometry and VSFilter looks like it grows
	// the widened border outward so we need to double the border width as the fill step will overwrite half of the
	// border.
	pRT->DrawGeometry(
		pTransformedGeometry,
		m_pEffect->m_pOutlineBrush,
		m_pEffect->m_fBorderWidth
		);

	// Fill in the rectangle
	pRT->FillGeometry(
		pTransformedGeometry,
		m_pEffect->m_pFillBrush
		);
}