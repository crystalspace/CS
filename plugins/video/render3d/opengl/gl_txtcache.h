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

#ifndef __CS_GL_TEXTURECACHE_H__
#define __CS_GL_TEXTURECACHE_H__

#include "csutil/scf.h"
#include "csgeom/csrect.h"
#include "csutil/leakguard.h"

struct iTextureHandle;
struct iObjectRegistry;
class csGLGraphics3D;

/**
 * Cache element for a texture. This element will be stored
 * in the OpenGL texture cache and is also kept with the polygon
 * itself.
 */
struct csTxtCacheData
{
  CS_LEAKGUARD_DECLARE (csTxtCacheData);

  /// The size of this texture.
  long Size;
  /// iTextureHandle.
  csRef<iTextureHandle> Source; // @@@ Why does * crash?
  /// GL texture handle.
  GLuint Handle;
  /// Linked list.
  csTxtCacheData *next, *prev;
};

SCF_VERSION ( iGLTextureCache, 0,0,1)
/**
 * Simple interface just to be able to marshal across plugin-border
 */
struct iGLTextureCache : iBase
{
  /// Make sure this texture is active in OpenGL.
  virtual void Cache (iTextureHandle *texture) = 0;
  /// Remove an individual texture from cache.
  virtual void Uncache (iTextureHandle *texh) = 0;
};

/**
 * This is the OpenGL texture cache.
 */
class csGLTextureCache : iGLTextureCache
{
private:
  csGLGraphics3D* G3D;
protected:
  int rstate_bilinearmap;

  /// the head and tail of the cache data
  csTxtCacheData *head, *tail;

  /// the maximum size of the cache
  long cache_size;
  /// number of items
  int num;
  /// Total size of all loaded textures
  long total_size;
public:
  SCF_DECLARE_IBASE;
  /// Takes the maximum size of the cache.
  csGLTextureCache (int max_size, csGLGraphics3D* G3D);
  ///
  virtual ~csGLTextureCache ();

  /// Make sure this texture is active in OpenGL.
  virtual void Cache (iTextureHandle *texture);
  /// Remove an individual texture from cache.
  virtual void Uncache (iTextureHandle *texh);

  /// Clear the cache.
  void Clear ();

  ///
  void SetBilinearMapping (int m) { rstate_bilinearmap = m; }
  ///
  int GetBilinearMapping () const { return rstate_bilinearmap; }

protected:

  /// Really load the texture in OpenGL memory.
  void Load (csTxtCacheData *d, bool reload = false);
  ///
  void Unload (csTxtCacheData *d);
};

#endif // __CS_GL_TEXTURECACHE_H__

