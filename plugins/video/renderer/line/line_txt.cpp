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
#include "cs3d/line/line_txt.h"
#include "cs3d/common/inv_cmap.h"
#include "csutil/scanstr.h"
#include "csutil/inifile.h"
#include "isystem.h"
#include "iimage.h"
#include "lightdef.h"

#define RESERVED_COLOR(c) ((c == 0) || (c == 255))

#define GAMMA(c) ((int) (256.0 * pow (float (c) / 256.0, 1.0 / Gamma)))

#define CLIP_RGB \
  if (r < 0) r = 0; else if (r > 255) r = 255; \
  if (g < 0) g = 0; else if (g > 255) g = 255; \
  if (b < 0) b = 0; else if (b > 255) b = 255;

/**
 * A nice observation about the properties of human eye:
 * Let's call the largest R or G or B component of a color "main".
 * If some other color component is much smaller than the main component,
 * we can change it in a large range without noting any change in
 * the color itself. Examples:
 * (128, 128, 128) - we note a change in color if we change any component
 * by 4 or more.
 * (192, 128, 128) - we should change of G or B components by 8 to note any
 * change in color.
 * (255, 128, 128) - we should change of G or B components by 16 to note any
 * change in color.
 * (255, 0, 0) - we can change any of G or B components by 32 and we
 * won't note any change.
 * Thus, we use this observation to create a palette that contains more
 * useful colors. We implement here a function to evaluate the "distance"
 * between two colors. tR,tG,tB define the color we are looking for (target);
 * sR, sG, sB define the color we're examining (source).
 */
static inline int rgb_dist (int tR, int tG, int tB, int sR, int sG, int sB)
{
  register int max = MAX (tR, tG);
  max = MAX (max, tB);

  sR -= tR; sG -= tG; sB -= tB;

  return R_COEF_SQ * sR * sR * (32 - ((max - tR) >> 3)) +
         R_COEF_SQ * sG * sG * (32 - ((max - tG) >> 3)) +
         R_COEF_SQ * sB * sB * (32 - ((max - tB) >> 3));
}

TxtCmapPrivate::TxtCmapPrivate ()
{
  memset (alloc, sizeof (alloc), 0);
  memset (rgb_values, sizeof (rgb_values), 0);
  memset (priv_to_global, sizeof (priv_to_global), 0);
}

int TxtCmapPrivate::find_rgb (int r, int g, int b, int *d)
{
  CLIP_RGB;

  int i, min, mindist;
  mindist = 0x7fffffff;
  min = -1;
  for (i = 1; i < 256; i++)		// Color 0 is reserved for transparency
    if (alloc [i])
    {
      register int dist = rgb_dist (r, g, b, rgb_values [(i << 2)],
        rgb_values [(i << 2) + 1], rgb_values [(i << 2) + 2]);
      if (dist < mindist) { mindist = dist; min = i; if (!dist) break; }
    }
  if (d) *d = mindist;
  return min;
}

int TxtCmapPrivate::alloc_rgb (int r, int g, int b, int dist)
{
  CLIP_RGB;

  int d, i = find_rgb (r, g, b, &d);
  if (i == -1 || d > dist)
  {
    for (int j = 1; j < 256; j++) // Color 0 is not used, reserved for transparency
      if (!alloc [j])
      {
        alloc[j] = true;
        rgb_values [(j << 2) + 0] = r;
        rgb_values [(j << 2) + 1] = g;
        rgb_values [(j << 2) + 2] = b;
        return j;
      }
    return i; // We couldn't allocate a new color, return best fit
  }
  else
    return i;
}

//---------------------------------------------------------------------------

csTextureMMLine::csTextureMMLine (iImage* image) : csTextureMM (image)
{
  priv_cmap = NULL;
}

csTextureMMLine::~csTextureMMLine ()
{
  CHK (delete priv_cmap);
}

void csTextureMMLine::convert_to_internal (csTextureManager* tex,
  iImage* imfile, unsigned char* bm)
{
  csTextureManagerLine* texs = (csTextureManagerLine*)tex;
  if (texs->txtMode == TXT_GLOBAL)
    convert_to_internal_global (texs, imfile, bm);
  else if (texs->txtMode == TXT_24BIT)
    convert_to_internal_24bit (texs, imfile, bm);
  else
    convert_to_internal_private (texs, imfile, bm);
}

void csTextureMMLine::convert_to_internal_global (csTextureManagerLine* tex,
  iImage* imfile, unsigned char* bm)
{
  int s = imfile->GetSize ();
  RGBPixel *bmsrc = (RGBPixel *)imfile->GetImageData ();
  if (GetTransparent ())
    for (; s > 0; s--, bmsrc++)
      if (transp_color == *bmsrc)
        *bm++ = 0;
      else
        *bm++ = tex->find_rgb (bmsrc->red, bmsrc->green, bmsrc->blue);
  else
    for (; s > 0; s--, bmsrc++)
      *bm++ = tex->find_rgb (bmsrc->red, bmsrc->green, bmsrc->blue);
}

void csTextureMMLine::convert_to_internal_24bit (csTextureManagerLine *tex,
  iImage* imfile, unsigned char* bm)
{
  (void)tex;

  int s = imfile->GetSize ();
  RGBPixel* bmsrc = (RGBPixel *)imfile->GetImageData ();
  ULong *bml = (ULong*)bm;
  if (GetTransparent ())
    for (; s > 0; s--, bmsrc++)
      if (transp_color == *bmsrc)
        *bml++ = 0;
      else
        *bml++ = (bmsrc->red << rs24) | (bmsrc->green << gs24) | (bmsrc->blue << bs24);
  else
    for (; s > 0; s--, bmsrc++)
      *bml++ = (bmsrc->red << rs24) | (bmsrc->green << gs24) | (bmsrc->blue << bs24);
}

void csTextureMMLine::convert_to_internal_private (csTextureManagerLine* /*tex*/,
  iImage* imfile, unsigned char* bm)
{
  int s = imfile->GetSize ();
  RGBPixel* bmsrc = (RGBPixel *)imfile->GetImageData ();
  if (GetTransparent ())
    for (; s > 0; s--, bmsrc++)
      if (transp_color == *bmsrc)
        *bm++ = 0;
      else
        *bm++ = priv_cmap->find_rgb (bmsrc->red, bmsrc->green, bmsrc->blue);
  else
    for (; s > 0; s--, bmsrc++)
      *bm++ = priv_cmap->find_rgb (bmsrc->red, bmsrc->green, bmsrc->blue);
}

void csTextureMMLine::remap_texture (csTextureManager* new_palette)
{
  if (!ifile) return;

  csTextureManagerLine* psoft = (csTextureManagerLine*)new_palette;

  // If we're running at 32bpp, save R,G,B shift values
  // since we will have to use the native pixel format
  // (so that unlighted textures can be fetched from bitmap pointer)
  const csPixelFormat &pfmt = psoft->pixel_format ();
  if (pfmt.PixelBytes == 4)
  {
    rs24 = pfmt.RedShift;
    gs24 = pfmt.GreenShift;
    bs24 = pfmt.BlueShift;
  }

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

void csTextureMMLine::remap_palette_global (csTextureManagerLine* new_palette, bool do_2d)
{
  compute_color_usage ();
  if (!usage) return;

  int* trans;
  int num_col = usage->get_num_colors ();
  int i;

  CHK (trans = new int [num_col]);
  for (i = 0 ; i < num_col ; i++)
  {
    if (GetTransparent () && (transp_color == get_usage (i)))
      trans[i] = 0;
    else
      trans[i] = ((csTextureManagerLine*)new_palette)->find_rgb
        (get_usage (i).red, get_usage (i).green, get_usage (i).blue);
  }

  RGBPixel* src = (RGBPixel *)ifile->GetImageData ();

  unsigned char *dest, *last;
  if (do_2d)
    dest = t2d->get_bitmap ();
  else
    dest = t1->get_bitmap ();
  int size = ifile->GetSize ();
  last = dest + size;

  i=0;
  for (; dest < last; dest++, src++)
    *dest = trans[usage->get_palInd()[i++]];

  //for (; dest < last; dest++, src++)
  //{
    //for (i = 0 ; i < num_col ; i++)
      //if (*src == get_usage(i))
        //break;
    //*dest = trans[i];
  //}

  CHK (delete [] trans);
}

void csTextureMMLine::remap_palette_private (csTextureManagerLine* new_palette)
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
    if (GetTransparent () && (transp_color == get_usage (i)))
      trans [i] = 0;
    else
      trans [i] = priv_cmap->alloc_rgb (get_usage(i).red, get_usage(i).green,
        get_usage(i).blue, dist);
    dist += ddist;
  }

  int w = ifile->GetWidth ();
  int h = ifile->GetHeight ();
  RGBPixel* src = (RGBPixel *)ifile->GetImageData ();
  unsigned char* dest = t1->get_bitmap ();

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
      ((csTextureManagerLine*)new_palette)->find_rgb (
         priv_cmap->rgb_values[(i<<2)+0],
         priv_cmap->rgb_values[(i<<2)+1],
         priv_cmap->rgb_values[(i<<2)+2]);
}

//---------------------------------------------------------------------------

UShort csTextureManagerLine::alpha_mask;

csTextureManagerLine::csTextureManagerLine (iSystem* iSys, iGraphics2D* iG2D) :
  csTextureManager (iSys, iG2D)
{
  txtMode = TXT_GLOBAL;
  force_txtMode = -1;
  initialized   = false;
  lt_truergb         = NULL;
  lt_truergb_private = NULL;
  lt_pal             = NULL;
  lt_alpha           = NULL;
}

void csTextureManagerLine::Initialize ()
{
  csTextureManager::Initialize ();

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
  alloc [0] = true;
  pal [0].red = 0;
  pal [0].green = 0;
  pal [0].blue = 0;
  alloc [255] = true;
  pal [255].red = 255;
  pal [255].green = 255;
  pal [255].blue = 255;

  truecolor = pfmt.PalEntries == 0;
  clear ();
  read_config ();

  initialized = true;

  lt_truergb = NULL;
  lt_truergb_private = NULL;
  lt_pal = NULL;
  lt_alpha = NULL;

  if (!truecolor)
  {
    // If we don't have truecolor we simulate 6:6:4 bits
    // for R:G:B in the masks anyway because we still need the
    // 16-bit format for our light mixing
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

bool csTextureManagerLine::force_txtmode (char* p)
{
  if (!strcmp (p, "global"))
    force_txtMode = TXT_GLOBAL;
  else if (!strcmp (p, "private"))
    force_txtMode = TXT_PRIVATE;
  else if (!strcmp (p, "24bit"))
    force_txtMode = TXT_24BIT;
  else
  {
    SysPrintf (MSG_FATAL_ERROR, "Bad value '%s' for TXTMODE (use 'global', 'private', or '24bit')!\n", p);
    return false;
  }
  return true;
}

void csTextureManagerLine::read_config ()
{
  char *p;

  do_blend_mipmap0 = config->GetYesNo ("Mipmapping", "BLEND_MIPMAP", false);
  prefered_dist = config->GetInt ("TextureManager", "RGB_DIST", PREFERED_DIST);
  p = config->GetStr ("Mipmapping", "MIPMAP_MODE", "nice");
  if (!strcmp (p, "nice"))
  {
    mipmap_mode = MIPMAP_NICE;
    if (verbose) SysPrintf (MSG_INITIALIZATION, "Mipmap calculation 'nice'.\n");
  }
  else if (!strcmp (p, "verynice"))
  {
    mipmap_mode = MIPMAP_VERYNICE;
    if (verbose) SysPrintf (MSG_INITIALIZATION, "Mipmap calculation 'verynice'\n  (Note: this is expensive for the texture cache)\n");
  }
  else
  {
    SysPrintf (MSG_FATAL_ERROR, "Bad value '%s' for MIPMAP_MODE!\n(Use 'verynice', 'nice', 'ugly', or 'default')\n", p);
    exit (0);	//@@@
  }

  if (force_txtMode == -1 && pfmt.PixelBytes == 4)
  {
    if (verbose) SysPrintf (MSG_INITIALIZATION, "Texture mode forced to '24bit' because we are in 32-bit mode.\n");
    force_txtMode = TXT_24BIT;
  }

  if (force_txtMode != -1)
    txtMode = force_txtMode;
  else
  {
    p = config->GetStr ("TextureManager", "TXTMODE", "global");
    if (!strcmp (p, "global"))
      txtMode = TXT_GLOBAL;
    else if (!strcmp (p, "private"))
      txtMode = TXT_PRIVATE;
    else if (!strcmp (p, "24bit"))
      txtMode = TXT_24BIT;
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

  if (force_txtMode != -1) txtMode = force_txtMode;

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
}

csTextureManagerLine::~csTextureManagerLine ()
{
  clear ();
}

void csTextureManagerLine::clear ()
{
  csTextureManager::clear ();
  CHK (delete lt_truergb); lt_truergb = NULL;
  CHK (delete lt_truergb_private); lt_truergb_private = NULL;
  CHK (delete lt_pal); lt_pal = NULL;
  CHK (delete lt_alpha); lt_alpha = NULL;
}

csTextureMMLine* csTextureManagerLine::new_texture (iImage* image)
{
  CHK (csTextureMMLine* tm = new csTextureMMLine (image));
  if (tm->loaded_correctly ())
    textures.Push (tm);
  else
  {
    delete tm;
    tm = NULL;
  }
  return tm;
}

csTexture* csTextureManagerLine::get_texture (int idx, int lev)
{
  return ((csTextureMMLine*)textures[idx])->get_texture (lev);
}

ULong csTextureManagerLine::encode_rgb (int r, int g, int b)
{
  return
    ((r >> (8-pfmt.RedBits))   << pfmt.RedShift) |
    ((g >> (8-pfmt.GreenBits)) << pfmt.GreenShift) |
    ((b >> (8-pfmt.BlueBits))  << pfmt.BlueShift);
}

int csTextureManagerLine::find_rgb (int r, int g, int b)
{
  CLIP_RGB;
  return lt_palette_table[encode_rgb (r, g, b)];
}

int csTextureManagerLine::find_color (int r, int g, int b)
{
  CLIP_RGB;
  if (truecolor)
    return encode_rgb (r, g, b);
  else
    return lt_palette_table[encode_rgb (r, g, b)];
}

ULong csTextureManagerLine::encode_rgb_safe (int r, int g, int b)
{
  CLIP_RGB;
  return encode_rgb (r, g, b);
}

int csTextureManagerLine::find_rgb_slow (int r, int g, int b, int *d)
{
  CLIP_RGB;

  int i, min, mindist;
  mindist = 0x7fffffff;
  min = -1;
  for(i = 1 ; i < 255 ; i++) // Exclude colors 0 and 255
    if (alloc[i])
    {
      register int dist = rgb_dist (r, g, b, pal[i].red,
        pal[i].green, pal[i].blue);
      if (dist < mindist) { mindist = dist; min = i; if (!dist) break; }
    }
  if (d) *d = mindist;
  return min;
}

int csTextureManagerLine::alloc_rgb (int r, int g, int b, int dist)
{
  CLIP_RGB;

  int d, i = find_rgb_slow (r, g, b, &d);

  if (i == -1 || d > dist)
  {
    for (int j = 1 ; j < 255 ; j++) // Exclude color 0 and 255
      if (!alloc[j])
      {
        alloc [j] = true;
        pal [j].red = r;
        pal [j].green = g;
        pal [j].blue = b;
        return j;
      }
    return i; // We couldn't allocate a new color, return best fit
  }
  else
    return i;
}

void csTextureManagerLine::create_lt_palette ()
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

  for (i = 0 ; i < 256 ; i++)
    lt_pal->pal_to_true[i] = encode_rgb (pal[i].red, pal[i].green, pal[i].blue);
}

void csTextureManagerLine::compute_palette ()
{
  int i, t;

  if (verbose) SysPrintf (MSG_INITIALIZATION, "Computing palette...\n");

  // First compute the usage of all colors of all textures.
  // These usage lists will be sorted so that the most frequently
  // used color of each textures is put first.
  for (i = 0 ; i < textures.Length () ; i++)
    ((csTextureMMLine*)textures[i])->compute_color_usage ();

  // Then allocate colors for all textures at the same time.
  {
    // Allocate first 6*6*4 colors in a uniformly-distributed fashion
    // since we'll get lighted/dimmed/colored textures more often
    // than original pixel values; thus be prepared for this.
//@@todo: this if should move a bit higher... after I modify the 16bit mode - A.Z.
  if (!truecolor)
    for (int __r = 0; __r < 6; __r++)
      for (int __g = 0; __g < 6; __g++)
        for (int __b = 0; __b < 4; __b++)
          alloc_rgb (20 + __r * 42, 20 + __g * 42, 30 + __b * 50, prefered_dist);

    // Compute number of free colors in palette
    int i, colors = 0;
    for (i = 0; i < 256; i++)
      if (!alloc [i])
        colors++;
    for (i = 0; i < colors; i++)
    {
      for (t = 0 ; t < textures.Length () ; t++)
      {
        csTextureMMLine* txt = (csTextureMMLine*)textures[t];
        if (i < txt->get_num_colors ())
          alloc_rgb (txt->get_usage (i).red,
                     txt->get_usage (i).green,
                     txt->get_usage (i).blue,
                     prefered_dist);
      }
    }
  }

  create_lt_palette ();

  // Remap all textures according to the new colormap.
  for (i = 0 ; i < textures.Length () ; i++)
    ((csTextureMMLine*)textures[i])->remap_texture (this);

  if (verbose) SysPrintf (MSG_INITIALIZATION, "DONE\n");
}

void csTextureManagerLine::create_lt_truergb ()
{
  CHK (delete lt_truergb);
  CHK (lt_truergb = new TextureTablesTrueRgb);
  lt_light = lt_truergb->lut;
  int i, l;
  int r, g, b;
  int tr, tg, tb;  // Temp working vars

  if (verbose) SysPrintf (MSG_INITIALIZATION, "  Calculate R/G/B tables...\n");

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

void csTextureManagerLine::create_lt_truergb_private ()
{
  CHK (delete lt_truergb_private);
  CHK (lt_truergb_private = new TextureTablesTrueRgbPriv);
  lt_light = lt_truergb_private->lut;
  int i, l;
  int tr, tg, tb;  // Temp working vars

  if (verbose) SysPrintf (MSG_INITIALIZATION, "  Calculate R/G/B tables private palette mode...\n");

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

void csTextureManagerLine::create_lt_alpha ()
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

int csTextureManagerLine::find_rgb_map (int r, int g, int b, int map_type, int l)
{
  int nr = r, ng = g, nb = b;

  switch (map_type)
  {
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
      nr = l*r / NORMAL_LIGHT_LEVEL;
      break;
    case TABLE_GREEN:
      ng = l*g / NORMAL_LIGHT_LEVEL;
      break;
    case TABLE_BLUE:
      nb = l*b / NORMAL_LIGHT_LEVEL;
      break;
  }
  return truecolor ? encode_rgb_safe (nr, ng, nb) : find_rgb (nr, ng, nb);
  //return truecolor ? encode_rgb_safe (nr, ng, nb) :
    //(accurate_rgb_mapper ? find_rgb_slow (nr, ng, nb) : find_rgb (nr, ng, nb));
}

void csTextureManagerLine::compute_light_tables ()
{
  // Light level NORMAL_LIGHT_LEVEL is normal

  if (verbose) SysPrintf (MSG_INITIALIZATION, "Computing all needed tables...\n");

  if (txtMode == TXT_PRIVATE)
    create_lt_truergb_private ();
  else
    create_lt_truergb ();
  if (!truecolor)
    create_lt_alpha ();

  if (verbose)
    SysPrintf (MSG_INITIALIZATION, "DONE\n");
}

void csTextureManagerLine::Prepare ()
{
  int i;

  // Check: we have to call Initialize () before Prepare()
  if (!initialized)
  {
    SysPrintf (MSG_FATAL_ERROR, "TextureManager::Initialize () should be called before TextureManager::Prepare()");
    abort ();
  }
  initialized = false;

  CHK (delete factory_3d); factory_3d = NULL;
  CHK (delete factory_2d); factory_2d = NULL;
  if (csTextureManagerLine::txtMode == TXT_24BIT)
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
    csTextureMMLine* txt = (csTextureMMLine*)textures[i];
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
      csTextureMMLine* txt = (csTextureMMLine*)textures[i];
      if (txt->for_3d ()) txt->blend_mipmap0 (this);
    }
  }

  if (verbose) SysPrintf (MSG_INITIALIZATION, "Computing mipmapped textures...\n");
  for (i = 0 ; i < textures.Length () ; i++)
  {
    csTextureMMLine* txt = (csTextureMMLine*)textures[i];
    if (txt->for_3d ()) txt->create_mipmaps (this);
    txt->free_usage_table ();
  }
  if (verbose) SysPrintf (MSG_INITIALIZATION, "DONE\n");
}

iTextureHandle *csTextureManagerLine::RegisterTexture (iImage* image,
  bool for3d, bool for2d)
{
  csTextureMMLine* txt = new_texture (image);
  txt->set_3d2d (for3d, for2d);
  return txt;
}

void csTextureManagerLine::UnregisterTexture (iTextureHandle* handle)
{
  csTextureMMLine *tex_mm = (csTextureMMLine *)handle->GetPrivateObject ();
  int idx = textures.Find (tex_mm);
  if (idx >= 0)
    textures.Delete (idx);
}

void csTextureManagerLine::MergeTexture (iTextureHandle* handle)
{
  csTextureMMLine* txt = (csTextureMMLine*)handle->GetPrivateObject ();
  if (txt->for_3d ()) txt->alloc_mipmaps (this);
  if (txt->for_2d ()) txt->alloc_2dtexture (this);
  txt->compute_color_usage ();

  // Allocate colors for all textures at the same time.
//@@todo: uncomment this after I fix 16 and 32 bpp modes
//  if (!truecolor)
    for (int i = 0 ; i < 256 ; i++)
    {
      if (i < txt->get_num_colors ())
        alloc_rgb (txt->get_usage (i).red,
                   txt->get_usage (i).green,
                   txt->get_usage (i).blue,
                   prefered_dist);
    }

  // Remap all textures according to the new colormap.
  txt->remap_texture (this);

  // create Mipmaps
  if (txt->for_3d ()) txt->create_mipmaps (this);
  txt->free_usage_table ();
}

void csTextureManagerLine::FreeImages ()
{
  int i;
  for (i = 0 ; i < textures.Length () ; i++)
  {
    csTextureMMLine* txt = (csTextureMMLine*)textures[i];
    txt->free_image ();
  }
}

void csTextureManagerLine::ReserveColor (int r, int g, int b)
{
  alloc_rgb (r, g, b, 0);
}

void csTextureManagerLine::AllocPalette ()
{
  int i;
  int r, g, b;
  for (i = 0 ; i < 256 ; i++)
  {
    r = GAMMA (pal[i].red);
    g = GAMMA (pal[i].green);
    b = GAMMA (pal[i].blue);
    CLIP_RGB;
    G2D->SetRGB (i, r, g, b);
  }

  if (truecolor)
  {
    red_color = encode_rgb (255, 0, 0);
    blue_color = encode_rgb (0, 0, 255);
    yellow_color = encode_rgb (255, 255, 0);
    green_color = encode_rgb (0, 255, 0);
    white_color = encode_rgb (255, 255, 255);
  }
  else
  {
    red_color = find_rgb_slow (255, 0, 0);
    blue_color = find_rgb_slow (0, 0, 255);
    yellow_color = find_rgb_slow (255, 255, 0);
    green_color = find_rgb_slow (0, 255, 0);
    white_color = 255;
  }
  black_color = 0;
}
