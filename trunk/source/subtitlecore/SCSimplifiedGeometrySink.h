#ifndef SCSIMPLIFIEDGEOMETRYSINK_H
#define SCSIMPLIFIEDGEOMETRYSINK_H
#pragma once

#include "stdafx.h"

namespace SubtitleCore
{
	// This class implements a specialized ID2D1SimplifiedGeometrySink that custom effects renderers can optionally
	// use. The main purpose of this class is to provide a clean way to created widened outlines via scanline 
	// rasterization.
	class SCSimplifiedGeometrySink : public ID2D1SimplifiedGeometrySink
	{

	};
};

#endif