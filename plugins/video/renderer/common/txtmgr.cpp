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

#include "sysdef.h"
#include "txtmgr.h"
#include "csutil/util.h"
#include "csutil/inifile.h"
#include "qint.h"
#include "iimage.h"
#include "isystem.h"
#include "lightdef.h"

/// This is not too good from theoretical point of view,
/// however I do not see other efficient solutions. An alternative could
/// be to store such a pointer into each csTextureMM, but this is definitely
/// a waste of memory.
static csTextureManager *texman = NULL;

//---------------------------------------------------------- csTextureMM -----//

IMPLEMENT_IBASE (csTextureMM)
  IMPLEMENTS_INTERFACE (iTextureHandle)
IMPLEMENT_IBASE_END

csTextureMM::csTextureMM (iImage* Image, int Flags)
{
  CONSTRUCT_IBASE (NULL);

  (image = Image)->IncRef ();
  flags = Flags;

  tex [0] = tex [1] = tex [2] = tex [3] = NULL;

  transp = false;
  transp_color.red = transp_color.green = transp_color.blue = 0;
  cachedata = NULL;
  gamma_aplied = false;
}

csTextureMM::~csTextureMM ()
{
  for (int i = 0; i < 4; i++)
    CHKB (delete tex [i]);
  free_image ();
}

void csTextureMM::free_image ()
{
  if (!image) return;
  image->DecRef ();
  image = NULL;
}

void csTextureMM::create_mipmaps (bool verynice, bool blend_mipmap0)
{
  if (!image) return;

  // Delete existing mipmaps, if any
  for (int i = 0; i < 4; i++)
    CHKB (delete tex [i]);

  int step = verynice ? 0 : 1;

  RGBPixel *tc = transp ? &transp_color : (RGBPixel *)NULL;

  if (flags & CS_TEXTURE_3D)
  {
    int n=1;
    int maxMM = CS_GET_TEXTURE_MMLEVEL(flags);
    maxMM = maxMM == 0 ? 4 : MIN( maxMM, 4 );
    iImage *i0 = blend_mipmap0 ? image->MipMap (0, tc) : (image->IncRef (), image);
#if 1
    // Create each new level by creating a level 2 mipmap from previous level
    iImage *i1 = ++n <= maxMM ? i0->MipMap (step, tc) : NULL;
    iImage *i2 = ++n <= maxMM ? i1->MipMap (1, tc) : NULL;
    iImage *i3 = ++n <= maxMM ? i2->MipMap (1, tc) : NULL;
#else
    // Create each mipmap level from original texture
    iImage *i1 = ++n <= maxMM ? image->MipMap (step++, tc) : NULL;
    iImage *i2 = ++n <= maxMM ? image->MipMap (step++, tc) : NULL;
    iImage *i3 = ++n <= maxMM ? image->MipMap (step++, tc) : NULL;
#endif

    tex [0] = new_texture (i0);
    tex [1] = i1 ? new_texture (i1) : NULL;
    tex [2] = i2 ? new_texture (i2) : NULL;
    tex [3] = i3 ? new_texture (i3) : NULL;
  }
  else
  {
    // 2D textures uses just the top-level mipmap
    image->IncRef ();
    tex [0] = new_texture (image);
  }

  compute_mean_color ();
}

void csTextureMM::SetTransparent (bool Enable)
{
  transp = Enable;
}

// This function must be called BEFORE calling TextureManager::Update().
void csTextureMM::SetTransparent (UByte red, UByte green, UByte blue)
{
  transp_color.red = red;
  transp_color.green = green;
  transp_color.blue = blue;
  transp = true;
}

/// Get the transparent color
void csTextureMM::GetTransparent (UByte &r, UByte &g, UByte &b)
{
  r = transp_color.red;
  g = transp_color.green;
  b = transp_color.blue;
}

bool csTextureMM::GetTransparent ()
{
  return transp;
}

void csTextureMM::GetMeanColor (UByte &r, UByte &g, UByte &b)
{ 
  r = mean_color.red;
  g = mean_color.green;
  b = mean_color.blue;
}

csTexture* csTextureMM::get_texture (int lev)
{
  return (lev >= 0) && (lev < 4) ? tex [lev] : NULL;
}

bool csTextureMM::GetMipMapDimensions (int mipmap, int& w, int& h) 
{ 
  csTexture *txt = get_texture (mipmap);
  if (txt)
  {
    w = txt->get_width ();
    h = txt->get_height ();
    return true;
  }
  return false;
}

void *csTextureMM::GetMipMapData (int mipmap)
{
  csTexture *txt = get_texture (mipmap);
  return txt ? txt->get_bitmap () : NULL;
}

void csTextureMM::adjust_size_po2 ()
{
  int newwidth  = image->GetWidth();
  int newheight = image->GetHeight();

  if (!IsPowerOf2(newwidth))
    newwidth = FindNearestPowerOf2 (image->GetWidth ()) / 2;

  if (!IsPowerOf2 (newheight))
    newheight = FindNearestPowerOf2 (image->GetHeight ()) / 2;

  if (newwidth != image->GetWidth () || newheight != image->GetHeight ())
    image->Rescale (newwidth, newheight);
}

void csTextureMM::apply_gamma ()
{
  if (gamma_aplied || !image)
    return;
  gamma_aplied = true;

  if (texman->Gamma == 1.0)
    return;

  RGBPixel *src = NULL;
  int pixels = 0;
  switch (image->GetFormat () & CS_IMGFMT_MASK)
  {
    case CS_IMGFMT_TRUECOLOR:
      src = (RGBPixel *)image->GetImageData ();
      pixels = image->GetWidth () * image->GetHeight ();
      break;
    case CS_IMGFMT_PALETTED8:
      src = image->GetPalette ();
      pixels = 256;
      break;
  }
  if (!src) return;
  while (pixels--)
  {
    src->red   = texman->GammaTable [src->red];
    src->green = texman->GammaTable [src->green];
    src->blue  = texman->GammaTable [src->blue];
    src++;
  }
}

//------------------------------------------------------------ csTexture -----//

void csTexture::compute_masks ()
{
  shf_w = csLog2 (w);
  and_w = (1 << shf_w) - 1;
  shf_h = csLog2 (h);
  and_h = (1 << shf_h) - 1;
}

//----------------------------------------------------- csTextureManager -----//

IMPLEMENT_IBASE (csTextureManager)
  IMPLEMENTS_INTERFACE (iTextureManager)
IMPLEMENT_IBASE_END

csTextureManager::csTextureManager (iSystem* iSys, iGraphics2D *iG2D)
  : textures (16, 16)
{
  texman = this;

  System = iSys;
  verbose = false;

  Gamma = 1.0;
  mipmap_mode = MIPMAP_NICE;
  do_blend_mipmap0 = false;
  pfmt = *iG2D->GetPixelFormat ();
}

csTextureManager::~csTextureManager()
{
  Clear ();
}

void csTextureManager::read_config (csIniFile *config)
{
  Gamma = config->GetFloat ("TextureManager", "GAMMA", 1.0);
  do_blend_mipmap0 = config->GetYesNo ("TextureManager", "BLEND_MIPMAP", false);
  const char *p = config->GetStr ("TextureManager", "MIPMAP_MODE", "nice");
  if (!strcmp (p, "nice"))
  {
    mipmap_mode = MIPMAP_NICE;
    if (verbose)
      System->Printf (MSG_INITIALIZATION, "Mipmap calculation 'nice'.\n");
  }
  else if (!strcmp (p, "verynice"))
  {
    mipmap_mode = MIPMAP_VERYNICE;
    if (verbose)
      System->Printf (MSG_INITIALIZATION, "Mipmap calculation 'verynice'\n");
  }
  else
    System->Printf (MSG_FATAL_ERROR, "Bad value '%s' for MIPMAP_MODE!\n(Use 'verynice' or 'nice')\n", p);
  compute_gamma_table ();
}

void csTextureManager::FreeImages ()
{
  for (int i = 0 ; i < textures.Length () ; i++)
    textures.Get (i)->free_image ();
}

int csTextureManager::GetTextureFormat ()
{
  return CS_IMGFMT_TRUECOLOR;
}

void csTextureManager::compute_gamma_table ()
{
  for (int i = 0; i < 256; i++)
    GammaTable [i] = QRound (255 * pow (i / 255.0, Gamma));
}

int csTextureManager::FindRGB (int r, int g, int b)
{
  if (r > 255) r = 255; else if (r < 0) r = 0;
  if (g > 255) g = 255; else if (g < 0) g = 0;
  if (b > 255) b = 255; else if (b < 0) b = 0;
  return
    ((r >> (8-pfmt.RedBits))   << pfmt.RedShift) |
    ((g >> (8-pfmt.GreenBits)) << pfmt.GreenShift) |
    ((b >> (8-pfmt.BlueBits))  << pfmt.BlueShift);
}

void csTextureManager::ReserveColor (int /*r*/, int /*g*/, int /*b*/)
{
}

void csTextureManager::SetPalette ()
{
}

void csTextureManager::ResetPalette ()
{
}
