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

csPtr<iImage> csLoader::LoadImage (const char* name, int Format)
{
  if (!ImageLoader)
     return csPtr<iImage> (NULL);

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

  csRef<iImage> ifile;
  char textureFileName[VFS_MAX_PATH_LEN];
  strcpy (textureFileName, name);
  bool found = VFS->Exists (textureFileName);
  if (!found)
  {
    ReportWarning (
	      "crystalspace.maploader.parse.image",
    	      "Could not find image file '%s' on VFS!", name);
    ReportNotify2 (
      "crystalspace.maploader.parse.image",
      " Attempting alternative filenames...");

    char* dot = strchr (textureFileName, '.');
    if (dot == NULL) dot = strchr (textureFileName, 0);

    const csVector& formats = ImageLoader->GetDescription ();
    int i = formats.Length();
    const char* lastMime = NULL;

    while ((i >= 0) && !found)
    {
#define CHECK_EXISTS				\
	if (VFS->Exists (textureFileName))	\
	{					\
	  found = true;				\
	  break;				\
	}					\

// jiggle extension case
#define JIGGLE_EXT(func)			\
	pos = dot;				\
	while (*pos) *pos++ = func(*pos);	\
	CHECK_EXISTS

// jiggle the filename case
#define JIGGLE_FN(func)				\
	pos = textureFileName;			\
	while (*pos) *pos++ = func(*pos);	\
	CHECK_EXISTS

	CHECK_EXISTS
	char* pos;
	JIGGLE_EXT(tolower)
	JIGGLE_EXT(toupper)
	JIGGLE_FN(tolower)
	JIGGLE_EXT(toupper)
	JIGGLE_FN(toupper)
	JIGGLE_EXT(tolower)

#undef JIGGLE_EXT
#undef JIGGLE_FN
#undef CHECK_EXISTS

      if (i == 0) break;

      iImageIO::FileFormatDescription *format;
      do
      {
	format = 
	  (iImageIO::FileFormatDescription*) formats[i-1];
	i--;
      }
      while ((i > 0) && 
	((lastMime != NULL) && !strcmp (lastMime, format->mime)));
      lastMime = format->mime;

      strcpy (textureFileName, name);
      if (*dot == 0) *dot = '.';
      if (i > 0)
      {
	const char* defext = strchr (format->mime, '/');
	if (defext)
	{
	  defext++;
	  // skip a leading "x-" in the mime type (eg "image/x-jng")
	  if (!strncmp (defext, "x-", 2)) defext += 2; 
	  strcpy (dot + 1, defext);
	}
	else
	  *(dot + 1) = 0;
      }
    }
    if (found)
    {
      ReportNotify2 (
	"crystalspace.maploader.parse.image",
	" ...using '%s'", &textureFileName[0]);
    }
    else
    {
      ReportNotify2 (
	"crystalspace.maploader.parse.image",
	" ...no alternative found.", &textureFileName[0]);
      strcpy (textureFileName, name);
    }
  }

  if (found)
  {
    csRef<iDataBuffer> buf (VFS->ReadFile (textureFileName));

    if (!buf || !buf->GetSize ())
    {
      ReportError (
		"crystalspace.maploader.parse.image",
    		"Could not open image file '%s' on VFS!", 
		&textureFileName[0]);
    }
    else
    {
      ifile = ImageLoader->Load (buf->GetUint8 (), buf->GetSize (), Format);

      if (!ifile)
      {
	ReportError (
		  "crystalspace.maploader.parse.image",
    		  "Could not load image '%s'. Unknown format or wrong extension!",
		  &textureFileName[0]);
      }
    }
  }
  if (!ifile)
  {
    ifile = csPtr<iImage> (csCreateXORPatternImage(32, 32, 5));
  }

  iDataBuffer *xname = VFS->ExpandPath (name);
  ifile->SetName (**xname);
  xname->DecRef ();

  ifile->IncRef ();	// IncRef() so that smart pointer will not clean it up.
  return csPtr<iImage> (ifile);
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

  iImage *Image = LoadImage (fname, Format);
  if (!Image)
    return csPtr<iTextureHandle> (NULL);

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
  Image->DecRef ();

  return csPtr<iTextureHandle> (TexHandle);
}

csPtr<iTextureWrapper> csLoader::LoadTexture (const char *name,
	const char *fname, int Flags, iTextureManager *tm, bool reg)
{
  if (!Engine)
    return NULL;

  iTextureHandle *TexHandle = LoadTexture (fname, Flags, tm);
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

  if (reg && tm)
  {
    TexWrapper->Register (tm);
    TexWrapper->GetTextureHandle()->Prepare ();
    MatWrapper->Register (tm);
    MatWrapper->GetMaterialHandle ()->Prepare ();
  }

  return TexWrapper;
}

