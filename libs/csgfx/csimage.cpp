/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein
    Contributions made by Ivan Avramovic <ivan@avramovic.com>

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

#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "cssysdef.h"
#include "qint.h"
#include "csgfx/csimage.h"
#include "csgfx/quantize.h"
#include "csutil/util.h"
#include "csutil/debug.h"

//---------------------- Helper functions ---------------------------

#define MIPMAP_NAME	mipmap_0
#define MIPMAP_LEVEL	0
#include "mipmap.inc"

#define MIPMAP_NAME	mipmap_0_t
#define MIPMAP_LEVEL	0
#define MIPMAP_TRANSPARENT
#include "mipmap.inc"

#define MIPMAP_NAME	mipmap_0_p
#define MIPMAP_LEVEL	0
#define MIPMAP_PALETTED
#include "mipmap.inc"

#define MIPMAP_NAME	mipmap_0_pt
#define MIPMAP_LEVEL	0
#define MIPMAP_PALETTED
#define MIPMAP_TRANSPARENT
#include "mipmap.inc"

#define MIPMAP_NAME	mipmap_0_a
#define MIPMAP_LEVEL	0
#define MIPMAP_ALPHA
#include "mipmap.inc"

#define MIPMAP_NAME	mipmap_1
#define MIPMAP_LEVEL	1
#include "mipmap.inc"

#define MIPMAP_NAME	mipmap_1_t
#define MIPMAP_LEVEL	1
#define MIPMAP_TRANSPARENT
#include "mipmap.inc"

#define MIPMAP_NAME	mipmap_1_p
#define MIPMAP_LEVEL	1
#define MIPMAP_PALETTED
#include "mipmap.inc"

#define MIPMAP_NAME	mipmap_1_pt
#define MIPMAP_LEVEL	1
#define MIPMAP_PALETTED
#define MIPMAP_TRANSPARENT
#include "mipmap.inc"

#define MIPMAP_NAME	mipmap_1_a
#define MIPMAP_LEVEL	1
#define MIPMAP_ALPHA
#include "mipmap.inc"

#define MIPMAP_NAME	mipmap_2
#define MIPMAP_LEVEL	2
#include "mipmap.inc"

#define MIPMAP_NAME	mipmap_2_t
#define MIPMAP_LEVEL	2
#define MIPMAP_TRANSPARENT
#include "mipmap.inc"

#define MIPMAP_NAME	mipmap_2_p
#define MIPMAP_LEVEL	2
#define MIPMAP_PALETTED
#include "mipmap.inc"

#define MIPMAP_NAME	mipmap_2_pt
#define MIPMAP_LEVEL	2
#define MIPMAP_PALETTED
#define MIPMAP_TRANSPARENT
#include "mipmap.inc"

#define MIPMAP_NAME	mipmap_2_a
#define MIPMAP_LEVEL	2
#define MIPMAP_ALPHA
#include "mipmap.inc"

#define MIPMAP_NAME	mipmap_3
#define MIPMAP_LEVEL	3
#include "mipmap.inc"

#define MIPMAP_NAME	mipmap_3_t
#define MIPMAP_LEVEL	3
#define MIPMAP_TRANSPARENT
#include "mipmap.inc"

#define MIPMAP_NAME	mipmap_3_p
#define MIPMAP_LEVEL	3
#define MIPMAP_PALETTED
#include "mipmap.inc"

#define MIPMAP_NAME	mipmap_3_pt
#define MIPMAP_LEVEL	3
#define MIPMAP_PALETTED
#define MIPMAP_TRANSPARENT
#include "mipmap.inc"

#define MIPMAP_NAME	mipmap_3_a
#define MIPMAP_LEVEL	3
#define MIPMAP_ALPHA
#include "mipmap.inc"

//-----------------------------------------------------------------------------

// Apply dithering while converting to 8-bits?
bool csImage_dither = false;

SCF_IMPLEMENT_IBASE (csImageFile)
  SCF_IMPLEMENTS_INTERFACE (iImage)
SCF_IMPLEMENT_IBASE_END

csImageFile::csImageFile (int iFormat)
{
  SCF_CONSTRUCT_IBASE (NULL);
  DG_ADDI (this, "NONAME");
  DG_TYPE (this, "csImageFile");
  Image = NULL;
  Palette = NULL;
  fName = NULL;
  Alpha = NULL;
  Format = iFormat;
  has_keycolour = 0;
  keycolour_r = 0;
  keycolour_g = 0;
  keycolour_b = 0;
}

csImageFile::~csImageFile ()
{
  FreeImage ();
  delete [] fName;
  DG_REM (this);
}

int csImageFile::GetWidth ()
{
  return Width;
}

int csImageFile::GetHeight ()
{
  return Height;
}

int csImageFile::GetSize ()
{
  return Width * Height;
}

void *csImageFile::GetImageData ()
{
  return Image;
}

void csImageFile::SetName (const char *iName)
{
  delete [] fName;
  fName = csStrNew (iName);
}

const char *csImageFile::GetName ()
{
  return fName;
}

int csImageFile::GetFormat ()
{
  return Format;
}

csRGBpixel *csImageFile::GetPalette ()
{
  return Palette;
}

UByte *csImageFile::GetAlpha ()
{
  return Alpha;
}

void csImageFile::set_dimensions (int w, int h)
{
  Width = w;
  Height = h;
  FreeImage ();
}

void csImageFile::FreeImage ()
{
  switch (Format & CS_IMGFMT_MASK)
  {
    case CS_IMGFMT_TRUECOLOR:
      delete [] (csRGBpixel *)Image;
      break;
    case CS_IMGFMT_PALETTED8:
      delete [] (UByte *)Image;
      break;
  }
  delete [] Palette;
  delete [] Alpha;
  Image = NULL; Palette = NULL; Alpha = NULL;
}

void csImageFile::Rescale (int newwidth, int newheight)
{
  if (newwidth == Width && newheight == Height)
    return;

  // This is a quick and dirty algorithm and it doesn't do funny things
  // such as blending multiple pixels together or bilinear filtering,
  // just a rough scale. It could be improved by someone in the future.

  unsigned long x, y;
  unsigned long dx = QInt16 (float (Width) / float (newwidth));
  unsigned long dy = QInt16 (float (Height) / float (newheight));

#define RESIZE(pt, field)				\
  {							\
    pt *dst = new pt [newwidth * newheight];		\
    pt *newfield = dst;					\
    y = 0;	\
	int ny, nx;  \
    for (ny = newheight; ny; ny--)			\
    {							\
      pt *src = ((pt *)field) + (y >> 16) * Width;	\
      y += dy; x = 0;					\
      for (nx = newwidth; nx; nx--)			\
      {							\
        *dst++ = src [x >> 16];				\
        x += dx;					\
      }							\
    }							\
    delete [] (pt *)field;				\
    field = newfield;					\
  }

  switch (Format & CS_IMGFMT_MASK)
  {
    case CS_IMGFMT_TRUECOLOR:
      RESIZE (csRGBpixel, Image)
      break;
    case CS_IMGFMT_PALETTED8:
      RESIZE (UByte, Image)
      break;
  }
  if (Alpha)
    RESIZE (UByte, Alpha)

  Width = newwidth;
  Height = newheight;
}

iImage *csImageFile::MipMap (int steps, csRGBpixel *transp)
{
  if ((steps < 0) || (steps > 3))
    return NULL;

  csImageFile* nimg = new csImageFile (Format);
  if (steps == 1 && (Width < 2 || Height < 2) && (Width > 1 || Height > 1))
    nimg->set_dimensions (MAX (1, Width >> steps), MAX( 1, Height >> steps));
  else
    nimg->set_dimensions (Width >> steps, Height >> steps);

  csRGBpixel *mipmap = new csRGBpixel [nimg->Width * nimg->Height];
  if (Alpha)
    nimg->Alpha = new UByte [nimg->Width * nimg->Height];

  int transpidx = -1;
  if (transp && Palette)
    transpidx = closest_index (transp);

  switch (Format & CS_IMGFMT_MASK)
  {
    case CS_IMGFMT_NONE:
    case CS_IMGFMT_PALETTED8:
      if (Image)
        if (transpidx < 0)
	{
	  if (steps == 0)
	    mipmap_0_p (Width, Height, (UByte *)Image, mipmap, Palette);
	  else if (steps == 1)
	    mipmap_1_p (Width, Height, (UByte *)Image, mipmap, Palette);
	  else if (steps == 2)
	    mipmap_2_p (Width, Height, (UByte *)Image, mipmap, Palette);
	  else
	    mipmap_3_p (Width, Height, (UByte *)Image, mipmap, Palette);
	}
        else
	{
	  if (steps == 0)
	    mipmap_0_pt(Width,Height,(UByte*)Image,mipmap,Palette,transpidx);
	  else if (steps == 1)
	    mipmap_1_pt(Width,Height,(UByte*)Image,mipmap,Palette,transpidx);
	  else if (steps == 2)
	    mipmap_2_pt(Width,Height,(UByte*)Image,mipmap,Palette,transpidx);
	  else
	    mipmap_3_pt(Width,Height,(UByte*)Image,mipmap,Palette,transpidx);
	}
      if (Alpha)
      {
	  if (steps == 0)
	    mipmap_0_a (Width, Height, (UByte *)Alpha, nimg->Alpha);
	  else if (steps == 1)
	    mipmap_1_a (Width, Height, (UByte *)Alpha, nimg->Alpha);
	  else if (steps == 2)
	    mipmap_2_a (Width, Height, (UByte *)Alpha, nimg->Alpha);
	  else
	    mipmap_3_a (Width, Height, (UByte *)Alpha, nimg->Alpha);
      }
      break;
    case CS_IMGFMT_TRUECOLOR:
      if (!transp)
      {
	if (steps == 0)
	  mipmap_0 (Width, Height, (csRGBpixel *)Image, mipmap);
	else if (steps == 1)
	  mipmap_1 (Width, Height, (csRGBpixel *)Image, mipmap);
	else if (steps == 2)
	  mipmap_2 (Width, Height, (csRGBpixel *)Image, mipmap);
	else
	  mipmap_3 (Width, Height, (csRGBpixel *)Image, mipmap);
      }
      else
      {
	if (steps == 0)
	  mipmap_0_t (Width, Height, (csRGBpixel *)Image, mipmap, *transp);
	else if (steps == 1)
	  mipmap_1_t (Width, Height, (csRGBpixel *)Image, mipmap, *transp);
	else if (steps == 2)
	  mipmap_2_t (Width, Height, (csRGBpixel *)Image, mipmap, *transp);
	else
	  mipmap_3_t (Width, Height, (csRGBpixel *)Image, mipmap, *transp);
      }
      break;
  }

  nimg->convert_rgba (mipmap);

  return nimg;
}

static inline unsigned sqr (int x)
{
  return (x * x);
}

int csImageFile::closest_index (csRGBpixel *iColor)
{
  if (!Palette)
    return -1;

  int closest_idx = -1;
  unsigned closest_dst = (unsigned)-1;

  int idx;
  for (idx = 0; idx < 256; idx++)
  {
    unsigned dst = sqr (iColor->red   - Palette [idx].red)   * R_COEF_SQ +
                   sqr (iColor->green - Palette [idx].green) * G_COEF_SQ +
                   sqr (iColor->blue  - Palette [idx].blue)  * B_COEF_SQ;
    if (dst == 0)
      return idx;
    if (dst < closest_dst)
    {
      closest_dst = dst;
      closest_idx = idx;
    } /* endif */
  }
  return closest_idx;
}

void csImageFile::CheckAlpha ()
{
  if (!(Format & CS_IMGFMT_ALPHA))
    return;

  int i, pixels = Width * Height;
  bool noalpha = true;
  switch (Format & CS_IMGFMT_MASK)
  {
    case CS_IMGFMT_NONE:
    case CS_IMGFMT_PALETTED8:
      if (Alpha)
        for (i = 0; i < pixels; i++)
          if (Alpha [i] != 255)
          {
            noalpha = false;
            break;
          }
      break;
    case CS_IMGFMT_TRUECOLOR:
      for (i = 0; i < pixels; i++)
        if (((csRGBpixel *)Image) [i].alpha != 255)
        {
          noalpha = false;
          break;
        }
      break;
  }
  if (noalpha)
  {
    if (Alpha)
    { delete [] Alpha; Alpha = NULL; }
    Format &= ~CS_IMGFMT_ALPHA;
  }
}

bool csImageFile::HasKeycolor ()
{
  return has_keycolour;
}

void csImageFile::GetKeycolor (int &r, int &g, int &b) 
{
  r = keycolour_r;
  g = keycolour_g;
  b = keycolour_b;
}

void csImageFile::convert_rgba (csRGBpixel *iImage)
{
  int pixels = Width * Height;

  if ((Format & CS_IMGFMT_MASK) == CS_IMGFMT_ANY)
    Format = (Format & ~CS_IMGFMT_MASK) | CS_IMGFMT_TRUECOLOR;

  switch (Format & CS_IMGFMT_MASK)
  {
    case CS_IMGFMT_NONE:
    case CS_IMGFMT_PALETTED8:
      if (Format & CS_IMGFMT_ALPHA)
      {
        if (!Alpha)
          Alpha = new UByte [pixels];
		int i;
        for (i = 0; i < pixels; i++)
          Alpha [i] = iImage [i].alpha;
      }
      if ((Format & CS_IMGFMT_MASK) == CS_IMGFMT_PALETTED8)
      {
        // The most complex case: reduce an RGB image to a paletted image.
        int maxcolors = 256;
        csQuantizeRGB (iImage, pixels, Width, (UByte *&)Image, Palette,
          maxcolors, csImage_dither);
      }
      delete [] iImage;
      break;
    case CS_IMGFMT_TRUECOLOR:
      if (Image != iImage)
        FreeImage ();
      Image = iImage;
      break;
  }
}

void csImageFile::convert_pal8 (UByte *iImage, csRGBpixel *iPalette,
  int /*nPalColors*/)
{
  int pixels = Width * Height;

  if ((Format & CS_IMGFMT_MASK) == CS_IMGFMT_ANY)
    Format = (Format & ~CS_IMGFMT_MASK) | CS_IMGFMT_PALETTED8;

  switch (Format & CS_IMGFMT_MASK)
  {
    case CS_IMGFMT_NONE:
      delete [] iImage;
      delete [] iPalette;
      break;
    case CS_IMGFMT_PALETTED8:
      Image = iImage;
      Palette = iPalette;
      break;
    case CS_IMGFMT_TRUECOLOR:
    {
      UByte *in = iImage;
      csRGBpixel *out;
      if (Image)
        out = (csRGBpixel *)Image;
      else
        Image = out = new csRGBpixel [pixels];

      if ((Format & CS_IMGFMT_ALPHA) && Alpha)
      {
        UByte *a = Alpha;
        while (pixels--)
        {
          *out = iPalette [*in++];
          out->alpha = *a++;
          out++;
        }
      }
      else
        while (pixels--)
          *out++ = iPalette [*in++];
      delete [] Alpha; Alpha = NULL;
      delete [] iImage;
      delete [] iPalette;
      break;
    }
  }
  if ((Format & CS_IMGFMT_ALPHA)
   && ((Format & CS_IMGFMT_MASK) != CS_IMGFMT_TRUECOLOR)
   && !Alpha)
    Format &= ~CS_IMGFMT_ALPHA;
}

void csImageFile::convert_pal8 (UByte *iImage, csRGBcolor *iPalette, int nPalColors)
{
  csRGBpixel *newpal = new csRGBpixel [256];
  int i;
  for (i = 0; i < nPalColors; i++) // Default csRGBpixel constructor ensures
    newpal [i] = iPalette [i];         // palette past nPalColors is sane.
  convert_pal8 (iImage, newpal);
}

void csImageFile::SetFormat (int iFormat)
{
  int pixels = Width * Height;
  int oldformat = Format;
  void *oldimage = Image;
  Image = NULL;
  Format = iFormat;

  if ((oldformat & CS_IMGFMT_MASK) == CS_IMGFMT_TRUECOLOR)
    convert_rgba ((csRGBpixel *)oldimage);
  else if ((oldformat & CS_IMGFMT_MASK) == CS_IMGFMT_PALETTED8)
  {
    if ((Format & CS_IMGFMT_ALPHA) && !Alpha)
      Alpha = new UByte [pixels];
    convert_pal8 ((UByte *)oldimage, Palette);
  }
  else if ((oldformat & CS_IMGFMT_MASK) == CS_IMGFMT_NONE)
  {
    if ((Format & CS_IMGFMT_ALPHA) && !Alpha)
      Alpha = new UByte [pixels];
    if ((Format & CS_IMGFMT_MASK) == CS_IMGFMT_PALETTED8)
      Image = new UByte [pixels];
    else if ((Format & CS_IMGFMT_MASK) == CS_IMGFMT_TRUECOLOR)
      Image = new csRGBpixel [pixels];
  }
}

iImage *csImageFile::Clone ()
{
  csImageFile* nimg = new csImageFile (Format);
  nimg->Width = Width;
  nimg->Height = Height;
  nimg->fName = NULL;
  nimg->Format = Format;
  nimg->Image = NULL;
  nimg->Palette = NULL;
  nimg->Alpha = NULL;

  int pixels = Width * Height;
  if (Alpha)
  {
    nimg->Alpha = new UByte [pixels];
    memcpy (nimg->Alpha, Alpha, pixels);
  }

  if (Palette)
  {
    nimg->Palette = new csRGBpixel [256];
    memcpy (nimg->Palette, Palette, 256 * sizeof (csRGBpixel));
  }

  if (Image)
  {
    switch (Format & CS_IMGFMT_MASK)
    {
      case CS_IMGFMT_NONE:
        break;
      case CS_IMGFMT_PALETTED8:
        nimg->Image = new UByte [pixels];
        memcpy (nimg->Image, Image, pixels);
        break;
      case CS_IMGFMT_TRUECOLOR:
        nimg->Image = new csRGBpixel [pixels];
        memcpy (nimg->Image, Image, pixels * sizeof (csRGBpixel));
        break;
    }
  }

  return nimg;
}

iImage *csImageFile::Crop ( int x, int y, int width, int height )
{
  if ( x+width > Width || y+height > Height ) return NULL;
  csImageFile* nimg = new csImageFile (Format);
  nimg->Width = width;
  nimg->Height = height;
  nimg->fName = NULL;
  nimg->Format = Format;
  nimg->Image = NULL;
  nimg->Palette = NULL;
  nimg->Alpha = NULL;

  int pixels = width * height;
  int i;
  if (Alpha)
  {
    nimg->Alpha = new UByte [pixels];
    for ( i=0; i<height; i++ )
      memcpy (nimg->Alpha + i*width, Alpha + (i+y)*Width + x, width);
  }

  if (Palette)
  {
    nimg->Palette = new csRGBpixel [256];
    memcpy (nimg->Palette, Palette, 256 * sizeof (csRGBpixel));
  }

  if (Image)
  {
    switch (Format & CS_IMGFMT_MASK)
    {
      case CS_IMGFMT_NONE:
        break;
      case CS_IMGFMT_PALETTED8:
        nimg->Image = new UByte [pixels];
        for ( i=0; i<height; i++ )
          memcpy ( (UByte*)nimg->Image + i*width, (UByte*)Image + (i+y)*Width + x, width);
        break;
      case CS_IMGFMT_TRUECOLOR:
        nimg->Image = new csRGBpixel [pixels];
        for ( i=0; i<height; i++ )
          memcpy ( (csRGBpixel*)nimg->Image + i*width, (csRGBpixel*)Image + (i+y)*Width + x, width * sizeof (csRGBpixel));
        break;
    }
  }

  return nimg;
}

