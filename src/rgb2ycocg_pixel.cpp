#include "rgb2ycocg_pixel.h"

#define REDUCE_CHROMA_16BPC

/*! \file rgb2ycocg_pixel.c
 *    Single pixel RGB to YCoCg color space conversion function */

//! Convert a single pixel from RGB to YCoCg
ycocg_pixel_t rgb2ycocg_pixel(int r, int g, int b, int bits)
{
	ycocg_pixel_t result;
	int t;
	int half = 1 << (bits - 1);

	// *MODEL NOTE* MN_ENC_CSC
	result.co = r - b;
	t = b + (result.co >> 1);
	result.cg = g - t;
	result.y = t + (result.cg >> 1);

#ifdef REDUCE_CHROMA_16BPC
	if(bits == 16) {
		result.co = ((result.co + 1) >> 1) + half;
		result.cg = ((result.cg + 1) >> 1) + half;
	} else
#endif
	{
		result.co = result.co + half * 2;
		result.cg = result.cg + half * 2;
	}

	return result;
}
