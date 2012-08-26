#include "stdafx.h"
#include "DrawingEffectRendererFactory.h"

#include "CustomDrawingEffects.h"
#include "OutlineDrawingEffectRenderer.h"

using namespace SubtitleCore;

DrawingEffectsRendererFactory* DrawingEffectsRendererFactory::instance = nullptr;

std::shared_ptr<ICustomDrawingEffectRenderer> DrawingEffectsRendererFactory::CreateEffectRenderer(const ICustomDrawingEffect& effect)
{
	unsigned int index = static_cast<unsigned int>(effect.GetType());
	unsigned int size = index + 1;
	
	m_RWLockFactory.lock();
	if (size > m_RendererCache.size())
	{
		m_RendererCache.resize(size);
	}
	m_RWLockFactory.unlock();

	switch(effect.GetType())
	{
	case ICustomDrawingEffect::DET_OUTLINEFILL:
		{
			if (m_RendererCache[index])
			{
				return m_RendererCache[index];
			}
			else
			{
				// Multiple threads creating multiple renderers at the same index is fine because shared_ptr will
				// automatically free previously created renderers.
				const OutlineAndFillDrawingEffect& outlineEffect = static_cast<const OutlineAndFillDrawingEffect&>(effect);
				m_RendererCache[index] = std::make_shared<OutlineFillEffectRenderer>(&outlineEffect);
				return m_RendererCache[index];
			}
		}
	default:
		{
			return nullptr;
		}
	}
}