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
#include "d3d_txtmgr.h"
#include "d3d_g3d.h"
#include "csutil/scanstr.h"
#include "itexture.h"
#include "ipolygon.h"
#include "ilghtmap.h"
#include "iimage.h"
#include "isystem.h"

#define SysPrintf System->Printf

//---------------------------------------------------------------------------

csTextureMMDirect3D::csTextureMMDirect3D (iImage* image, int flags,
  csGraphics3DDirect3DDx6 *iG3D) : csTextureMM (image, flags)
{
  G3D = iG3D;

  // Resize the image to fullfill device requirements
  int w = image->GetWidth ();
  int h = image->GetHeight ();

  if (w / h > G3D->m_MaxAspectRatio)
    h = w / G3D->m_MaxAspectRatio;
  if (h / w > G3D->m_MaxAspectRatio)
    w = h / G3D->m_MaxAspectRatio;
  if (w < G3D->m_Caps.minTexWidth)
    w  = G3D->m_Caps.minTexWidth;
  if (h < G3D->m_Caps.minTexHeight)
    h = G3D->m_Caps.minTexHeight;
  if (w > G3D->m_Caps.maxTexWidth)
    w  = G3D->m_Caps.maxTexWidth;
  if (h > G3D->m_Caps.maxTexHeight)
    h = G3D->m_Caps.maxTexHeight;

  // this is a NOP if size do not change
  image->Rescale (w, h);
}

csTexture *csTextureMMDirect3D::new_texture (iImage *Image)
{
  return new csTextureDirect3D (this, Image, G3D);
}

void csTextureMMDirect3D::compute_mean_color ()
{
  int pixels = image->GetWidth () * image->GetHeight ();
  RGBPixel *src = (RGBPixel *)image->GetImageData ();
  unsigned r = 0, g = 0, b = 0;
  for (int count = pixels; count; count--)
  {
    RGBPixel pix = *src++;
    r += pix.red;
    g += pix.green;
    b += pix.blue;
  }
  mean_color.red   = r / pixels;
  mean_color.green = g / pixels;
  mean_color.blue  = b / pixels;
}

//---------------------------------------------------------------------------

csTextureDirect3D::csTextureDirect3D (csTextureMM *Parent, iImage *Image,
  csGraphics3DDirect3DDx6 *iG3D) : csTexture (Parent)
{
  w = Image->GetWidth ();
  h = Image->GetHeight ();
  compute_masks ();

  // Convert the texture to the screen-dependent format
  int rsl = G3D->txtmgr->rsl;
  int rsr = G3D->txtmgr->rsr;
  int gsl = G3D->txtmgr->gsl;
  int gsr = G3D->txtmgr->gsr;
  int bsl = G3D->txtmgr->bsl;
  int bsr = G3D->txtmgr->bsr;

  RGBPixel *src = (RGBPixel *)Image->GetData ();
  int pixels = get_size ();
  switch (G3D->m_ddsdLightmapSurfDesc.ddpfPixelFormat.dwRGBBitCount)
  {
    case 16:
    {
      image = new UByte [pixels * sizeof (UShort)];
      UShort *dst = (UShort *)image;
      while (pixels--)
      {
        *dst = ((unsigned (src->red  ) >> rsr) << rsl) |
               ((unsigned (src->green) >> gsr) << gsl) |
               ((unsigned (src->blue ) >> bsr) << bsl);
        src++;
      }
      break;
    }
    case 32:
    {
      image = new UByte [pixels * sizeof (ULong)];
      ULong *dst = (ULong *)image;
      while (pixels--)
      {
        *dst = ((unsigned (src->red  ) >> rsr) << rsl) |
               ((unsigned (src->green) >> gsr) << gsl) |
               ((unsigned (src->blue ) >> bsr) << bsl);
        src++;
      }
      break;
    }
  }

  Image->DecRef ();
}

csTextureDirect3D::~csTextureDirect3D ()
{
  delete [] image;
}

void *csTextureDirect3D::get_bitmap ()
{
  csGraphics3DDirect3DDx6 *G3D = ((csTextureMMDirect3D *)parent)->G3D;
  G3D->texture_cache->cache_texture (parent);
  csD3DCacheData *cd = (csD3DCacheData *)parent->GetCacheData ();
  return (void *)cd;
}

//---------------------------------------------------------------------------

csTextureManagerDirect3D::csTextureManagerDirect3D (iSystem* iSys,
  iGraphics2D* iG2D, csIniFile *config) : csTextureManager (iSys, iG2D)
{
  read_config (config);
  Clear ();

  // Compute the shift masks for converting R8G8B8 into texture format
#define COMPUTE(sr, sl, mask)               \
  sr = 8; sl = 0;                           \
  {                                         \
    unsigned m = mask;                      \
    while ((m & 1) == 0) { sl++; m >>= 1; } \
    while ((m & 1) == 1) { sr--; m >>= 1; } \
    if (sr < 0) { sl -= sr; sr = 0; }       \
  }

  COMPUTE (rsr, rsl, m_ddsdTextureSurfDesc.ddpfPixelFormat.dwRBitMask);
  COMPUTE (gsr, gsl, m_ddsdTextureSurfDesc.ddpfPixelFormat.dwGBitMask);
  COMPUTE (bsr, bsl, m_ddsdTextureSurfDesc.ddpfPixelFormat.dwBBitMask);
#undef COMPUTE
}

csTextureManagerDirect3D::~csTextureManagerDirect3D ()
{
  Clear ();
}

void csTextureManagerDirect3D::PrepareTextures ()
{
  if (verbose) SysPrintf (MSG_INITIALIZATION, "Preparing textures...\n");

  if (verbose) SysPrintf (MSG_INITIALIZATION, "  Creating texture mipmaps...\n");

  // Create mipmaps for all textures
  for (int i = 0; i < textures.Length (); i++)
  {
    csTextureMM *txt = textures.Get (i);
    txt->apply_gamma ();
    txt->create_mipmaps (mipmap_mode == MIPMAP_VERYNICE, do_blend_mipmap0);
  }
}

iTextureHandle *csTextureManagerDirect3D::RegisterTexture (iImage* image,
  int flags)
{
  if (!image) return NULL;

  csTextureMMDirect3D* txt = new csTextureMMDirect3D (image, flags, G3D);
  textures.Push (txt);
  return txt;
}

void csTextureManagerDirect3D::PrepareTexture (iTextureHandle *handle)
{
  if (!handle) return;

  csTextureMMDirect3D *txt = (csTextureMMDirect3D *)handle->GetPrivateObject ();
  txt->apply_gamma ();
  txt->create_mipmaps (mipmap_mode == MIPMAP_VERYNICE, do_blend_mipmap0);
}

void csTextureManagerDirect3D::UnregisterTexture (iTextureHandle* handle)
{
  G3D->UncacheTexture (handle);
  csTextureMMOpenGL *tex_mm = (csTextureMMOpenGL *)handle->GetPrivateObject ();
  int idx = textures.Find (tex_mm);
  if (idx >= 0) textures.Delete (idx);
}
