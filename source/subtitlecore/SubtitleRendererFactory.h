#ifndef SUBTITLERENDERERFACTORY_H
#define SUBTITLERENDERERFACTORY_H
#pragma once

#include <memory>
#include "SubtitleCoreEnumerations.h"

namespace SubtitleCore
{
	// SubtitleRendererFactory
	//	This class will construct the correct subtitle renderer given a SubtitleType. These instances
	//	are then cached and reused on subsequent calls unless the user requests specifically
	//	for a new instance. This class returns shared_ptr to ensure that all created
	//	instances are properly deleted when their references are gone. The class is implemented
	//	as a singleton.
	class SubtitleRendererFactory
	{
	private:
		SubtitleRendererFactory();
		SubtitleRendererFactory(const SubtitleRendererFactory&);
		~SubtitleRendererFactory();

		SubtitleRendererFactory& operator= (const SubtitleRendererFactory&);

		static SubtitleRendererFactory* instance;
	};
};
#endif