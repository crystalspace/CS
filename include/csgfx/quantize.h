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

#ifndef __CS_QUANTIZE_H__
#define __CS_QUANTIZE_H__

#include "csgfx/rgbpixel.h"

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
 * <dl>
 * <dt>image<dd>input image
 * <dt>pixels<dd>number of pixels in input image
 * <dt>pixperline<dd>number of pixels in one line
 * <dt>outimage<dd>output image (allocated if NULL)
 * <dt>outpalette<dd>output palette (allocated if NULL)
 * <dt>maxcolors<dd>maximal number of colors in output palette
 *     (actual number of colors on return)
 * <dt>dither<dd>Use/do not use Floyd-Steinberg dithering
 * </dl>
 */
extern void csQuantizeRGB (csRGBpixel *image, int pixels, int pixperline,
  UByte *&outimage, csRGBpixel *&outpalette, int &maxcolors, bool dither);

/**
 * The following routines can be used to split the quantization process
 * into small steps. First, you should call csQuantizeBegin(). This will
 * allocate some memory for color histogram and also do some other
 * maintenance work. Then you call csQuantizeCount () as much times as
 * you want, to count the frequencies of colors. This is a quite fast
 * operation, thus you can use it, for example, to compute a optimal
 * palette for a large number of images (for example, software 3D renderer
 * uses it for computing the optimal palette in paletted modes, by passing
 * every texture to csQuantizeCount()).
 * <p>
 * When you're finished with images, call csQuantizePalette(). This will
 * compute the optimal palette. If you need just that, you can call
 * csQuantizeEnd() and you're done. If you need to remap all those textures
 * to the optimal palette, you can call csQuantizeRemap() as much times
 * as you wish. Finally you anyway should call csQuantizeEnd() to free
 * all the memory used by quantizer.
 * <p>
 * Now, a typical quantization sequence could look like this:
 * <pre>
 * csQuantizeBegin ();
 *
 *   csQuantizeCount (image, pixels);
 *   csQuantizeCount (...);
 *   ...
 *
 *   int maxcolors = 256;
 *   csQuantizePalette (outpalette, maxcolors);
 *   // now maxcolors contains the actual number of palette entries
 *
 *   csQuantizeRemap (image, pixels, outimage);
 *   csQuantizeRemap (...);
 *   ...
 *
 * csQuantizeEnd ();
 * </pre>
 * The quantizer itself keeps track of its current state, and will silently
 * ignore invalid calls. For example, if you call csQuantizeRemap() right
 * after calling csQuantizeBegin() nothing will happen. You still can call
 * csQuantizePalette() right after csQuantizeBegin(), and you will get the
 * expected black (or let's call it `empty') palette.
 *<p>
 * The csQuantizeBias routine can be used to introduce a bias inside the
 * currently open color histogram towards given colors. The "weight" parameter
 * shows how important those colors are (0 - not at all, 100 - very). This
 * can be used to bias the resulting palette towards some hard-coded values,
 * for example you may want to use it to create a relatively uniform palette
 * that is somewhat biased towards the colors contained in some picture(s).
 *<p>
 * Some routines accept an additional parameter (csRGBpixel *transp). If it is
 * NULL, nothing special is performed. If it is non-NULL, it should point
 * to an valid csRGBpixel object; the color of that pixel will be treated in
 * a special way: csQuantizeCount() will ignore pixels of that color,
 * csQuantizePalette() will allocate color index 0 for that color, and
 * csQuantizeRemap will map all such pixel values to index 0.
 */

/// Begin quantization
extern void csQuantizeBegin ();
/// Finish quantization
extern void csQuantizeEnd ();
/// Count the colors in a image and update the color histogram
extern void csQuantizeCount (csRGBpixel *image, int pixels,
  csRGBpixel *transp = NULL);
/// Bias the color histogram towards given colors (weight = 0..100)
extern void csQuantizeBias (csRGBpixel *colors, int count, int weight);
/// Compute the optimal palette for all images passed to QuantizeCount()
extern void csQuantizePalette (csRGBpixel *&outpalette, int &maxcolors,
  csRGBpixel *transp = NULL);
/// Remap a image to the palette computed by csQuantizePalette()
extern void csQuantizeRemap (csRGBpixel *image, int pixels,
  UByte *&outimage, csRGBpixel *transp = NULL);
/// Same but apply Floyd-Steinberg dithering for nicer (but slower) results
extern void csQuantizeRemapDither (csRGBpixel *image, int pixels, int pixperline,
  csRGBpixel *palette, int colors, UByte *&outimage, csRGBpixel *transp = NULL);

#endif // __CS_QUANTIZE_H__
