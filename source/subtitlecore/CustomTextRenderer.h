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

	};
};

#endif