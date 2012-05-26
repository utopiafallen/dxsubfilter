// Convenience function to convert hours, minutes, seconds and milliseconds to REFERENCE_TIME
// values (100ns units)

#include "strmif.h"

namespace SubtitleCoreUtilities
{
	inline REFERENCE_TIME ConvertTimeToReferenceTime(size_t hours, size_t minutes, size_t seconds, 
												size_t milliseconds)
	{
		return  (hours * 36000000000) +
				(minutes * 600000000) + 
				(seconds * 10000000) + 
				(milliseconds * 10000);
	}

	inline float ConvertFontPointToDIP(size_t pt)
	{
		// Note to self: See the following link for explanation of magic numbers:
		// http://msdn.microsoft.com/en-us/library/ff684173(v=vs.85).aspx

		static const float conversion_factor = 96.0f/72.0f;
		return static_cast<float>(pt) * conversion_factor;
	}
};