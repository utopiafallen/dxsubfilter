// Math utilities
#ifndef SCUMATH_H
#define SCUMATH_H
#pragma once

#include <xmmintrin.h>
#include <emmintrin.h>

// SCU - SubtitleCoreUtilities
namespace SCU
{
	// Computes intersection point of two lines in 2D. This function does not handle special cases where
	// the lines never interesect or the lines are colinear.
	// NB: For efficiency, the start and end points are grouped together into a 16-byte aligned array.
	__forceinline void LineLineIntersectSSE2(float startPoints[4], float endPoints[4], float& outIntersectX, float& outIntersectY)
	{
		_MM_ALIGN16 float result[4];

		__m128 input1 = _mm_load_ps(startPoints);
		__m128 input2 = _mm_load_ps(endPoints);

		__m128 coeff = _mm_sub_ps(input1, input2);							// (p0.x - p1.x), (p0.y - p1.y), (p2.x - p3.x), (p2.y - p3.y)

		__m128 temp = _mm_shuffle_ps(input2, input2, _MM_SHUFFLE(2,3,0,1)); // p1.y, p1.x, p3.y, p3.x
		__m128 det = _mm_mul_ps(input1, temp);								// p0.x * p1.y, p0.y * p1.x, p2.x * p3.y, p2.y * p3.x
		temp = _mm_shuffle_ps(det, det, _MM_SHUFFLE(2,3,0,1));				// p0.y * p1.x, p0.x * p1.y, p2.y * p3.x, p2.x * p3.y
		det = _mm_sub_ps(det, temp);										// det1, -det1, det2, -det2
		det = _mm_shuffle_ps(det, det, _MM_SHUFFLE(0,0,2,2));				// det2, det2, det1, det1

		temp = _mm_mul_ps(det, coeff);										// det2 * x1x2, det2 * y1y2, det1 * x3x4, det1 * y3y4
		__m128 resultxmm = _mm_shuffle_ps(temp, temp, _MM_SHUFFLE(1,0,3,2));// det1 * x3x4, det1 * y3y4, det2 * x1x2, det2 * y1y2
		resultxmm = _mm_sub_ps(temp, resultxmm);							// -resX, -resY, resX, resY

		temp = _mm_shuffle_ps(coeff, coeff, _MM_SHUFFLE(0,1,2,3));			// coeff3, coeff2, coeff1, coeff0
		temp = _mm_mul_ps(coeff, temp);										// coeff0 * coeff3, coeff1 * coeff2, coeff2 * coeff1, coeff3 * coeff0
		coeff = _mm_shuffle_ps(temp, temp, _MM_SHUFFLE(2,3,0,1));			// coeff1 * coeff2, coeff0 * coeff3, coeff3 * coeff0, coeff2 * coeff1
		coeff = _mm_sub_ps(temp, coeff);									// denom, -denom, -denom, denom
		resultxmm = _mm_div_ps(resultxmm, coeff);

		_mm_store_ps(result, resultxmm);

		outIntersectX = -result[2];
		outIntersectY = result[3];
	}

	// CPU version of the above function for reference.
	inline void LineLineIntersectCPU(float startPoints[4], float endPoints[4], float& outIntersectX, float& outIntersectY)
	{
		float p0x = startPoints[0];
		float p0y = startPoints[1];
		float p2x = startPoints[2];
		float p2y = startPoints[3];
		float p1x = endPoints[0];
		float p1y = endPoints[1];
		float p3x = endPoints[2];
		float p3y = endPoints[3];

		float x1_sub_x2 = p0x - p1x;
		float x3_sub_x4 = p2x - p3x;
		float y3_sub_y4 = p2y - p3y;
		float y1_sub_y2 = p0y - p1y;
		float det1 = p0x * p1y - p0y * p1x;
		float det2 = p2x * p3y - p2y * p3x;
		float denom = 1.0f / (x1_sub_x2 * y3_sub_y4 - y1_sub_y2 * x3_sub_x4);

		outIntersectX = (det1 * x3_sub_x4 - x1_sub_x2 * det2) * denom;
		outIntersectY = (det1 * y3_sub_y4 - y1_sub_y2 * det2) * denom;
	}
};

#endif