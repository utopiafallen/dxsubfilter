#include "stdafx.h"
#include "SubPicBlender.h"

using namespace DXSubFilter;

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