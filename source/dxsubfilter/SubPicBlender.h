#ifndef SUBPICBLENDER_H
#define SUBPICBLENDER_H
#pragma once

#include "stdafx.h"
#include "SubtitlePicture.h"

namespace DXSubFilter
{
	// We define our own min/max functions that uses bit twiddling rather than the trinary
	// operator to avoid potentially generating branches.
	template<typename T>
	__forceinline T min_nb(T x, T y)
	{
		return y ^ ((x ^ y) & -(x < y));
	}

	template<typename T>
	__forceinline T max_nb(T x, T y)
	{
		return x ^ ((x ^ y) & -(x < y));
	}

	// RGB to BT.601 8-bit YUV conversion. Right-shift all results 8 bits.
	// [Y]		[77		150		29]	[R]
	// [U]	=	[-37	-74	   111] [G]
	// [V]		[112	-94	   -18]	[B]

	// RGB to BT.709 8-bit YUV conversion. Right-shift all results 8 bits.
	// [Y]		[54		182		19]	[R]
	// [U]	=	[-25	-86	   111] [G]
	// [V]		[157	-142   -14]	[B]

	// Use look up tables rather than perform actual multiplication. Made available in the
	// header in case I need these elsewhere.
	_MM_ALIGN16 extern short YRConvTableBT601[256];
	_MM_ALIGN16 extern short YGConvTableBT601[256];
	_MM_ALIGN16 extern short YBConvTableBT601[256];

	_MM_ALIGN16 extern short URConvTableBT601[256];
	_MM_ALIGN16 extern short UGConvTableBT601[256];
	_MM_ALIGN16 extern short UBConvTableBT601[256];

	_MM_ALIGN16 extern short VRConvTableBT601[256];
	_MM_ALIGN16 extern short VGConvTableBT601[256];
	_MM_ALIGN16 extern short VBConvTableBT601[256];

	// [SrcValue][AlphaValue]
	_MM_ALIGN16 extern short AlphaBlendTable[256][256];

	class __declspec(novtable) SubPicBlender
	{
	public:
		// Note that width must equal stride for destination data.
		inline virtual void operator()(SubtitleCore::SubtitlePicture* subpic, BYTE* pData,
			size_t dstWidth, size_t dstHeight) = 0;
	protected:
		SubPicBlender();

		// Stored assuming BGR order
		static __m128i YCoeffBT601;
		static __m128i UCoeffBT601;
		static __m128i VCoeffBT601;

		static __m128i YCoeffBT709;
		static __m128i UCoeffBT709;
		static __m128i VCoeffBT709;

		// Same as the above, but 32-bits so we can left-shift the result by 16-bits.
		// Used for 10/16-bit YUV formats
		static __m128i YCoeff16BT601;
		static __m128i UCoeff16BT601;
		static __m128i VCoeff16BT601;

		static __m128i YCoeff16BT709;
		static __m128i UCoeff16BT709;
		static __m128i VCoeff16BT709;

		static __m128i zero;

		static bool m_bConvTablesInitialized;

		__forceinline static __m128i _mm_dot_epi16(__m128i a, __m128i b)
		{
			__m128i result = _mm_mullo_epi16(a, b);

			// 127    0
			// X Y W Z
			__m128i temp = _mm_shufflelo_epi16(result, _MM_SHUFFLE(1, 0, 3, 2));
			temp = _mm_shufflehi_epi16(temp, _MM_SHUFFLE(1, 0, 3, 2));

			result = _mm_add_epi16(result, temp);

			temp = _mm_shufflelo_epi16(result, _MM_SHUFFLE(2, 3, 0, 1));
			temp = _mm_shufflehi_epi16(temp, _MM_SHUFFLE(2, 3, 0, 1));

			return _mm_add_epi16(result, temp);
		}

		// BGRA has B at LSB.
		__forceinline static short ConvertBGRAToYBT601(unsigned int BGRA)
		{
			short R = (BGRA & 0x00FF0000) >> 16;
			short G = (BGRA & 0x0000FF00) >> 8;
			short B = (BGRA & 0x000000FF);

			return (YRConvTableBT601[R] + YGConvTableBT601[G] + YBConvTableBT601[B] + 128) >> 8;
		}

		__forceinline static short ConvertBGRAToUBT601(unsigned int BGRA)
		{
			short R = (BGRA & 0x00FF0000) >> 16;
			short G = (BGRA & 0x0000FF00) >> 8;
			short B = (BGRA & 0x000000FF);

			return (URConvTableBT601[R] + UGConvTableBT601[G] + UBConvTableBT601[B] + 128) >> 8;
		}

		__forceinline static short ConvertBGRAToVBT601(unsigned int BGRA)
		{
			short R = (BGRA & 0x00FF0000) >> 16;
			short G = (BGRA & 0x0000FF00) >> 8;
			short B = (BGRA & 0x000000FF);

			return (VRConvTableBT601[R] + VGConvTableBT601[G] + VBConvTableBT601[B] + 128) >> 8;
		}
	};

	class BlendBGRAWithNV12BT601 : public SubPicBlender
	{
	public:
		virtual void operator()(SubtitleCore::SubtitlePicture* subpic, BYTE* pData,
			size_t dstWidth, size_t dstHeight);
	};

	class BlendBGRAWithYV12BT601 : public SubPicBlender
	{
		virtual void operator()(SubtitleCore::SubtitlePicture* subpic, BYTE* pData,
			size_t dstWidth, size_t dstHeight);
	};

	// Keeping this around as backup
	class BlendBGRAWithNV12BT601SSE2 : public SubPicBlender
	{
	public:
		inline virtual void operator()(SubtitleCore::SubtitlePicture* subpic, BYTE* pData,
			size_t dstWidth, size_t dstHeight)
		{
			BYTE* pSubPicData = subpic->m_Data.get();
			BYTE* pUVPlane = pData + dstWidth * dstHeight; // NV12 has UV data packed into the same plane

			size_t uSubPicStride = subpic->m_uStride;
			size_t uSubPicWidth = subpic->m_uWidth;
			size_t uSubPicHeight = subpic->m_uHeight;

			int iDstOffsetX = subpic->m_iOriginX;
			int iDstOffsetY = subpic->m_iOriginY;

			// In case dimensions are not divisible by our SSE2 processing size
			size_t uSubPicWidthSSE2 = (uSubPicWidth / 8) * 8;

			// Blend with Y plane first
			// Get BGRA data and split into RGB and A registers. Each read gets us 4 pixels of 
			// data and each xmm register can store 2 pixels during conversion. We'll load 8
			// pixels worth of data and blend them at once.
			__m128i xmmSubPicBGR1, xmmSubPicBGR2, xmmSubPicBGR3, xmmSubPicBGR4, xmmSubPicAlpha;
			__m128i xmmData1, xmmData2;

			Concurrency::parallel_for(0U, uSubPicHeight, [&](size_t y)
			{
				for (size_t x = 0; x < uSubPicWidthSSE2; x+=8)
				{
					BYTE* pSrc = pSubPicData + uSubPicStride * y + x * 4; // 4 bytes per pixel
					BYTE* pDst = pData + dstWidth * (iDstOffsetY + y) + x + iDstOffsetX;

					unsigned int* pTest = reinterpret_cast<unsigned int*>(pSrc);
					if (pTest[0] != 0 || pTest[1] != 0 || pTest[2] != 0 || pTest[3] != 0 ||
						pTest[4] != 0 || pTest[5] != 0 || pTest[6] != 0 || pTest[7] != 0 )
					{
						// Load first 4 pixels
						xmmSubPicBGR1 = _mm_load_si128((__m128i*)pSrc);

						// Seperate the 8-bit components into 16-bit sections of XMM registers
						xmmSubPicBGR3 = _mm_unpacklo_epi8(xmmSubPicBGR1, zero);
						xmmSubPicBGR4 = _mm_unpackhi_epi8(xmmSubPicBGR1, zero);

						// Load second 4 pixels and unpack
						xmmSubPicBGR1 = _mm_load_si128((__m128i*)(pSrc + 16));
						xmmSubPicBGR2 = _mm_unpacklo_epi8(xmmSubPicBGR1, zero);
						xmmSubPicBGR1 = _mm_unpackhi_epi8(xmmSubPicBGR1, zero);

						// It looks rather hard to get all the alpha values isolated into their own
						// register so unless this proves to be really slow, I'm going to do this..
						_MM_ALIGN16 short alphas[8] = {
							255 - pSrc[3],
							255 - pSrc[7],
							255 - pSrc[11],
							255 - pSrc[15],
							255 - pSrc[19],
							255 - pSrc[23],
							255 - pSrc[27],
							255 - pSrc[31]
						};
						xmmSubPicAlpha = _mm_load_si128((__m128i*)alphas);

						// This might be really slow, but at least we're doing 2 dot products
						// at once..
						xmmSubPicBGR1 = _mm_dot_epi16(xmmSubPicBGR1, YCoeffBT601);
						xmmSubPicBGR2 = _mm_dot_epi16(xmmSubPicBGR2, YCoeffBT601);
						xmmSubPicBGR3 = _mm_dot_epi16(xmmSubPicBGR3, YCoeffBT601);
						xmmSubPicBGR4 = _mm_dot_epi16(xmmSubPicBGR4, YCoeffBT601);

						// Get all the Y values in one register in the right order
						xmmData1 = _mm_unpacklo_epi16(xmmSubPicBGR1, xmmSubPicBGR2);
						xmmData1 = _mm_unpackhi_epi16(xmmData1, xmmSubPicBGR1);
						xmmData1 = _mm_unpackhi_epi16(xmmData1, xmmSubPicBGR2);

						xmmData2 = _mm_unpacklo_epi16(xmmSubPicBGR3, xmmSubPicBGR4);
						xmmData2 = _mm_unpackhi_epi16(xmmData2, xmmSubPicBGR3);
						xmmData2 = _mm_unpackhi_epi16(xmmData2, xmmSubPicBGR4);

						xmmData1 = _mm_unpacklo_epi16(xmmData2, xmmData1);

						// Read in the Y values
						xmmData2 = _mm_loadu_si128((__m128i*)pDst);

						// Only care about the first 64-bits data since we're blending 8
						// pixels
						xmmData2 = _mm_unpacklo_epi8(xmmData2, zero);

						// Alpha blend
						xmmData2 = _mm_mullo_epi16(xmmData2, xmmSubPicAlpha);
						xmmData2 = _mm_add_epi16(xmmData2, xmmData1);
						xmmData2 = _mm_srli_epi16(xmmData2, 8);

						// Pack in data and extract
						xmmData2 = _mm_packus_epi16(xmmData2, zero);
						_mm_storel_epi64((__m128i*)pDst, xmmData2);

						UNREFERENCED_PARAMETER(pUVPlane);
					}
				}

				for (size_t x = uSubPicWidthSSE2; x < uSubPicWidth; x++)
				{
					BYTE* pSrc = pSubPicData + uSubPicStride * y + x * 4; // 4 bytes per pixel
					BYTE* pDst = pData + dstWidth * (iDstOffsetY + y) + x + iDstOffsetX;

					unsigned int BGRA = *(reinterpret_cast<unsigned int*>(pSrc));

					short A = (BGRA & 0xFF000000) >> 24;

					short result = ConvertBGRAToYBT601(BGRA) + ((*pDst * (255-A)) >> 8);

					*pDst = static_cast<unsigned char>(result);
				}
			});
		}
	};
};

#endif