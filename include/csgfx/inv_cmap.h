/*
 * This software is copyrighted as noted below.  It may be freely copied,
 * modified, and redistributed, provided that the copyright notice is
 * preserved on all copies.
 *
 * There is no warranty or other guarantee of fitness for this software,
 * it is provided solely "as is".  Bug reports or fixes may be sent
 * to the author, who may or may not act on them as he desires.
 *
 * You may not include this software in a program or other software product
 * without supplying the source, or without informing the end-user that the
 * source is available for no extra charge.
 *
 * If you modify this software, you should include a notice giving the
 * name of the person performing the modification, the date of modification,
 * and the reason for such modification.
 */

/*
 * inv_cmap.c - Compute an inverse colormap.
 *
 * Author:	Spencer W. Thomas
 * 		EECS Dept.
 * 		University of Michigan
 * Date:	Thu Sep 20 1990
 * Copyright (c) 1990, University of Michigan
 *
 * Modified:    12 Oct 1998 by G. C. Ewing
 *              Allow for different numbers of bits for r, g, and b.
 *              Convert to C++.
 */

#ifndef __CS_INV_CMAP_H__
#define __CS_INV_CMAP_H__

/**\file 
 * Compute an inverse colormap
 */

#include "csextern.h"

#include "rgbpixel.h"

/**
 * Compute an inverse colormap efficiently.
 * 
 * <pre>Inputs:
 * 	colors:		Number of colors in the forward colormap.
 * 	colormap:	The forward colormap.
 *      rbits, gbits, bbits:
 * 			Number of quantization bits.  The inverse
 * 			colormap will have N=(2^rbits)*(2^gbits)*(2^bbits)
 *                      entries.
 * 	dist_buf:	An array of N long integers to be used as scratch
 * 			space. If 0, the dist_buff will be allocated
 *                      and freed before exiting the routine.
 * Outputs:
 * 	rgbmap:		The output inverse colormap.  The entry
 * 			rgbmap[(r<<(gbits+bbits)) + (g<<bbits) + b]
 * 			is the colormap entry that is closest to the
 * 			(quantized) color (r,g,b). If 0, it will be
 *                      allocated with "new uint8* []"
 * Assumptions:
 * 	Quantization is performed by right shift (low order bits are
 * 	truncated).  Thus, the distance to a quantized color is
 * 	actually measured to the color at the center of the cell
 * 	(i.e., to r+.5, g+.5, b+.5, if (r,g,b) is a quantized color).
 * Algorithm:
 * 	Uses a "distance buffer" algorithm:
 * 	The distance from each representative in the forward color map
 * 	to each point in the rgb space is computed.  If it is less
 * 	than the distance currently stored in dist_buf, then the
 * 	corresponding entry in rgbmap is replaced with the current
 * 	representative (and the dist_buf entry is replaced with the
 * 	new distance).
 *
 * 	The distance computation uses an efficient incremental formulation.
 *
 * 	Distances are computed "outward" from each color.  If the
 * 	colors are evenly distributed in color space, the expected
 * 	number of cells visited for color I is N^3/I.
 * 	Thus, the complexity of the algorithm is O(log(K) N^3),
 * 	where K = colors, and N = 2^bits.
 * </pre>
 * BUGBUG: For some unknown reason the routine generates colormaps shifted
 * towards red if green is bigger than red (and vice versa, shifted to green
 * if red is bigger). Thus it is adviced to use same resolution for R and G.
 * If someone can find out why it happens, he is free to do it -- A.Z.
 */
extern CS_CRYSTALSPACE_EXPORT void csInverseColormap (int colors, 
  csRGBpixel *colormap, int rbits, int gbits, int bbits, uint8 *&rgbmap,
  uint32 *dist_buf = 0);

#endif // __CS_INV_CMAP_H__
