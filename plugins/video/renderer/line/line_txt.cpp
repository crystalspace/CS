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

//------------------------------------------------------------- csColorMapLine ---//

int csColorMapLine::find_rgb (int r, int g, int b, int *d)
{
  CLIP_RGB;

  int i, min, mindist;
  mindist = 0x7fffffff;
  min = -1;
  // Color 0 is reserved for transparency
  for (i = 1; i < 256; i++)
    if (alloc [i])
    {
      register int dist = rgb_dist (r, g, b,
        palette [i].red, palette [i].green, palette [i].blue);
      if (dist < mindist) { mindist = dist; min = i; if (!dist) break; }
    }
  if (d) *d = mindist;
  return min;
}

int csColorMapLine::alloc_rgb (int r, int g, int b, int dist)
{
  CLIP_RGB;

  int d, j, i = find_rgb (r, g, b, &d);
  if (i == -1 || d > dist)
  {
    for (j = 0; j < 256; j++)
      if (!alloc [j])
      {
        alloc[j] = true;
        palette [j].red = r;
        palette [j].green = g;
        palette [j].blue = b;
        return j;
      }
    return i; // We couldn't allocate a new color, return best fit
  }
  else
    return i;
}

int csColorMapLine::FreeEntries ()
{
  int i, colors = 0;
  for (i = 0; i < 256; i++)
    if (!alloc [i])
      colors++;
  return colors;
}

//-------------------------------------------------------- csTextureHandleLine ---//

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

csTexture *csTextureHandleLine::NewTexture (iImage *Image)
{
  return new csTextureLine (this, Image);
}

void csTextureHandleLine::ComputeMeanColor ()
{
  int i;

  // Compute a common palette for all three mipmaps
  csQuantizeBegin ();

  csRGBpixel *tc = transp ? &transp_color : 0;

  for (i = 0; i < 4; i++)
    if (tex [i])
    {
      csTextureLine *t = (csTextureLine *)tex [i];
      if (!t->image) break;
      csQuantizeCount ((csRGBpixel *)t->image->GetImageData (),
        t->get_size (), tc);
    }

  csRGBpixel *pal = palette;
  palette_size = 256;
  csQuantizePalette (pal, palette_size, tc);

  for (i = 0; i < 4; i++)
    if (tex [i])
    {
      csTextureLine *t = (csTextureLine *)tex [i];
      if (!t->image) break;

      csQuantizeRemap ((csRGBpixel *)t->image->GetImageData (),
        t->get_size (), t->bitmap, tc);

      // Very well. Now we don'tex need the iImage anymore, so free it
      t->image->DecRef ();
      t->image = NULL;
    }

  csQuantizeEnd ();

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
    case 1:
      delete [] (UByte *)pal2glob;
      pal2glob = new UByte [palette_size];
      for (i = 0; i < palette_size; i++)
        ((UByte *)pal2glob) [i] = txm->cmap.find_rgb (palette [i].red,
          palette [i].green, palette [i].blue);
      break;
    case 2:
      delete [] (UShort *)pal2glob;
      pal2glob = new UShort [palette_size];
      for (i = 0; i < palette_size; i++)
        ((UShort *)pal2glob) [i] = txm->encode_rgb (palette [i].red,
          palette [i].green, palette [i].blue);
      break;
    case 4:
      delete [] (ULong *)pal2glob;
      pal2glob = new ULong [palette_size];
      for (i = 0; i < palette_size; i++)
        ((ULong *)pal2glob) [i] = txm->encode_rgb (palette [i].red,
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

static UByte *GenLightmapTable (int bits)
{
  UByte *table = new UByte [64 * 256];
  UByte *dst = table;
  UByte maxv = (1 << bits) - 1;
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
  alpha_tables = NULL;
  ResetPalette ();
  read_config (config);
  G2D = iG2D;
  inv_cmap = NULL;
}

void csTextureManagerLine::SetPixelFormat (csPixelFormat &PixelFormat)
{
  pfmt = PixelFormat;

  truecolor = (pfmt.PalEntries == 0);

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
  prefered_dist = config->GetInt ("Video.Line.TextureManager.RGBDist", PREFERED_DIST);
  uniform_bias = config->GetInt ("Video.Line.TextureManager.UniformBias", 75);
  if (uniform_bias > 100) uniform_bias = 100;
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
  delete alpha_tables; alpha_tables = NULL;
}

ULong csTextureManagerLine::encode_rgb (int r, int g, int b)
{
  return
    ((r >> (8 - pfmt.RedBits))   << pfmt.RedShift) |
    ((g >> (8 - pfmt.GreenBits)) << pfmt.GreenShift) |
    ((b >> (8 - pfmt.BlueBits))  << pfmt.BlueShift);
}

int csTextureManagerLine::find_rgb (int r, int g, int b)
{
  CLIP_RGB;
  return inv_cmap [encode_rgb (r, g, b)];
}

int csTextureManagerLine::FindRGB (int r, int g, int b)
{
  CLIP_RGB;
  if (truecolor)
    return encode_rgb (r, g, b);
  else
    return inv_cmap [encode_rgb (r, g, b)];
}

void csTextureManagerLine::create_inv_cmap ()
{
  // We create a inverse colormap for finding fast the nearest palette index
  // given any R,G,B value. Usually this is done by scanning the entire palette
  // which is way too slow for realtime. Because of this we use an table that
  // that takes on input an 5:6:5 encoded value (like in 16-bit truecolor modes)
  // and on output we get the palette index.

  if (pfmt.PixelBytes != 1)
    return;

  // Greg Ewing, 12 Oct 1998
  delete inv_cmap;
  inv_cmap = NULL; // let the routine allocate the array itself
  csInverseColormap (256, &cmap [0], RGB2PAL_BITS_R, RGB2PAL_BITS_G,
    RGB2PAL_BITS_B, inv_cmap);

  // Color number 0 is reserved for transparency
  inv_cmap [encode_rgb (cmap [0].red, cmap [0].green, cmap [0].blue)] =
    cmap.find_rgb (cmap [0].red, cmap [0].green, cmap [0].blue);
}

void csTextureManagerLine::create_alpha_tables ()
{
  if (pfmt.PixelBytes != 1)
    return;

  if (!alpha_tables)
    alpha_tables = new csAlphaTables ();

  UByte *map50 = alpha_tables->alpha_map50;
  UByte *map25 = alpha_tables->alpha_map25;

  int i, j, r, g, b;
  for (i = 0 ; i < 256 ; i++)
    for (j = 0 ; j < 256 ; j++)
    {
      r = (cmap [i].red   + cmap [j].red  ) / 2;
      g = (cmap [i].green + cmap [j].green) / 2;
      b = (cmap [i].blue  + cmap [j].blue ) / 2;
      *map50++ = find_rgb (r, g, b);

      r = (cmap [i].red   + cmap [j].red   * 3) / 4;
      g = (cmap [i].green + cmap [j].green * 3) / 4;
      b = (cmap [i].blue  + cmap [j].blue  * 3) / 4;
      *map25++ = find_rgb (r, g, b);
    }
}

void csTextureManagerLine::compute_palette ()
{
  int i;

  if (truecolor) return;

  // Allocate first 6*6*4=144 colors in a uniformly-distributed fashion
  // since we'll get lighted/dimmed/colored textures more often
  // than original pixel values, thus we should be prepared for this.
  int _r, _g, _b;
  for (_r = 0; _r < 6; _r++)
    for (_g = 0; _g < 6; _g++)
      for (_b = 0; _b < 4; _b++)
        cmap.alloc_rgb (20 + _r * 42, 20 + _g * 42, 30 + _b * 50, prefered_dist);

  // Compute a common color histogram for all textures
  csQuantizeBegin ();

  for (i = textures.Length () - 1; i >= 0; i--)
  {
    csTextureHandleLine *txt = (csTextureHandleLine *)textures [i];
    csRGBpixel *colormap = txt->GetColorMap ();
    int colormapsize = txt->GetColorMapSize ();
    if (txt->GetKeyColor ())
      colormap++, colormapsize--;
    csQuantizeCount (colormap, colormapsize);
  }

  // Introduce the uniform colormap bias into the histogram
  csRGBpixel new_cmap [256];
  int colors = 0;
  for (i = 0; i < 256; i++)
    if (!locked [i] && cmap.alloc [i])
      new_cmap [colors++] = cmap [i];

  csQuantizeBias (new_cmap, colors, uniform_bias);

  // Now compute the actual colormap
  colors = 0;
  for (i = 0; i < 256; i++)
    if (!locked [i]) colors++;
  csRGBpixel *cmap_p = new_cmap;
  csQuantizePalette (cmap_p, colors);

  // Finally, put the computed colors back into the colormap
  int outci = 0;
  for (i = 0; i < colors; i++)
  {
    while (locked [outci]) outci++;
    cmap [outci++] = new_cmap [i];
  }

  csQuantizeEnd ();

  // Now create the inverse colormap
  create_inv_cmap ();

  // Also we need the alpha tables
  create_alpha_tables ();

  palette_ok = true;
}

void csTextureManagerLine::PrepareTextures ()
{
  // Drop all "color allocated" flags to locked colors.
  // We won't Clear the palette as we don't care about unused colors anyway.
  // The locked colors will stay the same.
  memcpy(cmap.alloc, locked, sizeof(locked));

  // Create mipmaps for all textures
  int i;
  for (i = 0; i < textures.Length (); i++)
  {
    csTextureHandle *txt = textures.Get (i);
    txt->CreateMipmaps ();
  }

  // The only thing left to do is to compute the palette
  // Everything other has been done during textures registration
  compute_palette ();

  // Remap all textures according to the new colormap.
  for (i = 0; i < textures.Length (); i++)
    ((csTextureHandleLine*)textures[i])->remap_texture (this);
}

iTextureHandle *csTextureManagerLine::RegisterTexture (iImage* image,
  int flags)
{
  if (!image) return NULL;

  csTextureHandleLine *txt = new csTextureHandleLine (this, image, flags);
  textures.Push (txt);
  return txt;
}

void csTextureManagerLine::UnregisterTexture (csTextureHandleLine* handle)
{
  int idx = textures.Find (handle);
  if (idx >= 0) textures.Delete (idx);
}

void csTextureManagerLine::ResetPalette ()
{
  memset (&locked, 0, sizeof (locked));
  locked [0] = true;
  locked [255] = true;
  cmap [0] = csRGBcolor (0, 0, 0);
  cmap [255] = csRGBcolor (255, 255, 255);
  memcpy(cmap.alloc, locked, sizeof(locked));
  palette_ok = false;
}

void csTextureManagerLine::ReserveColor (int r, int g, int b)
{
  if (!pfmt.PalEntries) return;
  int color = cmap.alloc_rgb (r, g, b, 0);
  locked [color] = true;
}

void csTextureManagerLine::SetPalette ()
{
  if (!truecolor && !palette_ok)
    compute_palette ();

  int i;
  for (i = 0; i < 256; i++)
    G2D->SetRGB (i, cmap [i].red, cmap [i].green, cmap [i].blue);

  iEventQueue* q = CS_QUERY_REGISTRY(object_reg, iEventQueue);
  if (q != 0)
    q->GetEventOutlet()->ImmediateBroadcast (cscmdPaletteChanged, this);
}
