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
#include <math.h>
#include <stdarg.h>
#include "csplugincommon/render3d/txtmgr.h"
#include "csutil/util.h"
#include "csutil/debug.h"
#include "csgfx/imagemanipulate.h"
#include "iutil/objreg.h"
#include "csqint.h"
#include "igraphic/image.h"
#include "ivideo/material.h"
#include "iengine/material.h"
#include "ivideo/graph2d.h"

csTexture::csTexture (csTextureHandle *Parent)
{
  parent = Parent;
  DG_ADD (this, 0);
  DG_TYPE (this, "csTexture");
}

csTexture::~csTexture ()
{
  DG_REM (this);
}

//----------------------------------------------------- csTextureHandle -----//

SCF_IMPLEMENT_IBASE (csTextureHandle)
  SCF_IMPLEMENTS_INTERFACE (iTextureHandle)
SCF_IMPLEMENT_IBASE_END

csTextureHandle::csTextureHandle (csTextureManager* texman, iImage* Image, 
				  int Flags)
{
  SCF_CONSTRUCT_IBASE (0);
  DG_ADDI (this, 0);
  DG_TYPE (this, "csTextureHandle");

  image = Image;
  DG_LINK (this, image);
  flags = Flags;

  tex [0] = tex [1] = tex [2] = tex [3] = 0;

  transp = false;
  transp_color.red = transp_color.green = transp_color.blue = 0;

  if (image.IsValid() && image->HasKeyColor ())
  {
    int r,g,b;
    image->GetKeyColor (r,g,b);
    SetKeyColor (r, g, b);
  }
  cachedata = 0;

  this->texman = texman;
  texClass = texman->texClassIDs.Request ("default");
}

csTextureHandle::~csTextureHandle ()
{
  int i;
  for (i = 0; i < 4; i++)
  {
    if (tex[i])
    {
      DG_UNLINK (this, tex[i]);
      delete tex [i];
    }
  }
  DG_REM (this);
  FreeImage ();
  SCF_DESTRUCT_IBASE()
}

void csTextureHandle::FreeImage ()
{
  if (!image) return;
  PrepareInt ();
  DG_UNLINK (this, image);
  image = 0;
}

void csTextureHandle::CreateMipmaps ()
{
  if (!image) return;

  csRGBpixel *tc = transp ? &transp_color : (csRGBpixel *)0;

  // Delete existing mipmaps, if any
  int i;
  for (i = 0; i < 4; i++)
  {
    if (tex[i])
    {
      DG_UNLINK (this, tex[i]);
      delete tex [i];
    }
  }

  // @@@ Jorrit: removed the following IncRef() because I can really
  // see no reason for it and it seems to be causing memory leaks.
#if 0
  // Increment reference counter on image since NewTexture() expects
  // a image with an already incremented reference counter
  image->IncRef ();
#endif
  tex [0] = NewTexture (image);
  DG_LINK (this, tex[0]);

  // 2D textures uses just the top-level mipmap
  if ((flags & (CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS)) == CS_TEXTURE_3D)
  {
    // Create each new level by creating a level 2 mipmap from previous level
    csRef<iImage> i1 = csImageManipulate::Mipmap (image, 1, tc);
    csRef<iImage> i2 = csImageManipulate::Mipmap (i1, 1, tc);
    csRef<iImage> i3 = csImageManipulate::Mipmap (i2, 1, tc);

    tex [1] = NewTexture (i1, true);
    DG_LINK (this, tex[1]);
    tex [2] = NewTexture (i2, true);
    DG_LINK (this, tex[2]);
    tex [3] = NewTexture (i3, true);
    DG_LINK (this, tex[3]);
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
void csTextureHandle::GetKeyColor (uint8 &r, uint8 &g, uint8 &b) const
{
  r = transp_color.red;
  g = transp_color.green;
  b = transp_color.blue;
}

bool csTextureHandle::GetKeyColor () const
{
  return transp;
}

void csTextureHandle::GetMeanColor (uint8 &r, uint8 &g, uint8 &b) const
{
  r = mean_color.red;
  g = mean_color.green;
  b = mean_color.blue;
}

bool csTextureHandle::GetRendererDimensions (int &mw, int &mh)
{
  PrepareInt ();
  if (!tex[0]) return false;
  mw = tex[0]->get_width();
  mh = tex[0]->get_height();
  return true;
}

void csTextureHandle::AdjustSizePo2 ()
{
  int newwidth, newheight;

  CalculateNextBestPo2Size (image->GetWidth(), newwidth);
  CalculateNextBestPo2Size (image->GetHeight(), newheight);

  if (newwidth != image->GetWidth () || newheight != image->GetHeight ())
    image = csImageManipulate::Rescale (image, newwidth, newheight);
}

void csTextureHandle::CalculateNextBestPo2Size (const int orgDim, int& newDim)
{
  newDim = csFindNearestPowerOf2 (orgDim);
  if (newDim != orgDim)
  {
    int dU = newDim - orgDim;
    int dD = orgDim - (newDim >> 1);
    if (dD < dU)
      newDim >>= 1;
  }
}

void csTextureHandle::SetTextureClass (const char* className)
{
  texClass = texman->texClassIDs.Request (className ? className : "default");
}

const char* csTextureHandle::GetTextureClass ()
{
  return texman->texClassIDs.Request (texClass);
}

//----------------------------------------------------- csMaterialHandle -----//

SCF_IMPLEMENT_IBASE (csMaterialHandle)
  SCF_IMPLEMENTS_INTERFACE (iMaterialHandle)
SCF_IMPLEMENT_IBASE_END

csMaterialHandle::csMaterialHandle (iMaterial* m, csTextureManager *parent)
{
  SCF_CONSTRUCT_IBASE (0);
  DG_ADDI (this, 0);
  DG_TYPE (this, "csMaterialHandle");
  texman = parent;
  material = m;
  if (material != 0)
  {
    csRef<iTextureHandle> texture = GetTexture ();
    // @@@ really, all textures should be linked
    if (texture)
    {
      DG_LINK (this, texture);
    }
  }
}

csMaterialHandle::csMaterialHandle (iTextureHandle* t, csTextureManager *parent)
{
  SCF_CONSTRUCT_IBASE (0);
  DG_ADDI (this, 0);
  DG_TYPE (this, "csMaterialHandle");
  // @@@ Need iMaterial, or a shader branch or so!!!
  texman = parent;
}

csMaterialHandle::~csMaterialHandle ()
{
  FreeMaterial ();
  texman->UnregisterMaterial (this);
  DG_REM (this);
  SCF_DESTRUCT_IBASE()
}

void csMaterialHandle::FreeMaterial ()
{
  csRef<iTextureHandle> texture = GetTexture ();
  if (texture)
  {
    DG_UNLINK (this, texture);
  }
  material = 0;
}

iTextureHandle* csMaterialHandle::GetTexture ()
{
  if (material)
  {
    return material->GetTexture (texman->nameDiffuseTexture);
  }
  else
  {
    return 0;
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

SCF_IMPLEMENT_IBASE (csTextureManager)
  SCF_IMPLEMENTS_INTERFACE (iTextureManager)
SCF_IMPLEMENT_IBASE_END

csTextureManager::csTextureManager (iObjectRegistry* object_reg,
	iGraphics2D *iG2D)
	: textures (16, 16), materials (16, 16)
{
  SCF_CONSTRUCT_IBASE (0);
  csTextureManager::object_reg = object_reg;

  pfmt = *iG2D->GetPixelFormat ();

  csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (
    object_reg, "crystalspace.shared.stringset", iStringSet);
  CS_ASSERT(strings != 0);
  nameDiffuseTexture = strings->Request (CS_MATERIAL_TEXTURE_DIFFUSE);
}

csTextureManager::~csTextureManager()
{
  Clear ();
  SCF_DESTRUCT_IBASE()
}

void csTextureManager::read_config (iConfigFile* /*config*/)
{
}

int csTextureManager::GetTextureFormat ()
{
  return CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA;
}

csPtr<iMaterialHandle> csTextureManager::RegisterMaterial (iMaterial* material)
{
  if (!material) return 0;
  csMaterialHandle *mat = new csMaterialHandle (material, this);
  materials.Push (mat);
  return csPtr<iMaterialHandle> (mat);
}

csPtr<iMaterialHandle> csTextureManager::RegisterMaterial (
	iTextureHandle* txthandle)
{
  if (!txthandle) return 0;
  csMaterialHandle *mat = new csMaterialHandle (txthandle, this);
  materials.Push (mat);
  return csPtr<iMaterialHandle> (mat);
}

void csTextureManager::UnregisterMaterial (csMaterialHandle* handle)
{
  size_t idx = materials.Find (handle);
  if (idx != csArrayItemNotFound) materials.DeleteIndexFast (idx);
}

void csTextureManager::FreeMaterials ()
{
  size_t i;
  for (i = 0; i < materials.Length (); i++)
  {
    csMaterialHandle* mat = materials.Get (i);
    if (mat) mat->FreeMaterial ();
  }
}
