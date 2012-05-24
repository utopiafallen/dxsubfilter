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
	//	This class is thread-safe.
	class SubtitleRendererFactory
	{
	public:
		// Use eager initialization for thread-safety purposes
		static SubtitleRendererFactory* getSingleton()
		{
			return instance;
		}

		// Returns a shared_ptr to a subtitle renderer that was created based on the SubtitleType
		// passed in. If bUniqueInstance was set to true, this is to a new instance of the relevant
		// subtitle renderer. Otherwise, this will be a pointer to A CACHED RESULT. Users must
		// be aware of this behaviour in multithreaded environments.
		//
		// Returns a nullptr if it was unable to create a subtitle renderer. This may be due to
		// unsupported type passed in or because the factory has not been initialized with all the
		// necessary data. User must call SetSubtitleCoreConfig and SetVideoInfo with the valid
		// data before making calls to this function.
		std::shared_ptr<ISubtitleRenderer> CreateSubtitleRenderer(SubtitleType type, bool bUniqueInstance = false);

		// Sets SubtitleCore configuration data. Must be called along with SetVideoInfo before
		// CreateSubtitleRenderer will return valid results
		void SetSubtitleCoreConfig(SubtitleCoreConfigurationData& config);

		// Sets the video info of the video frame that subtitles will be rendered into.
		void SetVideoInfo(VideoInfo& vidInfo);
	private: // Functions
		SubtitleRendererFactory();
		SubtitleRendererFactory(const SubtitleRendererFactory&);
		~SubtitleRendererFactory();

		SubtitleRendererFactory& operator= (const SubtitleRendererFactory&);

	private: // Data
		static SubtitleRendererFactory* instance;

		std::vector<std::vector<std::shared_ptr<ISubtitleRenderer>>> m_SubtitleRendererCache;

		std::shared_ptr<SubtitleCoreConfigurationData> m_SubCoreConfig;
		std::shared_ptr<VideoInfo> m_VideoInfo;

		// Shared amongst all renderers.
		IDWriteFactory* m_DWriteFactory;

		Concurrency::reader_writer_lock m_RWLockFactory;
	};
};
#endif