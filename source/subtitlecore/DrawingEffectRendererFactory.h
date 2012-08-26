#ifndef DRAWINGEFFECTSRENDERERFACTORY_H
#define DRAWINGEFFECTSRENDERERFACTORY_H
#pragma once

#include <memory>
#include <ppl.h>

namespace SubtitleCore
{
	// Forward declarations
	class ICustomDrawingEffectRenderer;
	class ICustomDrawingEffect;

	// DrawingEffectsRendererFactory
	//	This class returns the correct renderer based upon the passed in DrawingEffectType.
	//
	//	This is thread-safe.
	class DrawingEffectsRendererFactory
	{
	public:
		// Must call InitializeSingleton() before calling this.
		static DrawingEffectsRendererFactory* GetSingleton()
		{
			return instance;
		}

		// Must be called only by one thread. SubtitleRendererFactory::InitializeSingleton() takes care of
		// calling this function so that users of the subtitlecore library don't have to worry about internal
		// initializations.
		static void InitializeSingleton()
		{
			if (!instance)
			{
				instance = new DrawingEffectsRendererFactory();
			}
		}

		// Returns a shared_ptr to a CustomDrawingEffectRenderer based upon the passed in CustomDrawingEffect.
		std::shared_ptr<ICustomDrawingEffectRenderer> CreateEffectRenderer(const ICustomDrawingEffect& effect);

	private:	// Functions
		DrawingEffectsRendererFactory(){}
		DrawingEffectsRendererFactory(const DrawingEffectsRendererFactory&);
		DrawingEffectsRendererFactory& operator= (const DrawingEffectsRendererFactory&);
		~DrawingEffectsRendererFactory(){ m_RendererCache.clear(); }

	private:	// Data
		static DrawingEffectsRendererFactory* instance;

		// We cache the renderers because they can be shared between create calls.
		std::vector<std::shared_ptr<ICustomDrawingEffectRenderer>> m_RendererCache;

		Concurrency::reader_writer_lock m_RWLockFactory;
	};
};

#endif