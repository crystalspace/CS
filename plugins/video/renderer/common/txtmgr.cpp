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
#include "cs3d/common/txtmgr.h"
#include "cs3d/common/inv_cmap.h"
#include "csgfxldr/boxfilt.h"
#include "csutil/util.h"
#include "iimage.h"
#include "isystem.h"
#include "lightdef.h"

IMPLEMENT_IBASE (csTextureMM)
  IMPLEMENTS_INTERFACE (iTextureHandle)
IMPLEMENT_IBASE_END

csTextureMM::csTextureMM (iImageFile* image)
{
  CONSTRUCT_IBASE (NULL);

  t1 = t2 = t3 = t4 = t2d = NULL;
  rs24 = 16; gs24 = 8; bs24 = 0;

  transp_color.red = transp_color.green = transp_color.blue = 0;

  usage = NULL;
  istransp = false;

  ifile = image;
  ifile->IncRef ();
}

csTextureMM::~csTextureMM ()
{
  if (t1)
    CHKB (delete t1);
  if (t2)
    CHKB (delete t2);
  if (t3)
    CHKB (delete t3);
  if (t4)
  CHKB (delete t4);
  if (t2d)
    CHKB (delete t2d);
  free_image ();
  free_usage_table ();
}

void csTextureMM::create_blended_mipmap (csTextureManager* tex, unsigned char* bm)
{
  if (!ifile) return;
  iImageFile* if2;
  bool tran = GetTransparent ();
  if (tran)
  {
    // Just copy.
    convert_to_internal (tex, ifile, bm);
  }
  else
  {
    if2 = ifile->Blend (&csTextureManager::blend_filter);
    convert_to_internal (tex, if2, bm);
    if2->DecRef ();
  }
}

void csTextureMM::alloc_mipmaps (csTextureManager* tex)
{
  if (!ifile) return;

  AdjustSize();

  CHK (delete t1); t1 = NULL;
  CHK (delete t2); t2 = NULL;
  CHK (delete t3); t3 = NULL;
  CHK (delete t4); t4 = NULL;

  int w = ifile->GetWidth ();
  int h = ifile->GetHeight ();

  csTextureFactory* tfact = tex->factory_3d;

  t1 = tfact->new_texture (this, w, h);

  // Mipmap level 1.
  if (tex->mipmap_nice != MIPMAP_VERYNICE) { w /= 2; h /= 2; }
  t2 = tfact->new_texture (this, w, h);

  // Mipmap level 2.
  w /= 2; h /= 2;
  t3 = tfact->new_texture (this, w, h);

  // Mipmap level 3.
  w /= 2; h /= 2;
  t4 = tfact->new_texture (this, w, h);
}

void csTextureMM::blend_mipmap0 (csTextureManager* tex)
{
  if (!ifile) return;

  csTexture* tt;
  int w = ifile->GetWidth ();
  int h = ifile->GetHeight ();

  tt = tex->factory_3d->new_texture (this, w, h);
  create_blended_mipmap (tex, tt->get_bitmap ());
  t1->copy (tt);

  CHK (delete tt);
}

void csTextureMM::create_mipmap_bitmap (csTextureManager* tex, int steps, unsigned char* bm)
{
  if (!ifile) return;
  iImageFile* if2;
  bool tran = GetTransparent ();
  if (tran || tex->mipmap_nice == MIPMAP_UGLY)
    if2 = ifile->MipMap (steps); 
  else if (tex->mipmap_nice == MIPMAP_DEFAULT)
    if2 = ifile->MipMap (steps, NULL, NULL);
  else
    if2 = ifile->MipMap (steps, &csTextureManager::mipmap_filter_1, &csTextureManager::mipmap_filter_2);
  convert_to_internal (tex, if2, bm);
  if2->DecRef ();
}

void csTextureMM::create_mipmaps (csTextureManager* tex)
{
  if (!ifile) return;

  int w = ifile->GetWidth ();
  int h = ifile->GetHeight ();

  if (tex->mipmap_nice == MIPMAP_VERYNICE)
  {
    // Mipmap level 1 (the same size of texture as level 0 but blended a little)
    create_blended_mipmap (tex, t2->get_bitmap ());

    // Mipmap level 2 (mipmap starting from the blended version).
    create_mipmap_bitmap (tex, 1, t3->get_bitmap ());

    // Mipmap level 3 (mipmap starting from the blended version).
    create_mipmap_bitmap (tex, 2, t4->get_bitmap ());

    // Size of last mipmap (for computation of mean value).
    w /= 4; h /= 4;
  }
  else
  {
    // Mipmap level 1 (mipmap starting from original version).
    create_mipmap_bitmap (tex, 1, t2->get_bitmap ());

    // Mipmap level 2 (mipmap starting from original version).
    create_mipmap_bitmap (tex, 2, t3->get_bitmap ());

    // Mipmap level 3 (mipmap starting from level 1).
    create_mipmap_bitmap (tex, 3, t4->get_bitmap ());

    // Size of last mipmap (for computation of mean value).
    w /= 8; h /= 8;
  }

  // Compute one mean value for this texture. This is then used when texture
  // mapping is disabled.
  int x, y;
  ULong r, g, b;
  RGBPixel* d = ifile->GetImageData ();
  w = ifile->GetWidth ();
  h = ifile->GetHeight ();
  r = g = b = 0;
  for (x = 0 ; x < w ; x++)
    for (y = 0 ; y < h ; y++)
    {
      r += d->red;
      g += d->green;
      b += d->blue;
      d++;
    }
  r /= (w*h);
  g /= (w*h);
  b /= (w*h);

  mean_color.red = (float)r / 255.;
  mean_color.green = (float)g / 255.;
  mean_color.blue = (float)b / 255.;
  mean_idx = tex->find_color (r, g, b);
}

void csTextureMM::alloc_2dtexture (csTextureManager* tex)
{
  if (!ifile) return;

  CHK (delete t2d);

  int w = ifile->GetWidth ();
  int h = ifile->GetHeight ();
  t2d = tex->factory_2d->new_texture (this, w, h);
}

void csTextureMM::free_image ()
{
  if (ifile) { ifile->DecRef (); ifile = NULL; }
}

void csTextureMM::free_usage_table ()
{
  CHK (delete usage); usage = NULL;
}

// This function must be called BEFORE color remapping.
void csTextureMM::SetTransparent (int red, int green, int blue)
{
  istransp = (red >= 0) && (green >= 0) && (blue >= 0);
  if (istransp)
  {
    transp_color.red = red;
    transp_color.green = green;
    transp_color.blue = blue;
  }
}

csTexture* csTextureMM::get_texture (int lev)
{
  switch (lev)
  {
    case -1: return t2d;
    case 0: return t1;
    case 1: return t2;
    case 2: return t3;
    case 3: return t4;
  }
  return NULL;
}

void csTextureMM::compute_color_usage ()
{
  if (!ifile) return;
  if (usage) return;
  int s = ifile->GetSize ();
  RGBPixel* bm = ifile->GetImageData ();
  CHK (usage = new ImageColorInfo (bm, s));
}

void csTextureMM::remap_texture (csTextureManager* new_palette)
{
  if (!ifile) return;

  if (for_2d ())
    if (new_palette->get_display_depth () == 16) 
      remap_texture_16 (new_palette);
    else 
      remap_texture_32 (new_palette);

  if (for_3d ())
    remap_palette_24bit (new_palette);
}

void csTextureMM::remap_palette_24bit (csTextureManager* new_palette)
{
  compute_color_usage ();
  if (!usage) return;

  int s = ifile->GetSize ();
  RGBPixel* src = ifile->GetImageData ();
  ULong* dest = (ULong *)t1->get_bitmap ();

  // Map the texture to the RGB palette.

  // Approximation for black, because we can't use real black. Value 0 is
  // reserved for transparent pixels!
  ULong black = new_palette->get_almost_black ();

  if (GetTransparent ())
    // transparent Textures need special processing. Every texel, that is transp_color
    // will be transformed to 0, because that is the color we use internally to mark
    // transparent texels.
    for (; s; src++, s--)
      if (transp_color == *src)
        *dest++ = 0;
      else
      {
        ULong texel = (src->red << rs24) | (src->green << gs24) | (src->blue << bs24);
        *dest++ = texel ? texel : black; // transform 0 to become "black"
      }
  else
    for (; s; src++, s--)
      *dest++ = (src->red << rs24) | (src->green << gs24) | (src->blue << bs24);
}

void csTextureMM::remap_texture_16 (csTextureManager* new_palette)
{
  int s = ifile->GetSize ();
  RGBPixel* src = ifile->GetImageData ();
  UShort* dest = (UShort*)t2d->get_bitmap ();

  // Approximation for black, because we can't use real black. Value 0 is 
  // reserved for transparent pixels!
  UShort black = new_palette->get_almost_black();

  if (GetTransparent ())
    // transparent Textures need special processing. Every texel, that is transp_color
    // will be transformed to 0, because that is the color we use internally to mark
    // transparent texels.
    for (; s; src++, s--)
      if (transp_color == *src)
        *dest++ = 0;
      else
      {
        UShort texel = new_palette->find_color (src->red, src->green, src->blue);
        *dest++ = texel ? texel : black;	// transform 0 to become "black"
      }
  else
    for (; s; src++, s--)
      *dest++ = new_palette->find_color (src->red, src->green, src->blue);
}

void csTextureMM::remap_texture_32 (csTextureManager* new_palette)
{
  int s = ifile->GetSize ();
  RGBPixel* src = ifile->GetImageData ();
  ULong* dest = (ULong *)t2d->get_bitmap ();
  
  // Approximation for black, because we can't use real black. Value 0 is 
  // reserved for transparent pixels!
  ULong black = new_palette->get_almost_black(); 

  if (GetTransparent ())
    // transparent Textures need special processing. Every texel, that is transp_color
    // will be transformed to 0, because that is the color we use internally to mark
    // transparent texels.
    for (; s; src++, s--)
      if (transp_color == *src)
        *dest++ = 0;
      else
      {
        ULong texel = new_palette->find_color (src->red, src->green, src->blue);
        *dest++ = texel ? texel : black; //transform 0 to become "black"
      }
  else
    for (; s; src++, s--)
      *dest++ = new_palette->find_color (src->red, src->green, src->blue);
}

bool csTextureMM::GetTransparent ()
{
  return istransp;
}

int csTextureMM::GetNumberOfColors ()
{
  return usage->get_num_colors();
}
 
void csTextureMM::GetMipMapDimensions (int mipmap, int& w, int& h) 
{ 
  csTexture* txt = get_texture (mipmap);
  w = txt->get_width ();
  h = txt->get_height ();
}

void *csTextureMM::GetMipMapData (int mm)
{
  switch (mm)
  {
    case -1: return t2d ? t2d->get_bitmap () : NULL;
    case 0: return t1 ? t1->get_bitmap () : NULL;
    case 1: return t2 ? t2->get_bitmap () : NULL;
    case 2: return t3 ? t3->get_bitmap () : NULL;
    case 3: return t4 ? t4->get_bitmap () : NULL;
  }
  return NULL;
}

void csTextureMM::GetBitmapDimensions (int& w, int& h) 
{ 
  w = t2d->get_width ();
  h = t2d->get_height ();
}

void *csTextureMM::GetBitmapData ()
{ 
  return t2d->get_bitmap ();
}

void csTextureMM::GetMeanColor (float& r, float& g, float& b)
{ 
  r = mean_color.red;
  g = mean_color.green;
  b = mean_color.blue;
}

/// Get the transparent color
void csTextureMM::GetTransparent (int &red, int &green, int &blue)
{
  red   = transp_color.red;
  green = transp_color.green;
  blue  = transp_color.blue;
}

void csTextureMM::AdjustSize()
{
  int newwidth  = ifile->GetWidth();
  int newheight = ifile->GetHeight();

  if (!IsPowerOf2(newwidth))
  {
    newwidth  = FindNearestPowerOf2(ifile->GetWidth()) /2;
  }

  if (!IsPowerOf2(newheight))
  {
    newheight = FindNearestPowerOf2(ifile->GetHeight())/2;
  }

  if (newwidth  != ifile->GetWidth() ||
      newheight != ifile->GetHeight())
  {
    iImageFile* oldimage = ifile;
    ifile                = oldimage->Resize(newwidth, newheight);
    oldimage->DecRef();
  }
}

void csHardwareAcceleratedTextureMM::convert_to_internal
  (csTextureManager* tex, iImageFile* imfile, unsigned char* bm)
{
  int s = imfile->GetSize ();
  RGBPixel* src = imfile->GetImageData ();
  ULong* dest = (ULong*) bm;

  // Map the texture to the RGB palette.

  //Approximation for black, because we can't use real black. Value 0 is 
  //reserved for transparent pixels!
  ULong  black = tex->get_almost_black(); 

  if (GetTransparent ())
    // transparent Textures need special processing. Every texel, that is transp_color
    // will be transformed to 0, because that is the color we use internally to mark
    // transparent texels.
    for (; s; src++, s--)
      if (transp_color == *src)
        *dest++ = 0;
      else
      {
        ULong texel = (src->red << rs24) | (src->green << gs24) | (src->blue << bs24);
        *dest++ = texel ? texel : black; //transform 0 to become "black"
      }
  else
    for (; s; src++, s--)
      *dest++ = (src->red << rs24) | (src->green << gs24) | (src->blue << bs24);
}

//---------------------------------------------------------------------------

csTexture::csTexture (csTextureMM* p, int w, int h)
{
  parent = p;
  width = w;
  height = h;
  init ();
}


csTexture8::csTexture8 (csTextureMM* p, int w, int h) : csTexture (p, w, h)
{
  CHK (bitmap = new unsigned char [w*h]);
}

csTexture16::csTexture16 (csTextureMM* p, int w, int h) : csTexture (p, w, h)
{
  CHK (bitmap = (UByte *)new UShort [w*h]);
}

csTexture32::csTexture32 (csTextureMM* p, int w, int h) : csTexture (p, w, h)
{
  CHK (bitmap = (UByte *)new ULong [w*h]);
}

void csTexture::init ()
{
  switch (width)
  {
    case 4: shf_w = 2; and_w = 0x3; break;
    case 8: shf_w = 3; and_w = 0x7; break;
    case 16: shf_w = 4; and_w = 0xf; break;
    case 32: shf_w = 5; and_w = 0x1f; break;
    case 64: shf_w = 6; and_w = 0x3f; break;
    case 128: shf_w = 7; and_w = 0x7f; break;
    case 256: shf_w = 8; and_w = 0xff; break;
    case 512: shf_w = 9; and_w = 0x1ff; break;
    case 1024: shf_w = 10; and_w = 0x3ff; break;
  }
  switch (height)
  {
    case 4: shf_h = 2; and_h = 0x3; break;
    case 8: shf_h = 3; and_h = 0x7; break;
    case 16: shf_h = 4; and_h = 0xf; break;
    case 32: shf_h = 5; and_h = 0x1f; break;
    case 64: shf_h = 6; and_h = 0x3f; break;
    case 128: shf_h = 7; and_h = 0x7f; break;
    case 256: shf_h = 8; and_h = 0xff; break;
    case 512: shf_h = 9; and_h = 0x1ff; break;
    case 1024: shf_h = 10; and_h = 0x3ff; break;
  }
}

csTexture::~csTexture ()
{
}

csTexture8::~csTexture8 ()
{
  CHK (delete [] bitmap);
}

csTexture16::~csTexture16 ()
{
  CHK (delete [] bitmap);
}

csTexture32::~csTexture32 ()
{
  CHK (delete [] bitmap);
}

//---------------------------------------------------------------------------

IMPLEMENT_IBASE (csTextureManager)
  IMPLEMENTS_INTERFACE (iTextureManager)
IMPLEMENT_IBASE_END

Filter3x3 csTextureManager::mipmap_filter_1 =
{
  1, 2, 1,
  2, 4, 2,
  1, 2, 1,
  16
};

Filter5x5 csTextureManager::mipmap_filter_2 =
{
  1, 1, 1, 1, 1,
  1, 2, 4, 2, 1,
  1, 4, 8, 4, 1,
  1, 2, 4, 2, 1,
  1, 1, 1, 1, 1,
  48
};

Filter3x3 csTextureManager::blend_filter =
{
  1, 2, 1,
  2, 4, 2,
  1, 2, 1,
  16
};

csTextureManager::csTextureManager (iSystem* iSys, iGraphics2D* iG2D) :
  textures (64, 64)
{
  System = iSys;
  (G2D = iG2D)->IncRef ();
  verbose = false;
  factory_3d = NULL;
  factory_2d = NULL;

  Gamma = 1.0;
  mipmap_nice = 0;
  do_blend_mipmap0 = false;
  do_lightmapgrid = false;
  do_lightmaponly = false;
  mixing = MIX_TRUE_RGB;
  force_mix = -1;
  use_rgb = true;
}

void csTextureManager::Initialize ()
{
  pfmt = *G2D->GetPixelFormat ();
}

csTextureManager::~csTextureManager()
{
  clear ();
  CHK (delete factory_3d);
  CHK (delete factory_2d);
  if (G2D)
    G2D->DecRef ();
}

void csTextureManager::clear ()
{
  textures.DeleteAll ();
}

void csTextureManager::SysPrintf (int mode, char* szMsg, ...)
{
  char buf[1024];
  va_list arg;

  va_start (arg, szMsg);
  vsprintf (buf, szMsg, arg);
  va_end (arg);

  System->Print (mode, buf);
}

int csTextureManager::FindRGB (int r, int g, int b)
{
  return find_color (r, g, b);
}

int csTextureManager::get_almost_black ()
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
