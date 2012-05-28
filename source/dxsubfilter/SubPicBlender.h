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

	class __declspec(novtable) SubPicBlender
	{
	public:
		// Note that width must equal stride for destination data.
		inline virtual void operator()(SubtitleCore::SubtitlePicture* subpic, BYTE* pData,
			size_t dstWidth, size_t dstHeight) = 0;
	protected:
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

			return ((77 * R + 150 * G + 29 * B + 128) >> 8);
		}

		__forceinline static short ConvertBGRAToUBT601(unsigned int BGRA)
		{
			short R = (BGRA & 0x00FF0000) >> 16;
			short G = (BGRA & 0x0000FF00) >> 8;
			short B = (BGRA & 0x000000FF);

			return ((-37 * R + -74 * G + 111 * B + 128) >> 8) + 128;
		}

		__forceinline static short ConvertBGRAToVBT601(unsigned int BGRA)
		{
			short R = (BGRA & 0x00FF0000) >> 16;
			short G = (BGRA & 0x0000FF00) >> 8;
			short B = (BGRA & 0x000000FF);

			return (( 112 * R -  94 * G -  18 * B + 128) >> 8) + 128;
		}
	};

	class BlendBGRAWithNV12BT601 : public SubPicBlender
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

			// 4:2:0 downsampling
			size_t uUVPlaneHeight = (uSubPicHeight-1) / 2;
			size_t uUVPlaneWidth = (uSubPicWidth-1) / 2;

			int iDstOffsetYUV = iDstOffsetY / 2;

			// In case dimensions are not divisible by our unrolled processing size
			size_t uSubPicUnrolledWidth = (uSubPicWidth / 8) * 8;

			// Blend Y plane
			Concurrency::parallel_for(0U, uSubPicHeight, [&](size_t y)
			//for (size_t y = 0; y < uSubPicHeight; y++)
			{
				for (size_t x = 0; x < uSubPicUnrolledWidth; x+=8)
				{
					BYTE* __restrict pSrc = pSubPicData + uSubPicStride * y + x * 4; // 4 bytes per pixel
					BYTE* __restrict pDst = pData + dstWidth * (iDstOffsetY + y) + x + iDstOffsetX;

					unsigned int* __restrict pBGRAData = reinterpret_cast<unsigned int*>(pSrc);

					// Pixel 1
					unsigned int BGRA = pBGRAData[0];

					short A = (BGRA & 0xFF000000) >> 24;

					pDst[0] = static_cast<unsigned char>(ConvertBGRAToYBT601(BGRA) + 
																((pDst[0] * (255-A)) >> 8));

					// Pixel 2
					BGRA = pBGRAData[1];

					A = (BGRA & 0xFF000000) >> 24;

					pDst[1] = static_cast<unsigned char>(ConvertBGRAToYBT601(BGRA) + 
																((pDst[1] * (255-A)) >> 8));

					// Pixel 3
					BGRA = pBGRAData[2];

					A = (BGRA & 0xFF000000) >> 24;

					pDst[2] = static_cast<unsigned char>(ConvertBGRAToYBT601(BGRA) + 
																((pDst[2] * (255-A)) >> 8));

					// Pixel 4
					BGRA = pBGRAData[3];

					A = (BGRA & 0xFF000000) >> 24;

					pDst[3] = static_cast<unsigned char>(ConvertBGRAToYBT601(BGRA) + 
																((pDst[3] * (255-A)) >> 8));

					// Pixel 5
					BGRA = pBGRAData[4];

					A = (BGRA & 0xFF000000) >> 24;

					pDst[4] = static_cast<unsigned char>(ConvertBGRAToYBT601(BGRA) + 
																((pDst[4] * (255-A)) >> 8));

					// Pixel 6
					BGRA = pBGRAData[5];

					A = (BGRA & 0xFF000000) >> 24;

					pDst[5] = static_cast<unsigned char>(ConvertBGRAToYBT601(BGRA) + 
																((pDst[5] * (255-A)) >> 8));

					// Pixel 7
					BGRA = pBGRAData[6];

					A = (BGRA & 0xFF000000) >> 24;

					pDst[6] = static_cast<unsigned char>(ConvertBGRAToYBT601(BGRA) + 
																((pDst[6] * (255-A)) >> 8));

					// Pixel 8
					BGRA = pBGRAData[7];

					A = (BGRA & 0xFF000000) >> 24;

					pDst[7] = static_cast<unsigned char>(ConvertBGRAToYBT601(BGRA) + 
																((pDst[7] * (255-A)) >> 8));
				}

				for (size_t x = uSubPicUnrolledWidth; x < uSubPicWidth; x++)
				{
					BYTE* __restrict pSrc = pSubPicData + uSubPicStride * y + x * 4; // 4 bytes per pixel
					BYTE* __restrict pDst = pData + dstWidth * (iDstOffsetY + y) + x + iDstOffsetX;

					unsigned int* pBGRAData = reinterpret_cast<unsigned int*>(pSrc);

					unsigned int BGRA = pBGRAData[0];

					short A = (BGRA & 0xFF000000) >> 24;

					pDst[0] = static_cast<unsigned char>(ConvertBGRAToYBT601(BGRA) + 
																((pDst[0] * (255-A)) >> 8));
				}
			});

			// Blend UV plane
			//Concurrency::parallel_for(0U, uUVPlaneHeight, [&](size_t y)
			for (size_t y = 0; y < uUVPlaneHeight; y++)
			{
				for (size_t x = 0; x < uUVPlaneWidth; x++)
				{
					BYTE* __restrict pSrc = pSubPicData + uSubPicStride * y * 2 + x * 8; // 2 pixels @ 4 bytes per pixel 
					BYTE* __restrict pDst = pUVPlane + dstWidth * (iDstOffsetYUV + y) + (x*2) + iDstOffsetX ;

					unsigned int* pBGRAData = reinterpret_cast<unsigned int*>(pSrc);

					// Get 2x2 pixel block
					unsigned int BGRA1 = pBGRAData[0];
					unsigned int BGRA2 = pBGRAData[1];
					unsigned int BGRA3 = pBGRAData[uSubPicWidth];
					unsigned int BGRA4 = pBGRAData[uSubPicWidth + 1];

					short A1 = (BGRA1 & 0xFF000000) >> 24;
					short A2 = (BGRA2 & 0xFF000000) >> 24;
					short A3 = (BGRA3 & 0xFF000000) >> 24;
					short A4 = (BGRA4 & 0xFF000000) >> 24;

					short U1 = ConvertBGRAToUBT601(BGRA1);
					short U2 = ConvertBGRAToUBT601(BGRA2);
					short U3 = ConvertBGRAToUBT601(BGRA3);
					short U4 = ConvertBGRAToUBT601(BGRA4);

					short V1 = ConvertBGRAToVBT601(BGRA1);
					short V2 = ConvertBGRAToVBT601(BGRA2);
					short V3 = ConvertBGRAToVBT601(BGRA3);
					short V4 = ConvertBGRAToVBT601(BGRA4);

					// Average results
					short finalA = max_nb(A1, max_nb(A2, max_nb(A3,A4)));
					short finalU = (U1 + U2 + U3 + U4) / 4;
					short finalV = (V1 + V2 + V3 + V4) / 4;

					UNREFERENCED_PARAMETER(finalA);
					UNREFERENCED_PARAMETER(finalU);
					UNREFERENCED_PARAMETER(finalV);
					UNREFERENCED_PARAMETER(pDst);
					// Blend results
					//pDst[0] = static_cast<BYTE>((finalU * finalA + pDst[0] * (255 - finalA)) >> 8);
					//pDst[1] = static_cast<BYTE>((finalV * finalA + pDst[1] * (255 - finalA)) >> 8);
				}
			}
		}
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