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

/** \file
 * RGB to paletted image quantization routine
 */

/**\addtogroup gfx
 * @{
 */

#include "csextern.h"

#include "csgfx/rgbpixel.h"

struct csColorBox;

/**
 * Color quantizer.
 * <p>
 * The algorithm presented here is a variation of well-known and widely-used
 * Heckbert quantization algorithm. It works in the following way: first,
 * we build a 3-D histogram that describes which colors and how many times
 * the corresponding colors were used. Since we deal with RGB images, all
 * possible colors counts to 256^3 = 16777216 colors, too much to keep in
 * memory. Because of this, we ignore several lowest bits from each color
 * component, taking into consideration human eye's sensivity to different
 * color components we take 5 bits for R, 6 bits for G and 4 bits for B.
 * This summary reduces to 32*64*16 = 32768 histogram cells, which is
 * a reasonable size for an dynamically-allocated array.
 * <p>
 * Next, we take the entire color box and split it into subboxes.
 * The split happens along the longest of R,G,B axis into half
 * (and taking again into account eye sensivity). After which we take
 * again the biggest box, and split it again and so on until we
 * have that much boxes how much colors we want. Then we assign to each
 * box a color index, and compute the median RGB value for each box.
 * <p>
 * To keep memory requirements in reasonable bounds, we scale down R/G/B
 * components to 5/6/4 bits respectively. This produces results that are
 * different from those we'll get with, say, 6/7/6 resolution but its
 * a quite suitable solution, for a quality vs speed choice. I've tried
 * 5/6/5 as well as 6/6/4 and its really hard to say they are better -
 * the results are definitely different but it's hard to say they are
 * really better.
 * <p>
 * WARNING: If the sum of R+G+B bits exceeds 16, you will a need special
 * case for big endian machines for the INDEX_B macro (see below). With
 * big-endian machines the B component starts at bit 8, thus the right
 * shift counter becomes negative. Probably the best solution is to add
 * and #if B_BIT + 8 - HIST_B_BITS - HIST_G_BITS - HIST_R_BITS < 0.
 * 
 * <b>Quantizing</b><br>
 * The following routines can be used to split the quantization process
 * into small steps. First, you should call Begin(). This will
 * allocate some memory for color histogram and also do some other
 * maintenance work. Then you call Count () as much times as
 * you want, to count the frequencies of colors. This is a quite fast
 * operation, thus you can use it, for example, to compute a optimal
 * palette for a large number of images (for example, software 3D renderer
 * uses it for computing the optimal palette in paletted modes, by passing
 * every texture to Count()).
 * <p>
 * When you're finished with images, call Palette(). This will
 * compute the optimal palette. If you need just that, you can call
 * End() and you're done. If you need to remap all those textures
 * to the optimal palette, you can call Remap() as much times
 * as you wish. Finally you anyway should call End() to free
 * all the memory used by quantizer.
 * <p>
 * Now, a typical quantization sequence could look like this:
 * \code
 * Begin ();
 *
 *    Count (image, pixels);
 *   Count (...);
 *   ...
 *
 *   int maxcolors = 256;
 *   Palette (outpalette, maxcolors);
 *   // now maxcolors contains the actual number of palette entries
 *
 *   Remap (image, pixels, outimage);
 *   Remap (...);
 *   ...
 *
 * End ();
 * \endcode
 * The quantizer itself keeps track of its current state, and will silently
 * ignore invalid calls. For example, if you call Remap() right
 * after calling Begin() nothing will happen. You still can call
 * Palette() right after Begin(), and you will get the
 * expected black (or let's call it `empty') palette.
 *<p>
 * The Bias routine can be used to introduce a bias inside the
 * currently open color histogram towards given colors. The "weight"
 * parameter shows how important those colors are (0 - not at all, 100 -
 * very). This can be used to bias the resulting palette towards some
 * hard-coded values, for example you may want to use it to create a
 * relatively uniform palette that is somewhat biased towards the colors
 * contained in some picture(s).
 *<p>
 * Some routines accept an additional parameter (csRGBpixel *transp). If it
 * is 0, nothing special is performed. If it is non-0, it should point
 * to an valid csRGBpixel object; the color of that pixel will be treated in
 * a special way: Count() will ignore pixels of that color,
 * Palette() will allocate color index 0 for that color, and
 * Remap will map all such pixel values to index 0.
 */
class CS_CRYSTALSPACE_EXPORT csColorQuantizer
{
private:
  friend struct csColorBox;
  struct ColorIndex
  {
    int index;
    csColorBox* box;

    ColorIndex() : box(0) {}
  };

  // The storage for color usage histogram.
  uint16 *hist;
  // Total number of colors that were used to create the histogram.
  unsigned int hist_pixels;

  // The storage for color space boxes.
  csColorBox *box;
  // Number of valid color boxes.
  int boxcount;
  // The storage for color indices.
  ColorIndex* color_index;

  // The state of quantization variables
  enum
  {
    // Uninitialized: initial state
    qsNone,
    // Counting color frequencies
    qsCount,
    // Remapping input images to output
    qsRemap
  } qState;

  static int compare_boxes (const void *i1, const void *i2);

public:
  /// Construct a new quantizer object.
  csColorQuantizer ();
  /// Destruct and cleanup.
  ~csColorQuantizer ();

  /**
   * Quantize a RGB image into a paletted image.
   * This is a relatively expensive operation, in both CPU and
   * memory resources terms. It is pretty fast though (more than
   * 3.200.000 pixels/sec on a relatively weak P5/166) for what it does.
   * It uses a variation of well-known Heckbert quantization algorithm.
   * The side bonus after quantization is that the palette is ordered
   * in most-used-first fashion.
   * If outimage is 0, it is allocated. If outpalette is 0, it
   * is allocated as well. If it is non-0, the routine supposes that
   * the storage the pointers point to has enough size to store resulting
   * image and palette (the size of resulting image is exactly "pixels" bytes,
   * the size of resulting palette is palsize colors).
   * \param image input image
   * \param pixels number of pixels in input image
   * \param pixperline number of pixels in one line
   * \param outimage output image (allocated if 0)
   * \param outpalette output palette (allocated if 0)
   * \param maxcolors maximal number of colors in output palette
   *     (actual number of colors on return)
   * \param dither Use/do not use Floyd-Steinberg dithering
   */
  void DoRGB (csRGBpixel *image, int pixels, int pixperline,
    uint8 *&outimage, csRGBpixel *&outpalette, int &maxcolors, bool dither);

  /// Begin quantization
  void Begin ();
  /// Finish quantization
  void End ();
  /// Count the colors in a image and update the color histogram
  void Count (csRGBpixel *image, int pixels, csRGBpixel *transp = 0);
  /// Bias the color histogram towards given colors (weight = 0..100)
  void Bias (csRGBpixel *colors, int count, int weight);
  /// Compute the optimal palette for all images passed to QuantizeCount()
  void Palette (csRGBpixel *&outpalette, int &maxcolors,
    csRGBpixel *transp = 0);
  /// Remap a image to the palette computed by Palette()
  void Remap (csRGBpixel *image, int pixels, uint8 *&outimage,
    csRGBpixel *transp = 0);
  /// Same but apply Floyd-Steinberg dithering for nicer (but slower) results.
  void RemapDither (csRGBpixel *image, int pixels, int pixperline,
    csRGBpixel *palette, int colors, uint8 *&outimage,
    csRGBpixel *transp = 0);
};

/** @} */

#endif // __CS_QUANTIZE_H__
