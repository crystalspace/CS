/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein
    Copyright (C) 1998 by Dan Ogles.
    Copyright (C) 2003 by Marten Svanfeldt

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

#include "ivaria/reporter.h"

#include "gl_render3d.h"
#include "gl_txtcache.h"
#include "gl_txtmgr.h"
#include "plugins/video/canvas/openglcommon/glextmanager.h"

// need definitions of R24(), G24(), and B24()
#ifndef CS_NORMAL_LIGHT_LEVEL
#define CS_NORMAL_LIGHT_LEVEL 128
#endif

SCF_IMPLEMENT_IBASE(csGLTextureCache)
  SCF_IMPLEMENTS_INTERFACE(iGLTextureCache)
SCF_IMPLEMENT_IBASE_END

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

  csGLTextureHandle *txt_mm = (csGLTextureHandle *)
    d->Source->GetPrivateObject ();
  if (txt_mm->target == iTextureHandle::CS_TEX_IMG_1D)
    csGLTextureManager::UnsetTexture (GL_TEXTURE_1D, d->Handle);
  else if (txt_mm->target == iTextureHandle::CS_TEX_IMG_2D)
    csGLTextureManager::UnsetTexture (GL_TEXTURE_2D, d->Handle);
  else if (txt_mm->target == iTextureHandle::CS_TEX_IMG_3D)
    csGLTextureManager::UnsetTexture (GL_TEXTURE_3D, d->Handle);
  else if (txt_mm->target == iTextureHandle::CS_TEX_IMG_CUBEMAP)
    csGLTextureManager::UnsetTexture (GL_TEXTURE_CUBE_MAP, d->Handle);
  glDeleteTextures (1, &d->Handle);
  d->Handle = 0;

  num--;
  total_size -= d->Size;

  iTextureHandle *texh = (iTextureHandle *)d->Source;
  if (texh) texh->SetCacheData (0);

  delete d;
}

//----------------------------------------------------------------------------//

csGLTextureCache::csGLTextureCache (int max_size, csGLGraphics3D* G3D)
{
  SCF_CONSTRUCT_IBASE(0);
  rstate_bilinearmap = 2;
  cache_size = max_size;
  num = 0;
  head = tail = 0;
  total_size = 0;
  (csGLTextureCache::G3D = G3D)/*->IncRef ()*/;
}

csGLTextureCache::~csGLTextureCache ()
{
  Clear ();
  //G3D->DecRef ();
  SCF_DESTRUCT_IBASE();
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
  static const GLenum textureMinFilters[3] = {GL_NEAREST_MIPMAP_NEAREST, 
    GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR};
  static const GLenum textureMagFilters[3] = {GL_NEAREST, GL_LINEAR, 
    GL_LINEAR};

  iTextureHandle *txt_handle = (iTextureHandle *)d->Source;
  csGLTextureHandle *txt_mm = (csGLTextureHandle *)
    txt_handle->GetPrivateObject ();

  if (reload)
  {
    if(txt_mm->target == iTextureHandle::CS_TEX_IMG_1D)
      G3D->statecache->SetTexture (GL_TEXTURE_1D, d->Handle);
    else if(txt_mm->target == iTextureHandle::CS_TEX_IMG_2D)
      G3D->statecache->SetTexture (GL_TEXTURE_2D, d->Handle);
    else if(txt_mm->target == iTextureHandle::CS_TEX_IMG_3D)
      G3D->statecache->SetTexture (GL_TEXTURE_3D, d->Handle);
    else if(txt_mm->target == iTextureHandle::CS_TEX_IMG_CUBEMAP)
      G3D->statecache->SetTexture (GL_TEXTURE_CUBE_MAP, d->Handle);
  }
  else
  {
    GLuint texturehandle;
    glGenTextures (1, &texturehandle);

    d->Handle = texturehandle;
    const int texFilter = txt_mm->flags & CS_TEXTURE_NOFILTER ? 0 : 
      rstate_bilinearmap;

    if(txt_mm->target == iTextureHandle::CS_TEX_IMG_1D)
    {
      G3D->statecache->SetTexture (GL_TEXTURE_1D, d->Handle);
      if (txt_mm->flags & CS_TEXTURE_CLAMP)
      {
        glTexParameteri (GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      }
      else
      {
        glTexParameteri (GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      }

      glTexParameteri (GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER,
	textureMagFilters[texFilter]);

      glTexParameteri (GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER,
        textureMinFilters[texFilter]);

      if (G3D->ext->CS_GL_EXT_texture_filter_anisotropic)
      {
        glTexParameterf (GL_TEXTURE_1D, GL_TEXTURE_MAX_ANISOTROPY_EXT,
          G3D->txtmgr->texture_filter_anisotropy);
      }
    }
    else if(txt_mm->target == iTextureHandle::CS_TEX_IMG_2D)
    {
      G3D->statecache->SetTexture (GL_TEXTURE_2D, d->Handle);
      if (txt_mm->flags & CS_TEXTURE_CLAMP)
      {
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      }
      else
      {
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      }

      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
        textureMagFilters[texFilter]);

      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
        (txt_mm->flags & CS_TEXTURE_NOMIPMAPS) ?
          textureMagFilters[texFilter] : textureMinFilters[texFilter]);

      if (G3D->ext->CS_GL_EXT_texture_filter_anisotropic)
      {
        glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT,
          G3D->txtmgr->texture_filter_anisotropy);
      }
    }
    else if(txt_mm->target == iTextureHandle::CS_TEX_IMG_3D)
    {
      glEnable (GL_TEXTURE_3D);
      G3D->statecache->SetTexture (GL_TEXTURE_3D, d->Handle);
      if (txt_mm->flags & CS_TEXTURE_CLAMP)
      {
        glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
      }
      else
      {
        glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
      }

      // @@@ Not sure if the following makes sense with 3D textures.
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
        textureMagFilters[texFilter]);

      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
        textureMinFilters[texFilter]);

      if (G3D->ext->CS_GL_EXT_texture_filter_anisotropic)
      {
        glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT,
          G3D->txtmgr->texture_filter_anisotropy);
      }
    }
    else if(txt_mm->target == iTextureHandle::CS_TEX_IMG_CUBEMAP)
    {
      G3D->statecache->SetTexture (GL_TEXTURE_CUBE_MAP, d->Handle);
      // @@@ Temporarily disabled, although I don't know if REPEAT
      // makes sense with cubemaps.
      /*if (txt_mm->flags & CS_TEXTURE_CLAMP)
      {*/
        glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, 
          GL_CLAMP_TO_EDGE);
        glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, 
          GL_CLAMP_TO_EDGE);
        glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, 
          GL_CLAMP_TO_EDGE);
      /*}
      else 
      {
        glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_REPEAT);
      }*/

      // @@@ Not sure if the following makes sense with cubemaps.
      glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER,
        textureMinFilters[texFilter]);

      glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,
        (txt_mm->flags & CS_TEXTURE_NOMIPMAPS)?
          textureMagFilters[texFilter] :
          textureMinFilters[texFilter]);

      if (G3D->ext->CS_GL_EXT_texture_filter_anisotropic)
      {
        glTexParameterf (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_EXT,
          G3D->txtmgr->texture_filter_anisotropy);
      }
    }
  }


  if(txt_mm->target == iTextureHandle::CS_TEX_IMG_1D)
  {
    // @@@ Not implemented yet
  }
  else if(txt_mm->target == iTextureHandle::CS_TEX_IMG_2D)
  {
    int i;
    for (i = 0; i < txt_mm->vTex.Length(); i++)
    {
      csGLTexture *togl = txt_mm->vTex[i];
      if (togl->compressed == GL_FALSE)
      {
	glTexImage2D (
	  GL_TEXTURE_2D, i, 
	  txt_mm->TargetFormat(),
	  togl->get_width(), 
	  togl->get_height(),
	  0, 
	  txt_mm->SourceFormat(), 
	  txt_mm->SourceType(), 
	  togl->image_data);
      }
      else
      {
	G3D->ext->glCompressedTexImage2DARB (
	  GL_TEXTURE_2D, 
	  i, 
	  (GLenum)togl->internalFormat,
	  togl->get_width(), 
	  togl->get_height(), 
	  0,
	  togl->size, 
	  togl->image_data);
	//g3d->CheckGLError ("glCompressedTexImage2DARB()");
      }
    }
  }
  else if(txt_mm->target == iTextureHandle::CS_TEX_IMG_3D)
  {
    for (int i=0; i < txt_mm->vTex.Length (); i++)
    {
      csGLTexture *togl = txt_mm->vTex[i];
      G3D->ext->glTexImage3DEXT (GL_TEXTURE_3D, 
                    i,
                    txt_mm->TargetFormat (),
                    togl->get_width (),
                    togl->get_height (),
                    togl->get_depth (),
                    0,
                    txt_mm->SourceFormat (),
                    txt_mm->SourceType (),
                    togl->image_data);
    }
  }
  else if(txt_mm->target == iTextureHandle::CS_TEX_IMG_CUBEMAP)
  {
    for (int i=0; i < txt_mm->vTex.Length (); i++)
    {
      csGLTexture *togl = txt_mm->vTex[i];
      // TODO: load cubemaps into GL texture 

      // >>= 3 == /= 8 ... turning bits per pixel int bytes per pixel
      // componentcount

      int compcount = csGLTextureManager::glformats[txt_mm->formatidx]
      	.components;
      int cursize = togl->get_width() * togl->get_height () * compcount;

      uint8 *data = togl->image_data;

      int j;
      for(j = 0; j < 6; j++)
      {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j,
                      i,
                      /*txt_mm->TargetFormat ()*/GL_RGBA8,
                      togl->get_width (),
                      togl->get_height(),
                      0,
                      txt_mm->SourceFormat (),
                      txt_mm->SourceType (),
                      data);
        data += cursize;
      }
    }
  }
}
