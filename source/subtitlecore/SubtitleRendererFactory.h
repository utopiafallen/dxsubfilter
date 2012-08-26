#ifndef SUBTITLERENDERERFACTORY_H
#define SUBTITLERENDERERFACTORY_H
#pragma once

#include <memory>
#include <DWrite.h>
#include <ppl.h>
#include "SubtitleCoreEnumerations.h"
#include "ISubtitleRenderer.h"

namespace SubtitleCore
{
	// SubtitleRendererFactory
	//	This class will construct the correct subtitle renderer given a SubtitleType.
	//
	//	This class returns shared_ptr to ensure that all created instances are properly deleted when their 
	//	references are gone. The class is implemented
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

		// Must be called only by one thread.
		static void InitializeSingleton();

		// Returns a shared_ptr to a subtitle renderer that was created based on the SubtitleType
		// passed in.
		//
		// targetVideoFrameInfo describes the video frame that subtitles will be rendered into.
		//
		// Returns a nullptr if it was unable to create a subtitle renderer. This may be due to
		// unsupported type passed in or because the factory has not been initialized with all the
		// necessary data. User must call SetSubtitleCoreConfig with valid data before making calls to this function.
		std::shared_ptr<ISubtitleRenderer> CreateSubtitleRenderer(SubtitleType type, const VideoInfo& targetVideoFrameInfo) const;

		// Sets SubtitleCore configuration data. Must be called before CreateSubtitleRenderer 
		// will return valid results. Calls to this will invalidate and clear the cache so any
		// previously created SubtitleRenderers should no longer be used.
		void SetSubtitleCoreConfig(const SubtitleCoreConfigurationData& config);

	private: // Functions
		SubtitleRendererFactory();
		SubtitleRendererFactory(const SubtitleRendererFactory&);
		SubtitleRendererFactory& operator= (const SubtitleRendererFactory&);
		~SubtitleRendererFactory();

	private: // Data
		static SubtitleRendererFactory* instance;

		// Implemented as a pointer so we can use a nullptr check to determine whether it's safe to construct
		// a SubtitleRenderer.
		std::shared_ptr<SubtitleCoreConfigurationData> m_SubCoreConfig;

		// Shared amongst all renderers.
		IDWriteFactory* m_DWriteFactory;

		Concurrency::reader_writer_lock m_RWLockFactory;
	};
};
#endif