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

#include "cssysdef.h"
#include "isys/isystem.h"

#include "gltex.h"
#include "g3dgl.h"
#include "glproc.h"
#include "igraphic/iimage.h"
#include "csutil/scanstr.h"

#define SysPrintf System->Printf

#define RESERVED_COLOR(c) ((c == 0) || (c == 255))

csTextureGlide::csTextureGlide (csTextureHandle *Parent, iImage *Image)
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

csTextureHandleGlide::csTextureHandleGlide (csGraphics3DGlide *g3d,
  iImage* image, int flags) : csTextureHandle (image, flags)
{
  this->g3d = g3d;
  dyn = NULL;
  if ( flags & CS_TEXTURE_3D ) AdjustSizePo2 ();
}

csTextureHandleGlide::~csTextureHandleGlide ()
{
  g3d->txtmgr->UnregisterTexture (this);
  if (dyn) delete dyn;
}

csTexture *csTextureHandleGlide::NewTexture (iImage *Image)
{
  return new csTextureGlide (this, Image);
}

iGraphics3D *csTextureHandleGlide::GetProcTextureInterface()
{
  if ( (flags & CS_TEXTURE_PROC) && dyn == NULL)
  {
    dyn = new csGlideProcedural (NULL);
    dyn->SetTarget (g3d, this);
  }
  return dyn;
}

void csTextureHandleGlide::ComputeMeanColor ()
{
  csTextureGlide *tex;
  int i, pixels;
  csRGBpixel *src;
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
    csRGBpixel pix = *src++;
    r += pix.red;
    g += pix.green;
    b += pix.blue;
  }
  mean_color.red   = r / pixels;
  mean_color.green = g / pixels;
  mean_color.blue  = b / pixels;
}

void csTextureHandleGlide::remap_mm ()
{
  csTextureGlide *tex;
  int w, h, i;
  csRGBpixel* src;
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
         dest = new UShort[ w*h ] ;
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

void csTextureHandleGlide::Prepare ()
{
  CreateMipmaps ();
  remap_mm ();
}

//---------------------------------------------------------------------------

csTextureManagerGlide::csTextureManagerGlide (iSystem* iSys, iGraphics2D* iG2D,
  csGraphics3DGlide* iG3D, iConfigFile *config)
  : csTextureManager (iSys, iG2D)
{
  g3d = iG3D;
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
    csTextureHandle* txt = textures.Get (i);
    txt->CreateMipmaps ();
    ((csTextureHandleGlide *)txt)->remap_mm();
  }
}

iTextureHandle *csTextureManagerGlide::RegisterTexture (iImage* image,
  int flags)
{
  if (!image) { printf( "NULL image\n"); return NULL; }

  csTextureHandleGlide *txt = new csTextureHandleGlide (g3d, image, flags);
  textures.Push (txt);
  return txt;
}

void csTextureManagerGlide::UnregisterTexture (csTextureHandleGlide *handle)
{
  int idx = textures.Find (handle);
  if (idx >= 0) textures.Delete (idx);
}
