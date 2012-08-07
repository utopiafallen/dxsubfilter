#ifndef SUBTITLERENDERERFACTORY_H
#define SUBTITLERENDERERFACTORY_H
#pragma once

#include <memory>
#include <vector>
#include <DWrite.h>
#include <ppl.h>
#include "SubtitleCoreEnumerations.h"
#include "ISubtitleRenderer.h"

namespace SubtitleCore
{
	// SubtitleRendererFactory
	//	This class will construct the correct subtitle renderer given a SubtitleType. These instances
	//	are then cached and reused on subsequent calls unless the user requests specifically
	//	for a new instance. This class returns shared_ptr to ensure that all created
	//	instances are properly deleted when their references are gone. The class is implemented
	//	as a singleton. 
	//
	//	This class is thread-safe. However, initialization of the singleton must be done only once
	//	and in a thread-safe manner, and is left up to the user to do so.
	class SubtitleRendererFactory
	{
	public:
		// Must call InitializeSingleton() before calling this.
		static SubtitleRendererFactory* GetSingleton()
		{
			return instance;
		}

		// Must be called only once and only by one thread.
		static void InitializeSingleton()
		{
			instance = new SubtitleRendererFactory();
		}

		// Returns a shared_ptr to a subtitle renderer that was created based on the SubtitleType
		// passed in. If bUniqueInstance was set to true, this is to a new instance of the relevant
		// subtitle renderer. Otherwise, this will be a pointer to A CACHED RESULT. Users must
		// be aware of this behaviour in multithreaded environments.
		//
		// targetVideoFrameInfo describes the video frame that subtitles will be rendered into.
		//
		// Returns a nullptr if it was unable to create a subtitle renderer. This may be due to
		// unsupported type passed in or because the factory has not been initialized with all the
		// necessary data. User must call SetSubtitleCoreConfig with valid data before making calls to this function.
		std::shared_ptr<ISubtitleRenderer> CreateSubtitleRenderer(SubtitleType type, VideoInfo& targetVideoFrameInfo);

		// Sets SubtitleCore configuration data. Must be called before CreateSubtitleRenderer 
		// will return valid results. Calls to this will invalidate and clear the cache so any
		// previously created SubtitleRenderers should no longer be used.
		void SetSubtitleCoreConfig(SubtitleCoreConfigurationData& config);

	private: // Functions
		SubtitleRendererFactory();
		SubtitleRendererFactory(const SubtitleRendererFactory&);
		~SubtitleRendererFactory();

		SubtitleRendererFactory& operator= (const SubtitleRendererFactory&);

	private: // Data
		static SubtitleRendererFactory* instance;

		std::vector<std::vector<std::shared_ptr<ISubtitleRenderer>>> m_SubtitleRendererCache;

		std::shared_ptr<SubtitleCoreConfigurationData> m_SubCoreConfig;

		// Shared amongst all renderers.
		IDWriteFactory* m_DWriteFactory;

		Concurrency::reader_writer_lock m_RWLockFactory;
	};
};
#endif