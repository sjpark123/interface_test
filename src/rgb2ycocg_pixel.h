#ifndef RGB2YCOCG_PIXEL_H
#define RGB2YCOCG_PIXEL_H

#ifdef __cplusplus
extern "C" {
#endif

//! YCoCg pixel structure (C-compatible version)
typedef struct ycocg_pixel_s {
	int y;   // Y component
	int co;  // Co component
	int cg;  // Cg component
} ycocg_pixel_t;

//! Convert a single pixel from RGB to YCoCg
/*! \param r         Red component
	\param g         Green component
	\param b         Blue component
	\param bits      Bit depth
	\return          YCoCg pixel structure containing y, co, cg components */
ycocg_pixel_t rgb2ycocg_pixel(int r, int g, int b, int bits);

#ifdef __cplusplus
}
#endif

#endif // RGB2YCOCG_PIXEL_H
