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
#include "iimage.h"
#include "isystem.h"
#include "lightdef.h"

//---------------------------------------------------------------------------

IMPLEMENT_UNKNOWN_NODELETE (csTextureMM)

BEGIN_INTERFACE_TABLE (csTextureMM)
  IMPLEMENTS_COMPOSITE_INTERFACE (TextureHandleMM)
END_INTERFACE_TABLE ()

csTextureMM::csTextureMM (IImageFile* image)
{
  t1 = t2 = t3 = t4 = t2d = NULL;
  rs24 = 16; gs24 = 8; bs24 = 0;

  usage = NULL;
  istransp = false;

  ifile = image;
  ifile->AddRef ();
}

csTextureMM::~csTextureMM ()
{
  CHK (delete t1);
  CHK (delete t2);
  CHK (delete t3);
  CHK (delete t4);
  CHK (delete t2d);
  free_image ();
  free_usage_table ();
}

void csTextureMM::create_blended_mipmap (csTextureManager* tex, unsigned char* bm)
{
  if (!ifile) return;
  IImageFile* if2;
  bool tran = get_transparent ();
  if (tran)
  {
    // Just copy.
    convert_to_internal (tex, ifile, bm);
  }
  else
  {
    ifile->Blend (&csTextureManager::blend_filter, &if2);
    convert_to_internal (tex, if2, bm);
    if2->Release ();
  }
}

void csTextureMM::alloc_mipmaps (csTextureManager* tex)
{
  if (!ifile) return;

  CHK (delete t1); t1 = NULL;
  CHK (delete t2); t2 = NULL;
  CHK (delete t3); t3 = NULL;
  CHK (delete t4); t4 = NULL;

  int w, h;
  ifile->GetWidth (w);
  ifile->GetHeight (h);

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
  int w, h;
  ifile->GetWidth (w);
  ifile->GetHeight (h);

  tt = tex->factory_3d->new_texture (this, w, h);
  create_blended_mipmap (tex, tt->get_bitmap8 ());
  t1->copy (tt);

  CHK (delete tt);
}

void csTextureMM::create_mipmap_bitmap (csTextureManager* tex, int steps, unsigned char* bm)
{
  if (!ifile) return;
  IImageFile* if2;
  bool tran = get_transparent ();
  if (tran || tex->mipmap_nice == MIPMAP_UGLY) ifile->MipMap (steps, &if2); 
  else if (tex->mipmap_nice == MIPMAP_DEFAULT) ifile->MipMap (steps, NULL, NULL, &if2);
  else ifile->MipMap (steps, &csTextureManager::mipmap_filter_1, &csTextureManager::mipmap_filter_2, &if2);
  convert_to_internal (tex, if2, bm);
  if2->Release ();
}

void csTextureMM::create_mipmaps (csTextureManager* tex)
{
  if (!ifile) return;

  int w, h;
  ifile->GetWidth (w);
  ifile->GetHeight (h);

  if (tex->mipmap_nice == MIPMAP_VERYNICE)
  {
    // Mipmap level 1 (the same size of texture as level 0 but blended a little)
    create_blended_mipmap (tex, t2->get_bitmap8 ());

    // Mipmap level 2 (mipmap starting from the blended version).
    create_mipmap_bitmap (tex, 1, t3->get_bitmap8 ());

    // Mipmap level 3 (mipmap starting from the blended version).
    create_mipmap_bitmap (tex, 2, t4->get_bitmap8 ());

    // Size of last mipmap (for computation of mean value).
    w /= 4; h /= 4;
  }
  else
  {
    // Mipmap level 1 (mipmap starting from original version).
    create_mipmap_bitmap (tex, 1, t2->get_bitmap8 ());

    // Mipmap level 2 (mipmap starting from original version).
    create_mipmap_bitmap (tex, 2, t3->get_bitmap8 ());

    // Mipmap level 3 (mipmap starting from level 1).
    create_mipmap_bitmap (tex, 3, t4->get_bitmap8 ());

    // Size of last mipmap (for computation of mean value).
    w /= 8; h /= 8;
  }

  // Compute one mean value for this texture. This is then used when texture
  // mapping is disabled.
  int x, y;
  ULong r, g, b;
  RGBPixel* d;
  ifile->GetImageData (&d);
  ifile->GetWidth (w);
  ifile->GetHeight (h);
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

  CHK (delete t2d); t2d = NULL;

  int w, h;
  ifile->GetWidth (w);
  ifile->GetHeight (h);
  t2d = tex->factory_2d->new_texture (this, w, h);
}


void csTextureMM::free_image ()
{
  if (ifile) { ifile->Release (); ifile = NULL; }
}

void csTextureMM::free_usage_table ()
{
  CHK (delete usage); usage = NULL;
}

// This function must be called BEFORE color remapping.
void csTextureMM::set_transparent (int red, int green, int blue)
{
  transp_color.red = red;
  transp_color.green = green;
  transp_color.blue = blue;
  istransp = true;
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
  int s;
  ifile->GetSize (s);
  RGBPixel* bm;
  ifile->GetImageData (&bm);
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

  int s;
  ifile->GetSize (s);
  RGBPixel* src;
  ifile->GetImageData (&src);
  ULong* dest = t1->get_bitmap32 ();

  // Map the texture to the RGB palette.

  // Approximation for black, because we can't use real black. Value 0 is
  // reserved for transparent pixels!
  ULong black = new_palette->get_almost_black ();

  if (get_transparent ())
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
  int s;
  ifile->GetSize (s);
  RGBPixel* src;
  ifile->GetImageData (&src);
  UShort* dest = t2d->get_bitmap16 ();

  // Approximation for black, because we can't use real black. Value 0 is 
  // reserved for transparent pixels!
  UShort black = new_palette->get_almost_black();

  if (get_transparent ())
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
  int s;
  ifile->GetSize (s);
  RGBPixel* src;
  ifile->GetImageData (&src);
  ULong* dest = t2d->get_bitmap32 ();
  
  // Approximation for black, because we can't use real black. Value 0 is 
  // reserved for transparent pixels!
  ULong black = new_palette->get_almost_black(); 

  if (get_transparent ())
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


void csHardwareAcceleratedTextureMM::convert_to_internal
  (csTextureManager* tex, IImageFile* imfile, unsigned char* bm)
{
  int s;
  imfile->GetSize (s);
  RGBPixel* src;
  imfile->GetImageData (&src);
  ULong* dest = (ULong*) bm;

  // Map the texture to the RGB palette.

  //Approximation for black, because we can't use real black. Value 0 is 
  //reserved for transparent pixels!
  ULong  black = tex->get_almost_black(); 

  if (get_transparent ())
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
  CHK (bitmap = new UShort [w*h]);
}

csTexture32::csTexture32 (csTextureMM* p, int w, int h) : csTexture (p, w, h)
{
  CHK (bitmap = new ULong [w*h]);
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

IMPLEMENT_UNKNOWN (csTextureManager)

BEGIN_INTERFACE_TABLE (csTextureManager)
    IMPLEMENTS_INTERFACE (ITextureManager)
END_INTERFACE_TABLE ()

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

csTextureManager::csTextureManager (ISystem* piSystem, IGraphics2D* piG2D)
{
  m_piSystem = piSystem;
  m_piG2D = piG2D;
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

void csTextureManager::InitSystem ()
{
  IGraphicsInfo* piGI = NULL;
  VERIFY_SUCCESS (m_piG2D->QueryInterface((IID&)IID_IGraphicsInfo, (void**)&piGI));
  piGI->GetPixelFormat (&pfmt);
  piGI->Release ();
}

csTextureManager::~csTextureManager()
{
  clear ();
  CHK (delete factory_3d);
  CHK (delete factory_2d);
}

void csTextureManager::clear ()
{
  //int i;
  //for (i = 0 ; i < textures.Length () ; i++)
  //{
    //CHK (delete (csTextureMM*)(textures[i]));
    //textures[i] = NULL;
  //}
  //textures.DeleteAll ();
}

void csTextureManager::SysPrintf (int mode, char* szMsg, ...)
{
  char buf[1024];
  va_list arg;

  va_start (arg, szMsg);
  vsprintf (buf, szMsg, arg);
  va_end (arg);

  m_piSystem->Print (mode, buf);
}

STDMETHODIMP csTextureManager::FindRGB (int r, int g, int b, int& color)
{
  color = find_color (r, g, b);
  return S_OK;
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

//---------------------------------------------------------------------------

IMPLEMENT_COMPOSITE_UNKNOWN (csTextureMM, TextureHandleMM)

IMPLEMENT_GET_PROPERTY (GetTransparent, istransp, bool, csTextureMM, TextureHandleMM)
IMPLEMENT_GET_PROPERTY (GetMeanColor, mean_idx, int, csTextureMM, TextureHandleMM)
IMPLEMENT_GET_PROPERTY (GetNumberOfColors, usage->get_num_colors(), int, csTextureMM, TextureHandleMM)
 
STDMETHODIMP ITextureHandleMM::SetTransparent (int red, int green, int blue) 
{ 
  METHOD_PROLOGUE (csTextureMM, TextureHandleMM)
  pThis->set_transparent (red, green, blue);
  return S_OK; 
}

STDMETHODIMP ITextureHandleMM::GetMipMapDimensions (int mipmap, int& w, int& h) 
{ 
  METHOD_PROLOGUE (csTextureMM, TextureHandleMM)
  csTexture* txt = pThis->get_texture (mipmap);
  w = txt->get_width ();
  h = txt->get_height ();
  return S_OK; 
}

STDMETHODIMP ITextureHandleMM::GetBitmapDimensions (int& w, int& h) 
{ 
  METHOD_PROLOGUE (csTextureMM, TextureHandleMM)
  csTexture* txt = pThis->get_texture (-1);
  w = txt->get_width ();
  h = txt->get_height ();
  return S_OK; 
}

STDMETHODIMP ITextureHandleMM::GetBitmapData (void** bmdata)
{ 
  METHOD_PROLOGUE (csTextureMM, TextureHandleMM)
  csTexture* txt = pThis->get_texture (-1);
  *bmdata = (void*)(txt->get_bitmap8 ());
  return S_OK; 
}

STDMETHODIMP ITextureHandleMM::GetMeanColor (float& r, float& g, float& b)
{ 
  METHOD_PROLOGUE (csTextureMM, TextureHandleMM)
  r = pThis->mean_color.red;
  g = pThis->mean_color.green;
  b = pThis->mean_color.blue;
  return S_OK; 
}
