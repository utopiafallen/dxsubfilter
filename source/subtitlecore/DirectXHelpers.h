#include <D2DBaseTypes.h>

template <class Interface>
inline void SafeRelease(Interface** pInterfaceToRelease)
{
	if (*pInterfaceToRelease)
	{
		(*pInterfaceToRelease)->Release();
		(*pInterfaceToRelease) = nullptr;
	}
}

inline D2D_COLOR_F ConvertABGRToD2DCOLORF(unsigned int ABGR)
{
	static const float normalize = 1.0f/255.0f;
	D2D_COLOR_F result;
	result.r = static_cast<float>(ABGR & 0x000000ff) * normalize;
	result.g = static_cast<float>((ABGR & 0x0000ff00) >> 8) * normalize;
	result.b = static_cast<float>((ABGR & 0x00ff0000) >> 16) * normalize;
	result.a = static_cast<float>((ABGR & 0xff000000) >> 24) * normalize;

	return result;
}