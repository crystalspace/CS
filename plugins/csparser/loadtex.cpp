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

#include <ctype.h>
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
#include "csgfx/xorpat.h"

csPtr<iImage> csLoader::LoadImage (const char* fname, int Format)
{
  if (!ImageLoader)
     return NULL;

  if (Format & CS_IMGFMT_INVALID)
  {
    if (Engine)
      Format = Engine->GetTextureFormat ();
    else if (G3D)
      Format = G3D->GetTextureManager()->GetTextureFormat();
    else
      Format = CS_IMGFMT_TRUECOLOR;
  }

  csRef<iDataBuffer> buf (VFS->ReadFile (fname));
  if (!buf || !buf->GetSize ())
  {
    ReportError (
	"crystalspace.maploader.parse.image",
    	"Could not open image file '%s' on VFS!", fname);
    return NULL;
  }

  // we don't use csRef because we need to return an Increfed object later
  csRef<iImage> image (
    ImageLoader->Load (buf->GetUint8 (), buf->GetSize (), Format));
  if (!image)
  {
    ReportError (
	"crystalspace.maploader.parse.image",
	"Could not load image '%s'. Unknown format!",
	fname);
    return NULL;
  }
  
  csRef<iDataBuffer> xname (VFS->ExpandPath (fname));
  image->SetName (**xname);

  image->IncRef ();	// To avoid smart pointer release.
  return csPtr<iImage> (image);
}

csPtr<iTextureHandle> csLoader::LoadTexture (const char *fname, int Flags,
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

  csRef<iImage> Image = LoadImage (fname, Format);
  if (!Image)
  {
    ReportError (
	"crystalspace.maploader.parse.texture",
	"Couldn't load image '%s', using checkerboard instead!",
	fname);
    Image = csCreateXORPatternImage (32, 32, 5);
    if (!Image)
      return NULL;
  }

  if (!tm)
    return NULL;
  
  // we need to return an IncRefed object, so no csRef here
  csRef<iTextureHandle> TexHandle (tm->RegisterTexture (Image, Flags));
  if (!TexHandle)
  {
    ReportError (
	"crystalspace.maploader.parse.texture",
	"Cannot create texture from '%s'!", fname);
    return NULL;
  }

  TexHandle->IncRef ();	// Avoid smart pointer release.
  return csPtr<iTextureHandle> (TexHandle);
}

iTextureWrapper* csLoader::LoadTexture (const char *name,
	const char *fname, int Flags, iTextureManager *tm, bool reg)
{
  if (!Engine)
    return NULL;

  csRef<iTextureHandle> TexHandle (LoadTexture (fname, Flags, tm));
  if (!TexHandle)
    return NULL;

  iTextureWrapper *TexWrapper =
	Engine->GetTextureList ()->NewTexture(TexHandle);
  TexWrapper->QueryObject ()->SetName (name);

  csRef<iMaterial> material (Engine->CreateBaseMaterial (TexWrapper));

  iMaterialWrapper *MatWrapper = Engine->GetMaterialList ()->
    NewMaterial (material);
  MatWrapper->QueryObject ()->SetName (name);

  if (reg && tm)
  {
    TexWrapper->Register (tm);
    TexWrapper->GetTextureHandle()->Prepare ();
    MatWrapper->Register (tm);
    MatWrapper->GetMaterialHandle ()->Prepare ();
  }

  return TexWrapper;
}

