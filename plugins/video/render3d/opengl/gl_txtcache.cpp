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

#include "imesh/thing/polygon.h"  //@@@!!!
#include "ivaria/reporter.h"

#include "gl_render3d.h"
#include "gl_txtcache.h"
#include "gl_txtmgr.h"


// need definitions of R24(), G24(), and B24()
#ifndef CS_NORMAL_LIGHT_LEVEL
#define CS_NORMAL_LIGHT_LEVEL 128
#endif

//---------------------------------------------------------------------------//

/// Unload a texture cache element (common for both caches)
void csGLTextureCache::Unload (csTxtCacheData *d)
{
  if (d->next)
    d->next->prev = d->prev;
  else
    tail = d->prev;

  if (d->prev)
    d->prev->next = d->next;
  else
    head = d->next;

  glDeleteTextures (1, &d->Handle);
  d->Handle = 0;

  num--;
  total_size -= d->Size;

  iTextureHandle *texh = (iTextureHandle *)d->Source;
  if (texh) texh->SetCacheData (NULL);

  delete d;
}

//----------------------------------------------------------------------------//

csGLTextureCache::csGLTextureCache (int max_size,
  csGLRender3D* R3D)
{
  cache_size = max_size;
  num = 0;
  head = tail = NULL;
  total_size = 0;
  (csGLTextureCache::R3D = R3D)->IncRef ();
}

csGLTextureCache::~csGLTextureCache ()
{
  Clear ();
  R3D->DecRef ();
}

void csGLTextureCache::Cache (iTextureHandle *txt_handle)
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

      cached_texture->prev = NULL;
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
    csGLTextureHandle *txt_mm = (csGLTextureHandle *)
      txt_handle->GetPrivateObject ();

    // unit is not in memory. load it into the cache
    while (total_size + txt_mm->size >= cache_size)
      // out of memory. remove units from bottom of list.
      Unload (tail);

    // now load the unit.
    num++;
    total_size += txt_mm->size;

    cached_texture = new csTxtCacheData;

    cached_texture->next = head;
    cached_texture->prev = NULL;
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

void csGLTextureCache::Clear ()
{
  while (head)
    Unload (head);

  CS_ASSERT (!head);
  CS_ASSERT (!tail);
  CS_ASSERT (!total_size);
  CS_ASSERT (!num);
}

void csGLTextureCache::Uncache (iTextureHandle *texh)
{
  csTxtCacheData *cached_texture = (csTxtCacheData *)texh->GetCacheData ();
  if (cached_texture)
    Unload (cached_texture);
}

void csGLTextureCache::Load (csTxtCacheData *d, bool reload)
{
  iTextureHandle *txt_handle = (iTextureHandle *)d->Source;
  csGLTextureHandle *txt_mm = (csGLTextureHandle *)
    txt_handle->GetPrivateObject ();

  if (reload)
  {
    glBindTexture (GL_TEXTURE_2D, d->Handle);
  }
  else
  {
    GLuint texturehandle;

    glGenTextures (1, &texturehandle);
    d->Handle = texturehandle;
    glBindTexture (
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

  for (int i=0; i < txt_mm->vTex.Length (); i++)
  {
    csGLTexture *togl = txt_mm->vTex[i];
    if (togl->compressed == GL_FALSE)
      glTexImage2D (GL_TEXTURE_2D, i, txt_mm->TargetFormat (),
        togl->get_width (), togl->get_height (),
  0, txt_mm->SourceFormat (), txt_mm->SourceType (), togl->image_data);
    else
      R3D->ext.glCompressedTexImage2DARB (
        GL_TEXTURE_2D, i, (GLenum)togl->internalFormat,
  togl->get_width (), togl->get_height (), 0,
  togl->size, togl->image_data);
  }
}

