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

#define SysPrintf System->Printf

#define RESERVED_COLOR(c) ((c == 0) || (c == 255))

csTextureMMGlide::csTextureMMGlide (iImage* image, int flags) : 
  csTextureMM (image, flags)
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
  Clear ();
  read_config ();
}

csTextureManagerGlide::~csTextureManagerGlide ()
{
  Clear ();
}

ULong csTextureManagerGlide::encode_rgb (int r, int g, int b)
{
  return
    ((r >> (8-pfmt.RedBits))   << pfmt.RedShift) |
    ((g >> (8-pfmt.GreenBits)) << pfmt.GreenShift) |
    ((b >> (8-pfmt.BlueBits))  << pfmt.BlueShift);
}

void csTextureManagerGlide::remap_textures ()
{
  int i;

  // Remap all textures according to the new colormap.
  for (i = 0 ; i < textures.Length () ; i++)
    ((csTextureMMGlide*)textures[i])->remap_texture (this);
}

void csTextureManagerGlide::PrepareTextures ()
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

  remap_textures ();
}

iTextureHandle *csTextureManagerGlide::RegisterTexture (iImage* image,
  int flags)
{
  if (!image) return NULL;

  csTextureMMGlide *txt = new csTextureMMGlide (image, flags);
  textures.Push (txt);
  return txt;
}

void csTextureManagerOpenGL::PrepareTexture (iTextureHandle *handle)
{
  if (!handle) return;

  csTextureMMGlide *txt = (csTextureMMGlide *)handle->GetPrivateObject ();
  txt->create_mipmaps (mipmap_mode == MIPMAP_VERYNICE, do_blend_mipmap0);
  txt->remap_texture (this);
}

void csTextureManagerGlide::UnregisterTexture (iTextureHandle* handle)
{
  (void)handle;
  //@@@ Not implemented yet.
}

void csTextureManagerGlide::PrepareTexture (iTextureHandle* handle)
{
  (void)handle;
  //@@@ Not implemented yet.
}
