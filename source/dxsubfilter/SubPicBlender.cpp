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