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
#include "isys/event.h"
#include "isys/system.h"
#include "igraphic/image.h"
#include "iutil/cfgfile.h"
#include "ivaria/reporter.h"
#include "qint.h"
#include "protex3d.h"

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

//------------------------------------------------------------- csColorMap ---//

int csColorMap::find_rgb (int r, int g, int b, int *d)
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

int csColorMap::alloc_rgb (int r, int g, int b, int dist)
{
  CLIP_RGB;

  int d, i = find_rgb (r, g, b, &d);
  if (i == -1 || d > dist)
  {
    for (int j = 0; j < 256; j++)
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

int csColorMap::FreeEntries ()
{
  int colors = 0;
  for (int i = 0; i < 256; i++)
    if (!alloc [i])
      colors++;
  return colors;
}

//---------------------------------------------------- csTextureSoftProc -----//

csTextureSoftwareProc::~csTextureSoftwareProc ()
{ 
  if (texG3D) texG3D->DecRef ();
  if (image) image->DecRef ();
}

//---------------------------------------------------- csTextureHandleSoftware ---//

csTextureHandleSoftware::csTextureHandleSoftware (csTextureManagerSoftware *texman, 
  iImage *image, int flags) : csTextureHandle (image, flags)
{
  pal2glob = NULL;
  pal2glob8 = NULL;
  if (flags & CS_TEXTURE_3D)
    AdjustSizePo2 ();
  orig_palette = NULL;
  (this->texman = texman)->IncRef ();
}

csTextureHandleSoftware::~csTextureHandleSoftware ()
{
  texman->UnregisterTexture (this);
  texman->DecRef ();
  delete [] (UByte *)pal2glob;
  delete [] pal2glob8;
  delete [] orig_palette;
}

csTexture *csTextureHandleSoftware::NewTexture (iImage *Image)
{
  if ((flags & CS_TEXTURE_PROC) == CS_TEXTURE_PROC)
    return new csTextureSoftwareProc (this, Image);
  else
    return new csTextureSoftware (this, Image);
}

void csTextureHandleSoftware::ApplyGamma (UByte *GammaTable)
{
  if (!orig_palette)
    return;

  UByte *src = orig_palette;
  csRGBpixel *dst = palette;
  for (int i = palette_size; i > 0; i--)
  {
    dst->red   = GammaTable [*src++];
    dst->green = GammaTable [*src++];
    dst->blue  = GammaTable [*src++];
    dst++;
  }
} 

void csTextureHandleSoftware::ComputeMeanColor ()
{
  int i;

  // If the procedural textures are not running fully at 8bit and so do not have 
  // their own texture manager the procedural textures update the iImage, so we 
  // avoid destroying them here.
  bool destroy_image = ((flags & CS_TEXTURE_PROC) != CS_TEXTURE_PROC);

  // Compute a common palette for all three mipmaps
  csQuantizeBegin ();

  csRGBpixel *tc = transp ? &transp_color : 0;

  for (i = 0; i < 4; i++)
    if (tex [i])
    {
      csTextureSoftware *t = (csTextureSoftware *)tex [i];
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
      csTextureSoftware *t = (csTextureSoftware *)tex [i];
      if (!t->image) break;
      UByte* bmap = t->bitmap; // Temp assignment to pacify BeOS compiler.
      if (texman->dither_textures || (flags & CS_TEXTURE_DITHER))
        csQuantizeRemapDither ((csRGBpixel *)t->image->GetImageData (),
          t->get_size (), t->get_width (), pal, palette_size, bmap, tc);
      else
        csQuantizeRemap ((csRGBpixel *)t->image->GetImageData (),
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
	t->image->DecRef ();
	t->image = NULL;
      }
    }

  csQuantizeEnd ();

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

  // Now allocate the orig palette and compress the texture palette there
  src = palette;
  delete [] orig_palette;
  orig_palette = new UByte [palette_size * 3];
  UByte *dst = orig_palette;
  for (i = palette_size; i > 0; i--)
  {
    *dst++ = src->red;
    *dst++ = src->green;
    *dst++ = src->blue;
    src++;
  }
}

void csTextureHandleSoftware::GetOriginalColormap (csRGBpixel *oPalette, int &oCount)
{
  oCount = palette_size;
  UByte *src = orig_palette;
  csRGBpixel *dst = oPalette;
  for (int i = palette_size; i > 0; i--)
  {
    dst->red   = *src++;
    dst->green = *src++;
    dst->blue  = *src++;
    dst++;
  }
}

void csTextureHandleSoftware::remap_texture ()
{
  int i;
  CS_ASSERT (texman);
  ApplyGamma (texman->GammaTable);
  switch (texman->pfmt.PixelBytes)
  {
    case 1:
      delete [] (UByte *)pal2glob;
      pal2glob = new UByte [palette_size * sizeof (UByte)];
      delete [] pal2glob8;
      pal2glob8 = new uint16 [palette_size];
      for (i = 0; i < palette_size; i++)
      {
        ((UByte *)pal2glob) [i] = texman->cmap.find_rgb (palette [i].red,
          palette [i].green, palette [i].blue);
        pal2glob8 [i] = texman->encode_rgb (palette [i].red,
          palette [i].green, palette [i].blue);
      }
      break;
    case 2:
      delete [] (UShort *)pal2glob;
      pal2glob = new UByte [palette_size * sizeof (UShort)];
      for (i = 0; i < palette_size; i++)
        ((UShort *)pal2glob) [i] = texman->encode_rgb (palette [i].red,
          palette [i].green, palette [i].blue);
      break;
    case 4:
      delete [] (ULong *)pal2glob;
      pal2glob = new UByte [palette_size * sizeof (ULong)];
      for (i = 0; i < palette_size; i++)
      {
        ((ULong *)pal2glob) [i] = texman->encode_rgb (palette [i].red,
          palette [i].green, palette [i].blue);
      }
      break;
  }
}

void csTextureHandleSoftware::RemapProcToGlobalPalette (csTextureManagerSoftware *txtmgr)
{
  // This instance of the texture manager is fully 8bit and is either managing 
  // all the textures in the system or just a set of proc textures.
  csTextureSoftwareProc* t = (csTextureSoftwareProc*)tex[0];

  if (!Scan.inv_cmap || t->proc_ok)// || !t->texG3D)
    return;

  // Copy the global palette to the texture palette
  memcpy (palette, &txtmgr->cmap.palette [0], sizeof (csRGBpixel) * 256);
  // Remap image according to new palette if persistent
  if ((flags & CS_TEXTURE_PROC_PERSISTENT) == CS_TEXTURE_PROC_PERSISTENT)
  {
    csRGBpixel *src = (csRGBpixel *)t->image->GetImageData ();
    uint8 *dst = t->bitmap;
    for (int i = 0; i < t->get_size (); i++, dst++, src++)
      *dst = Scan.inv_cmap [txtmgr->encode_rgb(src->red, src->green, src->blue)];
  }
  // we have really finished with the image now
  t->image->DecRef ();
  t->image = NULL;

  // If a proc texture was initialised without an image the palette size will be
  // very small, therefore force it to the max, so the whole new palette is 
  // recognised.
  palette_size = 256;
  SetupFromPalette ();
  // Update remapping tables according to global palette
  remap_texture ();
  t->proc_ok = true;
}

void csTextureHandleSoftware::ProcTextureSync ()
{

}

void csTextureHandleSoftware::ReprepareProcTexture ()
{
  ComputeMeanColor ();
  remap_texture ();
}

iGraphics3D *csTextureHandleSoftware::GetProcTextureInterface ()
{
  if ((flags & CS_TEXTURE_PROC) != CS_TEXTURE_PROC)
    return NULL;

  if (!((csTextureSoftwareProc*)tex[0])->texG3D)
  {
    csSoftProcTexture3D *stex = new csSoftProcTexture3D (texman->G3D);
    void *imd = ((csTextureSoftware*) tex[0])->image ? 
      ((csTextureSoftware*) tex[0])->image->GetImageData () : NULL;
    if (!stex->Prepare (texman, this, imd,
			((csTextureSoftware*) tex[0])->bitmap))
      delete stex;
    else
    {
      ((csTextureSoftwareProc*)tex[0])->texG3D = stex;
      if (((flags & CS_TEXTURE_PROC_ALONE_HINT) == 
	   CS_TEXTURE_PROC_ALONE_HINT) && 
	  !texman->first_8bit_proc_tex &&
	  (texman->pfmt.PixelBytes != 1))
      {
	texman->first_8bit_proc_tex = (csGraphics3DSoftwareCommon*)stex;
	// IncRef so that if this texture is destroyed individually, it doesnt
	// destroy the shared resources
	stex->IncRef ();
      }
    }
  }
  return ((csTextureSoftwareProc*)tex[0])->texG3D;
}

void csTextureHandleSoftware::Prepare ()
{
  CreateMipmaps ();
  if ((flags & CS_TEXTURE_PROC)
   && (texman->pfmt.PixelBytes == 1))
    RemapProcToGlobalPalette (texman);
  else
    remap_texture ();
  if (texman->main_txtmgr)
    texman->main_txtmgr->Reprepare8BitProcs ();
}

//----------------------------------------------- csTextureManagerSoftware ---//

static UByte *GenLightmapTable (int bits)
{
  UByte *table = new UByte [64 * 256];
  UByte *dst = table;
  UByte maxv = (1 << bits) - 1;
  int rshf = (13 - bits);
  for (int i = 0; i < 64; i++)
    for (int j = 0; j < 256; j++)
    {
      int x = (i * j) >> rshf;
      *dst++ = (x > maxv) ? maxv : x;
    }
  return table;
}

csTextureManagerSoftware::csTextureManagerSoftware (iObjectRegistry *object_reg,
  csGraphics3DSoftwareCommon *iG3D, iConfigFile *config) 
  : csTextureManager (object_reg, iG3D->GetDriver2D())
{
  alpha_tables = NULL;
  ResetPalette ();
  read_config (config);
  G3D = iG3D;
  Scan.inv_cmap = NULL;
  Scan.GlobalCMap = NULL;

  first_8bit_proc_tex = NULL;
  main_txtmgr = NULL;
  proc_txtmgr = NULL;
}

void csTextureManagerSoftware::SetGamma (float iGamma)
{
  Gamma = iGamma;
  if (Gamma < EPSILON)
    Gamma = EPSILON;

  int i;
  float inv_Gamma = 1 / Gamma;
  for (i = 0; i < 256; i++)
    GammaTable [i] = QRound (255 * pow (i / 255.0, inv_Gamma));
  // Remap all textures according to the new colormap.
  if (truecolor)
    for (i = 0; i < textures.Length (); i++)
      ((csTextureHandleSoftware*)textures.Get (i))->remap_texture ();
  else if (textures.Length ())
    SetPalette ();
}

void csTextureManagerSoftware::SetPixelFormat (csPixelFormat &PixelFormat)
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

  if ((pfmt.PixelBytes == 1) && !Scan.GlobalCMap)
    Scan.GlobalCMap = new uint16 [256];
}

void csTextureManagerSoftware::read_config (iConfigFile *config)
{
  csTextureManager::read_config (config);
  prefered_dist = config->GetInt
        ("Video.Software.TextureManager.RGBDist", PREFERED_DIST);
  uniform_bias = config->GetInt
        ("Video.Software.TextureManager.UniformBias", 75);
  if (uniform_bias > 100) uniform_bias = 100;
  dither_textures = config->GetBool
        ("Video.Software.TextureManager.DitherTextures", true);
}

csTextureManagerSoftware::~csTextureManagerSoftware ()
{
  SCF_DEC_REF (first_8bit_proc_tex); 
  if (main_txtmgr == NULL)
  {
    delete [] Scan.GlobalCMap;
    delete [] Scan.inv_cmap;
  }
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
  delete alpha_tables; alpha_tables = NULL;
}

ULong csTextureManagerSoftware::encode_rgb (int r, int g, int b)
{
  return
    ((r >> (8 - pfmt.RedBits))   << pfmt.RedShift) |
    ((g >> (8 - pfmt.GreenBits)) << pfmt.GreenShift) |
    ((b >> (8 - pfmt.BlueBits))  << pfmt.BlueShift);
}

int csTextureManagerSoftware::find_rgb (int r, int g, int b)
{
  CLIP_RGB;
  return Scan.inv_cmap [encode_rgb (r, g, b)];
}

int csTextureManagerSoftware::FindRGB (int r, int g, int b)
{
  CLIP_RGB;
  if (truecolor)
    return encode_rgb (r, g, b);
  else
    return Scan.inv_cmap ? Scan.inv_cmap [encode_rgb (r, g, b)] : 0;
}

void csTextureManagerSoftware::create_inv_cmap ()
{
  // We create a inverse colormap for finding fast the nearest palette index
  // given any R,G,B value. Usually this is done by scanning the entire palette
  // which is way too slow for realtime. Because of this we use an table that
  // that takes on input an 5:6:5 encoded value (like in 16-bit truecolor modes)
  // and on output we get the palette index.

  if (pfmt.PixelBytes != 1)
    return;

  if (verbose)
    G3D->Report (CS_REPORTER_SEVERITY_NOTIFY,
    	"  Computing inverse colormap...");

  // Greg Ewing, 12 Oct 1998
  delete [] Scan.inv_cmap;
  Scan.inv_cmap = NULL; // let the routine allocate the array itself
  csInverseColormap (256, &cmap [0], RGB2PAL_BITS_R, RGB2PAL_BITS_G,
    RGB2PAL_BITS_B, Scan.inv_cmap);

  // Color number 0 is reserved for transparency
  Scan.inv_cmap [encode_rgb (cmap [0].red, cmap [0].green, cmap [0].blue)] =
    cmap.find_rgb (cmap [0].red, cmap [0].green, cmap [0].blue);

  // Now we'll encode the palette into 16-bit values
  for (int i = 0; i < 256; i++)
    Scan.GlobalCMap [i] = encode_rgb (cmap [i].red, cmap [i].green, cmap [i].blue);
}

void csTextureManagerSoftware::create_alpha_tables ()
{
  if (pfmt.PixelBytes != 1)
    return;

  if (verbose)
    G3D->Report (CS_REPORTER_SEVERITY_NOTIFY, "  Computing alpha tables...");

  if (!alpha_tables)
    alpha_tables = new csAlphaTables ();

  UByte *map50 = alpha_tables->alpha_map50;
  UByte *map25 = alpha_tables->alpha_map25;

  for (int i = 0 ; i < 256 ; i++)
    for (int j = 0 ; j < 256 ; j++)
    {
      int r, g, b;
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

void csTextureManagerSoftware::compute_palette ()
{
  if (truecolor) return;

  if (verbose)
    G3D->Report (CS_REPORTER_SEVERITY_NOTIFY, "  Computing palette...");

  // Allocate first 6*6*4=144 colors in a uniformly-distributed fashion
  // since we'll get lighted/dimmed/colored textures more often
  // than original pixel values, thus we should be prepared for this.
  for (int _r = 0; _r < 6; _r++)
    for (int _g = 0; _g < 6; _g++)
      for (int _b = 0; _b < 4; _b++)
        cmap.alloc_rgb (20 + _r * 42, 20 + _g * 42, 30 + _b * 50, prefered_dist);

  // Compute a common color histogram for all textures
  csQuantizeBegin ();

  for (int t = textures.Length () - 1; t >= 0; t--)
  {
    csTextureHandleSoftware *txt = (csTextureHandleSoftware *)textures [t];
    if ((txt->GetFlags () & CS_TEXTURE_PROC) == CS_TEXTURE_PROC)
      continue;
    csRGBpixel colormap [256];
    int colormapsize;
    txt->GetOriginalColormap (colormap, colormapsize);
    csRGBpixel *cmap = colormap;
    if (txt->GetKeyColor ())
      cmap++, colormapsize--;
    csQuantizeCount (cmap, colormapsize);
  }

  // Introduce the uniform colormap bias into the histogram
  csRGBpixel new_cmap [256];
  int ci, colors = 0;
  for (ci = 0; ci < 256; ci++)
    if (!locked [ci] && cmap.alloc [ci])
      new_cmap [colors++] = cmap [ci];

  csQuantizeBias (new_cmap, colors, uniform_bias);

  // Now compute the actual colormap
  colors = 0;
  for (ci = 0; ci < 256; ci++)
    if (!locked [ci]) colors++;
  csRGBpixel *cmap_p = new_cmap;
  csQuantizePalette (cmap_p, colors);

  // Finally, put the computed colors back into the colormap
  int outci = 0;
  for (ci = 0; ci < colors; ci++)
  {
    while (locked [outci]) outci++;
    cmap [outci++] = new_cmap [ci];
  }

  csQuantizeEnd ();

  // Now create the inverse colormap
  create_inv_cmap ();

  // Also we need the alpha tables
  create_alpha_tables ();

  palette_ok = true;
}

void csTextureManagerSoftware::PrepareTextures ()
{
  if (verbose)
    G3D->Report (CS_REPORTER_SEVERITY_NOTIFY,
    	"Preparing textures (%s dithering)...",
	dither_textures ? "with" : "no");

  // Drop all "color allocated" flags to locked colors.
  // We won't clear the palette as we don't care about unused colors anyway.
  // The locked colors will stay the same.
  memcpy (cmap.alloc, locked, sizeof(locked));

  if (verbose)
    G3D->Report (CS_REPORTER_SEVERITY_NOTIFY, "  Creating texture mipmaps...");

  int i;

  // Create mipmaps for all textures
  for (i = 0; i < textures.Length (); i++)
  {
    csTextureHandle *txt = textures.Get (i);
    if (((txt->GetFlags () & CS_TEXTURE_PROC) == CS_TEXTURE_PROC)
     && txt->get_texture (0))
      continue;
    txt->CreateMipmaps ();
  }

  // The only thing left to do is to compute the palette
  // Everything other has been done during textures registration
  compute_palette ();

  // Remap all textures according to the new colormap.
  for (i = 0; i < textures.Length (); i++)
  {
    csTextureHandleSoftware* txt = (csTextureHandleSoftware*)textures.Get (i);
    if ((pfmt.PixelBytes == 1)
     && (txt->GetFlags() & CS_TEXTURE_PROC))
      txt->RemapProcToGlobalPalette (this);
    else
      txt->remap_texture ();
  }

  // If we are the dedicated procedural texture manager in the system, we notify
  // the main texture manager to reprepare the procedural textures created with
  // the ALONE_HINT
  if (main_txtmgr)
    main_txtmgr->Reprepare8BitProcs ();
}

iTextureHandle *csTextureManagerSoftware::RegisterTexture (iImage* image,
  int flags)
{
  if (!image) return NULL;
  csTextureHandleSoftware *txt = new csTextureHandleSoftware (this, image, flags);
  textures.Push (txt);
  return txt;
}

void csTextureManagerSoftware::UnregisterTexture (csTextureHandleSoftware* handle)
{
  int idx = textures.Find (handle);
  if (idx >= 0) textures.Delete (idx, (bool)handle->GetRefCount());
}

void csTextureManagerSoftware::ResetPalette ()
{
  memset (&locked, 0, sizeof (locked));
  locked [0] = true;
  locked [255] = true;
  cmap [0] = csRGBcolor (0, 0, 0);
  cmap [255] = csRGBcolor (255, 255, 255);
  memcpy (cmap.alloc, locked, sizeof(locked));
  palette_ok = false;
}

void csTextureManagerSoftware::ReserveColor (int r, int g, int b)
{
  if (!pfmt.PalEntries) return;
  int color = cmap.alloc_rgb (r, g, b, 0);
  locked [color] = true;
}

void csTextureManagerSoftware::SetPalette ()
{
  int i;
  if (truecolor)
    return;
  if (!palette_ok)
    compute_palette ();

  iGraphics2D *G2D = G3D->GetDriver2D ();
  for (i = 0; i < 256; i++)
  {
    int r = GammaTable [cmap [i].red];
    int g = GammaTable [cmap [i].green];
    int b = GammaTable [cmap [i].blue];
    G2D->SetRGB (i, r, g, b);
  }

  iSystem* sys = CS_GET_SYSTEM (object_reg);	//@@@
  sys->GetSystemEventOutlet ()->ImmediateBroadcast (cscmdPaletteChanged, this);
}

void csTextureManagerSoftware::Reprepare8BitProcs ()
{
  // Remap all proc textures according to the new colormap.
  for (int i = 0; i < textures.Length (); i++)
  {
    csTextureHandleSoftware* txt = (csTextureHandleSoftware*)textures.Get (i);
    if ((txt->GetFlags() & (CS_TEXTURE_PROC | CS_TEXTURE_PROC_ALONE_HINT)) 
	== (CS_TEXTURE_PROC | CS_TEXTURE_PROC_ALONE_HINT))
      ((csTextureHandleSoftware*)txt)->RemapProcToGlobalPalette (proc_txtmgr);
  }
}
