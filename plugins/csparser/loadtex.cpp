/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein
    Copyright (C) 1998-2000 by Ivan Avramovic <ivan@avramovic.com>

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

/* This file contains functions to load textures from image files. They do
 * not handle parsing of texture statements in any way.
 */

#include "cssysdef.h"
#include "csloader.h"
#include "iutil/databuff.h"
#include "iutil/object.h"
#include "iutil/vfs.h"
#include "igraphic/image.h"
#include "igraphic/imageio.h"
#include "iengine/engine.h"
#include "iengine/texture.h"
#include "iengine/material.h"
#include "ivideo/graph3d.h"
#include "ivideo/material.h"
#include "ivideo/texture.h"

iImage* csLoader::LoadImage (const char* name, int Format)
{
  if (!ImageLoader)
     return NULL;

  if (Format & CS_IMGFMT_INVALID)
  {
    if (Engine)
    {
      Format = Engine->GetTextureFormat ();
    }
    else if (G3D)
    {
      Format = G3D->GetTextureManager()->GetTextureFormat();
    }
    else
    {
      Format = CS_IMGFMT_TRUECOLOR;
    }
  }

  iImage *ifile = NULL;
  iDataBuffer *buf = VFS->ReadFile (name);

  if (!buf || !buf->GetSize ())
  {
    if (buf) buf->DecRef ();
    ReportError (
	      "crystalspace.maploader.parse.image",
    	      "Could not open image file '%s' on VFS!", name);
    return NULL;
  }

  ifile = ImageLoader->Load (buf->GetUint8 (), buf->GetSize (), Format);
  buf->DecRef ();

  if (!ifile)
  {
    ReportError (
	      "crystalspace.maploader.parse.image",
    	      "Could not load image '%s'. Unknown format or wrong extension!",
	      name);
    return NULL;
  }

  iDataBuffer *xname = VFS->ExpandPath (name);
  ifile->SetName (**xname);
  xname->DecRef ();

  return ifile;
}

iTextureHandle *csLoader::LoadTexture (const char *fname, int Flags,
	iTextureManager *tm)
{
  if (!tm && G3D)
  {
    tm = G3D->GetTextureManager();
  }
  int Format;
  if (tm)
    Format = tm->GetTextureFormat ();
  else
    Format = CS_IMGFMT_TRUECOLOR;

  iImage *Image = LoadImage (fname, Format);
  if (!Image)
    return NULL;

  iTextureHandle *TexHandle;
  if (tm)
  {
    TexHandle = tm->RegisterTexture (Image, Flags);
    if (!TexHandle)
    {
      ReportError (
	      "crystalspace.maploader.parse.texture",
	      "Cannot create texture from '%s'!", fname);
    }
  }
  else
    TexHandle = NULL;

  return TexHandle;
}

iTextureWrapper *csLoader::LoadTexture (const char *name, const char *fname,
	int Flags, iTextureManager *tm)
{
  if (!Engine)
    return NULL;

  iTextureHandle *TexHandle = LoadTexture(fname, Flags, tm);
  if (!TexHandle)
    return NULL;

  iTextureWrapper *TexWrapper =
	Engine->GetTextureList ()->NewTexture(TexHandle);
  TexWrapper->QueryObject ()->SetName (name);
  TexHandle->DecRef ();
  
  iMaterial* material = Engine->CreateBaseMaterial (TexWrapper);

  iMaterialWrapper *MatWrapper = Engine->GetMaterialList ()->
    NewMaterial (material);
  MatWrapper->QueryObject ()->SetName (name);
  material->DecRef ();

  return TexWrapper;
}
