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

#include "cssysdef.h"
#include "line_txt.h"
#include "csgfx/inv_cmap.h"
#include "csgfx/quantize.h"
#include "csutil/scanstr.h"
#include "csutil/debug.h"
#include "iutil/cfgfile.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "igraphic/image.h"

#define RESERVED_COLOR(c) ((c == 0) || (c == 255))

#define CLIP_RGB \
  if (r < 0) r = 0; else if (r > 255) r = 255; \
  if (g < 0) g = 0; else if (g > 255) g = 255; \
  if (b < 0) b = 0; else if (b > 255) b = 255;

//--------------------------------------------------- csTextureHandleLine ---//

csTextureHandleLine::csTextureHandleLine (csTextureManagerLine *txtmgr,
  iImage *image, int flags) : csTextureHandle (image, flags)
{
  pal2glob = NULL;
  (texman = txtmgr)->IncRef ();
}

csTextureHandleLine::~csTextureHandleLine ()
{
  texman->UnregisterTexture (this);
  texman->DecRef ();
}

csTexture *csTextureHandleLine::NewTexture (iImage *Image, bool /*ismipmap*/)
{
  return new csTextureLine (this, Image);
}

void csTextureHandleLine::ComputeMeanColor ()
{
  int i;

  // Compute a common palette for all three mipmaps
  csColorQuantizer quant;
  quant.Begin ();

  csRGBpixel *tc = transp ? &transp_color : 0;

  for (i = 0; i < 4; i++)
    if (tex [i])
    {
      csTextureLine *t = (csTextureLine *)tex [i];
      if (!t->image) break;
      quant.Count ((csRGBpixel *)t->image->GetImageData (),
        t->get_size (), tc);
    }

  csRGBpixel *pal = palette;
  palette_size = 256;
  quant.Palette (pal, palette_size, tc);

  for (i = 0; i < 4; i++)
    if (tex [i])
    {
      csTextureLine *t = (csTextureLine *)tex [i];
      if (!t->image) break;

      quant.Remap ((csRGBpixel *)t->image->GetImageData (),
        t->get_size (), t->bitmap, tc);

      // Very well. Now we don'tex need the iImage anymore, so free it
      DG_UNLINK (t, t->image);
      t->image->DecRef ();
      t->image = NULL;
    }

  quant.End ();

  // Compute the mean color from the palette
  csRGBpixel *src = palette;
  unsigned r = 0, g = 0, b = 0;
  for (i = 0; i < palette_size; i++)
  {
    csRGBpixel pix = *src++;
    r += pix.red;
    g += pix.green;
    b += pix.blue;
  }
  mean_color.red   = r / palette_size;
  mean_color.green = g / palette_size;
  mean_color.blue  = b / palette_size;
}

void csTextureHandleLine::remap_texture (csTextureManager *texman)
{
  int i;
  csTextureManagerLine *txm = (csTextureManagerLine *)texman;
  switch (texman->pfmt.PixelBytes)
  {
    case 2:
      delete [] (uint16 *)pal2glob;
      pal2glob = new uint16 [palette_size];
      for (i = 0; i < palette_size; i++)
        ((uint16 *)pal2glob) [i] = txm->encode_rgb (palette [i].red,
          palette [i].green, palette [i].blue);
      break;
    case 4:
      delete [] (uint32 *)pal2glob;
      pal2glob = new uint32 [palette_size];
      for (i = 0; i < palette_size; i++)
        ((uint32 *)pal2glob) [i] = txm->encode_rgb (palette [i].red,
          palette [i].green, palette [i].blue);
      break;
  }
}

void csTextureHandleLine::Prepare ()
{
  CreateMipmaps ();
  remap_texture (texman);
}

//----------------------------------------------- csTextureManagerLine ---//

static uint8 *GenLightmapTable (int bits)
{
  uint8 *table = new uint8 [64 * 256];
  uint8 *dst = table;
  uint8 maxv = (1 << bits) - 1;
  int rshf = (13 - bits);
  int i, j, x;
  for (i = 0; i < 64; i++)
    for (j = 0; j < 256; j++)
    {
      x = (i * j) >> rshf;
      *dst++ = (x > maxv) ? maxv : x;
    }
  return table;
}

csTextureManagerLine::csTextureManagerLine (iObjectRegistry *object_reg,
  iGraphics2D *iG2D, iConfigFile *config) : csTextureManager (object_reg, iG2D)
{
  read_config (config);
  G2D = iG2D;
}

void csTextureManagerLine::SetPixelFormat (csPixelFormat &PixelFormat)
{
  pfmt = PixelFormat;

  // Create multiplication tables
  lightmap_tables [0] = GenLightmapTable (pfmt.RedBits);

  if (pfmt.GreenBits == pfmt.RedBits)
    lightmap_tables [1] = lightmap_tables [0];
  else
    lightmap_tables [1] = GenLightmapTable (pfmt.GreenBits);

  if (pfmt.BlueBits == pfmt.RedBits)
    lightmap_tables [2] = lightmap_tables [0];
  else if (pfmt.BlueBits == pfmt.GreenBits)
    lightmap_tables [2] = lightmap_tables [1];
  else
    lightmap_tables [2] = GenLightmapTable (pfmt.BlueBits);
}

void csTextureManagerLine::read_config (iConfigFile *config)
{
  csTextureManager::read_config (config);
}

csTextureManagerLine::~csTextureManagerLine ()
{
  delete [] lightmap_tables [0];
  if (lightmap_tables [1] != lightmap_tables [0])
    delete [] lightmap_tables [1];
  if (lightmap_tables [2] != lightmap_tables [1]
   && lightmap_tables [2] != lightmap_tables [0])
    delete [] lightmap_tables [2];
  Clear ();
}

void csTextureManagerLine::Clear ()
{
  csTextureManager::Clear ();
}

uint32 csTextureManagerLine::encode_rgb (int r, int g, int b)
{
  return
    ((r >> (8 - pfmt.RedBits))   << pfmt.RedShift) |
    ((g >> (8 - pfmt.GreenBits)) << pfmt.GreenShift) |
    ((b >> (8 - pfmt.BlueBits))  << pfmt.BlueShift);
}

void csTextureManagerLine::PrepareTextures ()
{
  // Create mipmaps for all textures
  int i;
  for (i = 0; i < textures.Length (); i++)
  {
    csTextureHandle *txt = textures.Get (i);
    txt->CreateMipmaps ();
  }

  // Remap all textures according to the new colormap.
  for (i = 0; i < textures.Length (); i++)
    ((csTextureHandleLine*)textures[i])->remap_texture (this);
}

csPtr<iTextureHandle> csTextureManagerLine::RegisterTexture (iImage* image,
  int flags)
{
  if (!image) return 0;

  csTextureHandleLine *txt = new csTextureHandleLine (this, image, flags);
  textures.Push (txt);
  return csPtr<iTextureHandle> (txt);
}

void csTextureManagerLine::UnregisterTexture (csTextureHandleLine* handle)
{
  int idx = textures.Find (handle);
  if (idx >= 0) textures.Delete (idx);
}

