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

csTextureGlide::csTextureGlide (csTextureMM *Parent, iImage *Image)
  : csTexture (Parent)
{
  image = Image;
  w = image->GetWidth ();
  h = image->GetHeight ();
  raw = NULL;
  compute_masks ();
}

csTextureGlide::~csTextureGlide ()
{
  if (image) image->DecRef ();
  if ( raw ) delete [] raw;
}

csTextureMMGlide::csTextureMMGlide (iImage* image, int flags) : 
  csTextureMM (image, flags)
{
  if ( flags & CS_TEXTURE_3D ) AdjustSizePo2 ();
  else printf( "no 3d tex: dim %d, %d\n", image->GetWidth(), image->GetHeight() );
}

csTexture *csTextureMMGlide::NewTexture (iImage *Image)
{
  return new csTextureGlide (this, Image);
}

void csTextureMMGlide::ComputeMeanColor ()
{
  csTextureGlide *tex;
  int i, pixels;
  RGBPixel *src;
  unsigned r = 0, g = 0, b = 0;
  
  // find smallest mipmap around
  for (i=3; i >= 0; i--)
  {
    tex = (csTextureGlide*)get_texture (i);
    if (tex) break;
  }
  if (!tex) return;
  
  i = pixels = tex->get_width () * tex->get_height ();
  src = tex->get_image_data ();
  
  while (i--)
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

void csTextureMMGlide::remap_mm ()
{

  csTextureGlide *tex;
  int w, h, i;
  RGBPixel* src;
  UShort *dest;
  int x, y;

  for (i=0; i < 4; i++)
  {
    tex = (csTextureGlide*)get_texture (i);
    if (tex)
    {
      w = tex->get_width ();
      h = tex->get_height ();
      src = tex->get_image_data ();
      dest = (UShort *)tex->get_bitmap ();
      if ( !dest )
      {
        CHK ( dest = new UShort[ w*h ] );
	tex->raw = dest;
      }
      
      for (y = 0 ; y < h ; y++)
        for (x = 0 ; x < w ; x++)
        {
          *dest++ = ((src->red >> 3 ) << 11) |
                    ((src->green >> 2 ) << 5) |
                    ((src->blue >> 3));
          src++;
        }
    }
  }
}

//---------------------------------------------------------------------------

csTextureManagerGlide::csTextureManagerGlide (iSystem* iSys, iGraphics2D* iG2D,
  csIniFile *config) :
  csTextureManager (iSys, iG2D)
{
  read_config (config);
  Clear ();
}

csTextureManagerGlide::~csTextureManagerGlide ()
{
  Clear ();
}

void csTextureManagerGlide::PrepareTextures ()
{
  int i;
  if (verbose) SysPrintf (MSG_INITIALIZATION, "Preparing textures...\n");
  if (verbose) SysPrintf (MSG_INITIALIZATION, "  Creating texture mipmaps...\n");
  for (i = 0 ; i < textures.Length () ; i++)
  {
    csTextureMM* txt = textures.Get (i);
    txt->CreateMipmaps ();
    ((csTextureMMGlide *)txt)->remap_mm();
  }
}

iTextureHandle *csTextureManagerGlide::RegisterTexture (iImage* image,
  int flags)
{
  if (!image) { printf( "NULL image\n"); return NULL; }

  csTextureMMGlide *txt = new csTextureMMGlide (image, flags);
  textures.Push (txt);
  return txt;
}

void csTextureManagerGlide::PrepareTexture (iTextureHandle *handle)
{
  if (!handle) { printf( "NULL image\n"); return; }

  csTextureMMGlide *txt = (csTextureMMGlide *)handle->GetPrivateObject ();
  txt->CreateMipmaps ();
  txt->remap_mm ();
}

void csTextureManagerGlide::UnregisterTexture (iTextureHandle* handle)
{
  (void)handle;
  //@@@ Not implemented yet.
}

