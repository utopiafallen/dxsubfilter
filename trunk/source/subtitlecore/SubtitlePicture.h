#ifndef SUBTITLEPICTURE_H
#define SUBTITLEPICTURE_H
#pragma once

#include "SubtitleCoreEnumerations.h"

namespace SubtitleCore
{
	// SubtitlePicture
	//	Structure to contain description about a specific subtitle picture and a pointer to
	//	raw data.
	//
	//	NOTE: a consumer should not hold on to the data pointer beyond when the SubtitlePicture
	//	was returned. There are no guarantees made about the validity of the data unless 
	//	otherwise noted by the renderer.
	struct SubtitlePicture
	{
		// Origin of where the image should be drawn. Given in pixel coordinates relative to the
		// video frame size. (0,0) is always top-left corner of video. If the values are negative,
		// the image is meant to be drawn starting from outside of the video frame.
		int m_iOriginX;
		int m_iOriginY;

		// Width and height of the image in pixels. It is guaranteed that the
		size_t m_iWidth;
		size_t m_iHeight;

		// Used to indicate what kind of data is pointed to by the underlying byte-stream.
		SubtitlePictureFormat m_SubPicFormat;

		// Raw image data. How to interpret this data depends on the SubtitlePictureFormat
		// specified.
		unsigned char* m_Data;

		SubtitlePicture() 
			: m_iOriginX(0), m_iOriginY(0)
			, m_iWidth(0U), m_iHeight(0U)
			, m_SubPicFormat(SBPF_RGBA32)
			, m_Data(nullptr)
		{
		}
	};
};
#endif