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
#include "av3dtxtm.h"
#include "csutil/util.h"
#include "csutil/debug.h"
#include "qint.h"
#include "igraphic/image.h"
#include "ivideo/material.h"
#include "iengine/material.h"
#include "ivideo/graph2d.h"

csTexture::csTexture (csTextureHandle *Parent)
{
  parent = Parent;
  DG_ADD (this, NULL);
  DG_TYPE (this, "csTexture");
}

csTexture::~csTexture ()
{
  DG_REM (this);
}

//----------------------------------------------------- csTextureHandle -----//
SCF_IMPLEMENT_IBASE(csTextureHandle)
  SCF_IMPLEMENTS_INTERFACE(iTextureHandle)
SCF_IMPLEMENT_IBASE_END

csTextureHandle::csTextureHandle (
  iImage *Image,
  int Flags)
{
  SCF_CONSTRUCT_IBASE (NULL);
  DG_ADDI (this, NULL);
  DG_TYPE (this, "csTextureHandle");
  (image = Image)->IncRef ();
  DG_LINK (this, image);
  flags = Flags;

  tex[0] = tex[1] = tex[2] = tex[3] = NULL;

  transp = false;
  transp_color.red = transp_color.green = transp_color.blue = 0;

  if (image->HasKeycolor ())
  {
    int r, g, b;
    image->GetKeycolor (r, g, b);
    SetKeyColor (r, g, b);
  }

  cachedata = NULL;
}

csTextureHandle::~csTextureHandle ()
{
  int i;
  for (i = 0; i < 4; i++)
  {
    if (tex[i])
    {
      DG_UNLINK (this, tex[i]);
      delete tex[i];
    }
  }

  DG_REM (this);
  FreeImage ();
}

void csTextureHandle::FreeImage ()
{
  if (!image) return ;
  DG_UNLINK (this, image);
  image->DecRef ();
  image = NULL;
}

void csTextureHandle::CreateMipmaps ()
{
  if (!image) return ;

  csRGBpixel *tc = transp ? &transp_color : (csRGBpixel *)NULL;

  // Delete existing mipmaps, if any
  int i;
  for (i = 0; i < 4; i++)
  {
    if (tex[i])
    {
      DG_UNLINK (this, tex[i]);
      delete tex[i];
    }
  }

  // @@@ Jorrit: removed the following IncRef() because I can really

  // see no reason for it and it seems to be causing memory leaks.
#if 0
  // Increment reference counter on image since NewTexture() expects

  // a image with an already incremented reference counter
  image->IncRef ();
#endif
  tex[0] = NewTexture (image);
  DG_LINK (this, tex[0]);

  // 2D textures uses just the top-level mipmap
  if ((flags & (CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS)) == CS_TEXTURE_3D)
  {
    // Create each new level by creating a level 2 mipmap from previous level
    iImage *i1 = image->MipMap (1, tc);
    iImage *i2 = i1->MipMap (1, tc);
    iImage *i3 = i2->MipMap (1, tc);

    tex[1] = NewTexture (i1);
    DG_LINK (this, tex[1]);
    i1->DecRef ();
    tex[2] = NewTexture (i2);
    DG_LINK (this, tex[2]);
    i2->DecRef ();
    tex[3] = NewTexture (i3);
    DG_LINK (this, tex[3]);
    i3->DecRef ();
  }

  ComputeMeanColor ();
}

void csTextureHandle::SetKeyColor (bool Enable)
{
  transp = Enable;
}

// This function must be called BEFORE calling TextureManager::Update().
void csTextureHandle::SetKeyColor (uint8 red, uint8 green, uint8 blue)
{
  transp_color.red = red;
  transp_color.green = green;
  transp_color.blue = blue;
  transp = true;
}

/// Get the transparent color
void csTextureHandle::GetKeyColor (uint8 &r, uint8 &g, uint8 &b)
{
  r = transp_color.red;
  g = transp_color.green;
  b = transp_color.blue;
}

bool csTextureHandle::GetKeyColor ()
{
  return transp;
}

void csTextureHandle::GetMeanColor (uint8 &r, uint8 &g, uint8 &b)
{
  r = mean_color.red;
  g = mean_color.green;
  b = mean_color.blue;
}

bool csTextureHandle::GetMipMapDimensions (int mipmap, int &w, int &h)
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

void csTextureHandle::AdjustSizePo2 ()
{
  int newwidth = image->GetWidth ();
  int newheight = image->GetHeight ();

  if (!csIsPowerOf2 (newwidth))
    newwidth = csFindNearestPowerOf2 (image->GetWidth ()) / 2;

  if (!csIsPowerOf2 (newheight))
    newheight = csFindNearestPowerOf2 (image->GetHeight ()) / 2;

  if (newwidth != image->GetWidth () || newheight != image->GetHeight ())
    image->Rescale (newwidth, newheight);
}

//----------------------------------------------------- csMaterialHandle -----//
SCF_IMPLEMENT_IBASE(csMaterialHandle)
  SCF_IMPLEMENTS_INTERFACE(iMaterialHandle)
SCF_IMPLEMENT_IBASE_END

csMaterialHandle::csMaterialHandle (
  iMaterial *m,
  csTextureManager *parent)
{
  SCF_CONSTRUCT_IBASE (NULL);
  DG_ADDI (this, NULL);
  DG_TYPE (this, "csMaterialHandle");
  num_texture_layers = 0;
  if ((material = m) != 0)
  {
    material->IncRef ();
    texture = material->GetTexture ();
    if (texture)
    {
      texture->IncRef ();
      DG_LINK (this, texture);
    }

    material->GetReflection (diffuse, ambient, reflection);
    material->GetFlatColor (flat_color);
    num_texture_layers = material->GetTextureLayerCount ();
    if (num_texture_layers > 4) num_texture_layers = 4;

    int i;
    for (i = 0; i < num_texture_layers; i++)
    {
      texture_layers[i] = *(material->GetTextureLayer (i));
      texture_layer_translate[i] = texture_layers[i].uscale != 1 ||
        texture_layers[i].vscale != 1 ||
        texture_layers[i].ushift != 0 ||
        texture_layers[i].vshift != 0;
    }
  }
  (texman = parent)->IncRef ();
}

csMaterialHandle::csMaterialHandle (
  iTextureHandle *t,
  csTextureManager *parent)
{
  SCF_CONSTRUCT_IBASE (NULL);
  DG_ADDI (this, NULL);
  DG_TYPE (this, "csMaterialHandle");
  material = NULL;
  num_texture_layers = 0;
  diffuse = 0.7;
  ambient = 0;
  reflection = 0;
  if ((texture = t) != 0)
  {
    DG_LINK (this, texture);
    texture->IncRef ();
  }
  (texman = parent)->IncRef ();
}

csMaterialHandle::~csMaterialHandle ()
{
  FreeMaterial ();
  texman->UnregisterMaterial (this);
  texman->DecRef ();
  DG_REM (this);
}

void csMaterialHandle::FreeMaterial ()
{
  if (texture)
  {
    DG_UNLINK (this, texture);
    SCF_DEC_REF (texture);
  }

  if (material)
  {
    material->DecRef ();
    material = NULL;
  }
}

void csMaterialHandle::Prepare ()
{
  if (material)
  {
    if (texture != material->GetTexture ())
    {
      DG_UNLINK (this, texture);
      SCF_DEC_REF (texture);
      texture = material->GetTexture ();
      if (texture)
      {
        texture->IncRef ();
        DG_LINK (this, texture);
      }
    }

    material->GetReflection (diffuse, ambient, reflection);
    material->GetFlatColor (flat_color);
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
SCF_IMPLEMENT_IBASE(csTextureManager)
  SCF_IMPLEMENTS_INTERFACE(iTextureManager)
SCF_IMPLEMENT_IBASE_END

csTextureManager::csTextureManager (
  iObjectRegistry *object_reg,
  iGraphics2D *iG2D) :
    textures(16, 16),
    materials(16, 16)
{
  SCF_CONSTRUCT_IBASE (NULL);
  csTextureManager::object_reg = object_reg;
  verbose = false;

  pfmt = *iG2D->GetPixelFormat ();
}

csTextureManager::~csTextureManager ()
{
  Clear ();
  printf ("Texture manager now going bye byes...\n"); //@@@ Debugging. MHV
}

void csTextureManager::read_config (iConfigFile *

/*config*/ )
{
}

void csTextureManager::FreeImages ()
{
  int i;
  for (i = 0; i < textures.Length (); i++) textures.Get (i)->FreeImage ();
}

int csTextureManager::GetTextureFormat ()
{
  return CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA;
}

int csTextureManager::FindRGB (int r, int g, int b)
{
  if (r > 255)
    r = 255;
  else if (r < 0)
    r = 0;
  if (g > 255)
    g = 255;
  else if (g < 0)
    g = 0;
  if (b > 255)
    b = 255;
  else if (b < 0)
    b = 0;
  return ((r >> (8 - pfmt.RedBits)) << pfmt.RedShift) |
    ((g >> (8 - pfmt.GreenBits)) << pfmt.GreenShift) |
      ((b >> (8 - pfmt.BlueBits)) << pfmt.BlueShift);
}

void csTextureManager::ReserveColor (int

/*r*/, int

/*g*/, int

/*b*/ )
{
}

void csTextureManager::SetPalette ()
{
}

void csTextureManager::ResetPalette ()
{
}

iMaterialHandle *csTextureManager::RegisterMaterial (iMaterial *material)
{
  if (!material) return NULL;

  csMaterialHandle *mat = new csMaterialHandle (material, this);
  materials.Push (mat);
  return mat;
}

iMaterialHandle *csTextureManager::RegisterMaterial (
  iTextureHandle *txthandle)
{
  if (!txthandle) return NULL;

  csMaterialHandle *mat = new csMaterialHandle (txthandle, this);
  materials.Push (mat);
  return mat;
}

void csTextureManager::UnregisterMaterial (csMaterialHandle *handle)
{
  int idx = materials.Find (handle);
  if (idx >= 0) materials.Delete (idx);
}

void csTextureManager::PrepareMaterials ()
{
  int i;
  for (i = 0; i < materials.Length (); i++) materials.Get (i)->Prepare ();
}

void csTextureManager::FreeMaterials ()
{
  int i;
  for (i = 0; i < materials.Length (); i++)
    materials.Get (i)->FreeMaterial ();
}
