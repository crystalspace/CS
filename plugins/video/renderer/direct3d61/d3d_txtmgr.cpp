/*
    Copyright (C) 1998 by Jorrit Tyberghein

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURP.OSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <stdarg.h>
#include <math.h>

#include "cssysdef.h"
#include "d3d_txtmgr.h"
#include "d3d_g3d.h"
#include "csutil/scanstr.h"
#include "itexture.h"
#include "ipolygon.h"
#include "ilghtmap.h"
#include "iimage.h"
#include "isystem.h"

//---------------------------------------------------------------------------

csTextureDirect3D::csTextureDirect3D (csTextureHandle *Parent,
  iImage *Image, csGraphics3DDirect3DDx6 *iG3D, bool For2d)
  : csTexture (Parent)
{
  w = Image->GetWidth ();
  h = Image->GetHeight ();
  compute_masks ();

  // Convert the texture to the screen-dependent format
  int rsl = For2d ? iG3D->ScreenRsl : iG3D->TextureRsl;
  int rsr = For2d ? iG3D->ScreenRsr : iG3D->TextureRsr;
  int gsl = For2d ? iG3D->ScreenGsl : iG3D->TextureGsl;
  int gsr = For2d ? iG3D->ScreenGsr : iG3D->TextureGsr;
  int bsl = For2d ? iG3D->ScreenBsl : iG3D->TextureBsl;
  int bsr = For2d ? iG3D->ScreenBsr : iG3D->TextureBsr;

  Image->SetFormat(CS_IMGFMT_TRUECOLOR);
  csRGBpixel *pPixels   = (csRGBpixel *)Image->GetImageData();
  int NumPixels = get_size ();

  bool transp = Parent->GetKeyColor ();
  UByte Transp_Red   = 0;
  UByte Transp_Green = 0;
  UByte Transp_Blue  = 0;
  if (transp)
  {
    Parent->GetKeyColor(Transp_Red, Transp_Green, Transp_Blue);
  }

  switch (For2d ? 
          iG3D->m_ddsdPrimarySurfDesc.ddpfPixelFormat.dwRGBBitCount :
          iG3D->m_ddsdLightmapSurfDesc.ddpfPixelFormat.dwRGBBitCount)
  {
    case 16:
    {
      image = new UByte [NumPixels * sizeof (UShort)];
      UShort *dst = (UShort *)image;
      UShort NearBlack = 1 << bsl;

      if (transp)
      {
        while (NumPixels--)
        {
          if ((pPixels->red   == Transp_Red) &&
              (pPixels->green == Transp_Green) &&
              (pPixels->blue  == Transp_Blue))
          {
            *dst++ = 0;
          }
          else
          {
            UShort c = ((unsigned (pPixels->red  ) >> rsr) << rsl) |
                       ((unsigned (pPixels->green) >> gsr) << gsl) |
                       ((unsigned (pPixels->blue ) >> bsr) << bsl);
            *dst++ = c ? c : NearBlack;
          }
          pPixels++;
        }
      }
      else
      {
        while (NumPixels--)
        {
          *dst++ = ((unsigned (pPixels->red  ) >> rsr) << rsl) |
                   ((unsigned (pPixels->green) >> gsr) << gsl) |
                   ((unsigned (pPixels->blue ) >> bsr) << bsl);
          pPixels++;
        }
      }
      break;
    }
    case 32:
    {
      image = new UByte [NumPixels * sizeof (ULong)];
      ULong *dst = (ULong *)image;
      ULong NearBlack = 1 << bsl;

      if (transp)
      {
        while (NumPixels--)
        {
          if ((pPixels->red   == Transp_Red) &&
              (pPixels->green == Transp_Green) &&
              (pPixels->blue  == Transp_Blue))
          {
            *dst++ = 0;
          }
          else
          {
            ULong c = ((unsigned (pPixels->red  ) >> rsr) << rsl) |
                      ((unsigned (pPixels->green) >> gsr) << gsl) |
                      ((unsigned (pPixels->blue ) >> bsr) << bsl);

            *dst++ = c ? c : NearBlack;
          }
          pPixels++;
        }
      }
      else
      {
        while (NumPixels--)
        {
          *dst++ = ((unsigned (pPixels->red  ) >> rsr) << rsl) |
                   ((unsigned (pPixels->green) >> gsr) << gsl) |
                   ((unsigned (pPixels->blue ) >> bsr) << bsl);
          pPixels++;
        }
      }
      break;
    }
  }
}

csTextureDirect3D::~csTextureDirect3D ()
{
  delete [] image;
}

//---------------------------------------------------------------------------

csTextureHandleDirect3D::csTextureHandleDirect3D (iImage* image, int flags,
  csGraphics3DDirect3DDx6 *iG3D) : csTextureHandle (image, flags)
{
  G3D = iG3D;
  m_pTexture2d = NULL;

  // Resize the image to fullfill device requirements
  int w = image->GetWidth ();
  int h = image->GetHeight ();

  G3D->AdjustToOptimalTextureSize(w,h);

  // this is a NOP if size do not change
  image->Rescale (w, h);
}

csTextureHandleDirect3D::~csTextureHandleDirect3D()
{
  G3D->txtmgr->UnregisterTexture (this);
  delete m_pTexture2d;
}

csTexture *csTextureHandleDirect3D::NewTexture (iImage *Image)
{
  return new csTextureDirect3D (this, Image, G3D, false);
}

void csTextureHandleDirect3D::ComputeMeanColor ()
{
  int pixels = image->GetWidth () * image->GetHeight ();
  csRGBpixel *src = (csRGBpixel *)image->GetImageData ();
  unsigned r = 0, g = 0, b = 0;
  for (int count = pixels; count; count--)
  {
    csRGBpixel pix = *src++;
    r += pix.red;
    g += pix.green;
    b += pix.blue;
  }
  if (pixels)
  {
    mean_color.red   = r / pixels;
    mean_color.green = g / pixels;
    mean_color.blue  = b / pixels;
  }
  else
    mean_color = csRGBpixel (0, 0, 0);
}

void csTextureHandleDirect3D::CreateMipmaps ()
{
  if (!image) return;

  // Delete existing mipmaps, if any
  for (int i = 0; i < 4; i++)
    delete tex [i];

  delete m_pTexture2d;

  if (flags & CS_TEXTURE_2D)
    m_pTexture2d = new csTextureDirect3D(this, image, G3D, true);
  else
    m_pTexture2d = NULL;

  csRGBpixel *tc = transp ? &transp_color : (csRGBpixel *)NULL;

  if (flags & CS_TEXTURE_3D)
  {
    image->IncRef();

    iImage *i0 = image;
    iImage *i1 = i0->MipMap (1, tc);
    iImage *i2 = i1->MipMap (1, tc);
    iImage *i3 = i2->MipMap (1, tc);

    tex [0] = NewTexture (i0);
    tex [1] = NewTexture (i1);
    tex [2] = NewTexture (i2);
    tex [3] = NewTexture (i3);
  }
  else
  {
    // 2D textures uses just the top-level mipmap
    image->IncRef ();
    tex [0] = NewTexture (image);
  }

  ComputeMeanColor ();
}

void csTextureHandleDirect3D::Prepare ()
{
  CreateMipmaps ();
}

//---------------------------------------------------------------------------

csTextureManagerDirect3D::csTextureManagerDirect3D (iSystem *iSys,
  iGraphics2D *iG2D, iConfigFileNew *config, csGraphics3DDirect3DDx6 *iG3D) :
  csTextureManager (iSys, iG2D)
{
  m_pG2D = iG2D;
  m_pG3D = iG3D;

  read_config (config);
  Clear ();
}

csTextureManagerDirect3D::~csTextureManagerDirect3D ()
{
  Clear ();
}

int csTextureManagerDirect3D::FindRGB (int r, int g, int b)
{
  if (r > 255) r = 255; else if (r < 0) r = 0;
  if (g > 255) g = 255; else if (g < 0) g = 0;
  if (b > 255) b = 255; else if (b < 0) b = 0;

  return ((r >> m_pG3D->ScreenRsr) << m_pG3D->ScreenRsl) |
         ((g >> m_pG3D->ScreenGsr) << m_pG3D->ScreenGsl) |
         ((b >> m_pG3D->ScreenBsr) << m_pG3D->ScreenBsl);
}

void csTextureManagerDirect3D::PrepareTextures ()
{
  if (verbose) System->Printf (MSG_INITIALIZATION, "Preparing textures...\n");

  if (verbose) System->Printf (MSG_INITIALIZATION, "  Creating texture mipmaps...\n");

  // Create mipmaps for all textures
  for (int i = 0; i < textures.Length (); i++)
  {
    csTextureHandle *txt = textures.Get (i);
    txt->CreateMipmaps ();
  }
}

iTextureHandle *csTextureManagerDirect3D::RegisterTexture (
  iImage* image, int flags)
{
  if (!image) return NULL;

  csTextureHandleDirect3D* txt = new csTextureHandleDirect3D (image, flags, m_pG3D);
  textures.Push (txt);
  return txt;
}

void csTextureManagerDirect3D::UnregisterTexture (csTextureHandleDirect3D* handle)
{
  int idx = textures.Find (handle);
  if (idx >= 0) textures.Delete (idx);
}
