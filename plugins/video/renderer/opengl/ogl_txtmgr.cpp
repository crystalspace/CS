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
#include "cs3d/opengl/ogl_txtmgr.h"
#include "csutil/scanstr.h"
#include "csutil/inifile.h"
#include "isystem.h"
#include "iimage.h"
#include "lightdef.h"

#define RESERVED_COLOR(c) ((c == 0) || (c == 255))

//---------------------------------------------------------------------------

csTextureManagerOpenGL::csTextureManagerOpenGL (iSystem* iSys,
  iGraphics2D* iG2D) : csTextureManager (iSys, iG2D)
{
}

void csTextureManagerOpenGL::Initialize ()
{
  csTextureManager::Initialize ();

  num_red = (pfmt.RedMask >> pfmt.RedShift) + 1;
  num_green = (pfmt.GreenMask >> pfmt.GreenShift) + 1;
  num_blue = (pfmt.BlueMask >> pfmt.BlueShift) + 1;

  clear ();
  read_config ();
}

void csTextureManagerOpenGL::read_config ()
{
  char *p;
  do_blend_mipmap0 = config->GetYesNo ("Mipmapping", "BLEND_MIPMAP", false);
  prefered_dist = config->GetInt ("TextureManager", "RGB_DIST", PREFERED_DIST);
  prefered_col_dist = config->GetInt ("TextureManager", "RGB_COL_DIST", PREFERED_COL_DIST);
  p = config->GetStr ("Mipmapping", "MIPMAP_MODE", "nice");
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

csTextureManagerOpenGL::~csTextureManagerOpenGL ()
{
  clear ();
}

void csTextureManagerOpenGL::clear ()
{
  csTextureManager::clear ();
}

csTextureMMOpenGL* csTextureManagerOpenGL::new_texture (iImage* image)
{
  CHK (csTextureMMOpenGL* tm = new csTextureMMOpenGL (image));
  if (tm->loaded_correctly ())
    textures.Push (tm);
  else
  {
    delete tm;
    tm = NULL;
  }
  return tm;
}

csTexture* csTextureManagerOpenGL::get_texture (int idx, int lev)
{
  return ((csTextureMMOpenGL*)textures[idx])->get_texture (lev);
}

ULong csTextureManagerOpenGL::encode_rgb (int r, int g, int b)
{
  return
    ((r >> (8-pfmt.RedBits))   << pfmt.RedShift) |
    ((g >> (8-pfmt.GreenBits)) << pfmt.GreenShift) |
    ((b >> (8-pfmt.BlueBits))  << pfmt.BlueShift);
}

int csTextureManagerOpenGL::find_color (int r, int g, int b)
{
  if (r>255) r=255; else if (r<0) r=0;
  if (g>255) g=255; else if (g<0) g=0;
  if (b>255) b=255; else if (b<0) b=0;
  return encode_rgb (r, g, b);
}

void csTextureManagerOpenGL::remap_textures ()
{
  int i;

  // Remap all textures according to the new colormap.
  for (i = 0 ; i < textures.Length () ; i++)
    ((csTextureMMOpenGL*)textures[i])->remap_texture (this);

  black_color = 0;
  white_color = encode_rgb (255, 255, 255);
  red_color = encode_rgb (255, 0, 0);
  blue_color = encode_rgb (0, 0, 255);
  yellow_color = encode_rgb (255, 255, 0);
  green_color = encode_rgb (0, 255, 0);
}

int csTextureManagerOpenGL::find_rgb_map (int r, int g, int b, int map_type, int l)
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

void csTextureManagerOpenGL::Prepare ()
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
    csTextureMMOpenGL* txt = (csTextureMMOpenGL*)textures[i];
//  if (txt->for_3d ()) // Mipmaps required by HighColorCache::Add() even for 2D.
      txt->alloc_mipmaps (this);
    if (txt->for_2d ())
      txt->alloc_2dtexture (this);
  }

  remap_textures ();

  if (do_blend_mipmap0)
  {
    if (verbose) SysPrintf (MSG_INITIALIZATION, "Blend textures...\n");
    for (i = 0 ; i < textures.Length () ; i++)
    {
      csTextureMMOpenGL* txt = (csTextureMMOpenGL*)textures[i];
      if (txt->for_3d ()) txt->blend_mipmap0 (this);
    }
  }

  if (verbose) SysPrintf (MSG_INITIALIZATION, "Computing mipmapped textures...\n");
  for (i = 0 ; i < textures.Length () ; i++)
  {
    csTextureMMOpenGL* txt = (csTextureMMOpenGL*)textures[i];
    if (txt->for_3d ()) txt->create_mipmaps (this);
    txt->free_usage_table ();
  }
  if (verbose) SysPrintf (MSG_INITIALIZATION, "DONE!\n");
}

iTextureHandle *csTextureManagerOpenGL::RegisterTexture (iImage* image,
  bool for3d, bool for2d)
{
  csTextureMMOpenGL* txt = new_texture (image);
  txt->set_3d2d (for3d, for2d);
  return txt;
}

void csTextureManagerOpenGL::UnregisterTexture (iTextureHandle* handle)
{
  (void)handle;
  //@@@ Not implemented yet.
}

void csTextureManagerOpenGL::MergeTexture (iTextureHandle* handle)
{
  (void)handle;
  //@@@ Not implemented yet.
}

void csTextureManagerOpenGL::FreeImages ()
{
  for (int i = 0 ; i < textures.Length () ; i++)
  {
    csTextureMMOpenGL* txt = (csTextureMMOpenGL*)textures[i];
    txt->free_image ();
  }
}

void csTextureManagerOpenGL::ReserveColor (int /*r*/, int /*g*/, int /*b*/)
{
}

void csTextureManagerOpenGL::AllocPalette ()
{
}
