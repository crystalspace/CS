/*
    RGB to paletted image quantization routine
    Copyright (C) 2000 by Andrew Zabolotny <bit@eltech.ru>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __QUANTIZE_H__
#define __QUANTIZE_H__

#include "csgfxldr/rgbpixel.h"

/**
 * Quantize a RGB image into a paletted image.
 * This is a relatively expensive operation, in both CPU and
 * memory resources terms. It is pretty fast though (more than
 * 3.200.000 pixels/sec on a relatively weak P5/166) for what it does.
 * It uses a variation of well-known Heckbert quantization algorithm.
 * The side bonus after quantization is that the palette is ordered
 * in most-used-first fashion.
 * If outimage is NULL, it is allocated. If outpalette is NULL, it
 * is allocated as well. If it is non-NULL, the routine supposes that
 * the storage the pointers point to has enough size to store resulting
 * image and palette (the size of resulting image is exactly "pixels" bytes,
 * the size of resulting palette is palsize colors).
 */
extern void csQuantizeRGB (RGBPixel *image, int pixels, int palsize,
  UByte *&outimage, RGBPixel *&outpalette);

#endif // __QUANTIZE_H__
