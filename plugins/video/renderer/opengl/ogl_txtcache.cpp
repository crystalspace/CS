/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein
    Copyright (C) 1998 by Dan Ogles.

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
#include "csgeom/subrec.h"

#include "ogl_txtcache.h"
#include "ogl_txtmgr.h"
#include "ogl_g3dcom.h"
#include "ivideo/graph3d.h"
#include "ivaria/reporter.h"
#include "plugins/video/canvas/openglcommon/glstates.h"

#include "igraphic/imageio.h"
#include "iutil/vfs.h"
#include "csgfx/memimage.h"

// need definitions of R24(), G24(), and B24()
#ifndef CS_NORMAL_LIGHT_LEVEL
#define CS_NORMAL_LIGHT_LEVEL 128
#endif

//---------------------------------------------------------------------------//

/// Unload a texture cache element (common for both caches)
void OpenGLTextureCache::Unload (csTxtCacheData *d)
{
  if (d->next)
    d->next->prev = d->prev;
  else
    tail = d->prev;

  if (d->prev)
    d->prev->next = d->next;
  else
    head = d->next;

  if (csGraphics3DOGLCommon::statecache->GetTexture (GL_TEXTURE_2D) == 
    d->Handle)
    csGraphics3DOGLCommon::statecache->SetTexture (GL_TEXTURE_2D, 0);
  glDeleteTextures (1, &d->Handle);
  d->Handle = 0;

  num--;
  total_size -= d->Size;

  iTextureHandle *texh = (iTextureHandle *)d->Source;
  if (texh) texh->SetCacheData (0);

  delete d;
}

//----------------------------------------------------------------------------//

OpenGLTextureCache::OpenGLTextureCache (int max_size,
  csGraphics3DOGLCommon* g3d)
{
  cache_size = max_size;
  num = 0;
  head = tail = 0;
  total_size = 0;
  peak_total_size = 0;
  OpenGLTextureCache::g3d = g3d;
}

OpenGLTextureCache::~OpenGLTextureCache ()
{
  Clear ();
}

void OpenGLTextureCache::Cache (iTextureHandle *txt_handle)
{
  csTxtCacheData *cached_texture = (csTxtCacheData *)
    txt_handle->GetCacheData ();

  if (cached_texture)
  {
    // move unit to front (MRU)
    if (cached_texture != head)
    {
      if (cached_texture->prev)
        cached_texture->prev->next = cached_texture->next;
      else
        head = cached_texture->next;
      if (cached_texture->next)
        cached_texture->next->prev = cached_texture->prev;
      else
        tail = cached_texture->prev;

      cached_texture->prev = 0;
      cached_texture->next = head;
      if (head)
        head->prev = cached_texture;
      else
        tail = cached_texture;
      head = cached_texture;
    }
  }
  else
  {
    csTextureHandleOpenGL *txt_mm = (csTextureHandleOpenGL *)
      txt_handle->GetPrivateObject ();

    // unit is not in memory. load it into the cache
    while (cache_size && (total_size + txt_mm->size >= cache_size))
      // out of memory. remove units from bottom of list.
      Unload (tail);

    // now load the unit.
    num++;
    total_size += txt_mm->size;
    peak_total_size = MAX(peak_total_size, total_size);

    cached_texture = new csTxtCacheData;

    cached_texture->next = head;
    cached_texture->prev = 0;
    if (head)
      head->prev = cached_texture;
    else
      tail = cached_texture;
    head = cached_texture;
    cached_texture->Source = txt_handle;
    cached_texture->Size = txt_mm->size;

    txt_handle->SetCacheData (cached_texture);
    Load (cached_texture);              // load it.
  }
}

void OpenGLTextureCache::Clear ()
{
  while (head)
    Unload (head);

  CS_ASSERT (!head);
  CS_ASSERT (!tail);
  CS_ASSERT (!total_size);
  CS_ASSERT (!num);
}

void OpenGLTextureCache::Uncache (iTextureHandle *texh)
{
  csTxtCacheData *cached_texture = (csTxtCacheData *)texh->GetCacheData ();
  if (cached_texture)
    Unload (cached_texture);
}

void OpenGLTextureCache::Load (csTxtCacheData *d, bool reload)
{
  iTextureHandle *txt_handle = (iTextureHandle *)d->Source;
  csTextureHandleOpenGL *txt_mm = (csTextureHandleOpenGL *)
    txt_handle->GetPrivateObject ();

  if (reload)
  {
    csGraphics3DOGLCommon::statecache->SetTexture (GL_TEXTURE_2D, d->Handle);
  }
  else
  {
    GLuint texturehandle;

    glGenTextures (1, &texturehandle);
    d->Handle = texturehandle;
    csGraphics3DOGLCommon::statecache->SetTexture (
      GL_TEXTURE_2D, texturehandle);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  }

  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
    rstate_bilinearmap ? GL_LINEAR : GL_NEAREST);
  if (((txt_mm->GetFlags () & (CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS))
  	== CS_TEXTURE_3D))
  {
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
      rstate_bilinearmap ? GL_LINEAR_MIPMAP_LINEAR
                         : GL_NEAREST_MIPMAP_NEAREST);
  }
  else
  {
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
      rstate_bilinearmap ? GL_LINEAR : GL_NEAREST);
  }
  if (g3d->EXT_texture_filter_anisotropic)
  {
    glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT,
      g3d->txtmgr->texture_filter_anisotropy);
  }

  glGetError ();

  for (int i=0; i < txt_mm->vTex.Length (); i++)
  {
    csTextureOpenGL *togl = txt_mm->vTex[i];
    if (togl->compressed == GL_FALSE)
    {
      glTexImage2D (GL_TEXTURE_2D, i, txt_mm->TargetFormat (),
        togl->get_width (), togl->get_height (),
	0, txt_mm->SourceFormat (), txt_mm->SourceType (), togl->image_data);
      g3d->CheckGLError ("glTexImage2D()");
    }
    else
    {
      csGraphics3DOGLCommon::glCompressedTexImage2DARB (
        GL_TEXTURE_2D, i, (GLenum)togl->internalFormat,
	togl->get_width (), togl->get_height (), 0,
	togl->size, togl->image_data);
      g3d->CheckGLError ("glCompressedTexImage2DARB()");
    }
  }
}


