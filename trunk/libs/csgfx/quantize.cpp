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

#include "cssysdef.h"
#include "csgfx/quantize.h"
#include "csgfx/inv_cmap.h"
#include <string.h>
#include <stdlib.h>

#define HIST_R_BITS	5
#define HIST_G_BITS	6
#define HIST_B_BITS	5

// Amount to shift left R value to get max (hist_r, hist_g, hist_b)
#define HIST_SHIFT_R	1
#define HIST_SHIFT_G	0
#define HIST_SHIFT_B	1

#define HIST_R_MAX	(1 << HIST_R_BITS)
#define HIST_G_MAX	(1 << HIST_G_BITS)
#define HIST_B_MAX	(1 << HIST_B_BITS)

#ifdef CS_LITTLE_ENDIAN
#  define R_BIT		0
#  define G_BIT		8
#  define B_BIT		16
#else
#  define R_BIT		24
#  define G_BIT		16
#  define B_BIT		8
#endif

// The mask for extracting just R/G/B from an uint32 or csRGBpixel
#ifdef CS_BIG_ENDIAN
#  define RGB_MASK 0xffffff00
#else
#  define RGB_MASK 0x00ffffff
#endif

// Compute masks for effectively separating R,G and B components from a uint32.
// For a little-endian machine they are respectively
// 0x000000f8, 0x0000fc00 and 0x00f80000
// For a big-endian machine they are respectively
// 0xf8000000, 0x00fc0000 and 0x0000f800
#define R_MASK		((HIST_R_MAX - 1) << (R_BIT + 8 - HIST_R_BITS))
#define G_MASK		((HIST_G_MAX - 1) << (G_BIT + 8 - HIST_G_BITS))
#define B_MASK		((HIST_B_MAX - 1) << (B_BIT + 8 - HIST_B_BITS))
// The following macro extract the respective color components from a uint32
// and transform them into a index in the histogram.
#define INDEX_R(l)	((l & R_MASK) >> (R_BIT + 8 - HIST_R_BITS))
#define INDEX_G(l)	((l & G_MASK) >> (G_BIT + 8 - HIST_G_BITS - HIST_R_BITS))
#define INDEX_B(l)	((l & B_MASK) >> (B_BIT + 8 - HIST_B_BITS - HIST_G_BITS - HIST_R_BITS))
// Calculate index into histogram for given R,G,B components
#define INDEX(r,g,b)	(r + (g << HIST_R_BITS) + (b << (HIST_R_BITS + HIST_G_BITS)))

/**
 * A box in color space.
 * Both minimal and maximal component bounds are inclusive, that is, the bounds
 * Rm = 0, Rx = 255 means the box covers the entire R component range.
 * <p>
 * This structure is meant to be highly fast, thus only atomic operations
 * are implemented for it. After some operations the box may be left in a
 * invalid state, thus take care.
 */
struct csColorBox
{
  csColorQuantizer* quant;

  // The minimal and maximal R
  uint8 Rm,Rx;
  // The minimal and maximal G
  uint8 Gm,Gx;
  // The minimal and maximal B
  uint8 Bm,Bx;
  // Color box volume
  unsigned Volume;
  // Number of pixels in this box
  unsigned PixelCount;
  // Number of non-zero different color values in this box
  unsigned ColorCount;

  // Useful function
  static inline unsigned Sqr (int x)
  { return x * x; }
  // Set box to given bounds
  void Set (uint8 rm, uint8 rx, uint8 gm, uint8 gx, uint8 bm, uint8 bx)
  { Rm = rm; Rx = rx; Gm = gm; Gx = gx; Bm = bm; Bx = bx; }
  // Compute the volume of box
  void ComputeVolume ()
  {
    // We compute the length of the diagonal of the box rather than the
    // proper volume. This also has the side effect that a long narrow
    // box looks more "voluminous" thus its more probably that it will
    // be split rather than a relatively cubic one.
    Volume = Sqr (Rx - Rm) * (R_COEF_SQ << HIST_SHIFT_R) +
             Sqr (Gx - Gm) * (G_COEF_SQ << HIST_SHIFT_G) +
             Sqr (Bx - Bm) * (B_COEF_SQ << HIST_SHIFT_B);
  }
  // Count number of non-zero colors within this box
  void CountPixels ()
  {
    PixelCount = ColorCount = 0;
    int b;
    for (b = Bm; b <= Bx; b++)
    {
      int g;
      for (g = Gm; g <= Gx; g++)
      {
        uint16 *hp = &quant->hist [INDEX (Rm, g, b)];
	int r;
        for (r = Rx - Rm; r >= 0; r--, hp++)
	{
          if (*hp)
          {
            PixelCount += *hp;
            ColorCount++;
          } /* endif */
	}
      } /* endfor */
    }
  }

  // Move Rm up until we find pixels that contain this value
  bool ShrinkRm ()
  {
    uint8 iRm = Rm;
    int g;
    for (; Rm <= Rx; Rm++)
    {
      uint8 b;
      for (b = Bm; b <= Bx; b++)
      {
        uint16 *hp = &quant->hist [INDEX (Rm, Gm, b)];
        for (g = Gx - Gm; g >= 0; g--, hp += HIST_R_MAX)
          if (*hp) return (Rm != iRm);
      }
    }
    return (Rm != iRm);
  }

  // Move Rx down until we find pixels that contain this value
  bool ShrinkRx ()
  {
    uint8 iRx = Rx;
    int g;
    for (; Rx >= Rm; Rx--)
      for (uint8 b = Bm; b <= Bx; b++)
      {
        uint16 *hp = &quant->hist [INDEX (Rx, Gm, b)];
        for (g = Gx - Gm; g >= 0; g--, hp += HIST_R_MAX)
          if (*hp) return (Rx != iRx);
      }
    return (Rx != iRx);
  }

  // Move Gm up until we find pixels that contain this value
  bool ShrinkGm ()
  {
    uint8 iGm = Gm;
    int r;

    for (; Gm <= Gx; Gm++)
    {
      uint8 b;
      for (b = Bm; b <= Bx; b++)
      {
        uint16 *hp = &quant->hist [INDEX (Rm, Gm, b)];
        for (r = Rx - Rm; r >= 0; r--, hp++)
          if (*hp) return (Gm != iGm);
      }
    }
    return (Gm != iGm);
  }

  // Move Gx down until we find pixels that contain this value
  bool ShrinkGx ()
  {
    uint8 iGx = Gx;
    int r;
    for (; Gx >= Gm; Gx--)
    {
      uint8 b;
      for (b = Bm; b <= Bx; b++)
      {
        uint16 *hp = &quant->hist [INDEX (Rm, Gx, b)];
        for (r = Rx - Rm; r >= 0; r--, hp++)
          if (*hp) return (Gx != iGx);
      }
    }
    return (Gx != iGx);
  }

  // Move Bm up until we find pixels that contain this value
  bool ShrinkBm ()
  {
    uint8 iBm = Bm;
    int r;
    for (; Bm <= Bx; Bm++)
    {
      uint8 g;
      for (g = Gm; g <= Gx; g++)
      {
        uint16 *hp = &quant->hist [INDEX (Rm, g, Bm)];
        for (r = Rx - Rm; r >= 0; r--, hp++)
          if (*hp) return (Bm != iBm);
      }
    }
    return (Bm != iBm);
  }

  // Move Bx down until we find pixels that contain this value
  bool ShrinkBx ()
  {
    uint8 iBx = Bx;
    int r;
    for (; Bx >= Bm; Bx--)
    {
      uint8 g;
      for (g = Gm; g <= Gx; g++)
      {
        uint16 *hp = &quant->hist [INDEX (Rm, g, Bx)];
        for (r = Rx - Rm; r >= 0; r--, hp++)
          if (*hp) return (Bx != iBx);
      }
    }
    return (Bx != iBx);
  }

  // Shrink box: move min/max bounds until we hit an existing color
  void Shrink ()
  {
    ShrinkRm (); ShrinkRx ();
    ShrinkGm (); ShrinkGx ();
    ShrinkBm (); ShrinkBx ();
  }
  /**
   * Compute the mean color for this box.
   * The computation is performed by taking into account each color's
   * weight, i.e. number of pixels with this color. Thus resulting palette
   * is biased towards most often used colors.
   */
  void GetMeanColor (csRGBpixel &color)
  {
    unsigned rs = 0, gs = 0, bs = 0;
    unsigned count = 0;
    int b, g, r;
    for (b = Bm; b <= Bx; b++)
      for (g = Gm; g <= Gx; g++)
      {
        uint16 *hp = &quant->hist [INDEX (Rm, g, b)];
        for (r = Rm; r <= Rx; r++, hp++)
          if (*hp)
          {
            unsigned pixc = *hp;
            count += pixc;
            rs += pixc * r;
            gs += pixc * g;
            bs += pixc * b;
          } /* endif */
      } /* endfor */
    // In some extreme cases (textures with zero pixels or
    // single-color textures with 1 transparent color)
    // we can end here with count == 0; avoid division by zero
    if (!count)
    {
      color = csRGBpixel (0, 0, 0);
      return;
    }
    color.red   = ((rs + count / 2) << (8 - HIST_R_BITS)) / count;
    color.green = ((gs + count / 2) << (8 - HIST_G_BITS)) / count;
    color.blue  = ((bs + count / 2) << (8 - HIST_B_BITS)) / count;
  }
  void FillInverseCMap (uint8 *icmap, uint8 index)
  {
    int Rcount = Rx - Rm + 1;
    int b;
    for (b = Bm; b <= Bx; b++)
    {
      int g;
      for (g = Gm; g <= Gx; g++)
        memset (&icmap [INDEX (Rm, g, b)], index, Rcount);
    }
  }
};

int csColorQuantizer::compare_boxes (const void *i1, const void *i2)
{
  CS_ASSERT (((ColorIndex*)i1)->box != 0);
  CS_ASSERT (((ColorIndex*)i2)->box != 0);
  int count1 = ((ColorIndex*)i1)->box->PixelCount;
  int count2 = ((ColorIndex*)i2)->box->PixelCount;
  return (count1 > count2) ? -1 : (count1 == count2) ? 0 : +1;
}

csColorQuantizer::csColorQuantizer ()
{
  hist = 0;
  hist_pixels = 0;
  box = 0;
  boxcount = 0;
  color_index = 0;
  qState = qsNone;
  Begin ();
}

csColorQuantizer::~csColorQuantizer ()
{
  End ();
}

void csColorQuantizer::Begin ()
{
  // Clean up, if previous quantization sequence was not finished
  End ();

  // First, allocate the histogram
  hist = new uint16 [HIST_R_MAX * HIST_G_MAX * HIST_B_MAX];
  memset (hist, 0, HIST_R_MAX * HIST_G_MAX * HIST_B_MAX * sizeof (uint16));

  hist_pixels = 0;
  qState = qsCount;
}

/// Clean up quantization. Is automatically done at destruction time.
void csColorQuantizer::End ()
{
  delete [] color_index; color_index = 0;
  delete [] box; box = 0;
  delete [] hist; hist = 0;
}

void csColorQuantizer::Count (csRGBpixel *image, int pixels,
		csRGBpixel *transp)
{
  // Sanity check
  if (!pixels || qState != qsCount)
    return;

  hist_pixels += pixels;

  // Now, count all colors in image
  uint32 *src = (uint32 *)image;
  if (transp)
  {
    uint32 tc = (*(uint32 *)transp) & RGB_MASK;
    while (pixels--)
    {
      uint32 pix = *src++;
      if (tc != (pix & RGB_MASK))
      {
        uint16 &pa = hist [INDEX_R (pix) + INDEX_G (pix) + INDEX_B (pix)];
        // do not permit overflow here; stick to MAX_USHORT
        if (!++pa) --pa;
      }
    }
  }
  else
    while (pixels--)
    {
      uint32 pix = *src++;
      uint16 &pa = hist [INDEX_R (pix) + INDEX_G (pix) + INDEX_B (pix)];
      // do not permit overflow here; stick to MAX_USHORT
      if (!++pa) --pa;
    }
}

void csColorQuantizer::Bias (csRGBpixel *colors, int count, int weight)
{
  // Sanity check
  if (!count || qState != qsCount)
    return;

  unsigned delta;
  if (hist_pixels < (0xffffffff / 100))
    delta = ((hist_pixels + 1) * weight / (100 * count));
  else
    delta = ((hist_pixels / count + 1) * weight) / 100;
  if (delta > 0xffff)
    delta = 0xffff;
  else if (!delta)
    return;

  // Now, count all colors in image
  uint32 *src = (uint32 *)colors;
  while (count--)
  {
    uint32 pix = *src++;
    uint16 &pa = hist [INDEX_R (pix) + INDEX_G (pix) + INDEX_B (pix)];
    // do not permit overflow here; stick to MAX_USHORT
    if (unsigned (pa) + delta > 0xffff) pa = 0xffff; else pa += delta;
  }
}

void csColorQuantizer::Palette (csRGBpixel *&outpalette,
		int &maxcolors, csRGBpixel *transp)
{
  // Sanity check
  if (qState != qsCount || !maxcolors)
    return;

  // Good. Now we create the array of color space boxes.
  box = new csColorBox [maxcolors];
  int i;
  for (i = 0 ; i < maxcolors ; i++)
    box [i].quant = this;

  box [0].Set (0, HIST_R_MAX - 1, 0, HIST_G_MAX - 1, 0, HIST_B_MAX - 1);
  box [0].Shrink ();
  box [0].ComputeVolume ();
  box [0].CountPixels ();
  boxcount = 1;

  if (transp)
    maxcolors--;

  // Loop until we have enough boxes (or we're out of pixels)
  while (boxcount < maxcolors)
  {
    // Find the box that should be split
    // We're making this decision the following way:
    // - first half of palette we prefer to split boxes that are
    // most populated with different colors.
    // - the rest of palette we prefer to split largest boxes.
    int bi, bestbox = -1;
    unsigned bestrating = 0;
    if (boxcount < maxcolors / 2)
    {
      for (bi = 0; bi < boxcount; bi++)
        if (bestrating < box [bi].ColorCount)
        {
          bestrating = box [bi].ColorCount;
          bestbox = bi;
        }
    }
    else
    {
      for (bi = 0; bi < boxcount; bi++)
        if (bestrating < box [bi].Volume)
        {
          bestrating = box [bi].Volume;
          bestbox = bi;
        }
    }
    // Out of splittable boxes?
    if (bestrating <= 1)
      break;

    csColorBox &srcbox = box [bestbox];
    csColorBox &dstbox = box [boxcount++];
    dstbox = srcbox;

    // Decide along which of R/G/B axis to split the box
    int rlen = (dstbox.Rx - dstbox.Rm) * (R_COEF << HIST_SHIFT_R);
    int glen = (dstbox.Gx - dstbox.Gm) * (G_COEF << HIST_SHIFT_G);
    int blen = (dstbox.Bx - dstbox.Bm) * (B_COEF << HIST_SHIFT_B);

    enum { axisR, axisG, axisB } axis =
      (glen < rlen) ?
        ((rlen < blen) ? axisB : axisR) :
        ((glen < blen) ? axisB : axisG);

    //
    // We split each box into two by the plane that goes through given color
    // component (one of R,G,B as choosen above). Any of resulting split boxes
    // possibly can become smaller if we move one of the five its faces (the
    // sixth face sure can't move because it was checked before - the one that
    // is opposed to the just-created new face, in the place of split).
    // Here goes some ASCII art:
    //
    //    C       G       K   The initial color box ABCD-IJKL was split by a
    //    *-------*-------*   plane and two boxes ABCD-EFGH and EFGH-IJKL
    //   /|      /|      /|   were created. The boxes cannot be shrinked
    // B/ |    F/ |    J/ |   by moving faces ABCD and IJKL (because the
    // *-------*-------*  |   boxes were previously adjusted and any surface
    // |  *----|--*----|--*   passes through at least one used color).
    // | /D    | /H    | /L   Now we also see that if the left box
    // |/      |/      |/     can be shrinked by moving face, say, ABFE
    // *-------*-------*      towards DCGH, it is impossible for the right
    // A       E       I      box to be shrinked by moving EFJI towards HGKL,
    //                        because the previous whole face ABJI is known
    // to pass through at least one used color (and if it is not in the ABFE
    // are, then it is surely in the EFJI area). We can say the same about
    // the DCGH/HGKL, BCGF/FGKJ and ADHE/EHLI pairs.
    //
    switch (axis)
    {
      case axisR:
        srcbox.Rx = (srcbox.Rm + srcbox.Rx) / 2;
        dstbox.Rm = srcbox.Rx + 1;
        srcbox.ShrinkRx ();
        dstbox.ShrinkRm ();
        if (!srcbox.ShrinkGm ())
          dstbox.ShrinkGm ();
        if (!srcbox.ShrinkGx ())
          dstbox.ShrinkGx ();
        if (!srcbox.ShrinkBm ())
          dstbox.ShrinkBm ();
        if (!srcbox.ShrinkBx ())
          dstbox.ShrinkBx ();
        break;
      case axisG:
        srcbox.Gx = (srcbox.Gm + srcbox.Gx) / 2;
        dstbox.Gm = srcbox.Gx + 1;
        srcbox.ShrinkGx ();
        dstbox.ShrinkGm ();
        if (!srcbox.ShrinkRm ())
          dstbox.ShrinkRm ();
        if (!srcbox.ShrinkRx ())
          dstbox.ShrinkRx ();
        if (!srcbox.ShrinkBm ())
          dstbox.ShrinkBm ();
        if (!srcbox.ShrinkBx ())
          dstbox.ShrinkBx ();
        break;
      case axisB:
        srcbox.Bx = (srcbox.Bm + srcbox.Bx) / 2;
        dstbox.Bm = srcbox.Bx + 1;
        srcbox.ShrinkBx ();
        dstbox.ShrinkBm ();
        if (!srcbox.ShrinkRm ())
          dstbox.ShrinkRm ();
        if (!srcbox.ShrinkRx ())
          dstbox.ShrinkRx ();
        if (!srcbox.ShrinkGm ())
          dstbox.ShrinkGm ();
        if (!srcbox.ShrinkGx ())
          dstbox.ShrinkGx ();
        break;
    } /* endswitch */

    dstbox.CountPixels ();
    srcbox.PixelCount -= dstbox.PixelCount;
    srcbox.ColorCount -= dstbox.ColorCount;
    srcbox.ComputeVolume ();
    dstbox.ComputeVolume ();
  } /* endwhile */

  // Either we're out of splittable boxes, or we have palsize boxes.

  // Assign successive palette indices to all boxes
  int count, delta = transp ? 1 : 0;
  color_index = new ColorIndex[boxcount + delta];
  for (count = 0; count < boxcount; count++)
  {
    color_index[count].index = count;
    color_index[count].box = box + count;
  }
  // Sort palette indices by usage (a side bonus to quantization)
  qsort (color_index, boxcount, sizeof (ColorIndex), compare_boxes);

  // Allocate the palette, if not already allocated
  if (!outpalette)
    outpalette = new csRGBpixel [maxcolors + delta];

  // Fill the unused colormap entries with zeros
  memset (&outpalette [boxcount + delta], 0,
    (maxcolors - boxcount) * sizeof (csRGBpixel));

  // Now compute the mean color for each box
  for (count = 0; count < boxcount; count++)
    color_index[count].box->GetMeanColor (outpalette [count + delta]);

  // If we have a transparent color, set colormap entry 0 to it
  if (delta)
  {
    for (count = boxcount; count; count--)
    {
      color_index[count].index = color_index[count - 1].index + 1;
      color_index[count].box = box + color_index[count].index;
    }
    color_index[0].index = 0;
    color_index[0].box = box;
    outpalette [0] = csRGBpixel (0, 0, 0);
  }

  maxcolors = boxcount + delta;
}

void csColorQuantizer::Remap (csRGBpixel *image, int pixels,
  uint8 *&outimage, csRGBpixel *transp)
{
  // Sanity check
  if (qState != qsCount && qState != qsRemap)
    return;

  int count;

  // We will re-use the histogram memory for a inverse colormap. However, we
  // will need just a byte per element, so we'll assign the address of
  // histogram memory block to a pointer of suitable type, and the second
  // half of histogram storage remains unused.
  uint8 *icmap = (uint8 *)hist;

  int delta = transp ? 1 : 0;
  if (qState == qsCount)
  {
    // Now, fill inverse colormap with color indices
    for (count = 0; count < boxcount; count++)
      box [color_index[count + delta].index - delta].FillInverseCMap (
	icmap, count + delta);
    qState = qsRemap;
  }

  // Allocate the picture and the palette
  if (!outimage) outimage = new uint8 [pixels];

  uint32 *src = (uint32 *)image;
  uint8 *dst = outimage;
  count = pixels;

  if (transp)
  {
    uint32 tc = (*(uint32 *)transp) & RGB_MASK;
    while (count--)
    {
      uint32 pix = *src++;
      if (tc == (pix & RGB_MASK))
        *dst++ = 0;
      else
        *dst++ = icmap [INDEX_R (pix) + INDEX_G (pix) + INDEX_B (pix)];
    }
  }
  else
    while (count--)
    {
      uint32 pix = *src++;
      *dst++ = icmap [INDEX_R (pix) + INDEX_G (pix) + INDEX_B (pix)];
    }
}

void csColorQuantizer::RemapDither (
  csRGBpixel *image, int pixels, int pixperline,
  csRGBpixel *palette, int colors, uint8 *&outimage, csRGBpixel *transp)
{
  // Sanity check
  if (qState != qsCount && qState != qsRemap)
    return;

  int count;

  // We will re-use the histogram memory for a inverse colormap. However, we
  // will need just a byte per element, so we'll assign the address of
  // histogram memory block to a pointer of suitable type, and the second
  // half of histogram storage remains unused.
  uint8 *icmap = (uint8 *)hist;

  int delta = transp ? 1 : 0;
  if (qState == qsCount)
  {
    // Build an inverse colormap (since during dithering we can get color
    // indices that did not existed in the original image)
    csInverseColormap (colors - delta, palette + delta,
      HIST_R_BITS, HIST_G_BITS, HIST_B_BITS, icmap);
    if (transp)
    {
      int i;
      for (i = 0; i < HIST_R_MAX * HIST_G_MAX * HIST_B_MAX; i++)
        icmap [i]++;
    }
    qState = qsRemap;
  }

  // Allocate the picture and the palette
  if (!outimage) outimage = new uint8 [pixels];

  csRGBpixel *src = image;
  uint8 *dst = outimage;
  count = pixels;

  CS_ALLOC_STACK_ARRAY (int, fserr, (2 * 3 * (pixperline + 2)));
  memset (fserr, 0, 3 * (pixperline + 2) * sizeof (int));
  // odd/even row
  unsigned char odd = 0;
  while (count > 0)
  {
    // The algorithm implements the widely-known and used Floyd-Steinberg
    // error distribution - based dithering. The errors are distributed with
    // the following weights to the surrounding pixels:
    //
    //       (here)   7/16
    // 3/16   5/16    1/16
    //
    // Even lines are traversed left to right, odd lines backwards.

    csRGBpixel *cursrc;
    uint8 *curdst;
    int *curerr, *nexterr;
    int dir;

    if (odd)
    {
      cursrc = src + pixperline - 1;
      curdst = dst + pixperline - 1;
      curerr = fserr + 2 * 3 * (pixperline + 2) - 6;
      nexterr = fserr + 3 * (pixperline + 2) - 3;
      dir = -1;
    }
    else
    {
      cursrc = src;
      curdst = dst;
      curerr = fserr + 3;
      nexterr = fserr + 3 * (pixperline + 2);
      dir = 1;
    }
    int dir3 = dir * 3;

    // We will keep the errors for pixels (x+1, y) in the variable "err10",
    // the error for the pixel right below us (x, y + 1) in "err01", and
    // the error at (x + 1, y + 1) in "err11". The error for the pixel at
    // (x - 1, y + 1) will be flushed into the errors array. This way, we
    // will have just one memory read and one memory write per pixel.
    // Well, in fact we have much more (x86 is terribly lacking registers)
    // but anyway they go through the cache.
    int err10r = 0, err01r = 0, err11r = 0;
    int err10g = 0, err01g = 0, err11g = 0;
    int err10b = 0, err01b = 0, err11b = 0;

	int fspix;
    for (fspix = pixperline; fspix; fspix--,
      cursrc += dir, curdst += dir,
      curerr += dir3, nexterr += dir3)
    {
      csRGBpixel srcpix = *cursrc;

      if (transp && transp->eq (srcpix))
      {
        *curdst = 0;
        err10r = err10g = err10b = 0;
        nexterr [0] = err01r; nexterr [1] = err01g; nexterr [2] = err01b;
        err01r = err11r; err01g = err11g; err01b = err11b;
        err11r = err11g = err11b = 0;
        continue;
      }

      int r = srcpix.red   + ((err10r + curerr [0]) / 16);
      if (r < 0) r = 0; if (r > 255) r = 255;

      int g = srcpix.green + ((err10g + curerr [1]) / 16);
      if (g < 0) g = 0; if (g > 255) g = 255;

      int b = srcpix.blue  + ((err10b + curerr [2]) / 16);
      if (b < 0) b = 0; if (b > 255) b = 255;

      uint8 pix = icmap [((r >> (8 - HIST_R_BITS)) << (HIST_G_BITS + HIST_B_BITS)) |
                         ((g >> (8 - HIST_G_BITS)) << HIST_B_BITS) |
                         ((b >> (8 - HIST_B_BITS)))];
      *curdst = pix;

      csRGBpixel realcolor = palette [pix];

      err10r = r - realcolor.red;
      nexterr [0] = err01r + err10r * 3;		// * 3
      err01r = err11r + err10r * 5;			// * 5
      err11r = err10r;					// * 1
      err10r *= 7;					// * 7

      err10g = g - realcolor.green;
      nexterr [1] = err01g + err10g * 3;		// * 3
      err01g = err11g + err10g * 5;			// * 5
      err11g = err10g;					// * 1
      err10g *= 7;					// * 7

      err10b = b - realcolor.blue;
      nexterr [2] = err01b + err10b * 3;		// * 3
      err01b = err11b + err10b * 5;			// * 5
      err11b = err10b;					// * 1
      err10b *= 7;					// * 7
    }
    // flush cached errors into error array
    nexterr [0] = err01r;
    nexterr [1] = err01g;
    nexterr [2] = err01b;

    src += pixperline;
    dst += pixperline;
    odd ^= 1;
    count -= pixperline;
  }
}

void csColorQuantizer::DoRGB (csRGBpixel *image, int pixels, int pixperline,
  uint8 *&outimage, csRGBpixel *&outpalette, int &maxcolors, bool dither)
{
  Begin ();

  Count (image, pixels);
  Palette (outpalette, maxcolors);
  if (dither)
    RemapDither (image, pixels, pixperline, outpalette, maxcolors, outimage);
  else
    Remap (image, pixels, outimage);

  End ();
}

