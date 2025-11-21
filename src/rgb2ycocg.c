/***************************************************************************
*    Copyright (c) 2013, Broadcom Corporation
*    All rights reserved.
*    VESA CONFIDENTIAL
*
*  Statement regarding contribution of copyrighted materials to VESA:
*
*  This code is owned by Broadcom Corporation and is contributed to VESA
*  for inclusion and use in its VESA Display Stream Compression specification.
*  Accordingly, VESA is hereby granted a worldwide, perpetual, non-exclusive
*  license to revise, modify and create derivative works to this code and
*  VESA shall own all right, title and interest in and to any derivative
*  works authored by VESA.
*
*  Terms and Conditions
*
*  Without limiting the foregoing, you agree that your use
*  of this software program does not convey any rights to you in any of
*  Broadcom's patent and other intellectual property, and you
*  acknowledge that your use of this software may require that
*  you separately obtain patent or other intellectual property
*  rights from Broadcom or third parties.
*
*  Except as expressly set forth in a separate written license agreement
*  between you and Broadcom, if applicable:
*
*  1. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
*  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
*  REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
*  OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
*  DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
*  NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
*  ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
*  CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
*  OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
*  BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
*  SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
*  IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*  IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
*  ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
*  OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
*  NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vdo.h"
#include "dsc_types.h"
#include "rgb2ycocg.h"

#define REDUCE_CHROMA_16BPC

/*! \file rgb2ycocg.c
 *    RGB to YCoCg color space conversion function */

//! Save RGB picture to file
void save_rgb_to_file(pic_t *ip, const char *filename, dsc_cfg_t *dsc_cfg)
{
	FILE *fp;
	int i, j;

	fp = fopen(filename, "wb");
	if (!fp) {
		fprintf(stderr, "ERROR: Cannot open file %s for writing\n", filename);
		exit(1);
	}

	// Write header: width, height, bits
	fwrite(&ip->w, sizeof(int), 1, fp);
	fwrite(&ip->h, sizeof(int), 1, fp);
	fwrite(&ip->bits, sizeof(int), 1, fp);
	fwrite(&dsc_cfg->ystart, sizeof(int), 1, fp);
	fwrite(&dsc_cfg->xstart, sizeof(int), 1, fp);
	fwrite(&dsc_cfg->slice_height, sizeof(int), 1, fp);
	fwrite(&dsc_cfg->slice_width, sizeof(int), 1, fp);

	// Write RGB pixels in the slice region
	for (i = dsc_cfg->ystart; i < dsc_cfg->ystart + dsc_cfg->slice_height; i++) {
		if(i >= ip->h)
			break;
		for (j = dsc_cfg->xstart; j < dsc_cfg->xstart + dsc_cfg->slice_width; j++) {
			if(j >= ip->w)
				break;
			int r = ip->data.rgb.r[i][j];
			int g = ip->data.rgb.g[i][j];
			int b = ip->data.rgb.b[i][j];
			fwrite(&r, sizeof(int), 1, fp);
			fwrite(&g, sizeof(int), 1, fp);
			fwrite(&b, sizeof(int), 1, fp);
		}
	}

	fclose(fp);
	printf("RGB picture saved to %s\n", filename);
}

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

//! Convert RGB file to YCoCg file (pixel by pixel)
void rgb2ycocg_file(const char *rgb_filename, const char *ycocg_filename, dsc_cfg_t *dsc_cfg)
{
	FILE *fp_in, *fp_out;
	int w, h, bits;
	int ystart, xstart, slice_height, slice_width;
	int i, j;
	int r, g, b;
	int y, co, cg;

	fp_in = fopen(rgb_filename, "rb");
	if (!fp_in) {
		fprintf(stderr, "ERROR: Cannot open file %s for reading\n", rgb_filename);
		exit(1);
	}

	// Read header
	fread(&w, sizeof(int), 1, fp_in);
	fread(&h, sizeof(int), 1, fp_in);
	fread(&bits, sizeof(int), 1, fp_in);
	fread(&ystart, sizeof(int), 1, fp_in);
	fread(&xstart, sizeof(int), 1, fp_in);
	fread(&slice_height, sizeof(int), 1, fp_in);
	fread(&slice_width, sizeof(int), 1, fp_in);

	fp_out = fopen(ycocg_filename, "wb");
	if (!fp_out) {
		fprintf(stderr, "ERROR: Cannot open file %s for writing\n", ycocg_filename);
		fclose(fp_in);
		exit(1);
	}

	// Write header to output
	fwrite(&w, sizeof(int), 1, fp_out);
	fwrite(&h, sizeof(int), 1, fp_out);
	fwrite(&bits, sizeof(int), 1, fp_out);
	fwrite(&ystart, sizeof(int), 1, fp_out);
	fwrite(&xstart, sizeof(int), 1, fp_out);
	fwrite(&slice_height, sizeof(int), 1, fp_out);
	fwrite(&slice_width, sizeof(int), 1, fp_out);

	// Process pixel by pixel
	for (i = ystart; i < ystart + slice_height; i++) {
		if(i >= h)
			break;
		for (j = xstart; j < xstart + slice_width; j++) {
			if(j >= w)
				break;

			// Read RGB pixel
			fread(&r, sizeof(int), 1, fp_in);
			fread(&g, sizeof(int), 1, fp_in);
			fread(&b, sizeof(int), 1, fp_in);

			// Convert pixel
			ycocg_pixel_t pixel = rgb2ycocg_pixel(r, g, b, bits);
			y = pixel.y;
			co = pixel.co;
			cg = pixel.cg;

			// Write YCoCg pixel
			fwrite(&y, sizeof(int), 1, fp_out);
			fwrite(&co, sizeof(int), 1, fp_out);
			fwrite(&cg, sizeof(int), 1, fp_out);
		}
	}

	fclose(fp_in);
	fclose(fp_out);
	printf("RGB to YCoCg conversion completed: %s -> %s\n", rgb_filename, ycocg_filename);
}

//! Load YCoCg data from file to picture
void load_ycocg_from_file(pic_t *op, const char *ycocg_filename, dsc_cfg_t *dsc_cfg)
{
	FILE *fp;
	int w, h, bits;
	int ystart, xstart, slice_height, slice_width;
	int i, j;
	int y, co, cg;

	fp = fopen(ycocg_filename, "rb");
	if (!fp) {
		fprintf(stderr, "ERROR: Cannot open file %s for reading\n", ycocg_filename);
		exit(1);
	}

	// Read header
	fread(&w, sizeof(int), 1, fp);
	fread(&h, sizeof(int), 1, fp);
	fread(&bits, sizeof(int), 1, fp);
	fread(&ystart, sizeof(int), 1, fp);
	fread(&xstart, sizeof(int), 1, fp);
	fread(&slice_height, sizeof(int), 1, fp);
	fread(&slice_width, sizeof(int), 1, fp);

	// Verify dimensions
	if (w != op->w || h != op->h) {
		fprintf(stderr, "ERROR: Picture size mismatch. File: %dx%d, Picture: %dx%d\n",
		        w, h, op->w, op->h);
		fclose(fp);
		exit(1);
	}

	// Load YCoCg pixels
	for (i = ystart; i < ystart + slice_height; i++) {
		if(i >= h)
			break;
		for (j = xstart; j < xstart + slice_width; j++) {
			if(j >= w)
				break;

			// Read YCoCg pixel
			fread(&y, sizeof(int), 1, fp);
			fread(&co, sizeof(int), 1, fp);
			fread(&cg, sizeof(int), 1, fp);

			// Store in picture
			op->data.yuv.y[i][j] = y;
			op->data.yuv.u[i][j] = co;
			op->data.yuv.v[i][j] = cg;
		}
	}

	fclose(fp);
	printf("YCoCg picture loaded from %s\n", ycocg_filename);
}

//! Convert a slice's worth of data from RGB to YCoCg
/*! \param ip		 Input picture (RGB)
	\param op        Output picture (YCoCg)
	\param dsc_cfg   DSC configuration structure (specifying slice characteristics) */
void rgb2ycocg(pic_t *ip, pic_t *op, dsc_cfg_t *dsc_cfg)
{
	int i, j;
	int r, g, b;
	int y, co, cg;
	int half;

	if (ip->chroma != YUV_444)
	{
		fprintf(stderr, "ERROR: rgb2yuv() Incorrect input chroma type.\n");
		exit(1);
	}

	if (ip->color != RGB)
	{
		fprintf(stderr, "ERROR: rgb2yuv() Incorrect input color type.\n");
		exit(1);
	}

	if (ip->w != op->w || ip->h != op->h)
	{
		fprintf(stderr, "ERROR: rgb2yuv() Incorrect picture size.\n");
		exit(1);
	}

	if (op->chroma != YUV_444)
	{
		fprintf(stderr, "ERROR: rgb2yuv() Incorrect output chroma type.\n");
		exit(1);
	}

	if (op->color != YUV_SD && op->color != YUV_HD)
	{
		fprintf(stderr, "ERROR: rgb2yuv() Incorrect output color type.\n");
		exit(1);
	}

	half = 1 << (ip->bits-1);

	for (i = dsc_cfg->ystart; i < dsc_cfg->ystart + dsc_cfg->slice_height; i++)
	{
		if(i >= ip->h)
			break;
		for (j = dsc_cfg->xstart; j < dsc_cfg->xstart + dsc_cfg->slice_width; j++)
		{
			int t;
			if(j >= ip->w)
				break;
			r = ip->data.rgb.r[i][j];
			g = ip->data.rgb.g[i][j];
			b = ip->data.rgb.b[i][j];

			// *MODEL NOTE* MN_ENC_CSC
			co = r - b;
			t = b + (co>>1);
			cg = g - t;
			y = t + (cg>>1);

			op->data.yuv.y[i][j] = y;
#ifdef REDUCE_CHROMA_16BPC
			if(ip->bits == 16)
			{
				op->data.yuv.u[i][j] = ((co+1)>>1) + half;
				op->data.yuv.v[i][j] = ((cg+1)>>1) + half;
			}
			else
#endif
			{
				op->data.yuv.u[i][j] = co + half*2;
				op->data.yuv.v[i][j] = cg + half*2;
			}
		}
	}
}
