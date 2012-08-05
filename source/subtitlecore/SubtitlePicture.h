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
		size_t m_uWidth;
		size_t m_uHeight;


		// Stride m_Data. Measured in bytes.
		size_t m_uStride;

		// Used to indicate what kind of data is pointed to by the underlying byte-stream.
		SubtitlePictureFormat m_SubPicFormat;

		// Raw image data. How to interpret this data depends on the SubtitlePictureFormat
		// specified. Data is guaranteed to be 16-byte aligned.
		std::shared_ptr<unsigned char> m_Data;

		// Custom deleter for m_Data. Needed for shared_ptrs.
		template <typename T>
		struct Deleter
		{
			void operator()(T* ptr)
			{
				_aligned_free(ptr);
				ptr = nullptr;
			}
		};

		SubtitlePicture() 
			: m_iOriginX(0), m_iOriginY(0)
			, m_uWidth(0U), m_uHeight(0U)
			, m_uStride(0)
			, m_SubPicFormat(SBPF_PBGRA32)
			, m_Data(nullptr)
		{
		}

		SubtitlePicture(int originX, int originY, size_t width, size_t height, size_t stride,
			SubtitlePictureFormat sbpf, unsigned char* data)
			: m_iOriginX(originX), m_iOriginY(originY)
			, m_uWidth(width), m_uHeight(height)
			, m_uStride(stride)
			, m_SubPicFormat(sbpf)
			, m_Data(data, Deleter<unsigned char>())
		{
		}

		bool operator== (const SubtitlePicture& other)
		{
			return m_iOriginX == other.m_iOriginX &&
				m_iOriginY == other.m_iOriginY &&
				m_uHeight == other.m_uHeight &&
				m_uWidth == other.m_uWidth &&
				m_SubPicFormat == other.m_SubPicFormat &&
				m_Data == other.m_Data;
		}
	};
};
#endif