// Convenience function to convert between units
#ifndef SCUCONVERSIONS_H
#define SCUCONVERSIONS_H
#pragma once
#include "strmif.h"

// SCU - SubtitleCoreUtilities
namespace SCU
{
	inline REFERENCE_TIME ConvertTimeToReferenceTime(size_t hours, size_t minutes, size_t seconds, 
												size_t milliseconds)
	{
		return  (hours * 36000000000i64) +
				(minutes * 600000000i64) + 
				(seconds * 10000000i64) + 
				(milliseconds * 10000i64);
	}
};

#endif