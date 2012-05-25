// Convenience function to convert hours, minutes, seconds and milliseconds to REFERENCE_TIME
// values (100ns units)

#include "strmif.h"

namespace SubtitleCoreUtilities
{
	REFERENCE_TIME ConvertTimeToReferenceTime(size_t hours, size_t minutes, size_t seconds, 
												size_t milliseconds)
	{
		return  (hours * 36000000000) +
				(minutes * 600000000) + 
				(seconds * 10000000) + 
				(milliseconds * 10000);
	}
};