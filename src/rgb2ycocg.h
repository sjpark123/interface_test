/***************************************************************************
*    Copyright (c) 2013, Broadcom Corporation
*    All rights reserved.
*    VESA CONFIDENTIAL
***************************************************************************/

#ifndef RGB2YCOCG_H
#define RGB2YCOCG_H

#include "vdo.h"
#include "dsc_types.h"

//! YCoCg pixel structure
struct ycocg_pixel_t {
	ycocg_pixel_t() {}
	ycocg_pixel_t(int y_in, int co_in, int cg_in) : y(y_in), co(co_in), cg(cg_in) {}
	int y;   // Y component
	int co;  // Co component
	int cg;  // Cg component

	// Comparison operator
	bool operator==(const ycocg_pixel_t &other) const {
		return ((y == other.y) && (co == other.co) && (cg == other.cg));
	}

	// Assignment operator
	ycocg_pixel_t &operator=(const ycocg_pixel_t &other) {
		y = other.y;
		co = other.co;
		cg = other.cg;
		return *this;
	}
};

//! Save RGB picture to file
/*! \param ip		 Input picture (RGB)
	\param filename  Output filename
	\param dsc_cfg   DSC configuration structure (specifying slice characteristics) */
void save_rgb_to_file(pic_t *ip, const char *filename, dsc_cfg_t *dsc_cfg);

//! Convert a single pixel from RGB to YCoCg
/*! \param r         Red component
	\param g         Green component
	\param b         Blue component
	\param bits      Bit depth
	\return          YCoCg pixel structure containing y, co, cg components */
ycocg_pixel_t rgb2ycocg_pixel(int r, int g, int b, int bits);

//! Convert RGB file to YCoCg file (pixel by pixel)
/*! \param rgb_filename   Input RGB filename
	\param ycocg_filename Output YCoCg filename
	\param dsc_cfg        DSC configuration structure */
void rgb2ycocg_file(const char *rgb_filename, const char *ycocg_filename, dsc_cfg_t *dsc_cfg);

//! Load YCoCg data from file to picture
/*! \param op             Output picture (YCoCg)
	\param ycocg_filename Input YCoCg filename
	\param dsc_cfg        DSC configuration structure */
void load_ycocg_from_file(pic_t *op, const char *ycocg_filename, dsc_cfg_t *dsc_cfg);

//! Convert a slice's worth of data from RGB to YCoCg
/*! \param ip		 Input picture (RGB)
	\param op        Output picture (YCoCg)
	\param dsc_cfg   DSC configuration structure (specifying slice characteristics) */
void rgb2ycocg(pic_t *ip, pic_t *op, dsc_cfg_t *dsc_cfg);

#endif // RGB2YCOCG_H
