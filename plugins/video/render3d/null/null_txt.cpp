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

#include "cssysdef.h"
#include "null_txt.h"
#include "csgfx/inv_cmap.h"
#include "csgfx/quantize.h"
#include "csutil/scanstr.h"
#include "csutil/debug.h"
#include "iutil/cfgfile.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "igraphic/image.h"
#include "igraphic/imgvec.h"

#define RESERVED_COLOR(c) ((c == 0) || (c == 255))

#define CLIP_RGB \
  if (r < 0) r = 0; else if (r > 255) r = 255; \
  if (g < 0) g = 0; else if (g > 255) g = 255; \
  if (b < 0) b = 0; else if (b > 255) b = 255;

/**
 * A nice observation about the properties of human eye:
 * Let's call the largest R or G or B component of a color "main".
 * If some other color component is much smaller than the main component,
 * we can change it in a large range without noting any change in
 * the color itself. Examples:
 * (128, 128, 128) - we note a change in color if we change any component
 * by 4 or more.
 * (192, 128, 128) - we should change of G or B components by 8 to note any
 * change in color.
 * (255, 128, 128) - we should change of G or B components by 16 to note any
 * change in color.
 * (255, 0, 0) - we can change any of G or B components by 32 and we
 * won't note any change.
 * Thus, we use this observation to create a palette that contains more
 * useful colors. We implement here a function to evaluate the "distance"
 * between two colors. tR,tG,tB define the color we are looking for (target);
 * sR, sG, sB define the color we're examining (source).
 */
static inline int rgb_dist (int tR, int tG, int tB, int sR, int sG, int sB)
{
  register int max = MAX (tR, tG);
  max = MAX (max, tB);

  sR -= tR; sG -= tG; sB -= tB;

  return R_COEF_SQ * sR * sR * (32 - ((max - tR) >> 3)) +
         G_COEF_SQ * sG * sG * (32 - ((max - tG) >> 3)) +
         B_COEF_SQ * sB * sB * (32 - ((max - tB) >> 3));
}

//--------------------------------------------------- csTextureHandleNull ---//

csTextureHandleNull::csTextureHandleNull (csTextureManagerNull *txtmgr,
  iImage *image, int flags) : csTextureHandle (txtmgr, image, flags)
{
  pal2glob = 0;
  (texman = txtmgr)->IncRef ();

  prepared = false;
  if (flags & CS_TEXTURE_3D)
    AdjustSizePo2 ();
}

csTextureHandleNull::~csTextureHandleNull ()
{
  texman->UnregisterTexture (this);
  texman->DecRef ();
  delete [] (uint8 *)pal2glob;
}

csTexture *csTextureHandleNull::NewTexture (iImage *Image, bool /*ismipmap*/)
{
  return new csTextureNull (this, Image);
}

void csTextureHandleNull::ComputeMeanColor ()
{
  int i;
  // Compute a common palette for all three mipmaps
  csColorQuantizer quant;
  quant.Begin ();

  csRGBpixel *tc = transp ? &transp_color : 0;

  for (i = 0; i < 4; i++)
    if (tex [i])
    {
      csTextureNull *t = (csTextureNull *)tex [i];
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
      csTextureNull *t = (csTextureNull *)tex [i];
      if (!t->image) break;
      if (flags & CS_TEXTURE_DITHER)
        quant.RemapDither ((csRGBpixel *)t->image->GetImageData (),
          t->get_size (), t->get_width (), pal, palette_size, t->bitmap, tc);
      else
        quant.Remap ((csRGBpixel *)t->image->GetImageData (),
          t->get_size (), t->bitmap, tc);

      // Get the alpha map for the texture, if present
      if (t->image->GetFormat () & CS_IMGFMT_ALPHA)
      {
        csRGBpixel *srcimg = (csRGBpixel *)t->image->GetImageData ();
        size_t imgsize = t->get_size ();
        uint8 *dstalpha = t->alphamap = new uint8 [imgsize];
        // In 8- and 16-bit modes we limit the alpha to 5 bits (32 values)
        // This is related to the internal implementation of alphamap
        // routine and is quite enough for 5-5-5 and 5-6-5 modes.
        if (texman->pfmt.PixelBytes != 4)
          while (imgsize--)
            *dstalpha++ = srcimg++->alpha >> 3;
        else
          while (imgsize--)
            *dstalpha++ = srcimg++->alpha;
      }

      // Very well, we don't need the iImage anymore, so free it
      DG_UNLINK (t, t->image);
      t->image->DecRef ();
      t->image = 0;
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

void csTextureHandleNull::remap_texture (csTextureManager *texman)
{
  int i;
  csTextureManagerNull *txm = (csTextureManagerNull *)texman;
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

void csTextureHandleNull::PrepareInt ()
{
  if (prepared) return;
  prepared = true;
  CreateMipmaps ();
  remap_texture (texman);
}

//----------------------------------------------- csTextureManagerNull ---//

csTextureManagerNull::csTextureManagerNull (iObjectRegistry *object_reg,
  iGraphics2D *iG2D, iConfigFile *config) : csTextureManager (object_reg, iG2D)
{
  read_config (config);
  G2D = iG2D;
}

csTextureManagerNull::~csTextureManagerNull ()
{
  Clear ();
}

void csTextureManagerNull::SetPixelFormat (csPixelFormat &PixelFormat)
{
  pfmt = PixelFormat;
}

void csTextureManagerNull::read_config (iConfigFile *config)
{
  csTextureManager::read_config (config);
}

void csTextureManagerNull::Clear ()
{
  csTextureManager::Clear ();
}

uint32 csTextureManagerNull::encode_rgb (int r, int g, int b)
{
  return
    ((r >> (8 - pfmt.RedBits))   << pfmt.RedShift) |
    ((g >> (8 - pfmt.GreenBits)) << pfmt.GreenShift) |
    ((b >> (8 - pfmt.BlueBits))  << pfmt.BlueShift);
}

csPtr<iTextureHandle> csTextureManagerNull::RegisterTexture (iImage* image,
  int flags)
{
  if (!image) return 0;

  csTextureHandleNull *txt = new csTextureHandleNull (this, image, flags);
  textures.Push (txt);
  return csPtr<iTextureHandle> (txt);
}

void csTextureManagerNull::UnregisterTexture (csTextureHandleNull* handle)
{
  size_t idx = textures.Find (handle);
  if (idx != csArrayItemNotFound) textures.DeleteIndexFast (idx);
}

csPtr<iSuperLightmap> csTextureManagerNull::CreateSuperLightmap (int w, int h)
{
  // @@@ implement a "NullRendererLightmap"
  return 0;
}
  
void csTextureManagerNull::GetMaxTextureSize (int& w, int& h, int& aspect)
{
  w = h = 2048;
  aspect = 32768;
}

void csTextureManagerNull::GetLightmapRendererCoords (
  int slmWidth, int slmHeight, int lm_x1, int lm_y1, int lm_x2, int lm_y2,
  float& lm_u1, float& lm_v1, float &lm_u2, float& lm_v2)
{
  lm_u1 = lm_x1;
  lm_v1 = lm_y1;
  lm_u2 = lm_x2;
  lm_v2 = lm_y2;
}
