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
#include "soft_txt.h"
#include "csgfx/inv_cmap.h"
#include "csgfx/quantize.h"
#include "csutil/scanstr.h"
#include "csutil/debug.h"
#include "iutil/cfgfile.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "igraphic/image.h"
#include "ivaria/reporter.h"
#include "qint.h"
#include "csgfx/memimage.h"
#include "csgfx/xorpat.h"
#include "soft_g3d.h"

#define RESERVED_COLOR(c) ((c == 0) || (c == 255))

#define CLIP_RGB \
  if (r < 0) r = 0; else if (r > 255) r = 255; \
  if (g < 0) g = 0; else if (g > 255) g = 255; \
  if (b < 0) b = 0; else if (b > 255) b = 255;

/**
 * A nice observation about the properties of the human eye:
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

//----------------------------------------------- csTextureHandleSoftware ---//

csTextureHandleSoftware::csTextureHandleSoftware (
	csTextureManagerSoftware *texman, iImage *image, int flags)
	: csTextureHandle (image, flags)
{
  pal2glob = NULL;
  if (flags & CS_TEXTURE_3D)
    AdjustSizePo2 ();
  (this->texman = texman)->IncRef ();
  use_332_palette = false;
  update_number = -1;
}

csTextureHandleSoftware::~csTextureHandleSoftware ()
{
  texman->UnregisterTexture (this);
  texman->DecRef ();
  delete [] (uint8 *)pal2glob;
}

void csTextureHandleSoftware::Setup332Palette ()
{
  if (use_332_palette) return;

  use_332_palette = true;
  // First remap the textures to use standard 3:3:2 palette.
  int i;
  for (i = 0 ; i < 4 ; i++)
  {
    if (tex [i])
    {
      csTextureSoftware *t = (csTextureSoftware *)tex [i];
      if (!t->bitmap) break;
      int size = t->get_width () * t->get_height ();
      uint8* bm = t->bitmap;
      while (size > 0)
      {
        const csRGBpixel& p = palette[*bm];
	*bm++ = ((p.red >> 5) << 5) |
	        ((p.green >> 5) << 2) |
	        (p.green >> 6);
        size--;
      }
    }
  }

  palette_size = 256;
  delete [] (uint8 *)pal2glob;

  // Remap the pal2glob array.
  if (texman->pfmt.PixelBytes == 2)
  {
    pal2glob = new uint8 [palette_size * sizeof (uint16)];
    uint16* p2g = (uint16*)pal2glob;
    for (i = 0 ; i < 256 ; i++)
    {
      int r = (i>>5) << 5;
      int g = ((i>>2) & 0x7) << 5;
      int b = (i & 0x3) << 6;
      *p2g++ = texman->encode_rgb (r, g, b);
    }
  }
  else
  {
    pal2glob = new uint8 [palette_size * sizeof (uint32)];
    uint32* p2g = (uint32*)pal2glob;
    for (i = 0 ; i < 256 ; i++)
    {
      int r = (i>>5) << 5;
      int g = ((i>>2) & 0x7) << 5;
      int b = (i & 0x3) << 6;
      *p2g++ = texman->encode_rgb (r, g, b);
    }
  }
  // Remap the palette itself.
  for (i = 0 ; i < 256 ; i++)
  {
    int r = (i>>5) << 5;
    int g = ((i>>2) & 0x7) << 5;
    int b = (i & 0x3) << 6;
    palette[i].red = r;
    palette[i].green = g;
    palette[i].blue = b;
  }
}

csTexture *csTextureHandleSoftware::NewTexture (iImage *newImage,
	bool ismipmap)
{
  csRef<iImage> Image;
  if (ismipmap && texman->sharpen_mipmaps)
  { 
    csRGBpixel *tc = transp ? &transp_color : (csRGBpixel *)NULL;
    Image = newImage->Sharpen (tc, texman->sharpen_mipmaps);
  }
  else
    Image = newImage;

  return new csTextureSoftware (this, Image);
}

void csTextureHandleSoftware::ComputeMeanColor ()
{
  int i;

  bool destroy_image = true;

  // Compute a common palette for all three mipmaps
  csColorQuantizer quant;
  quant.Begin ();

  csRGBpixel *tc = transp ? &transp_color : 0;

  for (i = 0; i < 4; i++)
    if (tex [i])
    {
      csTextureSoftware *t = (csTextureSoftware *)tex [i];
      if (!t->image) break;
      quant.Count ((csRGBpixel *)t->image->GetImageData (),
        t->get_size (), tc);
    }
#if 0
  // This code makes sure the palette also has sufficient
  // information to encode a standard 3:3:2 palette.
  // This is useful when the texture is used as a procedural
  // texture.
  csRGBpixel bias[256];
  for (i = 0 ; i < 256 ; i++)
  {
    int r = ((i & 0xe0) >> 5) << (8-3);
    int g = ((i & 0x1c) >> 2) << (8-3);
    int b = ((i & 0x03) >> 0) << (8-2);
    bias[i].red = r;
    bias[i].green = g;
    bias[i].blue = b;
  }
  quant.Count (bias, 256, NULL);
#endif

  csRGBpixel *pal = palette;
  palette_size = 256;
  quant.Palette (pal, palette_size, tc);

  for (i = 0; i < 4; i++)
    if (tex [i])
    {
      csTextureSoftware *t = (csTextureSoftware *)tex [i];
      if (!t->image) break;
      uint8* bmap = t->bitmap; // Temp assignment to pacify BeOS compiler.
      if (texman->dither_textures || (flags & CS_TEXTURE_DITHER))
        quant.RemapDither ((csRGBpixel *)t->image->GetImageData (),
          t->get_size (), t->get_width (), pal, palette_size, bmap, tc);
      else
        quant.Remap ((csRGBpixel *)t->image->GetImageData (),
          t->get_size (), bmap, tc);
      t->bitmap = bmap;

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

      if (destroy_image)
      {
	// Very well, we don't need the iImage anymore, so free it
	DG_UNLINK (t, t->image);
	t->image->DecRef ();
	t->image = NULL;
      }
    }

  quant.End ();

  SetupFromPalette ();
}

void csTextureHandleSoftware::SetupFromPalette ()
{
  int i;
  // Compute the mean color from the palette
  csRGBpixel *src = palette;
  unsigned r = 0, g = 0, b = 0;
  for (i = palette_size; i > 0; i--)
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

void csTextureHandleSoftware::remap_texture ()
{
  int i;
  CS_ASSERT (texman);
  switch (texman->pfmt.PixelBytes)
  {
    case 2:
      delete [] (uint16 *)pal2glob;
      pal2glob = new uint8 [palette_size * sizeof (uint16)];
      for (i = 0; i < palette_size; i++)
        ((uint16 *)pal2glob) [i] = texman->encode_rgb (palette [i].red,
          palette [i].green, palette [i].blue);
      break;
    case 4:
      delete [] (uint32 *)pal2glob;
      pal2glob = new uint8 [palette_size * sizeof (uint32)];
      for (i = 0; i < palette_size; i++)
      {
        ((uint32 *)pal2glob) [i] = texman->encode_rgb (palette [i].red,
          palette [i].green, palette [i].blue);
      }
      break;
  }
}

void csTextureHandleSoftware::Prepare ()
{
  CreateMipmaps ();
  remap_texture ();
}

//----------------------------------------------- csTextureManagerSoftware ---//

static uint8 *GenLightmapTable (int bits)
{
  uint8 *table = new uint8 [64 * 256];
  uint8 *dst = table;
  uint8 maxv = (1 << bits) - 1;
  int rshf = (13 - bits);
  int i, j;
  for (i = 0; i < 64; i++)
  {
    *dst++ = 0;
    for (j = 1; j < 256; j++)
    {
      int x = (i * j) >> rshf;
      *dst++ = (x > maxv) ? maxv : (x?x:1) ;
    }
  }
  return table;
}

csTextureManagerSoftware::csTextureManagerSoftware (
  iObjectRegistry *object_reg,
  csGraphics3DSoftwareCommon *iG3D, iConfigFile *config)
  : csTextureManager (object_reg, iG3D->GetDriver2D())
{
  ResetPalette ();
  read_config (config);
  G3D = iG3D;
}

void csTextureManagerSoftware::SetPixelFormat (csPixelFormat &PixelFormat)
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

void csTextureManagerSoftware::read_config (iConfigFile *config)
{
  csTextureManager::read_config (config);
  dither_textures = config->GetBool
        ("Video.Software.TextureManager.DitherTextures", true);
  sharpen_mipmaps = config->GetInt
        ("Video.Software.TextureManager.SharpenMipmaps", 0);
}

csTextureManagerSoftware::~csTextureManagerSoftware ()
{
  delete [] lightmap_tables [0];
  if (lightmap_tables [1] != lightmap_tables [0])
    delete [] lightmap_tables [1];
  if (lightmap_tables [2] != lightmap_tables [1]
   && lightmap_tables [2] != lightmap_tables [0])
    delete [] lightmap_tables [2];
  Clear ();
}

void csTextureManagerSoftware::Clear ()
{
  csTextureManager::Clear();
}

uint32 csTextureManagerSoftware::encode_rgb (int r, int g, int b)
{
  return
    ((r >> (8 - pfmt.RedBits))   << pfmt.RedShift) |
    ((g >> (8 - pfmt.GreenBits)) << pfmt.GreenShift) |
    ((b >> (8 - pfmt.BlueBits))  << pfmt.BlueShift);
}

int csTextureManagerSoftware::FindRGB (int r, int g, int b)
{
  CLIP_RGB;
  return encode_rgb (r, g, b);
}

void csTextureManagerSoftware::PrepareTextures ()
{
  if (verbose)
    G3D->Report (CS_REPORTER_SEVERITY_NOTIFY,
    	"Preparing textures (%s dithering)...",
	dither_textures ? "with" : "no");

  if (verbose)
    G3D->Report (CS_REPORTER_SEVERITY_NOTIFY, "  Creating texture mipmaps...");

  int i;

  // Create mipmaps for all textures
  for (i = 0; i < textures.Length (); i++)
  {
    csTextureHandle *txt = textures.Get (i);
    txt->CreateMipmaps ();
  }

  // Remap all textures according to the new colormap.
  for (i = 0; i < textures.Length (); i++)
  {
    csTextureHandleSoftware* txt = (csTextureHandleSoftware*)textures.Get (i);
    txt->remap_texture ();
  }
}

csPtr<iTextureHandle> csTextureManagerSoftware::RegisterTexture (iImage* image,
  int flags)
{
  if (!image)
  {
    G3D->Report(CS_REPORTER_SEVERITY_BUG,
      "BAAAD!!! csTextureManagerSoftware::RegisterTexture with NULL image!");

    csRef<iImage> im (csCreateXORPatternImage(32, 32, 5));
    image = im;
    im->IncRef ();	// Avoid smart pointer cleanup. @@@ UGLY
  }

  csTextureHandleSoftware *txt = new csTextureHandleSoftware (
  	this, image, flags);
  textures.Push (txt);
  return csPtr<iTextureHandle> (txt);
}

void csTextureManagerSoftware::UnregisterTexture (
		csTextureHandleSoftware* handle)
{
  int idx = textures.Find (handle);
  if (idx >= 0) textures.Delete (idx);
}


