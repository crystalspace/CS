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

#include <stdarg.h>
#include <math.h>

#include "sysdef.h"
#include "isystem.h"

#include "cs3d/glide2/gl_txtmgr.h"
#include "iimage.h"
#include "csutil/scanstr.h"
#include "lightdef.h"

#define RESERVED_COLOR(c) ((c == 0) || (c == 255))

csTextureMMGlide::csTextureMMGlide (iImage* image) : 
  csHardwareAcceleratedTextureMM (image)
{
}

csTextureMMGlide::~csTextureMMGlide ()
{
}

void csTextureMMGlide::convert_to_internal (csTextureManager* /*texman*/,
                                            iImage* imfile, unsigned char* bm)
{
  int x,y;

  int w = imfile->GetWidth ();
  int h = imfile->GetHeight ();

  RGBPixel* bmsrc = (RGBPixel *)imfile->GetImageData ();
  int r, g, b;
  
  UShort* bml = (UShort*)bm;
  for (y = 0 ; y < h ; y++)
  {
    for (x = 0 ; x < w ; x++)
    {
      r = bmsrc->red;
      g = bmsrc->green;
      b = bmsrc->blue;
      //*bml++ = (r<<16)|(g<<8)|b;
      *bml++ = ((r >> 3) << 11) |
               ((g >> 2) << 5) |
               ((b >> 3));
      bmsrc++;
    }
  }
}

void csTextureMMGlide::remap_palette_24bit (csTextureManager*)
{
  compute_color_usage ();
  if (!usage) return;

  int w = ifile->GetWidth ();
  int h = ifile->GetHeight ();
  RGBPixel* src = (RGBPixel *)ifile->GetImageData ();
  UShort* dest = (UShort *)t1->get_bitmap ();

  // Map the texture to the RGB palette.
  int x, y;
  for (y = 0 ; y < h ; y++)
    for (x = 0 ; x < w ; x++)
    {
      //*dest++ = (src->red<<16) | (src->green<<8) | src->blue;
      *dest++ = ((src->red >> 3 ) << 11) |
                ((src->green >> 2 ) << 5) |
                ((src->blue >> 3));
      src++;
    }
}

//---------------------------------------------------------------------------

csTextureManagerGlide::csTextureManagerGlide (iSystem* iSys, iGraphics2D* iG2D) :
	csTextureManager (iSys, iG2D)
{
}

void csTextureManagerGlide::Initialize ()
{
  csTextureManager::Initialize ();

  num_red = (pfmt.RedMask >> pfmt.RedShift) + 1;
  num_green = (pfmt.GreenMask >> pfmt.GreenShift) + 1;
  num_blue = (pfmt.BlueMask >> pfmt.BlueShift) + 1;

  clear ();
  read_config ();
}

void csTextureManagerGlide::read_config ()
{
  char* p;
//  iSystem* sys = m_piSystem;
  // @@@ WARNING! The following code only examines the
  // main cryst.cfg file and not the one which overrides values
  // in the world file. We need to support this someway in the iSystem
  // interface as well.

  do_blend_mipmap0 = System->ConfigGetYesNo ("TextureMapper", "BLEND_MIPMAP", false);
  prefered_dist = System->ConfigGetInt ("World", "RGB_DIST", PREFERED_DIST);
  prefered_col_dist = System->ConfigGetInt ("World", "RGB_COL_DIST", PREFERED_COL_DIST);
  p = System->ConfigGetStr ("TextureMapper", "MIPMAP_MODE", "nice");
  if (!strcmp (p, "nice"))
  {
    mipmap_mode = MIPMAP_NICE;
    if (verbose) SysPrintf (MSG_INITIALIZATION, "Mipmap calculation 'nice'.\n");
  }
  else if (!strcmp (p, "verynice"))
  {
    if (false/*caps.SupportsArbitraryMipMapping*/)
    {
      mipmap_mode = MIPMAP_VERYNICE;
      if (verbose) SysPrintf (MSG_INITIALIZATION, "Mipmap calculation 'verynice'\n  (Note: this is expensive for the texture cache)\n");
    }
    else
    {
      mipmap_mode = MIPMAP_NICE;
      if (verbose) SysPrintf (MSG_INITIALIZATION, "Mipmap calculation 'nice' ('verynice' not available for hardware accelerators).\n");
    }
  }
  else
  {
    SysPrintf (MSG_FATAL_ERROR, "Bad value '%s' for MIPMAP_MODE!\n(Use 'verynice', 'nice', 'ugly', or 'default')\n", p);
    exit (0);	//@@@
  }

  if (pfmt.PixelBytes == 4)
    { if (verbose) SysPrintf (MSG_INITIALIZATION, "Truecolor mode (32 bit).\n"); }
  else
    { if (verbose) SysPrintf (MSG_INITIALIZATION, "Truecolor mode (15/16 bit).\n"); }

  if (verbose)
    SysPrintf (MSG_INITIALIZATION, "Code all textures in 24-bit.\n");
}

csTextureManagerGlide::~csTextureManagerGlide ()
{
  clear ();
}

void csTextureManagerGlide::clear ()
{
  csTextureManager::clear ();
}

csTextureMMGlide* csTextureManagerGlide::new_texture (iImage* image)
{
  CHK (csTextureMMGlide* tm = new csTextureMMGlide (image));
  if (tm->loaded_correctly ())
    textures.Push (tm);
  else
  {
    delete tm;
    tm = NULL;
  }
  return tm;
}

csTexture* csTextureManagerGlide::get_texture (int idx, int lev)
{
  return ((csTextureMMGlide*)textures[idx])->get_texture (lev);
}

ULong csTextureManagerGlide::encode_rgb (int r, int g, int b)
{
  return
    ((r >> (8-pfmt.RedBits))   << pfmt.RedShift) |
    ((g >> (8-pfmt.GreenBits)) << pfmt.GreenShift) |
    ((b >> (8-pfmt.BlueBits))  << pfmt.BlueShift);
}

int csTextureManagerGlide::find_color (int r, int g, int b)
{
  if (r>255) r=255; else if (r<0) r=0;
  if (g>255) g=255; else if (g<0) g=0;
  if (b>255) b=255; else if (b<0) b=0;
  return encode_rgb (r, g, b);
}

void csTextureManagerGlide::remap_textures ()
{
  int i;

  // Remap all textures according to the new colormap.
  for (i = 0 ; i < textures.Length () ; i++)
    ((csTextureMMGlide*)textures[i])->remap_texture (this);

  black_color = 0;
  white_color = encode_rgb (255, 255, 255);
  red_color = encode_rgb (255, 0, 0);
  blue_color = encode_rgb (0, 0, 255);
  yellow_color = encode_rgb (255, 255, 0);
  green_color = encode_rgb (0, 255, 0);
}

int csTextureManagerGlide::find_rgb_map (int r, int g, int b, int map_type, int l)
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
  return find_color (nr, ng, nb);
}

void csTextureManagerGlide::Prepare ()
{
  int i;

  CHK (delete factory_3d); factory_3d = NULL;
  CHK (delete factory_2d); factory_2d = NULL;
  CHK (factory_3d = new csTextureFactory32 ());
  if (pfmt.PixelBytes == 1)
    { CHK (factory_2d = new csTextureFactory8 ()); }
  else if (pfmt.PixelBytes == 2)
    { CHK (factory_2d = new csTextureFactory16 ()); }
  else
    { CHK (factory_2d = new csTextureFactory32 ()); }

  for (i = 0 ; i < textures.Length () ; i++)
  {
    csTextureMMGlide* txt = (csTextureMMGlide*)textures[i];
    /*if (txt->for_3d ())*/ txt->alloc_mipmaps (this);
    if (txt->for_2d ()) txt->alloc_2dtexture (this);
  }

  remap_textures ();

  if (do_blend_mipmap0)
  {
    if (verbose) SysPrintf (MSG_INITIALIZATION, "Blend textures...\n");
    for (i = 0 ; i < textures.Length () ; i++)
    {
      csTextureMMGlide* txt = (csTextureMMGlide*)textures[i];
      if (txt->for_3d ()) txt->blend_mipmap0 (this);
    }
  }

  if (verbose) SysPrintf (MSG_INITIALIZATION, "Computing mipmapped textures...\n");
  for (i = 0 ; i < textures.Length () ; i++)
  {
    csTextureMMGlide* txt = (csTextureMMGlide*)textures[i];
    if (txt->for_3d ()) txt->create_mipmaps (this);
    txt->free_usage_table ();
  }
  if (verbose) SysPrintf (MSG_INITIALIZATION, "DONE!\n");
}

iTextureHandle *csTextureManagerGlide::RegisterTexture (iImage* image,
  bool for3d, bool for2d)
{
  csTextureMMGlide* txt = new_texture (image);
  txt->set_3d2d (for3d, for2d);
  return txt;
}

void csTextureManagerGlide::UnregisterTexture (iTextureHandle* handle)
{
  (void)handle;
  //@@@ Not implemented yet.
}

void csTextureManagerGlide::MergeTexture (iTextureHandle* handle)
{
  (void)handle;
  //@@@ Not implemented yet.
}

void csTextureManagerGlide::FreeImages ()
{
  int i;
  for (i = 0 ; i < textures.Length () ; i++)
  {
    csTextureMMGlide* txt = (csTextureMMGlide*)textures[i];
    txt->free_image ();
  }
}

void csTextureManagerGlide::ReserveColor (int /*r*/, int /*g*/, int /*b*/)
{
}

void csTextureManagerGlide::AllocPalette ()
{
}
