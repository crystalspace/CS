/*
    Copyright (C) 2000 by Jorrit Tyberghein

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
#include "inf_txt.h"
#include "csgfxldr/quantize.h"
#include "csutil/scanstr.h"
#include "csutil/inifile.h"
#include "isystem.h"
#include "iimage.h"
#include "lightdef.h"

#define SysPrintf System->Printf

//-------------------------------------------------------- csTextureMMInfinite ---//

csTextureMMInfinite::csTextureMMInfinite (iImage *image, int flags) :
  csTextureMM (image, flags)
{
}

csTextureMMInfinite::~csTextureMMInfinite ()
{
}

csTexture *csTextureMMInfinite::NewTexture (iImage *Image)
{
  return new csTextureInfinite (this, Image);
}

void csTextureMMInfinite::ComputeMeanColor ()
{
  int i;

  // Compute a common palette for all three mipmaps
  csQuantizeBegin ();

  RGBPixel *tc = transp ? &transp_color : NULL;

  for (i = 0; i < 4; i++)
    if (tex [i])
    {
      csTextureInfinite *t = (csTextureInfinite *)tex [i];
      if (!t->image) break;
      csQuantizeCount ((RGBPixel *)t->image->GetImageData (),
        t->get_size (), tc);
    }

  RGBPixel *pal = palette;
  palette_size = 256;
  csQuantizePalette (pal, palette_size, tc);

  for (i = 0; i < 4; i++)
    if (tex [i])
    {
      csTextureInfinite *t = (csTextureInfinite *)tex [i];
      if (!t->image) break;

      csQuantizeRemap ((RGBPixel *)t->image->GetImageData (),
        t->get_size (), t->bitmap, tc);

      // Very well. Now we don'tex need the iImage anymore, so free it
      t->image->DecRef ();
      t->image = NULL;
    }

  csQuantizeEnd ();

  mean_color.red   = 0;
  mean_color.green = 0;
  mean_color.blue  = 0;
}

//----------------------------------------------- csTextureManagerInfinite ---//

csTextureManagerInfinite::csTextureManagerInfinite (iSystem *iSys,
  iGraphics2D *iG2D, csIniFile *config) : csTextureManager (iSys, iG2D)
{
  read_config (config);
  G2D = iG2D;
}

ULong csTextureManagerInfinite::encode_rgb (int r, int g, int b)
{
  return
    ((r >> (8 - pfmt.RedBits))   << pfmt.RedShift) |
    ((g >> (8 - pfmt.GreenBits)) << pfmt.GreenShift) |
    ((b >> (8 - pfmt.BlueBits))  << pfmt.BlueShift);
}

void csTextureManagerInfinite::PrepareTextures ()
{
  if (verbose) SysPrintf (MSG_INITIALIZATION, "Preparing textures...\n");
  if (verbose) SysPrintf (MSG_INITIALIZATION, "  Creating texture mipmaps...\n");

  // Create mipmaps for all textures
  int i;
  for (i = 0; i < textures.Length (); i++)
  {
    csTextureMM *txt = textures.Get (i);
    txt->CreateMipmaps ();
  }
}

iTextureHandle *csTextureManagerInfinite::RegisterTexture (iImage* image,
  int flags)
{
  if (!image) return NULL;

  CHK (csTextureMMInfinite *txt = new csTextureMMInfinite (image, flags));
  textures.Push (txt);
  return txt;
}

void csTextureManagerInfinite::PrepareTexture (iTextureHandle *handle)
{
  if (!handle) return;

  csTextureMMInfinite *txt = (csTextureMMInfinite *)handle->GetPrivateObject ();
  txt->CreateMipmaps ();
}

void csTextureManagerInfinite::UnregisterTexture (iTextureHandle* handle)
{
  csTextureMMInfinite *tex_mm = (csTextureMMInfinite *)handle->GetPrivateObject ();
  int idx = textures.Find (tex_mm);
  if (idx >= 0) textures.Delete (idx);
}

