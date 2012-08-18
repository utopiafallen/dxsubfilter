#include "stdafx.h"
#include "SubPicBlender.h"

using namespace DXSubFilter;

namespace DXSubFilter
{
	_MM_ALIGN16 short YRConvTableBT601[256];
	_MM_ALIGN16 short YGConvTableBT601[256];
	_MM_ALIGN16 short YBConvTableBT601[256];

	_MM_ALIGN16 short URConvTableBT601[256];
	_MM_ALIGN16 short UGConvTableBT601[256];
	_MM_ALIGN16 short UBConvTableBT601[256];

	_MM_ALIGN16 short VRConvTableBT601[256];
	_MM_ALIGN16 short VGConvTableBT601[256];
	_MM_ALIGN16 short VBConvTableBT601[256];

	// [AlphaValue][SrcValue]
	_MM_ALIGN16 short AlphaBlendTable[256][256];
}

__m128i SubPicBlender::YCoeffBT601 = _mm_set_epi16(0, 77, 150, 29, 0, 77, 150, 29);
__m128i SubPicBlender::UCoeffBT601 = _mm_set_epi16(0, -37, -74, 111, 0, -37, -74, 111);
__m128i SubPicBlender::VCoeffBT601 = _mm_set_epi16(0, 112, -94, -18, 0, 112, -94, -18);

__m128i SubPicBlender::YCoeffBT709 = _mm_set_epi16(0, 54, 182, 19, 0, 54, 182, 19);
__m128i SubPicBlender::UCoeffBT709 = _mm_set_epi16(0, -25, -86, 111, 0, -25, -86, 111);
__m128i SubPicBlender::VCoeffBT709 = _mm_set_epi16(0, 157, -142, -14, 0, 157, -142, -14);

__m128i SubPicBlender::YCoeff16BT601 = _mm_set_epi32(0, 77, 150, 29);
__m128i SubPicBlender::UCoeff16BT601 = _mm_set_epi32(0, -37, -74, 111);
__m128i SubPicBlender::VCoeff16BT601 = _mm_set_epi32(0, 157, -131, -26);

__m128i SubPicBlender::YCoeff16BT709 = _mm_set_epi32(0, 54, 182, 19);
__m128i SubPicBlender::UCoeff16BT709 = _mm_set_epi32(0, -25, -86, 111);
__m128i SubPicBlender::VCoeff16BT709 = _mm_set_epi32(0, 157, -142, -14);

__m128i SubPicBlender::zero = _mm_setzero_si128();

bool SubPicBlender::m_bConvTablesInitialized = false;

SubPicBlender::SubPicBlender()
{
	if (m_bConvTablesInitialized == false)
	{
		m_bConvTablesInitialized = true;

		for (short i = 0; i < 256; i++)
		{
			YRConvTableBT601[i] = 77 * i;
			YGConvTableBT601[i] = 150 * i;
			YBConvTableBT601[i] = 29 * i;

			URConvTableBT601[i] = -37 * i;
			UGConvTableBT601[i] = -74 * i;
			UBConvTableBT601[i] = 111 * i;

			VRConvTableBT601[i] = 112 * i;
			VGConvTableBT601[i] = -94 * i;
			VBConvTableBT601[i] = -18 * i;

			for (short j = 0; j < 256; j++)
			{
				AlphaBlendTable[i][j] = (i * j) >> 8;
			}
		}
	}
}

void BlendBGRAWithNV12BT601::operator()(SubtitleCore::SubtitlePicture* subpic, BYTE* pData,
			size_t dstWidth, size_t dstHeight)
{
	BYTE* pSubPicData = subpic->m_Data.get();
	BYTE* pUVPlane = pData + dstWidth * dstHeight; // NV12 has UV data packed into the same plane

	size_t uSubPicStride = subpic->m_uStride;
	size_t uSubPicWidth = subpic->m_uWidth;
	size_t uSubPicHeight = subpic->m_uHeight;
	size_t uSubPicBGRAStride = uSubPicStride / 4;

	int iDstOffsetX = subpic->m_iOriginX;
	int iDstOffsetY = subpic->m_iOriginY;

	// 4:2:0 downsampling
	size_t uUVPlaneHeight = (uSubPicHeight-1) / 2;
	size_t uUVPlaneWidth = (uSubPicWidth-1) / 2;

	int iDstOffsetXUV = iDstOffsetX / 2;
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
														AlphaBlendTable[pDst[0]][(255-A)]);

			// Pixel 2
			BGRA = pBGRAData[1];

			A = (BGRA & 0xFF000000) >> 24;

			pDst[1] = static_cast<unsigned char>(ConvertBGRAToYBT601(BGRA) + 
														AlphaBlendTable[pDst[1]][(255-A)]);

			// Pixel 3
			BGRA = pBGRAData[2];

			A = (BGRA & 0xFF000000) >> 24;

			pDst[2] = static_cast<unsigned char>(ConvertBGRAToYBT601(BGRA) + 
														AlphaBlendTable[pDst[2]][(255-A)]);

			// Pixel 4
			BGRA = pBGRAData[3];

			A = (BGRA & 0xFF000000) >> 24;

			pDst[3] = static_cast<unsigned char>(ConvertBGRAToYBT601(BGRA) + 
														AlphaBlendTable[pDst[3]][(255-A)]);

			// Pixel 5
			BGRA = pBGRAData[4];

			A = (BGRA & 0xFF000000) >> 24;

			pDst[4] = static_cast<unsigned char>(ConvertBGRAToYBT601(BGRA) + 
														AlphaBlendTable[pDst[4]][(255-A)]);

			// Pixel 6
			BGRA = pBGRAData[5];

			A = (BGRA & 0xFF000000) >> 24;

			pDst[5] = static_cast<unsigned char>(ConvertBGRAToYBT601(BGRA) + 
														AlphaBlendTable[pDst[5]][(255-A)]);

			// Pixel 7
			BGRA = pBGRAData[6];

			A = (BGRA & 0xFF000000) >> 24;

			pDst[6] = static_cast<unsigned char>(ConvertBGRAToYBT601(BGRA) + 
														AlphaBlendTable[pDst[6]][(255-A)]);

			// Pixel 8
			BGRA = pBGRAData[7];

			A = (BGRA & 0xFF000000) >> 24;

			pDst[7] = static_cast<unsigned char>(ConvertBGRAToYBT601(BGRA) + 
														AlphaBlendTable[pDst[7]][(255-A)]);
		}

		for (size_t x = uSubPicUnrolledWidth; x < uSubPicWidth; x++)
		{
			BYTE* __restrict pSrc = pSubPicData + uSubPicStride * y + x * 4; // 4 bytes per pixel
			BYTE* __restrict pDst = pData + dstWidth * (iDstOffsetY + y) + x + iDstOffsetX;

			unsigned int* pBGRAData = reinterpret_cast<unsigned int*>(pSrc);

			unsigned int BGRA = pBGRAData[0];

			short A = (BGRA & 0xFF000000) >> 24;

			pDst[0] = static_cast<unsigned char>(ConvertBGRAToYBT601(BGRA) + 
														AlphaBlendTable[pDst[0]][(255-A)]);
		}
	});

	// Blend UV plane
	Concurrency::parallel_for(0U, uUVPlaneHeight, [&](size_t y)
	//for (size_t y = 0; y < uUVPlaneHeight; y++)
	{
		for (size_t x = 0; x < uUVPlaneWidth; x++)
		{
			BYTE* __restrict pSrc = pSubPicData + uSubPicStride * y * 2 + x * 8; // 2 pixels @ 4 bytes per pixel 
			BYTE* __restrict pDst = pUVPlane + dstWidth * (iDstOffsetYUV + y) + (x+iDstOffsetXUV) *2 ;

			unsigned int* pBGRAData = reinterpret_cast<unsigned int*>(pSrc);

			// Get left 2 pixels. MPEG-2 only samples the left values
			unsigned int BGRA1 = pBGRAData[0];
			unsigned int BGRA2 = pBGRAData[uSubPicBGRAStride];

			short A1 = (BGRA1 & 0xFF000000) >> 24;
			short A2 = (BGRA2 & 0xFF000000) >> 24;

			short U1 = ConvertBGRAToUBT601(BGRA1);
			short U2 = ConvertBGRAToUBT601(BGRA2);

			short V1 = ConvertBGRAToVBT601(BGRA1);
			short V2 = ConvertBGRAToVBT601(BGRA2);

			// Average results.
			short finalA = (A1 + A2) >> 1;
			short finalU = (U1 + U2) >> 1;
			short finalV = (V1 + V2) >> 1;

			// Blend results. The unbiasing/rebiasing is due to the biasing applied by MPEG2 standards
			// so that UV values are always positive.
			short dstU = (finalU + (((pDst[0]-128) * (255 - finalA)) >> 8)) + 128;
			short dstV = (finalV + (((pDst[1]-128) * (255 - finalA)) >> 8)) + 128;
					
			pDst[0] = static_cast<BYTE>(min_nb<short>(dstU, 255));
			pDst[1] = static_cast<BYTE>(min_nb<short>(dstV, 255));
		}
	});
}

void BlendBGRAWithYV12BT601::operator()(SubtitleCore::SubtitlePicture* subpic, BYTE* pData,
			size_t dstWidth, size_t dstHeight)
{
	BYTE* pSubPicData = subpic->m_Data.get();
	BYTE* pVPlane = pData + dstWidth * dstHeight; // YV12 has U and V on separate planes
	BYTE* pUPlane = pVPlane + (dstWidth * dstHeight) / 4;

	size_t uUVStride = dstWidth / 2;

	size_t uSubPicStride = subpic->m_uStride;
	size_t uSubPicWidth = subpic->m_uWidth;
	size_t uSubPicHeight = subpic->m_uHeight;
	size_t uSubPicBGRAStride = uSubPicStride / 4;

	int iDstOffsetX = subpic->m_iOriginX;
	int iDstOffsetY = subpic->m_iOriginY;

	// 4:2:0 downsampling
	size_t uUVPlaneHeight = (uSubPicHeight-1) / 2;
	size_t uUVPlaneWidth = (uSubPicWidth-1) / 2;

	int iDstOffsetXUV = iDstOffsetX / 2;
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
														AlphaBlendTable[pDst[0]][(255-A)]);

			// Pixel 2
			BGRA = pBGRAData[1];

			A = (BGRA & 0xFF000000) >> 24;

			pDst[1] = static_cast<unsigned char>(ConvertBGRAToYBT601(BGRA) + 
														AlphaBlendTable[pDst[1]][(255-A)]);

			// Pixel 3
			BGRA = pBGRAData[2];

			A = (BGRA & 0xFF000000) >> 24;

			pDst[2] = static_cast<unsigned char>(ConvertBGRAToYBT601(BGRA) + 
														AlphaBlendTable[pDst[2]][(255-A)]);

			// Pixel 4
			BGRA = pBGRAData[3];

			A = (BGRA & 0xFF000000) >> 24;

			pDst[3] = static_cast<unsigned char>(ConvertBGRAToYBT601(BGRA) + 
														AlphaBlendTable[pDst[3]][(255-A)]);

			// Pixel 5
			BGRA = pBGRAData[4];

			A = (BGRA & 0xFF000000) >> 24;

			pDst[4] = static_cast<unsigned char>(ConvertBGRAToYBT601(BGRA) + 
														AlphaBlendTable[pDst[4]][(255-A)]);

			// Pixel 6
			BGRA = pBGRAData[5];

			A = (BGRA & 0xFF000000) >> 24;

			pDst[5] = static_cast<unsigned char>(ConvertBGRAToYBT601(BGRA) + 
														AlphaBlendTable[pDst[5]][(255-A)]);

			// Pixel 7
			BGRA = pBGRAData[6];

			A = (BGRA & 0xFF000000) >> 24;

			pDst[6] = static_cast<unsigned char>(ConvertBGRAToYBT601(BGRA) + 
														AlphaBlendTable[pDst[6]][(255-A)]);

			// Pixel 8
			BGRA = pBGRAData[7];

			A = (BGRA & 0xFF000000) >> 24;

			pDst[7] = static_cast<unsigned char>(ConvertBGRAToYBT601(BGRA) + 
														AlphaBlendTable[pDst[7]][(255-A)]);
		}

		for (size_t x = uSubPicUnrolledWidth; x < uSubPicWidth; x++)
		{
			BYTE* __restrict pSrc = pSubPicData + uSubPicStride * y + x * 4; // 4 bytes per pixel
			BYTE* __restrict pDst = pData + dstWidth * (iDstOffsetY + y) + x + iDstOffsetX;

			unsigned int* pBGRAData = reinterpret_cast<unsigned int*>(pSrc);

			unsigned int BGRA = pBGRAData[0];

			short A = (BGRA & 0xFF000000) >> 24;

			pDst[0] = static_cast<unsigned char>(ConvertBGRAToYBT601(BGRA) + 
														AlphaBlendTable[pDst[0]][(255-A)]);
		}
	});

	// Blend UV plane
	Concurrency::parallel_for(0U, uUVPlaneHeight, [&](size_t y)
	//for (size_t y = 0; y < uUVPlaneHeight; y++)
	{
		for (size_t x = 0; x < uUVPlaneWidth; x++)
		{
			BYTE* __restrict pSrc = pSubPicData + uSubPicStride * y * 2 + x * 8; // 2 pixels @ 4 bytes per pixel 
			BYTE* __restrict pDstU = pUPlane + uUVStride * (iDstOffsetYUV + y) + (x+iDstOffsetXUV);
			BYTE* __restrict pDstV = pVPlane + uUVStride * (iDstOffsetYUV + y) + (x+iDstOffsetXUV);

			unsigned int* pBGRAData = reinterpret_cast<unsigned int*>(pSrc);

			// Get left 2 pixels. MPEG-2 only samples the left values
			unsigned int BGRA1 = pBGRAData[0];
			unsigned int BGRA2 = pBGRAData[uSubPicBGRAStride];

			short A1 = (BGRA1 & 0xFF000000) >> 24;
			short A2 = (BGRA2 & 0xFF000000) >> 24;

			short U1 = ConvertBGRAToUBT601(BGRA1);
			short U2 = ConvertBGRAToUBT601(BGRA2);

			short V1 = ConvertBGRAToVBT601(BGRA1);
			short V2 = ConvertBGRAToVBT601(BGRA2);

			// Average results.
			short finalA = (A1 + A2) >> 1;
			short finalU = (U1 + U2) >> 1;
			short finalV = (V1 + V2) >> 1;

			// Blend results. The unbiasing/rebiasing is due to the biasing applied by MPEG2 standards
			// so that UV values are always positive.
			short dstU = (finalU + (((*pDstU-128) * (255 - finalA)) >> 8)) + 128;
			short dstV = (finalV + (((*pDstV-128) * (255 - finalA)) >> 8)) + 128;
					
			*pDstU = static_cast<BYTE>(min_nb<short>(dstU, 255));
			*pDstV = static_cast<BYTE>(min_nb<short>(dstV, 255));
		}
	});
}