/*
    Copyright (C) 2001 by Christopher Nelson

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
#include "igraphic/image.h"
#include "igraphic/imageio.h"
#include "iutil/comp.h"
#include "iutil/eventh.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/vfs.h"
#include "ivaria/reporter.h"
#include "ivideo/texture.h"
#include "ivideo/txtmgr.h"
#include "awstex.h"
#include <string.h>
#include <stdio.h>

const bool DEBUG_GETTEX = false;

awsTextureManager::awsTexture::~awsTexture ()
{
}

awsTextureManager::awsTextureManager () : object_reg(0)
{
}

awsTextureManager::~awsTextureManager ()
{
}

unsigned long awsTextureManager::NameToId (const char *name) const
{
  return strset->Request(name);
}

void awsTextureManager::Initialize (iObjectRegistry *obj_reg)
{
  object_reg = obj_reg;

  if (!obj_reg) csPrintf ("aws-debug:  bad obj_reg (%s)\n", __FILE__);
  if (!object_reg) csPrintf ("aws-debug:  bad object_reg (%s)\n", __FILE__);

  loader = CS_QUERY_REGISTRY (object_reg, iImageIO);
  vfs = CS_QUERY_REGISTRY (object_reg, iVFS);

  if (!loader)
  {
    csReport (
      object_reg,
      CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.awstex",
      "could not load the image loader plugin. This is a fatal error.");
  }

  if (!vfs)
  {
    csReport (
      object_reg,
      CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.awstex",
      "could not load the VFS plugin. This is a fatal error.");
  }

  if (!vfs->Mount ("/aws", "./data/awsdef.zip"))
  {
    csReport (
      object_reg,
      CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.awstex",
      "could not mount the default aws skin (awsdef.zip)aws.");
  }

  strset = CS_QUERY_REGISTRY_TAG_INTERFACE(object_reg,
    "crystalspace.shared.stringset", iStringSet);
  if (!strset.IsValid())
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR, "crystalspace.awstex",
      "could not locate the global shared string set \""
      "crystalspace.shared.stringset\". This is a serious error.");
  }
}

iTextureHandle *awsTextureManager::GetTexture (
  const char *name,
  const char *filename,
  bool replace,
  unsigned char key_r,
  unsigned char key_g,
  unsigned char key_b
  )
{
  unsigned long id = NameToId (name);
  return GetTexturebyID (id, filename, replace, key_r, key_g, key_b);
}

iTextureHandle *awsTextureManager::GetTexturebyID (
  unsigned long id,
  const char *filename,
  bool replace,
  unsigned char key_r,
  unsigned char key_g,
  unsigned char key_b
  )
{
  awsTexture *awstxt = 0;
  bool txtfound = false;

  // Perform a lookup on the texture list.  We may consider doing this a little
  // more optimally in the future, like with a QuickSort.
  if (DEBUG_GETTEX)
    csPrintf (
      "aws-debug: (%s) texture count is: %lu\n",
      __FILE__,
      (unsigned long)textures.Length ());

  size_t i;
  for (i = 0; i < textures.Length () && txtfound == false; ++i)
  {
    awsTexture *awstxt = textures[i];

    if (DEBUG_GETTEX)
      csPrintf ("aws-debug: (%s) texture is: %p\n", __FILE__,
      	(iTextureHandle*)(awstxt->tex));

    if (awstxt && id == awstxt->id)
    {
      if (replace && filename != 0)
        txtfound = true;
      else
        return awstxt->tex;
    }
  }

  if (!txtfound && filename == 0) return 0;
  if (!txtfound) awstxt = 0;

  // If we have arrived here, then we know that the texture does not exist in
  // the cache.  Therefore, we will now attempt to load it from the disk,
  // register it, and pass back a handle.  If this fails, we'll return 0.
  if (DEBUG_GETTEX)
  {
    if (txtmgr == 0)
      csPrintf (
        "aws-debug: GetTexturebyID (%s) no texture manager.\n",
        __FILE__);
    if (vfs == 0)
      csPrintf ("aws-debug: GetTexturebyID (%s) no vfs.\n", __FILE__);
    if (loader == 0)
      csPrintf ("aws-debug: GetTexturebyID (%s) no loader.\n", __FILE__);

    if (!txtmgr || !vfs || !loader) return 0;
  }

  int Format = txtmgr->GetTextureFormat ();

  csRef<iImage> ifile;
  csRef<iDataBuffer> buf (vfs->ReadFile (filename, false));

  if (buf == 0 || buf->GetSize () == 0)
  {
    csReport (
      object_reg,
      CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.awstex",
      "Could not open image file '%s' on VFS!",
      filename);

    return 0;
  }

  ifile = loader->Load (buf, Format);

  if (!ifile)
  {
    csReport (
      object_reg,
      CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.awstex",
      "Could not load image '%s'. Unknown format or wrong extension!",
      filename);

    return 0;
  }

  // At this point, we have loaded the file from the disk, and all we're doing
  // now is creating a texture handle to the image.  The texture handle is
  // necessary to draw a pixmap with iGraphics3D::DrawPixmap()
  if (awstxt == 0)
  {
    awstxt = new awsTexture;
    memset (awstxt, 0, sizeof (awsTexture));
  }

  awstxt->img = ifile;
  awstxt->tex = txtmgr->RegisterTexture (ifile,
      CS_TEXTURE_2D | CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS);
  awstxt->id = id;

  // Post load work...
  awstxt->tex->SetKeyColor (key_r, key_g, key_b);
  awstxt->tex->SetTextureClass( "nocompress" );

  textures.Push (awstxt);

  return awstxt->tex;
}

void awsTextureManager::SetTextureManager (iTextureManager *newtxtmgr)
{
  if (txtmgr && newtxtmgr)
  {
    UnregisterTextures ();
  }

  if (newtxtmgr)
  {
    txtmgr = newtxtmgr;
    RegisterTextures ();
  }
}

void awsTextureManager::RegisterTextures ()
{
}

void awsTextureManager::UnregisterTextures ()
{
}
