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

#include "csgfxldr/quantize.h"
#include <string.h>
#include <stdlib.h>

#define CS_LITTLE_ENDIAN

/*
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
 *
 * Next, we take the entire color box and split it into subboxes.
 * The split happens along the longest of R,G,B axis into half
 * (and taking again into account eye sensivity). After which we take
 * again the biggest box, and split it again and so on until we
 * have that much boxes how much colors we want. Then we assign to each
 * box a color index, and compute the median RGB value for each box.
 *
 * To keep memory requirements in reasonable bounds, we scale down R/G/B
 * components to 5/6/4 bits respectively. This produces results that are
 * different from those we'll get with, say, 6/7/6 resolution but its
 * a quite suitable solution, for a quality vs speed choice. I've tried
 * 5/6/5 as well as 6/6/4 and its really hard to say they are better -
 * the results are definitely different but it's hard to say they are
 * really better.
 *
 * WARNING: If the sum of R+G+B bits exceeds 16, you will a need special
 * case for big endian machines for the INDEX_B macro (see below). With
 * big-endian machines the B component starts at bit 8, thus the right
 * shift counter becomes negative. Probably the best solution is to add
 * and #if B_BIT + 8 - HIST_B_BITS - HIST_G_BITS - HIST_R_BITS < 0.
 */
#define HIST_R_BITS	5
#define HIST_G_BITS	6
#define HIST_B_BITS	4

// Amount to shift left R value to get max (hist_r, hist_g, hist_b)
#define HIST_SHIFT_R	1
#define HIST_SHIFT_G	0
#define HIST_SHIFT_B	2

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

// Compute masks for effectively separating R,G and B components from a ULong.
// For a little-endian machine they are respectively
// 0x000000f8, 0x0000fc00 and 0x00f00000
// For a big-endian machine they are respectively
// 0xf8000000, 0x00fc0000 and 0x0000f000
#define R_MASK		((HIST_R_MAX - 1) << (R_BIT + 8 - HIST_R_BITS))
#define G_MASK		((HIST_G_MAX - 1) << (G_BIT + 8 - HIST_G_BITS))
#define B_MASK		((HIST_B_MAX - 1) << (B_BIT + 8 - HIST_B_BITS))
// The following macro extract the respective color components from a ULong
// and transform them into a index in the histogram.
#define INDEX_R(l)	((l & R_MASK) >> (R_BIT + 8 - HIST_R_BITS))
#define INDEX_G(l)	((l & G_MASK) >> (G_BIT + 8 - HIST_G_BITS - HIST_R_BITS))
#define INDEX_B(l)	((l & B_MASK) >> (B_BIT + 8 - HIST_B_BITS - HIST_G_BITS - HIST_R_BITS))
// Calculate index into histogram for given R,G,B components
#define INDEX(r,g,b)	(r + (g << HIST_R_BITS) + (b << (HIST_R_BITS + HIST_G_BITS)))

static UShort *hist;

/*
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
  // The minimal and maximal R
  UByte Rm,Rx;
  // The minimal and maximal G
  UByte Gm,Gx;
  // The minimal and maximal B
  UByte Bm,Bx;
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
  void Set (UByte rm, UByte rx, UByte gm, UByte gx, UByte bm, UByte bx)
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
    for (int b = Bm; b <= Bx; b++)
      for (int g = Gm; g <= Gx; g++)
      {
        UShort *hp = &hist [INDEX (Rm, g, b)];
        for (int r = Rx - Rm; r >= 0; r--, hp++)
          if (*hp)
          {
            PixelCount += *hp;
            ColorCount++;
          } /* endif */
      } /* endfor */
  }
  // Shrink box: move min/max bounds until we hit an existing color
  void Shrink ()
  {
    ShrinkRm (); ShrinkRx ();
    ShrinkGm (); ShrinkGx ();
    ShrinkBm (); ShrinkBx ();
  }
  // Move Rm up until we find pixels that contain this value
  bool csColorBox::ShrinkRm ()
  {
    UByte iRm = Rm;
    for (; Rm <= Rx; Rm++)
      for (UByte b = Bm; b <= Bx; b++)
      {
        UShort *hp = &hist [INDEX (Rm, Gm, b)];
        for (int g = Gx - Gm; g >= 0; g--, hp += HIST_R_MAX)
          if (*hp) return (Rm != iRm);
      }
    return (Rm != iRm);
  }
  // Move Rx down until we find pixels that contain this value
  bool csColorBox::ShrinkRx ()
  {
    UByte iRx = Rx;
    for (; Rx >= Rm; Rx--)
      for (UByte b = Bm; b <= Bx; b++)
      {
        UShort *hp = &hist [INDEX (Rx, Gm, b)];
        for (int g = Gx - Gm; g >= 0; g--, hp += HIST_R_MAX)
          if (*hp) return (Rx != iRx);
      }
    return (Rx != iRx);
  }
  // Move Gm up until we find pixels that contain this value
  bool csColorBox::ShrinkGm ()
  {
    UByte iGm = Gm;
    for (; Gm <= Gx; Gm++)
      for (UByte b = Bm; b <= Bx; b++)
      {
        UShort *hp = &hist [INDEX (Rm, Gm, b)];
        for (int r = Rx - Rm; r >= 0; r--, hp++)
          if (*hp) return (Gm != iGm);
      }
    return (Gm != iGm);
  }
  // Move Gx down until we find pixels that contain this value
  bool csColorBox::ShrinkGx ()
  {
    UByte iGx = Gx;
    for (; Gx >= Gm; Gx--)
      for (UByte b = Bm; b <= Bx; b++)
      {
        UShort *hp = &hist [INDEX (Rm, Gx, b)];
        for (int r = Rx - Rm; r >= 0; r--, hp++)
          if (*hp) return (Gx != iGx);
      }
    return (Gx != iGx);
  }
  // Move Bm up until we find pixels that contain this value
  bool csColorBox::ShrinkBm ()
  {
    UByte iBm = Bm;
    for (; Bm <= Bx; Bm++)
      for (UByte g = Gm; g <= Gx; g++)
      {
        UShort *hp = &hist [INDEX (Rm, g, Bm)];
        for (int r = Rx - Rm; r >= 0; r--, hp++)
          if (*hp) return (Bm != iBm);
      }
    return (Bm != iBm);
  }
  // Move Bx down until we find pixels that contain this value
  bool csColorBox::ShrinkBx ()
  {
    UByte iBx = Bx;
    for (; Bx >= Bm; Bx--)
      for (UByte g = Gm; g <= Gx; g++)
      {
        UShort *hp = &hist [INDEX (Rm, g, Bx)];
        for (int r = Rx - Rm; r >= 0; r--, hp++)
          if (*hp) return (Bx != iBx);
      }
    return (Bx != iBx);
  }
  /**
   * Compute the mean color for this box.
   * The computation is performed by taking into account each color's
   * weight, i.e. number of pixels with this color. Thus resulting palette
   * is biased towards most often used colors.
   */
  void GetMeanColor (RGBPixel &color)
  {
    unsigned rs = 0, gs = 0, bs = 0;
    unsigned count = 0;
    for (int b = Bm; b <= Bx; b++)
      for (int g = Gm; g <= Gx; g++)
      {
        UShort *hp = &hist [INDEX (Rm, g, b)];
        for (int r = Rm; r <= Rx; r++, hp++)
          if (*hp)
          {
            unsigned pixc = *hp;
            count += pixc;
            rs += pixc * r;
            gs += pixc * g;
            bs += pixc * b;
          } /* endif */
      } /* endfor */
    color.red   = ((rs + count / 2) << (8 - HIST_R_BITS)) / count;
    color.green = ((gs + count / 2) << (8 - HIST_G_BITS)) / count;
    color.blue  = ((bs + count / 2) << (8 - HIST_B_BITS)) / count;
  }
  void FillInverseCMap (UByte *icmap, UByte index)
  {
    int Rcount = Rx - Rm + 1;
    for (int b = Bm; b <= Bx; b++)
      for (int g = Gm; g <= Gx; g++)
        memset (&icmap [INDEX (Rm, g, b)], index, Rcount);
  }
};

static csColorBox *box;

static int compare_boxes (const void *i1, const void *i2)
{
  int count1 = box [*(UByte *)i1].PixelCount;
  int count2 = box [*(UByte *)i2].PixelCount;
  return (count1 < count2) ? -1 : (count1 == count2) ? 0 : +1;
}

void csQuantizeRGB (RGBPixel *image, int pixels, int palsize,
  UByte *&outimage, RGBPixel *&outpalette)
{
  // Sanity check
  if (!pixels || !palsize)
    return;

  // First, allocate the histogram
  hist = new UShort [HIST_R_MAX * HIST_G_MAX * HIST_B_MAX];
  memset (hist, 0, HIST_R_MAX * HIST_G_MAX * HIST_B_MAX);

  // Now, count all colors in image
  ULong *src = (ULong *)image;
  int count = pixels;
  while (count--)
  {
    ULong pix = *src++;
    UShort &pa = hist [INDEX_R (pix) + INDEX_G (pix) + INDEX_B (pix)];
    // do not permit overflow here; stick to MAX_USHORT
    if (!++pa) --pa;
  }

  // Good. Now we create the array of color space boxes.
  box = new csColorBox [palsize];
  int boxcount = 1;
  box [0].Set (0, HIST_R_MAX - 1, 0, HIST_G_MAX - 1, 0, HIST_B_MAX - 1);
  box [0].Shrink ();
  box [0].ComputeVolume ();
  box [0].CountPixels ();

  // Loop until we have enough boxes (or we're out of pixels)
  while (boxcount < palsize)
  {
    // Find the box that should be split
    // We're making this decision the following way:
    // - first half of palette we prefer to split boxes that are
    // most populated with different colors.
    // - the rest of palette we prefer to split largest boxes.
    int bi, bestbox = -1;
    unsigned bestrating = 0;
    if (boxcount < palsize / 2)
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
  UByte Index [palsize];
  for (count = 0; count < boxcount; count++)
    Index [count] = count;
  // Sort palette indices by usage (a side bonus to quantization)
  qsort (Index, boxcount, sizeof (UByte), compare_boxes);

  // Allocate the picture and the palette
  if (!outimage) outimage = new UByte [pixels];
  if (!outpalette) outpalette = new RGBPixel [palsize];

  // Now compute the mean color for each box
  for (count = 0; count < boxcount; count++)
    box [count].GetMeanColor (outpalette [Index [count]]);

  // Fill the unused colormap entries with zeros
  memset (&outpalette [boxcount], 0, (palsize - boxcount) * sizeof (RGBPixel));

  // We will re-use the histogram memory for a inverse colormap. However, we
  // will need just a byte per element, so we'll assign the address of
  // histogram memory block to a pointer of suitable type, and the second
  // half of histogram storage remains unused.
  UByte *icmap = (UByte *)hist;

  // Now, fill inverse colormap with color indices
  for (count = 0; count < boxcount; count++)
    box [count].FillInverseCMap (icmap, Index [count]);

  src = (ULong *)image;
  UByte *dst = outimage;
  count = pixels;
  while (count--)
  {
    ULong pix = *src++;
    *dst++ = icmap [INDEX_R (pix) + INDEX_G (pix) + INDEX_B (pix)];
  }

  delete [] box;
  delete [] hist;
}
