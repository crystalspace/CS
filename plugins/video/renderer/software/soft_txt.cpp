/*
    Copyright (C) 1998 by Jorrit Tyberghein

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

#include <math.h>
#include <stdarg.h>

#include "sysdef.h"
#include "cs3d/software/soft_txt.h"
#include "cs3d/common/inv_cmap.h"
#include "csgfxldr/boxfilt.h"
#include "csutil/scanstr.h"
#include "isystem.h"
#include "iimage.h"
#include "lightdef.h"

#if defined(COMP_MWERKS) && defined(PROC_POWERPC)
#if ! __option( global_optimizer )
#pragma global_optimizer on
#endif
#endif

#define RESERVED_COLOR(c) ((c == 0) || (c == 255))

//---------------------------------------------------------------------------

TxtCmapPrivate::TxtCmapPrivate ()
{
  int i;
  for (i = 0 ; i < 256 ; i++)
  {
    alloc[i] = false;
    rgb_values[(i<<2)+0] = 0;
    rgb_values[(i<<2)+1] = 0;
    rgb_values[(i<<2)+2] = 0;
    rgb_values[(i<<2)+3] = 0;
    priv_to_global[i] = 0;
  }
}

int TxtCmapPrivate::find_rgb (int r, int g, int b)
{
  if (r>255) r=255; else if (r<0) r=0;
  if (g>255) g=255; else if (g<0) g=0;
  if (b>255) b=255; else if (b<0) b=0;

  int i, min, mindist;
  mindist = 1000L*256*256;
  min = -1;
  register int red, green, blue, dist;
  for (i = 1 ; i < 256 ; i++) // Color 0 is not used, it is reserved for transparency
    if (alloc[i])
    {
      red = r - rgb_values[(i<<2)+0];
      green = g - rgb_values[(i<<2)+1];
      blue = b - rgb_values[(i<<2)+2];
      dist = (299*red*red) + (587*green*green) + (114*blue*blue);
      if (dist == 0) return i;
      if (dist < mindist) { mindist = dist; min = i; }
    }
  return min;
}

int TxtCmapPrivate::alloc_rgb (int r, int g, int b, int dist)
{
  int d, j;
  if (r < 0) r = 0; else if (r > 255) r = 255;
  if (g < 0) g = 0; else if (g > 255) g = 255;
  if (b < 0) b = 0; else if (b > 255) b = 255;

  d = 0;
  int i = find_rgb (r, g, b);
  if (i != -1)
  {
    d = 299*(r-rgb_values[(i<<2)+0])*(r-rgb_values[(i<<2)+0])+
      587*(g-rgb_values[(i<<2)+1])*(g-rgb_values[(i<<2)+1])+
      114*(b-rgb_values[(i<<2)+2])*(b-rgb_values[(i<<2)+2]);
  }
  if (i == -1 || d > dist)
  {
    for (j = 1 ; j < 256 ; j++) // Color 0 is not used, reserved for transparency
      if (!alloc[j])
      {
        alloc[j] = true;
        rgb_values[(j<<2)+0] = r;
        rgb_values[(j<<2)+1] = g;
        rgb_values[(j<<2)+2] = b;
        return j;
      }
    return i; // We couldn't allocate a new color, return best fit
  }
  else return i;
}

//---------------------------------------------------------------------------

csTextureMMSoftware::csTextureMMSoftware (IImageFile* image) : csTextureMM (image)
{
  priv_cmap = NULL;
}

csTextureMMSoftware::~csTextureMMSoftware ()
{
  CHK (delete priv_cmap);
}

void csTextureMMSoftware::convert_to_internal (csTextureManager* tex,
	IImageFile* imfile, unsigned char* bm)
{
  csTextureManagerSoftware* texs = (csTextureManagerSoftware*)tex;
  if (texs->txtMode == TXT_GLOBAL)
    convert_to_internal_global (texs, imfile, bm);
  else if (texs->txtMode == TXT_24BIT)
    convert_to_internal_24bit (texs, imfile, bm);
  else
    convert_to_internal_private (texs, imfile, bm);
}

void csTextureMMSoftware::convert_to_internal_global (csTextureManagerSoftware* tex,
	IImageFile* imfile, unsigned char* bm)
{
  int s;
  imfile->GetSize (s);
  RGBPixel *bmsrc;
  imfile->GetImageData (&bmsrc);
  if (get_transparent ())
    for (; s > 0; s--, bmsrc++)
      if (transp_color == *bmsrc)
        *bm++ = 0;
      else
        *bm++ = tex->find_rgb (bmsrc->red, bmsrc->green, bmsrc->blue);
  else
    for (; s > 0; s--, bmsrc++)
      *bm++ = tex->find_rgb (bmsrc->red, bmsrc->green, bmsrc->blue);
}

void csTextureMMSoftware::convert_to_internal_24bit (csTextureManagerSoftware* tex,
	IImageFile* imfile, unsigned char* bm)
{
  int s;
  imfile->GetSize (s);
  RGBPixel* bmsrc;
  imfile->GetImageData (&bmsrc);
  ULong *bml = (ULong*)bm;
  if (get_transparent ())
    for (; s > 0; s--, bmsrc++)
      if (transp_color == *bmsrc)
        *bml++ = 0;
      else
        *bml++ = (bmsrc->red << 16) | (bmsrc->green << 8) | bmsrc->blue;
  else
    for (; s > 0; s--, bmsrc++)
      *bml++ = (bmsrc->red << 16) | (bmsrc->green << 8) | bmsrc->blue;
}

void csTextureMMSoftware::convert_to_internal_private (csTextureManagerSoftware* tex,
	IImageFile* imfile, unsigned char* bm)
{
  int s;
  imfile->GetSize (s);
  RGBPixel* bmsrc;
  imfile->GetImageData (&bmsrc);
  if (get_transparent ())
    for (; s > 0; s--, bmsrc++)
      if (transp_color == *bmsrc)
        *bm++ = 0;
      else
        *bm++ = priv_cmap->find_rgb (bmsrc->red, bmsrc->green, bmsrc->blue);
  else
    for (; s > 0; s--, bmsrc++)
      *bm++ = priv_cmap->find_rgb (bmsrc->red, bmsrc->green, bmsrc->blue);
}

void csTextureMMSoftware::remap_texture (csTextureManager* new_palette)
{
  if (!ifile) return;
  csTextureManagerSoftware* psoft = (csTextureManagerSoftware*)new_palette;

  if (for_2d ())
    if (psoft->get_display_depth () == 8)
      remap_palette_global (psoft, true);
    else if (psoft->get_display_depth () == 16)
      remap_texture_16 (psoft);
    else
      remap_texture_32 (psoft);

  if (for_3d ())
    if (psoft->txtMode == TXT_GLOBAL)
      remap_palette_global (psoft);
    else if (psoft->txtMode == TXT_24BIT)
      remap_palette_24bit (psoft);
    else
      remap_palette_private (psoft);
}

void csTextureMMSoftware::remap_palette_global (csTextureManagerSoftware* new_palette, bool do_2d)
{
  compute_color_usage ();
  if (!usage) return;

  int* trans;
  int num_col = usage->get_num_colors ();
  int i;

  CHK (trans = new int [num_col]);
  for (i = 0 ; i < num_col ; i++)
  {
    if (get_transparent () && (transp_color == get_usage (i)))
      trans[i] = 0;
    else
      trans[i] = ((csTextureManagerSoftware*)new_palette)->find_rgb
        (get_usage (i).red, get_usage (i).green, get_usage (i).blue);
  }

  RGBPixel* src;
  ifile->GetImageData (&src);

  unsigned char *dest, *last;
  if (do_2d)
    dest = t2d->get_bitmap8 ();
  else
    dest = t1->get_bitmap8 ();
  int size;
  ifile->GetSize (size);
  last = dest + size;

  for (; dest < last; dest++, src++)
  {
    for (i = 0 ; i < num_col ; i++)
      if (*src == get_usage(i))
        break;
    *dest = trans[i];
  }

  CHK (delete [] trans);
}

void csTextureMMSoftware::remap_palette_private (csTextureManagerSoftware* new_palette)
{
  compute_color_usage ();
  if (!usage)
    return;

  int* trans;
  int num_col = usage->get_num_colors ();
  int i;

  CHK (delete priv_cmap); priv_cmap = NULL;
  CHK (priv_cmap = new TxtCmapPrivate);

  // First allocate colors in the private palette.
  CHK (trans = new int [num_col]);

  // Minimum allowed distance, the most used colors should use
  // a very low distance.
  int dist, ddist;
  if (num_col < 256)
  {
    // If the number of colors in our texture is less than 256 we
    // set the allocator to exact mode. This will simply copy the
    // palette.
    dist = 0;
    ddist = 0;
  }
  else if (num_col < 400)
  {
    // If we have less than 400 colors then we prefer a lower distance
    // in general.
    dist = 500; // @@@ Experiment with these values
    ddist = 50;
  }
  else
  {
    dist = 1000; // @@@ Experiment with these values
    ddist = 100;
  }
  for (i = 0 ; i < num_col ; i++)
  {
    if (get_transparent () && (transp_color == get_usage (i)))
      trans [i] = 0;
    else
      trans [i] = priv_cmap->alloc_rgb (get_usage(i).red, get_usage(i).green,
        get_usage(i).blue, dist);
    dist += ddist;
  }

  int w, h;
  ifile->GetWidth (w);
  ifile->GetHeight (h);
  RGBPixel* src;
  ifile->GetImageData (&src);
  unsigned char* dest = t1->get_bitmap8 ();

  // Map the texture to the private palette.
  int x, y;
  for (y = 0 ; y < h ; y++)
    for (x = 0 ; x < w ; x++)
    {
      for (i = 0 ; i < num_col ; i++)
        if (src->red == get_usage(i).red
         && src->green == get_usage(i).green
         && src->blue == get_usage(i).blue)
          break;
      *dest++ = trans[i];
      src++;
    }

  CHK (delete [] trans);

  // Make a table to convert the private colormap to the global colormap.
  for (i = 0 ; i < num_col ; i++)
    priv_cmap->priv_to_global[i] =
      ((csTextureManagerSoftware*)new_palette)->find_rgb (
         priv_cmap->rgb_values[(i<<2)+0],
         priv_cmap->rgb_values[(i<<2)+1],
         priv_cmap->rgb_values[(i<<2)+2]);
}

void csTextureMMSoftware::remap_palette_24bit (csTextureManagerSoftware*)
{
  compute_color_usage ();
  if (!usage)
    return;

  int size;
  ifile->GetSize (size);
  RGBPixel* src;
  ifile->GetImageData (&src);
  ULong *dest = t1->get_bitmap32 ();
  ULong *last = dest + size;

  // Map the texture to the RGB palette.
  for (; dest < last; src++)
    *dest++ = (src->red << 16) | (src->green << 8) | src->blue;
}

void csTextureMMSoftware::remap_texture_16 (csTextureManagerSoftware* new_palette)
{
  int size;
  ifile->GetSize (size);
  RGBPixel *src;
  ifile->GetImageData (&src);
  UShort *dest = t2d->get_bitmap16 ();
  UShort *last = dest + size;
  UShort black = new_palette->get_almost_black ();
  if (get_transparent ())
    for (; dest < last; src++, dest++)
      if (transp_color == *src)
        *dest = 0;
      else
      {
        UShort texel = new_palette->find_color (src->red, src->green, src->blue);
        *dest = texel ? texel : black;
      }
  else
    for (; dest < last; src++, dest++)
      *dest = new_palette->find_color (src->red, src->green, src->blue);
}

void csTextureMMSoftware::remap_texture_32 (csTextureManagerSoftware* new_palette)
{
  int size;
  ifile->GetSize (size);
  RGBPixel* src;
  ifile->GetImageData (&src);
  ULong *dest = t2d->get_bitmap32 ();
  ULong *last = dest + size;
  ULong black = new_palette->get_almost_black ();
  if (get_transparent ())
    for (; dest < last; src++, dest++)
      if (transp_color == *src)
        *dest = 0;
      else
      {
        ULong texel = new_palette->find_color (src->red, src->green, src->blue);
        *dest = texel ? texel : black;
      }
  else
    for (; dest < last; src++, dest++)
      *dest = new_palette->find_color (src->red, src->green, src->blue);
}

//---------------------------------------------------------------------------

UShort csTextureManagerSoftware::alpha_mask;

csTextureManagerSoftware::csTextureManagerSoftware (ISystem* piSystem, IGraphics2D* piG2D) :
	csTextureManager (piSystem, piG2D)
{
  txtMode = TXT_GLOBAL;
  force_txtMode = -1;
}

void csTextureManagerSoftware::InitSystem ()
{
  csTextureManager::InitSystem ();

  lt_truergb = NULL;
  lt_truergb_private = NULL;
  lt_white16 = NULL;
  lt_white8 = NULL;
  lt_pal = NULL;
  lt_alpha = NULL;

  truecolor = pfmt.PalEntries == 0;
  if (!truecolor)
  {
    // If we don't have truecolor we simulate 6:6:4 bits
    // for R:G:B in the masks anyway because we still need the
    // 16-bit format for our light mixing (in true_rgb mode).
    pfmt.RedMask = MASK_RED << (BITS_GREEN+BITS_BLUE);
    pfmt.RedShift = BITS_GREEN+BITS_BLUE;
    pfmt.GreenMask = MASK_GREEN << BITS_BLUE;
    pfmt.GreenShift = BITS_BLUE;
    pfmt.BlueMask = MASK_BLUE;
    pfmt.BlueShift = 0;
    long m;
    pfmt.RedBits = 0;  m = pfmt.RedMask >> pfmt.RedShift;    while (m) { pfmt.RedBits++; m >>= 1; }
    pfmt.GreenBits = 0;m = pfmt.GreenMask >> pfmt.GreenShift;while (m) { pfmt.GreenBits++; m >>= 1; }
    pfmt.BlueBits = 0; m = pfmt.BlueMask >> pfmt.BlueShift;  while (m) { pfmt.BlueBits++; m >>= 1; }
  }
  num_red = (pfmt.RedMask >> pfmt.RedShift) + 1;
  num_green = (pfmt.GreenMask >> pfmt.GreenShift) + 1;
  num_blue = (pfmt.BlueMask >> pfmt.BlueShift) + 1;

  alpha_mask = 0;
  alpha_mask |= 1 << (pfmt.RedShift);
  alpha_mask |= 1 << (pfmt.GreenShift);
  alpha_mask |= 1 << (pfmt.BlueShift);
  alpha_mask = ~alpha_mask;
}

bool csTextureManagerSoftware::force_txtmode (char* p)
{
  if (!strcmp (p, "global")) force_txtMode = TXT_GLOBAL;
  else if (!strcmp (p, "private")) force_txtMode = TXT_PRIVATE;
  else if (!strcmp (p, "24bit")) force_txtMode = TXT_24BIT;
  else
  {
    SysPrintf (MSG_FATAL_ERROR, "Bad value '%s' for TXTMODE (use 'global', 'private', or '24bit')!\n", p);
    return false;
  }
  return true;
}

bool csTextureManagerSoftware::force_mixing (char* mix)
{
  if (!strcmp (mix, "true_rgb")) force_mix = MIX_TRUE_RGB;
  else if (!strcmp (mix, "nocolor")) force_mix = MIX_NOCOLOR;
  else
  {
    SysPrintf (MSG_FATAL_ERROR, "Bad value '%s' for 'mixing' (use 'true_rgb' or 'nocolor')!\n", mix);
    return false;
  }
  return true;
}

void csTextureManagerSoftware::read_config ()
{
  char* p;
  ISystem* sys = m_piSystem;
  // @@@ WARNING! The following code only examines the
  // main cryst.cfg file and not the one which overrides values
  // in the world file. We need to support this someway in the ISystem
  // interface as well.

  sys->ConfigGetYesNo ("TextureMapper", "BLEND_MIPMAP", do_blend_mipmap0, false);

  sys->ConfigGetStr ("TextureMapper", "MIPMAP_FILTER_1", p, "-");
  if (*p != '-')
  {
    ScanStr (p, "%d,%d,%d,%d,%d,%d,%d,%d,%d",
      &mipmap_filter_1.f11, &mipmap_filter_1.f12, &mipmap_filter_1.f13,
      &mipmap_filter_1.f21, &mipmap_filter_1.f22, &mipmap_filter_1.f23,
      &mipmap_filter_1.f31, &mipmap_filter_1.f32, &mipmap_filter_1.f33);
    mipmap_filter_1.tot =
      mipmap_filter_1.f11+mipmap_filter_1.f12+mipmap_filter_1.f13+
      mipmap_filter_1.f21+mipmap_filter_1.f22+mipmap_filter_1.f23+
      mipmap_filter_1.f31+mipmap_filter_1.f32+mipmap_filter_1.f33;
  }
  sys->ConfigGetStr ("TextureMapper", "MIPMAP_FILTER_2", p, "-");
  if (*p != '-')
  {
    ScanStr (p, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
      &mipmap_filter_2.f00, &mipmap_filter_2.f01, &mipmap_filter_2.f02, &mipmap_filter_2.f03, &mipmap_filter_2.f04,
      &mipmap_filter_2.f10, &mipmap_filter_2.f11, &mipmap_filter_2.f12, &mipmap_filter_2.f13, &mipmap_filter_2.f14,
      &mipmap_filter_2.f20, &mipmap_filter_2.f21, &mipmap_filter_2.f22, &mipmap_filter_2.f23, &mipmap_filter_2.f24,
      &mipmap_filter_2.f30, &mipmap_filter_2.f31, &mipmap_filter_2.f32, &mipmap_filter_2.f33, &mipmap_filter_2.f34,
      &mipmap_filter_2.f40, &mipmap_filter_2.f41, &mipmap_filter_2.f42, &mipmap_filter_2.f43, &mipmap_filter_2.f44);
    mipmap_filter_2.tot =
      mipmap_filter_2.f00+mipmap_filter_2.f01+mipmap_filter_2.f02+mipmap_filter_2.f03+mipmap_filter_2.f04+
      mipmap_filter_2.f10+mipmap_filter_2.f11+mipmap_filter_2.f12+mipmap_filter_2.f13+mipmap_filter_2.f14+
      mipmap_filter_2.f20+mipmap_filter_2.f21+mipmap_filter_2.f22+mipmap_filter_2.f23+mipmap_filter_2.f24+
      mipmap_filter_2.f30+mipmap_filter_2.f31+mipmap_filter_2.f32+mipmap_filter_2.f33+mipmap_filter_2.f34+
      mipmap_filter_2.f40+mipmap_filter_2.f41+mipmap_filter_2.f42+mipmap_filter_2.f43+mipmap_filter_2.f44;
  }
  sys->ConfigGetStr ("TextureMapper", "BLEND_FILTER", p, "-");
  if (*p != '-')
  {
    ScanStr (p, "%d,%d,%d,%d,%d,%d,%d,%d,%d",
      &blend_filter.f11, &blend_filter.f12, &blend_filter.f13,
      &blend_filter.f21, &blend_filter.f22, &blend_filter.f23,
      &blend_filter.f31, &blend_filter.f32, &blend_filter.f33);
    blend_filter.tot =
      blend_filter.f11+blend_filter.f12+blend_filter.f13+
      blend_filter.f21+blend_filter.f22+blend_filter.f23+
      blend_filter.f31+blend_filter.f32+blend_filter.f33;
  }

  sys->ConfigGetInt ("World", "RGB_DIST", prefered_dist, PREFERED_DIST);
  sys->ConfigGetInt ("World", "RGB_COL_DIST", prefered_col_dist, PREFERED_COL_DIST);
  sys->ConfigGetStr ("TextureMapper", "MIPMAP_NICE", p, "nice");
  if (!strcmp (p, "nice"))
  {
    mipmap_nice = MIPMAP_NICE;
    if (verbose) SysPrintf (MSG_INITIALIZATION, "Mipmap calculation 'nice'.\n");
  }
  else if (!strcmp (p, "ugly"))
  {
    mipmap_nice = MIPMAP_UGLY;
    if (verbose) SysPrintf (MSG_INITIALIZATION, "Mipmap calculation 'ugly'.\n");
  }
  else if (!strcmp (p, "default"))
  {
    mipmap_nice = MIPMAP_DEFAULT;
    if (verbose) SysPrintf (MSG_INITIALIZATION, "Mipmap calculation 'default'.\n");
  }
  else if (!strcmp (p, "verynice"))
  {
    mipmap_nice = MIPMAP_VERYNICE;
    if (verbose) SysPrintf (MSG_INITIALIZATION, "Mipmap calculation 'verynice'\n  (Note: this is expensive for the texture cache)\n");
  }
  else
  {
    SysPrintf (MSG_FATAL_ERROR, "Bad value '%s' for MIPMAP_NICE!\n(Use 'verynice', 'nice', 'ugly', or 'default')\n", p);
    exit (0);	//@@@
  }

  if (force_mix != -1) mixing = force_mix;
  else
  {
    char buf[100];
    sys->ConfigGetStr ("World", "MIXLIGHTS", p, "true_rgb");
    strcpy (buf, p);

    if (!strcmp (p, "true_rgb")) mixing = MIX_TRUE_RGB;
    else if (!strcmp (p, "nocolor")) mixing = MIX_NOCOLOR;
    else
    {
      SysPrintf (MSG_FATAL_ERROR, "Bad value '%s' for MIXLIGHTS (use 'true_rgb' or 'nocolor')!\n", p);
      exit (0); //@@@
    }
  }

  if (force_txtMode == -1 && pfmt.PixelBytes == 4)
  {
    if (verbose) SysPrintf (MSG_INITIALIZATION, "Texture mode forced to '24bit' because we are in 32-bit mode.\n");
    force_txtMode = TXT_24BIT;
  }

  if (force_txtMode != -1) txtMode = force_txtMode;
  else
  {
    char buf[100];
    sys->ConfigGetStr ("World", "TXTMODE", p, "global");
    strcpy (buf, p);

    if (!strcmp (p, "global")) txtMode = TXT_GLOBAL;
    else if (!strcmp (p, "private")) txtMode = TXT_PRIVATE;
    else if (!strcmp (p, "24bit")) txtMode = TXT_24BIT;
    else
    {
      SysPrintf (MSG_FATAL_ERROR, "Bad value '%s' for TXTMODE (use 'global', 'private', or '24bit')!\n", p);
      exit (0); //@@@
    }
  }

  if (!truecolor && txtMode == TXT_24BIT)
  {
    SysPrintf (MSG_FATAL_ERROR, "TXTMODE '24bit' not yet supported in 8-bit display mode!\n");
    exit (0); //@@@
  }

  if (truecolor && pfmt.PixelBytes == 4)
    { if (verbose) SysPrintf (MSG_INITIALIZATION, "Truecolor mode (32 bit).\n"); }
  else if (truecolor)
    { if (verbose) SysPrintf (MSG_INITIALIZATION, "Truecolor mode (15/16 bit).\n"); }
  if (truecolor && mixing != MIX_TRUE_RGB)
  {
    if (verbose) SysPrintf (MSG_INITIALIZATION, "  Mixing mode forced to true_rgb.\n");
    mixing = MIX_TRUE_RGB;
  }
  if (txtMode != TXT_GLOBAL && mixing != MIX_TRUE_RGB)
  {
    if (verbose) SysPrintf (MSG_INITIALIZATION, "Private or 24-bit textures: mixing mode forced to true_rgb.\n");
    mixing = MIX_TRUE_RGB;
  }

  if (force_mix != -1) mixing = force_mix;
  if (force_txtMode != -1) txtMode = force_txtMode;

  if (truecolor)
  {
    mixing = MIX_TRUE_RGB;
  }
  if (txtMode != TXT_GLOBAL)
  {
    mixing = MIX_TRUE_RGB;
  }

  use_rgb = mixing == MIX_TRUE_RGB;

  switch (txtMode)
  {
    case TXT_GLOBAL:
      if (verbose) SysPrintf (MSG_INITIALIZATION, "One global palette for all textures.\n");
      break;
    case TXT_PRIVATE:
      if (verbose) SysPrintf (MSG_INITIALIZATION, "One private palette for every texture.\n");
      break;
    case TXT_24BIT:
      if (verbose) SysPrintf (MSG_INITIALIZATION, "Code all textures in 24-bit.\n");
      break;
  }

  if (verbose)
    if (mixing == MIX_TRUE_RGB)
      SysPrintf (MSG_INITIALIZATION, "Use RGB light mixing.\n");
    else
      SysPrintf (MSG_INITIALIZATION, "Use colorless lights.\n");
}

csTextureManagerSoftware::~csTextureManagerSoftware ()
{
  clear ();
}

void csTextureManagerSoftware::clear ()
{
  csTextureManager::clear ();
  int i;
  for (i = 0 ; i < textures.Length () ; i++)
  {
    CHK (delete (csTextureMMSoftware*)(textures[i]));
    textures[i] = NULL;
  }
  textures.DeleteAll ();
  CHK (delete lt_truergb); lt_truergb = NULL;
  CHK (delete lt_truergb_private); lt_truergb_private = NULL;
  CHK (delete lt_white16); lt_white16 = NULL;
  CHK (delete lt_white8); lt_white8 = NULL;
  CHK (delete lt_pal); lt_pal = NULL;
  CHK (delete lt_alpha); lt_alpha = NULL;
}

csTextureMMSoftware* csTextureManagerSoftware::new_texture (IImageFile* image)
{
  CHK (csTextureMMSoftware* tm = new csTextureMMSoftware (image));
  if (tm->loaded_correctly ()) textures.Push (tm);
  return tm;
}

csTexture* csTextureManagerSoftware::get_texture (int idx, int lev)
{
  return ((csTextureMMSoftware*)textures[idx])->get_texture (lev);
}

#define GAMMA(c) ((int)(256.*pow (((float)(c))/256., 1./Gamma)))

int csTextureManagerSoftware::find_rgb_real (int r, int g, int b)
{
  if (pfmt.PalEntries != 0)
  {
#if defined( OS_MACOS )
    if (r == 0 && g == 0 && b == 0) return 255;
    if (r == 255 && g == 255 && b == 255) return 0;
#else
    if (r == 0 && g == 0 && b == 0) return 0;
    if (r == 255 && g == 255 && b == 255) return 255;
#endif /* OS_MACOS */

    int i, min, dist, mindist;
    mindist = 1000*256*256; min = -1;
    int pr, pg, pb;

    for(i = 1 ; i < 255 ; i++) // Exclude colors 0 and 255
      if (alloc[i])
      {
        pr = GAMMA (pal[i].red); if (pr < 0) pr = 0; else if (pr > 255) pr = 255;
        pg = GAMMA (pal[i].green); if (pg < 0) pg = 0; else if (pg > 255) pg = 255;
        pb = GAMMA (pal[i].blue); if (pb < 0) pb = 0; else if (pb > 255) pb = 255;
        dist = 299*(r-pr)*(r-pr)+587*(g-pg)*(g-pg)+114*(b-pb)*(b-pb);
        if (dist == 0) return i;
        if (dist < mindist) { mindist = dist; min = i; }
      }
    return min;
  } else
    return encode_rgb (r, g, b);
}

ULong csTextureManagerSoftware::encode_rgb (int r, int g, int b)
{
  return
    ((r >> (8-pfmt.RedBits))   << pfmt.RedShift) |
    ((g >> (8-pfmt.GreenBits)) << pfmt.GreenShift) |
    ((b >> (8-pfmt.BlueBits))  << pfmt.BlueShift);
}

int csTextureManagerSoftware::find_rgb (int r, int g, int b)
{
  //GAC Now use lookup table
  if (r>255) r=255; else if (r<0) r=0;
  if (g>255) g=255; else if (g<0) g=0;
  if (b>255) b=255; else if (b<0) b=0;

  return lt_palette_table[encode_rgb (r, g, b)];
}

int csTextureManagerSoftware::find_color (int r, int g, int b)
{
  //GAC Now use lookup table
  if (r>255) r=255; else if (r<0) r=0;
  if (g>255) g=255; else if (g<0) g=0;
  if (b>255) b=255; else if (b<0) b=0;

  if (truecolor)
    return encode_rgb (r, g, b);
  else
    return lt_palette_table[encode_rgb (r, g, b)];
}

ULong csTextureManagerSoftware::encode_rgb_safe (int r, int g, int b)
{
  //GAC Now use lookup table
  if (r>255) r=255; else if (r<0) r=0;
  if (g>255) g=255; else if (g<0) g=0;
  if (b>255) b=255; else if (b<0) b=0;

  return encode_rgb (r, g, b);
}

int csTextureManagerSoftware::find_rgb_slow (int r, int g, int b)
{
  if (r>255) r=255; else if (r<0) r=0;
  if (g>255) g=255; else if (g<0) g=0;
  if (b>255) b=255; else if (b<0) b=0;

  int i, min, mindist;
  mindist = 1000L*256*256;
  min = -1;
  register int red, green, blue, dist;
  for(i = 1 ; i < 255 ; i++) // Exclude colors 0 and 255
    if (alloc[i])
    {
      red = r - pal[i].red;
      green = g - pal[i].green;
      blue = b - pal[i].blue;
      dist = (299*red*red) + (587*green*green) + (114*blue*blue);
      if (dist == 0) return i;
      if (dist < mindist) { mindist = dist; min = i; }
    }
  return min;
}

int csTextureManagerSoftware::alloc_rgb (int r, int g, int b, int dist)
{
  int d, j;
  if (r < 0) r = 0; else if (r > 255) r = 255;
  if (g < 0) g = 0; else if (g > 255) g = 255;
  if (b < 0) b = 0; else if (b > 255) b = 255;

  d = 0;
  int i = find_rgb_slow (r, g, b);
  if (i != -1)
  {
    d = 299*(r-pal[i].red)*(r-pal[i].red)+
      587*(g-pal[i].green)*(g-pal[i].green)+
      114*(b-pal[i].blue)*(b-pal[i].blue);
  }
  if (i == -1 || d > dist)
  {
    for (j = 1 ; j < 255 ; j++) // Exclude color 0 and 255
      if (!alloc[j])
      {
        alloc[j] = true;
        pal[j].red = r;
        pal[j].green = g;
        pal[j].blue = b;
        return j;
      }
    return i; // We couldn't allocate a new color, return best fit
  }
  else return i;
}

void csTextureManagerSoftware::create_lt_palette ()
{
  // GAC Fill palette_table for fast lookup using slow version of lookup
  // JTY This works by encoding the R,G,B values into one 16-bit word
  // (by default this is 6:6:4 bits for R:G:B in 8-bit mode). This lowers
  // the precision of our RGB mapper a bit which means that the display
  // will have slightly less quality (you have to look very closely to
  // be able to see this).
  //
  // In truecolor mode we do something similar. Only the number of bits
  // for every color can vary.

  CHK (delete lt_pal);
  CHK (lt_pal = new TextureTablesPalette);
  lt_palette_table = lt_pal->true_to_pal;

  int i;

#ifdef SLOW_create_lt_palette
  int tr, tg, tb, r, g, b;
  for (tr = 0; tr < num_red; tr++)
  {
    for (tg = 0; tg < num_green; tg++)
    {
      for (tb = 0; tb < num_blue; tb++)
      {
        r = tr << (8 - pfmt.RedBits);
        g = tg << (8 - pfmt.GreenBits);
        b = tb << (8 - pfmt.BlueBits);
        lt_palette_table[encode_rgb (r,g,b)] = find_rgb_slow(r, g, b);
      }
    }
  }
#else
  if (pfmt.PixelBytes == 4) return;

  // Greg Ewing, 12 Oct 1998
  unsigned char *colormap[3];
  unsigned long *dist_buf;
  for (i = 0; i < 3; i++)
    CHKB (colormap[i] = new unsigned char[256]);
  for (i = 0; i < 256; i++) {
    colormap[0][i] = pal[i].red;
    colormap[1][i] = pal[i].green;
    colormap[2][i] = pal[i].blue;
  }
  CHK (dist_buf = new unsigned long[num_red * num_green * num_blue]);
  inv_cmap(256, colormap, pfmt.RedBits, pfmt.GreenBits, pfmt.BlueBits,
           dist_buf, lt_palette_table);
  for (i = 0; i < 3; i++)
    CHKB (delete [] colormap[i]);
  CHK (delete [] dist_buf);
  // Color number 0 is reserved for transparency
  lt_palette_table [encode_rgb (pal [0].red, pal [0].green, pal [0].blue)] =
    find_rgb_slow (pal [0].red, pal [0].green, pal [0].blue);
#endif

  for (i = 0 ; i < 256 ; i++)
    lt_pal->pal_to_true[i] = encode_rgb (pal[i].red, pal[i].green, pal[i].blue);
}

void csTextureManagerSoftware::compute_palette ()
{
  int i, t;

  if (verbose) SysPrintf (MSG_INITIALIZATION, "Computing palette...\n");

  for (i = 1 ; i < 255 ; i++)
    alloc[i] = false;

  // First compute the usage of all colors of all textures.
  // These usage lists will be sorted so that the most frequently
  // used color of each textures is put first.
  for (i = 0 ; i < textures.Length () ; i++)
    ((csTextureMMSoftware*)textures[i])->compute_color_usage ();

  // The following code is for allocating an extra color for each
  // color table. This is so that we have a palette which is better
  // suited for colored lighting.
  // This is not needed for truecolor mode.
  int dist = 50;

  // Then allocate colors for all textures at the same time.
  for (i = 0 ; i < 256 ; i++)
  {
    for (t = 0 ; t < textures.Length () ; t++)
    {
      csTextureMMSoftware* txt = (csTextureMMSoftware*)textures[t];
      if (i < txt->get_num_colors ())
      {
        alloc_rgb (txt->get_usage (i).red,
                   txt->get_usage (i).green,
                   txt->get_usage (i).blue,
                   prefered_dist);
        if (!truecolor && mixing == MIX_TRUE_RGB)
        {
          alloc_rgb (txt->get_usage (i).red + dist,
                     txt->get_usage (i).green,
                     txt->get_usage (i).blue,
                     prefered_col_dist);
          alloc_rgb (txt->get_usage (i).red,
                     txt->get_usage (i).green + dist,
                     txt->get_usage (i).blue,
                     prefered_col_dist);
          alloc_rgb (txt->get_usage (i).red,
                     txt->get_usage (i).green,
                     txt->get_usage (i).blue + dist,
                     prefered_col_dist);
        }
      }
    }
  }

  create_lt_palette ();

  // Remap all textures according to the new colormap.
  for (i = 0 ; i < textures.Length () ; i++)
    ((csTextureMMSoftware*)textures[i])->remap_texture (this);

  black_color = 0;
  if (truecolor)
  {
    white_color = encode_rgb (255, 255, 255);
    red_color = encode_rgb (255, 0, 0);
    blue_color = encode_rgb (0, 0, 255);
    yellow_color = encode_rgb (255, 255, 0);
    green_color = encode_rgb (0, 255, 0);
  }
  else
  {
    white_color = 255;
    red_color = find_rgb (255, 0, 0);
    blue_color = find_rgb (0, 0, 255);
    yellow_color = find_rgb (255, 255, 0);
    green_color = find_rgb (0, 255, 0);
  }

  if (verbose) SysPrintf (MSG_INITIALIZATION, "DONE\n");
}

int csTextureManagerSoftware::find_rgb_map (int r, int g, int b, int map_type, int l)
{
  int nr = r, ng = g, nb = b;

  switch (map_type)
  {
    case TABLE_WHITE_HI:
      nr = (NORMAL_LIGHT_LEVEL+l)*r / NORMAL_LIGHT_LEVEL;
      ng = (NORMAL_LIGHT_LEVEL+l)*g / NORMAL_LIGHT_LEVEL;
      nb = (NORMAL_LIGHT_LEVEL+l)*b / NORMAL_LIGHT_LEVEL;
      break;
    case TABLE_WHITE:
      nr = l*r / NORMAL_LIGHT_LEVEL;
      ng = l*g / NORMAL_LIGHT_LEVEL;
      nb = l*b / NORMAL_LIGHT_LEVEL;
      break;
    case TABLE_RED_HI:
      nr = (NORMAL_LIGHT_LEVEL+l)*r / NORMAL_LIGHT_LEVEL;
      break;
    case TABLE_GREEN_HI:
      ng = (NORMAL_LIGHT_LEVEL+l)*g / NORMAL_LIGHT_LEVEL;
      break;
    case TABLE_BLUE_HI:
      nb = (NORMAL_LIGHT_LEVEL+l)*b / NORMAL_LIGHT_LEVEL;
      break;
    case TABLE_RED:
      if (use_rgb) nr = l*r / NORMAL_LIGHT_LEVEL;
      else nr = r+l*NORMAL_LIGHT_LEVEL/256;
      break;
    case TABLE_GREEN:
      if (use_rgb) ng = l*g / NORMAL_LIGHT_LEVEL;
      else ng = g+l*NORMAL_LIGHT_LEVEL/256;
      break;
    case TABLE_BLUE:
      if (use_rgb) nb = l*b / NORMAL_LIGHT_LEVEL;
      else nb = b+l*NORMAL_LIGHT_LEVEL/256;
      break;
  }
  return truecolor ? encode_rgb_safe (nr, ng, nb) : find_rgb (nr, ng, nb);
  //return truecolor ? encode_rgb_safe (nr, ng, nb) :
    //(accurate_rgb_mapper ? find_rgb_slow (nr, ng, nb) : find_rgb (nr, ng, nb));
}

void csTextureManagerSoftware::create_lt_white16 ()
{
  CHK (delete lt_white16);
  CHK (lt_white16 = new TextureTablesWhite16 ());

  int i, l;
  int r, g, b;

  if (verbose) SysPrintf (MSG_INITIALIZATION, "  Calculate 16-bit white tables...\n");

  for (i = 1 ; i < 256 ; i++)
    if (alloc[i])
    {
      r = pal[i].red;
      g = pal[i].green;
      b = pal[i].blue;
      for (l = 0 ; l < 256 ; l++)
      {
        lt_white16->white1_light[l][i] = find_rgb_map (r, g, b, TABLE_WHITE_HI, l);
        lt_white16->white2_light[l][i] = find_rgb_map (r, g, b, TABLE_WHITE, l);
      }
    }

    // Color 0 is mapped to 0 in all cases to make sure that transparent
    // colors remain transparent.
    for (l = 0 ; l < 256 ; l++)
    {
      lt_white16->white1_light[l][0] = 0;
      lt_white16->white2_light[l][0] = 0;
    }
}

void csTextureManagerSoftware::create_lt_white8 ()
{
  CHK (delete lt_white8);
  CHK (lt_white8 = new TextureTablesWhite8 ());

  int i, l;
  int r, g, b;

  if (verbose) SysPrintf (MSG_INITIALIZATION, "  Calculate 8-bit white tables...\n");
  for (i = 1 ; i < 256 ; i++)
    if (alloc[i])
    {
      r = pal[i].red;
      g = pal[i].green;
      b = pal[i].blue;
      for (l = 0 ; l < 256 ; l++)
      {
        lt_white8->white1_light[l][i] = find_rgb_map (r, g, b, TABLE_WHITE_HI, l);
        lt_white8->white2_light[l][i] = find_rgb_map (r, g, b, TABLE_WHITE, l);
      }
    }

  // Color 0 is mapped to 0 in all cases to make sure that transparent
  // colors remain transparent.
  for (l = 0 ; l < 256 ; l++)
  {
    lt_white8->white1_light[l][0] = 0;
    lt_white8->white2_light[l][0] = 0;
  }
}

void csTextureManagerSoftware::create_lt_truergb ()
{
  CHK (delete lt_truergb);
  CHK (lt_truergb = new TextureTablesTrueRgb);
  lt_light = lt_truergb->lut;
  int i, l;
  int r, g, b;
  int tr, tg, tb;  // Temp working vars

  if (verbose) SysPrintf (MSG_INITIALIZATION, "  Calculate tables for true_rgb mode...\n");

  for (i = 1 ; i < 256 ; i++)
    if (alloc[i])
    {
      r = pal[i].red;
      g = pal[i].green;
      b = pal[i].blue;
      for (l = 0 ; l < 256 ; l++)
      {
        tr = r*l/NORMAL_LIGHT_LEVEL;
        tr = tr<0 ? 0 : (tr>255 ? 255 : tr);
        lt_light[i].red[l] = encode_rgb (tr,0,0);

        tg = g*l/NORMAL_LIGHT_LEVEL;
        tg = tg<0 ? 0 : (tg>255 ? 255 : tg);
        lt_light[i].green[l] = encode_rgb (0,tg,0);

        tb = b*l/NORMAL_LIGHT_LEVEL;
        tb = tb<0 ? 0 : (tb>255 ? 255 : tb);
        lt_light[i].blue[l] = encode_rgb (0,0,tb);

        // Because truecolor 0 (completely black) is used for transparency
        // we can't allow 0 in the normal mappings. So if all lt_light[i].xxx
        // are 0 for one value we will set them to '1'.
        if (lt_light[i].red[l] == 0 && lt_light[i].green[l] == 0 && lt_light[i].blue[l] == 0)
        {
          lt_light[i].red[l] = 1<<pfmt.RedShift;
          lt_light[i].green[l] = 1<<pfmt.GreenShift;
          lt_light[i].blue[l] = 1<<pfmt.BlueShift;
        }
      }
    }

  // Color 0 is mapped to 0 in all cases to make sure that transparent
  // colors remain transparent.
  for (l = 0 ; l < 256 ; l++)
  {
    lt_light[0].red[l] = 0;
    lt_light[0].green[l] = 0;
    lt_light[0].blue[l] = 0;
  }
}

void csTextureManagerSoftware::create_lt_truergb_private ()
{
  CHK (delete lt_truergb_private);
  CHK (lt_truergb_private = new TextureTablesTrueRgbPriv);
  lt_light = lt_truergb_private->lut;
  int i, l;
  int tr, tg, tb;  // Temp working vars

  if (verbose) SysPrintf (MSG_INITIALIZATION, "  Calculate tables for true_rgb/private mode...\n");

  for (i = 0 ; i < 256 ; i++)
    if (alloc[i])
    {
      for (l = 0 ; l < 256 ; l++)
      {
        tr = i*l/NORMAL_LIGHT_LEVEL;
        tr = tr<0 ? 0 : (tr>255 ? 255 : tr);
        lt_light[i].red[l] = encode_rgb (tr,0,0);

        tg = i*l/NORMAL_LIGHT_LEVEL;
        tg = tg<0 ? 0 : (tg>255 ? 255 : tg);
        lt_light[i].green[l] = encode_rgb (0,tg,0);

        tb = i*l/NORMAL_LIGHT_LEVEL;
        tb = tb<0 ? 0 : (tb>255 ? 255 : tb);
        lt_light[i].blue[l] = encode_rgb (0,0,tb);

        // Because truecolor 0 (completely black) is used for transparency
        // we can't allow 0 in the normal mappings. So if all lt_light[i].xxx
        // are 0 for one value we will set them to '1'.
        if (lt_light[i].red[l] == 0 && lt_light[i].green[l] == 0 && lt_light[i].blue[l] == 0)
        {
          lt_light[i].red[l] = 1<<pfmt.RedShift;
          lt_light[i].green[l] = 1<<pfmt.GreenShift;
          lt_light[i].blue[l] = 1<<pfmt.BlueShift;
        }
      }
    }

  // Color 0 is mapped to 0 in all cases to make sure that transparent
  // colors remain transparent. @@@ RECHECK FOR PRIVATE COLORMAPS
  for (l = 0 ; l < 256 ; l++)
  {
    lt_light[0].red[l] = 0;
    lt_light[0].green[l] = 0;
    lt_light[0].blue[l] = 0;
  }
}

void csTextureManagerSoftware::create_lt_alpha ()
{
  CHK (delete lt_alpha);
  CHK (lt_alpha = new TextureTablesAlpha ());

  int i, j;
  int r, g, b;

  if (verbose) SysPrintf (MSG_INITIALIZATION, "  Calculate alpha tables...\n");
  for (i = 0 ; i < 256 ; i++)
    for (j = 0 ; j < 256 ; j++)
    {
      r = (pal[i].red + pal[j].red) / 2;
      g = (pal[i].green + pal[j].green) / 2;
      b = (pal[i].blue + pal[j].blue) / 2;
      lt_alpha->alpha_map50[i][j] = find_rgb (r, g, b);

      r = (pal[i].red*250 + pal[j].red*750) / 1000;
      g = (pal[i].green*250 + pal[j].green*750) / 1000;
      b = (pal[i].blue*250 + pal[j].blue*750) / 1000;
      lt_alpha->alpha_map25[i][j] = find_rgb (r, g, b);
    }
}

void csTextureManagerSoftware::compute_light_tables ()
{
  // Light level NORMAL_LIGHT_LEVEL is normal

  if (verbose) SysPrintf (MSG_INITIALIZATION, "Computing all needed tables...\n");

  if (txtMode == TXT_PRIVATE)
    create_lt_truergb_private ();
  else if (mixing == MIX_TRUE_RGB)
    create_lt_truergb ();
  if (truecolor)
    create_lt_white16 ();
  else
    create_lt_white8 ();
  if (!truecolor)
    create_lt_alpha ();

  if (verbose)
    SysPrintf (MSG_INITIALIZATION, "DONE\n");
}

void csTextureManagerSoftware::alloc_palette ()
{
  int i;
  int r, g, b;
  for (i = 0 ; i < 256 ; i++)
    if (alloc[i])
    {
      r = GAMMA (pal[i].red); if (r < 0) r = 0; else if (r > 255) r = 255;
      g = GAMMA (pal[i].green); if (g < 0) g = 0; else if (g > 255) g = 255;
      b = GAMMA (pal[i].blue); if (b < 0) b = 0; else if (b > 255) b = 255;
      m_piG2D->SetRGB (i, r, g, b);
    }

  if (truecolor)
  {
    red_color = encode_rgb (255, 0, 0);
    blue_color = encode_rgb (0, 0, 255);
    yellow_color = encode_rgb (255, 255, 0);
    green_color = encode_rgb (0, 255, 0);
    white_color = encode_rgb (255, 255, 255);
    black_color = 0;
  }
  else
  {
    red_color = find_rgb_slow (255, 0, 0);
    blue_color = find_rgb_slow (0, 0, 255);
    yellow_color = find_rgb_slow (255, 255, 0);
    green_color = find_rgb_slow (0, 255, 0);
    white_color = 255;
    black_color = 0;
  }
}

int csTextureManagerSoftware::get_almost_black ()
{
  // Since color 0 is used for transparent, prepare an "almost-black"
  // color to use instead of opaque black texels.
  switch (pfmt.PixelBytes)
  {
    case 1:
      // find_color cares about color 0 in 8-bit mode
      return find_color (0, 0, 0);
    case 2:
    {
      // Since the green channel usually has most bits (in all of
      // 5:5:5, 5:6:5 and 6:6:4 encodings), set a "1" in the LSB
      // of that channel and rest bits to zero
      return (1 << pfmt.GreenShift);
    }
    case 4:
    {
      // Since the human eye is less sensible to blue than to
      // other colors, we set a 1 in LSB of blue channel, and
      // all other bits to zero.
      return (1 << pfmt.BlueShift);
    }
  }
  // huh ?!
  return 0;
}

STDMETHODIMP csTextureManagerSoftware::Initialize ()
{
  // By allocating black at 0 and white at 255 we are compatible with
  // the Windows palette (those two colors cannot be modified).
  // This is important even for X Windows because X Windows can be run
  // in Windows NT (or 95) under X emulation and the same limitation
  // is also present then.
  //
  // Note that we can't use these colors for textures since it is
  // possible that we apply gamma correction on the colormap and this
  // is not possible for those two entries. However, they are very
  // useful as fixed black and white entries no matter what the
  // palette is (for messages and such).
  //
  // Another useful trick is to map color 0 to 0 in every lightlevel.
  // Using this feature transparent colors in textures remain transparent
  // even with lighting added.
  memset (alloc, 0, sizeof (alloc));
  memset (pal, 0, sizeof (pal));
  alloc[0] = true;
  pal[0].red = 0;
  pal[0].green = 0;
  pal[0].blue = 0;
  alloc[255] = true;
  pal[255].red = 255;
  pal[255].green = 255;
  pal[255].blue = 255;

  clear ();
  read_config ();

  return S_OK;
}

STDMETHODIMP csTextureManagerSoftware::Prepare ()
{
  int i;

  CHK (delete factory_3d); factory_3d = NULL;
  CHK (delete factory_2d); factory_2d = NULL;
  if (csTextureManagerSoftware::txtMode == TXT_24BIT)
    { CHK (factory_3d = new csTextureFactory32 ()); }
  else
    { CHK (factory_3d = new csTextureFactory8 ()); }

  if (pfmt.PixelBytes == 1)
    { CHK (factory_2d = new csTextureFactory8 ()); }
  else if (pfmt.PixelBytes == 2)
    { CHK (factory_2d = new csTextureFactory16 ()); }
  else
    { CHK (factory_2d = new csTextureFactory32 ()); }

  for (i = 0 ; i < textures.Length () ; i++)
  {
    csTextureMMSoftware* txt = (csTextureMMSoftware*)textures[i];
    if (txt->for_3d ()) txt->alloc_mipmaps (this);
    if (txt->for_2d ()) txt->alloc_2dtexture (this);
  }

  compute_palette ();
  compute_light_tables ();

  if (do_blend_mipmap0)
  {
    if (verbose) SysPrintf (MSG_INITIALIZATION, "Blend textures...\n");
    for (i = 0 ; i < textures.Length () ; i++)
    {
      csTextureMMSoftware* txt = (csTextureMMSoftware*)textures[i];
      if (txt->for_3d ()) txt->blend_mipmap0 (this);
    }
  }

  if (verbose) SysPrintf (MSG_INITIALIZATION, "Computing mipmapped textures...\n");
  for (i = 0 ; i < textures.Length () ; i++)
  {
    csTextureMMSoftware* txt = (csTextureMMSoftware*)textures[i];
    if (txt->for_3d ()) txt->create_mipmaps (this);
    txt->free_usage_table ();
  }
  if (verbose) SysPrintf (MSG_INITIALIZATION, "DONE!\n");

  return S_OK;
}

STDMETHODIMP csTextureManagerSoftware::RegisterTexture (IImageFile* image,
  ITextureHandle** handle, bool for3d, bool for2d)
{
  csTextureMMSoftware* txt = new_texture (image);
  txt->set_3d2d (for3d, for2d);
  *handle = GetITextureHandleFromcsTextureMM (txt);
  return S_OK;
}

STDMETHODIMP csTextureManagerSoftware::UnregisterTexture (ITextureHandle* handle)
{
  (void)handle;
  //@@@ Not implemented yet.
  return S_OK;
}

STDMETHODIMP csTextureManagerSoftware::MergeTexture (ITextureHandle* handle)
{
  (void)handle;
  //@@@ Not implemented yet.
  return S_OK;
}

STDMETHODIMP csTextureManagerSoftware::FreeImages ()
{
  int i;
  for (i = 0 ; i < textures.Length () ; i++)
  {
    csTextureMMSoftware* txt = (csTextureMMSoftware*)textures[i];
    txt->free_image ();
  }
  return S_OK;
}

STDMETHODIMP csTextureManagerSoftware::ReserveColor (int r, int g, int b, bool privcolor)
{
  alloc_rgb (r, g, b, 0);
  return S_OK;
}

STDMETHODIMP csTextureManagerSoftware::AllocPalette ()
{
  alloc_palette ();
  return S_OK;
}

//---------------------------------------------------------------------------
#if defined(COMP_MWERKS) && defined(PROC_POWERPC)
#pragma global_optimizer reset
#endif
