/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein

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
#include <stdarg.h>
#include <math.h>

#include "video/renderer/direct3d5/d3d_txtmgr.h"
#include "csutil/scanstr.h"
#include "iimage.h"
#include "isystem.h"
#include "lightdef.h"

#define RESERVED_COLOR(c) ((c == 0) || (c == 255))

csTextureManagerDirect3D::csTextureManagerDirect3D (iSystem* iSys,
  iGraphics2D* iG2D, csIniFile *config) : csTextureManager (iSys, iG2D)
{
  read_config (config);
  Clear ();
}

csTextureManagerDirect3D::~csTextureManagerDirect3D ()
{
  Clear ();
}

ULong csTextureManagerDirect3D::encode_rgb (int r, int g, int b)
{
  return
    ((r >> (8-pfmt.RedBits))   << pfmt.RedShift) |
    ((g >> (8-pfmt.GreenBits)) << pfmt.GreenShift) |
    ((b >> (8-pfmt.BlueBits))  << pfmt.BlueShift);
}

void csTextureManagerDirect3D::remap_textures ()
{
  int i;

  // Remap all textures according to the new colormap.
  for (i = 0 ; i < textures.Length () ; i++)
  {
    csTextureMMDirect3D* pTexture = (csTextureMMDirect3D*)textures[i];
    pTexture->remap_texture (this);
  }
}

void csTextureManagerDirect3D::PrepareTextures ()
{
  int i;

  delete factory_3d; factory_3d = NULL;
  delete factory_2d; factory_2d = NULL;
  factory_3d = new csTextureFactory32 ();
  if (pfmt.PixelBytes == 1)
    { factory_2d = new csTextureFactory8 (); }
  else if (pfmt.PixelBytes == 2)
    { factory_2d = new csTextureFactory16 (); }
  else
    { factory_2d = new csTextureFactory32 (); }

  remap_textures ();
}

iTextureHandle *csTextureManagerDirect3D::RegisterTexture (iImage* image,
  int flags)
{
  if (!image) return NULL;

  csTextureMMDirect3D *txt = new csTextureMMDirect3D (image, flags);
  txt->CreateMipmaps ();
  textures.Push (txt);
  return txt;
}

void csTextureManagerDirect3D::PrepareTexture (iTextureHandle *handle)
{
  if (!handle) return;

  csTextureMMDirect3D *txt = (csTextureMMDirect3D *)handle->GetPrivateObject ();
  txt->CreateMipmaps ();
  txt->remap_texture (this);
}

void csTextureManagerDirect3D::UnregisterTexture (iTextureHandle* handle)
{
  (void)handle;
  //@@@ Not implemented yet.
}
